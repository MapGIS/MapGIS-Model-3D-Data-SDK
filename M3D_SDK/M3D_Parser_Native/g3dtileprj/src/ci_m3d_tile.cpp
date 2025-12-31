#include "stdafx.h"
#include "ci_assist.h"
#include "ci_m3d_tile.h"
#include "rapidjson/document.h"

using namespace MapGIS::Tile;
namespace M3D
{
	class M3D20BoundingVolumeSerialization
	{
	public:
		M3D20BoundingVolumeSerialization() {};
		~M3D20BoundingVolumeSerialization() {};

		static gisLONG From(BoundingVolume& bounding, rapidjson::Value& jsonObj)
		{
			if (!jsonObj.IsObject())
				return 0;

			if (jsonObj.HasMember("boundingVolume") && jsonObj["boundingVolume"].IsObject())
			{
				rapidjson::Value& objBoundingVolume = jsonObj["boundingVolume"];
				if (objBoundingVolume.HasMember("boundingBox"))
				{
					bounding.type = BoundingType::Region;
					rapidjson::Value& value = objBoundingVolume["boundingBox"];
					if (value.IsObject())
					{
						if (value.HasMember("left") && value["left"].IsNumber())
							bounding.region.west = value["left"].GetDouble();

						if (value.HasMember("top") && value["top"].IsNumber())
							bounding.region.north = value["top"].GetDouble();

						if (value.HasMember("right") && value["right"].IsNumber())
							bounding.region.east = value["right"].GetDouble();

						if (value.HasMember("bottom") && value["bottom"].IsNumber())
							bounding.region.south = value["bottom"].GetDouble();

						if (value.HasMember("minHeight") && value["minHeight"].IsNumber())
							bounding.region.minHeight = value["minHeight"].GetDouble();

						if (value.HasMember("maxHeight") && value["maxHeight"].IsNumber())
							bounding.region.east = value["maxHeight"].GetDouble();
					}
				}
				else if (objBoundingVolume.HasMember("boundingSphere"))
				{
					bounding.type = BoundingType::Sphere;
					rapidjson::Value& value = objBoundingVolume["boundingSphere"];
					if (value.IsObject())
					{
						if (value.HasMember("radius") && value["radius"].IsNumber())
							bounding.sphere.radius = value["radius"].GetDouble();
						if (value.HasMember("center") && value["center"].IsObject())
						{
							rapidjson::Value& objCenter = value["center"];

							if (objCenter.HasMember("x") && objCenter["x"].IsNumber())
								bounding.sphere.center.x = objCenter["x"].GetDouble();

							if (objCenter.HasMember("y") && objCenter["y"].IsNumber())
								bounding.sphere.center.y = objCenter["y"].GetDouble();

							if (objCenter.HasMember("z") && objCenter["z"].IsNumber())
								bounding.sphere.center.z = objCenter["z"].GetDouble();
						}
					}
				}
				else if (objBoundingVolume.HasMember("box"))
				{
					bounding.type = BoundingType::Box;

					if (objBoundingVolume["box"] .IsArray() && objBoundingVolume["box"].Size() == 12)
					{
						rapidjson::Value& jsonArray = objBoundingVolume["box"];

						if (jsonArray[0].IsNumber() && jsonArray[1].IsNumber() && jsonArray[2].IsNumber())
						{
							bounding.box.center.x = jsonArray[0].GetDouble();
							bounding.box.center.y = jsonArray[1].GetDouble();
							bounding.box.center.z = jsonArray[2].GetDouble();
						}

						if (jsonArray[3].IsNumber() && jsonArray[4].IsNumber()&& jsonArray[5].IsNumber())
						{
							bounding.box.x_axis.x = jsonArray[3].GetDouble();
							bounding.box.x_axis.y = jsonArray[4].GetDouble();
							bounding.box.x_axis.z = jsonArray[5].GetDouble();
						}
						if (jsonArray[6].IsNumber() && jsonArray[7].IsNumber() && jsonArray[8].IsNumber())
						{
							bounding.box.y_axis.x = jsonArray[6].GetDouble();
							bounding.box.y_axis.y = jsonArray[7].GetDouble();
							bounding.box.y_axis.z = jsonArray[8].GetDouble();
						}
						if (jsonArray[9].IsNumber() && jsonArray[10].IsNumber() && jsonArray[11].IsNumber())
						{
							bounding.box.z_axis.x = jsonArray[9].GetDouble();
							bounding.box.z_axis.y = jsonArray[10].GetDouble();
							bounding.box.z_axis.z = jsonArray[11].GetDouble();
						}
					}
				}
			}
			return 1;
		}

