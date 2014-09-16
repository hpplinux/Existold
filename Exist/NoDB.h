// NoDB.h: interface for the NoDB class.
//
//////////////////////////////////////////////////////////////////////

#ifndef NOSQL_H
#define NOSQL_H

#include <ctime>
#include "../Micro-Development-Kit/include/mdk/ConfigFile.h"
#include "../Micro-Development-Kit/include/mdk/Signal.h"
#include "../Micro-Development-Kit/include/mdk/Logger.h"
#include "../Micro-Development-Kit/include/mdk/Thread.h"
#include "../Micro-Development-Kit/include/mdk/Lock.h"
#include "RHTable.h"
#include <string>

#include "../bstruct/include/BArray.h"

#include "../common/common.h"
#include "../common/Protocol.h"
#include "../common/BStructSvr.h"


class NoDB : public BStructSvr
{
/*
 *	NoDB上下文
 */
typedef struct NO_DB_CONTEXT
{
	Device::INFO device;
//	/*
//		分片号
//		distributed=true并且role=piece时必须配置，否则忽略
//		piece类型的结点负责提供服务的数据所在的数据区间编号
//		片号从0开始,片号的计算公式为:数据的hashvalue/分片大小
//		比如总数据中hashvalue最大的1234,分片大小被设置为100,那么总数据就被分为了11个分片
//		0号分片上是hashvalue为0~99的数据 0~99之间任何数字/100 = 0,
//		1号分片上是hashvalue为100~199的数据 100~199之间任何数字/100 = 1,
//		...
//		10号分片上是hashvalue为1000~1999的数据 1000~1999之间任何数字/100 = 10,
//
//		配置小于0的片号时,表示分片启动后,不加载数据,只是挂在网络中待机，
//		等待用户指令确定对几号片的数据进行加载,提供服务
//	*/
//	int pieceNo;
	std::string dataRootDir;
	unsigned int maxMemory;
	bool m_bStop;
}NO_DB_CONTEXT;

public:
	NoDB(char* cfgFile);
	virtual ~NoDB();
	mdk::Logger& GetLog();
		
protected:
	//服务器主线程
	virtual int Main();
	
	void OnConnect(mdk::STNetHost &host);//连接到达响应
	virtual void OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len);
	virtual void OnWork( mdk::STNetHost &host, bsp::BStruct &msg );//报文被响应返回true,否则返回false

	//接收Guide连接上的数据并处理，如果host不是Guide则什么都不做，返回false
	bool OnGuideMsg( mdk::STNetHost &host, bsp::BStruct &msg );
	//连接断开响应
	void OnCloseConnect(mdk::STNetHost &host);
	//结点校正响应
	void OnSetDeviceId( mdk::STNetHost &host, bsp::BStruct &msg );
	//设置数据库
	void OnDevicePostion( mdk::STNetHost &host, bsp::BStruct &msg );
	//设置主片
	void OnRunDevice( mdk::STNetHost &host, bsp::BStruct &msg );
		
	//接收client连接上的数据并处理，如果host是Guide则什么都不做，返回false
	bool OnClientMsg( mdk::STNetHost &host, bsp::BStruct &msg );
private:
	//退出程序
	bool CreateDir( const char *strDir );
	bool CreateFile( const char *strFile );
	void ReadData();//读取数据到固态硬盘
	void Heartbeat( time_t tCurTime );//心跳
	void SaveData(time_t tCurTime);//数据持久化
	
private:
	mdk::Logger m_log;
	mdk::ConfigFile m_cfgFile;
	NO_DB_CONTEXT m_context;//NoDB上下文
	Device::INFO m_motherBoard;//主板
	mdk::Thread t_exit;//退出程序线程
	typedef std::map<mdk::uint64, Device::INFO*> IpMap;
	IpMap m_hardDiskIpMap;
	typedef std::map<int, Device::INFO*> DeviceMap;
	DeviceMap m_hardDisks;//硬盘

	mdk::Thread t_loaddata;//数据加载线程
	mdk::Mutex m_ioLock;//io锁

	RHTable m_nosqlDB;
};

#endif // !defined NOSQL_H
