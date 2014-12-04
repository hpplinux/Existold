#include <cstdio>
#include "NoDB.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment ( lib, "../Micro-Development-Kit/lib/mdk_d.lib" )
#else
#pragma comment ( lib, "../Micro-Development-Kit/lib/mdk.lib" )
#endif
#endif
#include "../common/ExistFS.h"
#include "../common/common.h"
#include "mdk/Socket.h"

//external in stored
int main( int argc, char** argv )
{
	char exeDir[256];
	int size = 256;
	mdk::GetExeDir( exeDir, size );//ȡ�ÿ�ִ�г���λ��
	char configFile[256];
#ifdef EXIST_DEVICE
	sprintf( configFile, "%s/../conf/Exist.cfg", exeDir );
#endif
#ifdef SSD_DEVICE
	sprintf( configFile, "%s/../conf/SolidStateDrive.cfg", exeDir );
#endif
	NoDB ser( configFile );
	const char *ret = ser.Start();
	if ( NULL != ret )
	{
		ser.GetLog().Info( "info:","���������ʧ��:%s", ret );
		return 0;
	}
	ser.WaitStop();
	ser.GetLog().Info("info:", "������Ѵ������ϰγ�" );
	return 0;
}
