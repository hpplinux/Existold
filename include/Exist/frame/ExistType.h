#ifndef EXIST_TYPE_H
#define EXIST_TYPE_H

#include "mdk/FixLengthInt.h"

namespace DataType
{
	enum DataType
	{
		uninit = 0,
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//����
		lock,
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//�����������ͣ�ռint8~value��
		int8,
		uInt8,
		int16,
		uInt16,
		int32,
		uInt32,
		int64,
		uInt64,
		sFloat,			//��c++֧��
		sDouble,		//��c++֧��
		stream,			//�䳤������
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//��������
		vector,
		map,
	};
	bool IsValue( const char dt );//����ֵ����
	bool IsContainer( const char dt );//����������
}

namespace exist
{

#pragma pack(1)
	//����Key
	typedef struct DATA_KEY
	{
		char			key[256];			//key
		unsigned short	keySize;			//key����
		unsigned int	hashid;				//key����hash����õ���int32
		char			type;				//��������
		char			elementType;		//Ԫ������
	}DATA_KEY;
#pragma pack()
}

namespace UpdateType
{
	enum UpdateType
	{
		utCopy = 0,		// =
		utAddSelf,		// +=
		utSubtractSelf,	// -=
		utMultiplySelf,	// *=
		utDivideSelf,	// /=
	};
}

#endif //EXIST_TYPE_H