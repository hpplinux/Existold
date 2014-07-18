// Value.h: interface for the Value class.
//
//////////////////////////////////////////////////////////////////////

#ifndef VALUE_H
#define VALUE_H

#include "KVData.h"
#include "Element.h"

namespace Exist
{
/*
	��ֵ������
		���û��ӿ��࣬���û�͸��
		����ʵ��Int8, Int16, int32, int64==�����������ͷ��ʽӿ�ʱ�����Ӵ�������
 */
template<class T>
class Value : public KVData<T, Element<T> >
{
protected:
	Value()
		:KVData<T, Element<T> >()
	{
		m_type = DataType::value;
		m_elementType = DataType::data;
	}
	
	Value( const char *name )
		:KVData<T, Element<T> >(name)
	{
		m_type = DataType::value;
		m_elementType = DataType::data;
		m_data.Init(*this, 1, name, strlen(name) );
	}

public:
	Value( const Value &value )
		:KVData<T, Element<T> >(value)
	{
		m_data.Init(*this, 1, Key(), KeySize() );
	}
	
	Value& operator=( const Value &right )
	{
		Copy(right);
		m_data.Init(*this, 1, Key(), KeySize() );
		return *this;
	}

	virtual ~Value()
	{
	}

	operator T()
	{
		return m_data;
	}

	T operator=(const T& right)
	{
		m_data = right;
		return m_data;
	}

protected:
	Element<T> m_data;//��ֵ��ֻ��1��Ԫ�ص�����
};

}

#endif // ifndef VALUE_H
