#include "stdafx.h"
#include "ci_cache_storage.h"
#include "ci_assist.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include "boost/filesystem.hpp"
#include "cgfile.h"

using namespace MapGIS::Tile;

CGString MakePathByRelativePath(CGString orPath, CGString relativePath)
{
	CGString outPath = "";
	if (relativePath.GetLength() <= 0)
		return 0;
#ifdef _WIN32
	orPath.Convert(CGString::EncodeType::GB18030);
	relativePath.Convert(CGString::EncodeType::GB18030);
#else
	orPath.Convert(CGString::EncodeType::UTF8);
	relativePath.Convert(CGString::EncodeType::UTF8);
#endif
	orPath.Replace('\\', '/');
	relativePath.Replace('\\', '/');
	while (relativePath.StartsWith("./"))
	{
		relativePath = relativePath.Right(relativePath.GetLength() - 2);
	}

	int index = orPath.ReverseFind('/');
	if (index <= 0)
	{
		outPath = relativePath;
		return outPath;
	}
	CGString dirUrl = orPath.Left(index);
	int forwardNum = 0;
	while (relativePath.StartsWith("../"))
	{
		forwardNum++;
		relativePath = relativePath.Right(relativePath.GetLength() - 3);
	}

	for (int i = 0; i < forwardNum; i++)
	{
		index = dirUrl.ReverseFind('/');
		if (index <= 0)
			return "";
		dirUrl = dirUrl.Left(index);
	}
	outPath = dirUrl + CGString("/") + relativePath;
	return outPath;
}

G3DCacheStorage* G3DCacheStorage::CreateInstance(CGString url)
{
	G3DCacheStorage* rtn = NULL;
	url.Convert(CGString::EncodeType::GB18030);

#ifdef _USE_MAPGIS_SDK_
	if (url.GetLength() > 8 && strncmp(url.CStr(), "mongodb:", 8) == 0)
	{
		rtn = new Ci_G3DMongoDBCacheStorage(url);
	}
	else if (url.GetLength() > 8 && strncmp(url.CStr(), "postgresql:", 11) == 0)
	{
		rtn = new Ci_G3DPostgreSQLCacheStorage(url);
	}
	else
#endif
		rtn = new Ci_G3DLocalCacheStorage(url);

	return rtn;
}

G3DCacheStorage* G3DCacheStorage::CreateInstance(CGString url, CGString rootRelative)
{
	G3DCacheStorage* rtn = NULL;
	url.Convert(CGString::EncodeType::GB18030);
#ifdef _USE_MAPGIS_SDK_
	if (url.GetLength() > 8 && strncmp(url.CStr(), "mongodb:", 8) == 0)
	{
		rtn = new Ci_G3DMongoDBCacheStorage(url, rootRelative);
	}
	else if (url.GetLength() > 8 && strncmp(url.CStr(), "postgresql:", 11) == 0)
	{
		rtn = new Ci_G3DPostgreSQLCacheStorage(url, rootRelative);
	}
	else
#endif
		rtn = new Ci_G3DLocalCacheStorage(url, rootRelative);
	return rtn;
}

Ci_G3DLocalCacheStorage::Ci_G3DLocalCacheStorage(CGString url) :G3DCacheStorage(url)
{
	m_url.Replace('\\', '/');
	if (m_url.ReverseFind('/') != m_url.GetLength() - 1)
		m_url.Append("/");
}

Ci_G3DLocalCacheStorage::Ci_G3DLocalCacheStorage(CGString url, CGString rootRelative) :G3DCacheStorage(url, rootRelative)
{
	m_url.Replace('\\', '/');
	if (m_url.ReverseFind('/') != m_url.GetLength() - 1)
		m_url.Append("/");
	m_rootRelative = rootRelative;
	m_rootRelative.Convert(CGString::EncodeType::GB18030);
	m_rootRelative.Replace('\\', '/');
	while (m_rootRelative.StartsWith("./"))
	{
		m_rootRelative = m_rootRelative.Right(m_rootRelative.GetLength() - 2);
	}
	while (m_rootRelative.StartsWith("/"))
	{
		m_rootRelative = m_rootRelative.Right(m_rootRelative.GetLength() - 1);
	}

	if (m_rootRelative.GetLength() > 0)
	{
		if (m_rootRelative.ReverseFind('/') != m_rootRelative.GetLength() - 1)
			m_rootRelative.Append("/");
	}
}