		static gisLONG To(const BoundingVolume& bounding, rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator)
		{
			switch (bounding.type)
			{
			case BoundingType::Region:
			{
				rapidjson::Value objBoundingBox(rapidjson::kObjectType);
				rapidjson::Value objBoxValue(rapidjson::kObjectType);
				objBoxValue.AddMember("left", bounding.region.west, allocator);
				objBoxValue.AddMember("top", bounding.region.north, allocator);
				objBoxValue.AddMember("right", bounding.region.east, allocator);
				objBoxValue.AddMember("bottom", bounding.region.south, allocator);
				objBoxValue.AddMember("minHeight", bounding.region.minHeight, allocator);
				objBoxValue.AddMember("maxHeight", bounding.region.maxHeight, allocator);
				objBoundingBox.AddMember("boundingBox", objBoxValue, allocator);
				jsonObj.AddMember("boundingVolume", objBoundingBox, allocator);
			}
			break;
			case BoundingType::Sphere:
			{
				rapidjson::Value  objBoundingSphere(rapidjson::kObjectType);
				rapidjson::Value  objSphereValue(rapidjson::kObjectType);
				rapidjson::Value  objCenterValue(rapidjson::kObjectType);
				objCenterValue.AddMember("x", bounding.sphere.center.x, allocator);
				objCenterValue.AddMember("y", bounding.sphere.center.y, allocator);
				objCenterValue.AddMember("z", bounding.sphere.center.z, allocator);
				objSphereValue.AddMember("center", objCenterValue, allocator);
				objSphereValue.AddMember("radius", bounding.sphere.radius, allocator);
				objBoundingSphere.AddMember("boundingSphere", objSphereValue, allocator);
				jsonObj.AddMember("boundingVolume", objBoundingSphere, allocator);
			}
			break;
			case BoundingType::Box:
			{//m3d 不支持，转成 sphere ;
				double radius = 0;

				D_3DOT dot1 = D_3DOT{ + bounding.box.x_axis.x + bounding.box.y_axis.x + bounding.box.z_axis.x, + bounding.box.x_axis.y + bounding.box.y_axis.y +bounding.box.z_axis.y,+ bounding.box.x_axis.z + bounding.box.y_axis.z + bounding.box.z_axis.z };
				D_3DOT dot2 = D_3DOT{ + bounding.box.x_axis.x + bounding.box.y_axis.x - bounding.box.z_axis.x, + bounding.box.x_axis.y + bounding.box.y_axis.y -bounding.box.z_axis.y,+ bounding.box.x_axis.z + bounding.box.y_axis.z - bounding.box.z_axis.z };
				D_3DOT dot3 = D_3DOT{ + bounding.box.x_axis.x - bounding.box.y_axis.x + bounding.box.z_axis.x, + bounding.box.x_axis.y - bounding.box.y_axis.y +bounding.box.z_axis.y,+ bounding.box.x_axis.z - bounding.box.y_axis.z + bounding.box.z_axis.z };
				D_3DOT dot4 = D_3DOT{ + bounding.box.x_axis.x - bounding.box.y_axis.x - bounding.box.z_axis.x, + bounding.box.x_axis.y - bounding.box.y_axis.y -bounding.box.z_axis.y,+ bounding.box.x_axis.z - bounding.box.y_axis.z - bounding.box.z_axis.z };
				D_3DOT dot5 = D_3DOT{ - bounding.box.x_axis.x + bounding.box.y_axis.x + bounding.box.z_axis.x, - bounding.box.x_axis.y + bounding.box.y_axis.y +bounding.box.z_axis.y,- bounding.box.x_axis.z + bounding.box.y_axis.z + bounding.box.z_axis.z };
				D_3DOT dot6 = D_3DOT{ - bounding.box.x_axis.x + bounding.box.y_axis.x - bounding.box.z_axis.x, - bounding.box.x_axis.y + bounding.box.y_axis.y -bounding.box.z_axis.y,- bounding.box.x_axis.z + bounding.box.y_axis.z - bounding.box.z_axis.z };
				D_3DOT dot7 = D_3DOT{ - bounding.box.x_axis.x - bounding.box.y_axis.x + bounding.box.z_axis.x, - bounding.box.x_axis.y - bounding.box.y_axis.y +bounding.box.z_axis.y,- bounding.box.x_axis.z - bounding.box.y_axis.z + bounding.box.z_axis.z };
				D_3DOT dot8 = D_3DOT{ - bounding.box.x_axis.x - bounding.box.y_axis.x - bounding.box.z_axis.x, - bounding.box.x_axis.y - bounding.box.y_axis.y -bounding.box.z_axis.y,- bounding.box.x_axis.z - bounding.box.y_axis.z - bounding.box.z_axis.z };

				radius = max(sqrt((bounding.box.center.x - dot1.x)*(bounding.box.center.x - dot1.x) + (bounding.box.center.y - dot1.y)* (bounding.box.center.y - dot1.y) + (bounding.box.center.z - dot1.z)*(bounding.box.center.z - dot1.z)), radius);
				radius = max(sqrt((bounding.box.center.x - dot2.x)*(bounding.box.center.x - dot2.x) + (bounding.box.center.y - dot2.y)* (bounding.box.center.y - dot2.y) + (bounding.box.center.z - dot2.z)*(bounding.box.center.z - dot2.z)), radius);
				radius = max(sqrt((bounding.box.center.x - dot3.x)*(bounding.box.center.x - dot3.x) + (bounding.box.center.y - dot3.y)* (bounding.box.center.y - dot3.y) + (bounding.box.center.z - dot3.z)*(bounding.box.center.z - dot3.z)), radius);
				radius = max(sqrt((bounding.box.center.x - dot4.x)*(bounding.box.center.x - dot4.x) + (bounding.box.center.y - dot4.y)* (bounding.box.center.y - dot4.y) + (bounding.box.center.z - dot4.z)*(bounding.box.center.z - dot4.z)), radius);
				radius = max(sqrt((bounding.box.center.x - dot5.x)*(bounding.box.center.x - dot5.x) + (bounding.box.center.y - dot5.y)* (bounding.box.center.y - dot5.y) + (bounding.box.center.z - dot5.z)*(bounding.box.center.z - dot5.z)), radius);
				radius = max(sqrt((bounding.box.center.x - dot6.x)*(bounding.box.center.x - dot6.x) + (bounding.box.center.y - dot6.y)* (bounding.box.center.y - dot6.y) + (bounding.box.center.z - dot6.z)*(bounding.box.center.z - dot6.z)), radius);
				radius = max(sqrt((bounding.box.center.x - dot7.x)*(bounding.box.center.x - dot7.x) + (bounding.box.center.y - dot7.y)* (bounding.box.center.y - dot7.y) + (bounding.box.center.z - dot7.z)*(bounding.box.center.z - dot7.z)), radius);
				radius = max(sqrt((bounding.box.center.x - dot8.x)*(bounding.box.center.x - dot8.x) + (bounding.box.center.y - dot8.y)* (bounding.box.center.y - dot8.y) + (bounding.box.center.z - dot8.z)*(bounding.box.center.z - dot8.z)), radius);

				rapidjson::Value  objBoundingSphere(rapidjson::kObjectType);
				rapidjson::Value  objSphereValue(rapidjson::kObjectType);
				rapidjson::Value  objCenterValue(rapidjson::kObjectType);
				objCenterValue.AddMember("x", bounding.box.center.x, allocator);
				objCenterValue.AddMember("y", bounding.box.center.y, allocator);
				objCenterValue.AddMember("z", bounding.box.center.z, allocator);
				objSphereValue.AddMember("center", objCenterValue, allocator);
				objSphereValue.AddMember("radius", radius, allocator);
				objBoundingSphere.AddMember("boundingSphere", objSphereValue, allocator);
				jsonObj.AddMember("boundingVolume", objBoundingSphere, allocator);
				/*CGJsonObject objBoundingSphere;
				CGJsonArray objBoxValue;
				objBoxValue.append(bounding.box.center.x);
				objBoxValue.append(bounding.box.center.y);
				objBoxValue.append(bounding.box.center.z);

				objBoxValue.append(bounding.box.x_axis[0]);
				objBoxValue.append(bounding.box.x_axis[1]);
				objBoxValue.append(bounding.box.x_axis[2]);

				objBoxValue.append(bounding.box.y_axis[0]);
				objBoxValue.append(bounding.box.y_axis[1]);
				objBoxValue.append(bounding.box.y_axis[2]);

				objBoxValue.append(bounding.box.z_axis[0]);
				objBoxValue.append(bounding.box.z_axis[1]);
				objBoxValue.append(bounding.box.z_axis[2]);

				objBoundingSphere.insert(CGString("box"), objBoxValue);
				jsonObj.insert(CGString("boundingVolume"), objBoundingSphere);*/
			}
			break;
			default:
				break;
			}
			return 1;
		}
	};

	Ci_ChildTileInfo::Ci_ChildTileInfo()
	{
		Reset();
	}

	Ci_ChildTileInfo::~Ci_ChildTileInfo()
	{
	}

	void Ci_ChildTileInfo::Reset()
	{
		lodError = 0;
		boundingVolume = BoundingVolume();
		transform.clear();
		uri = "";
	}

	gisLONG Ci_ChildTileInfo::From(rapidjson::Value& jsonObj)
	{
		if (!jsonObj.IsObject())
			return -1;
		Reset();

		CGString    strValue;
		if (jsonObj.HasMember("lodError") && jsonObj["lodError"].IsNumber())
			this->lodError = jsonObj["lodError"].GetDouble();

		M3D20BoundingVolumeSerialization::From(this->boundingVolume, jsonObj);

		if (jsonObj.HasMember("transform") && jsonObj["transform"].IsArray())
		{
			this->transform.clear();
			rapidjson::Value& transformArray = jsonObj["transform"];
			if (transformArray.Size() == 16)
			{
				for (int i = 0; i < transformArray.Size(); i++)
				{
					if (transformArray[i].IsNumber())
						this->transform.push_back(transformArray[i].GetDouble());
				}
			}
			if (this->transform.size() != 16)
				this->transform.clear();
		}
		if (jsonObj.HasMember("uri") && jsonObj["uri"].IsString())
		{
			this->uri = CGString(jsonObj["uri"].GetString(),CGString::EncodeType::UTF8);
		}
		return 1;
	}

	gisLONG Ci_ChildTileInfo::To(rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator) const
	{
		if (!jsonObj.IsObject())
			return 0;

		jsonObj.AddMember("lodError", this->lodError, allocator);
		M3D20BoundingVolumeSerialization::To(this->boundingVolume, jsonObj, allocator);
		if (this->transform.size() == 16)
		{
			rapidjson::Value transformArray(rapidjson::kArrayType);

			for (int i = 0; i < 16; i++)
			{
				transformArray.PushBack(this->transform[i], allocator);
			}
			jsonObj.AddMember("transform", transformArray, allocator);
		}
		CGString gUri =  this->uri.Converted(CGString::EncodeType::UTF8);
		jsonObj.AddMember("uri", ToStringValue(gUri, allocator), allocator);
		return 1;
	}

	Ci_ChildTileInfo&  Ci_ChildTileInfo::operator=(const Ci_ChildTileInfo &other)
	{
		if (this == &other)
			return *this;
		this->lodError = other.lodError;
		this->boundingVolume = other.boundingVolume;
		this->transform.clear();
		this->transform.insert(this->transform.begin(), other.transform.begin(), other.transform.end());
		this->uri = other.uri;
		return *this;
	}

	Ci_Content::Ci_Content()
	{
		Reset();
	}

	Ci_Content::~Ci_Content()
	{
	}

