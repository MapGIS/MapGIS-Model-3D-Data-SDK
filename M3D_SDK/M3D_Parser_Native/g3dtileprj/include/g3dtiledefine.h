#pragma once
#ifndef _G3D_TILE_DEFINE_H_
#define _G3D_TILE_DEFINE_H_

#include <string.h>
#include "cgstring.h"
#include "basdefine70.h"

// 平台相关的导出/导入宏定义
#ifdef _WIN32
#ifdef _MAPGIS_G3D_TILE_EXPORT_
#define MAPGISG3DTILEEXPORT  __declspec(dllexport)
#else
#define MAPGISG3DTILEEXPORT  __declspec(dllimport)
#endif
#else
#define MAPGISG3DTILEEXPORT __attribute__((visibility("default")))
#endif

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 缓存类型枚举
		/// 定义了支持的不同数据格式版本类型
		//================================================================================
		enum class G3DCacheType : int {
			TypeM3DV20 = 0,			// M3D 2.0版本
			TypeM3DV21 = 1,			// M3D 2.1版本
			TypeM3DV22 = 2,			// M3D 2.2版本
			Type3DTilesV10 = 10,	// 3D Tiles 1.0标准版本
		};

		//================================================================================
		/// 几何压缩类型枚举
		/// 定义了几何数据支持的压缩算法类型
		//================================================================================
		enum class GeoCompressType :int {
			None = 0,		// 不压缩
			Draco = 1,		// Draco压缩算法
			MeshOpt = 2		// MeshOpt压缩算法
		};

		//================================================================================
		/// 瓦片加载模式枚举
		/// 定义了瓦片细化时的加载策略
		//================================================================================
		enum class RefineType :int
		{
			None = 0, 		// 未设置加载模式
			Add = 1,		// 追加模式（显示当前瓦片和子瓦片）
			Replace = 2,	// 替换模式（用子瓦片替换当前瓦片）
		};

		//================================================================================
		/// 瓦片内容格式枚举
		/// 定义了瓦片支持的不同内容格式类型
		//================================================================================
		enum class ContentType :int
		{
			None = -1,		// 未知格式

			GLB = 0,		// glb格式（3d tiles瓦片使用）
			B3DM = 1,		// B3DM格式（3d tiles瓦片使用）
			I3DM = 2,		// I3DM格式（3d tiles瓦片使用）
			PNTS = 3,		// PNTS格式（3d tiles瓦片使用）
			CMPT = 4,		// CMPT格式（3d tiles瓦片使用）

			M3DGLB = 10,	// glb格式（m3d瓦片使用，几何内容转换为该格式数据压缩在.m3d文件内部，属性内容写为json或att文件存放在.m3d文件内部）
			M3DB3DM = 11,	// B3DM格式（m3d瓦片使用，几何内容转换为该格式数据压缩在.m3d文件内部，属性内容写为json或att文件存放在.m3d文件内部）
			M3DI3DM = 12,	// I3DM格式（m3d瓦片使用，几何内容转换为该格式数据压缩在.m3d文件内部，属性内容写为json或att文件存放在.m3d文件内部）
			M3DPNTS = 13,	// PNTS格式（m3d瓦片使用，几何内容转换为该格式数据压缩在.m3d文件内部，属性内容写为json或att文件存放在.m3d文件内部）
			M3DCMPT = 14,	// CMPT格式（m3d瓦片使用，几何内容转换为该格式数据压缩在.m3d文件内部，属性内容写为json或att文件存放在.m3d文件内部）
			M3DVoxel = 15,	// voxel格式（m3d瓦片使用，栅格体元的范围及时间规划写在瓦片json中，各个时间点的属性写为att文件）
		};

		//================================================================================
		/// 数据类型枚举
		/// 定义了支持的不同数据内容类型
		//================================================================================
		enum class DataType :int
		{
			None = 0,				// 未知类型
			Vector = 1,				// 矢量数据
			TiltPhotography = 2,	// 倾斜摄影数据
			Model = 3,				// 模型数据
			BIM = 4,				// BIM数据
			PointCloud = 5,			// 点云数据
			PipeLine = 6,			// 管线数据
			GeoModel = 7,			// 地质体数据
			GeoGrid = 8,			// 地质体网格数据
			GeoDrill = 9,			// 地质钻孔数据
			GeoSection = 10,		// 地质剖面数据
			Voxel = 11,				// 栅格体元数据
			GaussianSplatting = 12,	// 高斯溅射数据
		};

		//================================================================================
		/// 目录树类型枚举
		/// 定义了空间索引支持的不同树结构类型
		//================================================================================
		enum class TreeType :int
		{
			QuadTree = 0,		// 四叉树结构
			Octree = 1,			// 八叉树结构
			KDTree = 2,			// K-D树结构
			RTree = 3,			// R树结构
		};

		//================================================================================
		/// 属性文件类型枚举
		/// 定义了属性数据支持的文件格式类型
		//================================================================================
		enum class AttFileType :int
		{
			JSON = 0,		// JSON格式文件
			//bin = 1,		// BIN格式文件（已废弃，未支持）
			ATT = 2,		// ATT格式文件
		};

		//================================================================================
		/// 外包范围类型枚举
		/// 定义了几何对象外包范围的不同表示方式
		//================================================================================
		enum class BoundingType :int
		{
			None = 0,		// 无外包范围
			Region = 1,		// 地理范围（经纬度表示的矩形区域）
			Box = 2,		// 有向包围盒（中心点+三个轴向量）
			Sphere = 3,		// 外包球（中心点+半径）
		};

		//================================================================================
		/// 有向包围盒结构体
		/// 使用中心点和三个轴向量定义一个有向包围盒
		//================================================================================
		struct OrientedBoundingBox
		{
			D_3DOT center;	// 包围盒中心点坐标
			D_3DOT x_axis;	// X轴方向向量和长度
			D_3DOT y_axis;	// Y轴方向向量和长度
			D_3DOT z_axis;	// Z轴方向向量和长度

			//================================================================================
			/// 默认构造函数
			/// 初始化所有成员变量为零值
			//================================================================================
			OrientedBoundingBox()
			{
				memset(&center, 0, sizeof(D_3DOT));
				memset(&x_axis, 0, sizeof(D_3DOT));
				memset(&y_axis, 0, sizeof(D_3DOT));
				memset(&z_axis, 0, sizeof(D_3DOT));
			};

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] box 源包围盒对象
			/// @return 当前对象引用
			//================================================================================
			OrientedBoundingBox& operator=(const OrientedBoundingBox& box)
			{
				if (this != &box)
				{
					this->center.x = box.center.x;
					this->center.y = box.center.y;
					this->center.z = box.center.z;

					this->x_axis.x = box.x_axis.x;
					this->x_axis.y = box.x_axis.y;
					this->x_axis.z = box.x_axis.z;

					this->y_axis.x = box.y_axis.x;
					this->y_axis.y = box.y_axis.y;
					this->y_axis.z = box.y_axis.z;

					this->z_axis.x = box.z_axis.x;
					this->z_axis.y = box.z_axis.y;
					this->z_axis.z = box.z_axis.z;
				}
				return *this;
			}
		};

		//================================================================================
		/// 外包球结构体
		/// 使用中心点和半径定义一个球形外包范围
		//================================================================================
		struct BoundingSphere
		{
			D_3DOT center;	// 球心坐标
			double radius;	// 球半径

			//================================================================================
			/// 默认构造函数
			/// 初始化所有成员变量为零值
			//================================================================================
			BoundingSphere()
			{
				memset(&center, 0, sizeof(D_3DOT));
				radius = 0;
			};

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] sphere 源球体对象
			/// @return 当前对象引用
			//================================================================================
			BoundingSphere& operator=(const BoundingSphere& sphere)
			{
				if (this != &sphere)
				{
					this->center.x = sphere.center.x;
					this->center.y = sphere.center.y;
					this->center.z = sphere.center.z;
					this->radius = sphere.radius;
				}
				return *this;
			}
		};

		//================================================================================
		/// 地理范围结构体
		/// 使用经纬度和高度定义一个地理外包范围
		//================================================================================
		struct BoundingRegion
		{
			double west;		// 西经边界（度）
			double south;		// 南纬边界（度）
			double east;		// 东经边界（度）
			double north;		// 北纬边界（度）
			double minHeight;	// 最小高度（米）
			double maxHeight;	// 最大高度（米）

			//================================================================================
			/// 默认构造函数
			/// 初始化所有成员变量为零值
			//================================================================================
			BoundingRegion()
			{
				west = 0;
				south = 0;
				east = 0;
				north = 0;
				minHeight = 0;
				maxHeight = 0;
			};

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] region 源地理范围对象
			/// @return 当前对象引用
			//================================================================================
			BoundingRegion& operator=(const BoundingRegion& region)
			{
				if (this != &region)
				{
					this->west = region.west;
					this->south = region.south;
					this->east = region.east;
					this->north = region.north;
					this->minHeight = region.minHeight;
					this->maxHeight = region.maxHeight;
				}
				return *this;
			}
		};

		//================================================================================
		/// 外包范围结构体
		/// 使用联合体存储不同类型的外包范围信息
		//================================================================================
		struct BoundingVolume
		{
			BoundingType type;				// 外包范围类型标识
			union
			{
				BoundingSphere sphere;		// 外包球
				OrientedBoundingBox box;	// 有向包围盒
				BoundingRegion  region;		// 地理范围
			};

			//================================================================================
			/// 默认构造函数
			/// 初始化类型为None
			//================================================================================
			BoundingVolume()
			{
				type = BoundingType::None;
			}

			//================================================================================
			/// 赋值运算符重载
			/// 根据类型实现对应外包范围的深拷贝操作
			///
			/// @param [in] boundingVolume 源外包范围对象
			/// @return 当前对象引用
			//================================================================================
			BoundingVolume& operator=(const BoundingVolume& boundingVolume)
			{
				if (this != &boundingVolume)
				{
					this->type = boundingVolume.type;
					switch (type)
					{
					case MapGIS::Tile::BoundingType::None:
						this->sphere = boundingVolume.sphere;
						break;
					case MapGIS::Tile::BoundingType::Region:
						this->region = boundingVolume.region;
						break;
					case MapGIS::Tile::BoundingType::Box:
						this->box = boundingVolume.box;
						break;
					case MapGIS::Tile::BoundingType::Sphere:
						this->sphere = boundingVolume.sphere;
						break;
					default:
						break;
					}
				}
				return *this;
			}
		};

		//================================================================================
		/// M3D版本枚举
		/// 定义了M3D格式支持的不同版本
		//================================================================================
		enum class M3DVersion :int
		{
			M3D20 = 0, // M3D 2.0版本
			M3D21 = 1,	// M3D 2.1版本
			M3D22 = 2,	// M3D 2.2版本
		};

		//================================================================================
		/// 文件压缩类型枚举
		/// 定义了支持的文件压缩算法类型（当前仅支持zip）
		//================================================================================
		enum class FileCompressType :int
		{
			Zip = 0,		// ZIP压缩格式
			//_7z =1,		// 7Z压缩格式（未支持）
			//rar =2,		// RAR压缩格式（未支持）
		};

		//================================================================================
		/// 空间参考系枚举
		/// 定义了支持的不同空间参考系类型
		//================================================================================
		enum class SpatialReference :int
		{
			WGS84 = 0,		// WGS84坐标系
			CGCS2000 = 1,	// CGCS2000坐标系（中国大地坐标系2000）
		};

		//================================================================================
		/// LOD切换模式枚举
		/// 定义了细节层次切换的判断标准
		//================================================================================
		enum class LodMode :int
		{
			Distance = 0,	// 距离切换模式（根据视点距离判断）
			Pixel = 1,		// 像素切换模式（根据屏幕像素大小判断）
			None = 10,		// 不设置切换模式
		};

		//================================================================================
		/// 高斯数据扩展类型枚举
		/// 定义了高斯溅射数据支持的扩展格式类型
		//================================================================================
		enum class GaussianExtMode : int {
			//KHR_gaussian_splatting = 0,                             // 基础高斯溅射扩展
			//KHR_spz_gaussian_splats_compression =1,                 // SPZ压缩的高斯溅射扩展
			KHRGaussianSplattingCompressionSpz2 = 2,           // SPZ2压缩的高斯溅射扩展
		};

		//================================================================================
		/// 坐标系统枚举（高斯数据使用）
		/// 使用12位二进制值表示三维坐标系中各轴的正方向指向
		/// 每个轴用4位表示：0010=左, 0011=右, 0100=下, 0101=上, 1000=后, 1001=前
		//================================================================================
		enum class CoordinateSystem : int {
			// 使用四位二进制值表示当前坐标轴指向,每个坐标系有三个轴（X Y Z）
			// 0010   表示正轴向左 
			// 0011   表示正轴向右 
			// 0100   表示正轴向下
			// 0101   表示正轴向上
			// 1000   表示正轴向后
			// 1001   表示正轴向前

			UNSPECIFIED = 0B000000000000,  // 未知坐标系
			LDB = 0B001001001000,          // x正轴向左,y正轴向下，z正轴向后
			RDB = 0B001101001000,          // x正轴向右,y正轴向下，z正轴向后
			LUB = 0B001001011000,          // x正轴向左,y正轴向上，z正轴向后
			RUB = 0B001101011000,          // x正轴向右,y正轴向上，z正轴向后
			LDF = 0B001001001001,          // x正轴向左,y正轴向下，z正轴向前
			RDF = 0B001101001001,          // x正轴向右,y正轴向下，z正轴向前
			LUF = 0B001001011001,          // x正轴向左,y正轴向上，z正轴向前
			RUF = 0B001101011001,          // x正轴向右,y正轴向上，z正轴向前

			LBD = 0B001010000100,          // x正轴向左,y正轴向后，z正轴向下
			RBD = 0B001110000100,          // x正轴向右,y正轴向后，z正轴向下
			LBU = 0B001010000101,          // x正轴向左,y正轴向后，z正轴向上
			RBU = 0B001110000101,          // x正轴向右,y正轴向后，z正轴向上
			LFD = 0B001010010100,          // x正轴向左,y正轴向前，z正轴向下
			RFD = 0B001110010100,          // x正轴向右,y正轴向前，z正轴向下
			LFU = 0B001010010101,          // x正轴向左,y正轴向前，z正轴向上
			RFU = 0B001110010101,          // x正轴向右,y正轴向前，z正轴向上

			DLB = 0B010000101000,          // x正轴向下,y正轴向左，z正轴向后
			DRB = 0B010000111000,          // x正轴向下,y正轴向右，z正轴向后
			ULB = 0B010100101000,          // x正轴向上,y正轴向左，z正轴向后
			URB = 0B010100111000,          // x正轴向上,y正轴向右，z正轴向后
			DLF = 0B010000101001,          // x正轴向下,y正轴向左，z正轴向前
			DRF = 0B010000111001,          // x正轴向下,y正轴向右，z正轴向前
			ULF = 0B010100101001,          // x正轴向上,y正轴向左，z正轴向前
			URF = 0B010100111001,          // x正轴向上,y正轴向右，z正轴向前

			DBL = 0B010010000010,          // x正轴向下,y正轴向后，z正轴向左
			DBR = 0B010010000011,          // x正轴向下,y正轴向后，z正轴向右
			UBL = 0B010110000010,          // x正轴向上,y正轴向后，z正轴向左
			UBR = 0B010110000011,          // x正轴向上,y正轴向后，z正轴向右
			DFL = 0B010010010010,          // x正轴向下,y正轴向前，z正轴向左
			DFR = 0B010010010011,          // x正轴向下,y正轴向前，z正轴向右
			UFL = 0B010110010010,          // x正轴向上,y正轴向前，z正轴向左
			UFR = 0B010110010011,          // x正轴向上,y正轴向前，z正轴向右

			BDL = 0B100001000010,          // x正轴向后,y正轴向下，z正轴向左
			BDR = 0B100001000011,          // x正轴向后,y正轴向下，z正轴向右
			BUL = 0B100001010010,          // x正轴向后,y正轴向上，z正轴向左
			BUR = 0B100001010011,          // x正轴向后,y正轴向上，z正轴向右
			FDL = 0B100101000010,          // x正轴向前,y正轴向下，z正轴向左
			FDR = 0B100101000011,          // x正轴向前,y正轴向下，z正轴向右
			FUL = 0B100101010010,          // x正轴向前,y正轴向上，z正轴向左
			FUR = 0B100101010011,          // x正轴向前,y正轴向上，z正轴向右

			BLD = 0B100000100100,          // x正轴向后,y正轴向左，z正轴向下
			BRD = 0B100000110100,          // x正轴向后,y正轴向右，z正轴向下
			BLU = 0B100000100101,          // x正轴向后,y正轴向左，z正轴向上
			BRU = 0B100000110101,          // x正轴向后,y正轴向右，z正轴向上
			FLD = 0B100100100100,          // x正轴向前,y正轴向左，z正轴向下
			FRD = 0B100100110100,          // x正轴向前,y正轴向右，z正轴向下
			FLU = 0B100100100101,          // x正轴向前,y正轴向左，z正轴向上
			FRU = 0B100100110101           // x正轴向前,y正轴向右，z正轴向上
		};
	}
}

// GUID结构体定义（跨平台兼容性处理）
#ifndef GUID_DEFINED
#define GUID_DEFINED
// 2013.11.05--zhc:存储于HDF文件中使用，因此需要保证大小一致
#ifdef MAPGIS_X64
typedef struct _GUID
{
	unsigned int  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} GUID;
#else
typedef struct _GUID
{
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} GUID;
#endif
#endif

// Windows平台下的自动链接库设置
#ifdef _WIN32
#define _MAPGIS_G3D_TILE_LIB_  "g3dtileprj.lib"

#ifndef _MAPGIS_G3D_TILE_EXPORT_
#pragma comment(lib,_MAPGIS_G3D_TILE_LIB_)
#endif

#ifndef __MAPGIS_G3D_TILE__HIDE_AUTOLINK_OUTPUT_
#pragma message("Automatically linking with " _MAPGIS_G3D_TILE_LIB_)
#endif
#endif

#endif