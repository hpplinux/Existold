// onnectPool.h: interface for the ConnectPool class.
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
	mdk::Logger m_log;
	mdk::Thread m_guideThread;
	void* RemoteCall GuideThread( void * );
	Device::INFO m_cpu;
	std::vector<Device::INFO>							m_existAdrs;

	bool m_sockInited;
	mdk::Signal m_sigInited;

	
	mdk::Mutex											m_connectsLock;
	std::map<unsigned int, std::vector<mdk::Socket> >	m_connectMap;
	
};

}
#endif // ifndef CONNECTPOOL_H

