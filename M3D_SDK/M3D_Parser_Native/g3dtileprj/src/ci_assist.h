#pragma once

#include "../include/g3dtilerecord.h"
#include "../include/g3dtilecontent.h"
#include "../include/g3dtilegeometry.h"

#include "gbytearray.h"
#include "cgstring.h"
#include <cstring>
#include "rapidjson/document.h"

using namespace std;

//================================================================================
/// 安全删除指针宏定义
//================================================================================
#ifndef SAFE_DELETE_PTR
#define SAFE_DELETE_PTR(p) { if(p) { delete (p); (p)=NULL; } }
#endif

//================================================================================
/// 安全删除数组宏定义
//================================================================================
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; } }
#endif

//================================================================================
/// 绝对值宏定义
//================================================================================
#define MY_ABS(a) ((a<0)?(-(a)):a)

//================================================================================
/// 最大值宏定义
//================================================================================
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

//================================================================================
/// 最小值宏定义
//================================================================================
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

//================================================================================
/// 双精度浮点数比较是否相等
///
/// @param [in] a 第一个双精度浮点数
/// @param [in] b 第二个双精度浮点数
/// @return 是否相等的判断结果
//================================================================================
inline bool IsEqual(double& a, double& b) { return fabs(a - b) < 1E-6; }

//================================================================================
/// 单精度浮点数比较是否相等
///
/// @param [in] a 第一个单精度浮点数
/// @param [in] b 第二个单精度浮点数
/// @return 是否相等的判断结果
//================================================================================
inline bool IsEqual(float& a, float& b) { return fabs(a - b) < 1E-6; }

//================================================================================
/// 不区分大小写的字符串比较
///
/// @param [in] str1 第一个字符串
/// @param [in] str2 第二个字符串
/// @return 比较结果（与strcmp类似）
//================================================================================
inline int StrICmp(const char* str1, const char* str2)
{
#ifdef _WIN32
    return stricmp(str1, str2);
#else
    return strcasecmp(str1, str2);
#endif // _WIN32
}

//================================================================================
/// 不区分大小写的字符串比较（限定长度）
///
/// @param [in] str1 第一个字符串
/// @param [in] str2 第二个字符串
/// @param [in] count 比较的最大字符数
/// @return 比较结果（与strncmp类似）
//================================================================================
inline int StrNICmp(const char* str1, const char* str2, size_t count)
{
#ifdef _WIN32
    return strnicmp(str1, str2, count);
#else
    return strncasecmp(str1, str2, count);
#endif // _WIN32
}

