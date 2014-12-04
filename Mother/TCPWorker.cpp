// TCPWorker.cpp: implementation of the TCPWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "TCPWorker.h"
#include <algorithm>

#ifdef WIN32
#include <io.h>
#include <direct.h>
#include <time.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TCPWorker::TCPWorker(char *cfgFile)
:m_cfgFile( cfgFile )
{
	m_log.SetLogName( "Mother" );
	m_log.SetPrintLog( true );
	m_log.SetMaxLogSize( 10 );
	m_log.SetMaxExistDay( 30 );

	ReadConfig();
	SetAverageConnectCount(1000);
	SetHeartTime( m_heartTime );
	Listen(m_svrPort);
}

TCPWorker::~TCPWorker()
{
}

bool TCPWorker::ReadConfig()
{
	m_svrPort = m_cfgFile["opt"]["port"];
	m_heartTime = m_cfgFile["opt"]["heartTime"];
	if ( 0 >= m_svrPort ) m_svrPort = 7250;
	if (  120 > m_heartTime ) m_heartTime = 120;

	m_existCount = m_cfgFile["Exist"]["count"];
	m_hardDiskCount = m_cfgFile["Hard disk"]["count"];
	
	return true;
}

mdk::Logger& TCPWorker::GetLog()
{
	return m_log;
}

/*
 *	���������߳�
 *  ��ʱд�ļ�
 */
int TCPWorker::Main()
{
	time_t tCurTime = time( NULL );
	if ( false )
	{
		exit(0);
	}

	return true;
}

