#include "stdafx.h"
#include "ci_gaussian_splat_read_write.h"
#include <vector>
#include <fstream>
#include <cgfile.h>

gisLONG Ci_SplatGaussianReadWrite::i_Read(CGString filePath,  MapGIS::Tile::GaussianModel & data) 
{
	std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)> callback = [&data]( MapGIS::Tile::GaussianFeature & item, bool& isStop)
	{
		data.features.push_back(item);
	};
	return i_Read(filePath, callback);
}
gisLONG Ci_SplatGaussianReadWrite::i_Read(CGString filePath, std::function<void( MapGIS::Tile::GaussianFeature & dot, bool& isStop)>& callback)
{

	gisLONG rtn = 0;
#if _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
	filePath.Replace('/', '\\');
#else
	filePath.Convert(CGString::EncodeType::UTF8);
	filePath.Replace('\\', '/');
#endif // _WIN32
	std::ifstream file(filePath.CStr(), std::ios::binary);

	if (!file.is_open()) {
	
		return rtn;
	}

	file.seekg(0, std::ios::end);
	// 获取文件大小
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg); // 回到文件开头

								  // 预分配内存
	std::vector<uint8_t> buffer(fileSize);

	// 一次性读取
	if (file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) 
	{

		int totalSplats = buffer.size()/32;
		int off =0;
		 MapGIS::Tile::GaussianFeature  item;
		bool isStop = false;
		double  COLOR_SCALE = 0.15;
		double  SH_C0 = 0.28209479177387814;
		for (int  i = 0; i < totalSplats; i++) 
		{
			item.position[0] = *(float*)(buffer.data() + off);
			off += 4;
			item.position[1] = *(float*)(buffer.data() + off);
			off += 4;
			item.position[2] = *(float*)(buffer.data() + off);
			off += 4;

			item.scale[0] = log(*(float*)(buffer.data() + off));
			off += 4;
			item.scale[1] = log(*(float*)(buffer.data() + off));
			off += 4;
			item.scale[2] = log(*(float*)(buffer.data() + off));
			off += 4;

			unsigned char ucValue = * (unsigned char*)(buffer.data() + off);
			item.color[0] = (ucValue / 255.0 - 0.5) / SH_C0;
			off += 1;

			ucValue = *(unsigned char*)(buffer.data() + off);
			item.color[1] = (ucValue / 255.0 - 0.5) / SH_C0;
			off += 1;

			ucValue = *(unsigned char*)(buffer.data() + off);
			item.color[2] = (ucValue / 255.0 - 0.5) / SH_C0;
			off += 1;


			ucValue = *(unsigned char*)(buffer.data() + off);
			item.alpha = -log(1.0/(ucValue / 255.0) -1);
			off += 1;


			ucValue = *(unsigned char*)(buffer.data() + off);
			item.rotation[3] = (ucValue  -128)/128.0;
			off += 1;


			ucValue = *(unsigned char*)(buffer.data() + off);
			item.rotation[0] = (ucValue - 128) / 128.0;
			off += 1;


			ucValue = *(unsigned char*)(buffer.data() + off);
			item.rotation[1] = (ucValue - 128) / 128.0;
			off += 1;

			ucValue = *(unsigned char*)(buffer.data() + off);
			item.rotation[2] = (ucValue - 128) / 128.0;
			off += 1;
			callback(item, isStop);
			if (isStop)
				break;
		}
		rtn = 1;
	}

	file.close();
	return rtn;
}
gisLONG Ci_SplatGaussianReadWrite::i_Write(CGString filePath, const  MapGIS::Tile::GaussianModel & data) 
{
	if (filePath.GetLength() <= 0 || CGFile::IsExists(filePath))
		return 0;
#if _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
	filePath.Replace('/', '\\');
#else
	filePath.Convert(CGString::EncodeType::UTF8);
	filePath.Replace('\\', '/');
#endif // _WIN32

	std::ofstream outstream_binary(filePath.CStr(), std::ios::out | std::ios::binary);
	if (outstream_binary.fail())
		return 0;

	char itemValue[32];
	int off = 0;
	double  SH_C0 = 0.28209479177387814;
	for (int i = 0; i < data.features.size(); i++)
	{
		off = 0;

		memcpy(&itemValue[off],&data.features[i].position[0],4);
		off += 4;
		memcpy(&itemValue[off], &data.features[i].position[1], 4);
		off += 4;
		memcpy(&itemValue[off], &data.features[i].position[2], 4);
		off += 4;

		float floatValue = exp(data.features[i].scale[0]);

		memcpy(&itemValue[off], &floatValue, 4);
		off += 4;
		floatValue = exp(data.features[i].scale[1]);
		memcpy(&itemValue[off], &floatValue, 4);
		off += 4;
		floatValue = exp(data.features[i].scale[2]);
		memcpy(&itemValue[off], &floatValue, 4);
		off += 4;

		unsigned char ucValue = (unsigned char)((data.features[i].color[0] * SH_C0 + 0.5)*255.0);
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		ucValue = (unsigned char)((data.features[i].color[1] * SH_C0 + 0.5)*255.0);
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		ucValue = (unsigned char)((data.features[i].color[2] * SH_C0 + 0.5)*255.0);
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		ucValue = (unsigned char)( 1 / (exp(-data.features[i].alpha) + 1) *255 );
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		ucValue = (unsigned char)(data.features[i].rotation[3] * 128.0 + 128);
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		ucValue = (unsigned char)(data.features[i].rotation[0] * 128.0 + 128);
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		ucValue = (unsigned char)(data.features[i].rotation[1] * 128.0 + 128);
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		ucValue = (unsigned char)(data.features[i].rotation[2] * 128.0 + 128);
		memcpy(&itemValue[off], &ucValue, 1);
		off += 1;

		outstream_binary.write(itemValue, 32);
	}
	outstream_binary.close();
	return 1;
}
