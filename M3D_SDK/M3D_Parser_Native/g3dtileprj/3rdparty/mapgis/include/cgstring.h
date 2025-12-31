#pragma once
#include <string>
#include <vector>
#include <set>
#include "bastypes.h"
using namespace std;


class CGString;
typedef std::vector<CGString> CGStringArray;
typedef std::set<CGString> CGStringSet;

class  CGString
{
public:
	// @brief 编码方式
	enum EncodeType
	{
		GB18030 = 0,
		UTF8 = 1,
	};

	// @brief 字符串编码比较方式
	enum EncodeCompareType
	{
		OnlyCompareStringBuffer = 0,
		EncodeSameThenCompare = 1,
	};
public:
	CGString();
	~CGString();

	CGString(const CGString& str);


	// @brief 构造函数，默认为GB18030编码
	CGString(const std::string& str);
	CGString(const std::string& str, EncodeType type);

	CGString(const char* szstr);
	CGString(const char* szstr, int nSize);

	CGString(const char* szstr, EncodeType type);
	CGString(const char* szstr, int nSize, EncodeType type);

	// @brief 构造函数，默认为GB18030编码
	explicit CGString(char c, unsigned int nRepeat = 1);

	CGString(CGString&& other);

	/**
	* @brief 追加字符串
	* @param szStr 要追加的字符串,
	* @param nSize 要追加的字节个数
	*/
	CGString& Append(const char* szStr, unsigned int nSize);
	CGString& Append(const char* szStr);
	CGString& Append(const std::string& szStr);
	CGString& Append(char c);


	/**
	* @brief 获得C字符串
	*/
	const char * CStr() const;

	inline const char * GetString() const
	{
		return CStr();
	}

	inline const unsigned char* ConstCStr() const
	{
		return (unsigned char*)CStr();
	}
	// @brief 获得C++字符串
	const std::string& StdString() const;

	// @brief 设置字符串内容（编码信息不变）
	void SetString(const char* str);
	void SetString(const std::string& str);

	// @brief 设置字符串内容和编码信息
	void SetString(const char* str, EncodeType encodeType);
	void SetString(const std::string& str, EncodeType encodeType);

	/**
	* @brief 获取指定索引的字节值
	* @param index 指定的索引
	* @return 返回指定索引的字节值。
	*/
	char GetAt(unsigned int index) const;

	/**
	* @brief 设置指定索引的字节值
	* @param index 指定的索引
	* @param c 设置的字节值
	*/
	void SetAt(unsigned int index, char c);

	/**
	* @brief 获取指定索引的字节。
	* @param  index 指定的索引
	* @return 返回指定索引的字节
	*/
	char operator[](unsigned int index) const;

	/**
	* @brief 获取指定索引的字节的引用
	* @param index 指定的索引
	* @return 返回指定索引的字节的引用
	*/
	char& operator[](unsigned int index);

	/**
	* @brief 保留指定字节个数的内存空间
	* @param nCapacity 可容纳的字节个数
	* @remarks 如果原有空间不够, 内存空间会增加。
	* 如果原有空间比指定的大, 也不会释放空间。
	*/
	void SetCapacity(unsigned int nCapacity);

	// @brief 获取目前所有空间(包括已经使用的和保留的)所占用的字节个数
	unsigned int GetCapacity() const;

	// @brief 获取字符串长度,字节数
	unsigned int GetLength() const;

	/**
	* @brief 获取该字符串中字符个数。
	* @param ignoreErrorCharacter 是否忽略错误字符
	* @remarks 如果忽略了错误字符则不符合该字符串编码规则的乱码不参与统计，否则乱码部分每个字节被统计为一个字符。
	*/
	unsigned int GetCharacterNum(bool ignoreErrorCharacter = false) const;

	/**
	* @brief 获取该字符串的每个字符到chars中。
	* @param chars 用于接收每个字符的字符串数组
	* @param ignoreErrorCharacter 是否忽略错误字符
	* @return 字符的个数
	* @remarks 考虑了一个字符由多个字节组成的情况。
	* 如果忽略了错误字符则不符合该字符串编码规则的乱码不参与统计，否则乱码部分每个字节被统计为一个字符。
	*/
	unsigned int GetCharacters(CGStringArray& chars, bool ignoreErrorCharacter = false) const;
	CGStringArray GetCharacters(bool ignoreErrorCharacter = false) const;

