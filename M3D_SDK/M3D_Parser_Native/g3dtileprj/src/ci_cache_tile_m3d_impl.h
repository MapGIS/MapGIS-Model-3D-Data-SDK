#pragma once

#include "ci_m3d_tile.h"
#include "ci_structure_tree.h"

#include "../include/g3dtile.h"
#define M3dTopInfoData 		"M3dTopInfoData"
static const  bool M3d20ExistI3dm = false;
namespace M3D
{
	//================================================================================
	/// M3D缓存瓦片实现类
	/// 继承自MapGIS::Tile::G3DTile，实现M3D格式瓦片的缓存操作
	//================================================================================
	class Ci_G3DM3DCacheTileImpl : public MapGIS::Tile::G3DTile
	{
	public:
		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3DCacheTileImpl();

		// 声明友元类
		friend class Ci_G3DM3DCacheTilesetImpl;
		friend class Ci_G3DM3D20CacheTilesetImpl;
		friend class Ci_G3DM3D21CacheTilesetImpl;

	protected:
		//================================================================================
		/// 构造函数
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		//================================================================================
		Ci_G3DM3DCacheTileImpl(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

	public:
		//================================================================================
		/// 设置瓦片名称
		///
		/// @param [in] tileName 瓦片名称
		//================================================================================
		virtual void SetName(CGString tileName);

		//================================================================================
		/// 获取瓦片名称
		///
		/// @return 瓦片名称
		//================================================================================
		virtual CGString GetName();

		//================================================================================
		/// 设置包围体
		///
		/// @param [in] bounding 包围体对象
		//================================================================================
		virtual void SetBounding(MapGIS::Tile::BoundingVolume& bounding);

		//================================================================================
		/// 获取包围体
		///
		/// @return 包围体对象
		//================================================================================
		virtual MapGIS::Tile::BoundingVolume GetBounding();

		//================================================================================
		/// 设置几何误差
		///
		/// @param [in] geometricError 几何误差值
		//================================================================================
		virtual void SetGeometricError(float geometricError);

		//================================================================================
		/// 获取几何误差
		///
		/// @return 几何误差值
		//================================================================================
		virtual float GetGeometricError();

		//================================================================================
		/// 设置细化类型
		///
		/// @param [in] refine 细化类型枚举值
		//================================================================================
		virtual void SetRefine(MapGIS::Tile::RefineType refine);

		//================================================================================
		/// 获取细化类型
		///
		/// @return 细化类型枚举值
		//================================================================================
		virtual MapGIS::Tile::RefineType GetRefine();

		//================================================================================
		/// 设置变换矩阵
		///
		/// @param [in] mMatrix 4x4变换矩阵
		//================================================================================
		virtual void SetMatrix(MapGIS::Tile::Matrix4D mMatrix);

		//================================================================================
		/// 获取变换矩阵
		///
		/// @return 4x4变换矩阵
		//================================================================================
		virtual MapGIS::Tile::Matrix4D GetMatrix();

		//================================================================================
		/// 判断是否为根瓦片
		///
		/// @return 是否为根瓦片的判断结果
		//================================================================================
		virtual bool IsRoot();

		//================================================================================
		/// 获取相对路径
		///
		/// @return 相对路径字符串
		//================================================================================
		virtual CGString GetRelativePath() const;

		//================================================================================
		/// 保存瓦片
		///
		/// @return 操作结果
		///         - gisLONG > 0：保存成功
		///         - gisLONG <= 0：保存失败
		//================================================================================
		virtual gisLONG Save();

		//================================================================================
		/// 关闭瓦片
		//================================================================================
		virtual void Close();

		//================================================================================
		/// 释放子对象
		//================================================================================
		virtual void ReleaseChildObjects();

		//================================================================================
		/// 获取缓存存储器
		///
		/// @return 缓存存储器指针
		//================================================================================
		virtual MapGIS::Tile::G3DCacheStorage* GetCacheStorage();

		//================================================================================
		/// 添加子瓦片
		///
		/// @param [in] mode 瓦片添加模式
		/// @return 新创建的子瓦片对象指针
		//================================================================================
		virtual G3DTile* AppendChild(MapGIS::Tile::TileAppendMode mode = MapGIS::Tile::TileAppendMode::AutoMode);

		//================================================================================
		/// 获取子瓦片数量
		///
		/// @return 子瓦片数量
		//================================================================================
		virtual gisLONG GetChildNum();

		//================================================================================
		/// 移除子瓦片
		///
		/// @param [in] pChildTile 子瓦片对象指针
		/// @return 操作结果
		///         - gisLONG > 0：移除成功
		///         - gisLONG <= 0：移除失败
		//================================================================================
		virtual gisLONG RemoveChild(G3DTile* pChildTile);

		//================================================================================
		/// 移除指定索引的子瓦片
		///
		/// @param [in] index 子瓦片索引
		/// @return 操作结果
		///         - gisLONG > 0：移除成功
		///         - gisLONG <= 0：移除失败
		//================================================================================
		virtual gisLONG RemoveChild(int index);

		//================================================================================
		/// 获取指定索引的子瓦片
		///
		/// @param [in] index 子瓦片索引
		/// @return 子瓦片对象指针
		//================================================================================
		virtual G3DTile* GetChild(int index);

		//================================================================================
		/// 获取内容信息
		///
		/// @return 瓦片内容信息对象
		//================================================================================
		virtual MapGIS::Tile::TileContentInfo GetContentInfo();

		//================================================================================
		/// 获取瓦片信息
		///
		/// @return 瓦片信息对象指针
		//================================================================================
		virtual MapGIS::Tile::G3DTileInfo* GetInfo();

	protected:
		//================================================================================
		/// 内部创建子瓦片实例（纯虚函数）
		///
		/// @return 新创建的子瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateChildInstance() = 0;

		//================================================================================
		/// 内部获取缓存类型（纯虚函数）
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType() = 0;

		//================================================================================
		/// 内部设置瓦片内容信息
		///
		/// @param [in] name          内容名称
		/// @param [in] dataType      数据类型
		/// @param [in] geometryType  几何类型
		/// @param [in] compressType  几何压缩类型
		/// @param [in] cacheType     缓存类型
		/// @param [in] hasTid        是否包含TID数据
		/// @param [in] hasAttribute  是否包含属性数据
		/// @param [in] attInContent  属性是否在内容中
		/// @param [in] contentType   内容类型
		//================================================================================
		virtual void i_SetTileContentInfo(CGString name, MapGIS::Tile::DataType dataType, MapGIS::Tile::GeometryType geometryType, MapGIS::Tile::GeoCompressType compressType, MapGIS::Tile::G3DCacheType cacheType, bool hasTid, bool hasAttribute, bool attInContent, MapGIS::Tile::ContentType contentType);

		//================================================================================
		/// 内部初始化子瓦片信息
		///
		/// @param [in] pChild 子瓦片对象指针
		//================================================================================
		void i_InitChildInfo(Ci_G3DM3DCacheTileImpl* pChild);

		//================================================================================
		/// 内部更新子瓦片信息
		///
		/// @return 操作结果
		///         - gisLONG > 0：更新成功
		///         - gisLONG <= 0：更新失败
		//================================================================================
		gisLONG i_UpdateChildInfo();

		//================================================================================
		/// 内部验证瓦片有效性
		///
		/// @return 是否有效的判断结果
		//================================================================================
		bool i_IsValid() const;

		//================================================================================
		/// 内部保存实现
		///
		/// @return 操作结果
		///         - gisLONG > 0：保存成功
		///         - gisLONG <= 0：保存失败
		//================================================================================
		virtual gisLONG i_Save();

	protected:
		CGString m_relativePath;                                // 相对路径
		M3D::Ci_Tile m_tile;                                    // M3D瓦片对象
		MapGIS::Tile::ContentType m_contentType;                // 内容类型
		MapGIS::Tile::G3DCacheStorage* m_pCacheStorage;         // 缓存存储器指针

		// 这里缓存的并不是所有子节点的对象，缓存的是用到的子节点对象。
		// 对象的顺序并不是子节点的顺序。
		vector<Ci_G3DM3DCacheTileImpl*> m_childs;               // 子瓦片对象向量
	};
	