	void Ci_Content::Reset()
	{
		tileData_uri = "";

		geometry_blobType = BlobType::glbx;
		geometry_uri = "";
		geometry_tid_uri = "";
		geometry_geometryType = GeometryType::Surface;
		geometry_geoCompressType = GeoCompressType::None;

		attribute_attType = AttFileType::JSON;
		attribute_uri = "";
		texture_uri = "";
		dataType = DataType::Model;
		mVoxelModel = VoxelModel();
	}
	Ci_Content& Ci_Content::operator=(const Ci_Content &other)
	{
		if (this == &other)
			return *this;
		tileData_uri = other.tileData_uri;
		geometry_blobType = other.geometry_blobType;
		geometry_uri = other.geometry_uri;
		geometry_tid_uri = other.geometry_tid_uri;
		geometry_geometryType = other.geometry_geometryType;
		geometry_geoCompressType = other.geometry_geoCompressType;
		attribute_attType = other.attribute_attType;
		attribute_uri = other.attribute_uri;
		texture_uri = other.texture_uri;
		dataType = other.dataType;
		mVoxelModel = other.mVoxelModel;
		return *this;
	}

	gisLONG Ci_Content::From(rapidjson::Value& jsonObj)
	{
		if (!jsonObj.IsObject())
			return 0;
		Reset();
		if (jsonObj.HasMember("tileData") && jsonObj["tileData"].IsObject())
		{
			if (jsonObj["tileData"].HasMember("uri") && jsonObj["tileData"]["uri"].IsString())
			{
				this->tileData_uri = CGString(jsonObj["tileData"]["uri"].GetString(), CGString::EncodeType::UTF8);
			}
		}

		if (jsonObj.HasMember("geometry") && jsonObj["geometry"].IsObject())
		{
			rapidjson::Value&  objGeometry = jsonObj["geometry"];
			if (objGeometry.HasMember("blobType") && objGeometry["blobType"].IsString())
			{
				const char* strValue = objGeometry["blobType"].GetString();
				if (StrICmp("glbx", strValue)== 0)
					this->geometry_blobType = BlobType::glbx;
				else if (StrICmp("glb", strValue) == 0)
					this->geometry_blobType = BlobType::glb;
				else if (StrICmp("b3dm", strValue) == 0 )
					this->geometry_blobType = BlobType::b3dm;
				else if (StrICmp("i3dm", strValue) == 0)
					this->geometry_blobType = BlobType::i3dm;
				else if (StrICmp("pnts", strValue) == 0 )
					this->geometry_blobType = BlobType::pnts;
				else if (StrICmp("cmpt", strValue) == 0)
					this->geometry_blobType = BlobType::cmpt;
			}

			if (objGeometry.HasMember("uri") && objGeometry["uri"].IsString())
			{
				this->geometry_uri = CGString(objGeometry["uri"].GetString(), CGString::EncodeType::UTF8);
			}

			if (objGeometry.HasMember("tidUri") && objGeometry["tidUri"].IsString())
			{
				this->geometry_tid_uri = CGString(objGeometry["tidUri"].GetString(), CGString::EncodeType::UTF8);
			}

			if (objGeometry.HasMember("geoCompressType") && objGeometry["geoCompressType"].IsString())
			{
				const char* strValue = objGeometry["geoCompressType"].GetString();

				if (StrICmp("draco", strValue) == 0)
					this->geometry_geoCompressType = GeoCompressType::Draco;
				else if (StrICmp("meshopt", strValue) == 0)
					this->geometry_geoCompressType = GeoCompressType::MeshOpt;
				else
					this->geometry_geoCompressType = GeoCompressType::None;
			}

			if (objGeometry.HasMember("geometryType") && objGeometry["geometryType"].IsString())
			{
				const char* strValue = objGeometry["geometryType"].GetString();
				if (StrICmp("Point", strValue) == 0)
					this->geometry_geometryType = GeometryType::Point;
				else if (StrICmp("Line", strValue) == 0)
					this->geometry_geometryType = GeometryType::Line;
				else if (StrICmp("Surface", strValue) == 0)
					this->geometry_geometryType = GeometryType::Surface;
				else if (StrICmp("Entity", strValue) == 0)
					this->geometry_geometryType = GeometryType::Entity;
				else
					this->geometry_geometryType = GeometryType::None;
			}
		}
		if (jsonObj.HasMember("attribute") && jsonObj["attribute"].IsObject())
		{
			rapidjson::Value&  objAttribute = jsonObj["attribute"];
			if (objAttribute.HasMember("attType") && objAttribute["attType"].IsString())
			{
				const char* strValue = objAttribute["attType"].GetString();
				/*if (i_stricmp("bin", strValue) == 0)
					this->attribute_attType = AttFileType::bin;
				else */
				if (StrICmp("json", strValue) == 0)
					this->attribute_attType = AttFileType::JSON;
				else if (StrICmp("att", strValue) == 0)
					this->attribute_attType = AttFileType::ATT;
			}
			if (objAttribute.HasMember("uri") && objAttribute["uri"].IsString())
			{
				this->attribute_uri = CGString(objAttribute["uri"].GetString(), CGString::EncodeType::UTF8);
			}
		}

		if (jsonObj.HasMember("texture") && jsonObj["texture"].IsObject())
		{
			if (jsonObj["texture"].HasMember("uri") && jsonObj["texture"]["uri"].IsString())
			{
				this->texture_uri = CGString(jsonObj["texture"]["uri"].GetString(), CGString::EncodeType::UTF8);
			}
		}

		if (jsonObj.HasMember("dataType") && jsonObj["dataType"].IsString())
		{
			const char* strValue = jsonObj["dataType"].GetString();
			if (StrICmp("Vector", strValue) == 0)
				dataType = DataType::Vector;
			else if (StrICmp("TiltPhotography", strValue))
				dataType = DataType::TiltPhotography;
			else if (StrICmp("Model", strValue))
				dataType = DataType::Model;
			else if (StrICmp("BIM", strValue))
				dataType = DataType::BIM;
			else if (StrICmp("PointCloud", strValue))
				dataType = DataType::PointCloud;
			else if (StrICmp("PipeLine", strValue))
				dataType = DataType::PipeLine;
			else if (StrICmp("GeoModel", strValue))
				dataType = DataType::GeoModel;
			else if (StrICmp("GeoGrid", strValue))
				dataType = DataType::GeoGrid;
			else if (StrICmp("GeoDrill", strValue))
				dataType = DataType::GeoDrill;
			else if (StrICmp("GeoSection", strValue))
				dataType = DataType::GeoSection;
			else if (StrICmp("Voxel", strValue))
				dataType = DataType::Voxel;
			else if (StrICmp("GaussianSplatting", strValue))
				dataType = DataType::GaussianSplatting;
		}
		if (DataType::Voxel == dataType)
		{
			JsonObjToVoxelModel(jsonObj, mVoxelModel);
		}
		return 1;
	};

