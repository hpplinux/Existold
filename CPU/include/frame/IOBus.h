// IOBus.h: interface for the IOBus class.
//
//////////////////////////////////////////////////////////////////////

#ifndef IOBUS_H
#define IOBUS_H

#include <cstring>
#include <map>
#include "mdk/Lock.h"

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
		element = 3,
		map = 4,
		vector = 5,
		value = 6,
		Int8 = 7
	};
}

/*
 	将外存条插入主板
	必须在第一访问外存前，调用此方法，将外存条插好
	成功返回true，失败返回false，错误原因请查看Exist.log日志
 */
bool PlugIn();
class ConnectPool;
/*
	IO总线类
		Exist的数据访问接口
		任何数据类都只是一个访问接口，所以IOBus是所有接口类的基类
 */
class IOBus
{
	//////////////////////////////////////////////////////////////////////////
	//运行时类型操作
public:
	static char s_defaultObj[1024];//默认对象，用于调用private protected类型的默认构造函数
	static std::map<void*, int> s_memoryMap;
	static mdk::Mutex s_lockMemoryMap;
	static void CreateExistObj( void *pObj, DataType::DataType type );
	static void ReleaseExistObj( void *pObj );
	static DataType::DataType GetDataType( void *pData );
	//////////////////////////////////////////////////////////////////////////
	//链接池
	static ConnectPool s_connectPool;

	//工具方法
	static int Hash( char *key, int keySize )//计算hash
	{
		return 0;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//本地接口
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
	//元素在容器中的Key(局部Key)转换成元素在Exist上的真实Key(全局Key)
	void ElementRealKey( const void* localKey, const int &localKeySize, char *globalkey, int &globalkeySize );
	
	
protected:
	/*
		因为运行时类型判断的方法是，将一个对象强转成IOBus之后，查看m_type指向的数据。
		但是，不排除某个指针指向的地址强转成IOBus后,m_type指向的数据正好是DataType定义值的巧合性，
		所以需要检查CalCheckData()计算后，与此成员是否相同
	*/
	DataType::DataType m_type;			//自身类型，给GetElementType()用
	DataType::DataType m_elementType;	//元素类型，通过GetElementType()得到
	DataType::DataType m_elementKeyType;		//元素Key类型
	
private:
	/*
		本线路对应数据的进程内引用计数，进程内不再访问时，会通知Exist释放本线路对应数据
		Exist是否会真的将数据释放，还要看其它进程是否引用了本数据，这就是Exist自己的策略了，不需要本线路关心
	*/
	int				*m_refCount;
	int				m_keySize;
	char			m_key[MAX_KEY_SIZE+1];	//本线路对应数据的key，value在Exist上
	unsigned int	m_hash;					//key通过hash计算后得到的hashid


	//////////////////////////////////////////////////////////////////////////
	//Exist::IO接口
public:
	bool CreateData( unsigned int hash, char *key, int keySize );
	bool DeleteData( unsigned int hash, char *key, int keySize );
	bool ReadData( int index, void *pData, int &size );
	bool WriteData( int index, void *pData, int size );
	bool ReadData( unsigned int hash, char *key, int keySize, void *pData, int &size );
	bool WriteData( unsigned int hash, char *key, int keySize, void *pData, int size );


};

}

#endif // ifndef IOBUS_H
