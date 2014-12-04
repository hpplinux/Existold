// IOBus.cpp: implementation of the IOBus class.
//
//////////////////////////////////////////////////////////////////////

#include "../../../include/Exist/frame/IOBus.h"
#include "../../../include/Exist/frame/ConnectPool.h"
#include "mdk/atom.h"
#include <cstring>
#include <cstdio>
#include "../../../common/MD5Helper.h"
#include "../../../common/Protocol.h"
#include "../../../common/common.h"


namespace Exist
{

bool PlugIn()
{
	return IOBus::s_connectPool.PlugIn();
}
	
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
char IOBus::s_defaultObj[1024] = {0};
std::map<void*, int> IOBus::s_memoryMap;
mdk::Mutex IOBus::s_lockMemoryMap;
ConnectPool IOBus::s_connectPool;

void IOBus::CreateExistObj( void *pObj, DataType::DataType type )
{
	mdk::AutoLock lock(&s_lockMemoryMap);
	if ( s_memoryMap.end() != s_memoryMap.find(pObj) ) return;
	s_memoryMap[pObj] = type;
}

void IOBus::ReleaseExistObj( void *pObj )
{
	mdk::AutoLock lock(&s_lockMemoryMap);
	s_memoryMap.erase( pObj );
}

DataType::DataType IOBus::GetDataType( void *pObj )
{
	std::map<void*, int>::iterator it = s_memoryMap.find(pObj);
	if ( s_memoryMap.end() == it ) return DataType::uninit;
	IOBus *pContainer = (IOBus*)pObj;
	return pContainer->Type();
}

IOBus::IOBus()
{
	m_type = DataType::uninit;			//�������ͣ���GetDataType()
	m_size = 0;
	m_elementType = DataType::uninit;	//Ԫ�����ͣ�ͨ��GetDataType()���õ�
	m_keySize = 0;
	m_key[m_keySize] = 0;
	m_hash = 820125;
	m_protracted = false;
}

IOBus::IOBus( const char *name, bool protracted )
{
	int size = strlen(name);
	if ( MAX_KEY_SIZE < size )
	{
		int *bug = NULL;
		*bug = 1;
	}
	m_size = 0;
	Init(name, size, protracted);
}

void IOBus::Init( const char *name, int keySize, bool protracted )
{
	m_protracted = protracted;
	m_keySize = keySize;
	memcpy( m_key, name, m_keySize );
	m_key[m_keySize] = 0;
	MD5Helper md5h;
	m_hash = md5h.HashValue(m_key,m_keySize);
}

void IOBus::DeleteData()
{
	DeleteData( m_hash, m_key, m_keySize );
}

IOBus::IOBus( const IOBus &container )
{
	Copy(container);
}

IOBus& IOBus::operator=( const IOBus &right )
{
	Copy(right);
	return *this;
}

void IOBus::Copy( const IOBus &right )
{
	m_protracted = right.m_protracted;
	m_type = right.m_type;			//�������ͣ���GetElementType()��
	m_size = right.m_size;
	m_elementType = right.m_elementType;	//Ԫ�����ͣ�ͨ��GetElementType()�õ�
	m_elementKeyType = right.m_elementKeyType;
	m_keySize = right.m_keySize;
	memcpy( m_key, right.m_key, m_keySize );
	m_key[m_keySize] = 0;
	m_hash = right.m_hash;
}

IOBus::~IOBus()
{
	s_memoryMap.erase( this );
}

bool IOBus::IsProtracted()
{
	return m_protracted;
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
	if ( DataType::IsValue(m_elementKeyType) 
		|| DataType::stream == m_elementKeyType )
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

bool IOBus::CreateData()
{	
	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( m_hash );
	char msg[1024];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	CREATE_DATA *pParam = (CREATE_DATA*)&msg[sizeof(MSG_HEADER)];
	pHeader->msgId = MsgId::createData;
	pHeader->msgSize = sizeof(CREATE_DATA);
	pParam->key.hashid = m_hash;
	pParam->key.keySize = m_keySize;
	memcpy(pParam->key.key, m_key, pParam->key.keySize);
	pParam->key.type = m_type;
	pParam->key.elementType = m_elementType;
	pParam->size = m_size;
	pParam->protracted = m_protracted;
	if ( 0 > cpu.Send(msg, sizeof(MSG_HEADER) + pHeader->msgSize) )
	{
		cpu.Close();
		return false;
	}

	return true;
}

bool IOBus::DeleteData( unsigned int hash, char *key, int keySize )
{
	//ɾ�����ܣ����ڴ�
	return false;

	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( hash );
	char msg[1024];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	exist::DATA_KEY *pParam = (exist::DATA_KEY*)&msg[sizeof(MSG_HEADER)];
	pHeader->msgId = MsgId::deleteData;
	pHeader->msgSize = sizeof(exist::DATA_KEY);
	pParam->hashid = m_hash;
	pParam->keySize = m_keySize;
	pParam->type = m_type;
	memcpy(pParam->key, m_key, pParam->keySize);
	if ( 0 > cpu.Send(msg, sizeof(MSG_HEADER) + pHeader->msgSize) )
	{
		cpu.Close();
		return false;
	}

	return true;
}

bool IOBus::QueryData()
{
	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( m_hash );
	char msg[1024];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	exist::DATA_KEY *pKey = (exist::DATA_KEY*)&msg[sizeof(MSG_HEADER)];
	pHeader->msgId = MsgId::readData;
	pHeader->msgSize = sizeof(exist::DATA_KEY);
	pKey->hashid = m_hash;
	pKey->keySize = m_keySize;
	pKey->type = m_type;
	memcpy(pKey->key, m_key, pKey->keySize);
	if ( 0 > cpu.Send(msg, sizeof(MSG_HEADER) + pHeader->msgSize) )
	{
		printf( "send close\n" );
		mdk::mdk_assert(false);
	}

	return true;
}

bool IOBus::SeekMsg()
{
	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( m_hash );
	char msg[1024];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	if ( 0 > cpu.Receive(msg, sizeof(MSG_HEADER)) )
	{
		cpu.Close();
		return false;
	}
	int size;
	while ( pHeader->msgSize > 0 )
	{
		if ( pHeader->msgSize > 1024 ) size = 1024;
		else size = pHeader->msgSize;
		pHeader->msgSize -= size;
		if ( 0 > cpu.Receive(msg, size) )
		{
			cpu.Close();
			return false;
		}
	}

	return true;
}

bool IOBus::ReadData( void *pData, int &size )
{
	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( m_hash );
	char msg[1024];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	if ( 0 > cpu.Receive(msg, sizeof(MSG_HEADER), true) )
	{
		cpu.Close();
		return false;
	}
	if ( size < pHeader->msgSize ) 
	{
		size = pHeader->msgSize;
		return false;
	}
	if ( 0 > cpu.Receive(msg, sizeof(MSG_HEADER)) )
	{
		cpu.Close();
		return false;
	}
	if ( pHeader->msgSize < 0 ) 
	{
		return false;
	}
	if ( pHeader->msgSize == 0 ) 
	{
		memset( pData, 0, size );
		size = 0;
		return true;
	}

	if ( 0 > cpu.Receive(pData, pHeader->msgSize) )
	{
		cpu.Close();
		return false;
	}
	size = pHeader->msgSize;

	return true;
}

bool IOBus::WriteData( void *pData, int size, UpdateType::UpdateType updateType )
{
	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( m_hash );
	char msg[1024];
	MSG_HEADER *pHeader = (MSG_HEADER*)msg;
	WRITE_DATA *pParam = (WRITE_DATA*)&msg[sizeof(MSG_HEADER)];
	pHeader->msgId = MsgId::writeData;
	pHeader->msgSize = sizeof(WRITE_DATA) + size;
	pParam->key.hashid = m_hash;
	pParam->key.keySize = m_keySize;
	memcpy(pParam->key.key, m_key, pParam->key.keySize);
	pParam->updateType = updateType;
	pParam->key.type = m_type;
	pParam->size = size;
	if ( 0 > cpu.Send(msg, sizeof(MSG_HEADER) + sizeof(WRITE_DATA)) )
	{
		cpu.Close();
		return false;
	}
	if ( 0 > cpu.Send(pData, size) )
	{
		cpu.Close();
		return false;
	}

	return true;
}

bool IOBus::ReadData( int index, void *pData, int &size )
{
	unsigned int hash = 0;
	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( hash );

	return true;
}

bool IOBus::WriteData( int index, void *pData, int size )
{
	unsigned int hash = 0;
	mdk::Socket &cpu = IOBus::s_connectPool.GetSocket( hash );

	return true;
}

}
