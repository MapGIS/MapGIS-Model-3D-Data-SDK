#include "stdafx.h"

#include "ci_3dmodel.h"
#include "ci_assist.h"
#include "ci_gltfsdktool.h"
#include "ci_dracotool.h"
#include "rapidjson/document.h"
using namespace MapGIS::Tile;

static RecordItemValue JsonValueToRecordValue(rapidjson::Value& item)
{
	if (item.IsBool())
		return RecordItemValue(item.GetBool());
	else if (item.IsNumber())
		return RecordItemValue(item.GetDouble());
	else if (item.IsString())
		return RecordItemValue(CGString(item.GetString(),CGString::EncodeType::UTF8));
	return RecordItemValue();
}

static gisLONG i_PointPNTS2Buffer(vector<float> &pointVector, vector<unsigned char> &colorVector, vector<unsigned int>& batchIDVector, vector<gisINT64>& OIDVector, bool isDraco, LayerAttribute* pRecords, D_3DOT* pRtcCenter, CGByteArray& vbuf,string idName)
{
	CGByteArray featureTableJsonByteArray;
	CGByteArray featureTableBinByteArray;
	CGByteArray batchTableJsonByteArray;
	CGByteArray batchTableBinByteArray;
	int posAttId = 0, colorAttId = 0, batchIdAttId = 0;

	int batchIDBinMore = 0;

	vector<char> BinData;
	if (isDraco)
	{
		BinData = Ci_DracoTool::CreatePointCloudDracoBinData(pointVector, colorVector, batchIDVector, posAttId, colorAttId, batchIdAttId);
		featureTableBinByteArray.append(&BinData[0], BinData.size());
	}
	else
	{
		featureTableBinByteArray.reserve(pointVector.size() * 4 + colorVector.size() + batchIDVector.size() * 4);
		for (int i = 0; i < pointVector.size(); i++)
		{
			FloatToByteArray(featureTableBinByteArray, pointVector[i]);
		}
		for (int i = 0; i < colorVector.size(); i++)
		{
			CharToByteArray(featureTableBinByteArray, colorVector[i]);
		}
		batchIDBinMore = (int)(ceil(featureTableBinByteArray.size() / 4.0) * 4 - featureTableBinByteArray.size());

		for (int i = 0; i < batchIDBinMore; i++)
			featureTableBinByteArray.append(" ");

		for (int i = 0; i < batchIDVector.size(); i++) {
			UnsignedIntToByteArray(featureTableBinByteArray, batchIDVector[i]);
		}
	}

	string BATCH_LENGTH = std::to_string(OIDVector.size());
	string POINTS_LENGTH = std::to_string(pointVector.size() / 3);

	string RGB_byteOffset = std::to_string(pointVector.size() * 4);
	string BATCH_ID_byteOffset = std::to_string(pointVector.size() * 4 + colorVector.size() + batchIDBinMore);

	featureTableJsonByteArray.reserve(2048);
	if (isDraco)
	{
		RGB_byteOffset = "0";
		BATCH_ID_byteOffset = "0";
	}

	featureTableJsonByteArray.append("{");

	featureTableJsonByteArray.append(("\"BATCH_LENGTH\":" + BATCH_LENGTH + ",").c_str());

	featureTableJsonByteArray.append(("\"POINTS_LENGTH\":" + POINTS_LENGTH + ",").c_str());

	featureTableJsonByteArray.append("\"POSITION\":{\"byteOffset\": 0},");

	featureTableJsonByteArray.append(("\"RGB\":{\"byteOffset\":" + RGB_byteOffset + "},").c_str());
	featureTableJsonByteArray.append(("\"BATCH_ID\":{\"byteOffset\":" + BATCH_ID_byteOffset + "," +"\"componentType\":\"UNSIGNED_INT\"}").c_str());

	if (isDraco)
	{
		featureTableJsonByteArray.append(",");
		featureTableJsonByteArray.append("\"extensions\" : {\"3DTILES_draco_point_compression\" : {\"properties\": {");
		featureTableJsonByteArray.append(("\"POSITION\":" + std::to_string(posAttId) + ",").c_str());
		featureTableJsonByteArray.append(("\"RGB\":" + std::to_string(colorAttId) + ",").c_str());
		featureTableJsonByteArray.append(("\"BATCH_ID\":" + std::to_string(batchIdAttId)).c_str());
		featureTableJsonByteArray.append("},\"byteOffset\":0,");
		featureTableJsonByteArray.append(("\"byteLength\" :" + std::to_string(featureTableBinByteArray.size())).c_str());
		featureTableJsonByteArray.append("}}");
	}

	featureTableJsonByteArray.append("}");

	if (pRecords == NULL)
	{
		if (idName.length() > 0)
		{
			batchTableJsonByteArray.append("{");
			batchTableJsonByteArray.append("\"");
			batchTableJsonByteArray.append(idName.c_str());
			batchTableJsonByteArray.append("\":[");

			for (int i = 0; i < OIDVector.size(); i++)
			{
				if (i == OIDVector.size() - 1)
				{
					batchTableJsonByteArray.append(std::to_string(OIDVector[i]).c_str());
				}
				else
				{
					batchTableJsonByteArray.append(std::to_string(OIDVector[i]).c_str());
					batchTableJsonByteArray.append(",");
				}
			}
			batchTableJsonByteArray.append("]}");
		}
	}
	else
	{
		map<gisINT64, gisINT64> idToBatchID;
		for (int i = 0; i < OIDVector.size(); i++)
		{
			idToBatchID.insert(make_pair(OIDVector[i], i));
		}
		Ci_3DTileBatchTable batchTable;
		batchTable.WriteTo(pRecords, &idToBatchID, batchTableJsonByteArray, batchTableBinByteArray, idName);
	}

	int featureTableJsonMore = (int)(ceil(featureTableJsonByteArray.size() / 4.0) * 4 - featureTableJsonByteArray.size());
	int featureTableBinMore = (int)(ceil(featureTableBinByteArray.size() / 4.0) * 4 - featureTableBinByteArray.size());
	int batchTableJsonMore = (int)(ceil(batchTableJsonByteArray.size() / 4.0) * 4 - batchTableJsonByteArray.size());
	int batchTableBinMore = (int)(ceil(batchTableBinByteArray.size() / 4.0) * 4 - batchTableBinByteArray.size());

	for (int i = 0; i < featureTableJsonMore; i++)
		featureTableJsonByteArray.append(" ");
	for (int i = 0; i < featureTableBinMore; i++)
		featureTableBinByteArray.append(" ");
	for (int i = 0; i < batchTableJsonMore; i++)
		batchTableJsonByteArray.append(" ");
	for (int i = 0; i < batchTableBinMore; i++)
		batchTableBinByteArray.append(" ");

	int featureTableJsonSize = featureTableJsonByteArray.size();
	int featureTableBinSize = featureTableBinByteArray.size();
	int batchTableJsonSize = batchTableJsonByteArray.size();
	int batchTableBinSize = batchTableBinByteArray.size();

	int pntsBinSize = 28 + featureTableJsonSize + featureTableBinSize + batchTableJsonSize + batchTableBinSize;

	vbuf.clear();
	vbuf.reserve(pntsBinSize);

	int version = 1;

	string pnts = "pnts";
	StringToByteArray(vbuf, pnts);
	IntToByteArray(vbuf, version);
	IntToByteArray(vbuf, pntsBinSize);
	IntToByteArray(vbuf, featureTableJsonSize);
	IntToByteArray(vbuf, featureTableBinSize);
	IntToByteArray(vbuf, batchTableJsonSize);
	IntToByteArray(vbuf, batchTableBinSize);

	vbuf.append(featureTableJsonByteArray.data(), featureTableJsonByteArray.size());
	vbuf.append(featureTableBinByteArray.data(), featureTableBinByteArray.size());
	vbuf.append(batchTableJsonByteArray.data(), batchTableJsonByteArray.size());
	vbuf.append(batchTableBinByteArray.data(), batchTableBinByteArray.size());
	return 1;
}