	/**
	* @brief 获取指定索引的字符。
	* @param index 索引
	* @param ignoreErrorCharacter 是否忽略错误字符
	* @return 字符
	* @remarks 考虑了一个字符由多个字节组成的情况。
	*          如果忽略了错误字符则不符合该字符串编码规则的乱码不参与统计，否则乱码部分每个字节被统计为一个字符。
	*/
	CGString GetCharacter(unsigned int index, bool ignoreErrorCharacter = false) const;

	/**
	* @brief 获取该字符串中Unicode字符个数，同GetCharacterNum函数。
	* @param ignoreErrorCharacter 是否忽略错误字符
	* @remarks 如果忽略了错误字符则不符合该字符串编码规则的乱码不参与统计，否则乱码部分每个字节被统计为一个字符。
	*/
	unsigned int GetUnicodeNum(bool ignoreErrorCharacter = false) const;

	/**
	* @brief 获得指定索引的字符对应的unicode code point(unicode 代码点)。
	* @param index 字符索引
	* @param ignoreErrorCharacter 是否忽略错误字符
	* @return unicode 代码点
	* @remarks 考虑了一个字符由多个字节组成的情况。
	*          如果忽略了错误字符则不符合该字符串编码规则的乱码不参与统计，否则乱码部分每个字节被统计为一个字符。
	*/
	uint32_t GetUnicode(unsigned int index, bool ignoreErrorCharacter = false) const;

	// @brief 把字符串内容转换为大写，不会修改非ASCII字符
	CGString& MakeUpper();

	// @brief  转大写，自身不变，返回新的字符串
	inline CGString ToUpper() const
	{
		CGString str = *this;
		return str.MakeUpper();
	}

	// @brief 把字符串内容转换为小写，不会修改非ASCII字符
	CGString& MakeLower();

	// @brief  转小写，自身不变，返回新的字符串
	inline CGString ToLower() const
	{
		CGString str = *this;
		return str.MakeLower();
	}

	/**
	* @brief 把字符串逆转
	* @remarks 支持中间带有中文(或者由多个字节组成的字符)转换
	*/
	CGString& MakeReverse();

	// @brief  字符串逆转，自身不变，返回新的字符串
	inline CGString Reversed() const
	{
		CGString str = *this;
		return str.MakeReverse();
	}

	/**
	* @brief 取出字符串对象左边的指定长度的子字符串
	* @param nCount 字节个数
	* @return 返回左边指定长度的子字符串
	*/
	CGString Left(unsigned int nCount) const;

	/**
	* @brief 取出字符串对象最右边的指定长度的子字符串
	* @param nCount 字节个数
	* @return 返回右边指定长度的子字符串
	*/
	CGString Right(unsigned int nCount) const;

	/**
	* @brief 从字符串对象中取出从指定索引起的子字符串对象
	* @param nFirst 指定的中间的索引序号
	* @return 返回从中间指定的索引开始,到最后的子字符串
	*/
	CGString Mid(unsigned int nFirst) const;

	/**
	* @brief 从字符串对象中获取从指定索引起、指定长度的子字符串对象
	* @param nFirst nFirst 索引值，大于等于零
	* @param nCount 要取出的字节个数
	*/
	CGString Mid(unsigned int nFirst, unsigned int nCount) const;

	/**
	* @brief 删除从指定索引起的指定长度的字符串
	* @param startIndex 开始删除的索引值
	* @param nCount 要删除的字节个数
	* @return 返回删除字符串后的结果字符串对象的长度
	*/
	unsigned int Delete(unsigned int startIndex, unsigned int nCount = 1);

	/**
	* @brief 在指定的索引之前插入字符串
	* @param index 指定的索引
	* @param szstr 要插入的字符串
	* @return 插入后,结果字符串的长度
	*/
	unsigned int Insert(unsigned int index, const char * szstr);

	/**
	* @brief 在指定的索引之前插入若干数目的字符
	* @param index 指定的索引
	* @param c 指定的字符
	* @param nCount 字符重复的次数
	* @return 插入后,结果字符串的长度
	*/
	unsigned int Insert(unsigned int index, char c, unsigned int nCount = 1);

