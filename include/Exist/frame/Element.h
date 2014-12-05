// Element.h: interface for the Element class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ELEMENT_H
#define ELEMENT_H

#include "KVData.h"

namespace Exist
{

namespace SearchType
{
	enum SearchType
	{
		stIndex = 0,
		stKey,
	};

}
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
		m_searchType = SearchType::stKey;
		m_index = NULL;
		m_keySize = 0;
		m_key[m_keySize] = 0;
	}
	
	Element( KVData<T, Element<T> > *owner, int indexType, const void *index, int size )
		:m_data((*(T*)IOBus::s_defaultObj))
	{
		Init( owner, indexType, index, size );
	}

	Element( const Element &value )
		:m_data((*(T*)IOBus::s_defaultObj))
	{
		Copy( value );
	}

	void Init( KVData<T, Element<T> > *owner, int searchType, const void *index, int size )
	{
		m_owner = owner;
/*
		m_searchType = (SearchType::SearchType)searchType;
		m_keySize = size;
		if ( SearchType::stKey == m_searchType )
		{
			if ( m_keySize > MAX_KEY_SIZE ) m_keySize = MAX_KEY_SIZE;
			memcpy(m_key, index, m_keySize);
			m_key[m_keySize] = 0;
			m_hash = IOBus::Hash(m_key, m_keySize);//����hash
		}
		else 
		{
			memcpy(&m_index, index, m_keySize);
		}
*/
	}

	void Copy( const Element &value )
	{
		m_owner = value.m_owner;
		m_searchType = value.m_searchType;
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

	T& operator=(const T &right)
	{
		/*
			�ݲ�֧�ֽ�һ����������һ��������Ԫ��
			Exist::Map<int, Exist<int, int> > a("Ƕ��map");
			Exist<int, int> b("map");
			a[1] = b;//��֧�������÷�
		*/
		mdk::mdk_assert( !DataType::IsContainer(m_owner->Type()) );

		m_data = right;
		if ( SearchType::stKey == m_searchType ) 
		{
			m_owner->WriteData(&m_data, sizeof(T), UpdateType::utCopy);
		}
		else 
		{
			m_owner->WriteData( m_index, &m_data, sizeof(T) );
		}
		return m_data;
	}

	T& operator+=(const T &right)
	{
		m_data = right;
		if ( SearchType::stKey == m_searchType ) 
		{
			m_owner->WriteData(&m_data, sizeof(T),UpdateType::utAddSelf);
		}
		else 
		{
			m_owner->WriteData( m_index, &m_data, sizeof(T) );
		}
		return m_data;
	}

	T& operator-=(const T &right)
	{
		m_data = right;
		if ( SearchType::stKey == m_searchType ) 
		{
			m_owner->WriteData(&m_data, sizeof(T), UpdateType::utSubtractSelf);
		}
		else 
		{
			m_owner->WriteData( m_index, &m_data, sizeof(T) );
		}
		return m_data;
	}

	T& operator*=(const T &right)
	{
		m_data = right;
		if ( SearchType::stKey == m_searchType ) 
		{
			m_owner->WriteData(&m_data, sizeof(T), UpdateType::utMultiplySelf);
		}
		else 
		{
			m_owner->WriteData( m_index, &m_data, sizeof(T) );
		}
		return m_data;
	}

	T& operator/=(const T &right)
	{
		//VC++6.0��֧��int ��0.0�Ƚϣ��ʶ��ȴ���T���Ͷ����ʼ��Ϊ0.0��Ȼ��T T�Ƚ�
		T zore = 0.0;
		mdk::mdk_assert( zore != right ); 

		m_data = right;
		if ( SearchType::stKey == m_searchType ) 
		{
			m_owner->WriteData(&m_data, sizeof(T), UpdateType::utDivideSelf);
		}
		else 
		{
			m_owner->WriteData( m_index, &m_data, sizeof(T) );
		}
		return m_data;
	}
	
	operator T()
	{
		//��ֵ����Ԫ�أ�ֱ�ӵ�Exist��ѯ����ֵ
		if ( DataType::IsValue(m_owner->Type()) )
		{
			int size = sizeof(T);
			if ( m_owner->QueryData() )
			{
				m_owner->ReadData(&m_data, size);
			}

			return m_data;
		}

/*
		//��������Ԫ�أ����ڴ�
		//�����������͵�Ԫ�أ�������Ҫ��ȡ���Լ���Exist�ϵ�key
		if ( SearchType::stKey == m_searchType ) //����Key������ʽ��Ԫ��key����Ԫ����Exist�ϵ���ʵkey������ֱ��copy����
		{
			((IOBus *)&m_data)->Init(m_key, m_keySize, m_owner->IsProtracted());
		}
		else//�����±������ʽ����Ҫ��Exist�ϲ�ѯ�Լ���key
		{
			char key[MAX_KEY_SIZE+1];
			int size = MAX_KEY_SIZE;
//			m_owner->ReadData(m_index, key, size);
			((IOBus *)&m_data)->Init(key, size, m_owner->IsProtracted());
		}
*/

		return m_data;
	}

protected:
	KVData<T, Element<T> >				*m_owner;
	SearchType::SearchType				m_searchType;	//������Ԫ�ؼ�����ʽ0���±��������0��key����
	int									m_index;
	int									m_hash;
	char								m_key[MAX_KEY_SIZE+1];
	int									m_keySize;
	T									m_data;
};

}

#endif // ifndef ELEMENT_H
