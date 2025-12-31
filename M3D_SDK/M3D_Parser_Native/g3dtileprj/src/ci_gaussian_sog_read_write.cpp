#include "stdafx.h"
#include "ci_gaussian_sog_read_write.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "basroot70.h"
#include "cgdir.h"
#include "cgfile.h"
#include "ci_ziptool.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/rapidjson.h"
#include "gimage.h"

gisLONG Ci_SogGaussianReadWrite::i_Read(CGString filePath,  MapGIS::Tile::GaussianModel & data)
{
	std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)> callback = [&data]( MapGIS::Tile::GaussianFeature & item, bool& isStop)
	{
		data.features.push_back(item);
	};
	return i_Read(filePath, callback);
}

void GetSogItem(rapidjson::Value& jsonValue , SogItem& item)
{
	if (jsonValue.HasMember("shape") && jsonValue["shape"].IsArray())
	{
		for (int i = 0; i < jsonValue["shape"].Size(); i++)
		{
			if (!jsonValue["shape"][i].IsNumber())
				continue;
			item.Shape.emplace_back((int)jsonValue["shape"][i].GetDouble());
		}
	}
	else if (jsonValue.HasMember("shape") && jsonValue["shape"].IsNumber())
	{
		item.Shape.emplace_back((int)jsonValue["shape"].GetDouble());
	}

	if (jsonValue.HasMember("dtype") && jsonValue["dtype"].IsString())
	{
		CGString dtype = CGString(jsonValue["dtype"].GetString(), CGString::EncodeType::UTF8);
		item.Dtype = dtype.Convert(CGString::EncodeType::GB18030).StdString();
	}

	if (jsonValue.HasMember("encoding") && jsonValue["encoding"].IsString())
	{
		CGString encoding = CGString(jsonValue["encoding"].GetString(), CGString::EncodeType::UTF8);
		item.Encoding = encoding.Convert(CGString::EncodeType::GB18030).StdString();
	}

	if (jsonValue.HasMember("count") && jsonValue["count"].IsNumber())
	{
		item.count = (int)jsonValue["count"].GetDouble();
	}

	if (jsonValue.HasMember("bands") && jsonValue["bands"].IsNumber())
	{
		item.bands = (int)jsonValue["bands"].GetDouble();
	}

	if (jsonValue.HasMember("mins") && jsonValue["mins"].IsArray())
	{
		for (int i = 0; i < jsonValue["mins"].Size(); i++)
		{
			if (!jsonValue["mins"][i].IsNumber())
				continue;
			item.Mins.emplace_back((float)jsonValue["mins"][i].GetDouble());
		}
	}
	else if (jsonValue.HasMember("mins") && jsonValue["mins"].IsNumber())
	{
		item.Mins.emplace_back((int)jsonValue["mins"].GetDouble());
	}

	if (jsonValue.HasMember("maxs") && jsonValue["maxs"].IsArray())
	{
		for (int i = 0; i < jsonValue["maxs"].Size(); i++)
		{
			if (!jsonValue["maxs"][i].IsNumber())
				continue;
			item.Maxs.emplace_back((float)jsonValue["maxs"][i].GetDouble());
		}
	}
	else if (jsonValue.HasMember("maxs") && jsonValue["maxs"].IsNumber())
	{
		item.Maxs.emplace_back((int)jsonValue["maxs"].GetDouble());
	}

	if (jsonValue.HasMember("codebook") && jsonValue["codebook"].IsArray())
	{
		for (int i = 0; i < jsonValue["codebook"].Size(); i++)
		{
			if (!jsonValue["codebook"][i].IsNumber())
				continue;
			item.Codebook.emplace_back((float)jsonValue["codebook"][i].GetDouble());
		}
	}
	else if (jsonValue.HasMember("codebook") && jsonValue["codebook"].IsNumber())
	{
		item.Codebook.emplace_back((float)jsonValue["codebook"].GetDouble());
	}

	if (jsonValue.HasMember("quantization") && jsonValue["quantization"].IsNumber())
	{
		item.Quantization = (int)jsonValue["quantization"].GetDouble();
	}

	if (jsonValue.HasMember("files") && jsonValue["files"].IsArray())
	{
		for (int i = 0; i < jsonValue["files"].Size(); i++)
		{
			if (!jsonValue["files"][i].IsString())
				continue;
			CGString file = CGString(jsonValue["files"][i].GetString(), CGString::EncodeType::UTF8);
			item.Files.emplace_back(file.Convert(CGString::EncodeType::GB18030).StdString());
		}
	}
	else if (jsonValue.HasMember("files") && jsonValue["files"].IsString()) 
	{
		CGString file = CGString(jsonValue["files"].GetString(), CGString::EncodeType::UTF8);
		item.Files.emplace_back(file.Convert(CGString::EncodeType::GB18030).StdString());
	}
}

