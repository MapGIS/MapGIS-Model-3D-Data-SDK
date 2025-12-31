#include "stdafx.h"
#include "ci_assist.h"
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <boost/uuid/uuid.hpp>            // uuid类定义
#include <boost/uuid/uuid_generators.hpp> // uuid生成器
#include <boost/uuid/uuid_io.hpp>         // 流操作符重载

bool JsonValueToBool(const rapidjson::Value& jsVal)
{
	bool val = false;
	if (jsVal.IsBool())
	{
		val = jsVal.GetBool();
	}
	return val;
}

unsigned char JsonValueToUChar(const rapidjson::Value& jsVal)
{
	unsigned char val = 0;
	if (jsVal.IsUint())
	{
		unsigned int value = jsVal.GetUint();
		if (value >= 0 && value <= 255)
		{
			val = (unsigned char)value;
		}
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value > 0 - 1E-6 && value < 255 + 1E-6 && fabs(value - (int)value)<1E-6)
		{
			val = (unsigned char)value;
		}
	}
	return val;
}
char JsonValueToChar(const rapidjson::Value& jsVal)
{
	char val = 0;
	if (jsVal.IsInt())
	{
		int value = jsVal.GetInt();
		if (value >= -128 && value <= 127)
		{
			val = (char)value;
		}
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value > -128 - 1E-6 && value < 127 + 1E-6 && fabs(value - (int)value)<1E-6)
		{
			val = (char)value;
		}
	}
	return val;
}

unsigned short JsonValueToUShort(const rapidjson::Value& jsVal)
{
	unsigned short val = 0;
	if (jsVal.IsUint())
	{
		unsigned int value = jsVal.GetUint();
		if (value >= 0 && value <= 65535)
		{
			val = (unsigned short)value;
		}
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value >  -1E-6 && value < 65535 + 1E-6 && fabs(value - (int)value)<1E-6)
		{
			val = (unsigned short)value;
		}
	}
	return val;
}
short JsonValueToShort(const rapidjson::Value& jsVal)
{
	short val = 0;
	if (jsVal.IsInt())
	{
		int value = jsVal.GetInt();
		if (value >= -32768 && value <= 32767)
		{
			val = (short)value;
		}
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value > -32768 - 1E-6 && value < 32767 + 1E-6 && fabs(value - (int)value)<1E-6)
		{
			val = (short)value;
		}
	}
	return val;
}

unsigned int JsonValueToUInt(const rapidjson::Value& jsVal)
{
	unsigned int val = 0;
	if (jsVal.IsUint())
	{
		val = jsVal.GetUint();
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value > -1E-6 && value < 4294967295 + 1E-6 && fabs(value - (unsigned int)value)<1E-6)
		{
			val = (unsigned int)value;
		}
	}
	return val;
}
int JsonValueToInt(const rapidjson::Value& jsVal)
{
	int val = 0;
	if (jsVal.IsInt())
	{
		val = jsVal.GetInt();
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value > -2147483648 - 1E-6 && value < 2147483647 + 1E-6 && fabs(value - (int)value)<1E-6)
		{
			val = (int)value;
		}
	}
	return val;
}

unsigned long long JsonValueToUInt64(const rapidjson::Value& jsVal)
{
	unsigned long long val = 0;
	if (jsVal.IsUint64())
	{
		val = jsVal.GetUint64();
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value > -1E-6 && value < 18446744073709551615 + 1E-6 && fabs(value - (unsigned long long)value)<1E-6)
		{
			val = (unsigned long long)value;
		}
	}
	return val;
}
long long JsonValueToInt64(const rapidjson::Value& jsVal)
{
	long long val = 0;
	if (jsVal.IsInt64())
	{
		val = jsVal.GetInt64();
	}
	else if (jsVal.IsNumber())
	{
		double  value = jsVal.GetDouble();
		if (value >-9223372036854775808 - 1E-6 && value < 9223372036854775807 + 1E-6 && fabs(value - (long long)value)<1E-6)
		{
			val = (long long)value;
		}
	}
	return val;
}