//================================================================================
/// 从JSON值转换为布尔值
///
/// @param [in] jsVal JSON值
/// @return 转换后的布尔值
//================================================================================
bool JsonValueToBool(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为无符号字符值
///
/// @param [in] jsVal JSON值
/// @return 转换后的无符号字符值
//================================================================================
unsigned char JsonValueToUChar(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为字符值
///
/// @param [in] jsVal JSON值
/// @return 转换后的字符值
//================================================================================
char JsonValueToChar(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为无符号短整型值
///
/// @param [in] jsVal JSON值
/// @return 转换后的无符号短整型值
//================================================================================
unsigned short JsonValueToUShort(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为短整型值
///
/// @param [in] jsVal JSON值
/// @return 转换后的短整型值
//================================================================================
short JsonValueToShort(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为无符号整型值
///
/// @param [in] jsVal JSON值
/// @return 转换后的无符号整型值
//================================================================================
unsigned int JsonValueToUInt(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为整型值
///
/// @param [in] jsVal JSON值
/// @return 转换后的整型值
//================================================================================
int JsonValueToInt(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为无符号长整型值
///
/// @param [in] jsVal JSON值
/// @return 转换后的无符号长整型值
//================================================================================
unsigned long long JsonValueToUInt64(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为长整型值
///
/// @param [in] jsVal JSON值
/// @return 转换后的长整型值
//================================================================================
long long JsonValueToInt64(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为单精度浮点值
///
/// @param [in] jsVal JSON值
/// @return 转换后的单精度浮点值
//================================================================================
float JsonValueToFloat(const rapidjson::Value& jsVal);

//================================================================================
/// 从JSON值转换为双精度浮点值
///
/// @param [in] jsVal JSON值
/// @return 转换后的双精度浮点值
//================================================================================
double JsonValueToDouble(const rapidjson::Value& jsVal);

//================================================================================
/// 对字符串进行JSON转义处理
///
/// @param [in] s 原始字符串
/// @return 转义后的字符串
//================================================================================
std::string EscapeJson(const CGString& s);

//================================================================================
/// 将字符串转换为JSON值
///
/// @param [in] str 字符串
/// @param [in] a JSON分配器
/// @return JSON值
//================================================================================
rapidjson::Value ToStringValue(const std::string& str, rapidjson::Document::AllocatorType& a);

//================================================================================
/// 将字符串转换为JSON值
///
/// @param [in] str 字符串
/// @param [in] a JSON分配器
/// @return JSON值
//================================================================================
rapidjson::Value ToStringValue(const CGString& str, rapidjson::Document::AllocatorType& a);

//================================================================================
/// 将字符串转换为JSON值
///
/// @param [in] str 字符串
/// @param [in] a JSON分配器
/// @return JSON值
//================================================================================
rapidjson::Value ToStringValue(CGString& str, rapidjson::Document::AllocatorType& a);

//================================================================================
/// 将GUID转换为字符串
///
/// @param [in] guid GUID对象
/// @param [out] szGuid 输出字符串缓冲区
/// @return 操作结果
//================================================================================
gisLONG GUID2String(GUID guid, char* szGuid);

//================================================================================
/// 将字符串转换为GUID
///
/// @param [in] szGuid 输入字符串
/// @param [out] guid 输出GUID对象
/// @return 操作结果
//================================================================================
gisLONG String2GUID(const char* szGuid, GUID& guid);

//================================================================================
/// 生成新的GUID并转换为字符串
///
/// @param [out] szGuid 输出字符串缓冲区
/// @return 操作结果
//================================================================================
gisLONG GetNewGuidToString(char* szGuid);

//================================================================================
/// 将记录项值转换为JSON值
///
/// @param [in] recordValue 记录项值
/// @return JSON值
//================================================================================
rapidjson::Value ToJsonValue(const MapGIS::Tile::RecordItemValue& recordValue);

//================================================================================
/// 从JSON值转换为记录项值
///
/// @param [in] jsVal JSON值
/// @param [in] type 字段类型
/// @return 记录项值
//================================================================================
MapGIS::Tile::RecordItemValue FromJsonValue(const rapidjson::Value& jsVal, MapGIS::Tile::FieldType type);

//================================================================================
/// 更新实例ID
///
/// @param [in,out] pInstances 模型实例向量
/// @param [in,out] ids ID向量
//================================================================================
void UpdateId(vector<MapGIS::Tile::ModelInstance>* pInstances, vector<gisINT64>& ids);

//================================================================================
/// 更新模型ID
///
/// @param [in,out] pModel 3D模型对象
/// @param [in,out] ids ID向量
//================================================================================
void UpdateId(MapGIS::Tile::G3DModel* pModel, vector<gisINT64>& ids);

//================================================================================
/// 解码固定32位整数（小端序）
///
/// @param [in] ptr 数据指针
/// @return 解码后的32位整数
//================================================================================
inline uint32_t DecodeFixed32(const char* ptr)
{
    const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);

    // Recent clang and gcc optimize this to a single mov / ldr instruction.
    return (static_cast<uint32_t>(buffer[0])) |
        (static_cast<uint32_t>(buffer[1]) << 8) |
        (static_cast<uint32_t>(buffer[2]) << 16) |
        (static_cast<uint32_t>(buffer[3]) << 24);
}

//================================================================================
/// 计算属性数据的哈希码
/// 算法来源：baseessencallprj中的murmur hash变种
///
/// @param [in] data 数据指针
/// @param [in] n 数据长度
/// @param [in] seed 种子值
/// @return 计算得到的哈希码
//================================================================================
inline uint32_t CalcAttHash(const char* data, size_t n, uint32_t seed)
{
    // Similar to murmur hash
    const uint32_t m = 0xc6a4a793;
    const uint32_t r = 24;
    const char* limit = data + n;
    uint32_t h = seed ^ (n * m);

    // Pick up four bytes at a time
    while (data + 4 <= limit) {
        uint32_t w = DecodeFixed32(data);
        data += 4;
        h += w;
        h *= m;
        h ^= (h >> 16);
    }

    // Pick up remaining bytes
    switch (limit - data) {
    case 3:
        h += data[2] << 16;
        // fall through
    case 2:
        h += data[1] << 8;
        // fall through
    case 1:
        h += data[0];
        h *= m;
        h ^= (h >> r);
        break;
    }
    return h;
}

//================================================================================
/// 将CGString类型字符串转换为字段类型枚举
///
/// @param [in] type CGString类型的字段类型字符串
/// @return 对应的字段类型枚举值
//================================================================================
inline MapGIS::Tile::FieldType  CGStringToFieldType(CGString type)
{
	if (strcmp(type.CStr(), "bool") == 0)
		return MapGIS::Tile::FieldType::BoolType;
	else if (strcmp(type.CStr(), "int8") == 0)
		return MapGIS::Tile::FieldType::Int8Type;
	else if (strcmp(type.CStr(), "uint8") == 0)
		return MapGIS::Tile::FieldType::Uint8Type;
	else if (strcmp(type.CStr(), "int16") == 0)
		return MapGIS::Tile::FieldType::Int16Type;
	else if (strcmp(type.CStr(), "uint16") == 0)
		return MapGIS::Tile::FieldType::Uint16Type;
	else if (strcmp(type.CStr(), "int32") == 0)
		return MapGIS::Tile::FieldType::Int32Type;
	else if (strcmp(type.CStr(), "uint32") == 0)
		return MapGIS::Tile::FieldType::Uint32Type;
	else if (strcmp(type.CStr(), "int64") == 0)
		return MapGIS::Tile::FieldType::Int64Type;
	else if (strcmp(type.CStr(), "uint64") == 0)
		return MapGIS::Tile::FieldType::Uint64Type;
	else if (strcmp(type.CStr(), "float") == 0)
		return MapGIS::Tile::FieldType::FloatType;
	else if (strcmp(type.CStr(), "double") == 0)
		return MapGIS::Tile::FieldType::DoubleType;
	else if (strcmp(type.CStr(), "text") == 0)
		return MapGIS::Tile::FieldType::TextType;
	else if (strcmp(type.CStr(), "datetime") == 0)
		return MapGIS::Tile::FieldType::DateTimeType;
	else
		return MapGIS::Tile::FieldType::Undefined;
}
//================================================================================
/// 将字段类型枚举转换为CGString类型字符串
///
/// @param [in] type 字段类型枚举值
/// @return 对应的CGString类型的字段类型字符串
//================================================================================

inline CGString FieldTypeToCGString(MapGIS::Tile::FieldType type)
{
	switch (type)
	{
	case MapGIS::Tile::FieldType::BoolType:
		return "bool";
	case MapGIS::Tile::FieldType::Int8Type:
		return "int8";
	case MapGIS::Tile::FieldType::Uint8Type:
		return "uint8";
	case MapGIS::Tile::FieldType::Int16Type:
		return "int16";
	case MapGIS::Tile::FieldType::Uint16Type:
		return "uint16";
	case MapGIS::Tile::FieldType::Int32Type:
		return "int32";
	case MapGIS::Tile::FieldType::Uint32Type:
		return "uint32";
	case MapGIS::Tile::FieldType::Int64Type:
		return "int64";
	case MapGIS::Tile::FieldType::Uint64Type:
		return "uint64";
	case MapGIS::Tile::FieldType::FloatType:
		return "float";
	case MapGIS::Tile::FieldType::DoubleType:
		return "double";
	case MapGIS::Tile::FieldType::TextType:
		return "text";
	case MapGIS::Tile::FieldType::DateTimeType:
		return "datetime";
	default:
		break;
	}
	return "text";
}
//================================================================================
/// 计算属性字段ID
///
/// @param [in] name 字段名称
/// @return 计算得到的字段ID（哈希码）
//================================================================================
inline unsigned int CalcAttFieldID(CGString name)
{
	name.Convert(CGString::GB18030);
	unsigned int code = CalcAttHash(name.CStr(), name.GetLength(), 0);
	return code;
}

//================================================================================
/// 计算属性图层ID
///
/// @param [in] dataSource 数据源路径
/// @param [in] layerName 图层名称
/// @return 计算得到的图层ID（哈希码）
//================================================================================
inline unsigned int CalcAttLayerID(CGString dataSource, CGString layerName)
{
	dataSource.Convert(CGString::GB18030);
	layerName.Convert(CGString::GB18030);
	CGString name = dataSource + layerName;
	unsigned int code = CalcAttHash(name.CStr(), name.GetLength(), 0);
	return code;
}

//================================================================================
/// 从字节数组中读取指定范围的数据到另一个字节数组
///
/// @param [in]  pnts   源字节数组
/// @param [in]  index  起始索引
/// @param [in]  length 读取长度
/// @param [out] out    输出字节数组
//================================================================================
inline void ReadByteArrayToByteArray(const CGByteArray& pnts, int index, int length, CGByteArray& out)
{
	out.clear();
	if (index + length <= pnts.length())
		out.append(&pnts.data()[index], length);
}

//================================================================================
/// 从字节数组中读取指定范围的数据转换为字符串
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     起始索引
/// @param [in] length    读取长度
/// @return 转换得到的字符串
//================================================================================
inline std::string ReadByteArrayToString(const CGByteArray& byteArray, int index, int length)
{
	std::string str = "";
	str.append(&byteArray.data()[index], length);
	return str;
}

//================================================================================
/// 从字符串缓冲区中读取指定范围的数据转换为字符串
///
/// @param [in] buffer 源字符串缓冲区
/// @param [in] index  起始索引
/// @param [in] length 读取长度
/// @return 转换得到的字符串
//================================================================================
inline string ReadBufferToString(const string& buffer, int index, int length)
{
	string str = "";
	for (int i = index; i < index + length; i++)
	{
		str += (char)buffer[i];
	}
	return str;
}

//================================================================================
/// 从字节数组中读取布尔值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的布尔值
//================================================================================
inline bool ReadByteArrayToBool(const CGByteArray& byteArray, int index)
{
	char cValue;
	memcpy(&cValue, &byteArray.data()[index], 1);
	if (cValue == 1)
		return true;
	else
		return false;
}

//================================================================================
/// 从字节数组中读取字符值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的字符值
//================================================================================
inline char ReadByteArrayToChar(const CGByteArray& byteArray, int index)
{
	char cValue;
	memcpy(&cValue, &byteArray.data()[index], 1);
	return cValue;
}

//================================================================================
/// 从字节数组中读取无符号字符值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的无符号字符值
//================================================================================
inline unsigned char ReadByteArrayToUnsignedChar(const CGByteArray& byteArray, int index)
{
	unsigned char cValue;
	memcpy(&cValue, &byteArray.data()[index], 1);
	return cValue;
}

//================================================================================
/// 从字节数组中读取16位短整型值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的16位短整型值
//================================================================================
inline short ReadByteArrayToInt16(const CGByteArray& byteArray, int index)
{
	short value;
	memcpy(&value, &byteArray.data()[index], 2);
	return value;
}

//================================================================================
/// 从字节数组中读取16位无符号短整型值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的16位无符号短整型值
//================================================================================
inline unsigned short ReadByteArrayToUnsignedInt16(const CGByteArray& byteArray, int index)
{
	unsigned short value;
	memcpy(&value, &byteArray.data()[index], 2);
	return value;
}

//================================================================================
/// 从字节数组中读取32位整型值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的32位整型值
//================================================================================
inline int ReadByteArrayToInt32(const CGByteArray& byteArray, int index)
{
	int value;
	memcpy(&value, &byteArray.data()[index], 4);
	return value;
}

//================================================================================
/// 从字符串中读取32位整型值
///
/// @param [in] pnts  源字符串
/// @param [in] index 数据索引
/// @return 读取的32位整型值
//================================================================================
inline int ReadBufferToInt32(const string& pnts, int index)
{
	int num;
	memcpy(&num, &pnts.data()[index], 4);
	return num;
}

//================================================================================
/// 从字节数组中读取32位无符号整型值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的32位无符号整型值
//================================================================================
inline unsigned int ReadByteArrayToUnsignedInt32(const CGByteArray& byteArray, int index)
{
	unsigned int value;
	memcpy(&value, &byteArray.data()[index], 4);
	return value;
}

//================================================================================
/// 从字节数组中读取64位长整型值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的64位长整型值
//================================================================================
inline gisINT64 ReadByteArrayToInt64(const CGByteArray& byteArray, int index)
{
	gisINT64 value;
	memcpy(&value, &byteArray.data()[index], 8);
	return value;
}

//================================================================================
/// 从字节数组中读取64位无符号长整型值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的64位无符号长整型值
//================================================================================
inline gisUINT64 ReadByteArrayToUnsignedInt64(const CGByteArray& byteArray, int index)
{
	gisUINT64 value;
	memcpy(&value, &byteArray.data()[index], 8);
	return value;
}

//================================================================================
/// 从字节数组中读取单精度浮点值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的单精度浮点值
//================================================================================
inline float ReadByteArrayToFloat(const CGByteArray& byteArray, int index)
{
	float value;
	memcpy(&value, &byteArray.data()[index], 4);
	return value;
}

//================================================================================
/// 从字节数组中读取双精度浮点值
///
/// @param [in] byteArray 源字节数组
/// @param [in] index     数据索引
/// @return 读取的双精度浮点值
//================================================================================
inline double ReadByteArrayToDouble(const CGByteArray& byteArray, int index)
{
	double value;
	memcpy(&value, &byteArray.data()[index], 8);
	return value;
}

//================================================================================
/// 将整型值转换为字节数组
///
/// @param [out] byteArray 目标字节数组
/// @param [in]  val       源整型值
//================================================================================
inline void IntToByteArray(CGByteArray& byteArray, int& val)
{
	char* p = (char*)&val;
	byteArray.append(p, 4);
}

//================================================================================
/// 将无符号整型值转换为字节数组
///
/// @param [out] byteArray 目标字节数组
/// @param [in]  val       源无符号整型值
//================================================================================
inline void UnsignedIntToByteArray(CGByteArray& byteArray, unsigned int& val)
{
	char* p = (char*)&val;
	byteArray.append(p, 4);
}

//================================================================================
/// 将单精度浮点值转换为字节数组
///
/// @param [out] byteArray 目标字节数组
/// @param [in]  val       源单精度浮点值
//================================================================================
inline void FloatToByteArray(CGByteArray& byteArray, float& val)
{
	char* p = (char*)&val;
	byteArray.append(p, 4);
}

//================================================================================
/// 将字符值转换为字节数组
///
/// @param [out] byteArray 目标字节数组
/// @param [in]  val       源字符值
//================================================================================
inline void CharToByteArray(CGByteArray& byteArray, unsigned char& val)
{
	char* p = (char*)&val;
	byteArray.append(p, 1);
}

//================================================================================
/// 将字符串转换为字节数组
///
/// @param [out] byteArray 目标字节数组
/// @param [in]  sz        源字符串
//================================================================================
inline void StringToByteArray(CGByteArray& byteArray, string& sz)
{
	byteArray.append(sz.c_str(), sz.length());
}

//================================================================================
/// 从字符数组中读取指定范围的数据到字节数组
///
/// @param [in]  arrayValue 源字符数组
/// @param [in]  index      起始索引
/// @param [in]  length     读取长度
/// @param [out] out        输出字节数组
//================================================================================
inline void ReadArrayToByteArray(const char* arrayValue, int index, int length, CGByteArray& out)
{
	out.clear();
	out.append(&arrayValue[index], length);
}

//================================================================================
/// 从字符数组中读取指定范围的数据转换为字符串
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      起始索引
/// @param [in] length     读取长度
/// @return 转换得到的字符串
//================================================================================
inline std::string ReadArrayToString(const char* arrayValue, int index, int length)
{
	std::string str = "";
	str.append(&arrayValue[index], length);
	return str;
}

//================================================================================
/// 从字符数组中读取布尔值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的布尔值
//================================================================================
inline bool ReadArrayToBool(const char* arrayValue, int index)
{
	char cValue;
	memcpy(&cValue, &arrayValue[index], 1);
	if (cValue == 1)
		return true;
	else
		return false;
}

//================================================================================
/// 从字符数组中读取字符值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的字符值
//================================================================================
inline char ReadArrayToChar(const char* arrayValue, int index)
{
	char cValue;
	memcpy(&cValue, &arrayValue[index], 1);
	return cValue;
}

//================================================================================
/// 从字符数组中读取无符号字符值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的无符号字符值
//================================================================================
inline unsigned char ReadArrayToUnsignedChar(const char* arrayValue, int index)
{
	unsigned char cValue;
	memcpy(&cValue, &arrayValue[index], 1);
	return cValue;
}

//================================================================================
/// 从字符数组中读取16位短整型值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的16位短整型值
//================================================================================
inline short ReadArrayToInt16(const char* arrayValue, int index)
{
	short value;
	memcpy(&value, &arrayValue[index], 2);
	return value;
}

//================================================================================
/// 从字符数组中读取16位无符号短整型值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的16位无符号短整型值
//================================================================================
inline unsigned short ReadArrayToUnsignedInt16(const char* arrayValue, int index)
{
	unsigned short value;
	memcpy(&value, &arrayValue[index], 2);
	return value;
}

//================================================================================
/// 从字符数组中读取32位整型值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的32位整型值
//================================================================================
inline int ReadArrayToInt32(const char* arrayValue, int index)
{
	int value;
	memcpy(&value, &arrayValue[index], 4);
	return value;
}

//================================================================================
/// 从字符数组中读取32位无符号整型值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的32位无符号整型值
//================================================================================
inline unsigned int ReadArrayToUnsignedInt32(const char* arrayValue, int index)
{
	unsigned int value;
	memcpy(&value, &arrayValue[index], 4);
	return value;
}

//================================================================================
/// 从字符数组中读取64位长整型值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的64位长整型值
//================================================================================
inline gisINT64 ReadArrayToInt64(const char* arrayValue, int index)
{
	gisINT64 value;
	memcpy(&value, &arrayValue[index], 8);
	return value;
}

//================================================================================
/// 从字符数组中读取64位无符号长整型值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的64位无符号长整型值
//================================================================================
inline gisUINT64 ReadArrayToUnsignedInt64(const char* arrayValue, int index)
{
	gisUINT64 value;
	memcpy(&value, &arrayValue[index], 8);
	return value;
}

//================================================================================
/// 从字符数组中读取单精度浮点值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的单精度浮点值
//================================================================================
inline float ReadArrayToFloat(const char* arrayValue, int index)
{
	float value;
	memcpy(&value, &arrayValue[index], 4);
	return value;
}

//================================================================================
/// 从字符数组中读取双精度浮点值
///
/// @param [in] arrayValue 源字符数组
/// @param [in] index      数据索引
/// @return 读取的双精度浮点值
//================================================================================
inline double ReadArrayToDouble(const char* arrayValue, int index)
{
	double value;
	memcpy(&value, &arrayValue[index], 8);
	return value;
}