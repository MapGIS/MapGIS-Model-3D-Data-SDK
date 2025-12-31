#include "stdafx.h"
#include "ci_3dtiles_tile.h"
#include "ci_assist.h"

namespace _3DTiles
{
	class G3dTilesBoundingVolumeSerialization
	{
	public:
		G3dTilesBoundingVolumeSerialization() {};
		~G3dTilesBoundingVolumeSerialization() {};

		static gisLONG ReadFrom(MapGIS::Tile::BoundingVolume& bounding, rapidjson::Value& jsonObj)
		{
			if (!jsonObj.IsObject())
				return 0;
			bounding.type = MapGIS::Tile::BoundingType::None;

			if (jsonObj.HasMember("box"))
			{
				if (jsonObj["box"].IsArray())
				{
					bounding.type = MapGIS::Tile::BoundingType::Box;
					rapidjson::Value& jsArr = jsonObj["box"];
					if (jsArr.Size() > 3)
					{
						if (jsArr[0].IsNumber())
							bounding.box.center.x = jsArr[0].GetDouble();

						if (jsArr[1].IsNumber())
							bounding.box.center.y= jsArr[1].GetDouble();

						if (jsArr[2].IsNumber())
							bounding.box.center.z = jsArr[2].GetDouble();

						if (jsArr.Size() == 12)
						{
							if (jsArr[3].IsNumber())
								bounding.box.x_axis.x = jsArr[3].GetDouble();

							if (jsArr[4].IsNumber())
								bounding.box.x_axis.y = jsArr[4].GetDouble();

							if (jsArr[5].IsNumber())
								bounding.box.x_axis.z = jsArr[5].GetDouble();

							if (jsArr[6].IsNumber())
								bounding.box.y_axis.x = jsArr[6].GetDouble();

							if (jsArr[7].IsNumber())
								bounding.box.y_axis.y = jsArr[7].GetDouble();

							if (jsArr[8].IsNumber())
								bounding.box.y_axis.z = jsArr[8].GetDouble();

							if (jsArr[9].IsNumber())
								bounding.box.z_axis.x = jsArr[9].GetDouble();

							if (jsArr[10].IsNumber())
								bounding.box.z_axis.y = jsArr[10].GetDouble();

							if (jsArr[11].IsNumber())
								bounding.box.z_axis.z = jsArr[11].GetDouble();
						}
					}
				}
			}

			if (jsonObj.HasMember("sphere"))
			{
				if (jsonObj["sphere"].IsArray())
				{
					bounding.type = MapGIS::Tile::BoundingType::Sphere;
					rapidjson::Value& jsArr = jsonObj["sphere"];
					if (jsArr.Size() == 4)
					{
						if (jsArr[0].IsNumber())
							bounding.sphere.center.x = jsArr[0].GetDouble();

						if (jsArr[1].IsNumber())
							bounding.sphere.center.y = jsArr[1].GetDouble();

						if (jsArr[2].IsNumber())
							bounding.sphere.center.z = jsArr[2].GetDouble();
						if (jsArr[3].IsNumber())
							bounding.sphere.radius = jsArr[3].GetDouble();
					}
				}
			}

			if (jsonObj.HasMember("region"))
			{
				if (jsonObj["region"].IsArray())
				{
					rapidjson::Value& jsArr = jsonObj["region"];
					if (jsArr.Size() == 6)
					{
						bounding.type = MapGIS::Tile::BoundingType::Region;
						if (jsArr[0].IsNumber())
							bounding.region.west = jsArr[0].GetDouble();

						if (jsArr[1].IsNumber())
							bounding.region.south = jsArr[1].GetDouble();

						if (jsArr[2].IsNumber())
							bounding.region.east = jsArr[2].GetDouble();

						if (jsArr[3].IsNumber())
							bounding.region.north = jsArr[3].GetDouble();

						if (jsArr[4].IsNumber())
							bounding.region.minHeight = jsArr[4].GetDouble();

						if (jsArr[5].IsNumber())
							bounding.region.maxHeight = jsArr[5].GetDouble();
					}
				}
			}
			return 1;
		}

