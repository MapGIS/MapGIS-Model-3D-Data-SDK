#include "stdafx.h"
#include "ci_3dmodel_attribute_att.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/rapidjson.h"
#include "ci_assist.h"
#include <map>
#include <unordered_map>
#include <algorithm>

using namespace MapGIS::Tile;

struct FeatureIndexData
{
	FeatureIndexData()
	{
		tid = 0;
		layerIndex = 0;
		featureIndex = 0;
	}
	FeatureIndexData(gisINT64	 tid, unsigned int layerIndex, unsigned int featureIndex)
	{
		this->tid = tid;
		this->layerIndex = layerIndex;
		this->featureIndex = featureIndex;
	}
	gisINT64	 tid;
	unsigned int layerIndex;
	unsigned int featureIndex;
};

#pragma region Ci_ModelCacheAttWriter

class Ci_ModelCacheAttWriter
{
public:
	Ci_ModelCacheAttWriter();
	~Ci_ModelCacheAttWriter();

	int AppendLayerInfo(const LayerFieldsInfo& layer, unsigned int featureSize);
	int AppendFeatureIndexData(FeatureIndexData &data);
	int AppendFieldData(unsigned int layerID, unsigned int fieldID, const RecordItemValue* pData);
	int Save(AttributeCompressType type, CGByteArray &byteArray);
private:
	int i_CalcJsonAndBin(CGByteArray& json, CGByteArray& bin);
	int i_GetFieldData(unsigned int layerID, unsigned int fieldID, CGByteArray& outByteArray);
private:
	vector<FeatureIndexData> m_FeatureIndexData;
	vector<pair<LayerFieldsInfo, unsigned int> > m_LayerInfos;
	map<unsigned int, map<unsigned int, vector<const RecordItemValue*>>> m_FieldDatas;
};

Ci_ModelCacheAttWriter::Ci_ModelCacheAttWriter()
{
}

Ci_ModelCacheAttWriter::~Ci_ModelCacheAttWriter()
{
}

int Ci_ModelCacheAttWriter::AppendLayerInfo(const LayerFieldsInfo& layer, unsigned int featureSize)
{
	if (find_if(m_LayerInfos.begin(), m_LayerInfos.end(),
		[layer](pair<LayerFieldsInfo, unsigned int>& layerInfo) {return layerInfo.first.layerID == layer.layerID; }) != m_LayerInfos.end())
		return 1;

	m_LayerInfos.push_back(pair<LayerFieldsInfo, unsigned int>(layer, featureSize));
	return 1;
}

int Ci_ModelCacheAttWriter::AppendFeatureIndexData(FeatureIndexData &data)
{
	m_FeatureIndexData.emplace_back(data);
	return 1;
}

int Ci_ModelCacheAttWriter::AppendFieldData(unsigned int layerID, unsigned int fieldID, const RecordItemValue* pData)
{
	auto iterLayer = m_FieldDatas.find(layerID);
	if (iterLayer == m_FieldDatas.end())
	{
		m_FieldDatas.insert(pair<unsigned int, map<unsigned int, vector<const RecordItemValue*>> >(layerID, map<unsigned int, vector<const RecordItemValue*>>()));
		iterLayer = m_FieldDatas.find(layerID);
	}
	if (iterLayer == m_FieldDatas.end())
		return 0;

	auto iterFieldData = iterLayer->second.find(fieldID);
	if (iterFieldData == iterLayer->second.end())
	{
		vector<const RecordItemValue*> records;
		records.emplace_back(pData);
		iterLayer->second.insert(pair<unsigned int, vector<const RecordItemValue*>>(fieldID, records));
		iterFieldData = iterLayer->second.find(fieldID);
	}
	else
		iterFieldData->second.emplace_back(pData);

	if (iterFieldData == iterLayer->second.end())
		return 0;
	return 1;
}

int Ci_ModelCacheAttWriter::Save(AttributeCompressType type, CGByteArray &byteArray)
{
	if (m_LayerInfos.empty() || m_FieldDatas.empty())
		return 0;

	//1 计算 json bin
	CGByteArray bin;
	CGByteArray json;
	if (i_CalcJsonAndBin(json, bin) <= 0)
		return 0;

	//2 写入数据流并保存
	byteArray.clear();
	unsigned int jsonLen = json.length();
	char jsonMagic[4] = { 'j','s','o','n' };
	unsigned int binLen = bin.length();
	char binMagic[4] = { 'b','i','n',0 };
	char fileMagic[4] = { 'a','t','t',0 };
	unsigned int version = 1;
	unsigned int compressType = 0;
	unsigned int dataLen = 0;

	byteArray.append(fileMagic, 4);
	byteArray.append((const char*)&version, 4);

	if (type == AttributeCompressType::PropCompressTypeNone)
	{
		compressType = 0;
		dataLen = 16 + 8 + jsonLen + 8 + bin.length();
		byteArray.append((const char*)&compressType, 4);
		byteArray.append((const char*)&dataLen, 4);
		byteArray.append((const char*)&jsonLen, 4);
		byteArray.append(jsonMagic, 4);
		byteArray.append(json);
		byteArray.append((const char*)&binLen, 4);
		byteArray.append(binMagic, 4);
		byteArray.append(bin);
	}
	else if (type == AttributeCompressType::PropCompressTypeGZip)
	{
		CGByteArray dataForCompress;
		dataForCompress.append((const char*)&jsonLen, 4);
		dataForCompress.append(jsonMagic, 4);
		dataForCompress.append(json);
		dataForCompress.append((const char*)&binLen, 4);
		dataForCompress.append(binMagic, 4);
		dataForCompress.append(bin);
		CGByteArray dataCompress = CGByteArray::Compress(dataForCompress, CGByteArray::CompressType_GZip);

		compressType = 1;
		dataLen = 16 + dataCompress.length();
		byteArray.append((const char*)&compressType, 4);
		byteArray.append((const char*)&dataLen, 4);
		byteArray.append(dataCompress);
	}
	return 1;
}

