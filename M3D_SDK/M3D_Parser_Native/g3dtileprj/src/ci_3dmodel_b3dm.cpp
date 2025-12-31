#include "stdafx.h"
#include "ci_3dmodel.h"
#include "ci_assist.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <algorithm>
using namespace MapGIS::Tile;

class Ci_B3dmFeatureTable :public Ci_3DTileFeatureTable
{
public:
	int m_nBatchLength;
	D_3DOT* m_pRtcCenter;

	Ci_B3dmFeatureTable();
	~Ci_B3dmFeatureTable();

	CGByteArray BuildJSON();

	CGByteArray BuildBinary();
};

class Ci_B3dmHeader :public Ci_3DTileModelHeader
{
public:
	Ci_B3dmHeader();
	~Ci_B3dmHeader();

	virtual unsigned int GetHeaderLength();
	void WriteTo(CGByteArray& writer);
};

class Ci_3DTileB3dm
{
public:
	Ci_3DTileB3dm();
	~Ci_3DTileB3dm();
	void SetRtcCenter(double x, double y, double z);
	void SetGltfData(const CGByteArray& value);

	void SetIdName(string idName);
	void SetIdToBatchID(map<gisINT64, gisINT64>* pIdToBatchID);

	void SetAttribute(LayerAttribute* pRecords);
	void WriteTo(CGByteArray& writer);
private:
	Ci_B3dmHeader m_header;
	Ci_B3dmFeatureTable m_featureTable;
	LayerAttribute* m_pRecords;
	map<gisINT64, gisINT64>* m_pIdToBatchID;
	CGByteArray m_gltfData;
	string m_idName;
};

#pragma region B3dmFeatureTable
Ci_B3dmFeatureTable::Ci_B3dmFeatureTable()
{
	m_nBatchLength = 0;
	m_pRtcCenter = NULL;
}
Ci_B3dmFeatureTable::~Ci_B3dmFeatureTable()
{
	SAFE_DELETE_PTR(m_pRtcCenter);
}

CGByteArray Ci_B3dmFeatureTable::BuildJSON()
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	doc.AddMember("BATCH_LENGTH", m_nBatchLength, allocator);
	if (m_pRtcCenter)
	{
		rapidjson::Value jsArr(rapidjson::kArrayType);
		jsArr.PushBack(m_pRtcCenter->x, allocator);
		jsArr.PushBack(m_pRtcCenter->y, allocator);
		jsArr.PushBack(m_pRtcCenter->z, allocator);
		doc.AddMember("RTC_CENTER", jsArr, allocator);
	}
	if (!m_extensions.IsNull())
	{
		doc.AddMember("extensions", m_extensions, allocator);
	}
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	char* pBuffer = const_cast<char*>(buffer.GetString());
	return CGByteArray(pBuffer, buffer.GetLength());
}

CGByteArray Ci_B3dmFeatureTable::BuildBinary()
{
	return CGByteArray();
}

#pragma endregion

#pragma region B3dmHeader
Ci_B3dmHeader::Ci_B3dmHeader()
{
	m_magic = "b3dm";
	m_version = 1;
}
Ci_B3dmHeader::~Ci_B3dmHeader()
{
}
unsigned int Ci_B3dmHeader::GetHeaderLength()
{
	return 28;
}
void Ci_B3dmHeader::WriteTo(CGByteArray& writer)
{
	CGByteArray tmpWriter;
	tmpWriter.reserve(GetHeaderLength());
	tmpWriter.append(m_magic.c_str(), 4);
	tmpWriter.append((const char*)&m_version, 4);
	tmpWriter.append((const char*)&m_byteLength, 4);
	tmpWriter.append((const char*)&m_featureTableJSONLength, 4);
	tmpWriter.append((const char*)&m_featureTableBinaryLength, 4);
	tmpWriter.append((const char*)&m_batchTableJSONLength, 4);
	tmpWriter.append((const char*)&m_batchTableBinaryLength, 4);

	writer.replace(0, GetHeaderLength(), tmpWriter.constData(), GetHeaderLength());
}

#pragma endregion

#pragma region Ci_3DTiles_B3dm

Ci_3DTileB3dm::Ci_3DTileB3dm()
{
	m_pRecords = NULL;
	m_pIdToBatchID = NULL;
	m_idName = "";
}
Ci_3DTileB3dm::~Ci_3DTileB3dm()
{
}
void Ci_3DTileB3dm::SetRtcCenter(double x, double y, double z)
{
	if (NULL == m_featureTable.m_pRtcCenter)
		m_featureTable.m_pRtcCenter = new D_3DOT();

	m_featureTable.m_pRtcCenter->x = x;
	m_featureTable.m_pRtcCenter->y = y;
	m_featureTable.m_pRtcCenter->z = z;
}

