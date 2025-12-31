#include "stdafx.h"
#include "ci_dracotool.h"
#include "draco/compression/encode.h"
#include "draco/compression/decode.h"
#include "draco/io/gltf_decoder.h"
#include "draco/io/gltf_encoder.h"
#include "draco/draco_features.h"
#include "draco/scene/scene_utils.h"
#include "draco/point_cloud/point_cloud_builder.h"
#include "draco/tools/draco_transcoder_lib.h"

#include <memory>
using namespace std;

std::unique_ptr<draco::PointCloud> CreateDracoPointCloud(const vector<float> &pointVector, const  vector<unsigned char> &colorVector, const  vector<unsigned int>& batchIDVector, int& posAttId, int& colorAttId, int& batchIdAttId)
{
	draco::PointCloudBuilder pc_builder;

	if (batchIDVector.size() <= 0 || (pointVector.size() / 3) != batchIDVector.size() || (colorVector.size() / 3) != batchIDVector.size())
		return  pc_builder.Finalize(false);

	const int kNumPoints = batchIDVector.size();
	pc_builder.Start(kNumPoints);

	posAttId = pc_builder.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
	colorAttId = pc_builder.AddAttribute(draco::GeometryAttribute::COLOR, 3, draco::DT_UINT8);
	batchIdAttId = pc_builder.AddAttribute(draco::GeometryAttribute::GENERIC, 1, draco::DT_UINT32);

	for (draco::PointIndex i(0); i < kNumPoints; ++i) {
		pc_builder.SetAttributeValueForPoint(posAttId, i, draco::Vector3f(pointVector[i.value() *3+0], pointVector[i.value() * 3 + 1], pointVector[i.value() * 3 + 2]).data());
		pc_builder.SetAttributeValueForPoint(colorAttId, i, vector<unsigned char>({ colorVector[i.value() * 3 + 0], colorVector[i.value() * 3 + 1], colorVector[i.value() * 3 + 2] }).data());
		pc_builder.SetAttributeValueForPoint(batchIdAttId, i, &batchIDVector[i.value()]);
	}
	return pc_builder.Finalize(false);
}

class DracoTransCodeWithBuf
{
public:
	DracoTransCodeWithBuf() {};
	virtual ~DracoTransCodeWithBuf() {};
public:
	static draco::StatusOr<std::unique_ptr<DracoTransCodeWithBuf>> Create(const draco::DracoTranscodingOptions &options);
	static draco::StatusOr<std::unique_ptr<DracoTransCodeWithBuf>> Create(const draco::DracoCompressionOptions &options);
	draco::Status TranscodeWithBuf(const string& in, string& out);

	draco::Status ReversalcodeWithFile(const string& in, string& out);

	draco::Status TranscodeWithBuf(const string& in, CGByteArray& out);
	draco::Status ReversalcodeWithFile(const string& in, CGByteArray& out);

	draco::Status TranscodeWithBuf(const CGByteArray& in, CGByteArray& out);
	draco::Status ReversalcodeWithFile(const CGByteArray& in, CGByteArray& out);
private:
	draco::Status ReadSceneWithBuf(const string& in);
	draco::Status ReadSceneWithBuf(const CGByteArray& in);
	draco::Status WriteSceneWithBuf(string& out);
	draco::Status WriteSceneWithBuf(CGByteArray& out);

	draco::Status CompressSceneWithBuf();
private:
	draco::GltfEncoder i_gltf_encoder_;
	std::unique_ptr<draco::Scene> i_scene_;
	draco::DracoTranscodingOptions i_transcoding_options_;
};

vector<char> Ci_DracoTool::CreatePointCloudDracoBinData(const vector<float> &pointVector, const  vector<unsigned char> &colorVector, const  vector<unsigned int>& batchIDVector, int& posAttId, int& colorAttId, int& batchIdAttId)
{
	draco::Encoder encoder;
	encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14);
	encoder.SetAttributeQuantization(draco::GeometryAttribute::COLOR, 8);
	encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, 0);
	draco::EncoderBuffer buffer;
	auto pointCloud = CreateDracoPointCloud(pointVector, colorVector, batchIDVector, posAttId, colorAttId, batchIdAttId);
	const draco::Status status = encoder.EncodePointCloudToBuffer(*pointCloud, &buffer);
	return std::move(*buffer.buffer());
}