	//================================================================================
	/// M3D 2.0缓存瓦片实现类
	/// 继承自Ci_G3DM3DCacheTileImpl，实现M3D 2.0版本瓦片的读写操作
	//================================================================================
	class Ci_G3DM3D20CacheTileImpl : public Ci_G3DM3DCacheTileImpl
	{
	public:
		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3D20CacheTileImpl();

	protected:
		//================================================================================
		/// 构造函数
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		//================================================================================
		Ci_G3DM3D20CacheTileImpl(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

		// 声明友元类
		friend class Ci_G3DM3D20CacheTilesetImpl;

	public:
		//================================================================================
		/// 内部创建子瓦片实例
		///
		/// @return 新创建的子瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateChildInstance();

		//================================================================================
		/// 内部获取缓存类型
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType();

		//================================================================================
		/// 写入内容数据
		///
		/// @param [in] contentName 内容名称
		/// @param [in] pContents   内容对象列表
		/// @param [in] param       写入参数基类引用
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteContent(CGString contentName, vector<MapGIS::Tile::ContentBase*>* pContents, MapGIS::Tile::WriteContentParamBase& param);

		//================================================================================
		/// 读取内容数据
		///
		/// @param [in] pContents 内容对象列表
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadContent(vector<MapGIS::Tile::ContentBase*>* pContents);

		//================================================================================
		/// 打开瓦片
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @param [in] relativePath  相对路径
		/// @return 瓦片对象指针
		//================================================================================
		static G3DTile* Open(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath);

	private:
		//================================================================================
		/// 写入批处理3D模型内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pModel          3D模型对象指针
		/// @param [in] pAttribute      属性数据指针（可选）
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteBatched3DModel(CGString contentName, MapGIS::Tile::G3DModel* pModel, MapGIS::Tile::Attribute* pAttribute = NULL, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::Model);

		//================================================================================
		/// 写入实例化3D模型内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pModel          3D模型对象指针
		/// @param [in] pInstances      模型实例向量指针
		/// @param [in] pAttribute      属性数据指针（可选）
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteInstanced3DModel(CGString contentName, MapGIS::Tile::G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, MapGIS::Tile::Attribute* pAttribute = NULL, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::Model);

		//================================================================================
		/// 写入点云内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pModel          点云模型对象指针
		/// @param [in] pAttribute      属性数据指针（可选）
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WritePoints(CGString contentName, MapGIS::Tile::PointsModel* pModel, MapGIS::Tile::Attribute* pAttribute = NULL, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::PointCloud);

		//================================================================================
		/// 写入组合内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pContents       内容对象列表
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteComposite(CGString contentName, vector<MapGIS::Tile::ContentBase*>* pContents, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::Model);

		//================================================================================
		/// 读取批处理3D模型内容
		///
		/// @param [in]  info       内容信息对象
		/// @param [in]  pModel     3D模型对象指针
		/// @param [out] pAttribute 属性数据指针
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadBatched3DModel(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::G3DModel* pModel, MapGIS::Tile::Attribute* pAttribute);

		//================================================================================
		/// 读取实例化3D模型内容
		///
		/// @param [in]  info        内容信息对象
		/// @param [in]  pModel      3D模型对象指针
		/// @param [out] pInstances  模型实例向量指针
		/// @param [out] pAttribute  属性数据指针
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadInstanced3DModel(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, MapGIS::Tile::Attribute* pAttribute);

		//================================================================================
		/// 读取点云内容
		///
		/// @param [in]  info       内容信息对象
		/// @param [in]  pModel     点云模型对象指针
		/// @param [out] pAttribute 属性数据指针
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadPoints(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::PointsModel* pModel, MapGIS::Tile::Attribute* pAttribute);

		//================================================================================
		/// 读取组合内容
		///
		/// @param [in]  info     内容信息对象
		/// @param [out] pContents 内容对象列表
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadComposite(MapGIS::Tile::TileContentInfo info, vector<MapGIS::Tile::ContentBase*>* pContents);
	};