struct Ci_PointsFeature
{
	vector<ColorPoint> colorPoints;
};

void GetPointInfo(MapGIS::Tile::PointsModel &pointModel, vector<float> &pointVector, vector<unsigned char>& colorVector, vector<unsigned int>& batchIDVector, vector<gisINT64>& OIDVector)
{
	gisLONG pointsNum = 0;
	for (int i = 0; i < pointModel.features.size(); i++)
	{
		for (int j = 0; j < pointModel.features[i].colorPoints.size(); j++)
		{
			pointsNum += pointModel.features[i].colorPoints[j].points.GetNum();
		}
	}
	unordered_map<gisINT64, gisINT64> IdMap;
	gisINT64 maxBatchID = 0;
	gisINT64 currentBatchID = 0;
	pointVector.reserve(pointsNum * 3);
	colorVector.reserve(pointsNum * 3);
	batchIDVector.reserve(pointsNum);
	for (int i = 0; i < pointModel.features.size(); i++)
	{
		if (IdMap.find(pointModel.features[i].id) != IdMap.end())
			currentBatchID = IdMap[pointModel.features[i].id];
		else
		{
			IdMap.insert(make_pair(pointModel.features[i].id, maxBatchID));
			OIDVector.push_back(pointModel.features[i].id);
			currentBatchID = maxBatchID;
			maxBatchID++;
		}

		for (int j = 0; j < pointModel.features[i].colorPoints.size(); j++)
		{
			MapGIS::Tile::Color4f& color = pointModel.features[i].colorPoints[j].color;
			CPoints& points = pointModel.features[i].colorPoints[j].points;
			int pointCount = points.GetNum();
			D_3DOT  * dots = points.GetBufPtr();
			for (int k = 0; k < pointCount; k++)
			{
				pointVector.emplace_back((float)dots[k].x);
				pointVector.emplace_back((float)dots[k].y);
				pointVector.emplace_back((float)dots[k].z);

				colorVector.emplace_back((unsigned char)(color.r * 255));
				colorVector.emplace_back((unsigned char)(color.g * 255));
				colorVector.emplace_back((unsigned char)(color.b * 255));
				batchIDVector.emplace_back(currentBatchID);
			}
		}
	}
}