void Ci_3DTileB3dm::SetGltfData(const CGByteArray& value)
{
	m_gltfData = value;
	int nRemainLen = m_gltfData.length() % 8;
	if (nRemainLen>0)
	{
		m_gltfData.append(8 - nRemainLen, ' ');
	}
}
void Ci_3DTileB3dm::SetIdName(string idName)
{
	m_idName = idName;
}
void Ci_3DTileB3dm::SetIdToBatchID(map<gisINT64, gisINT64>* pIdToBatchID)
{
	m_pIdToBatchID = pIdToBatchID;
}
void Ci_3DTileB3dm::SetAttribute(LayerAttribute* pRecords)
{
	m_pRecords = pRecords;
}

void Ci_3DTileB3dm::WriteTo(CGByteArray& writer)
{
	// 1 placeholder for model header
	writer.append(m_header.GetHeaderLength(), ' ');

	gisINT64 maxBatchId = -1;
	unordered_map<gisINT64, gisINT64>  batchIDToId;
	// 2 feature table
	{
		if (m_pIdToBatchID != NULL)
		{
			for (map<gisINT64, gisINT64>::const_iterator itr = m_pIdToBatchID->begin(); itr != m_pIdToBatchID->end(); itr++)
			{
				if (batchIDToId.find(itr->second) == batchIDToId.end())
					batchIDToId.insert(make_pair(itr->second, itr->first));
				maxBatchId = max(maxBatchId, itr->second);
			}
		}
		else  if (m_pRecords != NULL && m_pRecords->records.size() > 0)
		{
			for (vector<Record>::const_iterator itr = m_pRecords->records.begin(); itr != m_pRecords->records.end(); itr++)
				maxBatchId = max(maxBatchId, itr->GetID());
		}
		if (maxBatchId >= 0)
			m_featureTable.m_nBatchLength = maxBatchId + 1;
		CGByteArray ftBinaryData = m_featureTable.BuildBinary();
		CGByteArray ftJSData = m_featureTable.BuildJSON();
		writer.append(ftJSData.constData(), ftJSData.length());
		m_header.SetFeatureTableJSONLengthNoPadding(ftJSData.length());
		if (m_header.GetFeatureTableJSONPaddingCount() > 0)
			writer.append(m_header.GetFeatureTableJSONPaddingCount(), ' ');

		if (ftBinaryData.length() > 0)
		{
			m_header.SetFeatureTableBinaryLengthNoPadding(ftBinaryData.length());
			writer.append(ftBinaryData.constData(), ftBinaryData.length());
			if (m_header.GetFeatureTableBinaryPaddingCount() > 0)
				writer.append(m_header.GetFeatureTableBinaryPaddingCount(), ' ');
		}
	}

	//3 batch table
	if (m_pRecords != NULL && m_pRecords->records.size() > 0)
	{
		CGByteArray binData;
		CGByteArray btJsData;
		Ci_3DTileBatchTable batchTable;
		batchTable.WriteTo(m_pRecords, m_pIdToBatchID, btJsData, binData, m_idName);
		m_header.SetBatchTableJSONLengthNoPadding(btJsData.length());
		writer.append(btJsData);
		if (m_header.GetBatchTableJSONPaddingCount() > 0)
			writer.append(m_header.GetBatchTableJSONPaddingCount(), ' ');

		if (binData.length() > 0)
		{
			m_header.SetBatchTableBinaryLengthNoPadding(binData.length());
			writer.append(binData);
			if (m_header.GetBatchTableBinaryPaddingCount()>0)
				writer.append(m_header.GetBatchTableBinaryPaddingCount(), ' ');
		}
	}
	else if (m_pIdToBatchID != NULL &&!m_idName.empty())
	{
		CGByteArray btJsData;
		btJsData.append("{\"");
		btJsData.append(m_idName.c_str());
		btJsData.append("\":[");
		for (int i = 0; i <= maxBatchId; i++)
		{
			if (batchIDToId.find(i) != batchIDToId.end())
				btJsData.append(std::to_string(batchIDToId[i]).c_str());
			else
				btJsData.append("-1");

			if (i != maxBatchId)
				btJsData.append(",");
		}
		btJsData.append("]}");

		m_header.SetBatchTableJSONLengthNoPadding(btJsData.length());
		writer.append(btJsData);
		if (m_header.GetBatchTableJSONPaddingCount() > 0)
			writer.append(m_header.GetBatchTableJSONPaddingCount(), ' ');
	}

	// 5 gltf data
	writer.append(m_gltfData.constData(), m_gltfData.length());

	// 6 update header
	unsigned int byteLen = m_header.GetHeaderLength() +
		m_header.GetFeatureTableJSONLength() +
		m_header.GetFeatureTableBinaryLength() +
		m_header.GetBatchTableJSONLength() +
		m_header.GetBatchTableBinaryLength() +
		m_gltfData.length();

	m_header.SetByteLength(byteLen);
	m_header.WriteTo(writer);
}

