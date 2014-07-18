// TCPWorker.h: interface for the TCPWorker class.
//
//////////////////////////////////////////////////////////////////////

#ifndef TCPWORKER_H
#define TCPWORKER_H

#include "../Micro-Development-Kit/include/frame/netserver/STNetServer.h"
#include "../Micro-Development-Kit/include/mdk/ConfigFile.h"
#include "../Micro-Development-Kit/include/mdk/Signal.h"
#include "../Micro-Development-Kit/include/mdk/Executor.h"
#include "../bstruct/include/BStruct.h"
#include "../common/common.h"

#include <vector>
#include <string>
#include <map>
using namespace std;

#define COMMAND_TITLE "mgr:\\>"

class Console;
/*
	tcp������
	����tcp��Ϣ��ִ��ҵ��
 */
class TCPWorker : public mdk::STNetServer
{
	typedef struct Exist
	{
		int hostid;//guide��ע�������id
		std::string wanIP;//����ip
		unsigned short wanPort;//�����˿�
		std::string lanIP;//����ip
		unsigned short lanPort;//�����˿�
		NodeRole::NodeRole role;//��ɫ
		NodeStatus::NodeStatus status;//״̬
		unsigned int pieceNo;//Ƭ�ţ��ý�㸺�𱣴��hashid��Ƭ�ϵ�����
	}Exist;
public:
	TCPWorker(char *ip, unsigned short port, Console *pConsole);
	virtual ~TCPWorker();

	bool WaitConnect( unsigned long timeout );
	char* RemoteCall OnCommand(vector<string> *cmd);
	
private:
	void OnConnect(mdk::STNetHost &host);
	void OnMsg(mdk::STNetHost &host);
	void OnCloseConnect(mdk::STNetHost &host);
	void OnReply(mdk::STNetHost &host, bsp::BStruct &msg);
	void OnSystemStatus(mdk::STNetHost &host, bsp::BStruct &msg);
	//��ʾ�ֲ�ʽ������Ϣ
	bool ShowNet(vector<string> &argv);
	//���з�Ƭ
	bool StartPiece(vector<string> &argv);
	//ɾ�����
	bool DelNode(vector<string> &argv);
	
private:
	unsigned char *m_msgBuffer;//��Ϣ����
	mdk::Signal m_sigConnected;
	mdk::STNetHost *m_guide;
	string m_guideIP;
	unsigned short m_guidePort;
	Console *m_pConsole;
	mdk::Signal m_reply;
};	

#endif // !defined TCPWORKER_H