void TCPWorker::OnCloseConnect(mdk::STNetHost &host)
{
	DeviceMap::iterator it = m_hardDisks.begin();
	for ( ; it != m_hardDisks.end(); it++ ) 
	{
		if ( it->second.host.ID() == host.ID() )
		{
			m_log.Info( "Error", "%s(%s %d-%s %d)δ���", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
			m_hardDisks.erase( it );
			return;
		}
	}


	it = m_exists.begin();
	for ( ; it != m_exists.end(); it++ ) 
	{
		if ( it->second.host.ID() == host.ID() )
		{
			m_log.Info( "Error", "%s(%s %d-%s %d)δ���", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
			m_exists.erase( it );
			return;
		}
	}

	it = m_screens.begin();
	for ( ; it != m_screens.end(); it++ ) 
	{
		if ( it->second.host.ID() == host.ID() )
		{
			m_log.Info( "Error", "%s(%s %d-%s %d)δ���", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
			m_screens.erase( it );
			return;
		}
	}

	it = m_cpus.begin();
	for ( ; it != m_cpus.end(); it++ ) 
	{
		if ( it->second.host.ID() == host.ID() )
		{
			m_log.Info( "Error", "%s(%s %d-%s %d)δ���", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
			m_cpus.erase( it );
			return;
		}
	}


	return;
}

void TCPWorker::OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len)
{
	std::string ip;
	int port;
	if ( host.IsServer() ) host.GetServerAddress( ip, port );
	else host.GetAddress( ip, port );
	m_log.StreamInfo( "Error", msg, len, "����(%s-%d)���ͷǷ���ʽ����", ip.c_str(), port );
	host.Close();
}

/**
 * ���ݵ���ص�����
 * 
 * ������ʵ�־���Ͽ�����ҵ����
 * 
*/
void TCPWorker::OnWork( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData )
{
	bool isValidMsg = true;
	switch( header->msgId )
	{
	case MsgId::heartbeat://����
		break;
	case MsgId::plugInQuery ://�豸�������
		isValidMsg = OnPlugIn(host, header, (MSG_PLUG_IN_QUERY*)pData);
		break;
	default:
		isValidMsg = false;
		break;
	}

	if ( !isValidMsg ) OnInvalidMsg( host, BStructSvr::paramError, (unsigned char*)header, header->msgSize + MSG_HEAD_SIZE );

	return;
}

void TCPWorker::SetDeviceId( mdk::STNetHost &host, unsigned char deviceId )
{
	MSG_SET_DEVICE_ID *pData = (MSG_SET_DEVICE_ID*)GetDataBuffer();
	pData->deviceId = deviceId;
	SendMsg( host, MsgId::setDeviceId, sizeof(MSG_SET_DEVICE_ID) );
}

void TCPWorker::RunDevice( Device::INFO &device )
{
	m_log.Info( "Run", "����%s(%s %d-%s %d)", Device::Descript( device.type ), 
		device.wanIP.c_str(), device.wanPort, device.lanIP.c_str(), device.lanPort ); 
	DevicePostion( device.host );

	SendMsg( device.host, MsgId::runDevice, 0 );
	device.status = Device::Status::running;
}

void TCPWorker::DevicePostion( mdk::STNetHost &host  )
{
	MSG_DEVICE_POSTION *pData = (MSG_DEVICE_POSTION*)GetDataBuffer();
	pData->size = 0;

	//���������
	std::vector<int> ids;
	DeviceMap::iterator it = m_exists.begin();
	for ( ids.clear(); it != m_exists.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//�����
	std::sort_heap(ids.begin(), ids.end());//������
	
	unsigned int i = 0;
	for ( i = 0; i < ids.size(); i++ )
	{
		pData->devices[pData->size].device = (char)m_exists[ids[i]].type;
		pData->devices[pData->size].deviceId = (unsigned char)m_exists[ids[i]].deviceId;
		strcpy( pData->devices[pData->size].wanIP, m_exists[ids[i]].wanIP.c_str() );
		pData->devices[pData->size].wanPort = m_exists[ids[i]].wanPort;
		strcpy( pData->devices[pData->size].lanIP, m_exists[ids[i]].lanIP.c_str() );
		pData->devices[pData->size].lanPort = m_exists[ids[i]].lanPort;
		pData->size++;
	}

	//���й�̬Ӳ��
	it = m_hardDisks.begin();
	for ( ids.clear(); it != m_hardDisks.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//�����
	std::sort_heap(ids.begin(), ids.end());//������
	
	for ( i = 0; i < ids.size(); i++ )
	{
		pData->devices[pData->size].device = (char)m_hardDisks[ids[i]].type;
		pData->devices[pData->size].deviceId = (unsigned char)m_hardDisks[ids[i]].deviceId;
		strcpy( pData->devices[pData->size].wanIP, m_hardDisks[ids[i]].wanIP.c_str() );
		pData->devices[pData->size].wanPort = m_hardDisks[ids[i]].wanPort;
		strcpy( pData->devices[pData->size].lanIP, m_hardDisks[ids[i]].lanIP.c_str() );
		pData->devices[pData->size].lanPort = m_hardDisks[ids[i]].lanPort;
		pData->size++;
	}

	//���д�����
	it = m_screens.begin();
	for ( ids.clear(); it != m_screens.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//�����
	std::sort_heap(ids.begin(), ids.end());//������
	
	for ( i = 0; i < ids.size(); i++ )
	{
		pData->devices[pData->size].device = (char)m_screens[ids[i]].type;
		pData->devices[pData->size].deviceId = (unsigned char)m_screens[ids[i]].deviceId;
		strcpy( pData->devices[pData->size].wanIP, m_screens[ids[i]].wanIP.c_str() );
		pData->devices[pData->size].wanPort = m_screens[ids[i]].wanPort;
		strcpy( pData->devices[pData->size].lanIP, m_screens[ids[i]].lanIP.c_str() );
		pData->devices[pData->size].lanPort = m_screens[ids[i]].lanPort;
		pData->size++;
	}
	//����CPU
	it = m_cpus.begin();
	for ( ids.clear(); it != m_cpus.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//�����
	std::sort_heap(ids.begin(), ids.end());//������
	
	for ( i = 0; i < ids.size(); i++ )
	{
		pData->devices[pData->size].device = (char)m_cpus[ids[i]].type;
		pData->devices[pData->size].deviceId = (unsigned char)m_cpus[ids[i]].deviceId;
		strcpy( pData->devices[pData->size].wanIP, m_cpus[ids[i]].wanIP.c_str() );
		pData->devices[pData->size].wanPort = m_cpus[ids[i]].wanPort;
		strcpy( pData->devices[pData->size].lanIP, m_cpus[ids[i]].lanIP.c_str() );
		pData->devices[pData->size].lanPort = m_cpus[ids[i]].lanPort;
		pData->size++;
	}

	SendMsg( host, MsgId::devicePostion, sizeof(MSG_DEVICE_POSTION) );
}

//�豸�������
bool TCPWorker::OnPlugIn(mdk::STNetHost &host, MSG_HEADER *header, MSG_PLUG_IN_QUERY *pData)
{
	//////////////////////////////////////////////////////////////////////////
	/*
		��鱨�ĺϷ���
		1.���е��ֶ���û��
		2.�����Ƿ���Э��Լ�����
		3.�ַ����ȱ䳤�ֶγ��ȱ��벻С��1
	*/
	if ( header->msgSize != sizeof(MSG_PLUG_IN_QUERY) ) return false;
	Device::Type::Type device = (Device::Type::Type)pData->device;
	int deviceId = pData->deviceId;
	//////////////////////////////////////////////////////////////////////////
	//ҵ����
	/*
		�����豸id�������
		���ղ���˳�������
		���ݷ�Ƭʱ����Ϊ���͹�̬Ӳ�̵�id��0~n�����ģ�
		���Բ��ɸı��id�������
	*/
	if ( Device::Type::ssd== pData->device ) //��̬Ӳ�̲���
	{
		if ( m_hardDiskCount == (int)m_hardDisks.size() ) 
		{
			m_log.Info( "Error", "Ӳ�̲���Ѳ������ܾ�Ӳ��(%s %d-%s %d)����", 
				pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort );
			host.Close();
			return true;
		}
		if ( -1 != pData->deviceId ) 
		{
			DeviceMap::iterator it =  m_hardDisks.find( deviceId );
			if ( it != m_hardDisks.end() )
			{
				m_log.Info( "Error", "Ӳ���豸ID�ظ����ܾ�Ӳ��(%s %d-%s %d)����", 
					pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort );
				host.Close();
				return true;
			}
		}
		else 
		{
			deviceId = m_hardDisks.size();
			while ( m_hardDisks.end() != m_hardDisks.find(deviceId) ) deviceId++;
			SetDeviceId( host, deviceId );
		}

		m_hardDisks[deviceId].deviceId = deviceId;
		m_hardDisks[deviceId].host = host;
		m_hardDisks[deviceId].lanIP = pData->lanIP;
		m_hardDisks[deviceId].lanPort = pData->lanPort;
		m_hardDisks[deviceId].wanIP = pData->wanIP;
		m_hardDisks[deviceId].wanPort = pData->wanPort;
		m_hardDisks[deviceId].type = device;
		m_hardDisks[deviceId].status = Device::Status::running;
		m_log.Info( "Run", "%s(%s %d-%s %d)����", Device::Descript( device ), 
			pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort ); 
		RunDevice( m_hardDisks[deviceId] );//Ӳ�̲���Ҫ�κ�֧���豸1����Ϳ��Կ�ʼ����

		if ( m_hardDiskCount == (int)m_hardDisks.size() ) //����Ӳ�������У��������еȴ��е������
		{
			m_log.Info( "Run", "%d��%s��ȫ������", m_hardDiskCount, Device::Descript( device ) ); 
			DeviceMap::iterator it = m_exists.begin();
			for ( ; it != m_exists.end(); it++ )
			{
				if ( Device::Status::waitDevice == it->second.status ) 
				{
					RunDevice( it->second );
				}
			}
		}

		return true;
	}

	if ( Device::Type::exist == device ) //���������
	{
		if ( m_existCount == (int)m_exists.size() ) 
		{
			m_log.Info( "Error", "���������Ѳ������ܾ������(%s %d-%s %d)����", 
				pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort );
			host.Close();
			return true;
		}

		if ( -1 != deviceId ) 
		{
			DeviceMap::iterator it =  m_exists.find( deviceId );
			if ( it != m_exists.end() )
			{
				m_log.Info( "Error", "������豸ID�ظ����ܾ������(%s %d-%s %d)����", 
					pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort );
				host.Close();
				return true;
			}
		}
		else 
		{
			deviceId = m_exists.size();
			while ( m_exists.end() != m_exists.find(deviceId) ) deviceId++;
			SetDeviceId( host, deviceId );
		}

		m_exists[deviceId].deviceId = deviceId;
		m_exists[deviceId].host = host;
		m_exists[deviceId].lanIP = pData->lanIP;
		m_exists[deviceId].lanPort = pData->lanPort;
		m_exists[deviceId].wanIP = pData->wanIP;
		m_exists[deviceId].wanPort = pData->wanPort;
		m_exists[deviceId].type = device;
		m_exists[deviceId].status = Device::Status::waitDevice;
		m_log.Info( "Run", "%s(%s %d-%s %d)����", Device::Descript( device ), 
			pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort ); 

		if ( m_hardDiskCount == (int)m_hardDisks.size() ) //�������ҪӲ�����־û�֧�֣�Ҫ������ָ��������Ӳ�̶������ˣ����ܿ�ʼ����
		{
			RunDevice( m_exists[deviceId] );
		}

		if ( m_existCount == (int)m_exists.size() ) //CPU��Ҫ�����������֧�֣�Ҫ������ָ��������������������ˣ����ܿ�ʼ����
		{
			m_log.Info( "Run", "%d��%s��ȫ������", m_existCount, Device::Descript( device ) ); 
			DeviceMap::iterator it = m_cpus.begin();
			for ( ; it != m_cpus.end(); it++ )
			{
				if ( Device::Status::waitDevice == it->second.status ) 
				{
					RunDevice( it->second );
				}
			}
		}
		return true;
	}

	if ( Device::Type::cpu== device ) //CPU����
	{
		if ( -1 != deviceId ) 
		{
			DeviceMap::iterator it =  m_cpus.find( deviceId );
			if ( it != m_cpus.end() )
			{
				m_log.Info( "Error", "CPU�豸ID�ظ����ܾ�CPU����" );
				host.Close();
				return true;
			}
		}
		else 
		{
			deviceId = m_cpus.size();
			while ( m_cpus.end() != m_cpus.find(deviceId) ) deviceId++;
			SetDeviceId( host, deviceId );
		}
		m_cpus[deviceId].deviceId = deviceId;
		m_cpus[deviceId].host = host;
		m_cpus[deviceId].type = device;
		m_cpus[deviceId].lanIP = pData->lanIP;
		m_cpus[deviceId].lanPort = pData->lanPort;
		m_cpus[deviceId].wanIP = pData->wanIP;
		m_cpus[deviceId].wanPort = pData->wanPort;
		m_cpus[deviceId].status = Device::Status::waitDevice;
		m_log.Info( "Run", "%s(%s %d-%s %d)����", Device::Descript( device ), 
			pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort ); 

		if ( m_existCount == (int)m_exists.size() ) //CPU��Ҫ�����������֧�֣�Ҫ������ָ��������������������ˣ����ܿ�ʼ����
		{
			RunDevice( m_cpus[deviceId] );
		}
		
		return true;
	}

	if ( Device::Type::screen== device || Device::Type::touch== device ) //��ʾ������
	{
		if ( -1 != deviceId ) 
		{
			DeviceMap::iterator it =  m_screens.find( deviceId );
			if ( it != m_cpus.end() )
			{
				m_log.Info( "Error", "�������豸ID�ظ����ܾ�����������" );
				host.Close();
				return true;
			}
		}
		else 
		{
			deviceId = m_screens.size();
			while ( m_screens.end() != m_screens.find(deviceId) ) deviceId++;
			SetDeviceId( host, deviceId );
		}
		m_screens[deviceId].deviceId = deviceId;
		m_screens[deviceId].host = host;
		m_screens[deviceId].type = device;
		m_screens[deviceId].lanIP = pData->lanIP;
		m_screens[deviceId].lanPort = pData->lanPort;
		m_screens[deviceId].wanIP = pData->wanIP;
		m_screens[deviceId].wanPort = pData->wanPort;
		m_screens[deviceId].status = Device::Status::waitDevice;
		m_log.Info( "Run", "%s(%s %d-%s %d)����", Device::Descript( device ), 
			pData->wanIP, pData->wanPort, pData->lanIP, pData->lanPort ); 
		RunDevice( m_screens[deviceId] );//����������Ҫ�κ�֧���豸1����Ϳ��Կ�ʼ����

		return true;
	}

	return true;
}

