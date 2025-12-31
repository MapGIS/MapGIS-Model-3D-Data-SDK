#pragma once

#include "rapidjson/document.h"
#include <algorithm>
#include <map>
#include "../include/g3dtiledefine.h"
#include "../include/g3dtilegeometry.h"
#include "../include/g3dtilerecord.h"
using namespace std;

namespace M3D
{
#pragma region tile

	//================================================================================
	/// 几何数据二进制类型枚举
	//================================================================================
	enum class BlobType : int
	{
		glb = 0,    // GLB格式
		glbx = 1,   // GLBX格式
		b3dm = 2,   // B3DM格式
		i3dm = 3,   // I3DM格式
		pnts = 4,   // PNTS格式
		cmpt = 5    // CMPT格式
	};

#pragma endregion

	//================================================================================
	/// 子节点信息类
	/// 存储M3D瓦片树中子节点的相关信息
	//================================================================================
	class Ci_ChildTileInfo
	{
	public:
		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_ChildTileInfo();

		//================================================================================
		/// 析构函数
		//================================================================================
		~Ci_ChildTileInfo();

		float lodError;                                         // LOD误差
		MapGIS::Tile::BoundingVolume boundingVolume;            // 包围体
		vector<double> transform;                               // 转换矩阵
		CGString uri;                                           // 节点URI

		//================================================================================
		/// 从JSON对象读取数据
		///
		/// @param [in] jsonObj JSON对象
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		gisLONG From(rapidjson::Value& jsonObj);

		//================================================================================
		/// 将数据写入JSON对象
		///
		/// @param [out] jsonObj   JSON对象
		/// @param [in]  allocator JSON分配器
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		gisLONG To(rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator) const;

		//================================================================================
		/// 重置数据
		//================================================================================
		void Reset();

		//================================================================================
		/// 赋值运算符重载
		///
		/// @param [in] other 源对象
		/// @return 当前对象引用
		//================================================================================
		Ci_ChildTileInfo& operator=(const Ci_ChildTileInfo& other);
	};

	//================================================================================
	/// 内容信息类
	/// 存储M3D瓦片数据(.m3d)的信息
	//================================================================================
	class Ci_Content
	{
	public:
		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_Content();

		//================================================================================
		/// 析构函数
		//================================================================================
		~Ci_Content();

		CGString tileData_uri;                                  // 瓦片URI（例如："0.m3d"）

		BlobType geometry_blobType;                             // 几何文件二进制类型
		CGString geometry_uri;                                  // 几何文件URI（例如："./geometry/0.glbx"）

		CGString geometry_tid_uri;                              // 几何TID文件URI
		MapGIS::Tile::GeometryType geometry_geometryType;       // 几何类型
		MapGIS::Tile::GeoCompressType geometry_geoCompressType; // 几何压缩类型

		MapGIS::Tile::AttFileType attribute_attType;            // 属性文件类型
		CGString attribute_uri;                                 // 属性文件URI（例如："./attribute/0.json"）

		CGString texture_uri;                                   // 纹理文件URI

		MapGIS::Tile::DataType dataType;                        // 瓦片数据类型

		MapGIS::Tile::VoxelModel mVoxelModel;                   // 体素模型

		//================================================================================
		/// 从JSON对象读取数据
		///
		/// @param [in] jsonObj JSON对象
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		gisLONG From(rapidjson::Value& jsonObj);

		//================================================================================
		/// 将数据写入JSON对象
		///
		/// @param [out] jsonObj   JSON对象
		/// @param [in]  allocator JSON分配器
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		gisLONG To(rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator) const;

		//================================================================================
		/// 重置数据
		//================================================================================
		void Reset();

		//================================================================================
		/// 赋值运算符重载
		///
		/// @param [in] other 源对象
		/// @return 当前对象引用
		//================================================================================
		Ci_Content& operator=(const Ci_Content& other);

		//================================================================================
		/// 将体素模型转换为JSON对象
		///
		/// @param [in]  voxelModel 体素模型对象
		/// @param [out] jsonObj    JSON对象
		/// @param [in]  allocator  JSON分配器
		/// @return 操作结果
		///         - gisLONG > 0：转换成功
		///         - gisLONG <= 0：转换失败
		//================================================================================
		static gisLONG VoxelModelToJsonObj(const MapGIS::Tile::VoxelModel& voxelModel, rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator);

