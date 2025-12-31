#include "stdafx.h"
#include "../include/g3dtilelayer.h"

#include "ci_3dmodel_attribute_att.h"
#include "ci_m3d_tile.h"
#include "ci_assist.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/rapidjson.h"

gisLONG i_ColorXtoJsonArray(rapidjson::Value &colorStops, rapidjson::Document::AllocatorType& allocator, double value, int r, int g, int b, int a)
{
	rapidjson::Value  arr(rapidjson::kArrayType);
	arr.PushBack(r, allocator);
	arr.PushBack(g, allocator);
	arr.PushBack(b, allocator);
	arr.PushBack(a, allocator);

	rapidjson::Value fromColorObj(rapidjson::kObjectType);
	fromColorObj.AddMember("color", arr, allocator);
	fromColorObj.AddMember("position", value, allocator);
	colorStops.PushBack(fromColorObj, allocator);
	return 1;
}

gisLONG i_SetVoxelStyle(rapidjson::Value& style, rapidjson::Document::AllocatorType& allocator, MapGIS::Tile::VoxelStyle * pVoxelStyle)
{
	MapGIS::Tile::VoxelStretchStyle* pVoxelStretchStyle = dynamic_cast<MapGIS::Tile::VoxelStretchStyle*>(pVoxelStyle);
	if (pVoxelStretchStyle == NULL)
		return 0;
	//属性字段名
	style.AddMember("fieldName", ToStringValue(pVoxelStretchStyle->m_fieldName, allocator), allocator);
	style.AddMember("type", "stretch", allocator);

	//拉伸范围
	rapidjson::Value stretchRange(rapidjson::kArrayType);
	stretchRange.PushBack(pVoxelStretchStyle->m_stretchRangeMinValue, allocator);
	stretchRange.PushBack(pVoxelStretchStyle->m_stretchRangeMaxValue, allocator);
	style.AddMember("stretchRange", stretchRange, allocator);

	//图层过滤显示
	rapidjson::Value rangeFilter(rapidjson::kArrayType);
	rangeFilter.PushBack(pVoxelStretchStyle->m_rangeFilterMinValue, allocator);
	rangeFilter.PushBack(pVoxelStretchStyle->m_rangeFilterMaxValue, allocator);
	style.AddMember("rangeFilter", rangeFilter, allocator);

	rapidjson::Value colorStops(rapidjson::kArrayType);
	for (vector<pair<int, MapGIS::Tile::Color4f>>::iterator itr = pVoxelStretchStyle->m_colorStops.begin(); itr != pVoxelStretchStyle->m_colorStops.end(); itr++)
	{
		i_ColorXtoJsonArray(colorStops, allocator, itr->first, (int)(itr->second.r * 255), (int)(itr->second.g * 255), (int)(itr->second.b * 255), (int)(itr->second.a * 255));
	}
	style.AddMember("colorStops", colorStops, allocator);
	//透明
	rapidjson::Value transparentInfo(rapidjson::kObjectType);
	transparentInfo.AddMember("enable", pVoxelStretchStyle->enableTransparent, allocator);
	rapidjson::Value transStops(rapidjson::kArrayType);

	for (vector<pair<int, int>>::iterator itr = pVoxelStretchStyle->m_alphaStops.begin(); itr != pVoxelStretchStyle->m_alphaStops.end(); itr++)
	{
		rapidjson::Value alphaObj(rapidjson::kObjectType);
		alphaObj.AddMember("position", itr->first, allocator);
		alphaObj.AddMember("alpha", itr->second, allocator);
		transStops.PushBack(alphaObj, allocator);
	}
	transparentInfo.AddMember("alphaStops", transStops, allocator);
	style.AddMember("transparentInfo", transparentInfo, allocator);

	return 1;
}

