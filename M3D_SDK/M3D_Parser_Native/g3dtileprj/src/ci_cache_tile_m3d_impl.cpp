#include "stdafx.h"
#include "tiny_gltf.h"
#include "ci_cache_tile_m3d_impl.h"
#include "ci_ziptool.h"
#include "ci_dracotool.h"
#include "ci_meshopttool.h"
#include "ci_3dmodel.h"
#include "ci_3dmodel_attribute_json.h"
#include "ci_3dmodel_attribute_att.h"
#include "ci_3dmodel_tid.h"
#include "ci_3dmodel_attribute_sqlite_tool.h"
#include <mutex>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/rapidjson.h"
#include "ci_assist.h"
#ifdef _USE_MAPGIS_SDK_
#include "ylog.h"
#endif
using namespace MapGIS::Tile;
using namespace M3D;
#ifdef _USE_MAPGIS_SDK_
#ifdef MergePropertiesInThread
#include "cgthread.h"
CGThreadPool g_WriteSqliteThreadPool(1);
class CI_ImportAttributeTask : public CGThreadTask
{
public:
	CI_ImportAttributeTask()
	{
		m_pDestAttribute = NULL;
	}

	virtual ~CI_ImportAttributeTask()
	{
	}
public:
	virtual void run()
	{
		mtxWriteSqliteAttribute.lock();
		ImportAttribute(m_Attribute, *m_pDestAttribute);
		mtxWriteSqliteAttribute.unlock();
	}
	void SetAttribute(Attribute attribute, Attribute* pDestAttribute)
	{
		m_Attribute = attribute;
		m_pDestAttribute = pDestAttribute;
	}

private:
	Attribute m_Attribute;
	Attribute* m_pDestAttribute;
};
#endif
#endif
static std::mutex mtxWriteSqliteAttribute; // 定义一个互斥量

static gisLONG  ImportAttribute(const Attribute& orAtt, Attribute& desAtt)
{
	for (vector<LayerAttribute>::const_iterator itr = orAtt.layers.begin(); itr != orAtt.layers.end(); itr++)
	{
		bool isExist = false;
		for (vector<LayerAttribute>::iterator itr2 = desAtt.layers.begin(); itr2 != desAtt.layers.end(); itr2++)
		{
            if (itr->layerInfo == itr2->layerInfo)
			{
				isExist = true;
				itr2->records.insert(itr2->records.end(), itr->records.begin(), itr->records.end());
				break;
			}
		}
		if (!isExist)
		{
			desAtt.layers.emplace_back(*itr);
		}
	}
	return 1;
};

