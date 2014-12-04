// Stream.h: interface for the Stream class.
//
//////////////////////////////////////////////////////////////////////

#ifndef STREAM_H
#define STREAM_H

#include "./frame/KVData.h"
#include "./frame/Element.h"
#include <string>

namespace Exist
{

class Stream : public KVData<unsigned char*, Element<unsigned char*> >
{
protected:
	Stream();

public:
	Stream( const char *name );
	Stream( const Stream &value );
	Stream& operator=( const Stream &right );
	virtual ~Stream();

public:
	/*
		����new�����pData���󣬱������ݵ�pData��
		���򷵻�false,size������Ҫ�ĳ���
	*/
	bool GetData(unsigned char *pData, int &size);
	unsigned char* GetData(int &size);//��new���ʴ�С���ڴ浽���أ�����ǵ��ͷ�
	bool SetData(const std::string data, int size);
};

}

namespace SSD
{

class Stream : public Exist::KVData<unsigned char*, Exist::Element<unsigned char*> >
{
protected:
	Stream();

public:
	Stream( const char *name );
	Stream( const Stream &value );
	Stream& operator=( const Stream &right );
	virtual ~Stream();

public:
	/*
		����new�����pData���󣬱������ݵ�pData��
		���򷵻�false,size������Ҫ�ĳ���
	*/
	bool GetData(unsigned char *pData, int &size);
	unsigned char* GetData(int &size);//��new���ʴ�С���ڴ浽���أ�����ǵ��ͷ�
	bool SetData(const std::string data, int size);
};

}
#endif // ifndef INT8_H
