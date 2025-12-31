#pragma once
#include"stdafx.h"
#include "../include/g3dtilerecord.h"
#include <string>

using namespace std;
using namespace MapGIS::Tile;

RecordItemValue::RecordItemValue()
{
	this->m_type = FieldType::Undefined;
	m_isNull = true;
}

RecordItemValue::RecordItemValue(FieldType type)
{
	this->m_type = type;
	m_isNull = true;
}

RecordItemValue::RecordItemValue(const bool &b)
{
	this->m_type = FieldType::BoolType;
	if (b)
		m_fieldValue.push_back('1');
	else
		m_fieldValue.push_back('0');
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const char &c)
{
	this->m_type = FieldType::Int8Type;
	m_fieldValue.push_back(c);
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const unsigned char& c)
{
	this->m_type = FieldType::Uint8Type;
	m_fieldValue.push_back(c);
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const short &n)
{
	this->m_type = FieldType::Int16Type;
	char* p = (char*)&n;
	for (int i = 0; i < 2; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}
RecordItemValue::RecordItemValue(const unsigned short& n)
{
	this->m_type = FieldType::Uint16Type;
	char* p = (char*)&n;
	for (int i = 0; i < 2; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const int& n)
{
	this->m_type = FieldType::Int32Type;
	char* p = (char*)&n;
	for (int i = 0; i < 4; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}
RecordItemValue::RecordItemValue(const unsigned int& n)
{
	this->m_type = FieldType::Uint32Type;
	char* p = (char*)&n;
	for (int i = 0; i < 4; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const gisINT64& n, bool isDateTime)
{
	if (isDateTime)
		this->m_type = FieldType::DateTimeType;
	else
		this->m_type = FieldType::Int64Type;
	char* p = (char*)&n;
	for (int i = 0; i < 8; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const gisUINT64& n)
{
	this->m_type = FieldType::Uint64Type;
	char* p = (char*)&n;
	for (int i = 0; i < 8; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const float& f)
{
	this->m_type = FieldType::FloatType;
	char* p = (char*)&f;
	for (int i = 0; i < 4; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}
RecordItemValue::RecordItemValue(const double& f)
{
	this->m_type = FieldType::DoubleType;
	char* p = (char*)&f;
	for (int i = 0; i < 8; i++, p++) {
		m_fieldValue.push_back(*p);
	}
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const CGString &s)
{
	this->m_type = FieldType::TextType;
	if (s.GetEncodeType() == CGString::EncodeType::UTF8)
	{
		m_fieldValue.emplace_back('1');
	}
	else
		m_fieldValue.emplace_back('0');
	if (s.GetLength() > 0)
		m_fieldValue.insert(this->m_fieldValue.end(), s.CStr(), s.CStr() + s.GetLength());
	m_isNull = false;
}

RecordItemValue::RecordItemValue(const RecordItemValue &other) :m_type(other.m_type), m_fieldValue(other.m_fieldValue), m_isNull(other.m_isNull)
{
}

RecordItemValue::RecordItemValue(RecordItemValue &&other) : m_type(other.m_type), m_isNull(other.m_isNull)
{
	other.m_type = FieldType::Undefined;
	m_fieldValue.swap(other.m_fieldValue);
}

RecordItemValue& MapGIS::Tile::RecordItemValue::operator =(const RecordItemValue &other)
{
	if (this == &other)
		return *this;
	this->m_type = other.m_type;
	this->m_isNull = other.m_isNull;
	this->m_fieldValue.clear();
	this->m_fieldValue = other.m_fieldValue;
	return *this;
}

RecordItemValue & RecordItemValue::operator =(RecordItemValue &&other)
{
	if (this == &other)
		return *this;
	this->m_type = other.m_type;
	this->m_isNull = other.m_isNull;
	this->m_fieldValue.clear();
	this->m_fieldValue.swap(other.m_fieldValue);
	return *this;
}

void RecordItemValue::SetNull()
{
	m_isNull = true;
	m_fieldValue.clear();
}

FieldType RecordItemValue::GetFieldType() const
{
	return m_type;
}
RecordItemValue::~RecordItemValue() {};
bool RecordItemValue::IsNull() const { return m_isNull; }
bool RecordItemValue::IsBool() const { return m_type == FieldType::BoolType; }
bool RecordItemValue::IsInt8() const { return m_type == FieldType::Int8Type; }
bool RecordItemValue::IsUint8() const { return m_type == FieldType::Uint8Type; }
bool RecordItemValue::IsInt16() const { return m_type == FieldType::Int16Type; }
bool RecordItemValue::IsUint16() const { return m_type == FieldType::Uint16Type; }
bool RecordItemValue::IsInt32() const { return m_type == FieldType::Int32Type; }
bool RecordItemValue::IsUint32() const { return m_type == FieldType::Uint32Type; }
bool RecordItemValue::IsInt64() const { return m_type == FieldType::Int64Type; }
bool RecordItemValue::IsUint64() const { return m_type == FieldType::Uint64Type; }
bool RecordItemValue::IsFloat() const { return m_type == FieldType::FloatType; }
bool RecordItemValue::IsDouble() const { return m_type == FieldType::DoubleType; }

bool RecordItemValue::IsNumber() const
{
	if (m_type == FieldType::Int8Type || m_type == FieldType::Uint8Type || m_type == FieldType::Int16Type || m_type == FieldType::Uint16Type
		|| m_type == FieldType::Int32Type || m_type == FieldType::Uint32Type || m_type == FieldType::Int64Type || m_type == FieldType::Uint64Type
		|| m_type == FieldType::FloatType || m_type == FieldType::DoubleType)
		return true;
	return false;
}
bool RecordItemValue::IsText() const { return m_type == FieldType::TextType; }
bool RecordItemValue::IsDateTime() const { return m_type == FieldType::DateTimeType; }

template <typename T>
T ToNumber(const MapGIS::Tile::RecordItemValue& item)
{
	switch (item.GetFieldType())
	{
	case MapGIS::Tile::FieldType::BoolType:
		return (T)(item.ToBool());
	case MapGIS::Tile::FieldType::Int8Type:
		return  (T)(item.ToInt8());
	case MapGIS::Tile::FieldType::Uint8Type:
		return  (T)(item.ToUint8());
	case MapGIS::Tile::FieldType::Int16Type:
		return  (T)(item.ToInt16());
	case MapGIS::Tile::FieldType::Uint16Type:
		return  (T)(item.ToUint16());
	case MapGIS::Tile::FieldType::Int32Type:
		return  (T)(item.ToInt32());
	case MapGIS::Tile::FieldType::Uint32Type:
		return  (T)(item.ToUint32());
	case MapGIS::Tile::FieldType::Int64Type:
		return  (T)(item.ToInt64());
	case MapGIS::Tile::FieldType::Uint64Type:
		return  (T)(item.ToUint64());
	case MapGIS::Tile::FieldType::FloatType:
		return  (T)(item.ToFloat());
	case MapGIS::Tile::FieldType::DoubleType:
		return  (T)(item.ToDouble());
	case MapGIS::Tile::FieldType::DateTimeType:
		return (T)(item.ToDateTime());
	case MapGIS::Tile::FieldType::TextType:
	case MapGIS::Tile::FieldType::Undefined:
	default:
		return (T)0;
	}
	return (T)0;
}

bool MapGIS::Tile::RecordItemValue::ToBool() const
{
	if (m_type != FieldType::BoolType)
		return false;
	if (m_fieldValue.size() > 0 && m_fieldValue[0] == '1')
		return true;
	return false;
}

char MapGIS::Tile::RecordItemValue::ToInt8() const
{
	if (m_type == FieldType::Int8Type)
	{
		if (m_fieldValue.size() > 0)
			return m_fieldValue[0];
		return 0;
	}
	else
		return ToNumber<char>(*this);
}

unsigned char  MapGIS::Tile::RecordItemValue::ToUint8() const
{
	if (m_type == FieldType::Uint8Type)
	{
		if (m_fieldValue.size() > 0)
			return m_fieldValue[0];
		return 0;
	}
	else
		return ToNumber<unsigned char>(*this);
}

short MapGIS::Tile::RecordItemValue::ToInt16() const
{
	if (m_type == FieldType::Int16Type)
	{
		if (m_fieldValue.size() == 2)
		{
			short rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 2);
			return rtn;
		}
		return 0;
	}

	return ToNumber<short>(*this);
}

unsigned short MapGIS::Tile::RecordItemValue::ToUint16() const
{
	if (m_type == FieldType::Uint16Type)
	{
		if (m_fieldValue.size() == 2)
		{
			short rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 2);
			return rtn;
		}
		return 0;
	}
	return ToNumber<unsigned short>(*this);
}

int MapGIS::Tile::RecordItemValue::ToInt32() const
{
	if (m_type == FieldType::Int32Type)
	{
		if (m_fieldValue.size() == 4)
		{
			int rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 4);
			return rtn;
		}
		return 0;
	}
	return ToNumber<int>(*this);
}

unsigned int  MapGIS::Tile::RecordItemValue::ToUint32() const
{
	if (m_type == FieldType::Uint32Type)
	{
		if (m_fieldValue.size() == 4)
		{
			unsigned int  rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 4);
			return rtn;
		}
		return 0;
	}
	return ToNumber<unsigned int>(*this);
}

gisINT64 MapGIS::Tile::RecordItemValue::ToInt64() const
{
	if (m_type == FieldType::Int64Type)
	{
		if (m_fieldValue.size() == 8)
		{
			gisINT64 rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 8);
			return rtn;
		}
		return 0;
	}
	return ToNumber<gisINT64>(*this);
}

gisUINT64 MapGIS::Tile::RecordItemValue::ToUint64() const
{
	if (m_type == FieldType::Uint64Type)
	{
		if (m_fieldValue.size() == 8)
		{
			gisUINT64  rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 8);
			return rtn;
		}
		return 0;
	}
	return ToNumber<gisUINT64>(*this);
}

float MapGIS::Tile::RecordItemValue::ToFloat() const
{
	if (m_type == FieldType::FloatType)
	{
		if (m_fieldValue.size() == 4)
		{
			float  rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 4);
			return rtn;
		}
		return 0;
	}
	return ToNumber<float>(*this);
}

