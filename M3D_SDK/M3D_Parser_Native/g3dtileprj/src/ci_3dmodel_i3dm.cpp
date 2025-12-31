#include "stdafx.h"
#include "ci_3dmodel.h"
#include "ci_assist.h"
#include "cgfile.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/rapidjson.h"
#include <algorithm> // 包含 std::find

using namespace MapGIS::Tile;

class Ci_FeatureTableKeyValue
{
public:
	Ci_FeatureTableKeyValue();
	~Ci_FeatureTableKeyValue();

	void SetByteOffset(CGByteArray& writer);
	void WriteJson(rapidjson::Value& jsObj, rapidjson::Document::AllocatorType& allocator, bool outputComponentType = true);

protected:
	string m_key;
	string m_componentType;
	int m_componentCount;
	int m_componentTypeSize;
	int m_byteOffset;
};

class Ci_FeatureTableKeyValueFloat :public Ci_FeatureTableKeyValue
{
public:
	Ci_FeatureTableKeyValueFloat(string key, int componentCount = 3);
	~Ci_FeatureTableKeyValueFloat();

	void WriteBinary(CGByteArray& writer);

	vector<float> m_values;
};

class Ci_FeatureTableKeyValueUShort :public Ci_FeatureTableKeyValue
{
public:
	Ci_FeatureTableKeyValueUShort(string key, int componentCount = 3);
	~Ci_FeatureTableKeyValueUShort();

	void WriteBinary(CGByteArray& writer);

	vector<unsigned short> m_values;
};

class Ci_FeatureTableKeyValueUInt :public Ci_FeatureTableKeyValue
{
public:
	Ci_FeatureTableKeyValueUInt(string key, int componentCount = 3);
	~Ci_FeatureTableKeyValueUInt();

	void WriteBinary(CGByteArray& writer);

	vector<unsigned int> m_values;
};

class Ci_I3dmFeatureTableHeader
{
public:
	Ci_I3dmFeatureTableHeader();
	~Ci_I3dmFeatureTableHeader();

	CGByteArray GetBinaryData();
	void WriteJson(rapidjson::Value& jsObj, rapidjson::Document::AllocatorType& allocator);

	Ci_FeatureTableKeyValueFloat*  m_pPosition;
	Ci_FeatureTableKeyValueUShort* m_pPositionQuantized;
	Ci_FeatureTableKeyValueFloat*  m_pNormalUp;
	Ci_FeatureTableKeyValueFloat*  m_pNormalRight;
	Ci_FeatureTableKeyValueUShort* m_pNormalUpOCT32P;
	Ci_FeatureTableKeyValueUShort* m_pNormalRightOCT32P;
	Ci_FeatureTableKeyValueFloat*  m_pScale;
	Ci_FeatureTableKeyValueFloat*  m_pScaleNonUniform;
	Ci_FeatureTableKeyValueUInt*   m_pBatchId;
	Ci_FeatureTableKeyValueUInt*   m_pOId;

	unsigned int m_instancesLength;
	D_3DOT* m_pRtcCenter;
	MapGIS::Tile::Vector3D* m_pQuantizedVolumeOffset;
	MapGIS::Tile::Vector3D* m_pQuantizedVolumeScale;
	MapGIS::Tile::Vector3D* m_pEastNorthUp;
};

class Ci_I3dmFeatureTable :public Ci_3DTileFeatureTable
{
public:
	Ci_I3dmFeatureTableHeader m_featureTableHeader;

	CGString BuildJsonHeaderData();
};

class Ci_I3dmHeader :public Ci_3DTileModelHeader
{
public:
	Ci_I3dmHeader();
	~Ci_I3dmHeader();

	virtual unsigned int GetHeaderLength();

	void SetGltfFormat(unsigned int format);
	unsigned int GetGltfFormat();

	void WriteTo(CGByteArray& writer);

protected:
	unsigned int m_gltfFormat;
};

class Ci_3DTileI3dm
{
public:
	Ci_3DTileI3dm(CGString gltfFilePath);
	Ci_3DTileI3dm(const CGByteArray &gltfData);
	~Ci_3DTileI3dm();

	void SetRtcCenter(double x, double y, double z);
	void SetInstances(const vector<MapGIS::Tile::ModelInstance>& infos) { m_pInstance = &infos; };
	void SetAttribute(LayerAttribute* pRecords);

	//特殊属性
	//如果将属性设置为写在BathTable 中，则，m_idType 与m_resetID 设置无效，
	void SetIdInBathTable(bool inBathTable) { m_inBathTable = inBathTable; };
	void SetWriteIdType(WriteIdType idType) { m_idType = idType; };
	void SetResetID(bool resetID) { m_resetID = resetID; };

	void WriteTo(CGByteArray& writer);
private:
	void i_InitHead(vector<unsigned int >& batchIDToID);
private:
	Ci_I3dmHeader  m_header;
	Ci_I3dmFeatureTable m_featureTable;
	LayerAttribute* m_pRecords;
	CGByteArray m_gltfData;
	CGString    m_gltfFilePath;
	const vector<MapGIS::Tile::ModelInstance>*  m_pInstance;

	bool  m_inBathTable;
	WriteIdType m_idType;
	bool m_resetID;
};

