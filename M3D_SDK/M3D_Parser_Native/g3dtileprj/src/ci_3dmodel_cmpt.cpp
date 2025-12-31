#include "stdafx.h"
#include "ci_3dmodel.h"
#include "ci_assist.h"
using namespace MapGIS::Tile;

class Ci_3DTileCmpt
{
public:
	Ci_3DTileCmpt();
	~Ci_3DTileCmpt();

	CGByteArray Write(const vector<CGByteArray>& tiles);

private:
	CGByteArray i_WriteHead();

private:
	string m_magic;
	int m_version;
	int m_byteLength;
	int m_tilesSize;
};

Ci_3DTileCmpt::Ci_3DTileCmpt()
{
	m_magic = "cmpt";
	m_version = 1;
	m_byteLength = 0;
	m_tilesSize = 0;
}

Ci_3DTileCmpt::~Ci_3DTileCmpt()
{
}

CGByteArray Ci_3DTileCmpt::Write(const vector<CGByteArray>& tiles)
{
	CGByteArray rtvArr;
	if (tiles.empty())
		return rtvArr;

	m_tilesSize = tiles.size();
	vector<CGByteArray>::const_iterator iter;
	m_byteLength = 16;
	for (iter = tiles.begin(); iter != tiles.end(); ++iter)
	{
		m_byteLength += iter->length();
	}

	rtvArr.reserve(m_byteLength);

	CGByteArray headerInfo = i_WriteHead();

	rtvArr.append(headerInfo);

	for (iter = tiles.begin(); iter != tiles.end(); ++iter)
	{
		rtvArr.append(*iter);
	}

	return rtvArr;
}

CGByteArray Ci_3DTileCmpt::i_WriteHead()
{
	CGByteArray rtvArr;
	rtvArr.append(m_magic.c_str(), 4);
	rtvArr.append((const char*)&m_version, 4);
	rtvArr.append((const char*)&m_byteLength, 4);
	rtvArr.append((const char*)&m_tilesSize, 4);

	return rtvArr;
}

gisLONG Ci_ModelCmpt::From(vector<ContentBase*>* pContents, GeoCompressType compressType)
{
	if (pContents == NULL)
		return 0;
	vector<CGByteArray> modelArrays;
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
		if (pInstances == NULL ||   pInstances->size() <= 0)
		{//B3DM
			if (p3DModel->GetGeometryType() == GeometryType::Point)
			{
				Ci_ModelPnts modelPnts;
				if (pAttribute == NULL || pAttribute->layers.size() > 1 || pAttribute->layers.size() <= 0)
					modelPnts.From(*(MapGIS::Tile::PointsModel*)p3DModel, NULL, compressType == GeoCompressType::Draco, NULL);
				else
					modelPnts.From(*(MapGIS::Tile::PointsModel*)p3DModel,  &pAttribute->layers[0], compressType == GeoCompressType::Draco, NULL);
				modelArrays.push_back(*modelPnts.Get());
			}
			else
			{
				Ci_ModelB3dm modelB3dm;
				if (pAttribute == NULL || pAttribute->layers.size() > 1 || pAttribute->layers.size() <= 0)
					modelB3dm.From(p3DModel, NULL, compressType, NULL);
				else
					modelB3dm.From(p3DModel, &pAttribute->layers[0], compressType, NULL);
				modelArrays.push_back(*modelB3dm.Get());
			}
		}
		else
		{//i3DM
			Ci_ModelI3dm modelI3dm;
			if (pAttribute == NULL || pAttribute->layers.size() > 1 || pAttribute->layers.size() <= 0)
				modelI3dm.From(p3DModel,*pInstances, NULL, compressType, NULL);
			else
				modelI3dm.From(p3DModel,*pInstances, &pAttribute->layers[0], compressType, NULL);
			modelArrays.push_back(*modelI3dm.Get());
		}
	}
	return From(modelArrays);
}

gisLONG Ci_ModelCmpt::From(vector<CGByteArray>& arrays)
{
	Ci_3DTileCmpt cmpt;
	m_data =  cmpt.Write(arrays);
	return 1;
}

