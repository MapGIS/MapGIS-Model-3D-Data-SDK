#include "stdafx.h"
#include "ci_m3d_tile.h"
#include "../include/g3dtile.h"
#include "../include/g3dtileinfo.h"
#include "ci_ziptool.h"
#include "ci_assist.h"
#include "ci_3dmodel.h"
#include "ci_3dmodel_tid.h"
#include "ci_3dmodel_attribute_att.h"
#include "ci_3dmodel_attribute_json.h"
#include "ci_3dmodel_attribute_sqlite.h"
#include "ci_structure_tree.h"

#include "ci_m3d_tile.h"
#include "ci_cache_tile_m3d_impl.h"
using namespace M3D;

// 检查标准库头文件
#ifdef _HAS_3D_TILES_
#include "ci_cache_tile_3dtiles_impl.h"
using namespace _3DTiles;
#endif



MapGIS::Tile::G3DTile*  MapGIS::Tile::G3DTile::Open(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath, MapGIS::Tile::G3DCacheType cacheType)
{
	switch (cacheType)
	{
	case MapGIS::Tile::G3DCacheType::TypeM3DV20:
		return  Ci_G3DM3D20CacheTileImpl::Open(pCacheStorage, relativePath);
	case MapGIS::Tile::G3DCacheType::TypeM3DV21:
		return Ci_G3DM3D21CacheTileImpl::Open(pCacheStorage, relativePath);
	case MapGIS::Tile::G3DCacheType::TypeM3DV22:
		return Ci_G3DM3D22CacheTileImpl::Open(pCacheStorage, relativePath);

#ifdef _HAS_3D_TILES_
	case MapGIS::Tile::G3DCacheType::Type3DTilesV10:
		return Ci_G3D3DTiles10CacheTileImpl::Open(pCacheStorage, relativePath);
#endif
	default:
		break;
	}
	return NULL;
}

MapGIS::Tile::G3DTileset* MapGIS::Tile::G3DTileset::CreateInstance(MapGIS::Tile::G3DCacheType cacheType)
{
	switch (cacheType)
	{
	case MapGIS::Tile::G3DCacheType::TypeM3DV20:
		return new  Ci_G3DM3D20CacheTilesetImpl();
	case MapGIS::Tile::G3DCacheType::TypeM3DV21:
		return new  Ci_G3DM3D21CacheTilesetImpl();
	case MapGIS::Tile::G3DCacheType::TypeM3DV22:
		return new  Ci_G3DM3D22CacheTilesetImpl();

#ifdef _HAS_3D_TILES_
	case MapGIS::Tile::G3DCacheType::Type3DTilesV10:
		return new  Ci_G3D3DTiles10CacheTilesetImpl();
#endif
	default:
		break;
	}
	return NULL;
}

MapGIS::Tile::G3DM3DTileChildInfo::G3DM3DTileChildInfo(M3D::Ci_ChildTileInfo* info, bool inFreeInfo)
{
	m_pDelegate = info;
	if (inFreeInfo)
		m_flagDelegate = 0;
	else
		m_flagDelegate = 1;
}

BEGIN_SET_DELEGATE(MapGIS::Tile::G3DM3DTileChildInfo, M3D::Ci_ChildTileInfo*)
SET_DELEGATE
END_SET_DELEGATE

MapGIS::Tile::G3DM3DTileChildInfo::~G3DM3DTileChildInfo()
{
	DELETE_DELEGATE
}

MapGIS::Tile::BoundingVolume MapGIS::Tile::G3DM3DTileChildInfo::GetBoundingVolume()const
{
	if (m_pDelegate != NULL)
		return m_pDelegate->boundingVolume;
	return MapGIS::Tile::BoundingVolume();
}

//MapGIS::Tile::Matrix4_d MapGIS::Tile::G3DM3DTileChildInfo::GetMatrix()const
//{
//	if (m_pDelegate != NULL)
//	{
//		vector<double> transform = m_pDelegate->transform;
//		if (transform.size() == 16)
//		{
//			MapGIS::Tile::Matrix4_d	 matrix;
//			matrix[0][0] = transform[0]; matrix[0][1] = transform[4]; matrix[0][2] = transform[8]; matrix[0][3] = transform[12];
//			matrix[1][0] = transform[1]; matrix[1][1] = transform[5]; matrix[1][2] = transform[9]; matrix[1][3] = transform[13];
//			matrix[2][0] = transform[2]; matrix[2][1] = transform[6]; matrix[2][2] = transform[10]; matrix[2][3] = transform[14];
//			matrix[3][0] = transform[3]; matrix[3][1] = transform[7]; matrix[3][2] = transform[11]; matrix[3][3] = transform[15];
//			return matrix;
//		}
//
//	}
//	return MapGIS::Tile::Matrix4_d();
//}

CGString MapGIS::Tile::G3DM3DTileChildInfo::GetURL()const
{
	if (m_pDelegate != NULL)
		return m_pDelegate->uri;
	return "";
}