void CGByteArrayAddRecordItemValue(CGByteArray& bateArray, const RecordItemValue * itemValue, const Field& fieldInfo)
{
	switch (fieldInfo.type)
	{
	case MapGIS::Tile::FieldType::BoolType:
		if (itemValue->IsBool())
		{
			bool value = itemValue->ToBool();
			char cVal = value ? 1 : 0;
			bateArray.append((const char*)&cVal, 1);
		}
		else
		{
			char cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsBool())
			{
				cVal =  fieldInfo.noDataValue.ToBool() ? 1 : 0;
			}
			bateArray.append((const char*)&cVal, 1);
		}
		break;
	case MapGIS::Tile::FieldType::Int8Type:
		if (itemValue->IsInt8())
		{
			char value = itemValue->ToInt8();
			bateArray.append((const char*)&value, 1);
		}
		else
		{
			char cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsInt8())
			{
				cVal = fieldInfo.noDataValue.ToInt8();
			}
			bateArray.append((const char*)&cVal, 1);
		}

		break;
	case MapGIS::Tile::FieldType::Uint8Type:
		if (itemValue->IsUint8())
		{
			unsigned char value = itemValue->ToUint8();
			bateArray.append((const char*)&value, 1);
		}
		else
		{
			unsigned char cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsUint8())
			{
				cVal = fieldInfo.noDataValue.ToUint8();
			}
			bateArray.append((const char*)&cVal, 1);
		}

		break;
	case MapGIS::Tile::FieldType::Int16Type:
		if (itemValue->IsInt16())
		{
			short value = itemValue->ToInt16();
			bateArray.append((const char*)&value, 2);
		}
		else
		{
			short cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsInt16())
			{
				cVal = fieldInfo.noDataValue.ToInt16();
			}
			bateArray.append((const char*)&cVal, 2);
		}
		break;
	case MapGIS::Tile::FieldType::Uint16Type:
		if (itemValue->IsUint16())
		{
			unsigned short value = itemValue->ToUint16();
			bateArray.append((const char*)&value, 2);
		}
		else
		{
			unsigned short cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsUint16())
			{
				cVal = fieldInfo.noDataValue.ToUint16();
			}
			bateArray.append((const char*)&cVal, 2);
		}

		break;
	case MapGIS::Tile::FieldType::Int32Type:

		if (itemValue->IsInt32())
		{
			int value = itemValue->ToInt32();
			bateArray.append((const char*)&value, 4);
		}
		else
		{
			int cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsInt32())
			{
				cVal = fieldInfo.noDataValue.ToInt32();
			}
			bateArray.append((const char*)&cVal, 4);
		}

		break;
	case MapGIS::Tile::FieldType::Uint32Type:
		if (itemValue->IsUint32())
		{
			unsigned int value = itemValue->ToUint32();
			bateArray.append((const char*)&value, 4);
		}
		else
		{
			unsigned int  cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsUint32())
			{
				cVal = fieldInfo.noDataValue.ToUint32();
			}
			bateArray.append((const char*)&cVal, 4);
		}
		break;
	case MapGIS::Tile::FieldType::Int64Type:
		if (itemValue->IsInt64())
		{
			gisINT64 value = itemValue->ToInt64();
			bateArray.append((const char*)&value, 8);
		}
		else
		{
			gisINT64  cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsInt64())
			{
				cVal = fieldInfo.noDataValue.ToInt64();
			}
			bateArray.append((const char*)&cVal, 8);
		}
		break;
	case MapGIS::Tile::FieldType::Uint64Type:
		if (itemValue->IsUint64())
		{
			gisUINT64 value = itemValue->ToUint64();
			bateArray.append((const char*)&value, 8);
		}
		else
		{
			gisUINT64  cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsUint64())
			{
				cVal = fieldInfo.noDataValue.ToUint64();
			}
			bateArray.append((const char*)&cVal, 8);
		}
		break;
	case MapGIS::Tile::FieldType::FloatType:
		if (itemValue->IsFloat())
		{
			float value = itemValue->ToFloat();
			bateArray.append((const char*)&value, 4);
		}
		else
		{
			float  cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsFloat())
			{
				cVal = fieldInfo.noDataValue.ToFloat();
			}
			bateArray.append((const char*)&cVal, 4);
		}
		break;
	case MapGIS::Tile::FieldType::DoubleType:
		if (itemValue->IsDouble())
		{
			double value = itemValue->ToDouble();
			bateArray.append((const char*)&value, 8);
		}
		else
		{
			double  cVal = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsDouble())
			{
				cVal = fieldInfo.noDataValue.ToDouble();
			}
			bateArray.append((const char*)&cVal, 8);
		}
		break;
	case MapGIS::Tile::FieldType::DateTimeType:
		if (itemValue->IsDateTime())
		{
			gisINT64 value = itemValue->ToDateTime();
			bateArray.append((const char*)&value, 8);
		}
		else
		{
			gisINT64 value = 0;
			if (!fieldInfo.noDataValue.IsNull() && fieldInfo.noDataValue.IsDateTime())
			{
				value = fieldInfo.noDataValue.ToDateTime();
			}
			bateArray.append((const char*)&value, 8);
		}
		break;
	case MapGIS::Tile::FieldType::Undefined:
		break;
	default:
		break;
	}
}

