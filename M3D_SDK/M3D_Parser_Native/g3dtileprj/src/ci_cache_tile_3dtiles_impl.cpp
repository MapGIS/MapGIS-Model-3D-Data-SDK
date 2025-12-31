#include "stdafx.h"
#include "cgstring.h"
#include "ci_cache_tile_3dtiles_impl.h"
#include "ci_3dmodel.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/rapidjson.h"
#include "ci_assist.h"
#ifdef _USE_MAPGIS_SDK_
#include "ylog.h"
#endif
using namespace  _3DTiles;
using namespace MapGIS::Tile;


static CGString GetDataRelativePath(G3DCacheStorage* pCacheStorage, CGString relativePath)
{
	relativePath.Replace('\\', '/');
	while (relativePath.StartsWith("./"))
		relativePath = relativePath.Right(relativePath.GetLength() - 2);
	while (relativePath.StartsWith("/"))
		relativePath = relativePath.Right(relativePath.GetLength() - 1);
	int index = relativePath.ReverseFind('/');
	if (index <= 0)
		relativePath = "";
	else
		relativePath = relativePath.Left(index);
	CGString rootRelative = pCacheStorage->GetRootRelative();
	return MakePathByRelativePath(rootRelative, relativePath);
}

static gisLONG i_ClearId(G3DModel* pModel)
{
	if (pModel == NULL)
		return 0;
	if (pModel->GetGeometryType() == GeometryType::Point)
	{
		PointsModel* pPointModel = dynamic_cast<PointsModel*>(pModel);
		if (pPointModel != NULL)
		{
			for (vector<PointFeature>::iterator itr = pPointModel->features.begin(); itr != pPointModel->features.end(); itr++)
			{
				itr->id = -1;
			}
		}
	}
	else if (pModel->GetGeometryType() == GeometryType::Line)
	{
		LinesModel* pLineModel = dynamic_cast<LinesModel*>(pModel);
		if (pLineModel != NULL)
		{
			for (vector<LineFeature>::iterator itr = pLineModel->features.begin(); itr != pLineModel->features.end(); itr++)
			{
				itr->id = -1;
			}
		}
	}
	else if (pModel->GetGeometryType() == GeometryType::Surface)
	{
		SurfacesModel* pSurfacesModel = dynamic_cast<SurfacesModel*>(pModel);
		if (pSurfacesModel != NULL)
		{
			for (vector<SurfaceFeature>::iterator itr = pSurfacesModel->features.begin(); itr != pSurfacesModel->features.end(); itr++)
			{
				itr->id = -1;
				itr->ids.clear();
			}
		}
	}
	else if (pModel->GetGeometryType() == GeometryType::Entity)
	{
		EntitiesModel* pEntitiesModel = dynamic_cast<EntitiesModel*>(pModel);
		if (pEntitiesModel != NULL)
		{
			for (vector<EntityFeature>::iterator itr = pEntitiesModel->features.begin(); itr != pEntitiesModel->features.end(); itr++)
			{
				itr->id = -1;
				itr->ids.clear();
			}
		}
	}
	return 1;
}

static gisLONG i_ClearId(vector<ModelInstance>* pInstances)
{
	if (pInstances == NULL)
		return 0;
	for (vector<ModelInstance>::iterator itr = pInstances->begin(); itr != pInstances->end(); itr++)
	{
		itr->hasId = false;
		itr->id = 0;
	}
	return 1;
}


static gisLONG i_WriteGaussian3DModel(G3DCacheStorage* pCacheStorage, CGString relativePath, CGString contentName, GaussianContent* pContent, WriteGaussianContentParam& writeParam)
{
	if (pCacheStorage == NULL || pContent == NULL)
		return 0;
	Ci_ModelGltf gltf;
	gltf.FromGaussian(pContent, writeParam.GetGaussianExtMode());
	CGString contentPath = MakePathByRelativePath(relativePath, contentName + ".glb");
	pCacheStorage->SetContent(contentPath, *gltf.Get());
	return 1;
}

static gisLONG i_ReadGaussian3DModel(G3DCacheStorage* pCacheStorage, CGString relativePath, TileContentInfo& info,  GaussianContent* pContent)
{
	if (pCacheStorage == NULL || pContent == NULL || info.contentType != ContentType::GLB)
		return 0;
	CGString contentPath = MakePathByRelativePath(relativePath, info.contentURI);
	CGByteArray outInfo;
	gisLONG hasContent = pCacheStorage->GetContent(contentPath, outInfo);
	if (hasContent <= 0)
		return hasContent;
	Ci_ModelGltf gltf;
	((Ci_TileModel*)&gltf)->From(outInfo);
	return gltf.ToGaussian(pContent);
}

static gisLONG i_WriteBatched3DModel(G3DCacheStorage* pCacheStorage, CGString relativePath, CGString contentName, GeometryContent* pContent, WriteGeometryContentParam& writeParam)
{
	if (pContent == NULL)
		return 0;
	MapGIS::Tile::Attribute* pAttribute = pContent->GetAttribute();
	MapGIS::Tile::G3DModel* pModel = pContent->Get3DModel();
	if(pModel == NULL)
		return 0;
	GeometryType  geoType = pContent->GetGeometryType();
	if (geoType != GeometryType::Line &&  geoType != GeometryType::Surface&&  geoType != GeometryType::Entity)
		return 0;

	Ci_ModelB3dm b3dm;
	if (pAttribute != NULL && pAttribute->layers.size() == 1)
		b3dm.From(pModel, &pAttribute->layers[0], writeParam.GetGeoCompressType(), NULL);
	else
	{
		b3dm.From(pModel, NULL, writeParam.GetGeoCompressType(), NULL);
		if (pAttribute != NULL && pAttribute->layers.size() > 1)
		{
#ifdef _USE_MAPGIS_SDK_
			yLogError("属性输出错误，不支持输出多个图层的属性！");
#endif
		}
	}
	CGString contentPath = MakePathByRelativePath(relativePath, contentName + ".b3dm");
	pCacheStorage->SetContent(contentPath, *b3dm.Get());
	return 1;
}

static gisLONG i_ReadBatched3DModel(G3DCacheStorage* pCacheStorage, CGString relativePath, TileContentInfo& info, GeometryContent* pContent)
{
	if (pCacheStorage == NULL || info.contentType != ContentType::B3DM)
		return 0;
	if (pContent == NULL)
		return 0;
	MapGIS::Tile::G3DModel* pModel = pContent->Get3DModel();
	MapGIS::Tile::Attribute* pAttribute = pContent->GetAttribute();
	if (pModel !=NULL && pContent->GetGeometryType() != info.geometryType)
		return 0;

	CGString contentPath = MakePathByRelativePath(relativePath, info.contentURI);
	CGByteArray outInfo;
	gisLONG hasContent = pCacheStorage->GetContent(contentPath, outInfo);
	if (hasContent <= 0)
		return hasContent;

	Ci_ModelB3dm b3dm;
	((Ci_TileModel*)&b3dm)->From(outInfo);

	CGString rootRelative =    GetDataRelativePath(pCacheStorage, contentPath);
	G3DCacheStorage*  pStorage = G3DCacheStorage::CreateInstance(pCacheStorage->GetRootUrl(), rootRelative);
	pStorage->Open();
	b3dm.SetCacheStorage(pStorage);
	LayerAttribute records;
	D_3DOT rtcCenter;

	b3dm.To(pModel, records, rtcCenter);

	if (pStorage != NULL)
	{
		delete pStorage;
		pStorage = NULL;
	}

	if (pAttribute != NULL)
	{
		if (records.records.size() > 0)
		{
			pAttribute->layers.emplace_back(records);
		}
	}

	if (pAttribute == NULL || pAttribute->layers.size() <= 0)
		i_ClearId(pModel);
	else
	{
		bool existAtt = false;
		for (vector<LayerAttribute>::iterator layerItr = pAttribute->layers.begin(); layerItr != pAttribute->layers.end(); layerItr++)
		{
			if (layerItr->records.size() > 0)
			{
				existAtt = true;
				break;
			}
		}
		if(!existAtt)
			i_ClearId(pModel);
	}

	return 1;
}

