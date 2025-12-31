#pragma once
#include "g3dtilegaussian.h"

//================================================================================
/// ksplat 文件的主头信息结构体
/// 主头信息长度为 4096 字节，但只有少数字段被使用。未使用的空间可能是为未来的扩展预留的
//================================================================================
struct KsplatHeader {
	/** 主版本号 */
	unsigned char MajorVersion;

	/** 次版本号 */
	unsigned char MinorVersion;

	/** 分段数量 */
	int SectionCount;

	/** 点（splat）的总数 */
	int SplatCount;

	/** 压缩模式，代码支持 0、1 和 2 三种模式 */
	unsigned short CompressionMode;

	/** 球谐函数的最小值，默认为 -1.5 */
	float MinHarmonicsValue;

	/** 球谐函数的最大值，默认为 1.5 */
	float MaxHarmonicsValue;

	/** 球谐函数的度数 */
	int ShDegree;

	/** 保留字段，用于未来扩展 */
	unsigned char Reserved[4096 - sizeof(unsigned char) * 2 - sizeof(int) * 3 - sizeof(unsigned short) - sizeof(float) * 2 - sizeof(int)];
};

//================================================================================
/// 分段头信息结构体
/// 每个分段头信息长度为 1024 字节，有未使用的空间可能是为未来的扩展预留的
//================================================================================
struct SectionHeader {
	/** 实际点（splat）数量 */
	unsigned int SectionSplatCount;

	/** 能容纳的最大点数量 */
	unsigned int SectionSplatCapacity;

	/** 每个桶的容量 */
	unsigned int BucketCapacity;

	/** 桶的数量 */
	unsigned int BucketCount;

	/** 空间块的大小 */
	float BlockSize;

	/** 桶存储的大小 */
	unsigned short BucketSize;

	/** 量化范围，如果未指定则使用主头中的值 */
	unsigned int QuantizationRange;

	/** 满桶的数量 */
	unsigned int FullBucketCount;

	/** 未满桶的数量 */
	unsigned int PartialBucketCount;

	/** 球谐函数的级别 */
	unsigned short ShDegree;

	/** 保留字段，用于未来扩展 */
	unsigned char Reserved[1024 - sizeof(unsigned int) * 9 - sizeof(float) - sizeof(unsigned short) * 2];
};

//================================================================================
/// KSplat格式高斯数据读写类
/// 继承自CGGaussianReadWrite，实现对KSplat格式高斯数据的读写操作
//================================================================================
class Ci_KSplatGaussianReadWrite : public MapGIS::Tile::CGGaussianReadWrite
{
public:
	//================================================================================
	/// 默认构造函数
	//================================================================================
	Ci_KSplatGaussianReadWrite()
	{
	};

	//================================================================================
	/// 虚析构函数
	//================================================================================
	virtual ~Ci_KSplatGaussianReadWrite() {};

protected:
	//================================================================================
	/// 内部读取接口实现
	/// 从指定文件路径读取高斯数据到数据模型
	///
	/// @param [in]  filePath 文件路径
	/// @param [out] data     高斯数据模型引用
	/// @return 操作结果
	///         - gisLONG > 0：读取成功
	///         - gisLONG <= 0：读取失败
	//================================================================================
	virtual gisLONG i_Read(CGString filePath, MapGIS::Tile::GaussianModel& data);

	//================================================================================
	/// 内部回调读取接口实现
	/// 从指定文件路径读取高斯数据，通过回调函数逐个处理特征点
	///
	/// @param [in] filePath  文件路径
	/// @param [in] callback  回调函数，用于处理每个高斯特征点
	/// @return 操作结果
	///         - gisLONG > 0：读取成功
	///         - gisLONG <= 0：读取失败
	//================================================================================
	virtual gisLONG i_Read(CGString filePath, std::function<void(MapGIS::Tile::GaussianFeature& dot, bool& isStop)>& callback);

	//================================================================================
	/// 内部写入接口实现
	/// 将高斯数据模型写入到指定文件路径
	///
	/// @param [in] filePath 文件路径
	/// @param [in] data     高斯数据模型常量引用
	/// @return 操作结果
	///         - gisLONG > 0：写入成功
	///         - gisLONG <= 0：写入失败
	//================================================================================
	virtual gisLONG i_Write(CGString filePath, const MapGIS::Tile::GaussianModel& data);
};