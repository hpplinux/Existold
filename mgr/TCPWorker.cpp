// TCPWorker.cpp: implementation of the TCPWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "TCPWorker.h"
#include "../common/Protocol.h"
#include "../bstruct/include/BArray.h"
#include "../common/Console.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
TCPWorker::TCPWorker(char *ip, unsigned short port, Console *pConsole)
{
	m_guide = NULL;
	m_pConsole = pConsole;
	m_msgBuffer = new unsigned char[MAX_MSG_SIZE];
	if ( NULL == m_msgBuffer ) return;
	SetAverageConnectCount(10);
	m_guideIP = ip;
	m_guidePort = port;
	Connect( m_guideIP.c_str(), m_guidePort, 5 );
}

TCPWorker::~TCPWorker()
{
	if ( NULL != m_msgBuffer )
	{
		delete[]m_msgBuffer;
		m_msgBuffer = NULL;
	}
}

bool TCPWorker::WaitConnect( unsigned long timeout )
{
	return m_sigConnected.Wait( timeout );
}

void TCPWorker::OnConnect(mdk::STNetHost &host)
{
	if ( host.IsServer() ) 
	{
		m_guide = &host;
		m_sigConnected.Notify();
		m_pConsole->Print( "\nconnected with guide" );
	}
	return;
}

void TCPWorker::OnCloseConnect(mdk::STNetHost &host)
{
	if ( m_guide->ID() == host.ID() )
	{
		m_guide = NULL;
		m_pConsole->Print( "\ndisconnected with guide, reconnectting..." );
		Connect( m_guideIP.c_str(), m_guidePort );
		m_reply.Notify();
	}
	return;
}

char* TCPWorker::OnCommand(vector<string> *cmd)
{
	if ( NULL == m_guide ) return "unconnected with guide";
	vector<string> &argv = *cmd;
	bool bLegal = false;
	if ( "shownet" == argv[0] ) bLegal = ShowNet(argv);
	else if ( "piece" == argv[0] ) bLegal = StartPiece(argv);
	else if ( "delnode" == argv[0] ) bLegal = DelNode(argv);
	else return "Invalid command";
	if ( !bLegal ) return "Error param count";

	m_reply.Wait();
	
	return NULL;
}

/**
 * ���ݵ���ص�����
 * 
 * ������ʵ�־���Ͽ�����ҵ����
 * 
*/
void TCPWorker::OnMsg(mdk::STNetHost &host)
{
	//////////////////////////////////////////////////////////////////////////
	//����BStructЭ���ʽ�ı���
	bsp::BStruct msg;
	if ( !host.Recv(m_msgBuffer, MSG_HEAD_SIZE, false) ) return;
	unsigned int len = bsp::memtoi(&m_msgBuffer[1], sizeof(unsigned int));
	if ( len > MAX_BSTRUCT_SIZE ) 
	{
		host.Recv( m_msgBuffer, MSG_HEAD_SIZE );
		return;
	}
	if ( !host.Recv(m_msgBuffer, len + MSG_HEAD_SIZE) ) return;
	DataFormat::DataFormat df = (DataFormat::DataFormat)m_msgBuffer[0];
	if ( DataFormat::BStruct != df ) return;//ֻ����BStruct��ʽ�ı���
	if ( !msg.Resolve(&m_msgBuffer[MSG_HEAD_SIZE],len) ) return;//��������
	
	//////////////////////////////////////////////////////////////////////////
	//������
	if ( !msg["msgid"].IsValid() || sizeof(unsigned short) != msg["msgid"].m_size ) return;
		
	unsigned short msgid = msg["msgid"];
	bool bIsInvalidMsg = true;//Ĭ��Ϊ�Ƿ�����
	switch( msgid )
	{
	case msgid::gSystemStatus :
		OnSystemStatus(host, msg);
		break;
	case msgid::gCommandResult ://ָ��ִ�н��
		OnReply(host, msg);
		break;
	default:
		break;
	}
}

bool TCPWorker::ShowNet(vector<string> &argv)
{
	if ( 1 != argv.size() ) return false;
	bsp::BStruct msg;
	msg.Bind( &m_msgBuffer[MSG_HEAD_SIZE], MAX_BSTRUCT_SIZE );
	msg["msgid"] = (unsigned short)msgid::uCommand;
	msg["cmdCode"] = (unsigned char)UserCmd::QueryNodeList;
	SendBStruct(*m_guide, msg);
	return true;
}