MapGIS::Tile::G3DM3DTileInfo::G3DM3DTileInfo(M3D::Ci_Tile* pTile, bool inFreeInfo)
{
	m_pDelegate = pTile;
	if (inFreeInfo)
		m_flagDelegate = 0;
	else
		m_flagDelegate = 1;
}

BEGIN_SET_DELEGATE(MapGIS::Tile::G3DM3DTileInfo, M3D::Ci_Tile*)
SET_DELEGATE
END_SET_DELEGATE

MapGIS::Tile::G3DM3DTileInfo::~G3DM3DTileInfo()
{
	for (map<int, MapGIS::Tile::G3DTileChildInfo*>::iterator itr = m_childInfo.begin(); itr != m_childInfo.end(); itr++)
	{
		if (itr->second != NULL)
			delete itr->second;
	}
	m_childInfo.clear();
	DELETE_DELEGATE
}

MapGIS::Tile::BoundingVolume MapGIS::Tile::G3DM3DTileInfo::GetBoundingVolume()const
{
	if (m_pDelegate != NULL)
		return m_pDelegate->boundingVolume;
	return MapGIS::Tile::BoundingVolume();
}


MapGIS::Tile::Matrix4D MapGIS::Tile::G3DM3DTileInfo::GetMatrix()const
{
	if (m_pDelegate != NULL)
	{
		vector<double> transform = m_pDelegate->transform;
		if (transform.size() == 16)
		{
			MapGIS::Tile::Matrix4D	 matrix;
			matrix[0][0] = transform[0]; matrix[0][1] = transform[4]; matrix[0][2] = transform[8]; matrix[0][3] = transform[12];
			matrix[1][0] = transform[1]; matrix[1][1] = transform[5]; matrix[1][2] = transform[9]; matrix[1][3] = transform[13];
			matrix[2][0] = transform[2]; matrix[2][1] = transform[6]; matrix[2][2] = transform[10]; matrix[2][3] = transform[14];
			matrix[3][0] = transform[3]; matrix[3][1] = transform[7]; matrix[3][2] = transform[11]; matrix[3][3] = transform[15];
			return matrix;
		}

	}
	return MapGIS::Tile::Matrix4D();
}

MapGIS::Tile::TileContentInfo MapGIS::Tile::G3DM3DTileInfo::GetContentInfo() const
{
	if (m_pDelegate != NULL)
	{
		return GetContentInfo(m_pDelegate->tileDataInfoIndex);
	}
	return MapGIS::Tile::TileContentInfo();
}


gisLONG MapGIS::Tile::G3DM3DTileInfo::GetChildNum()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->childrenNode.size();
	}
	return 0;
}

MapGIS::Tile::G3DTileChildInfo* MapGIS::Tile::G3DM3DTileInfo::GetTileChildInfo(int index)
{
	if (m_childInfo.find(index) != m_childInfo.end())
		return m_childInfo[index];

	if (index >= 0 && index < GetChildNum())
	{
		G3DM3DTileChildInfo* rtn = new G3DM3DTileChildInfo(&m_pDelegate->childrenNode[index], false);
		m_childInfo.insert(make_pair(index, rtn));
		return rtn;
	}
	else
		throw std::out_of_range("index 取值错误");
}

CGString MapGIS::Tile::G3DM3DTileInfo::GetName()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->name;
	}
	return "";
}

gisLONG MapGIS::Tile::G3DM3DTileInfo::GetLevel()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->lodLevel;
	}
	return 0;
}

MapGIS::Tile::LodMode MapGIS::Tile::G3DM3DTileInfo::GetLodMode()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->lodMode;
	}
	return MapGIS::Tile::LodMode::None;
}

MapGIS::Tile::RefineType MapGIS::Tile::G3DM3DTileInfo::GetLodType()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->lodType;
	}
	return MapGIS::Tile::RefineType::None;
}

float MapGIS::Tile::G3DM3DTileInfo::GetLodError()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->lodError;
	}
	return 0;
}

CGString MapGIS::Tile::G3DM3DTileInfo::GetParentNodeURI()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->parentNode_uri;
	}
	return "";
}

CGString MapGIS::Tile::G3DM3DTileInfo::GetSharedURI()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->sharedUri;
	}
	return "";
}

gisLONG MapGIS::Tile::G3DM3DTileInfo::GetContentNum()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->tileDataInfoList.size();
	}
	return 0;
}

gisLONG MapGIS::Tile::G3DM3DTileInfo::GetUseContentIndex()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->tileDataInfoIndex;
	}
	return -1;
}