float JsonValueToFloat(const rapidjson::Value& jsVal)
{
	float val = 0;
	if (jsVal.IsFloat())
	{
		val = jsVal.GetFloat();
	}
	return val;
}
double JsonValueToDouble(const rapidjson::Value& jsVal)
{
	double val = 0;
	if (jsVal.IsNumber())
	{
		val = jsVal.GetDouble();
	}
	return val;
}

std::string EscapeJson(const CGString &s)
{
	std::ostringstream o;
	const char * value = s.CStr();
	for (int i = 0; i < s.GetLength(); i++)
	{
		switch (value[i]) {
		case '\"': o << "\\\""; break;
		case '\\': o << "\\\\"; break;
		case '\b': o << "\\b"; break;
		case '\f': o << "\\f"; break;
		case '\n': o << "\\n"; break;
		case '\r': o << "\\r"; break;
		case '\t': o << "\\t"; break;
		default:
			o << value[i];
		}
	}
	return o.rdbuf()->str();
}

rapidjson::Value ToStringValue(const std::string& str, rapidjson::Document::AllocatorType& a)
{
	rapidjson::Value v;
	CGString val = CGString(str, CGString::EncodeType::GB18030);
	val.Convert(CGString::EncodeType::UTF8);
	v.SetString(val.CStr(), (uint32_t)(val.GetLength()), a);
	return v;
}

rapidjson::Value ToStringValue(const CGString& str, rapidjson::Document::AllocatorType& a)
{
	rapidjson::Value v;
	CGString val = str.Converted(CGString::EncodeType::UTF8);
	v.SetString(val.CStr(), (uint32_t)(val.GetLength()), a);
	return v;
}

rapidjson::Value ToStringValue(CGString& str, rapidjson::Document::AllocatorType& a)
{
	rapidjson::Value v;
	str.Convert(CGString::EncodeType::UTF8);
	v.SetString(str.CStr(), (uint32_t)(str.GetLength()), a);
	return v;
}

gisLONG GUID2String(GUID guid, char* szGuid)
{
	std::stringstream ss;
	char buffer1[12];
	memset(buffer1, 0, sizeof(char) * 12);
	snprintf(buffer1, sizeof(buffer1), "%08X", guid.Data1); 
	ss << buffer1;

	memset(buffer1, 0, sizeof(char) * 12);
	snprintf(buffer1, sizeof(buffer1), "%04X", guid.Data2);
	ss << buffer1;

	memset(buffer1, 0, sizeof(char) * 12);
	snprintf(buffer1, sizeof(buffer1), "%04X", guid.Data3);
	ss << buffer1;
	for (int i = 0; i < 8; ++i) {
		memset(buffer1, 0, sizeof(char) * 12);
		snprintf(buffer1, sizeof(buffer1), "%02X", guid.Data4[i]);
		ss << buffer1;
	}
	strcpy(szGuid, ss.str().c_str());
	// 转为大写（生成新字符串）
	std::transform(szGuid, szGuid +strlen(szGuid), szGuid,
		[](unsigned char c) { return std::toupper(c); });
	return(1);
}

gisLONG String2GUID(const char* szGuid, GUID& guid)
{
	// 分段解析
	unsigned long  data1;
	unsigned short data2, data3;
	unsigned char  data4[8];

	// 使用 sscanf 解析 32 字符的字符串
	int items = sscanf(
		szGuid,
		"%08lx%04hx%04hx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
		&data1, &data2, &data3,
		&data4[0], &data4[1], &data4[2], &data4[3],
		&data4[4], &data4[5], &data4[6], &data4[7]);

	if (items != 11) {
		return false;
	}

	// 填充 GUID
	guid.Data1 = data1;
	guid.Data2 = data2;
	guid.Data3 = data3;
	memcpy(guid.Data4, data4, 8);
	return(1);
}

gisLONG  GetNewGuidToString(char* szGuid)
{
	GUID uid;
	// 创建一个基于随机数的UUID生成器
	boost::uuids::random_generator generator;
	// 生成一个UUID
	boost::uuids::uuid uuid = generator();
	std::memcpy(&uid, uuid.data, sizeof(GUID));

	return(GUID2String(uid, szGuid));
}

