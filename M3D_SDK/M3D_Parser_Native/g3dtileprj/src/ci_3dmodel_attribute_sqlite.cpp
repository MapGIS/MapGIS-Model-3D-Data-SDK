#include "stdafx.h"
#include "ci_3dmodel_attribute_sqlite.h"
#include "cgfile.h"
#include <cstdlib>
#include "ci_assist.h"

using namespace MapGIS::Tile;

map<string, Ci_3DModelAttributeSqlite*>  Ci_3DModelAttributeSqlite::m_mapSqlitePath;

Ci_3DModelAttributeSqlite::Ci_3DModelAttributeSqlite(string sqlitePath)
{
    m_pDb = NULL;
    m_sqlitePath = sqlitePath;
}
Ci_3DModelAttributeSqlite::~Ci_3DModelAttributeSqlite()
{
    Close(false);
}

Ci_3DModelAttributeSqlite* Ci_3DModelAttributeSqlite::GetInstance(string sqlitePath)
{
    map<string, Ci_3DModelAttributeSqlite*> ::iterator itr = m_mapSqlitePath.find(sqlitePath);
    if (itr != m_mapSqlitePath.end())
        return itr->second;
    else
    {
        Ci_3DModelAttributeSqlite* pSqlite = new Ci_3DModelAttributeSqlite(sqlitePath);
        m_mapSqlitePath.insert(pair<string, Ci_3DModelAttributeSqlite*>(sqlitePath, pSqlite));
        return pSqlite;
    }
}

void Ci_3DModelAttributeSqlite::Free3DModelAttributeSqlite(Ci_3DModelAttributeSqlite* pOwner)
{
    if (pOwner == NULL)
        return;

    map<string, Ci_3DModelAttributeSqlite*> ::iterator itr = m_mapSqlitePath.find(pOwner->m_sqlitePath);
    if (itr != m_mapSqlitePath.end())
        m_mapSqlitePath.erase(itr);
    if (pOwner != NULL)
    {
        delete pOwner;
    }
}