double MapGIS::Tile::RecordItemValue::ToDouble() const
{
	if (m_type == FieldType::DoubleType)
	{
		if (m_fieldValue.size() == 8)
		{
			double  rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 8);
			return rtn;
		}
		return 0;
	}
	return ToNumber<double>(*this);
}

CGString MapGIS::Tile::RecordItemValue::ToText() const
{
	if (m_type != FieldType::TextType || m_isNull || m_fieldValue.size() <= 1)
		return "";
	if (m_fieldValue[0] == '1')
	{
		CGString rtn = CGString("", CGString::EncodeType::UTF8);
		rtn.Append(&m_fieldValue[1], (unsigned int)m_fieldValue.size() - 1);
		return rtn;
	}
	else
	{
		CGString rtn = CGString("", CGString::EncodeType::GB18030);
		rtn.Append(&m_fieldValue[1], (unsigned int)m_fieldValue.size() - 1);
		return rtn;
	}
	return "";
}

CGString MapGIS::Tile::RecordItemValue::ToString() const
{
	if (m_isNull)
		return "";
	switch (m_type)
	{
	case MapGIS::Tile::FieldType::BoolType:
		return  to_string(ToBool());
		break;
	case MapGIS::Tile::FieldType::Int8Type:
		return  to_string(ToInt8());
		break;
	case MapGIS::Tile::FieldType::Uint8Type:
		return  to_string(ToUint8());
		break;
	case MapGIS::Tile::FieldType::Int16Type:
		return  to_string(ToInt16());
		break;
	case MapGIS::Tile::FieldType::Uint16Type:
		return  to_string(ToUint16());
		break;
	case MapGIS::Tile::FieldType::Int32Type:
		return  to_string(ToInt32());
		break;
	case MapGIS::Tile::FieldType::Uint32Type:
		return  to_string(ToUint32());
		break;
	case MapGIS::Tile::FieldType::Int64Type:
		return  to_string(ToInt64());
		break;
	case MapGIS::Tile::FieldType::Uint64Type:
		return  to_string(ToUint64());
		break;
	case MapGIS::Tile::FieldType::FloatType:
		return  to_string(ToFloat());
		break;
	case MapGIS::Tile::FieldType::DoubleType:
		return  to_string(ToDouble());
		break;
	case MapGIS::Tile::FieldType::TextType:
		return  ToText();
		break;
	case MapGIS::Tile::FieldType::DateTimeType:
		return to_string(ToDateTime());
		break;
	case MapGIS::Tile::FieldType::Undefined:
		break;
	default:
		break;
	}
	return "";
}