	//================================================================================
	/// M3D 2.1缓存瓦片实现类
	/// 继承自Ci_G3DM3DCacheTileImpl，实现M3D 2.1版本瓦片的读写操作
	//================================================================================
	class Ci_G3DM3D21CacheTileImpl : public Ci_G3DM3DCacheTileImpl
	{
	public:
		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3D21CacheTileImpl();

	protected:
		//================================================================================
		/// 构造函数
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		//================================================================================
		Ci_G3DM3D21CacheTileImpl(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

		// 声明友元类
		friend class Ci_G3DM3D21CacheTilesetImpl;

	public:
		//================================================================================
		/// 内部创建子瓦片实例
		///
		/// @return 新创建的子瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateChildInstance();

		//================================================================================
		/// 内部获取缓存类型
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType();

		//================================================================================
		/// 写入内容数据
		///
		/// @param [in] contentName 内容名称
		/// @param [in] pContents   内容对象列表
		/// @param [in] param       写入参数基类引用
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteContent(CGString contentName, vector<MapGIS::Tile::ContentBase*>* pContents, MapGIS::Tile::WriteContentParamBase& param);

		//================================================================================
		/// 读取内容数据
		///
		/// @param [in] pContents 内容对象列表
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadContent(vector<MapGIS::Tile::ContentBase*>* pContents);

		//virtual gisLONG WriteVoxelMode(CGString contentName, MapGIS::Tile::VoxelModel* pModel, const vector<MapGIS::Tile::Attribute*>& atts);

		//virtual gisLONG ReadVoxelMode(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::VoxelModel* pModel, vector<MapGIS::Tile::Attribute*>& atts);

		//================================================================================
		/// 打开瓦片
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @param [in] relativePath  相对路径
		/// @return 瓦片对象指针
		//================================================================================
		static G3DTile* Open(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath);

	protected:
		//================================================================================
		/// 写入批处理3D模型内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pModel          3D模型对象指针
		/// @param [in] pAttribute      属性数据指针（可选）
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteBatched3DModel(CGString contentName, MapGIS::Tile::G3DModel* pModel, MapGIS::Tile::Attribute* pAttribute = NULL, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::Model);

		//================================================================================
		/// 写入实例化3D模型内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pModel          3D模型对象指针
		/// @param [in] pInstances      模型实例向量指针
		/// @param [in] pAttribute      属性数据指针（可选）
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteInstanced3DModel(CGString contentName, MapGIS::Tile::G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, MapGIS::Tile::Attribute* pAttribute = NULL, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::Model);

		//================================================================================
		/// 写入点云内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pModel          点云模型对象指针
		/// @param [in] pAttribute      属性数据指针（可选）
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WritePoints(CGString contentName, MapGIS::Tile::PointsModel* pModel, MapGIS::Tile::Attribute* pAttribute = NULL, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::PointCloud);

		//================================================================================
		/// 写入组合内容
		///
		/// @param [in] contentName     内容名称
		/// @param [in] pContents       内容对象列表
		/// @param [in] attInContent    属性是否在内容中
		/// @param [in] geoCompressType 几何压缩类型
		/// @param [in] dataType        数据类型
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteComposite(CGString contentName, vector<MapGIS::Tile::ContentBase*>* pContents, bool attInContent = false, MapGIS::Tile::GeoCompressType geoCompressType = MapGIS::Tile::GeoCompressType::None, MapGIS::Tile::DataType dataType = MapGIS::Tile::DataType::Model);

		//================================================================================
		/// 读取批处理3D模型内容
		///
		/// @param [in]  info       内容信息对象
		/// @param [in]  pModel     3D模型对象指针
		/// @param [out] pAttribute 属性数据指针
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadBatched3DModel(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::G3DModel* pModel, MapGIS::Tile::Attribute* pAttribute);

		//================================================================================
		/// 读取实例化3D模型内容
		///
		/// @param [in]  info        内容信息对象
		/// @param [in]  pModel      3D模型对象指针
		/// @param [out] pInstances  模型实例向量指针
		/// @param [out] pAttribute  属性数据指针
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadInstanced3DModel(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, MapGIS::Tile::Attribute* pAttribute);

		//================================================================================
		/// 读取点云内容
		///
		/// @param [in]  info       内容信息对象
		/// @param [in]  pModel     点云模型对象指针
		/// @param [out] pAttribute 属性数据指针
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadPoints(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::PointsModel* pModel, MapGIS::Tile::Attribute* pAttribute);

		//================================================================================
		/// 读取组合内容
		///
		/// @param [in]  info     内容信息对象
		/// @param [out] pContents 内容对象列表
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadComposite(MapGIS::Tile::TileContentInfo info, vector<MapGIS::Tile::ContentBase*>* pContents);

		//================================================================================
		/// 写入体素内容
		///
		/// @param [in] contentName   内容名称
		/// @param [in] pVoxelContent 体素内容对象指针
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteVoxelMode(CGString contentName, MapGIS::Tile::VoxelContent* pVoxelContent);

		//================================================================================
		/// 读取体素内容
		///
		/// @param [in]  info         内容信息对象
		/// @param [out] pVoxelContent 体素内容对象指针
		/// @return 操作结果
		///         - gisLONG > 0：读取成功
		///         - gisLONG <= 0：读取失败
		//================================================================================
		virtual gisLONG ReadVoxelMode(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::VoxelContent* pVoxelContent);
	};