gisLONG Ci_ModelCmpt::To(vector<ContentBase*>* pContents)
{
	if (pContents == NULL)
		return 0;
	int c = 0;
	string magic = ReadByteArrayToString(m_data, c, 4); c += 4;
	int version = ReadByteArrayToInt32(m_data, c); c += 4;
	int bytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int tilesSize = ReadByteArrayToInt32(m_data, c); c += 4;
	if (StrNICmp(magic.c_str(), "cmpt", 4) != 0)
	{
		return 0;
	}
	if (m_data.size() < bytesLength)
	{
		return 0;
	}

	if (tilesSize <= 0)
		return 0;

	CGByteArray tempBuffer;
	for (int i = 0; i < tilesSize; i++)
	{
		string TempType = ReadByteArrayToString(m_data, c, 4);
		int tempLength = ReadByteArrayToInt32(m_data, c + 8);

		if (tempLength + c > bytesLength)
		{
			return 0;
		}
		tempBuffer.clear();
		tempBuffer.append(&m_data.data()[c], tempLength);
		if (StrNICmp(TempType.c_str(), "i3dm", 4) == 0)
		{
			GeometryContent * pItem = new GeometryContent();
			pContents->emplace_back(pItem);
			pItem->CreateData(true, GeometryType::Surface, true, true);
			vector<ModelInstance>* pInstances = pItem->GetModelInstance();
			Attribute* pAttribute = pItem->GetAttribute();
			G3DModel* p3DModel = pItem->Get3DModel();
			Ci_ModelI3dm modelI3dm;
			modelI3dm.SetCacheStorage(m_pStorage);
			((Ci_TileModel*)&modelI3dm)->From(tempBuffer);

			LayerAttribute layerAttribute;
			D_3DOT  rtcCenter;
			modelI3dm.To(p3DModel, *pInstances, layerAttribute, rtcCenter);

			if (layerAttribute.layerInfo.fieldInfos.size() > 0)
			{
				pAttribute->layers.emplace_back(layerAttribute);
			}
		}
		else if (StrNICmp(TempType.c_str(), "b3dm", 4) == 0)
		{
			GeometryContent * pItem = new GeometryContent();
			pContents->emplace_back(pItem);
			pItem->CreateData(true, GeometryType::Surface, true, true);
			vector<ModelInstance>* pInstances = pItem->GetModelInstance();
			Attribute* pAttribute = pItem->GetAttribute();
			G3DModel* p3DModel = pItem->Get3DModel();
			Ci_ModelB3dm modelB3dm;
			D_3DOT  rtcCenter;
			modelB3dm.SetCacheStorage(m_pStorage);
			((Ci_TileModel*)&modelB3dm)->From(tempBuffer);
			LayerAttribute layerAttribute;
			modelB3dm.To(p3DModel, layerAttribute, rtcCenter);

			if (layerAttribute.layerInfo.fieldInfos.size() > 0)
			{
				pAttribute->layers.emplace_back(layerAttribute);
			}
		}
		else if (StrNICmp(TempType.c_str(), "pnts", 4) == 0)
		{
			GeometryContent * pItem = new GeometryContent();
			pContents->emplace_back(pItem);
			pItem->CreateData(true,GeometryType::Point, true, true);
			vector<ModelInstance>* pInstances = pItem->GetModelInstance();
			Attribute* pAttribute = pItem->GetAttribute();
			G3DModel* p3DModel = pItem->Get3DModel();
			Ci_ModelPnts pnts;
			D_3DOT  rtcCenter;
			pnts.SetCacheStorage(m_pStorage);
			((Ci_TileModel*)&pnts)->From(tempBuffer);
			LayerAttribute layerAttribute;
			pnts.To(*(PointsModel*)p3DModel, layerAttribute, rtcCenter);
			if (layerAttribute.layerInfo.fieldInfos.size() > 0)
			{
				pAttribute->layers.emplace_back(layerAttribute);
			}
		}
		else if (StrNICmp(TempType.c_str(), "cmpt", 4) == 0)
		{
			Ci_ModelCmpt cmpt;
			cmpt.SetCacheStorage(m_pStorage);
			((Ci_TileModel*)&cmpt)->From(tempBuffer);
			cmpt.To(pContents);
		}
		c += tempLength;
	}
	return 1;
}