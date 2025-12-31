#include "stdafx.h"
#include "ci_assist.h"
#include "ci_3dmodel_3dtile.h"
#include <algorithm>
#include <cstring>

using namespace MapGIS::Tile;

#pragma region Ci_3dtiles_BatchTable

Ci_3DTileBatchTable::Ci_3DTileBatchTable()
{
}
Ci_3DTileBatchTable::~Ci_3DTileBatchTable()
{
}

inline void ByteArrayOffset(CGByteArray&  byteArray, int len)
{
	if (len > 0)
	{
		int binMore = len - byteArray.size() % len;
		if (binMore != len)
		{
			for (int i = 0; i < binMore; i++)
				byteArray.append(" ");
		}
	}
}

template<typename T>
inline void ByteArrayAddValue(CGByteArray&  byteArray, T& val, int len)
{
	if (len > 0)
	{
		const char* pValue = (const char*)&val;
		byteArray.append(pValue, len);
	}
}

gisLONG Ci_3DTileBatchTable::i_WriteRecordsTo(const LayerAttribute* pRecords, CGByteArray&  json, CGByteArray& bin, CGString& batchTableJson, gisLONG batchLength, unordered_map<gisINT64, gisINT64>& batchIDToIndex, bool existValue)
{
	RecordItemValue  item;
	unordered_map<string, string> valueBack;
	for (int i = 0; i < pRecords->layerInfo.fieldInfos.size(); i++)
	{
		FieldType fieldType = pRecords->layerInfo.fieldInfos[i].type;
		switch (fieldType)
		{
		case MapGIS::Tile::FieldType::Int8Type:
		{
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"BYTE\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");

			char value = 0;
			for (gisINT64 j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToInt8();
					}
				}
				ByteArrayAddValue<char>(bin, value, 1);
			}
		}
		break;
		case MapGIS::Tile::FieldType::Uint8Type:
		{
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"UNSIGNED_BYTE\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");

			unsigned char value = 0;
			for (int j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToUint8();
					}
				}
				ByteArrayAddValue<unsigned char>(bin, value, 1);
			}
		}
		break;
		case MapGIS::Tile::FieldType::Int16Type:
		{
			ByteArrayOffset(bin, 2);
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"SHORT\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");

			short value = 0;
			for (int j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToInt16();
					}
				}
				ByteArrayAddValue<short>(bin, value, 2);
			}
		}
		break;
		case MapGIS::Tile::FieldType::Uint16Type:
		{
			ByteArrayOffset(bin, 2);
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"UNSIGNED_SHORT\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");

			unsigned short value = 0;
			for (int j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToUint16();
					}
				}
				ByteArrayAddValue<unsigned short>(bin, value, 2);
			}
		}
		break;
		case MapGIS::Tile::FieldType::Int32Type:
		{
			ByteArrayOffset(bin, 4);
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"INT\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");

			int value = 0;
			for (int j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToInt32();
					}
				}
				ByteArrayAddValue<int>(bin, value, 4);
			}
		}
		break;
		case MapGIS::Tile::FieldType::Uint32Type:
		{
			ByteArrayOffset(bin, 4);
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"UNSIGNED_INT\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");
			unsigned int value = 0;
			for (int j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToUint32();
					}
				}
				ByteArrayAddValue<unsigned int>(bin, value, 4);
			}
		}
		break;
		case MapGIS::Tile::FieldType::FloatType:
		{
			ByteArrayOffset(bin, 4);
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"FLOAT\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");

			float value = 0;
			for (int j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToFloat();
					}
				}
				ByteArrayAddValue<float>(bin, value, 4);
			}
		}
		break;
		case MapGIS::Tile::FieldType::DoubleType:
		{
			ByteArrayOffset(bin, 8);
			if (existValue)
				batchTableJson.Append(",");
			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":{");
			batchTableJson.Append("\"byteOffset\":" + std::to_string(bin.size()) + ",");
			batchTableJson.Append("\"componentType\":\"DOUBLE\",");
			batchTableJson.Append("\"type\":\"SCALAR\"");
			batchTableJson.Append("}");

			double value = 0;
			for (int j = 0; j < batchLength; j++)
			{
				value = 0;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						value = item.ToDouble();
					}
				}
				ByteArrayAddValue<double>(bin, value, 8);
			}
		}
		break;
		case MapGIS::Tile::FieldType::BoolType:
		case MapGIS::Tile::FieldType::Int64Type:
		case MapGIS::Tile::FieldType::Uint64Type:
		case MapGIS::Tile::FieldType::TextType:
		case MapGIS::Tile::FieldType::DateTimeType:
		case MapGIS::Tile::FieldType::Undefined:
		default:
		{
			if (existValue)
				batchTableJson.Append(",");

			FieldType fieldType = pRecords->layerInfo.fieldInfos[i].type;

			bool isText = fieldType == FieldType::TextType;

			batchTableJson.Append("\"");
			batchTableJson.Append(EscapeJson(pRecords->layerInfo.fieldInfos[i].name.Converted(CGString::EncodeType::GB18030)).c_str());
			batchTableJson.Append("\":[");
			for (int j = 0; j < batchLength; j++)
			{
				if (j != 0)
					batchTableJson.Append(",");
				bool isAdd = false;
				unordered_map<gisINT64, gisINT64>::iterator batchIDItr = batchIDToIndex.find(j);
				if (batchIDItr != batchIDToIndex.end())
				{
					gisLONG num = pRecords->records[batchIDItr->second].GetNum();
					if (num > i)
					{
						pRecords->records[batchIDItr->second].GetItem(i, item);
						if (isText)
						{
							batchTableJson.Append("\"");
							unordered_map<string, string>::iterator valueBackItr = valueBack.find(item.ToText().CStr());
							if (valueBackItr != valueBack.end())
							{
								batchTableJson.Append(valueBackItr->second.c_str());
							}
							else
							{
								string value = EscapeJson(item.ToString(CGString::EncodeType::GB18030));
								valueBack.insert(make_pair(item.ToText().CStr(), value));
								batchTableJson.Append(value.c_str());
							}
							batchTableJson.Append("\"");
						}
						else
						{
							batchTableJson.Append(item.ToString().CStr());
						}
						isAdd = true;
					}
				}
				if (!isAdd)
				{
					if (isText)
						batchTableJson.Append("\"\"");
					else
						batchTableJson.Append("null");
				}
			}
			batchTableJson.Append("]");
		}
		break;
		}
		if (!existValue)
			existValue = true;
	}
	batchTableJson.Append("}");
	batchTableJson.Convert(CGString::UTF8);
	json.append(batchTableJson.CStr(), batchTableJson.GetLength());
	return 1;
}

