// BStructSvr.h: interface for the BStructSvr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef BSTRUCTSVR_H
#define BSTRUCTSVR_H

#include "../Micro-Development-Kit/include/frame/netserver/STNetServer.h"
#include "../Micro-Development-Kit/include/mdk/mapi.h"
#include "../bstruct/include/BStruct.h"

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

	virtual void OnMsg(mdk::STNetHost &host);//����BStruct��ʽ���ģ��ɹ�����OnWork()��ʧ�ܴ���OnInvalidMsg()
	virtual void OnWork( mdk::STNetHost &host, bsp::BStruct &msg ) = 0;//������Ӧ
	virtual void OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len);//�Ƿ�����
	
	bsp::BStruct& GetStruct();//ȡ�ó�ʼ���õ�BStruct��Ա��Ϊ�˹��췢�ͱ��ģ�ÿ�ε��ñ�����BStruct֮ǰ��������ݶ��������
	void SendBStruct( mdk::STNetHost &host );//������õ�BStruct�����ͳ�ȥ

private:
	unsigned char *m_iBuffer;//����BStruct���󻺳�
	unsigned char *m_oBuffer;//����BStruct���󻺳�
	bsp::BStruct m_struct;//���ڷ��͵�BStruct����
};

#endif // #ifndef BSTRUCTSVR_H
