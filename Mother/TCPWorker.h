// TCPWorker.h: interface for the TCPWorker class.
//
//////////////////////////////////////////////////////////////////////

#ifndef TCPWORKER_H
#define TCPWORKER_H

#include "../Micro-Development-Kit/include/mdk/ConfigFile.h"
#include "../Micro-Development-Kit/include/mdk/Logger.h"
#include "../common/BStructSvr.h"
#include "../common/common.h"
#include "../common/Protocol.h"

#include <vector>
#include <map>

/*
	tcp������
	����tcp��Ϣ��ִ��ҵ��
 */
class TCPWorker : public BStructSvr
{
	/*
	 *	�豸ӳ���
	 *	key = host.ID()
	 */
	typedef std::map<int, Device::INFO> DeviceMap;
public:
	TCPWorker( char *cfgFile );
	virtual ~TCPWorker();
	mdk::Logger& GetLog();

	virtual int Main();
	virtual void OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len);//�Ƿ�����
	virtual void OnWork( mdk::STNetHost &host, bsp::BStruct &msg );
	void OnCloseConnect(mdk::STNetHost &host);

private:
	bool ReadConfig();
	//////////////////////////////////////////////////////////////////////////
	//������Ӧ
	bool OnPlugIn(mdk::STNetHost &host, bsp::BStruct &msg);//�豸�������,������ȷ����true,���򷵻�false

	//////////////////////////////////////////////////////////////////////////
	//֪ͨ�豸
	void SetDeviceId( mdk::STNetHost &host, unsigned char deviceId );//�����豸ID
	void RunDevice( Device::INFO &device );//�����豸
	void DevicePostion( mdk::STNetHost &host  );//֪ͨ�豸λ��

private:
	mdk::Logger m_log;//����log
	mdk::ConfigFile m_cfgFile;//�����ļ�
	DeviceMap m_exists;//�Ѳ���������
	DeviceMap m_hardDisks;//�Ѳ���Ĺ�̬Ӳ��
	DeviceMap m_screens;//�Ѳ������ʾ��
	DeviceMap m_cpus;//�Ѳ����CPU
	int m_existCount;//֧��ϵͳ������Ҫ����������������m_existCount <= 0 ��ʾ����ҪӲ��
	int m_hardDiskCount;//֧��ϵͳ����������Ҫ�����Ӳ��������m_hardDiskCount <= 0 ��ʾ����ҪӲ��
	

	int m_svrPort;//����exist�������ӵĶ˿�
	int m_heartTime;//exist����ʱ��

};

#endif // !defined TCPWORKER_H