gisLONG Ci_3DTileBatchTable::WriteTo(const LayerAttribute* pRecords, const vector<unsigned int>*  pIndexToID, CGByteArray&  json, CGByteArray& bin, string IdName)
{
	if (pRecords == NULL)
		return 0;
	gisINT64 maxBatchId = 0;
	unordered_map<gisINT64, gisINT64> IDToIndex;
	unordered_map<gisINT64, gisINT64> batchIDToIndex;
	maxBatchId = pRecords->records.size() - 1;
	for (gisINT64 i = 0; i < pRecords->records.size(); i++)
	{
		batchIDToIndex.insert(make_pair(i, i));
	}
	unordered_map<string, string> valueBack;
	CGString batchTableJson("", CGString::EncodeType::GB18030);
	batchTableJson.Append("{");

	if (!IdName.empty() && pIndexToID != NULL )
	{
		batchTableJson.Append("\"");
		batchTableJson.Append(IdName.c_str());
		batchTableJson.Append("\":[");
		for (int i = 0; i <= maxBatchId; i++)
		{
			if (i != 0)
				batchTableJson.Append(",");
			gisINT64 id = -1;
			if (pIndexToID->size() > i)
				id = pIndexToID->at(i);
			batchTableJson.Append(std::to_string(id));
		}
	}

	batchTableJson.Append("]");
	return i_WriteRecordsTo(pRecords, json, bin, batchTableJson, maxBatchId + 1, batchIDToIndex, !IdName.empty());
}