	gisLONG Ci_Content::To(rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator) const
	{
		{
			rapidjson::Value objUri(rapidjson::kObjectType);
			CGString uil =  tileData_uri.Converted(CGString::EncodeType::UTF8);
			objUri.AddMember("uri", ToStringValue(uil, allocator), allocator);
			jsonObj.AddMember("tileData", objUri, allocator);
		}

		if (!this->geometry_uri.IsEmpty())
		{
			rapidjson::Value objGeometry(rapidjson::kObjectType);
			switch (this->geometry_blobType)
			{
			case BlobType::glb:
				objGeometry.AddMember("blobType", "glb", allocator);
				break;
			case BlobType::glbx:
				objGeometry.AddMember("blobType", "glbx", allocator);
				break;
			case BlobType::b3dm:
				objGeometry.AddMember("blobType", "b3dm", allocator);
				break;
			case BlobType::i3dm:
				objGeometry.AddMember("blobType", "i3dm", allocator);
				break;
			case BlobType::pnts:
				objGeometry.AddMember("blobType", "pnts", allocator);
				break;
			case BlobType::cmpt:
				objGeometry.AddMember("blobType", "cmpt", allocator);
				break;
			default:
				break;
			}
			CGString uil = geometry_uri.Converted(CGString::EncodeType::UTF8);

			objGeometry.AddMember("uri", ToStringValue(uil, allocator) , allocator);

			if (!this->geometry_tid_uri.IsEmpty())
			{
				CGString tid_uri = geometry_tid_uri.Converted(CGString::EncodeType::UTF8);
				objGeometry.AddMember("tidUri", ToStringValue(tid_uri, allocator), allocator);
			}

			switch (this->geometry_geoCompressType)
			{
			case GeoCompressType::Draco:
				objGeometry.AddMember("geoCompressType", "draco", allocator);
				break;
			case GeoCompressType::MeshOpt:
				objGeometry.AddMember("geoCompressType", "meshopt", allocator);
				break;
			default:
				break;
			}

			switch (this->geometry_geometryType)
			{
			case GeometryType::Point:
				objGeometry.AddMember("geometryType", "Point", allocator);
				break;
			case GeometryType::Line:
				objGeometry.AddMember("geometryType", "Line", allocator);
				break;
			case GeometryType::Surface:
				objGeometry.AddMember("geometryType", "Surface", allocator);
				break;
			case GeometryType::Entity:
				objGeometry.AddMember("geometryType", "Entity", allocator);
				break;
			default:
				break;
			}
			jsonObj.AddMember("geometry", objGeometry, allocator);
		}

		if (!this->attribute_uri.IsEmpty())
		{
			rapidjson::Value objAttribute(rapidjson::kObjectType);
			switch (this->attribute_attType)
			{
			case  AttFileType::JSON:
				objAttribute.AddMember("attType", "json", allocator);
				break;
			/*case  AttFileType::bin:
				objAttribute.AddMember("attType", "bin", allocator);
				break;*/
			case  AttFileType::ATT:
				objAttribute.AddMember("attType", "att", allocator);
				break;
			default:
				break;
			}
			CGString attributeUri = this->attribute_uri.Converted(CGString::EncodeType::UTF8);
			objAttribute.AddMember("uri",
				ToStringValue(attributeUri, allocator), allocator);
			jsonObj.AddMember("attribute", objAttribute, allocator);
		}

		if (!this->texture_uri.IsEmpty())
		{
			rapidjson::Value objTexture(rapidjson::kObjectType);
			CGString textureUri = this->texture_uri.Converted(CGString::EncodeType::UTF8);
			objTexture.AddMember("uri", ToStringValue(textureUri, allocator), allocator);
			jsonObj.AddMember("texture", objTexture, allocator);
		}
		switch (this->dataType)
		{
		case  DataType::Vector:
			jsonObj.AddMember("dataType", "Vector", allocator);
			break;
		case  DataType::TiltPhotography:
			jsonObj.AddMember("dataType", "TiltPhotography", allocator);
			break;
		case  DataType::Model:
			jsonObj.AddMember("dataType", "Model", allocator);
			break;
		case  DataType::BIM:
			jsonObj.AddMember("dataType", "BIM", allocator);
			break;
		case  DataType::PointCloud:
			jsonObj.AddMember("dataType", "PointCloud", allocator);
			break;
		case  DataType::PipeLine:
			jsonObj.AddMember("dataType", "PipeLine", allocator);
			break;
		case  DataType::GeoModel:
			jsonObj.AddMember("dataType", "GeoModel", allocator);
			break;
		case  DataType::GeoGrid:
			jsonObj.AddMember("dataType", "GeoGrid", allocator);
			break;
		case  DataType::GeoDrill:
			jsonObj.AddMember("dataType", "GeoDrill", allocator);
			break;
		case  DataType::GeoSection:
			jsonObj.AddMember("dataType", "GeoSection", allocator);
			break;
		case DataType::Voxel:
			jsonObj.AddMember("dataType", "Voxel", allocator);
			break;
		case DataType::GaussianSplatting:
			jsonObj.AddMember("dataType", "GaussianSplatting", allocator);
			break;
		default:
			break;
		}

		if (this->dataType == DataType::Voxel)
		{
			rapidjson::Value objVoxel(rapidjson::kObjectType);
			VoxelModelToJsonObj(mVoxelModel, objVoxel, allocator);
			jsonObj.AddMember("voxelInfo", objVoxel, allocator);
		}
		return 1;
	}

	gisLONG  Ci_Content::VoxelModelToJsonObj(const VoxelModel& voxelModel, rapidjson::Value& objVoxel, rapidjson::Document::AllocatorType& allocator)
	{
		//dim=
		rapidjson::Value arrDim(rapidjson::kArrayType);
		arrDim.PushBack(voxelModel.m_dimensionX, allocator);
		arrDim.PushBack(voxelModel.m_dimensionY, allocator);
		arrDim.PushBack(voxelModel.m_dimensionZ, allocator);
		objVoxel.AddMember("dimensions", arrDim, allocator);

		//boundingVolume
		rapidjson::Value objVolume(rapidjson::kObjectType);
		if (voxelModel.m_rangeType == VoxelModel::VoxelRangeTypeRegion)
		{
			rapidjson::Value objRegion(rapidjson::kObjectType);
			objRegion.AddMember("left", voxelModel.m_region.xmin, allocator);
			objRegion.AddMember("top", voxelModel.m_region.ymax, allocator);
			objRegion.AddMember("right", voxelModel.m_region.xmax, allocator);
			objRegion.AddMember("bottom", voxelModel.m_region.ymin, allocator);
			objRegion.AddMember("minHeight", voxelModel.m_region.zmin, allocator);
			objRegion.AddMember("maxHeight", voxelModel.m_region.zmax, allocator);
			objVolume.AddMember("boundingBox", objRegion, allocator);
		}
		else if (voxelModel.m_rangeType == VoxelModel::VoxelRangeTypeOrientedBox)
		{
			rapidjson::Value objOrientedBox(rapidjson::kObjectType);
			rapidjson::Value arrPosition(rapidjson::kArrayType);
			arrPosition.PushBack(voxelModel.m_boxPosition.x, allocator);
			arrPosition.PushBack(voxelModel.m_boxPosition.y, allocator);
			arrPosition.PushBack(voxelModel.m_boxPosition.z, allocator);
			objOrientedBox.AddMember("position", arrPosition, allocator);

			rapidjson::Value arrAxisX(rapidjson::kArrayType);
			arrAxisX.PushBack(voxelModel.m_boxAxisX.x, allocator);
			arrAxisX.PushBack(voxelModel.m_boxAxisX.y, allocator);
			arrAxisX.PushBack(voxelModel.m_boxAxisX.z, allocator);
			objOrientedBox.AddMember("axisX", arrAxisX, allocator);

			rapidjson::Value arrAxisY(rapidjson::kArrayType);
			arrAxisY.PushBack(voxelModel.m_boxAxisY.x, allocator);
			arrAxisY.PushBack(voxelModel.m_boxAxisY.y, allocator);
			arrAxisY.PushBack(voxelModel.m_boxAxisY.z, allocator);
			objOrientedBox.AddMember("axisY", arrAxisY, allocator);

			rapidjson::Value arrAxisZ(rapidjson::kArrayType);
			arrAxisZ.PushBack(voxelModel.m_boxAxisZ.x, allocator);
			arrAxisZ.PushBack(voxelModel.m_boxAxisZ.y, allocator);
			arrAxisZ.PushBack(voxelModel.m_boxAxisZ.z, allocator);
			objOrientedBox.AddMember("axisZ", arrAxisZ, allocator);
			objVolume.AddMember("orientedBox", objOrientedBox, allocator);
		}
		objVoxel.AddMember("boundingVolume", objVolume, allocator);

		//time
		rapidjson::Value objTime(rapidjson::kObjectType);
		objTime.AddMember("description", "", allocator);
		if (voxelModel.m_timeSpaceType == VoxelModel::VoxelTimeSpaceTypeRegular)
		{
			objTime.AddMember("size", voxelModel.m_regularTimeNodeCount, allocator);
			rapidjson::Value objRegular(rapidjson::kObjectType);
			objRegular.AddMember("offset", static_cast<int64_t>(voxelModel.m_regularTimeOffset), allocator);
			objRegular.AddMember("gap", static_cast<int64_t>(voxelModel.m_regularTimeGap), allocator);
			objTime.AddMember("regularSpacing", objRegular, allocator);
		}
		else if (voxelModel.m_timeSpaceType == VoxelModel::VoxelTimeSpaceTypeIrregular)
		{
			objTime.AddMember("size", (int)voxelModel.m_irregularTimeNodes.size(), allocator);

			rapidjson::Value objIrregular(rapidjson::kObjectType);
			rapidjson::Value arrTimeNode(rapidjson::kArrayType);
			for (int i = 0; i < voxelModel.m_irregularTimeNodes.size(); ++i)
			{
				arrTimeNode.PushBack(static_cast<int64_t>(voxelModel.m_irregularTimeNodes[i]), allocator);
			}
			objIrregular.AddMember("values", arrTimeNode, allocator);
			objTime.AddMember("irregularSpacing", objIrregular, allocator);
		}
		objVoxel.AddMember("time", objTime, allocator);
		//layerID
		objVoxel.AddMember("layerID", static_cast<int64_t>(voxelModel.m_layerID), allocator);
		return 1;
	}

