// IOBus.h: interface for the IOBus class.
//
//////////////////////////////////////////////////////////////////////

#ifndef IOBUS_H
#define IOBUS_H

#include <cstring>
#include <map>
#include "mdk/Lock.h"
#include "mdk/mapi.h"
#include "mdk/FixLengthInt.h"
#include "ExistType.h"

namespace Exist
{

#define MAX_KEY_SIZE 64

/*
 	���������������
	�����ڵ�һ�������ǰ�����ô˷���������������
	�ɹ�����true��ʧ�ܷ���false������ԭ����鿴Exist.log��־
 */
bool PlugIn();
class ConnectPool;
/*
	IO������
		Exist�����ݷ��ʽӿ�
		�κ������඼ֻ��һ�����ʽӿڣ�����IOBus�����нӿ���Ļ���
 */
class IOBus
{
	//////////////////////////////////////////////////////////////////////////
	//����ʱ���Ͳ���
public:
	static char s_defaultObj[1024];//Ĭ�϶������ڵ���private protected���͵�Ĭ�Ϲ��캯��
	static std::map<void*, int> s_memoryMap;
	static mdk::Mutex s_lockMemoryMap;
	static void CreateExistObj( void *pObj, DataType::DataType type );
	static void ReleaseExistObj( void *pObj );
	static DataType::DataType GetDataType( void *pData );
	//////////////////////////////////////////////////////////////////////////
	//���ӳ�
	static ConnectPool s_connectPool;

	//���߷���
	static int Hash( char *key, int keySize )//����hash
	{
		return 0;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//���ؽӿ�
public:
	void Init( const char *key, int keySize, bool protracted );
	virtual ~IOBus();
	IOBus& operator=( const IOBus &right );

	bool IsProtracted();
	char* Key();
	int KeySize();
	DataType::DataType Type();
	DataType::DataType ElementType();
	void DeleteData();
	
protected:
	IOBus();
	IOBus( const char *name, bool protracted );
	IOBus( const IOBus &container );
	void Copy( const IOBus &right );
	//Ԫ���������е�Key(�ֲ�Key)ת����Ԫ����Exist�ϵ���ʵKey(ȫ��Key)
	void ElementRealKey( const void* localKey, const int &localKeySize, char *globalkey, int &globalkeySize );
	
	
protected:
	bool			m_protracted;			//�����ǳ־û�����0�ǣ�0��Ԥ���ֶΣ��ݲ�ʵ�ֳ־û����ܣ�
	/*
		��Ϊ����ʱ�����жϵķ����ǣ���һ������ǿת��IOBus֮�󣬲鿴m_typeָ������ݡ�
		���ǣ����ų�ĳ��ָ��ָ��ĵ�ַǿת��IOBus��,m_typeָ�������������DataType����ֵ���ɺ��ԣ�
		������Ҫ���CalCheckData()�������˳�Ա�Ƿ���ͬ
	*/
	DataType::DataType	m_type;				//�������ͣ���GetElementType()��
	char				m_size;				//���ݴ�С��data��Ч��������Ч
	DataType::DataType	m_elementType;		//Ԫ�����ͣ�ͨ��GetElementType()�õ�
	DataType::DataType	m_elementKeyType;	//Ԫ��Key����
	
private:
	int				m_keySize;
	char			m_key[MAX_KEY_SIZE+1];	//����·��Ӧ���ݵ�key��value��Exist��
	unsigned int	m_hash;					//keyͨ��hash�����õ���hashid

	//////////////////////////////////////////////////////////////////////////
	//Exist::IO�ӿ�
public:
	bool CreateData();
	bool DeleteData( unsigned int hash, char *key, int keySize );
	bool QueryData();//��������
	bool SeekMsg();//����1������
	bool ReadData( void *pData, int &size );//��ȡ���ݣ��ɹ�����true,pData���Ȳ���(size�ط�����Ҫ�ĳ���)�������ӶϿ�����false
	//updateType ��������= += -= *= /=
	bool WriteData( void *pData, int size, UpdateType::UpdateType updateType );

	bool ReadData( int index, void *pData, int &size );
	bool WriteData( int index, void *pData, int size );
};

}

#endif // ifndef IOBUS_H