#pragma region

//常规B3dm
gisLONG  Ci_ModelB3dm::From(MapGIS::Tile::G3DModel *pModel, LayerAttribute* pRecords, GeoCompressType compressType, D_3DOT* m_pRtcCenter)
{
	if (pModel == NULL)
		return 0;
	Ci_ModelGltf gltf;
	map<gisINT64, gisINT64> IdBatchID;
	gltf.FromResetID(pModel, compressType, WriteIdType::batchID, IdBatchID);

	const CGByteArray* pByteArray = gltf.Get();
	Ci_3DTileB3dm b3dm;
	if (m_pRtcCenter != NULL)
	{
		b3dm.SetRtcCenter(m_pRtcCenter->x, m_pRtcCenter->y, m_pRtcCenter->z);
	}
	b3dm.SetGltfData(*pByteArray);
	b3dm.SetIdToBatchID(&IdBatchID);
	b3dm.SetIdName("OID");

	if (pRecords != NULL && pRecords->layerInfo.fieldInfos.size() > 0)
	{
		b3dm.SetAttribute(pRecords);
	}

	b3dm.WriteTo(m_data);
	return 1;
}
//专共M3D 2.1  与3D Tiles 1.0 Ext 使用
gisLONG Ci_ModelB3dm::From(MapGIS::Tile::G3DModel *pModel, GeoCompressType compressType, D_3DOT* m_pRtcCenter, bool resetID, vector<gisINT64>& batchIDToId)
{
	if (pModel == NULL)
		return 0;
	Ci_ModelGltf gltf;

	map<gisINT64, gisINT64> IdBatchID;

	if (resetID)
		gltf.FromResetID(pModel, compressType, WriteIdType::batchID, IdBatchID);
	else
		gltf.From(pModel, compressType, WriteIdType::batchID);

	gisINT64 maxBatchId = -1;
	unordered_map<gisINT64, gisINT64>  batchIDToIdMap;
	for (map<gisINT64, gisINT64>::const_iterator itr = IdBatchID.begin(); itr != IdBatchID.end(); itr++)
	{
		if (batchIDToIdMap.find(itr->second) == batchIDToIdMap.end())
			batchIDToIdMap.insert(make_pair(itr->second, itr->first));
		maxBatchId = max(maxBatchId, itr->second);
	}

	for (int i = 0; i <= maxBatchId; i++)
	{
		if (batchIDToIdMap.find(i) != batchIDToIdMap.end())
			batchIDToId.emplace_back(batchIDToIdMap[i]);
		else
			batchIDToId.emplace_back(-1);
	}

	const CGByteArray* pByteArray = gltf.Get();
	Ci_3DTileB3dm b3dm;
	if (resetID)
		b3dm.SetIdToBatchID(&IdBatchID);
	if (m_pRtcCenter != NULL)
		b3dm.SetRtcCenter(m_pRtcCenter->x, m_pRtcCenter->y, m_pRtcCenter->z);
	b3dm.SetGltfData(*pByteArray);
	b3dm.WriteTo(m_data);
	return 1;
}