int Ci_ModelCacheAttWriter::i_CalcJsonAndBin(CGByteArray& json, CGByteArray& bin)
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	//1 featureIndex
	if (m_FeatureIndexData.size() > 0)
	{
		CGByteArray featureIndexArray;
		//按照id的顺序从小到大排列
		sort(m_FeatureIndexData.begin(), m_FeatureIndexData.end(), [](const FeatureIndexData&v1, const FeatureIndexData &v2) {  return v1.tid < v2.tid; });

		//若id均无效，则不必写入feature index信息
		if (!(m_FeatureIndexData.front().tid <= 0 && m_FeatureIndexData.back().tid <= 0))
		{
			for (int i = 0; i < m_FeatureIndexData.size(); i++)
			{
				FeatureIndexData&data = m_FeatureIndexData.at(i);
				gisINT64  id = data.tid >= 0 ? (gisINT64)data.tid : 0;
				featureIndexArray.append((const char*)&id, 8);
				featureIndexArray.append((const char*)&data.layerIndex, 4);
				featureIndexArray.append((const char*)&data.featureIndex, 4);
			}

			rapidjson::Value indexObj(rapidjson::kObjectType);
			indexObj.AddMember("featureSize", static_cast<int64_t>(m_FeatureIndexData.size()), allocator);
			indexObj.AddMember("dataOffset", 0, allocator);
			indexObj.AddMember("dataLen", featureIndexArray.length(), allocator);
			doc.AddMember("featureIndexData", indexObj, allocator);

			bin.append(featureIndexArray);
			if (bin.length() % 8 > 0)
				bin.append(8 - bin.length() % 8, 0);
		}
	}
	//2 field info
	if (m_LayerInfos.size() > 0)
	{
		rapidjson::Value layerArr(rapidjson::kArrayType);
		for (auto iterL = m_LayerInfos.begin(); iterL != m_LayerInfos.end(); ++iterL)
		{
			if (iterL->first.fieldInfos.size() <= 0)
				continue;

			rapidjson::Value layerObj(rapidjson::kObjectType);
			layerObj.AddMember("dataSource", ToStringValue(iterL->first.dataSource, allocator), allocator);
			layerObj.AddMember("layerName", ToStringValue(iterL->first.layerName, allocator), allocator);
			layerObj.AddMember("layerID", static_cast<int64_t>(iterL->first.layerID), allocator);
			layerObj.AddMember("featureSize", static_cast<int64_t>(iterL->second), allocator);
			rapidjson::Value fieldArr(rapidjson::kArrayType);
			for (auto iterF = iterL->first.fieldInfos.begin(); iterF != iterL->first.fieldInfos.end(); ++iterF)
			{
				CGByteArray fieldData;
				int rtn = i_GetFieldData(iterL->first.layerID, iterF->fieldID, fieldData);
				if (rtn <= 0 || fieldData.length() <= 0)
					continue;

				rapidjson::Value fieldObj(rapidjson::kObjectType);
				fieldObj.AddMember("name", ToStringValue(iterF->name, allocator), allocator);
				fieldObj.AddMember("alias", ToStringValue(iterF->alias, allocator), allocator);
				fieldObj.AddMember("fieldID", static_cast<int64_t>(iterF->fieldID), allocator);
				fieldObj.AddMember("type", ToStringValue(FieldTypeToCGString(iterF->type), allocator), allocator);
				if (!iterF->noDataValue.IsNull())
				{
					fieldObj.AddMember("noData", ToJsonValue(iterF->noDataValue), allocator);
				}
				fieldObj.AddMember("dataOffset", bin.length(), allocator);
				fieldObj.AddMember("dataLen", fieldData.length(), allocator);
				fieldArr.PushBack(fieldObj, allocator);

				//添加数据 对齐
				bin.append(fieldData);
				if (bin.length() % 8 > 0)
					bin.append(8 - bin.length() % 8, 0);
			}
			layerObj.AddMember("fieldInfos", fieldArr, allocator);

			layerArr.PushBack(layerObj, allocator);
		}
		doc.AddMember("layerInfos", layerArr, allocator);
	}
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	char* pBuffer = const_cast<char*>(buffer.GetString());

	json.append(pBuffer, buffer.GetLength());
	if (json.length() % 8 > 0)
		json.append(8 - json.length() % 8, 0);

	return 1;
}