gisLONG i_ReadFilesInfo(sqlite3* pDb, vector<LayerFieldsInfo>& layerInfo)
{
	if (pDb == NULL)
		return -1;
	sqlite3_stmt* stmt;
	CGString sql = CGString("SELECT * FROM LayerFieldInfo;", CGString::EncodeType::GB18030).Converted(CGString::EncodeType::UTF8);

	map<int64_t, LayerFieldsInfo*> mapLayerInfo;
	LayerFieldsInfo* pLayerInfo = NULL;
	if (sqlite3_prepare_v2(pDb, sql.CStr(), -1, &stmt, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			int id = sqlite3_column_int64(stmt, 0);
			int64_t LayerID = sqlite3_column_int64(stmt, 1);
			const unsigned char* LayerDataSource = sqlite3_column_text(stmt, 2);
			const unsigned char* LayerName = sqlite3_column_text(stmt, 3);
			int64_t FieldID = sqlite3_column_int64(stmt, 4);
			const unsigned char* fieldName = sqlite3_column_text(stmt, 5);
			const unsigned char* fieldAlias = sqlite3_column_text(stmt, 6);
			const unsigned char* fieldType = sqlite3_column_text(stmt, 7);
			if (mapLayerInfo.find(LayerID) == mapLayerInfo.end())
			{
				pLayerInfo = new LayerFieldsInfo();
				mapLayerInfo.insert(pair<int64_t, LayerFieldsInfo*>(LayerID, pLayerInfo));
				pLayerInfo->layerID = LayerID;
				pLayerInfo->dataSource = CGString((char*)LayerDataSource, CGString::EncodeType::UTF8);
				pLayerInfo->layerName = CGString((char*)LayerName, CGString::EncodeType::UTF8);
			}
			else
				pLayerInfo = mapLayerInfo[LayerID];

			Field field;
			field.fieldID = FieldID;
			field.name = CGString((char*)fieldName, CGString::EncodeType::UTF8);
			field.alias = CGString((char*)fieldAlias, CGString::EncodeType::UTF8);
			string type = CGString((char*)fieldType, CGString::EncodeType::UTF8).Convert(CGString::EncodeType::GB18030).CStr();
			if (StrICmp(type.c_str(), "BOOL") == 0)
				field.type = FieldType::BoolType;
			else if (StrICmp(type.c_str(), "INT8") == 0)
				field.type = FieldType::Int8Type;
			else if (StrICmp(type.c_str(), "UINT8") == 0)
				field.type = FieldType::Uint8Type;
			else if (StrICmp(type.c_str(), "INT16") == 0)
				field.type = FieldType::Int16Type;
			else if (StrICmp(type.c_str(), "UINT16") == 0)
				field.type = FieldType::Uint16Type;
			else if (StrICmp(type.c_str(), "INT32") == 0)
				field.type = FieldType::Int32Type;
			else if (StrICmp(type.c_str(), "UINT32") == 0)
				field.type = FieldType::Uint32Type;
			else if (StrICmp(type.c_str(), "INT64") == 0)
				field.type = FieldType::Int64Type;
			else if (StrICmp(type.c_str(), "UINT64") == 0)
				field.type = FieldType::Uint64Type;
			else if (StrICmp(type.c_str(), "FLOAT") == 0)
				field.type = FieldType::FloatType;
			else if (StrICmp(type.c_str(), "DOUBLE") == 0)
				field.type = FieldType::DoubleType;
			else if (StrICmp(type.c_str(), "TEXT") == 0)
				field.type = FieldType::TextType;
			else if (StrICmp(type.c_str(), "DATETIME") == 0)
				field.type = FieldType::DateTimeType;
			else if (StrICmp(type.c_str(), "undefined") == 0)
				field.type = FieldType::Undefined;

			pLayerInfo->fieldInfos.emplace_back(field);
		}
		sqlite3_finalize(stmt);
	}
	for (map<int64_t, LayerFieldsInfo*>::iterator itr = mapLayerInfo.begin(); itr != mapLayerInfo.end(); itr++)
	{
		if (itr->second != NULL)
		{
			layerInfo.emplace_back(*itr->second);
			delete itr->second;
		}
	}
	mapLayerInfo.clear();
	return 1;
}



