#include "stdafx.h"
#include "ci_gaussian_spz_read_write.h"
#include "load-spz.h"
#include "cgfile.h"
#include <fstream>


gisLONG Ci_SpzGaussianReadWrite::Read(const vector<uint8_t>& in,  MapGIS::Tile::GaussianModel & data)
{
	gisLONG rtn = i_Read(in, data);
	return rtn;
}

gisLONG Ci_SpzGaussianReadWrite::Write(const  MapGIS::Tile::GaussianModel & data, vector<uint8_t>& out)
{
	return i_Write(data, out);
}

gisLONG Ci_SpzGaussianReadWrite::i_Read(CGString filePath,  MapGIS::Tile::GaussianModel & data)
{
#if _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
	filePath.Replace('/', '\\');
#else
	filePath.Convert(CGString::EncodeType::UTF8);
	filePath.Replace('\\', '/');
#endif // _WIN32
	spz::UnpackOptions o;
	spz::GaussianCloud outValue = spz::loadSpz(filePath.StdString(), o);
	std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)> callback = [&data]( MapGIS::Tile::GaussianFeature & item, bool& isStop)
	{
		data.features.push_back(item);
	};
	return i_Read(outValue, callback);
}

gisLONG Ci_SpzGaussianReadWrite::i_Read(CGString filePath, std::function<void( MapGIS::Tile::GaussianFeature & dot, bool& isStop)>& callback)
{
	if (callback == NULL)
		return 0;
#if _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
	filePath.Replace('/', '\\');
#else
	filePath.Convert(CGString::EncodeType::UTF8);
	filePath.Replace('\\', '/');
#endif // _WIN32
	spz::UnpackOptions o;
	spz::GaussianCloud outValue = spz::loadSpz(filePath.StdString(), o);
	return i_Read(outValue, callback);
}

gisLONG Ci_SpzGaussianReadWrite::i_Read(const vector<uint8_t>& in,  MapGIS::Tile::GaussianModel & data)
{
	spz::UnpackOptions o;
	spz::GaussianCloud outValue = spz::loadSpz(in, o);
	std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)> callback = [&data]( MapGIS::Tile::GaussianFeature & item, bool& isStop)
	{
		data.features.push_back(item);
	};
	return i_Read(outValue, callback);
}

gisLONG  Ci_SpzGaussianReadWrite::i_Read(spz::GaussianCloud &gaussianCloudValue, std::function<void( MapGIS::Tile::GaussianFeature & dot, bool& isStop)>&  callback)
{
	if (callback == NULL)
		return 0;
	bool isStop = false;
	int32_t num = gaussianCloudValue.numPoints;
	if (num > 0)
	{
		const int32_t shDim = static_cast<int>(gaussianCloudValue.sh.size() / num);

		for (int i = 0; i < num; i++)
		{

			 MapGIS::Tile::GaussianFeature item;
			item.position[0] = gaussianCloudValue.positions[i * 3 + 0];
			item.position[1] = gaussianCloudValue.positions[i * 3 + 1];
			item.position[2] = gaussianCloudValue.positions[i * 3 + 2];

			item.scale[0] = gaussianCloudValue.scales[i * 3 + 0];
			item.scale[1] = gaussianCloudValue.scales[i * 3 + 1];
			item.scale[2] = gaussianCloudValue.scales[i * 3 + 2];

			item.rotation[0] = gaussianCloudValue.rotations[i * 4 + 0];
			item.rotation[1] = gaussianCloudValue.rotations[i * 4 + 1];
			item.rotation[2] = gaussianCloudValue.rotations[i * 4 + 2];
			item.rotation[3] = gaussianCloudValue.rotations[i * 4 + 3];

			item.color[0] = gaussianCloudValue.colors[i * 3 + 0];
			item.color[1] = gaussianCloudValue.colors[i * 3 + 1];
			item.color[2] = gaussianCloudValue.colors[i * 3 + 2];

			item.alpha = gaussianCloudValue.alphas[i];

			for (int j = 0; j < shDim; j++)
			{
				item.sh.push_back(gaussianCloudValue.sh[i*shDim + j]);
			}

			callback(item, isStop);
			if (isStop)
				break;
		}
		return 1;
	}
	return 0;
}

int32_t degreeForDim(int32_t dim) {
	if (dim < 3)
		return 0;
	if (dim < 8)
		return 1;
	if (dim < 15)
		return 2;
	return 3;
}


gisLONG Ci_SpzGaussianReadWrite::i_Write(CGString filePath, const  MapGIS::Tile::GaussianModel & data)
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

	spz::GaussianCloud result;
	gisLONG rtn = i_Write(data, result);
	if (rtn > 0)
	{
		spz::PackOptions options;
		options.from = data.coordinateSystem;
		rtn = spz::saveSpz(result, options, filePath.StdString()) ? 1 : 0;
	}
	return rtn;
}

gisLONG Ci_SpzGaussianReadWrite::i_Write(const  MapGIS::Tile::GaussianModel & data, vector<uint8_t>& out)
{
	spz::GaussianCloud result;
	gisLONG rtn = i_Write(data, result);
	if (rtn > 0) 
	{
		spz::PackOptions options;
		options.from = data.coordinateSystem;
		rtn =  spz::saveSpz(result, options, &out) ? 1 : 0;
	}
	return rtn;
}


gisLONG Ci_SpzGaussianReadWrite::i_Write(const  MapGIS::Tile::GaussianModel & data, spz::GaussianCloud &result)
{
	if (data.features.size() <= 0)
		return 0;

	int32_t shDegree = 0;

	if (data.features[0].sh.size()/3 < 3)
		shDegree = 0;
	else if (data.features[0].sh.size() / 3 < 8)
		shDegree = 1;
	else if (data.features[0].sh.size() / 3 < 15)
		shDegree = 2;
	else
		shDegree = 3;

	result.numPoints = data.features.size();
	result.shDegree = shDegree;
	result.positions.reserve(data.features.size() * 3);
	result.scales.reserve(data.features.size() * 3);
	result.rotations.reserve(data.features.size() * 4);
	result.alphas.reserve(data.features.size() * 1);
	result.colors.reserve(data.features.size() * 3);

	if (data.features[0].sh.size() > 0)
		result.sh.reserve(data.features.size() *data.features[0].sh.size());

	for (int i = 0; i < data.features.size(); i++)
	{
		result.positions.push_back(data.features[i].position[0]);
		result.positions.push_back(data.features[i].position[1]);
		result.positions.push_back(data.features[i].position[2]);

		result.scales.push_back(data.features[i].scale[0]);
		result.scales.push_back(data.features[i].scale[1]);
		result.scales.push_back(data.features[i].scale[2]);

		result.rotations.push_back(data.features[i].rotation[0]);
		result.rotations.push_back(data.features[i].rotation[1]);
		result.rotations.push_back(data.features[i].rotation[2]);
		result.rotations.push_back(data.features[i].rotation[3]);

		result.colors.push_back(data.features[i].color[0]);
		result.colors.push_back(data.features[i].color[1]);
		result.colors.push_back(data.features[i].color[2]);
		result.alphas.push_back(data.features[i].alpha);

		for (int j = 0; j < data.features[i].sh.size(); j++)
		{
			result.sh.push_back(data.features[i].sh[j]);
		}
	}
	return 1;
}