#pragma region i3dm_feature_table_header
Ci_FeatureTableKeyValue::Ci_FeatureTableKeyValue()
{
	m_key = "";
	m_componentType = "";
	m_componentCount = 0;
	m_componentTypeSize = 0;
	m_byteOffset = 0;
}
Ci_FeatureTableKeyValue::~Ci_FeatureTableKeyValue()
{
}
void Ci_FeatureTableKeyValue::SetByteOffset(CGByteArray& writer)
{
	int perSize = m_componentTypeSize;
	int remainder = writer.length() % perSize;
	if (remainder > 0)
	{
		// padding
		writer.append(perSize - remainder, ' ');
	}
	m_byteOffset = writer.length();
	return;
}
void Ci_FeatureTableKeyValue::WriteJson(rapidjson::Value& jsObj, rapidjson::Document::AllocatorType& allocator, bool outputComponentType)
{
	rapidjson::Value objBodyRef(rapidjson::kObjectType);
	objBodyRef.AddMember("byteOffset", m_byteOffset, allocator);

	if (outputComponentType && m_componentType != "FLOAT")
	{
		objBodyRef.AddMember("componentType", ToStringValue(m_componentType, allocator), allocator);
	}
	jsObj.AddMember(rapidjson::Value::StringRefType(m_key.c_str()), objBodyRef, allocator);
	return;
}

Ci_FeatureTableKeyValueFloat::Ci_FeatureTableKeyValueFloat(string key, int componentCount)
{
	m_key = key;
	m_componentType = "FLOAT";
	m_componentCount = componentCount;
	m_componentTypeSize = 4;
}
Ci_FeatureTableKeyValueFloat::~Ci_FeatureTableKeyValueFloat()
{
}
void Ci_FeatureTableKeyValueFloat::WriteBinary(CGByteArray& writer)
{
	SetByteOffset(writer);
	writer.append((const char*)&m_values[0], m_values.size() * 4);
}

Ci_FeatureTableKeyValueUShort::Ci_FeatureTableKeyValueUShort(string key, int componentCount)
{
	m_key = key;
	m_componentType = "UNSIGNED_SHORT";
	m_componentCount = componentCount;
	m_componentTypeSize = 2;
}
Ci_FeatureTableKeyValueUShort::~Ci_FeatureTableKeyValueUShort()
{
}
void Ci_FeatureTableKeyValueUShort::WriteBinary(CGByteArray& writer)
{
	SetByteOffset(writer);
	writer.append((const char*)&m_values[0], m_values.size() * 2);
}

Ci_FeatureTableKeyValueUInt::Ci_FeatureTableKeyValueUInt(string key, int componentCount)
{
	m_key = key;
	m_componentType = "UNSIGNED_INT";
	m_componentCount = componentCount;
	m_componentTypeSize = 4;
}
Ci_FeatureTableKeyValueUInt::~Ci_FeatureTableKeyValueUInt()
{
}
void Ci_FeatureTableKeyValueUInt::WriteBinary(CGByteArray& writer)
{
	SetByteOffset(writer);
	writer.append((const char*)&m_values[0], m_values.size() * 4);
}

Ci_I3dmFeatureTableHeader::Ci_I3dmFeatureTableHeader()
{
	m_pPosition = NULL;
	m_pPositionQuantized = NULL;
	m_pNormalUp = NULL;
	m_pNormalRight = NULL;
	m_pNormalUpOCT32P = NULL;
	m_pNormalRightOCT32P = NULL;
	m_pScale = NULL;
	m_pScaleNonUniform = NULL;
	m_pBatchId = NULL;
	m_pOId = NULL;

	m_instancesLength = 0;
	m_pRtcCenter = NULL;
	m_pQuantizedVolumeOffset = NULL;
	m_pQuantizedVolumeScale = NULL;
	m_pEastNorthUp = NULL;
}
Ci_I3dmFeatureTableHeader::~Ci_I3dmFeatureTableHeader()
{
	SAFE_DELETE_PTR(m_pPosition);
	SAFE_DELETE_PTR(m_pPositionQuantized);
	SAFE_DELETE_PTR(m_pNormalUp);
	SAFE_DELETE_PTR(m_pNormalRight);
	SAFE_DELETE_PTR(m_pNormalUpOCT32P);
	SAFE_DELETE_PTR(m_pNormalRightOCT32P);
	SAFE_DELETE_PTR(m_pScale);
	SAFE_DELETE_PTR(m_pScaleNonUniform);
	SAFE_DELETE_PTR(m_pBatchId);
	SAFE_DELETE_PTR(m_pOId);
	SAFE_DELETE_PTR(m_pRtcCenter);
	SAFE_DELETE_PTR(m_pQuantizedVolumeOffset);
	SAFE_DELETE_PTR(m_pQuantizedVolumeScale);
	SAFE_DELETE_PTR(m_pEastNorthUp);
}

CGByteArray Ci_I3dmFeatureTableHeader::GetBinaryData()
{
	CGByteArray byteArr;
	if (m_pPosition)
	{
		m_pPosition->WriteBinary(byteArr);
	}
	if (m_pPositionQuantized)
	{
		m_pPositionQuantized->WriteBinary(byteArr);
	}
	if (m_pNormalUp)
	{
		m_pNormalUp->WriteBinary(byteArr);
	}
	if (m_pNormalRight)
	{
		m_pNormalRight->WriteBinary(byteArr);
	}
	if (m_pNormalUpOCT32P)
	{
		m_pNormalUpOCT32P->WriteBinary(byteArr);
	}
	if (m_pNormalRightOCT32P)
	{
		m_pNormalRightOCT32P->WriteBinary(byteArr);
	}
	if (m_pScale)
	{
		m_pScale->WriteBinary(byteArr);
	}
	if (m_pScaleNonUniform)
	{
		m_pScaleNonUniform->WriteBinary(byteArr);
	}

	if (m_pBatchId)
	{
		m_pBatchId->WriteBinary(byteArr);
	}

	if (m_pOId)
	{
		m_pOId->WriteBinary(byteArr);
	}
	return byteArr;
}

