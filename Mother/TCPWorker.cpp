// TCPWorker.cpp: implementation of the TCPWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "TCPWorker.h"
#include "../bstruct/include/BArray.h"
#include <algorithm>

#include <cstdio>
#include <cstdlib>
#include <io.h>
#include <direct.h>
#ifdef WIN32
#else
#include <unistd.h>
#endif

#ifdef WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TCPWorker::TCPWorker(char *cfgFile)
:m_cfgFile( cfgFile )
{
	m_log.SetLogName( "Exist-Mother" );
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
 *	服务器主线程
 *  定时写文件
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
			m_log.Info( "Error", "%s(%s %d-%s %d)未插紧", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
			m_hardDisks.erase( it );
			return;
		}
	}


	it = m_exists.begin();
	for ( ; it != m_exists.end(); it++ ) 
	{
		if ( it->second.host.ID() == host.ID() )
		{
			m_log.Info( "Error", "%s(%s %d-%s %d)未插紧", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
			m_exists.erase( it );
			return;
		}
	}

	it = m_screens.begin();
	for ( ; it != m_screens.end(); it++ ) 
	{
		if ( it->second.host.ID() == host.ID() )
		{
			m_log.Info( "Error", "%s(%s %d-%s %d)未插紧", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
			m_screens.erase( it );
			return;
		}
	}

	it = m_cpus.begin();
	for ( ; it != m_cpus.end(); it++ ) 
	{
		if ( it->second.host.ID() == host.ID() )
		{
			m_log.Info( "Error", "%s(%s %d-%s %d)未插紧", Device::Descript(it->second.type), it->second.wanIP.c_str(), it->second.wanPort, it->second.lanIP.c_str(), it->second.lanPort );
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
	m_log.StreamInfo( "Error", msg, len, "主机(%s-%d)发送非法格式报文", ip.c_str(), port );
	host.Close();
}

/**
 * 数据到达，回调方法
 * 
 * 派生类实现具体断开连接业务处理
 * 
*/
void TCPWorker::OnWork( mdk::STNetHost &host, bsp::BStruct &msg )
{
	bool isValidMsg = true;
	unsigned short msgid = msg["MsgId"];
	switch( msgid )
	{
	case MsgId::heartbeat://心跳
		break;
	case MsgId::plugInQuery ://设备请求插入
		isValidMsg = OnPlugIn(host, msg);
		break;
	default:
		isValidMsg = false;
		break;
	}

	if ( !isValidMsg ) OnInvalidMsg( host, ErrorType::paramError, msg.GetStream(), msg.GetSize() );

	return;
}

void TCPWorker::SetDeviceId( mdk::STNetHost &host, unsigned char deviceId )
{
	bsp::BStruct &msg = GetStruct();
	msg["MsgId"] = (unsigned short)MsgId::setDeviceId;
	msg["deviceId"] = deviceId;
	SendBStruct( host );
}

void TCPWorker::RunDevice( Device::INFO &device )
{
	m_log.Info( "Run", "运行%s(%s %d-%s %d)", Device::Descript( device.type ), 
		device.wanIP.c_str(), device.wanPort, device.lanIP.c_str(), device.lanPort ); 
	DevicePostion( device.host );

	bsp::BStruct &msg = GetStruct();
	msg["MsgId"] = (unsigned short)MsgId::runDevice;
	SendBStruct( device.host );

	device.status = Device::Status::running;
}

void TCPWorker::DevicePostion( mdk::STNetHost &host  )
{
	//所有外存条
	std::vector<int> ids;
	DeviceMap::iterator it = m_exists.begin();
	for ( ids.clear(); it != m_exists.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//构造堆
	std::sort_heap(ids.begin(), ids.end());//堆排序
	
	bsp::BStruct &msg = GetStruct();
	msg["MsgId"] = (unsigned short)MsgId::devicePostion;
	bsp::BArray devices;
	devices.Bind( msg.PreBuffer( "devices" ), msg.PreSize() );
	bsp::BStruct device;
	int i = 0;
	int count = 0;
	for ( i = 0; i < ids.size(); i++ )
	{
		device.Bind( devices.PreBuffer(), devices.PreSize() );
		device["device"] = (char)m_exists[ids[i]].type;
		device["deviceId"] = (unsigned char)m_exists[ids[i]].deviceId;
		device["wanIP"] = m_exists[ids[i]].wanIP.c_str();
		device["wanPort"] = m_exists[ids[i]].wanPort;
		device["lanIP"] = m_exists[ids[i]].lanIP.c_str();
		device["lanPort"] = m_exists[ids[i]].lanPort;
		devices[count++] = &device;
	}

	//所有固态硬盘
	it = m_hardDisks.begin();
	for ( ids.clear(); it != m_hardDisks.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//构造堆
	std::sort_heap(ids.begin(), ids.end());//堆排序
	
	for ( i = 0; i < ids.size(); i++ )
	{
		device.Bind( devices.PreBuffer(), devices.PreSize() );
		device["device"] = (char)m_hardDisks[ids[i]].type;
		device["deviceId"] = (unsigned char)m_hardDisks[ids[i]].deviceId;
		device["wanIP"] = m_hardDisks[ids[i]].wanIP.c_str();
		device["wanPort"] = m_hardDisks[ids[i]].wanPort;
		device["lanIP"] = m_hardDisks[ids[i]].lanIP.c_str();
		device["lanPort"] = m_hardDisks[ids[i]].lanPort;
		devices[count++] = &device;
	}

	//所有触摸屏
	it = m_screens.begin();
	for ( ids.clear(); it != m_screens.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//构造堆
	std::sort_heap(ids.begin(), ids.end());//堆排序
	
	for ( i = 0; i < ids.size(); i++ )
	{
		device.Bind( devices.PreBuffer(), devices.PreSize() );
		device["device"] = (char)m_screens[ids[i]].type;
		device["deviceId"] = (unsigned char)m_screens[ids[i]].deviceId;
		device["wanIP"] = m_screens[ids[i]].wanIP.c_str();
		device["wanPort"] = m_screens[ids[i]].wanPort;
		device["lanIP"] = m_screens[ids[i]].lanIP.c_str();
		device["lanPort"] = m_screens[ids[i]].lanPort;
		devices[count++] = &device;
	}

	//所有CPU
	it = m_cpus.begin();
	for ( ids.clear(); it != m_cpus.end(); it++ ) ids.push_back( it->first );
	std::make_heap(ids.begin(), ids.end());//构造堆
	std::sort_heap(ids.begin(), ids.end());//堆排序
	
	for ( i = 0; i < ids.size(); i++ )
	{
		device.Bind( devices.PreBuffer(), devices.PreSize() );
		device["device"] = (char)m_cpus[ids[i]].type;
		device["deviceId"] = (unsigned char)m_cpus[ids[i]].deviceId;
		device["wanIP"] = m_cpus[ids[i]].wanIP.c_str();
		device["wanPort"] = m_cpus[ids[i]].wanPort;
		device["lanIP"] = m_cpus[ids[i]].lanIP.c_str();
		device["lanPort"] = m_cpus[ids[i]].lanPort;
		devices[count++] = &device;
	}

	msg["devices"] = &devices;
	SendBStruct( host );
}

//设备请求插入
bool TCPWorker::OnPlugIn(mdk::STNetHost &host, bsp::BStruct &msg)
{
	//////////////////////////////////////////////////////////////////////////
	/*
		检查报文合法性
		1.该有的字段有没有
		2.长度是否与协议约定相等
		3.字符串等变长字段长度必须不小于1
	*/
	if ( !msg["device"].IsValid() || sizeof(char) != msg["device"].m_size ) return false;
	if ( msg["deviceId"].IsValid() && sizeof(char) != msg["deviceId"].m_size ) return false;
	Device::Type::Type device = (Device::Type::Type)(char)msg["device"];
	if ( Device::Type::cpu != device )
	{
		if ( !msg["wanIP"].IsValid() || 1 > msg["wanIP"].m_size ) return false;
		if ( !msg["wanPort"].IsValid() || sizeof(int) != msg["wanPort"].m_size ) return false;
		if ( !msg["lanIP"].IsValid() || 1 > msg["lanIP"].m_size ) return false;
		if ( !msg["lanPort"].IsValid() || sizeof(int) != msg["lanPort"].m_size ) return false;
	}
	
	int deviceId = -1;
	if ( msg["deviceId"].IsValid() ) deviceId = (unsigned char)msg["deviceId"];
	//////////////////////////////////////////////////////////////////////////
	//业务处理
	if ( Device::Type::ssd== device ) //固态硬盘插入
	{
		if ( m_hardDiskCount == m_hardDisks.size() ) 
		{
			m_log.Info( "Error", "硬盘插槽已插满，拒绝硬盘(%s %d-%s %d)插入", 
				((std::string)msg["wanIP"]).c_str(), (int)msg["wanPort"], ((std::string)msg["lanIP"]).c_str(), (int)msg["lanPort"] );
			host.Close();
			return true;
		}
		if ( -1 != deviceId ) 
		{
			DeviceMap::iterator it =  m_hardDisks.find( deviceId );
			if ( it != m_hardDisks.end() )
			{
				m_log.Info( "Error", "硬盘设备ID重复，拒绝硬盘(%s %d-%s %d)插入", 
					((std::string)msg["wanIP"]).c_str(), (int)msg["wanPort"], ((std::string)msg["lanIP"]).c_str(), (int)msg["lanPort"] );
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
		m_hardDisks[deviceId].lanIP = (std::string)msg["lanIP"];
		m_hardDisks[deviceId].lanPort = msg["lanPort"];
		m_hardDisks[deviceId].wanIP = (std::string)msg["wanIP"];
		m_hardDisks[deviceId].wanPort = msg["wanPort"];
		m_hardDisks[deviceId].type = device;
		m_hardDisks[deviceId].status = Device::Status::running;
		m_log.Info( "Run", "%s(%s %d-%s %d)插入", Device::Descript( device ), 
			m_hardDisks[deviceId].wanIP.c_str(), m_hardDisks[deviceId].wanPort, m_hardDisks[deviceId].lanIP.c_str(), m_hardDisks[deviceId].lanPort ); 
		RunDevice( m_hardDisks[deviceId] );//硬盘不需要任何支持设备1插入就可以开始运行

		if ( m_hardDiskCount == m_hardDisks.size() ) //所有硬盘已运行，运行所有等待中的外存条
		{
			m_log.Info( "Run", "%d个%s已全部插入", m_hardDiskCount, Device::Descript( device ) ); 
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

	if ( Device::Type::exist == device ) //外存条插入
	{
		if ( m_existCount == m_exists.size() ) 
		{
			m_log.Info( "Error", "外存条插槽已插满，拒绝外存条(%s %d-%s %d)插入", 
				((std::string)msg["wanIP"]).c_str(), (int)msg["wanPort"], ((std::string)msg["lanIP"]).c_str(), (int)msg["lanPort"] );
			host.Close();
			return true;
		}

		if ( -1 != deviceId ) 
		{
			DeviceMap::iterator it =  m_exists.find( deviceId );
			if ( it != m_exists.end() )
			{
				m_log.Info( "Error", "外存条设备ID重复，拒绝外存条(%s %d-%s %d)插入", 
					((std::string)msg["wanIP"]).c_str(), (int)msg["wanPort"], ((std::string)msg["lanIP"]).c_str(), (int)msg["lanPort"] );
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
		m_exists[deviceId].lanIP = (std::string)msg["lanIP"];
		m_exists[deviceId].lanPort = msg["lanPort"];
		m_exists[deviceId].wanIP = (std::string)msg["wanIP"];
		m_exists[deviceId].wanPort = msg["wanPort"];
		m_exists[deviceId].type = device;
		m_exists[deviceId].status = Device::Status::waitDevice;
		m_log.Info( "Run", "%s(%s %d-%s %d)插入", Device::Descript( device ), 
			m_exists[deviceId].wanIP.c_str(), m_exists[deviceId].wanPort, m_exists[deviceId].lanIP.c_str(), m_exists[deviceId].lanPort ); 

		if ( m_hardDiskCount == m_hardDisks.size() ) //外存条需要硬盘做持久化支持，要等配置指定的所有硬盘都插入了，才能开始运行
		{
			RunDevice( m_exists[deviceId] );
		}

		if ( m_existCount == m_exists.size() ) //CPU需要外存条做数据支持，要等配置指定的所有外存条都插入了，才能开始运行
		{
			m_log.Info( "Run", "%d个%s已全部插入", m_existCount, Device::Descript( device ) ); 
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

	if ( Device::Type::cpu== device ) //CPU插入
	{
		if ( -1 != deviceId ) 
		{
			DeviceMap::iterator it =  m_cpus.find( deviceId );
			if ( it != m_cpus.end() )
			{
				m_log.Info( "Error", "CPU设备ID重复，拒绝CPU插入" );
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
		m_cpus[deviceId].lanIP = (std::string)msg["lanIP"];
		m_cpus[deviceId].lanPort = msg["lanPort"];
		m_cpus[deviceId].wanIP = (std::string)msg["wanIP"];
		m_cpus[deviceId].wanPort = msg["wanPort"];
		m_cpus[deviceId].status = Device::Status::waitDevice;
		m_log.Info( "Run", "%s(%s %d-%s %d)插入", Device::Descript( device ), 
			m_cpus[deviceId].wanIP.c_str(), m_cpus[deviceId].wanPort, m_cpus[deviceId].lanIP.c_str(), m_cpus[deviceId].lanPort ); 

		if ( m_existCount == m_exists.size() ) //CPU需要外存条做数据支持，要等配置指定的所有外存条都插入了，才能开始运行
		{
			RunDevice( m_cpus[deviceId] );
		}
		
		return true;
	}

	if ( Device::Type::screen== device || Device::Type::touch== device ) //显示屏插入
	{
		if ( -1 != deviceId ) 
		{
			DeviceMap::iterator it =  m_screens.find( deviceId );
			if ( it != m_cpus.end() )
			{
				m_log.Info( "Error", "触摸屏设备ID重复，拒绝触摸屏插入" );
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
		m_screens[deviceId].lanIP = (std::string)msg["lanIP"];
		m_screens[deviceId].lanPort = msg["lanPort"];
		m_screens[deviceId].wanIP = (std::string)msg["wanIP"];
		m_screens[deviceId].wanPort = msg["wanPort"];
		m_screens[deviceId].status = Device::Status::waitDevice;
		m_log.Info( "Run", "%s(%s %d-%s %d)插入", Device::Descript( device ), 
			m_screens[deviceId].wanIP.c_str(), m_screens[deviceId].wanPort, m_screens[deviceId].lanIP.c_str(), m_screens[deviceId].lanPort ); 
		RunDevice( m_screens[deviceId] );//触摸屏不需要任何支持设备1插入就可以开始运行

		return true;
	}

	return true;
}

