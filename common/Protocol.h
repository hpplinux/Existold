// Protocl.h: interface for the Protocl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../Micro-Development-Kit/include/frame/netserver/STNetHost.h"
#include "../bstruct/include/BStruct.h"

#define MSG_HEAD_SIZE		sizeof(unsigned short) //����ͷ����
#define MAX_BSTRUCT_SIZE	1048576 //BStruct�ṹ��󳤶�1M
#define MAX_MSG_SIZE		(MSG_HEAD_SIZE + MAX_BSTRUCT_SIZE)//��Ϣ������󳤶ȣ�����ͷ+BStruct����С


//����id
namespace MsgId
{
	enum MsgId
	{
		unknow = 0,//δ֪����
		heartbeat = 1000,//����
		plugInQuery = 1001,//�豸�������
		setDeviceId = 1002,//�����豸ID
		runDevice = 1003,//�豸����֪ͨ�������豸����Կ�ʼ������
		devicePostion = 1004,//�豸λ��֪ͨ
	};
}

#endif // !defined(PROTOCOL_H)
