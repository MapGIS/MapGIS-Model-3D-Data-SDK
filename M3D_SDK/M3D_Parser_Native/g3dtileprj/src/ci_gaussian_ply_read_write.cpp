#include "stdafx.h"
#include "ci_gaussian_ply_read_write.h"
#include "tinyply.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cgfile.h>
#include "load-spz.h"

template<typename T, typename T2> 
void  copyDataValue(const T * orData , T2 * desData, int num)
{
	if (orData == NULL || desData == NULL || num <= 0)
		return;
	for (int i = 0; i < num; i++)
	{
		desData[i] = (T2)(orData[i]);
	}
}

void copyData(uint8_t* orData, tinyply::Type orType, int officeNumData, void* desData, tinyply::Type desType, int copyNum)
{
	int dataSize = 0;
	switch (orType)
	{
	case tinyply::Type::INT8:
	case tinyply::Type::UINT8:
		dataSize = sizeof(char);
		break;
	case tinyply::Type::INT16:
	case tinyply::Type::UINT16:
		dataSize = sizeof(short);
		break;
	case tinyply::Type::INT32:
	case tinyply::Type::UINT32:
		dataSize = sizeof(int);
		break;
	case tinyply::Type::FLOAT32:
		dataSize = sizeof(float);
		break;
	case tinyply::Type::FLOAT64:
		dataSize = sizeof(double);
		break;
	case tinyply::Type::INVALID:
	default:
		return;
	}
	if (orType == desType)
	{
		std::memcpy(desData, orData + dataSize * officeNumData, dataSize *copyNum);
	}
	else
	{
		void * startValue = orData + dataSize * officeNumData;
		if (desType == tinyply::Type::INT8)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (char*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (char*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (char*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (char*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (char*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (char*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (char*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (char*)desData, copyNum);  break;
			}
		}
		else if (desType == tinyply::Type::UINT8)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (unsigned char*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (unsigned char*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (unsigned char*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (unsigned char*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (unsigned char*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (unsigned char*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (unsigned char*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (unsigned char*)desData, copyNum);  break;
			}
		}
		else if (desType == tinyply::Type::INT16)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (short*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (short*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (short*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (short*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (short*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (short*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (short*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (short*)desData, copyNum);  break;
			}
		}
		else if (desType == tinyply::Type::UINT16)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (unsigned short*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (unsigned short*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (unsigned short*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (unsigned short*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (unsigned short*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (unsigned short*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (unsigned short*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (unsigned short*)desData, copyNum);  break;
			}
		}
		else if (desType == tinyply::Type::INT32)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (int*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (int*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (int*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (int*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (int*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (int*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (int*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (int*)desData, copyNum);  break;
			}
		}
		else if (desType == tinyply::Type::UINT32)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (unsigned int*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (unsigned int*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (unsigned int*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (unsigned int*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (unsigned int*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (unsigned int*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (unsigned int*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (unsigned int*)desData, copyNum);  break;
			}
		}
		else if (desType == tinyply::Type::FLOAT32)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (float*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (float*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (float*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (float*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (float*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (float*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (float*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (float*)desData, copyNum);  break;
			}
		}
		else if (desType == tinyply::Type::FLOAT64)
		{
			switch (orType)
			{
			case tinyply::Type::INT8: copyDataValue((char*)startValue, (double*)desData, copyNum);  break;
			case tinyply::Type::UINT8:copyDataValue((unsigned char*)startValue, (double*)desData, copyNum);  break;
			case tinyply::Type::INT16:copyDataValue((short*)startValue, (double*)desData, copyNum);  break;
			case tinyply::Type::UINT16:copyDataValue((unsigned short*)startValue, (double*)desData, copyNum);  break;
			case tinyply::Type::INT32:copyDataValue((int*)startValue, (double*)desData, copyNum);  break;
			case tinyply::Type::UINT32:copyDataValue((unsigned int*)startValue, (double*)desData, copyNum);  break;
			case tinyply::Type::FLOAT32:copyDataValue((float*)startValue, (double*)desData, copyNum);  break;
			case tinyply::Type::FLOAT64:copyDataValue((double*)startValue, (double*)desData, copyNum);  break;
			}
		}
	}
}


gisLONG Ci_PlyGaussianReadWrite::i_Read(CGString filePath,  MapGIS::Tile::GaussianModel & data)
{
	std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)> callback = [&data]( MapGIS::Tile::GaussianFeature & item, bool& isStop)
	{
		data.features.push_back(item);
	};
	return i_Read(filePath, callback);
}

gisLONG Ci_PlyGaussianReadWrite::i_Read(CGString filePath, std::function<void( MapGIS::Tile::GaussianFeature & item, bool& isStop)>& callback)
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

	std::ifstream file(filePath.CStr(), std::ios::binary);
	
	auto	getProperty = [](const std::string & key, const std::vector<tinyply::PlyProperty> & list)->
	int64_t	{
		for (size_t i = 0; i < list.size(); ++i) 
			if (list[i].name == key) 
				return i;
		return -1;
	};

	 MapGIS::Tile::GaussianFeature item;
	bool isStop = false; 
	try {
		tinyply::PlyFile reader;
		reader.parse_header(file);
		// 查找顶点位置元素
		auto vertices = reader.get_elements();
		for (const auto& vertice : vertices)
		{
			if (isStop)
				break;
			if (vertice.name == "vertex")
			{
				std::shared_ptr<tinyply::PlyData> pointX;
				std::shared_ptr<tinyply::PlyData> pointY;
				std::shared_ptr<tinyply::PlyData> pointZ;
				tinyply::Type pointXType = tinyply::Type::INVALID;
				tinyply::Type pointYType = tinyply::Type::INVALID;
				tinyply::Type pointZType = tinyply::Type::INVALID;

				std::shared_ptr<tinyply::PlyData> rot0;
				std::shared_ptr<tinyply::PlyData> rot1;
				std::shared_ptr<tinyply::PlyData> rot2;
				std::shared_ptr<tinyply::PlyData> rot3;
				tinyply::Type rot0Type = tinyply::Type::INVALID;
				tinyply::Type rot1Type = tinyply::Type::INVALID;
				tinyply::Type rot2Type = tinyply::Type::INVALID;
				tinyply::Type rot3Type = tinyply::Type::INVALID;

				std::shared_ptr<tinyply::PlyData> scale0;
				std::shared_ptr<tinyply::PlyData> scale1;
				std::shared_ptr<tinyply::PlyData> scale2;
				tinyply::Type scale0Type = tinyply::Type::INVALID;
				tinyply::Type scale1Type = tinyply::Type::INVALID;
				tinyply::Type scale2Type = tinyply::Type::INVALID;

				std::shared_ptr<tinyply::PlyData> f_dc0;
				std::shared_ptr<tinyply::PlyData> f_dc1;
				std::shared_ptr<tinyply::PlyData> f_dc2;
				tinyply::Type f_dc0Type = tinyply::Type::INVALID;
				tinyply::Type f_dc1Type = tinyply::Type::INVALID;
				tinyply::Type f_dc2Type = tinyply::Type::INVALID;

				std::shared_ptr<tinyply::PlyData> opacity;
				tinyply::Type opacityType = tinyply::Type::INVALID;


				std::vector<std::shared_ptr<tinyply::PlyData>>  shs;
				std::vector<tinyply::Type> shsType;



				int64_t index = getProperty("x", vertice.properties);
				if (index >= 0)
				{
					pointX = reader.request_properties_from_element("vertex", { "x" });
					pointXType = vertice.properties[index].propertyType;
				}
				index = getProperty("y", vertice.properties);
				if (index >= 0)
				{
					pointY = reader.request_properties_from_element("vertex", { "y" });
					pointYType = vertice.properties[index].propertyType;
				}
				index = getProperty("z", vertice.properties);
				if (index >= 0)
				{
					pointZ = reader.request_properties_from_element("vertex", { "z" });
					pointZType = vertice.properties[index].propertyType;
				}

				if (pointX.get() == NULL || pointY.get() == NULL || pointZ.get() == NULL)
					break;

				index = getProperty("rot_0", vertice.properties);
				if (index >= 0)
				{
					rot0 = reader.request_properties_from_element("vertex", { "rot_0" });
					rot0Type = vertice.properties[index].propertyType;
				}
				index = getProperty("rot_1", vertice.properties);
				if (index >= 0)
				{
					rot1 = reader.request_properties_from_element("vertex", { "rot_1" });
					rot1Type = vertice.properties[index].propertyType;
				}
				index = getProperty("rot_2", vertice.properties);
				if (index >= 0)
				{
					rot2 = reader.request_properties_from_element("vertex", { "rot_2" });
					rot2Type = vertice.properties[index].propertyType;
				}
				index = getProperty("rot_3", vertice.properties);
				if (index >= 0)
				{
					rot3 = reader.request_properties_from_element("vertex", { "rot_3" });
					rot3Type = vertice.properties[index].propertyType;
				}

				index = getProperty("scale_0", vertice.properties);
				if (index >= 0)
				{
					scale0 = reader.request_properties_from_element("vertex", { "scale_0" });
					scale0Type = vertice.properties[index].propertyType;
				}
				index = getProperty("scale_1", vertice.properties);
				if (index >= 0)
				{
					scale1 = reader.request_properties_from_element("vertex", { "scale_1" });
					scale1Type = vertice.properties[index].propertyType;
				}
				index = getProperty("scale_2", vertice.properties);
				if (index >= 0)
				{
					scale2 = reader.request_properties_from_element("vertex", { "scale_2" });
					scale2Type = vertice.properties[index].propertyType;
				}

				index = getProperty("f_dc_0", vertice.properties);
				if (index >= 0)
				{
					f_dc0 = reader.request_properties_from_element("vertex", { "f_dc_0" });
					f_dc0Type = vertice.properties[index].propertyType;
				}
				index = getProperty("f_dc_1", vertice.properties);
				if (index >= 0)
				{
					f_dc1 = reader.request_properties_from_element("vertex", { "f_dc_1" });
					f_dc1Type = vertice.properties[index].propertyType;
				}
				index = getProperty("f_dc_2", vertice.properties);
				if (index >= 0)
				{
					f_dc2 = reader.request_properties_from_element("vertex", { "f_dc_2" });
					f_dc2Type = vertice.properties[index].propertyType;
				}

				index = getProperty("opacity", vertice.properties);
				if (index >= 0)
				{
					opacity = reader.request_properties_from_element("vertex", { "opacity" });
					opacityType = vertice.properties[index].propertyType;
				}


				for (int i = 0; i < 60; i++)
				{
					string name = "f_rest_" + to_string(i);
					index = getProperty(name, vertice.properties);
					if (index >= 0)
					{
						shs.push_back(reader.request_properties_from_element("vertex", { name }));
						shsType.push_back(vertice.properties[index].propertyType) ;
					}
					else
						break;;

				}
				reader.read(file);

				size_t num = vertice.size;

				for (int i = 0; i < num; i++)
				{
					memset(&item, 0, sizeof( MapGIS::Tile::GaussianFeature));

					copyData(pointX->buffer.get(), pointXType, i, &item.position[0], tinyply::Type::FLOAT32, 1);
					copyData(pointY->buffer.get(), pointYType, i, &item.position[1], tinyply::Type::FLOAT32, 1);
					copyData(pointZ->buffer.get(), pointZType, i, &item.position[2], tinyply::Type::FLOAT32, 1);
				

					if (rot0.get() != NULL && rot1.get() != NULL && rot2.get() != NULL && rot3.get() != NULL)
					{
						copyData(rot0->buffer.get(), rot1Type, i, &item.rotation[3], tinyply::Type::FLOAT32, 1);
						copyData(rot1->buffer.get(), rot2Type, i, &item.rotation[0], tinyply::Type::FLOAT32, 1);
						copyData(rot2->buffer.get(), rot3Type, i, &item.rotation[1], tinyply::Type::FLOAT32, 1);
						copyData(rot3->buffer.get(), rot0Type, i, &item.rotation[2], tinyply::Type::FLOAT32, 1);
					}

					if (scale0.get() != NULL && scale1.get() != NULL && scale2.get() != NULL)
					{
						copyData(scale0->buffer.get(), scale0Type, i, &item.scale[0], tinyply::Type::FLOAT32, 1);
						copyData(scale1->buffer.get(), scale1Type, i, &item.scale[1], tinyply::Type::FLOAT32, 1);
						copyData(scale2->buffer.get(), scale2Type, i, &item.scale[2], tinyply::Type::FLOAT32, 1);
					}

					if (f_dc0.get() != NULL && f_dc1.get() != NULL && f_dc2.get() != NULL)
					{

						copyData(f_dc0->buffer.get(), f_dc0Type, i, &item.color[0], tinyply::Type::FLOAT32, 1);
						copyData(f_dc1->buffer.get(), f_dc1Type, i, &item.color[1], tinyply::Type::FLOAT32, 1);
						copyData(f_dc2->buffer.get(), f_dc2Type, i, &item.color[2], tinyply::Type::FLOAT32, 1);

					}

					if (opacity.get() != NULL)
					{
						copyData(opacity->buffer.get(), opacityType, i, &item.alpha, tinyply::Type::FLOAT32, 1);
					}

					if (shs.size() > 0)
					{
						item.sh.resize(shs.size());

						int shDim =  shs.size() / 3;

						for (int j = 0; j < shDim; j++) 
						{
							copyData(shs[j]->buffer.get(), shsType[j], i, &item.sh[j*3], tinyply::Type::FLOAT32, 1);

							copyData(shs[j + shDim]->buffer.get(), shsType[j + shDim], i, &item.sh[j * 3 +1], tinyply::Type::FLOAT32, 1);

							copyData(shs[j + 2* shDim]->buffer.get(), shsType[j + 2 * shDim], i, &item.sh[j * 3 + 2], tinyply::Type::FLOAT32, 1);
						}
					}
			
					callback(item, isStop);
					if (isStop)
						break;
				}
				rtn = 1;
			}
		}
		file.close();
	}
	catch (const std::exception& e)
	{
		rtn = 0;
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return rtn;
	/*
	#if _WIN32
	filePath.Convert(CGString::EncodeType::GB18030);
	filePath.Replace('/', '\\');
#else
	filePath.Convert(CGString::EncodeType::UTF8);
	filePath.Replace('\\', '/');
#endif // _WIN32
	spz::UnpackOptions o;
	o.to = spz::CoordinateSystem::RDF;
	spz::GaussianCloud gaussianCloudValue = spz::loadSplatFromPly(filePath.StdString(), o);
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

			m_Rect.xmin = min(m_Rect.xmin, (double)item.position[0]);
			m_Rect.ymin = min(m_Rect.ymin, (double)item.position[1]);
			m_Rect.zmin = min(m_Rect.zmin, (double)item.position[2]);

			m_Rect.xmax = max(m_Rect.xmax, (double)item.position[0]);
			m_Rect.ymax = max(m_Rect.ymax, (double)item.position[1]);
			m_Rect.zmax = max(m_Rect.zmax, (double)item.position[2]);

			callback(item, isStop);
			if (isStop)
				break;
		}
		return 1;
	}
	return 0;*/
}


gisLONG Ci_PlyGaussianReadWrite::i_Write(CGString filePath, const  MapGIS::Tile::GaussianModel & data)
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

	if (data.features.size() <= 0)
		return 0;

	int32_t shDegree = 0;

	if (data.features[0].sh.size() < 3)
		shDegree = 0;
	else if (data.features[0].sh.size() < 8)
		shDegree = 1;

	else if (data.features[0].sh.size() < 15)
		shDegree = 2;
	else
		shDegree = 3;
	spz::GaussianCloud result;
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
	spz::PackOptions options;
	options.from = data.coordinateSystem;
	return spz::saveSplatToPly(result, options, filePath.StdString()) ? 1 : 0;



	//if (data.coordinateSystem != GaussianCoordinateSystem::RDF) 
	//{

	//}
	//std::filebuf fWrite;
	//fWrite.open(filePath.CStr(), std::ios::out | std::ios::binary);
	//std::ostream outstream_binary(&fWrite);
	//if (outstream_binary.fail())
	//	return 0;

	//tinyply::PlyFile cube_file;
	//vector<float> Xvalue;
	//vector<float> Yvalue;
	//vector<float> Zvalue;
	//vector<float> rot0value;
	//vector<float> rot1value;
	//vector<float> rot2value;
	//vector<float> rot3value;
	//vector<float> scale0value;
	//vector<float> scale1value;
	//vector<float> scale2value;
	//vector<float> f_dc0value;
	//vector<float> f_dc1value;
	//vector<float> f_dc2value;
	//vector<float> opacityvalue;

	//Xvalue.reserve(data.features.size());
	//Yvalue.reserve(data.features.size());
	//Zvalue.reserve(data.features.size());
	//rot0value.reserve(data.features.size());
	//rot1value.reserve(data.features.size());
	//rot2value.reserve(data.features.size());
	//rot3value.reserve(data.features.size());
	//scale0value.reserve(data.features.size());
	//scale1value.reserve(data.features.size());
	//scale2value.reserve(data.features.size());
	//f_dc0value.reserve(data.features.size());
	//f_dc1value.reserve(data.features.size());
	//f_dc2value.reserve(data.features.size());
	//opacityvalue.reserve(data.features.size());

	//for (int i = 0; i < data.features.size(); i++)
	//{
	//	Xvalue.push_back(data.features[i].position[0]);
	//	Yvalue.push_back(data.features[i].position[1]);
	//	Zvalue.push_back(data.features[i].position[2]);
	//	rot0value.push_back(data.features[i].rotation[0]);
	//	rot1value.push_back(data.features[i].rotation[1]);
	//	rot2value.push_back(data.features[i].rotation[2]);
	//	rot3value.push_back(data.features[i].rotation[3]);
	//	scale0value.push_back(data.features[i].scale[0]);
	//	scale1value.push_back(data.features[i].scale[1]);
	//	scale2value.push_back(data.features[i].scale[2]);
	//	f_dc0value.push_back(data.features[i].color[0]);
	//	f_dc1value.push_back(data.features[i].color[1]);
	//	f_dc2value.push_back(data.features[i].color[2]);
	//	opacityvalue.push_back(data.features[i].alpha);
	//}

	//cube_file.add_properties_to_element("vertex", { "x" }, tinyply::Type::FLOAT32, Xvalue.size(), reinterpret_cast<uint8_t*>(Xvalue.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "y" }, tinyply::Type::FLOAT32, Yvalue.size(), reinterpret_cast<uint8_t*>(Yvalue.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "z" }, tinyply::Type::FLOAT32, Zvalue.size(), reinterpret_cast<uint8_t*>(Zvalue.data()), tinyply::Type::INVALID, 0);

	//cube_file.add_properties_to_element("vertex", { "rot_0" }, tinyply::Type::FLOAT32, rot0value.size(), reinterpret_cast<uint8_t*>(rot0value.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "rot_1" }, tinyply::Type::FLOAT32, rot1value.size(), reinterpret_cast<uint8_t*>(rot1value.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "rot_2" }, tinyply::Type::FLOAT32, rot2value.size(), reinterpret_cast<uint8_t*>(rot2value.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "rot_3" }, tinyply::Type::FLOAT32, rot3value.size(), reinterpret_cast<uint8_t*>(rot3value.data()), tinyply::Type::INVALID, 0);

	//cube_file.add_properties_to_element("vertex", { "scale_0" }, tinyply::Type::FLOAT32, scale0value.size(), reinterpret_cast<uint8_t*>(scale0value.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "scale_1" }, tinyply::Type::FLOAT32, scale1value.size(), reinterpret_cast<uint8_t*>(scale1value.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "scale_2" }, tinyply::Type::FLOAT32, scale2value.size(), reinterpret_cast<uint8_t*>(scale2value.data()), tinyply::Type::INVALID, 0);

	//cube_file.add_properties_to_element("vertex", { "f_dc_0" }, tinyply::Type::FLOAT32, f_dc0value.size(), reinterpret_cast<uint8_t*>(f_dc0value.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "f_dc_1" }, tinyply::Type::FLOAT32, f_dc1value.size(), reinterpret_cast<uint8_t*>(f_dc1value.data()), tinyply::Type::INVALID, 0);
	//cube_file.add_properties_to_element("vertex", { "f_dc_2" }, tinyply::Type::FLOAT32, f_dc2value.size(), reinterpret_cast<uint8_t*>(f_dc2value.data()), tinyply::Type::INVALID, 0);

	//cube_file.add_properties_to_element("vertex", { "opacity" }, tinyply::Type::FLOAT32, opacityvalue.size(), reinterpret_cast<uint8_t*>(opacityvalue.data()), tinyply::Type::INVALID, 0);

	//cube_file.get_comments().push_back("mapGIS by tinyply 2.3");

	//vector<float> shvalue;
	//shvalue.reserve(data.features.size());
	//if (data.features.size() > 0 && data.features[0].sh.size() > 0)
	//{

	//	for (int i = 0; i < data.features[0].sh.size(); i++)
	//	{
	//		shvalue.clear();
	//		for (int j = 0; j < data.features.size(); j++)
	//		{
	//			shvalue.push_back(data.features[j].sh[i]);
	//		}
	//		string name = "f_rest_" + to_string(i);
	//		cube_file.add_properties_to_element("vertex", { name }, tinyply::Type::FLOAT32, shvalue.size(), reinterpret_cast<uint8_t*>(shvalue.data()), tinyply::Type::INVALID, 0);
	//	}
	//}

	//// Write a binary file
	//cube_file.write(outstream_binary, true);
	//fWrite.close();
	//return 1;
}
