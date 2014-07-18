// test.cpp : Defines the entry point for the console application.
//

#include "include/frame/Container.h"
#include "include/Map.h"
#include <string>
#include "include/frame/Value.h"
#include "include/Int8.h"

#include <stdio.h>
#include <map>
std::map<void*, void*> m;
std::map<void*, void*>::iterator it;

int main(int argc, char* argv[])
{
	char sss[256];
	Exist::IOBus *p = (Exist::IOBus*)sss;
	memcpy( sss, "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 80 );
	Exist::Map<int, int> a( "test" );
	Exist::Map<int, int> a1 = a;
	a = a1;
	a[(Exist::Key<int>)2] = 0x12345678;
	a.Delete(2);
	a.Delete((Exist::Key<int>)2);
	
	int b = a[1];
	int c = a[1];
	Exist::Element<int> d = a[1];
	int jlk = d;
	Exist::Element<int> d1 = d;
	d = d1;
	
	Exist::Map<Exist::Map<int, int>, Exist::Map<char*, int> > a123( "123" );
	Exist::Map<char*, int> a456 = a123[a];
	Exist::Map<char*, int> a1231 = a456;
	a1231 = a456;

	Exist::Int8 dasds("23");
	dasds = 12;
	c = dasds;

	Exist::Int8 asad( "123" );
	asad = dasds;
	asad = 13;
	c = asad;
	Exist::Map<int, Exist::Int8> asa("123");
	char cdas = (Exist::Int8)asa[1] = 1;
	int casdsa = (Exist::Int8)asa[1];
	return 0;
}