gisLONG Ci_3DTileBatchTable::WriteTo(const LayerAttribute* pRecords, const map<gisINT64, gisINT64>* pIdToBatchID, CGByteArray&  json, CGByteArray& bin, string IdName)
{
	if (pRecords == NULL)
		return 0;
	gisINT64 maxBatchId = 0;
	bool existValue = false;
	unordered_map<gisINT64, gisINT64> batchIDToID;
	unordered_map<gisINT64, gisINT64> IDToIndex;
	unordered_map<gisINT64, gisINT64> batchIDToIndex;

	if (pIdToBatchID != NULL)
	{
		for (map<gisINT64, gisINT64>::const_iterator itr = pIdToBatchID->begin(); itr != pIdToBatchID->end(); itr++)
		{
			batchIDToID.insert(make_pair(itr->second, itr->first));
			maxBatchId = max(maxBatchId, itr->second);
		}
	}
	else
	{
		for (vector<Record>::const_iterator itr = pRecords->records.begin(); itr != pRecords->records.end(); itr++)
		{
			maxBatchId = max(maxBatchId, itr->GetID());
		}
	}

	for (gisINT64 i = 0; i < pRecords->records.size(); i++)
	{
		if (IDToIndex.find(pRecords->records[i].GetID()) == IDToIndex.end())
			IDToIndex.insert(make_pair(pRecords->records[i].GetID(), i));
	}

	if (pIdToBatchID != NULL)
	{
		gisINT64 index = -1;
		for (gisINT64 j = 0; j <= maxBatchId; j++)
		{
			index = -1;
			unordered_map<gisINT64, gisINT64>::iterator itr = batchIDToID.find(j);
			if (itr != batchIDToID.end())
			{
				unordered_map<gisINT64, gisINT64>::iterator oidItr = IDToIndex.find(itr->second);
				if (oidItr != IDToIndex.end())
				{
					index = oidItr->second;
				}
			}
			if (index >= 0)
				batchIDToIndex.insert(make_pair(j, index));
		}
	}
	else
	{
		batchIDToIndex = IDToIndex;
	}

	CGString batchTableJson("", CGString::EncodeType::GB18030);
	batchTableJson.Append("{");

	if (!IdName.empty())
	{
		batchTableJson.Append("\"");
		batchTableJson.Append(IdName);
		batchTableJson.Append("\":[");
		if (pIdToBatchID != NULL)
		{
			for (int i = 0; i <= maxBatchId; i++)
			{
				if (i != 0)
					batchTableJson.Append(",");

				unordered_map<gisINT64, gisINT64>::iterator itr = batchIDToID.find(i);
				if (itr != batchIDToID.end())
					batchTableJson.Append(std::to_string(itr->second));
				else
					batchTableJson.Append("-1");
			}
		}
		else
		{
			for (int i = 0; i <= maxBatchId; i++)
			{
				if (i != 0)
					batchTableJson.Append(",");
				if (IDToIndex.find(i) != IDToIndex.end())
					batchTableJson.Append(std::to_string(i));
				else
					batchTableJson.Append("-1");
			}
		}
		batchTableJson.Append("]");
		existValue = true;
	}
	return i_WriteRecordsTo(pRecords, json, bin, batchTableJson, maxBatchId + 1, batchIDToIndex, existValue);
}

MapGIS::Tile::FieldType GetFieldType(const rapidjson::Value& jsonValue)
{
	if (jsonValue.IsArray())
	{
		if (jsonValue.Size() > 0)
		{
			if (jsonValue[0].IsBool())
				return MapGIS::Tile::FieldType::BoolType;
			else  if (jsonValue[0].IsNumber())
				return MapGIS::Tile::FieldType::DoubleType;
			else  if (jsonValue[0].IsString())
				return MapGIS::Tile::FieldType::TextType;
		}
	}
	else if (jsonValue.IsObject())
	{
		if (jsonValue.HasMember("type") && jsonValue["type"].IsString())
		{
			const char* value = jsonValue["type"].GetString();
			if (StrICmp(value, "SCALAR") != 0)
				return MapGIS::Tile::FieldType::Undefined;
		}
		if (jsonValue.HasMember("componentType") && jsonValue["componentType"].IsString())
		{
			const char* value = jsonValue["componentType"].GetString();
			if (StrICmp(value, "DOUBLE") == 0)
				return MapGIS::Tile::FieldType::DoubleType;
			else if (StrICmp(value, "FLOAT") == 0)
				return MapGIS::Tile::FieldType::FloatType;
			else if (StrICmp(value, "UNSIGNED_INT") == 0)
				return MapGIS::Tile::FieldType::Uint32Type;
			else if (StrICmp(value, "INT") == 0)
				return MapGIS::Tile::FieldType::Int32Type;
			else if (StrICmp(value, "UNSIGNED_SHORT") == 0)
				return MapGIS::Tile::FieldType::Uint16Type;
			else if (StrICmp(value, "SHORT") == 0)
				return MapGIS::Tile::FieldType::Int16Type;
			else if (StrICmp(value, "UNSIGNED_BYTE") == 0)
				return MapGIS::Tile::FieldType::Uint8Type;
			else if (StrICmp(value, "BYTE") == 0)
				return MapGIS::Tile::FieldType::Int8Type;
		}
		return MapGIS::Tile::FieldType::Undefined;
	}
	return MapGIS::Tile::FieldType::Undefined;
};