SogMeta GetSogMetaByCGByteArray(CGByteArray& byteArray) 
{
	SogMeta rtn;
	rtn.Version = 0;
	rtn.Count = 0;
	rapidjson::Document doc;
	if (doc.Parse(byteArray.data(), byteArray.size()).HasParseError())
		return rtn;
	if (!doc.IsObject())
		return rtn;

	if (doc.HasMember("version") && doc["version"].IsNumber()) 
	{
		rtn.Version = (int)doc["version"].GetDouble();
	}

	if (doc.HasMember("count") && doc["count"].IsNumber())
	{
		rtn.Count = (int)doc["count"].GetDouble();
	}

	if (doc.HasMember("antialias") && doc["antialias"].IsBool())
	{
		rtn.Antialias = (int)doc["count"].GetBool();
	}

	if (doc.HasMember("means") && doc["means"].IsObject())
	{
		rapidjson::Value& means =   doc["means"];
		GetSogItem(means, rtn.Means);
	}

	if (doc.HasMember("scales") && doc["scales"].IsObject())
	{
		rapidjson::Value& scales = doc["scales"];
		GetSogItem(scales, rtn.Scales);
	}

	if (doc.HasMember("quats") && doc["quats"].IsObject())
	{
		rapidjson::Value& quats = doc["quats"];
		GetSogItem(quats, rtn.Quats);
	}

	if (doc.HasMember("sh0") && doc["sh0"].IsObject())
	{
		rapidjson::Value& sh0 = doc["sh0"];
		GetSogItem(sh0, rtn.Sh0);
	}

	if (doc.HasMember("shN") && doc["shN"].IsObject())
	{
		rapidjson::Value& shN = doc["shN"];
		GetSogItem(shN, rtn.ShN);
	}
	return rtn;
}


void DecodeRGBA(CGByteArray& inByte, CGByteArray&outByte, int& outWidit, int& outHeight)
{
	CGImage image;
	image.loadFromData(inByte,"webp");

	unsigned char* sss = image.bits();
	gisUINT64 ssss = 	image.sizeInBytes();

	image = image.convertToFormat(CGImage::Format::Format_RGBA8888);

	unsigned char* sssdd = image.bits();
	gisUINT64 ssssdd = image.sizeInBytes();


	outByte.clear();

	outByte.append((char*)image.bits(), image.sizeInBytes());
	outWidit = image.width();
	outHeight = image.height();
}

void DecodeRGBA(zip_t *zip, string path, CGByteArray&outByte, int& outWidit, int& outHeight)
{
	CGByteArray inByte;
	zip_entry_open(zip, path.c_str());
	{
		unsigned long long bufsize = zip_entry_size(zip);
		inByte.resize(bufsize);
		zip_entry_noallocread(zip, &inByte.data()[0], bufsize);
	}
	zip_entry_close(zip);
	DecodeRGBA(inByte, outByte, outWidit, outHeight);
}

void DecodeRGBA(CGString filePath, string path, CGByteArray&outByte, int& outWidit, int& outHeight)
{
	CGByteArray inByte = CGFile::ReadAllBytes(filePath);
	DecodeRGBA(inByte, outByte, outWidit, outHeight);
}



