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
 *	NoDB������
 */
typedef struct NO_DB_CONTEXT
{
	Device::INFO device;
//	/*
//		��Ƭ��
//		distributed=true����role=pieceʱ�������ã��������
//		piece���͵Ľ�㸺���ṩ������������ڵ�����������
//		Ƭ�Ŵ�0��ʼ,Ƭ�ŵļ��㹫ʽΪ:���ݵ�hashvalue/��Ƭ��С
//		������������hashvalue����1234,��Ƭ��С������Ϊ100,��ô�����ݾͱ���Ϊ��11����Ƭ
//		0�ŷ�Ƭ����hashvalueΪ0~99������ 0~99֮���κ�����/100 = 0,
//		1�ŷ�Ƭ����hashvalueΪ100~199������ 100~199֮���κ�����/100 = 1,
//		...
//		10�ŷ�Ƭ����hashvalueΪ1000~1999������ 1000~1999֮���κ�����/100 = 10,
//
//		����С��0��Ƭ��ʱ,��ʾ��Ƭ������,����������,ֻ�ǹ��������д�����
//		�ȴ��û�ָ��ȷ���Լ���Ƭ�����ݽ��м���,�ṩ����
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
	//���������߳�
	virtual int Main();
	
	void OnConnect(mdk::STNetHost &host);//���ӵ�����Ӧ
	virtual void OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len);
	virtual void OnWork( mdk::STNetHost &host, bsp::BStruct &msg );//���ı���Ӧ����true,���򷵻�false

	//����Guide�����ϵ����ݲ��������host����Guide��ʲô������������false
	bool OnGuideMsg( mdk::STNetHost &host, bsp::BStruct &msg );
	//���ӶϿ���Ӧ
	void OnCloseConnect(mdk::STNetHost &host);
	//���У����Ӧ
	void OnSetDeviceId( mdk::STNetHost &host, bsp::BStruct &msg );
	//�������ݿ�
	void OnDevicePostion( mdk::STNetHost &host, bsp::BStruct &msg );
	//������Ƭ
	void OnRunDevice( mdk::STNetHost &host, bsp::BStruct &msg );
		
	//����client�����ϵ����ݲ��������host��Guide��ʲô������������false
	bool OnClientMsg( mdk::STNetHost &host, bsp::BStruct &msg );
private:
	//�˳�����
	bool CreateDir( const char *strDir );
	bool CreateFile( const char *strFile );
	void ReadData();//��ȡ���ݵ���̬Ӳ��
	void Heartbeat( time_t tCurTime );//����
	void SaveData(time_t tCurTime);//���ݳ־û�
	
private:
	mdk::Logger m_log;
	mdk::ConfigFile m_cfgFile;
	NO_DB_CONTEXT m_context;//NoDB������
	Device::INFO m_motherBoard;//����
	mdk::Thread t_exit;//�˳������߳�
	typedef std::map<mdk::uint64, Device::INFO*> IpMap;
	IpMap m_hardDiskIpMap;
	typedef std::map<int, Device::INFO*> DeviceMap;
	DeviceMap m_hardDisks;//Ӳ��

	mdk::Thread t_loaddata;//���ݼ����߳�
	mdk::Mutex m_ioLock;//io��

	RHTable m_nosqlDB;
};

#endif // !defined NOSQL_H
