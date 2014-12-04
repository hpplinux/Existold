#ifndef EXISTFS_H
#define EXISTFS_H

#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include "../include/Exist/frame/ExistType.h"

namespace exist
{
typedef struct VALUE
{
	DATA_KEY			key;			//key
	bool				protracted;		//�����ǳ־û�����0�ǣ�0��Ԥ���ֶΣ��ݲ�ʵ�ֳ־û����ܣ�
	bool				delMark;		//ɾ�����
	char				*pData;			//����
	unsigned int		size;			//����
	//////////////////////////////////////////////////////////////////////////
	//�־û����
	bool				idxAble;		//��������idxPos��Ч
	mdk::uint64			idxPos;			//������ʼλ��
	VALUE				*pParent;		//������
}VALUE;

/*
	idx�ļ���ʽ
		key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)+...+key����(2)+����(1)+key(?)+db�ļ��п�ʼλ��(4)
	
	��������db�ļ���ʽ
		ֵ(n)+ֵ(n)+...+ֵ(n)

	�䳤����db�ļ���ʽ
		���ݳ���(4)+ֵ(?)+���ݳ���(4)+ֵ(?)+...+���ݳ���(4)++ֵ(?)
*/
class ExistFS
{
public:
	ExistFS();
	~ExistFS(void);
	void SetRootDir( const char *rootDir );
	const char* CreateTable( void *createParam, int createParamSize, unsigned char *path, int size );//������
	const char* CreateData( VALUE *pValue );
	const char* MoveFristTable();//�ƶ�����һ����
	const char* GetTable( unsigned char *createData, short &size );//ȡ�ñ����
	const char* ReadTable(exist::VALUE *pTable, std::vector<exist::VALUE*> &data );//��ȡ������
	const char* WriteValue( VALUE *pValue );

public:
	class ExistFile
	{
	public:
		ExistFile();
		virtual ~ExistFile();
		bool IsOpend();
		bool Open( const char *filename, const char *mode );
		void Close();
		operator FILE*();
		static bool CreateDir( const char *strDir );
		static bool CreateFile( const char *strFile );

	private:
		FILE		*m_pFile;
	};

private:
	void StringReplace(std::string &strBase, const char *src, const char *des);
	void StringTrim(std::string &strBase, const char c);//ѹ���������ظ��ַ���1��
	void GetOnwerDir( VALUE *pValue, std::string &dir );//ȡ�����������ļ�
	const char* SetDBIndex( const char *idxFileName, VALUE *pValue, mdk::uint64 newDBIndex, 
		mdk::uint64 oldDBIndex, mdk::uint8 newDelMark, mdk::uint8 oldDelMark );//����������Ϣ
	const char* GetDBIndex( const char *idxFileName, VALUE *pValue, mdk::uint64 &dbPos, mdk::uint8 &delMark );//ȡ��������Ϣ
	const char* GetDataSize(const char* dataFileName, mdk::uint64 dbIndex, mdk::uint32 &dataSize);//ȡ�����ݳ���

private:
	std::string m_rootDir;
	ExistFile m_tableFile;
};

}

#endif //EXISTFS_H