MapGIS::Tile::TileContentInfo MapGIS::Tile::G3DM3DTileInfo::GetContentInfo(gisLONG index)const
{
	TileContentInfo rtn;
	if (m_pDelegate != NULL)
	{

		if (m_pDelegate->tileDataInfoList.size() > 0 && index < m_pDelegate->tileDataInfoList.size())
		{
			Ci_Content content = m_pDelegate->tileDataInfoList[m_pDelegate->tileDataInfoIndex];

			CGString geoUri = content.geometry_uri;
			if (geoUri.ReverseFind(".") >= 0)
			{
				CGString geoType = geoUri.Right(geoUri.GetLength() - geoUri.ReverseFind(".") - 1);
				if (StrICmp("i3dm", geoType.CStr()) == 0)
					rtn.contentType = ContentType::M3DI3DM;
				else if (StrICmp("pnts", geoType.CStr()) == 0)
					rtn.contentType = ContentType::M3DPNTS;
				else if (StrICmp("cmpt", geoType.CStr()) == 0)
					rtn.contentType = ContentType::M3DCMPT;
				else
					rtn.contentType = ContentType::M3DGLB;
			}

			while (geoUri.StartsWith("./"))
			{
				geoUri = geoUri.Right(geoUri.GetLength() - 2);
			}
			rtn.tidURI = content.geometry_tid_uri;
			rtn.geometryURI = geoUri;
			rtn.dataType = content.dataType;
			rtn.geometryType = content.geometry_geometryType;
			rtn.contentURI = content.tileData_uri;
			if (!content.attribute_uri.IsEmpty())
			{
				rtn.attributeURI = content.attribute_uri;
			}
		}
	}
	return rtn;
}

gisLONG MapGIS::Tile::G3DM3DTileInfo::GetExtendNum()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->extend.size();
	}
	return 0;
}

map<CGString, CGString>  MapGIS::Tile::G3DM3DTileInfo::GetExtend()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->extend;
	}
	return map<CGString, CGString>();
}


MapGIS::Tile::G3DM3DTilesetInfo::G3DM3DTilesetInfo(M3D::Ci_Tileset* pTileset, bool inFreeInfo)
{
	m_pDelegate = pTileset;
	if (inFreeInfo)
		m_flagDelegate = 0;
	else
		m_flagDelegate = 1;

	if (m_pDelegate != NULL)
	{
		Ci_ChildTileInfo *pChildTileInfo = new Ci_ChildTileInfo();
		pChildTileInfo->uri = m_pDelegate->rootNode_uri;
		pRootChildInfo = new G3DM3DTileChildInfo(pChildTileInfo, true);
	}
	else
		pRootChildInfo = NULL;


}
BEGIN_SET_DELEGATE(MapGIS::Tile::G3DM3DTilesetInfo, M3D::Ci_Tileset*)
SET_DELEGATE
END_SET_DELEGATE

MapGIS::Tile::G3DM3DTilesetInfo::~G3DM3DTilesetInfo()
{
	if (pRootChildInfo != NULL)
		delete pRootChildInfo;
	DELETE_DELEGATE
}

MapGIS::Tile::BoundingVolume MapGIS::Tile::G3DM3DTilesetInfo::GetBoundingVolume()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->boundingVolume;
	}
	return MapGIS::Tile::BoundingVolume();
}

MapGIS::Tile::G3DTileChildInfo* MapGIS::Tile::G3DM3DTilesetInfo::GetRootInfo()const
{
	if (m_pDelegate != NULL)
	{
		return pRootChildInfo;
	}
	return NULL;
}

CGString MapGIS::Tile::G3DM3DTilesetInfo::GetAsset()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->asset;
	}
	return "";
}

MapGIS::Tile::M3DVersion MapGIS::Tile::G3DM3DTilesetInfo::GetVersion()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->version;
	}
	return MapGIS::Tile::M3DVersion::M3D20;
}

CGString MapGIS::Tile::G3DM3DTilesetInfo::GetName()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->dataName;
	}
	return "";
}

GUID MapGIS::Tile::G3DM3DTilesetInfo::GetGUID()const
{
	if (m_pDelegate != NULL)
	{
		GUID guid;
		String2GUID(m_pDelegate->guid.CStr(), guid);
		return guid;
	}
	return GUID();
}

MapGIS::Tile::FileCompressType MapGIS::Tile::G3DM3DTilesetInfo::GetCompressType()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->compressType;
	}
	return MapGIS::Tile::FileCompressType::Zip;
}

MapGIS::Tile::SpatialReference MapGIS::Tile::G3DM3DTilesetInfo::GetSpatialReference()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->spatialReference;
	}
	return MapGIS::Tile::SpatialReference::WGS84;
}

MapGIS::Tile::TreeType MapGIS::Tile::G3DM3DTilesetInfo::GetTreeType()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->treeType;
	}
	return MapGIS::Tile::TreeType::QuadTree;
}

MapGIS::Tile::RefineType MapGIS::Tile::G3DM3DTilesetInfo::GetLodType()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->lodType;
	}
	return MapGIS::Tile::RefineType::None;
}

D_3DOT MapGIS::Tile::G3DM3DTilesetInfo::GetPosition()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->position;
	}
	return D_3DOT{ 0,0,0 };
}

vector<MapGIS::Tile::Field> MapGIS::Tile::G3DM3DTilesetInfo::GetFields()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->fieldInfo;
	}
	return vector<MapGIS::Tile::Field>();
}

CGString MapGIS::Tile::G3DM3DTilesetInfo::GetRootNodeURI()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->rootNode_uri;
	}
	return "";
}

CGString MapGIS::Tile::G3DM3DTilesetInfo::GetLayerInfoURI()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->layerInfo_uri;
	}
	return "";
}

