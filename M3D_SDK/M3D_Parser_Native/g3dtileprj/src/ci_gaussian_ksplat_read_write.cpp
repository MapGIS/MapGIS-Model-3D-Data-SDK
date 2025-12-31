#include "stdafx.h"
#include "ci_gaussian_ksplat_read_write.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "cgstring.h"
#include "g3dtilegeometry.h"
#include "ci_assist.h"


gisLONG Ci_KSplatGaussianReadWrite::i_Read(CGString filePath,  MapGIS::Tile::GaussianModel & data)
{
	std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)> callback = [&data]( MapGIS::Tile::GaussianFeature & item, bool& isStop)
	{
		data.features.push_back(item);
	};
	return i_Read(filePath, callback);
}


float DecodeFloat16(unsigned short encoded)
{
	unsigned short signBit = (encoded >> 15) & 1;
	unsigned short exponent = (encoded >> 10) & 0x1f;
	unsigned short mantissa = encoded & 0x3ff;

	if (exponent == 0) {
		if (mantissa == 0) {
			return 0.0;
		}
		// Denormalized number
		unsigned int m = mantissa;
		int exp = -14;
		for (; (m & 0x400) == 0;) {
			m <<= 1;
			exp--;
		}
		m &= 0x3ff;
		unsigned int finalExp = exp + 127;
		unsigned int finalMantissa = m << 13;
		unsigned int bits = (signBit << 31) | (finalExp << 23) | finalMantissa;
		return *(float*)&bits;
	}

	if (exponent == 0x1f) {
		if (mantissa == 0) {
			if (signBit == 1) {
				return -3.402823E+38;
			}
			return 3.402823E+38;
		}
		return std::numeric_limits<float>::quiet_NaN();
	}
	unsigned int   finalExp = exponent - 15 + 127;
	unsigned int  finalMantissa = mantissa << 13;
	unsigned int  bits = (signBit << 31) | (finalExp << 23) | finalMantissa;
	return*(float*)&bits;
}


