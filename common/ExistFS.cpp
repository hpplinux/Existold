#include "ExistFS.h"
#include "MD5Helper.h"
#ifdef WIN32
#include <io.h>
#include <direct.h>
#else
#include   <unistd.h>                     //chdir() 
#include   <sys/stat.h>                 //mkdir() 
#include   <sys/types.h>               //mkdir() 
#include   <dirent.h>					//closedir()
#endif
#include <cstdio>
#include <cstring>

namespace exist
{

ExistFS::ExistFile::ExistFile()
{
	m_pFile = NULL;
}

ExistFS::ExistFile::~ExistFile()
{
	Close();
}

bool ExistFS::ExistFile::IsOpend()
{
	return NULL != m_pFile;
}

bool ExistFS::ExistFile::Open( const char *filename, const char *mode )
{
	if ( IsOpend() ) return true;

	m_pFile = fopen( filename, mode );
	if ( NULL != m_pFile ) 
	{
		return true;
	}
	CreateFile( filename );
	m_pFile = fopen( filename, mode );
	if ( NULL == m_pFile ) return false;

	return true;
}

ExistFS::ExistFile::operator FILE*()
{
	return m_pFile;
}

void ExistFS::ExistFile::Close()
{
	if ( !IsOpend() ) return;
	fclose( m_pFile );
	m_pFile = NULL;
}

bool ExistFS::ExistFile::CreateDir( const char *strDir )
{
	std::string path = strDir;
	int startPos = 0;
	int pos = path.find( '/', startPos );
	std::string dir;
	while ( true )
	{
		if ( -1 == pos ) dir = path;
		else dir.assign( path, 0, pos );
		if ( -1 == access( dir.c_str(), 0 ) )
		{
#ifdef WIN32
			if( 0 != mkdir(dir.c_str()) ) return false;
#else
			umask(0);
			if( 0 != mkdir(strDir, 0777) ) return false;
			umask(0);
			chmod(strDir,0777);
#endif
		}
		if ( -1 == pos ) break;
		startPos = pos + 1;
		pos = path.find( '/', startPos );
	}

	return true;
}

bool ExistFS::ExistFile::CreateFile( const char *strFile )
{
	if ( 0 == access( strFile, 0 ) ) 
	{
#ifndef WIN32
		umask(0);
		chmod(strFile,0777);
#endif
		return true;
	}

	std::string file = strFile;
	int pos = file.find_last_of( '/', file.size() );
	if ( -1 != pos ) 
	{
		std::string dir;
		dir.assign( file, 0, pos );
		if ( !CreateDir(dir.c_str()) ) return false;
	}
	FILE *fp = fopen( strFile, "w" );
	if ( NULL == fp ) return false;
	fclose( fp );
#ifdef WIN32
#else
	umask(0);
	chmod(strFile,0777);
#endif
	return true;
}

ExistFS::ExistFS(void)
{
}

ExistFS::~ExistFS(void)
{
}

void ExistFS::StringReplace(std::string &strBase, const char *src, const char *des)  
{  
	std::string::size_type pos = 0;  
	std::string::size_type srcLen = strlen(src);  
	std::string::size_type desLen = strlen(des);  
	pos=strBase.find(src, pos);   
	while ((pos != std::string::npos))  
	{  
		strBase.replace(pos, srcLen, des);  
		pos=strBase.find(src, (pos+desLen));  
	}  
}

void ExistFS::StringTrim(std::string &strBase, const char c)  
{
	char src[3];
	char des[2];
	des[0] = src[0] = src[1] = c;
	des[1] = src[2] = '\0';
	while ( true )
	{
		int size = strBase.size();
		StringReplace(strBase, src, des);
		if ( size == (int)strBase.size() ) break;
	}
}

void ExistFS::SetRootDir( const char *rootDir )
{
	m_rootDir = rootDir;
	StringReplace(m_rootDir, "\\", "/");//win·���ָ��ת��Ϊlinux
	StringTrim( m_rootDir, '/' );//ѹ������ָ���
	ExistFile::CreateDir( m_rootDir.c_str() );
}

void ExistFS::GetOnwerDir( VALUE *pValue, std::string &dir )
{
	if ( NULL == pValue  ) return;
	GetOnwerDir( pValue->pParent, dir );
	std::string key;
	key.assign(pValue->key.key, pValue->key.keySize);
	dir += "/" + key;
	return;
}

const char* ExistFS::WriteValue( VALUE *pValue )
{
	std::string indexFileName;
	std::string dataFileName;
	std::string dir = m_rootDir;
	GetOnwerDir( pValue->pParent, dir );
	if ( DataType::int8 == pValue->key.type ) dir += "/int8";
	else if ( DataType::uInt8 == pValue->key.type ) dir += "/uInt8";
	else if ( DataType::int16 == pValue->key.type ) dir += "/int16";
	else if ( DataType::uInt16 == pValue->key.type ) dir += "/uInt16";
	else if ( DataType::int32 == pValue->key.type ) dir += "/int32";
	else if ( DataType::uInt32 == pValue->key.type ) dir += "/uInt32";
	else if ( DataType::int64 == pValue->key.type ) dir += "/int64";
	else if ( DataType::uInt64 == pValue->key.type ) dir += "/uInt64";
	else if ( DataType::sFloat == pValue->key.type ) dir += "/float";
	else if ( DataType::sDouble == pValue->key.type ) dir += "/double";
	else if ( DataType::stream == pValue->key.type ) dir += "/stream";
	else return "��������δ����";

	char filename[256];
	sprintf(filename, "%d", (unsigned char)pValue->key.key[0]);
	indexFileName = dir + "/" + filename + ".idx";
	dataFileName = dir + "/" + filename + ".db";
	if ( !pValue->idxAble ) return "����δ��������";
	if ( pValue->idxPos < 0 ) return "������������";
	//��ȡ��������
	mdk::uint64 oldDBIndex, newDBIndex;
	mdk::uint8 oldDelMark;
	const char *ret = GetDBIndex( indexFileName.c_str(), pValue, oldDBIndex, oldDelMark );
	if ( NULL != ret ) return ret;
	newDBIndex = oldDBIndex;

	//�޸�����
	ExistFile dbFile;
	if ( !dbFile.Open(dataFileName.c_str(), "rb+") ) return "�޷��޸������ļ�";
	if ( DataType::IsValue(pValue->key.type) )//��������
	{
		if ( -1 == fseek(dbFile, oldDBIndex, SEEK_SET) ) return "��������";
		if ( pValue->size != fwrite( pValue->pData, sizeof(char), pValue->size, dbFile) ) return "д�볤�Ȳ���";
	}
	else if ( DataType::stream == pValue->key.type ) //�䳤����
	{
		mdk::uint32 oldSize = 0;
		ret = GetDataSize(dataFileName.c_str(), oldDBIndex, oldSize);
		if ( NULL != ret ) return ret;
		//��������
		if ( oldSize > pValue->size )//��Ҫ�������ݳ���
		{
			if ( -1 == fseek(dbFile, oldDBIndex, SEEK_SET) ) return "��������";
			if ( sizeof(mdk::uint32) != fwrite( &pValue->size, sizeof(char), sizeof(mdk::uint32), dbFile) ) return "д�볤�Ȳ���";
			if ( pValue->size != fwrite( pValue->pData, sizeof(char), pValue->size, dbFile) ) return "д�볤�Ȳ���";
		}
		else if ( oldSize == pValue->size )//ֱ�Ӹ�������
		{
			if ( -1 == fseek(dbFile, oldDBIndex + sizeof(mdk::uint32), SEEK_SET) ) return "��������";
			if ( pValue->size != fwrite( pValue->pData, sizeof(char), pValue->size, dbFile) ) return "д�볤�Ȳ���";
		}
		else//�����ƶ��ƶ����ļ�ĩβ
		{
			//���ļ�ĩβд��
			if ( -1 == fseek(dbFile, 0, SEEK_END) ) return "��������";
			newDBIndex = ftell(dbFile);//��¼������
			if ( sizeof(mdk::uint32) != fwrite( &pValue->size, sizeof(char), sizeof(mdk::uint32), dbFile) ) return "д�볤�Ȳ���";
			if ( pValue->size != fwrite( pValue->pData, sizeof(char), pValue->size, dbFile) ) return "д�볤�Ȳ���";
		}
	}
	else
	{
		return "δ������������";
	}
	dbFile.Close();

	return SetDBIndex(indexFileName.c_str(), pValue, newDBIndex, oldDBIndex, 0, oldDelMark);//�����Ҫ�����������
}

const char* ExistFS::CreateData( VALUE *pValue )
{
	std::string indexFileName;
	std::string dataFileName;
	std::string dir = m_rootDir;
	GetOnwerDir( pValue->pParent, dir );
	if ( DataType::int8 == pValue->key.type ) dir += "/int8";
	else if ( DataType::uInt8 == pValue->key.type ) dir += "/uInt8";
	else if ( DataType::int16 == pValue->key.type ) dir += "/int16";
	else if ( DataType::uInt16 == pValue->key.type ) dir += "/uInt16";
	else if ( DataType::int32 == pValue->key.type ) dir += "/int32";
	else if ( DataType::uInt32 == pValue->key.type ) dir += "/uInt32";
	else if ( DataType::int64 == pValue->key.type ) dir += "/int64";
	else if ( DataType::uInt64 == pValue->key.type ) dir += "/uInt64";
	else if ( DataType::sFloat == pValue->key.type ) dir += "/float";
	else if ( DataType::sDouble == pValue->key.type ) dir += "/double";
	else if ( DataType::stream == pValue->key.type ) dir += "/stream";
	else return "��������δ����";

	char filename[256];
	sprintf( filename, "%d", (unsigned char)(pValue->key.key[0]) );
	indexFileName = dir + "/" + filename + ".idx";
	dataFileName = dir + "/" + filename + ".db";
	ExistFile idxFile;
	ExistFile dbFile;
	if ( !idxFile.Open(indexFileName.c_str(), "rb+") ) return "�޷����������ļ�";
	if ( !dbFile.Open(dataFileName.c_str(), "rb+") ) return "�޷����������ļ�";

	//д���ʼֵ��������������д��0��stream����д��0��������
	mdk::uint8 delMark = 0;
	if ( -1 == fseek( dbFile, 0, SEEK_END ) ) return "������";
	mdk::uint64 idx = ftell(dbFile);
	if ( DataType::IsValue(pValue->key.type) )//��������
	{
		if ( pValue->size != fwrite( pValue->pData, sizeof(char), pValue->size, dbFile) ) 
		{
			return "д�볤�Ȳ���";
		}
		dbFile.Close();
	}
	else if ( DataType::stream == pValue->key.type )//�䳤����
	{
		mdk::uint32 dataSize = 0;
		if ( sizeof(mdk::uint32) != fwrite( &dataSize, sizeof(char), sizeof(mdk::uint32), dbFile) ) 
		{
			return "д�볤�Ȳ���";
		}
		dbFile.Close();
	}
	else
	{
		return "δ������������";
	}

	//��������:key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)
	if ( -1 == fseek( idxFile, 0, SEEK_END ) ) return "������";
	pValue->idxPos = ftell(idxFile);
	if ( sizeof(short) != fwrite( &pValue->key.keySize, sizeof(char), sizeof(short), idxFile) ) 
	{
		return "д�볤�Ȳ���";
	}
	if ( sizeof(mdk::uint8) != fwrite( &delMark, sizeof(char), sizeof(mdk::uint8), idxFile) ) 
	{
		return "д�볤�Ȳ���";
	}
	if ( pValue->key.keySize != fwrite( pValue->key.key, sizeof(char), pValue->key.keySize, idxFile) )
	{
		return "д�볤�Ȳ���";
	}
	if ( sizeof(mdk::uint64) != fwrite( &idx, sizeof(char), sizeof(mdk::uint64), idxFile) )
	{
		return "д�볤�Ȳ���";
	}
	pValue->idxAble = true;
	idxFile.Close();
	return NULL;
}

const char* ExistFS::CreateTable( void *createParam, int createParamSize, unsigned char *path, int pathSize )
{
	short sumSize = createParamSize + pathSize;
	if ( sizeof(short) == fwrite( &sumSize, sizeof(char), sizeof(short), m_tableFile ) ) 
	{
		m_tableFile.Close();
		return "д�볤�Ȳ�����";
	}
	if ( createParamSize == (int)fwrite( createParam, sizeof(char), createParamSize, m_tableFile ) ) 
	{
		m_tableFile.Close();
		return "д�볤�Ȳ�����";
	}
	if ( pathSize == (int)fwrite( path, sizeof(char), pathSize, m_tableFile ) ) 
	{
		m_tableFile.Close();
		return "д�볤�Ȳ�����";
	}

	return NULL;
}

const char* ExistFS::MoveFristTable()
{
	m_tableFile.Close();
	std::string tableFileName = m_rootDir + "/table.lst";
	ExistFile tableFile;
	if ( !m_tableFile.Open( tableFileName.c_str(), "rb" ) ) return "�޷����ʱ�����";

	return NULL;
}

const char* ExistFS::GetTable( unsigned char *createData, short &size )
{
	size = 0;
	if ( !m_tableFile.IsOpend() ) return "������δ��";
	//size(short)+CREATE_DATA+path
	int readSize = fread( &size, sizeof(char), sizeof(short), m_tableFile );
	if ( 0 == readSize )//���б��Ѷ���
	{
		m_tableFile.Close();
		//��ӷ�ʽ��
		std::string tableFileName = m_rootDir + "/table.lst";
		if ( !m_tableFile.Open( tableFileName.c_str(), "ab" ) )
		{
			char *p = NULL;
			*p = 0;//ǿ�Ʊ���
			return "�޷��޸ı�����";
		}
		return NULL;
	}
	if ( sizeof(short) != readSize ) 
	{
		m_tableFile.Close();
		return "��������";
	}
	if ( size == (short)fread( createData, sizeof(char), size, m_tableFile ) ) 
	{
		m_tableFile.Close();
		return "��ȡ���Ȳ���";
	}

	return NULL;
}

const char* ExistFS::ReadTable(exist::VALUE *pTable, std::vector<exist::VALUE*> &data )
{
	std::string dir = m_rootDir;
	GetOnwerDir( pTable, dir );
	exist::VALUE val;
	val.size = 0;
	if ( DataType::int8 == pTable->key.elementType ) 
	{
		dir += "/int8";
		val.size = sizeof(mdk::int8);
	}
	else if ( DataType::uInt8 == pTable->key.elementType ) 
	{
		dir += "/uInt8";
		val.size = sizeof(mdk::uint8);
	}
	else if ( DataType::int16 == pTable->key.elementType ) 
	{
		dir += "/int16";
		val.size = sizeof(mdk::int16);
	}
	else if ( DataType::uInt16 == pTable->key.elementType ) 
	{
		dir += "/uInt16";
		val.size = sizeof(mdk::uint16);
	}
	else if ( DataType::int32 == pTable->key.elementType ) 
	{
		dir += "/int32";
		val.size = sizeof(mdk::int32);
	}
	else if ( DataType::uInt32 == pTable->key.elementType ) 
	{
		dir += "/uInt32";
		val.size = sizeof(mdk::uint32);
	}
	else if ( DataType::int64 == pTable->key.elementType ) 
	{
		dir += "/int64";
		val.size = sizeof(mdk::int64);
	}
	else if ( DataType::uInt64 == pTable->key.elementType ) 
	{
		dir += "/uInt64";
		val.size = sizeof(mdk::uint64);
	}
	else if ( DataType::sFloat == pTable->key.elementType ) 
	{
		dir += "/float";
		val.size = sizeof(float);
	}
	else if ( DataType::sDouble == pTable->key.elementType ) 
	{
		dir += "/double";
		val.size = sizeof(double);
	}
	else if ( DataType::stream == pTable->key.elementType ) dir += "/stream";
	else return "��������δ����";

	std::string indexFileName;
	std::string dataFileName;
	char filename[256];
	MD5Helper md5Helper;
	exist::VALUE *pValue;
	mdk::uint8 delMark;

	int i = 0;
	for ( i = 0; i < 256; i++ )
	{
		sprintf( filename, "%d", i );
		indexFileName = dir + "/" + filename + ".idx";
		dataFileName = dir + "/" + filename + ".db";
		if ( 0 != access( indexFileName.c_str(), 0 ) ) continue;
		if ( 0 != access( dataFileName.c_str(), 0 ) ) continue;

		ExistFile indexFile;
		ExistFile dataFile;
		if ( !indexFile.Open( indexFileName.c_str(), "rb" ) ) continue;
		if ( !dataFile.Open( dataFileName.c_str(), "rb" ) ) continue;

		//key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)
		val.idxAble = true;
		val.pParent = pTable;
		val.protracted = true;
		val.key.type = pTable->key.elementType;
		val.delMark = false;
		mdk::uint64 dbIndex;
		while ( true )
		{
			val.idxPos = ftell(indexFile);
			//������
			if ( sizeof(short) != fread( &val.key.keySize, sizeof(char), sizeof(short), indexFile ) ) break;
			if ( val.key.keySize > 256 ) 
			{
				return "key���ȳ���256byte";
			}
			if ( sizeof(mdk::uint8) != fread( &delMark, sizeof(char), sizeof(mdk::uint8), indexFile ) ) 
			{
				return "������������";
			}
			if ( 0 != delMark ) 
			{
				fseek( indexFile, val.key.keySize+sizeof(mdk::uint64), SEEK_CUR );
				continue;
			}
			if ( val.key.keySize != fread( val.key.key, sizeof(char), val.key.keySize, indexFile ) ) 
			{
				return "������";
			}
			if ( sizeof(mdk::uint64) != fread( &dbIndex, sizeof(char), sizeof(mdk::uint64), indexFile ) ) 
			{
				return "������";
			}

			//������
			if ( DataType::IsValue(val.key.type) )//��������
			{
				if ( -1 == fseek( dataFile, dbIndex, SEEK_SET) ) return "��������";
				val.pData = new char[val.size];
				if ( NULL == val.pData )
				{
					return "�ڴ治��";
				}
				if ( val.size != fread( val.pData, sizeof(char), val.size, dataFile) )
				{
					delete[]val.pData;
					return "��������";
				}
				pValue = new exist::VALUE;
				if ( NULL == pValue )
				{
					delete[]val.pData;
					return "�ڴ治��";
				}
				val.key.hashid = md5Helper.HashValue(val.key.key, val.key.keySize);
				*pValue = val;
				data.push_back(pValue);
				continue;
			}
			if ( DataType::stream == val.key.type ) 
			{
				if ( -1 == fseek( dataFile, dbIndex, SEEK_SET) ) 
				{
					return "��������";
				}
				if ( sizeof(mdk::uint32) != fread( &val.size, sizeof(char), sizeof(mdk::uint32), dataFile) )
				{
					return "��������";
				}
				val.pData = new char[val.size];
				if ( NULL == val.pData )
				{
					return "�ڴ治��";
				}
				if ( val.size != fread( val.pData, sizeof(char), val.size, dataFile) )
				{
					delete[]val.pData;
					return "��������";
				}
				pValue = new exist::VALUE;
				if ( NULL == pValue )
				{
					delete[]val.pData;
					return "�ڴ治��";
				}
				val.key.hashid = md5Helper.HashValue(val.key.key, val.key.keySize);
				*pValue = val;
				data.push_back(pValue);
				continue;
			}

			return "δ������������";
		}
	}

	return NULL;
}

const char* ExistFS::GetDBIndex( const char *idxFileName, VALUE *pValue, mdk::uint64 &dbPos, mdk::uint8 &delMark )
{
	ExistFile idxFile;
	if ( !idxFile.Open( idxFileName, "rb" ) ) 
	{
		return "�޷��������������ļ�";
	}

	//idx�ļ���ʽ
	//key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)+...+key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)
	if ( -1 == fseek( idxFile, pValue->idxPos + 2, SEEK_SET) ) 
	{
		return "��������";
	}
	if ( sizeof(mdk::uint8) != fread( &delMark, sizeof(char), sizeof(mdk::uint8), idxFile) )
	{
		return "��������";
	}
	if ( -1 == fseek( idxFile, pValue->key.keySize, SEEK_CUR) ) 
	{
		return "��������";
	}
	if ( sizeof(mdk::uint64) != fread( &dbPos, sizeof(char), sizeof(mdk::uint64), idxFile) )
	{
		return "��������";
	}

	return NULL;
}

const char* ExistFS::SetDBIndex( const char *idxFileName, VALUE *pValue, mdk::uint64 newDBIndex, 
	mdk::uint64 oldDBIndex, mdk::uint8 newDelMark, mdk::uint8 oldDelMark )
{
	if ( newDBIndex == oldDBIndex && newDelMark == oldDelMark ) return NULL;//����Ҫ��������

	ExistFile idxFile;
	if ( !idxFile.Open( idxFileName, "rb+" ) ) 
	{
		return "�޷��������������ļ�";
	}

	//idx�ļ���ʽ
	//key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)+...+key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)
	if ( newDelMark != oldDelMark ) 
	{
		if ( -1 == fseek( idxFile, pValue->idxPos + 2, SEEK_SET) ) 
		{
			return "��������";
		}
		if ( sizeof(mdk::uint8) != fwrite( &newDelMark, sizeof(char), sizeof(mdk::uint8), idxFile) ) 
		{
			return "д��delMarkʧ��";
		}
		if ( newDBIndex == oldDBIndex ) return NULL;

		if ( -1 == fseek( idxFile, pValue->key.keySize, SEEK_CUR) ) 
		{
			return "��������";
		}
		if ( sizeof(mdk::uint64) != fwrite( &newDBIndex, sizeof(char), sizeof(mdk::uint64), idxFile) ) 
		{
			return "д������ʧ��";
		}
	}
	else
	{
		if ( -1 == fseek( idxFile, pValue->idxPos + 2 + sizeof(mdk::uint8) 
									+ pValue->key.keySize, SEEK_SET) ) 
		{
			return "��������";
		}
		if ( sizeof(mdk::uint64) != fwrite( &newDBIndex, sizeof(char), sizeof(mdk::uint64), idxFile) ) 
		{
			return "д������ʧ��";
		}
	}

	return NULL;
}

const char* ExistFS::GetDataSize(const char* dataFileName, mdk::uint64 dbIndex, mdk::uint32 &dataSize)
{
	ExistFile dbFile;
	if ( !dbFile.Open( dataFileName, "rb" ) ) 
	{
		return "�޷����������ļ�";
	}

	//�䳤����db�ļ���ʽ
	//���ݳ���(4)+����(1)+ֵ(?)+���ݳ���(4)+����(1)+ֵ(?)+...+���ݳ���(4)+����(1)++ֵ(?)
	if ( -1 == fseek(dbFile, dbIndex, SEEK_SET) ) 
	{
		return "������������";
	}
	if ( sizeof(mdk::uint32) != fread( &dataSize, sizeof(char), sizeof(mdk::uint32), dbFile ) )
	{
		return "��������";
	}

	return NULL;
}

}