static gisLONG WriteData(G3DCacheStorage* pCacheStorage, CGString relativePath, CGString contentName, Ci_TileModel*pModel, Attribute *pAttribute, CGByteArray* pTidArray, G3DCacheType cacheType, bool attInContent, ContentType contentType, int zipCompressionLevel = 6)
{
	if (pCacheStorage == NULL || pModel == NULL || relativePath.IsEmpty() || contentName.IsEmpty())
		return 0;

	string suffix = ".glbx";
	if (cacheType == G3DCacheType::TypeM3DV21 || cacheType == G3DCacheType::TypeM3DV22)
	{
		switch (contentType)
		{
		case MapGIS::Tile::ContentType::M3DGLB:
			suffix = ".glb";
			break;
		case MapGIS::Tile::ContentType::M3DI3DM:
			suffix = ".i3dm";
			break;
		case MapGIS::Tile::ContentType::M3DPNTS:
			suffix = ".pnts";
			break;
		case MapGIS::Tile::ContentType::M3DCMPT:
			suffix = ".cmpt";
			break;
		case MapGIS::Tile::ContentType::None:
		case MapGIS::Tile::ContentType::B3DM:
		case MapGIS::Tile::ContentType::I3DM:
		case MapGIS::Tile::ContentType::PNTS:
		case MapGIS::Tile::ContentType::CMPT:
		default:
			break;
		}
	}

	CGByteArray outInfo;
	if (pAttribute != NULL && pAttribute->layers.size() > 0)
	{
		if (cacheType == G3DCacheType::TypeM3DV20)
		{
			if (pAttribute->layers.size() == 1)
			{
				if (pAttribute->layers[0].records.size() > 0)
				{
					Ci_ModelAttributeJsonFile modelAttribute;
					modelAttribute.From(pAttribute->layers[0]);
					Ci_ZipTool::Buffer2ZipBuffer(*pModel->Get(), "geometry/0" + suffix, *modelAttribute.Get(), "attribute/0.json", CGByteArray(), "", outInfo, zipCompressionLevel);
				}
				else
					Ci_ZipTool::Buffer2ZipBuffer(*pModel->Get(), "geometry/0" + suffix, CGByteArray(), "", CGByteArray(), "", outInfo, zipCompressionLevel);
			}
			else
			{
#ifdef _USE_MAPGIS_SDK_
				yLogError("属性输出错误，不支持输出多个图层的属性！");
#endif
			}
		}
		else
		{
			Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(pCacheStorage, false);
			if (pSqliteTool !=NULL)
			{
				Attribute* pSqliteAttribute = pSqliteTool->GetAttribute();

				bool isImport = false;
#ifdef _USE_MAPGIS_SDK_
#ifdef MergePropertiesInThread
				CI_ImportAttributeTask* pTask = new CI_ImportAttributeTask();
				pTask->SetAttribute(*pAttribute, pSqliteAttribute);
				g_WriteSqliteThreadPool.StartTask(pTask);
				isImport = true;
#endif
#endif
				if (!isImport)
				{
					mtxWriteSqliteAttribute.lock();
					ImportAttribute(*pAttribute, *pSqliteAttribute);
					mtxWriteSqliteAttribute.unlock();
				}
			}
			else
			{
				Ci_ModelAttributeAttFile  attributeSerialization;
				CGByteArray byteArray;
				if (attributeSerialization.To(*pAttribute, byteArray, AttributeCompressType::PropCompressTypeGZip) > 0 && byteArray.size() > 0)
				{
					if (attInContent)
					{
						if (pTidArray != NULL)
							Ci_ZipTool::Buffer2ZipBuffer(*pModel->Get(), "geometry/0" + suffix, byteArray, "attribute/0.att", *pTidArray, "geometry/0.tid", outInfo, zipCompressionLevel);
						else
							Ci_ZipTool::Buffer2ZipBuffer(*pModel->Get(), "geometry/0" + suffix, byteArray, "attribute/0.att", CGByteArray(), "", outInfo, zipCompressionLevel);
					}
					else
					{
						CGString attPath = MakePathByRelativePath(relativePath, contentName + ".att");
						pCacheStorage->SetContent(attPath, byteArray);
					}
				}
			}
		}
	}

	if (outInfo.isEmpty())
	{
		if ((cacheType == G3DCacheType::TypeM3DV21 || cacheType == G3DCacheType::TypeM3DV22) && pTidArray != NULL)
			Ci_ZipTool::Buffer2ZipBuffer(*pModel->Get(), "geometry/0" + suffix, CGByteArray(), "", *pTidArray, "geometry/0.tid", outInfo, zipCompressionLevel);
		else
			Ci_ZipTool::Buffer2ZipBuffer(*pModel->Get(), "geometry/0" + suffix, CGByteArray(), "", CGByteArray(), "", outInfo, zipCompressionLevel);
	}
	CGString contentPath = MakePathByRelativePath(relativePath, contentName + ".m3d");
	pCacheStorage->SetContent(contentPath, outInfo);
	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ci_G3DM3DCacheTileImpl::Ci_G3DM3DCacheTileImpl(G3DCacheStorage* pCacheStorage)
{
	m_tile = Ci_Tile();
	m_relativePath = "";
	m_pCacheStorage = pCacheStorage;
	m_contentType = ContentType::None;
};

Ci_G3DM3DCacheTileImpl::~Ci_G3DM3DCacheTileImpl()
{
	Close();
};

void Ci_G3DM3DCacheTileImpl::SetBounding(MapGIS::Tile::BoundingVolume& bounding)
{
	if (i_IsValid())
		this->m_tile.boundingVolume = bounding;
}

MapGIS::Tile::BoundingVolume  Ci_G3DM3DCacheTileImpl::GetBounding()
{
	if (i_IsValid())
		return this->m_tile.boundingVolume;
	else
		return MapGIS::Tile::BoundingVolume();
}

void Ci_G3DM3DCacheTileImpl::SetGeometricError(float geometricError)
{
	if (i_IsValid())
		m_tile.lodError = geometricError;
}

float Ci_G3DM3DCacheTileImpl::GetGeometricError()
{
	if (i_IsValid())
		return m_tile.lodError;
	else
		return 0;
}

void Ci_G3DM3DCacheTileImpl::SetRefine(MapGIS::Tile::RefineType refine)
{
	if (i_IsValid())
		m_tile.lodType = refine;
}

MapGIS::Tile::RefineType Ci_G3DM3DCacheTileImpl::GetRefine()
{
	if (i_IsValid())
		return m_tile.lodType;
	else
		return MapGIS::Tile::RefineType::None;
}

void Ci_G3DM3DCacheTileImpl::SetMatrix(MapGIS::Tile::Matrix4D mMatrix)
{
	if (i_IsValid())
	{
		m_tile.transform.resize(16);
		m_tile.transform[0] = mMatrix[0][0]; m_tile.transform[4] = mMatrix[0][1]; m_tile.transform[8] = mMatrix[0][2];  m_tile.transform[12] = mMatrix[0][3];
		m_tile.transform[1] = mMatrix[1][0]; m_tile.transform[5] = mMatrix[1][1]; m_tile.transform[9] = mMatrix[1][2];  m_tile.transform[13] = mMatrix[1][3];
		m_tile.transform[2] = mMatrix[2][0]; m_tile.transform[6] = mMatrix[2][1]; m_tile.transform[10] = mMatrix[2][2]; m_tile.transform[14] = mMatrix[2][3];
		m_tile.transform[3] = mMatrix[3][0]; m_tile.transform[7] = mMatrix[3][1]; m_tile.transform[11] = mMatrix[3][2]; m_tile.transform[15] = mMatrix[3][3];
	}
}

MapGIS::Tile::Matrix4D Ci_G3DM3DCacheTileImpl::GetMatrix()
{
	if (i_IsValid() && m_tile.transform.size() == 16)
	{
		MapGIS::Tile::Matrix4D	 matrix;
		matrix[0][0] = m_tile.transform[0]; matrix[0][1] = m_tile.transform[4]; matrix[0][2] = m_tile.transform[8]; matrix[0][3] = m_tile.transform[12];
		matrix[1][0] = m_tile.transform[1]; matrix[1][1] = m_tile.transform[5]; matrix[1][2] = m_tile.transform[9]; matrix[1][3] = m_tile.transform[13];
		matrix[2][0] = m_tile.transform[2]; matrix[2][1] = m_tile.transform[6]; matrix[2][2] = m_tile.transform[10]; matrix[2][3] = m_tile.transform[14];
		matrix[3][0] = m_tile.transform[3]; matrix[3][1] = m_tile.transform[7]; matrix[3][2] = m_tile.transform[11]; matrix[3][3] = m_tile.transform[15];
		return matrix;
	}
	return MapGIS::Tile::Matrix4D();
}

bool Ci_G3DM3DCacheTileImpl::IsRoot()
{
	if (i_IsValid() && this->m_tile.parentNode_uri.IsEmpty())
		return true;
	return false;
}

CGString Ci_G3DM3DCacheTileImpl::GetRelativePath() const
{
	if (i_IsValid())
		return m_relativePath;
	return "";
}

void Ci_G3DM3DCacheTileImpl::SetName(CGString tileName)
{
	if (i_IsValid())
		this->m_tile.name = tileName;
}

CGString Ci_G3DM3DCacheTileImpl::GetName()
{
	if (i_IsValid())
		return this->m_tile.name;
	else
		return "";
}
gisLONG Ci_G3DM3DCacheTileImpl::Save()
{
	if (!i_IsValid())
		return 0;
	i_UpdateChildInfo();
	return i_Save();
}

gisLONG Ci_G3DM3DCacheTileImpl::i_Save()
{
	for (vector<Ci_G3DM3DCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		(*itr)->i_Save();
	}
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	if (m_tile.To(doc, allocator) <= 0)
		return 0;
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	gisLONG rtn = m_pCacheStorage->SetContent(m_relativePath, buffer.GetString(), buffer.GetLength());
	return  rtn;
}

void Ci_G3DM3DCacheTileImpl::Close()
{
	ReleaseChildObjects();
	m_tile = Ci_Tile();
	m_pCacheStorage = NULL;
	m_relativePath = "";
	m_contentType = ContentType::None;
}
void Ci_G3DM3DCacheTileImpl::ReleaseChildObjects()
{
	i_UpdateChildInfo();
	for (vector<Ci_G3DM3DCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		delete *itr;
	}
	m_childs.clear();
}
G3DCacheStorage* Ci_G3DM3DCacheTileImpl::GetCacheStorage()
{
	if (i_IsValid())
		return m_pCacheStorage;
	else
		return  NULL;
}

G3DTile* Ci_G3DM3DCacheTileImpl::AppendChild(TileAppendMode mode)
{
	if (!i_IsValid() || m_pCacheStorage == NULL)
		return NULL;
	Ci_G3DM3DCacheTileImpl *rtn = i_CreateChildInstance();
	if (rtn != NULL)
	{
		i_InitChildInfo(rtn);
		m_childs.emplace_back(rtn);
	}
	return rtn;
}

gisLONG Ci_G3DM3DCacheTileImpl::GetChildNum()
{
	if (i_IsValid())
		return this->m_tile.childrenNode.size();
	return 0;
}

gisLONG Ci_G3DM3DCacheTileImpl::RemoveChild(G3DTile* pChildTile)
{
	if (!i_IsValid() || pChildTile == NULL)
		return 0;
	Ci_G3DM3DCacheTileImpl* pM3dTile = dynamic_cast<Ci_G3DM3DCacheTileImpl*>(pChildTile);
	if (pM3dTile == NULL || !pM3dTile->i_IsValid())
		return 0;

	CGString relativePath = pM3dTile->m_relativePath;
	relativePath.Replace('\\', '/');

	while (relativePath.StartsWith("./"))
	{
		relativePath = relativePath.Right(relativePath.GetLength() - 2);
	}

	for (vector<Ci_ChildTileInfo>::iterator itr = this->m_tile.childrenNode.begin(); itr != this->m_tile.childrenNode.end(); itr++)
	{
		CGString tempTileRelativePath = MakePathByRelativePath(this->m_relativePath, itr->uri);

		tempTileRelativePath.Replace('\\', '/');
		while (tempTileRelativePath.StartsWith("./"))
		{
			tempTileRelativePath = tempTileRelativePath.Right(tempTileRelativePath.GetLength() - 2);
		}
		if (relativePath == tempTileRelativePath)
		{
			this->m_tile.childrenNode.erase(itr);
			break;
		}
	}
	vector<Ci_G3DM3DCacheTileImpl*>::iterator findItr = find(m_childs.begin(), m_childs.end(), pM3dTile);
	if (findItr != m_childs.end())
	{
		m_childs.erase(findItr);
		delete pM3dTile;
	}
	return 1;
}

gisLONG Ci_G3DM3DCacheTileImpl::RemoveChild(int index)
{
	if (!i_IsValid() || index >= this->m_tile.childrenNode.size())
		return 0;
	Ci_ChildTileInfo& info = this->m_tile.childrenNode[index];

	CGString tempTileRelativePath = MakePathByRelativePath(this->m_relativePath, info.uri);
	tempTileRelativePath.Replace('\\', '/');
	while (tempTileRelativePath.StartsWith("./"))
	{
		tempTileRelativePath = tempTileRelativePath.Right(tempTileRelativePath.GetLength() - 2);
	}

	for (vector<Ci_G3DM3DCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		CGString relativePath = ((Ci_G3DM3DCacheTileImpl*)*itr)->GetRelativePath();
		relativePath.Replace('\\', '/');
		while (relativePath.StartsWith("./"))
		{
			relativePath = relativePath.Right(relativePath.GetLength() - 2);
		}
		if (relativePath == tempTileRelativePath)
		{
			Ci_G3DM3DCacheTileImpl* pTile = *itr;
			m_childs.erase(itr);
			delete pTile;
			break;
		}
	}

	this->m_tile.childrenNode.erase(this->m_tile.childrenNode.begin() + index);

	return 1;
}

G3DTile* Ci_G3DM3DCacheTileImpl::GetChild(int index)
{
	if (!i_IsValid() || index >= this->m_tile.childrenNode.size() )
		return NULL;
	Ci_ChildTileInfo& info = this->m_tile.childrenNode[index];

	CGString tempTileRelativePath = MakePathByRelativePath(this->m_relativePath, info.uri);
	tempTileRelativePath.Replace('\\', '/');
	while (tempTileRelativePath.StartsWith("./"))
	{
		tempTileRelativePath = tempTileRelativePath.Right(tempTileRelativePath.GetLength() - 2);
	}

	for (vector<Ci_G3DM3DCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		CGString relativePath = (*itr)->GetRelativePath();
		relativePath.Replace('\\', '/');
		while (relativePath.StartsWith("./"))
		{
			relativePath = relativePath.Right(relativePath.GetLength() - 2);
		}
		if (relativePath == tempTileRelativePath)
			return *itr;
	}

	G3DTile* rtn = G3DTile::Open(m_pCacheStorage, tempTileRelativePath, i_GetCacheType());
	if (rtn != NULL && dynamic_cast<Ci_G3DM3DCacheTileImpl *>(rtn) != NULL)
		m_childs.emplace_back((Ci_G3DM3DCacheTileImpl *)rtn);
	return rtn;
}

TileContentInfo Ci_G3DM3DCacheTileImpl::GetContentInfo()
{
	TileContentInfo rtn;
	if (!i_IsValid())
		return rtn;
	if (this->m_tile.tileDataInfoList.size() > 0 && this->m_tile.tileDataInfoIndex <this->m_tile.tileDataInfoList.size())
	{
		Ci_Content content = this->m_tile.tileDataInfoList[this->m_tile.tileDataInfoIndex];
		CGString contentPath = MakePathByRelativePath(m_relativePath, content.tileData_uri);

		CGString geoUri = content.geometry_uri;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		if (m_contentType == ContentType::None)
		{
			CGByteArray outInfo;
			m_pCacheStorage->GetContent(contentPath, outInfo);
			CGByteArray geoArray;
			Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
			if (geoArray.size() > 0)
			{
				string magic = ReadByteArrayToString(geoArray, 0, 4);
				if (StrNICmp("i3dm", magic.c_str(), 4) == 0)
					m_contentType = ContentType::M3DI3DM;
				else if (StrNICmp("pnts", magic.c_str(), 4) == 0)
					m_contentType = ContentType::M3DPNTS;
				else if (StrNICmp("cmpt", magic.c_str(), 4) == 0)
					m_contentType = ContentType::M3DCMPT;
				else
					m_contentType = ContentType::M3DGLB;
			}
		}
		rtn.tidURI = content.geometry_tid_uri;
		rtn.contentType = m_contentType;
		rtn.geometryURI = geoUri;
		rtn.dataType = content.dataType;
		rtn.geometryType = content.geometry_geometryType;
		rtn.contentURI = content.tileData_uri;
		if (!content.attribute_uri.IsEmpty())
		{
			rtn.attributeURI = content.attribute_uri;
		}
	}
	return rtn;
}
MapGIS::Tile::G3DTileInfo* Ci_G3DM3DCacheTileImpl::GetInfo()
{
	G3DM3DTileInfo *rtn = new G3DM3DTileInfo(&m_tile,false);
	return rtn;
}

void Ci_G3DM3DCacheTileImpl::i_SetTileContentInfo(CGString contentName,DataType dataType ,GeometryType  geometryType, GeoCompressType compressType, G3DCacheType cacheType, bool hasTid,bool hasAttribute,bool attInContent, ContentType contentType)
{
	Ci_Content content;
	content.dataType = dataType;
	content.geometry_blobType = BlobType::glbx;
	content.geometry_uri = "./geometry/0.glbx";
	if (cacheType == G3DCacheType::TypeM3DV21 || cacheType == G3DCacheType::TypeM3DV22)
	{
		switch (contentType)
		{
		case MapGIS::Tile::ContentType::M3DGLB:
			content.geometry_blobType = BlobType::glb;
			content.geometry_uri = "./geometry/0.glb";

			break;
		case MapGIS::Tile::ContentType::M3DI3DM:
			content.geometry_blobType = BlobType::i3dm;
			content.geometry_uri = "./geometry/0.i3dm";
			break;
		case MapGIS::Tile::ContentType::M3DPNTS:
			content.geometry_blobType = BlobType::pnts;
			content.geometry_uri = "./geometry/0.pnts";
			break;
		case MapGIS::Tile::ContentType::M3DCMPT:
			content.geometry_blobType = BlobType::cmpt;
			content.geometry_uri = "./geometry/0.cmpt";
			break;
		case MapGIS::Tile::ContentType::None:
		case MapGIS::Tile::ContentType::B3DM:
		case MapGIS::Tile::ContentType::I3DM:
		case MapGIS::Tile::ContentType::PNTS:
		case MapGIS::Tile::ContentType::CMPT:
		default:
			break;
		}
	}
	if (hasTid)
	{
		content.geometry_tid_uri = "./geometry/0.tid";
	}

	content.geometry_geometryType = geometryType;
	content.geometry_geoCompressType = compressType;
	if (hasAttribute)
	{
		switch (cacheType)
		{
		case G3DCacheType::TypeM3DV20:
		{
			content.attribute_attType = AttFileType::JSON;
			content.attribute_uri = "./attribute/0.json";
			break;
		}

		case G3DCacheType::TypeM3DV21:
		case G3DCacheType::TypeM3DV22:
		{
			content.attribute_attType = AttFileType::ATT;
			if (attInContent)
				content.attribute_uri = "./attribute/0.att";
			else
				content.attribute_uri = "../" + contentName + ".att";
			break;
		}
		default:
			break;
		}
	}
	content.tileData_uri = contentName + ".m3d";
	this->m_tile.tileDataInfoList.insert(this->m_tile.tileDataInfoList.begin(), content);
	this->m_tile.tileDataInfoIndex = 0;
};

void Ci_G3DM3DCacheTileImpl::i_InitChildInfo(Ci_G3DM3DCacheTileImpl * pChild)
{
	if (pChild == NULL || m_pCacheStorage == NULL)
		return;
	size_t num = this->m_tile.childrenNode.size();
	Ci_ChildTileInfo info;
	if (this->IsRoot())
	{//根节点
		CGString childTileRelativePath = CGString("./node/" + std::to_string(num) + "/" + std::to_string(num) + ".json", CGString::EncodeType::GB18030);

		bool isExist = true;
		while (isExist)
		{
			isExist = false;
			for (vector<Ci_ChildTileInfo>::iterator itr = this->m_tile.childrenNode.begin(); itr != this->m_tile.childrenNode.end(); itr++)
			{
				CGString 	tempTileRelativePath = itr->uri;
				if (childTileRelativePath == tempTileRelativePath  || m_pCacheStorage->IsExistContent(MakePathByRelativePath(this->m_relativePath, childTileRelativePath)))
				{
					isExist = true;
					num++;
					childTileRelativePath = CGString("./node/" + std::to_string(num) + "/" + std::to_string(num) + ".json", CGString::EncodeType::GB18030);
					break;
				}
			}
		}
		pChild->m_relativePath = childTileRelativePath;
		info.uri = childTileRelativePath;
		pChild->m_tile.parentNode_uri = "../../rootNode.json";
		pChild->m_tile.name = "0_" + std::to_string(num);
		pChild->m_tile.lodLevel = 1;
	}
	else
	{
		CGString 	childTileRelativePath = "";
		childTileRelativePath = CGString("./" + std::to_string(num) + "/" + std::to_string(num) + ".json", CGString::EncodeType::GB18030);
		bool isExist = true;
		while (isExist)
		{
			isExist = false;
			for (vector<Ci_ChildTileInfo>::iterator itr = this->m_tile.childrenNode.begin(); itr != this->m_tile.childrenNode.end(); itr++)
			{
				CGString 	tempTileRelativePath = itr->uri;
				if (childTileRelativePath == tempTileRelativePath || m_pCacheStorage->IsExistContent(MakePathByRelativePath(this->m_relativePath, childTileRelativePath)))
				{
					isExist = true;
					num++;
					childTileRelativePath = CGString("./" + std::to_string(num) + "/" + std::to_string(num) + ".json", CGString::EncodeType::GB18030);
					break;
				}
			}
		}

		pChild->m_relativePath = MakePathByRelativePath(this->m_relativePath, childTileRelativePath);
		info.uri = childTileRelativePath;
		CGString filename = this->m_relativePath;
		string::size_type iPos = this->m_relativePath.ReverseFind('/');
		if (iPos >= 0)
		{
			filename = this->m_relativePath.Right(this->m_relativePath.GetLength() - iPos -1);
		}
		pChild->m_tile.parentNode_uri = "../" + filename;
		pChild->m_tile.name = this->m_tile.name + "_" + std::to_string(num);
		pChild->m_tile.lodLevel = this->m_tile.lodLevel + 1;
	}
	this->m_tile.childrenNode.emplace_back(info);
}

gisLONG Ci_G3DM3DCacheTileImpl::i_UpdateChildInfo()
{
	//没有对象的子节点，表示没有更改子节点信息。
	for (vector<Ci_G3DM3DCacheTileImpl*>::iterator itr = m_childs.begin(); itr != m_childs.end(); itr++)
	{
		(*itr)->i_UpdateChildInfo();
		CGString relativePath = (*itr)->m_relativePath;
		relativePath.Replace('\\', '/');
		while (relativePath.StartsWith("./"))
		{
			relativePath = relativePath.Right(relativePath.GetLength() - 2);
		}
		for (vector<Ci_ChildTileInfo>::iterator infoItr = this->m_tile.childrenNode.begin(); infoItr != this->m_tile.childrenNode.end(); infoItr++)
		{
			CGString tempTileRelativePath = MakePathByRelativePath(this->m_relativePath, infoItr->uri);
			tempTileRelativePath.Replace('\\', '/');
			while (tempTileRelativePath.StartsWith("./"))
			{
				tempTileRelativePath = tempTileRelativePath.Right(tempTileRelativePath.GetLength() - 2);
			}
			if (relativePath == tempTileRelativePath)
			{
				infoItr->lodError = (*itr)->m_tile.lodError;
				infoItr->boundingVolume = (*itr)->m_tile.boundingVolume;
				//infoItr->transform.clear();
			    //infoItr->transform.insert(infoItr->transform.begin(), (*itr)->m_tile.transform.begin(), (*itr)->m_tile.transform.end());
				break;
			}
		}
	}
	return 1;
}

bool Ci_G3DM3DCacheTileImpl::i_IsValid() const
{
	if (m_pCacheStorage != NULL && !m_relativePath.IsEmpty())
		return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ci_G3DM3D20CacheTileImpl::Ci_G3DM3D20CacheTileImpl(G3DCacheStorage* pCacheStorage) : Ci_G3DM3DCacheTileImpl(pCacheStorage)
{
}

Ci_G3DM3D20CacheTileImpl::~Ci_G3DM3D20CacheTileImpl()
{
}

Ci_G3DM3DCacheTileImpl* Ci_G3DM3D20CacheTileImpl::i_CreateChildInstance()
{
	Ci_G3DM3D20CacheTileImpl *rtn = new Ci_G3DM3D20CacheTileImpl(m_pCacheStorage);
	return rtn;
}

G3DCacheType Ci_G3DM3D20CacheTileImpl::i_GetCacheType()
{
	return G3DCacheType::TypeM3DV20;
}

gisLONG Ci_G3DM3D20CacheTileImpl::WriteBatched3DModel(CGString contentName, G3DModel* pModel, Attribute *pAttribute, bool attInContent, GeoCompressType  geoCompressType, DataType dataType)
{
	if (pModel == NULL || !i_IsValid())
		return 0;
	GeometryType  geoType = pModel->GetGeometryType();
	if (geoType != GeometryType::Line &&  geoType != GeometryType::Surface&&  geoType != GeometryType::Entity)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	bool hasAtt = false;
	if (pAttribute != NULL && pAttribute->layers.size() == 1 && pAttribute->layers[0].records.size() > 0)
		hasAtt = true;

	i_SetTileContentInfo(contentName, dataType, geoType, geoCompressType, G3DCacheType::TypeM3DV20, false, hasAtt, false, ContentType::M3DGLB);
	m_contentType = ContentType::M3DGLB;
	Ci_ModelGltf gltf;
	gltf.From(pModel, geoCompressType, WriteIdType::oid);
	return WriteData(m_pCacheStorage, m_relativePath, contentName, &gltf, pAttribute, NULL, G3DCacheType::TypeM3DV20, false, ContentType::M3DGLB, (!hasAtt && geoCompressType != GeoCompressType::None) ? 0 : 6);
}

gisLONG Ci_G3DM3D20CacheTileImpl::ReadBatched3DModel(TileContentInfo info, G3DModel* pModel, Attribute * pAttribute)
{
	if (!i_IsValid() || info.contentType != ContentType::M3DGLB)
		return 0;
	if(pModel !=NULL && pModel->GetGeometryType() != info.geometryType)
		return 0;

	CGString contentPath = MakePathByRelativePath(m_relativePath, info.contentURI);
	CGByteArray outInfo;
	m_pCacheStorage->GetContent(contentPath, outInfo);

	if (pModel != NULL)
	{
		CGByteArray geoArray;
		CGString geoUri = info.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelGltf gltf;
			((Ci_TileModel*)&gltf)->From(geoArray);
			LayerAttribute records;
			D_3DOT rtcCenter;
			gltf.To(pModel);
		}
	}

	if (pAttribute != NULL)
	{
		CGByteArray attArray;
		CGString attUri = info.attributeURI;
		while (attUri.StartsWith("./"))
		{
			attUri = attUri.Right(attUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
		if (attArray.size() > 0)
		{
			Ci_ModelAttributeJsonFile attributeMode;
			LayerAttribute layerAttribute;
			if (attributeMode.From(attArray)>0 && attributeMode.To(layerAttribute) > 0)
			{
				pAttribute->layers.emplace_back(layerAttribute);
			}
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D20CacheTileImpl::WriteInstanced3DModel(CGString contentName, G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, Attribute *pAttribute, bool isAttFile, GeoCompressType  geoCompressType, DataType dataType)
{
	if (!i_IsValid() || pModel == NULL || pInstances == NULL || pInstances->size() <= 0)
		return 0;
	if (M3d20ExistI3dm)
	{
		if (contentName.IsEmpty())
			contentName = GetName();
		GeometryType  geoType = pModel->GetGeometryType();
		Ci_ModelI3dm i3dm;
		vector<gisINT64> batchIDToId;
		i3dm.From(pModel, *pInstances, geoCompressType, NULL, false, WriteIdType::oid, batchIDToId);
		bool hasAtt = false;
		if (pAttribute != NULL && pAttribute->layers.size() == 1 && pAttribute->layers[0].records.size() > 0)
			hasAtt = true;
		m_contentType = ContentType::M3DI3DM;
		i_SetTileContentInfo(contentName, dataType, geoType, geoCompressType, G3DCacheType::TypeM3DV20, false, hasAtt, false, ContentType::M3DI3DM);
		return WriteData(m_pCacheStorage, m_relativePath, contentName, &i3dm, pAttribute, NULL, G3DCacheType::TypeM3DV20, false, ContentType::M3DI3DM, (!hasAtt && geoCompressType != GeoCompressType::None) ? 0 : 6);
	}
	else
	{
		if (contentName.IsEmpty())
			contentName = GetName();

		GeometryType  geoType = pModel->GetGeometryType();
		Ci_ModelGltf gltf;
		vector<ContentBase*> contentItems;
		GeometryContent item;
		item.Set3DModel(pModel);
		item.SetModelInstance(pInstances);
		contentItems.emplace_back(&item);
		gltf.From(&contentItems, geoCompressType, WriteIdType::oid,false);
		bool hasAtt = false;
		if (pAttribute != NULL && pAttribute->layers.size() == 1 && pAttribute->layers[0].records.size() > 0)
			hasAtt = true;
		m_contentType = ContentType::M3DGLB;
		i_SetTileContentInfo(contentName, dataType, geoType, geoCompressType, G3DCacheType::TypeM3DV20, false, hasAtt, false, m_contentType);
		return WriteData(m_pCacheStorage, m_relativePath, contentName, &gltf, pAttribute, NULL, G3DCacheType::TypeM3DV20, false, m_contentType, (!hasAtt && geoCompressType != GeoCompressType::None) ? 0 : 6);
	}
}

gisLONG Ci_G3DM3D20CacheTileImpl::ReadInstanced3DModel(TileContentInfo info, G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, Attribute * pAttribute)
{
	if (!i_IsValid() || pInstances == NULL || info.contentType != ContentType::M3DI3DM)
		return 0;
	if (pModel != NULL && pModel->GetGeometryType() != info.geometryType)
		return 0;
	CGString contentPath = MakePathByRelativePath(m_relativePath, info.contentURI);
	CGByteArray outInfo;
	m_pCacheStorage->GetContent(contentPath, outInfo);

	if (pModel != NULL)
	{
		CGByteArray geoArray;
		CGString geoUri = info.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelI3dm i3dm;
			((Ci_TileModel*)&i3dm)->From(geoArray);
			LayerAttribute records;
			D_3DOT rtcCenter;
			i3dm.To(pModel, *pInstances, records, rtcCenter);
		}
	}

	if (pAttribute != NULL)
	{
		CGByteArray attArray;
		CGString attUri = info.attributeURI;
		while (attUri.StartsWith("./"))
		{
			attUri = attUri.Right(attUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
		if (attArray.size() > 0)
		{
			Ci_ModelAttributeJsonFile attributeMode;
			LayerAttribute layerAttribute;
			if (attributeMode.From(attArray) > 0 && attributeMode.To(layerAttribute) > 0)
			{
				pAttribute->layers.emplace_back(layerAttribute);
			}
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D20CacheTileImpl::WritePoints(CGString contentName, PointsModel* pModel, Attribute *pAttribute, bool attInContent, GeoCompressType  geoCompressType, DataType dataType)
{
	if (!i_IsValid() || pModel == NULL)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	if (geoCompressType != GeoCompressType::Draco)
		geoCompressType = GeoCompressType::None;
	m_contentType = ContentType::M3DPNTS;

	bool hasAtt = false;
	if (pAttribute != NULL && pAttribute->layers.size() == 1 && pAttribute->layers[0].records.size()>0)
		hasAtt = true;
	i_SetTileContentInfo(contentName, dataType, GeometryType::Point, geoCompressType, G3DCacheType::TypeM3DV20, false, hasAtt, false, ContentType::M3DPNTS);
	Ci_ModelPnts pnts;
	pnts.From(*pModel, NULL, geoCompressType == GeoCompressType::Draco, NULL);
	return WriteData(m_pCacheStorage, m_relativePath, contentName, &pnts, pAttribute, NULL, G3DCacheType::TypeM3DV20, false, ContentType::M3DPNTS, (hasAtt || geoCompressType != GeoCompressType::Draco) ? 6 : 0);
}

gisLONG Ci_G3DM3D20CacheTileImpl::ReadPoints(TileContentInfo info, PointsModel* pModel, Attribute * pAttribute)
{
	if (!i_IsValid() || info.contentType != ContentType::M3DPNTS || info.geometryType != GeometryType::Point)
		return 0;
	if (pModel != NULL && pModel->GetGeometryType() != info.geometryType)
		return 0;

	CGString contentPath = MakePathByRelativePath(m_relativePath, info.contentURI);
	CGByteArray outInfo;
	m_pCacheStorage->GetContent(contentPath, outInfo);

	if (pModel != NULL)
	{
		CGByteArray geoArray;
		CGString geoUri = info.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelPnts pnts;
			((Ci_TileModel*)&pnts)->From(geoArray);
			LayerAttribute records;
			D_3DOT rtcCenter;

			if (pModel != NULL)
			{
				pnts.To(*pModel, records, rtcCenter);
			}
			else
			{
				PointsModel model;
				pnts.To(model, records, rtcCenter);
			}
		}
	}

	if (pAttribute != NULL)
	{
		CGByteArray attArray;
		CGString attUri = info.attributeURI;
		while (attUri.StartsWith("./"))
		{
			attUri = attUri.Right(attUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
		if (attArray.size() > 0)
		{
			Ci_ModelAttributeJsonFile  attributeMode;
			LayerAttribute		   layerAttribute;
			if (attributeMode.From(attArray) > 0 && attributeMode.To(layerAttribute) > 0)
			{
				pAttribute->layers.emplace_back(layerAttribute);
			}
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D20CacheTileImpl::WriteComposite(CGString contentName, vector<ContentBase*>* pContents, bool attInContent, GeoCompressType  geoCompressType, DataType dataType)
{
	if (contentName.IsEmpty())
		contentName = GetName();
	auto importAttribute = [](const Attribute & orAtt, Attribute & desAtt)
	{
		for (vector<LayerAttribute>::const_iterator itr = orAtt.layers.begin(); itr != orAtt.layers.end(); itr++)
		{
			bool isExist = false;

			for (vector<LayerAttribute>::iterator desItr = desAtt.layers.begin(); desItr != desAtt.layers.end(); desItr++)
			{
				if (itr->layerInfo == desItr->layerInfo)
				{
					isExist = true;
					desItr->records.insert(desItr->records.begin(), itr->records.begin(), itr->records.end());
					break;
				}
			}
			if (!isExist)
			{
				desAtt.layers.emplace_back(*itr);
			}
		}
	};

	Attribute attribute;
	GeometryType  geoType = GeometryType::Surface;
	for (vector<ContentBase*>::iterator itr = pContents->begin(); itr != pContents->end(); itr++)
	{
		if (*itr == NULL)
			continue;
		GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
		if (pGeoContent == NULL)
			continue;
		vector<ModelInstance>* pInstances = pGeoContent->GetModelInstance();
		Attribute* pAttribute = pGeoContent->GetAttribute();
		G3DModel* p3DModel = pGeoContent->Get3DModel();
		if (p3DModel == NULL)
			continue;
		geoType = p3DModel->GetGeometryType();
		if (pInstances == NULL || pInstances->size() <= 0)
		{
			if (p3DModel->GetGeometryType() == GeometryType::Point)
			{
				if (pAttribute != NULL)
					importAttribute(*pAttribute, attribute);
			}
			else
			{
				if (pAttribute != NULL)
					importAttribute(*pAttribute, attribute);
			}
		}
		else
		{
			if (pAttribute != NULL)
				importAttribute(*pAttribute, attribute);
		}
	}
	Ci_ModelGltf gltf;
	gltf.From(pContents, geoCompressType, WriteIdType::oid, false);
	bool hasAtt = false;
	if (attribute.layers.size() == 1 && attribute.layers[0].records.size() > 0)
		hasAtt = true;
	m_contentType = ContentType::M3DGLB;
	i_SetTileContentInfo(contentName, dataType, geoType, geoCompressType, G3DCacheType::TypeM3DV20, false, hasAtt, false, m_contentType);
	return WriteData(m_pCacheStorage, m_relativePath, contentName, &gltf, &attribute, NULL, G3DCacheType::TypeM3DV20, false, m_contentType, (!hasAtt && geoCompressType != GeoCompressType::None) ? 0 : 6);
}

gisLONG Ci_G3DM3D20CacheTileImpl::ReadComposite(TileContentInfo info, vector<ContentBase*>* pContents)
{
	return 0;
}

gisLONG Ci_G3DM3D20CacheTileImpl::WriteContent(CGString contentName, vector<ContentBase*>* pContents, MapGIS::Tile::WriteContentParamBase& param)
{
	if (pContents == NULL)
		return 0;
	WriteGeometryContentParam* pContentParam =  dynamic_cast<WriteGeometryContentParam*>(&param);
	if (pContentParam == NULL)
		return 0;
	if (pContents->size() > 1)
		return WriteComposite(contentName, pContents, pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
	ContentBase* item = pContents->at(0);
	GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(item);
	if (pGeoContent == NULL)
		return 0;
	G3DModel*  pModel = pGeoContent->Get3DModel();
	vector<ModelInstance>* pInstance = pGeoContent->GetModelInstance();
	if (pModel == NULL)
		return 0;
	if (pInstance != NULL &&pInstance->size() > 0)
		return WriteInstanced3DModel(contentName, pModel, pInstance, pGeoContent->GetAttribute(), pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
	if (pModel->GetGeometryType() == GeometryType::Point)
		return WritePoints(contentName, (PointsModel*)pModel, pGeoContent->GetAttribute(), pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
	return WriteBatched3DModel(contentName, (PointsModel*)pModel, pGeoContent->GetAttribute(), pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
}

gisLONG Ci_G3DM3D20CacheTileImpl::ReadContent(vector<ContentBase*>* pContents)
{
	if (pContents == NULL)
		return 0;
	TileContentInfo tileContentInfo = GetContentInfo();
	gisLONG rtn = 0;
	switch (tileContentInfo.contentType)
	{
	case ContentType::M3DGLB:
	{
		if (!i_IsValid())
			return 0;
		CGString contentPath = MakePathByRelativePath(m_relativePath, tileContentInfo.contentURI);
		CGByteArray outInfo;
		m_pCacheStorage->GetContent(contentPath, outInfo);

		CGByteArray geoArray;
		CGString geoUri = tileContentInfo.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelGltf gltf;
			((Ci_TileModel*)&gltf)->From(geoArray);
			gltf.To(pContents);
		}

		if (pContents->size() == 1  && dynamic_cast<GeometryContent*>(pContents->at(0)) != NULL)
		{
			MapGIS::Tile::Attribute*  pAttribute =  dynamic_cast<GeometryContent*>(pContents->at(0))->GetAttribute();
			if (pAttribute != NULL) 
			{
				CGByteArray attArray;
				CGString attUri = tileContentInfo.attributeURI;
				while (attUri.StartsWith("./"))
				{
					attUri = attUri.Right(attUri.GetLength() - 2);
				}
				Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
				if (attArray.size() > 0)
				{
					Ci_ModelAttributeJsonFile attributeMode;
					LayerAttribute layerAttribute;
					if (attributeMode.From(attArray) > 0 && attributeMode.To(layerAttribute) > 0)
					{
						pAttribute->layers.emplace_back(layerAttribute);
					}
				}
			}
		}

	}
	break;
	case ContentType::M3DI3DM:
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Surface, true, true);
		rtn = ReadInstanced3DModel(tileContentInfo, pItem->Get3DModel(), pItem->GetModelInstance(), pItem->GetAttribute());
		pContents->emplace_back(pItem);
	}

	break;
	case ContentType::M3DPNTS:
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Point, true, false);
		rtn = ReadPoints(tileContentInfo, (PointsModel*)pItem->Get3DModel(), pItem->GetAttribute());
		pContents->emplace_back(pItem);
	}
	break;
	case ContentType::M3DCMPT:
		rtn = ReadComposite(tileContentInfo, pContents);
		break;

	case ContentType::B3DM:
	case ContentType::I3DM:
	case ContentType::PNTS:
	case ContentType::CMPT:
	case ContentType::None:
	case ContentType::M3DVoxel:
	default:
		break;
	}
	return rtn;
}

G3DTile* Ci_G3DM3D20CacheTileImpl::Open(G3DCacheStorage* pCacheStorage, CGString relativePath)
{
	if (pCacheStorage == NULL || relativePath.IsEmpty())
		return NULL;
	CGByteArray value;
	if (pCacheStorage->GetContent(relativePath, value) <= 0 || value.isEmpty())
		return NULL;
	rapidjson::Document doc;
	if (doc.Parse(value.data(), value.size()).HasParseError())
		return NULL;
	Ci_G3DM3D20CacheTileImpl* rtn = new Ci_G3DM3D20CacheTileImpl(pCacheStorage);
	rtn->m_relativePath = relativePath;
	if (!doc.IsObject() || rtn->m_tile.From(doc) <= 0)
	{
		delete rtn;
		return NULL;
	}
	return rtn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ci_G3DM3D21CacheTileImpl::Ci_G3DM3D21CacheTileImpl(G3DCacheStorage* pCacheStorage) : Ci_G3DM3DCacheTileImpl(pCacheStorage)
{
	//M3D 2.1 不设置 lodMode
	m_tile.lodMode = LodMode::None;
}

Ci_G3DM3D21CacheTileImpl::~Ci_G3DM3D21CacheTileImpl()
{
}

Ci_G3DM3DCacheTileImpl* Ci_G3DM3D21CacheTileImpl::i_CreateChildInstance()
{
	Ci_G3DM3D21CacheTileImpl *rtn = new Ci_G3DM3D21CacheTileImpl(m_pCacheStorage);
	return rtn;
}
G3DCacheType Ci_G3DM3D21CacheTileImpl::i_GetCacheType()
{
	return G3DCacheType::TypeM3DV21;
}

gisLONG GetDataAttributeByAllAttribute(G3DModel* p3dModel, vector<ModelInstance>* pModelInstance, Attribute* pAttribute, Attribute* pAllAttribute)
{
	if (p3dModel == NULL || pAttribute  == NULL || pAllAttribute == NULL)
		return 0;
	set<gisINT64> ids;
	if (pModelInstance != NULL && pModelInstance->size() > 0)
	{
		for (vector<ModelInstance>::iterator insItr = pModelInstance->begin(); insItr != pModelInstance->end(); insItr++)
		{
			if (insItr->hasId)
				ids.insert(insItr->id);
		}
	}
	else
	{
		if (p3dModel != NULL)
		{
			if (p3dModel->GetGeometryType() == GeometryType::Surface)
			{
				for (vector<SurfaceFeature>::iterator sfItr = ((SurfacesModel*)p3dModel)->features.begin(); sfItr != ((SurfacesModel*)p3dModel)->features.end(); sfItr++)
				{
					if (sfItr->ids.size() > 0)
					{
						ids.insert(sfItr->ids.begin(), sfItr->ids.end());
					}
					else if (sfItr->id >= 0)
					{
						ids.insert(sfItr->id);
					}
				}
			}
			else if (p3dModel->GetGeometryType() == GeometryType::Entity)
			{
				for (vector<EntityFeature>::iterator sfItr = ((EntitiesModel*)p3dModel)->features.begin(); sfItr != ((EntitiesModel*)p3dModel)->features.end(); sfItr++)
				{
					if (sfItr->ids.size() > 0)
					{
						ids.insert(sfItr->ids.begin(), sfItr->ids.end());
					}
					else if (sfItr->id >= 0)
					{
						ids.insert(sfItr->id);
					}
				}
			}
			else if (p3dModel->GetGeometryType() == GeometryType::Line)
			{
				for (vector<LineFeature>::iterator sfItr = ((LinesModel*)p3dModel)->features.begin(); sfItr != ((LinesModel*)p3dModel)->features.end(); sfItr++)
				{
					if (sfItr->id >= 0)
					{
						ids.insert(sfItr->id);
					}
				}
			}
			else if (p3dModel->GetGeometryType() == GeometryType::Point)
			{
				for (vector<PointFeature>::iterator sfItr = ((PointsModel*)p3dModel)->features.begin(); sfItr != ((PointsModel*)p3dModel)->features.end(); sfItr++)
				{
					if (sfItr->id >= 0)
					{
						ids.insert(sfItr->id);
					}
				}
			}
		}
	}
	if (ids.size() <= 0)
		return 0;
	for (vector<LayerAttribute>::iterator layerItr = pAllAttribute->layers.begin(); layerItr != pAllAttribute->layers.end(); layerItr++)
	{
		vector<Record> records;
		for (vector<Record>::iterator recordItr = layerItr->records.begin(); recordItr != layerItr->records.end(); recordItr++)
		{
			if (ids.find(recordItr->GetID()) != ids.end())
			{
				records.emplace_back(*recordItr);
			}
		}
		if (records.size() > 0)
		{
			LayerAttribute layerAtt;
			layerAtt.layerInfo = layerItr->layerInfo;
			layerAtt.records.swap(records);
			pAttribute->layers.emplace_back(layerAtt);
		}
	}
	return 1;
}

gisLONG GetDataAttributeByAllAttribute(vector<ContentBase*>* pContents, Attribute* pAllAttribute)
{
	if (pContents == NULL || pContents->size() <= 0 || pAllAttribute == NULL)
		return 0;

	for (vector<ContentBase*>::iterator itr = pContents->begin(); itr != pContents->end(); itr++)
	{
		if (*itr == NULL)
			continue;
		GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
		if (pGeoContent == NULL)
			continue;

		Attribute* pAttribute = pGeoContent->GetAttribute();
		G3DModel* p3dModel = pGeoContent->Get3DModel();
		vector<ModelInstance>* pModelInstance = pGeoContent->GetModelInstance();
		GetDataAttributeByAllAttribute(p3dModel, pModelInstance, pAttribute, pAllAttribute);
	}
	return 1;
}

gisLONG Ci_G3DM3D21CacheTileImpl::WriteBatched3DModel(CGString contentName, G3DModel* pModel, Attribute *pAttribute, bool attInContent, GeoCompressType  geoCompressType, DataType dataType)
{
	if (!i_IsValid() || pModel == NULL)
		return 0;

	GeometryType  geoType = pModel->GetGeometryType();
	if (geoType != GeometryType::Line &&  geoType != GeometryType::Surface&&  geoType != GeometryType::Entity)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();

	Ci_ModelGltf gltf;
	map<gisINT64, gisINT64> resetIDMap;
	gltf.FromResetID(pModel, geoCompressType, WriteIdType::batchID, resetIDMap);

	bool hasAtt = false;
	if (pAttribute != NULL && pAttribute->layers.size() >= 1)
	{
		int num = 0;
		for (vector<LayerAttribute>::iterator itr = pAttribute->layers.begin(); itr != pAttribute->layers.end(); itr++)
		{
			if (itr->layerInfo.fieldInfos.size() > 0)
				num += itr->records.size();
		}

		if (num > 0)
			hasAtt = true;
	}

	Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
	m_contentType = ContentType::M3DGLB;
	i_SetTileContentInfo(contentName, dataType, geoType, geoCompressType, i_GetCacheType(), resetIDMap.size() > 0, hasAtt && pSqliteTool == NULL, attInContent, ContentType::M3DGLB);

	CGByteArray tidByteArray;
	if (resetIDMap.size() > 0)
	{
		unordered_map<gisINT64, gisINT64> batchIDToID;
		for (map<gisINT64, gisINT64>::iterator itr = resetIDMap.begin(); itr != resetIDMap.end(); itr++)
		{
			batchIDToID.insert(make_pair(itr->second, itr->first));
		}
		vector<gisINT64> ids;
		for (int i = 0; i < batchIDToID.size(); i++)
		{
			if (batchIDToID.find(i) != batchIDToID.end())
				ids.emplace_back(batchIDToID[i]);
			else
				ids.emplace_back(-1);
		}

		vector<vector<gisINT64>> batchIDToIds;
		batchIDToIds.emplace_back(ids);
		Ci_ModelTidFile tid;
		tid.To(batchIDToIds, tidByteArray);
	}

	return WriteData(m_pCacheStorage, m_relativePath, contentName, &gltf, pAttribute, tidByteArray.size() > 0 ? &tidByteArray : NULL, i_GetCacheType(), attInContent, ContentType::M3DGLB, geoCompressType != GeoCompressType::None ? 0 : 6);
}

gisLONG Ci_G3DM3D21CacheTileImpl::ReadBatched3DModel(TileContentInfo info, G3DModel* pModel, Attribute * pAttribute)
{
	if (!i_IsValid() || info.contentType != ContentType::M3DGLB)
		return 0;
	if (pModel != NULL && pModel->GetGeometryType() != info.geometryType)
		return 0;

	CGString contentPath = MakePathByRelativePath(m_relativePath, info.contentURI);
	CGByteArray outInfo;
	m_pCacheStorage->GetContent(contentPath, outInfo);

	if (pModel != NULL)
	{
		CGByteArray geoArray;
		CGString geoUri = info.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelGltf gltf;
			((Ci_TileModel*)&gltf)->From(geoArray);
			gltf.To(pModel);
		}
	}

	if (!info.tidURI.IsEmpty())
	{
		CGByteArray tidArray;
		CGString tidUri = info.tidURI;
		while (tidUri.StartsWith("./"))
		{
			tidUri = tidUri.Right(tidUri.GetLength() - 2);
		}
		vector<vector<gisINT64>> tidVer;
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, tidUri.CStr(), tidArray);
		if (tidArray.size() > 0)
		{
			Ci_ModelTidFile tid;
			tid.From(tidArray, tidVer);
			if (tidVer.size() == 1 && tidVer[0].size() > 0)
			{
				UpdateId(pModel, tidVer[0]);
			}
		}
	}

	if (pAttribute != NULL)
	{
		CGByteArray attArray;
		CGString attUri = info.attributeURI;
		while (attUri.StartsWith("./"))
		{
			attUri = attUri.Right(attUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
		if (attArray.size() > 0)
		{
			Ci_ModelAttributeAttFile  attributeSerialization;
			attributeSerialization.From(attArray, *pAttribute);
		}
		else
		{
			if (attUri.StartsWith("../"))
			{
				attUri = attUri.Right(attUri.GetLength() - 3);
			}
			CGString attPath = MakePathByRelativePath(m_relativePath, attUri.CStr());
			m_pCacheStorage->GetContent(attPath, attArray);
			if (attArray.size() > 0)
			{
				Ci_ModelAttributeAttFile  attributeSerialization;
				attributeSerialization.From(attArray, *pAttribute);
			}
		}
		if (pAttribute->layers.size() <= 0)
        {
			Ci_3DModelAttributeSqliteTool* pModelAttributeSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
			if (pModelAttributeSqliteTool != NULL)
			{
				Attribute* pReadAttribute = pModelAttributeSqliteTool->GetAttribute();
				if (pReadAttribute != NULL && pReadAttribute->layers.size() > 0)
				{
					GetDataAttributeByAllAttribute(pModel, NULL, pAttribute, pReadAttribute);
				}
			}
        }
	}
	return 1;
}

gisLONG Ci_G3DM3D21CacheTileImpl::WriteInstanced3DModel(CGString contentName, G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, Attribute *pAttribute, bool attInContent, GeoCompressType  geoCompressType, DataType dataType)
{
	if (!i_IsValid() || pModel == NULL || pInstances == NULL || pInstances->size() <= 0)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	GeometryType  geoType = pModel->GetGeometryType();

	Ci_ModelI3dm i3dm;
	vector<gisINT64> batchIDToId;
	i3dm.From(pModel, *pInstances, geoCompressType, NULL,true, WriteIdType::batchID, batchIDToId);

	bool hasAtt = false;
	if (pAttribute != NULL && pAttribute->layers.size() >= 1)
	{
		int num = 0;
		for (vector<LayerAttribute>::iterator itr = pAttribute->layers.begin(); itr != pAttribute->layers.end(); itr++)
		{
			if (itr->layerInfo.fieldInfos.size() > 0)
				num += itr->records.size();
		}
		if (num > 0)
			hasAtt = true;
	}
	Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
	m_contentType = ContentType::M3DI3DM;
	i_SetTileContentInfo(contentName, dataType, geoType, geoCompressType, i_GetCacheType(), batchIDToId.size() > 0, hasAtt &&pSqliteTool == NULL, attInContent, ContentType::M3DI3DM);

	Ci_ModelTidFile tid;
	vector<vector<gisINT64>> batchIDToIds;
	batchIDToIds.emplace_back(batchIDToId);
	CGByteArray tidByteArray;
	tid.To(batchIDToIds, tidByteArray);

	return WriteData(m_pCacheStorage, m_relativePath, contentName, &i3dm, pAttribute, tidByteArray.size() > 0 ? &tidByteArray : NULL, i_GetCacheType(), attInContent, ContentType::M3DPNTS, geoCompressType != GeoCompressType::None ? 0 : 6);
}

gisLONG Ci_G3DM3D21CacheTileImpl::ReadInstanced3DModel(TileContentInfo info, G3DModel* pModel, vector<MapGIS::Tile::ModelInstance>* pInstances, Attribute * pAttribute)
{
	if (!i_IsValid() || pInstances == NULL || info.contentType != ContentType::M3DI3DM)
		return 0;
	if (pModel != NULL && pModel->GetGeometryType() != info.geometryType)
		return 0;

	CGString contentPath = MakePathByRelativePath(m_relativePath, info.contentURI);
	CGByteArray outInfo;
	m_pCacheStorage->GetContent(contentPath, outInfo);

	if (pModel != NULL)
	{
		CGByteArray geoArray;
		CGString geoUri = info.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelI3dm gltf;
			((Ci_TileModel*)&gltf)->From(geoArray);
			LayerAttribute records;
			D_3DOT rtcCenter;
			gltf.To(pModel, *pInstances, records, rtcCenter);
		}
	}

	if (!info.tidURI.IsEmpty())
	{
		CGByteArray tidArray;
		CGString tidUri = info.tidURI;
		while (tidUri.StartsWith("./"))
		{
			tidUri = tidUri.Right(tidUri.GetLength() - 2);
		}
		vector<vector<gisINT64>> tidVer;
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, tidUri.CStr(), tidArray);
		if (tidArray.size() > 0)
		{
			Ci_ModelTidFile tid;
			tid.From(tidArray, tidVer);
			if (tidVer.size() == 1 && tidVer[0].size() > 0)
			{
				UpdateId(pInstances, tidVer[0]);
			}
		}
	}

	if (pAttribute != NULL)
	{
		CGByteArray attArray;
		CGString attUri = info.attributeURI;
		while (attUri.StartsWith("./"))
		{
			attUri = attUri.Right(attUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
		if (attArray.size() > 0)
		{
			Ci_ModelAttributeAttFile  attributeSerialization;
			attributeSerialization.From(attArray, *pAttribute);
		}
		else
		{
			if (attUri.StartsWith("../"))
			{
				attUri = attUri.Right(attUri.GetLength() - 3);
			}
			CGString attPath = MakePathByRelativePath(m_relativePath, attUri.CStr());
			m_pCacheStorage->GetContent(attPath, attArray);
			if (attArray.size() > 0)
			{
				Ci_ModelAttributeAttFile  attributeSerialization;
				attributeSerialization.From(attArray, *pAttribute);
			}
		}
		if (pAttribute->layers.size() <= 0)
		{
			Ci_3DModelAttributeSqliteTool* pModelAttributeSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
			if (pModelAttributeSqliteTool != NULL)
			{
				Attribute* pReadAttribute = pModelAttributeSqliteTool->GetAttribute();
				if (pReadAttribute != NULL && pReadAttribute->layers.size() > 0)
				{
					GetDataAttributeByAllAttribute(pModel, pInstances, pAttribute, pReadAttribute);
				}
			}
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D21CacheTileImpl::WritePoints(CGString contentName, PointsModel* pModel, Attribute *pAttribute, bool attInContent, GeoCompressType  geoCompressType, DataType dataType)
{
	if (!i_IsValid() || pModel == NULL)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	bool hasAtt = false;
	if (pAttribute != NULL && pAttribute->layers.size() == 1)
	{
		int num = 0;
		for (vector<LayerAttribute>::iterator itr = pAttribute->layers.begin(); itr != pAttribute->layers.end(); itr++)
		{
			if (itr->layerInfo.fieldInfos.size() > 0)
				num += itr->records.size();
		}
		if (num > 0)
			hasAtt = true;
	}
	if (geoCompressType != GeoCompressType::Draco)
		geoCompressType = GeoCompressType::None;
	m_contentType = ContentType::M3DPNTS;
	Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
	i_SetTileContentInfo(contentName, dataType, GeometryType::Point, geoCompressType, i_GetCacheType(), false, hasAtt &&pSqliteTool ==NULL, attInContent, ContentType::M3DPNTS);
	Ci_ModelPnts pnts;
	vector<gisINT64> batchIDToId;
	pnts.From(*pModel, geoCompressType == GeoCompressType::Draco, NULL, batchIDToId);
	if (pSqliteTool !=NULL)
	{
		Ci_ModelTidFile tid;
		vector<vector<gisINT64>> batchIDToIds;
		batchIDToIds.emplace_back(batchIDToId);
		CGByteArray tidByteArray;
		tid.To(batchIDToIds, tidByteArray);
		return WriteData(m_pCacheStorage, m_relativePath, contentName, &pnts, pAttribute, tidByteArray.size() > 0 ? &tidByteArray : NULL, i_GetCacheType(), attInContent, ContentType::M3DPNTS, geoCompressType != GeoCompressType::None ? 0 : 6);
	}
	else
	{
		CGByteArray outInfo;
		if (hasAtt)
		{
			Ci_ModelAttributeAttFile  attributeSerialization;
			CGByteArray byteArray;
			if (attributeSerialization.To(pAttribute->layers[0], byteArray, batchIDToId, AttributeCompressType::PropCompressTypeGZip) > 0 && byteArray.size() > 0)
			{
				if (attInContent)
				{
					Ci_ZipTool::Buffer2ZipBuffer(*pnts.Get(), "geometry/0.pnts", byteArray, "attribute/0.att", CGByteArray(), "", outInfo);
				}
				else
				{
					CGString attPath = MakePathByRelativePath(m_relativePath, contentName + ".att");
					m_pCacheStorage->SetContent(attPath, byteArray);
				}
			}
		}
		if (outInfo.isEmpty())
			Ci_ZipTool::Buffer2ZipBuffer(*pnts.Get(), "geometry/0.pnts", CGByteArray(), "", CGByteArray(), "", outInfo);
		CGString contentPath = MakePathByRelativePath(m_relativePath, contentName + ".m3d");
		return m_pCacheStorage->SetContent(contentPath, outInfo);
	}
}

gisLONG Ci_G3DM3D21CacheTileImpl::ReadPoints(TileContentInfo info, PointsModel* pModel, Attribute * pAttribute)
{
	if (!i_IsValid() || info.contentType != ContentType::M3DPNTS || info.geometryType != GeometryType::Point)
		return 0;
	if (pModel != NULL && pModel->GetGeometryType() != info.geometryType)
		return 0;

	CGString contentPath = MakePathByRelativePath(m_relativePath, info.contentURI);
	CGByteArray outInfo;
	m_pCacheStorage->GetContent(contentPath, outInfo);

	if (pModel != NULL)
	{
		CGByteArray geoArray;
		CGString geoUri = info.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelPnts pnts;
			((Ci_TileModel*)&pnts)->From(geoArray);
			LayerAttribute records;
			D_3DOT rtcCenter;
			pnts.To(*pModel, records, rtcCenter);
		}
	}

	if (!info.tidURI.IsEmpty())
	{
		CGByteArray tidArray;
		CGString tidUri = info.tidURI;
		while (tidUri.StartsWith("./"))
		{
			tidUri = tidUri.Right(tidUri.GetLength() - 2);
		}
		vector<vector<gisINT64>> tidVer;
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, tidUri.CStr(), tidArray);
		if (tidArray.size() > 0)
		{
			Ci_ModelTidFile tid;
			tid.From(tidArray, tidVer);
			if (tidVer.size() == 1 && tidVer[0].size() > 0)
			{
				UpdateId(pModel, tidVer[0]);
			}
		}
	}

	if (pAttribute != NULL)
	{
		CGByteArray attArray;
		CGString attUri = info.attributeURI;
		while (attUri.StartsWith("./"))
		{
			attUri = attUri.Right(attUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
		if (attArray.size() <= 0)
		{
			if (attUri.StartsWith("../"))
			{
				attUri = attUri.Right(attUri.GetLength() - 3);
			}
			CGString attPath = MakePathByRelativePath(m_relativePath, attUri.CStr());
			m_pCacheStorage->GetContent(attPath, attArray);
		}

		if (attArray.size() > 0)
		{
			Ci_ModelAttributeAttFile  attributeSerialization;
			attributeSerialization.From(attArray, *pAttribute);
		}
		if (pAttribute->layers.size() <= 0)
		{
			Ci_3DModelAttributeSqliteTool* pModelAttributeSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
			if (pModelAttributeSqliteTool != NULL)
			{
				Attribute* pReadAttribute = pModelAttributeSqliteTool->GetAttribute();
				if (pReadAttribute != NULL && pReadAttribute->layers.size() > 0)
				{
					GetDataAttributeByAllAttribute(pModel, NULL, pAttribute, pReadAttribute);
				}
			}
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D21CacheTileImpl::WriteComposite(CGString contentName, vector<ContentBase*>* pContents, bool attInContent, GeoCompressType  geoCompressType, DataType dataType)
{
	if (!i_IsValid() || pContents == NULL || pContents->size() <= 0)
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	GeometryType  geoType = GeometryType::None;
	Attribute attribute;
	vector<CGByteArray> modelArrays;
	vector<vector<gisINT64>> batchIDToIds;
	for (vector<ContentBase*>::iterator itr = pContents->begin(); itr != pContents->end(); itr++)
	{
		if (*itr == NULL)
			continue;
		GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
		if (pGeoContent == NULL)
			continue;
		vector<ModelInstance>* pInstances = pGeoContent->GetModelInstance();
		Attribute* pAttribute = pGeoContent->GetAttribute();
		G3DModel* p3DModel = pGeoContent->Get3DModel();
		if (p3DModel == NULL)
			continue;

		if (pInstances ==NULL ||  pInstances->size() <= 0)
		{
			if (p3DModel->GetGeometryType() == GeometryType::Point)
			{
				Ci_ModelPnts modelPnts;
				vector<gisINT64> batchIDToId;
				modelPnts.From(*(MapGIS::Tile::PointsModel*)p3DModel, geoCompressType == GeoCompressType::Draco, NULL, batchIDToId);
				modelArrays.push_back(*modelPnts.Get());
				if (pAttribute != NULL)
					ImportAttribute(*pAttribute, attribute);
				geoType = p3DModel->GetGeometryType();
				batchIDToIds.emplace_back(batchIDToId);
			}
			else
			{
				Ci_ModelB3dm modelB3dm;
				vector<gisINT64> batchIDToId;
				modelB3dm.From(p3DModel, geoCompressType, NULL, true, batchIDToId);
				modelArrays.push_back(*modelB3dm.Get());
				if (pAttribute != NULL)
					ImportAttribute(*pAttribute, attribute);
				geoType = p3DModel->GetGeometryType();
				batchIDToIds.emplace_back(batchIDToId);
			}
		}
		else
		{
			Ci_ModelI3dm modelI3dm;
			vector<gisINT64> batchIDToId;
			modelI3dm.From(p3DModel, *pInstances, geoCompressType, NULL, true, WriteIdType::batchID, batchIDToId);
			modelArrays.push_back(*modelI3dm.Get());
			if (pAttribute != NULL)
				ImportAttribute(*pAttribute, attribute);
			geoType = p3DModel->GetGeometryType();
			batchIDToIds.emplace_back(batchIDToId);
		}
	}

	Ci_ModelTidFile tid;
	CGByteArray tidByteArray;
	tid.To(batchIDToIds, tidByteArray);

	bool hasAtt = false;
	if (attribute.layers.size() >= 1)
	{
		int num = 0;
		for (vector<LayerAttribute>::iterator itr = attribute.layers.begin(); itr != attribute.layers.end(); itr++)
		{
			if (itr->layerInfo.fieldInfos.size() > 0)
				num += itr->records.size();
		}
		if (num > 0)
			hasAtt = true;
	}
	Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
	i_SetTileContentInfo(contentName, dataType, geoType, geoCompressType, i_GetCacheType(), tidByteArray.size() > 0, hasAtt &&pSqliteTool == NULL, attInContent, ContentType::M3DCMPT);
	m_contentType = ContentType::M3DCMPT;
	Ci_ModelCmpt cmpt;
	cmpt.From(modelArrays);
	return WriteData(m_pCacheStorage, m_relativePath, contentName, &cmpt, &attribute, tidByteArray.size() > 0 ? &tidByteArray : NULL, i_GetCacheType(), attInContent, ContentType::M3DCMPT, geoCompressType != GeoCompressType::None ? 0 : 6);
}

gisLONG Ci_G3DM3D21CacheTileImpl::ReadComposite(TileContentInfo info, vector<ContentBase*>* pContents)
{
	if (!i_IsValid() || pContents == NULL ||  info.contentType != ContentType::M3DCMPT)
		return 0;

	CGString contentPath = MakePathByRelativePath(m_relativePath, info.contentURI);
	CGByteArray outInfo;
	m_pCacheStorage->GetContent(contentPath, outInfo);

	CGByteArray geoArray;
	CGString geoUri = info.geometryURI;
	while (geoUri.StartsWith("./"))
	{
		geoUri = geoUri.Right(geoUri.GetLength() - 2);
	}
	Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
	if (geoArray.size() > 0)
	{
		Ci_ModelCmpt cmpt;
		((Ci_TileModel*)&cmpt)->From(geoArray);
		cmpt.To(pContents);
	}

	if (!info.tidURI.IsEmpty())
	{
		CGByteArray tidArray;
		CGString tidUri = info.tidURI;
		while (tidUri.StartsWith("./"))
		{
			tidUri = tidUri.Right(tidUri.GetLength() - 2);
		}
		vector<vector<gisINT64>> tidVer;
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, tidUri.CStr(), tidArray);

		Ci_ModelTidFile tid;
		tid.From(tidArray, tidVer);
		if (tidVer.size() == pContents->size())
		{
			for (int i = 0; i < pContents->size(); i++)
			{
				if (pContents->at(i) != NULL && dynamic_cast<GeometryContent*>(pContents->at(i)) !=NULL )
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

	Attribute attribute;
	CGByteArray attArray;

	CGString attUri = info.attributeURI;
	while (attUri.StartsWith("./"))
	{
		attUri = attUri.Right(attUri.GetLength() - 2);
	}
	Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
	if (attArray.size() > 0)
	{
		Ci_ModelAttributeAttFile  attributeSerialization;
		attributeSerialization.From(attArray, attribute);
	}
	else
	{
		if (attUri.StartsWith("../"))
		{
			attUri = attUri.Right(attUri.GetLength() - 3);
		}
		CGString attPath = MakePathByRelativePath(m_relativePath, attUri.CStr());
		m_pCacheStorage->GetContent(attPath, attArray);
		if (attArray.size() > 0)
		{
			Ci_ModelAttributeAttFile  attributeSerialization;
			attributeSerialization.From(attArray, attribute);
		}
	}

	if (attribute.layers.size() <= 0)
	{
		Ci_3DModelAttributeSqliteTool* pModelAttributeSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
		if (pModelAttributeSqliteTool != NULL)
		{
			Attribute* pReadAttribute = pModelAttributeSqliteTool->GetAttribute();
			attribute = *pReadAttribute;
		}
	}
	if (attribute.layers.size() > 0)
		GetDataAttributeByAllAttribute(pContents, &attribute);
	return 1;
}

gisLONG Ci_G3DM3D21CacheTileImpl::WriteVoxelMode(CGString contentName, MapGIS::Tile::VoxelContent* pVoxelContent)
{
	if (!i_IsValid() || NULL == pVoxelContent || pVoxelContent->GetAttribute() == NULL)
		return 0;
	MapGIS::Tile::VoxelModel* pModel =  pVoxelContent->Get3DModel();
	if (pModel == NULL)
		return 0;
	vector<MapGIS::Tile::Attribute> * pAtts =  pVoxelContent->GetAttribute();

	m_contentType = ContentType::M3DVoxel;
	if (contentName.IsEmpty())
		contentName = GetName();

	this->m_tile.tileDataInfoList.insert(this->m_tile.tileDataInfoList.begin(), Ci_Content());
	this->m_tile.tileDataInfoIndex = 0;
	this->m_tile.tileDataInfoList[0].dataType = DataType::Voxel;
	this->m_tile.tileDataInfoList[0].geometry_geometryType = GeometryType::None;
	this->m_tile.tileDataInfoList[0].geometry_geoCompressType = GeoCompressType::None;
	this->m_tile.tileDataInfoList[0].attribute_attType = AttFileType::ATT;
	this->m_tile.tileDataInfoList[0].attribute_uri = "../" + contentName + "_time{i}.att";
	this->m_tile.tileDataInfoList[0].mVoxelModel = *pModel;

	for (int i = 0; i < pAtts->size(); ++i)
	{;
		Ci_ModelAttributeAttFile  attSerial;
		CGByteArray byteArray;
		if (attSerial.To(pAtts->at(i), byteArray, AttributeCompressType::PropCompressTypeGZip) > 0 && byteArray.size() > 0)
		{
			CGString attPath = MakePathByRelativePath(m_relativePath, contentName + "_time" + CGString::From(i) + ".att");
			m_pCacheStorage->SetContent(attPath, byteArray);
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D21CacheTileImpl::ReadVoxelMode(MapGIS::Tile::TileContentInfo info, MapGIS::Tile::VoxelContent* pVoxelContent)
{
	if (m_contentType != ContentType::M3DVoxel)
		return 0;
	if (pVoxelContent == NULL)
		return 0;

	//model
	if (m_tile.tileDataInfoList.empty())
		return 0;
	if (DataType::Voxel != m_tile.tileDataInfoList.front().dataType)
		return 0;
	MapGIS::Tile::VoxelModel* pModel =pVoxelContent->Get3DModel();
	if (pModel != NULL)
	{
		*pModel = m_tile.tileDataInfoList.front().mVoxelModel;
	}

	//att

	vector<MapGIS::Tile::Attribute> *pAtt =  pVoxelContent->GetAttribute();
	if (pAtt != NULL)
	{
		CGString attUri = info.attributeURI;
		if (attUri.StartsWith("../"))
		{
			attUri = attUri.Right(attUri.GetLength() - 3);
		}
		CGString attPath = MakePathByRelativePath(m_relativePath, attUri.CStr());

		int nTimeNode = m_tile.tileDataInfoList.front().mVoxelModel.GetTimeNodeCount();
		for (int i = 0; i < nTimeNode; ++i)
		{
			string strIndex = to_string(i);
			CGString tmpAttPath = attPath;
			tmpAttPath.Replace("{i}", strIndex.c_str());
			CGByteArray tmpAtt;
			m_pCacheStorage->GetContent(tmpAttPath, tmpAtt);
			if (tmpAtt.isEmpty())
				break;
			Attribute attribute;
			Ci_ModelAttributeAttFile  attributeSerialization;
			attributeSerialization.From(tmpAtt, attribute);
			pAtt->push_back(attribute);
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D21CacheTileImpl::WriteContent(CGString contentName, vector<MapGIS::Tile::ContentBase*>* pContents, MapGIS::Tile::WriteContentParamBase& param )
{
	if (pContents == NULL)
		return 0;
	WriteGeometryContentParam* pContentParam = dynamic_cast<WriteGeometryContentParam*>(&param);
	if (pContents->size() > 1) 
	{
		if (pContentParam == NULL)
			return 0;
		return WriteComposite(contentName, pContents, pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
	}
	
	MapGIS::Tile::ContentBase* item = pContents->at(0);
	GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(item);
	if (pGeoContent != NULL)
	{
		if (pContentParam == NULL)
			return 0;

		G3DModel*  pModel = pGeoContent->Get3DModel();
		vector<ModelInstance>* pInstance = pGeoContent->GetModelInstance();
		if (pModel == NULL)
			return 0;
		if (pInstance != NULL &&pInstance->size() > 0)
			return WriteInstanced3DModel(contentName, pModel, pInstance, pGeoContent->GetAttribute(), pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
		if (pModel->GetGeometryType() == GeometryType::Point)
			return WritePoints(contentName, (PointsModel*)pModel, pGeoContent->GetAttribute(), pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
		return WriteBatched3DModel(contentName, (PointsModel*)pModel, pGeoContent->GetAttribute(), pContentParam->IsAttInContent(), pContentParam->GetGeoCompressType(), pContentParam->GetDataType());
	}
	else
	{
		VoxelContent*  pVoxelContent = dynamic_cast<VoxelContent*>(item);
		if (pVoxelContent != NULL)
		{
			return WriteVoxelMode(contentName, pVoxelContent);
		}
		return 0;
	}
}

gisLONG Ci_G3DM3D21CacheTileImpl::ReadContent(vector<ContentBase*>* pContents)
{
	if (pContents == NULL)
		return 0;
	TileContentInfo tileContentInfo = GetContentInfo();
	gisLONG rtn = 0;
	switch (tileContentInfo.contentType)
	{
	case ContentType::M3DGLB:
	{
		if (!i_IsValid() )
			return 0;
		CGString contentPath = MakePathByRelativePath(m_relativePath, tileContentInfo.contentURI);
		CGByteArray outInfo;
		m_pCacheStorage->GetContent(contentPath, outInfo);

		CGByteArray geoArray;
		CGString geoUri = tileContentInfo.geometryURI;
		while (geoUri.StartsWith("./"))
		{
			geoUri = geoUri.Right(geoUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, geoUri.CStr(), geoArray);
		if (geoArray.size() > 0)
		{
			Ci_ModelGltf gltf;
			((Ci_TileModel*)&gltf)->From(geoArray);
			gltf.To(pContents);
		}

		if (!tileContentInfo.tidURI.IsEmpty())
		{
			CGByteArray tidArray;
			CGString tidUri = tileContentInfo.tidURI;
			while (tidUri.StartsWith("./"))
			{
				tidUri = tidUri.Right(tidUri.GetLength() - 2);
			}
			vector<vector<gisINT64>> tidVer;
			Ci_ZipTool::ZipBuffer2Buffer(outInfo, tidUri.CStr(), tidArray);
			if (tidArray.size() > 0)
			{
				Ci_ModelTidFile tid;
				tid.From(tidArray, tidVer);
				if (tidVer.size() == 1 && tidVer[0].size() > 0)
				{
					for (vector<ContentBase*>::iterator itr = pContents->begin(); itr != pContents->end(); itr++)
					{
						GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
						if (pGeoContent == NULL)
							continue;
						G3DModel*  pModel = pGeoContent->Get3DModel();
						vector<ModelInstance>* pInstance = pGeoContent->GetModelInstance();
						if (pInstance != NULL &&pInstance->size() > 0)
							UpdateId(pInstance, tidVer[0]);
						else
							UpdateId(pModel, tidVer[0]);
					}
				}
			}
		}

		Attribute  outAttribute;
		CGByteArray attArray;
		CGString attUri = tileContentInfo.attributeURI;
		while (attUri.StartsWith("./"))
		{
			attUri = attUri.Right(attUri.GetLength() - 2);
		}
		Ci_ZipTool::ZipBuffer2Buffer(outInfo, attUri.CStr(), attArray);
		if (attArray.size() > 0)
		{
			Ci_ModelAttributeAttFile  attributeSerialization;
			attributeSerialization.From(attArray, outAttribute);
		}
		else
		{
			if (attUri.StartsWith("../"))
			{
				attUri = attUri.Right(attUri.GetLength() - 3);
			}
			CGString attPath = MakePathByRelativePath(m_relativePath, attUri.CStr());
			m_pCacheStorage->GetContent(attPath, attArray);
			if (attArray.size() > 0)
			{
				Ci_ModelAttributeAttFile  attributeSerialization;
				attributeSerialization.From(attArray, outAttribute);
			}
		}
		if (outAttribute.layers.size() <= 0)
		{
			Ci_3DModelAttributeSqliteTool* pModelAttributeSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
			if (pModelAttributeSqliteTool != NULL)
			{
				Attribute* pReadAttribute = pModelAttributeSqliteTool->GetAttribute();
				outAttribute = *pReadAttribute;
			}
		}

		if (outAttribute.layers.size() > 0)
			GetDataAttributeByAllAttribute(pContents, &outAttribute);
		return 1;
	}
	break;
	case ContentType::M3DI3DM:
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Surface, true, true);
		rtn = ReadInstanced3DModel(tileContentInfo, (PointsModel*)pItem->Get3DModel(), pItem->GetModelInstance(), pItem->GetAttribute());
		pContents->emplace_back(pItem);
	}

	break;
	case ContentType::M3DPNTS:
	{
		GeometryContent* pItem = new GeometryContent();
		pItem->CreateData(true, GeometryType::Point, true, false);
		rtn = ReadPoints(tileContentInfo, (PointsModel*)pItem->Get3DModel(), pItem->GetAttribute());
		pContents->emplace_back(pItem);
	}
	break;
	case ContentType::M3DCMPT:
		rtn = ReadComposite(tileContentInfo, pContents);
		break;

	case ContentType::M3DVoxel:
	{
		VoxelContent* pItem = new VoxelContent();
		pItem->CreateData(true, true);
		rtn = ReadVoxelMode(tileContentInfo, pItem);
		pContents->emplace_back(pItem);
	}

		break;
	case ContentType::B3DM:
	case ContentType::I3DM:
	case ContentType::PNTS:
	case ContentType::CMPT:
	case ContentType::None:
	default:
		break;
	}
	return rtn;
}

G3DTile* Ci_G3DM3D21CacheTileImpl::Open(G3DCacheStorage* pCacheStorage, CGString relativePath)
{
	if (pCacheStorage == NULL || relativePath.IsEmpty())
		return NULL;
	CGByteArray value;
	if (pCacheStorage->GetContent(relativePath, value) <= 0 || value.isEmpty())
		return NULL;

	rapidjson::Document doc;
	if (doc.Parse(value.data(), value.size()).HasParseError())
		return NULL;
	Ci_G3DM3D21CacheTileImpl* rtn = new Ci_G3DM3D21CacheTileImpl(pCacheStorage);
	rtn->m_relativePath = relativePath;
	if (!doc.IsObject() || rtn->m_tile.From(doc) <= 0)
	{
		delete rtn;
		return NULL;
	}
	return rtn;
}

Ci_G3DM3D22CacheTileImpl::Ci_G3DM3D22CacheTileImpl(G3DCacheStorage* pCacheStorage) : Ci_G3DM3D21CacheTileImpl(pCacheStorage)
{
	//M3D 2.1 不设置 lodMode
	m_tile.lodMode = LodMode::None;
}

Ci_G3DM3D22CacheTileImpl::~Ci_G3DM3D22CacheTileImpl()
{
}

Ci_G3DM3DCacheTileImpl* Ci_G3DM3D22CacheTileImpl::i_CreateChildInstance()
{
	Ci_G3DM3D22CacheTileImpl *rtn = new Ci_G3DM3D22CacheTileImpl(m_pCacheStorage);
	return rtn;
}

G3DCacheType Ci_G3DM3D22CacheTileImpl::i_GetCacheType()
{
	return G3DCacheType::TypeM3DV22;
}

gisLONG Ci_G3DM3D22CacheTileImpl::WriteContent(CGString contentName, vector<MapGIS::Tile::ContentBase*>* pContents, MapGIS::Tile::WriteContentParamBase& param)
{
	if (pContents == NULL)
		return 0;
	if (pContents == NULL || !i_IsValid())
		return 0;
	if (contentName.IsEmpty())
		contentName = GetName();
	if (pContents->size() == 1 && dynamic_cast<VoxelContent*>(pContents->at(0)) != NULL)
	{
		VoxelContent*  pVoxelContent = dynamic_cast<VoxelContent*>(pContents->at(0));
		if (pVoxelContent != NULL)
		{
			return WriteVoxelMode(contentName, pVoxelContent);
		}
		return 0;
	}
	else if (pContents->size() == 1 && dynamic_cast<GaussianContent*>(pContents->at(0)) != NULL)
	{
		WriteGaussianContentParam* pContentParam = dynamic_cast<WriteGaussianContentParam*>(&param);
		GaussianContent*  pGaussianContent = dynamic_cast<GaussianContent*>(pContents->at(0));
		GaussianExtMode mode = GaussianExtMode::KHRGaussianSplattingCompressionSpz2;
		if (pContentParam != NULL)
			mode = pContentParam->GetGaussianExtMode();

		Ci_ModelGltf gltf;
		map<gisINT64, gisINT64> resetIDMap;
		GeometryType  geoType = GeometryType::Point;
		gltf.FromGaussian(pGaussianContent, mode);
		i_SetTileContentInfo(contentName, pContentParam->GetDataType(), geoType, GeoCompressType::None, i_GetCacheType(),false,false, false, ContentType::M3DGLB);

		return  WriteData(m_pCacheStorage, m_relativePath, contentName, &gltf, NULL, NULL, i_GetCacheType(), false, ContentType::M3DGLB, 6);
	}
	else
	{
		WriteGeometryContentParam* pContentParam = dynamic_cast<WriteGeometryContentParam*>(&param);
		if (pContentParam == NULL)
			return 0;
		GeometryType  geoType = GeometryType::Surface;
		bool hasAtt = false;
		int num = 0;
		for (vector<ContentBase*> ::iterator itr = pContents->begin(); itr != pContents->end(); itr++)
		{
			GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
			if (pGeoContent == NULL)
				continue;
			Attribute* pAttribute = pGeoContent->GetAttribute();
			G3DModel* pModel = pGeoContent->Get3DModel();
			if (pModel == NULL)
				continue;
			geoType = pModel->GetGeometryType();

			if (pAttribute != NULL && pAttribute->layers.size() >= 1)
			{
				for (vector<LayerAttribute>::iterator layerItr = pAttribute->layers.begin(); layerItr != pAttribute->layers.end(); layerItr++)
				{
					if (layerItr->layerInfo.fieldInfos.size() > 0)
						num += layerItr->records.size();
				}
			}
		}
		if (num > 0)
			hasAtt = true;
		Ci_ModelGltf gltf;
		map<gisINT64, gisINT64> resetIDMap;
		gltf.FromResetID(pContents, pContentParam->GetGeoCompressType(), WriteIdType::batchID, resetIDMap);

		Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);

		if (pSqliteTool == NULL && pContents->size() == 1 && dynamic_cast<GeometryContent*>(pContents->at(0)) != NULL &&   dynamic_cast<GeometryContent*>(pContents->at(0))->GetGeometryType() == GeometryType::Point)
		{
			GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(pContents->at(0));

			Ci_ModelGltf gltf;
			map<gisINT64, gisINT64> resetIDMap;
			gltf.FromResetID(pContents, pContentParam->GetGeoCompressType(), WriteIdType::batchID, resetIDMap);
			CGByteArray outInfo;
			if (hasAtt)
			{
				unordered_map<gisINT64, gisINT64> batchIDToID;
				for (map<gisINT64, gisINT64>::iterator itr = resetIDMap.begin(); itr != resetIDMap.end(); itr++)
				{
					batchIDToID.insert(make_pair(itr->second, itr->first));
				}
				vector<gisINT64> ids;
				for (int i = 0; i < batchIDToID.size(); i++)
				{
					if (batchIDToID.find(i) != batchIDToID.end())
						ids.emplace_back(batchIDToID[i]);
					else
						ids.emplace_back(-1);
				}
				Attribute* pAttribute = pGeoContent->GetAttribute();
				Ci_ModelAttributeAttFile  attributeSerialization;
				CGByteArray byteArray;
				if (attributeSerialization.To(pAttribute->layers[0], byteArray, ids, AttributeCompressType::PropCompressTypeGZip) > 0 && byteArray.size() > 0)
				{
					if (pContentParam->IsAttInContent())
					{
						Ci_ZipTool::Buffer2ZipBuffer(*gltf.Get(), "geometry/0.glb", byteArray, "attribute/0.att", CGByteArray(), "", outInfo);
					}
					else
					{
						CGString attPath = MakePathByRelativePath(m_relativePath, contentName + ".att");
						m_pCacheStorage->SetContent(attPath, byteArray);
					}
				}
			}
			i_SetTileContentInfo(contentName, pContentParam->GetDataType(), GeometryType::Point, pContentParam->GetGeoCompressType(), i_GetCacheType(), false, hasAtt, pContentParam->IsAttInContent(), ContentType::M3DGLB);
			if (outInfo.isEmpty())
				Ci_ZipTool::Buffer2ZipBuffer(*gltf.Get(), "geometry/0.glb", CGByteArray(), "", CGByteArray(), "", outInfo);
			CGString contentPath = MakePathByRelativePath(m_relativePath, contentName + ".m3d");
			return m_pCacheStorage->SetContent(contentPath, outInfo);
		}
		else
		{
			m_contentType = ContentType::M3DGLB;
			i_SetTileContentInfo(contentName, pContentParam->GetDataType(), geoType, pContentParam->GetGeoCompressType(), i_GetCacheType(), resetIDMap.size() > 0, hasAtt && pSqliteTool == NULL, pContentParam->IsAttInContent(), ContentType::M3DGLB);

			Attribute attribute;
			for (vector<ContentBase*>::iterator itr = pContents->begin(); itr != pContents->end(); itr++)
			{
				if (*itr == NULL)
					continue;
				GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
				if (pGeoContent == NULL)
					continue;
				Attribute* pAttribute = pGeoContent->GetAttribute();
				if (pAttribute != NULL)
					ImportAttribute(*pAttribute, attribute);
			}
			CGByteArray tidByteArray;
			if (resetIDMap.size() > 0)
			{
				unordered_map<gisINT64, gisINT64> batchIDToID;
				for (map<gisINT64, gisINT64>::iterator itr = resetIDMap.begin(); itr != resetIDMap.end(); itr++)
				{
					batchIDToID.insert(make_pair(itr->second, itr->first));
				}
				vector<gisINT64> ids;
				for (int i = 0; i < batchIDToID.size(); i++)
				{
					if (batchIDToID.find(i) != batchIDToID.end())
						ids.emplace_back(batchIDToID[i]);
					else
						ids.emplace_back(-1);
				}
				vector<vector<gisINT64>> batchIDToIds;
				batchIDToIds.emplace_back(ids);
				Ci_ModelTidFile tid;
				tid.To(batchIDToIds, tidByteArray);
			}
			return  WriteData(m_pCacheStorage, m_relativePath, contentName, &gltf, &attribute, tidByteArray.size() > 0 ? &tidByteArray : NULL, i_GetCacheType(), pContentParam->IsAttInContent(), ContentType::M3DGLB, pContentParam->GetGeoCompressType() != GeoCompressType::None ? 0 : 6);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ci_G3DM3DCacheTilesetImpl::Ci_G3DM3DCacheTilesetImpl()
{
	m_tileset.compressType = FileCompressType::Zip;

	char szGuid[256];
	GetNewGuidToString(szGuid);
	m_tileset.guid = szGuid;
	m_tileset.lodType = RefineType::Replace;
	m_tileset.spatialReference = SpatialReference::WGS84;
	m_tileset.treeType = TreeType::QuadTree;
	m_tileset.version = M3DVersion::M3D20;
	m_tileset.rootNode_uri = "rootNode.json";
	m_pRootTile = NULL;
	m_tilesetName = "";
	m_pCacheStorage = NULL;
}

Ci_G3DM3DCacheTilesetImpl::~Ci_G3DM3DCacheTilesetImpl()
{
	if (m_pRootTile != NULL)
		delete m_pRootTile;
	m_pRootTile = NULL;
	/*Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
	if (pSqliteTool !=NULL  && m_pCacheStorage)
	{
		Ci_3DModelAttributeSqliteTool::Free3DModelAttributeSqlite(pSqliteTool);
	}*/
	m_pCacheStorage = NULL;
}

G3DCacheStorage* Ci_G3DM3DCacheTilesetImpl::GetCacheStorage()
{
	if (IsValid())
		return m_pCacheStorage;
	return NULL;
}

G3DTile* Ci_G3DM3DCacheTilesetImpl::GetRoot()
{
	if (IsValid())
		return (G3DTile*)m_pRootTile;
	return NULL;
}

void Ci_G3DM3DCacheTilesetImpl::SetGUID(GUID guid)
{
	if (IsValid())
	{
		char szGuid[256];
		GUID2String(guid, szGuid);
		m_tileset.guid = szGuid;
	}
}
GUID Ci_G3DM3DCacheTilesetImpl::GetGUID()
{
	GUID rtn;
	if (IsValid() && !m_tileset.guid.IsEmpty())
		String2GUID((char*)m_tileset.guid.CStr(), rtn);
	return rtn;
}

void Ci_G3DM3DCacheTilesetImpl::SetPosition(D_3DOT dot)
{
	if (IsValid())
		m_tileset.position = dot;
}
D_3DOT Ci_G3DM3DCacheTilesetImpl::GetPosition()
{
	if (IsValid())
		return m_tileset.position;
	return D_3DOT{ 0,0,0 };
}

void Ci_G3DM3DCacheTilesetImpl::SetTreeType(TreeType treeType)
{
	if (IsValid())
		m_tileset.treeType = treeType;
}

TreeType Ci_G3DM3DCacheTilesetImpl::GetTreeType()
{
	if (IsValid())
		return m_tileset.treeType;
	return TreeType::QuadTree;
}

CGString Ci_G3DM3DCacheTilesetImpl::i_GetTilesetRelative(G3DCacheStorage *pCacheStorage, CGString tilesetName)
{
	if (pCacheStorage == NULL || !pCacheStorage->IsValid())
		return "";
	MapGIS::Tile::StorageType type = pCacheStorage->GetStorageType();
	if (type != MapGIS::Tile::StorageType::LocalFileType && type != MapGIS::Tile::StorageType::NoneType)
	{
		return M3dTopInfoData;
	}
	else
	{
		if (tilesetName.EndsWith(".mcj", true))
			return tilesetName;
		else
			return tilesetName + ".mcj";
	}
	return "";
}

gisLONG Ci_G3DM3DCacheTilesetImpl::Save()
{
	if (!IsValid())
		return 0;
	m_pRootTile->i_UpdateChildInfo();
	m_tileset.boundingVolume = m_pRootTile->GetBounding();

	auto GetJwdValue = [](MapGIS::Tile::Matrix4D &rootMatrix, MapGIS::Tile::Matrix4D &tilesetMatrix, D_3DOT& dot)
	{
		MapGIS::Tile::Vector4D dot_d = rootMatrix* Vector4D(dot.x, dot.y, dot.z, 1);

		dot_d = tilesetMatrix * MapGIS::Tile::Vector4D(dot_d.x, dot_d.y, dot_d.z, 1);

		return WGS84Ellipsoid::GeographicaRadianFromCartesian(D_3DOT{ dot_d.x, dot_d.y, dot_d.z });
	};

	if (m_tileset.boundingVolume.type != BoundingType::Region && !m_pRootTile->GetMatrix().IsUnit())
	{
		if (m_tileset.boundingVolume.type == BoundingType::Box)
		{
			MapGIS::Tile::Matrix4D tilesetMatrix = MapGIS::Tile::WGS84Ellipsoid::GetLocationPointTransform(m_tileset.position.x / 180.0 * PI, m_tileset.position.y / 180.0 * PI, m_tileset.position.z);
			MapGIS::Tile::Matrix4D rootMatrix = m_pRootTile->GetMatrix();

			D_3DOT dot1 = D_3DOT{ m_tileset.boundingVolume.box.center.x + m_tileset.boundingVolume.box.x_axis.x + m_tileset.boundingVolume.box.y_axis.x + m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y + m_tileset.boundingVolume.box.x_axis.y + m_tileset.boundingVolume.box.y_axis.y + m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z + m_tileset.boundingVolume.box.x_axis.z + m_tileset.boundingVolume.box.y_axis.z + m_tileset.boundingVolume.box.z_axis.z };
			D_3DOT dot2 = D_3DOT{ m_tileset.boundingVolume.box.center.x + m_tileset.boundingVolume.box.x_axis.x + m_tileset.boundingVolume.box.y_axis.x - m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y + m_tileset.boundingVolume.box.x_axis.y + m_tileset.boundingVolume.box.y_axis.y - m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z + m_tileset.boundingVolume.box.x_axis.z + m_tileset.boundingVolume.box.y_axis.z - m_tileset.boundingVolume.box.z_axis.z };
			D_3DOT dot3 = D_3DOT{ m_tileset.boundingVolume.box.center.x + m_tileset.boundingVolume.box.x_axis.x - m_tileset.boundingVolume.box.y_axis.x + m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y + m_tileset.boundingVolume.box.x_axis.y - m_tileset.boundingVolume.box.y_axis.y + m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z + m_tileset.boundingVolume.box.x_axis.z - m_tileset.boundingVolume.box.y_axis.z + m_tileset.boundingVolume.box.z_axis.z };
			D_3DOT dot4 = D_3DOT{ m_tileset.boundingVolume.box.center.x + m_tileset.boundingVolume.box.x_axis.x - m_tileset.boundingVolume.box.y_axis.x - m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y + m_tileset.boundingVolume.box.x_axis.y - m_tileset.boundingVolume.box.y_axis.y - m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z + m_tileset.boundingVolume.box.x_axis.z - m_tileset.boundingVolume.box.y_axis.z - m_tileset.boundingVolume.box.z_axis.z };
			D_3DOT dot5 = D_3DOT{ m_tileset.boundingVolume.box.center.x - m_tileset.boundingVolume.box.x_axis.x + m_tileset.boundingVolume.box.y_axis.x + m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y - m_tileset.boundingVolume.box.x_axis.y + m_tileset.boundingVolume.box.y_axis.y + m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z - m_tileset.boundingVolume.box.x_axis.z + m_tileset.boundingVolume.box.y_axis.z + m_tileset.boundingVolume.box.z_axis.z };
			D_3DOT dot6 = D_3DOT{ m_tileset.boundingVolume.box.center.x - m_tileset.boundingVolume.box.x_axis.x + m_tileset.boundingVolume.box.y_axis.x - m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y - m_tileset.boundingVolume.box.x_axis.y + m_tileset.boundingVolume.box.y_axis.y - m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z - m_tileset.boundingVolume.box.x_axis.z + m_tileset.boundingVolume.box.y_axis.z - m_tileset.boundingVolume.box.z_axis.z };
			D_3DOT dot7 = D_3DOT{ m_tileset.boundingVolume.box.center.x - m_tileset.boundingVolume.box.x_axis.x - m_tileset.boundingVolume.box.y_axis.x + m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y - m_tileset.boundingVolume.box.x_axis.y - m_tileset.boundingVolume.box.y_axis.y + m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z - m_tileset.boundingVolume.box.x_axis.z - m_tileset.boundingVolume.box.y_axis.z + m_tileset.boundingVolume.box.z_axis.z };
			D_3DOT dot8 = D_3DOT{ m_tileset.boundingVolume.box.center.x - m_tileset.boundingVolume.box.x_axis.x - m_tileset.boundingVolume.box.y_axis.x - m_tileset.boundingVolume.box.z_axis.x,m_tileset.boundingVolume.box.center.y - m_tileset.boundingVolume.box.x_axis.y - m_tileset.boundingVolume.box.y_axis.y - m_tileset.boundingVolume.box.z_axis.y ,m_tileset.boundingVolume.box.center.z - m_tileset.boundingVolume.box.x_axis.z - m_tileset.boundingVolume.box.y_axis.z - m_tileset.boundingVolume.box.z_axis.z };

			dot1 = GetJwdValue(rootMatrix, tilesetMatrix, dot1);
			dot2 = GetJwdValue(rootMatrix, tilesetMatrix, dot2);
			dot3 = GetJwdValue(rootMatrix, tilesetMatrix, dot3);
			dot4 = GetJwdValue(rootMatrix, tilesetMatrix, dot4);
			dot5 = GetJwdValue(rootMatrix, tilesetMatrix, dot5);
			dot6 = GetJwdValue(rootMatrix, tilesetMatrix, dot6);
			dot7 = GetJwdValue(rootMatrix, tilesetMatrix, dot7);
			dot8 = GetJwdValue(rootMatrix, tilesetMatrix, dot8);
			MapGIS::Tile::BoundingVolume boundingRegion;
			boundingRegion.type = MapGIS::Tile::BoundingType::Region;
			vector<double> xv = { dot1.x ,dot2.x , dot3.x ,dot4.x , dot5.x , dot6.x , dot7.x , dot8.x };
			vector<double> yv = { dot1.y ,dot2.y , dot3.y ,dot4.y , dot5.y , dot6.y , dot7.y , dot8.y };
			vector<double> zv = { dot1.z ,dot2.z , dot3.z ,dot4.z , dot5.z , dot6.z , dot7.z , dot8.z };
			boundingRegion.region.west = *min_element(xv.begin(), xv.end());
			boundingRegion.region.south = *min_element(yv.begin(), yv.end());
			boundingRegion.region.east = *max_element(xv.begin(), xv.end());
			boundingRegion.region.north = *max_element(yv.begin(), yv.end());
			boundingRegion.region.minHeight = *min_element(zv.begin(), zv.end());
			boundingRegion.region.maxHeight = *max_element(zv.begin(), zv.end());
			m_tileset.boundingVolume = boundingRegion;
		}
		else
		{
			D_3DOT dot{ m_tileset.boundingVolume.sphere.center.x, m_tileset.boundingVolume.sphere.center.y, m_tileset.boundingVolume.sphere.center.z };
			MapGIS::Tile::Vector4D dot_d = m_pRootTile->GetMatrix()* Vector4D(dot.x, dot.y, dot.z, 1);
			m_tileset.boundingVolume.sphere.center.x = dot_d.x;
			m_tileset.boundingVolume.sphere.center.y = dot_d.y;
			m_tileset.boundingVolume.sphere.center.z = dot_d.z;
		}
	}
	Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
	if (pSqliteTool != NULL)
	{
#ifdef _USE_MAPGIS_SDK_
#ifdef MergePropertiesInThread
		g_WriteSqliteThreadPool.WaitForDone();
#endif
#endif
		pSqliteTool->Save();
		CGString sqlPath = pSqliteTool->GetGetRelativePath();
		m_tileset.sqlitePath = sqlPath;
	}
	m_tileset.lodType = m_pRootTile->GetRefine();
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	if (m_tileset.To(doc, allocator) <= 0)
		return 0;

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	CGString uri = i_GetTilesetRelative(m_pCacheStorage, m_tilesetName);
	gisLONG rtn = m_pCacheStorage->SetContent(uri, buffer.GetString(), buffer.GetLength());
	return m_pRootTile->Save();
}

gisLONG Ci_G3DM3DCacheTilesetImpl::Close()
{
	if (m_pRootTile != NULL)
		delete m_pRootTile;
	m_tileset.compressType = FileCompressType::Zip;
	char szGuid[256];
	GetNewGuidToString(szGuid);
	m_tileset.guid = szGuid;
	m_tileset.lodType = RefineType::Replace;
	m_tileset.spatialReference = SpatialReference::WGS84;
	m_tileset.treeType = TreeType::QuadTree;
	G3DCacheType cacheType = i_GetCacheType();
	switch (cacheType)
	{
	case G3DCacheType::TypeM3DV20:
		m_tileset.version = M3DVersion::M3D20;
		break;
	case G3DCacheType::TypeM3DV21:
		m_tileset.version = M3DVersion::M3D21;
		break;
	case G3DCacheType::TypeM3DV22:
		m_tileset.version =  M3DVersion::M3D22;
		break;
	default:
		m_tileset.version = M3DVersion::M3D21;
		break;
	}
	m_tileset.rootNode_uri = "rootNode.json";
	m_pRootTile = NULL;
	m_tilesetName = "";
	/*Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(m_pCacheStorage, false);
	if (pSqliteTool !=NULL && m_pCacheStorage )
	{
		Ci_3DModelAttributeSqliteTool::Free3DModelAttributeSqlite(pSqliteTool);
	}*/
	m_pCacheStorage = NULL;
	return 1;
}

gisLONG Ci_G3DM3DCacheTilesetImpl::Create(G3DCacheStorage *pCacheStorage, CGString tilesetName)
{
	Close();
	if (pCacheStorage == NULL || !pCacheStorage->IsValid()  || tilesetName.IsEmpty())
		return 0;
	m_pRootTile = i_CreateRootTile(pCacheStorage);
	if (m_pRootTile != NULL)
	{
		m_pRootTile->m_relativePath = "rootNode.json";
		m_pRootTile->m_tile.name = "rootNode";
		m_pRootTile->m_tile.lodLevel = 0;
#ifdef _USE_MAPGIS_SDK_
		Ci_G3DMongoDBCacheStorage* pMongoDB = dynamic_cast<Ci_G3DMongoDBCacheStorage*>(pCacheStorage);
		if (pMongoDB != NULL && !pCacheStorage->IsExistContent("CacheInfo"))
		{
			pMongoDB->SetMetaData(i_GetCacheType());
		}
#endif // _USE_MAPGIS_SDK_
		m_tilesetName = tilesetName;
		m_pCacheStorage = pCacheStorage;
		return 1;
	}
	return 0;
}
gisLONG Ci_G3DM3DCacheTilesetImpl::Open(G3DCacheStorage *pCacheStorage, CGString tilesetName)
{
	Close();
	if (pCacheStorage == NULL || !pCacheStorage->IsValid() || tilesetName.IsEmpty())
		return 0;
	CGString path = pCacheStorage->GetRootUrl();
	path.Convert(CGString::EncodeType::GB18030);
	path.Replace('\\', '/');
	CGString uri = i_GetTilesetRelative(pCacheStorage, tilesetName);
	CGByteArray value;
	pCacheStorage->GetContent(uri, value);
	if (value.isEmpty())
		return 0;
	rapidjson::Document doc;
	if (doc.Parse(value.data(), value.size()).HasParseError())
		return 0;
	if (!doc.IsObject())
		return 0;
	if (m_tileset.From(doc) <= 0)
		return 0;
	CGString rootUrl = m_tileset.rootNode_uri;
	CGString rootRelativePath = MakePathByRelativePath(uri, rootUrl);
	if (pCacheStorage->GetContent(rootRelativePath, value) <= 0)
		return 0;

	rapidjson::Document rootDoc;
	if (rootDoc.Parse(value.data(), value.size()).HasParseError())
		return 0;
	Ci_G3DM3DCacheTileImpl* pRoot = i_CreateRootTile(pCacheStorage);
	if (!rootDoc.IsObject() || pRoot->m_tile.From(rootDoc) <= 0)
	{
		delete pRoot;
		return 0;
	}
	m_pRootTile = pRoot;
	m_pRootTile->m_relativePath = rootRelativePath;
	m_tilesetName = tilesetName;
	m_pCacheStorage = pCacheStorage;
	return 1;
}

bool Ci_G3DM3DCacheTilesetImpl::IsValid()
{
	if (m_pCacheStorage != NULL && m_pRootTile != NULL)
		return true;
	return false;
}

G3DTilesetInfo* Ci_G3DM3DCacheTilesetImpl::GetInfo()
{
	G3DM3DTilesetInfo *rtn = new G3DM3DTilesetInfo(&m_tileset, false);
	return rtn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ci_G3DM3D20CacheTilesetImpl::Ci_G3DM3D20CacheTilesetImpl()
{
	m_tileset.version = M3DVersion::M3D20;
}

Ci_G3DM3D20CacheTilesetImpl::~Ci_G3DM3D20CacheTilesetImpl()
{
}

Ci_G3DM3DCacheTileImpl* Ci_G3DM3D20CacheTilesetImpl::i_CreateRootTile(G3DCacheStorage *pCacheStorage)
{
	Ci_G3DM3D20CacheTileImpl* pRoot = new Ci_G3DM3D20CacheTileImpl(pCacheStorage);
	return pRoot;
}

G3DCacheType Ci_G3DM3D20CacheTilesetImpl::i_GetCacheType()
{
	return G3DCacheType::TypeM3DV20;
}

void Ci_G3DM3D20CacheTilesetImpl::ClearLayerFieldsInfo()
{
	if (IsValid())
		m_tileset.fieldInfo.clear();
}
gisLONG Ci_G3DM3D20CacheTilesetImpl::SetLayerFieldsInfo(const LayersInfoBase *pLayersInfo)
{
	if (!IsValid() || pLayersInfo == NULL )
		return 0;

	const LayersInfo* pLayerFieldsInfo = dynamic_cast<const LayersInfo*>(pLayersInfo);

	if (pLayerFieldsInfo == NULL || pLayerFieldsInfo->GetLayerNum() != 1)
		return 0;

	const LayerFieldsInfo& info =  pLayerFieldsInfo->GetLayerInfo(0);
	m_tileset.fieldInfo.clear();
	m_tileset.fieldInfo.insert(m_tileset.fieldInfo.begin(), info.fieldInfos.begin(), info.fieldInfos.end());
	return 1;
}

gisLONG Ci_G3DM3D20CacheTilesetImpl::GetLayerFieldsInfo(LayersInfoBase *pLayersInfo)
{
	if (!IsValid() || pLayersInfo == NULL)
		return 0;
	LayersInfo* pLayerFieldsInfo = dynamic_cast<LayersInfo*>(pLayersInfo);
	if (pLayerFieldsInfo == NULL)
		return 0;
	LayerFieldsInfo info;
	info.fieldInfos.insert(info.fieldInfos.begin(), m_tileset.fieldInfo.begin(), m_tileset.fieldInfo.end());
	pLayerFieldsInfo->m_layersInfo.push_back(info);
	return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ci_G3DM3D21CacheTilesetImpl::Ci_G3DM3D21CacheTilesetImpl()
{
	m_tileset.version = M3DVersion::M3D21;
}

Ci_G3DM3D21CacheTilesetImpl::~Ci_G3DM3D21CacheTilesetImpl()
{
}

Ci_G3DM3DCacheTileImpl* Ci_G3DM3D21CacheTilesetImpl::i_CreateRootTile(G3DCacheStorage *pCacheStorage)
{
	Ci_G3DM3D21CacheTileImpl* pRoot = new Ci_G3DM3D21CacheTileImpl(pCacheStorage);
	return pRoot;
}
G3DCacheType Ci_G3DM3D21CacheTilesetImpl::i_GetCacheType()
{
	return G3DCacheType::TypeM3DV21;
}
void Ci_G3DM3D21CacheTilesetImpl::ClearLayerFieldsInfo()
{
	if (IsValid())
		m_pCacheStorage->DeleteContent("layerinfo.json");
}
gisLONG Ci_G3DM3D21CacheTilesetImpl::SetLayerFieldsInfo(const LayersInfoBase *pLayersInfo)
{
	if (!IsValid() || pLayersInfo == NULL)
		return 0;
	CGByteArray bayeArray;
	pLayersInfo->To(bayeArray);
	m_pCacheStorage->SetContent("layerinfo.json", bayeArray);
	m_tileset.layerInfo_uri = "layerinfo.json";
	return 1;
}

gisLONG Ci_G3DM3D21CacheTilesetImpl::GetLayerFieldsInfo(LayersInfoBase *pLayersInfo)
{
	if (!IsValid() || pLayersInfo == NULL)
		return 0;
	CGByteArray bayeArray;
	if (m_pCacheStorage->GetContent("layerinfo.json", bayeArray) <= 0)
		return 0;
	return pLayersInfo->From(bayeArray);
}

gisLONG Ci_G3DM3D21CacheTilesetImpl::SetLayerInfo(const CGByteArray& layerInfo)
{
	if (!IsValid() || layerInfo.isEmpty())
		return 0;
	m_pCacheStorage->SetContent("layerinfo.json", layerInfo);
	m_tileset.layerInfo_uri = "layerinfo.json";
	return 1;
}

gisLONG Ci_G3DM3D21CacheTilesetImpl::GetLayerInfo(CGByteArray& layerInfo)
{
	if (!IsValid())
		return 0;

	if (m_pCacheStorage->GetContent("layerinfo.json", layerInfo) <= 0)
		return 0;

	return 1;
}
//根据叶子节点更新组节点范围
gisLONG UpadteNodeBox(StructureTreeNode& node)
{
	gisLONG childNum = node.GetChildNum();
	if (childNum > 0)
	{
		double xmin = MAX_DOUBLE, ymin = MAX_DOUBLE, zmin = MAX_DOUBLE;
		double xmax = MIN_DOUBLE, ymax = MIN_DOUBLE, zmax = MIN_DOUBLE;
		bool hasBox = false;
		for (int i = 0; i < childNum; i++)
		{
			StructureTree* item = node.GetChild(i);

			StructureTreeNode* pStructureTreeNode = dynamic_cast<StructureTreeNode*>(item);
			StructureTreeLeaf* pStructureTreeLeaf = dynamic_cast<StructureTreeLeaf*>(item);
			if (pStructureTreeNode != NULL)
			{
				UpadteNodeBox(*pStructureTreeNode);
				if (pStructureTreeNode->box.size() == 6)
				{
					xmin = min(xmin, pStructureTreeNode->box[0]);
					ymin = min(ymin, pStructureTreeNode->box[1]);
					zmin = min(zmin, pStructureTreeNode->box[2]);

					xmax = max(xmax, pStructureTreeNode->box[3]);
					ymax = max(ymax, pStructureTreeNode->box[4]);
					zmax = max(zmax, pStructureTreeNode->box[5]);
					hasBox = true;
				}
			}
			else if (pStructureTreeLeaf != NULL)
			{
				if (pStructureTreeLeaf->box.size() == 6)
				{
					xmin = min(xmin, pStructureTreeLeaf->box[0]);
					ymin = min(ymin, pStructureTreeLeaf->box[1]);
					zmin = min(zmin, pStructureTreeLeaf->box[2]);

					xmax = max(xmax, pStructureTreeLeaf->box[3]);
					ymax = max(ymax, pStructureTreeLeaf->box[4]);
					zmax = max(zmax, pStructureTreeLeaf->box[5]);
					hasBox = true;
				}
			}
		}
		if (hasBox)
		{
			node.box.clear();
			node.box.emplace_back(xmin);
			node.box.emplace_back(ymin);
			node.box.emplace_back(zmin);
			node.box.emplace_back(xmax);
			node.box.emplace_back(ymax);
			node.box.emplace_back(zmax);
		}
	}
	return 1;
}

gisLONG Ci_G3DM3D21CacheTilesetImpl::SetStructureTreeInfo(StructureTreeNode& rootNode)
{
	if (!IsValid())
		return 0;
	UpadteNodeBox(rootNode);
	Ci_StructureTreeManager::Write(rootNode, m_pCacheStorage, "structuretree.json");
	return 1;
}

gisLONG Ci_G3DM3D21CacheTilesetImpl::GetStructureTreeInfo(StructureTreeNode& rootNode)
{
	return Ci_StructureTreeManager::Read(m_pCacheStorage, "structuretree.json", rootNode);
}

Ci_G3DM3D22CacheTilesetImpl::Ci_G3DM3D22CacheTilesetImpl()
{
	m_tileset.version = M3DVersion::M3D22;
}

Ci_G3DM3D22CacheTilesetImpl::~Ci_G3DM3D22CacheTilesetImpl()
{
}
gisLONG    Ci_G3DM3D22CacheTilesetImpl::StartWriteSqlite()
{
	Ci_3DModelAttributeSqliteTool*pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(GetCacheStorage(), true);
	return 1;
}
gisLONG    Ci_G3DM3D22CacheTilesetImpl::EndWriteSqlite()
{
	if (GetCacheStorage() != NULL)
	{
		Ci_3DModelAttributeSqliteTool*pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(GetCacheStorage(), false);
		if (pSqliteTool != NULL)
			Ci_3DModelAttributeSqliteTool::Free3DModelAttributeSqlite(pSqliteTool);
	}
	return 1;
}

void Ci_G3DM3D22CacheTilesetImpl::SetBoxInfoInSqlite(const map<gisINT64, vector<double> >& tid2Box) 
{
	if (GetCacheStorage() != NULL)
	{
		Ci_3DModelAttributeSqliteTool* pSqliteTool = Ci_3DModelAttributeSqliteTool::GetInstance(GetCacheStorage(), false);
		if (pSqliteTool != NULL)
			pSqliteTool->SetBoxInfo(tid2Box);
	}
	return ;
}

Ci_G3DM3DCacheTileImpl* Ci_G3DM3D22CacheTilesetImpl::i_CreateRootTile(G3DCacheStorage *pCacheStorage)
{
	Ci_G3DM3D22CacheTileImpl* pRoot = new Ci_G3DM3D22CacheTileImpl(pCacheStorage);
	return pRoot;
}
G3DCacheType Ci_G3DM3D22CacheTilesetImpl::i_GetCacheType()
{
	return G3DCacheType::TypeM3DV22;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////