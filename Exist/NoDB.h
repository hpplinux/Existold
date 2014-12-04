// NoDB.h: interface for the NoDB class.
//
//////////////////////////////////////////////////////////////////////

#ifndef NOSQL_H
#define NOSQL_H

#include <ctime>
#include "mdk/ConfigFile.h"
#include "mdk/Signal.h"
#include "mdk/Logger.h"
#include "mdk/Thread.h"
#include "mdk/Lock.h"
#include "mdk/Socket.h"
#include "RHTable.h"
#include <string>
#include <map>

#include "../common/common.h"
#include "../common/BStructSvr.h"
#include "../common/ExistFS.h"
#include "../include/Exist/frame/ExistType.h"
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
//		Ƭ�Ŵ�0��ʼ,Ƭ�ŵļ��㹫ʽΪ:���ݵ�hashexist::VALUE/��Ƭ��С
//		������������hashexist::VALUE����1234,��Ƭ��С������Ϊ100,��ô�����ݾͱ���Ϊ��11����Ƭ
//		0�ŷ�Ƭ����hashexist::VALUEΪ0~99������ 0~99֮���κ�����/100 = 0,
//		1�ŷ�Ƭ����hashexist::VALUEΪ100~199������ 100~199֮���κ�����/100 = 1,
//		...
//		10�ŷ�Ƭ����hashexist::VALUEΪ1000~1999������ 1000~1999֮���κ�����/100 = 10,
//
//		����С��0��Ƭ��ʱ,��ʾ��Ƭ������,����������,ֻ�ǹ��������д�����
//		�ȴ��û�ָ��ȷ���Լ���Ƭ�����ݽ��м���,�ṩ����
//	*/
//	int pieceNo;
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
	virtual void OnWork( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData );//���ı���Ӧ����true,���򷵻�false

	//����Guide�����ϵ����ݲ��������host����Guide��ʲô������������false
	bool OnGuideMsg( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData );
	//���ӶϿ���Ӧ
	void OnCloseConnect(mdk::STNetHost &host);
	//���У����Ӧ
	void OnSetDeviceId( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData );
	//�������ݿ�
	void OnDevicePostion( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData );
	//������Ƭ
	void OnRunDevice( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData );
		
	//����client�����ϵ����ݲ��������host��Guide��ʲô������������false
	bool OnClientMsg( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData );

	void OnCreateData(CREATE_DATA *pParam, unsigned char *path, int size);
	void OnDeleteData(exist::DATA_KEY *pParam, unsigned char *path, int size);
	void OnWriteData(WRITE_DATA *pParam, unsigned char* pData, unsigned char *path, int size);
	void OnReadData(mdk::STNetHost &host, exist::DATA_KEY *pParam, unsigned char *path, int size);

private:
	void Heartbeat( time_t tCurTime );//����
	static void AddSelf( exist::VALUE *pValue, unsigned char *pData );
	static void SubtractSelf( exist::VALUE *pValue, unsigned char *pData );
	static void MultiplySelf( exist::VALUE *pValue, unsigned char *pData );
	static bool DivideSelf( exist::VALUE *pValue, unsigned char *pData );
	RHTable* GetNosql(DataType::DataType type, unsigned char *path, int size, exist::VALUE **pParent);
	RHTable* FindNosql(RHTable* pNosql, unsigned char *path, int size, exist::VALUE **pParent);
	exist::VALUE* CreateData(CREATE_DATA *pParam, unsigned char *path, int size);//��������
	mdk::Socket* GetSSDConnect(int keyHashid);//ȡ�ù�̬Ӳ�̵�ͬ������

private:
	mdk::Logger m_log;
	mdk::ConfigFile m_cfgFile;
	NO_DB_CONTEXT m_context;//NoDB������
	Device::INFO m_motherBoard;//����
	mdk::Thread t_exit;//�˳������߳�

	RHTable m_nosqlDB;//�ܱ�
	//���������Ͷ�Ӧ��
	RHTable m_int8DB;
	RHTable m_uint8DB;
	RHTable m_int16DB;
	RHTable m_uint16DB;
	RHTable m_int32DB;
	RHTable m_uint32DB;
	RHTable m_int64DB;
	RHTable m_uint64DB;
	RHTable m_floatDB;
	RHTable m_doubleDB;

	//////////////////////////////////////////////////////////////////////////
	//�־û����
private:
	void ReadDataFromSSD( exist::VALUE *pValue, unsigned char *path, int size );//�ӹ�̬Ӳ��ȡ����
	void SaveDataToSSD( exist::VALUE *pValue, unsigned char *path, int size );//�������ݵ���̬Ӳ��

	void ReadData();//��ȡ���ݵ���̬Ӳ��
	const char* ReadRootData();//��ȡ��Ŀ¼���ݵ���̬Ӳ��
	void ReadDataFromFile( exist::VALUE *pValue );//���ļ���ȡһ������
	void AddSaveWaiter(exist::VALUE *pValue, WRITE_DATA *pParam);//�����¼���m_waitSaveDatas
	void SaveData(time_t tCurTime);//���ݳ־û�

private:
	typedef std::map<int, Device::INFO*> DeviceMap;
	DeviceMap m_hardDisks;//��̬Ӳ��
	std::map<int,mdk::Socket>	m_hardConnects;//��̬Ӳ��ͬ������
	std::map<exist::VALUE*, exist::VALUE*>	m_waitSaveDatas;//��д�����ݣ�5S/1000�����ºϲ�д��
	int m_maxCachedTime;//��󻺴�����(��λS)����ʼд���ļ�
	int m_maxCachedCount;//��󻺴���������(��������A������3�Σ�����B�´���������2�����ݴ�д��)����ʼд���ļ�

	exist::ExistFS m_existFS;//�ļ�ϵͳ�ӿ�
};

#endif // !defined NOSQL_H