void Ci_I3dmFeatureTableHeader::WriteJson(rapidjson::Value& jsObj, rapidjson::Document::AllocatorType& allocator)
{
	if (!jsObj.IsObject())
		return;

	jsObj.AddMember("INSTANCES_LENGTH", (int)m_instancesLength, allocator);

	if (m_pPosition)
	{
		m_pPosition->WriteJson(jsObj, allocator);
	}
	if (m_pPositionQuantized)
	{
		m_pPositionQuantized->WriteJson(jsObj, allocator);
	}
	if (m_pNormalUp)
	{
		m_pNormalUp->WriteJson(jsObj, allocator);
	}
	if (m_pNormalRight)
	{
		m_pNormalRight->WriteJson(jsObj, allocator);
	}
	if (m_pNormalUpOCT32P)
	{
		m_pNormalUpOCT32P->WriteJson(jsObj, allocator);
	}
	if (m_pNormalRightOCT32P)
	{
		m_pNormalRightOCT32P->WriteJson(jsObj, allocator);
	}
	if (m_pScale)
	{
		m_pScale->WriteJson(jsObj, allocator);
	}
	if (m_pScaleNonUniform)
	{
		m_pScaleNonUniform->WriteJson(jsObj, allocator);
	}
	if (m_pBatchId)
	{
		m_pBatchId->WriteJson(jsObj, allocator);
	}
	if (m_pOId)
	{
		m_pOId->WriteJson(jsObj, allocator);
	}
	if (m_pRtcCenter)
	{
		rapidjson::Value jsVal(rapidjson::kArrayType);
		jsVal.PushBack(m_pRtcCenter->x, allocator);
		jsVal.PushBack(m_pRtcCenter->y, allocator);
		jsVal.PushBack(m_pRtcCenter->z, allocator);
		jsObj.AddMember("RTC_CENTER", jsVal, allocator);
	}
	if (m_pQuantizedVolumeOffset)
	{
		rapidjson::Value jsVal(rapidjson::kArrayType);
		jsVal.PushBack(m_pQuantizedVolumeOffset->x, allocator);
		jsVal.PushBack(m_pQuantizedVolumeOffset->y, allocator);
		jsVal.PushBack(m_pQuantizedVolumeOffset->z, allocator);
		jsObj.AddMember("QUANTIZED_VOLUME_OFFSET", jsVal, allocator);
	}
	if (m_pQuantizedVolumeScale)
	{
		rapidjson::Value jsVal(rapidjson::kArrayType);
		jsVal.PushBack(m_pQuantizedVolumeScale->x, allocator);
		jsVal.PushBack(m_pQuantizedVolumeScale->y, allocator);
		jsVal.PushBack(m_pQuantizedVolumeScale->z, allocator);
		jsObj.AddMember("QUANTIZED_VOLUME_SCALE", jsVal, allocator);
	}
	if (m_pEastNorthUp)
	{
		rapidjson::Value jsVal(rapidjson::kArrayType);
		jsVal.PushBack(m_pEastNorthUp->x, allocator);
		jsVal.PushBack(m_pEastNorthUp->y, allocator);
		jsVal.PushBack(m_pEastNorthUp->z, allocator);
		jsObj.AddMember("EAST_NORTH_UP", jsVal, allocator);
	}
	return;
}
#pragma endregion

#pragma region I3dmFeatureTable
CGString Ci_I3dmFeatureTable::BuildJsonHeaderData()
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	m_featureTableHeader.WriteJson(doc, allocator);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	CGString strJsonData = CGString(buffer.GetString(), buffer.GetLength(),CGString::EncodeType::UTF8);
	return strJsonData;
}
#pragma endregion

#pragma region I3dmHeader
Ci_I3dmHeader::Ci_I3dmHeader()
{
	m_magic = "i3dm";
	m_gltfFormat = 1;
	m_version = 1;
}
Ci_I3dmHeader::~Ci_I3dmHeader()
{
}
unsigned int Ci_I3dmHeader::GetHeaderLength()
{
	return 32;
}
void Ci_I3dmHeader::SetGltfFormat(unsigned int format)
{
	m_gltfFormat = format;
}
unsigned int Ci_I3dmHeader::GetGltfFormat()
{
	return m_gltfFormat;
}
void Ci_I3dmHeader::WriteTo(CGByteArray& writer)
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
	tmpWriter.append((const char*)&m_gltfFormat, 4);

	writer.replace(0, GetHeaderLength(), tmpWriter.constData(), GetHeaderLength());
}
#pragma endregion

#pragma region Ci_3DTiles_I3dm
Ci_3DTileI3dm::Ci_3DTileI3dm(CGString gltfFilePath)
{
	m_gltfFilePath = gltfFilePath;
	m_pRecords = NULL;
	m_gltfData.clear();
	m_pInstance = NULL;
	m_resetID = true;
	m_inBathTable = true;
	m_idType = WriteIdType::batchID;
}

