// Map.h: interface for the Map class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MAP_H
#define MAP_H

#include "frame/Container.h"
#include "frame/Key.h"
namespace Exist
{
/*
 *	��key��intʱ
 *		[]�����������Ϊ�����intֵ���±꣬�ᰴ���±�ȥ��λԪ��
 *		���ϣ����key��λ����������д��[(Exist::Key<int>)�㴫���intֵ]
 *		����[]������㴫���intֵ��һ��key���������±�
 *		���key����int����ô�Ͳ���Ҫ(Exist::Key<����>)��
 */
template<class K, class T>
class Map :public Container<T>
{
	
public:
	Map( const char *name )
		:Container<T>(name)
	{
		this->m_type = DataType::map;
		IOBus::CreateExistObj( this, this->m_type );
		InitKeyType();
	}
	
	Map( const Map &value )
		:Container<T>(value)
	{
		this->m_type = DataType::map;
		IOBus::CreateExistObj( this, this->m_type );
		InitKeyType();
	}

	Map& operator=( const Map &right )
	{
		this->Copy(right);
		return *this;
	}

	virtual ~Map()
	{
	}

	bool Find( const K &key  )
	{
		char name[MAX_KEY_SIZE+1];
		int size = 0;
		this->ElementRealKey( &key, sizeof(K), name, size );

		return true;
	}
	
	Element<T> operator[]( const Exist::Key<K> &key )
	{
		K k = (Exist::Key<K>)key;
		char name[MAX_KEY_SIZE+1];
		int size = 0;
		this->ElementRealKey( &k, sizeof(K), name, size );
		Element<T> e(*this, 1, name, size);
		return e;
	}
	
	void Delete( const Exist::Key<K> &key )
	{
		K k = (Exist::Key<K>)key;
		char name[MAX_KEY_SIZE+1];
		int size = 0;
		this->ElementRealKey( &k, sizeof(K), name, size );
		return;
	}
	
	Element<T> operator[]( const int &index )
	{
		return this->Get( index );
	}
	
	void Delete( const int index )
	{
		Container<T>::Delete(index);
	}
	
	
protected:
	Map()
		:Container<T>()
	{
		this->m_type = DataType::map;
		IOBus::CreateExistObj( this, this->m_type );
		InitKeyType();
	}
	
	void InitKeyType()
	{
		K k = (*(K*)IOBus::s_defaultObj);
		this->m_elementKeyType = IOBus::GetDataType(&k);
	}
		
};

}

#endif // ifndef MAP_H