		static gisLONG WriteTo(const MapGIS::Tile::BoundingVolume& bounding, rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator)
		{
			if (bounding.type == MapGIS::Tile::BoundingType::None)
				return 0;
			if (bounding.type == MapGIS::Tile::BoundingType::Box)
			{
				rapidjson::Value jsArr(rapidjson::kArrayType);
				jsArr.PushBack(bounding.box.center.x, allocator);
				jsArr.PushBack(bounding.box.center.y, allocator);
				jsArr.PushBack(bounding.box.center.z, allocator);

				jsArr.PushBack(bounding.box.x_axis.x, allocator);
				jsArr.PushBack(bounding.box.x_axis.y, allocator);
				jsArr.PushBack(bounding.box.x_axis.z, allocator);

				jsArr.PushBack(bounding.box.y_axis.x, allocator);
				jsArr.PushBack(bounding.box.y_axis.y, allocator);
				jsArr.PushBack(bounding.box.y_axis.z, allocator);

				jsArr.PushBack(bounding.box.z_axis.x, allocator);
				jsArr.PushBack(bounding.box.z_axis.y, allocator);
				jsArr.PushBack(bounding.box.z_axis.z, allocator);
				jsonObj.AddMember("box", jsArr, allocator);
			}
			else if (bounding.type == MapGIS::Tile::BoundingType::Sphere)
			{
				rapidjson::Value jsArr(rapidjson::kArrayType);
				jsArr.PushBack(bounding.sphere.center.x, allocator);
				jsArr.PushBack(bounding.sphere.center.y, allocator);
				jsArr.PushBack(bounding.sphere.center.z, allocator);
				jsArr.PushBack(bounding.sphere.radius, allocator);
				jsonObj.AddMember("sphere", jsArr, allocator);
			}
			else if (bounding.type == MapGIS::Tile::BoundingType::Region)
			{
				rapidjson::Value jsArr(rapidjson::kArrayType);
				jsArr.PushBack(bounding.region.west, allocator);
				jsArr.PushBack(bounding.region.south, allocator);
				jsArr.PushBack(bounding.region.east, allocator);
				jsArr.PushBack(bounding.region.north, allocator);
				jsArr.PushBack(bounding.region.minHeight, allocator);
				jsArr.PushBack(bounding.region.maxHeight, allocator);

				jsonObj.AddMember("region", jsArr, allocator);
			}
			return 1;
		}
	};

#pragma region Asset
	Ci_Asset::Ci_Asset() {}
	Ci_Asset::~Ci_Asset() {}
	void Ci_Asset::WriteTo(rapidjson::Value& tilesetObj, rapidjson::Document::AllocatorType& allocator) const
	{
		CGString version(m_version, CGString::EncodeType::GB18030);
		version.Convert(CGString::EncodeType::UTF8);
		CGString tilesetVersion(m_tilesetVersion, CGString::EncodeType::GB18030);
		tilesetVersion.Convert(CGString::EncodeType::UTF8);

		rapidjson::Value assetObj(rapidjson::kObjectType);
		assetObj.AddMember("version", ToStringValue(version, allocator), allocator);
		if (m_tilesetVersion.length() > 0)
			assetObj.AddMember("tilesetVersion", ToStringValue(tilesetVersion, allocator), allocator);
		tilesetObj.AddMember("asset", assetObj, allocator);
	}

