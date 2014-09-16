// Element.h: interface for the Element class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ELEMENT_H
#define ELEMENT_H

#include "KVData.h"

namespace Exist
{
/*
	元素类
		不管是容器还是数值，都有自己的元素，数值是只有1个元素的容器
		不早不晚，及时且正确时机的去进行网络IO操作
		1.可以准确的判断，用户是要读还是写
			对于KVData及其子类来说，在取得一个接口对象时，无法判断用户是要用这个接口来读还是写
			所以需要此类来
		2.也可以用于对一个元素反复IO
			对于一个KVData,如果取出后保存在c/c++基础/自定义类型中，则无法使用这个对象来操作数据
			所以需要此类来操作
 */
template<class T>
class Element
{
public:
	Element()
	{
		IOBus::CreateExistObj( this, DataType::element );
		m_type = IOBus::GetDataType( &m_data );
		m_indexType = 0;
		m_index = NULL;
		m_keySize = 0;
		m_key[m_keySize] = 0;
	}
	
	Element( KVData<T, Element<T> > &owner, int indexType, const void *index, int size )
		:m_data((*(T*)IOBus::s_defaultObj))
	{
		IOBus::CreateExistObj( this, DataType::element );
		m_type = IOBus::GetDataType( &m_data );
		Init( owner, indexType, index, size );
	}

	Element( const Element &value )
		:m_data((*(T*)IOBus::s_defaultObj))
	{
		IOBus::CreateExistObj( this, DataType::element );
		m_type = IOBus::GetDataType( &m_data );
		Copy( value );
	}

	DataType::DataType Type()
	{
		return m_type;
	}

	void Init( KVData<T, Element<T> > &owner, int indexType, const void *index, int size )
	{
		m_owner = owner;
		m_indexType = indexType;
		m_keySize = size;
		if ( 0 == m_indexType ) memcpy(&m_index, index, m_keySize);
		else 
		{
			if ( m_keySize > MAX_KEY_SIZE ) m_keySize = MAX_KEY_SIZE;
			memcpy(m_key, index, m_keySize);
			m_key[m_keySize] = 0;
			m_hash = IOBus::Hash(m_key, m_keySize);//计算hash
		}
	}

	void Copy( const Element &value )
	{
		m_owner = value.m_owner;
		m_indexType = value.m_indexType;
		m_keySize = value.m_keySize;
		m_index = value.m_index;
		memcpy(m_key, value.m_key, m_keySize);
		m_key[m_keySize] = 0;
		m_data = value.m_data;
	}
	
	virtual ~Element()
	{
		IOBus::ReleaseExistObj( this );
	}
		
	Element& operator=(const Element& right)
	{
		Copy(right);
		return *this;
	}
	
	T& operator=(const T& right)
	{
		m_data = right;
		if ( 0 == m_indexType ) 
		{
			m_owner.WriteData( m_index, &m_data, sizeof(T) );
		}
		else 
		{
			m_owner.WriteData(0, m_key, m_keySize, &m_data, sizeof(T));
		}
		return m_data;
	}
	
	operator T()
	{
		//基础数据类型，直接到Exist查询数据值
		if ( DataType::data == m_owner.ElementType() )
		{
			int size = sizeof(T);
			if ( 0 == m_indexType ) 
			{
				m_owner.ReadData(m_index, &m_data, size);
			}
			else 
			{
				m_owner.ReadData(0, m_key, m_keySize, &m_data, size);
			}

			return m_data;
		}

		//对于容器类型的元素，就是需要取得自己在Exist上的key
		if ( 1 == m_indexType ) //对于拥有Key的元素，这里的key就是元素在Exist上的真实key，所以直接copy即可
		{
			((IOBus *)&m_data)->Init(m_key, m_keySize);
		}
		else//需要到Exist上查询自己的key
		{
			char key[MAX_KEY_SIZE+1];
			int size = MAX_KEY_SIZE;
			m_owner.ReadData(m_index, key, size);
			((IOBus *)&m_data)->Init(key, size);
		}

		return m_data;
	}

protected:
	int									m_type;
	KVData<T, Element<T> >				m_owner;
	int									m_indexType;
	int									m_index;
	int									m_hash;
	char								m_key[MAX_KEY_SIZE+1];
	int									m_keySize;
	T									m_data;
};

}

#endif // ifndef ELEMENT_H