template <typename T1, typename T2>
void CopyValue(const CGByteArray &bin, int start, int num, vector<T1>& outValues)
{
	int length = sizeof(T2) * num;
	if (start + length > bin.length())
		return;
	T2 tempValue = 0;
	for (int i = 0; i < num; i++)
	{
		const char * DD = &bin.data()[start];

		memcpy(&tempValue, &bin.data()[start], sizeof(T2));
		start += sizeof(T2);
		outValues.emplace_back((T1)tempValue);
	}
}

template <typename T>
T JsonValueToValue(const rapidjson::Value& jsVal)
{
	if (std::is_same<T, char>::value)
		return (T)JsonValueToChar(jsVal);
	else if (std::is_same<T, unsigned char>::value)
		return (T)JsonValueToUChar(jsVal);
	else if (std::is_same<T, short>::value)
		return (T)JsonValueToShort(jsVal);
	else if (std::is_same<T, unsigned short>::value)
		return (T)JsonValueToUShort(jsVal);
	else if (std::is_same<T, int>::value)
		return (T)JsonValueToInt(jsVal);
	else if (std::is_same<T, unsigned int>::value)
		return (T)JsonValueToUInt(jsVal);
	else if (std::is_same<T, long long>::value)
		return (T)JsonValueToInt64(jsVal);
	else if (std::is_same<T, unsigned long long>::value)
		return (T)JsonValueToUInt64(jsVal);
	else if (std::is_same<T, float>::value)
		return (T)JsonValueToFloat(jsVal);
	else if (std::is_same<T, double>::value)
		return (T)JsonValueToDouble(jsVal);
	return (T)JsonValueToDouble(jsVal);
}

template <typename T>
void MapGIS::Tile::GetTableValue(rapidjson::Document& doc, CGString name, int num, const CGByteArray &bin, vector<T>& outValues)
{
	outValues.clear();
	if (num <= 0)
		return;
	if (!doc.IsObject())
		return;
	name.Convert(CGString::EncodeType::UTF8);

	if (doc.HasMember(name.CStr()))
	{
		if (doc[name.CStr()].IsNumber())
			outValues.emplace_back(JsonValueToValue<T>(doc[name.CStr()]));
		else if (doc[name.CStr()].IsArray())
		{
			if (doc[name.CStr()].Size() != num)
				return;
			for (int i = 0; i < num; i++)
			{
				if (doc[name.CStr()][i].IsNumber())
					outValues.emplace_back(JsonValueToValue<T>(doc[name.CStr()][i]));
				else
					outValues.emplace_back((T)0);
			}
		}
		else if (doc[name.CStr()].IsObject())
		{
			const rapidjson::Value & valueObject = doc[name.CStr()];
			int start = 0;
			if (valueObject.HasMember("byteOffset"))
				start = JsonValueToInt(valueObject["byteOffset"]);
			MapGIS::Tile::FieldType type = GetFieldType(valueObject);
			int length = 0;
			switch (type)
			{
			case MapGIS::Tile::FieldType::Int8Type:
				CopyValue<T, gisCHAR>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Uint8Type:
				CopyValue<T, gisUCHAR>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Int16Type:
				CopyValue<T, gisSHORT>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Uint16Type:
				CopyValue<T, gisUSHORT>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Int32Type:
				CopyValue<T, gisINT>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Uint32Type:
				CopyValue<T, gisUINT>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Int64Type:
				CopyValue<T, gisINT64>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Uint64Type:
				CopyValue<T, gisUINT64>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::FloatType:
				CopyValue <T, float>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::DoubleType:
				CopyValue <T, double>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::Undefined:
				CopyValue <T, T>(bin, start, num, outValues);
				break;
			case MapGIS::Tile::FieldType::BoolType:
			case MapGIS::Tile::FieldType::TextType:
			case MapGIS::Tile::FieldType::DateTimeType:
			default:
				break;
			}
		}
	}
}

