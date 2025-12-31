#include "stdafx.h"
#include "ci_3dmodel_attribute_sqlite_tool.h"
#include "cgfile.h"
#ifndef _USE_MAPGIS_SDK_
#include <boost/filesystem.hpp>
#else
#include "basroot70.h"
#endif // !_USE_MAPGIS_SDK_

using namespace MapGIS::Tile;

map<G3DCacheStorage*, Ci_3DModelAttributeSqliteTool*>  Ci_3DModelAttributeSqliteTool::m_mapSqliteTool;

bool Ci_3DModelAttributeSqliteTool::ExistInstance(G3DCacheStorage* pCacheStorage)
{
	map<G3DCacheStorage*, Ci_3DModelAttributeSqliteTool*> ::iterator itr = m_mapSqliteTool.find(pCacheStorage);
	if (itr != m_mapSqliteTool.end())
        return true;
	return false;
}
Ci_3DModelAttributeSqliteTool* Ci_3DModelAttributeSqliteTool::GetInstance(G3DCacheStorage* pCacheStorage, bool noExistIsCreate)
{
	map<G3DCacheStorage*, Ci_3DModelAttributeSqliteTool*> ::iterator itr = m_mapSqliteTool.find(pCacheStorage);
	if (itr != m_mapSqliteTool.end())
		return itr->second;
	else
	{
		Ci_3DModelAttributeSqliteTool* pSqlite = new Ci_3DModelAttributeSqliteTool(pCacheStorage, noExistIsCreate);

		if (pSqlite->m_pAttributeSqlite == NULL)
		{
			delete pSqlite;
			return NULL;
		}
		m_mapSqliteTool.insert(pair<G3DCacheStorage*, Ci_3DModelAttributeSqliteTool*>(pCacheStorage, pSqlite));
		return pSqlite;
	}
}

void Ci_3DModelAttributeSqliteTool::Free3DModelAttributeSqlite(Ci_3DModelAttributeSqliteTool* pOwner)
{
	if (pOwner == NULL)
		return;

	map<G3DCacheStorage*, Ci_3DModelAttributeSqliteTool*> ::iterator itr = m_mapSqliteTool.find(pOwner->m_pCacheStorage);
	if (itr != m_mapSqliteTool.end())
		m_mapSqliteTool.erase(itr);
	delete pOwner;
}

CGString Ci_3DModelAttributeSqliteTool::GetGetRelativePath()
{
	return CGString(m_sqlistName, CGString::EncodeType::GB18030);
}
Attribute* Ci_3DModelAttributeSqliteTool::GetAttribute()
{
	if (m_pAttributeSqlite != NULL)
		return m_pAttributeSqlite->GetAttribute();
	return NULL;
}
void Ci_3DModelAttributeSqliteTool::SetBoxInfo(const map<gisINT64, vector<double> >& tid2Box)
{
	if (m_pAttributeSqlite != NULL)
		return m_pAttributeSqlite->SetBoxInfo(tid2Box);
	
}
const map<gisINT64, vector<double>> Ci_3DModelAttributeSqliteTool::GetBoxInfo() const
{
	if (m_pAttributeSqlite != NULL)
		return m_pAttributeSqlite->GetBoxInfo();
	return map<gisINT64, vector<double>>();
}

gisLONG Ci_3DModelAttributeSqliteTool::Save()
{
	if (m_pAttributeSqlite != NULL && m_pCacheStorage != NULL)
	{
		gisLONG rtn = m_pAttributeSqlite->Save();
		if (rtn > 0 && !m_isLocalStorage)
		{
			string sqlPath = m_pAttributeSqlite->GetSqlitePath();
			CGByteArray buffer = CGFile::ReadAllBytes(CGString(sqlPath, CGString::EncodeType::GB18030));
			rtn = m_pCacheStorage->SetContent(GetGetRelativePath(), buffer);
		}
		return rtn;
	}
	return 0;
}

