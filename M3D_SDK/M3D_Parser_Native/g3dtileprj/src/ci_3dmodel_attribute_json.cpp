#include "stdafx.h"
#include "ci_assist.h"
#include "ci_3dmodel_attribute_json.h"
#include "rapidjson/document.h"
#include <unordered_map>

namespace MapGIS
{
	namespace Tile
	{
		struct Ci_RecordInfoM3D20
		{
			string fldName;
			string rcdValue;
		};

		struct Ci_RecordM3D20
		{
			gisINT64 OID;
			vector<Ci_RecordInfoM3D20> recordInfos;
		};
	};
}

//gisLONG MapGIS::Tile::Ci_ModelAttribute::From(vector<MapGIS::Tile::Record> &records, size_t recordNum)
//{
//	if (records.size() <= 0 || records.size() < recordNum)
//		return 0;
//
//	string value="";
//	if (records.size() <= 0)
//	{
//		value.append("{\"records\":[]}");
//		return 1;
//	}
//	value.append("{\"records\":[");
//	unordered_map<string, string> valueBack;
//	for (int i = 0; i < records.size(); i++)
//	{
//		if (i != 0)
//			value.append(",");
//		value.append("{\"oid\":");
//		value.append(std::to_string(records[i].OID).c_str());
//		value.append(",\"rcdValues\":[{");
//		for (int j = 0; j < records[i].recordInfos.size(); j++)
//		{
//			if (j != 0)
//				value.append(",");
//
//			value.append("\"");
//			if (records[i].recordInfos[j].fldName.length() <= 0 && records[i].recordInfos.size() == records[0].recordInfos.size())
//			{
//				unordered_map<string, string>::iterator itr = valueBack.find(records[0].recordInfos[j].fldName);
//				if (itr != valueBack.end())
//				{
//					value.append(itr->second.c_str());
//				}
//				else
//				{
//					string name = EscapeJson(records[0].recordInfos[j].fldName);
//					valueBack.insert(make_pair(records[0].recordInfos[j].fldName, name));
//					value.append(name.c_str());
//				}
//			}
//			else if (records[i].recordInfos[j].fldName.length() > 0)
//			{
//				unordered_map<string, string>::iterator itr = valueBack.find(records[i].recordInfos[j].fldName);
//				if (itr != valueBack.end())
//				{
//					value.append(itr->second.c_str());
//				}
//				else
//				{
//					string name = EscapeJson(records[i].recordInfos[j].fldName);
//					valueBack.insert(make_pair(records[i].recordInfos[j].fldName, name));
//					value.append(name.c_str());
//				}
//			}
//			value.append("\":\"");
//
//			unordered_map<string, string>::iterator itr = valueBack.find(records[i].recordInfos[j].rcdValue);
//			if (itr != valueBack.end())
//			{
//				value.append(itr->second.c_str());
//			}
//			else
//			{
//				string rcdValue = EscapeJson(records[i].recordInfos[j].rcdValue);
//				valueBack.insert(make_pair(records[i].recordInfos[j].rcdValue, rcdValue));
//				value.append(rcdValue.c_str());
//			}
//			value.append("\"");
//		}
//		value.append("}]}");
//	}
//	value.append("]}");
//	CGString convertValue(value, CGString::EncodeType::GB18030);
//	convertValue.Convert(CGString::EncodeType::UTF8);
//	m_data.append(convertValue.CStr(), convertValue.GetLength());
//	return 1;
//}
//
//gisLONG MapGIS::Tile::Ci_ModelAttribute::From(vector<MapGIS::Tile::Record> & records)
//{
//	return this->From(records, records.size());
//}

gisLONG MapGIS::Tile::Ci_ModelAttributeJsonFile::From(LayerAttribute& attribute)
{
	string value = "";
	value.reserve(4096);
	if (attribute.layerInfo.fieldInfos.size() <= 0)
	{
		value.append("{\"records\":[]}");
		return 1;
	}
	value.append("{\"records\":[");
	unordered_map<string, string> valueBack;

	vector<string> names;
	vector<bool> isText;
	for (int j = 0; j < attribute.layerInfo.fieldInfos.size(); j++)
	{
		CGString gbkName=  attribute.layerInfo.fieldInfos[j].name.Converted(CGString::EncodeType::GB18030);
		string name = EscapeJson(gbkName);
		valueBack.insert(make_pair(gbkName.CStr(), name));
		names.emplace_back(name.c_str());
		isText.push_back(attribute.layerInfo.fieldInfos[j].type == FieldType::TextType);
	}
	RecordItemValue item;
	for (vector<Record>::iterator itr = attribute.records.begin(); itr != attribute.records.end(); itr++)
	{
		gisLONG num =  itr->GetNum();
		if (itr != attribute.records.begin())
			value.append(",");
		value.append("{\"oid\":");
		value.append(std::to_string(itr->GetID()).c_str());
		value.append(",\"rcdValues\":[{");
		for (int j = 0; j < attribute.layerInfo.fieldInfos.size(); j++)
		{
			itr->GetItem(j, item);
			if (j != 0)
				value.append(",");
			value.append("\"");
			value.append(names[j].c_str());
			value.append("\":\"");
			if (num > j)
			{
				if (isText[j])
				{
					CGString itemValue = item.ToString(CGString::EncodeType::GB18030);
					unordered_map<string, string>::iterator valueItr = valueBack.find(itemValue.CStr());
					if (valueItr != valueBack.end())
					{
						value.append(valueItr->second.c_str());
					}
					else
					{
						string rcdValue = EscapeJson(itemValue.CStr());
						valueBack.insert(make_pair(itemValue.CStr(), rcdValue));
						value.append(rcdValue.c_str());
					}
				}
				else
				{
					value.append(item.ToString().CStr());
				}
			}
			value.append("\"");
		}
		value.append("}]}");
	}
	value.append("]}");

	CGString convertValue(value, CGString::EncodeType::GB18030);
	convertValue.Convert(CGString::EncodeType::UTF8);
	m_data.append(convertValue.CStr(), convertValue.GetLength());
	return 1;
}

