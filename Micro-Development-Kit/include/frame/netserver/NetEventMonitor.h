// NetEventMonitor.h: interface for the NetEventMonitor class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MDK_NETEVENTMONITOR_H
#define MDK_NETEVENTMONITOR_H

#include "../../../include/mdk/Socket.h"
#include "../../../include/mdk/FixLengthInt.h"
#include <string>
#define MAXPOLLSIZE 20000 //���socket��

namespace mdk
{

typedef struct IOCP_DATA
{
	int64 connectId;
	char *buf;
	unsigned short bufSize;
}IOCP_DATA;

class NetEventMonitor
{
public:
	NetEventMonitor();
	virtual ~NetEventMonitor();

public:
	//��ʼ����
	virtual bool Start( int nMaxMonitor ) = 0;
	//ֹͣ����
	virtual bool Stop();
	//����һ���������󵽼����б�
	virtual bool AddMonitor( SOCKET socket, char* pData, unsigned short dataSize );
	virtual bool AddConnectMonitor( SOCKET sock );
	virtual bool AddDataMonitor( SOCKET sock, char* pData, unsigned short dataSize );
	virtual bool AddSendableMonitor( SOCKET sock, char* pData, unsigned short dataSize );
	//�ȴ��¼�����
	virtual bool WaitEvent( void *eventArray, int &count, bool block );

	//����һ���������ӵĲ����������ӽ�����WaitEvent�᷵��
	virtual bool AddAccept(SOCKET socket);
	//����һ���������ݵĲ����������ݵ��WaitEvent�᷵��
	virtual bool AddRecv( SOCKET socket, char* pData, unsigned short dataSize );
	//����һ���������ݵĲ�����������ɣ�WaitEvent�᷵��
	virtual bool AddSend( SOCKET socket, char* pData, unsigned short dataSize );

	//ɾ��һ����������Ӽ����б�
	virtual bool DelMonitor( SOCKET socket );

	//ȡ�����Ĵ���
	const char* GetInitError();

protected:
	std::string m_initError;//������Ϣ
};

}//namespace mdk

#endif // MDK_NETEVENTMONITOR_H
