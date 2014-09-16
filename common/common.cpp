
#include "common.h"
#include "Protocol.h"
#include "mdk/Socket.h"

namespace Device
{

char* Descript( Device::Type::Type type )
{
	if ( Device::Type::motherboard == type ) return "ASUS Z9PE-D8 WS 超强双芯片工作站主板";
	if ( Device::Type::exist == type ) return "创星世纪(CtreaStar)外存条";
	if ( Device::Type::ssd == type ) return "饥饿鲨（OCZ） 企业级SSD RevoDrive 350系列 960G PCI-E固态硬盘";
	if ( Device::Type::cpu == type ) return "Intel core(TM) i7-4960X 3.6GHZ,15MB LGA2011 CPU";
	if ( Device::Type::screen == type ) return "SAMSUNG LED广视角 MD230 23英寸6连屏超大显示屏";
	if ( Device::Type::touch == type ) return "仙视 GM84S1 84英寸LED背光交互式智能6点触控触摸屏";

	return "神器";
}

//状态描述
char* Descript(Device::Status::Status status)
{
	if ( Device::Status::idle == status) return "idle";
	else if ( Device::Status::loadData == status) return "loadData";
	else if ( Device::Status::running == status) return "running";
	else if ( Device::Status::closing == status) return "closing";
	else if ( Device::Status::waitDevice == status) return "waitDevice";
	else if ( Device::Status::unPlugIn == status) return "unPlugIn";
	else if ( Device::Status::unknow == status) return "unknow";
	return "unknow";
}

}

namespace Exist
{

void SendBStruct( mdk::Socket &recver, bsp::BStruct &msg )
{
	unsigned char *buf = msg.GetStream();
	buf -= MSG_HEAD_SIZE;
	bsp::itomem( buf, msg.GetSize(), MSG_HEAD_SIZE );
	recver.Send( buf, MSG_HEAD_SIZE + msg.GetSize() );
}

int Recv( mdk::Socket &sender, bsp::BStruct &msg, unsigned char *buf )
{
	short len = sender.Receive( buf, sizeof(short), true );
	if ( mdk::Socket::seError == len || mdk::Socket::seSocketClose == len ) return 1;//与主板失去链接
	len = bsp::memtoi( buf, sizeof(short) );
	len += sizeof(short);
	if ( len > MAX_BSTRUCT_SIZE ) return 2;//主板发送信号超长
	len = sender.Receive( buf, len );
	if ( mdk::Socket::seError == len || mdk::Socket::seSocketClose == len ) return 1;//与主板失去链接
	if ( !msg.Resolve( &buf[sizeof(short)], len - sizeof(short) ) ) return 3;//主板发送非法信号
	if ( !msg["MsgId"].IsValid() || sizeof(unsigned short) != msg["MsgId"].m_size ) return 4;//检查有无MsgId字段
	
	return 0;
}

}