Ci_3DModelAttributeSqliteTool::Ci_3DModelAttributeSqliteTool(G3DCacheStorage* pCacheStorage,bool noExistIsCreate)
{
	m_pCacheStorage = pCacheStorage;
	m_isLocalStorage = false;
	m_sqlistName = "DataAttribute.db";
	m_pAttributeSqlite = NULL;
	if (m_pCacheStorage != NULL)
	{
		bool isExist =  m_pCacheStorage->IsExistContent(CGString(m_sqlistName, CGString::EncodeType::GB18030));
		if (!isExist && noExistIsCreate)
		{
			Ci_G3DLocalCacheStorage* pLocalCacheStorage = dynamic_cast<Ci_G3DLocalCacheStorage*>(m_pCacheStorage);

			if (pLocalCacheStorage != NULL)
			{
				m_isLocalStorage = true;
				CGString sqlPath = MakePathByRelativePath(pLocalCacheStorage->GetRootUrl(), CGString(m_sqlistName, CGString::EncodeType::GB18030));
				sqlPath.Convert(CGString::EncodeType::GB18030);
				m_pAttributeSqlite = Ci_3DModelAttributeSqlite::GetInstance(sqlPath.CStr());
			}
			else
			{
#ifdef _USE_MAPGIS_SDK_
				GISENV* pGISENV = rt_GetEnv();
				string temp = pGISENV->temp;
				CGString tempPath = CGString(temp, CGString::EncodeType::GB18030);
#else
				boost::filesystem::path temp_dir = boost::filesystem::temp_directory_path();
				string temp = temp_dir.string();
	#ifdef _WIN32
				CGString tempPath = CGString(temp, CGString::EncodeType::GB18030);
	#else
				CGString tempPath = CGString(temp, CGString::EncodeType::UTF8);
	#endif // _WIN32
#endif

				CGString sqlPath = MakePathByRelativePath(tempPath, CGString(m_sqlistName, CGString::EncodeType::GB18030));
				int index = 0;
				while (CGFile::IsExists(sqlPath))
				{
					sqlPath = MakePathByRelativePath(tempPath, CGString(to_string(index++) + ".db", CGString::EncodeType::GB18030));
				}
				sqlPath.Convert(CGString::EncodeType::GB18030);
				m_pAttributeSqlite = Ci_3DModelAttributeSqlite::GetInstance(sqlPath.CStr());
			}
			m_pAttributeSqlite->Create();
		}
		else  if(isExist)
		{
			Ci_G3DLocalCacheStorage* pLocalCacheStorage = dynamic_cast<Ci_G3DLocalCacheStorage*>(m_pCacheStorage);
			if (pLocalCacheStorage != NULL)
			{
				m_isLocalStorage = true;
				CGString sqlPath = MakePathByRelativePath(pLocalCacheStorage->GetRootUrl(), CGString(m_sqlistName, CGString::EncodeType::GB18030));
				sqlPath.Convert(CGString::EncodeType::GB18030);
				m_pAttributeSqlite = Ci_3DModelAttributeSqlite::GetInstance(sqlPath.CStr());
			}
			else
			{
#ifdef _USE_MAPGIS_SDK_
				GISENV* pGISENV = rt_GetEnv();
				string temp = pGISENV->temp;
				CGString tempPath = CGString(temp, CGString::EncodeType::GB18030);
#else
				boost::filesystem::path temp_dir = boost::filesystem::temp_directory_path();
				string temp = temp_dir.string();
#ifdef _WIN32
				CGString tempPath = CGString(temp, CGString::EncodeType::GB18030);
#else
				CGString tempPath = CGString(temp, CGString::EncodeType::UTF8);
#endif // _WIN32
#endif

				CGString sqlPath = MakePathByRelativePath(tempPath, CGString(m_sqlistName, CGString::EncodeType::GB18030));
				int index = 0;
				while (CGFile::IsExists(sqlPath))
				{
					sqlPath = MakePathByRelativePath(tempPath, CGString(to_string(index++) + ".db", CGString::EncodeType::GB18030));
				}
				sqlPath.Convert(CGString::EncodeType::GB18030);

				CGByteArray buffer;
				m_pCacheStorage->GetContent(CGString(m_sqlistName, CGString::EncodeType::GB18030), buffer);
				CGFile::WriteAllBytes(sqlPath, buffer);
				m_pAttributeSqlite = Ci_3DModelAttributeSqlite::GetInstance(sqlPath.CStr());
			}
			m_pAttributeSqlite->Open();
		}
	}
}

Ci_3DModelAttributeSqliteTool::~Ci_3DModelAttributeSqliteTool()
{
	if (m_pAttributeSqlite != NULL)
	{
		string sqlPath = m_pAttributeSqlite->GetSqlitePath();
		Ci_3DModelAttributeSqlite::Free3DModelAttributeSqlite(m_pAttributeSqlite);
		if (!m_isLocalStorage)
		{
			CGFile::Remove(CGString(sqlPath, CGString::EncodeType::GB18030));
		}
	}
	m_pAttributeSqlite = NULL;
	m_pCacheStorage = NULL;
}

//
////-1:未定义， 0:不写入，1:写入
//int g_iAttributeIsWriteSqlite = -1;
//
//
////#ifdef MergePropertiesInThread
//
//bool GetAttributeIsWriteSqlite()
//{
//	if (g_iAttributeIsWriteSqlite == -1)
//	{
//		g_iAttributeIsWriteSqlite = 0;
//
//		bool rtn = false;
//		std::string configPath;
//		configPath = rt_GetConfigDir(CFG_DIR_TYPE_ROOT);
//		configPath += "/DataCache/DefaultDataCacheParameter.xml";
//		CXMLDocment		xmlDoc;
//		CXMLNodes       xmlNodes;
//		CXMLNode		rootNode;
//		xmlDoc.Open(CString(configPath.c_str()));
//		if (xmlDoc.IsOpen())
//		{
//			rootNode = xmlDoc.GetRoot();
//			if (!rootNode.IsNull())
//			{
//				xmlNodes = rootNode.GetChilds();
//				for (gisLONG i = 0; i < xmlNodes.GetCount(); ++i)
//				{
//					CXMLNode xmlNode = xmlNodes[i];
//					if (xmlNode.IsNull())
//						continue;
//					CString strKey;
//					CString strValue;
//					xmlNode.GetAttribute("key", strKey);
//					xmlNode.GetAttribute("value", strValue);
//					if (strKey.IsEmpty() || strValue.IsEmpty())
//						continue;
//					std::string key = strKey.GetBuffer(strKey.GetLength());
//					std::string value = strValue.GetBuffer(strValue.GetLength());
//					if (key == "AttributeWriteSqlite")
//					{
//						g_iAttributeIsWriteSqlite = strcmpi(value.c_str(), "true") == 0 ? 1 : 0;
//						break;
//					}
//				}
//			}
//		}
//	}
//
//	if (g_iAttributeIsWriteSqlite == 0)
//	{
//		return false;
//	}
//	else if (g_iAttributeIsWriteSqlite == 1)
//	{
//		return true;
//	}
//	return false;
//}