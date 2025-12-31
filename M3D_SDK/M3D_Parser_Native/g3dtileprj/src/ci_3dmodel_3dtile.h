#pragma once
#include <vector>
#include <string>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <unordered_map>
#include <map>

#include "cgstring.h"
#include "gbytearray.h"
#include "../include/g3dtilerecord.h"

using namespace std;

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 3D Tiles FeatureTable基类
		/// 用于存储和管理3D Tiles中FeatureTable的相关数据和扩展信息
		//================================================================================
		class Ci_3DTileFeatureTable
		{
		public:
			//================================================================================
			/// 扩展信息
			//================================================================================
			rapidjson::Value m_extensions;
		};

		//================================================================================
		/// 3D Tiles BatchTable基类
		/// 用于处理3D Tiles中BatchTable的读写操作
		//================================================================================
		class Ci_3DTileBatchTable
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_3DTileBatchTable();

			//================================================================================
			/// 虚析构函数
			//================================================================================
			virtual ~Ci_3DTileBatchTable();

			//================================================================================
			/// 将属性记录写入BatchTable
			///
			/// @param [in]  pRecords      属性记录数据
			/// @param [in]  pIdToBatchID  ID到BatchID的映射表
			/// @param [out] json          输出的JSON数据流
			/// @param [out] bin           输出的二进制数据流
			/// @param [in]  IdName        ID字段名称
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			gisLONG WriteTo(const LayerAttribute* pRecords, const map<gisINT64, gisINT64>* pIdToBatchID, CGByteArray& json, CGByteArray& bin, string IdName);

			//================================================================================
			/// 将属性记录写入BatchTable（I3DM专用）
			/// I3DM中使用，pRecords中ID无效,pIndexToID记录序号到ID的转换
			///
			/// @param [in]  pRecords      属性记录数据
			/// @param [in]  pIndexToID    序号到ID的转换向量
			/// @param [out] json          输出的JSON数据流
			/// @param [out] bin           输出的二进制数据流
			/// @param [in]  IdName        ID字段名称
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			gisLONG WriteTo(const LayerAttribute* pRecords, const vector<unsigned int>* pIndexToID, CGByteArray& json, CGByteArray& bin, string IdName);

			//================================================================================
			/// 从BatchTable读取属性记录
			///
			/// @param [in]  json          输入的JSON数据流
			/// @param [in]  bin           输入的二进制数据流
			/// @param [in]  IdName        ID字段名称
			/// @param [in]  batchLength   Batch长度
			/// @param [out] records       输出的属性记录
			/// @return 操作结果
			///         - gisLONG > 0：读取成功
			///         - gisLONG <= 0：读取失败
			//================================================================================
			gisLONG ReadFrom(const CGByteArray& json, const CGByteArray& bin, CGString IdName, int batchLength, LayerAttribute& records);

		private:
			//================================================================================
			/// 内部实现：将属性记录写入BatchTable
			///
			/// @param [in]  pRecords         属性记录数据
			/// @param [out] json             输出的JSON数据流
			/// @param [out] bin              输出的二进制数据流
			/// @param [in,out] batchTableJson BatchTable的JSON字符串
			/// @param [in]  batchLength      Batch长度
			/// @param [in,out] batchIDToIndex BatchID到索引的映射表
			/// @param [in]  existValue       是否存在有效值
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			gisLONG i_WriteRecordsTo(const LayerAttribute* pRecords, CGByteArray& json, CGByteArray& bin, CGString& batchTableJson, gisLONG batchLength, unordered_map<gisINT64, gisINT64>& batchIDToIndex, bool existValue);
		};

		//================================================================================
		/// 3D Tiles模型数据头信息基类
		/// 管理3D Tiles各种模型格式的头部信息，包括FeatureTable和BatchTable的长度信息
		//================================================================================
		class Ci_3DTileModelHeader
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_3DTileModelHeader();

			//================================================================================
			/// 虚析构函数
			//================================================================================
			virtual ~Ci_3DTileModelHeader();

			//================================================================================
			/// 设置FeatureTable JSON长度（无填充）
			///
			/// @param [in] len 长度值
			//================================================================================
			void SetFeatureTableJSONLengthNoPadding(unsigned int len);

			//================================================================================
			/// 获取FeatureTable JSON长度（无填充）
			///
			/// @return FeatureTable JSON长度（无填充）
			//================================================================================
			unsigned int GetFeatureTableJSONLengthNoPadding();

			//================================================================================
			/// 获取FeatureTable JSON填充计数
			///
			/// @return FeatureTable JSON填充计数
			//================================================================================
			unsigned int GetFeatureTableJSONPaddingCount();

			//================================================================================
			/// 获取FeatureTable JSON长度（含填充）
			///
			/// @return FeatureTable JSON长度（含填充）
			//================================================================================
			unsigned int GetFeatureTableJSONLength();

			//================================================================================
			/// 设置FeatureTable二进制长度（无填充）
			///
			/// @param [in] len 长度值
			//================================================================================
			void SetFeatureTableBinaryLengthNoPadding(unsigned int len);

			//================================================================================
			/// 获取FeatureTable二进制长度（无填充）
			///
			/// @return FeatureTable二进制长度（无填充）
			//================================================================================
			unsigned int GetFeatureTableBinaryLengthNoPadding();

			//================================================================================
			/// 获取FeatureTable二进制填充计数
			///
			/// @return FeatureTable二进制填充计数
			//================================================================================
			unsigned int GetFeatureTableBinaryPaddingCount();

			//================================================================================
			/// 获取FeatureTable二进制长度（含填充）
			///
			/// @return FeatureTable二进制长度（含填充）
			//================================================================================
			unsigned int GetFeatureTableBinaryLength();

			//================================================================================
			/// 设置BatchTable JSON长度（无填充）
			///
			/// @param [in] len 长度值
			//================================================================================
			void SetBatchTableJSONLengthNoPadding(unsigned int len);

			//================================================================================
			/// 获取BatchTable JSON长度（无填充）
			///
			/// @return BatchTable JSON长度（无填充）
			//================================================================================
			unsigned int GetBatchTableJSONLengthNoPadding();

			//================================================================================
			/// 获取BatchTable JSON填充计数
			///
			/// @return BatchTable JSON填充计数
			//================================================================================
			unsigned int GetBatchTableJSONPaddingCount();

			//================================================================================
			/// 获取BatchTable JSON长度（含填充）
			///
			/// @return BatchTable JSON长度（含填充）
			//================================================================================
			unsigned int GetBatchTableJSONLength();

			//================================================================================
			/// 设置BatchTable二进制长度（无填充）
			///
			/// @param [in] len 长度值
			//================================================================================
			void SetBatchTableBinaryLengthNoPadding(unsigned int len);

			//================================================================================
			/// 获取BatchTable二进制长度（无填充）
			///
			/// @return BatchTable二进制长度（无填充）
			//================================================================================
			unsigned int GetBatchTableBinaryLengthNoPadding();

			//================================================================================
			/// 获取BatchTable二进制填充计数
			///
			/// @return BatchTable二进制填充计数
			//================================================================================
			unsigned int GetBatchTableBinaryPaddingCount();

			//================================================================================
			/// 获取BatchTable二进制长度（含填充）
			///
			/// @return BatchTable二进制长度（含填充）
			//================================================================================
			unsigned int GetBatchTableBinaryLength();

			//================================================================================
			/// 设置总字节长度
			///
			/// @param [in] len 总字节长度
			//================================================================================
			void SetByteLength(unsigned int len);

			//================================================================================
			/// 获取总字节长度
			///
			/// @return 总字节长度
			//================================================================================
			unsigned int GetByteLength();

			//================================================================================
			/// 获取头部长度（纯虚函数）
			///
			/// @return 头部长度
			//================================================================================
			virtual unsigned int GetHeaderLength() = 0;

		protected:
			string m_magic;                             // 魔数标识
			unsigned int m_version;                     // 版本号
			unsigned int m_byteLength;                  // 总字节长度

			unsigned int m_featureTableJSONLength;      // FeatureTable JSON长度
			unsigned int m_featureTableJSONPaddingCount; // FeatureTable JSON填充计数
			unsigned int m_featureTableJsonLengthNoPadding; // FeatureTable JSON长度（无填充）

			unsigned int m_featureTableBinaryLength;    // FeatureTable二进制长度
			unsigned int m_featureTableBinaryLengthNoPadding; // FeatureTable二进制长度（无填充）
			unsigned int m_featureTableBinaryPaddingCount; // FeatureTable二进制填充计数

			unsigned int m_batchTableJSONLength;        // BatchTable JSON长度
			unsigned int m_batchTableJSONLengthNoPadding; // BatchTable JSON长度（无填充）
			unsigned int m_batchTableJSONPaddingCount;  // BatchTable JSON填充计数

			unsigned int m_batchTableBinaryLength;      // BatchTable二进制长度
			unsigned int m_batchTableBinaryLengthNoPadding; // BatchTable二进制长度（无填充）
			unsigned int m_batchTableBinaryPaddingCount; // BatchTable二进制填充计数
		};

		//================================================================================
		/// 从FeatureTable或BatchTable中获取指定类型的数据
		/// 支持常规的整型、浮点型、布尔型、字符串类型数据的获取
		///
		/// @param [in]  doc        FeatureTable或BatchTable的JSON文档
		/// @param [in]  name       要获取内容的字段名称
		/// @param [in]  num        数据个数
		/// @param [in]  bin        FeatureTable或BatchTable的二进制数据流
		/// @param [out] outValues  获取到的数据内容（空代表获取失败）
		//================================================================================
		template <typename T>
		void GetTableValue(rapidjson::Document& doc, CGString name, int num, const CGByteArray& bin, vector<T>& outValues);

		//================================================================================
		/// 字符串类型特化版本
		//================================================================================
		template<>
		void GetTableValue<CGString>(rapidjson::Document& doc, CGString name, int num, const CGByteArray& bin, vector<CGString>& outValues);

		//================================================================================
		/// 布尔类型特化版本
		//================================================================================
		template<>
		void GetTableValue<bool>(rapidjson::Document& doc, CGString name, int num, const CGByteArray& bin, vector<bool>& outValues);
	}
}