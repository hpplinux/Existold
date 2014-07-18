// IOBus.cpp: implementation of the IOBus class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/frame/IOBus.h"
#include "mdk/atom.h"
#include <cstring>
#include <cstdio>

namespace Exist
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
char IOBus::s_defaultObj[1024] = {0};
std::map<void*, int> IOBus::s_memoryMap;
void IOBus::AddMemoryMap( void *pObj, DataType::DataType type )
{
	if ( s_memoryMap.end() != s_memoryMap.find(pObj) ) return;
	s_memoryMap[pObj] = type;
}

DataType::DataType IOBus::GetDataType( void *pObj )
{
	std::map<void*, int>::iterator it = s_memoryMap.find(pObj);
	if ( s_memoryMap.end() == it ) return DataType::data;
	IOBus *pContainer = (IOBus*)pObj;
	if ( DataType::data > pContainer->Type() || DataType::Int8 < pContainer->Type() ) 
	{
		return DataType::data;
	}
	else 
	{
		return pContainer->Type();
	}
}

IOBus::IOBus()
{
	m_refCount = NULL;
	m_type = DataType::uninit;			//自身类型，给GetDataType()
	m_elementType = DataType::uninit;	//元素类型，通过GetDataType()得用到
	m_keySize = 0;
	m_key[m_keySize] = 0;
	m_hash = 820125;
}

IOBus::IOBus( const char *name )
{
	m_refCount = NULL;
	int size = strlen(name);
	if ( MAX_KEY_SIZE < size )
	{
		int *bug = NULL;
		*bug = 1;
	}
	Init(name, size);
	CreateData(m_hash, m_key, m_keySize );
}

void IOBus::Init( const char *name, int keySize )
{
	m_refCount = new int;
	*m_refCount = 1;
	m_keySize = keySize;
	memcpy( m_key, name, m_keySize );
	m_key[m_keySize] = 0;
	m_hash = 0;
}

void IOBus::Release()
{
	if ( NULL == m_refCount ) return;
	
	if ( 1 == mdk::AtomDec( m_refCount, 1 ) )//引用计数减少,最后一个引用，通知Exist释放数据
	{
	}
}

IOBus::IOBus( const IOBus &container )
{
	m_refCount = NULL;
	Copy(container);
}

IOBus& IOBus::operator=( const IOBus &right )
{
	Copy(right);
	return *this;
}

void IOBus::Copy( const IOBus &right )
{
	Release();
	if ( NULL != right.m_refCount )
	{
		mdk::AtomAdd(right.m_refCount, 1);//引用计数增加，需要改为原子操作
	}
	m_refCount = right.m_refCount;
	m_type = right.m_type;			//自身类型，给GetElementType()用
	m_elementType = right.m_elementType;	//元素类型，通过GetElementType()得到
	m_elementKeyType = right.m_elementKeyType;
	m_keySize = right.m_keySize;
	memcpy( m_key, right.m_key, m_keySize );
	m_key[m_keySize] = 0;
	m_hash = right.m_hash;
}

IOBus::~IOBus()
{
	s_memoryMap.erase( this );
	Release();
}

DataType::DataType IOBus::Type()
{
	return m_type;
}

DataType::DataType IOBus::ElementType()
{
	return m_elementType;
}

char* IOBus::Key()
{
	return m_key; 
}

int IOBus::KeySize()
{
	return m_keySize; 
}

void IOBus::ElementRealKey( const void* localKey, const int &localKeySize, char *globalkey, int &globalkeySize )
{
	globalkeySize = 0;
	if ( DataType::data == m_elementKeyType )
	{
		memcpy( &globalkey[globalkeySize], Key(), KeySize() );
		globalkeySize += KeySize();
		memcpy( &globalkey[globalkeySize], "::", 2 );
		globalkeySize += 2;
		memcpy( &globalkey[globalkeySize], (char*)&localKey, localKeySize );
		globalkeySize += localKeySize;
		globalkey[globalkeySize] = 0;
	}
	else
	{
		IOBus *p = (IOBus*)localKey;
		memcpy( &globalkey[globalkeySize], Key(), KeySize() );
		globalkeySize += KeySize();
		memcpy( &globalkey[globalkeySize], "::", 2 );
		globalkeySize += 2;
		memcpy( &globalkey[globalkeySize], p->Key(), p->KeySize() );
		globalkeySize += p->KeySize();
		globalkey[globalkeySize] = 0;
	}
}

bool IOBus::CreateData( unsigned int hash, char *key, int keySize )
{
	printf( "IOBus::CreateData(%d, %s, %d)\n", hash, key, keySize );
	return true;
}

bool IOBus::ReadData( int index, void *pData, int &size )
{
	printf( "IOBus::%s[%d]:Read %d byte data by index\n", m_key, index, size );
	return true;
}

bool IOBus::WriteData( int index, void *pData, int size )
{
	printf( "IOBus::%s[%d]:Write %d byte data by index\n", m_key, index, size );
	unsigned char *p = (unsigned char *)pData;
	int i = 0;
	printf( "data:" );
	for ( ; i < size; i++ )
	{
		if ( 0 < i ) printf( ",");
		printf( "%x", p[i] );
	}
	printf( "\n");
	return true;
}

bool IOBus::ReadData( unsigned int hash, char *key, int keySize, void *pData, int &size )
{
	printf( "IOBus::%s[%s]:Read %d byte data by key\n", m_key, key, size );
	return true;
}

bool IOBus::WriteData( unsigned int hash, char *key, int keySize, void *pData, int size )
{
	printf( "IOBus::%s[%s]:Write %d byte data by key\n", m_key, key, size );
	unsigned char *p = (unsigned char *)pData;
	int i = 0;
	printf( "data:" );
	for ( ; i < size; i++ )
	{
		if ( 0 < i ) printf( ",");
		printf( "%x", p[i] );
	}
	printf( "\n");
	return true;
}

}