template<>
void MapGIS::Tile::GetTableValue<CGString>(rapidjson::Document& doc, CGString name, int num, const CGByteArray &bin, vector<CGString>& outValues)
{
	outValues.clear();
	if (num <= 0)
		return;

	if (!doc.IsObject())
		return;
	name.Convert(CGString::EncodeType::UTF8);
	if (doc.HasMember(name.CStr()))
	{
		const rapidjson::Value& value = doc[name.CStr()];
		if (value.IsString())
		{
			const char* outValue =   value.GetString();
			outValues.emplace_back(CGString(outValue,CGString::EncodeType::UTF8));
		}
		else if (value.IsArray())
		{
			if (value.Size() != num)
				return;
			for (int i = 0; i < num; i++)
			{
				if (value[i].IsString())
					outValues.emplace_back(CGString(value[i].GetString(), CGString::EncodeType::UTF8));
				else
					outValues.emplace_back("");
			}
		}
	}
}

template<>
void MapGIS::Tile::GetTableValue<bool>(rapidjson::Document& doc, CGString name, int num, const CGByteArray &bin, vector<bool>& outValues)
{
	outValues.clear();
	if (num <= 0)
		return;
	if (!doc.IsObject())
		return;
	name.Convert(CGString::EncodeType::UTF8);
	if (doc.HasMember(name.CStr()))
	{
		const rapidjson::Value& value = doc[name.CStr()];
		if (value.IsBool())
			outValues.push_back(value.GetBool());
		else if (value.IsArray())
		{
			if (value.Size() != num)
				return;
			for (int i = 0; i < num; i++)
			{
				if (value[i].IsBool())
					outValues.push_back(value[i].GetBool());
				else
					outValues.push_back(false);
			}
		}
	}
}

template <typename T>
bool AddAttributeValues(rapidjson::Document& doc, CGString name, FieldType type, CGString idName, int batchLength, const CGByteArray& bin, LayerAttribute& records, vector<T> values)
{
	GetTableValue<T>(doc, name, batchLength, bin, values);
	if (values.size() != batchLength)
		return false;
	if (!idName.IsEmpty() && name == idName)
	{
		for (int j = 0; j < values.size(); j++)
		{
			records.records[j].SetID(values[j]);
		}
	}
	else
	{
		Field field;
		field.name = name;
		field.type = type;
		field.fieldID = CalcAttFieldID(name);
		records.layerInfo.fieldInfos.emplace_back(field);
		for (int j = 0; j < batchLength; j++)
		{
			records.records[j].AppendItem(RecordItemValue(values[j]));
		}
	}
	return true;
}

template <>
bool AddAttributeValues<CGString>(rapidjson::Document& doc, CGString name, FieldType type, CGString idName, int batchLength, const CGByteArray& bin, LayerAttribute& records, vector<CGString> values)
{
	GetTableValue<CGString>(doc, name, batchLength, bin, values);
	if (values.size() != batchLength)
		return false;
	Field field;
	field.name = name;
	field.type = FieldType::TextType;
	field.fieldID = CalcAttFieldID(name);
	records.layerInfo.fieldInfos.emplace_back(field);
	for (int j = 0; j < batchLength; j++)
	{
		records.records[j].AppendItem(RecordItemValue(values[j]));
	}
	return true;
}

