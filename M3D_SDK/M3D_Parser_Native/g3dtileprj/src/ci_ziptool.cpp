#include "stdafx.h"
#include "ci_ziptool.h"
#include <sstream>
#include <iostream>
#include <fstream>

gisLONG Ci_ZipTool::Buffer2ZipBuffer(vector<ZipItem> &Buffers, CGByteArray& outInfo, int compressionLevel)
{
	outInfo.clear();
	if (Buffers.size() <= 0)
		return -1;
	zip_t * zipArchive = zip_stream_open(NULL, 0, compressionLevel, 'w');
	if (zipArchive != NULL)
	{
		for (auto itor : Buffers)
		{
			zip_entry_open(zipArchive, itor.szName);
			zip_entry_write(zipArchive, itor.szBuffer, itor.nLen);
			zip_entry_close(zipArchive);
		}
		void* buf;
		ssize_t bufsize = 0;
		zip_stream_copy(zipArchive, &buf, &bufsize);
		outInfo.append((char*)buf, bufsize);
		free(buf);
		zip_stream_close(zipArchive);
		zipArchive = NULL;
		return 1;
	}
	return 0;
}

gisLONG Ci_ZipTool::Buffer2ZipBuffer(const CGByteArray &geoBuf, string geoName, const CGByteArray &attBuf, string attName, const CGByteArray &tidBuf, string tidName, CGByteArray& outInfo, int compressionLevel)
{
	vector<ZipItem> buffers;
	ZipItem item;
	strcpy(item.szName, geoName.c_str());
	item.szBuffer = (unsigned char*)geoBuf.data();
	item.nLen = geoBuf.size();
	buffers.push_back(item);
	if (attBuf.size() > 0)
	{
		strcpy(item.szName, attName.c_str());
		item.szBuffer = (unsigned char*)attBuf.data();
		item.nLen = attBuf.size();
		buffers.push_back(item);
	}
	if (tidBuf.size() > 0)
	{
		strcpy(item.szName, tidName.c_str());
		item.szBuffer = (unsigned char*)tidBuf.data();
		item.nLen = tidBuf.size();
		buffers.push_back(item);
	}
	return Ci_ZipTool::Buffer2ZipBuffer(buffers, outInfo, compressionLevel);
}

gisLONG Ci_ZipTool::ZipBuffer2Buffer(const CGByteArray &inZipBuf, string name, CGByteArray& outInfo)
{
	zip_t *zip = zip_stream_open(&inZipBuf.data()[0], inZipBuf.size(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
	if (NULL == zip)
		return 0;

	outInfo.clear();
	zip_entry_open(zip, name.c_str());
	{
		unsigned long long bufsize = zip_entry_size(zip);
		outInfo.resize(bufsize);
		zip_entry_noallocread(zip, &outInfo.data()[0], bufsize);
	}
	zip_entry_close(zip);
	zip_stream_close(zip);
	zip = NULL;
	return true;
}

gisLONG Ci_ZipTool::ZipBuffer2Buffer(const CGByteArray &inZipBuf, CGByteArray &geoBuf, string& geoName, CGByteArray &attBuf, string& attName, CGByteArray &tidBuf, string& tidName) 
{
	zip_t *zip = zip_stream_open(&inZipBuf.data()[0], inZipBuf.size(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
	if (NULL == zip)
		return 0;
	unsigned long long size = zip_entries_total(zip);
	for (int i = 0; i < size; i++)
	{
		if (zip_entry_openbyindex(zip, i) == 0)
		{
			const char *name = zip_entry_name(zip);
			if (strstr(name, "tid") && strstr(name, "geometry"))
			{
				tidName = name;
				unsigned long long bufsize = zip_entry_size(zip);
				tidBuf.resize(bufsize);
				zip_entry_noallocread(zip, &tidBuf.data()[0], bufsize);

			}
			else if ((strstr(name, "json") || strstr(name, "att")) && strstr(name, "attribute"))
			{
				attName = name;
				unsigned long long bufsize = zip_entry_size(zip);
				attBuf.resize(bufsize);
				zip_entry_noallocread(zip, &attBuf.data()[0], bufsize);
			}
			else if (strstr(name, "geometry"))
			{
				geoName = name;
				unsigned long long bufsize = zip_entry_size(zip);
				geoBuf.resize(bufsize);
				zip_entry_noallocread(zip, &geoBuf.data()[0], bufsize);
			}
			zip_entry_close(zip);
		}
	}
	zip_stream_close(zip);
	zip = NULL;
	return 1;
}