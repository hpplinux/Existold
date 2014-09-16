
#include "common.h"
#include "Protocol.h"
#include "mdk/Socket.h"

namespace Device
{

char* Descript( Device::Type::Type type )
{
	if ( Device::Type::motherboard == type ) return "ASUS Z9PE-D8 WS ��ǿ˫оƬ����վ����";
	if ( Device::Type::exist == type ) return "��������(CtreaStar)�����";
	if ( Device::Type::ssd == type ) return "�����裨OCZ�� ��ҵ��SSD RevoDrive 350ϵ�� 960G PCI-E��̬Ӳ��";
	if ( Device::Type::cpu == type ) return "Intel core(TM) i7-4960X 3.6GHZ,15MB LGA2011 CPU";
	if ( Device::Type::screen == type ) return "SAMSUNG LED���ӽ� MD230 23Ӣ��6����������ʾ��";
	if ( Device::Type::touch == type ) return "���� GM84S1 84Ӣ��LED���⽻��ʽ����6�㴥�ش�����";

	return "����";
}

//״̬����
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
	if ( mdk::Socket::seError == len || mdk::Socket::seSocketClose == len ) return 1;//������ʧȥ����
	len = bsp::memtoi( buf, sizeof(short) );
	len += sizeof(short);
	if ( len > MAX_BSTRUCT_SIZE ) return 2;//���巢���źų���
	len = sender.Receive( buf, len );
	if ( mdk::Socket::seError == len || mdk::Socket::seSocketClose == len ) return 1;//������ʧȥ����
	if ( !msg.Resolve( &buf[sizeof(short)], len - sizeof(short) ) ) return 3;//���巢�ͷǷ��ź�
	if ( !msg["MsgId"].IsValid() || sizeof(unsigned short) != msg["MsgId"].m_size ) return 4;//�������MsgId�ֶ�
	
	return 0;
}

}
