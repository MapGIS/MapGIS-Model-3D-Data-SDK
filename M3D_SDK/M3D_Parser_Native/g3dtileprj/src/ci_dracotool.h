#ifndef CI_DRACOTOOL_H
#define CI_DRACOTOOL_H

#include "gbytearray.h"
#include <vector>
#include <string>
using namespace std;

//================================================================================
/// Draco压缩工具类
/// 提供点云和几何数据的Draco压缩与解压缩功能
//================================================================================
class Ci_DracoTool
{
public:
	//================================================================================
	/// 创建点云Draco二进制数据
	///
	/// @param [in]  pointVector    点坐标数据向量（XYZXYZ...格式）
	/// @param [in]  colorVector    颜色数据向量（RGBRGB...格式）
	/// @param [in]  batchIDVector  批次ID数据向量
	/// @param [out] posAttId       位置属性ID
	/// @param [out] colorAttId     颜色属性ID
	/// @param [out] batchIdAttId   批次ID属性ID
	///
	/// @return 压缩后的Draco二进制数据
	//================================================================================
	static std::vector<char> CreatePointCloudDracoBinData(
		const std::vector<float>& pointVector,
		const std::vector<unsigned char>& colorVector,
		const std::vector<unsigned int>& batchIDVector,
		int& posAttId,
		int& colorAttId,
		int& batchIdAttId);

	//================================================================================
	/// 几何数据压缩（字符串版本）
	///
	/// @param [in]  in  输入数据字符串
	/// @param [out] out 输出压缩后的数据字符串
	///
	/// @return 操作结果
	///         - true: 压缩成功
	///         - false: 压缩失败
	//================================================================================
	static bool GeometryCompress(const std::string& in, std::string& out);

	//================================================================================
	/// 几何数据解压缩（字符串版本）
	///
	/// @param [in]  in  输入压缩数据字符串
	/// @param [out] out 输出解压后的数据字符串
	///
	/// @return 操作结果
	///         - true: 解压成功
	///         - false: 解压失败
	//================================================================================
	static bool GeometryDecompression(const std::string& in, std::string& out);

	//================================================================================
	/// 几何数据压缩（字符串到字节数组版本）
	///
	/// @param [in]  in  输入数据字符串
	/// @param [out] out 输出压缩后的字节数组
	///
	/// @return 操作结果
	///         - true: 压缩成功
	///         - false: 压缩失败
	//================================================================================
	static bool GeometryCompress(const std::string& in, CGByteArray& out);

	//================================================================================
	/// 几何数据解压缩（字符串到字节数组版本）
	///
	/// @param [in]  in  输入压缩数据字符串
	/// @param [out] out 输出解压后的字节数组
	///
	/// @return 操作结果
	///         - true: 解压成功
	///         - false: 解压失败
	//================================================================================
	static bool GeometryDecompression(const std::string& in, CGByteArray& out);

	//================================================================================
	/// 几何数据压缩（字节数组版本）
	///
	/// @param [in]  in  输入数据字节数组
	/// @param [out] out 输出压缩后的字节数组
	///
	/// @return 操作结果
	///         - true: 压缩成功
	///         - false: 压缩失败
	//================================================================================
	static bool GeometryCompress(const CGByteArray& in, CGByteArray& out);

	//================================================================================
	/// 几何数据解压缩（字节数组版本）
	///
	/// @param [in]  in  输入压缩数据字节数组
	/// @param [out] out 输出解压后的字节数组
	///
	/// @return 操作结果
	///         - true: 解压成功
	///         - false: 解压失败
	//================================================================================
	static bool GeometryDecompression(const CGByteArray& in, CGByteArray& out);

	//================================================================================
	/// 解析点云Draco二进制数据
	///
	/// @param [in]  buf          输入数据缓冲区
	/// @param [in]  bufSize      缓冲区大小
	/// @param [out] positions    解析出的位置数据（XYZXYZ...格式）
	/// @param [out] colors       解析出的颜色数据（RGBRGB...格式）
	/// @param [out] batchIDs     解析出的批次ID数据
	/// @param [in]  pointsLength 点数量
	///
	/// @return 操作结果
	///         - true: 解析成功
	///         - false: 解析失败
	//================================================================================
	static bool ParsePointCloudDracoBinData(
		const char* buf,
		std::size_t bufSize,
		std::vector<float>& positions,
		std::vector<unsigned char>& colors,
		std::vector<unsigned int>& batchIDs,
		unsigned int pointsLength);
};

#endif