//常规点云数据
gisLONG Ci_ModelPnts::From(MapGIS::Tile::PointsModel &pointModel, LayerAttribute* pRecords, bool isDracoCompress, D_3DOT*  pRtcCenter)
{
	if (pointModel.features.size() <= 0)
		return 0;
	vector<float> pointVector;
	vector<unsigned char> colorVector;
	vector<unsigned int> batchIDVector;
	vector<gisINT64> OIDVector;
	GetPointInfo(pointModel, pointVector, colorVector, batchIDVector, OIDVector);
	return i_PointPNTS2Buffer(pointVector, colorVector, batchIDVector, OIDVector, isDracoCompress, pRecords, pRtcCenter, m_data,"OID");
}

gisLONG Ci_ModelPnts::From(MapGIS::Tile::PointsModel &pointModel, bool isDracoCompress, D_3DOT* pRtcCenter, vector<gisINT64>& batchIDToId)
{
	if (pointModel.features.size() <= 0)
		return 0;
	vector<float> pointVector;
	vector<unsigned char> colorVector;
	vector<unsigned int> batchIDVector;
	vector<gisINT64> OIDVector;
	GetPointInfo(pointModel, pointVector, colorVector, batchIDVector, OIDVector);
	batchIDToId.clear();
	batchIDToId.insert(batchIDToId.begin(), OIDVector.begin(), OIDVector.end());
	return i_PointPNTS2Buffer(pointVector, colorVector, batchIDVector, OIDVector, isDracoCompress, NULL, pRtcCenter, m_data, "");
}