	void Ci_Asset::ReadFrom(rapidjson::Value& tilesetObj)
	{
		if (!tilesetObj.IsObject())
			return;
		if (tilesetObj.HasMember("asset"))
		{
			rapidjson::Value& value = tilesetObj["asset"];
			if (!value.IsNull() && value.IsObject())
			{
				if (value.HasMember("version") && value["version"].IsString())
				{
					CGString version(value["version"].GetString(), CGString::EncodeType::UTF8);
					version.Convert(CGString::EncodeType::GB18030);
					m_version = version.StdString();
				}
				if (value.HasMember("tilesetVersion") && value["tilesetVersion"].IsString())
				{
					CGString tilesetVersion(value["tilesetVersion"].GetString(), CGString::EncodeType::UTF8);
					tilesetVersion.Convert(CGString::EncodeType::GB18030);
					m_tilesetVersion = tilesetVersion.StdString();
				}
			}
		}
	}
#pragma endregion

#pragma region TileContent
	Ci_3DTileContent::Ci_3DTileContent() { m_innerData.m_uriType = UriObjectType::uriObjectType_Unknown; }
	Ci_3DTileContent::~Ci_3DTileContent() {}
	void Ci_3DTileContent::WriteTo(rapidjson::Value& tileContentObj, rapidjson::Document::AllocatorType& allocator) const
	{
		if (m_innerData.m_uriType == UriObjectType::uriObjectType_FileModel)
		{
			CGString uri(m_innerData.m_uri, CGString::EncodeType::GB18030);
			uri.Convert(CGString::EncodeType::UTF8);
			tileContentObj.AddMember("uri", ToStringValue(uri, allocator) , allocator);
		}
		else if (m_innerData.m_uriType == UriObjectType::uriObjectType_Tileset)
		{
			CGString uri(m_innerData.m_uri, CGString::EncodeType::GB18030);
			uri.Convert(CGString::EncodeType::UTF8);
			tileContentObj.AddMember("uri", ToStringValue(uri, allocator) , allocator);
		}
		/*	else if (InnerData.UriType == UriObjectType.Base64Model)
		{
		UriObject uo = new UriObject(Model.GetFileData(), Path.GetExtension(Model.ModelFile));
		jObject.Add("uri", uo.WriteTo(new JObject()));
		}*/
	}

	void Ci_3DTileContent::ReadFrom(rapidjson::Value& tileContentObj)
	{
		if (!tileContentObj.IsObject())
			return;

		if (tileContentObj.HasMember("uri") && tileContentObj["uri"].IsString())
		{
			CGString uri(tileContentObj["uri"].GetString(), CGString::EncodeType::UTF8);
			uri.Convert(CGString::EncodeType::GB18030);
			m_innerData.m_uri = uri.StdString();
			if (m_innerData.m_uri.find(".json") != string::npos)
				m_innerData.m_uriType = UriObjectType::uriObjectType_Tileset;
			else
				m_innerData.m_uriType = UriObjectType::uriObjectType_FileModel;
		}
		else if (tileContentObj.HasMember("url") && tileContentObj["url"].IsString())
		{
			CGString uri(tileContentObj["url"].GetString(), CGString::EncodeType::UTF8);
			uri.Convert(CGString::EncodeType::GB18030);
			m_innerData.m_uri = uri.StdString();
			if (m_innerData.m_uri.find(".json") != string::npos)
				m_innerData.m_uriType = UriObjectType::uriObjectType_Tileset;
			else
				m_innerData.m_uriType = UriObjectType::uriObjectType_FileModel;
		}
	}

#pragma endregion

#pragma region Tile
	Ci_Tile::Ci_Tile()
	{
		m_refine = MapGIS::Tile::RefineType::None;
		m_geometricError = 0;
	}
	Ci_Tile::~Ci_Tile()
	{
	}
	void Ci_Tile::WriteTo(rapidjson::Value& tile, rapidjson::Document::AllocatorType& allocator) const
	{
		if (m_refine != MapGIS::Tile::RefineType::None)
		{
			string strRefineName = "NONE";
			switch (m_refine)
			{
			case MapGIS::Tile::RefineType::None:
				//strRefineName = "NONE";
				break;
			case MapGIS::Tile::RefineType::Add:
				strRefineName = "ADD";
				break;
			case MapGIS::Tile::RefineType::Replace:
				strRefineName = "REPLACE";
				break;
			default:
				break;
			}
			tile.AddMember("refine", ToStringValue(strRefineName, allocator) , allocator);
		}
		tile.AddMember("geometricError", (float)m_geometricError, allocator);

		rapidjson::Value boundingVolumeObj(rapidjson::kObjectType);
		G3dTilesBoundingVolumeSerialization::WriteTo(m_boundVolume,boundingVolumeObj, allocator);
		tile.AddMember("boundingVolume", boundingVolumeObj, allocator);

		if (m_content.m_innerData.m_uriType!= UriObjectType::uriObjectType_Unknown)
		{
			rapidjson::Value contentObj(rapidjson::kObjectType);
			m_content.WriteTo(contentObj, allocator);
			tile.AddMember("content", contentObj, allocator);
		}

		if (!m_matrix.IsUnit())
		{
			rapidjson::Value jsArr(rapidjson::kArrayType);
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					jsArr.PushBack(m_matrix[j][i], allocator);
				}
			}