CGString MapGIS::Tile::RecordItemValue::ToString(CGString::EncodeType encodeType) const
{
	CGString rtn = ToString();
	if (encodeType == CGString::EncodeType::UTF8 && rtn.GetEncodeType() != CGString::EncodeType::UTF8)
		rtn.Convert(CGString::EncodeType::UTF8);
	else if (encodeType == CGString::EncodeType::GB18030 && rtn.GetEncodeType() != CGString::EncodeType::GB18030)
		rtn.Convert(CGString::EncodeType::GB18030);
	return rtn;
}

gisINT64 MapGIS::Tile::RecordItemValue::ToDateTime() const
{
	if (m_type == FieldType::DateTimeType)
	{
		if (m_fieldValue.size() == 8)
		{
			gisINT64 rtn = 0;
			memcpy(&rtn, m_fieldValue.data(), 8);
			return rtn;
		}
	}
	return 0;
}

bool MapGIS::Tile::RecordItemValue::operator==(const RecordItemValue& val) const
{
	if (m_type != val.m_type)
		return false;
	if (m_isNull != val.m_isNull)
		return false;
	if (m_fieldValue.size() != val.m_fieldValue.size())
		return false;
	for (int i = 0; i < m_fieldValue.size(); ++i)
	{
		if (m_fieldValue[i] != val.m_fieldValue[i])
			return false;
	}
	return true;
}