	//================================================================================
	/// M3D 2.2缓存瓦片实现类
	/// 继承自Ci_G3DM3D21CacheTileImpl，实现M3D 2.2版本瓦片的读写操作
	//================================================================================
	class Ci_G3DM3D22CacheTileImpl : public Ci_G3DM3D21CacheTileImpl
	{
	public:
		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3D22CacheTileImpl();

	protected:
		//================================================================================
		/// 构造函数
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		//================================================================================
		Ci_G3DM3D22CacheTileImpl(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

		// 声明友元类
		friend class Ci_G3DM3D22CacheTilesetImpl;

	public:
		//================================================================================
		/// 写入内容数据
		///
		/// @param [in] contentName 内容名称
		/// @param [in] pContents   内容对象列表
		/// @param [in] param       写入参数基类引用
		/// @return 操作结果
		///         - gisLONG > 0：写入成功
		///         - gisLONG <= 0：写入失败
		//================================================================================
		virtual gisLONG WriteContent(CGString contentName, vector<MapGIS::Tile::ContentBase*>* pContents, MapGIS::Tile::WriteContentParamBase& param);

		//================================================================================
		/// 内部创建子瓦片实例
		///
		/// @return 新创建的子瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateChildInstance();

		//================================================================================
		/// 内部获取缓存类型
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType();
	};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//================================================================================
	/// M3D缓存瓦片集实现类
	/// 继承自MapGIS::Tile::G3DTileset，实现M3D格式瓦片集的缓存操作
	//================================================================================
	class Ci_G3DM3DCacheTilesetImpl : public MapGIS::Tile::G3DTileset
	{
	public:
		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_G3DM3DCacheTilesetImpl();

		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3DCacheTilesetImpl();