	gisLONG  Ci_Content::JsonObjToVoxelModel(const rapidjson::Value& jsonObj, VoxelModel& mVoxelModel)
	{
		if (jsonObj.HasMember("voxelInfo") && jsonObj["voxelInfo"].IsObject())
		{
			const rapidjson::Value&  objVoxel = jsonObj["voxelInfo"];

			if (objVoxel.HasMember("dimensions") && objVoxel["dimensions"].IsArray() && objVoxel["dimensions"].Size() == 3)
			{
				mVoxelModel.m_dimensionX = JsonValueToInt(objVoxel["dimensions"][0]);
				mVoxelModel.m_dimensionY = JsonValueToInt(objVoxel["dimensions"][1]);
				mVoxelModel.m_dimensionZ = JsonValueToInt(objVoxel["dimensions"][2]);
			}

			mVoxelModel.m_rangeType = VoxelModel::VoxelRangeTypeUnknown;

			if (objVoxel.HasMember("boundingVolume") && objVoxel["boundingVolume"].IsObject())
			{
				if (objVoxel["boundingVolume"].HasMember("boundingBox") && objVoxel["boundingVolume"]["boundingBox"].IsObject())
				{
					const rapidjson::Value& objRegion = objVoxel["boundingVolume"]["boundingBox"];
					mVoxelModel.m_rangeType = VoxelModel::VoxelRangeTypeRegion;

					if (objRegion.HasMember("left") && objRegion["left"].IsNumber())
						mVoxelModel.m_region.xmin = objRegion["left"].GetDouble();

					if (objRegion.HasMember("right") && objRegion["right"].IsNumber())
						mVoxelModel.m_region.xmax = objRegion["right"].GetDouble();

					if (objRegion.HasMember("bottom") && objRegion["bottom"].IsNumber())
						mVoxelModel.m_region.ymin = objRegion["bottom"].GetDouble();

					if (objRegion.HasMember("top") && objRegion["top"].IsNumber())
						mVoxelModel.m_region.ymax = objRegion["top"].GetDouble();

					if (objRegion.HasMember("minHeight") && objRegion["minHeight"].IsNumber())
						mVoxelModel.m_region.zmin = objRegion["minHeight"].GetDouble();

					if (objRegion.HasMember("maxHeight") && objRegion["maxHeight"].IsNumber())
						mVoxelModel.m_region.zmax = objRegion["maxHeight"].GetDouble();
				}
				else if (objVoxel["boundingVolume"].HasMember("orientedBox") && objVoxel["boundingVolume"]["orientedBox"].IsObject())
				{
					mVoxelModel.m_rangeType = VoxelModel::VoxelRangeTypeOrientedBox;

					const rapidjson::Value&  objOrientedBox = objVoxel["boundingVolume"]["orientedBox"];

					if (objOrientedBox.HasMember("position") && objOrientedBox["position"].IsArray() && objOrientedBox["position"].Size() == 3)
					{
						if (objOrientedBox["position"][0].IsNumber())
							mVoxelModel.m_boxPosition.x = objOrientedBox["position"][0].GetDouble();

						if (objOrientedBox["position"][1].IsNumber())
							mVoxelModel.m_boxPosition.y = objOrientedBox["position"][1].GetDouble();

						if (objOrientedBox["position"][2].IsNumber())
							mVoxelModel.m_boxPosition.z = objOrientedBox["position"][2].GetDouble();
					}

					if (objOrientedBox.HasMember("axisX") && objOrientedBox["axisX"].IsArray() && objOrientedBox["axisX"].Size() == 3)
					{
						if (objOrientedBox["axisX"][0].IsNumber())
							mVoxelModel.m_boxAxisX.x = objOrientedBox["axisX"][0].GetDouble();

						if (objOrientedBox["axisX"][1].IsNumber())
							mVoxelModel.m_boxAxisX.y = objOrientedBox["axisX"][1].GetDouble();

						if (objOrientedBox["axisX"][2].IsNumber())
							mVoxelModel.m_boxAxisX.z = objOrientedBox["axisX"][2].GetDouble();
					}

					if (objOrientedBox.HasMember("axisY") && objOrientedBox["axisY"].IsArray() && objOrientedBox["axisY"].Size() == 3)
					{
						if (objOrientedBox["axisY"][0].IsNumber())
							mVoxelModel.m_boxAxisY.x = objOrientedBox["axisY"][0].GetDouble();

						if (objOrientedBox["axisY"][1].IsNumber())
							mVoxelModel.m_boxAxisY.y = objOrientedBox["axisY"][1].GetDouble();

						if (objOrientedBox["axisY"][2].IsNumber())
							mVoxelModel.m_boxAxisY.z = objOrientedBox["axisY"][2].GetDouble();
					}
					if (objOrientedBox.HasMember("axisZ") && objOrientedBox["axisZ"].IsArray() && objOrientedBox["axisZ"].Size() == 3)
					{
						if (objOrientedBox["axisZ"][0].IsNumber())
							mVoxelModel.m_boxAxisZ.x = objOrientedBox["axisZ"][0].GetDouble();

						if (objOrientedBox["axisZ"][1].IsNumber())
							mVoxelModel.m_boxAxisZ.y = objOrientedBox["axisZ"][1].GetDouble();

						if (objOrientedBox["axisZ"][2].IsNumber())
							mVoxelModel.m_boxAxisZ.z = objOrientedBox["axisZ"][2].GetDouble();
					}
				}
			}

			mVoxelModel.m_timeSpaceType = VoxelModel::VoxelTimeSpaceTypeUnknown;

			if (objVoxel.HasMember("time") && objVoxel["time"].IsObject())
			{
				const rapidjson::Value& objTime = objVoxel["time"];
				if (objTime.HasMember("regularSpacing") && objTime["regularSpacing"].IsObject())
				{
					const rapidjson::Value& objRegular = objTime["regularSpacing"];
					if (objTime.HasMember("size"))
						mVoxelModel.m_regularTimeNodeCount = JsonValueToInt(objTime["size"]);

					if (objRegular.HasMember("offset"))
						mVoxelModel.m_regularTimeOffset = JsonValueToInt64(objRegular["offset"]);

					if (objRegular.HasMember("gap"))
						mVoxelModel.m_regularTimeGap = JsonValueToInt64(objRegular["gap"]);
				}
				else if (objTime.HasMember("irregularSpacing") && objTime["irregularSpacing"].IsObject())
				{
					const rapidjson::Value& objIrregular = objTime["irregularSpacing"];
					if (objIrregular.HasMember("values") && objIrregular["values"].IsArray())
					{
						for (int i = 0; i < objIrregular["values"].Size(); ++i)
						{
							mVoxelModel.m_irregularTimeNodes.push_back(JsonValueToInt64(objIrregular["values"][i]));
						}
					}
					mVoxelModel.m_regularTimeNodeCount = mVoxelModel.m_irregularTimeNodes.size();
				}
				if (objVoxel.HasMember("layerID"))
					mVoxelModel.m_layerID = JsonValueToUInt(objVoxel["layerID"]);
			}
		}

		return 1;
	}

	Ci_Tile::Ci_Tile()
	{
		Reset();
	}

	Ci_Tile::~Ci_Tile()
	{
	}

	void Ci_Tile::Reset()
	{
		name = "";
		lodLevel = 0;
		boundingVolume = BoundingVolume();
		lodMode = LodMode::Pixel;
		lodType = RefineType::Replace;
		lodError = 0;
		transform.clear();
		parentNode_uri = "";
		childrenNode.clear();
		sharedUri = "";
		tileDataInfoIndex = 0;
		tileDataInfoList.clear();
		extend.clear();
	}