static gisLONG i_WriteInstanced3DModel(G3DCacheStorage* pCacheStorage, CGString relativePath, CGString contentName, GeometryContent* pContent, WriteGeometryContentParam& writeParam)
{
	if (pContent == NULL || pContent->GetModelInstance() == NULL || pContent->GetModelInstance()->size() <= 0)
		return 0;
	MapGIS::Tile::G3DModel* pModel = pContent->Get3DModel();
	MapGIS::Tile::Attribute* pAttribute = pContent->GetAttribute();
	vector<MapGIS::Tile::ModelInstance>*pInstances =  pContent->GetModelInstance();
	if (pModel == NULL || pInstances == NULL)
		return 0;

	Ci_ModelI3dm i3dm;
	if (pAttribute != NULL && pAttribute->layers.size() == 1)
	{
		i3dm.From(pModel, *pInstances, &pAttribute->layers[0], writeParam.GetGeoCompressType(), NULL);
	}
	else
	{
#ifdef _USE_MAPGIS_SDK_
		yLogError("属性输出错误，不支持输出多个图层的属性！");
#endif
		i3dm.From(pModel, *pInstances, NULL, writeParam.GetGeoCompressType(), NULL);
	}
	CGString contentPath = MakePathByRelativePath(relativePath, contentName + ".i3dm");
	pCacheStorage->SetContent(contentPath, *i3dm.Get());
	return 1;
}

static gisLONG i_ReadInstanced3DModel(G3DCacheStorage* pCacheStorage, CGString relativePath, TileContentInfo& info, GeometryContent* pContent)
{
	if (pContent == NULL || info.contentType != ContentType::I3DM)
		return 0;
	vector<MapGIS::Tile::ModelInstance> temp;
	MapGIS::Tile::G3DModel* pModel = pContent->Get3DModel();
	MapGIS::Tile::Attribute* pAttribute = pContent->GetAttribute();
	vector<MapGIS::Tile::ModelInstance>*pInstances = pContent->GetModelInstance();
	if (pModel == NULL && pAttribute == NULL && pInstances == NULL)
		return 0;
	if(pModel!=NULL &&pModel->GetGeometryType() != info.geometryType)
		return 0;
	CGString contentPath = MakePathByRelativePath(relativePath, info.contentURI);
	CGByteArray outInfo;
	gisLONG hasContent =  pCacheStorage->GetContent(contentPath, outInfo);
	if (hasContent <= 0)
		return hasContent;

	if (pInstances == NULL)
		pInstances = &temp;

	Ci_ModelI3dm i3dm;
	((Ci_TileModel*)&i3dm)->From(outInfo);
	CGString rootRelative = GetDataRelativePath(pCacheStorage, contentPath);
	G3DCacheStorage*  pStorage = G3DCacheStorage::CreateInstance(pCacheStorage->GetRootUrl(), rootRelative);
	pStorage->Open();
	i3dm.SetCacheStorage(pStorage);
	LayerAttribute records;
	D_3DOT rtcCenter;
	i3dm.To(pModel, *pInstances, records, rtcCenter);
	if (pStorage != NULL)
	{
		delete pStorage;
		pStorage = NULL;
	}

	if (pAttribute != NULL)
	{
		if (records.records.size() > 0)
		{
			pAttribute->layers.emplace_back(records);
		}
		
	}

	if (pAttribute == NULL || pAttribute->layers.size() <= 0)
		i_ClearId(pInstances);
	else
	{
		bool existAtt = false;
		for (vector<LayerAttribute>::iterator layerItr = pAttribute->layers.begin(); layerItr != pAttribute->layers.end(); layerItr++)
		{
			if (layerItr->records.size() > 0)
			{
				existAtt = true;
				break;
			}
		}
		if (!existAtt)
			i_ClearId(pInstances);
	}
	return 1;
}

static gisLONG i_WritePoints(G3DCacheStorage* pCacheStorage, CGString relativePath, CGString contentName, GeometryContent* pContent, WriteGeometryContentParam& writeParam)
{
	if (pContent == NULL || pContent->Get3DModel() == NULL || pContent->GetGeometryType() != MapGIS::Tile::GeometryType::Point)
		return 0;

	MapGIS::Tile::PointsModel* pModel = dynamic_cast<MapGIS::Tile::PointsModel*>(pContent->Get3DModel());

	MapGIS::Tile::Attribute* pAttribute = pContent->GetAttribute();
	if (pModel == NULL)
		return 0;
	Ci_ModelPnts pnts;
	GeoCompressType geoCompressType =  writeParam.GetGeoCompressType();
	if (geoCompressType != GeoCompressType::Draco)
		geoCompressType = GeoCompressType::None;
	if (pAttribute != NULL &&  pAttribute->layers.size() == 1)
		pnts.From(*pModel, &pAttribute->layers[0], geoCompressType == GeoCompressType::Draco, NULL);
	else
	{
#ifdef _USE_MAPGIS_SDK_
		yLogError("属性输出错误，不支持输出多个图层的属性！");
#endif
		pnts.From(*pModel, NULL, geoCompressType == GeoCompressType::Draco, NULL);
	}
	CGString contentPath = MakePathByRelativePath(relativePath, contentName + ".pnts");
	pCacheStorage->SetContent(contentPath, *pnts.Get());
	return 1;
}


