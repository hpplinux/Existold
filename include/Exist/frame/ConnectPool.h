// ConnectPool.h: interface for the ConnectPool class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONNECTPOOL_H
#define CONNECTPOOL_H

#include "mdk/Socket.h"
#include "mdk/Thread.h"
#include "mdk/Signal.h"
#include "mdk/Logger.h"
#include "mdk/Lock.h"
#include <map>
#include <vector>
#include <string>
#include "../../../common/common.h"

namespace Exist
{
	
class ConnectPool  
{
public:
	ConnectPool();
	virtual ~ConnectPool();

	bool PlugIn();
	mdk::Socket& GetSocket( unsigned int hashId );

private:
	bool CreateDir( const char *strDir );
	bool CreateFile( const char *strFile );
private:
	mdk::Logger m_log;
	mdk::Thread m_guideThread;
	void* RemoteCall GuideThread( void * );
	Device::INFO										m_cpu;//�豸��Ϣ
	std::vector<Device::INFO>							m_existAdrs;//����ַ��
	bool												m_sockInited;//windows���ѵ���WSAStartup()
	mdk::Signal											m_sigInited;//��ʼ���ź�	
	mdk::Mutex											m_connectsLock;//m_connectMap�̰߳�ȫ
	std::map<mdk::uint64, std::vector<mdk::Socket> >	m_connectMap;//[�߳�Id]<->[�̶߳�ռ��������ӱ�]ӳ��
	
};

}
#endif // ifndef CONNECTPOOL_H

