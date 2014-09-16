// NoDB.cpp: implementation of the NoDB class.
//
//////////////////////////////////////////////////////////////////////

#include "NoDB.h"

#include <cstdio>
#include <cstdlib>
#include <io.h>
#include <direct.h>
#ifdef WIN32
#else
#include <unistd.h>
#endif

#ifdef WIN32
#else
#include <sys/time.h>
#endif

#include "../Micro-Development-Kit/include/mdk/Socket.h"
#include "../Micro-Development-Kit/include/mdk/mapi.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool NoDB::CreateDir( const char *strDir )
{
	std::string path = strDir;
	int startPos = 0;
	int pos = path.find( '\\', startPos );
	std::string dir;
	while ( true )
	{
		if ( -1 == pos ) dir = path;
		else dir.assign( path, 0, pos );
		if ( -1 == access( dir.c_str(), 0 ) )
		{
#ifdef WIN32
			if( 0 != mkdir(dir.c_str()) ) return false;
#else
			umask(0);
			if( 0 != mkdir(strDir, 0777) ) return false;
			umask(0);
			chmod(strDir,0777);
#endif
		}
		if ( -1 == pos ) break;
		startPos = pos + 1;
		pos = path.find( '\\', startPos );
	}
	
	return true;
}

bool NoDB::CreateFile( const char *strFile )
{
	if ( 0 == access( strFile, 0 ) ) 
	{
#ifndef WIN32
		umask(0);
		chmod(strFile,0777);
#endif
		return true;
	}
	
	std::string file = strFile;
	int pos = file.find_last_of( '\\', file.size() );
	if ( -1 != pos ) 
	{
		std::string dir;
		dir.assign( file, 0, pos );
		if ( !CreateDir(dir.c_str()) ) return false;
	}
	FILE *fp = fopen( strFile, "w" );
	if ( NULL == fp ) return false;
	fclose( fp );
#ifdef WIN32
#else
	umask(0);
	chmod(strFile,0777);
#endif
	return true;
}

NoDB::NoDB(char* cfgFile)
:m_cfgFile(cfgFile)
{
	m_log.SetLogName( "Exist-Exist" );
	m_log.SetPrintLog( true );
	m_log.SetMaxLogSize( 10 );
	m_log.SetMaxExistDay( 30 );

	m_context.device.status = Device::Status::unPlugIn;
#ifdef EXIST_DEVICE
	m_context.device.type = Device::Type::exist;
#else
	m_context.device.type = Device::Type::ssd;
#endif
	if ( !m_cfgFile["opt"]["id"].IsNull() )
	{
		int deviceId = m_cfgFile["opt"]["id"];
		if ( 0 > deviceId || deviceId > 255 )
		{
			m_log.Info( "Run", "��������ã��豸IDֻ����0��255", m_context.device.deviceId );
			exit(0);
		}
		m_context.device.deviceId = deviceId;
		m_log.Info( "Run", "�ҵ��豸ID:%d", m_context.device.deviceId );
	}
	else m_context.device.deviceId = -1;
	m_context.device.wanIP = (std::string)m_cfgFile["opt"]["wan ip"];
	m_context.device.wanPort = m_cfgFile["opt"]["wan port"];
	m_context.device.lanIP = (std::string)m_cfgFile["opt"]["lan ip"];
	m_context.device.lanPort = m_cfgFile["opt"]["lan port"];

	m_motherBoard.status = Device::Status::unknow;
	m_motherBoard.wanIP = (std::string)m_cfgFile["Mother"]["ip"];
	m_motherBoard.wanPort = m_cfgFile["Mother"]["port"];

	m_context.m_bStop = false;
	m_context.dataRootDir = (std::string)m_cfgFile["exist"]["data root dir"];
	m_context.maxMemory = m_cfgFile["exist"]["max memory"];

	if ( Device::Type::ssd == m_context.device.type ) CreateDir( m_context.dataRootDir.c_str() );
	m_nosqlDB.SetRemoteMode(true);

	if ( Device::Type::ssd == m_context.device.type ) ReadData();//Ӳ���������ǳ־û��ģ�����Ҫ�ȶ�ȡ����
	m_log.Info( "Run", "Ѱ������(%s %d)", m_motherBoard.wanIP.c_str(), m_motherBoard.wanPort );
	Connect( m_motherBoard.wanIP.c_str(), m_motherBoard.wanPort, 5 );//Ѱ������
}

NoDB::~NoDB()
{
}