	Ci_Tile& Ci_Tile::operator=(const Ci_Tile &other)
	{
		if (this == &other)
			return *this;
		name = other.name;
		lodLevel = other.lodLevel;
		boundingVolume = other.boundingVolume;
		lodMode = other.lodMode;
		lodType = other.lodType;
		lodError = other.lodError;

		this->transform.clear();
		this->transform.insert(this->transform.begin(), other.transform.begin(), other.transform.end());

		parentNode_uri = other.parentNode_uri;

		childrenNode.clear();
		for (vector<Ci_ChildTileInfo>::const_iterator itr = other.childrenNode.begin(); itr != other.childrenNode.end(); itr++)
		{
			childrenNode.emplace_back(*itr);
		}
		sharedUri = other.sharedUri;
		tileDataInfoIndex = other.tileDataInfoIndex;

		tileDataInfoList.clear();
		for (vector<Ci_Content>::const_iterator itr = other.tileDataInfoList.begin(); itr != other.tileDataInfoList.end(); itr++)
		{
			tileDataInfoList.emplace_back(*itr);
		}
		extend.clear();
		for (map<CGString, CGString>::const_iterator itr = other.extend.begin(); itr != other.extend.end(); itr++)
		{
			extend.insert(make_pair(itr->first, itr->second));
		}
		return *this;
	}

	gisLONG Ci_Tile::From(rapidjson::Value& jsonObj)
	{
		if (!jsonObj.IsObject())
			return 0;
		Reset();

		if (jsonObj.HasMember("name") && jsonObj["name"].IsString())
		{
			this->name = CGString(jsonObj["name"].GetString(), CGString::EncodeType::UTF8);
		}

		if (jsonObj.HasMember("lodLevel") )
		{
			this->lodLevel  = JsonValueToInt(jsonObj["lodLevel"]);
		}

		M3D20BoundingVolumeSerialization::From(this->boundingVolume, jsonObj);
		if (jsonObj.HasMember("lodMode") && jsonObj["lodMode"].IsString())
		{
			const char* strLodMode = jsonObj["lodMode"].GetString();
			if(StrICmp("pixel", strLodMode)==0)
				this->lodMode = LodMode::Pixel;
			else if(StrICmp("distance", strLodMode) == 0)
				this->lodMode = LodMode::Distance;
			else
				this->lodMode = LodMode::None;
		}

		if (jsonObj.HasMember("lodType") && jsonObj["lodType"].IsString())
		{
			const char* strLodType = jsonObj["lodType"].GetString();
			if (StrICmp("ADD", strLodType) == 0)
				this->lodType = RefineType::Add;
			else if (StrICmp("REPLACE", strLodType) == 0)
				this->lodType = RefineType::Replace;
		}

		if (jsonObj.HasMember("lodError") && jsonObj["lodError"].IsNumber())
		{
			this->lodError = jsonObj["lodError"].GetDouble();
		}

		if (jsonObj.HasMember("transform") && jsonObj["transform"].IsArray())
		{
			this->transform.clear();
			rapidjson::Value& transformArray = jsonObj["transform"];
			if (transformArray.Size() == 16)
			{
				for (int i = 0; i < transformArray.Size(); i++)
				{
					if (transformArray[i].IsNumber())
						this->transform.push_back(transformArray[i].GetDouble());
				}
			}
			if (this->transform.size() != 16)
				this->transform.clear();
		}

		if (jsonObj.HasMember("parentNode") && jsonObj["parentNode"].IsObject())
		{
			if (jsonObj["parentNode"].HasMember("uri") && jsonObj["parentNode"]["uri"].IsString())
			{
				this->parentNode_uri = CGString(jsonObj["parentNode"]["uri"].GetString(), CGString::EncodeType::UTF8);
			}
		}

		// 提前清空所有节点
		this->childrenNode.clear();

		if (jsonObj.HasMember("childrenNode") && jsonObj["childrenNode"].IsArray())
		{
			if (jsonObj["childrenNode"].Size() > 0)
			{
				for (int i = 0; i <jsonObj["childrenNode"].Size(); i++)
				{
					if (jsonObj["childrenNode"][i].IsObject())
					{
						rapidjson::Value& objChildrenNode = jsonObj["childrenNode"][i];
						Ci_ChildTileInfo info;
						info.From(objChildrenNode);
						this->childrenNode.emplace_back(info);
					}
				}
			}
		}

		if (jsonObj.HasMember("shared") && jsonObj["shared"].IsObject())
		{
			if (jsonObj["shared"].HasMember("uri") && jsonObj["shared"]["uri"].IsString())
			{
				this->sharedUri = CGString(jsonObj["shared"]["uri"].GetString(), CGString::EncodeType::UTF8);
			}
		}

		if (jsonObj.HasMember("tileDataInfoIndex"))
		{
			this->tileDataInfoIndex = JsonValueToInt(jsonObj["tileDataInfoIndex"]);
		}

		this->tileDataInfoList.clear();
		if (jsonObj.HasMember("tileDataInfoList") && jsonObj["tileDataInfoList"].IsArray())
		{
			rapidjson::Value& objTileDataInfoArray = jsonObj["tileDataInfoList"];
			if (objTileDataInfoArray.Size() > 0)
			{
				for (int i = 0; i < objTileDataInfoArray.Size(); i++)
				{
					if (objTileDataInfoArray[i].IsObject())
					{
						Ci_Content info;
						info.From(objTileDataInfoArray[i]);
						this->tileDataInfoList.emplace_back(info);
					}
				}
			}
		}

		this->extend.clear();
		if (jsonObj.HasMember("extend") && jsonObj["extend"].IsArray())
		{
			rapidjson::Value& objExtendArray = jsonObj["extend"];

			if (objExtendArray.Size() > 0)
			{
				for (int i = 0; i < objExtendArray.Size(); i++)
				{
					if (objExtendArray[i].IsObject() &&objExtendArray[i].MemberCount() > 0)
					{
						for (rapidjson::Value::MemberIterator itr = objExtendArray[i].MemberBegin(); itr != objExtendArray[i].MemberEnd(); itr++)
						{
							if (itr->value.IsString() && itr->name.IsString())
							{
								CGString name = CGString(itr->name.GetString(), CGString::EncodeType::UTF8);
								if (this->extend.find(name) == this->extend.end())
								{
									this->extend.insert(make_pair(name, CGString(itr->value.GetString(), CGString::EncodeType::UTF8)));
								}
							}
						}
					}
				}
			}
		}

		return 1;
	}

	gisLONG Ci_Tile::To(rapidjson::Value& jsonObj, rapidjson::Document::AllocatorType& allocator)const
	{
		CGString strName = this->name.Converted(CGString::EncodeType::UTF8);
		jsonObj.AddMember("name", ToStringValue(strName, allocator), allocator);
		jsonObj.AddMember("lodLevel", this->lodLevel, allocator);

		M3D20BoundingVolumeSerialization::To(this->boundingVolume, jsonObj, allocator);
		switch (this->lodMode)
		{
		case LodMode::Distance:
			jsonObj.AddMember("lodMode", "distance", allocator);
			break;
		case LodMode::Pixel:
			jsonObj.AddMember("lodMode", "pixel", allocator);
			break;
		default:
			break;
		}

		switch (this->lodType)
		{
		case RefineType::Add:
			jsonObj.AddMember("lodType", "ADD", allocator);
			break;
		case RefineType::Replace:
			jsonObj.AddMember("lodType", "REPLACE", allocator);
			break;
		default:
			break;
		}
		jsonObj.AddMember("lodError", this->lodError, allocator);

		if (this->transform.size() == 16) {
			rapidjson::Value transformArray(rapidjson::kArrayType);
			for (int i = 0; i < 16; i++)
			{
				transformArray.PushBack(this->transform[i], allocator);
			}
			jsonObj.AddMember("transform", transformArray, allocator);
		}

		if (!this->parentNode_uri.IsEmpty())
		{
			rapidjson::Value objUri(rapidjson::kObjectType);
			CGString parentNodeUri =  this->parentNode_uri.Converted(CGString::EncodeType::UTF8);
			objUri.AddMember("uri", ToStringValue(parentNodeUri, allocator), allocator);
			jsonObj.AddMember("parentNode", objUri, allocator);
		}

		if (!this->childrenNode.empty())
		{
			rapidjson::Value childrenArray(rapidjson::kArrayType);
			for (vector<Ci_ChildTileInfo>::const_iterator it = this->childrenNode.begin(); it != this->childrenNode.end(); it++)
			{
				rapidjson::Value objChildren(rapidjson::kObjectType);
				it->To(objChildren, allocator);
				childrenArray.PushBack(objChildren, allocator);
			}
			jsonObj.AddMember("childrenNode", childrenArray, allocator);
		}
		if (!this->sharedUri.IsEmpty())
		{
			rapidjson::Value objUri(rapidjson::kObjectType);
			CGString uri = this->sharedUri.Converted(CGString::EncodeType::UTF8);
			objUri.AddMember("uri", ToStringValue(uri, allocator), allocator);
			jsonObj.AddMember("shared", objUri, allocator);
		}

		if (this->tileDataInfoIndex >= 0 && this->tileDataInfoIndex < this->tileDataInfoList.size())
		{
			jsonObj.AddMember("tileDataInfoIndex", this->tileDataInfoIndex, allocator);
		}

		if (!this->tileDataInfoList.empty())
		{
			rapidjson::Value objTileDataInfoArray(rapidjson::kArrayType);
			for (vector<Ci_Content>::const_iterator it = this->tileDataInfoList.begin(); it != this->tileDataInfoList.end(); it++)
			{
				rapidjson::Value objTileDataInfo(rapidjson::kObjectType);
				it->To(objTileDataInfo, allocator);
				objTileDataInfoArray.PushBack(objTileDataInfo, allocator);
			}
			jsonObj.AddMember("tileDataInfoList", objTileDataInfoArray, allocator);
		}

		if (!this->extend.empty())
		{
			rapidjson::Value objExtendArray(rapidjson::kArrayType);
			for (map<CGString, CGString>::const_iterator it = this->extend.begin(); it != this->extend.end(); it++)
			{
				rapidjson::Value objExtend(rapidjson::kObjectType);
				rapidjson::Value::StringRefType key( it->first.Converted(CGString::EncodeType::UTF8).CStr());
				CGString value = it->second.Converted(CGString::EncodeType::UTF8);
				objExtend.AddMember(key, ToStringValue(value, allocator), allocator);
				objExtendArray.PushBack(objExtend, allocator);
			}
			jsonObj.AddMember("extend", objExtendArray, allocator);
		}
		return 1;
	}