//���з�Ƭ
bool TCPWorker::StartPiece(vector<string> &argv)
{
	if ( 2 < argv.size() ) return false;
	unsigned int pieceNo;
	if ( 1 == argv.size() ) pieceNo = -1;
	else pieceNo = atoi(argv[1].c_str());

	bsp::BStruct msg;
	msg.Bind( &m_msgBuffer[MSG_HEAD_SIZE], MAX_BSTRUCT_SIZE );
	msg["msgid"] = (unsigned short)msgid::uCommand;
	msg["cmdCode"] = (unsigned char)UserCmd::StartPiece;
	msg["pieceNo"] = pieceNo;
	SendBStruct(*m_guide, msg);
	return true;	
}

//ɾ�����
bool TCPWorker::DelNode(vector<string> &argv)
{
	if ( 2 != argv.size() ) return false;
	int hostID = atoi(argv[1].c_str());
	bsp::BStruct msg;
	msg.Bind( &m_msgBuffer[MSG_HEAD_SIZE], MAX_BSTRUCT_SIZE );
	msg["msgid"] = (unsigned short)msgid::uCommand;
	msg["cmdCode"] = (unsigned char)UserCmd::NodeDel;
	msg["hostID"] = hostID;
	SendBStruct(*m_guide, msg);
	return true;
}

//��ʾ�ֲ�ʽ������Ϣ
void TCPWorker::OnSystemStatus(mdk::STNetHost &host, bsp::BStruct &msg)
{
	bsp::BStruct db = msg["database"];
	bsp::BStruct master = msg["master"];
	bsp::BArray pieceList = msg["pieceList"];
	if ( !db.IsValid() || !master.IsValid() || !pieceList.IsValid() ) return;
	m_pConsole->Print( "nodes in system, the following" );
	m_pConsole->Line();
	int hostid;//����
	string wanIP;//����ip
	string lanIP;//����ip
	unsigned short port;//�˿�
	unsigned char status;//״̬
	unsigned int pieceNo;//Ƭ�ţ��ý�㸺�𱣴��hashid��Ƭ�ϵ�����
	char nodeInfo[256];
	hostid = db["hostID"];
	status =  db["status"];
	wanIP = (string)db["wan_ip"];
	lanIP = (string)db["lan_ip"];
	port = db["port"];
	if ( NodeStatus::NotOnline != status )
	{
		sprintf( nodeInfo, "hostid=%d\nrole=database, status=%s, wanIP=%s, lanIP=%s, port=%d ", 
			hostid, DesStatus(status), wanIP.c_str(), lanIP.c_str(), port );
		m_pConsole->Print( nodeInfo );
		m_pConsole->Line();
	}

	hostid = master["hostID"];
	status =  master["status"];
	wanIP = (string)master["wan_ip"];
	lanIP = (string)master["lan_ip"];
	port = master["port"];
	if ( NodeStatus::NotOnline != status ) 
	{
		sprintf( nodeInfo, "hostid=%d\nrole=master, status=%s, wanIP=%s, lanIP=%s, port=%d ", 
			hostid, DesStatus(status), wanIP.c_str(), lanIP.c_str(), port );
		m_pConsole->Print( nodeInfo );
		m_pConsole->Line();
	}
	int i = 0;
	bsp::BStruct node; 
	for ( i = 0; pieceList[i].IsValid(); i++ )
	{
		node = pieceList[i];
		hostid = node["hostID"];
		status =  node["status"];
		wanIP = (string)node["wan_ip"];
		lanIP = (string)node["lan_ip"];
		port = node["port"];
		pieceNo = node["pieceNo"];
		sprintf( nodeInfo, "hostid=%d\nrole=piece, status=%s, pieceNo=%d, wanIP=%s, lanIP=%s, port=%d", 
			hostid, DesStatus(status), pieceNo, wanIP.c_str(), lanIP.c_str(), port );
		m_pConsole->Print( nodeInfo );
		m_pConsole->Line();
	}
	m_reply.Notify();
	return;
}

//ָ���Ӧ
void TCPWorker::OnReply(mdk::STNetHost &host, bsp::BStruct &msg)
{
	//////////////////////////////////////////////////////////////////////////
	/*
		��鱨�ĺϷ���
		1.���е��ֶ���û��
		2.�����Ƿ���Э��Լ�����
		3.�ַ����ȱ䳤�ֶγ��ȱ��벻С��1
	*/
	if ( !msg["success"].IsValid() || sizeof(unsigned char) != msg["success"].m_size ) return;
	unsigned char success = msg["success"];
	if ( !success )
	{
		if ( !msg["reason"].IsValid() || 1 > msg["reason"].m_size ) return;
		string reason = msg["reason"];
		m_pConsole->Print(reason.c_str());
		m_reply.Notify();
		return;
	}
	m_pConsole->Print("success");
	m_reply.Notify();
	return;
}
