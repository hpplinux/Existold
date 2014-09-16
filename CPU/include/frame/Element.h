// Element.h: interface for the Element class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ELEMENT_H
#define ELEMENT_H

#include "KVData.h"

namespace Exist
{
/*
	Ԫ����
		����������������ֵ�������Լ���Ԫ�أ���ֵ��ֻ��1��Ԫ�ص�����
		���粻����ʱ����ȷʱ����ȥ��������IO����
		1.����׼ȷ���жϣ��û���Ҫ������д
			����KVData����������˵����ȡ��һ���ӿڶ���ʱ���޷��ж��û���Ҫ������ӿ���������д
			������Ҫ������
		2.Ҳ�������ڶ�һ��Ԫ�ط���IO
			����һ��KVData,���ȡ���󱣴���c/c++����/�Զ��������У����޷�ʹ�������������������
			������Ҫ����������
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
			m_hash = IOBus::Hash(m_key, m_keySize);//����hash
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
		//�����������ͣ�ֱ�ӵ�Exist��ѯ����ֵ
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

		//�����������͵�Ԫ�أ�������Ҫȡ���Լ���Exist�ϵ�key
		if ( 1 == m_indexType ) //����ӵ��Key��Ԫ�أ������key����Ԫ����Exist�ϵ���ʵkey������ֱ��copy����
		{
			((IOBus *)&m_data)->Init(m_key, m_keySize);
		}
		else//��Ҫ��Exist�ϲ�ѯ�Լ���key
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
