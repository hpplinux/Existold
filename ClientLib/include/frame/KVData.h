// KVData.h: interface for the KVData class.
//
//////////////////////////////////////////////////////////////////////

#ifndef KVDATA_H
#define KVDATA_H

#include "IOBus.h"
	
namespace Exist
{
/*
 	key-value������
		���û��ӿ��࣬���û�͸��
		�������ݽӿڵ�ģ��Ļ���
		��������ֵ��������������Exist�϶������Ӧ��һ��key��value����
 */
template<class T, class Element>
class KVData : public IOBus
{
public:
	KVData()
		:IOBus()
	{
	}
	
	KVData( const char *name )
		:IOBus(name)
	{
	}
	
	KVData( const KVData &value )
		:IOBus(value)	
	{
	}
	
	KVData& operator=( const KVData &right )
	{
		Copy(right);
		return *this;
	}

	virtual ~KVData()
	{
	}

};

}

#endif // ifndef KVDATA_H