CGString MapGIS::Tile::G3DM3DTilesetInfo::GetStructureTree()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->structureTree_uri;
	}
	return "";
}

CGString MapGIS::Tile::G3DM3DTilesetInfo::GetSqlitePath()const
{
	if (m_pDelegate != NULL)
	{
		return m_pDelegate->sqlitePath;
	}
	return "";
}

unsigned int MapGIS::Tile::Common::API_CalcAttFieldID(CGString name)
{
	return CalcAttFieldID(name);
}

unsigned int MapGIS::Tile::Common::API_CalcAttLayerID(CGString dataSource, CGString layerName)
{
	return CalcAttLayerID(dataSource, layerName);
}

CGString MapGIS::Tile::Common::API_MakePathByRelativePath(CGString orPath, CGString relativePath)
{
	return MakePathByRelativePath(orPath, relativePath);
}
gisLONG MapGIS::Tile::Common::API_ReadGltfContent(const CGByteArray& byteArray, G3DCacheStorage *pCacheStorage, MapGIS::Tile::G3DModel*pModel)
{
	MapGIS::Tile::Ci_ModelGltf gltf;
	if (pCacheStorage != NULL &&pCacheStorage->IsValid())
		gltf.SetCacheStorage(pCacheStorage);
	((Ci_TileModel*)&gltf)->From(byteArray);
	return gltf.To(pModel);
}

gisLONG MapGIS::Tile::Common::API_WriteGltfContent(MapGIS::Tile::G3DModel*pModel, G3DCacheStorage *pCacheStorage, GeoCompressType  geoCompressType, int idMode, D_3DOT centerDot, CGByteArray& byteArray)
{
	MapGIS::Tile::Ci_ModelGltf gltf;
	if (pCacheStorage != NULL &&pCacheStorage->IsValid())
		gltf.SetCacheStorage(pCacheStorage);
	gltf.From(pModel, geoCompressType, (WriteIdType)idMode, centerDot);
	return ((Ci_TileModel*)&gltf)->To(byteArray);
}
gisLONG MapGIS::Tile::Common::API_ReadGltfContent(const CGByteArray& byteArray, G3DCacheStorage *pCacheStorage, vector<ContentBase*>* pContents)
{
	MapGIS::Tile::Ci_ModelGltf gltf;
	if (pCacheStorage != NULL &&pCacheStorage->IsValid())
		gltf.SetCacheStorage(pCacheStorage);
	((Ci_TileModel*)&gltf)->From(byteArray);
	return gltf.To(pContents);
}

gisLONG MapGIS::Tile::Common::API_WriteGltfContent(vector<ContentBase*>* pContents, G3DCacheStorage *pCacheStorage, GeoCompressType  geoCompressType, int idMode, bool addEXT_mesh_gpu_instancing, D_3DOT centerDot, CGByteArray& byteArray)
{
	MapGIS::Tile::Ci_ModelGltf gltf;
	if (pCacheStorage != NULL &&pCacheStorage->IsValid())
		gltf.SetCacheStorage(pCacheStorage);
	gltf.From(pContents, geoCompressType, (WriteIdType)idMode, addEXT_mesh_gpu_instancing, centerDot);
	return ((Ci_TileModel*)&gltf)->To(byteArray);
}


MapGIS::Tile::G3DCacheStorage* GetStorageAndFileName(const CGString& path, CGString & fileName)
{
#ifdef _WIN32
	CGString newPath = path.Converted(CGString::EncodeType::GB18030);
#else
	CGString newPath = path.Converted(CGString::EncodeType::UTF8);
#endif
	newPath.Replace('\\', '/');
	int index = newPath.ReverseFind('/');
	if (index <= 0)
	{
		return NULL;
	}
	CGString dirUrl = newPath.Left(index);
	fileName = newPath.Right(newPath.GetLength() - index - 1);
	MapGIS::Tile::G3DCacheStorage* rtn = MapGIS::Tile::G3DCacheStorage::CreateInstance(dirUrl);
	if (rtn == NULL)
		return NULL;
	if (rtn->Open() <= 0)
	{
		delete rtn;
		return NULL;
	}
	return rtn;


}

MapGIS::Tile::G3DM3DTileInfo* MapGIS::Tile::Common::API_GetM3DTileInfo(const CGString& path)
{
	MapGIS::Tile::G3DM3DTileInfo* rtn = NULL;
	CGString fileName = "";
	MapGIS::Tile::G3DCacheStorage* pStorage = GetStorageAndFileName(path, fileName);

	if (pStorage != NULL)
	{
		CGByteArray byteArray;
		pStorage->GetContent(fileName, byteArray);
		rtn = API_GetM3DTileInfo(byteArray);
		delete pStorage;
	}
	return rtn;

}

MapGIS::Tile::G3DM3DTilesetInfo* MapGIS::Tile::Common::API_GetM3DTilesetInfo(const CGString& path)
{
	MapGIS::Tile::G3DM3DTilesetInfo* rtn = NULL;
	CGString fileName = "";
	MapGIS::Tile::G3DCacheStorage* pStorage = GetStorageAndFileName(path, fileName);
	if (pStorage != NULL)
	{
		CGByteArray byteArray;
		pStorage->GetContent(fileName, byteArray);
		rtn = API_GetM3DTilesetInfo(byteArray);
		delete pStorage;
	}
	return rtn;
}

