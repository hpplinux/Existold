// ConnectPool.cpp: implementation of the ConnectPool class.
//
//////////////////////////////////////////////////////////////////////

#include "../../../include/Exist/frame/ConnectPool.h"
#include "mdk/mapi.h"
#include "mdk/ConfigFile.h"
#include "../../../common/Protocol.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

unsigned char g_buf[256];
namespace Exist
{
	
ConnectPool::ConnectPool()
{
	m_sockInited = false;
	m_log.SetLogName( "cpu" );
	m_log.SetPrintLog( true );
	m_log.SetMaxLogSize( 10 );
	m_log.SetMaxExistDay( 30 );

	m_cpu.deviceId = -1;
	m_cpu.type = Device::Type::cpu;
	m_cpu.status = Device::Status::unPlugIn;
}

ConnectPool::~ConnectPool()
{
	if ( m_sockInited ) mdk::Socket::SocketDestory();
}

bool ConnectPool::PlugIn()
{
	mdk::Socket::SocketInit();	
	m_sockInited = true;

	m_guideThread.Run( mdk::Executor::Bind(&ConnectPool::GuideThread), this, 0 );
	m_sigInited.Wait();

	return true;
}

void* ConnectPool::GuideThread( void * )
{
	char exeDir[256];
	int size = 256;
	mdk::GetExeDir( exeDir, size );//ȡ�ÿ�ִ�г���λ��
	char configFile[256];
	sprintf( configFile, "%s/cpu.cfg", exeDir );
	
	mdk::ConfigFile cfg;
	if ( !cfg.ReadConfig( configFile ) )
	{
		m_log.Info( "Error", "������[%s]ʧ��", configFile );
		mdk::mdk_assert( false );
		return NULL;
	}

	if ( !cfg["opt"]["id"].IsNull() ) 
	{
		m_cpu.deviceId = cfg["opt"]["id"];
		m_log.Info( "Run", "�ҵ��豸ID:%d", m_cpu.deviceId );
	}
	m_cpu.wanIP = (std::string)cfg["opt"]["wan ip"];
	m_cpu.wanPort = cfg["opt"]["wan port"];
	m_cpu.lanIP = (std::string)cfg["opt"]["lan ip"];
	m_cpu.lanPort = cfg["opt"]["lan port"];
	std::string ip = cfg["Mother board"]["ip"];
	int port = cfg["Mother board"]["port"];

	unsigned char *outBuf = new unsigned char[MAX_MSG_SIZE];
	unsigned char *inBuf = new unsigned char[MAX_MSG_SIZE];
	mdk::Socket	board;
	while ( true )
	{
		if ( !board.Init( mdk::Socket::tcp ) )
		{
			m_log.Info( "Error", "�����׽���ʧ��" );
			mdk::mdk_assert( false );
			return NULL;
		}
		
		m_log.Info( "Run", "Ѱ������" );
		while ( !board.Connect( ip.c_str(), port ) )
		{
			m_log.Info( "Error", "δ�ҵ�����,1�������" );
			mdk::m_sleep( 1000 );
		}

		m_log.Info( "Run", "��������" );
		MSG_PLUG_IN_QUERY *pData = (MSG_PLUG_IN_QUERY*)GetDataBuffer(outBuf);
		pData->deviceId = -1;
		if ( !cfg["opt"]["id"].IsNull() ) 
		{
			pData->deviceId = m_cpu.deviceId;
		}
		pData->device = (unsigned char)Device::Type::cpu;
		strcpy( pData->wanIP, m_cpu.wanIP.c_str() );
		pData->wanPort = m_cpu.wanPort;
		strcpy( pData->lanIP, m_cpu.lanIP.c_str() );
		pData->lanPort = m_cpu.lanPort;
		SendMsg( board, MsgId::plugInQuery, (unsigned char*)pData, sizeof(MSG_PLUG_IN_QUERY) );

		MSG_HEADER header;
		int ret = 0;
		while ( true )
		{
			ret = Exist::Recv(board, header, inBuf );
			if ( 1 == ret ) m_log.Info( "Error", "����ɶ�" );
			else if ( 2 == ret ) m_log.Info( "Error", "���巢���źų���" );
			else if ( 3 == ret ) m_log.Info( "Error", "���巢�ͷǷ��ź�" );
			else if ( 4 == ret ) m_log.Info( "Error", "�����ź�ȱ�ٲ���MsgId" );
			if ( 0 != ret ) 
			{
				board.Close();
				break;
			}

			switch ( header.msgId )
			{
			case MsgId::setDeviceId :
				if ( header.msgSize == sizeof(MSG_SET_DEVICE_ID) )
				{
					if ( !cfg["opt"]["id"].IsNull() )
					{
						m_log.Info( "Waring", "�����豸ID:%d���ܾ��޸�", m_cpu.deviceId );
						continue;
					}
					MSG_SET_DEVICE_ID *pData = (MSG_SET_DEVICE_ID*)GetDataBuffer(inBuf);
					cfg["opt"]["id"] = m_cpu.deviceId = pData->deviceId;
					cfg.Save();
					m_log.Info( "Run", "�����豸ID:%d", m_cpu.deviceId );
				}
				break;
			case MsgId::devicePostion :
				if ( Device::Status::unPlugIn != m_cpu.status ) continue;

				if ( header.msgSize == sizeof(MSG_DEVICE_POSTION) )
				{
					MSG_DEVICE_POSTION *pData = (MSG_DEVICE_POSTION*)GetDataBuffer(inBuf);
					int i = 0;
					Device::INFO ds;
					Device::Type::Type device;
					for ( i = 0; i < pData->size; i++ )
					{
						device = (Device::Type::Type)pData->devices[i].device;
						if ( Device::Type::exist != device ) continue;
						m_log.Info( "Run", "����%s��Ϣ", Device::Descript(Device::Type::exist) );
						ds.type = Device::Type::exist;
						ds.deviceId = pData->devices[i].deviceId;
						ds.wanIP = pData->devices[i].wanIP;
						ds.wanPort = pData->devices[i].wanPort;
						ds.lanIP = pData->devices[i].lanIP;
						ds.lanPort = pData->devices[i].lanPort;
						ds.status = Device::Status::running;
						m_existAdrs.push_back(ds);
					}
				}
				break;
			case MsgId::runDevice :
				if ( 0 != header.msgSize ) continue;
				m_log.Info( "Run", "%s��ʼ����", Device::Descript(m_cpu.type) );
				m_cpu.status = Device::Status::running;
  				m_sigInited.Notify();
				break;
			default:
				break;
			}
		}
		board.Detach();
	}
	delete[]outBuf;
	delete[]inBuf;

	return NULL;
}

mdk::Socket& ConnectPool::GetSocket( unsigned int hashId )
{
	mdk::uint64 threadId = mdk::CurThreadId();
	if ( Device::Status::unPlugIn == m_cpu.status )
	{
		m_log.Info( "Error", "δ������������������棿CPU�����ڵ�ǰ�߳�(%llu)����ǿ��ֹͣ������CPU��һ�η������ǰ������Exist::PlugIn()���������á�", threadId );
		mdk::mdk_assert( false );
	}
	
	mdk::AutoLock lock(&m_connectsLock);
	std::map<mdk::uint64, std::vector<mdk::Socket> >::iterator it = m_connectMap.find( threadId );
	if ( it == m_connectMap.end() )
	{
		mdk::Socket cpu;
		int i = 0;
		for ( i = 0; i < (int)m_existAdrs.size(); i++ ) m_connectMap[threadId].push_back( cpu );
	}
	std::vector<mdk::Socket> &cpus = m_connectMap[threadId];
	lock.Unlock();

	int index = hashId % m_existAdrs.size();

	mdk::Socket &cpu = cpus[index];
	if ( cpu.IsClosed() )
	{
		cpu.Init( mdk::Socket::tcp );
		Device::INFO adr = m_existAdrs[index];
		std::string ip;
		int port;
		if ( adr.wanIP == m_cpu.wanIP && adr.lanIP == m_cpu.lanIP )
		{
			ip = adr.lanIP;
			port = adr.lanPort;
		}
		else
		{
			ip = adr.wanIP;
			port = adr.lanPort;
		}

		while ( !cpu.Connect( ip.c_str(), port ) )
		{
			m_log.Info( "Error", "�����(%s %d - %s %d)�Ӵ�����,1������²��", adr.wanIP.c_str(), adr.wanPort, adr.lanIP.c_str(), adr.lanPort );
			mdk::m_sleep( 1000 );
		}
		cpu.SetNoDelay(true);
	}

	return cpu;
}

}
