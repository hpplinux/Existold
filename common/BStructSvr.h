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
		outBufSize = 0,		//报文超出最大长度
		resolveUnable = 1,	//无法解析成BStruct
		noMsgId = 2,		//缺少参数MsgId
		paramError = 3,		//参数错误：缺少参数/参数类型不正确
	};

public:
	BStructSvr();
	virtual ~BStructSvr();

	virtual void OnMsg(mdk::STNetHost &host);//接收BStruct格式报文，成功触发OnWork()，失败触发OnInvalidMsg()
	virtual void OnWork( mdk::STNetHost &host, bsp::BStruct &msg ) = 0;//报文响应
	virtual void OnInvalidMsg(mdk::STNetHost &host, ErrorType type, unsigned char *msg, unsigned short len);//非法报文
	
	bsp::BStruct& GetStruct();//取得初始化好的BStruct成员，为了构造发送报文，每次调用本方法BStruct之前填入的数据都将被清空
	void SendBStruct( mdk::STNetHost &host );//将构造好的BStruct对象发送出去

private:
	unsigned char *m_iBuffer;//接收BStruct对象缓冲
	unsigned char *m_oBuffer;//发送BStruct对象缓冲
	bsp::BStruct m_struct;//用于发送的BStruct对象
};

#endif // #ifndef BSTRUCTSVR_H
