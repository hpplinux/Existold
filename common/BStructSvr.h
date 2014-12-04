// BStructSvr.h: interface for the BStructSvr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef BSTRUCTSVR_H
#define BSTRUCTSVR_H

#include "frame/netserver/STNetServer.h"
#include "mdk/mapi.h"
#include "Protocol.h"

class BStructSvr : public mdk::STNetServer
{
protected:
	enum ErrorType
	{
		outBufSize = 0,		//���ĳ�����󳤶�
		resolveUnable = 1,	//�޷�������BStruct
		noMsgId = 2,		//ȱ�ٲ���MsgId
		paramError = 3,		//��������ȱ�ٲ���/�������Ͳ���ȷ
	};

public:
	BStructSvr();
	virtual ~BStructSvr();

	virtual void OnMsg(mdk::STNetHost &host);//���ձ��ģ��ɹ�����OnWork()��ʧ�ܴ���OnInvalidMsg()
	virtual void OnWork( mdk::STNetHost &host, MSG_HEADER *header, unsigned char *pData ) = 0;//������Ӧ
	virtual void OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len);//�Ƿ�����
	unsigned char* GetDataBuffer();//ȡ�÷��ͱ����������յ�ַ
	void SendMsg( mdk::STNetHost &host, MsgId::MsgId msgId, short size );//������õı��ķ��ͳ�ȥ

private:
	unsigned char *m_iBuffer;//���ձ��Ļ���
	unsigned char *m_oBuffer;//���ͱ��Ļ���
};

#endif // #ifndef BSTRUCTSVR_H
