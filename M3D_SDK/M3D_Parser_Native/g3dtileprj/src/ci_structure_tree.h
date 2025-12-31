#pragma once
#include <vector>
#include <algorithm>
#include "ci_cache_storage.h"
#include "rapidjson/document.h"
#include "../include/g3dtilestructuretree.h"

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 结构树管理器类
		/// 负责结构树数据的读写操作
		//================================================================================
		class Ci_StructureTreeManager
		{
		public:
			//================================================================================
			/// 虚析构函数
			//================================================================================
			virtual ~Ci_StructureTreeManager()
			{
			}

			//================================================================================
			/// 写入结构树数据
			///
			/// @param [in] rootNode       根节点对象引用
			/// @param [in] pCacheStorage  缓存存储器指针
			/// @param [in] relativePath   相对路径
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			static gisLONG Write(StructureTreeNode& rootNode, MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath);

			//================================================================================
			/// 读取结构树数据
			///
			/// @param [in]  pCacheStorage  缓存存储器指针
			/// @param [in]  relativePath   相对路径
			/// @param [out] rootNode       根节点对象引用
			/// @return 操作结果
			///         - gisLONG > 0：读取成功
			///         - gisLONG <= 0：读取失败
			//================================================================================
			static gisLONG Read(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath, StructureTreeNode& rootNode);

		private:
			//================================================================================
			/// 构造函数
			///
			/// @param [in] rootNode 根节点对象引用
			//================================================================================
			Ci_StructureTreeManager(StructureTreeNode& rootNode)
			{
				m_nItemSizeFile = 200;      // 一个叶子节点存放多少个子项
				m_pRootNode = &rootNode;
			}

			//================================================================================
			/// 内部写入实现
			///
			/// @param [in] pCacheStorage 缓存存储器指针
			/// @param [in] relativePath  相对路径
			/// @return 操作结果
			///         - gisLONG > 0：写入成功
			///         - gisLONG <= 0：写入失败
			//================================================================================
			gisLONG i_Write(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath);

			//================================================================================
			/// 内部读取实现
			///
			/// @param [in] pCacheStorage 缓存存储器指针
			/// @param [in] relativePath  相对路径
			/// @return 操作结果
			///         - gisLONG > 0：读取成功
			///         - gisLONG <= 0：读取失败
			//================================================================================
			gisLONG i_Read(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath);

			//================================================================================
			/// 保存结构树叶子节点
			///
			/// @param [in] doc           JSON文档对象
			/// @param [in] itemArray     子项数组
			/// @param [in] pCacheStorage 缓存存储器指针
			/// @param [in] relativePath  相对路径
			/// @param [in] name          名称
			/// @param [in] isSaveSubfolder 是否保存子文件夹
			/// @return 保存后的相对路径
			//================================================================================
			CGString SaveStructureTreeLeaf(rapidjson::Document& doc, rapidjson::Value& itemArray, MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath, string name, bool isSaveSubfolder);

			//================================================================================
			/// 将结构树叶子节点转换为JSON对象
			///
			/// @param [in]  leaf       叶子节点对象
			/// @param [out] nodeObj    JSON节点对象
			/// @param [in]  allocator  JSON分配器
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG StructureTreeLeafToJsonObject(const StructureTreeLeaf& leaf, rapidjson::Value& nodeObj, rapidjson::Value::AllocatorType& allocator);

			//================================================================================
			/// 将结构树节点转换为JSON对象
			///
			/// @param [in]  pCacheStorage  缓存存储器指针
			/// @param [in]  node           结构树节点对象
			/// @param [out] parentNodeObj  父节点JSON对象
			/// @param [in]  allocator      JSON分配器
			/// @param [in]  parentName     父节点名称
			/// @param [in]  maxLevel       最大层级
			/// @param [in]  currentLevel   当前层级
			/// @param [in]  relativePath   相对路径
			/// @param [in]  isRoot         是否为根节点
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG StructureTreeNodeToJsonObject(MapGIS::Tile::G3DCacheStorage* pCacheStorage, const StructureTreeNode& node, rapidjson::Value& parentNodeObj, rapidjson::Value::AllocatorType& allocator, string parentName, int maxLevel, int currentLevel, CGString relativePath, bool isRoot);

			//================================================================================
			/// 将结构树节点子项保存为JSON文件
			///
			/// @param [in] pCacheStorage 缓存存储器指针
			/// @param [in] node          结构树节点对象
			/// @param [in] treeName      树名称
			/// @param [in] relativePath  相对路径
			/// @param [in] isRootFile    是否为根文件
			/// @return 保存后的相对路径
			//================================================================================
			CGString StructureTreeNodechildToJsonFile(MapGIS::Tile::G3DCacheStorage* pCacheStorage, const StructureTreeNode& node, string treeName, CGString relativePath, bool isRootFile);

			int m_nItemSizeFile;                // 一个叶子节点存放多少个子项
			StructureTreeNode* m_pRootNode;     // 根节点指针
		};
	}
}