	/**
	* @brief 删除字符串对象中等于指定字符的字符
	* @param c 要删除的字符
	* @return 返回删除的字符个数
	*/
	unsigned int Remove(char c);

	// @brief  删除字符串对象首尾的空白字符（包括空格、制表符 tab、换行符）
	CGString& Trim();

	// @brief  删除字符串对象首尾的空白字符（包括空格、制表符 tab、换行符），自身不变，返回新的字符串
	inline CGString Trimmed()const
	{
		CGString str = *this;
		return str.Trim();
	}

	// @brief 删除字符串对象左边的空白字符（包括空格、制表符 tab、换行符）
	CGString& TrimLeft();

	/**
	* @brief 删除字符串对象左边的指定字符
	* @param c 要删除的字符
	*/
	CGString& TrimLeft(char c);

	/**
	* @brief 删除字符串中的前导字符
	* @param szStr 前导字符集合
	* @remarks 对于前导字符集合,是要删除左边的存在于字符集合中的每一个字符
	*/
	CGString& TrimLeft(const char* szStr);

	// @brief 删除字符串对象右边的空白字符（包括空格、制表符 tab、换行符）
	CGString& TrimRight();

	/**
	* @brief 删除字符串对象右边的指定字符
	* @param c 指定的字符
	*/
	CGString& TrimRight(char c);

	/**
	* @brief 删除字符串中的尾随字符
	* @param szStr 尾随字符集合
	* @remarks 对于尾随字符集合,是要删除右边的存在于字符集合中的每一个字符
	*/
	CGString& TrimRight(const char* szStr);

	// @brief 判断字符串是否为空
	bool IsEmpty() const;

	// @brief 清空字符串
	void Empty();

	/**
	* @brief 比较两个字符串的大小,区分大小写
	* @param szStr 要比较的字符串
	* @return 小于返回负数；等于返回0；大于返回正数
	*/
	int Compare(const char* szStr) const;

	/**
	* @brief 比较两个字符串的大小,区分大小写
	* @param str 要比较的字符串
	* @param compareType 字符编码不一致时比较方式
	* @return 小于返回负数；等于返回0；大于返回正数
	*/
	int Compare(const CGString& str, EncodeCompareType compareType = OnlyCompareStringBuffer) const;

	/**
	* @brief 比较两个字符串的大小,不区分大小写
	* @param szStr 要比较的字符串
	* @return 小于返回负数；等于返回0；大于返回正数
	*/
	int CompareNoCase(const char* szStr) const;

	/**
	* @brief 比较两个字符串的大小,不区分大小写
	* @param str 要比较的字符串
	* @param compareType 字符编码不一致时比较方式
	* @return 小于返回负数；等于返回0；大于返回正数
	*/
	int CompareNoCase(const CGString& str, EncodeCompareType compareType = OnlyCompareStringBuffer) const;

	/**
	* @brief 是否以指定的字符串开始
	* @param szStr 指定的字符串
	* @param ignoreCase 是否忽略大小写
	*/
	bool StartsWith(const char* szStr, bool ignoreCase = false) const;

	/**
	* @brief 是否以指定的字符串结束
	* @param szStr 指定的字符串
	* @param ignoreCase 是否忽略大小写
	*/
	bool EndsWith(const char* szStr, bool ignoreCase = false) const;

	/**
	* @brief 从指定位置开始，查找指定的字符
	* @param c 要查找的字符。
	* @param nStart 查找起始位置，对应的字符会被查找。
	* @return 返回找到的字符的索引值，从0算起。找不到返回-1
	*/
	int Find(char c, int nStart = 0) const;

	/**
	* @brief 从指定位置开始，查找完全相等的子字符串
	* @param pStr 要查找的子字符串。
	* @param nStart 查找起始位置，对应的子字符串会被查找。
	* @param ignoreCase 是否忽略大小写
	* @return 返回找到的子字符串的索引值，从0算起。找不到返回-1
	*/
	int Find(const char* pStr, int nStart = 0, bool ignoreCase = false) const;