draco::StatusOr<std::unique_ptr<DracoTransCodeWithBuf>> DracoTransCodeWithBuf::Create(const draco::DracoTranscodingOptions &options)
{
	DRACO_RETURN_IF_ERROR(options.geometry.Check());
	std::unique_ptr<DracoTransCodeWithBuf> dt(new DracoTransCodeWithBuf());
	dt->i_transcoding_options_ = options;
	return std::move(dt);
}

draco::StatusOr<std::unique_ptr<DracoTransCodeWithBuf>> DracoTransCodeWithBuf::Create(const draco::DracoCompressionOptions &options)
{
	draco::DracoTranscodingOptions new_options;
	new_options.geometry = options;
	return Create(new_options);
}

draco::Status DracoTransCodeWithBuf::TranscodeWithBuf(const string& in, string& out)
{
	DRACO_RETURN_IF_ERROR(ReadSceneWithBuf(in));
	DRACO_RETURN_IF_ERROR(CompressSceneWithBuf());
	DRACO_RETURN_IF_ERROR(WriteSceneWithBuf(out));
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::TranscodeWithBuf(const string& in, CGByteArray& out)
{
	DRACO_RETURN_IF_ERROR(ReadSceneWithBuf(in));
	DRACO_RETURN_IF_ERROR(CompressSceneWithBuf());
	DRACO_RETURN_IF_ERROR(WriteSceneWithBuf(out));
	return draco::OkStatus();
}
draco::Status DracoTransCodeWithBuf::ReversalcodeWithFile(const string& in, CGByteArray& out)
{
	DRACO_RETURN_IF_ERROR(ReadSceneWithBuf(in));
	DRACO_RETURN_IF_ERROR(CompressSceneWithBuf());
	DRACO_RETURN_IF_ERROR(WriteSceneWithBuf(out));
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::TranscodeWithBuf(const CGByteArray& in, CGByteArray& out)
{
	DRACO_RETURN_IF_ERROR(ReadSceneWithBuf(in));
	DRACO_RETURN_IF_ERROR(CompressSceneWithBuf());
	DRACO_RETURN_IF_ERROR(WriteSceneWithBuf(out));
	return draco::OkStatus();
}
draco::Status DracoTransCodeWithBuf::ReversalcodeWithFile(const CGByteArray& in, CGByteArray& out)
{
	DRACO_RETURN_IF_ERROR(ReadSceneWithBuf(in));
	DRACO_RETURN_IF_ERROR(CompressSceneWithBuf());
	DRACO_RETURN_IF_ERROR(WriteSceneWithBuf(out));
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::ReversalcodeWithFile(const string& in, string& out)
{
	DRACO_RETURN_IF_ERROR(ReadSceneWithBuf(in));
	draco::SceneUtils::SetDracoCompressionOptions(nullptr, i_scene_.get());
	DRACO_RETURN_IF_ERROR(WriteSceneWithBuf(out));
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::ReadSceneWithBuf(const string& in)
{
	draco::DecoderBuffer* buffer = new draco::DecoderBuffer();
	if (buffer)
	{
		buffer->Init(&in[0], in.size());
		draco::GltfDecoder decoder;
		DRACO_ASSIGN_OR_RETURN(i_scene_, decoder.DecodeFromBufferToScene(buffer));
		delete buffer;
	}
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::ReadSceneWithBuf(const CGByteArray& in)
{
	draco::DecoderBuffer* buffer = new draco::DecoderBuffer();
	if (buffer)
	{
		buffer->Init(in.data(), in.size());
		draco::GltfDecoder decoder;
		DRACO_ASSIGN_OR_RETURN(i_scene_, decoder.DecodeFromBufferToScene(buffer));
		delete buffer;
	}
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::WriteSceneWithBuf(string& out)
{
	draco::EncoderBuffer* out_buffer = new draco::EncoderBuffer();
	if (out_buffer)
	{
		DRACO_RETURN_IF_ERROR(i_gltf_encoder_.EncodeToBuffer<draco::Scene>(*i_scene_, out_buffer));
		out = string(out_buffer->buffer()->data(), out_buffer->buffer()->size());
		delete out_buffer;
	}
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::WriteSceneWithBuf(CGByteArray& out)
{
	draco::EncoderBuffer* out_buffer = new draco::EncoderBuffer();
	out.clear();
	if (out_buffer)
	{
		DRACO_RETURN_IF_ERROR(i_gltf_encoder_.EncodeToBuffer<draco::Scene>(*i_scene_, out_buffer));
		out.append(out_buffer->buffer()->data(), out_buffer->buffer()->size());
		delete out_buffer;
	}
	return draco::OkStatus();
}

draco::Status DracoTransCodeWithBuf::CompressSceneWithBuf()
{
	//开源库通用的参数设置
	i_transcoding_options_.geometry.compression_level = 5;
	i_transcoding_options_.geometry.quantization_position.SetQuantizationBits(14);
	i_transcoding_options_.geometry.quantization_bits_tex_coord = 12;
	i_transcoding_options_.geometry.quantization_bits_normal = 10;
	i_transcoding_options_.geometry.quantization_bits_color = 8;
	i_transcoding_options_.geometry.quantization_bits_generic = 8;

	draco::SceneUtils::SetDracoCompressionOptions(&i_transcoding_options_.geometry, i_scene_.get());
	return draco::OkStatus();
}

draco::Status i_CompressByDraco(const string& in, string& out)
{
	draco::DracoTranscodingOptions transcode_options;
	DRACO_ASSIGN_OR_RETURN(std::unique_ptr<DracoTransCodeWithBuf> dt, DracoTransCodeWithBuf::Create(transcode_options));
	DRACO_RETURN_IF_ERROR(dt->TranscodeWithBuf(in, out));
	return draco::OkStatus();
}

draco::Status i_CompressByDraco(const string& in, CGByteArray& out)
{
	draco::DracoTranscodingOptions transcode_options;
	DRACO_ASSIGN_OR_RETURN(std::unique_ptr<DracoTransCodeWithBuf> dt, DracoTransCodeWithBuf::Create(transcode_options));
	DRACO_RETURN_IF_ERROR(dt->TranscodeWithBuf(in, out));
	return draco::OkStatus();
}

draco::Status i_CompressByDraco(const CGByteArray& in, CGByteArray& out)
{
	draco::DracoTranscodingOptions transcode_options;
	DRACO_ASSIGN_OR_RETURN(std::unique_ptr<DracoTransCodeWithBuf> dt, DracoTransCodeWithBuf::Create(transcode_options));
	DRACO_RETURN_IF_ERROR(dt->TranscodeWithBuf(in, out));
	return draco::OkStatus();
}

draco::Status i_DecompressByDraco(const string& in, string& out)
{
	draco::DracoTranscodingOptions transcode_options;
	DRACO_ASSIGN_OR_RETURN(std::unique_ptr<DracoTransCodeWithBuf> dt, DracoTransCodeWithBuf::Create(transcode_options));
	DRACO_RETURN_IF_ERROR(dt->ReversalcodeWithFile(in, out));
	return draco::OkStatus();
}

draco::Status i_DecompressByDraco(const string& in, CGByteArray& out)
{
	draco::DracoTranscodingOptions transcode_options;
	DRACO_ASSIGN_OR_RETURN(std::unique_ptr<DracoTransCodeWithBuf> dt, DracoTransCodeWithBuf::Create(transcode_options));
	DRACO_RETURN_IF_ERROR(dt->ReversalcodeWithFile(in, out));
	return draco::OkStatus();
}

draco::Status i_DecompressByDraco(const CGByteArray& in, CGByteArray& out)
{
	draco::DracoTranscodingOptions transcode_options;
	DRACO_ASSIGN_OR_RETURN(std::unique_ptr<DracoTransCodeWithBuf> dt, DracoTransCodeWithBuf::Create(transcode_options));
	DRACO_RETURN_IF_ERROR(dt->ReversalcodeWithFile(in, out));
	return draco::OkStatus();
}

bool Ci_DracoTool::GeometryCompress(const string& in, string& out)
{
	try
	{
		draco::Status  rtn = i_CompressByDraco(in, out);
		return rtn.ok();
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Ci_DracoTool::GeometryDecompression(const string& in, string& out)
{
	try
	{
		draco::Status  rtn = i_DecompressByDraco(in, out);
		return rtn.ok();
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool Ci_DracoTool::GeometryCompress(const string& in, CGByteArray& out)
{
	try
	{
		draco::Status  rtn = i_CompressByDraco(in, out);
		return rtn.ok();
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Ci_DracoTool::GeometryDecompression(const string& in, CGByteArray& out)
{
	try
	{
		draco::Status  rtn = i_DecompressByDraco(in, out);
		return rtn.ok();
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool Ci_DracoTool::GeometryCompress(const CGByteArray& in, CGByteArray& out)
{
	try
	{
		draco::Status  rtn = i_CompressByDraco(in, out);
		return rtn.ok();
	}
	catch (const std::exception&)
	{
		return false;
	}
}
bool Ci_DracoTool::GeometryDecompression(const CGByteArray& in, CGByteArray& out)
{
	try
	{
		draco::Status  rtn = i_DecompressByDraco(in, out);
		return rtn.ok();
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool Ci_DracoTool::ParsePointCloudDracoBinData(const char*buf, std::size_t bufSize, vector<float> & positions, vector<unsigned char>  & colors, vector<unsigned int> &batchIDs,  unsigned int pointsLength)
{
	draco::Decoder decoder;
	draco::DecoderBuffer buffer;
	buffer.Init(buf, bufSize);
	draco::StatusOr<std::unique_ptr<draco::PointCloud>> dracoResult = decoder.DecodePointCloudFromBuffer(&buffer);
	if (!dracoResult.ok())
		return false;
	const std::unique_ptr<draco::PointCloud>& pPointCloud = dracoResult.value();
	int num =  pPointCloud->num_attributes();
	for (int i = 0; i < num; i++)
	{
		draco::PointAttribute* pAttribute = pPointCloud->attribute(i);

		if (pAttribute == NULL)
			continue;
		if (pAttribute->attribute_type() == draco::GeometryAttribute::Type::POSITION)
		{
			if (pAttribute->data_type() != draco::DT_FLOAT32 || pAttribute->num_components() != 3)
				return false;
			draco::DataBuffer* decodedBuffer = pAttribute->buffer();
			int64_t decodedByteOffset = pAttribute->byte_offset();
			unsigned char* pos = decodedBuffer->data() + decodedByteOffset;
			positions.resize(pointsLength *3);
			memcpy(&positions[0], pos, pointsLength * 3 * 4);
		}
		else if (pAttribute->attribute_type() == draco::GeometryAttribute::Type::COLOR)
		{
			if (pAttribute->data_type() != draco::DT_UINT8)
				continue;
			draco::DataBuffer* decodedBuffer = pAttribute->buffer();
			int64_t decodedByteOffset = pAttribute->byte_offset();
			unsigned char* pos = decodedBuffer->data() + decodedByteOffset;

			if (pAttribute->num_components() == 3)
			{
				colors.resize(pointsLength * 3);
				memcpy(&colors[0], pos, pointsLength * 3);
			}
			else if (pAttribute->num_components() == 4)
			{
				colors.resize(pointsLength * 4);
				memcpy(&colors[0], pos, pointsLength * 4);
			}
		}
		else if (pAttribute->attribute_type() == draco::GeometryAttribute::Type::GENERIC)
		{
			if (pAttribute->data_type() != draco::DT_UINT32  || pAttribute->num_components() != 1)
				continue;
			draco::DataBuffer* decodedBuffer = pAttribute->buffer();
			int64_t decodedByteOffset = pAttribute->byte_offset();
			unsigned char* pos = decodedBuffer->data() + decodedByteOffset;

			batchIDs.resize(pointsLength);
			memcpy(&batchIDs[0], pos, pointsLength * 4);
		}
	}
	return true;
}