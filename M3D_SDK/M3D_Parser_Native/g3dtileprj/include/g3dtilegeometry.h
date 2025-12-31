#pragma once
#ifndef _G3D_TILE_GEOMETRY_H_
#define _G3D_TILE_GEOMETRY_H_

#include "g3dtiledefine.h"
#include "basclass70.h"
#include "math.h"
namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 几何类型枚举
		/// 定义了几何对象的不同类型分类
		//================================================================================
		enum class GeometryType :int
		{
			None = 0,		// 未知类型
			Point = 1,		// 点几何类型
			Line = 2,		// 线几何类型
			Surface = 3,	// 面几何类型
			Entity = 4,		// 体几何类型
		};

		//================================================================================
		/// 纹理放大过滤模式枚举
		/// 定义纹理在放大时使用的采样算法
		//================================================================================
		enum class MagFilterMode :int
		{
			MagFilterNearest = 9728, // 最近邻过滤（选择最近的纹素）
			MagFilterLinear = 9729,  // 线性过滤（对相邻纹素进行线性插值）
		};

		//================================================================================
		/// 纹理缩小过滤模式枚举
		/// 定义纹理在缩小时使用的采样算法，支持Mipmap技术
		//================================================================================
		enum class MinFilterMode :int
		{
			MinFilterNearest = 9728,				// 最近邻过滤（不使用Mipmap）
			MinFilterLinear = 9729,				// 线性过滤（不使用Mipmap）
			MinFilterNearestMipmapNearest = 9984,// 选择最合适的Mipmap层，然后使用最近邻过滤
			MinFilterLinearMipmapNearest = 9985, // 选择最合适的Mipmap层，然后使用线性过滤
			MinFilterNearestMipmapLinear = 9986,	// 在两个最合适的Mipmap层之间进行线性插值，每层内使用最近邻过滤
			MinFilterLinearMipmapLinear = 9987,	// 在两个最合适的Mipmap层之间进行线性插值，每层内也使用线性过滤（也称为三线性过滤）
		};

		//================================================================================
		/// 纹理坐标环绕模式枚举
		/// 定义纹理坐标超出[0,1]范围时的处理方式
		//================================================================================
		enum class WrapMode :int
		{
			WrapRepeat = 10497,			// 重复模式，纹理会在超出[0,1]范围时重复铺满整个区域
			WrapClampToEdge = 33071,		// 边缘钳制模式，纹理坐标会被钳制到边缘，超出[0,1]范围的部分会使用边缘纹素的颜色进行延伸
			WrapMirroredRepeat = 33648	// 镜像重复模式，纹理会在超出[0,1]范围时镜像重复
		};

		//================================================================================
		/// 材质透明度处理模式枚举
		/// 定义材质透明度的处理方式
		//================================================================================
		enum class AlphaMode :int
		{
			AlphaUnknown = 0,	// 未指定透明度模式
			AlphaOpaque,		// 不透明模式，材质完全不透明，不进行任何透明度处理
			AlphaBlend,		// 混合模式，使用alpha值进行透明度混合，实现半透明效果
			AlphaMask			// 遮罩模式，使用固定的alpha阈值（通过alphaCutoff指定）进行裁剪，alpha值低于阈值的像素完全透明，高于阈值的完全不透明
		};

		//================================================================================
		/// 图片信息结构体
		/// 存储纹理图片的相关信息和数据
		//================================================================================
		struct MAPGISG3DTILEEXPORT Image
		{
			CGString imageFileName;				// 图片文件名，可以为空
			vector<unsigned char> data;			// 图片数据字节流
			CGString mimeType;					// 图片的MIME类型（如"image/jpeg"、"image/png"等）

			//================================================================================
			/// 默认构造函数
			/// 初始化成员变量为默认值
			//================================================================================
			Image()
			{
				imageFileName = "";
				mimeType = "";
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] img 源图片对象
			/// @return 当前对象引用
			//================================================================================
			Image& operator=(const Image& img)
			{
				if (this == &img)
					return *this;
				imageFileName = img.imageFileName;
				data.clear();
				data.insert(data.begin(), img.data.begin(), img.data.end());
				mimeType = img.mimeType;
				return *this;
			}

			//================================================================================
			/// 等于比较运算符重载
			/// 比较两个图片对象是否相等
			///
			/// @param [in] img 待比较的图片对象
			/// @return 比较结果（true表示相等，false表示不相等）
			//================================================================================
			bool operator==(const Image& img)
			{
				if (this->imageFileName.CompareNoCase(img.imageFileName, CGString::EncodeCompareType::EncodeSameThenCompare) != 0 ||
					this->mimeType.CompareNoCase(img.mimeType, CGString::EncodeCompareType::EncodeSameThenCompare) != 0 ||
					this->data.size() != img.data.size() || this->data != img.data)
					return false;
				return true;
			}
		};

		//================================================================================
		/// 纹理采样器结构体
		/// 定义纹理采样和滤波的相关参数
		//================================================================================
		struct MAPGISG3DTILEEXPORT Sampler
		{
			MagFilterMode magFilter;	// 放大时使用的过滤模式
			MinFilterMode minFilter;	// 缩小时使用的过滤模式
			WrapMode wrapS;				// S坐标（U坐标）的环绕模式
			WrapMode wrapT;				// T坐标（V坐标）的环绕模式

			//================================================================================
			/// 默认构造函数
			/// 初始化为常用的默认采样参数
			//================================================================================
			Sampler()
			{
				magFilter = MagFilterMode::MagFilterLinear;					// 放大时使用线性过滤
				minFilter = MinFilterMode::MinFilterLinearMipmapLinear;		// 缩小时使用三线性Mipmap过滤
				wrapS = WrapMode::WrapRepeat;									// S坐标使用重复模式
				wrapT = WrapMode::WrapRepeat;									// T坐标使用重复模式
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] other 源采样器对象
			/// @return 当前对象引用
			//================================================================================
			Sampler& operator=(const Sampler& other)
			{
				if (this == &other)
					return *this;
				magFilter = other.magFilter;
				minFilter = other.minFilter;
				wrapS = other.wrapS;
				wrapT = other.wrapT;
				return *this;
			}

			//================================================================================
			/// 等于比较运算符重载
			/// 比较两个采样器对象是否相等
			///
			/// @param [in] other 待比较的采样器对象
			/// @return 比较结果（true表示相等，false表示不相等）
			//================================================================================
			bool operator==(const Sampler& other)
			{
				if (this->magFilter != other.magFilter || this->minFilter != other.minFilter ||
					this->wrapS != other.wrapS || this->wrapT != other.wrapT)
					return false;
				return true;
			}
		};

		//================================================================================
		/// 纹理结构体
		/// 关联采样器和图片的索引信息
		//================================================================================
		struct MAPGISG3DTILEEXPORT Texture
		{
			int samplerIndex;	// 采样器在采样器数组中的索引（-1表示无效）
			int imageIndex;		// 图片在图片数组中的索引（-1表示无效）

			//================================================================================
			/// 默认构造函数
			/// 初始化索引为无效值
			//================================================================================
			Texture()
			{
				samplerIndex = -1;
				imageIndex = -1;
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] other 源纹理对象
			/// @return 当前对象引用
			//================================================================================
			Texture& operator=(const Texture& other)
			{
				if (this == &other)
					return *this;
				samplerIndex = other.samplerIndex;
				imageIndex = other.imageIndex;
				return *this;
			}

			//================================================================================
			/// 等于比较运算符重载
			/// 比较两个纹理对象是否相等
			///
			/// @param [in] other 待比较的纹理对象
			/// @return 比较结果（true表示相等，false表示不相等）
			//================================================================================
			bool operator==(const Texture& other)
			{
				if (this->samplerIndex != other.samplerIndex || this->imageIndex != other.imageIndex)
					return false;
				return true;
			}
		};

		//================================================================================
		/// 纹理信息结构体
		/// 存储材质中使用的纹理相关信息
		//================================================================================
		struct MAPGISG3DTILEEXPORT TextureInfo
		{
			int textureIndex;		// 纹理在纹理数组中的索引（-1表示无效）
			size_t texCoord;		// 纹理坐标属性的索引

			//================================================================================
			/// 默认构造函数
			/// 初始化成员变量为默认值
			//================================================================================
			TextureInfo()
			{
				textureIndex = -1;
				texCoord = 0U;
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] other 源纹理信息对象
			/// @return 当前对象引用
			//================================================================================
			TextureInfo& operator=(const TextureInfo& other)
			{
				if (this == &other)
					return *this;
				textureIndex = other.textureIndex;
				texCoord = other.texCoord;
				return *this;
			}

			//================================================================================
			/// 等于比较运算符重载
			/// 比较两个纹理信息对象是否相等
			///
			/// @param [in] other 待比较的纹理信息对象
			/// @return 比较结果（true表示相等，false表示不相等）
			//================================================================================
			bool operator==(const TextureInfo& other)
			{
				// 若两个贴图均无效，则说明其信息一致
				if (this->textureIndex < 0 && other.textureIndex < 0)
					return true;
				if (this->textureIndex != other.textureIndex || this->texCoord != other.texCoord)
					return false;
				return true;
			}
		};

		//================================================================================
		/// 三元颜色结构体（RGB）
		/// 表示不包含透明度的RGB颜色值
		//================================================================================
		struct MAPGISG3DTILEEXPORT Color3f
		{
			float r;	// 红色分量，取值范围[0,1]
			float g;	// 绿色分量，取值范围[0,1]
			float b;	// 蓝色分量，取值范围[0,1]

			//================================================================================
			/// 默认构造函数
			/// 初始化为白色（1.0, 1.0, 1.0）
			//================================================================================
			Color3f() :r(1.0f), g(1.0f), b(1.0f)
			{
			}

			//================================================================================
			/// 构造函数
			/// 使用指定的RGB值初始化
			///
			/// @param [in] r 红色分量
			/// @param [in] g 绿色分量
			/// @param [in] b 蓝色分量
			//================================================================================
			Color3f(float r, float g, float b) :r(r), g(g), b(b)
			{
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] other 源颜色对象
			/// @return 当前对象引用
			//================================================================================
			Color3f& operator=(const Color3f& other)
			{
				if (this == &other)
					return *this;
				this->r = other.r;
				this->g = other.g;
				this->b = other.b;
				return *this;
			}

			//================================================================================
			/// 等于比较运算符重载
			/// 比较两个颜色对象是否相等（考虑浮点数精度）
			///
			/// @param [in] other 待比较的颜色对象
			/// @return 比较结果（true表示相等，false表示不相等）
			//================================================================================
			bool operator==(const Color3f& other)
			{
				if (fabs(this->r - other.r) >= 1E-6 || fabs(this->g - other.g) >= 1E-6 || fabs(this->b - other.b) >= 1E-6)
					return false;
				return true;
			}
		};

		//================================================================================
		/// 四元颜色结构体（RGBA）
		/// 表示包含透明度的RGBA颜色值
		//================================================================================
		struct MAPGISG3DTILEEXPORT Color4f
		{
			float r;	// 红色分量，取值范围[0,1]
			float g;	// 绿色分量，取值范围[0,1]
			float b;	// 蓝色分量，取值范围[0,1]
			float a;	// 透明度分量，取值范围[0,1]

			//================================================================================
			/// 默认构造函数
			/// 初始化为不透明白色（1.0, 1.0, 1.0, 1.0）
			//================================================================================
			Color4f() : r(1.0f), g(1.0f), b(1.0f), a(1.0f)
			{
			}

			//================================================================================
			/// 构造函数
			/// 使用指定的RGBA值初始化
			///
			/// @param [in] r 红色分量
			/// @param [in] g 绿色分量
			/// @param [in] b 蓝色分量
			/// @param [in] a 透明度分量
			//================================================================================
			Color4f(float r, float g, float b, float a) : r(r), g(g), b(b), a(a)
			{
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] other 源颜色对象
			/// @return 当前对象引用
			//================================================================================
			Color4f& operator=(const Color4f& other)
			{
				if (this == &other)
					return *this;

				this->r = other.r;
				this->g = other.g;
				this->b = other.b;
				this->a = other.a;

				return *this;
			}

			//================================================================================
			/// 等于比较运算符重载
			/// 比较两个颜色对象是否相等（考虑浮点数精度）
			///
			/// @param [in] other 待比较的颜色对象
			/// @return 比较结果（true表示相等，false表示不相等）
			//================================================================================
			bool operator==(const Color4f& other)
			{
				if (fabs(this->r - other.r) >= 1E-6 || fabs(this->g - other.g) >= 1E-6 ||
					fabs(this->b - other.b) >= 1E-6 || fabs(this->a - other.a) >= 1E-6)
					return false;
				return true;
			}
		};

		//================================================================================
		/// 材质结构体
		/// 定义3D模型表面的渲染属性，基于PBR（基于物理的渲染）标准
		//================================================================================
		struct MAPGISG3DTILEEXPORT Material
		{
			//================================================================================
			/// PBR金属粗糙度材质模型
			/// 基于glTF标准的金属粗糙度工作流
			//================================================================================
			struct PbrMetallicRoughness
			{
				Color4f baseColorFactor;                // 基础颜色因子（RGBA），用于调制基础纹理颜色
				TextureInfo baseColorTexture;           // 基础颜色纹理信息
				float metallicFactor;                   // 金属度因子，0.0表示完全非金属，1.0表示完全金属
				float roughnessFactor;                  // 粗糙度因子，0.0表示完全光滑，1.0表示完全粗糙
				TextureInfo metallicRoughnessTexture;   // 金属粗糙度纹理信息（通常存储在纹理的B和G通道）

				//================================================================================
				/// 默认构造函数
				/// 初始化为默认的PBR材质参数
				//================================================================================
				PbrMetallicRoughness() :
					baseColorFactor(1.0f, 1.0f, 1.0f, 1.0f),  // 白色不透明
					metallicFactor(0.0f),                      // 完全非金属
					roughnessFactor(0.5f)                      // 中等粗糙度
				{
				}

				//================================================================================
				/// 赋值运算符重载
				/// 实现深拷贝操作
				///
				/// @param [in] other 源PBR金属粗糙度对象
				/// @return 当前对象引用
				//================================================================================
				PbrMetallicRoughness& operator=(const PbrMetallicRoughness& other)
				{
					if (this == &other)
						return *this;
					baseColorFactor = other.baseColorFactor;
					baseColorTexture = other.baseColorTexture;
					metallicFactor = other.metallicFactor;
					roughnessFactor = other.roughnessFactor;
					metallicRoughnessTexture = other.metallicRoughnessTexture;
					return *this;
				}

				//================================================================================
				/// 等于比较运算符重载
				/// 比较两个PBR金属粗糙度对象是否相等
				///
				/// @param [in] other 待比较的PBR金属粗糙度对象
				/// @return 比较结果（true表示相等，false表示不相等）
				//================================================================================
				bool operator==(const PbrMetallicRoughness& other)
				{
					if (!(this->baseColorFactor == other.baseColorFactor) || !(this->baseColorTexture == other.baseColorTexture) ||
						fabs(this->metallicFactor - other.metallicFactor) >= 1E-6 || fabs(this->roughnessFactor - other.roughnessFactor) >= 1E-6 ||
						!(this->metallicRoughnessTexture == other.metallicRoughnessTexture))
						return false;
					return true;
				}
			};

			//================================================================================
			/// 法线纹理信息结构体
			/// 扩展TextureInfo结构体，添加法线纹理特有的缩放参数
			//================================================================================
			struct NormalTextureInfo : TextureInfo
			{
				float scale;    // 法线纹理的缩放因子，用于控制法线贴图的强度

				//================================================================================
				/// 默认构造函数
				/// 初始化法线纹理缩放因子为1.0（无缩放）
				//================================================================================
				NormalTextureInfo() :
					scale(1.0f)
				{
				}

				//================================================================================
				/// 赋值运算符重载
				/// 实现深拷贝操作
				///
				/// @param [in] other 源法线纹理信息对象
				/// @return 当前对象引用
				//================================================================================
				NormalTextureInfo& operator=(const NormalTextureInfo& other)
				{
					if (this == &other)
						return *this;
					scale = other.scale;
					textureIndex = other.textureIndex;
					texCoord = other.texCoord;
					return *this;
				}

				//================================================================================
				/// 等于比较运算符重载
				/// 比较两个法线纹理信息对象是否相等
				///
				/// @param [in] other 待比较的法线纹理信息对象
				/// @return 比较结果（true表示相等，false表示不相等）
				//================================================================================
				bool operator==(const NormalTextureInfo& other)
				{
					if (this->textureIndex != other.textureIndex || this->texCoord != other.texCoord || fabs(this->scale - other.scale) >= 1E-6)
						return false;
					return true;
				}
			};

			//================================================================================
			/// 遮挡纹理信息结构体
			/// 扩展TextureInfo结构体，添加遮挡纹理特有的强度参数
			//================================================================================
			struct OcclusionTextureInfo : TextureInfo
			{
				float strength;    // 遮挡贴图强度因子，0.0表示无遮挡效果，1.0表示完全遮挡

				//================================================================================
				/// 默认构造函数
				/// 初始化遮挡纹理强度因子为1.0（完全强度）
				//================================================================================
				OcclusionTextureInfo() :
					strength(1.0f)
				{
				}

				//================================================================================
				/// 赋值运算符重载
				/// 实现深拷贝操作
				///
				/// @param [in] other 源遮挡纹理信息对象
				/// @return 当前对象引用
				//================================================================================
				OcclusionTextureInfo& operator=(const OcclusionTextureInfo& other)
				{
					if (this == &other)
						return *this;
					strength = other.strength;
					textureIndex = other.textureIndex;
					texCoord = other.texCoord;
					return *this;
				}

				//================================================================================
				/// 等于比较运算符重载
				/// 比较两个遮挡纹理信息对象是否相等
				///
				/// @param [in] other 待比较的遮挡纹理信息对象
				/// @return 比较结果（true表示相等，false表示不相等）
				//================================================================================
				bool operator==(const OcclusionTextureInfo& other)
				{
					if (this->textureIndex != other.textureIndex || this->texCoord != other.texCoord || fabs(this->strength - other.strength) >= 1E-6)
						return false;
					return true;
				}
			};

			//================================================================================
			/// 默认构造函数
			/// 初始化材质为默认参数
			//================================================================================
			Material() :
				emissiveFactor(0.0f, 0.0f, 0.0f),        // 无自发光
				alphaMode(AlphaMode::AlphaOpaque),      // 不透明模式
				alphaCutoff(0.5f),                       // 透明度阈值0.5
				doubleSided(false)                       // 单面渲染
			{
			}

			PbrMetallicRoughness metallicRoughness;     // PBR金属粗糙度材质信息
			NormalTextureInfo normalTexture;            // 法线纹理信息
			OcclusionTextureInfo occlusionTexture;      // 遮挡纹理信息
			TextureInfo emissiveTexture;                // 自发光纹理信息
			Color3f emissiveFactor;                     // 自发光颜色因子
			AlphaMode alphaMode;                        // 透明度处理模式
			float alphaCutoff;                          // 透明度阈值（仅在ALPHA_MASK模式下有效）
			bool doubleSided;                           // 双面渲染标志

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] other 源材质对象
			/// @return 当前对象引用
			//================================================================================
			Material& operator=(const Material& other)
			{
				if (this == &other)
					return *this;
				metallicRoughness = other.metallicRoughness;
				normalTexture = other.normalTexture;
				occlusionTexture = other.occlusionTexture;
				emissiveTexture = other.emissiveTexture;
				emissiveFactor = other.emissiveFactor;
				alphaMode = other.alphaMode;
				alphaCutoff = other.alphaCutoff;
				doubleSided = other.doubleSided;
				return *this;
			}

			//================================================================================
			/// 等于比较运算符重载
			/// 比较两个材质对象是否相等
			///
			/// @param [in] other 待比较的材质对象
			/// @return 比较结果（true表示相等，false表示不相等）
			//================================================================================
			bool operator==(const Material& other)
			{
				if (!(this->metallicRoughness == other.metallicRoughness) || !(this->normalTexture == other.normalTexture) ||
					!(this->occlusionTexture == other.occlusionTexture) || !(this->emissiveTexture == other.emissiveTexture) ||
					!(this->emissiveFactor == other.emissiveFactor) || this->alphaMode != other.alphaMode ||
					fabs(this->alphaCutoff - other.alphaCutoff) >= 1E-6 || this->doubleSided != other.doubleSided)
					return false;
				return true;
			}
		};

		//================================================================================
		/// 带颜色信息的点集结构体
		/// 存储具有统一颜色属性的点几何数据
		//================================================================================
		struct MAPGISG3DTILEEXPORT ColorPoint
		{
			CPoints points;     // 点坐标集合
			Color4f color;      // 统一颜色值（RGBA）

			//================================================================================
			/// 默认构造函数
			//================================================================================
			ColorPoint()
			{
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作，特别处理了Linux平台的编译兼容性问题
			///
			/// @param [in] val 源颜色点对象
			/// @return 当前对象引用
			//================================================================================
			ColorPoint& operator=(const ColorPoint& val)
			{
				if (this == &val)
					return *this;
				// 修改说明: 解决linux编译问题
				// 修改时间: 2024-12-26
				ColorPoint tempVal;
				tempVal = val;
				points.Empty();
				points.Set(tempVal.points.GetBufPtr(), tempVal.points.GetNum());
				color = val.color;
				return *this;
			}
		};

		//================================================================================
		/// 点要素结构体
		/// 表示一个具有唯一标识的点几何要素，可包含多个带颜色的点集
		//================================================================================
		struct MAPGISG3DTILEEXPORT PointFeature
		{
			gisINT64 id;                    // 要素唯一标识符
			vector<ColorPoint> colorPoints; // 带颜色的点集集合

			//================================================================================
			/// 默认构造函数
			/// 初始化要素ID为无效值(-1)
			//================================================================================
			PointFeature()
			{
				id = -1;
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] val 源点要素对象
			/// @return 当前对象引用
			//================================================================================
			PointFeature& operator=(const PointFeature& val)
			{
				if (this == &val)
					return *this;
				id = val.id;
				colorPoints.clear();
				colorPoints.insert(colorPoints.begin(), val.colorPoints.begin(), val.colorPoints.end());
				return *this;
			}
		};

		//================================================================================
		/// 带颜色信息的线数据结构体
		/// 存储具有统一颜色属性的线几何数据
		//================================================================================
		struct MAPGISG3DTILEEXPORT ColorLine
		{
			vector<CVarLine> lines;    // 线坐标集合
			Color4f color;             // 统一颜色值（RGBA）

			//================================================================================
			/// 默认构造函数
			//================================================================================
			ColorLine() {}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] val 源颜色线对象
			/// @return 当前对象引用
			//================================================================================
			ColorLine& operator=(const ColorLine& val)
			{
				if (this == &val)
					return *this;
				color = val.color;
				lines.clear();
				for (vector<CVarLine>::const_iterator itr = val.lines.begin(); itr != val.lines.end(); ++itr)
				{
					lines.push_back(CVarLine(*itr));
				}
				return *this;
			}
		};

		//================================================================================
		/// 线要素结构体
		/// 表示一个具有唯一标识的线几何要素，可包含多个带颜色的线集合
		//================================================================================
		struct MAPGISG3DTILEEXPORT LineFeature
		{
			gisINT64 id;                  // 线要素唯一标识符
			vector<ColorLine> colorLines; // 带颜色的线集合

			//================================================================================
			/// 默认构造函数
			/// 初始化要素ID为无效值(-1)
			//================================================================================
			LineFeature()
			{
				id = -1;
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] val 源线要素对象
			/// @return 当前对象引用
			//================================================================================
			LineFeature& operator=(const LineFeature& val)
			{
				if (this == &val)
					return *this;
				id = val.id;
				colorLines.assign(val.colorLines.begin(), val.colorLines.end());
				return *this;
			}
		};

		//================================================================================
		/// 网格结构体
		/// 表示一个包含几何表面和材质信息的网格单元
		//================================================================================
		struct MAPGISG3DTILEEXPORT Mesh
		{
			CAnySurface surface;        // 面几何数据
			gisLONG materialIndex;      // 材质索引（-1表示无效）

			//================================================================================
			/// 默认构造函数
			/// 初始化材质索引为无效值
			//================================================================================
			Mesh()
			{
				materialIndex = -1;
			}

			//================================================================================
			/// 赋值运算符重载（非const版本）
			/// 实现深拷贝操作
			///
			/// @param [in] val 源网格对象
			/// @return 当前对象引用
			//================================================================================
			Mesh& operator=(Mesh& val)
			{
				if (this == &val)
					return *this;
				surface = val.surface;
				materialIndex = val.materialIndex;
				return *this;
			}

			//================================================================================
			/// 赋值运算符重载（const版本）
			/// 实现深拷贝操作
			///
			/// @param [in] val 源网格对象
			/// @return 当前对象引用
			//================================================================================
			Mesh& operator=(const Mesh& val)
			{
				if (this == &val)
					return *this;
				CAnySurface tempSurface = CAnySurface(val.surface);
				surface = tempSurface;
				materialIndex = val.materialIndex;
				return *this;
			}
		};

		//================================================================================
		/// 面要素结构体
		/// 表示一个具有唯一标识的面几何要素，可包含多个网格
		//================================================================================
		struct MAPGISG3DTILEEXPORT SurfaceFeature
		{
			gisINT64 id;                // 要素唯一标识符
			vector<gisINT64> ids;       // 当该值存在时，保证大小是所有mesh点个数的和，优先考虑
			vector<Mesh> meshes;        // 网格集合

			//================================================================================
			/// 默认构造函数
			/// 初始化要素ID为无效值(-1)
			//================================================================================
			SurfaceFeature()
			{
				id = -1;
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] val 源面要素对象
			/// @return 当前对象引用
			//================================================================================
			SurfaceFeature& operator=(const SurfaceFeature& val)
			{
				if (this == &val)
					return *this;
				id = val.id;
				ids.assign(val.ids.begin(), val.ids.end());
				meshes.assign(val.meshes.begin(), val.meshes.end());
				return *this;
			}
		};

		//================================================================================
		/// 体要素结构体
		/// 表示一个具有唯一标识的体几何要素
		//================================================================================
		struct MAPGISG3DTILEEXPORT EntityFeature
		{
			gisINT64 id;                            // 要素唯一标识符
			vector<gisINT64> ids;                   // 每个点对应一个OID的情况，优先考虑
			CAnyEntity entity;                      // 体几何数据
			vector<gisLONG> materialIndexArray;     // 材质索引集合

			//================================================================================
			/// 默认构造函数
			/// 初始化要素ID为无效值(-1)
			//================================================================================
			EntityFeature()
			{
				id = -1;
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] val 源体要素对象
			/// @return 当前对象引用
			//================================================================================
			EntityFeature& operator=(const EntityFeature& val)
			{
				if (this == &val)
					return *this;
				id = val.id;
				ids.assign(val.ids.begin(), val.ids.end());
				CAnyEntity tempEntity = val.entity;

				entity = tempEntity;
				materialIndexArray.assign(val.materialIndexArray.begin(), val.materialIndexArray.end());
				return *this;
			}
		};

		//================================================================================
		/// 高斯数据结构体
		/// 存储高斯溅射点的相关参数信息
		//================================================================================
		struct GaussianFeature
		{
			gisINT64  id;           // ID值，很多数据没有
			float position[3];      // 高斯中心的3D坐标 (x, y, z)
			float rotation[4];      // 表示旋转的四元数 (x, y, z, w)
			float scale[3];         // 三个主轴上的尺度（标准差）
			float color[3];         // 基础颜色/SH0
			float alpha;            // 不透明度
			vector<float> sh;       // 高阶球谐函数系数，存储顺序是 R G B R G B ...，必须是3的倍数

			//================================================================================
			/// 默认构造函数
			/// 初始化ID为无效值，alpha为0
			//================================================================================
			GaussianFeature()
			{
				id = -1;
				alpha = 0;
			}
		};

		//================================================================================
		/// 三维模型基类
		/// 所有几何模型类型的抽象基类，定义统一接口
		//================================================================================
		class MAPGISG3DTILEEXPORT G3DModel
		{
		protected:
			//================================================================================
			/// 受保护的默认构造函数
			/// 防止直接实例化基类
			//================================================================================
			G3DModel() {};

		public:
			//================================================================================
			/// 虚析构函数
			/// 确保派生类能够正确析构
			//================================================================================
			virtual ~G3DModel() {};

			//================================================================================
			/// 获取几何类型（纯虚函数）
			/// 由派生类实现具体的几何类型返回
			///
			/// @return 几何类型枚举值
			//================================================================================
			virtual GeometryType GetGeometryType() const = 0;
		};

		//================================================================================
		/// 点模型类
		/// 表示点几何类型的三维模型
		//================================================================================
		class MAPGISG3DTILEEXPORT PointsModel : public G3DModel
		{
		public:
			vector<PointFeature> features;    // 点要素集合

			//================================================================================
			/// 默认构造函数
			//================================================================================
			PointsModel()
			{
			}

			//================================================================================
			/// 析构函数
			//================================================================================
			~PointsModel()
			{
			}

			//================================================================================
			/// 获取几何类型
			///
			/// @return GeometryType::point（点几何类型）
			//================================================================================
			virtual GeometryType GetGeometryType() const { return GeometryType::Point; };
		};

		//================================================================================
		/// 线模型类
		/// 表示线几何类型的三维模型
		//================================================================================
		class MAPGISG3DTILEEXPORT LinesModel : public G3DModel
		{
		public:
			vector<LineFeature> features;    // 线要素集合

			//================================================================================
			/// 默认构造函数
			//================================================================================
			LinesModel()
			{
			}

			//================================================================================
			/// 析构函数
			//================================================================================
			~LinesModel()
			{
			}

			//================================================================================
			/// 获取几何类型
			///
			/// @return GeometryType::line（线几何类型）
			//================================================================================
			virtual GeometryType GetGeometryType() const { return GeometryType::Line; };
		};

		//================================================================================
		/// 面模型类
		/// 表示面几何类型的三维模型
		//================================================================================
		class MAPGISG3DTILEEXPORT SurfacesModel : public G3DModel
		{
		public:
			vector<SurfaceFeature> features;    // 面要素集合
			vector<Image> images;               // 图片集合
			vector<Sampler> samplers;           // 采样器集合
			vector<Texture> textures;           // 纹理集合
			vector<Material> materials;         // 材质集合

			//================================================================================
			/// 默认构造函数
			//================================================================================
			SurfacesModel()
			{
			}

			//================================================================================
			/// 析构函数
			//================================================================================
			~SurfacesModel()
			{
			}

			//================================================================================
			/// 获取几何类型
			///
			/// @return GeometryType::surface（面几何类型）
			//================================================================================
			virtual GeometryType GetGeometryType() const { return GeometryType::Surface; };
		};

		//================================================================================
		/// 体模型类
		/// 表示体几何类型的三维模型
		//================================================================================
		class MAPGISG3DTILEEXPORT EntitiesModel : public G3DModel
		{
		public:
			vector<EntityFeature> features;    // 体要素集合
			vector<Image> images;              // 图片集合
			vector<Sampler> samplers;          // 采样器集合
			vector<Texture> textures;          // 纹理集合
			vector<Material> materials;        // 材质集合

			//================================================================================
			/// 默认构造函数
			//================================================================================
			EntitiesModel()
			{
			}

			//================================================================================
			/// 析构函数
			//================================================================================
			~EntitiesModel()
			{
			}

			//================================================================================
			/// 获取几何类型
			///
			/// @return GeometryType::entity（体几何类型）
			//================================================================================
			virtual GeometryType GetGeometryType() const { return GeometryType::Entity; };
		};

		//================================================================================
		/// 栅格体元模型类
		/// 表示栅格体元（Voxel）几何类型的三维模型，支持时间和空间维度的数据组织
		//================================================================================
		class MAPGISG3DTILEEXPORT VoxelModel : public G3DModel
		{
		public:
			//================================================================================
			/// 栅格体元模型的时间空间类型枚举
			/// 定义了体元数据在时间维度上的分布规律
			//================================================================================
			enum VoxelTimeSpaceType
			{
				VoxelTimeSpaceTypeUnknown = 0,		// 未知类型，表示时间空间类型未定义或未指定
				VoxelTimeSpaceTypeRegular = 1,		// 规则类型，表示体素模型的时间节点是规则分布的，可以通过起始时间、时间间隔和节点数来描述
				VoxelTimeSpaceTypeIrregular = 2	// 不规则类型，表示体素模型的时间节点是不规则分布的，需要通过具体的时间节点列表来描述
			};

			//================================================================================
			/// 栅格体元模型的空间范围类型枚举
			/// 定义了体元数据在空间维度上的范围表示方式
			//================================================================================
			enum VoxelRangeType
			{
				VoxelRangeTypeUnknown = 0,		// 未知类型，表示体素模型的空间范围未定义或未指定
				VoxelRangeTypeOrientedBox = 1,	// 有向包围盒类型，表示体素模型的空间范围由一个有向包围盒定义，通过中心点和三个轴向量来描述
				VoxelRangeTypeRegion = 2       // 区域类型，表示体素模型的空间范围由一个矩形区域定义，通过经纬度范围来描述
			};

			//================================================================================
			/// 获取空间范围类型
			///
			/// @return 当前模型的空间范围类型枚举值
			//================================================================================
			VoxelRangeType GetRangeType() const
			{
				return m_rangeType;
			}

			//================================================================================
			/// 获取有向包围盒信息
			///
			/// @param [out] center 包围盒中心点坐标
			/// @param [out] axisX  包围盒X轴向量
			/// @param [out] axisY  包围盒Y轴向量
			/// @param [out] axisZ  包围盒Z轴向量
			///
			/// @return 操作结果
			///         - 1: 成功获取包围盒信息
			///         - 0: 当前范围类型不是有向包围盒类型，获取失败
			//================================================================================
			int GetBox(D_3DOT& center, D_3DOT& axisX, D_3DOT& axisY, D_3DOT& axisZ) const
			{
				if (m_rangeType != VoxelRangeTypeOrientedBox)
					return 0;
				center = m_boxPosition;
				axisX = m_boxAxisX;
				axisY = m_boxAxisY;
				axisZ = m_boxAxisZ;
				return 1;
			}

			//================================================================================
			/// 设置有向包围盒信息
			///
			/// @param [in] center 包围盒中心点坐标
			/// @param [in] axisX  包围盒X轴向量
			/// @param [in] axisY  包围盒Y轴向量
			/// @param [in] axisZ  包围盒Z轴向量
			///
			/// @return 操作结果
			///         - 1: 设置成功
			///         - 0: 设置失败
			//================================================================================
			int SetBox(D_3DOT center, D_3DOT axisX, D_3DOT axisY, D_3DOT axisZ)
			{
				m_rangeType = VoxelRangeTypeOrientedBox;
				m_boxPosition = center;
				m_boxAxisX = axisX;
				m_boxAxisY = axisY;
				m_boxAxisZ = axisZ;
				return 1;
			}

			//================================================================================
			/// 获取地理区域范围信息
			///
			/// @param [out] region 三维地理范围（经纬度表示的矩形区域）
			///
			/// @return 操作结果
			///         - 1: 成功获取区域信息
			///         - 0: 当前范围类型不是区域类型，获取失败
			//================================================================================
			int GetRegion(D_3RECT& region) const
			{
				if (m_rangeType != VoxelRangeTypeRegion)
					return 0;
				region = m_region;
				return 1;
			}

			//================================================================================
			/// 设置地理区域范围信息
			///
			/// @param [in] region 三维地理范围（经纬度表示的矩形区域）
			///
			/// @return 操作结果
			///         - 1: 设置成功
			///         - 0: 设置失败
			//================================================================================
			int SetRegion(D_3RECT region)
			{
				m_rangeType = VoxelRangeTypeRegion;
				m_region = region;
				return 1;
			}

			//================================================================================
			/// 获取体元在三个维度上的分辨率（格网数）
			///
			/// @param [out] x X轴方向的格网数
			/// @param [out] y Y轴方向的格网数
			/// @param [out] z Z轴方向的格网数
			///
			/// @return 操作结果
			///         - 1: 成功获取维度信息
			///         - 0: 获取失败
			//================================================================================
			int GetDimensions(int& x, int& y, int& z) const
			{
				x = m_dimensionX;
				y = m_dimensionY;
				z = m_dimensionZ;
				return 1;
			}

			//================================================================================
			/// 设置体元在三个维度上的分辨率（格网数）
			///
			/// @param [in] x X轴方向的格网数（必须大于0）
			/// @param [in] y Y轴方向的格网数（必须大于0）
			/// @param [in] z Z轴方向的格网数（必须大于0）
			///
			/// @return 操作结果
			///         - 1: 设置成功
			///         - 0: 参数无效或设置失败
			//================================================================================
			int SetDimensions(int x, int y, int z)
			{
				if (x < 1 || y < 1 || z < 1)
					return 0;

				m_dimensionX = x;
				m_dimensionY = y;
				m_dimensionZ = z;
				return 1;
			}

			//================================================================================
			/// 获取时间节点个数
			///
			/// @return 时间节点个数
			///         - 根据时间空间类型返回相应的时间节点数量
			//================================================================================
			int GetTimeNodeCount() const
			{
				switch (m_timeSpaceType)
				{
				case VoxelTimeSpaceTypeUnknown:
					return 0;
				case VoxelTimeSpaceTypeRegular:
					return m_regularTimeNodeCount;
				case VoxelTimeSpaceTypeIrregular:
					return m_irregularTimeNodes.size();
				default:
					return 0;
				}
				return 0;
			}

			//================================================================================
			/// 获取规则时间空间信息
			///
			/// @param [out] offset       时间节点偏移（起始时间）
			/// @param [out] timeGap      时间间隔
			/// @param [out] timeNodeCount 时间节点个数
			///
			/// @return 操作结果
			///         - 1: 成功获取时间信息
			///         - 0: 当前时间空间类型不是规则类型，获取失败
			//================================================================================
			int GetRegularTimeSpaceInfo(gisINT64& offset, gisINT64& timeGap, int& timeNodeCount) const
			{
				if (m_timeSpaceType != VoxelTimeSpaceTypeRegular)
					return 0;
				offset = m_regularTimeOffset;
				timeGap = m_regularTimeGap;
				timeNodeCount = m_regularTimeNodeCount;
				return 1;
			}

			//================================================================================
			/// 设置规则时间空间信息
			///
			/// @param [in] offset       时间节点偏移（起始时间）
			/// @param [in] timeGap      时间间隔（必须大于0，当节点数大于1时）
			/// @param [in] timeNodeCount 时间节点个数（必须大于0）
			///
			/// @return 操作结果
			///         - 1: 设置成功
			///         - 0: 参数无效或设置失败
			//================================================================================
			int SetRegularTimeSpaceInfo(gisINT64 offset, gisINT64 timeGap, int timeNodeCount)
			{
				if (timeNodeCount <= 0)
					return 0;
				if (timeNodeCount > 1 && timeGap <= 0)
					return 0;
				m_timeSpaceType = VoxelTimeSpaceTypeRegular;
				m_regularTimeOffset = offset;
				m_regularTimeGap = timeGap;
				m_regularTimeNodeCount = timeNodeCount;
				return 1;
			}

			//================================================================================
			/// 获取不规则时间节点列表
			///
			/// @param [out] timeNodes 时间节点列表
			///
			/// @return 操作结果
			///         - 1: 成功获取时间列表
			///         - 0: 当前时间空间类型不是不规则类型，获取失败
			//================================================================================
			int GetIrregularTimeNodes(vector<gisINT64>& timeNodes) const
			{
				if (m_timeSpaceType != VoxelTimeSpaceTypeIrregular)
					return 0;
				timeNodes = m_irregularTimeNodes;
				return 1;
			}

			//================================================================================
			/// 设置不规则时间节点列表
			///
			/// @param [in] timeNodes 时间节点列表
			///
			/// @return 操作结果
			///         - 1: 设置成功
			///         - 0: 设置失败
			//================================================================================
			int SetIrregularTimeNodes(const vector<gisINT64>& timeNodes)
			{
				m_timeSpaceType = VoxelTimeSpaceTypeIrregular;
				m_irregularTimeNodes = timeNodes;
				return 1;
			}

			//================================================================================
			/// 默认构造函数
			/// 初始化体元模型的默认参数
			//================================================================================
			VoxelModel()
			{
				m_dimensionX = m_dimensionY = m_dimensionZ = 1;   // 默认分辨率为1x1x1
				m_rangeType = VoxelRangeTypeUnknown;                // 默认范围类型为未知
				m_region = {};
				m_boxPosition = {};
				m_boxAxisX = {};
				m_boxAxisY = {};
				m_boxAxisZ = {};

				m_timeSpaceType = VoxelTimeSpaceTypeUnknown;        // 默认时间空间类型为未知
				m_regularTimeOffset = 0;                             // 默认起始时间为0
				m_regularTimeGap = 0;                                // 默认时间间隔为0
				m_regularTimeNodeCount = 0;                          // 默认时间节点个数为0
				m_layerID = 0;                                       // 默认层ID为0
			}

			//================================================================================
			/// 析构函数
			//================================================================================
			~VoxelModel()
			{
			}

			//================================================================================
			/// 赋值运算符重载
			/// 实现深拷贝操作
			///
			/// @param [in] other 源体元模型对象
			/// @return 当前对象引用
			//================================================================================
			VoxelModel& operator=(const VoxelModel& other)
			{
				if (this == &other)
					return *this;
				m_dimensionX = other.m_dimensionX;
				m_dimensionY = other.m_dimensionY;
				m_dimensionZ = other.m_dimensionZ;
				m_rangeType = other.m_rangeType;
				m_region = other.m_region;
				m_boxPosition = other.m_boxPosition;
				m_boxAxisX = other.m_boxAxisX;
				m_boxAxisY = other.m_boxAxisY;
				m_boxAxisZ = other.m_boxAxisZ;
				m_timeSpaceType = other.m_timeSpaceType;
				m_regularTimeOffset = other.m_regularTimeOffset;
				m_regularTimeGap = other.m_regularTimeGap;
				m_regularTimeNodeCount = other.m_regularTimeNodeCount;
				m_irregularTimeNodes = other.m_irregularTimeNodes;
				m_layerID = other.m_layerID;
				return *this;
			}

			//================================================================================
			/// 获取几何类型
			///
			/// @return GeometryType::None（体元模型的几何类型）
			//================================================================================
			virtual GeometryType GetGeometryType() const { return GeometryType::None; };

			int m_dimensionX;						// X轴方向的格网数（分辨率）
			int m_dimensionY;						// Y轴方向的格网数（分辨率）
			int m_dimensionZ;						// Z轴方向的格网数（分辨率）
			VoxelRangeType m_rangeType;				// 空间范围类型
			D_3RECT m_region;						// 地理区域范围（单位：弧度）
			D_3DOT m_boxPosition;					// 包围盒中心点坐标
			D_3DOT m_boxAxisX;						// 包围盒X轴向量
			D_3DOT m_boxAxisY;						// 包围盒Y轴向量
			D_3DOT m_boxAxisZ;						// 包围盒Z轴向量

			VoxelTimeSpaceType m_timeSpaceType;		// 时间空间类型
			gisINT64 m_regularTimeOffset;			// 规则时间节点偏移（起始时间，单位：毫秒）
			gisINT64 m_regularTimeGap;				// 规则时间间隔（单位：毫秒）
			int m_regularTimeNodeCount;				// 规则时间节点个数
			vector<gisINT64> m_irregularTimeNodes;	// 不规则时间节点列表

			unsigned int m_layerID;					// 层ID（用于标识数据层）
		};

		//================================================================================
		/// 高斯溅射模型类
		/// 表示基于高斯溅射（Gaussian Splatting）技术的三维模型
		//================================================================================
		class MAPGISG3DTILEEXPORT GaussianModel : public G3DModel
		{
		public:
			vector<GaussianFeature> features;					// 高斯特征要素集合
			MapGIS::Tile::CoordinateSystem coordinateSystem;	// 模型使用的坐标系统

			//================================================================================
			/// 默认构造函数
			/// 初始化坐标系统为未指定状态
			//================================================================================
			GaussianModel()
			{
				coordinateSystem = MapGIS::Tile::CoordinateSystem::UNSPECIFIED;
			}

			//================================================================================
			/// 析构函数
			//================================================================================
			~GaussianModel()
			{
			}

			//================================================================================
			/// 获取几何类型
			///
			/// @return GeometryType::None（高斯模型的几何类型）
			//================================================================================
			virtual GeometryType GetGeometryType() const { return GeometryType::None; };
		};
		
	}
}

#endif