#pragma once

#include "../include/g3dtilerecord.h"
#include "gbytearray.h"
#include <vector>

using namespace std;

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 模型图层字段信息类
		/// 处理图层字段信息的序列化和反序列化操作
		//================================================================================
		class Ci_ModelLayerFieldsInfo
		{
		public:
			//================================================================================
			/// 从字节数组读取图层字段信息
			///
			/// @param [in]  in  输入字节数组
			/// @param [out] out 输出图层字段信息向量
			/// @return 操作结果
			///         - gisLONG > 0：读取成功
			///         - gisLONG <= 0：读取失败
			//================================================================================
			gisLONG From(const CGByteArray& in, vector<LayerFieldsInfo>& out);

			//================================================================================
			/// 将图层字段信息写入字节数组
			///
			/// @param [in]  in  输入图层字段信息向量
			/// @param [out] out 输出字节数组
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			gisLONG To(const vector<LayerFieldsInfo>& in, CGByteArray& out);
		};

		//================================================================================
		/// 模型属性ATT文件处理类
		/// 处理属性数据的读写操作，支持压缩格式
		//================================================================================
		class Ci_ModelAttributeAttFile
		{
		public:
			//================================================================================
			/// 从字节数组读取属性信息
			///
			/// @param [in]  in           输入字节数组
			/// @param [out] out          输出属性信息
			/// @return 操作结果
			///         - gisLONG > 0：读取成功
			///         - gisLONG <= 0：读取失败
			//================================================================================
			gisLONG From(const CGByteArray& in, Attribute& out);

			//================================================================================
			/// 将属性信息写入字节数组
			///
			/// @param [in]  in           输入属性信息
			/// @param [out] out          输出字节数组
			/// @param [in]  compressType 数据压缩模式
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			gisLONG To(const Attribute& in, CGByteArray& out, AttributeCompressType compressType);

			//================================================================================
			/// 将图层属性信息写入字节数组（点云专用）
			/// 点云属性输出成流，不再写featureIndexData，可以根据索引号直接查找属性
			///
			/// @param [in]  in           输入图层属性信息
			/// @param [out] out          输出字节数组
			/// @param [in]  ids          按照该ID序列的顺序写属性信息
			/// @param [in]  compressType 数据压缩模式
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			gisLONG To(const LayerAttribute& in, CGByteArray& out, vector<gisINT64>& ids, AttributeCompressType compressType);
		};
	}
}