MapGIS::Tile::G3DM3DTileInfo* MapGIS::Tile::Common::API_GetM3DTileInfo(const CGByteArray& byteArray)
{
	rapidjson::Document doc;
	if (doc.Parse(byteArray.data(), byteArray.size()).HasParseError())
		return NULL;
	M3D::Ci_Tile *pTiel = new M3D::Ci_Tile();
	if (pTiel->From(doc) <= 0)
	{
		delete pTiel;
		return NULL;
	}
	MapGIS::Tile::G3DM3DTileInfo* rtn = new MapGIS::Tile::G3DM3DTileInfo(pTiel, true);
	return rtn;
}

MapGIS::Tile::G3DM3DTilesetInfo* MapGIS::Tile::Common::API_GetM3DTilesetInfo(const CGByteArray& byteArray)
{
	rapidjson::Document doc;
	if (doc.Parse(byteArray.data(), byteArray.size()).HasParseError())
		return NULL;
	M3D::Ci_Tileset *pTiel = new M3D::Ci_Tileset();
	if (pTiel->From(doc) <= 0)
	{
		delete pTiel;
		return NULL;
	}
	MapGIS::Tile::G3DM3DTilesetInfo* rtn = new MapGIS::Tile::G3DM3DTilesetInfo(pTiel, true);
	return rtn;

}

gisLONG MapGIS::Tile::Common::API_ReadM3DContent(const CGString& m3dPath, const CGString* attPath, vector<MapGIS::Tile::ContentBase*>* pContents)
{
	gisLONG rtn = 0;
	CGString m3dFileName = "";
	MapGIS::Tile::G3DCacheStorage* pM3DStorage = GetStorageAndFileName(m3dPath, m3dFileName);

	if (pM3DStorage != NULL)
	{
		CGByteArray m3dByteArray;
		CGByteArray attByteArray;
		pM3DStorage->GetContent(m3dFileName, m3dByteArray);
		if (attPath != NULL)
		{
			CGString attFileName = "";
			MapGIS::Tile::G3DCacheStorage* pAttStorage = GetStorageAndFileName(*attPath, attFileName);
			if (pAttStorage != NULL)
			{
				pAttStorage->GetContent(attFileName, attByteArray);
				delete pAttStorage;
			}
		}
		if (attByteArray.size() > 0)
			rtn = API_ReadM3DContent(m3dByteArray, &attByteArray, pContents);
		else
			rtn = API_ReadM3DContent(m3dByteArray, NULL, pContents);
		delete pM3DStorage;
	}
	return rtn;
}

gisLONG MapGIS::Tile::Common::API_ReadM3DByteArray(const CGString& m3dPath, CGByteArray& content, CGByteArray& tid, CGByteArray& att, ContentType& contentType)
{
	gisLONG rtn = 0;
	CGString fileName = "";
	MapGIS::Tile::G3DCacheStorage* pStorage = GetStorageAndFileName(m3dPath, fileName);
	if (pStorage != NULL)
	{
		CGByteArray byteArray;
		pStorage->GetContent(fileName, byteArray);
		rtn = API_ReadM3DByteArray(byteArray, content, tid, att, contentType);
		delete pStorage;
	}
	return rtn;
}

gisLONG MapGIS::Tile::Common::API_ReadM3DByteArray(const CGByteArray& m3d, CGByteArray& content, CGByteArray& tid, CGByteArray& att, ContentType& contentType)
{
	string geoName = "";
	string attName = "";
	string tidName = "";
	if (Ci_ZipTool::ZipBuffer2Buffer(m3d, content, geoName, att, attName, tid, tidName) > 0)
	{
		if (content.size() > 0)
		{
			string magic = ReadByteArrayToString(content, 0, 4);
			if (StrNICmp("i3dm", magic.c_str(), 4) == 0)
				contentType = ContentType::M3DI3DM;
			else if (StrNICmp("pnts", magic.c_str(), 4) == 0)
				contentType = ContentType::M3DPNTS;
			else if (StrNICmp("cmpt", magic.c_str(), 4) == 0)
				contentType = ContentType::M3DCMPT;
			else
				contentType = ContentType::M3DGLB;
		}
		return 1;
	}
	return 0;
}


gisLONG MapGIS::Tile::Common::API_ReadM3DContent(const CGByteArray& m3dByteArray, const CGByteArray* attByteArray, vector<MapGIS::Tile::ContentBase*>* pContents)
{
	gisLONG rtn = 0;
	CGByteArray content;
	CGByteArray tid;
	CGByteArray att;
	ContentType contentType;
	rtn = API_ReadM3DByteArray(m3dByteArray, content, tid, att, contentType);
	if (rtn <= 0)
		return rtn;
	if (att.size() <= 0 && attByteArray != NULL)
	{
		att = *attByteArray;
	}
	return API_ByteArrayToContent(content, tid, att, pContents);
}

