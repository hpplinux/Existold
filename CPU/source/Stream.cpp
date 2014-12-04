// Stream.cpp: implementation of the Stream class.
//
//////////////////////////////////////////////////////////////////////

#include "../../include/Exist/Stream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace Exist
{

Stream::Stream()
	:KVData<unsigned char*, Element<unsigned char*> >()
{
	this->m_type = DataType::stream;
	this->m_elementType = DataType::uninit;
	IOBus::CreateExistObj( this, m_type );
	CreateData();

}

Stream::Stream( const char *name )
	:KVData<unsigned char*, Element<unsigned char*> >(name, false)
{
	this->m_type = DataType::stream;
	this->m_elementType = DataType::uninit;
	IOBus::CreateExistObj( this, m_type );
	CreateData();
}

Stream::Stream( const Stream &value )
	:KVData<unsigned char*, Element<unsigned char*> >(value)
{
	this->m_type = DataType::stream;
	this->m_elementType = DataType::uninit;
	IOBus::CreateExistObj( this, m_type );
}

Stream& Stream::operator=( const Stream &right )
{
	this->Copy(right);
	return *this;
}

Stream::~Stream()
{
}

bool Stream::GetData(unsigned char *pData, int &size)
{
	if ( !QueryData() ) return false;
	if ( !ReadData(pData, size) ) 
	{
		SeekMsg();
		return false;
	}

	return true;
}


unsigned char* Stream::GetData(int &size)
{
	size = 0;
	unsigned char *pData = NULL;
	if ( QueryData() )
	{
		if ( !ReadData(pData, size) )
		{
			if ( 0 < size )
			{
				pData = new unsigned char[size];
				if ( NULL == pData ) return NULL;
				ReadData(pData, size);
			}
		}
	}

	return pData;
}

bool Stream::SetData(const std::string data, int size)
//bool SetData(const unsigned char *pData, int size)
{
	char *pData = (char*)data.data();
	return WriteData(pData, size, UpdateType::utCopy);
}

}
namespace SSD
{

	Stream::Stream()
		:Exist::KVData<unsigned char*, Exist::Element<unsigned char*> >()
	{
		this->m_type = DataType::stream;
		this->m_elementType = DataType::uninit;
		Exist::IOBus::CreateExistObj( this, m_type );
		CreateData();

	}

	Stream::Stream( const char *name )
		:Exist::KVData<unsigned char*, Exist::Element<unsigned char*> >(name, true)
	{
		this->m_type = DataType::stream;
		this->m_elementType = DataType::uninit;
		Exist::IOBus::CreateExistObj( this, m_type );
		CreateData();
	}

	Stream::Stream( const Stream &value )
		:Exist::KVData<unsigned char*, Exist::Element<unsigned char*> >(value)
	{
		this->m_type = DataType::stream;
		this->m_elementType = DataType::uninit;
		Exist::IOBus::CreateExistObj( this, m_type );
	}

	Stream& Stream::operator=( const Stream &right )
	{
		this->Copy(right);
		return *this;
	}

	Stream::~Stream()
	{
	}

	bool Stream::GetData(unsigned char *pData, int &size)
	{
		if ( !QueryData() ) return false;
		if ( !ReadData(pData, size) ) 
		{
			SeekMsg();
			return false;
		}

		return true;
	}


	unsigned char* Stream::GetData(int &size)
	{
		size = 0;
		unsigned char *pData = NULL;
		if ( QueryData() )
		{
			if ( !ReadData(pData, size) )
			{
				if ( 0 < size )
				{
					pData = new unsigned char[size];
					if ( NULL == pData ) return NULL;
					ReadData(pData, size);
				}
			}
		}

		return pData;
	}

	bool Stream::SetData(const std::string data, int size)
		//bool SetData(const unsigned char *pData, int size)
	{
		char *pData = (char*)data.data();
		return WriteData(pData, size, UpdateType::utCopy);
	}

}