int Ci_ModelCacheAttWriter::i_GetFieldData(unsigned int layerID, unsigned int fieldID, CGByteArray& outByteArray)
{
	auto iterLayer = m_FieldDatas.find(layerID);
	if (iterLayer == m_FieldDatas.end())
		return 0;

	auto iterFieldData = iterLayer->second.find(fieldID);
	if (iterFieldData == iterLayer->second.end())
		return 0;

	Field fieldInfo;
	for (vector<pair<LayerFieldsInfo, unsigned int> >::iterator itr = m_LayerInfos.begin(); itr != m_LayerInfos.end(); itr++)
	{
		if (itr->first.layerID == layerID)
		{
			for (vector<Field>::const_iterator fieldItr = itr->first.fieldInfos.cbegin(); fieldItr != itr->first.fieldInfos.cend(); fieldItr++)
			{
				if (fieldItr->fieldID == fieldID)
				{
					fieldInfo = *fieldItr;
					break;
				}
			}
			if (fieldInfo.type != FieldType::Undefined)
				break;
		}
	}
	vector<const RecordItemValue*> & records = iterFieldData->second;

	if (fieldInfo.type == FieldType::TextType)
	{
		CGByteArray tempByteArraySize;
		CGByteArray tempByteArrayDate;
		for (int i = 0; i < records.size(); i++)
		{
			if (records.at(i)->IsNull())
			{
				CGString value = "";
				if (!fieldInfo.noDataValue.IsNull())
				{
					value = fieldInfo.noDataValue.ToString(CGString::EncodeType::UTF8);
				}
				if (value.GetLength() <= 0)
				{
					unsigned int nLen = 0;
					tempByteArraySize.append((const char*)&nLen, 4);
				}
				else
				{
					unsigned int nLen = value.GetLength();
					if (value.GetLength() > 0 && value[value.GetLength() - 1] != '\0')
						nLen += 1;
					tempByteArraySize.append((const char*)&nLen, 4);
					tempByteArrayDate.append(value.CStr(), value.GetLength());
					if (value.GetLength() > 0 && value[value.GetLength() - 1] != '\0')
						tempByteArrayDate.append('\0');
				}
			}
			else
			{
				CGString strVal = records.at(i)->ToString(CGString::EncodeType::UTF8);
				unsigned int nLen = strVal.GetLength();
				if (strVal.GetLength() > 0 && strVal[strVal.GetLength() - 1] != '\0')
					nLen += 1;
				tempByteArraySize.append((const char*)&nLen, 4);
				tempByteArrayDate.append(strVal.CStr(), strVal.GetLength());
				if (strVal.GetLength() > 0 && strVal[strVal.GetLength() - 1] != '\0')
					tempByteArrayDate.append('\0');
			}
		}
		outByteArray.append(tempByteArraySize);
		outByteArray.append(tempByteArrayDate);
	}
	else
	{
		for (int i = 0; i < records.size(); i++)
		{
			CGByteArrayAddRecordItemValue(outByteArray, records.at(i), fieldInfo);
		}
	}
	return 1;
}
#pragma endregion

#pragma region Ci_ModelCacheAttRead

class Ci_ModelCacheAttRead
{
public:
	Ci_ModelCacheAttRead();
	~Ci_ModelCacheAttRead();
	int Load(const CGByteArray& data);

	int GetLayerNum();
	LayerFieldsInfo* GetLayerInfo(int index);
	unordered_map<unsigned int, unordered_map<unsigned int, gisINT64>>* GetFeatureIndexData();
	vector<RecordItemValue>* GetRecordItems(unsigned int layerID, unsigned int fieldID);

private:
	int i_LoadContent(const CGByteArray& data, int startIndex);
	int i_ParseJsonAndBin(CGByteArray& json, CGByteArray& bin);
	int i_RecordItemValues(CGByteArray& bin, FieldType   type, unsigned int dataOffset, unsigned int dataLen, unsigned int featureSize, vector<RecordItemValue> & records);
private:

	unordered_map<unsigned int, unordered_map<unsigned int, gisINT64>> m_FeatureIndexData;

	vector<pair<LayerFieldsInfo, unsigned int>> m_LayerInfos;
	map<unsigned int, map<unsigned int, vector<RecordItemValue>>> m_FieldDatas;
};
Ci_ModelCacheAttRead::Ci_ModelCacheAttRead()
{
}

Ci_ModelCacheAttRead::~Ci_ModelCacheAttRead()
{
}