mdk::Logger& NoDB::GetLog()
{
	return m_log;
}

/*
 *	���������߳�
 *  ��ʱд�ļ�
 */
int NoDB::Main()
{
	time_t tCurTime = time( NULL );
	Heartbeat( tCurTime );//��������
	if ( Device::Type::ssd == m_context.device.type ) SaveData( tCurTime );//��������

	if ( false )
	{
		exit(0);
	}
	return true;
}

//���ӵ�����Ӧ
void NoDB::OnConnect(mdk::STNetHost &host)
{
	std::string ip;
	int port;
	if ( host.IsServer() )
	{
		host.GetServerAddress( ip, port );
		
		//����������
		if ( ip == m_motherBoard.wanIP && port == m_motherBoard.wanPort )
		{
			m_motherBoard.host = host;
			m_motherBoard.status = Device::Status::running;
			//��������
			m_log.Info( "Run", "��������" );
			bsp::BStruct &msg = GetStruct();
			msg["MsgId"] = (unsigned short)MsgId::plugInQuery;
			if ( !m_cfgFile["opt"]["id"].IsNull() ) 
			{
				msg["deviceId"] = m_context.device.deviceId;
			}
			if (  Device::Type::ssd == m_context.device.type ) msg["device"] = (unsigned char)Device::Type::ssd;
			else msg["device"] = (unsigned char)Device::Type::exist;
			msg["wanIP"] = m_context.device.wanIP.c_str();
			msg["wanPort"] = m_context.device.wanPort;
			msg["lanIP"] = m_context.device.lanIP.c_str();
			msg["lanPort"] = m_context.device.lanPort;

			SendBStruct(m_motherBoard.host);
			return;
		}

		//������������ӹ�̬Ӳ��
		if ( m_context.device.type = Device::Type::exist )
		{
			mdk::uint64 ip64;
			mdk::addrToI64( ip64, ip.c_str(), port );
			IpMap::iterator it = m_hardDiskIpMap.find(ip64);
			if ( m_hardDiskIpMap.end() != it )
			{
				m_log.Info( "Run", "����%s(%s %d-%s %d)", Device::Descript(Device::Type::ssd), 
					it->second->wanIP.c_str(), it->second->wanPort, it->second->lanIP.c_str(), it->second->lanPort );
				it->second->host = host;
				return;
			}
		}

		m_log.Info( "Waring", "δ֪�豸(%s %d)���Ͽ�����", ip.c_str(), port );
		host.Close();
		return;
	}

	host.GetAddress( ip, port );
	if ( m_context.device.type = Device::Type::exist ) m_log.Info( "Run", "����CPU(%s %d)", ip.c_str(), port );
	else m_log.Info( "Run", "���������(%s %d)", ip.c_str(), port );

}

//���ӶϿ���Ӧ
void NoDB::OnCloseConnect(mdk::STNetHost &host)
{
	mdk::STNetHost emptyHost;
	std::string ip;
	int port;
	if ( host.IsServer() )
	{
		host.GetServerAddress( ip, port );
		
		//û�������
		if ( ip == m_motherBoard.wanIP && port == m_motherBoard.wanPort )
		{
			m_log.Info( "Error", "����ɶ�" );
			m_motherBoard.host = emptyHost;
			m_motherBoard.status = Device::Status::unknow;
			m_context.device.status = Device::Status::unPlugIn;
			return;
		}

		//��������ܶ�ʧ��̬Ӳ��
		if ( m_context.device.type = Device::Type::exist )
		{
			mdk::uint64 ip64;
			mdk::addrToI64( ip64, ip.c_str(), port );
			IpMap::iterator it = m_hardDiskIpMap.find(ip64);
			if ( m_hardDiskIpMap.end() != it )
			{
				m_log.Info( "Error", "%s(%s %d-%s %d)��ʧ", Device::Descript(Device::Type::ssd), 
					it->second->wanIP.c_str(), it->second->wanPort, it->second->lanIP.c_str(), it->second->lanPort );
				it->second->host = emptyHost;
				it->second->status = Device::Status::unknow;
				return;
			}
		}
		m_log.Info( "Waring", "�ѶϿ�δ֪�豸(%s %d)", ip.c_str(), port );

		return;
	}

	host.GetAddress( ip, port );
	if ( m_context.device.type = Device::Type::exist ) m_log.Info( "Run", "CPU(%s %d)��ʧ", ip.c_str(), port );
	else m_log.Info( "Run", "�����(%s %d)��ʧ", ip.c_str(), port );

	return;
}

