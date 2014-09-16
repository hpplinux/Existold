// Int8.cpp: implementation of the Int8 class.
//
//////////////////////////////////////////////////////////////////////

#include "../include/Int8.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace Exist
{

Int8::Int8()
:Value<char>()
{
	m_type = DataType::Int8;
	IOBus::CreateExistObj( this, m_type );
	m_elementType = DataType::data;
}

Int8::Int8( const char *name )
:Value<char>(name)
{
	m_type = DataType::Int8;
	IOBus::CreateExistObj( this, m_type );
	m_elementType = DataType::data;
}

Int8::Int8( const Int8 &value )
:Value<char>(value)
{
	IOBus::CreateExistObj( this, m_type );
}

Int8& Int8::operator=( const Int8 &right )
{
	Copy(right);
	return *this;
}

Int8::~Int8()
{
}

Int8& Int8::operator=(const char& right)
{
	m_data = right;
	return *this;
}
	
}