gisLONG  i_GetVoxelStyle(rapidjson::Value& style, MapGIS::Tile::VoxelStyle * pVoxelStyle, bool isFreeVoxelStyle)
{
	if (style.HasMember("type") && style["type"].IsString())
	{
		if (StrICmp(style["type"].GetString(), "stretch") != 0)
			return 0;
		if (pVoxelStyle != NULL && isFreeVoxelStyle)
			delete pVoxelStyle;
		MapGIS::Tile::VoxelStretchStyle* pVoxelStretchStyle = new  MapGIS::Tile::VoxelStretchStyle();
		pVoxelStyle = pVoxelStretchStyle;
		isFreeVoxelStyle = true;

		if (style.HasMember("fieldName") && style["fieldName"].IsString())
		{
			pVoxelStretchStyle->m_fieldName = CGString(style["fieldName"].GetString(), CGString::EncodeType::UTF8);
		}
		if (style.HasMember("stretchRange") && style["stretchRange"].IsArray() && style["stretchRange"].Size() == 2)
		{
			if (style["stretchRange"][0].IsNumber())
				pVoxelStretchStyle->m_stretchRangeMinValue = style["stretchRange"][0].GetDouble();
			if (style["stretchRange"][1].IsNumber())
				pVoxelStretchStyle->m_stretchRangeMaxValue = style["stretchRange"][1].GetDouble();
		}
		if (style.HasMember("rangeFilter") && style["rangeFilter"].IsArray() && style["rangeFilter"].Size() == 2)
		{
			if (style["rangeFilter"][0].IsNumber())
				pVoxelStretchStyle->m_rangeFilterMinValue = style["rangeFilter"][0].GetDouble();
			if (style["rangeFilter"][1].IsNumber())
				pVoxelStretchStyle->m_rangeFilterMaxValue = style["rangeFilter"][1].GetDouble();
		}

		if (style.HasMember("colorStops") && style["colorStops"].IsArray())
		{
			for (int i = 0; i < style["colorStops"].Size(); i++)
			{
				rapidjson::Value& colorStopsItem = style["colorStops"][i];
				int position = 0;
				if (colorStopsItem.HasMember("position") && colorStopsItem["position"].IsNumber())
				{
					position = JsonValueToInt(colorStopsItem["position"]);
				}
				MapGIS::Tile::Color4f color;
				if (colorStopsItem.HasMember("color") && colorStopsItem["color"].IsArray() && colorStopsItem["color"].Size() == 4)
				{
					if (colorStopsItem["color"][0].IsNumber())
						color.r = (colorStopsItem["color"][0].GetDouble() / 255.0);
					if (colorStopsItem["color"][1].IsNumber())
						color.g = (colorStopsItem["color"][1].GetDouble() / 255.0);
					if (colorStopsItem["color"][2].IsNumber())
						color.b = (colorStopsItem["color"][2].GetDouble() / 255.0);
					if (colorStopsItem["color"][3].IsNumber())
						color.a = (colorStopsItem["color"][3].GetDouble() / 255.0);
				}
				pVoxelStretchStyle->m_colorStops.push_back(make_pair(position, color));
			}
		}
		if (style.HasMember("transparentInfo") && style["transparentInfo"].IsObject())
		{
			rapidjson::Value& transparentInfo = style["transparentInfo"];

			if (transparentInfo.HasMember("enable") && transparentInfo["enable"].IsBool())
				pVoxelStretchStyle->enableTransparent = transparentInfo["enable"].GetBool();

			if (transparentInfo.HasMember("alphaStops") && transparentInfo["alphaStops"].IsArray())
			{
				rapidjson::Value& alphaStops = transparentInfo["alphaStops"];
				for (int i = 0; i < alphaStops.Size(); i++)
				{
					int position = 0;
					int alpha = 0;
					if (alphaStops[i].IsObject() && alphaStops[i].HasMember("position") && alphaStops[i]["position"].IsNumber())
						position = JsonValueToInt(alphaStops[i]["position"]);

					if (alphaStops[i].IsObject() && alphaStops[i].HasMember("alpha") && alphaStops[i]["alpha"].IsNumber())
						alpha = JsonValueToInt(alphaStops[i]["alpha"]);
					pVoxelStretchStyle->m_alphaStops.push_back(make_pair(position, alpha));
				}
			}
		}
	}
	return 0;
}

