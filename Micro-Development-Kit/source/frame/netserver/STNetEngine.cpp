
#include "../../../include/frame/netserver/STIocp.h"
#include "../../../include/frame/netserver/STEpoll.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#define strnicmp strncasecmp
#endif

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <time.h>

#include "../../../include/mdk/Socket.h"
#include "../../../include/mdk/atom.h"
#include "../../../include/mdk/MemoryPool.h"
#include "../../../include/mdk/mapi.h"

#include "../../../include/frame/netserver/STNetEngine.h"
#include "../../../include/frame/netserver/STNetConnect.h"
#include "../../../include/frame/netserver/STNetServer.h"

using namespace std;
namespace mdk
{
	
STNetEngine::STNetEngine()
{
	Socket::SocketInit();
	m_pConnectPool = NULL;
	m_stop = true;//ֹͣ��־
	m_startError = "";
	m_nHeartTime = 0;//�������(S)��Ĭ�ϲ����
#ifdef WIN32
	m_pNetMonitor = new STIocp;
#else
	m_pNetMonitor = new STEpoll;
#endif
	m_pNetServer = NULL;
	m_averageConnectCount = 5000;
}

STNetEngine::~STNetEngine()
{
	Stop();
	if ( NULL != m_pConnectPool )
	{
		delete m_pConnectPool;
		m_pConnectPool = NULL;
	}
	Socket::SocketDestory();
}

//����ƽ��������
void STNetEngine::SetAverageConnectCount(int count)
{
	m_averageConnectCount = count;
}

//��������ʱ��
void STNetEngine::SetHeartTime( int nSecond )
{
	m_nHeartTime = nSecond;
}

/**
 * ��ʼ����
 * �ɹ�����true��ʧ�ܷ���false
 */
bool STNetEngine::Start()
{
	if ( !m_stop ) return true;
	m_stop = false;	
	
	int memoryCount = 2;
	for ( memoryCount = 2; memoryCount * memoryCount < m_averageConnectCount * 2; memoryCount++ );
	if ( memoryCount < 200 ) memoryCount = 200;
	if ( NULL != m_pConnectPool )//֮ǰStop()��,����������
	{
		delete m_pConnectPool;
		m_pConnectPool = NULL;
	}
	m_pConnectPool = new MemoryPool( sizeof(STNetConnect), memoryCount );
	if ( NULL == m_pConnectPool )
	{
		m_startError = "�ڴ治�㣬�޷�����NetConnect�ڴ��";
		Stop();
		return false;
	}
	if ( !m_pNetMonitor->Start( MAXPOLLSIZE ) ) 
	{
		m_startError = m_pNetMonitor->GetInitError();
		Stop();
		return false;
	}
	if ( !ListenAll() )
	{
		Stop();
		return false;
	}
	ConnectAll();
	return m_mainThread.Run( Executor::Bind(&STNetEngine::Main), this, 0 );
}

bool STNetEngine::WINIO(int timeout)
{
#ifdef WIN32
	STIocp::IO_EVENT e;
	if ( !m_pNetMonitor->WaitEvent( e, timeout ) ) return false;
	switch( e.type )
	{
	case STIocp::timeout :
		break;
	case STIocp::stop :
		return false;
		break;
	case STIocp::connect :
		OnConnect( e.client, false );
		m_pNetMonitor->AddAccept( e.sock );
		break;
	case STIocp::recv :
		OnData( e.sock, e.pData, e.uDataSize );
		break;
	case STIocp::close :
		OnClose( e.sock );
		break;
	case STIocp::send :
		OnSend( e.sock, e.uDataSize );
		break;
	default:
		break;
	}
	return true;
#endif
	return false;
}

bool STNetEngine::LinuxIO( int timeout )
{
#ifndef WIN32
	int nCount = 0;
	int eventType = 0;
	int i = 0;
	Socket sockListen;
	Socket sockClient;
	SOCKET sock;
	map<SOCKET,int>::iterator it;
	pair<map<SOCKET,int>::iterator,bool> ret;
	
	//û�п�io��socket��ȴ��¿�io��socket
	//�������Ƿ����µĿ�io��socket������ȡ�����뵽m_ioList�У�û��Ҳ���ȴ�
	//��������m_ioList�е�socket����io����
	if ( 0 >= m_ioList.size() ) nCount = m_pNetMonitor->WaitEvent( timeout );
	else nCount = m_pNetMonitor->WaitEvent( 0 );
	if ( 0 > nCount ) return false;
	//���뵽m_ioList��
	for ( i = 0; i < nCount; i++ )
	{
		sock = m_pNetMonitor->GetSocket(i);
		if ( INVALID_SOCKET == sock ) return false;//STEpoll�ѹر�
		if ( m_pNetMonitor->IsAcceptAble(i) )//��������ֱ��ִ��ҵ�� 
		{
			while ( true )
			{
				sockListen.Detach();
				sockListen.Attach(sock);
				sockListen.Accept( sockClient );
				if ( INVALID_SOCKET == sockClient.GetSocket() ) break;
				sockClient.SetSockMode();
				OnConnect(sockClient.Detach(), false);
			}
			continue;
		}
		//���Ǽ���socketһ����io�¼�
		//���뵽io�б�ͳһ����
		if ( m_pNetMonitor->IsWriteAble(i) ) eventType = 1|2;//recv+send�¼�
		else eventType = 1;//recv�¼�
		ret = m_ioList.insert(map<SOCKET,int>::value_type(sock,eventType) );//���ӿ�io�Ķ���
		if ( !ret.second ) ret.first->second = ret.first->second|eventType;//�������¼�
	}
	//����m_ioList��ִ��1��io
	for ( it = m_ioList.begin(); it != m_ioList.end(); it++ )
	{
		if ( 1&it->second ) //�ɶ�
		{
			if ( ok != OnData( it->first, 0, 0 ) ) //�����Ѷ���������ѶϿ�
			{
				it->second = it->second&~1;//����¼�
			}
		}
		if ( 2&it->second ) //��д
		{
			if ( ok != OnSend( it->first, 0 ) )//�����Ѿ������꣬��socket�Ѿ��Ͽ�����socket����д
			{
				it->second = it->second&~2;//����¼�
			}
		}
	}
	
	//������io��socket���
	it = m_ioList.begin();
	while (  it != m_ioList.end() ) 
	{
		if ( 0 == it->second ) 
		{
			m_ioList.erase(it);
			it = m_ioList.begin();
			continue;
		}
		it++;
	}
	return true;
#endif
	return false;
}

//�ȴ�ֹͣ
void STNetEngine::WaitStop()
{
	m_mainThread.WaitStop();
}

//ֹͣ����
void STNetEngine::Stop()
{
	if ( m_stop ) return;
	m_stop = true;
	m_pNetMonitor->Stop();
	m_mainThread.Stop(3000);
#ifndef WIN32
	m_ioList.clear();
#endif
}

//���߳�
void* STNetEngine::Main(void*)
{
	time_t lastConnect = time(NULL);
	time_t curTime = time(NULL);
	
	bool mainFinished = false;
	while ( !m_stop ) 
	{
		if ( !mainFinished )
		{
			if ( 0== m_pNetServer->Main() ) mainFinished = true;
		}
#ifdef WIN32
		if ( !WINIO( 10000 ) ) break;
#else
		if ( !LinuxIO( 10000 ) ) break;
#endif
		Select();
		curTime = time(NULL);
		if ( 10000 <= curTime - lastConnect ) continue;
		lastConnect = curTime;
		HeartMonitor();
		ConnectAll();
	}
	return NULL;
}

//�����߳�
void STNetEngine::HeartMonitor()
{
	if ( 0 >= m_nHeartTime ) return;
	//////////////////////////////////////////////////////////////////////////
	//�ر�������������
	ConnectList::iterator it;
	STNetConnect *pConnect;
	time_t tCurTime = 0;
	/*	
		����һ����ʱ���ͷ��б���Ҫ�ͷŵĶ��󣬵ȱ�������1���Ե���ȴ��ͷŶ����б�
		������ѭ������Ϊ�ظ���Ϊ�ȴ��ͷ��б�ķ��ʶ���������
	 */
	tCurTime = time( NULL );
	time_t tLastHeart;
	for ( it = m_connectList.begin(); it != m_connectList.end(); )//����ʱ��<=0����������,��������
	{
		pConnect = it->second;
		if ( pConnect->m_host.IsServer() ) //�������� �����������
		{
			it++;
			continue;
		}
		//�������
		tLastHeart = pConnect->GetLastHeart();
		if ( tCurTime < tLastHeart || tCurTime - tLastHeart < m_nHeartTime )//������
		{
			it++;
			continue;
		}
		//������/�����ѶϿ���ǿ�ƶϿ����ӣ������ͷ��б�
		CloseConnect( it );
		it = m_connectList.begin();
	}
}

//�ر�һ������
void STNetEngine::CloseConnect( ConnectList::iterator it )
{
	/*
	   ������ɾ���ٹرգ�˳���ܻ���
	   ����رպ�eraseǰ��������client���ӽ�����
	   ϵͳ���̾ͰѸ����ӷ������clientʹ�ã������client�ڲ���m_connectListʱʧ��
	*/
	STNetConnect *pConnect = it->second;
	m_connectList.erase( it );//֮�󲻿�����MsgWorker()��������ΪOnData�����Ѿ��Ҳ���������
	AtomDec(&pConnect->m_useCount, 1);//m_connectList�������
	pConnect->GetSocket()->Close();
	pConnect->m_bConnect = false;
	if ( 0 == AtomAdd(&pConnect->m_nReadCount, 1) ) NotifyOnClose(pConnect);
	pConnect->Release();//���ӶϿ��ͷŹ������
	return;
}

void STNetEngine::NotifyOnClose(STNetConnect *pConnect)
{
	if ( 0 == AtomAdd(&pConnect->m_nDoCloseWorkCount, 1) )//ֻ��1���߳�ִ��OnClose���ҽ�ִ��1��
	{
		SetServerClose(pConnect);//���ӵķ���Ͽ�
		m_pNetServer->OnCloseConnect( pConnect->m_host );
	}
}

int g_c = 0;
bool STNetEngine::OnConnect( SOCKET sock, bool isConnectServer )
{
	AtomAdd(&g_c, 1);
	STNetConnect *pConnect = new (m_pConnectPool->Alloc())STNetConnect(sock, isConnectServer, m_pNetMonitor, this, m_pConnectPool);
	if ( NULL == pConnect ) 
	{
		closesocket(sock);
		return false;
	}
	//��������б�
	pConnect->RefreshHeart();
	AtomAdd(&pConnect->m_useCount, 1);//��m_connectList����
	pair<ConnectList::iterator, bool> ret = m_connectList.insert( ConnectList::value_type(pConnect->GetSocket()->GetSocket(),pConnect) );
	//ִ��ҵ��
	STNetHost accessHost = pConnect->m_host;//��������ʣ��ֲ������뿪ʱ�����������Զ��ͷŷ���
	m_pNetServer->OnConnect( pConnect->m_host );
	/*
		��������
		�����OnConnect��ɣ��ſ��Կ�ʼ���������ϵ�IO�¼�
		���򣬿���ҵ�����δ������ӳ�ʼ�����������յ�OnMsg֪ͨ��
		����ҵ��㲻֪������δ�����Ϣ
	 */
	bool bMonitor = true;
	if ( !m_pNetMonitor->AddMonitor(sock) ) return false;
#ifdef WIN32
	bMonitor = m_pNetMonitor->AddRecv( 
		sock, 
		(char*)(pConnect->PrepareBuffer(BUFBLOCK_SIZE)), 
		BUFBLOCK_SIZE );
#else
	bMonitor = m_pNetMonitor->AddIO( sock, true, false );
#endif
	if ( !bMonitor ) CloseConnect(pConnect->GetSocket()->GetSocket());
	return true;
}

void STNetEngine::OnClose( SOCKET sock )
{
	ConnectList::iterator itNetConnect = m_connectList.find(sock);
	if ( itNetConnect == m_connectList.end() )return;//�ײ��Ѿ������Ͽ�
	CloseConnect( itNetConnect );
}

connectState STNetEngine::OnData( SOCKET sock, char *pData, unsigned short uSize )
{
	connectState cs = unconnect;
	ConnectList::iterator itNetConnect = m_connectList.find(sock);//client�б������
	if ( itNetConnect == m_connectList.end() ) return cs;//�ײ��Ѿ��Ͽ�
	STNetConnect *pConnect = itNetConnect->second;
	STNetHost accessHost = pConnect->m_host;//��������ʣ��ֲ������뿪ʱ�����������Զ��ͷŷ���

	pConnect->RefreshHeart();
	cs = RecvData( pConnect, pData, uSize );
	if ( unconnect == cs )
	{
		OnClose( sock );
		return cs;
	}
	if ( 0 != AtomAdd(&pConnect->m_nReadCount, 1) ) return cs;
	//ִ��ҵ��STNetServer::OnMsg();
	MsgWorker(pConnect);
	return cs;
}

void* STNetEngine::MsgWorker( STNetConnect *pConnect )
{
	for ( ; !m_stop; )
	{
		m_pNetServer->OnMsg( pConnect->m_host );//�޷���ֵ���������߼������ڿͻ�ʵ��
		if ( !pConnect->m_bConnect ) break;
		if ( !pConnect->IsReadAble() ) break;
	}
	AtomDec(&pConnect->m_nReadCount,1);
	//ȷ��NetServer::OnClose()һ��������NetServer::OnMsg()���֮��
	if ( !pConnect->m_bConnect ) NotifyOnClose(pConnect);
	return 0;
}

connectState STNetEngine::RecvData( STNetConnect *pConnect, char *pData, unsigned short uSize )
{
#ifdef WIN32
	pConnect->WriteFinished( uSize );
	if ( !m_pNetMonitor->AddRecv(  pConnect->GetSocket()->GetSocket(), 
		(char*)(pConnect->PrepareBuffer(BUFBLOCK_SIZE)), BUFBLOCK_SIZE ) )
	{
		return unconnect;
	}
#else
	unsigned char* pWriteBuf = NULL;	
	int nRecvLen = 0;
	unsigned int nMaxRecvSize = 0;
	//������1M���ݣ��ø��������ӽ���io
	while ( nMaxRecvSize < 1048576 )
	{
		pWriteBuf = pConnect->PrepareBuffer(BUFBLOCK_SIZE);
		nRecvLen = pConnect->GetSocket()->Receive(pWriteBuf, BUFBLOCK_SIZE);
		if ( nRecvLen < 0 ) return unconnect;
		if ( 0 == nRecvLen ) 
		{
			if ( !m_pNetMonitor->AddIO(pConnect->GetSocket()->GetSocket(), true, false) ) return unconnect;
			return wait_recv;
		}
		nMaxRecvSize += nRecvLen;
		pConnect->WriteFinished( nRecvLen );
	}
#endif
	return ok;
}

//�ر�һ������
void STNetEngine::CloseConnect( SOCKET sock )
{
	ConnectList::iterator itNetConnect = m_connectList.find( sock );
	if ( itNetConnect == m_connectList.end() ) return;//�ײ��Ѿ������Ͽ�
	CloseConnect( itNetConnect );
}

//��Ӧ��������¼�
connectState STNetEngine::OnSend( SOCKET sock, unsigned short uSize )
{
	connectState cs = unconnect;
	ConnectList::iterator itNetConnect = m_connectList.find(sock);
	if ( itNetConnect == m_connectList.end() )return cs;//�ײ��Ѿ������Ͽ�
	STNetConnect *pConnect = itNetConnect->second;
	STNetHost accessHost = pConnect->m_host;//��������ʣ��ֲ������뿪ʱ�����������Զ��ͷŷ���
	if ( pConnect->m_bConnect ) cs = SendData(pConnect, uSize);

	return cs;
}

connectState STNetEngine::SendData(STNetConnect *pConnect, unsigned short uSize)
{
#ifdef WIN32
	unsigned char buf[BUFBLOCK_SIZE];
	if ( uSize > 0 ) pConnect->m_sendBuffer.ReadData(buf, uSize);
	int nLength = pConnect->m_sendBuffer.GetLength();
	if ( 0 >= nLength ) 
	{
		pConnect->SendEnd();//���ͽ���
		nLength = pConnect->m_sendBuffer.GetLength();//�ڶ��μ�鷢�ͻ���
		if ( 0 >= nLength ) 
		{
		/*
		���1���ⲿ�����߳�δ��ɷ��ͻ���д��
		�ⲿ�߳����д��ʱ�������ڷ������̣����߳�SendStart()�ض��ɹ�
		���ۣ�����©����
		����������������������
			*/
			return ok;//û�д��������ݣ��˳������߳�
		}
		/*
		�ⲿ�����߳�����ɷ��ͻ���д��
		���̲߳���SendStart()��ֻ��һ���ɹ�
		���ۣ�������ֲ������ͣ�Ҳ����©����
		*/
		if ( !pConnect->SendStart() ) return ok;//�Ѿ��ڷ���
		//�������̿�ʼ
	}
	
	if ( nLength > BUFBLOCK_SIZE )
	{
		pConnect->m_sendBuffer.ReadData(buf, BUFBLOCK_SIZE, false);
		m_pNetMonitor->AddSend( pConnect->GetSocket()->GetSocket(), (char*)buf, BUFBLOCK_SIZE );
	}
	else
	{
		pConnect->m_sendBuffer.ReadData(buf, nLength, false);
		m_pNetMonitor->AddSend( pConnect->GetSocket()->GetSocket(), (char*)buf, nLength );
	}
	return ok;
#else
	connectState cs = wait_send;//Ĭ��Ϊ�ȴ�״̬
	//////////////////////////////////////////////////////////////////////////
	//ִ�з���
	unsigned char buf[BUFBLOCK_SIZE];
	int nSize = 0;
	int nSendSize = 0;
	int nFinishedSize = 0;
	nSendSize = pConnect->m_sendBuffer.GetLength();
	if ( 0 < nSendSize )
	{
		nSize = 0;
		//һ�η���4096byte
		if ( BUFBLOCK_SIZE < nSendSize )//1�η����꣬����Ϊ����״̬
		{
			pConnect->m_sendBuffer.ReadData(buf, BUFBLOCK_SIZE, false);
			nSize += BUFBLOCK_SIZE;
			nSendSize -= BUFBLOCK_SIZE;
			cs = ok;
		}
		else//1�οɷ��꣬����Ϊ�ȴ�״̬
		{
			pConnect->m_sendBuffer.ReadData(buf, nSendSize, false);
			nSize += nSendSize;
			nSendSize = 0;
			cs = wait_send;
		}
		nFinishedSize = pConnect->GetSocket()->Send((char*)buf, nSize);//����
		if ( -1 == nFinishedSize ) cs = unconnect;
		else
		{
			pConnect->m_sendBuffer.ReadData(buf, nFinishedSize);//�����ͳɹ������ݴӻ������
			if ( nFinishedSize < nSize ) //sock��д��������Ϊ�ȴ�״̬
			{
				cs = wait_send;
			}
		}
		
	}
	if ( ok == cs || unconnect == cs ) return cs;//����״̬�����ӹر�ֱ�ӷ��أ����ӹرղ��ؽ����������̣�pNetConnect����ᱻ�ͷţ����������Զ�����
	
	//�ȴ�״̬���������η��ͣ��������·�������
	pConnect->SendEnd();//���ͽ���
	//////////////////////////////////////////////////////////////////////////
	//����Ƿ���Ҫ��ʼ�µķ�������
	if ( 0 >= pConnect->m_sendBuffer.GetLength() ) return cs;
	/*
	�ⲿ�����߳�����ɷ��ͻ���д��
	���̲߳���SendStart()��ֻ��һ���ɹ�
	���ۣ�������ֲ������ͣ�Ҳ����©����
	*/
	if ( !pConnect->SendStart() ) return cs;//�Ѿ��ڷ���
	return cs;
#endif
	return ok;
}

bool STNetEngine::Listen(int port)
{
	pair<map<int,SOCKET>::iterator,bool> ret 
		= m_serverPorts.insert(map<int,SOCKET>::value_type(port,INVALID_SOCKET));
	map<int,SOCKET>::iterator it = ret.first;
	if ( !ret.second && INVALID_SOCKET != it->second ) return true;
	if ( m_stop ) return true;

	it->second = ListenPort(port);
	if ( INVALID_SOCKET == it->second ) return false;
	return true;
}

SOCKET STNetEngine::ListenPort(int port)
{
	Socket listenSock;//����socket
	if ( !listenSock.Init( Socket::tcp ) ) return INVALID_SOCKET;
	listenSock.SetSockMode();
	if ( !listenSock.StartServer( port ) ) 
	{
		listenSock.Close();
		return INVALID_SOCKET;
	}
	if ( !m_pNetMonitor->AddMonitor( listenSock.GetSocket() ) ) 
	{
		listenSock.Close();
		return INVALID_SOCKET;
	}
	if ( !m_pNetMonitor->AddAccept( listenSock.GetSocket() ) )
	{
		listenSock.Close();
		return INVALID_SOCKET;
	}

	return listenSock.Detach();
}


bool STNetEngine::ListenAll()
{
	bool ret = true;
	map<int,SOCKET>::iterator it = m_serverPorts.begin();
	char strPort[256];
	string strFaild;
	for ( ; it != m_serverPorts.end(); it++ )
	{
		if ( INVALID_SOCKET != it->second ) continue;
		it->second = ListenPort(it->first);
		if ( INVALID_SOCKET == it->second ) 
		{
			sprintf( strPort, "%d", it->first );
			strFaild += strPort;
			strFaild += " ";
			ret = false;
		}
	}
	if ( !ret ) m_startError += "listen port:" + strFaild + "faild";
	return ret;
}


bool STNetEngine::Connect(const char* ip, int port, int reConnectTime)
{
	uint64 addr64 = 0;
	if ( !addrToI64(addr64, ip, port) ) return false;
	
	vector<SVR_CONNECT*> sockArray;
	map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.find(addr64);
	if ( it == m_keepIPList.end() ) m_keepIPList.insert( map<uint64,vector<SVR_CONNECT*> >::value_type(addr64,sockArray) );
	SVR_CONNECT *pSvr = new SVR_CONNECT;
	pSvr->reConnectSecond = reConnectTime;
	pSvr->lastConnect = 0;
	pSvr->sock = INVALID_SOCKET;
	pSvr->addr = addr64;
	pSvr->state = SVR_CONNECT::unconnected;
	m_keepIPList[addr64].push_back(pSvr);
	if ( m_stop ) return false;
	
	//�������ӽ��
	pSvr->lastConnect = time(NULL);
	if ( ConnectOtherServer(ip, port, pSvr->sock) )
	{
		pSvr->state = SVR_CONNECT::connected;
		OnConnect(pSvr->sock, true);
	}
	else
	{
		pSvr->state = SVR_CONNECT::connectting;
	}
	
	return true;
}

bool STNetEngine::ConnectOtherServer(const char* ip, int port, SOCKET &svrSock)
{
	svrSock = INVALID_SOCKET;
	Socket sock;//����socket
	if ( !sock.Init( Socket::tcp ) ) return false;
	sock.SetSockMode();
	svrSock = sock.Detach();
	bool successed = AsycConnect(svrSock, ip, port);
	return successed;
}

bool STNetEngine::ConnectAll()
{
	if ( m_stop ) return false;
	time_t curTime = time(NULL);
	char ip[24];
	int port;
	int i = 0;
	int count = 0;
	SOCKET sock = INVALID_SOCKET;
	
	//��������
	SVR_CONNECT *pSvr = NULL;
	map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.begin();
	vector<SVR_CONNECT*>::iterator itSvr;
	for ( ; it != m_keepIPList.end(); it++ )
	{
		i64ToAddr(ip, port, it->first);
		itSvr = it->second.begin();
		for ( ; itSvr != it->second.end();  )
		{
			pSvr = *itSvr;
			if ( SVR_CONNECT::connectting == pSvr->state 
				|| SVR_CONNECT::connected == pSvr->state 
				|| SVR_CONNECT::unconnectting == pSvr->state 
				) 
			{
				itSvr++;
				continue;
			}
			if ( 0 > pSvr->reConnectSecond && 0 != pSvr->lastConnect ) 
			{
				itSvr = it->second.erase(itSvr);
				delete pSvr;
				continue;
			}
			if ( curTime - pSvr->lastConnect < pSvr->reConnectSecond ) 
			{
				itSvr++;
				continue;
			}
			
			pSvr->lastConnect = curTime;
			if ( ConnectOtherServer(ip, port, pSvr->sock) )
			{
				pSvr->state = SVR_CONNECT::connected;
				OnConnect(pSvr->sock, true);
			}
			else 
			{
				pSvr->state = SVR_CONNECT::connectting;
			}
			itSvr++;
		}
	}
	
	return true;
}

void STNetEngine::SetServerClose(STNetConnect *pConnect)
{
	if ( !pConnect->m_host.IsServer() ) return;
	SOCKET sock = pConnect->GetID();
	map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.begin();
	int i = 0;
	int count = 0;
	SVR_CONNECT *pSvr = NULL;
	for ( ; it != m_keepIPList.end(); it++ )
	{
		count = it->second.size();
		for ( i = 0; i < count; i++ )
		{
			pSvr = it->second[i];
			if ( sock != pSvr->sock ) continue;
			pSvr->sock = INVALID_SOCKET;
			pSvr->state = SVR_CONNECT::unconnected;
			return;
		}
	}
}

//��ĳ�����ӹ㲥��Ϣ(ҵ���ӿ�)
void STNetEngine::BroadcastMsg( int *recvGroupIDs, int recvCount, char *msg, unsigned int msgsize, int *filterGroupIDs, int filterCount )
{
	ConnectList::iterator it;
	STNetConnect *pConnect;
	vector<STNetConnect*> recverList;
	STNetHost accessHost;
	//���������й㲥�������Ӹ��Ƶ�һ��������
	for ( it = m_connectList.begin(); it != m_connectList.end(); it++ )
	{
		pConnect = it->second;
		if ( !pConnect->IsInGroups(recvGroupIDs, recvCount) 
			|| pConnect->IsInGroups(filterGroupIDs, filterCount) ) continue;
		recverList.push_back(pConnect);
		accessHost = pConnect->m_host;//���û����ʣ��ֲ������뿪ʱ�����������Զ��ͷŷ���
	}
	
	//������е����ӿ�ʼ�㲥
	vector<STNetConnect*>::iterator itv = recverList.begin();
	for ( ; itv != recverList.end(); itv++ )
	{
		pConnect = *itv;
		if ( pConnect->m_bConnect ) pConnect->SendData((const unsigned char*)msg,msgsize);
	}
}

//��ĳ����������Ϣ(ҵ���ӿ�)
void STNetEngine::SendMsg( int hostID, char *msg, unsigned int msgsize )
{
	ConnectList::iterator itNetConnect = m_connectList.find(hostID);
	if ( itNetConnect == m_connectList.end() ) return;//�ײ��Ѿ������Ͽ�
	STNetConnect *pConnect = itNetConnect->second;
	STNetHost accessHost = pConnect->m_host;//���û����ʣ��ֲ������뿪ʱ�����������Զ��ͷŷ���

	if ( pConnect->m_bConnect ) pConnect->SendData((const unsigned char*)msg,msgsize);

	return;
}

const char* STNetEngine::GetInitError()//ȡ������������Ϣ
{
	return m_startError.c_str();
}

void STNetEngine::Select()
{
	fd_set readfds; 
	fd_set sendfds; 
	std::vector<SVR_CONNECT*> clientList;
	int clientCount;
	int startPos = 0;
	int endPos = 0;
	int i = 0;
	SOCKET maxSocket = 0;
	sockaddr_in sockAddr;
	char ip[32];
	int port;
	SVR_CONNECT *pSvr = NULL;
	
	//��������connectting״̬��sock�������б�
	clientList.clear();
	{
		vector<SVR_CONNECT*> sockArray;
		map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.begin();
		for ( ; it != m_keepIPList.end(); it++ )
		{
			for ( i = 0; i < it->second.size(); i++ )
			{
				pSvr = it->second[i];
				if ( SVR_CONNECT::connectting != pSvr->state ) continue;
				clientList.push_back(pSvr);
			}
		}
	}
	
	//��ʼ������ÿ�μ���1000��sock,select1��������1024��
	clientCount = clientList.size();
	SOCKET svrSock;
	for ( endPos = 0; endPos < clientCount; )
	{
		maxSocket = 0;
		FD_ZERO(&readfds);     
		FD_ZERO(&sendfds);  
		startPos = endPos;//��¼���μ���sock��ʼλ��
		for ( i = 0; i < FD_SETSIZE - 1 && endPos < clientCount; i++ )
		{
			pSvr = clientList[endPos];
			if ( maxSocket < pSvr->sock ) maxSocket = pSvr->sock;
			i64ToAddr(ip, port, pSvr->addr);
			FD_SET(pSvr->sock, &readfds); 
			FD_SET(pSvr->sock, &sendfds); 
			endPos++;
		}
		
		//��ʱ����
		timeval outtime;
		outtime.tv_sec = 0;
		outtime.tv_usec = 0;
		int nSelectRet;
		nSelectRet=::select( maxSocket + 1, &readfds, &sendfds, NULL, &outtime ); //����д״̬
		if ( SOCKET_ERROR == nSelectRet ) continue;
		
		int readSize = 0;
		bool successed = true;
		for ( i = startPos; i < endPos; i++ )
		{
			pSvr = clientList[i];
			svrSock = pSvr->sock;
			i64ToAddr(ip, port, pSvr->addr);
			if ( 0 != nSelectRet && !FD_ISSET(svrSock, &readfds) && !FD_ISSET(svrSock, &sendfds) ) continue;

			successed = true;
			memset(&sockAddr, 0, sizeof(sockAddr));
			socklen_t nSockAddrLen = sizeof(sockAddr);
			int gpn = getpeername( svrSock, (sockaddr*)&sockAddr, &nSockAddrLen );
			if ( SOCKET_ERROR == gpn ) successed = false;
			
			if ( !successed )
			{
				pSvr->state = SVR_CONNECT::unconnectting;
				ConnectFailed( pSvr );
			}
			else
			{
				pSvr->state = SVR_CONNECT::connected;
				OnConnect(svrSock, true);
			}
		}
	}
	
	return;
}

#ifndef WIN32
#include <netdb.h>
#endif

bool STNetEngine::AsycConnect( SOCKET svrSock, const char *lpszHostAddress, unsigned short nHostPort )
{
	if ( NULL == lpszHostAddress ) return false;
	//������ת��Ϊ��ʵIP�����lpszHostAddress��������ip����Ӱ��ת�����
	char ip[64]; //��ʵIP
#ifdef WIN32
	PHOSTENT hostinfo;   
#else
	struct hostent * hostinfo;   
#endif
	strcpy( ip, lpszHostAddress ); 
	if((hostinfo = gethostbyname(lpszHostAddress)) != NULL)   
	{
		strcpy( ip, inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list) ); 
	}

	//ʹ����ʵip��������
	sockaddr_in sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(ip);
	sockAddr.sin_port = htons( nHostPort );

	if ( SOCKET_ERROR != connect(svrSock, (sockaddr*)&sockAddr, sizeof(sockAddr)) ) return true;

	return false;
}

void* STNetEngine::ConnectFailed( STNetEngine::SVR_CONNECT *pSvr )
{
	if ( NULL == pSvr ) return NULL;
	char ip[32];
	int port;
	int reConnectSecond;
	i64ToAddr(ip, port, pSvr->addr);
	reConnectSecond = pSvr->reConnectSecond;
	SOCKET svrSock = pSvr->sock;
	pSvr->sock = INVALID_SOCKET;
	pSvr->state = SVR_CONNECT::unconnected;
	closesocket(svrSock);

	m_pNetServer->OnConnectFailed( ip, port, reConnectSecond );

	return NULL;
}

}
// namespace mdk

