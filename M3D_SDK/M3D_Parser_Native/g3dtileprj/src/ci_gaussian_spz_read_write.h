#pragma once
#include "g3dtilegaussian.h"
#include "load-spz.h"

//================================================================================
/// SPZ格式高斯数据读写类
/// 继承自CGGaussianReadWrite，实现对SPZ格式高斯数据的读写操作
//================================================================================
class Ci_SpzGaussianReadWrite : public MapGIS::Tile::CGGaussianReadWrite
{
public:
	//================================================================================
	/// 默认构造函数
	//================================================================================
	Ci_SpzGaussianReadWrite()
	{
	};

	//================================================================================
	/// 虚析构函数
	//================================================================================
	virtual ~Ci_SpzGaussianReadWrite() {};

	//================================================================================
	/// 从字节数组读取SPZ格式高斯数据
	///
	/// @param [in]  in   输入字节数组
	/// @param [out] data 高斯数据模型引用
	/// @return 操作结果
	///         - gisLONG > 0：读取成功
	///         - gisLONG <= 0：读取失败
	//================================================================================
	gisLONG Read(const vector<uint8_t>& in, MapGIS::Tile::GaussianModel& data);

	//================================================================================
	/// 将高斯数据写入字节数组（SPZ格式）
	///
	/// @param [in]  data 高斯数据模型常量引用
	/// @param [out] out  输出字节数组
	/// @return 操作结果
	///         - gisLONG > 0：写入成功
	///         - gisLONG <= 0：写入失败
	//================================================================================
	gisLONG Write(const MapGIS::Tile::GaussianModel& data, vector<uint8_t>& out);

protected:
	//================================================================================
	/// 内部读取接口实现
	/// 从指定SPZ文件路径读取高斯数据到数据模型
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
	/// 从指定SPZ文件路径读取高斯数据，通过回调函数逐个处理特征点
	///
	/// @param [in] filePath  文件路径
	/// @param [in] callback  回调函数，用于处理每个高斯特征点
	/// @return 操作结果
	///         - gisLONG > 0：读取成功
	///         - gisLONG <= 0：读取失败
	//================================================================================
	virtual gisLONG i_Read(CGString filePath, std::function<void(MapGIS::Tile::GaussianFeature& dot, bool& isStop)>& callback);

	//================================================================================
	/// 内部读取接口实现
	/// 从字节数组读取SPZ格式高斯数据到数据模型
	///
	/// @param [in]  in   输入字节数组
	/// @param [out] data 高斯数据模型引用
	/// @return 操作结果
	///         - gisLONG > 0：读取成功
	///         - gisLONG <= 0：读取失败
	//================================================================================
	virtual gisLONG i_Read(const vector<uint8_t>& in, MapGIS::Tile::GaussianModel& data);

	//================================================================================
	/// 内部回调读取接口实现
	/// 从SPZ高斯云对象读取数据，通过回调函数逐个处理特征点
	///
	/// @param [in] gaussianCloudValue SPZ高斯云对象
	/// @param [in] callback           回调函数，用于处理每个高斯特征点
	/// @return 操作结果
	///         - gisLONG > 0：读取成功
	///         - gisLONG <= 0：读取失败
	//================================================================================
	virtual gisLONG i_Read(spz::GaussianCloud& gaussianCloudValue, std::function<void(MapGIS::Tile::GaussianFeature& dot, bool& isStop)>& callback);

	//================================================================================
	/// 内部写入接口实现
	/// 将高斯数据模型写入到指定SPZ文件路径
	///
	/// @param [in] filePath 文件路径
	/// @param [in] data     高斯数据模型常量引用
	/// @return 操作结果
	///         - gisLONG > 0：写入成功
	///         - gisLONG <= 0：写入失败
	//================================================================================
	virtual gisLONG i_Write(CGString filePath, const MapGIS::Tile::GaussianModel& data);

	//================================================================================
	/// 内部写入接口实现
	/// 将高斯数据模型写入到字节数组（SPZ格式）
	///
	/// @param [in]  data 高斯数据模型常量引用
	/// @param [out] out  输出字节数组
	/// @return 操作结果
	///         - gisLONG > 0：写入成功
	///         - gisLONG <= 0：写入失败
	//================================================================================
	virtual gisLONG i_Write(const MapGIS::Tile::GaussianModel& data, vector<uint8_t>& out);

	//================================================================================
	/// 内部写入接口实现
	/// 将高斯数据模型写入到SPZ高斯云对象
	///
	/// @param [in]  data              高斯数据模型常量引用
	/// @param [out] gaussianCloudValue SPZ高斯云对象
	/// @return 操作结果
	///         - gisLONG > 0：写入成功
	///         - gisLONG <= 0：写入失败
	//================================================================================
	virtual gisLONG i_Write(const MapGIS::Tile::GaussianModel& data, spz::GaussianCloud& gaussianCloudValue);
};