	/**
	* @brief 从后向前查找指定的字符
	* @param c 要查找的字符。
	* @param nStart 查找起始位置，对应的子字符会被查找 -1代表从末尾查找。
	* @return 返回找到的字符的索引值，从0算起;找不到返回-1
	*/
	int ReverseFind(char c, int nStart = -1) const;

	/**
	* @brief 从后向前查找指定的字符串。
	* @param pStr 要查找的字符串。
	* @param nStart 查找起始位置，对应的子字符串会被查找 -1代表从末尾查找。
	* @param ignoreCase 是否忽略大小写
	* @return 返回找到的字符串的索引值，从0算起;找不到返回-1
	*/
	int ReverseFind(const char* pStr, int nStart = -1, bool ignoreCase = false) const;

	/**
	* @brief 在此字符串中搜索与pStrCharSet中包含的任何字符相匹配的第一个字符
	* @param pStrCharSet 指定的字符集合。
	* @return 返回找到的字符的索引，从0算起。找不到返回-1
	*/
	int FindOneOf(const char* pStrCharSet) const;

	/**
	* @brief 替换函数，把字符串对象中指定的字符替换为新的字符。
	* @param chOld 指定要被替换的字符。
	* @param chNew 新的字符。
	* @return 返回替换的次数。
	* @remarks 比较时区分大小写。
	*/
	unsigned int Replace(char chOld, char chNew);

	/**
	* @brief 替换函数，把字符串对象中指定的子字符串替换为新的字符串
	* @param pStrOld 指定要被替换的子字符串。
	* @param pStrNew 新的字符串。
	* @param ignoreCase 是否忽略大小写
	* @return 返回替换的次数。
	* @remarks 比较时区分大小写。
	*/
	unsigned int Replace(const char* pStrOld, const char* pStrNew, bool ignoreCase = false);

	// @brief 替换函数，把字符串对象中指定的子字符串替换为新的字符串		
	CGString& Replace(unsigned int startIndex, unsigned int length, const char* pStrNew);

	/**
	* @brief 格式化函数，类似于sprintf。
	* @param fmt 用来表达格式的字符串。
	*/
	CGString& Format(const char* fmt, ...);

	CGString& FormatV(const char* fmt, va_list args);

	CGString& operator=(const CGString& str);
	CGString& operator=(CGString&& str);
	CGString& operator=(const std::string& str);
	CGString& operator=(const char* szStr);

	CGString operator+(char c) const;
	CGString operator+(const char* szStr) const;

	/**
	* @brief 字符串相加。
	* @return 返回相加后的结果，编码类型同 + 左侧字符串编码类型
	* @remarks 两个字符串字符编码方式不同时，对str进行了编码转换再进行的相加，返回的字符串编码方式等于+号左侧的字符串
	*/
	CGString operator+(const CGString& str) const;
	CGString& operator+=(const CGString& str);

	CGString& operator+=(char c);
	CGString& operator+=(const char* szStr);
	CGString& operator+=(const std::string& szStr);

	friend CGString operator+(char c, const CGString &str);
	friend CGString operator+(const char* szStr, const CGString &str);

	bool operator==(const char* szStr) const;
	/**
	* @brief 字符串是否相等。
	* @remarks 基于语义的比较。等同于Compare函数EncodeSameThenCompare。其他比较运算也是如此
	*/
	bool operator==(const CGString& str) const;

	bool operator!=(const char* szStr) const;
	bool operator!=(const CGString& str) const;

	bool operator<(const CGString& str) const;
	bool operator>(const CGString& str) const;

	bool operator<=(const CGString& str) const;
	bool operator>=(const CGString& str) const;

	/**
	* @brief 把gisINT64型的数据转化为CGString型字符串
	* @param lVal 要转化的gisINT64型数据
	* @param base 进制,支持8 10 16进制
	* @return 返回转化后的字符串
	*/
	static CGString From(gisINT64 lVal, unsigned int base = 10, EncodeType resultType = UTF8);

	// @brief 把gisUINT64型的数据转化为CGString型字符串
	static CGString From(gisUINT64 lVal, unsigned int base = 10, EncodeType resultType = UTF8);

	// @brief 把int型的数据转化为CGString型字符串
	static CGString From(int nVal, unsigned int base = 10, EncodeType resultType = UTF8);

	// @brief 把unsigned int型的数据转化为CGString型字符串
	static CGString From(unsigned int nVal, unsigned int base = 10, EncodeType resultType = UTF8);