// 定义整个 JSON 文件的结构
struct SogDataArray {
	CGByteArray  meansFile0;
	CGByteArray  meansFile1;
	CGByteArray  scalesFile0;
	CGByteArray  quatsFile0;
	CGByteArray  sh0File0;
	CGByteArray  shNFile0;
	CGByteArray  shNFile1;

	int  meansFile0Widit;
	int  meansFile1Widit;
	int  scalesFile0Widit;
	int  quatsFile0Widit;
	int  sh0File0Widit;
	int  shNFile0Widit;
	int  shNFile1Widit;

	int  meansFile0Height;
	int  meansFile1Height;
	int  scalesFile0Height;
	int  quatsFile0Height;
	int  sh0File0Height;
	int  shNFile0Height;
	int  shNFile1Height;
	SogDataArray()
	{
		meansFile0Widit = 0;
		meansFile1Widit = 0;
		scalesFile0Widit = 0;
		quatsFile0Widit = 0;
		sh0File0Widit = 0;
		shNFile0Widit = 0;
		shNFile1Widit = 0;

		meansFile0Height = 0;
		meansFile1Height = 0;
		scalesFile0Height = 0;
		quatsFile0Height = 0;
		sh0File0Height = 0;
		shNFile0Height = 0;
		shNFile1Height = 0;
	}
};




