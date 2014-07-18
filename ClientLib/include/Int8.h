// Int8.h: interface for the Int8 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef INT8_H
#define INT8_H

#include "frame/Value.h"

namespace Exist
{

//typedef Value<int> Int8;

class Int8 : public Value<char>
{
public:
	Int8( const char *name );
	Int8( const Int8 &value );
	Int8& operator=( const Int8 &right );
	virtual ~Int8();
	Int8& operator=(const char& right);

protected:
	Int8();
};

}

#endif // ifndef INT8_H