gisLONG MapGIS::Tile::Common::API_ByteArrayToContent(const CGByteArray& contentArray, const CGByteArray& tidArray, const CGByteArray& attArray, vector<MapGIS::Tile::ContentBase*>* pContents)
{
	string magic = ReadByteArrayToString(contentArray, 0, 4);
	if (StrNICmp("i3dm", magic.c_str(), 4) == 0)
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true,GeometryType::Surface, true, true);
		Ci_ModelI3dm gltf;
		((Ci_TileModel*)&gltf)->From(contentArray);
		LayerAttribute records;
		D_3DOT rtcCenter;
		gltf.To((PointsModel*)pItem->Get3DModel(), *pItem->GetModelInstance(), records, rtcCenter);
		pContents->emplace_back(pItem);
	}
	else if (StrNICmp("pnts", magic.c_str(), 4) == 0)
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Point, true, false);
		Ci_ModelPnts pnts;
		((Ci_TileModel*)&pnts)->From(contentArray);
		LayerAttribute records;
		D_3DOT rtcCenter;
		pnts.To(*(MapGIS::Tile::PointsModel*)pItem->Get3DModel(), records, rtcCenter);
		pContents->emplace_back(pItem);
	}
	else if (StrNICmp("cmpt", magic.c_str(), 4) == 0)
	{
		Ci_ModelCmpt cmpt;
		((Ci_TileModel*)&cmpt)->From(contentArray);
		cmpt.To(pContents);
	}
	else if (StrNICmp("b3dm", magic.c_str(), 4) == 0)
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Surface, false, false);
		Ci_ModelB3dm b3dm;
		((Ci_TileModel*)&b3dm)->From(contentArray);
		LayerAttribute records;
		D_3DOT rtcCenter;
		b3dm.To(pItem->Get3DModel(), records, rtcCenter);
		pContents->emplace_back(pItem);
	}
	else  if (StrNICmp("gltf", magic.c_str(), 4) == 0)
	{
		Ci_ModelGltf gltf;
		((Ci_TileModel*)&gltf)->From(contentArray);
		gltf.To(pContents);
	}
	if (tidArray.size() > 0)
	{
		vector<vector<gisINT64>> tidVer;
		Ci_ModelTidFile tid;
		tid.From(tidArray, tidVer);
		if (tidVer.size() > 0 && tidVer.size() == pContents->size())
		{
			for (int i = 0; i < pContents->size(); i++)
			{
				if (pContents->at(i) != NULL && dynamic_cast<GeometryContent*>(pContents->at(i)) != NULL)
				{
					GeometryContent* pGeoContent = dynamic_cast<GeometryContent*>(pContents->at(i));

					if (pGeoContent->GetModelInstance() != NULL && pGeoContent->GetModelInstance()->size() > 0)
						UpdateId(pGeoContent->GetModelInstance(), tidVer[i]);
					else
						UpdateId(pGeoContent->Get3DModel(), tidVer[i]);
				}
			}
		}
	}

	if (attArray.size() > 0)
	{
		Attribute  outAttribute;
		if (API_GetAttribute(attArray, outAttribute) > 0)
			GetDataAttributeByAllAttribute(pContents, &outAttribute);
	}
	return 1;
}

bool PntsHasOID(const CGByteArray& contentArray)
{
	bool hassId = false;
	string magic = ReadByteArrayToString(contentArray, 0, 4);
	if (StrNICmp("pnts", magic.c_str(), 4) == 0)
	{
		gisUINT POINTS_LENGTH = 0;
		gisUINT BATCH_LENGTH = 0;
		int c = 0;
		string magic = ReadByteArrayToString(contentArray, c, 4); c += 4;
		int version = ReadByteArrayToInt32(contentArray.data(), c); c += 4;
		int pntsBytesLength = ReadByteArrayToInt32(contentArray, c); c += 4;
		int featureTableJsonBytesLength = ReadByteArrayToInt32(contentArray, c); c += 4;
		int featureTableBinBytesLength = ReadByteArrayToInt32(contentArray, c); c += 4;
		int batchTableJsonBytesLength = ReadByteArrayToInt32(contentArray, c); c += 4;
		int batchTableBinaBytesLength = ReadByteArrayToInt32(contentArray, c); c += 4;

		CGByteArray batchTableJsonByteArray;
		c += featureTableJsonBytesLength + featureTableBinBytesLength;

		ReadByteArrayToByteArray(contentArray, c, batchTableJsonBytesLength, batchTableJsonByteArray);
		c += batchTableJsonBytesLength;

		rapidjson::Document doc = rapidjson::Document();
		if (!doc.Parse(batchTableJsonByteArray.data(), batchTableJsonByteArray.size()).HasParseError())
		{
			if (doc.IsObject())
			{
				if (doc.HasMember("OID"))
				{
					hassId = true;
				}
			}
		}
	}
	return hassId;
}


