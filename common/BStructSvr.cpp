// IReceiver.cpp: implementation of the IReceiver class.
//
//////////////////////////////////////////////////////////////////////

#include "BStructSvr.h"
#include "Protocol.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BStructSvr::BStructSvr()
{
	m_iBuffer = new unsigned char[MAX_MSG_SIZE];
	mdk::mdk_assert( NULL != m_iBuffer );
	m_oBuffer = new unsigned char[MAX_MSG_SIZE];
	mdk::mdk_assert( NULL != m_oBuffer );
}

BStructSvr::~BStructSvr()
{
	if ( NULL != m_iBuffer )
	{
		delete[]m_iBuffer;
		m_iBuffer = NULL;
	}
	if ( NULL != m_oBuffer )
	{
		delete[]m_oBuffer;
		m_oBuffer = NULL;
	}
}

void BStructSvr::OnMsg(mdk::STNetHost &host)
{
	//////////////////////////////////////////////////////////////////////////
	//接收BStruct协议格式的报文
	bsp::BStruct msg;
	if ( !host.Recv(m_iBuffer, MSG_HEAD_SIZE, false) ) return;
	unsigned int len = bsp::memtoi(&m_iBuffer[0], sizeof(unsigned short));
	if ( len > MAX_BSTRUCT_SIZE ) 
	{
		host.Recv( m_iBuffer, MSG_HEAD_SIZE );
		OnInvalidMsg(host, outBufSize, m_iBuffer, MSG_HEAD_SIZE);
		return;
	}
	if ( !host.Recv(m_iBuffer, len + MSG_HEAD_SIZE) ) return;
	if ( !msg.Resolve(&m_iBuffer[MSG_HEAD_SIZE], len) ) //解析报文
	{
		OnInvalidMsg(host, resolveUnable, m_iBuffer, len + MSG_HEAD_SIZE);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	//处理报文
	if ( !msg["MsgId"].IsValid() || sizeof(unsigned short) != msg["MsgId"].m_size ) //检查有无msgid字段
	{
		OnInvalidMsg(host, noMsgId, m_iBuffer, len + MSG_HEAD_SIZE);
		return;
	}

	OnWork( host, msg );
}

void BStructSvr::OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len)
{
	host.Close();
}

bsp::BStruct& BStructSvr::GetStruct()
{
	m_struct.Bind( &m_oBuffer[MSG_HEAD_SIZE], MAX_BSTRUCT_SIZE );
	return m_struct;
}

void BStructSvr::SendBStruct( mdk::STNetHost &host )
{
	bsp::itomem( m_oBuffer, m_struct.GetSize(), MSG_HEAD_SIZE );
	host.Send( m_oBuffer, MSG_HEAD_SIZE + m_struct.GetSize() );
}