rapidjson::Value ToJsonValue(const MapGIS::Tile::RecordItemValue & recordValue)
{
	//rapidjson::Value dd = rapidjson::Value(rapidjson::Type::kNullType);
	if (recordValue.IsNull())
		return rapidjson::Value(rapidjson::Type::kNullType);
	switch (recordValue.GetFieldType())
	{
	case MapGIS::Tile::FieldType::BoolType:
		return  rapidjson::Value(recordValue.ToBool());
	case MapGIS::Tile::FieldType::Int8Type:
		return  rapidjson::Value((int)recordValue.ToInt8());
	case MapGIS::Tile::FieldType::Uint8Type:
		return  rapidjson::Value((int)recordValue.ToUint8());
	case MapGIS::Tile::FieldType::Int16Type:
		return  rapidjson::Value((int)recordValue.ToInt16());
	case MapGIS::Tile::FieldType::Uint16Type:
		return  rapidjson::Value((int)recordValue.ToUint16());
	case MapGIS::Tile::FieldType::Int32Type:
		return  rapidjson::Value(recordValue.ToInt32());
	case MapGIS::Tile::FieldType::Uint32Type:
		return  rapidjson::Value(static_cast<int64_t>(recordValue.ToUint32()));
	case MapGIS::Tile::FieldType::Int64Type:
		return  rapidjson::Value((static_cast<int64_t>(recordValue.ToInt64())));
	case MapGIS::Tile::FieldType::Uint64Type:
		return  rapidjson::Value(static_cast<int64_t>(recordValue.ToUint64()));
	case MapGIS::Tile::FieldType::FloatType:
		return  rapidjson::Value(recordValue.ToFloat());
	case MapGIS::Tile::FieldType::DoubleType:
		return  rapidjson::Value(recordValue.ToDouble());
	case MapGIS::Tile::FieldType::TextType:
	{
		CGString text = recordValue.ToString(CGString::EncodeType::UTF8);
		return  rapidjson::Value(text.CStr(), text.GetLength());
	}
	case MapGIS::Tile::FieldType::DateTimeType:
		return rapidjson::Value(static_cast<int64_t>(recordValue.ToDateTime()));
	default:
		return rapidjson::Value();
	}
	return rapidjson::Value();
}

MapGIS::Tile::RecordItemValue FromJsonValue(const rapidjson::Value& jsVal, MapGIS::Tile::FieldType type)
{
	switch (type)
	{
	case MapGIS::Tile::FieldType::BoolType:
	{
		if (jsVal.IsBool())
			return jsVal.GetBool();
		else
			return  false;
	}
	break;
	case MapGIS::Tile::FieldType::Int8Type:
	{
		return JsonValueToChar(jsVal);
	}
	break;
	case MapGIS::Tile::FieldType::Uint8Type:
	{
		return JsonValueToUChar(jsVal);
	}
	break;
	case MapGIS::Tile::FieldType::Int16Type:
	{
		return JsonValueToShort(jsVal);
	}
	break;
	case MapGIS::Tile::FieldType::Uint16Type:
	{
		return JsonValueToUShort(jsVal);
	}
	break;
	case MapGIS::Tile::FieldType::Int32Type:
	{
		return JsonValueToInt(jsVal);
	}
	break;
	case MapGIS::Tile::FieldType::Uint32Type:
	{
		return JsonValueToUInt(jsVal);
	}
	break;
	case MapGIS::Tile::FieldType::Int64Type:
	case MapGIS::Tile::FieldType::DateTimeType:
	{
		long long val = JsonValueToInt64(jsVal);
		if (type == MapGIS::Tile::FieldType::Int64Type)
			return val;
		else
			return MapGIS::Tile::RecordItemValue(val, true);
	}
	break;
	case MapGIS::Tile::FieldType::Uint64Type:
	{
		return JsonValueToUInt64(jsVal);
	}
	break;
	case MapGIS::Tile::FieldType::FloatType:
	{
		float val = 0;
		if (jsVal.IsFloat())
		{
			val = jsVal.GetFloat();
		}
		return val;
	}
	break;
	case MapGIS::Tile::FieldType::DoubleType:
	{
		double val = 0;
		if (jsVal.IsNumber())
		{
			val = jsVal.GetDouble();
		}
		return val;
	}
	break;
	case MapGIS::Tile::FieldType::TextType:
	{
		CGString val = "";
		if (jsVal.IsString())
		{
			val = jsVal.GetString();
			val.SetEncodeType(CGString::EncodeType::UTF8);
		}
		return val;
	}
	break;
	case MapGIS::Tile::FieldType::Undefined:
	default:

		return MapGIS::Tile::RecordItemValue();
	}
}