gisLONG ToRecordVector(CGByteArray & data,vector<MapGIS::Tile::Ci_RecordM3D20> &records)
{
	if (data.size() <= 0)
		return 0;
	CGString bufferStr("",CGString::EncodeType::UTF8);
	bufferStr.Append(data.data(), data.size());
	bufferStr.Convert(CGString::EncodeType::GB18030);

	rapidjson::Document doc;
	if (doc.Parse(bufferStr.CStr(), bufferStr.GetLength()).HasParseError())
		return 0;
	if (!doc.IsObject())
		return 0;

	if (doc.HasMember("records") && doc["records"].IsArray())
	{
		rapidjson::Value& recordsArray = doc["records"];
		records.reserve(recordsArray.Size());
		for (int i = 0; i < recordsArray.Size(); i++)
		{
			if (!recordsArray[i].IsObject())
				continue;

			MapGIS::Tile::Ci_RecordM3D20 item;
			rapidjson::Value& objItem = recordsArray[i];

			if(objItem.HasMember("oid"))
				item.OID = JsonValueToInt64(objItem["oid"]);

			if (objItem.HasMember("rcdValues") && objItem["rcdValues"].IsArray())
			{
				rapidjson::Value& valuesArray = objItem["rcdValues"];
				for (int j = 0; j < valuesArray.Size(); j++)
				{
					if (valuesArray[i].IsObject())
					{
						rapidjson::Value& objvalues = valuesArray[i];
						item.recordInfos.reserve(objvalues.MemberCount());

						for (rapidjson::Value::MemberIterator memberItr = objvalues.MemberBegin(); memberItr != objvalues.MemberEnd(); memberItr++)
						{
							const rapidjson::Value& jsonValue = memberItr->value;
							if (memberItr->name.IsString() && memberItr->value.IsString())
							{
								CGString name (memberItr->name.GetString(),CGString::EncodeType::UTF8) ;
								name.Convert(CGString::EncodeType::GB18030);
								CGString value(memberItr->value.GetString(), CGString::EncodeType::UTF8);
								value.Convert(CGString::EncodeType::GB18030);
								MapGIS::Tile::Ci_RecordInfoM3D20 info;
								info.fldName = name.StdString();
								info.rcdValue = value.StdString();
								item.recordInfos.emplace_back(info);
							}
						}
					}
				}
			}
			records.emplace_back(item);
		}
	}
	return 1;
}

gisLONG MapGIS::Tile::Ci_ModelAttributeJsonFile::To(LayerAttribute &attribute)
{
	vector<MapGIS::Tile::Ci_RecordM3D20> records;
	ToRecordVector(m_data, records);
	unordered_map<string, int> indexMap;
	if (records.size() > 0)
	{
		for (vector<MapGIS::Tile::Ci_RecordM3D20>::iterator itr = records.begin(); itr != records.end(); itr++)
		{
			for (vector<Ci_RecordInfoM3D20>::iterator infoItr = itr->recordInfos.begin(); infoItr != itr->recordInfos.end(); infoItr++)
			{
				if (indexMap.find(infoItr->fldName) == indexMap.end())
				{
					indexMap.insert(make_pair(infoItr->fldName, indexMap.size()));
					Field field;
					field.name = infoItr->fldName;
					field.type = FieldType::TextType;
					field.fieldID = CalcAttFieldID(infoItr->fldName);
					attribute.layerInfo.fieldInfos.emplace_back(field);
				}
			}
		}

		for (vector<MapGIS::Tile::Ci_RecordM3D20>::iterator itr = records.begin(); itr != records.end(); itr++)
		{
			Record record;
			record.SetID(itr->OID);
			//record.recordInfos.resize(indexMap.size());
			for (vector<Ci_RecordInfoM3D20>::iterator infoItr = itr->recordInfos.begin(); infoItr != itr->recordInfos.end(); infoItr++)
			{
				unordered_map<string, int>::iterator indexItr = indexMap.find(infoItr->fldName);
				if (indexItr != indexMap.end())
				{
					record.AppendItem(infoItr->rcdValue);
				}
			}
			attribute.records.emplace_back(record);
		}
		return 1;
	}
	return 1;
}