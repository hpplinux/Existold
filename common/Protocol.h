// Protocl.h: interface for the Protocl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../Micro-Development-Kit/include/frame/netserver/STNetHost.h"
#include "../bstruct/include/BStruct.h"

#define MSG_HEAD_SIZE		sizeof(unsigned short) //报文头长度
#define MAX_BSTRUCT_SIZE	1048576 //BStruct结构最大长度1M
#define MAX_MSG_SIZE		(MSG_HEAD_SIZE + MAX_BSTRUCT_SIZE)//消息缓冲最大长度，报文头+BStruct最大大小


//报文id
namespace MsgId
{
	enum MsgId
	{
		unknow = 0,//未知报文
		heartbeat = 1000,//心跳
		plugInQuery = 1001,//设备请求插入
		setDeviceId = 1002,//设置设备ID
		runDevice = 1003,//设备运行通知，告诉设备你可以开始工作了
		devicePostion = 1004,//设备位置通知
	};
}

#endif // !defined(PROTOCOL_H)