gisLONG Ci_ModelB3dm::To(MapGIS::Tile::G3DModel *pModel, LayerAttribute& records,  D_3DOT& rtcCenter)
{
	if (m_data.size() <= 0)
		return  0;

	int c = 0;
	string magic = ReadByteArrayToString(m_data, c, 4); c += 4;
	int version = ReadByteArrayToInt32(m_data.data(), c); c += 4;
	int b3dmBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int featureTableJsonBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int featureTableBinBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int batchTableJsonBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int batchTableBinaBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	if (StrICmp("b3dm", magic.c_str()) != 0)
		return 0;
	int gltfLength = b3dmBytesLength - 28 - featureTableJsonBytesLength - featureTableBinBytesLength - batchTableJsonBytesLength - batchTableBinaBytesLength;

	CGByteArray featureTableJsonByteArray;
	CGByteArray featureTableBinByteArray;
	CGByteArray batchTableJsonByteArray;
	CGByteArray batchTableBinByteArray;
	CGByteArray gltfByteArray;
	vector<float> outRtcCenter;
	vector<gisUINT> outBatchLength;
	gisUINT batchLength = 0;

	ReadByteArrayToByteArray(m_data, c, featureTableJsonBytesLength, featureTableJsonByteArray);
	c += featureTableJsonBytesLength;

	ReadByteArrayToByteArray(m_data, c, featureTableBinBytesLength, featureTableBinByteArray);
	c += featureTableBinBytesLength;

	ReadByteArrayToByteArray(m_data, c, batchTableJsonBytesLength, batchTableJsonByteArray);
	c += batchTableJsonBytesLength;

	ReadByteArrayToByteArray(m_data, c, batchTableBinaBytesLength, batchTableBinByteArray);
	c += batchTableBinaBytesLength;

	ReadByteArrayToByteArray(m_data, c, gltfLength, gltfByteArray);
	c += gltfLength;

	rapidjson::Document doc;
	if (!doc.Parse(featureTableJsonByteArray.data(), featureTableJsonByteArray.size()).HasParseError())
	{
		rtcCenter.x = 0;
		rtcCenter.y = 0;
		rtcCenter.z = 0;
		GetTableValue(doc, "RTC_CENTER", 3, featureTableBinByteArray, outRtcCenter);
		if (outRtcCenter.size() == 3)
		{
			rtcCenter.x = outRtcCenter[0];
			rtcCenter.y = outRtcCenter[1];
			rtcCenter.z = outRtcCenter[2];
		}
		GetTableValue(doc, "BATCH_LENGTH", 1, featureTableBinByteArray, outBatchLength);
		if (outBatchLength.size() == 1)
			batchLength = outBatchLength[0];
	}

	if (batchLength > 0 && batchTableJsonBytesLength > 0)
	{
		Ci_3DTileBatchTable batchTable;
		batchTable.ReadFrom(batchTableJsonByteArray, batchTableBinByteArray, "OID", batchLength, records);
	}

	if (gltfLength > 0)
	{
		Ci_ModelGltf gltf;
		((Ci_TileModel*)&gltf)->From(gltfByteArray);
		gltf.SetCacheStorage(m_pStorage);
		gltf.To(pModel);
		//重置ID
		if (records.records.size() > 0 && pModel != NULL)
		{
			vector<gisINT64> oids;
			for (vector<Record>::iterator itr = records.records.begin(); itr != records.records.end(); itr++)
			{
				oids.emplace_back(itr->GetID());
			}
			if (pModel->GetGeometryType() == GeometryType::Line)
			{
				MapGIS::Tile::LinesModel* pLineModel = dynamic_cast<MapGIS::Tile::LinesModel *>(pModel);
				if (pLineModel != NULL)
				{
					for (vector<LineFeature>::iterator itr = pLineModel->features.begin(); itr != pLineModel->features.end(); itr++)
					{
						if (itr->id >= 0 && itr->id < oids.size())
							itr->id = oids[itr->id];
						else
							itr->id = -1;
					}
				}
			}
			else if (pModel->GetGeometryType() == GeometryType::Surface)
			{
				MapGIS::Tile::SurfacesModel * pSurfacesModel = dynamic_cast<MapGIS::Tile::SurfacesModel *>(pModel);
				if (pSurfacesModel != NULL)
				{
					for (vector<SurfaceFeature>::iterator itr = pSurfacesModel->features.begin(); itr != pSurfacesModel->features.end(); itr++)
					{
						if (itr->ids.size() > 0)
						{
							itr->id = -1;
							for (vector<gisINT64>::iterator oidItr = itr->ids.begin(); oidItr != itr->ids.end(); oidItr++)
							{
								if (*oidItr >= 0 && *oidItr < oids.size())
									*oidItr = oids[*oidItr];
								else
									*oidItr = -1;
							}
						}
						else
						{
							if (itr->id >= 0 && itr->id < oids.size())
								itr->id = oids[itr->id];
							else
								itr->id = -1;
						}
					}
				}
			}
			else if (pModel->GetGeometryType() == GeometryType::Entity)
			{
				MapGIS::Tile::EntitiesModel * pEntitiesModel = dynamic_cast<MapGIS::Tile::EntitiesModel *>(pModel);
				if (pEntitiesModel != NULL)
				{
					for (vector<EntityFeature>::iterator itr = pEntitiesModel->features.begin(); itr != pEntitiesModel->features.end(); itr++)
					{
						if (itr->ids.size() > 0)
						{
							itr->id = -1;
							for (vector<gisINT64>::iterator oidItr = itr->ids.begin(); oidItr != itr->ids.end(); oidItr++)
							{
								if (*oidItr >= 0 && *oidItr < oids.size())
									*oidItr = oids[*oidItr];
								else
									*oidItr = -1;
							}
						}
						else
						{
							if (itr->id >= 0 && itr->id < oids.size())
								itr->id = oids[itr->id];
							else
								itr->id = -1;
						}
					}
				}
			}
		}
	}
	return 1;
}