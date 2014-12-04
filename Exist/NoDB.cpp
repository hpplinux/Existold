// NoDB.cpp: implementation of the NoDB class.
//
//////////////////////////////////////////////////////////////////////

#include "NoDB.h"

#include <cstdio>
#include <cstdlib>

#ifdef WIN32
#else
#include <sys/time.h>
#endif

#include "mdk/Socket.h"
#include "mdk/mapi.h"
#include "../common/Protocol.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NoDB::NoDB(char* cfgFile)
:m_cfgFile(cfgFile)
{
#ifdef EXIST_DEVICE
	m_log.SetLogName( "Exist" );
#endif
#ifdef SSD_DEVICE
	m_log.SetLogName( "SolidStateDrive" );
#endif
	m_log.SetPrintLog( true );
	m_log.SetMaxLogSize( 10 );
	m_log.SetMaxExistDay( 30 );
	OpenNoDelay();
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
	m_context.maxMemory = m_cfgFile["opt"]["max memory"];
	m_nosqlDB.SetRemoteMode(true);
	m_int8DB.SetRemoteMode(true);
	m_uint8DB.SetRemoteMode(true);
	m_int16DB.SetRemoteMode(true);
	m_uint16DB.SetRemoteMode(true);
	m_int32DB.SetRemoteMode(true);
	m_uint32DB.SetRemoteMode(true);
	m_int64DB.SetRemoteMode(true);
	m_uint64DB.SetRemoteMode(true);
	m_floatDB.SetRemoteMode(true);
	m_doubleDB.SetRemoteMode(true);

#ifdef SSD_DEVICE
	m_existFS.SetRootDir(((std::string)m_cfgFile["opt"]["data root dir"]).c_str());
	m_maxCachedTime = m_cfgFile["opt"]["max cached time"];
	m_maxCachedCount = m_cfgFile["opt"]["max cached count"];
	ReadData();//Ӳ���������ǳ־û��ģ�����Ҫ�ȶ�ȡ����
#endif
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
#ifdef SSD_DEVICE
	SaveData( tCurTime );//�־û�����
#endif

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
		if ( ip == m_motherBoard.wanIP && port == (int)m_motherBoard.wanPort )
		{
			m_motherBoard.host = host;
			m_motherBoard.status = Device::Status::running;
			//��������
			m_log.Info( "Run", "��������" );
			MSG_PLUG_IN_QUERY *pData = (MSG_PLUG_IN_QUERY*)GetDataBuffer();
			pData->deviceId = -1;
			if ( !m_cfgFile["opt"]["id"].IsNull() ) 
			{
				pData->deviceId = m_context.device.deviceId;
			}
			if (  Device::Type::ssd == m_context.device.type ) pData->device = (unsigned char)Device::Type::ssd;
			else pData->device = (unsigned char)Device::Type::exist;
			strcpy( pData->wanIP, m_context.device.wanIP.c_str() );
			pData->wanPort = m_context.device.wanPort;
			strcpy( pData->lanIP, m_context.device.lanIP.c_str() );
			pData->lanPort = m_context.device.lanPort;
			SendMsg( host, MsgId::plugInQuery, sizeof(MSG_PLUG_IN_QUERY) );
			return;
		}

		m_log.Info( "Waring", "δ֪�豸(%s %d)���Ͽ�����", ip.c_str(), port );
		host.Close();
		return;
	}

	host.GetAddress( ip, port );
	if ( Device::Type::exist == m_context.device.type ) m_log.Info( "Run", "����CPU(%s %d)", ip.c_str(), port );
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
		if ( ip == m_motherBoard.wanIP && port == (int)m_motherBoard.wanPort )
		{
			m_log.Info( "Error", "����ɶ�" );
			m_motherBoard.host = emptyHost;
			m_motherBoard.status = Device::Status::unknow;
			return;
		}

		m_log.Info( "Waring", "�ѶϿ�δ֪�豸(%s %d)", ip.c_str(), port );

		return;
	}

	host.GetAddress( ip, port );
	if ( Device::Type::exist == m_context.device.type ) m_log.Info( "Run", "CPU(%s %d)��ʧ", ip.c_str(), port );
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
void NoDB::OnWork( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData )
{
	if ( OnGuideMsg( host, header, pData ) ) return;
	OnClientMsg( host, header, pData );
}

//����Guide�����ϵ����ݲ��������host����Guide��ʲô������������false
bool NoDB::OnGuideMsg( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData )
{
	if ( host.ID() != m_motherBoard.host.ID() ) return false;
	
	switch( header->msgId )
	{
	case MsgId::setDeviceId :
		OnSetDeviceId( host, header, pData );
		break;
	case MsgId::devicePostion :
		OnDevicePostion( host, header, pData );
		break;
	case MsgId::runDevice :
		OnRunDevice( host, header, pData );
		break;
	default:
		break;
	}

	return true;
}

