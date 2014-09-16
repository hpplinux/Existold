#ifndef COMMOND_H
#define COMMOND_H

#include "../Micro-Development-Kit/include/frame/netserver/STNetServer.h"
#include "BStruct.h"

//�豸
namespace Device
{
	//����
	namespace Type
	{
		enum Type
		{
			motherboard = 0,	//���壺	ASUS Z9PE-D8 WS ��ǿ˫оƬ����վ���壡֧��4·SLI ��CrossfireX����ͼ�μ��������������У����� �� �ۣ���4999.00��
			exist = 1,			//�������	��������(CtreaStar)�������ϵ�һ����������ο��ۣ���1.00��
			ssd = 2,			//��̬Ӳ�̣������裨OCZ�� ��ҵ��SSD RevoDrive 350ϵ�� 960G PCI-E��̬Ӳ�̣��� �� �ۣ���9599.00��
			cpu = 3,			//CPU��		Intel core(TM) i7-4960X 3.6GHZ,15MB LGA2011���� �� �ۣ���7699.00��
			screen = 4,			//��ʾ����	SAMSUNG LED���ӽ� MD230 23Ӣ��6���������� �� �ۣ���29998.00��
			touch = 5,			//��������	Goodview�����ӣ� GM84S1 84Ӣ��LED���⽻��ʽ����6�㴥�ش����������� �� �ۣ���59999.00��
		};
	}

	//����
	namespace Status
	{
		enum Status
		{
			unknow = 0,
			unPlugIn = 1,
			idle = 2,
			waitDevice = 3,
			running = 4,
			loadData = 5,
			closing = 6,
		};
	}

	typedef struct INFO
	{
		unsigned char deviceId;
		Type::Type type;
		Status::Status status;//״̬
		mdk::STNetHost host;//����
		std::string wanIP;//����ip
		std::string lanIP;//����ip
		unsigned int wanPort;//��������˿�
		unsigned int lanPort;//��������˿�
	}INFO;

	char* Descript( Device::Type::Type type );
	char* Descript( Device::Status::Status status );
}

namespace Exist
{

void SendBStruct( mdk::Socket &sock, bsp::BStruct &msg );
int Recv( mdk::Socket &sender, bsp::BStruct &msg, unsigned char *buf );

}
#endif //COMMOND_H