		//================================================================================
		/// 从JSON对象转换为体素模型
		///
		/// @param [in]  jsonObj    JSON对象
		/// @param [out] voxelModel 体素模型对象
		/// @return 操作结果
		///         - gisLONG > 0：转换成功
		///         - gisLONG <= 0：转换失败
		//================================================================================
		static gisLONG JsonObjToVoxelModel(const rapidjson::Value& jsonObj, MapGIS::Tile::VoxelModel& voxelModel);
	};

	//================================================================================
	/// 瓦片类
	/// 存储M3D瓦片的相关信息
	//================================================================================
	class Ci_Tile
	{
	public:
		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_Tile();

		//================================================================================
		/// 析构函数
		//================================================================================
		~Ci_Tile();

		CGString name;                                          // 名称
		gisLONG lodLevel;                                       // LOD级别
		MapGIS::Tile::BoundingVolume boundingVolume;            // 包围体
		MapGIS::Tile::LodMode lodMode;                          // LOD切换模式
		MapGIS::Tile::RefineType lodType;                       // LOD类型
		float lodError;                                         // LOD误差
		vector<double> transform;                               // 转换矩阵(齐次矩阵)

		CGString parentNode_uri;                                // 父节点URI
		vector<Ci_ChildTileInfo> childrenNode;                  // 子节点列表
		CGString sharedUri;                                     // 共享资源URI

		gisLONG tileDataInfoIndex;                              // 当前节点显示的瓦片数据(.m3d)序号（从0开始）
		vector<Ci_Content> tileDataInfoList;                    // 瓦片数据(.m3d)信息列表

		map<CGString, CGString> extend;                         // 扩展信息

		//================================================================================
		/// 从JSON对象读取数据
		///
		/// @param [in] jsonObj JSON对象
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		gisLONG From(rapidjson::Value& jsonObj);

		//================================================================================
		/// 将数据写入JSON对象
		///
		/// @param [out] jsonObj   JSON对象
		/// @param [in]  allocator JSON分配器
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		gisLONG To(rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator) const;

		//================================================================================
		/// 重置数据
		//================================================================================
		void Reset();

		//================================================================================
		/// 赋值运算符重载
		///
		/// @param [in] other 源对象
		/// @return 当前对象引用
		//================================================================================
		Ci_Tile& operator=(const Ci_Tile& other);
	};

	//================================================================================
	/// 瓦片集类
	/// 存储M3D瓦片集的相关信息
	//================================================================================
	class Ci_Tileset
	{
	public:
		CGString asset;                                         // 描述
		MapGIS::Tile::M3DVersion version;                       // 版本
		CGString dataName;                                      // 名称
		CGString guid;                                          // GUID
		MapGIS::Tile::FileCompressType compressType;            // 文件压缩类型
		MapGIS::Tile::SpatialReference spatialReference;        // 空间参照系
		MapGIS::Tile::TreeType treeType;                        // 目录树类型
		MapGIS::Tile::RefineType lodType;                       // 加载下一级模式
		MapGIS::Tile::BoundingVolume boundingVolume;            // 包围体
		D_3DOT position;                                        // 中心点
		CGString rootNode_uri;                                  // 根节点URI
		CGString layerInfo_uri;                                 // 数据层信息URI
		CGString structureTree_uri;                             // 结构树信息URI
		vector<MapGIS::Tile::Field> fieldInfo;                  // 属性字段
		CGString sqlitePath;                                    // SQLite数据库路径

		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_Tileset();

		//================================================================================
		/// 析构函数
		//================================================================================
		~Ci_Tileset();

		//================================================================================
		/// 从JSON对象读取数据
		///
		/// @param [in] jsonObj JSON对象
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		gisLONG From(rapidjson::Value& jsonObj);

		//================================================================================
		/// 将数据写入JSON对象
		///
		/// @param [out] jsonObj   JSON对象
		/// @param [in]  allocator JSON分配器
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		gisLONG To(rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator) const;

		//================================================================================
		/// 重置数据
		//================================================================================
		void Reset();

		//================================================================================
		/// 赋值运算符重载
		///
		/// @param [in] other 源对象
		/// @return 当前对象引用
		//================================================================================
		Ci_Tileset& operator=(const Ci_Tileset& other);
	};
};