static gisLONG i_ReadPoints(G3DCacheStorage* pCacheStorage, CGString relativePath, TileContentInfo info, GeometryContent* pContent)
{
	if (pCacheStorage == NULL || info.geometryType != GeometryType::Point || info.contentType != ContentType::PNTS)
		return 0;
	MapGIS::Tile::G3DModel* pModel = pContent->Get3DModel();
	MapGIS::Tile::Attribute* pAttribute = pContent->GetAttribute();
	if (pModel != NULL && pModel->GetGeometryType() != info.geometryType)
		return 0;

	CGString contentPath = MakePathByRelativePath(relativePath, info.contentURI);
	CGByteArray outInfo;
	gisLONG hasContent = pCacheStorage->GetContent(contentPath, outInfo);
	if (hasContent <= 0)
		return hasContent;

	Ci_ModelPnts pnts;
	((Ci_TileModel*)&pnts)->From(outInfo);

	CGString rootRelative = GetDataRelativePath(pCacheStorage, contentPath);
	G3DCacheStorage*  pStorage = G3DCacheStorage::CreateInstance(pCacheStorage->GetRootUrl(), rootRelative);
	pStorage->Open();
	pnts.SetCacheStorage(pStorage);
	LayerAttribute records;
	D_3DOT rtcCenter;
	if (dynamic_cast<PointsModel*>(pModel) != NULL)
		pnts.To(*dynamic_cast<PointsModel*>(pModel), records, rtcCenter);
	else
	{
		PointsModel model;
		pnts.To(model, records, rtcCenter);
	}
	if (pStorage != NULL)
	{
		delete pStorage;
		pStorage = NULL;
	}
	if (pAttribute != NULL)
	{
		if (records.records.size() > 0)
		{
			pAttribute->layers.emplace_back(records);
		}
	}

	if (pAttribute == NULL || pAttribute->layers.size() <= 0)
		i_ClearId(pModel);
	else
	{
		bool existAtt = false;
		for (vector<LayerAttribute>::iterator layerItr = pAttribute->layers.begin(); layerItr != pAttribute->layers.end(); layerItr++)
		{
			if (layerItr->records.size() > 0)
			{
				existAtt = true;
				break;
			}
		}
		if (!existAtt)
			i_ClearId(pModel);
	}
	return 1;
}

static gisLONG i_WriteComposite(G3DCacheStorage* pCacheStorage, CGString relativePath, CGString contentName, vector<ContentBase*>* pContents, WriteGeometryContentParam& writeParam)
{
	if (pContents == NULL || pContents->size() <= 0)
		return 0;

	Ci_ModelCmpt cmpt;
	cmpt.From(pContents, writeParam.GetGeoCompressType());
	CGString contentPath = MakePathByRelativePath(relativePath, contentName + ".cmpt");
	pCacheStorage->SetContent(contentPath, *cmpt.Get());
	return 1;
}

static gisLONG i_ReadComposite(G3DCacheStorage* pCacheStorage, CGString relativePath, TileContentInfo info, vector<ContentBase*>* pContents)
{
	if (pCacheStorage == NULL || pContents == NULL || info.contentType != ContentType::CMPT)
		return 0;

	CGString contentPath = MakePathByRelativePath(relativePath, info.contentURI);
	CGByteArray outInfo;
	gisLONG hasContent = pCacheStorage->GetContent(contentPath, outInfo);
	if (hasContent <= 0)
		return hasContent;

	Ci_ModelCmpt cmpt;
	((Ci_TileModel*)&cmpt)->From(outInfo);

	CGString rootRelative = GetDataRelativePath(pCacheStorage, contentPath);
	G3DCacheStorage*  pStorage = G3DCacheStorage::CreateInstance(pCacheStorage->GetRootUrl(), rootRelative);
	pStorage->Open();
	cmpt.SetCacheStorage(pStorage);

	LayerAttribute records;
	D_3DOT rtcCenter;
	cmpt.To(pContents);
	if (pStorage != NULL)
	{
		delete pStorage;
		pStorage = NULL;
	}

	for (vector<ContentBase*>::iterator itr = pContents->begin(); itr != pContents->end(); itr++)
	{
		if (*itr == NULL)
			continue;
		GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
		if (pGeoContent == NULL)
			continue;
		Attribute* pAttribute = pGeoContent->GetAttribute();
		G3DModel* p3dModel = pGeoContent->Get3DModel();
		vector<ModelInstance>* pInstances = pGeoContent->GetModelInstance();
		if (p3dModel == NULL)
			continue;

		if (pAttribute == NULL || pAttribute->layers.size() <= 0)
		{
			if (pInstances->size() > 0)
				i_ClearId(pInstances);
			else
				i_ClearId(p3dModel);
		}
		else
		{
			bool existAtt = false;
			for (vector<LayerAttribute>::iterator layerItr = pAttribute->layers.begin(); layerItr != pAttribute->layers.end(); layerItr++)
			{
				if (layerItr->records.size() > 0)
				{
					existAtt = true;
					break;
				}
			}
			if (!existAtt)
			{
				if (pInstances->size() > 0)
					i_ClearId(pInstances);
				else
					i_ClearId(p3dModel);
			}
		}
	}
	return 1;
}

Ci_G3D3DTilesCacheTileImpl::Ci_G3D3DTilesCacheTileImpl(G3DCacheStorage* pCacheStorage)
{
	m_pTile = NULL;
	m_pTileset = NULL;
	m_jsonRelativePath = "";
	m_relativePath = "";
	m_subtreeLevel = 0;
	m_levelsLimit = 4;
	m_pCacheStorage = pCacheStorage;
	m_contentType = ContentType::None;
}

Ci_G3D3DTilesCacheTileImpl::~Ci_G3D3DTilesCacheTileImpl()
{
	Close();
}

void Ci_G3D3DTilesCacheTileImpl::SetName(CGString tileName)
{
	if(i_IsValid())
		m_tileName = tileName;
};

CGString Ci_G3D3DTilesCacheTileImpl::GetName()
{
	if (!i_IsValid())
		return "";

	if(!m_tileName.IsEmpty())
		return m_tileName;
	CGString tileName = this->m_relativePath;
	tileName.Replace('\\', '/');
	string::size_type iPos = tileName.ReverseFind('/');
	if (iPos >= 0)
		tileName = tileName.Right(tileName.GetLength() - iPos - 1);
	iPos = tileName.ReverseFind('.');
	if (iPos >= 0)
		tileName = tileName.Left(iPos);

	return tileName;
};

void Ci_G3D3DTilesCacheTileImpl::SetBounding(BoundingVolume& bounding)
{
	if (i_IsValid())
	{
		Ci_Tile*   pTile = i_GetTile();
		if (pTile != NULL)
		{
			pTile->m_boundVolume = bounding;
		}
	}
}

BoundingVolume Ci_G3D3DTilesCacheTileImpl::GetBounding()
{
	if (i_IsValid())
	{
		Ci_Tile*   pTile = i_GetTile();
		if (pTile != NULL)
		{
			return pTile->m_boundVolume;
		}
	}
	return BoundingVolume();
}

void Ci_G3D3DTilesCacheTileImpl::SetGeometricError(float geometricError)
{
	if (i_IsValid())
	{
		if (m_pTileset != NULL)
		{
			m_pTileset->m_geometricError = geometricError;
			m_pTileset->m_root.m_geometricError = geometricError;
		}
		else if (m_pTile != NULL)
		{
			m_pTile->m_geometricError = geometricError;
		}
	}
}

float Ci_G3D3DTilesCacheTileImpl::GetGeometricError()
{
	if (i_IsValid())
	{
		Ci_Tile*   pTile = i_GetTile();
		if (pTile != NULL)
		{
			return pTile->m_geometricError;
		}
	}
	return 0;
}

void Ci_G3D3DTilesCacheTileImpl::SetRefine(RefineType Refine)
{
	if (i_IsValid())
	{
		Ci_Tile*   pTile = i_GetTile();
		if (pTile != NULL)
		{
			pTile->m_refine = Refine;
		}
	}
}

RefineType Ci_G3D3DTilesCacheTileImpl::GetRefine()
{
	if (i_IsValid())
	{
		Ci_Tile*   pTile = i_GetTile();
		if (pTile != NULL)
		{
			return pTile->m_refine;
		}
	}
	return RefineType::None;
}