gisLONG Ci_3DTileBatchTable::ReadFrom(const CGByteArray&  json, const CGByteArray& bin, CGString IdName, int batchLength, LayerAttribute& records)
{
	if (batchLength <= 0)
		return true;
	rapidjson::Document doc = rapidjson::Document();
	if (!doc.Parse(json.data(), json.size()).HasParseError())
	{
		if(doc.IsObject())
		{
			bool existId = false;
			if (!IdName.IsEmpty())
			{
				IdName.Convert(CGString::EncodeType::UTF8);
				if (doc.HasMember(IdName.CStr()))
				{
					const rapidjson::Value& value =  doc[IdName.CStr()];
					MapGIS::Tile::FieldType fieldType = GetFieldType(value);
					if (fieldType != MapGIS::Tile::FieldType::BoolType &&  fieldType != MapGIS::Tile::FieldType::TextType && fieldType != MapGIS::Tile::FieldType::Undefined)
					{
						existId = true;
					}
				}
			}
			records.records.resize(batchLength);

			for (rapidjson::Value::MemberIterator memberItr = doc.MemberBegin(); memberItr != doc.MemberEnd(); memberItr++)
			{
				const rapidjson::Value& jsonValue = memberItr->value;
				CGString name = "";
				if(memberItr->name.IsString())
				{
					name = CGString(memberItr->name.GetString(), CGString::EncodeType::UTF8);
				}
				MapGIS::Tile::FieldType fieldType = GetFieldType(jsonValue);
				switch (fieldType)
				{
				case MapGIS::Tile::FieldType::BoolType:
				{
					vector<bool>  outValues;
					if (!AddAttributeValues<bool>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Int8Type:
				{
					vector<gisCHAR>  outValues;
					if (!AddAttributeValues<gisCHAR>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Uint8Type:
				{
					vector<gisUCHAR>  outValues;
					if (!AddAttributeValues<gisUCHAR>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Int16Type:
				{
					vector<gisSHORT>  outValues;
					if (!AddAttributeValues<gisSHORT>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Uint16Type:
				{
					vector<gisUSHORT>  outValues;
					if (!AddAttributeValues<gisUSHORT>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Int32Type:
				{
					vector<gisINT>  outValues;
					if (!AddAttributeValues<gisINT>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Uint32Type:
				{
					vector<gisUINT>  outValues;
					if (!AddAttributeValues<gisUINT>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Int64Type:
				{
					vector<gisINT64>  outValues;
					if (!AddAttributeValues<gisINT64>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::Uint64Type:
				{
					vector<gisUINT64>  outValues;
					if (!AddAttributeValues<gisUINT64>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::FloatType:
				{
					vector<float>  outValues;
					if (!AddAttributeValues(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::DoubleType:
				{
					vector<double>  outValues;
					if (!AddAttributeValues<double>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::TextType:
				{
					vector<CGString>  outValues;
					if (!AddAttributeValues<CGString>(doc, name, fieldType, IdName, batchLength, bin, records, outValues))
						return false;
				}
				break;
				case MapGIS::Tile::FieldType::DateTimeType:
				case MapGIS::Tile::FieldType::Undefined:
				default:
					return false;
				}
			}
			if (!existId)
			{
				for (int i = 0; i < batchLength; i++)
				{
					records.records[i].SetID(i);
				}
			}
		}
	}
	return true;
}

#pragma endregion

#pragma region ModelHeader
Ci_3DTileModelHeader::Ci_3DTileModelHeader()
{
	string Magic = "";
	m_version = 0;
	m_byteLength = 0;

	m_featureTableJSONLength = 0;
	m_featureTableJSONPaddingCount = 0;
	m_featureTableJsonLengthNoPadding = 0;

	m_featureTableBinaryLength = 0;
	m_featureTableBinaryLengthNoPadding = 0;
	m_featureTableBinaryPaddingCount = 0;

	m_batchTableJSONLength = 0;
	m_batchTableJSONLengthNoPadding = 0;
	m_batchTableJSONPaddingCount = 0;

	m_batchTableBinaryLength = 0;
	m_batchTableBinaryLengthNoPadding = 0;
	m_batchTableBinaryPaddingCount = 0;
}
Ci_3DTileModelHeader::~Ci_3DTileModelHeader()
{
}
void Ci_3DTileModelHeader::SetFeatureTableJSONLengthNoPadding(unsigned int len)
{
	m_featureTableJsonLengthNoPadding = len;
	unsigned int remainder = ((unsigned int)GetHeaderLength() + m_featureTableJsonLengthNoPadding) % 8;
	if (remainder == 0)
	{
		m_featureTableJSONLength = m_featureTableJsonLengthNoPadding;
		m_featureTableJSONPaddingCount = 0;
	}
	else
	{
		m_featureTableJSONPaddingCount = 8 - remainder;
		m_featureTableJSONLength = m_featureTableJsonLengthNoPadding + m_featureTableJSONPaddingCount;
	}
}
unsigned int Ci_3DTileModelHeader::GetFeatureTableJSONLengthNoPadding()
{
	return m_featureTableJsonLengthNoPadding;
}
unsigned int Ci_3DTileModelHeader::GetFeatureTableJSONPaddingCount()
{
	return m_featureTableJSONPaddingCount;
}
unsigned int Ci_3DTileModelHeader::GetFeatureTableJSONLength()
{
	return m_featureTableJSONLength;
}
void Ci_3DTileModelHeader::SetFeatureTableBinaryLengthNoPadding(unsigned int len)
{
	m_featureTableBinaryLengthNoPadding = len;
	unsigned int remainder = m_featureTableBinaryLengthNoPadding % 8;
	if (remainder == 0)
	{
		m_featureTableBinaryLength = m_featureTableBinaryLengthNoPadding;
		m_featureTableBinaryPaddingCount = 0;
	}
	else
	{
		m_featureTableBinaryPaddingCount = 8 - remainder;
		m_featureTableBinaryLength = m_featureTableBinaryLengthNoPadding + m_featureTableBinaryPaddingCount;
	}
}
unsigned int Ci_3DTileModelHeader::GetFeatureTableBinaryLengthNoPadding()
{
	return m_featureTableBinaryLengthNoPadding;
}
unsigned int Ci_3DTileModelHeader::GetFeatureTableBinaryPaddingCount()
{
	return m_featureTableBinaryPaddingCount;
}
unsigned int Ci_3DTileModelHeader::GetFeatureTableBinaryLength()
{
	return m_featureTableBinaryLength;
}

void Ci_3DTileModelHeader::SetBatchTableJSONLengthNoPadding(unsigned int len)
{
	m_batchTableJSONLengthNoPadding = len;
	unsigned int remainder = m_batchTableJSONLengthNoPadding % 8;
	if (remainder == 0)
	{
		m_batchTableJSONLength = m_batchTableJSONLengthNoPadding;
		m_batchTableJSONPaddingCount = 0;
	}
	else
	{
		m_batchTableJSONPaddingCount = 8 - remainder;
		m_batchTableJSONLength = m_batchTableJSONLengthNoPadding + m_batchTableJSONPaddingCount;
	}
}
unsigned int Ci_3DTileModelHeader::GetBatchTableJSONLengthNoPadding()
{
	return m_batchTableJSONLengthNoPadding;
}
unsigned int Ci_3DTileModelHeader::GetBatchTableJSONPaddingCount()
{
	return m_batchTableJSONPaddingCount;
}
unsigned int Ci_3DTileModelHeader::GetBatchTableJSONLength()
{
	return m_batchTableJSONLength;
}

void Ci_3DTileModelHeader::SetBatchTableBinaryLengthNoPadding(unsigned int len)
{
	m_batchTableBinaryLengthNoPadding = len;
	unsigned int remainder = m_batchTableBinaryLengthNoPadding % 8;
	if (remainder == 0)
	{
		m_batchTableBinaryLength = m_batchTableBinaryLengthNoPadding;
		m_batchTableBinaryPaddingCount = 0;
	}
	else
	{
		m_batchTableBinaryPaddingCount = 8 - remainder;
		m_batchTableBinaryLength = m_batchTableBinaryLengthNoPadding + m_batchTableBinaryPaddingCount;
	}
}
unsigned int Ci_3DTileModelHeader::GetBatchTableBinaryLengthNoPadding()
{
	return m_batchTableBinaryLengthNoPadding;
}
unsigned int Ci_3DTileModelHeader::GetBatchTableBinaryPaddingCount()
{
	return m_batchTableBinaryPaddingCount;
}
unsigned int Ci_3DTileModelHeader::GetBatchTableBinaryLength()
{
	return m_batchTableBinaryLength;
}

void Ci_3DTileModelHeader::SetByteLength(unsigned int len)
{
	m_byteLength = len;
}
unsigned int Ci_3DTileModelHeader::GetByteLength()
{
	return m_byteLength;
}

#pragma endregion