gisLONG ReadSogV1(SogDataArray &data, SogMeta& meta, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{
	if (meta.Means.Shape.size() <= 0)
		return 0;
	int count = meta.Means.Shape[0];
	if (count <= 0)
		return 0;

	//datas  = make([] * SplatData, count)
	int shDegree = 0;
	if (meta.ShN.Files.size() > 0 && meta.ShN.Shape.size() > 1)
	{
		switch (meta.ShN.Shape[1])
		{
		case 45:
		case 15:
			shDegree = 3;
			break;
		case 24:
		case 8:
			shDegree = 2;
			break;
		case 9:
		case 3:
			shDegree = 1;
			break;
		default:
			break;
		}
	}
	bool isStop = false;

	float SQRT2 = 1.4142135623730951; 

	for (int i = 0; i < count; i++)
	{
		 MapGIS::Tile::GaussianFeature  item;

		float fx = (((uint16_t)(unsigned char)data.meansFile1.data()[i * 4 + 0]) << 8 | ((uint16_t)(unsigned char)data.meansFile0.data()[i * 4 + 0])) / 65535.0;
		float fy = (((uint16_t)(unsigned char)data.meansFile1.data()[i * 4 + 1]) << 8 | ((uint16_t)(unsigned char)data.meansFile0.data()[i * 4 + 1])) / 65535.0;
		float fz = (((uint16_t)(unsigned char)data.meansFile1.data()[i * 4 + 2]) << 8 | ((uint16_t)(unsigned char)data.meansFile0.data()[i * 4 + 2])) / 65535.0;

		float xScale = fabs(meta.Means.Maxs[0] - meta.Means.Mins[0]) > 1e-7 ? (meta.Means.Maxs[0] - meta.Means.Mins[0]) : 1;
		float yScale = fabs(meta.Means.Maxs[1] - meta.Means.Mins[1]) > 1e-7 ? (meta.Means.Maxs[1] - meta.Means.Mins[1]) : 1;
		float zScale = fabs(meta.Means.Maxs[2] - meta.Means.Mins[2]) > 1e-7 ? (meta.Means.Maxs[2] - meta.Means.Mins[2]) : 1;

		float x = meta.Means.Mins[0] + xScale*fx;
		float y = meta.Means.Mins[1] + yScale*fy;
		float z = meta.Means.Mins[2] + zScale*fz;
		if (x < 0)
			x = -(exp(fabs(x)) - 1.0);
		else
			x = exp(fabs(x)) - 1.0;
		if (y < 0)
			y = -(exp(fabs(y)) - 1.0);
		else
			y = exp(fabs(y)) - 1.0;
		if (z < 0)
			z = -(exp(fabs(z)) - 1.0);
		else
			z = exp(fabs(z)) - 1.0;
		item.position[0] = x;
		item.position[1] = y;
		item.position[2] = z;

		float sx = ((unsigned char)data.scalesFile0.data()[i * 4 + 0]) / 255.0;
		float sy = ((unsigned char)data.scalesFile0.data()[i * 4 + 1]) / 255.0;
		float sz = ((unsigned char)data.scalesFile0.data()[i * 4 + 2]) / 255.0;

		float scaleXScale = fabs(meta.Scales.Maxs[0] - meta.Scales.Mins[0]) > 1e-7 ? (meta.Scales.Maxs[0] - meta.Scales.Mins[0]) : 1;
		float scaleYScale = fabs(meta.Scales.Maxs[1] - meta.Scales.Mins[1]) > 1e-7 ? (meta.Scales.Maxs[1] - meta.Scales.Mins[1]) : 1;
		float scaleZScale = fabs(meta.Scales.Maxs[2] - meta.Scales.Mins[2]) > 1e-7 ? (meta.Scales.Maxs[2] - meta.Scales.Mins[2]) : 1;

		item.scale[0] = meta.Scales.Mins[0] + scaleXScale*sx;
		item.scale[1] = meta.Scales.Mins[1] + scaleYScale*sy;
		item.scale[2] = meta.Scales.Mins[2] + scaleZScale*sz;


		float r0 = (((unsigned char)data.quatsFile0.data()[i * 4 + 0]) / 255.0 - 0.5) * SQRT2;
		float r1 = (((unsigned char)data.quatsFile0.data()[i * 4 + 1]) / 255.0 - 0.5) * SQRT2;
		float r2 = (((unsigned char)data.quatsFile0.data()[i * 4 + 2]) / 255.0 - 0.5) * SQRT2;
	
		float ri = 0;
		if(1.0 - r0*r0 - r1*r1 - r2*r2>0)
			ri = sqrt(1.0 - r0*r0 - r1*r1 - r2*r2);
		unsigned char idx = ((unsigned char)data.quatsFile0.data()[i * 4 + 3]) - 252;
		switch (idx)
		{
		case 0:
			item.rotation[3] = ri;
			item.rotation[0] = r0;
			item.rotation[1] = r1;
			item.rotation[2] = r2;
			break;
		case 1:

			item.rotation[3] = r0;
			item.rotation[0] = ri;
			item.rotation[1] = r1;
			item.rotation[2] = r2;
			break;
		case 2:
			item.rotation[3] = r0;
			item.rotation[0] = r1;
			item.rotation[1] = ri;
			item.rotation[2] = r2;
			break;
		case 3:

			item.rotation[3] = r0;
			item.rotation[0] = r1;
			item.rotation[1] = r2;
			item.rotation[2] = ri;
			break;
		}

		item.color[0] = meta.Sh0.Mins[0] + (meta.Sh0.Maxs[0] - meta.Sh0.Mins[0])*(((unsigned char)data.sh0File0.data()[i * 4 + 0]) / 255.0);
		item.color[1] = meta.Sh0.Mins[1] + (meta.Sh0.Maxs[1] - meta.Sh0.Mins[1])*(((unsigned char)data.sh0File0.data()[i * 4 + 1]) / 255.0);
		item.color[2] = meta.Sh0.Mins[2] + (meta.Sh0.Maxs[2] - meta.Sh0.Mins[2])*(((unsigned char)data.sh0File0.data()[i * 4 + 2]) / 255.0);
		item.alpha = meta.Sh0.Mins[3] + (meta.Sh0.Maxs[3] - meta.Sh0.Mins[3])*(((unsigned char)data.sh0File0.data()[i * 4 + 3]) / 255.0);


		if (shDegree > 0)
		{
			int label = ((unsigned char)data.shNFile1.data()[i * 4 + 0]) + (((int)(unsigned char)data.shNFile1.data()[i * 4 + 1]) << 8);

			int col = (label & 63) * 15; // 同 (n % 64) * 15
			int row = label >> 6;       // 同 Math.floor(n / 64)
			int offset = row*data.shNFile0Widit + col;

			float sh1[9];
			float sh2[15];
			float sh3[21];
			for (int d = 0; d < 3; d++)
			{
				if (shDegree >= 1)
				{
					for (int k = 0; k < 3; k++)
					{
						sh1[k * 3 + d] = (meta.ShN.Maxs[0] - meta.ShN.Mins[0]) * ((unsigned char)data.shNFile0.data()[(offset + k) * 4 + d]) / 255.0 + meta.ShN.Mins[0];
					}
				}
				if (shDegree >= 2)
				{
					for (int k = 0; k < 5; k++)
					{
						sh2[k * 3 + d] = (meta.ShN.Maxs[0] - meta.ShN.Mins[0]) *  ((unsigned char)data.shNFile0.data()[(offset + 3 + k) * 4 + d]) / 255.0 + meta.ShN.Mins[0];
					}
				}
				if (shDegree == 3)
				{
					for (int k = 0; k < 7; k++)
					{
						sh3[k * 3 + d] = (meta.ShN.Maxs[0] - meta.ShN.Mins[0]) * ((unsigned char)data.shNFile0.data()[(offset + 8 + k) * 4 + d]) / 255.0 + meta.ShN.Mins[0];
					}
				}
			}

			if (shDegree >= 1)
			{
				for (int k = 0; k < 9; k++)
				{
					item.sh.emplace_back(sh1[k]);
				}
			}

			if (shDegree >= 2)
			{
				for (int k = 0; k < 15; k++)
				{
					item.sh.emplace_back(sh2[k]);
				}
			}
			if (shDegree == 3)
			{
				for (int k = 0; k < 21; k++)
				{
					item.sh.emplace_back(sh3[k]);
				}
			}
		}
		callback(item, isStop);
		if (isStop)
			return 1;
	}
	return 1;
}


gisLONG ReadSogV2(SogDataArray &data, SogMeta& meta, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{

	int count = meta.Count;
	//datas  = make([] * SplatData, count)
	int shDegree = 0;
	if (meta.ShN.Files.size() > 0 ||  meta.ShN.bands >0)
	{
		if(meta.ShN.bands >0)
			shDegree = meta.ShN.bands;
		else 
		{
			shDegree = 3;
		}
	}

	bool isStop = false;

	float SQRT2 = 1.4142135623730951; // math.Sqrt(2.0)

	for (int i = 0; i < count; i++)
	{
		 MapGIS::Tile::GaussianFeature  item;

		float fx = (((uint16_t)(unsigned char)data.meansFile1.data()[i * 4 + 0]) << 8 | ((uint16_t)(unsigned char)data.meansFile0.data()[i * 4 + 0])) / 65535.0;
		float fy = (((uint16_t)(unsigned char)data.meansFile1.data()[i * 4 + 1]) << 8 | ((uint16_t)(unsigned char)data.meansFile0.data()[i * 4 + 1])) / 65535.0;
		float fz = (((uint16_t)(unsigned char)data.meansFile1.data()[i * 4 + 2]) << 8 | ((uint16_t)(unsigned char)data.meansFile0.data()[i * 4 + 2])) / 65535.0;
	
		float xScale = fabs(meta.Means.Maxs[0] - meta.Means.Mins[0]) > 1e-7 ? (meta.Means.Maxs[0] - meta.Means.Mins[0]) : 1;
		float yScale = fabs(meta.Means.Maxs[1] - meta.Means.Mins[1]) > 1e-7 ? (meta.Means.Maxs[1] - meta.Means.Mins[1]) : 1;
		float zScale = fabs(meta.Means.Maxs[2] - meta.Means.Mins[2]) > 1e-7 ? (meta.Means.Maxs[2] - meta.Means.Mins[2]) : 1;

		float x = meta.Means.Mins[0] + xScale*fx;
		float y = meta.Means.Mins[1] + yScale*fy;
		float z = meta.Means.Mins[2] + zScale*fz;
		if (x < 0)
			x = -(exp(fabs(x)) - 1.0);
		else
			x = exp(fabs(x)) - 1.0;
		if (y < 0)
			y = -(exp(fabs(y)) - 1.0);
		else
			y = exp(fabs(y)) - 1.0;
		if (z < 0)
			z = -(exp(fabs(z)) - 1.0);
		else
			z = exp(fabs(z)) - 1.0;
		item.position[0] = x;
		item.position[1] = y;
		item.position[2] = z;


		item.scale[0] = meta.Scales.Codebook[((unsigned char)data.scalesFile0.data()[i * 4 + 0])];
		item.scale[1] = meta.Scales.Codebook[((unsigned char)data.scalesFile0.data()[i * 4 + 1])];
		item.scale[2] = meta.Scales.Codebook[((unsigned char)data.scalesFile0.data()[i * 4 + 2])];


		float r0 = (((unsigned char)data.quatsFile0.data()[i * 4 + 0]) / 255.0 - 0.5) * SQRT2;
		float r1 = (((unsigned char)data.quatsFile0.data()[i * 4 + 1]) / 255.0 - 0.5) * SQRT2;
		float r2 = (((unsigned char)data.quatsFile0.data()[i * 4 + 2]) / 255.0 - 0.5) * SQRT2;
		float ri = 0;
		if ((1.0 - r0*r0 - r1*r1 - r2*r2)>0)
			ri = sqrt(1.0 - r0*r0 - r1*r1 - r2*r2);
		unsigned char idx = ((unsigned char)data.quatsFile0.data()[i * 4 + 3]) - 252;
		switch (idx)
		{
		case 0:
			item.rotation[3] = ri;
			item.rotation[0] = r0;
			item.rotation[1] = r1;
			item.rotation[2] = r2;
			break;
		case 1:

			item.rotation[3] = r0;
			item.rotation[0] = ri;
			item.rotation[1] = r1;
			item.rotation[2] = r2;
			break;
		case 2:
			item.rotation[3] = r0;
			item.rotation[0] = r1;
			item.rotation[1] = ri;
			item.rotation[2] = r2;
			break;
		case 3:

			item.rotation[3] = r0;
			item.rotation[0] = r1;
			item.rotation[1] = r2;
			item.rotation[2] = ri;
			break;
		}

		item.color[0] = meta.Sh0.Codebook[(unsigned char)data.sh0File0.data()[i * 4 + 0]];
		item.color[1] = meta.Sh0.Codebook[(unsigned char)data.sh0File0.data()[i * 4 + 1]];
		item.color[2] = meta.Sh0.Codebook[(unsigned char)data.sh0File0.data()[i*4+2]];
		unsigned char alpha = (unsigned char)data.sh0File0.data()[i * 4 + 3];
		item.alpha = -log(1.0 / (alpha / 255.0) - 1);




		if (shDegree > 0)
		{
			int label = ((unsigned char)data.shNFile1.data()[i * 4 + 0]) + (((int)(unsigned char)data.shNFile1.data()[i * 4 + 1]) << 8);

			int col = (label & 63) * 15; // 同 (n % 64) * 15
			int row = label >> 6;       // 同 Math.floor(n / 64)
			int offset = row*data.shNFile0Widit + col;

			float sh1[9];
			float sh2[15];
			float sh3[21];
			for (int d = 0; d < 3; d++)
			{
				if (shDegree >= 1)
				{
					for (int k = 0; k < 3; k++)
					{
						sh1[k * 3 + d] = meta.ShN.Codebook[(unsigned char)data.shNFile0.data()[(offset + k) * 4 + d]];
					}
				}
				if (shDegree >= 2)
				{
					for (int k = 0; k < 5; k++)
					{
						sh2[k * 3 + d] = meta.ShN.Codebook[(unsigned char)data.shNFile0.data()[(offset + 3 + k) * 4 + d]];
					}
				}
				if (shDegree == 3)
				{
					for (int k = 0; k < 7; k++)
					{
						sh3[k * 3 + d] = meta.ShN.Codebook[(unsigned char)data.shNFile0.data()[(offset + 8 + k) * 4 + d]];
					}
				}
			}

			if (shDegree >= 1)
			{
				for (int k = 0; k < 9; k++)
				{
					item.sh.emplace_back(sh1[k]);
				}
			}

			if (shDegree >= 2)
			{
				for (int k = 0; k < 15; k++)
				{
					item.sh.emplace_back(sh2[k]);
				}
			}
			if (shDegree == 3)
			{
				for (int k = 0; k < 21; k++)
				{
					item.sh.emplace_back(sh3[k]);
				}
			}
		}
		callback(item, isStop);
		if (isStop)
			return 1;
	}
	return 1;

}


gisLONG ReadSogV1(zip_t *zip, SogMeta& meta, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{
	SogDataArray data;
	if (meta.Means.Files.size() >= 2)
	{
		DecodeRGBA(zip, meta.Means.Files[0], data.meansFile0, data.meansFile0Widit, data.meansFile0Height);
		DecodeRGBA(zip, meta.Means.Files[1], data.meansFile1, data.meansFile1Widit, data.meansFile1Height);
	}
	if (meta.Scales.Files.size() >= 1)
		DecodeRGBA(zip, meta.Scales.Files[0], data.scalesFile0, data.scalesFile0Widit, data.scalesFile0Height);
	if (meta.Quats.Files.size() >= 1)
		DecodeRGBA(zip, meta.Quats.Files[0], data.quatsFile0, data.quatsFile0Widit, data.quatsFile0Height);
	if (meta.Sh0.Files.size() >= 1)
		DecodeRGBA(zip, meta.Sh0.Files[0], data.sh0File0, data.sh0File0Widit, data.sh0File0Height);
	if (meta.ShN.Files.size() >= 2)
	{
		DecodeRGBA(zip, meta.ShN.Files[0], data.shNFile0, data.shNFile0Widit, data.shNFile0Height);
		DecodeRGBA(zip, meta.ShN.Files[0], data.shNFile1, data.shNFile1Widit, data.shNFile1Height);
	}
	return ReadSogV1(data, meta, callback);
}


gisLONG ReadSogV2(zip_t *zip , SogMeta& meta ,std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{
	SogDataArray data;
	if (meta.Means.Files.size() >= 2)
	{
		DecodeRGBA(zip, meta.Means.Files[0], data.meansFile0, data.meansFile0Widit, data.meansFile0Height);
		DecodeRGBA(zip, meta.Means.Files[1], data.meansFile1, data.meansFile1Widit, data.meansFile1Height);
	}
	if (meta.Scales.Files.size() >= 1)
		DecodeRGBA(zip, meta.Scales.Files[0], data.scalesFile0, data.scalesFile0Widit, data.scalesFile0Height);
	if (meta.Quats.Files.size() >= 1)
		DecodeRGBA(zip, meta.Quats.Files[0], data.quatsFile0, data.quatsFile0Widit, data.quatsFile0Height);
	if (meta.Sh0.Files.size() >= 1)
		DecodeRGBA(zip, meta.Sh0.Files[0], data.sh0File0, data.sh0File0Widit, data.sh0File0Height);
	if (meta.ShN.Files.size() >= 2)
	{
		DecodeRGBA(zip, meta.ShN.Files[0], data.shNFile0, data.shNFile0Widit, data.shNFile0Height);
		DecodeRGBA(zip, meta.ShN.Files[0], data.shNFile1, data.shNFile1Widit, data.shNFile1Height);
	}
	return ReadSogV2(data, meta, callback);
}

gisLONG ReadSogV1(CGString filePath, SogMeta& meta, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{
	SogDataArray data;
	if (meta.Means.Files.size() >= 2)
	{
		DecodeRGBA(filePath, meta.Means.Files[0], data.meansFile0, data.meansFile0Widit, data.meansFile0Height);
		DecodeRGBA(filePath, meta.Means.Files[1], data.meansFile1, data.meansFile1Widit, data.meansFile1Height);
	}
	if (meta.Scales.Files.size() >= 1)
		DecodeRGBA(filePath, meta.Scales.Files[0], data.scalesFile0, data.scalesFile0Widit, data.scalesFile0Height);
	if (meta.Quats.Files.size() >= 1)
		DecodeRGBA(filePath, meta.Quats.Files[0], data.quatsFile0, data.quatsFile0Widit, data.quatsFile0Height);
	if (meta.Sh0.Files.size() >= 1)
		DecodeRGBA(filePath, meta.Sh0.Files[0], data.sh0File0, data.sh0File0Widit, data.sh0File0Height);
	if (meta.ShN.Files.size() >= 2)
	{
		DecodeRGBA(filePath, meta.ShN.Files[0], data.shNFile0, data.shNFile0Widit, data.shNFile0Height);
		DecodeRGBA(filePath, meta.ShN.Files[0], data.shNFile1, data.shNFile1Widit, data.shNFile1Height);
	}
	return ReadSogV1(data, meta, callback);
}


gisLONG ReadSogV2(CGString filePath, SogMeta& meta, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{
	SogDataArray data;
	if (meta.Means.Files.size() >= 2)
	{
		DecodeRGBA(filePath, meta.Means.Files[0], data.meansFile0, data.meansFile0Widit, data.meansFile0Height);
		DecodeRGBA(filePath, meta.Means.Files[1], data.meansFile1, data.meansFile1Widit, data.meansFile1Height);
	}
	if (meta.Scales.Files.size() >= 1)
		DecodeRGBA(filePath, meta.Scales.Files[0], data.scalesFile0, data.scalesFile0Widit, data.scalesFile0Height);
	if (meta.Quats.Files.size() >= 1)
		DecodeRGBA(filePath, meta.Quats.Files[0], data.quatsFile0, data.quatsFile0Widit, data.quatsFile0Height);
	if (meta.Sh0.Files.size() >= 1)
		DecodeRGBA(filePath, meta.Sh0.Files[0], data.sh0File0, data.sh0File0Widit, data.sh0File0Height);
	if (meta.ShN.Files.size() >= 2)
	{
		DecodeRGBA(filePath, meta.ShN.Files[0], data.shNFile0, data.shNFile0Widit, data.shNFile0Height);
		DecodeRGBA(filePath, meta.ShN.Files[0], data.shNFile1, data.shNFile1Widit, data.shNFile1Height);
	}
	return ReadSogV2(data, meta, callback);
}

gisLONG Ci_SogGaussianReadWrite::i_Read(CGString filePath, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
{
	gisLONG rtn = 0;
	if (callback == NULL)
		return 0;
	
#if _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
	filePath.Replace('/', '\\');
#else
	filePath.Convert(CGString::EncodeType::UTF8);
	filePath.Replace('\\', '/');
#endif // _WIN32
	int index = filePath.ReverseFind(".");
	if (index < 0)
		return 0;
	CGString suffix = filePath.Right(filePath.GetLength() - index - 1);
	if (suffix.CompareNoCase("sog") == 0)
	{
		CGByteArray byteArray = CGFile::ReadAllBytes(filePath);
		zip_t *zip = zip_stream_open(&byteArray.data()[0], byteArray.size(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
		if (NULL == zip)
			return 0;
		CGByteArray metaArray;
		zip_entry_open(zip, "meta.json");
		{
			unsigned long long bufsize = zip_entry_size(zip);
			metaArray.resize(bufsize);
			zip_entry_noallocread(zip, &metaArray.data()[0], bufsize);
		}
		zip_entry_close(zip);
		SogMeta meta = GetSogMetaByCGByteArray(metaArray);

		if (meta.Version == 2)
		{
			rtn = ReadSogV2(zip, meta, callback);
		}
		else if (meta.Version == 0 || meta.Version == 1)
		{
			rtn = ReadSogV1(zip, meta, callback);

		}
		
		zip_stream_close(zip);
		zip = NULL;
	}
	else 
	{
		//选中的 meta.json 文件，说明数据在同文件夹内
		CGByteArray metaArray = CGFile::ReadAllBytes(filePath);
		SogMeta meta = GetSogMetaByCGByteArray(metaArray);
		if (meta.Version == 2)
		{
			rtn = ReadSogV2(filePath, meta, callback);
		}
		else if (meta.Version == 0 || meta.Version == 1)
		{
			rtn = ReadSogV1(filePath, meta, callback);
		}
	}
	return rtn;
}


gisLONG Ci_SogGaussianReadWrite::i_Write(CGString filePath, const  MapGIS::Tile::GaussianModel & data)
{
	//暂时不支持写

	return 0;

}
