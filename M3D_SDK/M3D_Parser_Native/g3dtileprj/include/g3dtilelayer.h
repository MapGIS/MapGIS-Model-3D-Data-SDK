#pragma once
#ifndef _G3D_TILE_LAYER_H_
#define _G3D_TILE_LAYER_H_

#include "g3dtiledefine.h"
#include "g3dtilerecord.h"
#include "g3dtilegeometry.h"
#include "gbytearray.h"

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 栅格体元样式基类
		//================================================================================
		class MAPGISG3DTILEEXPORT VoxelStyle
		{
		public:
			VoxelStyle() {};
			virtual ~VoxelStyle() {};
		};

		//================================================================================
		/// 栅格体元 Stretch样式
		//================================================================================
		class MAPGISG3DTILEEXPORT VoxelStretchStyle :public VoxelStyle
		{
		public:
			VoxelStretchStyle()
			{
				m_fieldName = "";
				m_stretchRangeMinValue = 0;
				m_stretchRangeMaxValue = 0;
				m_rangeFilterMinValue = 0;
				m_rangeFilterMaxValue = 0;
				enableTransparent = false;
			};
			virtual ~VoxelStretchStyle() {};

			CGString m_fieldName;						//字段名
			double m_stretchRangeMinValue;				//拉伸最小值
			double m_stretchRangeMaxValue;				//拉伸最大值
			double m_rangeFilterMinValue;				//筛选最小值
			double m_rangeFilterMaxValue;				//筛选最大值
			vector<pair<int, Color4f>> m_colorStops;	//颜色列表
			bool enableTransparent;						//是否启用透明
			vector<pair<int, int>> m_alphaStops;		//透明度列表

			VoxelStretchStyle& operator=(const VoxelStretchStyle& other)
			{
				if (this == &other)
					return *this;
				m_fieldName = other.m_fieldName;
				m_stretchRangeMinValue = other.m_stretchRangeMinValue;
				m_stretchRangeMaxValue = other.m_stretchRangeMaxValue;
				m_rangeFilterMinValue = other.m_rangeFilterMinValue;
				m_rangeFilterMaxValue = other.m_rangeFilterMaxValue;
				enableTransparent = other.enableTransparent;
				m_colorStops.assign(other.m_colorStops.begin(), other.m_colorStops.end());
				m_alphaStops.assign(other.m_alphaStops.begin(), other.m_alphaStops.end());
				return *this;
			}
		};

		
		//================================================================================
		/// 图层信息基类
		//================================================================================
		class MAPGISG3DTILEEXPORT LayersInfoBase
		{
		public:
			LayersInfoBase() {};
			virtual ~LayersInfoBase() {};

			//================================================================================
			///反序列化（将json内容转为图层信息）
			//================================================================================
			virtual gisLONG From(const CGByteArray& in) = 0;

			//================================================================================
			///序列化（将图层信息转json内容）
			//================================================================================
			virtual gisLONG To(CGByteArray& out) const = 0;
		};

		
		//================================================================================
		///三维模型图层信息
		//================================================================================
		class MAPGISG3DTILEEXPORT LayersInfo :public LayersInfoBase
		{
		public:
			LayersInfo();
			virtual ~LayersInfo();

			//================================================================================
			///反序列化（将json内容转为图层信息）
			//================================================================================
			virtual gisLONG From(const CGByteArray& in);

			//================================================================================
			///序列化（将图层信息转json内容）
			//================================================================================
			virtual gisLONG To(CGByteArray& out) const;
			
			//================================================================================
			///图层数
			//================================================================================
			int GetLayerNum() const;

			//================================================================================
			///获取指定索引的图层信息
			//================================================================================
			const LayerFieldsInfo& GetLayerInfo(int index)const;

		public:
			std::vector<LayerFieldsInfo> m_layersInfo;
		};

		//================================================================================
		///体元栅格图层信息
		//================================================================================
		class MAPGISG3DTILEEXPORT VoxelLayersInfo :public LayersInfoBase
		{
		public:
			VoxelLayersInfo();
			virtual ~VoxelLayersInfo();
			
			//================================================================================
			///反序列化（将json内容转为图层信息）
			//================================================================================
			virtual gisLONG From(const CGByteArray& in);

			//================================================================================
			///序列化（将图层信息转json内容）
			//================================================================================
			virtual gisLONG To(CGByteArray& out) const;
		public:
			
			LayerFieldsInfo m_layerInfo;	//图层信息
			VoxelModel m_voxelModel;		//栅格体元模型
			VoxelStyle * m_pVoxelStyle;		//栅格体元样式信息
			bool isFreeVoxelStyle;			//内部是否销毁样式信息
		};
	}
}
#endif