Ci_G3DLocalCacheStorage::~Ci_G3DLocalCacheStorage()
{
}

gisLONG Ci_G3DLocalCacheStorage::Open(bool isCreateIfNotExist)
{
#ifdef _WIN32
	m_url.Convert(CGString::EncodeType::GB18030);
#else
	m_url.Convert(CGString::EncodeType::UTF8);
#endif
	const boost::filesystem::path ghc_path(m_url.CStr());

	if (!boost::filesystem::exists(ghc_path))
	{
		if (isCreateIfNotExist && boost::filesystem::create_directories(ghc_path))
			return 1;
		return 0;
	}
	return 1;
}

bool Ci_G3DLocalCacheStorage::IsValid()
{
#ifdef _WIN32
	m_url.Convert(CGString::EncodeType::GB18030);
#else
	m_url.Convert(CGString::EncodeType::UTF8);
#endif
	const boost::filesystem::path ghc_path(m_url.CStr());

	if (boost::filesystem::exists(ghc_path))
		return true;
	return false;
}

gisLONG Ci_G3DLocalCacheStorage::Close()
{
	return 1;
}

gisLONG  Ci_G3DLocalCacheStorage::GetContent(CGString relative, CGByteArray & buffer)
{
	buffer = "";
	gisLONG rtn = 0;
	if (!IsValid() || relative.GetLength() <= 0)
		return rtn;

	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	CGString filePath = "";
	filePath = MakePathByRelativePath(m_url, relative);
	CGByteArray bytes = CGFile::ReadAllBytes(filePath);
	buffer = bytes;
	return 1;
}

gisLONG  Ci_G3DLocalCacheStorage::SetContent(CGString relative, const CGByteArray & buffer)
{
	gisLONG rtn = 0;
	if (!IsValid() || relative.GetLength() <= 0)
		return rtn;
	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	CGString filePath = "";
	filePath = MakePathByRelativePath(m_url, relative);

#ifdef _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
#else
	filePath.Convert(CGString::EncodeType::UTF8);
#endif
	filePath.Replace('\\', '/');
	size_t index = filePath.ReverseFind('/');
	if (index == string::npos)
		return 0;
	CGString dirPath = filePath.Left(index);
#ifdef _WIN32
	dirPath.Convert(CGString::EncodeType::GB18030);
#else
	dirPath.Convert(CGString::EncodeType::UTF8);
#endif
	const boost::filesystem::path ghc_path(dirPath.CStr());
	if (!boost::filesystem::exists(ghc_path))
		boost::filesystem::create_directories(ghc_path);
	CGFile::WriteAllBytes(filePath, buffer);
	return 1;
}

gisLONG Ci_G3DLocalCacheStorage::SetContent(CGString relative, const char *buffer, gisLONG len)
{
	gisLONG rtn = 0;
	if (!IsValid() || relative.GetLength() <= 0)
		return rtn;
	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}
	CGString filePath = "";
	filePath = MakePathByRelativePath(m_url, relative);

#ifdef _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
#else
	filePath.Convert(CGString::EncodeType::UTF8);
#endif
	filePath.Replace('\\', '/');
	size_t index = filePath.ReverseFind('/');
	if (index == string::npos)
		return 0;
	CGString dirPath = filePath.Left(index);
#ifdef _WIN32
	dirPath.Convert(CGString::EncodeType::GB18030);
#else
	dirPath.Convert(CGString::EncodeType::UTF8);
#endif
	const boost::filesystem::path ghc_path(dirPath.CStr());
	if (!boost::filesystem::exists(ghc_path))
		boost::filesystem::create_directories(ghc_path);
	CGFile::WriteAllBytes(filePath, buffer, len);
	return 1;
}