gisLONG i_ReadRecordValue(sqlite3* pDb, LayerFieldsInfo& layerInfo, vector<Record>& records)
{
	if (pDb == NULL)
		return -1;
	sqlite3_stmt* stmt;
	CGString sql = CGString("SELECT * FROM ", CGString::EncodeType::GB18030).Converted(CGString::EncodeType::UTF8);
	sql += CGString("\'", CGString::EncodeType::GB18030) + CGString(to_string(layerInfo.layerID).c_str(), CGString::EncodeType::UTF8) + CGString("\'", CGString::EncodeType::GB18030);
	sql += CGString(";", CGString::EncodeType::GB18030).Converted(CGString::EncodeType::UTF8);

	if (sqlite3_prepare_v2(pDb, sql.CStr(), -1, &stmt, NULL) == SQLITE_OK) {
		int num = sqlite3_column_count(stmt);
		if (num != layerInfo.fieldInfos.size() + 1)
		{
			sqlite3_finalize(stmt);
			return -1;
		}
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			Record record;
			int64_t id = sqlite3_column_int64(stmt, 0);
			record.SetID(id);
			for (int i = 0; i < num - 1; i++)
			{
				int dataType = sqlite3_column_type(stmt, i + 1);
				if (dataType == SQLITE_NULL)
					record.AppendItem(layerInfo.fieldInfos[i].type);
				else
				{
					if (dataType == SQLITE_INTEGER)
					{
						int64_t value = sqlite3_column_int64(stmt, i + 1);
						switch (layerInfo.fieldInfos[i].type)
						{
						case FieldType::BoolType:
							record.AppendItem((bool)value);
							break;
						case FieldType::Int8Type:
							record.AppendItem((char)value);
							break;
						case FieldType::Uint8Type:
							record.AppendItem((unsigned char)value);
							break;
						case FieldType::Int16Type:
							record.AppendItem((short)value);
							break;
						case FieldType::Uint16Type:
							record.AppendItem((unsigned short)value);
							break;
						case FieldType::Int32Type:
							record.AppendItem((int)value);
							break;
						case FieldType::Uint32Type:
							record.AppendItem((unsigned int)value);
							break;
						case FieldType::Int64Type:
							record.AppendItem((gisINT64)value);
							break;
						default:
							record.AppendItem(layerInfo.fieldInfos[i].type);
							break;
						}
					}
					else if (dataType == SQLITE3_TEXT)
					{
						const unsigned char* value = sqlite3_column_text(stmt, i + 1);

						switch (layerInfo.fieldInfos[i].type)
						{
						case FieldType::DateTimeType:
						{
							gisINT64 numValue = stoll((char*)value);
							record.AppendItem(numValue, true);
						}
						break;
						case FieldType::Uint64Type:
						{
							gisUINT64 numValue = stoull((char*)value);
							record.AppendItem(numValue);
						}
						break;
						case FieldType::TextType:
						{
							CGString textValue = CGString((const char*)value, CGString::EncodeType::UTF8);
							record.AppendItem(textValue);
						}
						break;
						case FieldType::Undefined:
						default:
							record.AppendItem(layerInfo.fieldInfos[i].type);
							break;
						}
					}
					else if (dataType == SQLITE_FLOAT)
					{
						int64_t value = sqlite3_column_int64(stmt, i + 1);

						switch (layerInfo.fieldInfos[i].type)
						{
						case FieldType::FloatType:
						{
							double numValue = sqlite3_column_double(stmt, i + 1);
							record.AppendItem((float)numValue);
						}
						break;
						case FieldType::DoubleType:
						{
							double numValue = sqlite3_column_double(stmt, i + 1);
							record.AppendItem((double)numValue);
						}
						break;
						default:
							record.AppendItem(layerInfo.fieldInfos[i].type);
							break;
						}
					}
					else
					{
						record.AppendItem(layerInfo.fieldInfos[i].type);
					}
				}
			}
			records.emplace_back(record);
		}
		sqlite3_finalize(stmt);
	}
	return 1;
}

gisLONG Ci_3DModelAttributeSqlite::GetAttributeByMemory(const char * data, int length, MapGIS::Tile::Attribute& outAtt) 
{

	sqlite3* pDb =NULL;

	int exit = sqlite3_open_v2(
		reinterpret_cast<const char*>(data), // 文件名参数被解释为内存映射的起始地址
		&pDb,
		SQLITE_OPEN_READONLY |     // 通常用于只读访问
		SQLITE_OPEN_MEMORY,       // 告诉 SQLite 数据在内存中
		nullptr                   // 使用默认 VFS
	);
	if (exit != SQLITE_OK) 
	{
		return -1;
	}
	vector<LayerFieldsInfo> layerInfo;
	i_ReadFilesInfo(pDb, layerInfo);
	if (layerInfo.size() > 0)
	{
		for (vector<LayerFieldsInfo>::iterator itr = layerInfo.begin(); itr != layerInfo.end(); itr++)
		{
			LayerFieldsInfo layerInfo = *itr;
			vector<Record> records;
			i_ReadRecordValue(pDb, layerInfo, records);
			if (layerInfo.fieldInfos.size() > 0 && records.size() > 0)
			{
				LayerAttribute layerAttribute;
				layerAttribute.layerInfo = layerInfo;
				layerAttribute.records.swap(records);
				outAtt.layers.emplace_back(layerAttribute);
			}
		}
	}

	if (pDb != NULL)
	{
		sqlite3_close_v2(pDb);
		pDb = NULL;
	}
	return 1;
}