Ci_3DTileI3dm::Ci_3DTileI3dm(const CGByteArray &gltfData)
{
	m_gltfFilePath = "";
	m_pRecords = NULL;
	m_gltfData.clear();
	m_gltfData.append(gltfData);
	m_pInstance = NULL;
	m_inBathTable = true;
	m_idType = WriteIdType::batchID;
}

Ci_3DTileI3dm::~Ci_3DTileI3dm()
{
}

void Ci_3DTileI3dm::SetRtcCenter(double x, double y, double z)
{
	if (NULL == m_featureTable.m_featureTableHeader.m_pRtcCenter)
		m_featureTable.m_featureTableHeader.m_pRtcCenter = new D_3DOT();

	m_featureTable.m_featureTableHeader.m_pRtcCenter->x = x;
	m_featureTable.m_featureTableHeader.m_pRtcCenter->y = y;
	m_featureTable.m_featureTableHeader.m_pRtcCenter->z = z;
	return;
}

void Ci_3DTileI3dm::SetAttribute(LayerAttribute* pRecords)
{
	m_pRecords = pRecords;
}

void Ci_3DTileI3dm::i_InitHead(vector<unsigned int>& indexToID)
{
	if (m_pInstance == NULL ||  m_pInstance->size() <= 0)
		return;

	const MapGIS::Tile::ModelInstance & instance = m_pInstance->at(0);
	bool hasNormalUp = instance.hasNormalUp;
	bool hasNormalRight = instance.hasNormalRight;
	bool hasScaleNonUniform = instance.hasScaleNonUniform;
	bool hasScale = instance.hasScale;

	bool hasId = false;

	if (m_inBathTable || m_resetID)
	{
		for (auto itr = m_pInstance->begin(); itr != m_pInstance->end(); itr++)
		{
			if (itr->hasId)
			{
				if (!hasId)
					hasId = true;
				if (find(indexToID.begin(), indexToID.end(), itr->id) == indexToID.end())
					indexToID.emplace_back(itr->id);
			}
		}
	}
	for (auto itr = m_pInstance->begin(); itr != m_pInstance->end(); itr++)
	{
		m_featureTable.m_featureTableHeader.m_instancesLength++;
		if (!m_featureTable.m_featureTableHeader.m_pPosition)
			m_featureTable.m_featureTableHeader.m_pPosition = new Ci_FeatureTableKeyValueFloat("POSITION");
		m_featureTable.m_featureTableHeader.m_pPosition->m_values.push_back(itr->position.x);
		m_featureTable.m_featureTableHeader.m_pPosition->m_values.push_back(itr->position.y);
		m_featureTable.m_featureTableHeader.m_pPosition->m_values.push_back(itr->position.z);

		if (hasNormalUp)
		{
			if (!m_featureTable.m_featureTableHeader.m_pNormalUp)
				m_featureTable.m_featureTableHeader.m_pNormalUp = new Ci_FeatureTableKeyValueFloat("NORMAL_UP");
			if (itr->hasNormalUp)
			{
				m_featureTable.m_featureTableHeader.m_pNormalUp->m_values.push_back(itr->normalUp.x);
				m_featureTable.m_featureTableHeader.m_pNormalUp->m_values.push_back(itr->normalUp.y);
				m_featureTable.m_featureTableHeader.m_pNormalUp->m_values.push_back(itr->normalUp.z);
			}
			else
			{
				m_featureTable.m_featureTableHeader.m_pNormalUp->m_values.push_back(0);
				m_featureTable.m_featureTableHeader.m_pNormalUp->m_values.push_back(0);
				m_featureTable.m_featureTableHeader.m_pNormalUp->m_values.push_back(0);
			}
		}
		if (hasNormalRight)
		{
			if (!m_featureTable.m_featureTableHeader.m_pNormalRight)
				m_featureTable.m_featureTableHeader.m_pNormalRight = new Ci_FeatureTableKeyValueFloat("NORMAL_RIGHT");
			if (itr->hasScaleNonUniform)
			{
				m_featureTable.m_featureTableHeader.m_pNormalRight->m_values.push_back(itr->normalRight.x);
				m_featureTable.m_featureTableHeader.m_pNormalRight->m_values.push_back(itr->normalRight.y);
				m_featureTable.m_featureTableHeader.m_pNormalRight->m_values.push_back(itr->normalRight.z);
			}
			else
			{
				m_featureTable.m_featureTableHeader.m_pNormalRight->m_values.push_back(0);
				m_featureTable.m_featureTableHeader.m_pNormalRight->m_values.push_back(0);
				m_featureTable.m_featureTableHeader.m_pNormalRight->m_values.push_back(0);
			}
		}

		if (hasScaleNonUniform)
		{
			if (!m_featureTable.m_featureTableHeader.m_pScaleNonUniform)
				m_featureTable.m_featureTableHeader.m_pScaleNonUniform = new Ci_FeatureTableKeyValueFloat("SCALE_NON_UNIFORM");
			if (itr->hasScaleNonUniform)
			{
				m_featureTable.m_featureTableHeader.m_pScaleNonUniform->m_values.push_back(itr->scaleNonUniform.x);
				m_featureTable.m_featureTableHeader.m_pScaleNonUniform->m_values.push_back(itr->scaleNonUniform.y);
				m_featureTable.m_featureTableHeader.m_pScaleNonUniform->m_values.push_back(itr->scaleNonUniform.z);
			}
			else
			{
				m_featureTable.m_featureTableHeader.m_pScaleNonUniform->m_values.push_back(0);
				m_featureTable.m_featureTableHeader.m_pScaleNonUniform->m_values.push_back(0);
				m_featureTable.m_featureTableHeader.m_pScaleNonUniform->m_values.push_back(0);
			}
		}

		if (hasScale)
		{
			if (!m_featureTable.m_featureTableHeader.m_pScale)
				m_featureTable.m_featureTableHeader.m_pScale = new Ci_FeatureTableKeyValueFloat("SCALE",1);
			if (itr->hasScale)
			{
				m_featureTable.m_featureTableHeader.m_pScale->m_values.push_back(itr->scale);
			}
			else
			{
				m_featureTable.m_featureTableHeader.m_pScale->m_values.push_back(1);
			}
		}

		if (hasId)
		{
			if (m_inBathTable)
			{
				if (!m_featureTable.m_featureTableHeader.m_pBatchId)
					m_featureTable.m_featureTableHeader.m_pBatchId = new Ci_FeatureTableKeyValueUInt("BATCH_ID", 1);
				vector<unsigned int> ::iterator idItr = find(indexToID.begin(), indexToID.end(), itr->id);
				if (idItr != indexToID.end())
				{
					int index = std::distance(indexToID.begin(), idItr);
					m_featureTable.m_featureTableHeader.m_pBatchId->m_values.push_back(index);
				}
				else
					m_featureTable.m_featureTableHeader.m_pBatchId->m_values.push_back(indexToID.size());
			}else
			{
				if (m_resetID)
				{
					if (m_idType == WriteIdType::batchID)
					{
						if (!m_featureTable.m_featureTableHeader.m_pBatchId)
							m_featureTable.m_featureTableHeader.m_pBatchId = new Ci_FeatureTableKeyValueUInt("BATCH_ID", 1);
						vector<unsigned int> ::iterator idItr = find(indexToID.begin(), indexToID.end(), itr->id);
						if (idItr != indexToID.end())
						{
							int index = std::distance(indexToID.begin(), idItr);
							m_featureTable.m_featureTableHeader.m_pBatchId->m_values.push_back(index);
						}
						else
							m_featureTable.m_featureTableHeader.m_pBatchId->m_values.push_back(indexToID.size());
					}
					else if (m_idType == WriteIdType::oid)
					{
						if (!m_featureTable.m_featureTableHeader.m_pOId)
							m_featureTable.m_featureTableHeader.m_pOId = new Ci_FeatureTableKeyValueUInt("oid", 1);
						vector<unsigned int> ::iterator idItr = find(indexToID.begin(), indexToID.end(), itr->id);
						if (idItr != indexToID.end())
						{
							int index = std::distance(indexToID.begin(), idItr);
							m_featureTable.m_featureTableHeader.m_pOId->m_values.push_back(index);
						}
						else
							m_featureTable.m_featureTableHeader.m_pOId->m_values.push_back(indexToID.size());
					}
				}
				else
				{
					if (m_idType == WriteIdType::batchID)
					{
						if (!m_featureTable.m_featureTableHeader.m_pBatchId)
							m_featureTable.m_featureTableHeader.m_pBatchId = new Ci_FeatureTableKeyValueUInt("BATCH_ID", 1);
						m_featureTable.m_featureTableHeader.m_pBatchId->m_values.push_back(itr->id);
					}
					else if (m_idType == WriteIdType::oid)
					{
						if (!m_featureTable.m_featureTableHeader.m_pOId)
							m_featureTable.m_featureTableHeader.m_pOId = new Ci_FeatureTableKeyValueUInt("oid", 1);
						m_featureTable.m_featureTableHeader.m_pOId->m_values.push_back(itr->id);
					}
				}
			}
		}
	}
}