gisLONG MapGIS::Tile::Common::API_ContentToGltf(const CGByteArray& contentArray, const CGByteArray& tidArray, CGByteArray& gltfByteArray, MapGIS::Tile::GeoCompressType  geoCompressType, bool useInstance)
{
	string magic = ReadByteArrayToString(contentArray, 0, 4);
	if (StrNICmp("gltf", magic.c_str(), 4) == 0)
	{
		bool isSkip = true;
		if (geoCompressType == MapGIS::Tile::GeoCompressType::Draco)
		{
			if (strstr(contentArray.data(), "KHR_draco_mesh_compression") == NULL)
				isSkip = false;
		}
		else
		{
			if (strstr(contentArray.data(), "EXT_meshopt_compression") == NULL)
				isSkip = false;
		}
		if (!useInstance)
		{
			if (strstr(contentArray.data(), "EXT_mesh_gpu_instancing"))
				isSkip = false;
		}

		if (isSkip)
		{
			gltfByteArray = contentArray;
			return 1;
		}
	}
	CGByteArray attArray;
	vector<MapGIS::Tile::ContentBase*> contents;
	if (API_ByteArrayToContent(contentArray, tidArray, attArray, &contents) <= 0)
		return 0;
	WriteIdType idType = WriteIdType::batchID;
	bool hasOid = false;

	if (StrNICmp("pnts", magic.c_str(), 4) == 0)
	{
		hasOid = PntsHasOID(contentArray);
	}
	else if (StrNICmp("gltf", magic.c_str(), 4) == 0)
	{
		if (strstr(contentArray.data(), "_OID"))
			hasOid = true;
	}
	Ci_ModelGltf gltf;
	gisLONG rtn = gltf.From(&contents, geoCompressType, (hasOid ? WriteIdType::oid : WriteIdType::batchID), useInstance);
	if (rtn > 0)
		rtn = ((Ci_TileModel*)&gltf)->To(gltfByteArray);
	for (vector<MapGIS::Tile::ContentBase*>::iterator itr = contents.begin(); itr != contents.end(); itr++)
	{
		if ((*itr) != NULL)
			delete (*itr);
	}
	contents.clear();
	return rtn;
}


gisLONG MapGIS::Tile::Common::API_ContentToGltf(const CGByteArray& contentArray, const CGByteArray& tidArray, vector<CGByteArray> & gltfByteArray, MapGIS::Tile::GeoCompressType  geoCompressType, bool useInstance)
{
	string magic = ReadByteArrayToString(contentArray, 0, 4);

	if (StrNICmp("gltf", magic.c_str(), 4) == 0)
	{
		bool isSkip = true;
		if (geoCompressType == MapGIS::Tile::GeoCompressType::Draco)
		{
			if (strstr(contentArray.data(), "KHR_draco_mesh_compression") == NULL)
				isSkip = false;
		}
		else
		{
			if (strstr(contentArray.data(), "EXT_meshopt_compression") == NULL)
				isSkip = false;
		}
		if (!useInstance)
		{
			if (strstr(contentArray.data(), "EXT_mesh_gpu_instancing"))
				isSkip = false;
		}

		if (isSkip)
		{
			gltfByteArray.push_back(contentArray);
			return 1;
		}
	}
	CGByteArray attArray;
	vector<MapGIS::Tile::ContentBase*> contents;
	if (API_ByteArrayToContent(contentArray, tidArray, attArray, &contents) <= 0)
		return 0;
	WriteIdType idType = WriteIdType::batchID;
	bool hasOid = false;

	if (StrNICmp("pnts", magic.c_str(), 4) == 0)
	{
		hasOid = PntsHasOID(contentArray);
	}
	else if (StrNICmp("gltf", magic.c_str(), 4) == 0)
	{
		if (strstr(contentArray.data(), "_OID"))
			hasOid = true;
	}

	gisLONG rtn = 1;
	for (vector<MapGIS::Tile::ContentBase*>::iterator itr = contents.begin(); itr != contents.end(); itr++)
	{
		if (rtn > 0)
		{
			vector<MapGIS::Tile::ContentBase*> tempContents;
			tempContents.push_back(*itr);
			Ci_ModelGltf gltf;
			rtn = gltf.From(&tempContents, geoCompressType, (hasOid ? WriteIdType::oid : WriteIdType::batchID), useInstance);
			CGByteArray tempGltfArray;
			if (rtn > 0)
				rtn = ((Ci_TileModel*)&gltf)->To(tempGltfArray);

			gltfByteArray.push_back(tempGltfArray);
		}
	}
	for (vector<MapGIS::Tile::ContentBase*>::iterator itr = contents.begin(); itr != contents.end(); itr++)
	{
		if ((*itr) != NULL)
			delete (*itr);
	}
	contents.clear();
	return 1;
}



gisLONG MapGIS::Tile::Common::API_ContentToGltf(const CGByteArray& contentArray, CGByteArray& gltfByteArray, MapGIS::Tile::GeoCompressType  geoCompressType, bool useInstance)
{
	CGByteArray tidArray;
	return API_ContentToGltf(contentArray, tidArray, gltfByteArray, geoCompressType, useInstance);
}