gisLONG Ci_KSplatGaussianReadWrite::i_Read(CGString filePath, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{
	gisLONG rtn = 0;

	if (callback == NULL)
		return 0;
	double  SH_C0 = 0.28209479177387814;
#if _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
	filePath.Replace('/', '\\');
#else
	filePath.Convert(CGString::EncodeType::UTF8);
	filePath.Replace('\\', '/');
#endif // _WIN32

	std::ifstream file(filePath.CStr(), std::ios::binary);
	if (!file.is_open()) {
		return 0;
	}
	char headInfo[4096];
	file.read(headInfo,4096);
	// 获取实际读取的字节数
	std::streamsize bytesRead = file.gcount();

	if (bytesRead < 4096) {
		// 说明数据连头信息没有存储完
		file.close();
		return 0;
	}
	KsplatHeader  header;
	header.MajorVersion = ReadArrayToUnsignedChar(headInfo, 0);
	header.MinorVersion = ReadArrayToUnsignedChar(headInfo, 1);
	header.SectionCount = ReadArrayToInt32(headInfo, 4);
	header.SplatCount = ReadArrayToInt32(headInfo, 16);
	header.CompressionMode = ReadArrayToUnsignedInt16(headInfo, 20);
	header.MinHarmonicsValue = ReadArrayToFloat(headInfo, 36);
	header.MaxHarmonicsValue = ReadArrayToFloat(headInfo, 40);
	header.ShDegree = 0;
	if (header.MajorVersion != 0 || header.MinorVersion != 1)
	{
		// unsupported version
		file.close();
		return 0;
	}
	if (header.CompressionMode > 2)
	{
		// invalid compression mode
		file.close();
		return 0;
	}

	if (header.SplatCount == 0)
	{
		// data empty
		file.close();
		return 0;
	}
	if (header.MinHarmonicsValue == 0)
	{
		header.MinHarmonicsValue = -1.5;
	}

	if (header.MaxHarmonicsValue == 0)
	{
		header.MaxHarmonicsValue = 1.5;
	}

	int  centerBytes;
	int  scaleBytes;
	int  rotationBytes;
	int  colorBytes;
	int  harmonicsBytes;
	int  scaleStartByte;
	int  rotationStartByte;
	int  colorStartByte;
	int  harmonicsStartByte;
	unsigned int  scaleQuantRange;
	// 按压缩模式对应
	switch (header.CompressionMode)
	{
	case 0:
		centerBytes = 12;
		scaleBytes = 12;
		rotationBytes = 16;
		colorBytes = 4;
		harmonicsBytes = 4;
		scaleStartByte = 12;
		rotationStartByte = 24;
		colorStartByte = 40;
		harmonicsStartByte = 44;
		scaleQuantRange = 1;
		break;
	case 1:
		centerBytes = 6;
		scaleBytes = 6;
		rotationBytes = 8;
		colorBytes = 4;
		harmonicsBytes = 2;
		scaleStartByte = 6;
		rotationStartByte = 12;
		colorStartByte = 20;
		harmonicsStartByte = 24;
		scaleQuantRange = 32767;
		break;
	case 2:
		centerBytes = 6;
		scaleBytes = 6;
		rotationBytes = 8;
		colorBytes = 4;
		harmonicsBytes = 1;
		scaleStartByte = 6;
		rotationStartByte = 12;
		colorStartByte = 20;
		harmonicsStartByte = 24;
		scaleQuantRange = 32767;
		break;
	default:
		break;
	}
	
	// 分段头读取
	SectionHeader *secHeaders = new SectionHeader[header.SectionCount];
	char  SectionInfo[1024];
	for (int i = 0; i < header.SectionCount; i++) 
	{
		file.read(SectionInfo, 1024);
		bytesRead = file.gcount();
		if (bytesRead < 1024) {
			//块信息读取错误
			file.close();
			delete[] secHeaders;
			return 0;
		}

		secHeaders[i].SectionSplatCount = ReadArrayToUnsignedInt32(SectionInfo, 0);
		secHeaders[i].SectionSplatCapacity = ReadArrayToUnsignedInt32(SectionInfo, 4);
		secHeaders[i].BucketCapacity = ReadArrayToUnsignedInt32(SectionInfo, 8);
		secHeaders[i].BucketCount = ReadArrayToUnsignedInt32(SectionInfo, 12);
		secHeaders[i].BlockSize = ReadArrayToFloat(SectionInfo, 16);
		secHeaders[i].BucketSize = ReadArrayToUnsignedInt16(SectionInfo, 20);
		secHeaders[i].QuantizationRange = ReadArrayToUnsignedInt32(SectionInfo, 24);
		secHeaders[i].FullBucketCount = ReadArrayToUnsignedInt32(SectionInfo, 32);
		secHeaders[i].PartialBucketCount = ReadArrayToUnsignedInt32(SectionInfo, 36);
		secHeaders[i].ShDegree = ReadArrayToUnsignedInt16(SectionInfo, 40);

		if (secHeaders[i].QuantizationRange == 0)
			secHeaders[i].QuantizationRange = scaleQuantRange;
		if (header.ShDegree < secHeaders[i].ShDegree) {
			header.ShDegree = secHeaders[i].ShDegree;
		}
	}

	int shDims[] = { 0, 9, 24, 45 };
	int shComponents = shDims[header.ShDegree];
	bool isStop = false;

	rtn = 1;
	for (int i = 0; i < header.SectionCount; i++)
	{
		int bytesPerSplat = centerBytes + scaleBytes + rotationBytes + colorBytes + harmonicsBytes*shComponents;
		double	positionScaleFactor = secHeaders[i].BlockSize / 2.0 / secHeaders[i].QuantizationRange;

		// 部分桶元数据
		int partialBucketMetaSize = secHeaders[i].PartialBucketCount * 4; // 各未满桶中的点数量
		char * partialBucketSizes = new char[partialBucketMetaSize];
		file.read(partialBucketSizes, partialBucketMetaSize);
		bytesRead = file.gcount();
		if (bytesRead != partialBucketMetaSize)
		{

			delete[] partialBucketSizes;
			rtn = 0;
			break;
		}

		// 桶中心数据
		int bucketCentersSize = secHeaders[i].BucketCount * 3 * 4;// 每个桶相应有xyz坐标基数
		char * bucketCenters = new char[bucketCentersSize];
		file.read(bucketCenters, bucketCentersSize);
		bytesRead = file.gcount();
		if (bytesRead != bucketCentersSize)
		{
			delete[] partialBucketSizes;
			delete[] bucketCenters;
			rtn = 0;
			break;
		}

		// 点数据
		int sectionDataSize = bytesPerSplat * secHeaders[i].SectionSplatCapacity; // 按分段数据最大容量读取
		char * splatData = new char[sectionDataSize];
		file.read(splatData, sectionDataSize);
		bytesRead = file.gcount();
		if (bytesRead != sectionDataSize)
		{
			delete[] splatData;
			delete[] partialBucketSizes;
			delete[] bucketCenters;
			rtn = 0;
			break;
		}
		int fullBucketSplats = secHeaders[i].FullBucketCount * secHeaders[i].BucketCapacity;
		int	currentPartialBucket = secHeaders[i].FullBucketCount;
		int	currentPartialBase = fullBucketSplats;
		int	sectionSplatCount = secHeaders[i].SectionSplatCount;
		
		for (int j = 0; j < sectionSplatCount; j++)
		{

			int bucketIdx = 0;
			if (secHeaders[i].BucketCapacity > 0)
			{
				if (j < fullBucketSplats)
				{
					bucketIdx = floor(j* 1.0 / secHeaders[i].BucketCapacity);
				}
				else
				{
					int partialIdx = currentPartialBucket - secHeaders[i].FullBucketCount;                              // 未满桶范围的下标
					unsigned int currentBucketSize = ReadArrayToUnsignedInt32(partialBucketSizes, partialIdx * 4); // 未满桶中的数量
					if (j >= currentPartialBase + currentBucketSize)
					{
						// 当前未满桶已读取完，计算准备下一个未满桶
						currentPartialBucket++;                  // 下一个未满桶
						currentPartialBase += currentBucketSize; // 总数累加
					}
					bucketIdx = currentPartialBucket;
				}
			}

			 MapGIS::Tile::GaussianFeature  item;


			// Decode position
			int splatByteOffset = j * bytesPerSplat;

			if (header.CompressionMode == 0)
			{
				item.position[0] = ReadArrayToFloat(splatData, splatByteOffset);
				item.position[1] = ReadArrayToFloat(splatData, splatByteOffset + 4);
				item.position[2] = ReadArrayToFloat(splatData, splatByteOffset + 8);
			}
			else
			{
				item.position[0] = ((float)ReadArrayToUnsignedInt16(splatData, splatByteOffset)  - secHeaders[i].QuantizationRange)* positionScaleFactor + ReadArrayToFloat(bucketCenters, bucketIdx * 3 * 4);
				item.position[1] = ((float)ReadArrayToUnsignedInt16(splatData, splatByteOffset + 2) - secHeaders[i].QuantizationRange)* positionScaleFactor + ReadArrayToFloat(bucketCenters, bucketIdx * 3 * 4 + 4);
				item.position[2] = ((float)ReadArrayToUnsignedInt16(splatData, splatByteOffset + 4) - secHeaders[i].QuantizationRange)* positionScaleFactor + ReadArrayToFloat(bucketCenters, bucketIdx * 3 * 4 + 8);
			}

			// Decode scales
			if (header.CompressionMode == 0)
			{
				item.scale[0] = log(ReadArrayToFloat(splatData, splatByteOffset + scaleStartByte));
				item.scale[1] = log(ReadArrayToFloat(splatData, splatByteOffset + scaleStartByte + 4));
				item.scale[2] = log(ReadArrayToFloat(splatData, splatByteOffset + scaleStartByte + 8));
			}
			else
			{
				item.scale[0] = log(DecodeFloat16(ReadArrayToUnsignedInt16(splatData, splatByteOffset + scaleStartByte)));
				item.scale[1] = log(DecodeFloat16(ReadArrayToUnsignedInt16(splatData, splatByteOffset + scaleStartByte + 2)));
				item.scale[2] = log(DecodeFloat16(ReadArrayToUnsignedInt16(splatData, splatByteOffset + scaleStartByte + 4)));
			}


			// Decode rotation quaternion
			if (header.CompressionMode == 0)
			{

				item.rotation[3] = ReadArrayToFloat(splatData, splatByteOffset + rotationStartByte);
				item.rotation[0] = ReadArrayToFloat(splatData, splatByteOffset + rotationStartByte + 4);
				item.rotation[1] = ReadArrayToFloat(splatData, splatByteOffset + rotationStartByte + 8);
				item.rotation[2] = ReadArrayToFloat(splatData, splatByteOffset + rotationStartByte + 12);
			}
			else
			{
				item.rotation[3] = DecodeFloat16(ReadArrayToUnsignedInt16(splatData, splatByteOffset + rotationStartByte));
				item.rotation[0] = DecodeFloat16(ReadArrayToUnsignedInt16(splatData, splatByteOffset + rotationStartByte + 2));
				item.rotation[1] = DecodeFloat16(ReadArrayToUnsignedInt16(splatData, splatByteOffset + rotationStartByte + 4));
				item.rotation[2] = DecodeFloat16(ReadArrayToUnsignedInt16(splatData, splatByteOffset + rotationStartByte + 6));
			}

			// Decode color and opacity
			unsigned char colorValue = ReadArrayToUnsignedChar(splatData, splatByteOffset + colorStartByte);
			item.color[0] = (colorValue / 255.0 - 0.5) / SH_C0;

			colorValue = ReadArrayToUnsignedChar(splatData, splatByteOffset + colorStartByte + 1);
			item.color[1] = (colorValue / 255.0 - 0.5) / SH_C0;

			colorValue = ReadArrayToUnsignedChar(splatData, splatByteOffset + colorStartByte + 2);
			item.color[2] = (colorValue / 255.0 - 0.5) / SH_C0;

			colorValue = ReadArrayToUnsignedChar(splatData, splatByteOffset + colorStartByte + 3);
			item.alpha = -log(1.0 / (colorValue / 255.0) - 1);

			// Decode Harmonic
			if (header.ShDegree > 0)
			{
				int shIndexs[] = { 0, 3, 6, 1, 4, 7, 2, 5, 8,
					9, 14, 19, 10, 15, 20, 11, 16, 21, 12, 17, 22, 13, 18, 23,
					24, 31, 38, 25, 32, 39, 26, 33, 40, 27, 34, 41, 28, 35, 42, 29, 36, 43, 30, 37, 44 };
				int shDims[] = { 0, 3, 8, 15 };
				int shCnt = shDims[header.ShDegree] * 3;
				float shs[45];
				int cnt = 0;
				for (int k = 0; k < shCnt; k++)
				{
					switch (header.CompressionMode)
					{
					case 0:
					{
						int shOffset = splatByteOffset + harmonicsStartByte + shIndexs[k] * 4;
						shs[cnt] = ReadArrayToFloat(splatData, shOffset);
					}
					break;
					case 1:
					{
						int shOffset = splatByteOffset + harmonicsStartByte + shIndexs[k] * 2;
						shs[cnt] = DecodeFloat16(ReadArrayToUnsignedInt16(splatData, shOffset));
					}
					break;
					case 2:
					{
						int shOffset = splatByteOffset + harmonicsStartByte + shIndexs[k];
						unsigned char shValue = ReadArrayToUnsignedChar(splatData, shOffset);

						shs[cnt] = header.MinHarmonicsValue + shValue / 255.0 * (header.MaxHarmonicsValue - header.MinHarmonicsValue);
					}
					break;
					}
					cnt++;
				}


				item.sh.resize(cnt);
				for (int k = 0; k < cnt; k++)
				{
					item.sh[k] = shs[k];
				}
			}
			callback(item, isStop);
			if (isStop)
				break;
		}
		delete[] splatData;
		delete[] partialBucketSizes;
		delete[] bucketCenters;
	}
	delete[] secHeaders;
	file.close();
	return rtn;
}


gisLONG Ci_KSplatGaussianReadWrite::i_Write(CGString filePath, const  MapGIS::Tile::GaussianModel & data)
{
	//暂时不支持写

	return 0;
	
}