void NoDB::OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len)
{
	std::string ip;
	int port;
	if ( host.IsServer() ) host.GetServerAddress( ip, port );
	else host.GetAddress( ip, port );
	m_log.StreamInfo( "Error", msg, len, "����(%s-%d)���ͷǷ��ź�", ip.c_str(), port );
	host.Close();
}

//���ݵ�����Ӧ
void NoDB::OnWork( mdk::STNetHost &host, bsp::BStruct &msg )
{
	if ( OnGuideMsg( host, msg ) ) return;
	OnClientMsg( host, msg );
}

//����Guide�����ϵ����ݲ��������host����Guide��ʲô������������false
bool NoDB::OnGuideMsg( mdk::STNetHost &host, bsp::BStruct &msg )
{
	if ( host.ID() != m_motherBoard.host.ID() ) return false;
	
	unsigned short msgid = msg["MsgId"];
	bool bIsInvalidMsg = true;//Ĭ��Ϊ�Ƿ�����
	switch( msgid )
	{
	case MsgId::setDeviceId :
		OnSetDeviceId( host, msg );
		break;
	case MsgId::devicePostion :
		OnDevicePostion( host, msg );
		break;
	case MsgId::runDevice :
		OnRunDevice( host, msg );
		break;
	default:
		break;
	}

	return true;
}

/*
	��Ӳ�̶�ȡ����
	����Exist��ֲ�ʽExist�����ݿ��㣬����
*/
void NoDB::ReadData()
{
	if ( Device::Type::ssd != m_context.device.type  ) return;
	m_log.Info( "Run", "���ڼ�������" );
	m_log.Info( "Run", "�������" );
	return;
}

//���ݳ־û�
void NoDB::SaveData( time_t tCurTime )
{
	if ( Device::Type::ssd != m_context.device.type ) return;
	static time_t tLastSave = tCurTime;
	if ( tCurTime - tLastSave <= 60 ) return;
	tLastSave = tCurTime;
}

//����
void NoDB::Heartbeat( time_t tCurTime )
{
	static time_t tLastHeart = tCurTime;
	if ( Device::Status::unPlugIn == m_context.device.status ) return; //δ�嵽���壬����������
	if ( tCurTime - tLastHeart <= 60 ) return; //���ͼ��δ��1���ӣ�������

	bsp::BStruct &msg = GetStruct();
	msg["MsgId"] = (unsigned short)MsgId::heartbeat;
	SendBStruct(m_motherBoard.host);
	tLastHeart = tCurTime;
}

void NoDB::OnSetDeviceId( mdk::STNetHost &host, bsp::BStruct &msg )
{
	if ( !m_cfgFile["opt"]["id"].IsNull() )
	{
		m_log.Info( "Waring", "�����豸ID:%d���ܾ��޸�", m_context.device.deviceId );
		return;
	}

	m_cfgFile["opt"]["id"] = m_context.device.deviceId = msg["deviceId"];
	m_cfgFile.Save();
	m_log.Info( "Run", "�����豸ID:%d", m_context.device.deviceId );
}

void NoDB::OnDevicePostion( mdk::STNetHost &host, bsp::BStruct &msg )
{
	if ( m_context.device.type == Device::Type::ssd ) return;

	m_log.Info( "Run", "����%s��Ϣ", Device::Descript(Device::Type::ssd) );
	bsp::BArray devices = msg["devices"];
	bsp::BStruct item;
	int i = 0;
	Device::Type::Type device;
	Device::INFO *pHardDisk;
	mdk::uint64 ip64;
	bool validHD = false;
	for ( i = 0; i < devices.GetCount(); i++ )
	{
		item = devices[i];
		device = (Device::Type::Type)(char)item["device"];
		if ( Device::Type::ssd != device ) continue;
		pHardDisk = new Device::INFO;
		pHardDisk->deviceId = item["deviceId"];
		pHardDisk->type = device;
		pHardDisk->status = Device::Status::running;
		pHardDisk->wanIP = (std::string)item["wanIP"];
		pHardDisk->wanPort = item["wanPort"];
		pHardDisk->lanIP = (std::string)item["lanIP"];
		pHardDisk->lanPort = item["lanPort"];

		validHD = false;
		if ( mdk::addrToI64( ip64, pHardDisk->wanIP.c_str(), pHardDisk->wanPort ) )
		{
			m_hardDiskIpMap[ip64] = pHardDisk;
			validHD = true;
		}
		else 
		{
			m_log.Info( "Waring", "����Ĺ�̬Ӳ��������ַ%s %d", pHardDisk->wanIP.c_str(), pHardDisk->wanPort );
		}
		if ( mdk::addrToI64( ip64, pHardDisk->lanIP.c_str(), pHardDisk->lanPort ) )
		{
			m_hardDiskIpMap[ip64] = pHardDisk;
			validHD = true;
		}
		else 
		{
			m_log.Info( "Waring", "����Ĺ�̬Ӳ��������ַ%s %d", pHardDisk->lanIP.c_str(), pHardDisk->lanPort );
		}
		if ( validHD )
		{
			m_hardDisks[pHardDisk->deviceId] = pHardDisk;
		}
		else 
		{
			delete pHardDisk;
		}
	}
}