void Ci_3DTileI3dm::WriteTo(CGByteArray& writer)
{
	vector<unsigned int> indexToID;
	i_InitHead(indexToID);
	LayerAttribute records;
	LayerAttribute* pRecords = m_pRecords;

	if (indexToID.size() > 0 && m_pRecords != NULL &&m_pRecords->records.size() > 0)
	{
		int fieldNum = m_pRecords->layerInfo.fieldInfos.size();
		records.layerInfo = m_pRecords->layerInfo;
		records.records.reserve(m_pInstance->size());

		unordered_map<gisINT64, gisINT64> idIndex;
		for (int i = 0; i < m_pRecords->records.size(); i++)
		{
			if (idIndex.find(m_pRecords->records[i].GetID()) == idIndex.end())
				idIndex.insert(make_pair(m_pRecords->records[i].GetID(), i));
		}

		for (int i = 0; i < m_pInstance->size(); i++)
		{
			if (indexToID.size() > i && idIndex.find(indexToID[i]) != idIndex.end())
			{
				Record record = m_pRecords->records[idIndex[indexToID[i]]];
				record.SetID(i);
				records.records.emplace_back(record);
			}
			else
			{
				Record record;
				record.SetID(indexToID.size());
				for (int j = 0; j < fieldNum; j++)
					record.AppendItem(m_pRecords->layerInfo.fieldInfos[j].type);
				records.records.emplace_back(record);
			}
		}
		pRecords = &records;
	}

	//1 placeholder for model header
	writer.append(m_header.GetHeaderLength(), ' ');

	//2 feature table
	{
		CGByteArray data = m_featureTable.m_featureTableHeader.GetBinaryData();
		// 2ed: feature table json header
		CGString jsonData = m_featureTable.BuildJsonHeaderData();
		jsonData.Convert(CGString::UTF8);
		m_header.SetFeatureTableJSONLengthNoPadding(jsonData.GetLength());
		writer.append(jsonData.CStr(), jsonData.GetLength());
		if (m_header.GetFeatureTableJSONPaddingCount() > 0)
			writer.append(m_header.GetFeatureTableJSONPaddingCount(), ' ');

		// 3rd: write feature table binary data
		if (data.length() > 0)
		{
			m_header.SetFeatureTableBinaryLengthNoPadding(data.length());
			writer.append(data.constData(), data.length());
			if (m_header.GetFeatureTableBinaryPaddingCount() > 0)
				writer.append(m_header.GetFeatureTableBinaryPaddingCount(), ' ');
		}
	}

	if (m_inBathTable)
	{
		//3 batch table
		if (pRecords != NULL && pRecords->records.size() > 0)
		{
			CGByteArray binData;
			CGByteArray btJsData;
			Ci_3DTileBatchTable batchTable;

			batchTable.WriteTo(pRecords, &indexToID, btJsData, binData, "OID" );

			m_header.SetBatchTableJSONLengthNoPadding(btJsData.length());
			writer.append(btJsData);
			if (m_header.GetBatchTableJSONPaddingCount() > 0)
				writer.append(m_header.GetBatchTableJSONPaddingCount(), ' ');

			if (binData.length() > 0)
			{
				m_header.SetBatchTableBinaryLengthNoPadding(binData.length());
				writer.append(binData);
				if (m_header.GetBatchTableBinaryPaddingCount() > 0)
					writer.append(m_header.GetBatchTableBinaryPaddingCount(), ' ');
			}
		}
		else
		{
			CGByteArray btJsData;
			btJsData.append("{\"OID\":[");
			for (int i = 0; i < m_pInstance->size(); i++)
			{
				if (indexToID.size() > i)
					btJsData.append(std::to_string(indexToID[i]).c_str());
				else
					btJsData.append(std::to_string(-1).c_str());

				if (i != m_pInstance->size() - 1)
					btJsData.append(",");
			}
			btJsData.append("]}");

			m_header.SetBatchTableJSONLengthNoPadding(btJsData.length());
			writer.append(btJsData);
			if (m_header.GetBatchTableJSONPaddingCount() > 0)
				writer.append(m_header.GetBatchTableJSONPaddingCount(), ' ');
		}
	}

	//4 gltfdata
	if (m_gltfFilePath.GetLength() > 0)
	{
		ifstream inFile;
		CGString strPath = m_gltfFilePath;
		strPath.Convert(CGString::GB18030);
		m_gltfData = CGFile::ReadAllBytes(strPath);
		int nRemainLen = m_gltfData.length() % 8;
		if (nRemainLen > 0)
			m_gltfData.append(8 - nRemainLen, ' ');
	}
	else if (m_gltfData.length() > 0)
	{
		int nRemainLen = m_gltfData.length() % 8;
		if (nRemainLen > 0)
			m_gltfData.append(8 - nRemainLen, ' ');
	}

	//5 update header
	writer.append(m_gltfData.constData(), m_gltfData.length());
	unsigned int byteLen = m_header.GetHeaderLength() +
		m_header.GetFeatureTableJSONLength() +
		m_header.GetFeatureTableBinaryLength() +
		m_header.GetBatchTableJSONLength() +
		m_header.GetBatchTableBinaryLength() +
		(unsigned int)m_gltfData.length();

	m_header.SetByteLength(byteLen);
	m_header.WriteTo(writer);

	//测试用
	int nByteLen1 = writer.length();
	if (nByteLen1 % 8 > 0)
	{
		int a = 0;
	}
}
#pragma endregion