gisLONG MapGIS::Tile::Common::API_ContentToGltf(const CGByteArray& contentArray, vector<CGByteArray> & gltfByteArray, GeoCompressType  geoCompressType, bool useInstance)
{
	CGByteArray tidArray;
	return API_ContentToGltf(contentArray, tidArray, gltfByteArray, geoCompressType, useInstance);
}

gisLONG MapGIS::Tile::Common::API_M3DToGltf(const CGString& m3dPath, CGByteArray& gltfByteArray, GeoCompressType  geoCompressType, bool useInstance)
{
	gisLONG rtn = 0;
	CGString fileName = "";
	MapGIS::Tile::G3DCacheStorage* pStorage = GetStorageAndFileName(m3dPath, fileName);
	if (pStorage != NULL)
	{
		CGByteArray byteArray;
		pStorage->GetContent(fileName, byteArray);
		rtn = API_M3DToGltf(byteArray, gltfByteArray, geoCompressType, useInstance);
		delete pStorage;
	}
	return rtn;
}

gisLONG MapGIS::Tile::Common::API_M3DToGltf(const CGByteArray& m3d, CGByteArray& gltfByteArray, GeoCompressType  geoCompressType, bool useInstance)
{
	CGByteArray contentArray;
	CGByteArray tidArray;
	CGByteArray attArray;
	ContentType contentType;
	gisLONG rtn = API_ReadM3DByteArray(m3d, contentArray, tidArray, attArray, contentType);
	if (rtn < 0)
		return 0;
	return API_ContentToGltf(contentArray, tidArray, gltfByteArray, geoCompressType, useInstance);
}
gisLONG MapGIS::Tile::Common::API_M3DToGltf(const CGString& m3dPath, vector<CGByteArray>& gltfByteArray, GeoCompressType  geoCompressType, bool useInstance)
{
	gisLONG rtn = 0;
	CGString fileName = "";
	MapGIS::Tile::G3DCacheStorage* pStorage = GetStorageAndFileName(m3dPath, fileName);
	if (pStorage != NULL)
	{
		CGByteArray byteArray;
		pStorage->GetContent(fileName, byteArray);
		rtn = API_M3DToGltf(byteArray, gltfByteArray, geoCompressType, useInstance);
		delete pStorage;
	}
	return rtn;
}


gisLONG MapGIS::Tile::Common::API_M3DToGltf(const CGByteArray& m3d, vector<CGByteArray>& gltfByteArray, GeoCompressType  geoCompressType, bool useInstance)
{
	CGByteArray contentArray;
	CGByteArray tidArray;
	CGByteArray attArray;
	ContentType contentType;
	gisLONG rtn = API_ReadM3DByteArray(m3d, contentArray, tidArray, attArray, contentType);
	if (rtn < 0)
		return 0;
	return API_ContentToGltf(contentArray, tidArray, gltfByteArray, geoCompressType, useInstance);
}

gisLONG MapGIS::Tile::Common::API_GetAttribute(const CGByteArray& attArray, Attribute& att)
{
	string magic = ReadByteArrayToString(attArray, 0, 4);
	if (StrNICmp("att", magic.c_str(), 3) == 0)
	{
		Ci_ModelAttributeAttFile  attributeSerialization;
		return attributeSerialization.From(attArray, att);
	}
	else
	{
		Ci_ModelAttributeJsonFile attributeMode;
		LayerAttribute layerAttribute;
		if (attributeMode.From(attArray) > 0 && attributeMode.To(layerAttribute) > 0)
		{
			att.layers.emplace_back(layerAttribute);
			return 1;
		}
	}
	return 0;
}



gisLONG MapGIS::Tile::Common::API_GetAttributeBySqlite(const CGString& sqlitePath, Attribute& att)
{
	gisLONG rtn = 0;
	CGString fileName = "";
	MapGIS::Tile::G3DCacheStorage* pStorage = GetStorageAndFileName(sqlitePath, fileName);
	if (pStorage != NULL)
	{
		CGByteArray byteArray;
		pStorage->GetContent(fileName, byteArray);
		rtn = API_GetAttributeBySqlite(byteArray, att);
		delete pStorage;
	}
	return rtn;
}

gisLONG MapGIS::Tile::Common::API_GetAttributeBySqlite(const CGByteArray& byteArray, Attribute& att)
{
	return Ci_3DModelAttributeSqlite::GetAttributeByMemory(byteArray.data(), byteArray.size(), att);
}

gisLONG MapGIS::Tile::Common::API_GetTid(const CGByteArray& tidArray, vector<vector<gisINT64>>& tidVer)
{
	Ci_ModelTidFile tid;
	return tid.From(tidArray, tidVer);
}

gisLONG MapGIS::Tile::Common::API_GetStructureTree(const CGString& path, StructureTreeNode& tree)
{
	gisLONG rtn = 0;
	CGString fileName = "";
	MapGIS::Tile::G3DCacheStorage* pStorage = GetStorageAndFileName(path, fileName);
	if (pStorage != NULL)
	{
		rtn = Ci_StructureTreeManager::Read(pStorage, fileName, tree);
		delete pStorage;
	}
	return rtn;
}