bool MapGIS::Tile::RecordItemValue::operator!=(const RecordItemValue& val) const
{
	return !(*this == val);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MapGIS::Tile::RecordItems
{
public:

	RecordItems();
	RecordItems(const RecordItems &items);
	RecordItems(RecordItems &&other);
	~RecordItems();
	void AppendItem();

	void AppendItem(FieldType type);

	void AppendItem(const bool& b);

	void AppendItem(const char& c);

	void AppendItem(const unsigned char& c);

	void AppendItem(const short& n);

	void AppendItem(const unsigned short& n);

	void AppendItem(const int& n);

	void AppendItem(const unsigned int& n);

	void AppendItem(const gisINT64& n, bool isDateTime = false);

	void AppendItem(const gisUINT64& n);

	void AppendItem(const float& f);

	void AppendItem(const double& f);

	void AppendItem(const CGString &s);

	void AppendItem(const RecordItemValue &item);

	gisLONG GetNum() const;

	gisLONG GetItem(int index, RecordItemValue&value) const;

	RecordItems& operator =(const RecordItems &other);
	RecordItems& operator =(RecordItems &&other);

private:
	vector<int> m_fieldDescription;//记录类型与起始位置
	vector<char> m_fieldValues;
};

void RecordItems::AppendItem()
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Undefined);
	m_fieldDescription.emplace_back((int)m_fieldValues.size());
}