bool Ci_G3DLocalCacheStorage::IsExistContent(CGString relative)
{
	if (!IsValid() || relative.GetLength() <= 0)
		return false;

	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	CGString filePath = "";
	filePath = MakePathByRelativePath(m_url, relative);
	return CGFile::IsExists(filePath);
}

gisLONG  Ci_G3DLocalCacheStorage::DeleteContent(CGString relative)
{
	if (!IsValid() || relative.GetLength() <= 0)
		return 0;

	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	CGString filePath = "";
	filePath = MakePathByRelativePath(m_url, relative);
	if (!CGFile::IsExists(filePath))
		return 1;
	return CGFile::Remove(filePath) ? 1 : 0;
}

StorageType Ci_G3DLocalCacheStorage::GetStorageType()
{
	return StorageType::LocalFileType;
}

#ifdef _USE_MAPGIS_SDK_
Ci_G3DServerDBCacheStorage::Ci_G3DServerDBCacheStorage(CGString url):G3DCacheStorage(url)
{
	m_pModelCache = NULL;
}

Ci_G3DServerDBCacheStorage::Ci_G3DServerDBCacheStorage(CGString url, CGString rootRelative) : G3DCacheStorage(url, rootRelative)
{
	m_pModelCache = NULL;
	m_rootRelative = rootRelative;
	m_rootRelative.Convert(CGString::EncodeType::GB18030);
	m_rootRelative.Replace('\\', '/');
	while (m_rootRelative.StartsWith("./"))
	{
		m_rootRelative = m_rootRelative.Right(m_rootRelative.GetLength() - 2);
	}
	while (m_rootRelative.StartsWith("/"))
	{
		m_rootRelative = m_rootRelative.Right(m_rootRelative.GetLength() - 1);
	}

	if (m_rootRelative.GetLength() > 0)
	{
		if (m_rootRelative.ReverseFind('/') != m_rootRelative.GetLength() - 1)
			m_rootRelative.Append("/");
	}
}

Ci_G3DServerDBCacheStorage::~Ci_G3DServerDBCacheStorage()
{
	i_FreeModelCache();
}

bool Ci_G3DServerDBCacheStorage::IsValid()
{
	if (m_pModelCache != NULL)
		return true;
	return false;
}

gisLONG Ci_G3DServerDBCacheStorage::Close()
{
	i_FreeModelCache();
	return 1;
}

gisLONG Ci_G3DServerDBCacheStorage::GetContent(CGString relative, CGByteArray &buffer)
{
	buffer = "";
	gisLONG rtn = 0;
	if (m_pModelCache == NULL)
		return rtn;
	relative.Convert(CGString::EncodeType::GB18030);
	relative.Replace('\\', '/');
	while (relative.StartsWith("./"))
	{
		relative = relative.Right(relative.GetLength() - 2);
	}
	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	gisLONG cacheLen = 0;
	char* info = m_pModelCache->LoadData(relative.CStr(), &cacheLen);
	if (cacheLen > 0 && info != NULL)
	{
		buffer.append(info, cacheLen);
		m_pModelCache->FreeData(info);
		rtn = 1;
	}
	return rtn;
}