gisLONG Ci_ModelPnts::To(MapGIS::Tile::PointsModel &pointModel, LayerAttribute & records, D_3DOT& rtcCenter)
{
	if (m_data.size() <= 0)
		return  0;
	gisUINT POINTS_LENGTH = 0;
	gisUINT BATCH_LENGTH = 0;

	int c = 0;
	string magic = ReadByteArrayToString(m_data, c, 4); c += 4;
	int version = ReadByteArrayToInt32(m_data.data(), c); c += 4;
	int pntsBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int featureTableJsonBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int featureTableBinBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int batchTableJsonBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;
	int batchTableBinaBytesLength = ReadByteArrayToInt32(m_data, c); c += 4;

	CGByteArray featureTableJsonByteArray;
	CGByteArray featureTableBinByteArray;
	CGByteArray batchTableJsonByteArray;
	CGByteArray batchTableBinByteArray;
	ReadByteArrayToByteArray(m_data, c, featureTableJsonBytesLength, featureTableJsonByteArray);
	c += featureTableJsonBytesLength;

	ReadByteArrayToByteArray(m_data, c, featureTableBinBytesLength, featureTableBinByteArray);
	c += featureTableBinBytesLength;

	ReadByteArrayToByteArray(m_data, c, batchTableJsonBytesLength, batchTableJsonByteArray);
	c += batchTableJsonBytesLength;

	ReadByteArrayToByteArray(m_data, c, batchTableBinaBytesLength, batchTableBinByteArray);
	c += batchTableBinaBytesLength;

	rapidjson::Document doc;
	if (doc.Parse(featureTableJsonByteArray.data(), featureTableJsonByteArray.size()).HasParseError())
		return 0;

	vector<gisUINT> outPointsLength;
	vector<float> outRtcCenter;
	vector<gisUINT> outBatchLength;

	vector<float> outQuantizedVolumeOffset;
	vector<float> outQuantizedVolumeScale;
	vector<gisCHAR>  outConstantRgba;

	GetTableValue(doc, "POINTS_LENGTH", 1, featureTableBinByteArray, outPointsLength);
	if (outPointsLength.size() == 1)
		POINTS_LENGTH = outPointsLength[0];
	if (POINTS_LENGTH <= 0)
		return 0;

	rtcCenter.x = 0;
	rtcCenter.y = 0;
	rtcCenter.z = 0;
	GetTableValue(doc, "RTC_CENTER", 3, featureTableBinByteArray, outRtcCenter);
	if (outRtcCenter.size() == 3)
	{
		rtcCenter.x = outRtcCenter[0];
		rtcCenter.y = outRtcCenter[1];
		rtcCenter.z = outRtcCenter[2];
	}

	GetTableValue(doc, "BATCH_LENGTH", 1, featureTableBinByteArray, outBatchLength);
	if (outBatchLength.size() == 1)
		BATCH_LENGTH = outBatchLength[0];

	GetTableValue(doc, "QUANTIZED_VOLUME_OFFSET", 3, featureTableBinByteArray, outQuantizedVolumeOffset);
	GetTableValue(doc, "QUANTIZED_VOLUME_SCALE", 3, featureTableBinByteArray, outQuantizedVolumeScale);
	GetTableValue(doc, "CONSTANT_RGBA", 4, featureTableBinByteArray, outConstantRgba);

	vector<float> outPosition;
	vector<unsigned char> outColors;
	vector<unsigned int> outBatchIDs;
	char* findIndex = strstr(featureTableJsonByteArray.data(), "3DTILES_draco_point_compression");
	if (findIndex == NULL)
	{
		GetTableValue(doc, "POSITION", 3* POINTS_LENGTH, featureTableBinByteArray, outPosition);
		if (outPosition.size() != 3 * POINTS_LENGTH)
		{
			if (outQuantizedVolumeOffset.size() != 3 || outQuantizedVolumeScale.size() != 3)
				return 0;
			vector<gisUSHORT> outPositionQuantized;
			GetTableValue(doc, "POSITION_QUANTIZED", 3 * POINTS_LENGTH, featureTableBinByteArray, outPositionQuantized);
			if(outPositionQuantized.size() != 3 * POINTS_LENGTH)
				return 0;
			outPosition.resize(3 * POINTS_LENGTH);
			for (int i = 0; i < POINTS_LENGTH; i++) {
				for (int j = 0; j < 3; j++) {
					outPosition[i * 3 + j] = (outPositionQuantized[i * 3 + j] / 65535.0) * outQuantizedVolumeScale[j] + outQuantizedVolumeOffset[j];
				}
			}
		}

		GetTableValue(doc, "RGBA", 4 * POINTS_LENGTH, featureTableBinByteArray, outColors);
		if (outColors.size() != 4 * POINTS_LENGTH)
		{
			outColors.clear();
			GetTableValue(doc, "RGB", 3 * POINTS_LENGTH, featureTableBinByteArray, outColors);
			if (outColors.size() != 3 * POINTS_LENGTH)
			{
				vector<unsigned short> outColor565;
				GetTableValue(doc, "RGB565", POINTS_LENGTH, featureTableBinByteArray, outColor565);
				if (outColor565.size() ==  POINTS_LENGTH)
				{
					outColors.resize(3 * POINTS_LENGTH);

					for (int i = 0; i < POINTS_LENGTH; i++)
					{
						outColors[i * 3] = outColor565[i] & 0xF800;
						outColors[i * 3 + 1] = outColor565[i] & 0x07E0;
						outColors[i * 3 + 2] = outColor565[i] & 0x001F;
					}
				}
			}
		}
		GetTableValue(doc, "BATCH_ID", POINTS_LENGTH, featureTableBinByteArray, outBatchIDs);
	}
	else
	{
		Ci_DracoTool::ParsePointCloudDracoBinData(featureTableBinByteArray.data(), featureTableBinBytesLength, outPosition, outColors, outBatchIDs, POINTS_LENGTH);
	}

	Ci_3DTileBatchTable batchTable;
	batchTable.ReadFrom(batchTableJsonByteArray, batchTableBinByteArray, "OID", BATCH_LENGTH, records);

	vector<unsigned int> oidIndex;
	unordered_map<unsigned int, MapGIS::Tile::PointFeature*> features;

	bool isRAGA = false;
	bool hasColor = false;
	if (outColors.size() == POINTS_LENGTH * 4 || outColors.size() == POINTS_LENGTH * 3)
		hasColor = true;
	if (outColors.size() == POINTS_LENGTH * 4)
		isRAGA = true;

	for (int j = 0; j < POINTS_LENGTH; j++)
	{
		int oid = -1;
		if (outBatchIDs.size() == POINTS_LENGTH)
		{
			int bacthId = outBatchIDs[j];
			oid = bacthId;
			if (bacthId < records.records.size() && records.records[bacthId].GetID() >= 0)
				oid = records.records[bacthId].GetID();
		}
		else
		{
			oid = j;
			if (j < records.records.size() && records.records[j].GetID() >= 0)
				oid = records.records[j].GetID();
		}

		unordered_map<unsigned int, MapGIS::Tile::PointFeature*> ::iterator itr = features.find(oid);
		if (itr == features.end())
		{
			MapGIS::Tile::PointFeature* pFeature = new MapGIS::Tile::PointFeature();
			pFeature->id = oid;
			features.insert(make_pair(oid, pFeature));
			oidIndex.emplace_back(oid);
			itr = features.find(oid);
		}

		if (itr != features.end())
		{
			Color4f color;
			if (hasColor)
			{
				if (isRAGA)
				{
					color.r = (outColors[4 * j]) / 255.0;
					color.g = (outColors[4 * j + 1]) / 255.0;
					color.b = (outColors[4 * j + 2]) / 255.0;
					color.a = (outColors[4 * j + 3]) / 255.0;
				}
				else
				{
					color.r = (outColors[3 * j]) / 255.0;
					color.g = (outColors[3 * j + 1]) / 255.0;
					color.b = (outColors[3 * j + 2]) / 255.0;
				}
			}
			else if (outConstantRgba.size() == 4)
			{
				color.r = (outConstantRgba[0]) / 255.0;
				color.g = (outConstantRgba[1]) / 255.0;
				color.b = (outConstantRgba[2]) / 255.0;
				color.a = (outConstantRgba[3]) / 255.0;
			}

			D_3DOT dot{ outPosition[3 * j],outPosition[3 * j + 1],outPosition[3 * j + 2] };
			bool isExist = false;
			for (vector<ColorPoint>::iterator colorItr = itr->second->colorPoints.begin(); colorItr != itr->second->colorPoints.end(); colorItr++)
			{
				if (colorItr->color == color)
				{
					colorItr->points.Append(dot);
					isExist = true;
					break;
				}
			}
			if(!isExist)
			{
				MapGIS::Tile::ColorPoint point;
				point.points.Append(dot);
				point.color = color;
				itr->second->colorPoints.emplace_back(point);
			}
		}
	}

	for (vector<unsigned int>::iterator itr = oidIndex.begin(); itr != oidIndex.end(); itr++)
	{
		unordered_map<unsigned int, MapGIS::Tile::PointFeature*>::iterator featureItr =   features.find(*itr);
		if(featureItr!= features.end())
			pointModel.features.emplace_back(*featureItr->second);
	}

	for (unordered_map<unsigned int, MapGIS::Tile::PointFeature*>::iterator itr = features.begin(); itr != features.end(); itr++)
	{
		delete itr->second;
	}
	features.clear();
	return 1;
}