	Ci_Tileset::Ci_Tileset()
	{
		Reset();
	}

	Ci_Tileset::~Ci_Tileset()
	{
	}

	gisLONG Ci_Tileset::From(rapidjson::Value& jsonObj)
	{
		if (!jsonObj.IsObject())
			return 0;

		if (jsonObj.HasMember("asset") && jsonObj["asset"].IsString())
		{
			this->asset = CGString(jsonObj["asset"].GetString(), CGString::EncodeType::UTF8);
		}

		if (jsonObj.HasMember("version") && jsonObj["version"].IsString())
		{
			const char* version = jsonObj["version"].GetString();
			if(StrICmp("2.1", version) ==0 )
				this->version = M3DVersion::M3D21;
			else if(StrICmp("2.1=2", version) == 0)
				this->version = M3DVersion::M3D22;
		}

		if (jsonObj.HasMember("attInfo") && jsonObj["attInfo"].IsObject())
		{
			rapidjson::Value& objAttInfo = jsonObj["attInfo"];
			if (objAttInfo.HasMember("type") && objAttInfo["type"].IsString())
			{
				if (StrICmp("db", objAttInfo["type"].GetString()) == 0)
				{
					if (objAttInfo.HasMember("uri") && objAttInfo["uri"].IsString())
					{
						const char* uri = objAttInfo["uri"].GetString();
						this->sqlitePath = CGString(uri,CGString::EncodeType::UTF8);
					}
				}
			}
		}
		if (jsonObj.HasMember("dataName") && jsonObj["dataName"].IsString())
		{
			this->dataName = CGString(jsonObj["dataName"].GetString(), CGString::EncodeType::UTF8);
		}

		if (jsonObj.HasMember("guid") && jsonObj["guid"].IsString())
		{
			this->guid = CGString(jsonObj["guid"].GetString(), CGString::EncodeType::UTF8);
		}

		if (jsonObj.HasMember("compressType") && jsonObj["compressType"].IsString())
		{
			const char* compressType = jsonObj["compressType"].GetString();
			if (StrICmp(compressType, "ZIP") == 0)
				this->compressType = FileCompressType::Zip;
		}

		if (jsonObj.HasMember("spatialReference") && jsonObj["spatialReference"].IsString())
		{
			const char* spatialReference = jsonObj["spatialReference"].GetString();
			if (StrICmp(spatialReference, "WGS84") == 0)
				this->spatialReference = SpatialReference::WGS84;
			else if (StrICmp(spatialReference, "CGCS2000") == 0)
				this->spatialReference = SpatialReference::CGCS2000;
		}

		if (jsonObj.HasMember("treeType") && jsonObj["treeType"].IsString())
		{
			const char* treeType = jsonObj["treeType"].GetString();
			if (StrICmp(treeType, "OCTree") == 0)
				this->treeType = TreeType::Octree;
			else if (StrICmp(treeType, "K_DTree") == 0)
				this->treeType = TreeType::KDTree;
			else if (StrICmp(treeType, "RTree") == 0)
				this->treeType = TreeType::RTree;
			else
				this->treeType = TreeType::QuadTree;
		}

		if (jsonObj.HasMember("lodType") && jsonObj["lodType"].IsString())
		{
			const char* lodType = jsonObj["lodType"].GetString();
			if (StrICmp(lodType, "ADD") == 0)
				this->lodType = RefineType::Add;
			else if (StrICmp(lodType, "REPLACE") == 0)
				this->lodType = RefineType::Replace;
		}

		M3D20BoundingVolumeSerialization::From(this->boundingVolume, jsonObj);
		if (jsonObj.HasMember("position") && jsonObj["position"].IsObject())
		{
			if(jsonObj["position"].HasMember("x") && jsonObj["position"]["x"].IsNumber())
				this->position.x = jsonObj["position"]["x"].GetDouble();
			if (jsonObj["position"].HasMember("y") && jsonObj["position"]["y"].IsNumber())
				this->position.y = jsonObj["position"]["y"].GetDouble();
			if (jsonObj["position"].HasMember("z") && jsonObj["position"]["z"].IsNumber())
				this->position.z = jsonObj["position"]["z"].GetDouble();
		}
		if (jsonObj.HasMember("rootNode") && jsonObj["rootNode"].IsObject())
		{
			rapidjson::Value& objRootNode = jsonObj["rootNode"];
			if(objRootNode.HasMember("uri") && objRootNode["uri"].IsString())
			{
				this->rootNode_uri = CGString(objRootNode["uri"].GetString(),CGString::EncodeType::UTF8);
			}
		}

		if (jsonObj.HasMember("fieldInfo") && jsonObj["fieldInfo"].IsArray())
		{
			rapidjson::Value& fieldArray = jsonObj["fieldInfo"];

			for (int i = 0; i < fieldArray.Size(); i++)
			{
				if (fieldArray[i].IsObject())
				{
					rapidjson::Value& objField = fieldArray[i];
					Field field;
					if (objField.HasMember("name") && objField["name"].IsString())
					{
						field.name = CGString(objField["name"].GetString(), CGString::EncodeType::UTF8);
					}

					if (objField.HasMember("alias") && objField["alias"].IsString())
					{
						field.alias = CGString(objField["alias"].GetString(), CGString::EncodeType::UTF8);
					}

					if (objField.HasMember("type") && objField["type"].IsString())
					{
						const char* typeValue = objField["type"].GetString();
						if (StrICmp(typeValue, "bool") == 0)
							field.type = MapGIS::Tile::FieldType::BoolType;
						else if (StrICmp(typeValue, "int8") == 0 )
							field.type = MapGIS::Tile::FieldType::Int8Type;
						else if (StrICmp(typeValue, "uint8") == 0)
							field.type = MapGIS::Tile::FieldType::Uint8Type;
						else if (StrICmp(typeValue, "int16") == 0)
							field.type = MapGIS::Tile::FieldType::Int16Type;
						else if (StrICmp(typeValue, "uint16") == 0)
							field.type = MapGIS::Tile::FieldType::Uint16Type;
						else if (StrICmp(typeValue, "int32") == 0)
							field.type = MapGIS::Tile::FieldType::Int32Type;
						else if (StrICmp(typeValue, "uint32") == 0)
							field.type = MapGIS::Tile::FieldType::Uint32Type;
						else if (StrICmp(typeValue, "int64") == 0 )
							field.type = MapGIS::Tile::FieldType::Int64Type;
						else if (StrICmp(typeValue, "uint64") == 0)
							field.type = MapGIS::Tile::FieldType::Uint64Type;
						else if (StrICmp(typeValue, "float") == 0)
							field.type = MapGIS::Tile::FieldType::FloatType;
						else if (StrICmp(typeValue, "double") == 0)
							field.type = MapGIS::Tile::FieldType::DoubleType;
						else if (StrICmp(typeValue, "text") == 0)
							field.type = MapGIS::Tile::FieldType::TextType;
						else if (StrICmp(typeValue, "timestamp") == 0)
							field.type = MapGIS::Tile::FieldType::DateTimeType;
						else
							field.type = MapGIS::Tile::FieldType::Undefined;
					}
					this->fieldInfo.emplace_back(field);
				}
			}
		}
		if (jsonObj.HasMember("layerInfo") && jsonObj["layerInfo"].IsObject())
		{
			rapidjson::Value& objLayerInfo = jsonObj["layerInfo"];
			if (objLayerInfo.HasMember("uri") && objLayerInfo["uri"].IsString())
			{
				this->layerInfo_uri = CGString(objLayerInfo["uri"].GetString(), CGString::EncodeType::UTF8);
			}
		}

		if (jsonObj.HasMember("structureTree") && jsonObj["structureTree"].IsObject())
		{
			rapidjson::Value& objStructureTree = jsonObj["structureTree"];

			if (objStructureTree.HasMember("uri") && objStructureTree["uri"].IsString())
			{
				this->structureTree_uri = CGString(objStructureTree["uri"].GetString(), CGString::EncodeType::UTF8);
			}
		}

		return 1;
	}