gisLONG Ci_3DModelAttributeSqlite::Open()
{
    if (!IsExist())
        return -1;
    m_Attribute.layers.clear();
    gisLONG rtn = i_OpenDb();
    vector<LayerFieldsInfo> layerInfo;
    i_ReadFilesInfo(m_pDb, layerInfo);

    if (layerInfo.size() > 0)
    {
        for (vector<LayerFieldsInfo>::iterator itr = layerInfo.begin(); itr != layerInfo.end(); itr++)
        {
            LayerFieldsInfo layerInfo = *itr;
            vector<Record> records;
            i_ReadRecordValue(m_pDb, layerInfo, records);
            if (layerInfo.fieldInfos.size() > 0 && records.size() > 0)
            {
                LayerAttribute layerAttribute;
                layerAttribute.layerInfo = layerInfo;
                layerAttribute.records.swap(records);
                m_Attribute.layers.emplace_back(layerAttribute);
            }
        }
    }
    return rtn;
}


gisLONG Ci_3DModelAttributeSqlite::Close(bool isSave )
{
    if (isSave)
        Save();
    if (m_pDb != NULL)
    {
        sqlite3_close(m_pDb);
        m_pDb = NULL;
    }
    return 1;
}

gisLONG Ci_3DModelAttributeSqlite::Create()
{
    if (IsExist())
        return -1;
    return i_OpenDb();
}

gisLONG Ci_3DModelAttributeSqlite::IsExist()
{
    return CGFile::IsExists(CGString(m_sqlitePath,CGString::EncodeType::GB18030));
}

gisLONG Ci_3DModelAttributeSqlite::IsOpen()
{
    return m_pDb != NULL;
}