void RecordItems::AppendItem(MapGIS::Tile::FieldType type)
{
	m_fieldDescription.emplace_back((int)type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
}

void RecordItems::AppendItem(const bool& b)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::BoolType);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	if (b)
		m_fieldValues.emplace_back('1');
	else
		m_fieldValues.emplace_back('0');
}

void RecordItems::AppendItem(const char& c)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Int8Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	m_fieldValues.emplace_back(c);
}

void RecordItems::AppendItem(const unsigned char& c)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Uint8Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	m_fieldValues.emplace_back(c);
}

void RecordItems::AppendItem(const short& n)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Int16Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	char* p = (char*)&n;
	for (int i = 0; i < 2; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const unsigned short& n)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Uint16Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());

	char* p = (char*)&n;
	for (int i = 0; i < 2; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const int& n)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Int32Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	char* p = (char*)&n;
	for (int i = 0; i < 4; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const unsigned int& n)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Uint32Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());

	char* p = (char*)&n;
	for (int i = 0; i < 4; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const gisINT64& n, bool isDateTime)
{
	if (isDateTime)
		m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::DateTimeType);
	else
		m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Int64Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	char* p = (char*)&n;
	for (int i = 0; i < 8; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const gisUINT64& n)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::Uint64Type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	char* p = (char*)&n;
	for (int i = 0; i < 8; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const float& f)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::FloatType);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	char* p = (char*)&f;
	for (int i = 0; i < 4; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const double& f)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::DoubleType);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	char* p = (char*)&f;
	for (int i = 0; i < 8; i++, p++) {
		m_fieldValues.emplace_back(*p);
	}
}

void RecordItems::AppendItem(const CGString &s)
{
	m_fieldDescription.emplace_back((int)MapGIS::Tile::FieldType::TextType);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	if (s.GetEncodeType() == CGString::EncodeType::UTF8)
	{
		m_fieldValues.emplace_back('1');
	}
	else
		m_fieldValues.emplace_back('0');
	m_fieldValues.insert(this->m_fieldValues.end(), s.CStr(), s.CStr() + s.GetLength());
}

void RecordItems::AppendItem(const RecordItemValue &item)
{
	m_fieldDescription.emplace_back((int)item.m_type);
	m_fieldDescription.emplace_back(m_fieldValues.size());
	m_fieldValues.insert(this->m_fieldValues.end(), item.m_fieldValue.begin(), item.m_fieldValue.end());
}

gisLONG RecordItems::GetNum() const { return (gisLONG)m_fieldDescription.size() / 2; }

gisLONG RecordItems::GetItem(int index, RecordItemValue&value) const
{
	if (index >= m_fieldDescription.size() / 2)
		return 0;
	value.m_type = (MapGIS::Tile::FieldType)m_fieldDescription[2 * index];
	value.m_fieldValue.clear();
	if (value.m_type != MapGIS::Tile::FieldType::Undefined)
	{
		if (index + 1 == m_fieldDescription.size() / 2)
		{
			if (m_fieldValues.begin() + m_fieldDescription[2 * index + 1] == m_fieldValues.end())
				value.m_isNull = true;
			else
			{
				value.m_isNull = false;
				value.m_fieldValue.insert(value.m_fieldValue.begin(), m_fieldValues.begin() + m_fieldDescription[2 * index + 1], m_fieldValues.end());
			}
		}
		else
		{
			if (m_fieldValues.begin() + m_fieldDescription[2 * index + 1] == m_fieldValues.begin() + m_fieldDescription[2 * index + 3])
			{
				value.m_isNull = true;
			}
			else
			{
				value.m_isNull = false;
				value.m_fieldValue.insert(value.m_fieldValue.begin(), m_fieldValues.begin() + m_fieldDescription[2 * index + 1], m_fieldValues.begin() + m_fieldDescription[2 * index + 3]);
			}
		}
	}
	return 1;
}