	public:
		//================================================================================
		/// 获取缓存存储器
		///
		/// @return 缓存存储器指针
		//================================================================================
		virtual MapGIS::Tile::G3DCacheStorage* GetCacheStorage();

		//================================================================================
		/// 获取根瓦片
		///
		/// @return 根瓦片对象指针
		//================================================================================
		virtual MapGIS::Tile::G3DTile* GetRoot();

		//================================================================================
		/// 设置GUID
		///
		/// @param [in] guid GUID对象
		//================================================================================
		virtual void SetGUID(GUID guid);

		//================================================================================
		/// 获取GUID
		///
		/// @return GUID对象
		//================================================================================
		virtual GUID GetGUID();

		//================================================================================
		/// 设置位置坐标
		///
		/// @param [in] dot 三维坐标点
		//================================================================================
		virtual void SetPosition(D_3DOT dot);

		//================================================================================
		/// 获取位置坐标
		///
		/// @return 三维坐标点
		//================================================================================
		virtual D_3DOT GetPosition();

		//================================================================================
		/// 设置树类型
		///
		/// @param [in] treeType 树类型枚举值
		//================================================================================
		virtual void SetTreeType(MapGIS::Tile::TreeType treeType);

		//================================================================================
		/// 获取树类型
		///
		/// @return 树类型枚举值
		//================================================================================
		virtual MapGIS::Tile::TreeType GetTreeType();

