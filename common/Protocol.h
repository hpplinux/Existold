// Protocl.h: interface for the Protocl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../include/Exist/frame/ExistType.h"

#pragma pack(1)
typedef struct MSG_HEADER
{
	unsigned short		msgId;				//����ID
	unsigned short		msgSize;			//���ĳ���
}MSG_HEADER;
#pragma pack()

#define MSG_HEAD_SIZE		sizeof(MSG_HEADER) //����ͷ����
#define MAX_VALUE_SIZE		5242880 //nosql��¼������󳤶�5M
#define MAX_PARAM_SIZE		4096//���Ĳ�����󳤶�
#define MAX_DATA_SIZE		(MAX_PARAM_SIZE + MAX_VALUE_SIZE) //����������󳤶�
#define MAX_MSG_SIZE		(MSG_HEAD_SIZE + MAX_DATA_SIZE)//��Ϣ������󳤶ȣ�����ͷ+�䳤�����ṹ+value������󳤶�
#define	MAX_DEVICE_COUNT	500	//�����ֲ�ʽ�����������ɵ��豸����

//����id
namespace MsgId
{
	enum MsgId
	{
		unknow = 0,//δ֪����
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//�ֲ�ʽ����
		heartbeat = 1000,//����
		plugInQuery = 1001,//�豸�������
		setDeviceId = 1002,//�����豸ID
		runDevice = 1003,//�豸����֪ͨ�������豸����Կ�ʼ������
		devicePostion = 1004,//�豸λ��֪ͨ

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//ҵ����
		createData = 3001,//��������/���ݵ�����
		deleteData = 3002,//�ͷ�����/���ݵ�����
		readData	= 3003,//������
		writeData	= 3004,//д����
	};
}

#pragma pack(1)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//�ֲ�ʽ����
//����
typedef struct MSG_HEART_BEAT
{
}MSG_HEART_BEAT;

//�豸�������
typedef struct MSG_PLUG_IN_QUERY
{
	short				deviceId;				//�豸ID(ѡ��)
	unsigned char		device;					//�豸����,�ο�DeviceType
	char				wanIP[16];				//����IP (Device ��= cpuʱ������)
	int					wanPort;				//�����˿� (Device ��= cpuʱ������)
	char				lanIP[16];				//����IP (Device ��= cpuʱ������)
	int					lanPort;				//�����˿� (Device ��= cpuʱ������)
}MSG_PLUG_IN_QUERY;

//�����豸ID
typedef struct MSG_SET_DEVICE_ID
{
	unsigned char		deviceId;		//�豸id��ͬ�����豸��id���֡�256������������� + 256��Ӳ�̻��������ã�
}MSG_SET_DEVICE_ID;

//�豸����֪ͨ�������豸����Կ�ʼ������
typedef struct MSG_RUN_DEVICE
{
}MSG_RUN_DEVICE;

//�豸λ��֪ͨ
//�豸���飬����deviceId��С�����ź�
typedef struct MSG_DEVICE_POSTION
{
	short				size;
	MSG_PLUG_IN_QUERY	devices[MAX_DEVICE_COUNT];
}MSG_DEVICE_POSTION;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ҵ����
//��������/���ݵ�����
typedef	struct CREATE_DATA
{
	exist::DATA_KEY	key;			//����key
	unsigned char	size;				//data������Ч������������Ч
	char			protracted;			//�����ǳ־û�����0�ǣ�0��Ԥ���ֶΣ��ݲ�ʵ�ֳ־û����ܣ�
}CREATE_DATA;

//д����
typedef struct WRITE_DATA
{
	exist::DATA_KEY	key;			//����key
	char			updateType;			//�������Ͳο�enum UpdateType
	unsigned int	size;				//���ݴ�С
}WRITE_DATA;

#pragma pack()

#endif // !defined(PROTOCOL_H)