			tile.AddMember("transform", jsArr, allocator);
		}

		if (!m_childs.empty())
		{
			rapidjson::Value childrenArr(rapidjson::kArrayType);
			for (int i = 0; i < m_childs.size(); ++i)
			{
				rapidjson::Value tmpChildObj(rapidjson::kObjectType);
				m_childs[i].WriteTo(tmpChildObj, allocator);
				childrenArr.PushBack(tmpChildObj, allocator);
			}
			tile.AddMember("children", childrenArr, allocator);
		}
	}

	void  Ci_Tile::ReadFrom(rapidjson::Value& tile)
	{
		if (!tile.IsObject())
			return;
		if(tile.HasMember("refine") && tile["refine"].IsString())
		{
			if ( StrICmp("REPLACE", tile["refine"].GetString())==0)
				m_refine = MapGIS::Tile::RefineType::Replace;
			else if (StrICmp("ADD", tile["refine"].GetString()) == 0)
				m_refine = MapGIS::Tile::RefineType::Add;
			else
				m_refine = MapGIS::Tile::RefineType::None;
		}

		if (tile.HasMember("geometricError") && tile["geometricError"].IsNumber())
		{
			m_geometricError = tile["geometricError"].GetDouble();
		}
		if (tile.HasMember("boundingVolume") && tile["boundingVolume"].IsObject())
		{
			G3dTilesBoundingVolumeSerialization::ReadFrom(m_boundVolume, tile["boundingVolume"]);
		}
		m_content.m_innerData.m_uriType = UriObjectType::uriObjectType_Unknown;
		if (tile.HasMember("content") && tile["content"].IsObject())
		{
			m_content.ReadFrom(tile["content"]);
		}
		m_matrix[0][0] = 1; m_matrix[0][1] = 0; m_matrix[0][2] = 0; m_matrix[0][3] = 0;
		m_matrix[1][0] = 0; m_matrix[1][1] = 1; m_matrix[1][2] = 0; m_matrix[1][3] = 0;
		m_matrix[2][0] = 0; m_matrix[2][1] = 0; m_matrix[2][2] = 1; m_matrix[2][3] = 0;
		m_matrix[3][0] = 0; m_matrix[3][1] = 0; m_matrix[3][2] = 0; m_matrix[3][3] = 1;

		if (tile.HasMember("transform") && tile["transform"].IsArray())
		{
			rapidjson::Value& jsArr = tile["transform"];

			if (jsArr.Size() == 16)
			{
				if (jsArr[0].IsNumber())
					m_matrix[0][0] = jsArr[0].GetDouble();
				if (jsArr[1].IsNumber())
					m_matrix[1][0] = jsArr[1].GetDouble();
				if (jsArr[2].IsNumber())
					m_matrix[2][0] = jsArr[2].GetDouble();
				if (jsArr[3].IsNumber())
					m_matrix[3][0] = jsArr[3].GetDouble();
				if (jsArr[4].IsNumber())
					m_matrix[0][1] = jsArr[4].GetDouble();
				if (jsArr[5].IsNumber())
					m_matrix[1][1] = jsArr[5].GetDouble();
				if (jsArr[6].IsNumber())
					m_matrix[2][1] = jsArr[6].GetDouble();
				if (jsArr[7].IsNumber())
					m_matrix[3][1] = jsArr[7].GetDouble();
				if (jsArr[8].IsNumber())
					m_matrix[0][2] = jsArr[8].GetDouble();
				if (jsArr[9].IsNumber())
					m_matrix[1][2] = jsArr[9].GetDouble();
				if (jsArr[10].IsNumber())
					m_matrix[2][2] = jsArr[10].GetDouble();
				if (jsArr[11].IsNumber())
					m_matrix[3][2] = jsArr[11].GetDouble();
				if (jsArr[12].IsNumber())
					m_matrix[0][3] = jsArr[12].GetDouble();
				if (jsArr[13].IsNumber())
					m_matrix[1][3] = jsArr[13].GetDouble();
				if (jsArr[14].IsNumber())
					m_matrix[2][3] = jsArr[14].GetDouble();
				if (jsArr[15].IsNumber())
					m_matrix[3][3] = jsArr[15].GetDouble();
			}
		}
		if (tile.HasMember("children") && tile["children"].IsArray())
		{
			for (int i = 0; i < tile["children"].Size(); i++)
			{
				if (tile["children"][i].IsObject())
				{
					m_childs.emplace_back(Ci_Tile());
					m_childs[m_childs.size() - 1].ReadFrom(tile["children"][i]);
				}
			}
		}
	}