		//================================================================================
		/// 创建瓦片集
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @param [in] tilesetName   瓦片集名称
		/// @return 操作结果
		///         - gisLONG > 0：创建成功
		///         - gisLONG <= 0：创建失败
		//================================================================================
		virtual gisLONG Create(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString tilesetName);

		//================================================================================
		/// 打开瓦片集
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @param [in] tilesetName   瓦片集名称
		/// @return 操作结果
		///         - gisLONG > 0：打开成功
		///         - gisLONG <= 0：打开失败
		//================================================================================
		virtual gisLONG Open(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString tilesetName);

		//================================================================================
		/// 保存瓦片集
		///
		/// @return 操作结果
		///         - gisLONG > 0：保存成功
		///         - gisLONG <= 0：保存失败
		//================================================================================
		virtual gisLONG Save();

		//================================================================================
		/// 关闭瓦片集
		///
		/// @return 操作结果
		///         - gisLONG > 0：关闭成功
		///         - gisLONG <= 0：关闭失败
		//================================================================================
		virtual gisLONG Close();

		//================================================================================
		/// 验证瓦片集有效性
		///
		/// @return 是否有效的判断结果
		//================================================================================
		virtual bool IsValid();

		//================================================================================
		/// 开始写入SQLite数据库
		///
		/// @return 操作结果
		///         - gisLONG > 0：开始成功
		///         - gisLONG <= 0：开始失败
		//================================================================================
		virtual gisLONG StartWriteSqlite() { return 0; };

		//================================================================================
		/// 设置SQLite数据库中的包围盒信息
		/// 将TID与对应包围盒信息的映射关系写入SQLite数据库中
		///
		/// @param [in] tid2Box TID到包围盒信息的映射表，其中：
		///                     - key (gisINT64): TID标识符
		///                     - value (vector<double>):笛卡尔坐标系下的包围盒信息，包含6个值表示(xmin, ymin, zmin, xmax, ymax, zmax)
		//================================================================================
		virtual void SetBoxInfoInSqlite(const map<gisINT64, vector<double> >& tid2Box) {};

		//================================================================================
		/// 结束写入SQLite数据库
		///
		/// @return 操作结果
		///         - gisLONG > 0：结束成功
		///         - gisLONG <= 0：结束失败
		//================================================================================
		virtual gisLONG EndWriteSqlite() { return 0; };

		//================================================================================
		/// 获取瓦片集信息
		///
		/// @return 瓦片集信息对象指针
		//================================================================================
		virtual MapGIS::Tile::G3DTilesetInfo* GetInfo();

		//================================================================================
		/// 设置结构树信息
		///
		/// @param [in] rootNode 根节点对象引用
		/// @return 操作结果
		///         - gisLONG > 0：设置成功
		///         - gisLONG <= 0：设置失败
		//================================================================================
		virtual gisLONG SetStructureTreeInfo(MapGIS::Tile::StructureTreeNode& rootNode) { return 0; };

		//================================================================================
		/// 获取结构树信息
		///
		/// @param [in] rootNode 根节点对象引用
		/// @return 操作结果
		///         - gisLONG > 0：获取成功
		///         - gisLONG <= 0：获取失败
		//================================================================================
		virtual gisLONG GetStructureTreeInfo(MapGIS::Tile::StructureTreeNode& rootNode) { return 0; };

	protected:
		//================================================================================
		/// 内部创建根瓦片（纯虚函数）
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @return 根瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateRootTile(MapGIS::Tile::G3DCacheStorage* pCacheStorage) = 0;

