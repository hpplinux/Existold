#include "../../../include/Exist/frame/ExistType.h"

namespace DataType
{

	bool IsValue( const char dt )
	{
		if ( DataType::sFloat == dt
			|| DataType::sDouble == dt
			|| DataType::int8 == dt
			|| DataType::uInt8 == dt
			|| DataType::int16 == dt
			|| DataType::uInt16 == dt
			|| DataType::int32 == dt
			|| DataType::uInt32 == dt
			|| DataType::int64 == dt
			|| DataType::uInt64 == dt
			) 
		{
			return true;
		}

		return false;
	}

	bool IsContainer( const char dt )
	{
		if ( DataType::vector == dt
			|| DataType::map == dt ) 
		{
			return true;
		}

		return false;
	}

}