MapGIS::Tile::LayersInfo::LayersInfo() {}
MapGIS::Tile::LayersInfo::~LayersInfo(){}

gisLONG MapGIS::Tile::LayersInfo:: From(const CGByteArray& in)
{
	MapGIS::Tile::Ci_ModelLayerFieldsInfo info;
	m_layersInfo.clear();
	return info.From(in ,m_layersInfo);
}

gisLONG MapGIS::Tile::LayersInfo::To(CGByteArray& out) const
{
	MapGIS::Tile::Ci_ModelLayerFieldsInfo info;
	return info.To(m_layersInfo, out);
}

int MapGIS::Tile::LayersInfo::GetLayerNum() const
{
	return m_layersInfo.size();
}

const  MapGIS::Tile::LayerFieldsInfo&  MapGIS::Tile::LayersInfo::GetLayerInfo(int index)const
{
	if (index >= 0 && index < m_layersInfo.size())
		return m_layersInfo[index];
	else
		throw std::out_of_range("index 取值错误");
}

MapGIS::Tile::VoxelLayersInfo::VoxelLayersInfo()
{
	m_pVoxelStyle = NULL;
	isFreeVoxelStyle = false;
}
MapGIS::Tile::VoxelLayersInfo::~VoxelLayersInfo()
{
	if (isFreeVoxelStyle)
		delete m_pVoxelStyle;
	isFreeVoxelStyle = false;
	m_pVoxelStyle = NULL;
}

gisLONG MapGIS::Tile::VoxelLayersInfo::From(const CGByteArray& in)
{
	MapGIS::Tile::Ci_ModelLayerFieldsInfo tool;

	CGByteArray bayeArray;
	vector<LayerFieldsInfo> verInfo;
	tool.From(in, verInfo);
	if (verInfo.size() == 1)
	{
		m_layerInfo = verInfo[0];
	}
	rapidjson::Document doc;
	if (doc.Parse(bayeArray.data(), bayeArray.size()).HasParseError())
		return 0;
	if (!doc.IsArray() || doc.Size() <= 0)
		return 0;
	rapidjson::Value& layerIndex = doc[0];
	if (layerIndex.HasMember("voxelInfo") && layerIndex["voxelInfo"].IsObject())
	{
		M3D::Ci_Content::JsonObjToVoxelModel(layerIndex["voxelInfo"], m_voxelModel);
	}
	if (layerIndex.HasMember("style") && layerIndex["style"].IsArray() && layerIndex["style"].Size()>0)
	{
		i_GetVoxelStyle(layerIndex["style"][0],m_pVoxelStyle,isFreeVoxelStyle);
	}
	return 1;
}

gisLONG MapGIS::Tile::VoxelLayersInfo::To(CGByteArray& out)  const
{
	MapGIS::Tile::Ci_ModelLayerFieldsInfo tool;

	CGByteArray bayeArray;
	vector<LayerFieldsInfo> verInfo;
	verInfo.push_back(m_layerInfo);
	tool.To(verInfo, bayeArray);
	rapidjson::Document doc;
	if (doc.Parse(bayeArray.data(), bayeArray.size()).HasParseError())
		return 0;
	if (!doc.IsArray() || doc.Size() <= 0)
		return 0;

	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	rapidjson::Value objVoxel(rapidjson::kObjectType);

	M3D::Ci_Content::VoxelModelToJsonObj(m_voxelModel, objVoxel, allocator);

	if (objVoxel.HasMember("layerID"))
		objVoxel.RemoveMember("layerID");
	doc[0].AddMember("voxelInfo", objVoxel, allocator);

	//3、默认样式
	rapidjson::Value style(rapidjson::kObjectType);
	i_SetVoxelStyle(style, allocator, m_pVoxelStyle);
	if (!style.IsNull())
	{
		rapidjson::Value styles(rapidjson::kArrayType);
		styles.PushBack(style, allocator);
		doc[0].AddMember("style", styles, allocator);
	}
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	char* pBuffer = const_cast<char*>(buffer.GetString());
	out.append(pBuffer, buffer.GetLength());
	return 1;
}