	/**
	* @brief 把double型的数据转化为CGString型字符串
	* @param dValue 数值
	* @param format 格式，可选g,G,e,E,f。
	* @param precision 精度
	* @param resultType 转换为的编码类型
	* @remarks format选择不同时的语义：
	*     e	格式为 [-]9.9e[+|-]999
	*     E	格式为 [-]9.9E[+|-]999
	*     f	格式为 [-]9.9
	*     g	使用e或f格式，以最简洁的为准
	*     G	使用E或f格式，以最简洁的为准
	*     precision语义：对于“e”、“E”和“f”格式，表示小数点后的位数。对于“g”和“G”格式，精度表示有效数字的最大数量（省略尾随零）
	*/
	static CGString From(double dValue, char format /*= 'g'*/, int precision = 6, EncodeType resultType = UTF8);

	/**
	* @brief 把double型的数据转化为CGString型字符串。采用Grisu2算法实现，保证正确且简洁。
	* @param dValue 数值
	* @param resultType 转换为的编码类型
	*/
	static CGString From(double dvalue, EncodeType resultType = UTF8);

	/* @brief 把unicode字符串转换为指定编码的CGString
	* @param pUnicode unicode字符串
	* @param size 字符串长度。当为-1时，自动转换到0为结尾处
	* @param resultType 转换为的编码类型
	* @return 返回转化后的字符串
	*/
	static CGString FromUnicode(const uint32_t* pUnicode, int size = -1, EncodeType resultType = UTF8);
	static CGString FromUnicode(const std::vector<uint32_t>& unicodes, EncodeType resultType = UTF8);
	static CGString FromUnicode(uint32_t unicode, EncodeType resultType = UTF8);

	/* @brief 把utf16字符串转换为指定编码的CGString
	* @param pUtf16 unicode16字符串
	* @param size 字符串长度。当为-1时，自动转换到0为结尾处
	* @param resultType 转换为的编码类型
	* @return 返回转化后的字符串
	*/
	static CGString FromUtf16(const uint16_t* pUtf16, int size = -1, EncodeType resultType = UTF8);
	static CGString FromUtf16(const std::vector<uint16_t>& utf16, EncodeType resultType = UTF8);

	// @brief 把字符串转换为int型数值
	int ToInt(bool* ok = NULL) const;

	// @brief 把字符串转换为无符号unsigned int型数值
	unsigned int ToUInt(bool* ok = NULL) const;

	// @brief 把字符串转换为gisINT64型数值
	gisINT64 ToLong(bool* ok = NULL) const;

	// @brief 把字符串转换为无符号gisUINT64型数值
	gisUINT64 ToULong(bool* ok = NULL) const;

	// @brief 把字符串转换为float型数值
	float ToFloat(bool *ok = NULL) const;

	// @brief 把字符串转换为double型数值	
	double ToDouble(bool *ok = NULL) const;

	/**
	* @brief 把字符串按照指定的分隔字符串分割为一个字符串数组。
	* @param strDests 生成的字符串数组。
	* @param separator 指定的分隔字符串。
	* @param bKeepEmptyString 是否保留分割后 空的字符串
	* @return 分割后字符串数组的长度。
	*/
	unsigned int Split(CGStringArray &strDests, const char* separator, bool bKeepEmptyString = false) const;
	CGStringArray Split(const char* separator, bool bKeepEmptyString = false) const;


	/**
	* @brief 把字符串按照指定的分隔字符串分割为一个字符串数组。
	* @param strDests 生成的字符串数组。
	* @param separator 指定的分隔字符串。当编码方式不一致时会将separator转为一致再分割。
	* @param bKeepEmptyString 是否保留分割后 空的字符串
	* @return 分割后字符串数组的长度。
	*/
	unsigned int Split(CGStringArray &strDests, CGString separator, bool bKeepEmptyString = false) const;
	CGStringArray Split(CGString separator, bool bKeepEmptyString = false) const;

