#pragma once
#ifndef _MAPGIS_CGFILE_H_
#define _MAPGIS_CGFILE_H_

#include "cgstring.h"
#include "gbytearray.h"

class CGFile
{
public:
	/**
	* @brief 判断文件是否存在
	* @param filePath 文件路径
	*/
	static bool IsExists(const CGString &fileName);

	/**
	* @brief 移除文件
	* @param filePath 文件路径
	*/
	static bool Remove(const CGString &fileName);

	/**
	* @brief 重命名文件
	* @param oldName 原始文件名称
	* @param newName 新文件名称
	*/
	static bool Rename(const CGString &oldName, const CGString &newName);

	/**
	* @brief 拷贝文件
	* @param fileName 文件路径
	* @param newName 新文件
	* @param converFileIfExist 是否覆盖
	*/
	static bool Copy(const CGString &fileName, const CGString &newName , bool converFileIfExist);

	/**
	* @brief 从文件中读取二进制内容
	* @param filePath 文件路径
	* @return 二进制内容
	*/
	static CGByteArray ReadAllBytes(const CGString& filePath);

	/**
	* @brief 创建一个新文件，在其中写入指定的二进制内容，然后关闭该文件。 如果目标文件已存在，则覆盖该文件。
	* @param filePath 文件路径
	* @param bytes 二进制内容
	*/
	static void WriteAllBytes(const CGString& filePath, const CGByteArray& bytes);

	/**
	* @brief 创建一个新文件，在其中写入指定的二进制内容，然后关闭该文件。 如果目标文件已存在，则覆盖该文件。
	* @param filePath 文件路径
	* @param data 二进制内容
	* @param len 二进制长度
	*/
	static void WriteAllBytes(const CGString& filePath, const char *data, int len);
};

#endif//_MAPGIS_CGFILE_H_