void Ci_G3D3DTilesCacheTileImpl::SetMatrix(Matrix4D mMatrix)
{
	if (i_IsValid())
	{
		Ci_Tile*   pTile = i_GetTile();
		if (pTile != NULL)
		{
			pTile->m_matrix = mMatrix;
		}
	}
}

Matrix4D Ci_G3D3DTilesCacheTileImpl::GetMatrix()
{
	if (i_IsValid())
	{
		Ci_Tile*   pTile = i_GetTile();
		if (pTile != NULL)
		{
			return pTile->m_matrix;
		}
	}
	return Matrix4D();
}

TileContentInfo Ci_G3D3DTilesCacheTileImpl::GetContentInfo()
{
	TileContentInfo rtn;
	if (!i_IsValid())
		return rtn;
	Ci_Tile*   pTile = i_GetTile();
	CGString uri = pTile->m_content.m_innerData.m_uri;
	if (uri.GetLength() > 0)
	{
		CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, uri);

		if (m_contentType == ContentType::None)
		{
			CGByteArray  buffer;
			m_pCacheStorage->GetContent(relativePath, buffer);
			if (buffer.size() > 4)
			{
				string magic = ReadByteArrayToString(buffer, 0, 4);
				if (StrNICmp(magic.c_str(), "cmpt", 4) == 0)
					m_contentType = ContentType::CMPT;
				else if (StrNICmp(magic.c_str(), "b3dm", 4) == 0)
					m_contentType = ContentType::B3DM;
				else if (StrNICmp(magic.c_str(), "i3dm", 4) == 0)
					m_contentType = ContentType::I3DM;
				else if (StrNICmp(magic.c_str(), "pnts", 4) == 0)
					m_contentType = ContentType::PNTS;
			}
		}

		rtn.contentType = m_contentType;
		if (m_contentType != ContentType::None)
			rtn.contentURI = pTile->m_content.m_innerData.m_uri;

		if (rtn.contentType == ContentType::PNTS)
		{
			rtn.dataType = DataType::PointCloud;
			rtn.geometryType = GeometryType::Point;
		}
		else
		{
			rtn.dataType = DataType::Model;
			rtn.geometryType = GeometryType::Surface;
		}
		if (rtn.contentType != ContentType::None)
		{
			CGString name = "";
			if (uri.ReverseFind(".") > 0)
			{
				name = uri.Left(uri.ReverseFind(".")) + ".att";
			}
			else
				name = uri + ".att";
			CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, name);
			if (m_pCacheStorage != NULL &&m_pCacheStorage->IsExistContent(relativePath))
				rtn.attributeURI = name;
			name = "";
			if (uri.ReverseFind(".") > 0)
			{
				name = uri.Left(uri.ReverseFind(".")) + ".tid";
			}
			else
				name = uri + ".tid";
			relativePath = MakePathByRelativePath(m_jsonRelativePath, name);
			if (m_pCacheStorage != NULL &&m_pCacheStorage->IsExistContent(relativePath))
				rtn.tidURI = name;
		}
	}
	return rtn;
}

bool Ci_G3D3DTilesCacheTileImpl::IsRoot()
{
	return false;
}
CGString Ci_G3D3DTilesCacheTileImpl::GetRelativePath() const
{
	if (i_IsValid() && m_pTileset != NULL)
		return m_jsonRelativePath;
	else
		return "";
}
void Ci_G3D3DTilesCacheTileImpl::Close()
{
	ReleaseChildObjects();
	if (m_pTile != NULL)
		delete m_pTile;
	if (m_pTileset != NULL)
		delete m_pTileset;
	m_pTile = NULL;
	m_pTileset = NULL;
	m_pCacheStorage = NULL;
	m_jsonRelativePath = "";
	m_relativePath = "";
	m_subtreeLevel = 0;
	m_levelsLimit = 4;
	m_contentType = ContentType::None;
}

void Ci_G3D3DTilesCacheTileImpl::ReleaseChildObjects()
{
	i_UpdateChildInfo();
	for (map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		delete itr->second;
	}
	m_childs.clear();
}
G3DCacheStorage* Ci_G3D3DTilesCacheTileImpl::GetCacheStorage()
{
	if (i_IsValid())
		return m_pCacheStorage;
	else
		return NULL;
}

Ci_Tile*  Ci_G3D3DTilesCacheTileImpl::i_GetTile() const
{
	if (m_pTileset != NULL)
		return &m_pTileset->m_root;
	else if (m_pTile != NULL)
		return m_pTile;
	return NULL;
}
void Ci_G3D3DTilesCacheTileImpl::i_InItChildInfo(Ci_G3D3DTilesCacheTileImpl * pChild, CGString parentName, bool isSubTree, bool isSubfolder)
{
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL)
		return;
	size_t num = pTile->m_childs.size();
	CGString 	childTileRelativePath = "";
	string tileName = parentName.Convert(CGString::EncodeType::GB18030).StdString() + "_" + std::to_string(num);
	CGString 	tileRelativePath = MakePathByRelativePath(this->m_jsonRelativePath, m_relativePath);

	if (isSubTree)
	{
		if(isSubfolder)
			childTileRelativePath = CGString("./" + tileName + "/" + tileName + ".json", CGString::EncodeType::GB18030);
		else
			childTileRelativePath = CGString("./" + tileName + ".json", CGString::EncodeType::GB18030);

		CGString 	relativePath = "";
		bool isExist = true;
		while (isExist)
		{
			relativePath = childTileRelativePath;
			relativePath.Replace('\\', '/');
			while (relativePath.StartsWith("./"))
			{
				relativePath = relativePath.Right(relativePath.GetLength() - 2);
			}
			isExist = false;
			for (vector<Ci_Tile>::iterator itr = pTile->m_childs.begin(); itr != pTile->m_childs.end(); itr++)
			{
				if (itr->m_content.m_innerData.m_uriType == UriObjectType::uriObjectType_Tileset)
				{
					CGString tempTileRelativePath = itr->m_content.m_innerData.m_uri;

					tempTileRelativePath.Replace('\\', '/');
					while (tempTileRelativePath.StartsWith("./"))
					{
						tempTileRelativePath = tempTileRelativePath.Right(tempTileRelativePath.GetLength() - 2);
					}

					if (relativePath == tempTileRelativePath || m_pCacheStorage->IsExistContent(MakePathByRelativePath(tileRelativePath, childTileRelativePath)))
					{
						isExist = true;
						num++;
						tileName = parentName.Convert(CGString::EncodeType::GB18030).StdString() + "_" + std::to_string(num);
						if (isSubfolder)
							childTileRelativePath = CGString("./" + tileName + "/" + tileName + ".json", CGString::EncodeType::GB18030);
						else
							childTileRelativePath = CGString("./" + tileName + ".json", CGString::EncodeType::GB18030);
						break;
					}
				}
			}
		}
	}
	else
	{
		if (isSubfolder)
			childTileRelativePath = CGString("./" + tileName + "/" + tileName, CGString::EncodeType::GB18030);
		else
			childTileRelativePath = CGString("./" +  tileName, CGString::EncodeType::GB18030);
	}
	childTileRelativePath.Convert(CGString::EncodeType::GB18030);
	if (isSubTree)
	{
		pChild->m_pTileset = new Ci_Tileset();
		pChild->m_jsonRelativePath = MakePathByRelativePath(tileRelativePath, childTileRelativePath);
		pChild->m_relativePath = "./" + childTileRelativePath.Right(childTileRelativePath.GetLength() - childTileRelativePath.ReverseFind("/") - 1);
		Ci_Tile childTile;
		childTile.m_content.m_innerData.m_uriType = UriObjectType::uriObjectType_Tileset;
		childTile.m_content.m_innerData.m_uri = childTileRelativePath.StdString();
		pTile->m_childs.emplace_back(childTile);
	}
	else
	{
		pChild->m_jsonRelativePath = this->m_jsonRelativePath;
		pChild->m_relativePath = MakePathByRelativePath(m_relativePath, childTileRelativePath);
		pChild->m_pTile = new Ci_Tile();
		pTile->m_childs.emplace_back(Ci_Tile());
	}
}