gisLONG Ci_ModelI3dm::From(MapGIS::Tile::G3DModel *pModel, const vector<MapGIS::Tile::ModelInstance>& infos, LayerAttribute* pRecords, GeoCompressType compressType, D_3DOT* pRtcCenter)
{
	if (pModel  == NULL || infos.size() <= 0)
		return 0;

	Ci_ModelGltf gltf;
	gltf.From(pModel, compressType, MapGIS::Tile::WriteIdType::None);
	const CGByteArray *gltfData= gltf.Get();

	Ci_3DTileI3dm i3dm(*gltfData);
	i3dm.SetInstances(infos);
	i3dm.SetIdInBathTable(true);
	i3dm.SetWriteIdType(WriteIdType::batchID);
	i3dm.SetResetID(true);
	if (pRtcCenter != NULL)
	{
		i3dm.SetRtcCenter(pRtcCenter->x, pRtcCenter->y, pRtcCenter->z);
	}
	if (pRecords != NULL)
	{
		i3dm.SetAttribute(pRecords);
	}
	i3dm.WriteTo(m_data);
	return 1;
}

//专共M3D 2.1  与3D Tiles 1.0 Ext 使用，
gisLONG Ci_ModelI3dm::From(MapGIS::Tile::G3DModel *pModel,  const vector<MapGIS::Tile::ModelInstance>& infos, GeoCompressType compressType, D_3DOT* pRtcCenter, bool resetID, WriteIdType idType, vector<gisINT64>& batchIDToId)
{
	if (pModel == NULL || infos.size() <= 0)
		return 0;

	Ci_ModelGltf gltf;
	gltf.From(pModel, compressType, MapGIS::Tile::WriteIdType::None);
	const CGByteArray *gltfData = gltf.Get();

	Ci_3DTileI3dm i3dm(*gltfData);
	i3dm.SetInstances(infos);
	i3dm.SetIdInBathTable(false);
	i3dm.SetWriteIdType(idType);
	i3dm.SetResetID(resetID);
	if (pRtcCenter != NULL)
	{
		i3dm.SetRtcCenter(pRtcCenter->x, pRtcCenter->y, pRtcCenter->z);
	}
	i3dm.WriteTo(m_data);

	if (resetID)
	{
		bool existNoId = false;
		for (auto itr = infos.begin(); itr != infos.end(); itr++)
		{
			if (itr->hasId)
			{
				if (find(batchIDToId.begin(), batchIDToId.end(), itr->id) == batchIDToId.end())
					batchIDToId.emplace_back(itr->id);
			}
			else
				existNoId = true;
		}
		if (existNoId)
			batchIDToId.emplace_back(-1);
	}
	return 1;
}
//默认 normalOct32p 为两个UINT16 的数组
void octDecodeInRange(gisUSHORT* normalOct32p, vector<float>&normal)
{
	double x = normalOct32p[0] / 65535 * 2 - 1;
	double y = normalOct32p[1] / 65535 * 2 - 1;
	double z = 1.0 - (fabs(x) + fabs(y));
	if (z < 0)
	{
		double oldVX = x;
		x = (1.0 - fabs(y)) *  (oldVX < 0 ? -1 : 1);
		y = (1.0 - fabs(oldVX)) *  (y < 0 ? -1 : 1);
	}
	normal.emplace_back(x);
	normal.emplace_back(y);
	normal.emplace_back(z);
}

