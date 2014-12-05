// test.cpp : Defines the entry point for the console application.
//

#include <string>
#include "../include/Exist/Exist.h"

#include <stdio.h>
#ifdef WIN32
#ifdef _DEBUG
#pragma comment ( lib, "../lib/cpu_d.lib" )
#pragma comment ( lib, "mdk_d.lib" )
#else
#pragma comment ( lib, "../lib/cpu.lib" )
#pragma comment ( lib, "mdk.lib" )
#endif
#endif

#ifdef WIN32
#include "windows.h"
#else
mdk::uint64 GetTickCount();
#endif

#include "mdk/Socket.h"
#include <sstream>

#include <algorithm>
#include <iostream>
#include <ctime>

#pragma pack(1)
typedef struct MSG_HEADER
{
	unsigned short		msgId;				//����ID
	unsigned short		msgSize;			//���ĳ���
}MSG_HEADER;
#pragma pack()

int main(int argc, char* argv[])
{
	Exist::PlugIn();
	{//����10���д��+30��β�ѯ

		int i = 0;
		char key[256];
		float value;
		mdk::uint64 clt = GetTickCount();
		mdk::uint64 opt = GetTickCount();
		mdk::uint64 start = GetTickCount();
		unsigned char pData[256];
		int size;
		char pValue[256];
		for ( i = 0; i < 100000; i++ )
		{
			sprintf( key, "%d", i );
			sprintf( pValue, "%ie", i );
			SSD::Stream data(key);//��1��������
			size = 256;
			data.GetData(pData, size);//��2��������
			if ( size != strlen(pValue) || 0 != memcmp( pData, pValue, size ) )
			{
				/*
					�ϴ�д������ݣ�������Ӧ����ͬ����һ�����в��Գ���ʱ��
					��Ϊ������δ����Exist,���Բ�ƥ��������
				*/
				printf( "��ʼ��ֵ����ǰд��ֵ��=\n" );
			}
			sprintf( pValue, "%ie", i );
			data.SetData( pValue, strlen(pValue) );//��1��д����
			size = 256;
			data.GetData(pData, size);//��3��������
			if ( 0 != memcmp( pData, pValue, size ) )
			{
				printf( "������д�벻=\n" );
			}
		}
		mdk::uint64 useTime = GetTickCount() - start;
		printf( "10���д��+30��β�ѯ��ʱ��%llu���룬ƽ��%f������/��\n", useTime, (100000+300000)*1000/useTime );
	}

	{//����50���д��+70��β�ѯ
		int i = 0;
		char key[256];
		float value;
		mdk::uint64 start = GetTickCount();
		for ( i = 0; i < 100000; i++ )
		{
			sprintf( key, "%d", i );
			SSD::Float data(key);//��1��������
			float checkData = data;//��2��������
			value = i + 0.5;
			value += 2.5;
			value *= 2.5;
			value -= 2.5;
			value /= 2.5;
			if ( value != checkData )
			{
				/*
					�ϴ�д������ݣ�������Ӧ����ͬ����һ�����в��Գ���ʱ��
					��Ϊ������δ����Exist,���Բ�ƥ��������
				*/
				printf( "��ʼ��ֵ����ǰд��ֵ��=\n" );
			}
			data = i + 0.5;//��1��д����
			value = i + 0.5;
			checkData = data;//��3��������
			if ( value != checkData )
			{
				printf( "������д�벻=\n" );
			}
			data += 2.5;//��2��д����
 			value += 2.5;
			checkData = data;//��4��������
			if ( value != checkData )
			{
				printf( "������+���㲻=\n" );
			}
			data *= 2.5;//��3��д����
			value *= 2.5;
			checkData = data;//��5��������
			if ( value != checkData )
			{
				printf( "������*���㲻=\n" );
			}
			data -= 2.5;//��4��д����
			value -= 2.5;
			checkData = data;//��6��������
			if ( value != checkData )
			{
				printf( "������-���㲻=\n" );
			}
			data /= 2.5;//��5��д����
			value /= 2.5;
			checkData = data;//��7��������
		}
		mdk::uint64 useTime = GetTickCount() - start;
		printf( "50���д��+70��β�ѯ��ʱ��%llu���룬ƽ��%f������/��\n", useTime, (700000+500000)*1000/useTime );
	}

	printf( "���Գ�����ɣ��˳�����\n" );
	return 0;
}