void UpdateId(vector<MapGIS::Tile::ModelInstance>* pInstances, vector<gisINT64> & ids)
{
	if (pInstances == NULL || ids.size() <= 0)
		return;
	for (vector<MapGIS::Tile::ModelInstance>::iterator itr = pInstances->begin(); itr != pInstances->end(); itr++)
	{
		if (itr->hasId && ids.size() > itr->id)
		{
			itr->id = ids[itr->id];
		}
	}
}

void UpdateId(MapGIS::Tile::G3DModel* pModel, vector<gisINT64> & ids)
{
	if (pModel == NULL || ids.size() <= 0)
		return;
	if (pModel->GetGeometryType() == MapGIS::Tile::GeometryType::Point)
	{
		MapGIS::Tile::PointsModel* pPointsModel = dynamic_cast<MapGIS::Tile::PointsModel *>(pModel);
		if (pPointsModel != NULL)
		{
			for (vector<MapGIS::Tile::PointFeature>::iterator itr = pPointsModel->features.begin(); itr != pPointsModel->features.end(); itr++)
			{
				if (ids.size() > itr->id)
					itr->id = ids[itr->id];
			}
		}
	}
	else if (pModel->GetGeometryType() == MapGIS::Tile::GeometryType::Line)
	{
		MapGIS::Tile::LinesModel* pLineModel = dynamic_cast<MapGIS::Tile::LinesModel *>(pModel);
		if (pLineModel != NULL)
		{
			for (vector<MapGIS::Tile::LineFeature>::iterator itr = pLineModel->features.begin(); itr != pLineModel->features.end(); itr++)
			{
				if (ids.size() > itr->id)
					itr->id = ids[itr->id];
			}
		}
	}
	else if (pModel->GetGeometryType() == MapGIS::Tile::GeometryType::Surface)
	{
		MapGIS::Tile::SurfacesModel * pSurfacesModel = dynamic_cast<MapGIS::Tile::SurfacesModel *>(pModel);
		if (pSurfacesModel != NULL)
		{
			for (vector<MapGIS::Tile::SurfaceFeature>::iterator itr = pSurfacesModel->features.begin(); itr != pSurfacesModel->features.end(); itr++)
			{
				if (!itr->ids.empty())
				{
					for (int i = 0; i < itr->ids.size(); i++)
					{
						if (ids.size() > itr->ids[i])
							itr->ids[i] = ids[itr->ids[i]];
					}
				}
				if (itr->id >= 0)
				{
					if (ids.size() > itr->id)
						itr->id = ids[itr->id];
				}
			}
		}
	}
	else if (pModel->GetGeometryType() == MapGIS::Tile::GeometryType::Entity)
	{
		MapGIS::Tile::EntitiesModel * pEntitiesModel = dynamic_cast<MapGIS::Tile::EntitiesModel *>(pModel);
		if (pEntitiesModel != NULL)
		{
			for (vector<MapGIS::Tile::EntityFeature>::iterator itr = pEntitiesModel->features.begin(); itr != pEntitiesModel->features.end(); itr++)
			{
				if (!itr->ids.empty())
				{
					for (int i = 0; i < itr->ids.size(); i++)
					{
						if (ids.size() > itr->ids[i])
							itr->ids[i] = ids[itr->ids[i]];
					}
				}

				if (itr->id >= 0)
				{
					if (ids.size() > itr->id)
						itr->id = ids[itr->id];
				}
			}
		}
	}
}