gisLONG Ci_G3DServerDBCacheStorage::SetMetaData(MapGIS::Tile::G3DCacheType cacheType)
{
	MODEL_CACHE_INFO cacheInfo;
	string type = "";
	memset(&cacheInfo, 0, sizeof(MODEL_CACHE_INFO));
	switch (cacheType)
	{
	case MapGIS::Tile::G3DCacheType::TypeM3DV20:
		strcpy(cacheInfo.cacheType, "ModelCacheM3D_V2.0");
		type = "m3d 2.0";
		break;
	case MapGIS::Tile::G3DCacheType::TypeM3DV21:
		strcpy(cacheInfo.cacheType, "ModelCacheM3D_V2.1");
		type = "m3d 2.1";
		break;
	case MapGIS::Tile::G3DCacheType::TypeM3DV22:
		strcpy(cacheInfo.cacheType, "ModelCacheM3D_V2.2");
		type = "m3d 2.2";
		break;
	case MapGIS::Tile::G3DCacheType::Type3DTilesV10:
		strcpy(cacheInfo.cacheType, "ModelCache3DTiles");
		type = "3dtiles 1.0";
		break;
	default:
		break;
	}
	m_pModelCache->SetCacheInfo(&cacheInfo);

	if (!IsExistContent("CacheInfo"))
	{
		char szVersion[128] = { 0 };
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		sprintf_s(szVersion, 128, "{\n\"version\":\"%d-%d-%d %d:%d:%d\",\n\"data_type\":\"%s\"\n}", timeinfo->tm_year + 1900,
			timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, type.c_str());
		CGByteArray version = szVersion;
		SetContent("CacheInfo", version);
	}

	return 1;
}

bool Ci_G3DServerDBCacheStorage::IsExistContent(CGString relative)
{
	bool rtn = false;
	if (m_pModelCache == NULL)
		return rtn;
	gisLONG cacheLen = 0;

	relative.Convert(CGString::EncodeType::GB18030);
	relative.Replace('\\', '/');
	while (relative.StartsWith("./"))
	{
		relative = relative.Right(relative.GetLength() - 2);
	}

	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	char* info = m_pModelCache->LoadData(relative.CStr(), &cacheLen);
	if (cacheLen > 0 && info != NULL)
	{
		m_pModelCache->FreeData(info);
		rtn = true;
	}
	return rtn;
}

gisLONG Ci_G3DServerDBCacheStorage::DeleteContent(CGString relative)
{
	gisLONG rtn = 0;
	if (m_pModelCache == NULL)
		return rtn;
	gisLONG cacheLen = 0;
	relative.Convert(CGString::EncodeType::GB18030);
	relative.Replace('\\', '/');
	while (relative.StartsWith("./"))
	{
		relative = relative.Right(relative.GetLength() - 2);
	}

	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	char* info = m_pModelCache->LoadData(relative.CStr(), &cacheLen);
	if (cacheLen > 0 && info != NULL)
	{
		m_pModelCache->FreeData(info);
		m_pModelCache->UpdateData(relative.CStr(), "", 0);
	}
	rtn = 1;
	return rtn;
}

StorageType Ci_G3DServerDBCacheStorage::GetStorageType()
{
	if (m_pModelCache != NULL)
	{
		return (StorageType)(int)m_pModelCache->GetStorageType();
	}
	return StorageType::NoneType;
}

void Ci_G3DServerDBCacheStorage::i_FreeModelCache()
{
	if (m_pModelCache != NULL)
	{
		m_pModelCache->Close();
		delete m_pModelCache;
	}
	m_pModelCache = NULL;
}

Ci_G3DMongoDBCacheStorage::Ci_G3DMongoDBCacheStorage(CGString url) :Ci_G3DServerDBCacheStorage(url)
{
}

Ci_G3DMongoDBCacheStorage::Ci_G3DMongoDBCacheStorage(CGString url, CGString rootRelative) : Ci_G3DServerDBCacheStorage(url, rootRelative)
{
}

Ci_G3DMongoDBCacheStorage::~Ci_G3DMongoDBCacheStorage()
{
}

gisLONG Ci_G3DMongoDBCacheStorage::Open(bool isCreateIfNotExist)
{
	if (m_url.GetLength() <= 0)
		return 0;
	m_url.Convert(CGString::EncodeType::GB18030);
	i_FreeModelCache();
	m_pModelCache = new CModelCache();
	gisLONG rtn = m_pModelCache->Open(m_url.CStr());
	if (rtn == ERR_MONGO_DATA_TBL_NOT_EXIST && isCreateIfNotExist)
	{
		m_pModelCache->Close();
		MODEL_CACHE_INFO cacheInfo;
		memset(&cacheInfo, 0, sizeof(MODEL_CACHE_INFO));

		rtn = m_pModelCache->Create(m_url.CStr(), &cacheInfo, CACHE_STORAGE_TYPE::MONGO_CACHE);
		if (rtn > 0)
			rtn = m_pModelCache->Open(m_url.CStr());
	}
	if (rtn <= 0)
	{
		i_FreeModelCache();
	}
	return rtn;
}