gisLONG Ci_ModelI3dm::To(MapGIS::Tile::G3DModel *pModel, vector<MapGIS::Tile::ModelInstance>& infos, LayerAttribute& records, D_3DOT& rtcCenter)
{
	if (m_data.size() <= 0)
		return  0;

	int c = 0;
	string magic = ReadByteArrayToString(m_data, c, 4); c += 4;
	int version = ReadByteArrayToInt32(m_data.data(), c); c += 4;
	int i3dmBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int featureTableJsonBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int featureTableBinBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int batchTableJsonBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int batchTableBinaBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int gltfFormat = ReadByteArrayToInt32(m_data, c); c += 4;

	if (StrICmp("i3dm", magic.c_str()) != 0)
		return 0;

	if (m_data.size() < i3dmBytesLength)
		return 0;

	CGByteArray featureTableJsonByteArray;
	CGByteArray featureTableBinByteArray;
	CGByteArray batchTableJsonByteArray;
	CGByteArray batchTableBinByteArray;
	CGByteArray gltfByteArray;
	ReadByteArrayToByteArray(m_data, c, featureTableJsonBytesLength, featureTableJsonByteArray);
	c += featureTableJsonBytesLength;

	ReadByteArrayToByteArray(m_data, c, featureTableBinBytesLength, featureTableBinByteArray);
	c += featureTableBinBytesLength;

	ReadByteArrayToByteArray(m_data, c, batchTableJsonBytesLength, batchTableJsonByteArray);
	c += batchTableJsonBytesLength;

	ReadByteArrayToByteArray(m_data, c, batchTableBinaBytesLength, batchTableBinByteArray);
	c += batchTableBinaBytesLength;
	ReadByteArrayToByteArray(m_data, c, i3dmBytesLength - c, gltfByteArray);

	vector<int> outInstancesLength;
	int instancesLength = 0;

	vector<double> outRtcCenter;
	vector<float> outQuantizedVolumeOffset;
	vector<float> outQuantizedVolumeScale;

	vector<float> outPosition;
	vector<gisUSHORT> outPositionQuantized;
	vector<float> outNormalUp;
	vector<float> outNormalRight;
	vector<gisUSHORT> outNormalUpOct32p;
	vector<gisUSHORT> outNormalRightOct32p;
	vector<float> outScale;
	vector<float> outScaleNonUniform;
	vector<gisUINT> outBatchID;
	vector<gisUINT> outOid;

	rapidjson::Document doc;
	if (doc.Parse(featureTableJsonByteArray.data(), featureTableJsonByteArray.size()).HasParseError())
		return 0;
	if(!doc.IsObject())
		return 0;

	GetTableValue(doc, "INSTANCES_LENGTH", 1, featureTableBinByteArray, outInstancesLength);

	if (outInstancesLength.size() == 1)
		instancesLength = outInstancesLength[0];
	if (instancesLength <= 0)
		return 0;

	if (batchTableJsonBytesLength > 0)
	{
		Ci_3DTileBatchTable batchTable;
		batchTable.ReadFrom(batchTableJsonByteArray, batchTableBinByteArray, "OID", instancesLength, records);
	}

	GetTableValue(doc, "RTC_CENTER", 3, featureTableBinByteArray, outRtcCenter);
	GetTableValue(doc, "QUANTIZED_VOLUME_OFFSET", 3, featureTableBinByteArray, outQuantizedVolumeOffset);
	GetTableValue(doc, "QUANTIZED_VOLUME_SCALE", 3, featureTableBinByteArray, outQuantizedVolumeScale);

	GetTableValue(doc, "POSITION", instancesLength * 3, featureTableBinByteArray, outPosition);
	GetTableValue(doc, "POSITION_QUANTIZED", instancesLength * 3, featureTableBinByteArray, outPositionQuantized);
	GetTableValue(doc, "NORMAL_UP", instancesLength * 3, featureTableBinByteArray, outNormalUp);
	GetTableValue(doc, "NORMAL_RIGHT", instancesLength * 3, featureTableBinByteArray, outNormalRight);
	GetTableValue(doc, "NORMAL_UP_OCT32P", instancesLength * 2, featureTableBinByteArray, outNormalUpOct32p);
	GetTableValue(doc, "NORMAL_RIGHT_OCT32P", instancesLength * 2, featureTableBinByteArray, outNormalRightOct32p);
	GetTableValue(doc, "SCALE", instancesLength, featureTableBinByteArray, outScale);
	GetTableValue(doc, "SCALE_NON_UNIFORM", instancesLength * 3, featureTableBinByteArray, outScaleNonUniform);
	GetTableValue(doc, "BATCH_ID", instancesLength, featureTableBinByteArray, outBatchID);
	GetTableValue(doc, "oid", instancesLength, featureTableBinByteArray, outOid);

	if (outRtcCenter.size() == 3)
	{
		rtcCenter.x = outRtcCenter[0];
		rtcCenter.y = outRtcCenter[1];
		rtcCenter.z = outRtcCenter[2];
	}

	if (outPosition.size() <= 0)
	{
		if (outPositionQuantized.size() <= 0 || outQuantizedVolumeOffset.size() <= 0 || outQuantizedVolumeScale.size() <= 0)
			return  0;
		outPosition.resize(instancesLength * 3);
		for (int i = 0; i < instancesLength; i++) {
			for (int j = 0; j < 3; j++) {
				outPosition[i * 3 + j] = (outPositionQuantized[i * 3 + j] / 65535.0) * outQuantizedVolumeScale[j] + outQuantizedVolumeOffset[j];
			}
		}
	}

	if (outNormalUp.size() <= 0 && outNormalUpOct32p.size() > 0)
	{
		if (outNormalRightOct32p.size() == outNormalUpOct32p.size())
		{
			outNormalUp.clear();
			outNormalRight.clear();
			for (int i = 0; i < instancesLength; i++)
			{
				octDecodeInRange(&outNormalUpOct32p[i * 2], outNormalUp);
				octDecodeInRange(&outNormalRightOct32p[i * 2], outNormalRight);
			}
		}
	}

	for (int i = 0; i < instancesLength; i++)
	{
		MapGIS::Tile::ModelInstance instance;
		instance.position = D_3DOT{ outPosition[i * 3], outPosition[i * 3 + 1], outPosition[i * 3 + 2] };
		if (outNormalUp.size() > (i * 3 + 2))
		{
			instance.hasNormalUp = true;
			instance.normalUp = D_3DOT{ outNormalUp[i * 3], outNormalUp[i * 3 + 1], outNormalUp[i * 3 + 2] };

			instance.hasNormalRight = true;
			instance.normalRight = D_3DOT{ outNormalRight[i * 3], outNormalRight[i * 3 + 1], outNormalRight[i * 3 + 2] };
		}

		if (outScale.size() > i)
		{
			instance.hasScale = true;
			instance.scale = outScale[i];
		}

		if (outScaleNonUniform.size() > (i * 3 + 2))
		{
			instance.hasScaleNonUniform = true;
			instance.scaleNonUniform = D_3DOT{ outScaleNonUniform[i * 3], outScaleNonUniform[i * 3 + 1], outScaleNonUniform[i * 3 + 2] };
		}

		if (outBatchID.size() > i && records.records.size() > outBatchID[i])
		{
			instance.hasId = true;
			instance.id = records.records[outBatchID[i]].GetID();
		}
		else if (outBatchID.size() <= 0 && records.records.size() == instancesLength)
		{
			instance.hasId = true;
			instance.id = records.records[i].GetID();
		}
		else if (outBatchID.size() > i)
		{
			instance.hasId = true;

			instance.id = outBatchID[i];
		}
		else if (outOid.size() == instancesLength)
		{
			instance.hasId = true;
			instance.id = outOid[i];
		}
		infos.emplace_back(instance);
	}

	bool isNewStorage = false;
	G3DCacheStorage* pStorage = m_pStorage;
	if (gltfFormat == 0 && pStorage != NULL)
	{
		CGString path = CGString(gltfByteArray.data(), CGString::EncodeType::UTF8);
		gltfByteArray.clear();
		pStorage->GetContent(path, gltfByteArray);

		path.Replace('\\', '/');
		while (path.StartsWith("./"))
			path = path.Right(path.GetLength() - 2);
		while (path.StartsWith("/"))
			path = path.Right(path.GetLength() - 1);
		int index = path.ReverseFind('/');
		if (index <= 0)
			path = "";
		else
			path = path.Left(index);
		if (path.GetLength() > 0)
		{
			CGString rootRelative =  pStorage->GetRootRelative();
			CGString newPath =  MakePathByRelativePath(rootRelative, path);
			pStorage = G3DCacheStorage::CreateInstance(pStorage->GetRootUrl(), newPath);
			pStorage->Open();
			isNewStorage = true;
		}
	}
	if (gltfByteArray.size()>0)
	{
		Ci_ModelGltf gltf;
		((Ci_TileModel*)&gltf)->From(gltfByteArray);
		((Ci_TileModel*)&gltf)->SetCacheStorage(pStorage);
		gltf.To(pModel);
	}

	if (isNewStorage && pStorage != NULL)
	{
		delete pStorage;
		pStorage = NULL;
	}
	return 1;
}