void  Ci_G3D3DTilesCacheTileImpl::SetLevelsLimit(int levelsLimit)
{
	if (levelsLimit <= 0)
		return;
	m_levelsLimit = levelsLimit;
	for (map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		if (itr->second != NULL)
		{
			itr->second->SetLevelsLimit(levelsLimit);
		}
	}
}

gisLONG Ci_G3D3DTilesCacheTileImpl::Save()
{
	if (!i_IsValid())
		return 0;
	i_UpdateChildInfo();
	return i_Save();
}

gisLONG Ci_G3D3DTilesCacheTileImpl::i_Save()
{
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL)
		return 0;
	if (m_childs.size() > 0)
	{
		for (map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
		{
			if (itr->second != NULL)
			{
				itr->second->i_Save();
			}
		}
	}
	if (m_pCacheStorage == NULL || m_pTileset == NULL)
		return 1;
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	m_pTileset->WriteTo(doc, allocator);
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	gisLONG rtn = m_pCacheStorage->SetContent(m_jsonRelativePath, buffer.GetString(), buffer.GetLength());
	return 1;
}

gisLONG Ci_G3D3DTilesCacheTileImpl::i_UpdateChildInfo()
{
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL)
		return 0;
	if (m_childs.size() > 0)
	{
		for (map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
		{
			if (itr->second != NULL)
			{
				itr->second->i_UpdateChildInfo();

				if (pTile->m_childs.size() > itr->first)
				{
					if (itr->second->m_pTileset !=NULL)
					{
						pTile->m_childs[itr->first].m_boundVolume = itr->second->GetBounding();
						pTile->m_childs[itr->first].m_geometricError = itr->second->GetGeometricError();
						pTile->m_childs[itr->first].m_matrix = itr->second->GetMatrix();
						pTile->m_childs[itr->first].m_refine = itr->second->GetRefine();
					}
					else  if (itr->second->m_pTile != NULL)
					{
						pTile->m_childs[itr->first] = *itr->second->m_pTile;
					}
				}
			}
		}
	}

	return 1;
}

bool Ci_G3D3DTilesCacheTileImpl::i_IsValid()  const
{
	Ci_Tile*   pTile = i_GetTile();
	if (pTile != NULL && m_pCacheStorage !=NULL)
		return true;
	return false;
}

