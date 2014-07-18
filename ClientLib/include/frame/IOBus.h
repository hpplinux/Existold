// IOBus.h: interface for the IOBus class.
//
//////////////////////////////////////////////////////////////////////

#ifndef IOBUS_H
#define IOBUS_H

#include <cstring>
#include <map>

namespace Exist
{

#define MAX_KEY_SIZE 64

namespace DataType
{
	enum DataType
	{
		uninit = 0,
		data = 1,
		lock = 2,
		map = 3,
		vector = 4,
		value = 5,
		Int8 = 6
	};
}

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
	static void AddMemoryMap( void *pObj, DataType::DataType type );
	static DataType::DataType GetDataType( void *pData );

	//���߷���
	static int Hash( char *key, int keySize )//����hash
	{
		return 0;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//���ؽӿ�
public:
	void Init( const char *key, int keySize );
	virtual ~IOBus();
	IOBus& operator=( const IOBus &right );

	char* Key();
	int KeySize();
	DataType::DataType Type();
	DataType::DataType ElementType();
	
protected:
	IOBus();
	IOBus( const char *name );
	IOBus( const IOBus &container );
	void Release();
	void Copy( const IOBus &right );
	//Ԫ���������е�Key(�ֲ�Key)ת����Ԫ����Exist�ϵ���ʵKey(ȫ��Key)
	void ElementRealKey( const void* localKey, const int &localKeySize, char *globalkey, int &globalkeySize );
	
	
protected:
	/*
		��Ϊ����ʱ�����жϵķ����ǣ���һ������ǿת��IOBus֮�󣬲鿴m_typeָ������ݡ�
		���ǣ����ų�ĳ��ָ��ָ��ĵ�ַǿת��IOBus��,m_typeָ�������������DataType����ֵ���ɺ��ԣ�
		������Ҫ���CalCheckData()�������˳�Ա�Ƿ���ͬ
	*/
	DataType::DataType m_type;			//�������ͣ���GetElementType()��
	DataType::DataType m_elementType;	//Ԫ�����ͣ�ͨ��GetElementType()�õ�
	DataType::DataType m_elementKeyType;		//Ԫ��Key����
	
private:
	/*
		����·��Ӧ���ݵĽ��������ü����������ڲ��ٷ���ʱ����֪ͨExist�ͷű���·��Ӧ����
		Exist�Ƿ����Ľ������ͷţ���Ҫ�����������Ƿ������˱����ݣ������Exist�Լ��Ĳ����ˣ�����Ҫ����·����
	*/
	int				*m_refCount;
	int				m_keySize;
	char			m_key[MAX_KEY_SIZE+1];	//����·��Ӧ���ݵ�key��value��Exist��
	unsigned int	m_hash;					//keyͨ��hash�����õ���hashid


	//////////////////////////////////////////////////////////////////////////
	//Exist::IO�ӿ�
public:
	bool CreateData( unsigned int hash, char *key, int keySize );
	bool ReadData( int index, void *pData, int &size );
	bool WriteData( int index, void *pData, int size );
	bool ReadData( unsigned int hash, char *key, int keySize, void *pData, int &size );
	bool WriteData( unsigned int hash, char *key, int keySize, void *pData, int size );


};

}

#endif // ifndef IOBUS_H