void NoDB::OnRunDevice( mdk::STNetHost &host, bsp::BStruct &msg )
{
	m_log.Info( "Run", "%s��ʼ����", Device::Descript(m_context.device.type) );
	Listen( m_context.device.lanPort );
	m_context.device.status = Device::Status::running;
}

/*
 *	2 byte ���ĳ���
 *	2 byte key����
 *	n byte key
 *	2 byte value����
 *	n byte value
 *	4 byte hashvalue
 */
unsigned int g_insertCount = 0;
#ifdef WIN32
SYSTEMTIME t1,t2;
#else
struct timeval tEnd,tStart;
struct timezone tz;
#endif
//����client�����ϵ����ݲ��������host��Guide��ʲô������������false
bool NoDB::OnClientMsg( mdk::STNetHost &host, bsp::BStruct &msg )
{
//	if ( host.ID() == m_motherBoard.host.ID() ) return false;
//	unsigned char msg[600];
//	unsigned short len = 2;
//	
//	if ( !host.Recv( msg, len, false ) ) 
//	{
//		return true;
//	}
//	memcpy( &len, msg, sizeof(unsigned short) );
//	if ( 2 >= len ) //�Ƿ�����
//	{
//		printf( "del msg" );
//		host.Recv( msg, len );//ɾ������
//		return true;
//	}
//	len += 2;
//	if ( !host.Recv( msg, len ) ) 
//	{
//		return true;
//	}
//	unsigned short keySize = 0;
//	unsigned short valueSize = 0;
//	memcpy( &keySize, &msg[2], sizeof(unsigned short) );
//	if ( 2 + 2 + keySize > len ) 
//	{
//		printf( "del msg 2 + 2 + keySize > len" );
//		return true;//�Ƿ�����
//	}
//	memcpy( &valueSize, &msg[2+2+keySize], sizeof(unsigned short) );
//	if ( 2 + 2 + keySize + 2 + valueSize + 4 > len ) 
//	{
//		printf( "del msg 2 + 2 + keySize + 2 + valueSize + 4 > len" );
//		return true;//�Ƿ�����
//	}
//	unsigned char *key = &msg[2+2];
//	unsigned char *value = NULL;//new unsigned char[valueSize];
//	//	memcpy( value, &msg[2+2+keySize+2], valueSize );
//	value = &msg[2+2+keySize+2];
//	unsigned int hashValue = 0;
//	memcpy( &hashValue, &msg[2+2+keySize+2+valueSize], sizeof(unsigned int) );
//	RHTable::OP_R *pR = m_nosqlDB.Insert(key, keySize, value, hashValue );
//	g_insertCount++;
//	if ( 1 == g_insertCount )
//	{
//#ifdef WIN32
//		GetSystemTime(&t1);
//#else
//		gettimeofday (&tStart , &tz);
//#endif
//	}
//	if ( g_insertCount >= 200000 )
//	{
//		g_insertCount = 0;
//#ifdef WIN32
//		GetSystemTime(&t2);
//		int s = ((t2.wHour - t1.wHour)*3600 + (t2.wMinute - t1.wMinute) * 60 
//			+ t2.wSecond - t1.wSecond) * 1000 + t2.wMilliseconds - t1.wMilliseconds;
//		printf( "%f", s / 1000.0 );
//#else
//		gettimeofday (&tEnd , &tz);
//		printf("time:%ld", (tEnd.tv_sec-tStart.tv_sec)*1000+(tEnd.tv_usec-tStart.tv_usec)/1000);
//#endif
//	}

	return true;
}

