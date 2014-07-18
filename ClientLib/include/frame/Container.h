// Container.h: interface for the CContainer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONTAINER_H
#define CONTAINER_H

#include "KVData.h"
#include "Element.h"

namespace Exist
{
/*
	容器基础类
		非用户接口类，对用户透明
		Map, Vector, String等，所有容器接口都从此类派生
 */
template<class T>
class Container : public KVData<T, Element<T> >
{
public:
	Container( const Container &value )
		:KVData<T, Element<T> >(value)
	{
		InitElementType();
	}
	
	Container& operator=( const Container &right )
	{
		Copy(right);
		return *this;
	}

	virtual ~Container()
	{
	}

	int Count()
	{
		int size = 0;
		int sizeLen = 0;
		if ( !ReadData( m_hash, m_key, m_keySize, &size, sizeLen) ) return 0;
		return size;
	}
	
	Element<T> operator[]( const int index )
	{
		return Get( index );
	}

	Element<T> Get( const int index )
	{
		Element<T> e(*this, 0, &index, sizeof(int));
		return e;
	}
	
	void Delete( const int index )
	{
	}

protected:
	Container()
		:KVData<T, Element<T> >()
	{
		InitElementType();
	}
	
	Container( const char *name )
		:KVData<T, Element<T> >(name)
	{
		InitElementType();
	}
	
	void InitElementType()
	{
		T e = (*(T*)IOBus::s_defaultObj);
		m_elementType = IOBus::GetDataType(&e);
	}
	
private:

};

}

#endif //ifndef CONTAINER_H