		//================================================================================
		/// 内部获取缓存类型（纯虚函数）
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType() = 0;

		//================================================================================
		/// 内部获取瓦片集相对路径
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @param [in] tilesetName   瓦片集名称
		/// @return 相对路径字符串
		//================================================================================
		CGString i_GetTilesetRelative(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString tilesetName);

		CGString m_tilesetName;                                 // 瓦片集名称
		M3D::Ci_Tileset m_tileset;                              // M3D瓦片集对象
		Ci_G3DM3DCacheTileImpl* m_pRootTile;                    // 根瓦片对象指针
		MapGIS::Tile::G3DCacheStorage* m_pCacheStorage;         // 缓存存储器指针
	};

	//================================================================================
	/// M3D 2.0缓存瓦片集实现类
	/// 继承自Ci_G3DM3DCacheTilesetImpl，实现M3D 2.0版本瓦片集的操作
	//================================================================================
	class Ci_G3DM3D20CacheTilesetImpl : public Ci_G3DM3DCacheTilesetImpl
	{
	public:
		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_G3DM3D20CacheTilesetImpl();

		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3D20CacheTilesetImpl();

	public:
		//================================================================================
		/// 清除图层字段信息
		//================================================================================
		virtual void ClearLayerFieldsInfo();

		//================================================================================
		/// 设置图层字段信息
		///
		/// @param [in] pLayersInfo 图层信息基类对象指针
		/// @return 操作结果
		///         - gisLONG > 0：设置成功
		///         - gisLONG <= 0：设置失败
		//================================================================================
		virtual gisLONG SetLayerFieldsInfo(const MapGIS::Tile::LayersInfoBase* pLayersInfo);

		//================================================================================
		/// 获取图层字段信息
		///
		/// @param [in] pLayersInfo 图层信息基类对象指针
		/// @return 操作结果
		///         - gisLONG > 0：获取成功
		///         - gisLONG <= 0：获取失败
		//================================================================================
		virtual gisLONG GetLayerFieldsInfo(MapGIS::Tile::LayersInfoBase* pLayersInfo);

	private:
		//================================================================================
		/// 内部创建根瓦片
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @return 根瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateRootTile(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

		//================================================================================
		/// 内部获取缓存类型
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType();
	};

	//================================================================================
	/// M3D 2.1缓存瓦片集实现类
	/// 继承自Ci_G3DM3DCacheTilesetImpl，实现M3D 2.1版本瓦片集的操作
	//================================================================================
	class Ci_G3DM3D21CacheTilesetImpl : public Ci_G3DM3DCacheTilesetImpl
	{
	public:
		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_G3DM3D21CacheTilesetImpl();

		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3D21CacheTilesetImpl();

	public:
		//================================================================================
		/// 清除图层字段信息
		//================================================================================
		virtual void ClearLayerFieldsInfo();

		//================================================================================
		/// 设置图层字段信息
		///
		/// @param [in] pLayersInfo 图层信息基类对象指针
		/// @return 操作结果
		///         - gisLONG > 0：设置成功
		///         - gisLONG <= 0：设置失败
		//================================================================================
		virtual gisLONG SetLayerFieldsInfo(const MapGIS::Tile::LayersInfoBase* pLayersInfo);

		//================================================================================
		/// 获取图层字段信息
		///
		/// @param [in] pLayersInfo 图层信息基类对象指针
		/// @return 操作结果
		///         - gisLONG > 0：获取成功
		///         - gisLONG <= 0：获取失败
		//================================================================================
		virtual gisLONG GetLayerFieldsInfo(MapGIS::Tile::LayersInfoBase* pLayersInfo);

		//================================================================================
		/// 设置图层信息
		///
		/// @param [in] layerInfo 图层信息字节数组
		/// @return 操作结果
		///         - gisLONG > 0：设置成功
		///         - gisLONG <= 0：设置失败
		//================================================================================
		virtual gisLONG SetLayerInfo(const CGByteArray& layerInfo);