RecordItems& RecordItems::operator =(const RecordItems &other)
{
	if (this == &other)
		return *this;
	this->m_fieldDescription.clear();
	this->m_fieldValues.clear();
	this->m_fieldDescription.insert(this->m_fieldDescription.begin(), other.m_fieldDescription.begin(), other.m_fieldDescription.end());
	this->m_fieldValues.insert(this->m_fieldValues.begin(), other.m_fieldValues.begin(), other.m_fieldValues.end());
	return *this;
}

RecordItems& RecordItems::operator =(RecordItems &&other)
{
	if (this == &other)
		return *this;
	this->m_fieldDescription.clear();
	this->m_fieldValues.clear();
	this->m_fieldDescription.swap(other.m_fieldDescription);
	this->m_fieldValues.swap(other.m_fieldValues);
	return *this;
}

RecordItems::RecordItems()
{
	m_fieldDescription.reserve(100);
	m_fieldValues.reserve(512);
}
RecordItems::RecordItems(const RecordItems &items)
{
	this->m_fieldDescription.insert(this->m_fieldDescription.begin(), items.m_fieldDescription.begin(), items.m_fieldDescription.end());
	this->m_fieldValues.insert(this->m_fieldValues.begin(), items.m_fieldValues.begin(), items.m_fieldValues.end());
}

RecordItems::RecordItems(RecordItems &&other)
{
	*this = move(other);
}
RecordItems::~RecordItems() {};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Field::Field()
{
	fieldID = 0;
	type = FieldType::Undefined;
}

Field::Field(const Field& field) :fieldID(field.fieldID), name(field.name), alias(field.alias), type(field.type)
, noDataValue(field.noDataValue), minValue(field.minValue), maxValue(field.maxValue)

{
}

Field::Field(Field&& field)
{
	fieldID = field.fieldID;
	name = move(field.name);
	alias = move(field.alias);
	type = field.type;

	noDataValue = move(field.noDataValue);
	minValue = move(field.minValue);
	maxValue = move(field.maxValue);
}

Field& Field::operator=(const Field& field)
{
	if (this == &field)
		return *this;
	fieldID = field.fieldID;
	name = field.name;
	alias = field.alias;
	type = field.type;
	noDataValue = field.noDataValue;
	minValue = field.minValue;
	maxValue = field.maxValue;
	return *this;
}

Field& Field::operator=(Field&& field)
{
	if (this == &field)
		return *this;
	fieldID = field.fieldID;
	name = move(field.name);
	alias = move(field.alias);
	type = field.type;

	noDataValue = move(field.noDataValue);
	minValue = move(field.minValue);
	maxValue = move(field.maxValue);
	return *this;
}

bool Field::operator == (const Field& field) const
{
	if (fieldID != field.fieldID)
		return false;
	if (type != field.type)
		return false;
	if (name != field.name)
		return false;
	if (alias != field.alias)
		return false;
	if (noDataValue != field.noDataValue)
		return false;
	if (minValue != field.minValue)
		return false;
	if (maxValue != field.maxValue)
		return false;
	return true;
}

