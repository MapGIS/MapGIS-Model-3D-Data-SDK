#pragma once
#include "ci_3dmodel_3dtile.h"
#include "ci_cache_storage.h"
#include "../include/g3dtile.h"

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 模型基类
		//================================================================================
		class Ci_TileModel
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_TileModel() { m_pStorage = NULL; };

			//================================================================================
			/// 虚析构函数
			//================================================================================
			virtual ~Ci_TileModel() {};

		public:
			//================================================================================
			/// 从字节数组加载数据
			///
			/// @param [in] value 输入字节数组
			/// @return 操作结果
			///         - gisLONG > 0：加载成功
			///         - gisLONG <= 0：加载失败
			//================================================================================
			virtual gisLONG From(const CGByteArray& value) { m_data = value; return 1; };

			//================================================================================
			/// 将数据保存到字节数组
			///
			/// @param [out] value 输出字节数组
			/// @return 操作结果
			///         - gisLONG > 0：保存成功
			///         - gisLONG <= 0：保存失败
			//================================================================================
			virtual gisLONG To(CGByteArray& value) { value = m_data; return 1; };

			//================================================================================
			/// 获取数据字节数组
			///
			/// @return 数据字节数组指针
			//================================================================================
			virtual const CGByteArray* Get() { return &m_data; };

			//================================================================================
			/// 设置缓存存储器
			/// 暂时用来读取相对路径下的纹理图片数据，或流信息
			///
			/// @param [in] pStorage 缓存存储器指针
			/// @return 操作结果
			///         - gisLONG > 0：设置成功
			///         - gisLONG <= 0：设置失败
			//================================================================================
			gisLONG SetCacheStorage(G3DCacheStorage* pStorage) { m_pStorage = pStorage; return 1; }

		protected:
			CGByteArray m_data;                 // 数据字节数组
			G3DCacheStorage* m_pStorage;        // 缓存存储器指针
		};

		//================================================================================
		/// GLTF写ID类型枚举
		/// 定义GLTF模型写入ID的类型
		//================================================================================
		enum class WriteIdType : int
		{
			// 不写ID
			None = 0,
			// OID
			oid = 1,
			// BatchID
			batchID = 2,
		};

		//================================================================================
		/// GLTF模型类
		/// 继承自Ci_TileModel，实现GLTF格式模型的读写操作
		//================================================================================
		class Ci_ModelGltf : public Ci_TileModel
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_ModelGltf() {};

			//================================================================================
			/// 析构函数
			//================================================================================
			~Ci_ModelGltf() {};

			//================================================================================
			/// 将模型数据转换为GLTF流
			///
			/// @param [in]  pModel         几何数据
			/// @param [in]  compressType   几何压缩模式
			/// @param [in]  type           ID写入类型
			/// @param [in]  centerDot      中心点坐标
			/// @param [in]  imageInData    是否将只有URI的数据嵌入到GLB中（需设置m_pStorage才有效）
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::G3DModel* pModel, GeoCompressType compressType, WriteIdType type, D_3DOT centerDot = { 0,0,0 }, bool imageInData = false);

			//================================================================================
			/// 将模型数据转换为GLTF流（ID从零重新编号）
			///
			/// @param [in]     pModel         几何数据
			/// @param [in]     compressType   几何压缩模式
			/// @param [in]     type           ID写入类型
			/// @param [in,out] resetIDMap     ID与重新编号的对应表
			/// @param [in]     centerDot      中心点坐标
			/// @param [in]     imageInData    是否将只有URI的数据嵌入到GLB中（需设置m_pStorage才有效）
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG FromResetID(MapGIS::Tile::G3DModel* pModel, GeoCompressType compressType, WriteIdType type, map<gisINT64, gisINT64>& resetIDMap, D_3DOT centerDot = { 0,0,0 }, bool imageInData = false);

			//================================================================================
			/// 将GLTF流转换为模型数据（不考虑实例信息）
			///
			/// @param [in,out] pModel 模型数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::G3DModel* pModel);

			//================================================================================
			/// 将模型数据转换为GLTF流
			///
			/// @param [in]  pModel                    几何数据
			/// @param [in]  compressType              几何压缩模式
			/// @param [in]  type                      ID写入类型
			/// @param [in]  addEXT_mesh_gpu_instancing 是否添加GPU实例化扩展
			/// @param [in]  centerDot                 中心点坐标
			/// @param [in]  imageInData               是否将只有URI的数据嵌入到GLB中（需设置m_pStorage才有效）
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(vector<ContentBase*>* pModel, GeoCompressType compressType, WriteIdType type, bool addEXT_mesh_gpu_instancing = true, D_3DOT centerDot = { 0,0,0 }, bool imageInData = false);

			//================================================================================
			/// 将模型数据转换为GLTF流（ID从零重新编号）
			///
			/// @param [in]     pModel                    几何数据
			/// @param [in]     compressType              几何压缩模式
			/// @param [in]     type                      ID写入类型
			/// @param [in,out] resetIDMap                ID与重新编号的对应表
			/// @param [in]     centerDot                 中心点坐标
			/// @param [in]     addEXT_mesh_gpu_instancing 是否添加GPU实例化扩展
			/// @param [in]     imageInData               是否将只有URI的数据嵌入到GLB中（需设置m_pStorage才有效）
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG FromResetID(vector<ContentBase*>* pModel, GeoCompressType compressType, WriteIdType type, map<gisINT64, gisINT64>& resetIDMap, D_3DOT centerDot = { 0,0,0 }, bool addEXT_mesh_gpu_instancing = true, bool imageInData = false);

			//================================================================================
			/// 将GLTF流转换为模型数据
			///
			/// @param [in,out] pModel 模型数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(vector<ContentBase*>* pModel);

			//================================================================================
			/// 将高斯溅射数据转换为GLTF流
			///
			/// @param [in]  pModel       高斯溅射数据
			/// @param [in]  mode         高斯扩展模式
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG FromGaussian(GaussianContent* pModel, GaussianExtMode mode);

			//================================================================================
			/// 将GLTF流转换为高斯溅射数据
			///
			/// @param [in,out] pModel 高斯溅射数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG ToGaussian(GaussianContent* pModel);

		private:
			//================================================================================
			/// 内部实现：将模型数据转换为GLTF流
			///
			/// @param [in]     pModel         几何数据
			/// @param [in]     compressType   几何压缩模式
			/// @param [in]     type           ID写入类型
			/// @param [in]     resetID        是否重新编号ID
			/// @param [in,out] resetIDMap     ID与重新编号的对应表
			/// @param [in]     centerDot      中心点坐标
			/// @param [in]     imageInData    是否将只有URI的数据嵌入到GLB中（需设置m_pStorage才有效）
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::G3DModel* pModel, GeoCompressType compressType, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, D_3DOT centerDot = { 0,0,0 }, bool imageInData = false);

			//================================================================================
			/// 内部实现：将模型数据转换为GLTF流
			///
			/// @param [in]     pModel                    几何数据
			/// @param [in]     compressType              几何压缩模式
			/// @param [in]     type                      ID写入类型
			/// @param [in]     resetID                   是否重新编号ID
			/// @param [in,out] resetIDMap                ID与重新编号的对应表
			/// @param [in]     centerDot                 中心点坐标
			/// @param [in]     addEXT_mesh_gpu_instancing 是否添加GPU实例化扩展
			/// @param [in]     imageInData               是否将只有URI的数据嵌入到GLB中（需设置m_pStorage才有效）
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(vector<ContentBase*>* pModel, GeoCompressType compressType, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, D_3DOT centerDot = { 0,0,0 }, bool addEXT_mesh_gpu_instancing = true, bool imageInData = false);

			//================================================================================
			/// 内部实现：将GLTF流转换为实体模型数据
			///
			/// @param [in,out] entityModel 实体模型数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::EntitiesModel& entityModel);

			//================================================================================
			/// 内部实现：将GLTF流转换为面模型数据
			///
			/// @param [in,out] surfaceModel 面模型数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::SurfacesModel& surfaceModel);

			//================================================================================
			/// 内部实现：将GLTF流转换为线模型数据
			///
			/// @param [in,out] lineModel 线模型数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::LinesModel& lineModel);

			//================================================================================
			/// 内部实现：将GLTF流转换为点模型数据
			///
			/// @param [in,out] lineModel 点模型数据
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::PointsModel& lineModel);
		};

		//================================================================================
		/// PNTS模型类
		/// 继承自Ci_TileModel，实现PNTS格式点云数据的读写操作
		//================================================================================
		class Ci_ModelPnts : public Ci_TileModel
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_ModelPnts() {};

			//================================================================================
			/// 析构函数
			//================================================================================
			~Ci_ModelPnts() {};

			//================================================================================
			/// 将点云数据转换为PNTS流
			///
			/// @param [in]     pointModel      点云数据
			/// @param [in]     pRecords        属性数据
			/// @param [in]     isDracoCompress 是否进行Draco压缩
			/// @param [in]     pRtcCenter      RTC中心点
			/// @param [in,out] resetIDMap      ID与重新编号的对应表
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::PointsModel& pointModel, LayerAttribute* pRecords, bool isDracoCompress, D_3DOT* pRtcCenter);

			//================================================================================
			/// 将点云数据转换为PNTS流（专供M3D 2.1与3D Tiles 1.0 Ext使用）
			///
			/// @param [in]  pointModel      点云数据
			/// @param [in]  isDracoCompress 是否进行Draco压缩
			/// @param [in]  pRtcCenter      RTC中心点
			/// @param [in,out] batchIDToId  BatchID与ID的对应关系，vector的索引号就是batchid
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::PointsModel& pointModel, bool isDracoCompress, D_3DOT* pRtcCenter, vector<gisINT64>& batchIDToId);

			//================================================================================
			/// 将PNTS流转换为点云数据
			///
			/// @param [in]  pointModel 点云数据
			/// @param [in]  records    属性数据
			/// @param [in]  rtcCenter  RTC中心点
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::PointsModel& pointModel, LayerAttribute& records, D_3DOT& rtcCenter);
		};
		
		//================================================================================
		/// I3DM模型类
		/// 继承自Ci_TileModel，实现I3DM格式实例化模型数据的读写操作
		//================================================================================
		class Ci_ModelI3dm : public Ci_TileModel
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_ModelI3dm() {};

			//================================================================================
			/// 析构函数
			//================================================================================
			~Ci_ModelI3dm() {};

			//================================================================================
			/// 将模型数据转换为I3DM流
			///
			/// @param [in]  pModel         模型数据
			/// @param [in]  infos          实例化信息
			/// @param [in]  pRecords       属性数据
			/// @param [in]  compressType   几何压缩类型
			/// @param [in]  pRtcCenter     RTC中心点
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::G3DModel* pModel, const vector<MapGIS::Tile::ModelInstance>& infos, LayerAttribute* pRecords, GeoCompressType compressType, D_3DOT* pRtcCenter);

			//================================================================================
			/// 将模型数据转换为I3DM流（专供M3D 2.1 与3D Tiles 1.0 Ext使用）
			///
			/// @param [in]  pModel         模型数据
			/// @param [in]  infos          实例化信息
			/// @param [in]  compressType   几何压缩类型
			/// @param [in]  pRtcCenter     RTC中心点
			/// @param [in]  resetID        是否重置ID
			/// @param [in]  idType         ID写入类型
			/// @param [in,out] batchIDToId batchid与ID的对应关系，vector的索引号就是batchid
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::G3DModel* pModel, const vector<MapGIS::Tile::ModelInstance>& infos, GeoCompressType compressType, D_3DOT* pRtcCenter, bool resetID, WriteIdType idType, vector<gisINT64>& batchIDToId);

			//================================================================================
			/// 将I3DM流转换为模型数据
			///
			/// @param [in]     pModel     模型数据
			/// @param [in,out] infos      实例化信息
			/// @param [in]     records    属性数据
			/// @param [in]     rtcCenter  RTC中心点
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>& infos, LayerAttribute& records, D_3DOT& rtcCenter);
		};

		//================================================================================
		/// B3DM模型类
		/// 继承自Ci_TileModel，实现B3DM格式批处理模型数据的读写操作
		//================================================================================
		class Ci_ModelB3dm : public Ci_TileModel
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_ModelB3dm() {};

			//================================================================================
			/// 析构函数
			//================================================================================
			~Ci_ModelB3dm() {};

			//================================================================================
			/// 将模型数据转换为B3DM流
			///
			/// @param [in]  pModel         模型数据
			/// @param [in]  pRecords       属性数据
			/// @param [in]  compressType   几何压缩类型
			/// @param [in]  m_pRtcCenter   RTC中心点
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::G3DModel* pModel, LayerAttribute* pRecords, GeoCompressType compressType, D_3DOT* m_pRtcCenter);

			//================================================================================
			/// 将模型数据转换为B3DM流（专供M3D 2.1 与3D Tiles 1.0 Ext使用）
			///
			/// @param [in]  pModel         模型数据
			/// @param [in]  compressType   几何压缩类型
			/// @param [in]  m_pRtcCenter   RTC中心点
			/// @param [in]  resetID        是否重置ID
			/// @param [in,out] batchIDToId batchid与ID的对应关系，vector的索引号就是batchid
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(MapGIS::Tile::G3DModel* pModel, GeoCompressType compressType, D_3DOT* m_pRtcCenter, bool resetID, vector<gisINT64>& batchIDToId);

			//================================================================================
			/// 将B3DM流转换为模型数据
			///
			/// @param [in]  pModel     模型数据
			/// @param [in]  records    属性数据
			/// @param [in]  rtcCenter  RTC中心点
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(MapGIS::Tile::G3DModel* pModel, LayerAttribute& records, D_3DOT& rtcCenter);
		};

		//================================================================================
		/// CMPT模型类
		/// 继承自Ci_TileModel，实现CMPT格式组合模型数据的读写操作
		//================================================================================
		class Ci_ModelCmpt : public Ci_TileModel
		{
		public:
			//================================================================================
			/// 默认构造函数
			//================================================================================
			Ci_ModelCmpt() { };

			//================================================================================
			/// 析构函数
			//================================================================================
			~Ci_ModelCmpt() {};

			//================================================================================
			/// 将组合数据转换为CMPT流
			///
			/// @param [in]  pContents      组合数据信息
			/// @param [in]  compressType   几何压缩类型
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(vector<ContentBase*>* pContents, GeoCompressType compressType);

			//================================================================================
			/// 将组合数据流转换为CMPT流
			///
			/// @param [in]  arrays     I3DM/B3DM流数组
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG From(vector<CGByteArray>& arrays);

			//================================================================================
			/// 将CMPT流转换为组合数据
			///
			/// @param [in]  pContents 组合数据信息
			///
			/// @return 操作结果
			///         - gisLONG > 0：转换成功
			///         - gisLONG <= 0：转换失败
			//================================================================================
			gisLONG To(vector<ContentBase*>* pContents);
		};
	}
}