		//================================================================================
		/// 获取图层信息
		///
		/// @param [out] layerInfo 图层信息字节数组
		/// @return 操作结果
		///         - gisLONG > 0：获取成功
		///         - gisLONG <= 0：获取失败
		//================================================================================
		virtual gisLONG GetLayerInfo(CGByteArray& layerInfo);

		//================================================================================
		/// 设置结构树信息
		///
		/// @param [in] rootNode 结构树根节点对象引用
		/// @return 操作结果
		///         - gisLONG > 0：设置成功
		///         - gisLONG <= 0：设置失败
		//================================================================================
		virtual gisLONG SetStructureTreeInfo(MapGIS::Tile::StructureTreeNode& rootNode);

		//================================================================================
		/// 获取结构树信息
		///
		/// @param [in] rootNode 结构树根节点对象引用
		/// @return 操作结果
		///         - gisLONG > 0：获取成功
		///         - gisLONG <= 0：获取失败
		//================================================================================
		virtual gisLONG GetStructureTreeInfo(MapGIS::Tile::StructureTreeNode& rootNode);

	private:
		//================================================================================
		/// 内部创建根瓦片
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @return 根瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateRootTile(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

		//================================================================================
		/// 内部获取缓存类型
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType();
	};

	//================================================================================
	/// M3D 2.2缓存瓦片集实现类
	/// 继承自Ci_G3DM3D21CacheTilesetImpl，实现M3D 2.2版本瓦片集的操作
	//================================================================================
	class Ci_G3DM3D22CacheTilesetImpl : public Ci_G3DM3D21CacheTilesetImpl
	{
	public:
		//================================================================================
		/// 默认构造函数
		//================================================================================
		Ci_G3DM3D22CacheTilesetImpl();

		//================================================================================
		/// 虚析构函数
		//================================================================================
		virtual ~Ci_G3DM3D22CacheTilesetImpl();

		//================================================================================
		/// 开始写入SQLite数据库
		///
		/// @return 操作结果
		///         - gisLONG > 0：开始成功
		///         - gisLONG <= 0：开始失败
		//================================================================================
		virtual gisLONG StartWriteSqlite();

		//================================================================================
		/// 设置SQLite数据库中的包围盒信息
		/// 将TID与对应包围盒信息的映射关系写入SQLite数据库中
		///
		/// @param [in] tid2Box TID到包围盒信息的映射表，其中：
		///                     - key (gisINT64): TID标识符
		///                     - value (vector<double>):笛卡尔坐标系下的包围盒信息，包含6个值表示(xmin, ymin, zmin, xmax, ymax, zmax)
		//================================================================================
		virtual void SetBoxInfoInSqlite(const map<gisINT64, vector<double> >& tid2Box);
	
		//================================================================================
		/// 结束写入SQLite数据库
		///
		/// @return 操作结果
		///         - gisLONG > 0：结束成功
		///         - gisLONG <= 0：结束失败
		//================================================================================
		virtual gisLONG EndWriteSqlite();

	private:
		//================================================================================
		/// 内部创建根瓦片
		///
		/// @param [in] pCacheStorage 缓存存储器指针
		/// @return 根瓦片对象指针
		//================================================================================
		virtual Ci_G3DM3DCacheTileImpl* i_CreateRootTile(MapGIS::Tile::G3DCacheStorage* pCacheStorage);

		//================================================================================
		/// 内部获取缓存类型
		///
		/// @return 缓存类型枚举值
		//================================================================================
		virtual MapGIS::Tile::G3DCacheType i_GetCacheType();
	};

}
//================================================================================
/// 从所有内容中获取数据属性
///
/// @param [in]  pContents     内容对象列表
/// @param [out] pAllAttribute 输出的所有属性数据
/// @return 操作结果
///         - gisLONG > 0：获取成功
///         - gisLONG <= 0：获取失败
//================================================================================
gisLONG GetDataAttributeByAllAttribute(vector<MapGIS::Tile::ContentBase*>* pContents, MapGIS::Tile::Attribute* pAllAttribute);