gisLONG  Ci_G3DMongoDBCacheStorage::SetContent(CGString relative, const CGByteArray & buffer)
{
	return SetContent(relative, buffer.constData(), buffer.length());
}

gisLONG  Ci_G3DMongoDBCacheStorage::SetContent(CGString relative, const char *buffer, gisLONG len)
{
	gisLONG rtn = 0;
	if (m_pModelCache == NULL)
		return rtn;
	relative.Convert(CGString::EncodeType::GB18030);
	relative.Replace('\\', '/');
	while (relative.StartsWith("./"))
	{
		relative = relative.Right(relative.GetLength() - 2);
	}

	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}
	//使用SaveData写key值相同的数据，会写多个数据，这里使用UpdateData来写数据
	//因为大于16MB的数据UpdateDate暂时不支持写，所以还是调用SaveData
	//这里有个隐患，如果写多次写相同KEY值的数据，存在大于16MB,则数据库中就存在多个数据。
	if (len > 16777216)//大于16MB的数据
		rtn = m_pModelCache->SaveData(relative.CStr(), buffer, len);
	else
		rtn = m_pModelCache->UpdateData(relative.CStr(), buffer, len);

	return rtn;
}

Ci_G3DPostgreSQLCacheStorage::Ci_G3DPostgreSQLCacheStorage(CGString url) :Ci_G3DServerDBCacheStorage(url)
{
}

Ci_G3DPostgreSQLCacheStorage::Ci_G3DPostgreSQLCacheStorage(CGString url, CGString rootRelative) : Ci_G3DServerDBCacheStorage(url, rootRelative)
{
}

Ci_G3DPostgreSQLCacheStorage::~Ci_G3DPostgreSQLCacheStorage()
{
}

gisLONG Ci_G3DPostgreSQLCacheStorage::Open(bool isCreateIfNotExist)
{
	if (m_url.GetLength() <= 0)
		return 0;
	m_url.Convert(CGString::EncodeType::GB18030);
	i_FreeModelCache();
	m_pModelCache = new CModelCache();
	gisLONG rtn = m_pModelCache->Open(m_url.CStr());

	//pg没有类似mongodb错误码(不存在集合)
	if (rtn != 1 && isCreateIfNotExist)
	{
		m_pModelCache->Close();
		MODEL_CACHE_INFO cacheInfo;
		memset(&cacheInfo, 0, sizeof(MODEL_CACHE_INFO));

		rtn = m_pModelCache->Create(m_url.CStr(), &cacheInfo, CACHE_STORAGE_TYPE::POSTGRESQL_CACHE);
		if (rtn > 0)
			rtn = m_pModelCache->Open(m_url.CStr());
	}
	if (rtn <= 0)
	{
		i_FreeModelCache();
	}
	return rtn;
}

gisLONG Ci_G3DPostgreSQLCacheStorage::SetContent(CGString relative, const CGByteArray &buffer)
{
	return SetContent(relative, buffer.constData(), buffer.length());
}

gisLONG Ci_G3DPostgreSQLCacheStorage::SetContent(CGString relative, const char *buffer, gisLONG len)
{
	gisLONG rtn = 0;
	if (m_pModelCache == NULL)
		return rtn;
	relative.Convert(CGString::EncodeType::GB18030);
	relative.Replace('\\', '/');
	while (relative.StartsWith("./"))
	{
		relative = relative.Right(relative.GetLength() - 2);
	}

	if (!m_rootRelative.IsEmpty())
	{
		relative = MakePathByRelativePath(m_rootRelative, relative);
	}

	//若pg集合中没有key,调用update(key,value)插入此数据
	rtn = m_pModelCache->SaveData(relative.CStr(), buffer, len);
	return rtn;
}
#endif