gisLONG Ci_G3D3DTilesCacheTileImpl::RemoveChild(G3DTile* pChildTile)
{
	if (!i_IsValid()  || pChildTile == NULL)
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	Ci_G3D3DTilesCacheTileImpl* p3DTilesCacheTile = dynamic_cast<Ci_G3D3DTilesCacheTileImpl*>(pChildTile);
	if (p3DTilesCacheTile == NULL)
		return 0;
	int startIndex = -1;
	for (map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		if (itr->second == p3DTilesCacheTile)
		{
			startIndex = itr->first;
			delete itr->second;
			m_childs.erase(itr);
			if (pTile->m_childs.size() > startIndex)
				pTile->m_childs.erase(pTile->m_childs.begin() + startIndex);
			break;
		}
	}
	if (startIndex >= 0 && m_childs.size()>0)
	{
		map<int, Ci_G3D3DTilesCacheTileImpl*> temp;
		for (map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
		{
			if (itr->first < startIndex)
				temp.insert(make_pair(itr->first, itr->second));
			else if (itr->first > startIndex)
				temp.insert(make_pair(itr->first - 1, itr->second));
		}
		m_childs.swap(temp);
	}
	return 0;
}

gisLONG Ci_G3D3DTilesCacheTileImpl::RemoveChild(int index)
{
	if (!i_IsValid() || index < 0)
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.find(index);
	if (itr != m_childs.end())
	{
		delete itr->second;
		m_childs.erase(itr);
		if (m_childs.size() > 0)
		{
			map<int, Ci_G3D3DTilesCacheTileImpl*> temp;
			for (map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
			{
				if (itr->first < index)
					temp.insert(make_pair(itr->first, itr->second));
				else if (itr->first > index)
					temp.insert(make_pair(itr->first - 1, itr->second));
			}
			m_childs.swap(temp);
		}
	}
	if (pTile->m_childs.size() > index)
		pTile->m_childs.erase(pTile->m_childs.begin() + index);
	return 1;
}

gisLONG Ci_G3D3DTilesCacheTileImpl::GetChildNum()
{
	if (!i_IsValid())
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	return 	pTile->m_childs.size();
}

G3DTile* Ci_G3D3DTilesCacheTileImpl::AppendChild(TileAppendMode mode)
{
	if (!i_IsValid())
		return  NULL;
	Ci_Tile*   pTile = i_GetTile();
	Ci_G3D3DTilesCacheTileImpl *rtn = i_CreateChildInstance();
	CGString tileName = this->m_relativePath;
	tileName.Replace('\\', '/');
	string::size_type iPos = tileName.ReverseFind('/');
	if (iPos >= 0)
	{
		tileName = tileName.Right(tileName.GetLength() - iPos - 1);
	}

	iPos = tileName.ReverseFind('.');
	if (iPos >= 0)
	{
		tileName = tileName.Left(iPos);
	}

	switch (mode)
	{
	case TileAppendMode::AutoMode:
	case TileAppendMode::AutoMode2:
	case TileAppendMode::AutoMode3:
	{
		if (m_subtreeLevel == m_levelsLimit)
		{
			rtn->m_subtreeLevel = 0;
			if (mode == TileAppendMode::AutoMode)
				i_InItChildInfo(rtn, tileName, true, true);
			else if (mode == TileAppendMode::AutoMode2)
				i_InItChildInfo(rtn, tileName, false, true);
			else
				i_InItChildInfo(rtn, tileName, true, false);
		}
		else
		{
			i_InItChildInfo(rtn, tileName, false, false);
			rtn->m_subtreeLevel = m_subtreeLevel + 1;
		}
	}
	break;
	case TileAppendMode::CreateSubTree:
	{
		i_InItChildInfo(rtn, tileName, true, true);
		rtn->m_subtreeLevel = 0;
	}
	break;
	case TileAppendMode::DoNotSubTree:
	{
		i_InItChildInfo(rtn, tileName, false, false);
		rtn->m_subtreeLevel = m_subtreeLevel + 1;
	}
	break;
	default:
		break;
	}
	rtn->m_levelsLimit = m_levelsLimit;
	m_childs.insert(make_pair(pTile->m_childs.size() - 1, rtn));
	return rtn;
}

G3DTile* Ci_G3D3DTilesCacheTileImpl::GetChild(int index)
{
	if (!i_IsValid())
		return NULL;
	Ci_Tile*   pTile = i_GetTile();
	if ( pTile->m_childs.size() <= index)
		return NULL;
	map<int, Ci_G3D3DTilesCacheTileImpl*>::iterator itr = m_childs.find(index);
	if (itr != m_childs.end())
	{
		return itr->second;
	}
	else
	{
		Ci_Tile & tile = pTile->m_childs[index];
		Ci_3DTileUriObject uriObj = tile.m_content.m_innerData;
		if (uriObj.m_uriType == UriObjectType::uriObjectType_Tileset)
		{
			string tilesetUri = uriObj.m_uri;
			CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, tilesetUri);
			G3DTile* pChildTile = G3DTile::Open(m_pCacheStorage, relativePath,i_GetCacheType());
			if (pChildTile != NULL)
			{
				((Ci_G3D3DTilesCacheTileImpl*)pChildTile)->m_levelsLimit = m_levelsLimit;
				((Ci_G3D3DTilesCacheTileImpl*)pChildTile)->m_subtreeLevel = m_subtreeLevel + 1;
				m_childs.insert(make_pair(index, (Ci_G3D3DTilesCacheTileImpl*)pChildTile));
				return pChildTile;
			}
			else
				return NULL;
		}
		else if (uriObj.m_uriType == UriObjectType::uriObjectType_FileModel)
		{
			Ci_G3D3DTilesCacheTileImpl *rtn = i_CreateChildInstance();
			rtn->m_jsonRelativePath = this->m_jsonRelativePath;
			string tilesetUri = uriObj.m_uri;
			CGString relativePath = tilesetUri;
			if (relativePath.ReverseFind(".") > 0)
			{
				relativePath = relativePath.Left(relativePath.ReverseFind("."));
			}
			rtn->m_relativePath = relativePath;
			rtn->m_levelsLimit = m_levelsLimit;
			rtn->m_subtreeLevel = m_subtreeLevel +1;
			rtn->m_pTile = new Ci_Tile();
			*rtn->m_pTile = tile;
			m_childs.insert(make_pair(index, rtn));
			return rtn;
		}
		else
		{
			Ci_G3D3DTilesCacheTileImpl *rtn = i_CreateChildInstance();
			rtn->m_jsonRelativePath = this->m_jsonRelativePath;

			if (this->IsRoot())
				rtn->m_relativePath = "0_" + std::to_string(index);
			else
				rtn->m_relativePath = this->m_relativePath + "_" + std::to_string(index);
			rtn->m_levelsLimit = m_levelsLimit;
			rtn->m_subtreeLevel = m_subtreeLevel + 1;
			rtn->m_pTile = new Ci_Tile();
			*rtn->m_pTile = tile;
			m_childs.insert(make_pair(index, rtn));
			return rtn;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ci_G3D3DTiles10CacheTileImpl::Ci_G3D3DTiles10CacheTileImpl(G3DCacheStorage* pCacheStorage) :Ci_G3D3DTilesCacheTileImpl(pCacheStorage)
{
}

Ci_G3D3DTiles10CacheTileImpl::~Ci_G3D3DTiles10CacheTileImpl()
{
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::WriteGaussian3DModel(CGString contentName, GaussianContent* pContent, WriteGaussianContentParam& writeParam)
{
	if (!i_IsValid() || pContent == NULL)
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();

	m_contentType = ContentType::GLB;
	pTile->m_content.m_innerData.m_uriType = UriObjectType::uriObjectType_FileModel;
	CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, m_relativePath);
	CGString contentPath = MakePathByRelativePath(m_relativePath, contentName + ".glb");
	pTile->m_content.m_innerData.m_uri = contentPath.Convert(CGString::EncodeType::GB18030).CStr();
	return i_WriteGaussian3DModel(m_pCacheStorage, relativePath, contentName, pContent, writeParam);
}
gisLONG Ci_G3D3DTiles10CacheTileImpl::ReadGaussian3DModel(MapGIS::Tile::TileContentInfo info, GaussianContent* pContent)
{
	if (!i_IsValid())
		return 0;
	return i_ReadGaussian3DModel(m_pCacheStorage, m_jsonRelativePath, info, pContent);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::WriteBatched3DModel(CGString contentName,  GeometryContent* pContent, WriteGeometryContentParam& writeParam)
{
	if (!i_IsValid() || pContent == NULL)
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL )
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();

	m_contentType = ContentType::B3DM;
	pTile->m_content.m_innerData.m_uriType = UriObjectType::uriObjectType_FileModel;
	CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, m_relativePath);
	CGString contentPath = MakePathByRelativePath(m_relativePath, contentName + ".b3dm");
	pTile->m_content.m_innerData.m_uri = contentPath.Convert(CGString::EncodeType::GB18030).CStr();
	return i_WriteBatched3DModel(m_pCacheStorage, relativePath, contentName, pContent, writeParam);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::ReadBatched3DModel(TileContentInfo info, GeometryContent* pContent)
{
	if (!i_IsValid())
		return 0;
	return i_ReadBatched3DModel(m_pCacheStorage, m_jsonRelativePath, info, pContent);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::WriteInstanced3DModel(CGString contentName, GeometryContent* pContent, WriteGeometryContentParam& writeParam)
{
	if (!i_IsValid() || pContent == NULL)
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	m_contentType = ContentType::I3DM;
	pTile->m_content.m_innerData.m_uriType = UriObjectType::uriObjectType_FileModel;
	CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, m_relativePath);
	CGString contentPath = MakePathByRelativePath(m_relativePath, contentName + ".i3dm");
	pTile->m_content.m_innerData.m_uri = contentPath.Convert(CGString::EncodeType::GB18030).CStr();
	return i_WriteInstanced3DModel(m_pCacheStorage, relativePath, contentName, pContent, writeParam);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::ReadInstanced3DModel(TileContentInfo info, GeometryContent* pContent)
{
	if (!i_IsValid())
		return 0;
	return i_ReadInstanced3DModel(m_pCacheStorage, m_jsonRelativePath, info, pContent);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::WritePoints(CGString contentName, GeometryContent* pContent, WriteGeometryContentParam& writeParam)
{
	if (!i_IsValid() || pContent == NULL )
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	m_contentType = ContentType::PNTS;
	pTile->m_content.m_innerData.m_uriType = UriObjectType::uriObjectType_FileModel;
	CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, m_relativePath);
	CGString contentPath = MakePathByRelativePath(m_relativePath, contentName + ".pnts");
	pTile->m_content.m_innerData.m_uri = contentPath.Convert(CGString::EncodeType::GB18030).CStr();
	return i_WritePoints(m_pCacheStorage, relativePath, contentName, pContent, writeParam);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::ReadPoints(TileContentInfo info, GeometryContent* pContent)
{
	if (!i_IsValid())
		return 0;
	return i_ReadPoints(m_pCacheStorage, m_jsonRelativePath, info, pContent);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::WriteComposite(CGString contentName, vector<ContentBase*>* pContents, WriteGeometryContentParam& writeParam)
{
	if (!i_IsValid() || pContents == NULL || pContents->size() <= 0)
		return 0;
	Ci_Tile*   pTile = i_GetTile();
	if (pTile == NULL)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	m_contentType = ContentType::CMPT;
	pTile->m_content.m_innerData.m_uriType = UriObjectType::uriObjectType_FileModel;
	CGString relativePath = MakePathByRelativePath(m_jsonRelativePath, m_relativePath);
	CGString contentPath = MakePathByRelativePath(m_relativePath, contentName + ".cmpt");
	pTile->m_content.m_innerData.m_uri = contentPath.Convert(CGString::EncodeType::GB18030).CStr();
	return i_WriteComposite(m_pCacheStorage, relativePath, contentName, pContents, writeParam);
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::ReadComposite(TileContentInfo info, vector<ContentBase*>* pContents)
{
	if (!i_IsValid())
		return 0;
	return i_ReadComposite(m_pCacheStorage, m_jsonRelativePath, info, pContents);
}

G3DTile* Ci_G3D3DTiles10CacheTileImpl::Open(G3DCacheStorage* pCacheStorage, CGString relativePath)
{
	if (pCacheStorage == NULL || relativePath.IsEmpty())
		return NULL;
	CGByteArray outInfo;
	pCacheStorage->GetContent(relativePath, outInfo);
	if (outInfo.size() <= 0)
		return NULL;

	rapidjson::Document doc;
	if (doc.Parse(outInfo.data(), outInfo.size()).HasParseError())
		return NULL;
	if (!doc.IsObject())
		return NULL;
	Ci_G3D3DTiles10CacheTileImpl* rtn = new Ci_G3D3DTiles10CacheTileImpl(pCacheStorage);
	rtn->m_pTileset = new Ci_Tileset();
	rtn->m_jsonRelativePath = relativePath;
	relativePath.Replace('\\', '/');
	if (relativePath.Find('/') > 0)
	{
		relativePath = relativePath.Right(relativePath.GetLength() - relativePath.ReverseFind("/") - 1);
	}
	rtn->m_relativePath = relativePath;
	rtn->m_pTileset->ReadFrom(doc);
	return rtn;
};
Ci_G3D3DTilesCacheTileImpl* Ci_G3D3DTiles10CacheTileImpl::i_CreateChildInstance()
{
	Ci_G3D3DTiles10CacheTileImpl* rtn = new Ci_G3D3DTiles10CacheTileImpl(m_pCacheStorage);
	return rtn;
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::WriteContent(CGString contentName, vector<ContentBase*>* pContents, WriteContentParamBase& param)
{
	if (pContents == NULL)
		return 0;
	if (pContents->size() > 1)
	{
		if (dynamic_cast<WriteGeometryContentParam*>(&param) != NULL) 
			return WriteComposite(contentName, pContents, *dynamic_cast<WriteGeometryContentParam*>(&param));
		else
		{
			WriteGeometryContentParam contentParam;
			return WriteComposite(contentName, pContents, contentParam);
		}

	}
	ContentBase* item = pContents->at(0);
	GaussianContent*  pGaussianContent = dynamic_cast<GaussianContent*>(item);
	GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(item);
	if (pGaussianContent != NULL) 
	{
		WriteGaussianContentParam tempParam;
		WriteGaussianContentParam *pContentParam = dynamic_cast<WriteGaussianContentParam*>(&param);
		if (pContentParam == NULL)
			pContentParam = &tempParam;
		return WriteGaussian3DModel(contentName, pGaussianContent, *pContentParam);
	}
	else if (pGeoContent != NULL) 
	{
		WriteGeometryContentParam tempParam;
		WriteGeometryContentParam *pContentParam = dynamic_cast<WriteGeometryContentParam*>(&param);
		if (pContentParam == NULL)
			pContentParam = &tempParam;
		G3DModel*  pModel = pGeoContent->Get3DModel();
		vector<ModelInstance>* pInstance = pGeoContent->GetModelInstance();
		if (pModel == NULL)
			return 0;
		if (pInstance != NULL &&pInstance->size() > 0)
			return WriteInstanced3DModel(contentName, pGeoContent, *pContentParam);
		if (pModel->GetGeometryType() == GeometryType::Point)
			return WritePoints(contentName, pGeoContent, *pContentParam);
		return WriteBatched3DModel(contentName, pGeoContent, *pContentParam);
	}
	return 0;
}

gisLONG Ci_G3D3DTiles10CacheTileImpl::ReadContent(vector<ContentBase*>* pContents)
{
	if (pContents == NULL)
		return 0;
	TileContentInfo tileContentInfo = GetContentInfo();
	gisLONG rtn = 0;
	switch (tileContentInfo.contentType)
	{
	case ContentType::B3DM:
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Surface, true, false);
		rtn = ReadBatched3DModel(tileContentInfo, pItem);
		pContents->emplace_back(pItem);
	}
	break;

	case ContentType::I3DM:
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Surface, true, true);
		rtn = ReadInstanced3DModel(tileContentInfo, pItem);
		pContents->emplace_back(pItem);
	}
	break;

	case ContentType::PNTS:
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Point, true, false);
		rtn = ReadPoints(tileContentInfo, pItem);
		pContents->emplace_back(pItem);
	}
	break;

	case ContentType::CMPT:
		rtn = ReadComposite(tileContentInfo, pContents);
		break;

	case ContentType::None:
	case ContentType::M3DGLB:
	case ContentType::M3DI3DM:
	case ContentType::M3DPNTS:
	case ContentType::M3DCMPT:
	case ContentType::M3DVoxel:
	case ContentType::GLB:
	{//现在不支持3d tiles 1.1 所以glb是高斯溅射数据

		if (tileContentInfo.dataType == DataType::GaussianSplatting)
		{
			GaussianContent* pItem = new GaussianContent();
			pItem->CreateData(true);
			rtn = ReadGaussian3DModel(tileContentInfo, pItem);
			pContents->emplace_back(pItem);
		}
	}
	break;

	default:
		break;
	}
	return rtn;
}

G3DCacheType Ci_G3D3DTiles10CacheTileImpl::i_GetCacheType()
{
	return G3DCacheType::Type3DTilesV10;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ci_G3D3DTilesCacheTilesetImpl::Ci_G3D3DTilesCacheTilesetImpl()
{
	m_pRootTile = NULL;
	m_pCacheStorage = NULL;
	m_dot = { 0,0,0 };
};
Ci_G3D3DTilesCacheTilesetImpl::~Ci_G3D3DTilesCacheTilesetImpl()
{
	Close();
};
void Ci_G3D3DTilesCacheTilesetImpl::SetTreeType(TreeType treeType)
{
	if (IsValid())
	{
		if (treeType == TreeType::Octree)
			m_pRootTile->SetLevelsLimit(4);
		else if (treeType == TreeType::QuadTree)
			m_pRootTile->SetLevelsLimit(5);
		else if (treeType == TreeType::KDTree)
			m_pRootTile->SetLevelsLimit(10);
		else if (treeType == TreeType::RTree)
			m_pRootTile->SetLevelsLimit(4);
	}
};
gisLONG Ci_G3D3DTilesCacheTilesetImpl::Save()
{
	if (IsValid() && m_pRootTile != NULL &&m_pRootTile->m_pTile != NULL)
	{
		m_pRootTile->Save();
		Matrix4D matrix;
		if (fabs(m_dot.x) > 1e-7 || fabs(m_dot.y) > 1e-7 || fabs(m_dot.z) > 1e-7)
			matrix = WGS84Ellipsoid::GetLocationPointTransform(m_dot.x / 180 * PI, m_dot.y / 180 * PI, m_dot.z);
		Matrix4D rootMatrix = m_pRootTile->GetMatrix();
		m_tileset.m_root = *m_pRootTile->m_pTile;
		m_tileset.m_root.m_matrix = matrix*rootMatrix;

		rapidjson::Document doc;
		doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

		m_tileset.WriteTo(doc, allocator);
		m_tileset.m_root.m_matrix = rootMatrix;
		CGByteArray jsonData;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		jsonData.append(buffer.GetString(), buffer.GetLength());
		gisLONG rtn = m_pCacheStorage->SetContent("tileset.json", jsonData);
		return 1;
	}
	return 1;
}
gisLONG Ci_G3D3DTilesCacheTilesetImpl::Close()
{
	if (m_pRootTile != NULL)
		delete m_pRootTile;
	m_pRootTile = NULL;
	m_pCacheStorage = NULL;
	m_dot.x = 0;
	m_dot.y = 0;
	m_dot.z = 0;
	return 1;
}
bool Ci_G3D3DTilesCacheTilesetImpl::IsValid()
{
	if (m_pCacheStorage != NULL && m_pRootTile != NULL)
		return true;
	return false;
}

gisLONG Ci_G3D3DTilesCacheTilesetImpl::Create(G3DCacheStorage *pCacheStorage, CGString tilesetName)
{
	Close();
	if (pCacheStorage == NULL || !pCacheStorage->IsValid() || tilesetName.IsEmpty())
		return 0;
	m_tilesetName = tilesetName;
	m_pRootTile = i_CreateRootTile(pCacheStorage);
	m_pRootTile->m_relativePath = "0";
	m_pRootTile->m_pTile = new Ci_Tile();
#ifdef _USE_MAPGIS_SDK_
	Ci_G3DServerDBCacheStorage* pMongoDB = dynamic_cast<Ci_G3DServerDBCacheStorage*>(pCacheStorage);
	if (pMongoDB != NULL && !pCacheStorage->IsExistContent("CacheInfo"))
	{
		pMongoDB->SetMetaData(i_GetCacheType());
	}
#endif // _USE_MAPGIS_SDK_

	m_pCacheStorage = pCacheStorage;
	return 1;
}

gisLONG Ci_G3D3DTilesCacheTilesetImpl::Open(G3DCacheStorage *pCacheStorage, CGString tilesetName)
{
	Close();
	if (pCacheStorage == NULL || !pCacheStorage->IsValid() || tilesetName.IsEmpty())
		return 0;
	CGString path = pCacheStorage->GetRootUrl();
	path.Convert(CGString::EncodeType::GB18030);
	CGByteArray value;
	pCacheStorage->GetContent(tilesetName + ".json", value);
	if (value.isEmpty())
	{
		pCacheStorage->GetContent("tileset.json", value);
	}
	if (value.isEmpty())
		return 0;
	rapidjson::Document doc;
	if (doc.Parse(value.data(), value.size()).HasParseError())
		return 0;
	if (!doc.IsObject())
		return 0;
	m_tileset.ReadFrom(doc);
	m_tilesetName = tilesetName;
	m_pRootTile = i_CreateRootTile(pCacheStorage);
	m_pRootTile->m_relativePath = "0";
	m_pRootTile->m_pTile = new Ci_Tile();
	*m_pRootTile->m_pTile = m_tileset.m_root;
	m_pCacheStorage = pCacheStorage;
	return 1;
}

////////////////////////////////////////////////////////////////////

class Tiles10RootTile : public  Ci_G3D3DTiles10CacheTileImpl
{
public:
	virtual	bool  IsRoot() { return true; };

	virtual void SetGeometricError(float geometricError)
	{
		if (m_pTiles10Impl != NULL)
		{
			Ci_Tileset* pCiTileset = m_pTiles10Impl->GetCiTileset();
			if (pCiTileset != NULL)
			{
				pCiTileset->m_geometricError = geometricError;
				pCiTileset->m_root.m_geometricError = geometricError;
			}
		}
		Ci_G3D3DTiles10CacheTileImpl::SetGeometricError(geometricError);
	}
	Tiles10RootTile(G3DCacheStorage* pCacheStorage, Ci_G3D3DTiles10CacheTilesetImpl *pTiles10Impl) :Ci_G3D3DTiles10CacheTileImpl(pCacheStorage), m_pTiles10Impl(pTiles10Impl) {}

private:
	Ci_G3D3DTiles10CacheTilesetImpl *m_pTiles10Impl;
};

Ci_G3D3DTiles10CacheTilesetImpl::Ci_G3D3DTiles10CacheTilesetImpl()
{
}

Ci_G3D3DTiles10CacheTilesetImpl::~Ci_G3D3DTiles10CacheTilesetImpl()
{
}
void Ci_G3D3DTiles10CacheTilesetImpl::ClearLayerFieldsInfo()
{
	m_tileset.m_fieldInfo.clear();
}
gisLONG Ci_G3D3DTiles10CacheTilesetImpl::SetLayerFieldsInfo(const LayersInfoBase *pLayersInfo)
{
	if (!IsValid() || pLayersInfo == NULL)
		return 0;
	const LayersInfo* pLayerFieldsInfo = dynamic_cast<const LayersInfo*>(pLayersInfo);
	if (pLayerFieldsInfo == NULL || pLayerFieldsInfo->m_layersInfo.size() != 1)
		return 0;
	m_tileset.m_fieldInfo.insert(m_tileset.m_fieldInfo.begin(), pLayerFieldsInfo->m_layersInfo[0].fieldInfos.begin(), pLayerFieldsInfo->m_layersInfo[0].fieldInfos.end());
	return 1;
}
gisLONG Ci_G3D3DTiles10CacheTilesetImpl::GetLayerFieldsInfo(LayersInfoBase *pLayersInfo)
{
	if (!IsValid() || pLayersInfo == NULL)
		return 0;
	LayersInfo* pLayerFieldsInfo = dynamic_cast<LayersInfo*>(pLayersInfo);
	if (pLayerFieldsInfo == NULL)
		return 0;
	pLayerFieldsInfo->m_layersInfo.clear();
	LayerFieldsInfo info;
	info.fieldInfos.insert(info.fieldInfos.begin(), m_tileset.m_fieldInfo.begin(), m_tileset.m_fieldInfo.end());
	pLayerFieldsInfo->m_layersInfo.emplace_back(info);
	return 1;
}

Ci_G3D3DTilesCacheTileImpl* Ci_G3D3DTiles10CacheTilesetImpl::i_CreateRootTile(G3DCacheStorage *pCacheStorage)
{
	Tiles10RootTile* rtn = new Tiles10RootTile(pCacheStorage,this);
	return rtn;
}

G3DCacheType Ci_G3D3DTiles10CacheTilesetImpl::i_GetCacheType()
{
	return G3DCacheType::Type3DTilesV10;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
