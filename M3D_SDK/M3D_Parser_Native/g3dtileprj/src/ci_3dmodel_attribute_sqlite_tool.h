#pragma once
#include <map>
#include "ci_cache_storage.h"
#include "ci_3dmodel_attribute_sqlite.h"
namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 3D模型属性SQLite工具类
		/// 属性写入SQLite工具，因为不能直接写到MongoDB，这里先在本地写，然后拷贝到MongoDB
		//================================================================================
		class Ci_3DModelAttributeSqliteTool
		{
		public:
			//================================================================================
			/// 获取单例实例
			///
			/// @param [in] pCacheStorage 缓存存储器指针
			/// @param [in] noExistIsCreate 不存在时是否创建
			/// @return SQLite工具对象指针
			//================================================================================
			static Ci_3DModelAttributeSqliteTool* GetInstance(MapGIS::Tile::G3DCacheStorage* pCacheStorage, bool noExistIsCreate);

			//================================================================================
			/// 检查实例是否存在
			///
			/// @param [in] pCacheStorage 缓存存储器指针
			/// @return 是否存在的判断结果
			//================================================================================
			static bool ExistInstance(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

			//================================================================================
			/// 释放SQLite工具对象
			///
			/// @param [in] pOwner SQLite工具对象指针
			//================================================================================
			static void Free3DModelAttributeSqlite(Ci_3DModelAttributeSqliteTool* pOwner);

			//================================================================================
			/// 获取相对路径
			///
			/// @return 相对路径字符串
			//================================================================================
			CGString GetGetRelativePath();

			//================================================================================
			/// 获取属性数据
			///
			/// @return 属性数据指针
			//================================================================================
			MapGIS::Tile::Attribute* GetAttribute();

			//================================================================================
			/// 设置id与范围的关系
			/// 
			/// @param [in] tid2Box id与范围关系
			/// @return 属性数据指针
			//================================================================================
			void SetBoxInfo(const map<gisINT64, vector<double> >& tid2Box);

			//================================================================================
			/// 设置id与范围的关系
			///
			/// @return id与范围的关系
			//================================================================================
			const map<gisINT64, vector<double>> GetBoxInfo() const;

			//================================================================================
			/// 保存数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：保存成功
			///         - gisLONG <= 0：保存失败
			//================================================================================
			gisLONG Save();

		public:
			string m_sqlistName;    // SQLite数据库文件名 // = "DataAttribute.db";

		private:
			//================================================================================
			/// 构造函数
			///
			/// @param [in] pCacheStorage   缓存存储器指针
			/// @param [in] isCreateOrOpen  是否创建或打开标志
			//================================================================================
			Ci_3DModelAttributeSqliteTool(MapGIS::Tile::G3DCacheStorage* pCacheStorage, bool isCreateOrOpen);

			//================================================================================
			/// 析构函数
			//================================================================================
			~Ci_3DModelAttributeSqliteTool();

			MapGIS::Tile::G3DCacheStorage* m_pCacheStorage;                 // 缓存存储器指针
			Ci_3DModelAttributeSqlite* m_pAttributeSqlite;                  // 属性SQLite操作对象指针
			static map<MapGIS::Tile::G3DCacheStorage*, Ci_3DModelAttributeSqliteTool*> m_mapSqliteTool;  // SQLite工具映射表
			bool m_isLocalStorage;                                          // 是否为本地存储标志
		};
	}
}