	gisLONG Ci_Tileset::To(rapidjson::Value& jsonObj,rapidjson::Document::AllocatorType& allocator) const
	{
		if (!jsonObj.IsObject())
			return 0;

		CGString strAsset = this->asset.Converted(CGString::EncodeType::UTF8);
		jsonObj.AddMember("asset", ToStringValue(strAsset, allocator), allocator);

		if(version == M3DVersion::M3D20)
			jsonObj.AddMember("version", "2.0", allocator);
		else if (version == M3DVersion::M3D21)
			jsonObj.AddMember("version", "2.1", allocator);
		else if (version == M3DVersion::M3D22)
			jsonObj.AddMember("version", "2.2", allocator);

		CGString strDataName = this->dataName.Converted(CGString::EncodeType::UTF8);
		jsonObj.AddMember("dataName", ToStringValue(strDataName, allocator), allocator);

		CGString strGUID = this->guid.Converted(CGString::EncodeType::UTF8);
		jsonObj.AddMember("guid", ToStringValue(strGUID, allocator) , allocator);

		if (version != M3DVersion::M3D20 && !sqlitePath.IsEmpty())
		{
			CGString strSqlitePath = this->sqlitePath.Converted(CGString::EncodeType::UTF8);
			rapidjson::Value objUri(rapidjson::kObjectType);
			objUri.AddMember("uri", ToStringValue(strSqlitePath, allocator), allocator);
			objUri.AddMember("type", "db", allocator);
			jsonObj.AddMember("attInfo", objUri, allocator);
		}

		switch (this->compressType)
		{
		case FileCompressType::Zip:
			jsonObj.AddMember("compressType", "ZIP", allocator);
			break;
		default:
			break;
		}

		switch (this->spatialReference)
		{
		case SpatialReference::WGS84:
			jsonObj.AddMember("spatialReference", "WGS84", allocator);
			break;
		case SpatialReference::CGCS2000:
			jsonObj.AddMember("spatialReference", "CGCS2000", allocator);
			break;
		default:
			break;
		}

		switch (this->treeType)
		{
		case TreeType::Octree:
			jsonObj.AddMember("treeType", "OCTree", allocator);
			break;
		case TreeType::KDTree:
			jsonObj.AddMember("treeType", "K_DTree", allocator);
			break;
		case TreeType::RTree:
			jsonObj.AddMember("treeType", "RTree", allocator);
			break;
		default:
			jsonObj.AddMember("treeType", "QuadTree", allocator);
			break;
		}

		switch (this->lodType)
		{
		case RefineType::Add:
			jsonObj.AddMember("lodType", "ADD", allocator);
			break;
		case RefineType::Replace:
			jsonObj.AddMember("lodType", "REPLACE", allocator);
			break;
		default:
			break;
		}
		M3D20BoundingVolumeSerialization::To(this->boundingVolume, jsonObj, allocator);

		{
			rapidjson::Value objPositionValue(rapidjson::kObjectType);
			objPositionValue.AddMember("x", this->position.x, allocator);
			objPositionValue.AddMember("y", this->position.y, allocator);
			objPositionValue.AddMember("z", this->position.z, allocator);
			jsonObj.AddMember("position", objPositionValue, allocator);
		}

		{
			rapidjson::Value objUri(rapidjson::kObjectType);
			CGString rootNodeUri = this->rootNode_uri.Converted(CGString::EncodeType::UTF8);
			objUri.AddMember("uri", ToStringValue(rootNodeUri, allocator), allocator);
			jsonObj.AddMember("rootNode", objUri, allocator);
		}

		if (!this->fieldInfo.empty())
		{
			rapidjson::Value fieldArray(rapidjson::kArrayType);
			for (vector<Field>::const_iterator it = this->fieldInfo.begin(); it != this->fieldInfo.end(); it++)
			{
				rapidjson::Value objField(rapidjson::kObjectType);
				CGString strAlias = it->alias.Converted(CGString::EncodeType::UTF8);
				CGString strName = it->name.Converted(CGString::EncodeType::UTF8);
				objField.AddMember("alias", ToStringValue(strAlias, allocator), allocator);
				objField.AddMember("name", ToStringValue(strName, allocator), allocator);
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
					objField.AddMember("type", "timestamp", allocator);
					break;
				case MapGIS::Tile::FieldType::Undefined:
					break;
				default:
					break;
				}
				objField.AddMember("size", "0", allocator);
				fieldArray.PushBack(objField, allocator);
			}
			jsonObj.AddMember("fieldInfo", fieldArray, allocator);
		}

		if (version != M3DVersion::M3D20 && !layerInfo_uri.IsEmpty())
		{
			rapidjson::Value objUri(rapidjson::kObjectType);
			CGString strUri = this->layerInfo_uri.Converted(CGString::EncodeType::UTF8);
			objUri.AddMember("uri", ToStringValue(strUri, allocator) , allocator);
			jsonObj.AddMember("layerInfo", objUri, allocator);
		}
		if (version != M3DVersion::M3D20 && !structureTree_uri.IsEmpty())
		{
			rapidjson::Value objUri(rapidjson::kObjectType);
			CGString strUri = this->structureTree_uri.Converted(CGString::EncodeType::UTF8);
			objUri.AddMember("uri", ToStringValue(strUri, allocator), allocator);
			jsonObj.AddMember("structureTree", objUri, allocator);
		}

		return 1;
	}

	void Ci_Tileset::Reset()
	{
		asset = "Zondy Inc.";
		version = M3DVersion::M3D20;
		dataName = "";
		guid = "";
		compressType = FileCompressType::Zip;
		spatialReference = SpatialReference::WGS84;
		treeType = TreeType::QuadTree;
		lodType = RefineType::Replace;
		boundingVolume = BoundingVolume();
		position = { 0,0,0 };
		rootNode_uri = "";
		layerInfo_uri = "";
		structureTree_uri = "";
		fieldInfo.clear();
		sqlitePath = "";
	}

	Ci_Tileset& Ci_Tileset::operator=(const Ci_Tileset &other)
	{
		if (this == &other)
			return *this;
		asset = other.asset;
		version = other.version;
		guid = other.guid;
		compressType = other.compressType;
		spatialReference = other.spatialReference;
		treeType = other.treeType;
		lodType = other.lodType;
		boundingVolume = other.boundingVolume;
		position = other.position;
		rootNode_uri = other.rootNode_uri;
		layerInfo_uri = other.layerInfo_uri;
		structureTree_uri = other.structureTree_uri;
		fieldInfo.clear();
		for (vector<Field>::const_iterator itr = other.fieldInfo.begin(); itr != other.fieldInfo.end(); itr++)
		{
			fieldInfo.emplace_back(*itr);
		}
		return *this;
	}
}