bool Field::operator != (const Field& field) const
{
	return !(*this == field);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LayerFieldsInfo::LayerFieldsInfo()
{
	layerID = 0;
}

LayerFieldsInfo::LayerFieldsInfo(const LayerFieldsInfo& layerInfo)
{
	*this = layerInfo;
}

LayerFieldsInfo::LayerFieldsInfo(LayerFieldsInfo&& layerInfo)
{
	layerID = layerInfo.layerID;
	dataSource = move(layerInfo.dataSource);
	layerName = move(layerInfo.layerName);
	fieldInfos.swap(layerInfo.fieldInfos);
}

LayerFieldsInfo& LayerFieldsInfo::operator=(const LayerFieldsInfo& layerInfo)
{
	if (this == &layerInfo)
		return *this;
	layerID = layerInfo.layerID;
	dataSource = layerInfo.dataSource;
	layerName = layerInfo.layerName;
	fieldInfos = layerInfo.fieldInfos;
	return *this;
}
LayerFieldsInfo& LayerFieldsInfo::operator=(LayerFieldsInfo&& layerInfo)
{
	if (this == &layerInfo)
		return *this;
	layerID = layerInfo.layerID;
	dataSource = move(layerInfo.dataSource);
	layerName = move(layerInfo.layerName);
	fieldInfos.swap(layerInfo.fieldInfos);
	return *this;
}

bool LayerFieldsInfo::operator == (const LayerFieldsInfo& layerInfo) const
{
	if (layerID == layerInfo.layerID && dataSource == layerInfo.dataSource && layerName == layerInfo.layerName &&fieldInfos.size() == layerInfo.fieldInfos.size())
	{
		for (int i = 0; i < fieldInfos.size(); i++)
		{
			if (fieldInfos.at(i) != layerInfo.fieldInfos.at(i))
				return false;
		}
		return true;
	}
	return false;
}

bool LayerFieldsInfo::operator != (const LayerFieldsInfo& layerInfo) const
{
	if (layerID != layerInfo.layerID || dataSource != layerInfo.dataSource || layerName != layerInfo.layerName || fieldInfos.size() != layerInfo.fieldInfos.size())
		return true;
	for (int i = 0; i < fieldInfos.size(); i++)
	{
		if (fieldInfos.at(i) != layerInfo.fieldInfos.at(i))
			return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Record::Record()
{
	pRecordItems = new RecordItems();
	id = -1;
}

Record::Record(const Record& other)
{
	id = other.id;
	pRecordItems = new RecordItems();
	*pRecordItems = *other.pRecordItems;
}
Record::Record(Record&& other)
{
	id = other.id;
	pRecordItems = other.pRecordItems;

	other.pRecordItems =NULL;
}

Record::~Record()
{
	if (pRecordItems != NULL)
		delete pRecordItems;
	pRecordItems = NULL;
}

gisINT64 Record::GetID() const
{
	return id;
}
void Record::SetID(gisINT64 id)
{
	this->id = id;
}

void Record::AppendItem()
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem();
}

void Record::AppendItem(FieldType type)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(type);
}

void Record::AppendItem(const bool& b)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(b);
}

void Record::AppendItem(const char& c)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(c);
}

void Record::AppendItem(const unsigned char& c)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(c);
}

void Record::AppendItem(const short& n)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(n);
}

void Record::AppendItem(const unsigned short& n)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(n);
}

void Record::AppendItem(const int& n)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(n);
}

void Record::AppendItem(const unsigned int& n)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(n);
}

void Record::AppendItem(const gisINT64& n, bool isDateTime)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(n, isDateTime);
}

void Record::AppendItem(const gisUINT64& n)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(n);
}

void Record::AppendItem(const float& f)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(f);
}

void Record::AppendItem(const double& f)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(f);
}

void Record::AppendItem(const CGString &s)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(s);
}

void Record::AppendItem(const RecordItemValue &item)
{
	if (pRecordItems != NULL)
		pRecordItems->AppendItem(item);
}

gisLONG Record::GetNum() const
{
	if (pRecordItems != NULL)
		return pRecordItems->GetNum();
	return 0;
}

gisLONG Record::GetItem(int index, RecordItemValue&value) const
{
	if (pRecordItems != NULL)
		return pRecordItems->GetItem(index, value);
	return 0;
}
Record& Record::operator =(const Record& other)
{
	if (this == &other)
		return *this;
	this->id = other.id;
	*this->pRecordItems = *other.pRecordItems;
	return *this;
}

Record& Record::operator =(Record&& other)
{
	if (this == &other)
		return *this;
	this->id = other.id;
	this->pRecordItems = other.pRecordItems;
	other.pRecordItems = NULL;
	return *this;
}