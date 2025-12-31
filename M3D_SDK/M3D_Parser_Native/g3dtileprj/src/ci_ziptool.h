#pragma once
#include "mini_zip.h"
#include <vector>
#include "basclass70.h"
#include "gbytearray.h"

using namespace std;

//================================================================================
/// ZIP项结构体
/// 存储ZIP压缩包中单个文件的信息
//================================================================================
struct ZipItem
{
    char szName[256];           // 文件名
    unsigned char* szBuffer;    // 文件数据缓冲区
    size_t nLen;                // 文件数据长度
};

//================================================================================
/// ZIP工具类
/// 提供ZIP压缩和解压缩功能
//================================================================================
class Ci_ZipTool
{
public:
    //================================================================================
    /// 将多个缓冲区压缩为ZIP格式
    ///
    /// @param [in]  Buffers          要压缩的ZIP项列表
    /// @param [out] outInfo          输出的ZIP压缩数据
    /// @param [in]  compressionLevel 压缩级别（0-9，默认为6）
    ///
    /// @return 操作结果
    ///         - gisLONG > 0：压缩成功
    ///         - gisLONG <= 0：压缩失败
    //================================================================================
    static gisLONG Buffer2ZipBuffer(vector<ZipItem>& Buffers, CGByteArray& outInfo, int compressionLevel = 6);

    //================================================================================
    /// 将三个缓冲区压缩为ZIP格式
    ///
    /// @param [in]  geoBuf           几何数据缓冲区
    /// @param [in]  geoName          几何数据文件名
    /// @param [in]  attBuf           属性数据缓冲区
    /// @param [in]  attName          属性数据文件名
    /// @param [in]  tidBuf           TID数据缓冲区
    /// @param [in]  tidName          TID数据文件名
    /// @param [out] outInfo          输出的ZIP压缩数据
    /// @param [in]  compressionLevel 压缩级别（0-9，默认为6）
    ///
    /// @return 操作结果
    ///         - gisLONG > 0：压缩成功
    ///         - gisLONG <= 0：压缩失败
    //================================================================================
    static gisLONG Buffer2ZipBuffer(const CGByteArray& geoBuf, string geoName, const CGByteArray& attBuf, string attName, const CGByteArray& tidBuf, string tidName, CGByteArray& outInfo, int compressionLevel = 6);

    //================================================================================
    /// 从ZIP缓冲区解压指定文件
    ///
    /// @param [in]  inZipBuf         输入的ZIP压缩数据
    /// @param [in]  name             要解压的文件名
    /// @param [out] outInfo          输出的解压数据
    ///
    /// @return 操作结果
    ///         - gisLONG > 0：解压成功
    ///         - gisLONG <= 0：解压失败
    //================================================================================
    static gisLONG ZipBuffer2Buffer(const CGByteArray& inZipBuf, string name, CGByteArray& outInfo);

    //================================================================================
    /// 从ZIP缓冲区解压三个文件
    ///
    /// @param [in]  inZipBuf         输入的ZIP压缩数据
    /// @param [out] geoBuf           输出的几何数据缓冲区
    /// @param [out] geoName          输出的几何数据文件名
    /// @param [out] attBuf           输出的属性数据缓冲区
    /// @param [out] attName          输出的属性数据文件名
    /// @param [out] tidBuf           输出的TID数据缓冲区
    /// @param [out] tidName          输出的TID数据文件名
    ///
    /// @return 操作结果
    ///         - gisLONG > 0：解压成功
    ///         - gisLONG <= 0：解压失败
    //================================================================================
    static gisLONG ZipBuffer2Buffer(const CGByteArray& inZipBuf, CGByteArray& geoBuf, string& geoName, CGByteArray& attBuf, string& attName, CGByteArray& tidBuf, string& tidName);
};