//����
void NoDB::Heartbeat( time_t tCurTime )
{
	static time_t tLastHeart = tCurTime;
	if ( -1 == m_motherBoard.host.ID() ) return; //δ�嵽���壬����������
	if ( tCurTime - tLastHeart <= 60 ) return; //���ͼ��δ��1���ӣ�������

	SendMsg( m_motherBoard.host, MsgId::heartbeat, 0 );
	tLastHeart = tCurTime;
}

void NoDB::OnSetDeviceId( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData )
{
	if ( header->msgSize != sizeof(MSG_SET_DEVICE_ID) ) return;
	if ( !m_cfgFile["opt"]["id"].IsNull() )
	{
		m_log.Info( "Waring", "�����豸ID:%d���ܾ��޸�", m_context.device.deviceId );
		return;
	}

	MSG_SET_DEVICE_ID *pParam = (MSG_SET_DEVICE_ID*)pData;
	m_cfgFile["opt"]["id"] = m_context.device.deviceId = pParam->deviceId;
	m_cfgFile.Save();
	m_log.Info( "Run", "�����豸ID:%d", m_context.device.deviceId );
}

void NoDB::OnDevicePostion( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData )
{
	if ( Device::Status::running == m_context.device.status ) return;
	if ( header->msgSize != sizeof(MSG_DEVICE_POSTION) ) return;
	if ( m_context.device.type == Device::Type::ssd ) return;//Ӳ�̲������κ��豸���������κ��豸��Ϣ

	m_log.Info( "Run", "����%s��Ϣ", Device::Descript(Device::Type::ssd) );
	MSG_DEVICE_POSTION *pParam = (MSG_DEVICE_POSTION*)pData;

	int i = 0;
	Device::Type::Type device;
	Device::INFO *pHardDisk;
	mdk::uint64 ip64;
	bool validHD = false;
	for ( i = 0; i < pParam->size; i++ )
	{
		device = (Device::Type::Type)pParam->devices[i].device;
		if ( Device::Type::ssd != device ) continue;
		pHardDisk = new Device::INFO;
		pHardDisk->deviceId = pParam->devices[i].deviceId;
		pHardDisk->type = device;
		pHardDisk->status = Device::Status::running;
		pHardDisk->wanIP = pParam->devices[i].wanIP;
		pHardDisk->wanPort = pParam->devices[i].wanPort;
		pHardDisk->lanIP = pParam->devices[i].lanIP;
		pHardDisk->lanPort = pParam->devices[i].lanPort;

		validHD = false;
		if ( mdk::addrToI64( ip64, pHardDisk->wanIP.c_str(), pHardDisk->wanPort ) )
		{
			validHD = true;
		}
		else 
		{
			m_log.Info( "Waring", "����Ĺ�̬Ӳ��������ַ%s %d", pHardDisk->wanIP.c_str(), pHardDisk->wanPort );
		}
		if ( mdk::addrToI64( ip64, pHardDisk->lanIP.c_str(), pHardDisk->lanPort ) )
		{
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

void NoDB::OnRunDevice( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData )
{
	if ( Device::Status::running == m_context.device.status ) return;
	if ( header->msgSize != 0 ) return;
	m_log.Info( "Run", "%s��ʼ���� listen%d", Device::Descript(m_context.device.type), m_context.device.lanPort );
	if ( !Listen( m_context.device.lanPort ) )
	{
		m_log.Info( "Run", "�����˿�%dʧ��", m_context.device.lanPort );
	}
	m_context.device.status = Device::Status::running;
	m_log.Info( "Run", "��ʼ����" );
}

bool NoDB::OnClientMsg( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData )
{
/*
	{
		static mdk::uint64 count = 0;
		static mdk::uint64 start = 0;
		static mdk::uint64 use = 0;
		if ( 0 == count ) 
		{
			start = GetTickCount();
		}
		count++;
		if ( 0 == count % 1000 ) 
		{
			use = GetTickCount() - start;
			m_log.Info( "ͳ��", "�������ܣ�%llu�β�������ʱ%llums ƽ��%fms/���� ", 
				count, use, use * 1.0 / count );
		}
	}
*/
	switch ( header->msgId )
	{
	case MsgId::createData://MSG_HEADER+����pData(CREATE_DATA+path)
		OnCreateData((CREATE_DATA*)pData, 
			header->msgSize == sizeof(CREATE_DATA)?NULL:&pData[sizeof(CREATE_DATA)], 
			header->msgSize-sizeof(CREATE_DATA));
		break;
	case MsgId::deleteData://MSG_HEADER+����pData(DATA_KEY+path)
		OnDeleteData((exist::DATA_KEY*)pData, 
			header->msgSize == sizeof(exist::DATA_KEY)?NULL:&pData[sizeof(exist::DATA_KEY)], 
			header->msgSize-sizeof(exist::DATA_KEY));
		break;
	case MsgId::writeData://MSG_HEADER+����pData(WRITE_DATA+path+data)
		{
			WRITE_DATA *pParam = (WRITE_DATA*)pData;
			int pathSize = header->msgSize - sizeof(WRITE_DATA) - pParam->size;
			unsigned char *pValue = &pData[sizeof(WRITE_DATA) + pathSize];
			unsigned char *path = NULL;
			if ( 0 < pathSize ) path = &pData[sizeof(WRITE_DATA)];
			OnWriteData(pParam, pValue, path, pathSize);
		}
		break;
	case MsgId::readData://MSG_HEADER+����pData(DATA_KEY+path)
		OnReadData(host, (exist::DATA_KEY*)pData, 
			header->msgSize == sizeof(exist::DATA_KEY)?NULL:&pData[sizeof(exist::DATA_KEY)], 
			header->msgSize-sizeof(exist::DATA_KEY));
		break;
	default:
		break;
	}

	return true;
}

void NoDB::AddSelf( exist::VALUE *pValue, unsigned char *pData )
{
	if ( DataType::int8 == pValue->key.type )
	{
		mdk::int8 *vl, *vr;
		vl = (mdk::int8*)pValue->pData;
		vr = (mdk::int8*)pData;
		*vl += *vr;
	}
	if ( DataType::uInt8 == pValue->key.type )
	{
		mdk::uint8 *vl, *vr;
		vl = (mdk::uint8*)pValue->pData;
		vr = (mdk::uint8*)pData;
		*vl += *vr;
	}
	if ( DataType::int16 == pValue->key.type )
	{
		mdk::int16 *vl, *vr;
		vl = (mdk::int16*)pValue->pData;
		vr = (mdk::int16*)pData;
		*vl += *vr;
	}
	if ( DataType::uInt16 == pValue->key.type )
	{
		mdk::uint16 *vl, *vr;
		vl = (mdk::uint16*)pValue->pData;
		vr = (mdk::uint16*)pData;
		*vl += *vr;
	}
	if ( DataType::int32 == pValue->key.type )
	{
		mdk::int32 *vl, *vr;
		vl = (mdk::int32*)pValue->pData;
		vr = (mdk::int32*)pData;
		*vl += *vr;
	}
	if ( DataType::uInt32 == pValue->key.type )
	{
		mdk::uint32 *vl, *vr;
		vl = (mdk::uint32*)pValue->pData;
		vr = (mdk::uint32*)pData;
		*vl += *vr;
	}
	if ( DataType::int64 == pValue->key.type )
	{
		mdk::int64 *vl, *vr;
		vl = (mdk::int64*)pValue->pData;
		vr = (mdk::int64*)pData;
		*vl += *vr;
	}
	if ( DataType::uInt64 == pValue->key.type )
	{
		mdk::uint64 *vl, *vr;
		vl = (mdk::uint64*)pValue->pData;
		vr = (mdk::uint64*)pData;
		*vl += *vr;
	}
	if ( DataType::sFloat == pValue->key.type )
	{
		float *vl, *vr;
		vl = (float*)pValue->pData;
		vr = (float*)pData;
		*vl += *vr;
	}
	if ( DataType::sDouble == pValue->key.type )
	{
		double *vl, *vr;
		vl = (double*)pValue->pData;
		vr = (double*)pData;
		*vl += *vr;
	}
}

void NoDB::SubtractSelf( exist::VALUE *pValue, unsigned char *pData )
{
	if ( DataType::int8 == pValue->key.type )
	{
		mdk::int8 *vl, *vr;
		vl = (mdk::int8*)pValue->pData;
		vr = (mdk::int8*)pData;
		*vl -= *vr;
	}
	if ( DataType::uInt8 == pValue->key.type )
	{
		mdk::uint8 *vl, *vr;
		vl = (mdk::uint8*)pValue->pData;
		vr = (mdk::uint8*)pData;
		*vl -= *vr;
	}
	if ( DataType::int16 == pValue->key.type )
	{
		mdk::int16 *vl, *vr;
		vl = (mdk::int16*)pValue->pData;
		vr = (mdk::int16*)pData;
		*vl -= *vr;
	}
	if ( DataType::uInt16 == pValue->key.type )
	{
		mdk::uint16 *vl, *vr;
		vl = (mdk::uint16*)pValue->pData;
		vr = (mdk::uint16*)pData;
		*vl -= *vr;
	}
	if ( DataType::int32 == pValue->key.type )
	{
		mdk::int32 *vl, *vr;
		vl = (mdk::int32*)pValue->pData;
		vr = (mdk::int32*)pData;
		*vl -= *vr;
	}
	if ( DataType::uInt32 == pValue->key.type )
	{
		mdk::uint32 *vl, *vr;
		vl = (mdk::uint32*)pValue->pData;
		vr = (mdk::uint32*)pData;
		*vl -= *vr;
	}
	if ( DataType::int64 == pValue->key.type )
	{
		mdk::int64 *vl, *vr;
		vl = (mdk::int64*)pValue->pData;
		vr = (mdk::int64*)pData;
		*vl -= *vr;
	}
	if ( DataType::uInt64 == pValue->key.type )
	{
		mdk::uint64 *vl, *vr;
		vl = (mdk::uint64*)pValue->pData;
		vr = (mdk::uint64*)pData;
		*vl -= *vr;
	}
	if ( DataType::sFloat == pValue->key.type )
	{
		float *vl, *vr;
		vl = (float*)pValue->pData;
		vr = (float*)pData;
		*vl -= *vr;
	}
	if ( DataType::sDouble == pValue->key.type )
	{
		double *vl, *vr;
		vl = (double*)pValue->pData;
		vr = (double*)pData;
		*vl -= *vr;
	}
}

void NoDB::MultiplySelf( exist::VALUE *pValue, unsigned char *pData )
{
	if ( DataType::int8 == pValue->key.type )
	{
		mdk::int8 *vl, *vr;
		vl = (mdk::int8*)pValue->pData;
		vr = (mdk::int8*)pData;
		*vl *= *vr;
	}
	if ( DataType::uInt8 == pValue->key.type )
	{
		mdk::uint8 *vl, *vr;
		vl = (mdk::uint8*)pValue->pData;
		vr = (mdk::uint8*)pData;
		*vl *= *vr;
	}
	if ( DataType::int16 == pValue->key.type )
	{
		mdk::int16 *vl, *vr;
		vl = (mdk::int16*)pValue->pData;
		vr = (mdk::int16*)pData;
		*vl *= *vr;
	}
	if ( DataType::uInt16 == pValue->key.type )
	{
		mdk::uint16 *vl, *vr;
		vl = (mdk::uint16*)pValue->pData;
		vr = (mdk::uint16*)pData;
		*vl *= *vr;
	}
	if ( DataType::int32 == pValue->key.type )
	{
		mdk::int32 *vl, *vr;
		vl = (mdk::int32*)pValue->pData;
		vr = (mdk::int32*)pData;
		*vl *= *vr;
	}
	if ( DataType::uInt32 == pValue->key.type )
	{
		mdk::uint32 *vl, *vr;
		vl = (mdk::uint32*)pValue->pData;
		vr = (mdk::uint32*)pData;
		*vl *= *vr;
	}
	if ( DataType::int64 == pValue->key.type )
	{
		mdk::int64 *vl, *vr;
		vl = (mdk::int64*)pValue->pData;
		vr = (mdk::int64*)pData;
		*vl *= *vr;
	}
	if ( DataType::uInt64 == pValue->key.type )
	{
		mdk::uint64 *vl, *vr;
		vl = (mdk::uint64*)pValue->pData;
		vr = (mdk::uint64*)pData;
		*vl *= *vr;
	}
	if ( DataType::sFloat == pValue->key.type )
	{
		float *vl, *vr;
		vl = (float*)pValue->pData;
		vr = (float*)pData;
		*vl *= *vr;
	}
	if ( DataType::sDouble == pValue->key.type )
	{
		double *vl, *vr;
		vl = (double*)pValue->pData;
		vr = (double*)pData;
		*vl *= *vr;
	}
}

bool NoDB::DivideSelf( exist::VALUE *pValue, unsigned char *pData )
{
	if ( DataType::int8 == pValue->key.type )
	{
		mdk::int8 *vl, *vr;
		vr = (mdk::int8*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::int8*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::uInt8 == pValue->key.type )
	{
		mdk::uint8 *vl, *vr;
		vr = (mdk::uint8*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::uint8*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::int16 == pValue->key.type )
	{
		mdk::int16 *vl, *vr;
		vr = (mdk::int16*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::int16*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::uInt16 == pValue->key.type )
	{
		mdk::uint16 *vl, *vr;
		vr = (mdk::uint16*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::uint16*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::int32 == pValue->key.type )
	{
		mdk::int32 *vl, *vr;
		vr = (mdk::int32*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::int32*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::uInt32 == pValue->key.type )
	{
		mdk::uint32 *vl, *vr;
		vr = (mdk::uint32*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::uint32*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::int64 == pValue->key.type )
	{
		mdk::int64 *vl, *vr;
		vr = (mdk::int64*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::int64*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::uInt64 == pValue->key.type )
	{
		mdk::uint64 *vl, *vr;
		vr = (mdk::uint64*)pData;
		if ( 0 == *vr ) return false;
		vl = (mdk::uint64*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::sFloat == pValue->key.type )
	{
		float *vl, *vr;
		vr = (float*)pData;
		if ( 0.0 == *vr ) return false;
		vl = (float*)pValue->pData;
		*vl /= *vr;
	}
	if ( DataType::sDouble == pValue->key.type )
	{
		double *vl, *vr;
		vr = (double*)pData;
		if ( 0.0 == *vr ) return false;
		vl = (double*)pValue->pData;
		*vl /= *vr;
	}

	return true;
}

RHTable* NoDB::GetNosql(DataType::DataType type, unsigned char *path, int size, exist::VALUE **pParent)
{
	if ( NULL == path )
	{
		if ( DataType::int8 == type ) return &m_int8DB;
		if ( DataType::uInt8 == type ) return &m_uint8DB;
		if ( DataType::int16 == type ) return &m_int16DB;
		if ( DataType::uInt16 == type ) return &m_uint16DB;
		if ( DataType::int32 == type ) return &m_int32DB;
		if ( DataType::uInt32 == type ) return &m_uint32DB;
		if ( DataType::int64 == type ) return &m_int64DB;
		if ( DataType::uInt64 == type ) return &m_uint64DB;
		if ( DataType::sFloat == type ) return &m_floatDB;
		if ( DataType::sDouble == type ) return &m_doubleDB;

		return &m_nosqlDB;
	}
	if ( size > (int)(10 * sizeof(exist::DATA_KEY)) ) 
	{
		m_log.Info("Error","����Ƕ�׳���10�㣬�ݲ�֧��");
		return NULL;
	}
	return FindNosql(&m_nosqlDB, path, size, pParent);
}

RHTable* NoDB::FindNosql(RHTable* pNosql, unsigned char *path, int size, exist::VALUE **pParent)
{
	if ( size < (int)sizeof(exist::DATA_KEY) ) return NULL;
	exist::DATA_KEY *pKey = (exist::DATA_KEY*)path;
	exist::VALUE *pValue = (exist::VALUE *)m_nosqlDB.Find( (unsigned char*)pKey->key, pKey->keySize, pKey->hashid );
	if ( sizeof(exist::DATA_KEY) == size ) 
	{
		*pParent = pValue;
		return (RHTable*)pValue->pData;
	}
	return FindNosql( (RHTable*)pValue->pData, &path[sizeof(exist::DATA_KEY)], size - sizeof(exist::DATA_KEY), pParent );
}

exist::VALUE* NoDB::CreateData(CREATE_DATA *pParam, unsigned char *path, int size)
{
	exist::VALUE *pParent = NULL;
	RHTable *pNosql = GetNosql((DataType::DataType)pParam->key.type, path, size, &pParent);
	if ( NULL == pNosql ) return NULL;
	exist::VALUE *pValue = (exist::VALUE*)pNosql->Find((unsigned char*)pParam->key.key, pParam->key.keySize, 
		pParam->key.hashid);
	if ( NULL != pValue ) return NULL;

	pValue = new exist::VALUE;
	if ( NULL == pValue )
	{
		m_log.Info( "Error", "�ڴ治�㣬�޷���������" );
		return NULL;
	}
	pValue->protracted = pParam->protracted;
	pValue->key = pParam->key;
	pValue->delMark = false;
	pValue->size = 0;
	pValue->pData = NULL;
	pValue->pParent = pParent;
	pValue->idxAble = false;

	if ( DataType::IsValue(pParam->key.type) )//������ֵ�������ͳ�ʼ��ֵΪ0
	{
		pValue->size = pParam->size;
		pValue->pData = new char[pValue->size];
		memset( pValue->pData, 0, pValue->size );
	}
	else if ( DataType::stream == pParam->key.type)//�䳤���ͣ����ڴ�
	{
	}
	else if ( DataType::IsContainer(pParam->key.type) )//�������ͣ����ڴ�
	{
	}
	pNosql->Insert((unsigned char*)pParam->key.key, pParam->key.keySize, pValue, pParam->key.hashid);

	return pValue;
}

void NoDB::OnCreateData(CREATE_DATA *pParam, unsigned char *path, int size)
{
	exist::VALUE *pValue = CreateData(pParam, path, size);
	if ( NULL == pValue ) return;

#ifdef EXIST_DEVICE
	if ( DataType::IsValue(pParam->key.type) 
		|| DataType::stream == pParam->key.type )
	{
		ReadDataFromSSD(pValue, path, size);
	}
	else if ( DataType::IsContainer(pParam->key.type) )
	{
	}
#endif

#ifdef SSD_DEVICE
	if ( DataType::IsValue(pParam->key.type) 
		|| DataType::stream == pParam->key.type )
	{
		m_existFS.CreateData(pValue);
//		ReadDataFromFile(pValue);

	}
	else if ( DataType::IsContainer(pParam->key.type) )
	{
		m_existFS.CreateTable(pParam, sizeof(CREATE_DATA), path, size);
	}
	else
	{
	}
#endif
}

void NoDB::OnDeleteData(exist::DATA_KEY *pParam, unsigned char *path, int size)
{
	//�ݲ�֧��ɾ������
	return;

	exist::VALUE *pParent = NULL;
	RHTable *pNosql = GetNosql((DataType::DataType)pParam->type, path, size, &pParent);
	if ( NULL == pNosql ) return;

	exist::VALUE *pValue = (exist::VALUE*)pNosql->Find((unsigned char*)pParam->key, pParam->keySize, pParam->hashid);
	if ( NULL == pValue ) return;

	if ( pValue->delMark ) return;

	pValue->delMark = true;
	if ( DataType::IsContainer(pValue->key.type) )//������Ҫ�����Ԫ��
	{
		return;
	}
	if ( pValue->protracted )//�־û���Ҫ�޸��ļ�
	{

	}

	//ɾ������
	pNosql->Delete((unsigned char*)pParam->key, pParam->keySize, pParam->hashid);
	if ( NULL != pValue->pData )
	{
		delete[]pValue->pData;
		pValue->pData = NULL;
	}
	delete pValue;
}

void NoDB::OnWriteData(WRITE_DATA *pParam, unsigned char* pData, unsigned char *path, int size)
{
	exist::VALUE *pParent = NULL;
	RHTable *pNosql = GetNosql((DataType::DataType)pParam->key.type, path, size, &pParent);
	if ( NULL == pNosql ) return;

	exist::VALUE *pValue = (exist::VALUE*)pNosql->Find((unsigned char*)pParam->key.key, pParam->key.keySize, pParam->key.hashid);
	if ( NULL == pValue ) return;

	if ( pParam->size == pValue->size )
	{
		if ( UpdateType::utCopy == pParam->updateType )
		{
			memcpy(pValue->pData, pData, pValue->size);
		}
		else if ( DataType::IsValue(pValue->key.type) ) 
		{
			if ( UpdateType::utAddSelf == pParam->updateType )
			{
				AddSelf(pValue, pData);
			}
			else if ( UpdateType::utSubtractSelf == pParam->updateType )
			{
				SubtractSelf(pValue, pData);
			}
			else if ( UpdateType::utMultiplySelf == pParam->updateType )
			{
				MultiplySelf(pValue, pData);
			}
			else if ( UpdateType::utDivideSelf == pParam->updateType )
			{
				DivideSelf(pValue, pData);
			}
		}
		else
		{
			m_log.Info( "Waring", "�Է���ֵ�����������������" );
			return;
		}
	}
	else 
	{
		if ( NULL != pValue->pData )
		{
			delete[]pValue->pData;
			pValue->pData = NULL;
		}
		pValue->size = pParam->size;
		pValue->pData = new char[pValue->size];
		memcpy(pValue->pData, pData, pValue->size);
	}
	//////////////////////////////////////////////////////////////////////////
	//�־û�
#ifdef EXIST_DEVICE
	SaveDataToSSD(pValue, path, size);
#endif
#ifdef SSD_DEVICE
	AddSaveWaiter(pValue, pParam);//��ǰ�豸�ǹ�̬Ӳ�̣�������У����ﵽ�����ϲ�
	SaveData(time(NULL));
#endif
}

void NoDB::OnReadData(mdk::STNetHost &host, exist::DATA_KEY *pParam, unsigned char *path, int size)
{
	exist::VALUE *pParent = NULL;
	RHTable *pNosql = GetNosql((DataType::DataType)pParam->type, path, size, &pParent);
	if ( NULL == pNosql ) return;

	MSG_HEADER header;
	header.msgId = MsgId::readData;
	exist::VALUE *pValue = (exist::VALUE*)pNosql->Find((unsigned char*)pParam->key, pParam->keySize, pParam->hashid);
	if ( NULL == pValue || 0 >= pValue->size ) 
	{
		header.msgSize = 0;
		host.Send((unsigned char*)&header, sizeof(MSG_HEADER));
		return;
	}
	header.msgSize = pValue->size;
	host.Send((unsigned char*)&header, sizeof(MSG_HEADER));
	host.Send((unsigned char*)pValue->pData, header.msgSize);
}

mdk::Socket* NoDB::GetSSDConnect( int keyHashId )
{
	if ( 0 >= m_hardDisks.size() ) 
	{
		m_log.Info( "Error", "��̬Ӳ��δ����" );
		return NULL;
	}

	int deviceId = keyHashId%m_hardDisks.size();
	mdk::Socket *pSSD = &m_hardConnects[deviceId];
	if ( !pSSD->IsClosed() ) return pSSD;

	pSSD->Init( mdk::Socket::tcp );
	Device::INFO *pAdr = m_hardDisks[deviceId];
	std::string ip;
	int port;
	if ( pAdr->wanIP == m_context.device.wanIP && pAdr->lanIP == m_context.device.lanIP )
	{
		ip = pAdr->lanIP;
		port = pAdr->lanPort;
	}
	else
	{
		ip = pAdr->wanIP;
		port = pAdr->lanPort;
	}

	if ( !pSSD->Connect( ip.c_str(), port ) )
	{
		m_log.Info( "Error", "Ӳ��(%s %d - %s %d)�Ӵ�����", 
			pAdr->wanIP.c_str(), pAdr->wanPort, pAdr->lanIP.c_str(), pAdr->lanPort );
		return NULL;
	}
	pSSD->SetNoDelay(true);

	return pSSD;
}

void NoDB::ReadDataFromSSD( exist::VALUE *pValue, unsigned char *path, int size )
{
	if ( !pValue->protracted ) return;

	mdk::Socket *pSSD = GetSSDConnect(pValue->key.hashid);
	if ( NULL == pSSD ) return;

	char msg[1024+(sizeof(exist::DATA_KEY)*10)];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	{//��������:MSG_HEADER+CREATE_DATA+path
		CREATE_DATA *pData = (CREATE_DATA*)&msg[sizeof(MSG_HEADER)];
		pHeader->msgId = MsgId::createData;
		pHeader->msgSize = sizeof(CREATE_DATA);
		pData->key = pValue->key;
		pData->size = pValue->size;
		pData->protracted = pValue->protracted;
		pHeader->msgSize += size;
		memcpy(&msg[sizeof(MSG_HEADER)+sizeof(CREATE_DATA)], path, size);
		if ( 0 > pSSD->Send(msg, sizeof(MSG_HEADER) + pHeader->msgSize) )
		{
			pSSD->Close();
			m_log.Info( "Error", "��̬Ӳ�̽Ӵ��������޷������־û�����" );
			return;
		}
	}
	if ( DataType::IsContainer(pValue->key.type) ) return;//���������ݴ洢�����ڴ�

	{//��ѯ����:MSG_HEADER+DATA_KEY+path
		exist::DATA_KEY *pKey = (exist::DATA_KEY*)&msg[sizeof(MSG_HEADER)];
		pHeader->msgId = MsgId::readData;
		pHeader->msgSize = sizeof(exist::DATA_KEY);
		*pKey = pValue->key;
		pHeader->msgSize += size;
		memcpy(&msg[sizeof(MSG_HEADER)+sizeof(CREATE_DATA)], path, size);
		pSSD->Send(msg, sizeof(MSG_HEADER) + pHeader->msgSize);

		if ( 0 > pSSD->Receive(msg, sizeof(MSG_HEADER)) )
		{
			pSSD->Close();
			m_log.Info( "Error", "��̬Ӳ�̽Ӵ��������޷���ȡ�־û�����" );
			return;
		}
		if ( pHeader->msgSize <= 0 ) return;
		if ( DataType::stream == pValue->key.type )
		{
			pValue->size = pHeader->msgSize;
			pValue->pData = new char[pValue->size];
		}
		if ( pValue->size != pHeader->msgSize )
		{
			m_log.Info( "Error", "�������Ͳ�ƥ�䣬�޷���ȡ�־û�����" );
			return;
		}
		if ( 0 > pSSD->Receive(pValue->pData, pValue->size) )
		{
			pSSD->Close();
			m_log.Info( "Error", "��̬Ӳ�̽Ӵ��������޷���ȡ�־û�����" );
			return;
		}
	}
}

void NoDB::SaveDataToSSD( exist::VALUE *pValue, unsigned char *path, int size )
{
	if ( !pValue->protracted ) return;

	mdk::Socket *pSSD = GetSSDConnect(pValue->key.hashid);
	if ( NULL == pSSD ) return;

	char msg[1024+(+sizeof(exist::DATA_KEY)*10)];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	if ( DataType::IsContainer(pValue->key.type) ) return;//���������ݴ洢�����ڴ�

	//д����:MSG_HEADER+WRITE_DATA+path+data
	WRITE_DATA *pData = (WRITE_DATA*)&msg[sizeof(MSG_HEADER)];
	pHeader->msgId = MsgId::writeData;
	pHeader->msgSize = sizeof(WRITE_DATA) + size + pValue->size;
	pData->key = pValue->key;
	pData->updateType = UpdateType::utCopy;
	pData->size = pValue->size;
	memcpy(&msg[sizeof(MSG_HEADER)+sizeof(WRITE_DATA)], path, size);
	if ( 0 > pSSD->Send(msg, sizeof(MSG_HEADER)+sizeof(WRITE_DATA)+size) )
	{
		pSSD->Close();
		m_log.Info( "Error", "��̬Ӳ�̽Ӵ��������޷��־û�����" );
		return;
	}
	if ( 0 > pSSD->Send(pValue->pData, pValue->size) )
	{
		pSSD->Close();
		m_log.Info( "Error", "��̬Ӳ�̽Ӵ��������޷��־û�����" );
		return;
	}
}

const char* NoDB::ReadRootData()
{
	exist::VALUE rootMap;
	rootMap.key.key[0] = '\0';
	rootMap.key.keySize = 0;
	rootMap.pParent = NULL;
	const char *ret;
	std::vector<exist::VALUE*> data;
	DataType::DataType typeList[256];
	typeList[0] = DataType::stream;
	typeList[1] = DataType::sFloat;
	typeList[2] = DataType::sDouble;
	typeList[3] = DataType::int8;
	typeList[4] = DataType::uInt8;
	typeList[5] = DataType::int16;
	typeList[6] = DataType::uInt16;
	typeList[7] = DataType::int32;
	typeList[8] = DataType::uInt32;
	typeList[9] = DataType::int64;
	typeList[10] = DataType::uInt64;
	int i = 0;
	for ( i = 0; i < 11; i++ )
	{
		rootMap.key.elementType = typeList[i];
		ret = m_existFS.ReadTable(&rootMap, data);//��ȡȫ�ֱ�int8����
		if ( NULL != ret )
		{
			m_log.Info( "Error", "�������ݴ���:%s", ret );
			mdk::mdk_assert(false);
			return ret;
		}
		unsigned int i = 0;
		exist::VALUE *pValue = NULL;
		exist::VALUE *pParent = NULL;
		RHTable *pNosql;
		for ( i = 0; i < data.size(); i++ )
		{
			pValue = data[i];
			pValue->pParent = NULL;
			pNosql = GetNosql((DataType::DataType)pValue->key.type, NULL, 0, &pParent);
			pNosql->Insert((unsigned char*)pValue->key.key, pValue->key.keySize, pValue, pValue->key.hashid);
		}
		data.clear();
	}

	return NULL;
}
/*
	��Ӳ�̶�ȡ����
	����Exist��ֲ�ʽExist�����ݿ��㣬����
*/
void NoDB::ReadData()
{
	m_log.Info( "Run", "���ڼ�������..." );

	const char *ret = ReadRootData();
	if ( NULL != ret )
	{
		m_log.Info( "Error", "�������ݴ���:%s", ret );
		mdk::mdk_assert(false);
		return;
	}
	std::vector<exist::VALUE*> data;
 	exist::VALUE *pValue = NULL;
 	exist::VALUE *pTable = NULL;
	unsigned int i = 0;
	RHTable *pNosql;
	//��ȡ����������
	ret = m_existFS.MoveFristTable();//�ƶ�����һ�ű�
	if ( NULL != ret )
	{
		m_log.Info( "Error", "�������ݴ���:%s", ret );
		mdk::mdk_assert(false);
		return;
	}

	unsigned char createData[1024];
	short size;
	while ( true )
	{
		ret = m_existFS.GetTable( createData, size );//ȡ��ǰ�����
		if ( 0 == size ) break;

		if ( NULL != ret )
		{
			m_log.Info( "Error", "�������ݴ���:%s", ret );
			mdk::mdk_assert(false);
			return;
		}
		CREATE_DATA *pParam = (CREATE_DATA*)createData;
		unsigned char *path = NULL;
		size -= sizeof(CREATE_DATA);
		if ( 0 < size ) path = &createData[sizeof(CREATE_DATA)];
		pTable = CreateData(pParam, path, size);
		if ( NULL == pTable ) 
		{
			m_log.Info( "Waring", "�����ظ�������" );
			continue;
		}

		if ( DataType::IsValue(pTable->key.elementType) 
			|| DataType::stream > pTable->key.elementType ) //Ԫ�����������ͣ���ȡ����
		{
			data.clear();
			ret = m_existFS.ReadTable(pTable, data);//��ȡ������
			if ( NULL != ret )
			{
				m_log.Info( "Error", "�������ݴ���:%s", ret );
				mdk::mdk_assert(false);
				return;
			}
			pNosql = (RHTable*)pTable->pData;
			for ( i = 0; i < data.size(); i++ )
			{
				pValue = data[i];
				pValue->pParent = pTable;
				pNosql->Insert((unsigned char*)pValue->key.key, pValue->key.keySize, pValue, pValue->key.hashid);
			}
		}
	}
	m_log.Info( "Run", "�������" );

	return;
}

void NoDB::AddSaveWaiter(exist::VALUE *pValue, WRITE_DATA *pParam)
{
	if ( !pValue->protracted ) return;//���ݲ���Ҫ�־û�

	if ( m_waitSaveDatas.end() != m_waitSaveDatas.find(pValue) ) return;//�Ѿ������ڶ�����
	//д����У��Ժ�ϲ�д��
	/*
		pValueָ����ڴ�ֻҪ�������Ͳ��ᱻ�ͷ�
		���ݸ���ֻ���滻pValue->pData
		����ɾ����ֻ�Ǳ�ǣ��´��������ʱ������
	*/
	m_waitSaveDatas[pValue] = pValue;
}

void NoDB::ReadDataFromFile( exist::VALUE *pValue )
{
	if ( !pValue->protracted ) return;
//	m_existFS.ReadValue(pValue);
}

//���ݳ־û�
void NoDB::SaveData( time_t tCurTime )
{
	static time_t tLastSave = tCurTime;
	if ( m_maxCachedCount > (int)m_waitSaveDatas.size() 
		&& m_maxCachedTime > tCurTime - tLastSave ) return;
	std::map<exist::VALUE*, exist::VALUE*>::iterator it = m_waitSaveDatas.begin();//��д�����ݣ�5S/1000�����ºϲ�д��
	exist::VALUE *pValue;
	bool delMark = false;
	const char *ret = NULL;
	for ( ; it != m_waitSaveDatas.end(); it++ )
	{
		delMark = false;
		pValue = it->second;
		while ( NULL != pValue ) 
		{
			if ( pValue->delMark ) 
			{
				delMark = true;
				break;
			}
			pValue = pValue->pParent;
		}
		if ( delMark ) continue;
		ret = m_existFS.WriteValue(it->second);
		if ( NULL != ret )
		{
			m_log.Info( "Error", "д����ʧ�ܣ�%s", ret );
		}
	}
	m_waitSaveDatas.clear();
	tLastSave = tCurTime;
}