int Ci_ModelCacheAttRead::i_RecordItemValues(CGByteArray& bin, FieldType   type, unsigned int dataOffset, unsigned int dataLen, unsigned int featureSize, vector<RecordItemValue> & records)
{
	if (bin.length() < dataOffset + dataLen)
		return 0;

	if (type == MapGIS::Tile::FieldType::TextType)
	{
		unsigned int index = dataOffset;
		vector<unsigned int> len;

		for (unsigned int i = 0; i < featureSize; i++)
		{
			unsigned int value = ReadByteArrayToUnsignedInt32(bin, index);
			index += 4;
			len.emplace_back(value);
		}

		for (unsigned int i = 0; i < featureSize && index <= dataLen + dataOffset; i++)
		{
			if (len[i] > 0)
			{
				std::string text = ReadByteArrayToString(bin, index, len[i]);
				index += len[i];
				records.emplace_back(CGString(text, CGString::UTF8));
			}
			else
				records.emplace_back(RecordItemValue(FieldType::TextType));
		}
	}
	else
	{
		for (unsigned int index = dataOffset; index < dataLen + dataOffset;)
		{
			switch (type)
			{
			case MapGIS::Tile::FieldType::BoolType:
			{
				bool value = ReadByteArrayToBool(bin, index);
				index += 1;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Int8Type:
			{
				char value = ReadByteArrayToChar(bin, index);
				index += 1;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Uint8Type:
			{
				char value = ReadByteArrayToUnsignedChar(bin, index);
				index += 1;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Int16Type:
			{
				short value = ReadByteArrayToInt16(bin, index);
				index += 2;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Uint16Type:
			{
				unsigned short value = ReadByteArrayToUnsignedInt16(bin, index);
				index += 2;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Int32Type:
			{
				int value = ReadByteArrayToInt32(bin, index);
				index += 4;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Uint32Type:
			{
				unsigned int value = ReadByteArrayToUnsignedInt32(bin, index);
				index += 4;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Int64Type:
			{
				gisINT64 value = ReadByteArrayToInt64(bin, index);
				index += 8;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::Uint64Type:
			{
				gisUINT64 value = ReadByteArrayToUnsignedInt64(bin, index);
				index += 8;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::FloatType:
			{
				float value = ReadByteArrayToFloat(bin, index);
				index += 4;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::DoubleType:
			{
				double value = ReadByteArrayToDouble(bin, index);
				index += 8;
				records.emplace_back(value);
			}
			break;
			case MapGIS::Tile::FieldType::DateTimeType:
			{
				gisINT64 value = ReadByteArrayToInt64(bin, index);
				index += 8;
				records.emplace_back(RecordItemValue(value, true));
			}
			break;
			case MapGIS::Tile::FieldType::Undefined:
			default:
				return 0;
				break;
			}
		}
	}
	return 1;
}

int Ci_ModelCacheAttRead::i_ParseJsonAndBin(CGByteArray& json, CGByteArray& bin)
{
	CGString jsonData = json.data();
	if (jsonData.GetLength() <= 0)
		return 0;
	jsonData.SetEncodeType(CGString::EncodeType::UTF8);
	rapidjson::Document doc;
	if (doc.Parse(jsonData.CStr(), jsonData.GetLength()).HasParseError())
		return 0;
	if (!doc.IsObject())
		return 0;
	if (doc.HasMember("featureIndexData"))
	{
		gisINT64 featureIndexSize = 0;
		gisINT64 dataOffset = 0;
		gisINT64 dataLen = 0;

		if (doc["featureIndexData"].IsObject())
		{
			rapidjson::Value & value = doc["featureIndexData"]["featureSize"];
			featureIndexSize = JsonValueToInt64(value);

			value = doc["featureIndexData"]["dataOffset"];
			dataOffset = JsonValueToInt64(value);

			value = doc["featureIndexData"]["dataLen"];
			dataLen = JsonValueToInt64(value);
		}
		if (bin.length() < dataLen + dataOffset)
			return 0;
		if (dataLen / 16 != featureIndexSize)
			return 0;

		for (gisINT64 i = dataOffset; i < (dataOffset + dataLen);)
		{
			gisINT64 tid = ReadByteArrayToInt64(bin, i);
			i += 8;
			unsigned int layerIndex = ReadByteArrayToUnsignedInt32(bin, i);
			i += 4;
			unsigned int featureIndex = ReadByteArrayToUnsignedInt32(bin, i);
			i += 4;
			unordered_map<unsigned int, unordered_map<unsigned int, gisINT64>>::iterator layerItr = m_FeatureIndexData.find(layerIndex);
			if (layerItr == m_FeatureIndexData.end())
			{
				unordered_map<unsigned int, gisINT64> featureMap;
				featureMap.insert(make_pair(featureIndex, tid));
				m_FeatureIndexData.insert(make_pair(layerIndex, featureMap));
			}
			else
			{
				unordered_map<unsigned int, gisINT64>::iterator  featureItr = layerItr->second.find(featureIndex);
				if (featureItr == layerItr->second.end())
				{
					layerItr->second.insert(make_pair(featureIndex, tid));
				}
			}
		}
	}

	if (doc.HasMember("layerInfos"))
	{
		if (doc["layerInfos"].IsArray())
		{
			const rapidjson::Value& layerInfos = doc["layerInfos"];
			for (int i = 0; i < layerInfos.Size(); i++)
			{
				const rapidjson::Value &value = layerInfos[i];
				if (value.IsObject())
				{
					LayerFieldsInfo layerFildsInfo;

					CGString dataSource = "";
					CGString layerName = "";
					unsigned int layerID = 0;
					unsigned int featureSize = 0;

					if (value.HasMember("dataSource") && value["dataSource"].IsString())
						dataSource = value["dataSource"].GetString();
					dataSource.SetEncodeType(CGString::EncodeType::UTF8);

					if (value.HasMember("layerName") && value["layerName"].IsString())
						layerName = value["layerName"].GetString();
					layerName.SetEncodeType(CGString::EncodeType::UTF8);

					if (value.HasMember("layerID"))
						layerID = JsonValueToUInt(value["layerID"]);

					if (value.HasMember("featureSize") )
						featureSize = JsonValueToUInt(value["featureSize"]);

					layerFildsInfo.dataSource = dataSource;
					layerFildsInfo.layerName = layerName;
					layerFildsInfo.layerID = layerID;

					if (value.HasMember("fieldInfos") && value["fieldInfos"] .IsArray())
					{
						const rapidjson::Value& fieldArr = value["fieldInfos"];
						for (int j = 0; j < fieldArr.Size(); j++)
						{
							if (fieldArr[i].IsObject())
							{
								const rapidjson::Value& fieldObj = fieldArr[i];
								Field field;
								unsigned int dataOffset = 0;
								unsigned int dataLen = 0;
								if(fieldObj.HasMember("name") && fieldObj["name"].IsString())
									field.name = fieldObj["name"].GetString();
								field.name.SetEncodeType(CGString::EncodeType::UTF8);

								if (fieldObj.HasMember("alias") && fieldObj["alias"].IsString())
									field.alias = fieldObj["alias"].GetString();
								field.alias.SetEncodeType(CGString::EncodeType::UTF8);

								if (fieldObj.HasMember("fieldID"))
									field.fieldID = JsonValueToUInt(fieldObj["fieldID"]);

								if (fieldObj.HasMember("type") && fieldObj["type"].IsString())
								{
									CGString type = fieldObj["type"].GetString();
									type.SetEncodeType(CGString::EncodeType::UTF8);
									field.type = CGStringToFieldType(type);
								}

								if (fieldObj.HasMember("noData"))
								{
									const rapidjson::Value& noDataValue =  fieldObj["noData"];
									field.noDataValue = FromJsonValue(value, field.type);
								}
								if (fieldObj.HasMember("dataOffset"))
								{
									dataOffset = JsonValueToUInt(fieldObj["dataOffset"]);
								}

								if (fieldObj.HasMember("dataLen"))
								{
									dataLen = JsonValueToUInt(fieldObj["dataLen"]) ;
								}

								layerFildsInfo.fieldInfos.emplace_back(field);

								vector<RecordItemValue>  records;
								if (i_RecordItemValues(bin, field.type, dataOffset, dataLen, featureSize, records) <= 0 || records.size() != featureSize)
								{
									return 0;
								}

								if (!field.noDataValue.IsNull())
								{
									for (vector<RecordItemValue>::iterator itr = records.begin(); itr != records.end(); itr++)
									{
										if (*itr == field.noDataValue)
										{
											itr->SetNull();
										}
									}
								}

								map<unsigned int, map<unsigned int, vector<RecordItemValue>>>::iterator itr = m_FieldDatas.find(layerID);
								if (itr == m_FieldDatas.end())
								{
									map<unsigned int, vector<RecordItemValue>> fieldMap;
									fieldMap.insert(make_pair(field.fieldID, records));
									m_FieldDatas.insert(make_pair(layerID, fieldMap));
								}
								else
								{
									map<unsigned int, vector<RecordItemValue>>::iterator fieldItr = itr->second.find(field.fieldID);
									if (fieldItr == itr->second.end())
									{
										itr->second.insert(make_pair(field.fieldID, records));
									}
								}
							}
						}
					}
					m_LayerInfos.emplace_back(make_pair(layerFildsInfo, featureSize));
				}
			}
		}
	}
	return 1;
}

int Ci_ModelCacheAttRead::i_LoadContent(const CGByteArray& data, int startIndex)
{
	unsigned int jsonLen = ReadByteArrayToUnsignedInt32(data, startIndex); startIndex += 4;
	std::string jsonMagic = ReadByteArrayToString(data, startIndex, 4); startIndex += 4;
	if (strncmp("json", jsonMagic.c_str(), 4) != 0)
		return 0;
	CGByteArray bin;
	CGByteArray json;
	json.append(&data.data()[startIndex], jsonLen);
	startIndex += jsonLen;
	unsigned int binLen = ReadByteArrayToUnsignedInt32(data, startIndex); startIndex += 4;
	std::string binMagic = ReadByteArrayToString(data, startIndex, 4); startIndex += 4;
	if (strncmp("bin", binMagic.c_str(), 3) != 0)
		return 0;
	bin.append(&data.data()[startIndex], binLen);
	return i_ParseJsonAndBin(json, bin);
}

int Ci_ModelCacheAttRead::Load(const CGByteArray& data)
{
	if (data.length() < 16)
		return 0;
	m_FeatureIndexData.clear();
	m_LayerInfos.clear();
	m_FieldDatas.clear();
	int c = 0;
	std::string magic = ReadByteArrayToString(data, c, 4); c += 4;
	unsigned int version = ReadByteArrayToUnsignedInt32(data, c); c += 4;
	if (strncmp("att", magic.c_str(), 3) != 0 || version != 1)
		return 0;
	unsigned int compressType = ReadByteArrayToUnsignedInt32(data, c); c += 4;
	unsigned int dataLen = ReadByteArrayToUnsignedInt32(data, c); c += 4;

	if (data.length() < dataLen)
		return 0;
	if (compressType == 1)
	{//压缩的数据，需要进行解压缩
		unsigned int dataCompressLength = dataLen - 16;
		CGByteArray compressData;
		compressData.append(&data.data()[c], dataCompressLength);
		CGByteArray decompressData = CGByteArray::Decompress(compressData, CGByteArray::CompressType_GZip);
		return i_LoadContent(decompressData, 0);
	}
	else
	{
		return i_LoadContent(data, c);
	}
}

int Ci_ModelCacheAttRead::GetLayerNum() { return m_LayerInfos.size(); }

LayerFieldsInfo* Ci_ModelCacheAttRead::GetLayerInfo(int index)
{
	if (index >= m_LayerInfos.size())
		return NULL;
	return &m_LayerInfos.at(index).first;
}

unordered_map<unsigned int, unordered_map<unsigned int, gisINT64>>* Ci_ModelCacheAttRead::GetFeatureIndexData()
{
	return &m_FeatureIndexData;
}

vector<RecordItemValue>* Ci_ModelCacheAttRead::GetRecordItems(unsigned int layerID, unsigned int fieldID)
{
	map<unsigned int, map<unsigned int, vector<RecordItemValue>>>::iterator layerItr = m_FieldDatas.find(layerID);
	if (layerItr == m_FieldDatas.end())
		return NULL;
	map<unsigned int, vector<RecordItemValue>>::iterator fieldItr = layerItr->second.find(fieldID);
	if (fieldItr == layerItr->second.end())
		return NULL;
	return &fieldItr->second;
}
#pragma endregion

gisLONG MapGIS::Tile::Ci_ModelLayerFieldsInfo::From(const CGByteArray& in, vector<LayerFieldsInfo>&  out)
{
	if (in.length() <= 0)
		return 0;
	rapidjson::Document doc;
	if (doc.Parse(in.data(), in.size()).HasParseError())
		return 0;
	if (!doc.IsArray() || doc.Size()<=0)
		return 0;
	for (int i = 0; i <  doc.Size(); i++)
	{
		if (doc[i].IsObject())
		{
			const rapidjson::Value& layerObj = doc[i];
			LayerFieldsInfo info;

			if (layerObj.HasMember("dataSource" ) && layerObj["dataSource"] .IsString())
				info.dataSource = layerObj["dataSource"].GetString();
			info.dataSource.SetEncodeType(CGString::EncodeType::UTF8);

			if (layerObj.HasMember("layerName") && layerObj["layerName"].IsString())
				info.layerName = layerObj["layerName"].GetString();
			info.layerName.SetEncodeType(CGString::EncodeType::UTF8);

			if (layerObj.HasMember("layerID"))
				info.layerID = JsonValueToUInt(layerObj["layerID"]);

			if (layerObj.HasMember("fieldInfos") && layerObj["fieldInfos"].IsArray())
			{
				for (int j = 0; j <layerObj["fieldInfos"].Size(); ++j)
				{
					if (layerObj["fieldInfos"][i].IsObject())
					{
						const rapidjson::Value& fieldObj = layerObj["fieldInfos"][i];
						info.fieldInfos.push_back(Field());

						if (fieldObj.HasMember("name") && fieldObj["name"].IsString())
							info.fieldInfos.back().name = fieldObj["name"].GetString();
						info.fieldInfos.back().name.SetEncodeType(CGString::EncodeType::UTF8);

						if (fieldObj.HasMember("alias") && fieldObj["alias"].IsString())
							info.fieldInfos.back().alias = fieldObj["alias"].GetString();
						info.fieldInfos.back().alias.SetEncodeType(CGString::EncodeType::UTF8);

						if (fieldObj.HasMember("fieldID"))
							info.fieldInfos.back().fieldID = JsonValueToUInt(fieldObj["fieldID"]);

						if (fieldObj.HasMember("type") && fieldObj["type"].IsString())
						{
							CGString type = fieldObj["type"].GetString();
							type.SetEncodeType(CGString::EncodeType::UTF8);
							info.fieldInfos.back().type = CGStringToFieldType(type);
						}
						if (fieldObj.HasMember("noData"))
						{
							info.fieldInfos.back().noDataValue = FromJsonValue(fieldObj["noData"], info.fieldInfos.back().type);
						}
						if (fieldObj.HasMember("minValue"))
						{
							info.fieldInfos.back().minValue = FromJsonValue(fieldObj["minValue"], info.fieldInfos.back().type);
						}
						if (fieldObj.HasMember("maxValue"))
						{
							info.fieldInfos.back().maxValue = FromJsonValue(fieldObj["maxValue"], info.fieldInfos.back().type);
						}
					}
				}
			}

			out.push_back(info);
		}
	}
	return 1;
}

gisLONG MapGIS::Tile::Ci_ModelLayerFieldsInfo::To(const vector<LayerFieldsInfo>&  in, CGByteArray& out)
{
	if (in.size() <= 0)
		return 0;
	rapidjson::Document doc;
	doc.SetArray();
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	for (int i = 0; i < in.size(); i++)
	{
		rapidjson::Value layerObj(rapidjson::kObjectType);
		CGString dataSource = in[i].dataSource;
		CGString layerName = in[i].layerName;
		layerObj.AddMember("dataSource", ToStringValue(dataSource, allocator) ,allocator);
		layerObj.AddMember("layerName", ToStringValue(layerName, allocator), allocator);
		layerObj.AddMember("layerID", static_cast<int64_t>(in[i].layerID), allocator);

		rapidjson::Value fieldInfosArr(rapidjson::kArrayType);
		for (int j = 0; j < in[i].fieldInfos.size(); ++j)
		{
			rapidjson::Value fieldObj(rapidjson::kObjectType);
			CGString name = in[i].fieldInfos[j].name;
			CGString alias = in[i].fieldInfos[j].alias;
			CGString type = FieldTypeToCGString(in[i].fieldInfos[j].type);

			fieldObj.AddMember("name", ToStringValue(name, allocator) , allocator);
			fieldObj.AddMember("alias", ToStringValue(alias, allocator), allocator);
			fieldObj.AddMember("fieldID",in[i].fieldInfos[j].fieldID, allocator);
			fieldObj.AddMember("type", ToStringValue(type, allocator), allocator);
			if (!in[i].fieldInfos[j].noDataValue.IsNull())
			{
				fieldObj.AddMember("noData", ToJsonValue(in[i].fieldInfos[j].noDataValue), allocator);
			}
			if (!in[i].fieldInfos[j].minValue.IsNull())
			{
				fieldObj.AddMember("minValue", ToJsonValue(in[i].fieldInfos[j].minValue), allocator);
			}
			if (!in[i].fieldInfos[j].maxValue.IsNull())
			{
				fieldObj.AddMember("maxValue", ToJsonValue(in[i].fieldInfos[j].maxValue), allocator);
			}
			fieldInfosArr.PushBack(fieldObj, allocator);
		}
		layerObj.AddMember("fieldInfos", fieldInfosArr, allocator);
		doc.PushBack(layerObj, allocator);
	}
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	char* pBuffer = const_cast<char*>(buffer.GetString());
	out.append(pBuffer, buffer.GetLength());
	return 1;
}

gisLONG  MapGIS::Tile::Ci_ModelAttributeAttFile::From(const CGByteArray& in, Attribute&  out)
{
	Ci_ModelCacheAttRead attWriter;
	if (attWriter.Load(in) <= 0)
		return 0;
	out.layers.clear();
	int num = attWriter.GetLayerNum();
	unordered_map<unsigned int, unordered_map<unsigned int, gisINT64>>* pFeatureIndexData = attWriter.GetFeatureIndexData();
	gisLONG rtn = 0;
	for (int i = 0; i < num; i++)
	{
		rtn = 1;
		LayerAttribute layerAttribute;
		LayerFieldsInfo* pInfo = attWriter.GetLayerInfo(i);
		if (pInfo == NULL)
			continue;
		unsigned int layerID = pInfo->layerID;

		int featureNum = 0;
		vector<RecordItemValue>** pRecordsArr = new vector<RecordItemValue>*[pInfo->fieldInfos.size()];
		for (int j = 0; j < pInfo->fieldInfos.size(); j++)
		{
			vector<RecordItemValue>* pRecords = attWriter.GetRecordItems(layerID, pInfo->fieldInfos.at(j).fieldID);
			if (pRecords == NULL)
			{
				rtn = 0;
				goto END;
			}
			if (j == 0)
				featureNum = pRecords->size();
			else if (featureNum != pRecords->size())
			{
				rtn = 0;
				goto END;
			}
			pRecordsArr[j] = pRecords;
		}
		for (int j = 0; j < featureNum; j++)
		{
			Record record;
			record.SetID(j);
			if (pFeatureIndexData != NULL && pFeatureIndexData->size() > 0)
			{
				unordered_map<unsigned int, unordered_map<unsigned int, gisINT64>>::iterator layerItr = pFeatureIndexData->find(i);
				if (layerItr != pFeatureIndexData->end())
				{
					unordered_map<unsigned int, gisINT64>::iterator featureItr = layerItr->second.find(j);
					if (featureItr != layerItr->second.end())
					{
						record.SetID(featureItr->second);
					}
				}
			}
			for (int k = 0; k < pInfo->fieldInfos.size(); k++)
			{
				record.AppendItem((pRecordsArr[k])->at(j));
			}

			layerAttribute.records.emplace_back(record);
		}
		layerAttribute.layerInfo = *pInfo;
		out.layers.push_back(layerAttribute);
	END:
		delete[] pRecordsArr;
		if (rtn == 0)
			break;
	}
	return rtn;
}

gisLONG  MapGIS::Tile::Ci_ModelAttributeAttFile::To(const Attribute&  in, CGByteArray& out, AttributeCompressType compressType)
{
	Ci_ModelCacheAttWriter attWriter;
	vector<RecordItemValue*> items;
	gisLONG rtn = 0;
	if (in.layers.size() > 0)
	{
		gisLONG layerIndex = 0;
		for (int i = 0; i < in.layers.size(); i++)
		{
			const LayerAttribute &layerAttribute = in.layers.at(i);
			const LayerFieldsInfo& layerInfo = layerAttribute.layerInfo;
			const vector<Record>& records = layerAttribute.records;
			if (layerInfo.fieldInfos.size() <= 0)
				continue;
			for (int j = 0; j < records.size(); j++)
			{
				const Record & record = records.at(j);
				gisLONG num = record.GetNum();
				if (layerInfo.fieldInfos.size() != num)
					goto END;
				for (int k = 0; k < num; k++)
				{
					unsigned int fieldID = layerInfo.fieldInfos.at(k).fieldID;
					RecordItemValue* pItem = new RecordItemValue();
					record.GetItem(k, *pItem);
					items.emplace_back(pItem);
					attWriter.AppendFieldData(layerInfo.layerID, fieldID, pItem);
				}

				FeatureIndexData feaIdxData(records[j].GetID(), layerIndex, j);
				attWriter.AppendFeatureIndexData(feaIdxData);
			}
			attWriter.AppendLayerInfo(layerInfo, records.size());
			layerIndex++;
		}
	}
	attWriter.Save(compressType, out);
	rtn = 1;
END:
	for (vector<RecordItemValue*>::iterator itr = items.begin(); itr != items.end(); itr++)
	{
		delete *itr;
	}
	items.clear();
	return rtn;
}

gisLONG MapGIS::Tile::Ci_ModelAttributeAttFile::To(const LayerAttribute&  in, CGByteArray& out, vector<gisINT64> &ids, AttributeCompressType compressType)
{
	if (ids.size() <= 0)
		return 0;
	Ci_ModelCacheAttWriter attWriter;
	vector<RecordItemValue*> items;
	int rtn = 0;

	const LayerAttribute &layerAttribute = in;
	LayerFieldsInfo layerInfo = layerAttribute.layerInfo;
	const vector<Record>& records = layerAttribute.records;

	unordered_map<gisINT64, gisINT64> recordIndex;
	for (int j = 0; j < records.size(); j++)
	{
		const Record & record = records.at(j);
		gisLONG num = record.GetNum();
		if (layerInfo.fieldInfos.size() != num)
			goto END;
		recordIndex.insert(make_pair(record.GetID(), j));
	}

	for (vector<gisINT64>::iterator itr = ids.begin(); itr != ids.end(); itr++)
	{
		if (recordIndex.find(*itr) == recordIndex.end())
			goto END;
		const Record & record = records.at(recordIndex.at(*itr));
		gisLONG num = record.GetNum();
		for (int k = 0; k < num; k++)
		{
			unsigned int fieldID = layerInfo.fieldInfos.at(k).fieldID;
			RecordItemValue* pItem = new RecordItemValue();
			record.GetItem(k, *pItem);
			items.emplace_back(pItem);
			attWriter.AppendFieldData(layerInfo.layerID, fieldID, pItem);
		}
	}
	attWriter.AppendLayerInfo(layerInfo, records.size());
	attWriter.Save(compressType, out);
	rtn = 1;
END:
	for (vector<RecordItemValue*>::iterator itr = items.begin(); itr != items.end(); itr++)
	{
		delete *itr;
	}
	items.clear();
	return rtn;
}