	/**
	* @brief 把字符串按照指定的分隔字符分割为一个字符串数组。
	* @param strDests 生成的字符串数组。
	* @param separator 指定的分隔字符。
	* @param bKeepEmptyString 是否保留分割后 空的字符串
	* @return 分割后字符串数组的长度。
	*/
	unsigned int Split(CGStringArray &strDests, char separator, bool bKeepEmptyString = false) const;
	CGStringArray Split(char separator, bool bKeepEmptyString = false) const;

	/**
	* @brief 把字符串按照指定的一组分隔字符分割为一个字符串数组。
	* @param strDests 生成的字符串数组。
	* @param separator 指定的分隔字符数组。
	* @param count 分隔字符的个数。
	* @param bKeepEmptyString 是否保留分割后 空的字符串
	* @return 分割后字符串数组的长度。
	*/
	unsigned int Split(CGStringArray &strDests, char separator[], int count, bool bKeepEmptyString = false) const;
	CGStringArray Split(char separator[], int count, bool bKeepEmptyString = false) const;

	// @brief 设置字符串里面的字符的编码方式 
	void SetEncodeType(CGString::EncodeType type);

	// @brief 获取字符串里面的字符的编码方式
	EncodeType GetEncodeType() const;

	// @brief 获取错误编码位置。无错误则返回-1。
	int FindError()const;

	// @brief 字符编码方式转换,自身转换
	CGString& Convert(CGString::EncodeType type);

	// @brief 字符编码方式转换,自身不变，返回新的字符串
	inline CGString Converted(CGString::EncodeType type) const
	{
		CGString str = *this;
		return str.Convert(type);
	}

	/**
	* @brief 转换为std::wstring
	* @param ignoreErrorCharacter 是否忽略错误字符
	*/
	std::wstring ToStdWSring(bool ignoreErrorCharacter = false) const;

	/**
	* @brief 转换为std::u16string
	* @param ignoreErrorCharacter 是否忽略错误字符
	*/
	std::u16string ToStdU16String(bool ignoreErrorCharacter = false) const;

	/**
	* @brief 转换为std::u32string
	* @param ignoreErrorCharacter 是否忽略错误字符
	*/
	std::u32string ToStdU32String(bool ignoreErrorCharacter = false) const;

	/**
	* @brief 字符串中每个字符转换为对应的unicode code point(unicode 代码点)。
	* @param unicodes unicode代码点
	* @param ignoreErrorCharacter 是否忽略错误字符
	* @return 转换得到的unicode字符个数。
	* @remarks 得到的unicode codepoints不是以 0 结束的。
	*          考虑了一个字符由多个字节组成的情况。
	*          如果忽略了错误字符则不符合该字符串编码规则的乱码不参与统计，否则乱码部分每个字节被统计为一个字符。
	*/
	unsigned int ToUnicodes(std::vector<uint32_t>& unicodes, bool ignoreErrorCharacter = false) const;
	std::vector<uint32_t> ToUnicodes(bool ignoreErrorCharacter = false) const;

	// @brief 把std::wstring字符串转换为指定编码的CGString
	static CGString FromStdWString(const std::wstring &s, EncodeType resultType = UTF8);

	// @brief 把std::u16string字符串转换为指定编码的CGString
	static CGString FromStdU16String(const std::u16string &s, EncodeType resultType = UTF8);

	// @brief 把std::u32string字符串转换为指定编码的CGString
	static CGString FromStdU32String(const std::u32string &s, CGString::EncodeType resultType = UTF8);

	/**
	* @brief 把指定编码的字符串CGString
	* @param inString 输入的字符串
	* @param incode 输入字符串字节流的编码方式
	* @param resultType 转换为CGString的编码方式
	*/
	static CGString FromStdString(const std::string& inString, const std::string& inCode, CGString::EncodeType resultType = UTF8);

	// @brief GB18030字符串转为UTF8
	static std::string GB18030ToUTF8(const std::string& strGB18030);
	static std::string GB18030ToUTF8(const char* szGB18030);

	// @brief UTF8字符串转为GB18030
	static std::string UTF8ToGB18030(const std::string& strUTF8);
	static std::string UTF8ToGB18030(const char* szUTF8);

	// @brief 字符编码转换
	static std::string EncodeConvert(const std::string & inString, const std::string& inEncode, const std::string & outEncode);
private:
	// @brief 字符编码方式
	CGString::EncodeType m_encodeType;

	// @brief 字符串
	std::string m_string;
};