#pragma endregion

#pragma region Tileset
	Ci_Tileset::Ci_Tileset(string file) :m_geometricError(0)
	{
	}
	Ci_Tileset::Ci_Tileset() : m_geometricError(0)
	{
		m_asset.m_version = "1.0";
	}
	Ci_Tileset::~Ci_Tileset()
	{
	}
	void Ci_Tileset::WriteTo(rapidjson::Value& tileset, rapidjson::Document::AllocatorType& allocator) const
	{
		m_asset.WriteTo(tileset, allocator);

		tileset.AddMember("geometricError", m_geometricError, allocator);

		rapidjson::Value rootObj(rapidjson::kObjectType);
		m_root.WriteTo(rootObj, allocator);
		tileset.AddMember("root", rootObj, allocator);

		rapidjson::Value fieldArray(rapidjson::kArrayType);
		for (vector<MapGIS::Tile::Field>::const_iterator it = this->m_fieldInfo.begin(); it != this->m_fieldInfo.end(); it++)
		{
			rapidjson::Value objField(rapidjson::kObjectType);
			CGString alias = it->alias.Converted(CGString::EncodeType::UTF8);
			CGString name = it->name.Converted(CGString::EncodeType::UTF8);

			objField.AddMember("alias", ToStringValue(alias, allocator), allocator);
			objField.AddMember("name", ToStringValue(name, allocator), allocator);
			switch (it->type)
			{
			case MapGIS::Tile::FieldType::BoolType:
				objField.AddMember("type", "bool", allocator);
				break;
			case MapGIS::Tile::FieldType::Int8Type:
				objField.AddMember("type", "int8", allocator);
				break;
			case MapGIS::Tile::FieldType::Uint8Type:
				objField.AddMember("type", "uint8", allocator);
				break;
			case MapGIS::Tile::FieldType::Int16Type:
				objField.AddMember("type", "int16", allocator);
				break;
			case MapGIS::Tile::FieldType::Uint16Type:
				objField.AddMember("type", "uint16", allocator);
				break;
			case MapGIS::Tile::FieldType::Int32Type:
				objField.AddMember("type", "int32", allocator);
				break;
			case MapGIS::Tile::FieldType::Uint32Type:
				objField.AddMember("type", "uint32", allocator);
				break;
			case MapGIS::Tile::FieldType::Int64Type:
				objField.AddMember("type", "int64", allocator);
				break;
			case MapGIS::Tile::FieldType::Uint64Type:
				objField.AddMember("type", "uint64", allocator);
				break;
			case MapGIS::Tile::FieldType::FloatType:
				objField.AddMember("type", "float", allocator);
				break;
			case MapGIS::Tile::FieldType::DoubleType:
				objField.AddMember("type", "double", allocator);
				break;
			case MapGIS::Tile::FieldType::TextType:
				objField.AddMember("type", "text", allocator);
				break;
			case MapGIS::Tile::FieldType::DateTimeType:
				objField.AddMember("type", "dateTime", allocator);
				break;
			case MapGIS::Tile::FieldType::Undefined:
				break;
			default:
				break;
			}
			fieldArray.PushBack(objField, allocator);
		}
		if (fieldArray.Size() > 0)
		{
			rapidjson::Value extensionsObj(rapidjson::kObjectType);
			extensionsObj.AddMember("mapgis_attribute_field", fieldArray, allocator);
			tileset.AddMember("extensions", extensionsObj, allocator);
		}
	}

	void Ci_Tileset::ReadFrom(rapidjson::Value& tileset)
	{
		if (!tileset.IsObject())
			return;
		m_asset.ReadFrom(tileset);
		if (tileset.HasMember("geometricError") && tileset["geometricError"].IsNumber())
		{
			m_geometricError = tileset["geometricError"].GetDouble();
		}

		if (tileset.HasMember("root") && tileset["root"].IsObject())
		{
			m_root.ReadFrom(tileset["root"]);
		}
		if (tileset.HasMember("extensions") && tileset["extensions"].IsObject())
		{
			if (tileset["extensions"].HasMember("mapgis_attribute_field"))
			{
				rapidjson::Value& value = tileset["extensions"]["mapgis_attribute_field"];
				if (value.IsArray())
				{
					for (int i = 0; i < value.Size(); i++)
					{
						if (value[i].IsObject())
						{
							rapidjson::Value& objField = value[i];
							MapGIS::Tile::Field field;
							if(objField.HasMember("name") && objField["name"].IsString())
							{
								field.name = CGString(objField["name"].GetString(), CGString::EncodeType::UTF8);
							}
							if (objField.HasMember("alias") && objField["alias"].IsString())
							{
								field.alias = CGString(objField["alias"].GetString(), CGString::EncodeType::UTF8);
							}

							if (objField.HasMember("type") && objField["type"].IsString())
							{
								CGString typeValue = CGString(objField["type"].GetString(), CGString::EncodeType::UTF8).Convert(CGString::EncodeType::GB18030);
								if (typeValue == "bool")
									field.type = MapGIS::Tile::FieldType::BoolType;
								else if (typeValue == "int8")
									field.type = MapGIS::Tile::FieldType::Int8Type;
								else if (typeValue == "uint8")
									field.type = MapGIS::Tile::FieldType::Uint8Type;
								else if (typeValue == "int16")
									field.type = MapGIS::Tile::FieldType::Int16Type;
								else if (typeValue == "uint16")
									field.type = MapGIS::Tile::FieldType::Uint16Type;
								else if (typeValue == "int32")
									field.type = MapGIS::Tile::FieldType::Int32Type;
								else if (typeValue == "uint32")
									field.type = MapGIS::Tile::FieldType::Uint32Type;
								else if (typeValue == "int64")
									field.type = MapGIS::Tile::FieldType::Int64Type;
								else if (typeValue == "uint64")
									field.type = MapGIS::Tile::FieldType::Uint64Type;
								else if (typeValue == "float")
									field.type = MapGIS::Tile::FieldType::FloatType;
								else if (typeValue == "double")
									field.type = MapGIS::Tile::FieldType::DoubleType;
								else if (typeValue == "text")
									field.type = MapGIS::Tile::FieldType::TextType;
								else if (typeValue == "timestamp")
									field.type = MapGIS::Tile::FieldType::DateTimeType;
								else
									field.type = MapGIS::Tile::FieldType::Undefined;
							}
							this->m_fieldInfo.emplace_back(field);
						}
					}
				}
			}
		}
	}
#pragma endregion
}