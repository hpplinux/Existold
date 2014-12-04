#include <cstdio>
#include "TCPWorker.h"
#include "mdk/mapi.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment ( lib, "../Micro-Development-Kit/lib/mdk_d.lib" )
#else
#pragma comment ( lib, "../Micro-Development-Kit/lib/mdk.lib" )
#endif
#endif

//״̬������
int main( int argc, char** argv )
{
	char exeDir[256];
	int size = 256;
	mdk::GetExeDir( exeDir, size );//ȡ�ÿ�ִ�г���λ��
	char configFile[256];
	sprintf( configFile, "%s/../conf/Mother.cfg", exeDir );
	
	TCPWorker ser( configFile );
	const char *ret = ser.Start();
	if ( NULL != ret )
	{
		ser.GetLog().Info( "info:","����%sʧ��:%s", Device::Descript(Device::Type::motherboard), ret );
		return 0;
	}
	ser.GetLog().Info("info:", "%s��ʼ����", Device::Descript(Device::Type::motherboard) );
	ser.WaitStop();
	ser.GetLog().Info("info:", "%s�ѹرյ�Դ", Device::Descript(Device::Type::motherboard) );

	return 0;
}