gisLONG Ci_3DModelAttributeSqlite::i_AddTidByLayerInfo()
{
    if (m_pDb == NULL)
        return -1;

    CGString sql = "";
    if (m_TID2Box.empty())
    {
        sql = CGString("CREATE TABLE IF NOT EXISTS TidToLayerInfo("
            "TID INTEGER PRIMARY KEY NOT NULL,"
            "LayerID INTEGER"
            ");", CGString::EncodeType::GB18030).Converted(CGString::EncodeType::UTF8);
    }
    else
    {
        sql = CGString("CREATE TABLE IF NOT EXISTS TidToLayerInfo("
            "TID INTEGER PRIMARY KEY NOT NULL,"
            "LayerID INTEGER, xmin DOUBLE, ymin DOUBLE, zmin DOUBLE, xmax DOUBLE, ymax DOUBLE, zmax DOUBLE"
            ");", CGString::EncodeType::GB18030).Converted(CGString::EncodeType::UTF8);
    }

    if (sqlite3_exec(m_pDb, sql.CStr(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        return -1;
    }
    //2、向表中插入数据
    sqlite3_exec(m_pDb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    sqlite3_stmt* stmt;
    if (m_TID2Box.empty())
    {
        sql = CGString("INSERT INTO TidToLayerInfo(TID,LayerID) VALUES (?, ?)", CGString::EncodeType::GB18030);
    }
    else
    {
        sql = CGString("INSERT INTO TidToLayerInfo(TID,LayerID,xmin,ymin,zmin,xmax,ymax,zmax) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", CGString::EncodeType::GB18030);
    }
    sql.Convert(CGString::EncodeType::UTF8);
    // 准备SQL语句
    if (sqlite3_prepare_v2(m_pDb, sql.CStr(), -1, &stmt, NULL) != SQLITE_OK) {
        return -1;
    }
    for (vector<LayerAttribute>::iterator layerItr = m_Attribute.layers.begin(); layerItr != m_Attribute.layers.end(); layerItr++)
    {
        for (vector<Record>::iterator recordItr = layerItr->records.begin(); recordItr != layerItr->records.end(); recordItr++)
        {
            int index = 1;
            sqlite3_bind_int64(stmt, index++, recordItr->GetID());
            sqlite3_bind_int64(stmt, index++, layerItr->layerInfo.layerID);
            if (!m_TID2Box.empty())
            {
                auto iterBox = m_TID2Box.find(recordItr->GetID());
                if (iterBox != m_TID2Box.end() && iterBox->second.size() == 6)
                {
                    sqlite3_bind_double(stmt, index++, iterBox->second[0]);
                    sqlite3_bind_double(stmt, index++, iterBox->second[1]);
                    sqlite3_bind_double(stmt, index++, iterBox->second[2]);
                    sqlite3_bind_double(stmt, index++, iterBox->second[3]);
                    sqlite3_bind_double(stmt, index++, iterBox->second[4]);
                    sqlite3_bind_double(stmt, index++, iterBox->second[5]);
                }
                else
                {
                    sqlite3_bind_double(stmt, index++, 0);
                    sqlite3_bind_double(stmt, index++, 0);
                    sqlite3_bind_double(stmt, index++, 0);
                    sqlite3_bind_double(stmt, index++, 0);
                    sqlite3_bind_double(stmt, index++, 0);
                    sqlite3_bind_double(stmt, index++, 0);
                }
            }
            // 执行插入操作
            sqlite3_step(stmt);
            // 重置语句以便绑定新的参数值
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
        }
    }
    // 完成后最终化语句
    sqlite3_finalize(stmt);
    // 提交事务
    sqlite3_exec(m_pDb, "COMMIT;", NULL, NULL, NULL);
    return 1;
}

gisLONG Ci_3DModelAttributeSqlite::i_AddFilesInfo()
{
    if (m_pDb == NULL)
        return -1;
    CGString sql = CGString("CREATE TABLE IF NOT EXISTS LayerFieldInfo("
        "ID INTEGER PRIMARY KEY NOT NULL,"
        "LayerID INTEGER,"
        "LayerDataSource TEXT,"
        "LayerName TEXT,"
        "FieldID INTEGER,"
        "FieldName TEXT,"
        "FieldAlias TEXT,"
        "FieldType TEXT"
        ");",CGString::EncodeType::GB18030).Converted(CGString::EncodeType::UTF8);
    if (sqlite3_exec(m_pDb, sql.CStr(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        return -1;
    }
    if (m_Attribute.layers.size() > 0)
    {
        // 开始一个事务
        sqlite3_exec(m_pDb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        sqlite3_stmt* stmt;
        sql = CGString("INSERT INTO LayerFieldInfo (LayerID, LayerDataSource, LayerName, FieldID, FieldName, FieldAlias, FieldType) VALUES (?, ?, ?, ?, ?, ?, ?);", CGString::EncodeType::GB18030).Converted(CGString::EncodeType::UTF8);

        // 准备SQL语句
        if (sqlite3_prepare_v2(m_pDb, sql.CStr(), -1, &stmt, NULL) != SQLITE_OK) {
            return -1;
        }
        for (vector<LayerAttribute>::iterator layerItr = m_Attribute.layers.begin(); layerItr != m_Attribute.layers.end(); layerItr++)
        {
            if (layerItr->layerInfo.fieldInfos.size() <= 0)
                continue;
            vector<gisINT64> layerIDs;
            vector<CGString> dataSources;
            vector<CGString> layerNames;
            vector<gisINT64> fieldIDs;
            vector<CGString> fieldNames;
            vector<CGString> fieldAliases;
            vector<CGString> fieldTypes;

            for (vector<Field>::iterator fieldItr = layerItr->layerInfo.fieldInfos.begin(); fieldItr != layerItr->layerInfo.fieldInfos.end(); fieldItr++)
            {
                layerIDs.emplace_back(layerItr->layerInfo.layerID);
                CGString value = "";
                value = layerItr->layerInfo.dataSource.Converted(CGString::EncodeType::UTF8);
                dataSources.emplace_back(value);
                value = layerItr->layerInfo.layerName.Converted(CGString::EncodeType::UTF8);
                layerNames.emplace_back(value);

                fieldIDs.emplace_back(fieldItr->fieldID);

                value = fieldItr->name.Converted(CGString::EncodeType::UTF8);
                fieldNames.emplace_back(value);
                value = fieldItr->alias.Converted(CGString::EncodeType::UTF8);
                fieldAliases.emplace_back(value);
                switch (fieldItr->type)
                {
                case FieldType::BoolType:
                    value = CGString("BOOL", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Int8Type:
                    value = CGString("INT8", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Uint8Type:
                    value = CGString("UINT8", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Int16Type:
                    value = CGString("INT16", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Uint16Type:
                    value = CGString("UINT16", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Int32Type:
                    value = CGString("INT32", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Uint32Type:
                    value = CGString("UINT32", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Int64Type:
                    value = CGString("INT64", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Uint64Type:
                    value = CGString("UINT64", CGString::EncodeType::GB18030);
                    break;
                case FieldType::FloatType:
                    value = CGString("FLOAT", CGString::EncodeType::GB18030);
                    break;
                case FieldType::DoubleType:
                    value = CGString("DOUBLE", CGString::EncodeType::GB18030);
                    break;
                case FieldType::TextType:
                    value = CGString("TEXT", CGString::EncodeType::GB18030);
                    break;
                case FieldType::DateTimeType:
                    value = CGString("DATETIME", CGString::EncodeType::GB18030);
                    break;
                case FieldType::Undefined:
                default:
                    value = CGString("UNDEFINED", CGString::EncodeType::GB18030);
                    break;
                }
                fieldTypes.emplace_back(value.Convert(CGString::EncodeType::UTF8));
            }
            for (int i = 0; i < layerIDs.size(); i++)
            {
                sqlite3_bind_int64(stmt, 1, layerIDs[i]);
                sqlite3_bind_text(stmt, 2, dataSources[i].CStr(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, layerNames[i].CStr(), -1, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 4, fieldIDs[i]);
                sqlite3_bind_text(stmt, 5, fieldNames[i].CStr(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 6, fieldAliases[i].CStr(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 7, fieldTypes[i].CStr(), -1, SQLITE_STATIC);
                // 执行插入操作
                sqlite3_step(stmt);
                // 重置语句以便绑定新的参数值
                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);
            }
        }
        // 完成后最终化语句
        sqlite3_finalize(stmt);
        // 提交事务
        sqlite3_exec(m_pDb, "COMMIT;", NULL, NULL, NULL);
    }
    return 1;
}

CGString Ci_3DModelAttributeSqlite::i_GetFieldType(FieldType type)
{
    switch (type)
    {
    case MapGIS::Tile::FieldType::BoolType:
    case MapGIS::Tile::FieldType::Int8Type:
    case MapGIS::Tile::FieldType::Uint8Type:
    case MapGIS::Tile::FieldType::Int16Type:
    case MapGIS::Tile::FieldType::Uint16Type:
    case MapGIS::Tile::FieldType::Int32Type:
    case MapGIS::Tile::FieldType::Uint32Type:
    case MapGIS::Tile::FieldType::Int64Type:
        return "INTEGER";
        break;
    case MapGIS::Tile::FieldType::FloatType:
    case MapGIS::Tile::FieldType::DoubleType:
        return "REAL";
        break;
    case MapGIS::Tile::FieldType::DateTimeType:
    case MapGIS::Tile::FieldType::Uint64Type:
    case MapGIS::Tile::FieldType::TextType:
    case MapGIS::Tile::FieldType::Undefined:
    default:
        return "TEXT";
    }
}

gisLONG Ci_3DModelAttributeSqlite::i_AddRecordValue()
{
    if (m_pDb == NULL)
        return -1;
    if (m_Attribute.layers.size() <= 0)
        return 1;

    for (vector<LayerAttribute>::iterator layerItr = m_Attribute.layers.begin(); layerItr != m_Attribute.layers.end(); layerItr++)
    {
        if (layerItr->layerInfo.fieldInfos.size() <= 0)
            continue;
        if (layerItr->records.size() <= 0)
            continue;

        //1、创建表
        CGString tableName = CGString(std::to_string(layerItr->layerInfo.layerID).c_str(), CGString::EncodeType::GB18030);
        vector<CGString> fieldNames;
        vector<CGString> fieldTypes;
        for (vector<Field>::iterator fieldItr = layerItr->layerInfo.fieldInfos.begin(); fieldItr != layerItr->layerInfo.fieldInfos.end(); fieldItr++)
        {
            fieldNames.emplace_back(fieldItr->name);
            fieldTypes.emplace_back(i_GetFieldType(fieldItr->type));
        }
        if (fieldNames.size() <= 0)
            continue;
        CGString sql = CGString("CREATE TABLE IF NOT EXISTS ", CGString::EncodeType::GB18030);
        sql += CGString("\'", CGString::EncodeType::GB18030) +  tableName + CGString("\'", CGString::EncodeType::GB18030) + CGString("(", CGString::EncodeType::GB18030);
        sql += CGString("TID INTEGER PRIMARY KEY NOT NULL,", CGString::EncodeType::GB18030);
        for (int i = 0; i < fieldNames.size(); i++)
        {
            sql += CGString("\'", CGString::EncodeType::GB18030)  + fieldNames[i] + CGString("\'", CGString::EncodeType::GB18030) + CGString(" ", CGString::EncodeType::GB18030) + fieldTypes[i];

            if (i != fieldNames.size() - 1)
                sql += CGString(",", CGString::EncodeType::GB18030);
        }
        sql += CGString(");", CGString::EncodeType::GB18030);
        sql.Convert(CGString::EncodeType::UTF8);
        if (sqlite3_exec(m_pDb, sql.CStr(), nullptr, nullptr, nullptr) != SQLITE_OK) {
            continue;
        }
        //2、向表中插入数据
        sqlite3_exec(m_pDb, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        sqlite3_stmt* stmt;

        sql = CGString("INSERT INTO ", CGString::EncodeType::GB18030);
        sql += CGString("\'", CGString::EncodeType::GB18030) + tableName + CGString("\'", CGString::EncodeType::GB18030) + CGString("(TID,", CGString::EncodeType::GB18030);
        for (int i = 0; i < fieldNames.size(); i++)
        {
            sql += CGString("\'", CGString::EncodeType::GB18030) + fieldNames[i]+ CGString("\'", CGString::EncodeType::GB18030);
            if (i != fieldNames.size() - 1)
                sql += CGString(",", CGString::EncodeType::GB18030);
        }
        sql += CGString(") VALUES (?,", CGString::EncodeType::GB18030);
        for (int i = 0; i < fieldNames.size(); i++)
        {
            sql += CGString("?", CGString::EncodeType::GB18030);
            if (i != fieldNames.size() - 1)
                sql += CGString(",", CGString::EncodeType::GB18030);
        }
        sql += CGString(")", CGString::EncodeType::GB18030);
        sql.Convert(CGString::EncodeType::UTF8);
        // 准备SQL语句
        if (sqlite3_prepare_v2(m_pDb, sql.CStr(), -1, &stmt, NULL) != SQLITE_OK) {
            continue;
        }

        for (vector<Record>::iterator recordItr = layerItr->records.begin(); recordItr != layerItr->records.end(); recordItr++)
        {
            int index = 1;
            sqlite3_bind_int64(stmt, index++, recordItr->GetID());
            int num = recordItr->GetNum();
            if (num != fieldNames.size())
                continue;
            RecordItemValue value;
            for (int i = 0; i < num; i++)
            {
                recordItr->GetItem(i, value);
                RecordItemValue noDataValue = layerItr->layerInfo.fieldInfos[i].noDataValue;

                if (!noDataValue.IsNull() && value == noDataValue)
                    sqlite3_bind_null(stmt, index++);
                else if (value.IsNull())
                    sqlite3_bind_null(stmt, index++);
                else
                {
                    if (fieldTypes[i] == "INTEGER")
                    {
                        gisINT64 numValue = value.ToInt64();
                        sqlite3_bind_int64(stmt, index++, numValue);
                    }
                    else if (fieldTypes[i] == "REAL")
                    {
                        double numValue = value.ToDouble();
                        sqlite3_bind_double(stmt, index++, numValue);
                    }
                    else if (fieldTypes[i] == "TEXT")
                    {
                        CGString textValue = value.ToString(CGString::EncodeType::UTF8);
                        sqlite3_bind_text(stmt, index++, textValue.CStr(), -1, SQLITE_TRANSIENT);
                    }
                    else
                        sqlite3_bind_null(stmt, index++);
                }
            }
            // 执行插入操作
            sqlite3_step(stmt);
            // 重置语句以便绑定新的参数值
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
        }
        // 完成后最终化语句
        sqlite3_finalize(stmt);
        // 提交事务
        sqlite3_exec(m_pDb, "COMMIT;", NULL, NULL, NULL);
    }
    return 1;
}

gisLONG Ci_3DModelAttributeSqlite::Save()
{
    // 删除所有表
    i_ClearAllTable();
    //添加属性结构信息
    i_AddFilesInfo();
    //添加tid与图层的对应关系
    i_AddTidByLayerInfo();
    //添加属性值
    i_AddRecordValue();
    return 1;
}

Attribute* Ci_3DModelAttributeSqlite::GetAttribute()
{
    return &m_Attribute;
}
string Ci_3DModelAttributeSqlite::GetSqlitePath()
{
    return m_sqlitePath;
}

void  Ci_3DModelAttributeSqlite::SetBoxInfo(const map<gisINT64, vector<double> >& tid2Box)
{
    m_TID2Box = tid2Box;
}

const map<gisINT64, vector<double>>& Ci_3DModelAttributeSqlite::GetBoxInfo() const
{
    return m_TID2Box;
}

gisLONG Ci_3DModelAttributeSqlite::i_OpenDb()
{
    if (m_sqlitePath.size() <= 0)
        return -1;
    CGString path = CGString(m_sqlitePath, CGString::EncodeType::GB18030);
    path.Convert(CGString::EncodeType::UTF8);
    // 打开数据库
    int exit = sqlite3_open(path.CStr(), &m_pDb);
    if (exit != SQLITE_OK) {
        m_pDb = NULL;
        return -1;
    }
    return 1;
}

gisLONG Ci_3DModelAttributeSqlite::i_ClearAllTable()
{
    if (m_pDb == NULL)
        return -1;
    sqlite3_stmt* stmt;
    CGString query = CGString("SELECT name FROM sqlite_master WHERE type='table';", CGString::EncodeType::GB18030);
    query.Convert(CGString::EncodeType::UTF8);
    // 准备SQL语句
    if (sqlite3_prepare_v2(m_pDb, query.CStr(), -1, &stmt, nullptr) == SQLITE_OK) {
        // 遍历所有结果
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            CGString tableName = CGString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)), CGString::EncodeType::UTF8);
            CGString dropTableQuery = CGString("DROP TABLE IF EXISTS '", CGString::EncodeType::GB18030).Convert(CGString::EncodeType::UTF8) + tableName + CGString("';", CGString::EncodeType::GB18030).Convert(CGString::EncodeType::UTF8);
            sqlite3_exec(m_pDb, dropTableQuery.CStr(), nullptr, nullptr, nullptr);
        }
        // 完成后最终化语句
        sqlite3_finalize(stmt);
    }
    return 1;
}