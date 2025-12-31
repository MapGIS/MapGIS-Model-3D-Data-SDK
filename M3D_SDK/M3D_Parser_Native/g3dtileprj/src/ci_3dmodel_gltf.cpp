#include "stdafx.h"
#include "ci_3dmodel.h"
#include "ci_assist.h"
#include "ci_gltfsdktool.h"
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/GLTF.h>
#include "ci_dracotool.h"
#include "ci_meshopttool.h"
#include "ci_gltfextensions.h"
#include "ci_gaussian_spz_read_write.h"


using namespace MapGIS::Tile;

#define RGBA_M(r,g,b,a) ((unsigned long )((((unsigned char)(r)|((unsigned char)((unsigned char)(g))<<8))|(((gisULONG)(unsigned char)(b))<<16))|(((gisULONG)(unsigned char)(a))<<24)))

static void Matrix4Unit(MapGIS::Tile::Matrix4D& matrix)
{
	matrix[0][0] = 1; matrix[0][1] = 0; matrix[0][2] = 0; matrix[0][3] = 0;
	matrix[1][0] = 0; matrix[1][1] = 1; matrix[1][2] = 0; matrix[1][3] = 0;
	matrix[2][0] = 0; matrix[2][1] = 0; matrix[2][2] = 1; matrix[2][3] = 0;
	matrix[3][0] = 0; matrix[3][1] = 0; matrix[3][2] = 0; matrix[3][3] = 1;
}

#ifndef _WIN32
	#define TYPENAME typename //error: expected ‘;’ before ‘it’
#else
	#define TYPENAME
#endif

template<typename T>
std::vector<T> ReadBinaryData(const GLTFResourceReader& resourceReader, const Document& gltfDocument, const Accessor& accessor)
{
	bool isValid;
	switch (accessor.componentType)
	{
	case COMPONENT_BYTE:
		isValid = std::is_same<T, int8_t>::value;
		break;
	case COMPONENT_UNSIGNED_BYTE:
		isValid = std::is_same<T, uint8_t>::value;
		break;
	case COMPONENT_SHORT:
		isValid = std::is_same<T, int16_t>::value;
		break;
	case COMPONENT_UNSIGNED_SHORT:
		isValid = std::is_same<T, uint16_t>::value;
		break;
	case COMPONENT_UNSIGNED_INT:
		isValid = std::is_same<T, uint32_t>::value;
		break;
	case COMPONENT_FLOAT:
		isValid = std::is_same<T, float>::value;
		break;
	default:
		throw GLTFException("Unsupported accessor ComponentType");
	}
	if (!isValid)
	{
		std::vector<T> rtn;
		switch (accessor.componentType)
		{
		case COMPONENT_BYTE:
		{
			std::vector<int8_t> temp = resourceReader.ReadBinaryData<int8_t>(gltfDocument, accessor);
			rtn.reserve(temp.size());
			for (int i = 0; i < temp.size(); i++)
			{
				rtn.emplace_back((T)temp[i]);
			}
			break;
		}

		case COMPONENT_UNSIGNED_BYTE:
		{
			std::vector<uint8_t> temp = resourceReader.ReadBinaryData<uint8_t>(gltfDocument, accessor);
			rtn.reserve(temp.size());
			for (int i = 0; i < temp.size(); i++)
			{
				rtn.emplace_back((T)temp[i]);
			}
			break;
		}
		case COMPONENT_SHORT:
		{
			std::vector<int16_t> temp = resourceReader.ReadBinaryData<int16_t>(gltfDocument, accessor);
			rtn.reserve(temp.size());
			for (int i = 0; i < temp.size(); i++)
			{
				rtn.emplace_back((T)temp[i]);
			}
			break;
		}
		case COMPONENT_UNSIGNED_SHORT:
		{
			std::vector<uint16_t> temp = resourceReader.ReadBinaryData<uint16_t>(gltfDocument, accessor);
			rtn.reserve(temp.size());
			for (int i = 0; i < temp.size(); i++)
			{
				rtn.emplace_back((T)temp[i]);
			}
			break;
		}
		case COMPONENT_UNSIGNED_INT:
		{
			std::vector<uint32_t> temp = resourceReader.ReadBinaryData<uint32_t>(gltfDocument, accessor);
			rtn.reserve(temp.size());
			for (int i = 0; i < temp.size(); i++)
			{
				rtn.emplace_back((T)temp[i]);
			}
			break;
		}
		case COMPONENT_FLOAT:
		{
			std::vector<float> temp = resourceReader.ReadBinaryData<float>(gltfDocument, accessor);
			rtn.reserve(temp.size());
			for (int i = 0; i < temp.size(); i++)
			{
				rtn.emplace_back((T)temp[i]);
			}
			break;
		}
		}
		return rtn;
	}
	else
	{
		return  resourceReader.ReadBinaryData<T>(gltfDocument, accessor);
	}
}

class CacheStorageReader : public IStreamReader
{
public:
	CacheStorageReader(G3DCacheStorage* pStorage) : m_pStorage(pStorage)
	{
	}
	std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const override
	{
		/*auto streamPath = std::experimental::filesystem::u8path(filename);
		auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::in);
		if(stream != NULL && stream->get() != NULL)
			return stream;*/
		if (m_pStorage != NULL)
		{
			CGByteArray buffer;
			m_pStorage->GetContent(CGString(filename, CGString::EncodeType::UTF8), buffer);
			auto rtn = std::make_shared<std::istringstream>(string(buffer.data(), buffer.size()));
			if (buffer.size() > 0)
				return rtn;
		}
		throw std::runtime_error("Unable to create a valid input stream for uri: " + filename);
	}

private:
	G3DCacheStorage*        m_pStorage;
};

struct GpuInstancingInfo
{
	std::string translatton;
	std::string rotation;
	std::string scale;
	std::string batchid;
	GpuInstancingInfo() :translatton(""), rotation(""), scale(""), batchid("") {}

	bool isEmpty()  const
	{
		if (translatton.empty() && rotation.empty() && scale.empty() && batchid.empty())
			return true;
		return false;
	}
};

struct NodeTransitionInfo
{
	Matrix4D gltfMatrix;
	GpuInstancingInfo  gpuInstancingInfo;
	NodeTransitionInfo()
	{
		Matrix4Unit(gltfMatrix);
	}

	inline bool operator != (const NodeTransitionInfo& m2) const
	{
		if (gltfMatrix != m2.gltfMatrix || gpuInstancingInfo.translatton != m2.gpuInstancingInfo.translatton
			|| gpuInstancingInfo.rotation != m2.gpuInstancingInfo.rotation
			|| gpuInstancingInfo.scale != m2.gpuInstancingInfo.scale
			|| gpuInstancingInfo.batchid != m2.gpuInstancingInfo.batchid)
			return true;
		return false;
	}
};

struct GltfMeshInfo
{
	const MeshPrimitive *meshPrimitive;
	vector<NodeTransitionInfo>  nodeTransitionInfo;
	GeometryType type;
	GltfMeshInfo()
	{
		meshPrimitive = NULL;
		type = GeometryType::None;
	}
};

Matrix4D  GetMatrix4(MapGIS::Tile::Vector3D& translation, MapGIS::Tile::Vector4D& rotation, MapGIS::Tile::Vector3D& scale)
{
	Matrix4D item;
	double a1 = 1.0 - 2.0 * (rotation.y * rotation.y + rotation.z * rotation.z);
	double a2 = 2.0 * (rotation.x * rotation.y - rotation.z * rotation.w);
	double a3 = 2.0 * (rotation.x * rotation.z + rotation.y * rotation.w);

	double b1 = 2.0 * (rotation.x * rotation.y + rotation.z * rotation.w);
	double b2 = 1.0 - 2.0 * (rotation.x * rotation.x + rotation.z * rotation.z);
	double b3 = 2.0 * (rotation.y * rotation.z - rotation.x * rotation.w);

	double c1 = 2.0 * (rotation.x * rotation.z - rotation.y * rotation.w);
	double c2 = 2.0 * (rotation.y * rotation.z + rotation.x * rotation.w);
	double c3 = 1.0 - 2.0 * (rotation.x * rotation.x + rotation.y * rotation.y);

	item[0][0] = a1 * scale.x;
	item[0][1] = a2 * scale.x;
	item[0][2] = a3 * scale.x;
	item[0][3] = translation.x;

	item[1][0] = b1 * scale.y;
	item[1][1] = b2 * scale.y;
	item[1][2] = b3 * scale.y;
	item[1][3] = translation.y;

	item[2][0] = c1 * scale.z;
	item[2][1] = c2 * scale.z;
	item[2][2] = c3 * scale.z;
	item[2][3] = translation.z;

	item[3][0] = 0.0;
	item[3][1] = 0.0;
	item[3][2] = 0.0;
	item[3][3] = 1.0;
	return item;
}

void DecomposeMatrix4(const Matrix4D  & matrix, MapGIS::Tile::Vector3D& translation, MapGIS::Tile::Vector4D& rotation, MapGIS::Tile::Vector3D& scale)
{
	/* extract translation */
	translation[0] = matrix[0][3];
	translation[1] = matrix[1][3];
	translation[2] = matrix[2][3];

	/* extract the columns of the matrix. */
	Vector3D vCols[3] = {
	Vector3D(matrix[0][0],matrix[1][0],matrix[2][0]),
	Vector3D(matrix[0][1],matrix[1][1],matrix[2][1]),
	Vector3D(matrix[0][2],matrix[1][2],matrix[2][2])
	};

	/* extract the scaling factors */
	scale[0] = vCols[0].Length();
	scale[1] = vCols[1].Length();
	scale[2] = vCols[2].Length();

	/* and the sign of the scaling */
	if (matrix.Determinant() < 0)
	{
		scale[0] = -scale[0];
		scale[1] = -scale[1];
		scale[2] = -scale[2];
	}

	/* and remove all scaling from the matrix */
	if (scale[0]) vCols[0] /= scale[0];
	if (scale[1]) vCols[1] /= scale[1];
	if (scale[2]) vCols[2] /= scale[2];

	double t = vCols[0].x + vCols[1].y + vCols[2].z;
	// large enough
	if (t > 0)
	{
		double s = std::sqrt(1 + t) * static_cast<double>(2.0);
		rotation.x = (vCols[1].z - vCols[2].y) / s;
		rotation.y = (vCols[2].x - vCols[0].z) / s;
		rotation.z = (vCols[0].y - vCols[1].x) / s;
		rotation.w = 0.25 * s;
	} // else we have to check several cases
	else if (vCols[0].x > vCols[1].y && vCols[0].x > vCols[2].z)
	{
		// Column 0:
		double s = std::sqrt(1.0 + vCols[0].x - vCols[1].y - vCols[2].z) * 2.0;
		rotation.x = 0.25 * s;
		rotation.y = (vCols[0].y + vCols[1].x) / s;
		rotation.z = (vCols[2].x + vCols[0].z) / s;
		rotation.w = (vCols[1].z - vCols[2].y) / s;
	}
	else if (vCols[1].y > vCols[2].z)
	{
		// Column 1:
		double s = std::sqrt(1.0 + vCols[1].y - vCols[0].x - vCols[2].z) * 2.0;
		rotation.x = (vCols[0].y + vCols[1].x) / s;
		rotation.y = 0.25 * s;
		rotation.z = (vCols[1].z + vCols[2].y) / s;
		rotation.w = (vCols[2].x - vCols[0].z) / s;
	}
	else
	{
		// Column 2:
		double s = std::sqrt(1.0 + vCols[2].z - vCols[0].x - vCols[1].y) * 2.0;
		rotation.x = (vCols[2].x + vCols[0].z) / s;
		rotation.y = (vCols[1].z + vCols[2].y) / s;
		rotation.z = 0.25 * s;
		rotation.w = (vCols[0].y - vCols[1].x) / s;
	}
	//const double epsilon = std::numeric_limits<double>::epsilon();

	//rotation[1] = std::asin(-vCols[0].z);// D. Angle around oY.

	//double C = std::cos(rotation[1]);

	//if (std::fabs(C) > epsilon)
	//{
	//	// Finding angle around oX.
	//	double tan_x = vCols[2].z / C;// A
	//	double tan_y = vCols[1].z / C;// B

	//	rotation[0] = std::atan2(tan_y, tan_x);
	//	// Finding angle around oZ.
	//	tan_x = vCols[0].x / C;// E
	//	tan_y = vCols[0].y / C;// F
	//	rotation[2] = std::atan2(tan_y, tan_x);
	//}
	//else
	//{// oY is fixed.
	//	rotation[0] = 0;// Set angle around oX to 0. => A == 1, B == 0, C == 0, D == 1.
	//					// And finding angle around oZ.
	//	double tan_x = vCols[1].y;// BDF+AE => E
	//	double tan_y = -vCols[1].x;// BDE-AF => F
	//	rotation[2] = std::atan2(tan_y, tan_x);
	//}
	return;
}

static bool ImageToBufferBuilder(Document& document, const MapGIS::Tile::Image &image, BufferBuilder& bufferBuilder, vector<string>& imageArray, vector<string>& imageMimeTypeArray, G3DCacheStorage* pStorage, bool imageInData)
{
	string imageId = "";
	CGString imageFileName = image.imageFileName.Converted(CGString::EncodeType::UTF8).StdString();
	//glb将纹理都内嵌到数据内部。
	if (image.data.size() > 0)
	{
		string bufferViewIdImage = Ci_GltfSdkTool::AddImageBufferView(bufferBuilder, image.data);
		imageId = Ci_GltfSdkTool::AddImage(document, bufferViewIdImage, image.mimeType.CStr());
	}
	else if (imageInData && !image.imageFileName.IsEmpty() && pStorage != NULL)
	{
		CGByteArray buffer;
		if (pStorage->GetContent(imageFileName, buffer) > 0)
		{
			string mimeType = "";
			size_t index = imageFileName.ReverseFind(".");
			if (index != string::npos)
			{
				CGString tempStr = imageFileName.Mid(index, imageFileName.GetLength() - index);

				if (StrICmp(tempStr.CStr(), ".webp") == 0)
					mimeType = "image/webp";
				else if (StrICmp(tempStr.CStr(), ".ktx2") == 0)
					mimeType = "image/ktx2";
				else if (StrICmp(tempStr.CStr(), ".png") == 0)
					mimeType = "image/png";
				else if (StrICmp(tempStr.CStr(), ".jpg") == 0 || StrICmp(tempStr.CStr(), ".jpeg") == 0)
					mimeType = "image/jpeg";
			}
			if (mimeType.size() > 0)
			{
				vector<unsigned char> tempData;
				tempData.reserve(buffer.size());
				tempData.insert(tempData.begin(), buffer.data(), buffer.data() + buffer.size());
				string bufferViewIdImage = Ci_GltfSdkTool::AddImageBufferView(bufferBuilder, tempData);
				imageId = Ci_GltfSdkTool::AddImage(document, bufferViewIdImage, mimeType);
			}
			else
				imageId = Ci_GltfSdkTool::AddImage(document, imageFileName.StdString());
		}
		else
			imageId = Ci_GltfSdkTool::AddImage(document, imageFileName.StdString());
	}
	else if (!image.imageFileName.IsEmpty())
	{
		imageId = Ci_GltfSdkTool::AddImage(document, imageFileName.StdString());
	}
	imageArray.push_back(imageId);

	if (image.mimeType.IsEmpty() && !image.imageFileName.IsEmpty())
	{
		size_t index = imageFileName.ReverseFind(".");
		if (index != string::npos)
		{
			CGString tempStr = imageFileName.Mid(index, imageFileName.GetLength() - index);

			if (StrICmp(tempStr.CStr(), ".webp") == 0)
				imageMimeTypeArray.push_back("image/webp");
			else if (StrICmp(tempStr.CStr(), ".ktx2") == 0)
				imageMimeTypeArray.push_back("image/ktx2");
			else if (StrICmp(tempStr.CStr(), ".png") == 0)
				imageMimeTypeArray.push_back("image/png");
			else if (StrICmp(tempStr.CStr(), ".jpg") == 0 || StrICmp(tempStr.CStr(), ".jpeg") == 0)
				imageMimeTypeArray.push_back("image/jpeg");
			else
				imageMimeTypeArray.push_back("");
		}
		else
			imageMimeTypeArray.push_back("");
	}
	else
	{
		CGString mimeType = image.mimeType.Converted(CGString::EncodeType::UTF8).StdString();
		imageMimeTypeArray.push_back(mimeType.StdString());
	}

	return true;
}

static bool GltfImageToImage(const Document& document, const GLBResourceReader& resourceReader, const Microsoft::glTF::Image& gltfImage , MapGIS::Tile::Image&image)
{
	if (!gltfImage.uri.empty())
	{
		image.imageFileName = gltfImage.uri.substr(gltfImage.uri.find_last_of('/') + 1, gltfImage.uri.size() - gltfImage.uri.find_last_of('/') - 1);
		image.imageFileName.SetEncodeType(CGString::EncodeType::UTF8);
	}
	else
	{
		image.imageFileName = "";
		image.data = resourceReader.ReadBinaryData(document, gltfImage);
		image.mimeType = gltfImage.mimeType;
	}
	return true;
}

static bool SamplerToBufferBuilder(Document& document, const MapGIS::Tile::Sampler& sampler, BufferBuilder& bufferBuilder, vector<string>& samplerArray)
{
	Microsoft::glTF::Sampler gltfSampler;

	switch (sampler.magFilter)
	{
	case  MapGIS::Tile::MagFilterMode::MagFilterLinear:
		gltfSampler.magFilter = Microsoft::glTF::MagFilterMode::MagFilter_LINEAR;
		break;
	case MapGIS::Tile::MagFilterMode::MagFilterNearest:
		gltfSampler.magFilter = Microsoft::glTF::MagFilterMode::MagFilter_NEAREST;
		break;
	default:
		gltfSampler.magFilter = Microsoft::glTF::MagFilterMode::MagFilter_LINEAR;
		break;
	}

	switch (sampler.minFilter)
	{
	case MapGIS::Tile::MinFilterMode::MinFilterLinear:
		gltfSampler.minFilter = Microsoft::glTF::MinFilterMode::MinFilter_LINEAR;
		break;
	case MapGIS::Tile::MinFilterMode::MinFilterLinearMipmapLinear:
		gltfSampler.minFilter = Microsoft::glTF::MinFilterMode::MinFilter_LINEAR_MIPMAP_LINEAR;
		break;
	case MapGIS::Tile::MinFilterMode::MinFilterLinearMipmapNearest:
		gltfSampler.minFilter = Microsoft::glTF::MinFilterMode::MinFilter_LINEAR_MIPMAP_NEAREST;
		break;
	case MapGIS::Tile::MinFilterMode::MinFilterNearest:
		gltfSampler.minFilter = Microsoft::glTF::MinFilterMode::MinFilter_NEAREST;
		break;
	case MapGIS::Tile::MinFilterMode::MinFilterNearestMipmapLinear:
		gltfSampler.minFilter = Microsoft::glTF::MinFilterMode::MinFilter_NEAREST_MIPMAP_LINEAR;
		break;
	case MapGIS::Tile::MinFilterMode::MinFilterNearestMipmapNearest:
		gltfSampler.minFilter = Microsoft::glTF::MinFilterMode::MinFilter_NEAREST_MIPMAP_NEAREST;
		break;
	default:
		gltfSampler.minFilter = Microsoft::glTF::MinFilterMode::MinFilter_NEAREST_MIPMAP_LINEAR;
		break;
	}

	switch (sampler.wrapS)
	{
	case MapGIS::Tile::WrapMode::WrapClampToEdge:
		gltfSampler.wrapS = Microsoft::glTF::WrapMode::Wrap_CLAMP_TO_EDGE;
		break;
	case MapGIS::Tile::WrapMode::WrapMirroredRepeat:
		gltfSampler.wrapS = Microsoft::glTF::WrapMode::Wrap_MIRRORED_REPEAT;
		break;
	case MapGIS::Tile::WrapMode::WrapRepeat:
		gltfSampler.wrapS = Microsoft::glTF::WrapMode::Wrap_REPEAT;
		break;
	default:
		gltfSampler.wrapS = Microsoft::glTF::WrapMode::Wrap_REPEAT;
		break;
	}

	switch (sampler.wrapT)
	{
	case MapGIS::Tile::WrapMode::WrapClampToEdge:
		gltfSampler.wrapT = Microsoft::glTF::WrapMode::Wrap_CLAMP_TO_EDGE;
		break;
	case MapGIS::Tile::WrapMode::WrapMirroredRepeat:
		gltfSampler.wrapT = Microsoft::glTF::WrapMode::Wrap_MIRRORED_REPEAT;
		break;
	case MapGIS::Tile::WrapMode::WrapRepeat:
		gltfSampler.wrapT = Microsoft::glTF::WrapMode::Wrap_REPEAT;
		break;
	default:
		gltfSampler.wrapT = Microsoft::glTF::WrapMode::Wrap_REPEAT;
		break;
	}
	string samplerId = Ci_GltfSdkTool::AddSampler(document, gltfSampler);

	samplerArray.push_back(samplerId);
	return true;
}

static bool GltfSamplerToSampler(const Microsoft::glTF::Sampler& gltfSampler, MapGIS::Tile::Sampler &sampler)
{
	switch (gltfSampler.magFilter.Get())
	{
	case  Microsoft::glTF::MagFilterMode::MagFilter_LINEAR:
		sampler.magFilter = MapGIS::Tile::MagFilterMode::MagFilterLinear;
		break;
	case Microsoft::glTF::MagFilterMode::MagFilter_NEAREST:
		sampler.magFilter = MapGIS::Tile::MagFilterMode::MagFilterNearest;
		break;
	default:
		sampler.magFilter = MapGIS::Tile::MagFilterMode::MagFilterLinear;
		break;
	}

	switch (gltfSampler.minFilter.Get())
	{
	case Microsoft::glTF::MinFilterMode::MinFilter_LINEAR:
		sampler.minFilter = MapGIS::Tile::MinFilterMode::MinFilterLinear;
		break;
	case Microsoft::glTF::MinFilterMode::MinFilter_LINEAR_MIPMAP_LINEAR:
		sampler.minFilter = MapGIS::Tile::MinFilterMode::MinFilterLinearMipmapLinear;
		break;
	case Microsoft::glTF::MinFilterMode::MinFilter_LINEAR_MIPMAP_NEAREST:
		sampler.minFilter = MapGIS::Tile::MinFilterMode::MinFilterLinearMipmapNearest;
		break;
	case Microsoft::glTF::MinFilterMode::MinFilter_NEAREST:
		sampler.minFilter = MapGIS::Tile::MinFilterMode::MinFilterNearest;
		break;
	case Microsoft::glTF::MinFilterMode::MinFilter_NEAREST_MIPMAP_LINEAR:
		sampler.minFilter = MapGIS::Tile::MinFilterMode::MinFilterNearestMipmapLinear;
		break;
	case Microsoft::glTF::MinFilterMode::MinFilter_NEAREST_MIPMAP_NEAREST:
		sampler.minFilter = MapGIS::Tile::MinFilterMode::MinFilterNearestMipmapNearest;
		break;
	default:
		sampler.minFilter = MapGIS::Tile::MinFilterMode::MinFilterNearestMipmapLinear;
		break;
	}

	switch (gltfSampler.wrapS)
	{
	case Microsoft::glTF::WrapMode::Wrap_CLAMP_TO_EDGE:
		sampler.wrapS = MapGIS::Tile::WrapMode::WrapClampToEdge;
		break;
	case Microsoft::glTF::WrapMode::Wrap_MIRRORED_REPEAT:
		sampler.wrapS = MapGIS::Tile::WrapMode::WrapMirroredRepeat;
		break;
	case Microsoft::glTF::WrapMode::Wrap_REPEAT:
		sampler.wrapS = MapGIS::Tile::WrapMode::WrapRepeat;
		break;
	default:
		sampler.wrapS = MapGIS::Tile::WrapMode::WrapRepeat;
		break;
	}

	switch (gltfSampler.wrapT)
	{
	case Microsoft::glTF::WrapMode::Wrap_CLAMP_TO_EDGE:
		sampler.wrapT = MapGIS::Tile::WrapMode::WrapClampToEdge;
		break;
	case Microsoft::glTF::WrapMode::Wrap_MIRRORED_REPEAT:
		sampler.wrapT = MapGIS::Tile::WrapMode::WrapMirroredRepeat;
		break;
	case Microsoft::glTF::WrapMode::Wrap_REPEAT:
		sampler.wrapT = MapGIS::Tile::WrapMode::WrapRepeat;
		break;
	default:
		sampler.wrapT = MapGIS::Tile::WrapMode::WrapRepeat;
		break;
	}
	return true;
}

static bool TextureToBufferBuilder(Document& document, const MapGIS::Tile::Texture& texture, const vector<string>& imageArray, const vector<string>& imageMimeTypeArray, const vector<string>& samplerArray, BufferBuilder& bufferBuilder, vector<string> &textureArray)
{
	string gltfImageId = "";
	string gltfSamplerId = "";
	string gltfImageMimeType = "";

	if (texture.imageIndex > -1)
	{
		gltfImageId = imageArray[texture.imageIndex];
		gltfImageMimeType = imageMimeTypeArray[texture.imageIndex];
	}

	if (texture.samplerIndex > -1)
	{
		gltfSamplerId = samplerArray[texture.samplerIndex];
	}

	string textureId = Ci_GltfSdkTool::AddTexture(document, gltfImageId, gltfSamplerId, gltfImageMimeType);

	textureArray.push_back(textureId);
	return true;
}

static bool GltfTextureToTexture(const Document& document, const Microsoft::glTF::Texture gltfTexture,MapGIS::Tile::Texture& texture)
{
	texture.samplerIndex = Ci_GltfSdkTool::FindSamplerIndex(document, gltfTexture.samplerId);
	texture.imageIndex = Ci_GltfSdkTool::FindImageIndex(document, gltfTexture.imageId);
	return true;
}

static bool MaterialToBufferBuilder(Document& document, const MapGIS::Tile::Material& material, const vector<string>& textureArray, BufferBuilder& bufferBuilder, vector<string> &materialArray)
{
	Microsoft::glTF::Material gltfMaterial;

	gltfMaterial.metallicRoughness.baseColorFactor.r = material.metallicRoughness.baseColorFactor.r;
	gltfMaterial.metallicRoughness.baseColorFactor.g = material.metallicRoughness.baseColorFactor.g;
	gltfMaterial.metallicRoughness.baseColorFactor.b = material.metallicRoughness.baseColorFactor.b;
	gltfMaterial.metallicRoughness.baseColorFactor.a = material.metallicRoughness.baseColorFactor.a;

	if (material.metallicRoughness.baseColorTexture.textureIndex > -1)
	{
		gltfMaterial.metallicRoughness.baseColorTexture.textureId = textureArray[material.metallicRoughness.baseColorTexture.textureIndex];
		gltfMaterial.metallicRoughness.baseColorTexture.texCoord = material.metallicRoughness.baseColorTexture.texCoord;
	}

	gltfMaterial.metallicRoughness.metallicFactor = material.metallicRoughness.metallicFactor;
	gltfMaterial.metallicRoughness.roughnessFactor = material.metallicRoughness.roughnessFactor;

	if (material.metallicRoughness.metallicRoughnessTexture.textureIndex > -1)
	{
		gltfMaterial.metallicRoughness.metallicRoughnessTexture.textureId = textureArray[material.metallicRoughness.metallicRoughnessTexture.textureIndex];
		gltfMaterial.metallicRoughness.metallicRoughnessTexture.texCoord = material.metallicRoughness.metallicRoughnessTexture.texCoord;
	}

	if (material.normalTexture.textureIndex > -1)
	{
		gltfMaterial.normalTexture.textureId = textureArray[material.normalTexture.textureIndex];
		gltfMaterial.normalTexture.texCoord = material.normalTexture.texCoord;
		gltfMaterial.normalTexture.scale = material.normalTexture.scale;
	}

	if (material.occlusionTexture.textureIndex > -1)
	{
		gltfMaterial.occlusionTexture.textureId = textureArray[material.occlusionTexture.textureIndex];
		gltfMaterial.occlusionTexture.texCoord = material.occlusionTexture.texCoord;
		gltfMaterial.occlusionTexture.strength = material.occlusionTexture.strength;
	}

	if (material.emissiveTexture.textureIndex > -1)
	{
		gltfMaterial.emissiveTexture.textureId = textureArray[material.emissiveTexture.textureIndex];
		gltfMaterial.emissiveTexture.texCoord = material.emissiveTexture.texCoord;
	}

	gltfMaterial.emissiveFactor.r = material.emissiveFactor.r;
	gltfMaterial.emissiveFactor.g = material.emissiveFactor.g;
	gltfMaterial.emissiveFactor.b = material.emissiveFactor.b;

	switch (material.alphaMode)
	{
	case MapGIS::Tile::AlphaMode::AlphaBlend:
		gltfMaterial.alphaMode = Microsoft::glTF::AlphaMode::ALPHA_BLEND;
		break;
	case MapGIS::Tile::AlphaMode::AlphaMask:
		gltfMaterial.alphaMode = Microsoft::glTF::AlphaMode::ALPHA_MASK;
		break;
	case MapGIS::Tile::AlphaMode::AlphaOpaque:
		gltfMaterial.alphaMode = Microsoft::glTF::AlphaMode::ALPHA_OPAQUE;
		break;
	case MapGIS::Tile::AlphaMode::AlphaUnknown:
		gltfMaterial.alphaMode = Microsoft::glTF::AlphaMode::ALPHA_UNKNOWN;
		break;
	default:
		gltfMaterial.alphaMode = Microsoft::glTF::AlphaMode::ALPHA_BLEND;
		break;
	}

	gltfMaterial.alphaCutoff = material.alphaCutoff;
	gltfMaterial.doubleSided = material.doubleSided;

	string materialId = Ci_GltfSdkTool::AddMaterial(document, gltfMaterial);

	materialArray.push_back(materialId);

	return true;
}

static bool GltfMaterialToMaterial(const Document& document, const Microsoft::glTF::Material gltfMaterial, MapGIS::Tile::Material& material)
{
	material.metallicRoughness.baseColorFactor.r = gltfMaterial.metallicRoughness.baseColorFactor.r;
	material.metallicRoughness.baseColorFactor.g = gltfMaterial.metallicRoughness.baseColorFactor.g;
	material.metallicRoughness.baseColorFactor.b = gltfMaterial.metallicRoughness.baseColorFactor.b;
	material.metallicRoughness.baseColorFactor.a = gltfMaterial.metallicRoughness.baseColorFactor.a;
	material.metallicRoughness.baseColorTexture.texCoord = gltfMaterial.metallicRoughness.baseColorTexture.texCoord;
	material.metallicRoughness.baseColorTexture.textureIndex = Ci_GltfSdkTool::FindTextureIndex(document, gltfMaterial.metallicRoughness.baseColorTexture.textureId);
	material.metallicRoughness.metallicFactor = gltfMaterial.metallicRoughness.metallicFactor;
	material.metallicRoughness.roughnessFactor = gltfMaterial.metallicRoughness.roughnessFactor;
	material.metallicRoughness.metallicRoughnessTexture.texCoord = gltfMaterial.metallicRoughness.metallicRoughnessTexture.texCoord;
	material.metallicRoughness.metallicRoughnessTexture.textureIndex = Ci_GltfSdkTool::FindTextureIndex(document, gltfMaterial.metallicRoughness.metallicRoughnessTexture.textureId);

	material.normalTexture.scale = gltfMaterial.normalTexture.scale;
	material.normalTexture.texCoord = gltfMaterial.normalTexture.texCoord;
	material.normalTexture.textureIndex = Ci_GltfSdkTool::FindTextureIndex(document, gltfMaterial.normalTexture.textureId);

	material.occlusionTexture.strength = gltfMaterial.occlusionTexture.strength;
	material.occlusionTexture.texCoord = gltfMaterial.occlusionTexture.texCoord;
	material.occlusionTexture.textureIndex = Ci_GltfSdkTool::FindTextureIndex(document, gltfMaterial.occlusionTexture.textureId);

	material.emissiveTexture.texCoord = gltfMaterial.emissiveTexture.texCoord;
	material.emissiveTexture.textureIndex = Ci_GltfSdkTool::FindTextureIndex(document, gltfMaterial.emissiveTexture.textureId);
	material.emissiveFactor.r = gltfMaterial.emissiveFactor.r;
	material.emissiveFactor.g = gltfMaterial.emissiveFactor.g;
	material.emissiveFactor.b = gltfMaterial.emissiveFactor.b;

	switch (gltfMaterial.alphaMode)
	{
	case Microsoft::glTF::AlphaMode::ALPHA_BLEND:
		material.alphaMode = MapGIS::Tile::AlphaMode::AlphaBlend;
		break;
	case Microsoft::glTF::AlphaMode::ALPHA_MASK:
		material.alphaMode = MapGIS::Tile::AlphaMode::AlphaMask;
		break;
	case Microsoft::glTF::AlphaMode::ALPHA_OPAQUE:
		material.alphaMode = MapGIS::Tile::AlphaMode::AlphaOpaque;
		break;
	case Microsoft::glTF::AlphaMode::ALPHA_UNKNOWN:
		material.alphaMode = MapGIS::Tile::AlphaMode::AlphaUnknown;
		break;
	default:
		material.alphaMode = MapGIS::Tile::AlphaMode::AlphaBlend;
		break;
	}

	material.alphaCutoff = gltfMaterial.alphaCutoff;
	material.doubleSided = gltfMaterial.doubleSided;
	return true;
}

template <typename T>
void MakeIds(gisLONG pointNum, const gisINT64 &inID, const vector<gisINT64> &inIDs, size_t &idIndex,  bool resetID, vector<T> &outIDs, map<gisINT64, gisINT64>& resetIDMap, unsigned int& maxID)
{
	outIDs.clear();

	if (inID < 0 && inIDs.empty())
		return;

	if (!resetID)
	{
		if (inIDs.size() > 0 || inID >= 0)
		{
			if (!inIDs.empty())
			{
				if (idIndex + pointNum <= inIDs.size())
				{
					outIDs.reserve(pointNum);
					for (vector<gisINT64>::const_iterator itr = inIDs.begin() + idIndex; itr < inIDs.begin() + idIndex + pointNum; itr++)
					{
						outIDs.emplace_back((T)*itr);
					}
					idIndex += pointNum;
				}
				else
				{
					outIDs.resize(pointNum);
					fill(outIDs.begin(), outIDs.begin() + pointNum, (T)0);
				}
			}
			else
			{
				outIDs.resize(pointNum);
				fill(outIDs.begin(), outIDs.begin() + pointNum, (T)inID);
			}
		}
	}
	else
	{
		T tempId;
		vector<T> tempIds;

		if (!inIDs.empty())
		{
			tempIds.resize(inIDs.size(), 0);
			set<gisINT64> setBatchIds(inIDs.begin(), inIDs.end());
			for (set<gisINT64>::iterator itr = setBatchIds.begin(); itr != setBatchIds.end(); itr++)
			{
				gisINT64 tempOid = *itr;
				gisINT64 tempBatchId = 0;
				if (tempOid >= 0)
				{
					map<gisINT64, gisINT64>::iterator batchidItr = resetIDMap.find(tempOid);
					if (batchidItr == resetIDMap.end())
					{
						resetIDMap.insert(make_pair(tempOid, maxID));
						tempBatchId = maxID;
						maxID++;
					}
					else
					{
						tempBatchId = batchidItr->second;
					}
					for (int i = 0; i < inIDs.size(); i++)
					{
						if (inIDs[i] == tempOid)
							tempIds[i] = (T)tempBatchId;
					}
				}
			}
		}
		else
		{
			map<gisINT64, gisINT64>::iterator itr = resetIDMap.find(inID);
			if (itr == resetIDMap.end())
			{
				resetIDMap.insert(make_pair(inID, maxID));
				tempId = maxID;
				maxID++;
			}
			else
			{
				tempId = itr->second;
			}
		}

		outIDs.resize(pointNum);
		if (!tempIds.empty())
		{
			if (idIndex + pointNum <= tempIds.size())
			{
				copy(tempIds.begin() + idIndex, tempIds.begin() + idIndex + pointNum, outIDs.begin());
				idIndex += pointNum;
			}
			else
			{
				fill(outIDs.begin(), outIDs.begin() + pointNum, (T)0);
			}
		}
		else
		{
			fill(outIDs.begin(), outIDs.begin() + pointNum, tempId);
		}
	}
}

static bool BatchIDToBufferBuilder(gisLONG pointNum, const gisINT64 &batchID, const vector<gisINT64> &batchIDs, size_t &idIndex, BufferBuilder& bufferBuilder, vector<string>& accessorIdBatchIDs, bool resetID, map<gisINT64, gisINT64>& resetIDMap, unsigned int& maxID)
{
	if (batchID < 0 && batchIDs.empty())
	{
		accessorIdBatchIDs.push_back("");
		return true;
	}

	bool isUshort = true;
	if (!batchIDs.empty())
	{
		for (auto & value : batchIDs)
		{
			if (value > 65535)
			{
				isUshort = false;
				break;
			}
		}
	}
	else if (batchID > 65535)
		isUshort = false;

	if (isUshort)
	{
		vector<unsigned short> vectorIDs;
		MakeIds(pointNum, batchID, batchIDs, idIndex, resetID, vectorIDs, resetIDMap, maxID);
		if (vectorIDs.size() > 0)
		{
			accessorIdBatchIDs.push_back(Ci_GltfSdkTool::AddBatchIDsAccessor(bufferBuilder, vectorIDs));
		}
		else
		{
			accessorIdBatchIDs.push_back("");
		}
	}
	else
	{
		vector<float> vectorIDs;
		MakeIds(pointNum, batchID, batchIDs, idIndex, resetID, vectorIDs, resetIDMap, maxID);
		if (vectorIDs.size() > 0)
		{
			accessorIdBatchIDs.push_back(Ci_GltfSdkTool::AddBatchIDsAccessor(bufferBuilder, vectorIDs));
		}
		else
		{
			accessorIdBatchIDs.push_back("");
		}
	}
	return true;
}

static bool OidToBufferBuilder(gisLONG pointNum, const gisINT64 &oid, const vector<gisINT64> &OIDs, size_t &oidIndex, BufferBuilder& bufferBuilder, vector<string>& accessorIdOIDs, bool resetID, map<gisINT64, gisINT64>& resetIDMap, unsigned int& maxID)
{
	if (oid < 0 && OIDs.empty())
	{
		accessorIdOIDs.push_back("");
		return true;
	}
	vector<unsigned int> vectorIDs;
	MakeIds(pointNum, oid, OIDs, oidIndex, resetID, vectorIDs, resetIDMap, maxID);
	if (vectorIDs.size() > 0)
	{
		accessorIdOIDs.push_back(Ci_GltfSdkTool::AddOIDsAccessor(bufferBuilder, vectorIDs));
	}
	else
	{
		accessorIdOIDs.push_back("");
	}
	return true;
}

template<typename T>
inline void SetIDValue(std::vector<T>&lst, gisINT64& ID, vector<gisINT64>& IDs)
{
	if (lst.size() == 0)
	{
		ID = 0;
	}
	else
	{
		vector<gisINT64> temp;
		for (TYPENAME vector<T>::iterator itr = lst.begin(); itr != lst.end(); itr++)
		{
			temp.emplace_back((gisINT64)*itr);
		}
		bool isOneId = true;
		for (vector<gisINT64>::iterator itr = temp.begin() + 1; itr != temp.end(); itr++)
		{
			if (temp[0] != *itr)
			{
				isOneId = false;
				break;
			}
		}
		if (isOneId)
		{
			ID = temp[0];
		}
		else
		{
			IDs.swap(temp);
		}
	}
}

static bool GltfMeshToID(const Document& document, const GLBResourceReader& resourceReader, const Microsoft::glTF::MeshPrimitive& meshPrimitive, gisINT64& ID, vector<gisINT64>& IDs)
{
	string accessorId;
	if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_OID, accessorId))
	{
		vector<unsigned char> gltfOID;
		const Accessor& accessor = document.accessors.Get(accessorId);
		gltfOID = ReadBinaryData<unsigned char>(resourceReader, document, accessor);
		if (gltfOID.size() == 0)
		{
			ID = 0;
		}
		else if (gltfOID.size() == 4)
		{
			ID = RGBA_M(gltfOID[0], gltfOID[1], gltfOID[2], gltfOID[3]);
		}
		else if (gltfOID.size() % 4 == 0)
		{
			vector<gisINT64> oids;
			oids.reserve(gltfOID.size() / 4);
			bool isOneOid = true;
			unsigned char value1 = 0;
			unsigned char value2 = 0;
			unsigned char value3 = 0;
			unsigned char value4 = 0;
			for (int i = 0; i < gltfOID.size(); i += 4)
			{
				oids.push_back(RGBA_M(gltfOID[i], gltfOID[i + 1], gltfOID[i + 2], gltfOID[i + 3]));
				if (isOneOid)
				{
					if (i == 0)
					{
						value1 = gltfOID[i];
						value2 = gltfOID[i + 1];
						value3 = gltfOID[i + 2];
						value4 = gltfOID[i + 3];
					}
					else
					{
						if (value1 != gltfOID[i] || value2 != gltfOID[i + 1] || value3 != gltfOID[i + 2] || value4 != gltfOID[i + 3])
							isOneOid = false;
					}
				}
			}
			if (isOneOid)
			{
				ID = RGBA_M(value1, value2, value3, value4);
			}
			else
			{
				IDs.swap(oids);
			}
		}
	}
	else if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_BatchID, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);

		if (accessor.componentType == ComponentType::COMPONENT_FLOAT)
		{
			vector<float> gltfBatchID;
			gltfBatchID = ReadBinaryData<float>(resourceReader, document, accessor);
			SetIDValue(gltfBatchID, ID, IDs);
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
		{
			vector<gisUSHORT> gltfBatchID;
			gltfBatchID = ReadBinaryData<gisUSHORT>(resourceReader, document, accessor);
			SetIDValue(gltfBatchID, ID, IDs);
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_INT)
		{
			vector<gisUINT> gltfBatchID;
			gltfBatchID = ReadBinaryData<gisUINT>(resourceReader, document, accessor);
			SetIDValue(gltfBatchID, ID, IDs);
		}
	}
	return true;
}

static bool AnySurfaceToBufferBuilder(CAnySurface &surface, BufferBuilder& bufferBuilder, vector<string>& accessorIdIndices, vector<string>& accessorIdPositions, vector<string>& accessorIdNormals, vector<vector<string>>& accessorIdTexCoords, vector<string>& accessorIdColors,vector<Matrix4D> * pMatrix)
{
	gisLONG triangleNum = surface.GetTriangleNum();
	gisULONG*  triangles = surface.GetTriangles();
	gisLONG pointNum = surface.GetPointNum();
	D_3DOT* points = surface.GetPoints();
	gisLONG hasNormalVector = surface.HasNormalVector();
	gisLONG hasTexturePosition = surface.HasTexturePosition();
	gisLONG hasColor = surface.HasColor();
	vector<float> positions;
	if (pMatrix != NULL &&pMatrix->size() > 0)
	{
		positions.reserve(pointNum * 3 * pMatrix->size());
		for (gisLONG i = 0; i < pMatrix->size(); i++)
		{
			if (pMatrix->at(i).IsUnit())
			{
				for (gisLONG k = 0; k < pointNum; k++)
				{
					positions.push_back((float)((points[k]).x));
					positions.push_back((float)((points[k]).z));
					positions.push_back((float)(-(points[k]).y));
				}
			}
			else
			{
				for (gisLONG k = 0; k < pointNum; k++)
				{
					Vector4D item = pMatrix->at(i)*Vector4D(points[k].x, points[k].y, points[k].z, 1);
					positions.push_back((float)(item.x));
					positions.push_back((float)(item.z));
					positions.push_back((float)(-item.y));
				}
			}
		}
	}
	else
	{
		positions.reserve(pointNum * 3);
		for (gisLONG k = 0; k < pointNum; k++)
		{
			positions.push_back((float)((points[k]).x));
			positions.push_back((float)((points[k]).z));
			positions.push_back((float)(-(points[k]).y));
		}
	}
	accessorIdPositions.push_back(Ci_GltfSdkTool::AddPositionsAccessor(bufferBuilder, positions));
	vector<unsigned int> indices;

	if (pMatrix != NULL &&pMatrix->size() > 0)
	{
		indices.reserve(triangleNum * 3 * pMatrix->size());
		for (gisLONG i = 0; i < pMatrix->size(); i++)
		{
			for (gisLONG k = 0; k < triangleNum * 3; k++)
			{
				indices.push_back((unsigned int)(triangles[k] + i *pointNum));
			}
		}
	}
	else
	{
		indices.reserve(triangleNum * 3);
		for (gisLONG k = 0; k < triangleNum * 3; k++)
		{
			indices.push_back((unsigned int)(triangles[k]));
		}
	}
	accessorIdIndices.push_back(Ci_GltfSdkTool::AddIndexAccessor(bufferBuilder, indices));

	if (hasNormalVector)
	{
		D_3DOT* normalVector = surface.GetNormalVector();
		vector<float> normals;
		if (pMatrix != NULL &&pMatrix->size() > 0)
		{
			normals.reserve(pointNum * 3 * pMatrix->size());
			for (gisLONG i = 0; i < pMatrix->size(); i++)
			{
				if (pMatrix->at(i).IsUnit())
				{
					for (gisLONG k = 0; k < pointNum; k++)
					{
						normals.push_back((float)((normalVector[k]).x));
						normals.push_back((float)((normalVector[k]).z));
						normals.push_back((float)(-(normalVector[k]).y));
					}
				}
				else
				{
					MapGIS::Tile::Vector3D translation;
					MapGIS::Tile::Vector4D rotation;
					MapGIS::Tile::Vector3D scale;
					DecomposeMatrix4(pMatrix->at(i), translation, rotation, scale);
					//法向量只使用旋转参数，其他参数不需要
					MapGIS::Tile::Vector3D tempTrans((const double)0,(const double) 0,(const double) 0);
					MapGIS::Tile::Vector3D tempScale((const double)1, (const double)1, (const double)1);
					Matrix4D rotationMatrix = GetMatrix4(tempTrans, rotation, tempScale);
					for (gisLONG k = 0; k < pointNum; k++)
					{
						Vector4D item = rotationMatrix *Vector4D(normalVector[k].x, normalVector[k].y, normalVector[k].z, 1);
						normals.push_back((float)(item.x));
						normals.push_back((float)(item.z));
						normals.push_back((float)(-item.y));
					}
				}
			}
		}
		else
		{
			normals.reserve(pointNum * 3);
			for (gisLONG k = 0; k < pointNum; k++)
			{
				normals.push_back((float)((normalVector[k]).x));
				normals.push_back((float)((normalVector[k]).z));
				normals.push_back((float)(-(normalVector[k]).y));
			}
		}
		accessorIdNormals.push_back(Ci_GltfSdkTool::AddNormalsAccessor(bufferBuilder, normals));
	}
	else
	{
		accessorIdNormals.push_back("");
	}

	if (hasTexturePosition)
	{
		vector<string> item;
		map<int, string> itemMap;
		gisLONG layerNum = surface.GetTextureLayerNum();
		if (layerNum > 0)
		{
			D_DOT* texturePosition = surface.GetTexturePosition();
			for (int i = 0; i < layerNum; i++)
			{
				bool isExist = false;
				gisLONG startIndex = i * pointNum;
				if (i > 0)
				{
					for (int j = 0; j < i; j++)
					{
						if (memcmp(&texturePosition[startIndex], &texturePosition[j * pointNum], sizeof(D_DOT) * pointNum) == 0)
						{
							if (itemMap.find(j) != itemMap.end())
							{
								isExist = true;
								item.push_back(itemMap[j]);
								break;
							}
						}
					}
				}
				if (!isExist)
				{
					vector<float> texCoords;
					if (pMatrix != NULL &&pMatrix->size() > 0)
						texCoords.reserve(pointNum * 2 * pMatrix->size());
					else
						texCoords.reserve(pointNum * 2);

					for (gisLONG k = 0; k < pointNum; k++)
					{
						texCoords.push_back((float)((texturePosition[k + startIndex]).x));
						texCoords.push_back((float)((texturePosition[k + startIndex]).y));
					}
					if (pMatrix != NULL &&pMatrix->size() > 1)
					{
						for (gisLONG i = 1; i < pMatrix->size(); i++)
						{
							texCoords.insert(texCoords.end(), texCoords.begin(), texCoords.begin() + pointNum * 2);
						}
					}
					item.emplace_back(Ci_GltfSdkTool::AddTexCoordsAccessor(bufferBuilder, texCoords));
					itemMap.insert(make_pair(i, item.back()));
				}
			}
		}
		accessorIdTexCoords.push_back(item);
	}
	else
	{
		vector<string> item;
		accessorIdTexCoords.push_back(item);
	}

	if (hasColor)
	{
		gisULONG* color = surface.GetColor();
		vector<float> colors;

		if(pMatrix != NULL &&pMatrix->size() > 0)
			colors.reserve(pointNum * 4 * pMatrix->size());
		else
			colors.reserve(pointNum * 4);

		for (gisLONG k = 0; k < pointNum; k++)
		{
			float a = ((unsigned char)(color[k] >> 24 & 0xff)) / 255.0f;
			float b = ((unsigned char)(color[k] >> 16 & 0xff)) / 255.0f;
			float g = ((unsigned char)(color[k] >> 8 & 0xff)) / 255.0f;
			float r = ((unsigned char)(color[k] & 0xff)) / 255.0f;

			colors.push_back(r);
			colors.push_back(g);
			colors.push_back(b);
			colors.push_back(a);
		}
		if (pMatrix != NULL &&pMatrix->size() > 1)
		{
			for (gisLONG i = 1; i < pMatrix->size(); i++)
			{
				colors.insert(colors.end(), colors.begin(), colors.begin() + pointNum * 4);
			}
		}

		accessorIdColors.push_back(Ci_GltfSdkTool::AddTexColorsAccessor(bufferBuilder, colors));
	}
	else
	{
		accessorIdColors.push_back("");
	}
	return true;
}

static  vector<float> GltfDataNormalized(const Document& document, const GLBResourceReader& resourceReader,const Accessor& accessor)
{
	vector<float> rtn;
	switch (accessor.componentType)
	{
	case COMPONENT_BYTE:
	{
		vector<char> temp = ReadBinaryData<char>(resourceReader, document, accessor);
		for (vector<char>::iterator itr = temp.begin(); itr != temp.end(); itr++)
			rtn.emplace_back((float)(*itr / 127.0));
	}
	break;
	case COMPONENT_UNSIGNED_BYTE:
	{
		vector<unsigned char> temp = ReadBinaryData<unsigned char>(resourceReader, document, accessor);
		for (vector<unsigned char>::iterator itr = temp.begin(); itr != temp.end(); itr++)
			rtn.emplace_back((float)(*itr / 255.0));
	}
	break;
	case COMPONENT_SHORT:
	{
		vector<short> temp = ReadBinaryData<short>(resourceReader, document, accessor);
		for (vector<short>::iterator itr = temp.begin(); itr != temp.end(); itr++)
			rtn.emplace_back((float)(*itr / 32767.0));
	}
	break;
	case COMPONENT_UNSIGNED_SHORT:
	{
		vector<unsigned short> temp = ReadBinaryData<unsigned short>(resourceReader, document, accessor);
		for (vector<unsigned short>::iterator itr = temp.begin(); itr != temp.end(); itr++)
			rtn.emplace_back((float)(*itr / 65535.0));
	}
	break;
	default:
		break;
	}
	return  rtn;
}

static vector<float> GetGltfTexCoord(const Document& document, const GLBResourceReader& resourceReader, const Microsoft::glTF::MeshPrimitive& meshPrimitive)
{
	string accessorId;
	if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_TEXCOORD_0, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		if(accessor.componentType == COMPONENT_FLOAT)
			return ReadBinaryData<float>(resourceReader, document, accessor);
		else if(accessor.normalized)
			return GltfDataNormalized(document, resourceReader, accessor);
	}
	return vector<float>();
}

static vector<float> GetGltfCOLOR(const Document& document, const GLBResourceReader& resourceReader, const Microsoft::glTF::MeshPrimitive& meshPrimitive)
{
	string accessorId;
	if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_COLOR_0, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		if (accessor.type == AccessorType::TYPE_VEC4)
		{
			if (accessor.componentType == COMPONENT_FLOAT)
				return ReadBinaryData<float>(resourceReader, document, accessor);
			else if (accessor.normalized)
				return GltfDataNormalized(document, resourceReader, accessor);
		}
		else if (accessor.type == AccessorType::TYPE_VEC3)
		{
			vector<float> rtn;
			std::vector<float> temp;
			if (accessor.componentType == COMPONENT_FLOAT)
				temp = ReadBinaryData<float>(resourceReader, document, accessor);
			else if (accessor.normalized)
				temp = GltfDataNormalized(document, resourceReader, accessor);
			if (temp.size() % 3 == 0)
			{
				for (int i = 0; i + 2 < temp.size(); i = i + 3)
				{
					rtn.emplace_back(temp[i]);
					rtn.emplace_back(temp[i + 1]);
					rtn.emplace_back(temp[i + 2]);
					rtn.emplace_back(1);
				}
			}
			return rtn;
		}
	}
	return vector<float>();
}

static bool GltfMeshToAnySurface(const Document& document, const GLBResourceReader& resourceReader, const Microsoft::glTF::MeshPrimitive& meshPrimitive, Matrix4D gltfMatrix, CAnySurface& anySurface)
{

	if (meshPrimitive.mode != MeshMode::MESH_TRIANGLES)
		return false;
	vector<uint8_t> gltfIndices_8;
	vector<uint16_t> gltfIndices_16;
	vector<unsigned int> gltfIndices_32;
	vector<float> gltfPosition;
	vector<float> gltfNoraml;
	vector<float> gltfTexCoord;
	vector<float> gltfColor;

	const Accessor& accessor = document.accessors.Get(meshPrimitive.indicesAccessorId);
	if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_BYTE)
	{
		gltfIndices_8 = ReadBinaryData<uint8_t>(resourceReader, document, accessor);
	}
	else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
	{
		gltfIndices_16 = ReadBinaryData<uint16_t>(resourceReader, document, accessor);
	}
	else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_INT)
	{
		gltfIndices_32 = ReadBinaryData<unsigned int>(resourceReader, document, accessor);
	}

	string accessorId;

	if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_POSITION, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);

		gltfPosition = ReadBinaryData<float>(resourceReader, document, accessor);
	}

	if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_NORMAL, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);

		gltfNoraml = ReadBinaryData<float>(resourceReader, document, accessor);
	}

	gltfTexCoord = GetGltfTexCoord(document, resourceReader, meshPrimitive);

	gltfColor = GetGltfCOLOR(document, resourceReader, meshPrimitive);

	/*if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_TEXCOORD_0, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);

		gltfTexCoord = ReadBinaryData<float>(resourceReader, document, accessor);
	}

	if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_COLOR_0, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_BYTE)
		{
			vector<unsigned char>	tempColor = ReadBinaryData<unsigned char>(resourceReader, document, accessor);
			for (vector<unsigned char>::iterator itr = tempColor.begin(); itr != tempColor.end(); itr++)
				gltfColor.emplace_back(*itr / 255.0);
		}
		else
		{
			gltfColor = ReadBinaryData<float>(resourceReader, document, accessor);
		}
	}*/
	vector<D_3DOT> M3DDots;
	vector<gisULONG> M3DTriangle;
	vector<gisULONG> M3DColor;
	vector<D_3DOT> M3DNormal;
	vector<D_DOT> M3Duv;
	M3DDots.reserve(gltfPosition.size());
	for (gisLONG i = 0; i < gltfPosition.size(); i += 3)
	{
		D_3DOT dot = gltfMatrix*D_3DOT{ gltfPosition[i] ,gltfPosition[i + 1] ,gltfPosition[i + 2] };
		M3DDots.push_back(D_3DOT{ dot.x,-dot.z,dot.y });
	}

	if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_BYTE)
	{
		M3DTriangle.reserve(gltfIndices_8.size());
		for (gisLONG i = 0; i < gltfIndices_8.size(); i++)
		{
			M3DTriangle.push_back(gltfIndices_8[i]);
		}
	}
	else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
	{
		M3DTriangle.reserve(gltfIndices_16.size());
		for (gisLONG i = 0; i < gltfIndices_16.size(); i++)
		{
			M3DTriangle.push_back(gltfIndices_16[i]);
		}
	}

	else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_INT)
	{
		M3DTriangle.reserve(gltfIndices_32.size());
		for (gisLONG i = 0; i < gltfIndices_32.size(); i++)
		{
			M3DTriangle.push_back(gltfIndices_32[i]);
		}
	}
	M3DColor.reserve(gltfColor.size() / 4);
	for (gisLONG i = 0; i < gltfColor.size(); i += 4)
	{
		gisLONG r = (int)(gltfColor[i] * 255);
		gisLONG g = (int)(gltfColor[i + 1] * 255);
		gisLONG b = (int)(gltfColor[i + 2] * 255);
		gisLONG a = (int)(gltfColor[i + 3] * 255);

		M3DColor.push_back(RGBA_M(r, g, b, a));
	}
	M3DNormal.reserve(gltfNoraml.size() / 3);
	for (gisLONG i = 0; i < gltfNoraml.size(); i += 3)
	{
		D_3DOT normal;

		normal.x = gltfNoraml[i];
		normal.y = -gltfNoraml[i + 2];
		normal.z = gltfNoraml[i + 1];

		M3DNormal.push_back(normal);
	}
	M3Duv.reserve(gltfTexCoord.size() / 2);
	for (gisLONG i = 0; i < gltfTexCoord.size(); i += 2)
	{
		D_DOT uv;

		uv.x = gltfTexCoord[i];
		uv.y = gltfTexCoord[i + 1];

		M3Duv.push_back(uv);
	}
	anySurface.Set(M3DDots.size(), &M3DDots[0], M3DTriangle.size() / 3, &M3DTriangle[0], 0,
		M3DColor.size() > 0 ? &M3DColor[0] : NULL,
		M3DNormal.size() > 0 ? &M3DNormal[0] : NULL,
		M3Duv.size() > 0 ? M3Duv.size() / M3DDots.size() : 0,
		M3Duv.size() > 0 ? &M3Duv[0] : NULL);

	return true;
}

static bool  GltfNodeMatrix(const Document& document, gisLONG nodeIndex, Matrix4D &gltfMatrix)
{
	Matrix4Unit(gltfMatrix);
	if (document.nodes[nodeIndex].HasValidTransformType())
	{
		if (document.nodes[nodeIndex].GetTransformationType() == TransformationType::TRANSFORMATION_MATRIX)
		{
			gltfMatrix = Matrix4D(document.nodes[nodeIndex].matrix.values[0], document.nodes[nodeIndex].matrix.values[4], document.nodes[nodeIndex].matrix.values[8], document.nodes[nodeIndex].matrix.values[12]
				, document.nodes[nodeIndex].matrix.values[1], document.nodes[nodeIndex].matrix.values[5], document.nodes[nodeIndex].matrix.values[9], document.nodes[nodeIndex].matrix.values[13]
				, document.nodes[nodeIndex].matrix.values[2], document.nodes[nodeIndex].matrix.values[6], document.nodes[nodeIndex].matrix.values[10], document.nodes[nodeIndex].matrix.values[14]
				, document.nodes[nodeIndex].matrix.values[3], document.nodes[nodeIndex].matrix.values[7], document.nodes[nodeIndex].matrix.values[11], document.nodes[nodeIndex].matrix.values[15]
			);
		}
		else if (document.nodes[nodeIndex].GetTransformationType() == TransformationType::TRANSFORMATION_TRS)
		{
			MapGIS::Tile::Vector3D vec3d1(document.nodes[nodeIndex].translation.x, document.nodes[nodeIndex].translation.y, document.nodes[nodeIndex].translation.z);
			MapGIS::Tile::Vector4D vec4d(document.nodes[nodeIndex].rotation.x, document.nodes[nodeIndex].rotation.y, document.nodes[nodeIndex].rotation.z, document.nodes[nodeIndex].rotation.w);
			MapGIS::Tile::Vector3D vec3d2(document.nodes[nodeIndex].scale.x, document.nodes[nodeIndex].scale.y, document.nodes[nodeIndex].scale.z);
			gltfMatrix = GetMatrix4(vec3d1, vec4d, vec3d2);
		}
	}

	return true;
}

bool ReadGLTFInfo(const stringstream & buffer, Document& document, unique_ptr<Microsoft::glTF::GLBResourceReader>& resourceReader, G3DCacheStorage* pStorage)
{
	string       manifest;
	streambuf *sbuf = buffer.rdbuf();
#ifdef _WIN32
	auto streamReader = pStorage!=NULL? (std::shared_ptr<const IStreamReader>)make_unique<CacheStorageReader>(pStorage):(std::shared_ptr<const IStreamReader>)make_unique<StreamReader>("");
	auto glbStream = make_shared<istream>(sbuf);
	auto glbResourceReader = make_unique<GLBResourceReader>(move(streamReader), move(glbStream));
#else
	auto streamReader = (pStorage != nullptr)
		? std::shared_ptr<const IStreamReader>(new CacheStorageReader(pStorage))
		: std::shared_ptr<const IStreamReader>(new StreamReader(""));

	std::shared_ptr<std::istream> glbStream = std::make_shared<std::istream>(sbuf);

	std::unique_ptr<GLBResourceReader> glbResourceReader(new GLBResourceReader(std::move(streamReader), std::move(glbStream)));
#endif

	manifest = glbResourceReader->GetJson();

	resourceReader = move(glbResourceReader);

	try
	{
		document = Deserialize(manifest);
	}
	catch (const GLTFException& ex)
	{
		stringstream ss;

		ss << "Microsoft::glTF::Deserialize failed: ";
		ss << ex.what();

		throw runtime_error(ss.str());
	}
	return true;
}

gisLONG Ci_ModelGltf::From(MapGIS::Tile::G3DModel *pModel, GeoCompressType compressType, WriteIdType type, D_3DOT centerDot, bool imageInData)
{
	if (pModel == NULL)
		return 0;
	map<gisINT64, gisINT64> OidBatchID;
	return From(pModel, compressType, type, false, OidBatchID, centerDot, imageInData);
}

gisLONG Ci_ModelGltf::FromResetID(MapGIS::Tile::G3DModel *pModel, GeoCompressType compressType, WriteIdType type, map<gisINT64, gisINT64> &resetIDMap, D_3DOT centerDot, bool imageInData)
{
	if (pModel == NULL)
		return 0;
	return From(pModel, compressType, type, true, resetIDMap, centerDot, imageInData);
}

gisLONG Ci_ModelGltf::To(MapGIS::Tile::G3DModel *pModel)
{
	if (pModel == NULL)
		return 0;

	if (pModel->GetGeometryType() == GeometryType::Point)
	{
		MapGIS::Tile::PointsModel * pointModel = dynamic_cast<MapGIS::Tile::PointsModel *>(pModel);
		if (pointModel != NULL)
			return To(*pointModel);
	}
	else if (pModel->GetGeometryType() == GeometryType::Line)
	{
		MapGIS::Tile::LinesModel* pLineModel = dynamic_cast<MapGIS::Tile::LinesModel *>(pModel);
		if (pLineModel != NULL)
			return To(*pLineModel);
		else
			return 0;
	}
	else if (pModel->GetGeometryType() == GeometryType::Surface)
	{
		MapGIS::Tile::SurfacesModel * pSurfacesModel = dynamic_cast<MapGIS::Tile::SurfacesModel *>(pModel);
		if (pSurfacesModel != NULL)
			return To(*pSurfacesModel);
		else
			return 0;
	}
	else if (pModel->GetGeometryType() == GeometryType::Entity)
	{
		MapGIS::Tile::EntitiesModel * pEntitiesModel = dynamic_cast<MapGIS::Tile::EntitiesModel *>(pModel);
		if (pEntitiesModel != NULL)
			return To(*pEntitiesModel);
		else
			return 0;
	}
	return 0;
}

void GetQuaternionByNormalUpAndNormalRight(const vector<double> &normalUp, const vector<double> &normalRight, MapGIS::Tile::Vector4D &rotationQuaternion)
{
	Vector3D  vector3_d;
	vector3_d.x = normalRight[1] * normalUp[2] - normalRight[2] * normalUp[1];
	vector3_d.y = normalRight[2] * normalUp[0] - normalRight[0] * normalUp[2];
	vector3_d.z = normalRight[0] * normalUp[1] - normalRight[1] * normalUp[0];
	vector3_d.Normalise();

	auto GetValue = [normalUp, normalRight, vector3_d](int  column, int row)
	{
		if (column == 0)
			return  normalRight[row];
		else if (column == 1)
			return  normalUp[row];
		else if (column == 2)
			return (double)vector3_d[row];

		return (double)0.0;
	};

	double root = 0;
	double x = 0;
	double y = 0;
	double z = 0;
	double w = 1;
	double trace = normalRight[0] + normalUp[1] + vector3_d[2];
	if (trace > 0.0)
	{
		root = sqrt(trace + 1.0);
		w = 0.5 * root;
		root = 0.5 / root; // 1/(4w)

		x = (normalUp[2] - vector3_d[1]) * root;
		y = (vector3_d[0] - normalRight[2]) * root;
		z = (normalRight[1] - normalUp[0]) * root;
	}
	else
	{
		// |w| <= 1/2
		Vector3D next(1, 2, 0);

		int i = 0;
		if (normalUp[1] > normalRight[0]) {
			i = 1;
		}
		if (vector3_d[2] > normalRight[0] && vector3_d[2] > normalUp[1]) {
			i = 2;
		}

		int j = next[i];
		int k = next[j];

		root = sqrt(GetValue(i, i) - GetValue(j, j) - GetValue(k, k) + 1.0);
		double quat[3];
		quat[i] = 0.5 * root;
		root = 0.5 / root;
		w = (GetValue(k, j) - GetValue(j, k)) *root;
		quat[j] = (GetValue(j, i) + GetValue(i, j)) * root;
		quat[k] = (GetValue(k, i) + GetValue(i, k)) *root;
		x = -quat[0];
		y = -quat[1];
		z = -quat[2];
	}
	rotationQuaternion[0] = x;
	rotationQuaternion[1] = y;
	rotationQuaternion[2] = z;
	rotationQuaternion[3] = w;
}

MapGIS::Tile::Vector3D QuaternionVector3d(MapGIS::Tile::Vector4D &quaternion, MapGIS::Tile::Vector3D v)
{
	MapGIS::Tile::Vector3D qvec(quaternion.x, quaternion.y, quaternion.z);
	MapGIS::Tile::Vector3D uv = qvec.CrossProduct(v);
	MapGIS::Tile::Vector3D uuv = qvec.CrossProduct(uv);

	uv.x *= (2.0f * quaternion.w);
	uv.y *= (2.0f * quaternion.w);
	uv.z *= (2.0f * quaternion.w);

	uuv.x *= 2.0f;
	uuv.y *= 2.0f;
	uuv.z *= 2.0f;

	return MapGIS::Tile::Vector3D(v.x + uv.x + uuv.x, v.y + uv.y + uuv.y, v.z + uv.z + uuv.z);
}

MapGIS::Tile::Vector3D ToEulerAngles(MapGIS::Tile::Vector4D rotation) {
	MapGIS::Tile::Vector3D angles;

	// roll (x-axis rotation)
	double sinr_cosp = 2 * (rotation.w * rotation.x + rotation.y * rotation.z);
	double cosr_cosp = 1 - 2 * (rotation.x * rotation.x + rotation.y * rotation.y);
	angles.x = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = 2 * (rotation.w * rotation.y - rotation.z * rotation.x);
	if (std::abs(sinp) >= 1)
		angles.y = std::copysign(PI / 2, sinp); // use 90 degrees if out of range
	else
		angles.y = std::asin(sinp);

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (rotation.w * rotation.z + rotation.x * rotation.y);
	double cosy_cosp = 1 - 2 * (rotation.y * rotation.y + rotation.z * rotation.z);
	angles.z = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}
gisLONG i_InstanceToBufferBuilder(Microsoft::glTF::Document &document, Microsoft::glTF::BufferBuilder & bufferBuilder, vector<ModelInstance>* pInstance, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, unsigned int& maxID, string &translationId, string &rotationId, string &scaleId, string &batchidId)
{
	if (pInstance == NULL || pInstance->size() <= 0)
		return 0;
	vector<float> position;
	vector<float> rotation;
	vector<float> scale;
	vector<float> batchid;

	vector<double> normalUp;
	vector<double> normalRight;

	position.reserve(pInstance->size() * 3);
	rotation.reserve(pInstance->size() * 4);
	scale.reserve(pInstance->size() * 3);
	normalUp.resize(3);
	normalRight.resize(3);

	bool hasId = false;
	Matrix4D 	YupToZupMatrix(
		1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, -1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	Matrix4D 	ZupToYupMatrix4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, -1.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);

	for (int i = 0; i < pInstance->size(); i++)
	{
		MapGIS::Tile::Vector3D translation(pInstance->at(i).position.x, pInstance->at(i).position.y, pInstance->at(i).position.z);

		MapGIS::Tile::Vector4D rotationQuaternion;
		if (pInstance->at(i).hasNormalUp && pInstance->at(i).hasNormalRight)
		{
			normalUp[0] = pInstance->at(i).normalUp.x;
			normalUp[1] = pInstance->at(i).normalUp.y;
			normalUp[2] = pInstance->at(i).normalUp.z;

			normalRight[0] = pInstance->at(i).normalRight.x;
			normalRight[1] = pInstance->at(i).normalRight.y;
			normalRight[2] = pInstance->at(i).normalRight.z;
			GetQuaternionByNormalUpAndNormalRight(normalUp, normalRight, rotationQuaternion);
		}
		else
		{
			rotationQuaternion[0]=(0);
			rotationQuaternion[1]=(0);
			rotationQuaternion[2]=(0);
			rotationQuaternion[3]=(1);
		}

		MapGIS::Tile::Vector3D scaleItem;
		if (pInstance->at(i).hasScale)
		{
			scaleItem[0] = pInstance->at(i).scale;
			scaleItem[1] = pInstance->at(i).scale;
			scaleItem[2] = pInstance->at(i).scale;
		}
		else if (pInstance->at(i).hasScaleNonUniform)
		{
			scaleItem[0] = pInstance->at(i).scaleNonUniform.x;
			scaleItem[1] = pInstance->at(i).scaleNonUniform.y;
			scaleItem[2] = pInstance->at(i).scaleNonUniform.z;
		}
		else
		{
			scaleItem[0] = 1;
			scaleItem[1] = 1;
			scaleItem[2] = 1;
		}

		Matrix4D item = GetMatrix4(translation, rotationQuaternion, scaleItem);

		item = YupToZupMatrix * item * ZupToYupMatrix4;

		DecomposeMatrix4(item, translation, rotationQuaternion, scaleItem);

		position.emplace_back(translation.x);
		position.emplace_back(translation.y);
		position.emplace_back(translation.z);

		rotation.emplace_back(rotationQuaternion.x);
		rotation.emplace_back(rotationQuaternion.y);
		rotation.emplace_back(rotationQuaternion.z);
		rotation.emplace_back(rotationQuaternion.w);

		scale.emplace_back(scaleItem.x);
		scale.emplace_back(scaleItem.y);
		scale.emplace_back(scaleItem.z);

		if (type == WriteIdType::batchID &&    pInstance->at(i).hasId)
		{
			hasId = true;
			if (!resetID)
			{
				batchid.emplace_back(pInstance->at(i).id);
			}
			else
			{
				map<gisINT64, gisINT64>::iterator batchidItr = resetIDMap.find(pInstance->at(i).id);
				if (batchidItr == resetIDMap.end())
				{
					resetIDMap.insert(make_pair(pInstance->at(i).id, maxID));
					batchid.emplace_back(maxID);
					maxID++;
				}
				else
				{
					batchid.emplace_back(batchidItr->second);
				}
			}
		}
		else
			batchid.emplace_back(0);
	}

	translationId = Ci_GltfSdkTool::AddInstancingTranslationAccessor(bufferBuilder, position);
	rotationId = Ci_GltfSdkTool::AddInstancingRotationAccessor(bufferBuilder, rotation);
	scaleId = Ci_GltfSdkTool::AddInstancingScaleAccessor(bufferBuilder, scale);

	if (hasId)
		batchidId = Ci_GltfSdkTool::AddBatchIDsAccessor(bufferBuilder, batchid);
	else
		batchidId = "";
	return 1;
}

vector<Matrix4D> GetI3DDMMatrix4(vector<MapGIS::Tile::ModelInstance>* pInstances)
{
	vector<Matrix4D> rtn;
	if (pInstances == NULL || pInstances->size() <= 0)
		return rtn;
	for (int i = 0; i < pInstances->size(); i++)
	{
		auto &item = pInstances->at(i);
		MapGIS::Tile::Vector4D rotationQuaternion(0, 0, 0, 1);
		if (item.hasNormalUp &&item.hasNormalRight)
		{
			GetQuaternionByNormalUpAndNormalRight(vector<double>{item.normalUp.x, item.normalUp.y,item.normalUp.z},
				vector<double>{item.normalRight.x, item.normalRight.y, item.normalRight.z},  rotationQuaternion);
		}
		double scalingX = 1;
		double scalingY = 1;
		double scalingZ = 1;
		if (item.hasScale)
		{
			scalingX = item.scale;
			scalingY = item.scale;
			scalingZ = item.scale;
		}
		else if (item.hasScaleNonUniform)
		{
			scalingX = item.scaleNonUniform.x;
			scalingY = item.scaleNonUniform.y;
			scalingZ = item.scaleNonUniform.z;
		}
		Vector3D vec3d1(item.position.x, item.position.y, item.position.z);
		Vector3D vec3d2(scalingX, scalingY, scalingZ);
		Matrix4D matrix = GetMatrix4(vec3d1,rotationQuaternion, vec3d2);
		rtn.emplace_back(matrix);
	}
	return rtn;
}

gisLONG i_From(Microsoft::glTF::Document &document, Microsoft::glTF::Scene &scene, Microsoft::glTF::BufferBuilder & bufferBuilder, MapGIS::Tile::SurfacesModel &surfaceModel, vector<ModelInstance>* pInstance, unsigned int& maxBatchID, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, bool addEXT_mesh_gpu_instancing, G3DCacheStorage* pStorage, bool imageInData, D_3DOT centerDot)
{
	vector<string> accessorIdIndices;
	vector<string> accessorIdPositions;
	vector<string> accessorIdNormals;
	vector<vector<string>> accessorIdTexCoords;
	vector<string> accessorIdColors;
	vector<string> accessorIdOIDs;
	vector<string> accessorIdBatchIDs;

	vector<string> imageArray;
	vector<string> samplerArray;
	vector<string> textureArray;
	vector<string> materialArray;
	vector<string> imageMimeTypeArray;
	string translationId = "";
	string rotationId = "";
	string scaleId = "";
	string batchidId = "";

	for (gisLONG i = 0; i < surfaceModel.images.size(); i++)
	{
		ImageToBufferBuilder(document, surfaceModel.images[i], bufferBuilder, imageArray, imageMimeTypeArray, pStorage,imageInData);
	}

	for (gisLONG i = 0; i < surfaceModel.samplers.size(); i++)
	{
		SamplerToBufferBuilder(document, surfaceModel.samplers[i], bufferBuilder, samplerArray);
	}

	for (gisLONG i = 0; i < surfaceModel.textures.size(); i++)
	{
		TextureToBufferBuilder(document, surfaceModel.textures[i], imageArray, imageMimeTypeArray, samplerArray, bufferBuilder, textureArray);
	}

	for (gisLONG i = 0; i < surfaceModel.materials.size(); i++)
	{
		MaterialToBufferBuilder(document, surfaceModel.materials[i], textureArray, bufferBuilder, materialArray);
	}

	bool hasInstance = false;
	vector<Matrix4D> instanceMatrix;
	if (addEXT_mesh_gpu_instancing && pInstance != NULL &&pInstance->size() > 0)
	{
		hasInstance = true;
		i_InstanceToBufferBuilder(document, bufferBuilder, pInstance, type, resetID, resetIDMap, maxBatchID, translationId, rotationId, scaleId, batchidId);
		type = WriteIdType::None;
	}
	else if (!addEXT_mesh_gpu_instancing&& pInstance != NULL &&pInstance->size() > 0)
	{
		instanceMatrix = GetI3DDMMatrix4(pInstance);
		hasInstance = true;
	}

	if (instanceMatrix.size() == 0)
		instanceMatrix.emplace_back(Matrix4D());

	Microsoft::glTF::Mesh  gltfMesh;
	Microsoft::glTF::Node  gltfNode;
	gisLONG                  surfaceCount = 0;
	for (gisLONG i = 0; i < surfaceModel.features.size(); i++)
	{
		MapGIS::Tile::SurfaceFeature &feature = surfaceModel.features[i];
		vector<MapGIS::Tile::Mesh>& meshes = feature.meshes;
		size_t oidIndex = 0;
		for (gisLONG j = 0; j < meshes.size(); j++)
		{
			MapGIS::Tile::Mesh &mesh = feature.meshes[j];
			gisLONG pointNum = mesh.surface.GetPointNum();
			AnySurfaceToBufferBuilder(mesh.surface, bufferBuilder, accessorIdIndices, accessorIdPositions, accessorIdNormals, accessorIdTexCoords, accessorIdColors, &instanceMatrix);
			if (type == WriteIdType::oid)
			{
				accessorIdBatchIDs.emplace_back("");
				if (hasInstance && !addEXT_mesh_gpu_instancing)
				{
					if (instanceMatrix.size() > 0)
					{
						vector<unsigned int> vectorIDs;
						bool hasId = false;
						for (int k = 0; k < instanceMatrix.size(); k++)
						{
							if (pInstance->at(k).hasId)
								hasId = true;
							vector<unsigned int> tempIDs;
							MakeIds(pointNum, pInstance->at(k).hasId ? (gisINT64)pInstance->at(k).id : (gisINT64)-1, vector<gisINT64>(), oidIndex, resetID, tempIDs, resetIDMap, maxBatchID);
							vectorIDs.insert(vectorIDs.end(), tempIDs.begin(), tempIDs.end());
						}

						if (hasId && vectorIDs.size() > 0)
						{
							accessorIdOIDs.push_back(Ci_GltfSdkTool::AddOIDsAccessor(bufferBuilder, vectorIDs));
						}
						else
						{
							accessorIdOIDs.push_back("");
						}
					}
				}
				else
				{
					OidToBufferBuilder(pointNum, feature.id, feature.ids, oidIndex, bufferBuilder, accessorIdOIDs, resetID, resetIDMap, maxBatchID);
				}
			}
			else if (type == WriteIdType::batchID)
			{
				accessorIdOIDs.emplace_back("");
				if (hasInstance && !addEXT_mesh_gpu_instancing)
				{
					vector<float> vectorIDs;
					bool hasId = false;
					for (int k = 0; k < instanceMatrix.size(); k++)
					{
						if (pInstance->at(k).hasId)
							hasId = true;
						vector<unsigned int> tempIDs;
						MakeIds(pointNum, pInstance->at(k).hasId ? pInstance->at(k).id : -1, vector<gisINT64>(), oidIndex,resetID, tempIDs, resetIDMap, maxBatchID);
						vectorIDs.insert(vectorIDs.end(), tempIDs.begin(), tempIDs.end());
					}

					if (hasId &&vectorIDs.size() > 0)
					{
						accessorIdBatchIDs.push_back(Ci_GltfSdkTool::AddBatchIDsAccessor(bufferBuilder, vectorIDs));
					}
					else
					{
						accessorIdBatchIDs.push_back("");
					}
				}
				else
					BatchIDToBufferBuilder(pointNum, feature.id, feature.ids, oidIndex, bufferBuilder, accessorIdBatchIDs, resetID, resetIDMap, maxBatchID);
			}
			else
			{
				accessorIdOIDs.emplace_back("");
				accessorIdBatchIDs.emplace_back("");
			}

			MeshMode mode = MESH_TRIANGLES;
			if (mesh.materialIndex == -1)
			{
				string materialIndex = "";
				Ci_GltfSdkTool::MeshAddPrimitive(gltfMesh, mode, materialIndex, accessorIdIndices[surfaceCount], accessorIdPositions[surfaceCount], accessorIdNormals[surfaceCount],
					accessorIdTexCoords[surfaceCount], accessorIdColors[surfaceCount], accessorIdOIDs[surfaceCount], accessorIdBatchIDs[surfaceCount]);
			}
			else
			{
				Ci_GltfSdkTool::MeshAddPrimitive(gltfMesh, mode, materialArray[mesh.materialIndex], accessorIdIndices[surfaceCount], accessorIdPositions[surfaceCount], accessorIdNormals[surfaceCount],
					accessorIdTexCoords[surfaceCount], accessorIdColors[surfaceCount], accessorIdOIDs[surfaceCount], accessorIdBatchIDs[surfaceCount]);
			}

			surfaceCount++;
		}
	}
	std::string meshID = Ci_GltfSdkTool::AddMesh(document, gltfMesh);
	gltfNode.meshId = meshID;
	gltfNode.translation = Vector3(centerDot.x, centerDot.z, -centerDot.y);
	Ci_GltfSdkTool::NodeAddGPUInstancingEx(document, gltfNode, translationId, rotationId, scaleId, batchidId);
	std::string nodeID = Ci_GltfSdkTool::AddNode(document, gltfNode);
	scene.nodes.push_back(nodeID);
	return 1;
}

gisLONG i_From(Microsoft::glTF::Document &document, Microsoft::glTF::Scene &scene, Microsoft::glTF::BufferBuilder & bufferBuilder, MapGIS::Tile::EntitiesModel &entityModel, vector<ModelInstance>* pInstance, unsigned int& maxBatchID, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, bool addEXT_mesh_gpu_instancing, G3DCacheStorage* pStorage, bool imageInData, D_3DOT centerDot)
{
	vector<string> accessorIdIndices;
	vector<string> accessorIdPositions;
	vector<string> accessorIdNormals;
	vector<vector<string>> accessorIdTexCoords;
	vector<string> accessorIdColors;
	vector<string> accessorIdOIDs;
	vector<string> accessorIdBatchIDs;

	vector<string> imageArray;
	vector<string> samplerArray;
	vector<string> textureArray;
	vector<string> materialArray;
	vector<string> imageMimeTypeArray;

	string translationId = "";
	string rotationId = "";
	string scaleId = "";
	string batchidId = "";

	for (gisLONG i = 0; i < entityModel.images.size(); i++)
	{
		ImageToBufferBuilder(document, entityModel.images[i], bufferBuilder, imageArray, imageMimeTypeArray, pStorage,imageInData);
	}

	for (gisLONG i = 0; i < entityModel.samplers.size(); i++)
	{
		SamplerToBufferBuilder(document, entityModel.samplers[i], bufferBuilder, samplerArray);
	}

	for (gisLONG i = 0; i < entityModel.textures.size(); i++)
	{
		TextureToBufferBuilder(document, entityModel.textures[i], imageArray, imageMimeTypeArray, samplerArray, bufferBuilder, textureArray);
	}

	for (gisLONG i = 0; i < entityModel.materials.size(); i++)
	{
		MaterialToBufferBuilder(document, entityModel.materials[i], textureArray, bufferBuilder, materialArray);
	}

	bool hasInstance = false;
	vector<Matrix4D> instanceMatrix;
	if (addEXT_mesh_gpu_instancing && pInstance != NULL &&pInstance->size()>0)
	{
		hasInstance = true;
		i_InstanceToBufferBuilder(document, bufferBuilder, pInstance, type, resetID, resetIDMap, maxBatchID, translationId, rotationId, scaleId, batchidId);
		type = WriteIdType::None;
	}
	else if (!addEXT_mesh_gpu_instancing&& pInstance != NULL &&pInstance->size()>0)
	{
		instanceMatrix = GetI3DDMMatrix4(pInstance);
		hasInstance = true;
	}

	if (instanceMatrix.size() == 0)
		instanceMatrix.emplace_back(Matrix4D());

	gisLONG surfaceCount = 0;
	Microsoft::glTF::Mesh  gltfMesh;
	Microsoft::glTF::Node  gltfNode;
	MeshMode mode = MESH_TRIANGLES;

	for (gisLONG i = 0; i < entityModel.features.size(); i++)
	{
		MapGIS::Tile::EntityFeature &feature = entityModel.features[i];
		size_t oidIndex = 0;
		CAnyEntity &entity = feature.entity;
		for (gisLONG j = 0; j < entity.GetSurfaceNum(); j++)
		{
			CAnySurface* pSurface = entity.GetSurface(j);
			gisLONG pointNum = pSurface->GetPointNum();

			AnySurfaceToBufferBuilder(*pSurface, bufferBuilder, accessorIdIndices, accessorIdPositions, accessorIdNormals, accessorIdTexCoords, accessorIdColors, &instanceMatrix);

			if (type == WriteIdType::oid)
			{
				accessorIdBatchIDs.emplace_back("");

				if (hasInstance && !addEXT_mesh_gpu_instancing)
				{
					if (instanceMatrix.size() > 0)
					{
						vector<unsigned int> vectorIDs;
						bool hasId = false;
						for (int k = 0; k < instanceMatrix.size(); k++)
						{
							if (pInstance->at(k).hasId)
								hasId = true;
							vector<unsigned int> tempIDs;
							MakeIds(pointNum, pInstance->at(k).hasId ? (gisINT64)pInstance->at(k).id : (gisINT64)-1, vector<gisINT64>(), oidIndex, resetID, tempIDs, resetIDMap, maxBatchID);
							vectorIDs.insert(vectorIDs.end(), tempIDs.begin(), tempIDs.end());
						}

						if (hasId && vectorIDs.size() > 0)
						{
							accessorIdOIDs.push_back(Ci_GltfSdkTool::AddOIDsAccessor(bufferBuilder, vectorIDs));
						}
						else
						{
							accessorIdOIDs.push_back("");
						}
					}
				}
				else
					OidToBufferBuilder(pointNum, feature.id, feature.ids, oidIndex, bufferBuilder, accessorIdOIDs, resetID, resetIDMap, maxBatchID);
			}
			else  if (type == WriteIdType::batchID)
			{
				accessorIdOIDs.emplace_back("");
				if (hasInstance && !addEXT_mesh_gpu_instancing)
				{
					vector<float> vectorIDs;
					bool hasId = false;
					for (int k = 0; k < instanceMatrix.size(); k++)
					{
						if (pInstance->at(k).hasId)
							hasId = true;
						vector<unsigned int> tempIDs;
						MakeIds(pointNum, pInstance->at(k).hasId ? pInstance->at(k).id : -1, vector<gisINT64>(), oidIndex, resetID, tempIDs, resetIDMap, maxBatchID);
						vectorIDs.insert(vectorIDs.end(), tempIDs.begin(), tempIDs.end());
					}

					if (hasId &&vectorIDs.size() > 0)
					{
						accessorIdBatchIDs.push_back(Ci_GltfSdkTool::AddBatchIDsAccessor(bufferBuilder, vectorIDs));
					}
					else
					{
						accessorIdBatchIDs.push_back("");
					}
				}
				else
					BatchIDToBufferBuilder(pointNum, feature.id, feature.ids, oidIndex, bufferBuilder, accessorIdBatchIDs, resetID, resetIDMap, maxBatchID);
			}
			else
			{
				accessorIdOIDs.emplace_back("");
				accessorIdBatchIDs.emplace_back("");
			}

			if (feature.materialIndexArray[j] == -1)
			{
				string materialIndex = "";

				Ci_GltfSdkTool::MeshAddPrimitive(gltfMesh, mode, materialIndex, accessorIdIndices[surfaceCount], accessorIdPositions[surfaceCount], accessorIdNormals[surfaceCount],
					accessorIdTexCoords[surfaceCount], accessorIdColors[surfaceCount], accessorIdOIDs[surfaceCount], accessorIdBatchIDs[surfaceCount]);
			}
			else
			{
				Ci_GltfSdkTool::MeshAddPrimitive(gltfMesh, mode, materialArray[feature.materialIndexArray[j]], accessorIdIndices[surfaceCount], accessorIdPositions[surfaceCount], accessorIdNormals[surfaceCount],
					accessorIdTexCoords[surfaceCount], accessorIdColors[surfaceCount], accessorIdOIDs[surfaceCount], accessorIdBatchIDs[surfaceCount]);
			}
			surfaceCount++;
		}
	}

	std::string meshID = Ci_GltfSdkTool::AddMesh(document, gltfMesh);
	gltfNode.meshId = meshID;
	gltfNode.translation = Vector3(centerDot.x, centerDot.z, -centerDot.y);
	Ci_GltfSdkTool::NodeAddGPUInstancingEx(document, gltfNode, translationId, rotationId, scaleId, batchidId);
	std::string nodeID = Ci_GltfSdkTool::AddNode(document, gltfNode);
	scene.nodes.push_back(nodeID);

	return 1;
}

gisLONG i_From(Microsoft::glTF::Document &document, Microsoft::glTF::Scene &scene, Microsoft::glTF::BufferBuilder & bufferBuilder, MapGIS::Tile::LinesModel &lineModel, vector<ModelInstance>* pInstance, unsigned int& maxBatchID, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, bool addEXT_mesh_gpu_instancing, D_3DOT centerDot)
{
	vector<string> accessorIdIndices;
	vector<string> accessorIdPositions;
	vector<string> accessorIdColors;
	vector<string> accessorIdOIDs;
	vector<string> accessorIdBatchIDs;
	vector<MeshMode> accessorModes;

	string translationId = "";
	string rotationId = "";
	string scaleId = "";
	string batchidId = "";

	auto GetCurrentID = [&maxBatchID, resetID, &resetIDMap](gisINT64 id)
	{
		gisINT64 currentID = id;
		if (id > 0 && resetID)
		{
			map<gisINT64, gisINT64>::iterator itr = resetIDMap.find(id);
			if (itr == resetIDMap.end())
			{
				resetIDMap.insert(make_pair(id, maxBatchID));
				currentID = maxBatchID;
				maxBatchID++;
			}
			else
			{
				currentID = itr->second;
			}
		}
		if (currentID < 0)
			currentID = 0;
		return currentID;
	};

	auto MakeGlbData = [resetID](CVarLine& line, MapGIS::Tile::Color4f &color, gisINT64 id, vector<unsigned int> &indices, vector<float>& positions, vector<float>& colors, vector<gisINT64>& ids, Matrix4D* pMatrix)
	{
		gisLONG pointNum = line.dotNum();
		gisLONG dim = line.dim();
		D_3DOT dot;
		for (gisLONG m = 0; m < pointNum; m++)
		{
			indices.push_back(indices.size());
			dot.x = line.GetX(m);
			dot.y = line.GetY(m);
			if (dim == 3)
				dot.z = line.GetZ(m);
			else
				dot.z = 0;

			if (pMatrix != NULL)
			{
				Vector4D item =(*pMatrix)*Vector4D(dot.x, dot.y, dot.z, 1.0);
				dot.x = item.x;
				dot.y = item.y;
				dot.z = item.z;
			}
			positions.push_back((float)dot.x);
			positions.push_back((float)dot.z);
			positions.push_back((float)-dot.y);
			colors.push_back(color.r);
			colors.push_back(color.g);
			colors.push_back(color.b);
			colors.push_back(color.a);
			if (id >= 0)
				ids.push_back(id);
		}
	};

	auto MakeGlbIdData = [&accessorIdOIDs, &accessorIdBatchIDs, &type, &bufferBuilder](vector<gisINT64> &linesIDs, vector<unsigned int>& linesIndices)
	{
		if (linesIDs.size() != linesIndices.size())
		{
			accessorIdOIDs.push_back("");
			accessorIdBatchIDs.push_back("");
		}
		else
		{
			if (type == WriteIdType::oid)
			{
				vector<unsigned int> OIDs;

				for (vector<gisINT64>::iterator itr = linesIDs.begin(); itr != linesIDs.end(); itr++)
				{
					OIDs.emplace_back((unsigned int)*itr);
				}

				accessorIdOIDs.push_back(Ci_GltfSdkTool::AddOIDsAccessor(bufferBuilder, OIDs));
				accessorIdBatchIDs.push_back("");
			}
			else if (type == WriteIdType::batchID)
			{
				vector<float> batchIDs;
				for (vector<gisINT64>::iterator itr = linesIDs.begin(); itr != linesIDs.end(); itr++)
				{
					batchIDs.emplace_back((float)*itr);
				}
				accessorIdOIDs.push_back("");
				accessorIdBatchIDs.push_back(Ci_GltfSdkTool::AddBatchIDsAccessor(bufferBuilder, batchIDs));
			}
			else
			{
				accessorIdOIDs.push_back("");
				accessorIdBatchIDs.push_back("");
			}
		}
	};

	//管线中数据多数都是线段，这里线将线中是两个点得特殊处理
	vector<unsigned int> linesIndices;
	vector<float> linesPositions;
	vector<float> linesColors;
	vector<gisINT64> linesIDs;
	bool hasInstance = false;
	vector<Matrix4D> instanceMatrix;
	if (addEXT_mesh_gpu_instancing && pInstance != NULL &&pInstance->size()>0)
	{
		hasInstance = true;
		i_InstanceToBufferBuilder(document, bufferBuilder, pInstance, type, resetID, resetIDMap, maxBatchID, translationId, rotationId, scaleId, batchidId);
		type = WriteIdType::None;
	}
	else if (!addEXT_mesh_gpu_instancing&& pInstance != NULL &&pInstance->size()>0)
	{
		instanceMatrix = GetI3DDMMatrix4(pInstance);
		hasInstance = true;
	}
	if(instanceMatrix.size()==0)
		instanceMatrix.emplace_back(Matrix4D());

	for (gisLONG m = 0; m < instanceMatrix.size(); m++)
	{
		gisINT64 currentID = -1;
		if (hasInstance && !addEXT_mesh_gpu_instancing &&  pInstance->at(m).hasId)
			currentID = GetCurrentID(pInstance->at(m).id);
		for (gisLONG i = 0; i < lineModel.features.size(); i++)
		{
			MapGIS::Tile::LineFeature& feature = lineModel.features.at(i);
			if (!hasInstance)
				currentID = GetCurrentID(feature.id);

			for (gisLONG j = 0; j < feature.colorLines.size(); j++)
			{
				MapGIS::Tile::ColorLine &colorLine = feature.colorLines.at(j);

				MapGIS::Tile::Color4f color = colorLine.color;

				for (gisLONG k = 0; k < colorLine.lines.size(); k++)
				{
					CVarLine& line = colorLine.lines.at(k);
					gisLONG pointNum = line.dotNum();
					gisLONG dim = line.dim();
					if (pointNum == 2)
					{
						MakeGlbData(line, color, currentID, linesIndices, linesPositions, linesColors, linesIDs, &instanceMatrix.at(m));
					}
				}
			}
		}
	}

	if (linesIndices.size() > 0)
	{
		accessorIdIndices.push_back(Ci_GltfSdkTool::AddIndexAccessor(bufferBuilder, linesIndices));
		accessorIdPositions.push_back(Ci_GltfSdkTool::AddPositionsAccessor(bufferBuilder, linesPositions));
		accessorIdColors.push_back(Ci_GltfSdkTool::AddTexColorsAccessor(bufferBuilder, linesColors));
		accessorModes.emplace_back(MESH_LINES);
		MakeGlbIdData(linesIDs, linesIndices);
	}

	for (gisLONG m = 0; m < instanceMatrix.size(); m++)
	{
		gisINT64 currentID = -1;
		if (hasInstance && !addEXT_mesh_gpu_instancing&& pInstance->at(m).hasId)
			currentID = GetCurrentID(pInstance->at(m).id);
		for (gisLONG i = 0; i < lineModel.features.size(); i++)
		{
			MapGIS::Tile::LineFeature& feature = lineModel.features.at(i);
			if (!hasInstance)
				currentID = GetCurrentID(feature.id);

			for (gisLONG j = 0; j < feature.colorLines.size(); j++)
			{
				MapGIS::Tile::ColorLine &colorLine = feature.colorLines.at(j);

				MapGIS::Tile::Color4f color = colorLine.color;

				for (gisLONG k = 0; k < colorLine.lines.size(); k++)
				{
					CVarLine& line = colorLine.lines.at(k);
					gisLONG pointNum = line.dotNum();
					if (pointNum > 2)
					{
						vector<unsigned int> indices;
						vector<float> positions;
						vector<float> colors;
						vector<gisINT64> IDs;
						MakeGlbData(line, color, currentID, indices, positions, colors, IDs, &instanceMatrix.at(m));
						accessorIdIndices.push_back(Ci_GltfSdkTool::AddIndexAccessor(bufferBuilder, indices));
						accessorIdPositions.push_back(Ci_GltfSdkTool::AddPositionsAccessor(bufferBuilder, positions));
						accessorIdColors.push_back(Ci_GltfSdkTool::AddTexColorsAccessor(bufferBuilder, colors));
						accessorModes.emplace_back(MESH_LINE_STRIP);
						MakeGlbIdData(IDs, indices);
					}
				}
			}
		}
	}
	string   materialId = "";
	string   accessorIdNormals = "";
	vector<string> accessorIdTexCoords;
	Microsoft::glTF::Mesh  gltfMesh;
	Microsoft::glTF::Node  gltfNode;
	if (accessorIdIndices.size() > 0)
	{
		for (int i = 0; i < accessorIdIndices.size(); i++)
		{
			Ci_GltfSdkTool::MeshAddPrimitive(gltfMesh, accessorModes[i], materialId, accessorIdIndices[i], accessorIdPositions[i], accessorIdNormals,
				accessorIdTexCoords, accessorIdColors[i], accessorIdOIDs[i], accessorIdBatchIDs[i]);
		}
	}

	std::string meshID = Ci_GltfSdkTool::AddMesh(document, gltfMesh);
	gltfNode.meshId = meshID;

	gltfNode.translation = Vector3(centerDot.x, centerDot.z, -centerDot.y);
	Ci_GltfSdkTool::NodeAddGPUInstancingEx(document, gltfNode, translationId, rotationId, scaleId, batchidId);
	std::string nodeID = Ci_GltfSdkTool::AddNode(document, gltfNode);
	scene.nodes.push_back(nodeID);

	return 1;
}

gisLONG i_From(Microsoft::glTF::Document &document, Microsoft::glTF::Scene &scene, Microsoft::glTF::BufferBuilder & bufferBuilder, MapGIS::Tile::PointsModel &pointModel, vector<ModelInstance>* pInstance, unsigned int& maxBatchID, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, bool addEXT_mesh_gpu_instancing, D_3DOT centerDot)
{
	vector<string> accessorIdPositions;
	vector<string> accessorIdColors;
	vector<string> accessorIdOIDs;
	vector<string> accessorIdBatchIDs;
	string translationId = "";
	string rotationId = "";
	string scaleId = "";
	string batchidId = "";
	auto GetCurrentID = [&maxBatchID, resetID, &resetIDMap](gisINT64 id)
	{
		gisINT64 currentID = id;
		if (id> 0 && resetID)
		{
			map<gisINT64, gisINT64>::iterator itr = resetIDMap.find(id);
			if (itr == resetIDMap.end())
			{
				resetIDMap.insert(make_pair(id, maxBatchID));
				currentID = maxBatchID;
				maxBatchID++;
			}
			else
			{
				currentID = itr->second;
			}
		}
		if (currentID < 0)
			currentID = 0;
		return currentID;
	};

	auto MakeGlbIdData = [&accessorIdOIDs, &accessorIdBatchIDs, &type, &bufferBuilder](vector<gisINT64> &pinttIDs)
	{
		if (pinttIDs.size() > 0)
		{
			if (type == WriteIdType::oid)
			{
				vector<unsigned int> OIDs;

				for (vector<gisINT64>::iterator itr = pinttIDs.begin(); itr != pinttIDs.end(); itr++)
				{
					OIDs.emplace_back((unsigned int)*itr);
				}

				accessorIdOIDs.push_back(Ci_GltfSdkTool::AddOIDsAccessor(bufferBuilder, OIDs));
				accessorIdBatchIDs.push_back("");
			}
			else if (type == WriteIdType::batchID)
			{
				vector<float> batchIDs;
				for (vector<gisINT64>::iterator itr = pinttIDs.begin(); itr != pinttIDs.end(); itr++)
				{
					batchIDs.emplace_back((float)*itr);
				}
				accessorIdOIDs.push_back("");
				accessorIdBatchIDs.push_back(Ci_GltfSdkTool::AddBatchIDsAccessor(bufferBuilder, batchIDs));
			}
			else
			{
				accessorIdOIDs.push_back("");
				accessorIdBatchIDs.push_back("");
			}
		}
	};

	vector<Matrix4D> instanceMatrix;
	bool hasInstance = false;

	if (addEXT_mesh_gpu_instancing && pInstance != NULL &&pInstance->size()>0)
	{
		hasInstance = true;
		i_InstanceToBufferBuilder(document, bufferBuilder, pInstance, type, resetID, resetIDMap, maxBatchID, translationId, rotationId, scaleId, batchidId);
		type = WriteIdType::None;
	}
	else if(!addEXT_mesh_gpu_instancing&& pInstance != NULL &&pInstance->size()>0)
	{
		instanceMatrix = GetI3DDMMatrix4(pInstance);
		hasInstance = true;
	}

	if (instanceMatrix.size() == 0)
		instanceMatrix.emplace_back(Matrix4D());

	vector<float> positions;
	vector<float> colors;
	vector<gisINT64> IDs;

	for (gisLONG k = 0; k< instanceMatrix.size(); k++)
	{
		gisINT64 currentID = -1;
		if (hasInstance && !addEXT_mesh_gpu_instancing &&  pInstance->at(k).hasId)
			currentID = GetCurrentID(pInstance->at(k).id);
		for (gisLONG i = 0; i < pointModel.features.size(); i++)
		{
			MapGIS::Tile::PointFeature& feature = pointModel.features.at(i);
			if (!hasInstance)
				currentID = GetCurrentID(feature.id);
			for (gisLONG j = 0; j < feature.colorPoints.size(); j++)
			{
				MapGIS::Tile::ColorPoint &colorLine = feature.colorPoints.at(j);
				MapGIS::Tile::Color4f& color = colorLine.color;
				CPoints& points = colorLine.points;
				gisLONG pntNum = points.GetNum();
				D_3DOT * pDots = points.GetBufPtr();
				for (gisLONG m = 0; m < pntNum; m++)
				{
					Vector4D dot = instanceMatrix.at(k)*Vector4D(pDots[m].x, pDots[m].y, pDots[m].z, 1.0);
					positions.push_back((float)(dot.x));
					positions.push_back((float)(dot.z));
					positions.push_back((float)(-dot.y));
					colors.push_back(color.r);
					colors.push_back(color.g);
					colors.push_back(color.b);
					colors.push_back(color.a);
					if (currentID >=0)
						IDs.push_back(currentID);
				}
			}
		}
	}

	accessorIdPositions.push_back(Ci_GltfSdkTool::AddPositionsAccessor(bufferBuilder, positions));
	accessorIdColors.push_back(Ci_GltfSdkTool::AddTexColorsAccessor(bufferBuilder, colors));

	if (IDs.size()>0)
		MakeGlbIdData(IDs);

	MeshMode mode = MESH_POINTS;
	string   materialId = "";
	string   accessorIdNormals = "";
	string   accessorIdIndices = "";

	vector<string> accessorIdTexCoords;
	if (accessorIdPositions.size() > 0)
	{
		Microsoft::glTF::Mesh  gltfMesh;
		Microsoft::glTF::Node  gltfNode;

		Ci_GltfSdkTool::MeshAddPrimitive(gltfMesh, mode, materialId, accessorIdIndices, accessorIdPositions[0], accessorIdNormals,
			accessorIdTexCoords, accessorIdColors[0], accessorIdOIDs[0], accessorIdBatchIDs[0]);

		std::string meshID = Ci_GltfSdkTool::AddMesh(document, gltfMesh);
		gltfNode.meshId = meshID;
		gltfNode.translation = Vector3(centerDot.x, centerDot.z, -centerDot.y);
		Ci_GltfSdkTool::NodeAddGPUInstancingEx(document, gltfNode, translationId, rotationId, scaleId, batchidId);
		std::string nodeID = Ci_GltfSdkTool::AddNode(document, gltfNode);
		scene.nodes.push_back(nodeID);
	}
	return 1;
}

gisLONG Ci_ModelGltf::From(MapGIS::Tile::G3DModel *pModel, GeoCompressType compressType, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap, D_3DOT centerDot, bool imageInData)
{
#ifdef _WIN32
	unique_ptr<iostream>           tempBufferStream = make_unique<stringstream>();
	unique_ptr<GLBResourceWriterMemory> resourceWriter = make_unique<GLBResourceWriterMemory>(move(tempBufferStream));
#else
	std::unique_ptr<std::stringstream> tempBufferStream(new std::stringstream());
	std::unique_ptr<GLBResourceWriterMemory> resourceWriter(new GLBResourceWriterMemory(std::move(tempBufferStream)));
#endif
	Microsoft::glTF::Document                document;
	Microsoft::glTF::BufferBuilder           bufferBuilder(move(resourceWriter));
	GLBResourceWriterMemory                 *glbResourceWriterMemory = dynamic_cast<GLBResourceWriterMemory*>(&bufferBuilder.GetResourceWriter());
	if (glbResourceWriterMemory == NULL)
		return 0;

	if (resetID)
	{
		resetIDMap.clear();
	}

	unsigned int maxBatchID = 0;
	//转换
	bufferBuilder.AddBuffer(GLB_BUFFER_ID);
	Scene   scene;
	bool hasValue = false;

	if (pModel != NULL &&  pModel->GetGeometryType() == GeometryType::Point)
	{
		MapGIS::Tile::PointsModel * pointModel = dynamic_cast<MapGIS::Tile::PointsModel *>(pModel);
		if (pointModel != NULL)
		{
			i_From(document, scene, bufferBuilder, *pointModel, NULL, maxBatchID, type, resetID, resetIDMap,false, centerDot);
			if (pointModel->features.size() > 0)
				hasValue = true;
		}
	}
	else if (pModel != NULL &&  pModel->GetGeometryType() == GeometryType::Line)
	{
		MapGIS::Tile::LinesModel * lineModel = dynamic_cast<MapGIS::Tile::LinesModel *>(pModel);
		if (lineModel != NULL)
		{
			i_From(document, scene, bufferBuilder, *lineModel, NULL, maxBatchID, type, resetID, resetIDMap, false, centerDot);
			if (lineModel->features.size() > 0)
				hasValue = true;
		}
	}
	else if (pModel != NULL &&  pModel->GetGeometryType() == GeometryType::Surface)
	{
		MapGIS::Tile::SurfacesModel* surfacesModel = dynamic_cast<MapGIS::Tile::SurfacesModel*>(pModel);
		if (surfacesModel != NULL)
		{
			i_From(document, scene, bufferBuilder, *surfacesModel, NULL, maxBatchID, type, resetID, resetIDMap, false, m_pStorage, imageInData, centerDot);
			if (surfacesModel->features.size() > 0)
				hasValue = true;
		}
	}
	else if (pModel != NULL &&  pModel->GetGeometryType() == GeometryType::Entity)
	{
		MapGIS::Tile::EntitiesModel* entitiesModel = dynamic_cast<MapGIS::Tile::EntitiesModel*>(pModel);
		if (entitiesModel != NULL)
		{
			i_From(document, scene, bufferBuilder, *entitiesModel, NULL, maxBatchID, type, resetID, resetIDMap, false, m_pStorage, imageInData, centerDot);
			if (entitiesModel->features.size() > 0)
				hasValue = true;
		}
	}

	bufferBuilder.Output(document);
	document.SetDefaultScene(move(scene), AppendIdPolicy::GenerateOnEmpty);
	string manifest;
	try
	{
		manifest = Serialize(document, SerializeFlags::None);
	}
	catch (const GLTFException& ex)
	{
		stringstream ss;

		ss << "Microsoft::glTF::Serialize failed: ";
		ss << ex.what();

		throw runtime_error(ss.str());
	}
	m_data.clear();
	string buf = "";
	glbResourceWriterMemory->GetGLBInfo(manifest, buf);

	if (hasValue)
	{
		switch (compressType)
		{
		case GeoCompressType::Draco:
			Ci_DracoTool::GeometryCompress(buf, m_data);
			break;
		case GeoCompressType::MeshOpt:
			MeshOpt_Gltfpack::GeometryCompress(buf, m_data);
			break;
		default:
			break;
		}
	}
	if (m_data.isEmpty())
	{
		m_data.append(buf.c_str(), buf.length());
	}
	return 1;
}

gisLONG Ci_ModelGltf::From(vector<ContentBase*> *pModel, GeoCompressType compressType, WriteIdType type, bool resetID, map<gisINT64, gisINT64>& resetIDMap,D_3DOT centerDot, bool addEXT_mesh_gpu_instancing, bool imageInData)
{
	if (pModel == NULL || pModel->size() <= 0)
		return -1;

	resetIDMap.clear();
	bool hasValue = false;
	unsigned int maxBatchID = 0;
	Microsoft::glTF::Scene   scene;
#ifdef _WIN32
	unique_ptr<iostream>				tempBufferStream = make_unique<stringstream>();
	unique_ptr<GLBResourceWriterMemory> resourceWriter = make_unique<GLBResourceWriterMemory>(move(tempBufferStream));
#else
	std::unique_ptr<std::stringstream> tempBufferStream(new std::stringstream());
	std::unique_ptr<GLBResourceWriterMemory> resourceWriter(new GLBResourceWriterMemory(std::move(tempBufferStream)));
#endif
	Microsoft::glTF::Document			document;
	Microsoft::glTF::BufferBuilder		bufferBuilder(move(resourceWriter));
	GLBResourceWriterMemory				*glbResourceWriterMemory = dynamic_cast<GLBResourceWriterMemory*>(&bufferBuilder.GetResourceWriter());
	if (glbResourceWriterMemory == NULL)
		return 0;
	//转换
	bufferBuilder.AddBuffer(GLB_BUFFER_ID);
	bool hasPoint = false;
	for (vector<ContentBase*>::iterator itr = pModel->begin(); itr != pModel->end(); itr++)
	{
		if (*itr == NULL)
			continue;
		GeometryContent*  pGeoContent = dynamic_cast<GeometryContent*>(*itr);
		if (pGeoContent == NULL)
			continue;
		G3DModel* p3DModel = pGeoContent->Get3DModel();
		vector<ModelInstance>* pInstance = pGeoContent->GetModelInstance();
		map<gisINT64, gisINT64> resetIDMapitem;
		if (p3DModel !=NULL &&  p3DModel->GetGeometryType() == GeometryType::Point)
		{
			hasPoint = true;
			MapGIS::Tile::PointsModel * pointModel = dynamic_cast<MapGIS::Tile::PointsModel *>(p3DModel);
			if (pointModel != NULL)
			{
				i_From(document, scene, bufferBuilder, *pointModel, pInstance, maxBatchID, type, resetID, resetIDMap, addEXT_mesh_gpu_instancing, centerDot);
				if (pointModel->features.size() > 0)
					hasValue = true;
			}
		}
		else if (p3DModel != NULL && p3DModel->GetGeometryType() == GeometryType::Line)
		{
			MapGIS::Tile::LinesModel * lineModel = dynamic_cast<MapGIS::Tile::LinesModel *>(p3DModel);
			if (lineModel != NULL)
			{
				i_From(document, scene, bufferBuilder, *lineModel, pInstance, maxBatchID, type, resetID, resetIDMap, addEXT_mesh_gpu_instancing, centerDot);
				if (lineModel->features.size() > 0)
					hasValue = true;
			}
		}
		else if (p3DModel != NULL && p3DModel->GetGeometryType() == GeometryType::Surface)
		{
			MapGIS::Tile::SurfacesModel* surfacesModel = dynamic_cast<MapGIS::Tile::SurfacesModel*>(p3DModel);
			if (surfacesModel != NULL)
			{
				i_From(document, scene, bufferBuilder, *surfacesModel, pInstance, maxBatchID, type, resetID, resetIDMap, addEXT_mesh_gpu_instancing, m_pStorage, imageInData, centerDot);
				if (surfacesModel->features.size() > 0)
					hasValue = true;
			}
		}
		else if (p3DModel != NULL && p3DModel->GetGeometryType() == GeometryType::Entity)
		{
			MapGIS::Tile::EntitiesModel* entitiesModel = dynamic_cast<MapGIS::Tile::EntitiesModel*>(p3DModel);
			if (entitiesModel != NULL)
			{
				i_From(document, scene, bufferBuilder, *entitiesModel, pInstance, maxBatchID, type, resetID, resetIDMap, addEXT_mesh_gpu_instancing, m_pStorage,imageInData, centerDot);
				if (entitiesModel->features.size() > 0)
					hasValue = true;
			}
		}
	}

	bufferBuilder.Output(document);
	document.SetDefaultScene(move(scene), AppendIdPolicy::GenerateOnEmpty);
	string manifest;
	try
	{
		manifest = Microsoft::glTF::Serialize(document, SerializeFlags::None);
	}
	catch (const GLTFException& ex)
	{
		stringstream ss;
		ss << "Microsoft::glTF::Serialize failed: ";
		ss << ex.what();
		throw runtime_error(ss.str());
	}
	string buf = "";
	string outbuf = "";
	m_data.clear();
	glbResourceWriterMemory->GetGLBInfo(manifest, buf);

	//点不支持Draco压缩。
	if (hasPoint && compressType == GeoCompressType::Draco)
		compressType = GeoCompressType::None;

	if (hasValue)
	{
		switch (compressType)
		{
		case GeoCompressType::Draco:
			Ci_DracoTool::GeometryCompress(buf, m_data);
			break;
		case GeoCompressType::MeshOpt:
			MeshOpt_Gltfpack::GeometryCompress(buf, m_data);
			break;
		default:
			break;
		}
	}
	if (m_data.isEmpty())
	{
		m_data.append(buf.c_str(), buf.length());
	}
	return 1;
}

gisLONG Ci_ModelGltf::From(vector<ContentBase*> *pModel, GeoCompressType compressType, WriteIdType type, bool addEXT_mesh_gpu_instancing, D_3DOT centerDot, bool imageInData)
{
	map<gisINT64, gisINT64> resetIDMap;
	return From(pModel, compressType, type, false, resetIDMap, centerDot, addEXT_mesh_gpu_instancing, imageInData);
}

gisLONG Ci_ModelGltf::FromResetID(vector<ContentBase*> *pModel, GeoCompressType compressType, WriteIdType type, map<gisINT64, gisINT64>& resetIDMap, D_3DOT centerDot , bool addEXT_mesh_gpu_instancing, bool imageInData)
{
	return From(pModel, compressType, type, true, resetIDMap, centerDot, addEXT_mesh_gpu_instancing, imageInData);
}

gisLONG i_To(const Document& document, const GLBResourceReader& resourceReader, const MeshPrimitive *pMeshPrimitive, Matrix4D& gltfMatrix, MapGIS::Tile::EntitiesModel &entityModel, bool onlyLoadData)
{
	if (pMeshPrimitive->mode != MeshMode::MESH_TRIANGLES)
		return 0;
	if (!onlyLoadData)
	{
		entityModel.samplers.reserve(document.samplers.Size());
		for (const auto& gltfSampler : document.samplers.Elements())
		{
			MapGIS::Tile::Sampler sampler;
			GltfSamplerToSampler(gltfSampler, sampler);
			entityModel.samplers.push_back(sampler);
		}

		entityModel.images.reserve(document.images.Size());
		for (const auto& gltfImage : document.images.Elements())
		{
			MapGIS::Tile::Image  image;
			GltfImageToImage(document, resourceReader, gltfImage, image);
			entityModel.images.push_back(image);
		}

		entityModel.textures.reserve(document.textures.Size());
		for (const auto& gltfTexture : document.textures.Elements())
		{
			MapGIS::Tile::Texture texture;
			GltfTextureToTexture(document, gltfTexture, texture);
			entityModel.textures.push_back(texture);
		}

		entityModel.materials.reserve(document.materials.Size());
		for (const auto& gltfMaterial : document.materials.Elements())
		{
			MapGIS::Tile::Material material;
			GltfMaterialToMaterial(document, gltfMaterial, material);
			entityModel.materials.push_back(material);
		}
	}

	MapGIS::Tile::EntityFeature		feature;
	gisINT64						ID = -1;
	vector<gisINT64>				IDs;
	CAnySurface						anySurface;

	gisLONG materialIndex = Ci_GltfSdkTool::FindMaterialIndex(document, pMeshPrimitive->materialId);

	GltfMeshToAnySurface(document, resourceReader, *pMeshPrimitive, gltfMatrix, anySurface);

	GltfMeshToID(document, resourceReader, *pMeshPrimitive, ID, IDs);
	feature.id = ID;
	feature.ids.swap(IDs);
	bool existOID = false;
	if (feature.ids.size() <= 0)
	{
		for (gisLONG i = 0; i < entityModel.features.size(); i++)
		{
			if (entityModel.features[i].id == feature.id)
			{
				CAnySurface* newSurface = entityModel.features[i].entity.AppendSurface();
				*newSurface = anySurface;
				entityModel.features[i].materialIndexArray.push_back(materialIndex);
				existOID = true;
			}
		}
	}

	if (!existOID)
	{
		CAnySurface* newSurface = feature.entity.AppendSurface();
		*newSurface = anySurface;
		feature.materialIndexArray.push_back(materialIndex);
		entityModel.features.push_back(feature);
	}
	return 1;
}

gisLONG i_To(const Document& document, const GLBResourceReader& resourceReader, const MeshPrimitive *pMeshPrimitive, Matrix4D& gltfMatrix, MapGIS::Tile::SurfacesModel &surfaceModel, bool onlyLoadData)
{

	if (pMeshPrimitive->mode != MeshMode::MESH_TRIANGLES)
		return 0;

	if (!onlyLoadData)
	{
		surfaceModel.samplers.reserve(document.samplers.Size());
		for (const auto& gltfSampler : document.samplers.Elements())
		{
			MapGIS::Tile::Sampler sampler;
			GltfSamplerToSampler(gltfSampler, sampler);
			surfaceModel.samplers.push_back(sampler);
		}

		surfaceModel.images.reserve(document.images.Size());
		for (const auto& gltfImage : document.images.Elements())
		{
			MapGIS::Tile::Image  image;
			GltfImageToImage(document, resourceReader, gltfImage, image);
			surfaceModel.images.push_back(image);
		}

		surfaceModel.textures.reserve(document.textures.Size());
		for (const auto& gltfTexture : document.textures.Elements())
		{
			MapGIS::Tile::Texture texture;
			GltfTextureToTexture(document, gltfTexture, texture);
			surfaceModel.textures.push_back(texture);
		}

		surfaceModel.materials.reserve(document.materials.Size());
		for (const auto& gltfMaterial : document.materials.Elements())
		{
			MapGIS::Tile::Material material;
			GltfMaterialToMaterial(document, gltfMaterial, material);
			surfaceModel.materials.push_back(material);
		}
	}
	SurfaceFeature surfaceFeature;
	MapGIS::Tile::Mesh mesh;
	mesh.materialIndex = Ci_GltfSdkTool::FindMaterialIndex(document, pMeshPrimitive->materialId);
	if (!GltfMeshToAnySurface(document, resourceReader, *pMeshPrimitive, gltfMatrix, mesh.surface))
		return false;
	surfaceFeature.meshes.emplace_back(mesh);
	gisINT64 ID = -1;
	vector<gisINT64> IDs;
	GltfMeshToID(document, resourceReader, *pMeshPrimitive, ID, IDs);
	surfaceFeature.id = ID;
	surfaceFeature.ids.swap(IDs);
	surfaceModel.features.emplace_back(surfaceFeature);
	return 1;
}

gisLONG i_To(const Document& document, const GLBResourceReader& resourceReader, const  MeshPrimitive *pMeshPrimitive, Matrix4D& gltfMatrix, MapGIS::Tile::LinesModel &lineModel)
{
	vector<unsigned char> gltfOID;
	vector<float> gltfPosition;
	vector<float> gltfColor;
	string accessorId;
	vector<gisINT64> gltfIDs;
	vector<unsigned int> gltfIndices;

	if ((pMeshPrimitive->mode != MeshMode::MESH_LINES) &&(pMeshPrimitive->mode != MeshMode::MESH_LINE_LOOP)  && (pMeshPrimitive->mode != MeshMode::MESH_LINE_STRIP))
		return 0;
	if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_POSITION, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);

		gltfPosition = ReadBinaryData<float>(resourceReader, document, accessor);
	}
	gltfColor = GetGltfCOLOR(document, resourceReader, *pMeshPrimitive);
	/*if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_COLOR_0, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_BYTE)
		{
			vector<unsigned char>	tempColor = ReadBinaryData<unsigned char>(resourceReader, document, accessor);
			for (vector<unsigned char>::iterator itr = tempColor.begin(); itr != tempColor.end(); itr++)
				gltfColor.emplace_back(*itr / 255.0);
		}
		else
		{
			gltfColor = ReadBinaryData<float>(resourceReader, document, accessor);
		}
	}*/

	if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_OID, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		gltfOID = ReadBinaryData<unsigned char>(resourceReader, document, accessor);

		if (gltfOID.size() >= 4 && gltfOID.size() % 4 == 0)
		{
			for (int i = 0; i < gltfOID.size(); i += 4)
			{
				gisINT64 ID = RGBA_M(gltfOID[i], gltfOID[i + 1], gltfOID[i + 2], gltfOID[i + 3]);
				gltfIDs.emplace_back(ID);
			}
		}
	}
	else if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_BatchID, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		if (accessor.componentType == ComponentType::COMPONENT_FLOAT)
		{
			vector<float> gltfBatchID;
			gltfBatchID = ReadBinaryData<float>(resourceReader, document, accessor);

			for (int i = 0; i < gltfBatchID.size(); i++)
			{
				gltfIDs.emplace_back(gltfBatchID[i]);
			}
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
		{
			vector<gisUSHORT> gltfBatchID;
			gltfBatchID = ReadBinaryData<gisUSHORT>(resourceReader, document, accessor);
			for (int i = 0; i < gltfBatchID.size(); i++)
			{
				gltfIDs.emplace_back(gltfBatchID[i]);
			}
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_INT)
		{
			vector<gisUINT> gltfBatchID;
			gltfBatchID = ReadBinaryData<gisUINT>(resourceReader, document, accessor);
			for (int i = 0; i < gltfBatchID.size(); i++)
			{
				gltfIDs.emplace_back(gltfBatchID[i]);
			}
		}
	}

	{
		const Accessor& accessor = document.accessors.Get(pMeshPrimitive->indicesAccessorId);
		if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_BYTE)
		{
			vector<char> indices = ReadBinaryData<char>(resourceReader, document, accessor);

			for (int i = 0; i < indices.size(); i++)
			{
				gltfIndices.emplace_back(indices[i]);
			}
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
		{
			vector<gisUSHORT> indices;
			indices = ReadBinaryData<gisUSHORT>(resourceReader, document, accessor);
			for (int i = 0; i < indices.size(); i++)
			{
				gltfIndices.emplace_back(indices[i]);
			}
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_INT)
		{
			gltfIndices = ReadBinaryData<gisUINT>(resourceReader, document, accessor);
		}
	}
	vector<D_3DOT> M3DDots;
	for (gisLONG i = 0; i < gltfPosition.size(); i += 3)
	{
		D_3DOT dot = gltfMatrix*D_3DOT{ gltfPosition[i] ,gltfPosition[i + 1] ,gltfPosition[i + 2] };
		M3DDots.push_back(D_3DOT{ dot.x,-dot.z,dot.y });
	}

	vector<CVarLine> lines;
	vector<gisINT64> IDs;
	vector<MapGIS::Tile::Color4f> colors;

	CVarLine line;
	MapGIS::Tile::Color4f color;
	if (pMeshPrimitive->mode == MESH_LINE_STRIP || pMeshPrimitive->mode == MESH_LINE_LOOP)
	{
		if (gltfIDs.size() > 0)
			IDs.emplace_back(gltfIDs[0]);
		else
			IDs.emplace_back(0);
		vector<D_3DOT> newDots;
		for (int i = 0; i < gltfIndices.size(); i++)
		{
			if (gltfIndices[i] < M3DDots.size())
			{
				newDots.emplace_back(M3DDots[gltfIndices[i]]);
			}
		}
		if (pMeshPrimitive->mode == MESH_LINE_LOOP && newDots.size() > 0)
			newDots.emplace_back(newDots[0]);
		line.Set(&newDots[0], newDots.size(), 3);
		lines.emplace_back(line);
		if (gltfColor.size() >= 4)
		{
			color.r = gltfColor[0];
			color.g = gltfColor[1];
			color.b = gltfColor[2];
			color.a = gltfColor[3];
			colors.emplace_back(color);
		}
	}
	else if (pMeshPrimitive->mode == MESH_LINES)
	{
		D_3DOT newDots[2];
		for (int i = 0; i + 1 < gltfIndices.size(); i += 2)
		{
			if (gltfIndices[i] < M3DDots.size() && gltfIndices[i + 1] < M3DDots.size())
			{
				if (gltfIDs.size() > i)
					IDs.emplace_back(gltfIDs[i]);
				else
					IDs.emplace_back(0);

				newDots[0] = M3DDots[gltfIndices[i]];
				newDots[1] = M3DDots[gltfIndices[i + 1]];
				line.Set(&newDots[0], 2, 3);
				lines.emplace_back(line);
				if (gltfColor.size() / 4 == M3DDots.size())
				{
					color.r = gltfColor[gltfIndices[i] * 4];
					color.g = gltfColor[gltfIndices[i] * 4 + 1];
					color.b = gltfColor[gltfIndices[i] * 4 + 2];
					color.a = gltfColor[gltfIndices[i] * 4 + 3];
					colors.emplace_back(color);
				}
			}
		}
	}

	float r = 0, g = 0, b = 0, a = 0, r1 = 0, g1 = 0, b1 = 0, a1 = 0;
	accessorId = pMeshPrimitive->materialId;
	if (!pMeshPrimitive->materialId.empty())
	{
		auto material = document.materials[accessorId];
		r = material.metallicRoughness.baseColorFactor.r;
		g = material.metallicRoughness.baseColorFactor.g;
		b = material.metallicRoughness.baseColorFactor.b;
		a = material.metallicRoughness.baseColorFactor.a;
	}
	unordered_map<gisINT64, int> oidIndex;
	oidIndex.reserve(lines.size());
	for (int i = 0; i < lines.size(); i++)
	{
		if (!pMeshPrimitive->materialId.empty())
		{
			r1 = r;
			g1 = g;
			b1 = b;
			a1 = a;
		}
		else  if (colors.size() == lines.size())
		{
			r1 = colors[i].r;
			g1 = colors[i].g;
			b1 = colors[i].b;
			a1 = colors[i].a;
		}
		bool existOID = false;
		if (oidIndex.find(IDs[i]) != oidIndex.end())
		{
			bool isExist = false;
			for (vector<ColorLine>::iterator colorLineItr = lineModel.features[oidIndex[IDs[i]]].colorLines.begin(); colorLineItr != lineModel.features[oidIndex[IDs[i]]].colorLines.end(); colorLineItr++)
			{
				if (IsEqual(colorLineItr->color.r, r1) && IsEqual(colorLineItr->color.g, g1) && IsEqual(colorLineItr->color.b, b1) && IsEqual(colorLineItr->color.a, a1))
				{
					colorLineItr->lines.emplace_back(lines[i]);
					isExist = true;
					break;
				}
			}
			if (!isExist)
			{
				ColorLine colorLine;
				colorLine.lines.push_back(lines[i]);
				colorLine.color.r = r1;
				colorLine.color.g = g1;
				colorLine.color.b = b1;
				colorLine.color.a = a1;
				lineModel.features[oidIndex[IDs[i]]].colorLines.push_back(colorLine);
			}
			existOID = true;
		}

		if (!existOID)
		{
			MapGIS::Tile::ColorLine colorLine;
			colorLine.lines.push_back(lines[i]);
			colorLine.color.r = r1;
			colorLine.color.g = g1;
			colorLine.color.b = b1;
			colorLine.color.a = a1;

			MapGIS::Tile::LineFeature feature;
			feature.id = IDs[i];
			feature.colorLines.push_back(colorLine);
			lineModel.features.push_back(feature);
			oidIndex.insert(make_pair(IDs[i], lineModel.features.size() - 1));
		}
	}
	return 1;
}

gisLONG i_To(const Document& document, const GLBResourceReader& resourceReader, const MeshPrimitive *pMeshPrimitive, Matrix4D& gltfMatrix, MapGIS::Tile::PointsModel &pointModel)
{
	vector<unsigned char> gltfOID;
	vector<float> gltfPosition;
	vector<float> gltfColor;
	string accessorId;
	vector<gisINT64> gltfIDs;

	if (pMeshPrimitive->mode != MeshMode::MESH_POINTS)
		return 0;
	if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_POSITION, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		gltfPosition = ReadBinaryData<float>(resourceReader, document, accessor);
	}
	gltfColor = GetGltfCOLOR(document, resourceReader, *pMeshPrimitive);
	/*if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_COLOR_0, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_BYTE)
		{
			vector<unsigned char>	tempColor = ReadBinaryData<unsigned char>(resourceReader, document, accessor);
			for (vector<unsigned char>::iterator itr = tempColor.begin(); itr != tempColor.end(); itr++)
				gltfColor.emplace_back(*itr / 255.0);
		}
		else
		{
			gltfColor = ReadBinaryData<float>(resourceReader, document, accessor);
		}
	}*/

	if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_OID, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		gltfOID = ReadBinaryData<unsigned char>(resourceReader, document, accessor);

		if (gltfOID.size() >= 4 && gltfOID.size() % 4 == 0)
		{
			for (int i = 0; i < gltfOID.size(); i += 4)
			{
				gisINT64 ID = RGBA_M(gltfOID[i], gltfOID[i + 1], gltfOID[i + 2], gltfOID[i + 3]);
				gltfIDs.emplace_back(ID);
			}
		}
	}
	else if (pMeshPrimitive->TryGetAttributeAccessorId(ACCESSOR_BatchID, accessorId))
	{
		const Accessor& accessor = document.accessors.Get(accessorId);
		if (accessor.componentType == ComponentType::COMPONENT_FLOAT)
		{
			vector<float> gltfBatchID;
			gltfBatchID = ReadBinaryData<float>(resourceReader, document, accessor);

			for (int i = 0; i < gltfBatchID.size(); i++)
			{
				gltfIDs.emplace_back(gltfBatchID[i]);
			}
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
		{
			vector<gisUSHORT> gltfBatchID;
			gltfBatchID = ReadBinaryData<gisUSHORT>(resourceReader, document, accessor);
			for (int i = 0; i < gltfBatchID.size(); i++)
			{
				gltfIDs.emplace_back(gltfBatchID[i]);
			}
		}
		else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_INT)
		{
			vector<gisUINT> gltfBatchID;
			gltfBatchID = ReadBinaryData<gisUINT>(resourceReader, document, accessor);
			for (int i = 0; i < gltfBatchID.size(); i++)
			{
				gltfIDs.emplace_back(gltfBatchID[i]);
			}
		}
	}

	vector<D_3DOT> M3DDots;
	for (gisLONG i = 0; i < gltfPosition.size(); i += 3)
	{
		D_3DOT dot = gltfMatrix*D_3DOT{ gltfPosition[i] ,gltfPosition[i + 1] ,gltfPosition[i + 2] };
		M3DDots.push_back(D_3DOT{ dot.x,-dot.z,dot.y });
	}

	float r = 0, g = 0, b = 0, a = 0, r1 = 0, g1 = 0, b1 = 0, a1 = 0;
	accessorId = pMeshPrimitive->materialId;
	if (!pMeshPrimitive->materialId.empty())
	{
		auto material = document.materials[accessorId];
		r = material.metallicRoughness.baseColorFactor.r;
		g = material.metallicRoughness.baseColorFactor.g;
		b = material.metallicRoughness.baseColorFactor.b;
		a = material.metallicRoughness.baseColorFactor.a;
	}

	gisINT64 id = 0;
	pointModel.features.reserve(gltfIDs.size());

	PointFeature feature;
	ColorPoint colorPoint;
	unordered_map<gisINT64, int> oidIndex;
	oidIndex.reserve(gltfIDs.size());
	for (int i = 0; i < M3DDots.size(); i++)
	{
		feature.id = -1;
		feature.colorPoints.clear();
		if (gltfIDs.size() == M3DDots.size())
			id = gltfIDs[i];

		if (gltfColor.size() / 4 == M3DDots.size())
		{
			r1 = gltfColor[i * 4];
			g1 = gltfColor[i * 4 + 1];
			b1 = gltfColor[i * 4 + 2];
			a1 = gltfColor[i * 4 + 3];
		}
		else
		{
			r1 = r;
			g1 = g;
			b1 = b;
			a1 = a;
		}

		bool existOID = false;
		if (oidIndex.find(id) != oidIndex.end())
		{
			bool isExist = false;
			for (vector<ColorPoint>::iterator colorPointItr = pointModel.features[oidIndex[id]].colorPoints.begin(); colorPointItr != pointModel.features[oidIndex[id]].colorPoints.end(); colorPointItr++)
			{
				if (IsEqual(colorPointItr->color.r, r1) && IsEqual(colorPointItr->color.g, g1) && IsEqual(colorPointItr->color.b, b1) && IsEqual(colorPointItr->color.a, a1))
				{
					colorPointItr->points.Append(M3DDots[i]);
					isExist = true;
					break;
				}
			}
			if (!isExist)
			{
				colorPoint.points.Set(&M3DDots[i], 1);
				colorPoint.color.r = r1;
				colorPoint.color.g = g1;
				colorPoint.color.b = b1;
				colorPoint.color.a = a1;
				pointModel.features[oidIndex[id]].colorPoints.emplace_back(colorPoint);
			}
			existOID = true;
		}

		if (!existOID)
		{
			colorPoint.points.Set(&M3DDots[i], 1);
			colorPoint.color.r = r1;
			colorPoint.color.g = g1;
			colorPoint.color.b = b1;
			colorPoint.color.a = a1;
			feature.id = id;
			feature.colorPoints.push_back(colorPoint);
			pointModel.features.push_back(feature);
			oidIndex.insert(make_pair(id, pointModel.features.size() - 1));
		}
	}
	return 1;
}

bool  GltfNodeToSurfacesModel(const Document& document, const GLBResourceReader& resourceReader, const string nodeId, Matrix4D gltfMatrix, MapGIS::Tile::SurfacesModel &surfaceModel)
{
	gisLONG nodeIndex = Ci_GltfSdkTool::FindNodeIndex(document, nodeId);
	if (nodeIndex < 0 || nodeIndex >= document.nodes.Size())
	{
		return false;
	}
	Matrix4D tempMatrix;
	GltfNodeMatrix(document, nodeIndex, tempMatrix);
	gltfMatrix = gltfMatrix* tempMatrix;

	string meshId = document.nodes[nodeIndex].meshId;
	gisLONG meshIndex = Ci_GltfSdkTool::FindMesheIndex(document, meshId);
	if (meshIndex >= 0 && meshIndex < document.meshes.Size())
	{
		const auto& gltfMesh = document.meshes[meshIndex];
		for (const auto& meshPrimitive : gltfMesh.primitives)
		{
			SurfaceFeature surfaceFeature;
			MapGIS::Tile::Mesh mesh;
			mesh.materialIndex = Ci_GltfSdkTool::FindMaterialIndex(document, meshPrimitive.materialId);
			if (!GltfMeshToAnySurface(document, resourceReader, meshPrimitive, gltfMatrix, mesh.surface))
				return false;
			surfaceFeature.meshes.emplace_back(mesh);
			gisINT64 ID = -1;
			vector<gisINT64> IDs;
			GltfMeshToID(document, resourceReader, meshPrimitive, ID, IDs);
			surfaceFeature.id = ID;
			surfaceFeature.ids.swap(IDs);
			surfaceModel.features.emplace_back(surfaceFeature);
		}
	}
	std::vector<std::string>  children = document.nodes[nodeIndex].children;
	if (children.size() > 0)
	{
		for (std::vector<std::string>::iterator itr = children.begin(); itr != children.end(); itr++)
		{
			if (!GltfNodeToSurfacesModel(document, resourceReader, *itr, gltfMatrix, surfaceModel))
				return false;
		}
	}
	return true;
}

bool  GltfNodeToEntitiesModel(const Document& document, const GLBResourceReader& resourceReader, const string nodeId, Matrix4D gltfMatrix, MapGIS::Tile::EntitiesModel &entityModel)
{
	gisLONG nodeIndex = Ci_GltfSdkTool::FindNodeIndex(document, nodeId);
	if (nodeIndex < 0 || nodeIndex >= document.nodes.Size())
	{
		return false;
	}
	Matrix4D tempMatrix;
	GltfNodeMatrix(document, nodeIndex, tempMatrix);
	gltfMatrix = gltfMatrix* tempMatrix;

	string meshId = document.nodes[nodeIndex].meshId;
	gisLONG meshIndex = Ci_GltfSdkTool::FindMesheIndex(document, meshId);
	if (meshIndex >= 0 && meshIndex < document.meshes.Size())
	{
		const auto& gltfMesh = document.meshes[meshIndex];
		for (const auto& meshPrimitive : gltfMesh.primitives)
		{
			MapGIS::Tile::EntityFeature		feature;
			gisINT64				ID = -1;
			vector<gisINT64>		IDs;
			CAnySurface			anySurface;

			gisLONG materialIndex = Ci_GltfSdkTool::FindMaterialIndex(document, meshPrimitive.materialId);

			GltfMeshToAnySurface(document, resourceReader, meshPrimitive, gltfMatrix, anySurface);

			GltfMeshToID(document, resourceReader, meshPrimitive, ID, IDs);
			feature.id = ID;
			feature.ids.swap(IDs);

			bool existOID = false;
			if (feature.ids.size() <= 0)
			{
				for (gisLONG i = 0; i < entityModel.features.size(); i++)
				{
					if (entityModel.features[i].id == feature.id)
					{
						CAnySurface* newSurface = entityModel.features[i].entity.AppendSurface();
						*newSurface = anySurface;
						entityModel.features[i].materialIndexArray.push_back(materialIndex);
						existOID = true;
					}
				}
			}

			if (!existOID)
			{
				CAnySurface* newSurface = feature.entity.AppendSurface();
				*newSurface = anySurface;
				feature.materialIndexArray.push_back(materialIndex);
				entityModel.features.push_back(feature);
			}
		}
	}
	std::vector<std::string>  children = document.nodes[nodeIndex].children;
	if (children.size() > 0)
	{
		for (std::vector<std::string>::iterator itr = children.begin(); itr != children.end(); itr++)
		{
			if (!GltfNodeToEntitiesModel(document, resourceReader, *itr, gltfMatrix, entityModel))
				return false;
		}
	}
	return true;
}

bool  GltfNodeToLinesModel(const Document& document, const GLBResourceReader& resourceReader, const string nodeId, Matrix4D gltfMatrix, MapGIS::Tile::LinesModel &lineModel)
{
	gisLONG nodeIndex = Ci_GltfSdkTool::FindNodeIndex(document, nodeId);
	if (nodeIndex < 0 || nodeIndex >= document.nodes.Size())
	{
		return false;
	}
	Matrix4D tempMatrix;
	GltfNodeMatrix(document, nodeIndex, tempMatrix);
	gltfMatrix = gltfMatrix* tempMatrix;

	string meshId = document.nodes[nodeIndex].meshId;
	gisLONG meshIndex = Ci_GltfSdkTool::FindMesheIndex(document, meshId);
	if (meshIndex >= 0 && meshIndex < document.meshes.Size())
	{
		const auto& gltfMesh = document.meshes[meshIndex];
		for (const auto& meshPrimitive : gltfMesh.primitives)
		{
			i_To(document, resourceReader, &meshPrimitive, gltfMatrix, lineModel);
		}
	}
	std::vector<std::string>  children = document.nodes[nodeIndex].children;
	if (children.size() > 0)
	{
		for (std::vector<std::string>::iterator itr = children.begin(); itr != children.end(); itr++)
		{
			if (!GltfNodeToLinesModel(document, resourceReader, *itr, gltfMatrix, lineModel))
				return false;
		}
	}
	return true;
}

bool  GltfNodeToPointsModel(const Document& document, const GLBResourceReader& resourceReader, const string nodeId, Matrix4D gltfMatrix, MapGIS::Tile::PointsModel &pointModel)
{
	gisLONG nodeIndex = Ci_GltfSdkTool::FindNodeIndex(document, nodeId);
	if (nodeIndex < 0 || nodeIndex >= document.nodes.Size())
	{
		return false;
	}
	Matrix4D tempMatrix;
	GltfNodeMatrix(document, nodeIndex, tempMatrix);
	gltfMatrix = gltfMatrix* tempMatrix;

	string meshId = document.nodes[nodeIndex].meshId;
	gisLONG meshIndex = Ci_GltfSdkTool::FindMesheIndex(document, meshId);
	if (meshIndex >= 0 && meshIndex < document.meshes.Size())
	{
		const auto& gltfMesh = document.meshes[meshIndex];
		for (const auto& meshPrimitive : gltfMesh.primitives)
		{
			i_To(document, resourceReader, &meshPrimitive, gltfMatrix, pointModel);
		}
	}
	std::vector<std::string>  children = document.nodes[nodeIndex].children;
	if (children.size() > 0)
	{
		for (std::vector<std::string>::iterator itr = children.begin(); itr != children.end(); itr++)
		{
			if (!GltfNodeToPointsModel(document, resourceReader, *itr, gltfMatrix, pointModel))
				return false;
		}
	}
	return true;
}

gisLONG Ci_ModelGltf::To(MapGIS::Tile::EntitiesModel &entityModel)
{
	if (m_data.size() <= 0)
		return  0;

	string buffer = "";
	buffer.append(m_data.data(), m_data.length());
	size_t index = buffer.find("KHR_draco_mesh_compression");
	if (index != string::npos)
	{
		Ci_DracoTool::GeometryDecompression(buffer, buffer);
	}
	else
	{
		index = buffer.find("EXT_meshopt_compression");
		if (index != string::npos)
		{
			MeshOpt_Gltfpack::GeometryDecompression(buffer, buffer);
		}
	}
	int bytesLength = ReadBufferToInt32(buffer, 8);

	Document document;
	unique_ptr<GLBResourceReader> resourceReader;
	stringstream m_istr;
	m_istr.str("");
	m_istr.write((char* const)(&buffer[0]), bytesLength);
	if (!ReadGLTFInfo(m_istr, document, resourceReader, m_pStorage))
		return 0;

	entityModel.samplers.reserve(document.samplers.Size());
	for (const auto& gltfSampler : document.samplers.Elements())
	{
		MapGIS::Tile::Sampler sampler;
		GltfSamplerToSampler(gltfSampler, sampler);
		entityModel.samplers.push_back(sampler);
	}

	entityModel.images.reserve(document.images.Size());
	for (const auto& gltfImage : document.images.Elements())
	{
		MapGIS::Tile::Image  image;
		GltfImageToImage(document, *resourceReader, gltfImage, image);
		entityModel.images.push_back(image);
	}

	entityModel.textures.reserve(document.textures.Size());
	for (const auto& gltfTexture : document.textures.Elements())
	{
		MapGIS::Tile::Texture texture;
		GltfTextureToTexture(document, gltfTexture, texture);
		entityModel.textures.push_back(texture);
	}

	entityModel.materials.reserve(document.materials.Size());
	for (const auto& gltfMaterial : document.materials.Elements())
	{
		MapGIS::Tile::Material material;
		GltfMaterialToMaterial(document, gltfMaterial, material);
		entityModel.materials.push_back(material);
	}

	const auto& gltfScene = document.scenes[0];
	for (const auto& gltfNodeId : gltfScene.nodes)
	{
		Matrix4D gltfMatrix;
		Matrix4Unit(gltfMatrix);
		GltfNodeToEntitiesModel(document, *resourceReader, gltfNodeId, gltfMatrix, entityModel);
	}
	return 1;
}

gisLONG Ci_ModelGltf::To(MapGIS::Tile::SurfacesModel &surfaceModel)
{
	if (m_data.size() <= 0)
		return  0;

	string buffer = "";
	buffer.append(m_data.data(), m_data.length());
	size_t index = buffer.find("KHR_draco_mesh_compression");
	if (index != string::npos)
	{
		Ci_DracoTool::GeometryDecompression(buffer, buffer);
	}
	else
	{
		index = buffer.find("EXT_meshopt_compression");
		if (index != string::npos)
		{
			MeshOpt_Gltfpack::GeometryDecompression(buffer, buffer);
		}
	}
	int bytesLength = ReadBufferToInt32(buffer, 8);

	Document document;
	stringstream m_istr;
	m_istr.str("");
	m_istr.write((char* const)(&buffer[0]), bytesLength);

	unique_ptr<GLBResourceReader> resourceReader;
	if (!ReadGLTFInfo(m_istr, document, resourceReader, m_pStorage))
		return 0;

	surfaceModel.samplers.reserve(document.samplers.Size());
	for (const auto& gltfSampler : document.samplers.Elements())
	{
		MapGIS::Tile::Sampler sampler;
		GltfSamplerToSampler(gltfSampler,sampler);
		surfaceModel.samplers.push_back(sampler);
	}

	surfaceModel.images.reserve(document.images.Size());
	for (const auto& gltfImage : document.images.Elements())
	{
		MapGIS::Tile::Image  image;
		GltfImageToImage(document, *resourceReader, gltfImage, image);
		surfaceModel.images.push_back(image);
	}

	surfaceModel.textures.reserve(document.textures.Size());
	for (const auto& gltfTexture : document.textures.Elements())
	{
		MapGIS::Tile::Texture texture;
		GltfTextureToTexture(document, gltfTexture, texture);
		surfaceModel.textures.push_back(texture);
	}

	surfaceModel.materials.reserve(document.materials.Size());
	for (const auto& gltfMaterial : document.materials.Elements())
	{
		MapGIS::Tile::Material material;
		GltfMaterialToMaterial(document, gltfMaterial, material);
		surfaceModel.materials.push_back(material);
	}

	const auto& gltfScene = document.scenes[0];
	for (const auto& gltfNodeId : gltfScene.nodes)
	{
		Matrix4D gltfMatrix;
		Matrix4Unit(gltfMatrix);
		GltfNodeToSurfacesModel(document, *resourceReader, gltfNodeId, gltfMatrix, surfaceModel);
	}
	return 1;
}

gisLONG Ci_ModelGltf::To(MapGIS::Tile::LinesModel &lineModel)
{
	if (m_data.size() <= 0)
		return  0;
	string buffer = "";
	buffer.append(m_data.data(), m_data.length());
	size_t index = buffer.find("KHR_draco_mesh_compression");
	if (index != string::npos)
	{
		Ci_DracoTool::GeometryDecompression(buffer, buffer);
	}
	else
	{
		index = buffer.find("EXT_meshopt_compression");
		if (index != string::npos)
		{
			MeshOpt_Gltfpack::GeometryDecompression(buffer, buffer);
		}
	}
	int bytesLength = ReadBufferToInt32(buffer, 8);
	Document document;
	unique_ptr<GLBResourceReader> resourceReader;
	stringstream m_istr;
	m_istr.str("");
	m_istr.write((char* const)(&buffer[0]), buffer.size());
	if (!ReadGLTFInfo(m_istr, document, resourceReader, m_pStorage))
		return 0;

	const auto& gltfScene = document.scenes[0];
	for (const auto& gltfNodeId : gltfScene.nodes)
	{
		Matrix4D gltfMatrix;
		Matrix4Unit(gltfMatrix);
		GltfNodeToLinesModel(document, *resourceReader, gltfNodeId, gltfMatrix, lineModel);
	}
	return 1;
}

gisLONG Ci_ModelGltf::To(MapGIS::Tile::PointsModel &pointModel)
{
	if (m_data.size() <= 0)
		return  0;
	string buffer = "";
	buffer.append(m_data.data(), m_data.length());
	size_t index = buffer.find("KHR_draco_mesh_compression");
	if (index != string::npos)
	{
		Ci_DracoTool::GeometryDecompression(buffer, buffer);
	}
	else
	{
		index = buffer.find("EXT_meshopt_compression");
		if (index != string::npos)
		{
			MeshOpt_Gltfpack::GeometryDecompression(buffer, buffer);
		}
	}

	int bytesLength = ReadBufferToInt32(buffer, 8);
	Document document;
	unique_ptr<GLBResourceReader> resourceReader;
	stringstream m_istr;
	m_istr.str("");
	m_istr.write((char* const)(&buffer[0]), buffer.size());
	if (!ReadGLTFInfo(m_istr, document, resourceReader, m_pStorage))
		return 0;
	const auto& gltfScene = document.scenes[0];
	for (const auto& gltfNodeId : gltfScene.nodes)
	{
		Matrix4D gltfMatrix;
		Matrix4Unit(gltfMatrix);
		GltfNodeToPointsModel(document, *resourceReader, gltfNodeId, gltfMatrix, pointModel);
	}
	return 1;
}

void StatistMeshInfo(const Document& document, const GLBResourceReader& resourceReader, const string nodeId, vector<NodeTransitionInfo>  nodeTransitionInfo, vector<GltfMeshInfo> & meshInfos)
{
	gisLONG nodeIndex = Ci_GltfSdkTool::FindNodeIndex(document, nodeId);
	if (nodeIndex < 0 || nodeIndex >= document.nodes.Size())
		return;
	Matrix4D tempMatrix;
	GltfNodeMatrix(document, nodeIndex, tempMatrix);
	NodeTransitionInfo info;
	bool isAdd = false;
	if (!tempMatrix.IsUnit())
	{
		if (nodeTransitionInfo.size() > 0)
		{
			NodeTransitionInfo & backInfo =  nodeTransitionInfo.back();
			if (backInfo.gpuInstancingInfo.isEmpty())
				backInfo.gltfMatrix = backInfo.gltfMatrix * tempMatrix;
			else
			{
				info.gltfMatrix = tempMatrix;
				isAdd = true;
			}
		}
		else
		{
			info.gltfMatrix = tempMatrix;
			isAdd = true;
		}
	}

	if (document.nodes[nodeIndex].extensions.size() > 0 && document.nodes[nodeIndex].extensions.find(Vendor::MeshGpuInstancingInfos::MESH_GPU_INSTANCING_NAME) != document.nodes[nodeIndex].extensions.end())
	{
		ExtensionDeserializer extensionDeserializer;
		std::string json = document.nodes[nodeIndex].extensions.at(Vendor::MeshGpuInstancingInfos::MESH_GPU_INSTANCING_NAME);
		std::unique_ptr<Extension> extension = 	Vendor::MeshGpuInstancingInfos::DeserializeMeshGpuInstancing(json, extensionDeserializer);
		Vendor::MeshGpuInstancingInfos::MeshGpuInstancing*  pGpuInstancing = dynamic_cast<Vendor::MeshGpuInstancingInfos::MeshGpuInstancing*>(extension.get());
		if (pGpuInstancing != NULL)
		{
			if (isAdd)
			{
				info.gpuInstancingInfo.translatton = pGpuInstancing->translatton;
				info.gpuInstancingInfo.rotation = pGpuInstancing->rotation;
				info.gpuInstancingInfo.scale = pGpuInstancing->scale;
				info.gpuInstancingInfo.batchid = pGpuInstancing->batchid;
			}
			else
			{
				if(nodeTransitionInfo.size() > 0)
				{
					NodeTransitionInfo & backInfo = nodeTransitionInfo.back();
					backInfo.gpuInstancingInfo.translatton = pGpuInstancing->translatton;
					backInfo.gpuInstancingInfo.rotation = pGpuInstancing->rotation;
					backInfo.gpuInstancingInfo.scale = pGpuInstancing->scale;
					backInfo.gpuInstancingInfo.batchid = pGpuInstancing->batchid;
				}
				else
				{
					isAdd = true;
					info.gpuInstancingInfo.translatton = pGpuInstancing->translatton;
					info.gpuInstancingInfo.rotation = pGpuInstancing->rotation;
					info.gpuInstancingInfo.scale = pGpuInstancing->scale;
					info.gpuInstancingInfo.batchid = pGpuInstancing->batchid;
				}
			}
		}
	}
	if (isAdd)
		nodeTransitionInfo.emplace_back(info);
	string meshId = document.nodes[nodeIndex].meshId;
	gisLONG meshIndex = Ci_GltfSdkTool::FindMesheIndex(document, meshId);
	if (meshIndex >= 0 && meshIndex < document.meshes.Size())
	{
		const auto& gltfMesh = document.meshes[meshIndex];
		for (const auto& meshPrimitive : gltfMesh.primitives)
		{
			GltfMeshInfo meshInfo;
			meshInfo.meshPrimitive = &meshPrimitive;
			meshInfo.nodeTransitionInfo = nodeTransitionInfo;

			switch (meshPrimitive.mode)
			{
			case MeshMode::MESH_LINES:
			case MeshMode::MESH_LINE_LOOP:
			case MeshMode::MESH_LINE_STRIP:
				meshInfo.type = GeometryType::Line;
				break;
			case MeshMode::MESH_POINTS:
				meshInfo.type = GeometryType::Point;
				break;
			case MeshMode::MESH_TRIANGLES:
				meshInfo.type = GeometryType::Surface;
				break;
			default:
				break;
			}
			if (meshInfo.type != GeometryType::None)
				meshInfos.emplace_back(meshInfo);
		}
	}
	std::vector<std::string>  children = document.nodes[nodeIndex].children;
	if (children.size() > 0)
	{
		for (std::vector<std::string>::iterator itr = children.begin(); itr != children.end(); itr++)
		{
			StatistMeshInfo(document, resourceReader, *itr, nodeTransitionInfo, meshInfos);
		}
	}
}

void MeshInfoGroup(vector<GltfMeshInfo>  meshInfos, vector <vector<GltfMeshInfo>>& groupMeshInfos)
{
	groupMeshInfos.clear();
	vector<int> hasIndex;
	for (int i = 0; i < meshInfos.size(); i++)
	{
		if (find(hasIndex.begin(), hasIndex.end(), i) != hasIndex.end())
			continue;
		vector<GltfMeshInfo> infos;
		infos.emplace_back(meshInfos[i]);
		hasIndex.emplace_back(i);
		for (int j = i + 1; j < meshInfos.size(); j++)
		{
			if (find(hasIndex.begin(), hasIndex.end(), j) != hasIndex.end())
				continue;
			if (meshInfos[i].type == meshInfos[j].type)
			{
				bool isEqual = true;
				if (meshInfos[i].nodeTransitionInfo.size() == meshInfos[j].nodeTransitionInfo.size())
				{
					int transitionNum =  meshInfos[i].nodeTransitionInfo.size();
					for (int k = 0; k < transitionNum; k++)
					{
						if (meshInfos[i].nodeTransitionInfo[k] != meshInfos[j].nodeTransitionInfo[k])
						{
							isEqual = false;
							break;
						}
					}
				}
				else
					isEqual = false;

				if (isEqual)
				{
					infos.emplace_back(meshInfos[j]);
					hasIndex.emplace_back(j);
				}
			}
		}
		groupMeshInfos.emplace_back(infos);
	}
}

void MakeInstanceAndMatrix(const Document& document, const GLBResourceReader& resourceReader, const vector<NodeTransitionInfo> & nodeTransitionInfo,vector<ModelInstance>* pInstance, Matrix4D &matrix)
{
	if (pInstance == NULL)
		return;
	vector<Matrix4D> instanceMatrix;
	vector<gisINT64>  instanceBatchids;
	for (int i = 0; i < nodeTransitionInfo.size(); i++)
	{
		matrix = matrix * nodeTransitionInfo[i].gltfMatrix;
		if (!nodeTransitionInfo[i].gpuInstancingInfo.isEmpty())
		{
			vector<float> instancingTranslatton;
			vector<float> instancingRotation;
			vector<float> instancingScale;
			vector<gisINT64> instancingBatchid;

			int instancingCount = 0;
			bool isError = false;

			if (!nodeTransitionInfo[i].gpuInstancingInfo.translatton.empty())
			{
				const Accessor& accessor = document.accessors.Get(nodeTransitionInfo[i].gpuInstancingInfo.translatton);
				if (accessor.componentType == ComponentType::COMPONENT_FLOAT)
				{
					instancingCount = accessor.count;
					instancingTranslatton = ReadBinaryData<float>(resourceReader, document, accessor);
				}
			}

			if (!nodeTransitionInfo[i].gpuInstancingInfo.rotation.empty())
			{
				const Accessor& accessor = document.accessors.Get(nodeTransitionInfo[i].gpuInstancingInfo.rotation);

				if (accessor.count > 0)
				{
					if (instancingCount > 0 && accessor.count != instancingCount)
						isError = true;
					else
						instancingCount = accessor.count;
				}

				if (accessor.componentType == ComponentType::COMPONENT_FLOAT)
				{
					instancingRotation = ReadBinaryData<float>(resourceReader, document, accessor);
				}
				else if (accessor.componentType == ComponentType::COMPONENT_BYTE && accessor.normalized)
				{
					instancingRotation = GltfDataNormalized(document, resourceReader, accessor);
				}
				else if (accessor.componentType == ComponentType::COMPONENT_SHORT && accessor.normalized)
				{
					instancingRotation = GltfDataNormalized(document, resourceReader, accessor);
				}
			}

			if (!nodeTransitionInfo[i].gpuInstancingInfo.scale.empty())
			{
				const Accessor& accessor = document.accessors.Get(nodeTransitionInfo[i].gpuInstancingInfo.scale);

				if (accessor.count > 0)
				{
					if (instancingCount > 0 && accessor.count != instancingCount)
						isError = true;
					else
						instancingCount = accessor.count;
				}

				if (accessor.componentType == ComponentType::COMPONENT_FLOAT)
				{
					instancingScale = ReadBinaryData<float>(resourceReader, document, accessor);
				}
			}
			vector<Matrix4D> rtn;
			if (!isError &&instancingCount > 0)
			{
				double translattonX = 0;
				double translattonY = 0;
				double translattonZ = 0;

				double quaternionX = 0;
				double quaternionY = 0;
				double quaternionZ = 0;
				double quaternionW = 1;

				double scalingX = 1;
				double scalingY = 1;
				double scalingZ = 1;

				for (int j = 0; j < instancingCount; j++)
				{
					translattonX = 0;
					translattonY = 0;
					translattonZ = 0;

					quaternionX = 0;
					quaternionY = 0;
					quaternionZ = 0;
					quaternionW = 1;

					scalingX = 1;
					scalingY = 1;
					scalingZ = 1;
					if (instancingTranslatton.size() > j * 3 + 2)
					{
						translattonX = instancingTranslatton[j * 3];
						translattonY = instancingTranslatton[j * 3 + 1];
						translattonZ = instancingTranslatton[j * 3 + 2];
					}

					if (instancingRotation.size() > j * 4 + 3)
					{
						quaternionX = instancingRotation[j * 4];
						quaternionY = instancingRotation[j * 4 + 1];
						quaternionZ = instancingRotation[j * 4 + 2];
						quaternionW = instancingRotation[j * 4 + 3];
					}

					if (instancingScale.size() > j * 3 + 2)
					{
						scalingX = instancingScale[j * 3];
						scalingY = instancingScale[j * 3 + 1];
						scalingZ = instancingScale[j * 3 + 2];
					}
					Vector3D vec3d1(translattonX, translattonY, translattonZ);
					Vector4D vec4d(quaternionX, quaternionY, quaternionZ, quaternionW);
					Vector3D vec3d2(scalingX, scalingY, scalingZ);
					Matrix4D item = GetMatrix4(vec3d1, vec4d, vec3d2);
					item = matrix * item;

					if (instanceMatrix.size() > 0)
					{
						for (int k = 0; k < instanceMatrix.size(); k++)
						{
							item = instanceMatrix[k] * item;
							rtn.emplace_back(item);
						}
					}
					else
						rtn.emplace_back(item);
				}
				Matrix4Unit(matrix);
			}

			if (!nodeTransitionInfo[i].gpuInstancingInfo.batchid.empty())
			{
				gisINT64 ID = 0;
				const Accessor& accessor = document.accessors.Get(nodeTransitionInfo[i].gpuInstancingInfo.rotation);
				if (accessor.componentType == ComponentType::COMPONENT_FLOAT)
				{
					vector<float> gltfBatchID;
					gltfBatchID = ReadBinaryData<float>(resourceReader, document, accessor);
					SetIDValue(gltfBatchID, ID, instancingBatchid);
				}
				else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_SHORT)
				{
					vector<gisUSHORT> gltfBatchID;
					gltfBatchID = ReadBinaryData<gisUSHORT>(resourceReader, document, accessor);
					SetIDValue(gltfBatchID, ID, instancingBatchid);
				}
				else if (accessor.componentType == ComponentType::COMPONENT_UNSIGNED_INT)
				{
					vector<gisUINT> gltfBatchID;
					gltfBatchID = ReadBinaryData<gisUINT>(resourceReader, document, accessor);
					SetIDValue(gltfBatchID, ID, instancingBatchid);
				}

				if (instancingBatchid.size() <= 0 && ID > 0)
					instancingBatchid.insert(instancingBatchid.end(), accessor.count, ID);
			}
			if (instancingBatchid.size() > 0 && !isError &&instancingCount == instancingBatchid.size())
			{
				if (instanceBatchids.size() <= 0)
					instanceBatchids.insert(instanceBatchids.end(), instancingBatchid.begin(), instancingBatchid.end());
				else
				{
					//出现嵌套实例信息时，要素id按照最初的id
					int num = instanceBatchids.size();
					for (int j = 1; j < instancingBatchid.size(); j++)
					{
						instanceBatchids.insert(instanceBatchids.end(), instanceBatchids.begin(), instanceBatchids.begin() + num);
					}
				}
			}
			instanceMatrix.clear();
			instanceMatrix.insert(instanceMatrix.end(), rtn.begin(), rtn.end());
		}
	}

	Matrix4D 	YupToZupMatrix(
		1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, -1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	Matrix4D 	ZupToYupMatrix4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, -1.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);

	if (instanceMatrix.size() > 0)
	{
		MapGIS::Tile::Vector3D translation;
		MapGIS::Tile::Vector4D rotation;
		MapGIS::Tile::Vector3D scale;

		for (int i = 0; i < instanceMatrix.size(); i++)
		{//UP
			Matrix4D tempMatrix = ZupToYupMatrix4 *  instanceMatrix[i] * YupToZupMatrix;

			ModelInstance item;
			DecomposeMatrix4(tempMatrix, translation, rotation, scale);

			item.position.x = translation.x;
			item.position.y = translation.y;
			item.position.z = translation.z;

			if (fabs(rotation.x) > 1e-7 || fabs(rotation.y) > 1e-7 || fabs(rotation.z) > 1e-7 || fabs(rotation.w - 1) > 1e-7)
			{
				MapGIS::Tile::Vector3D UNIT_Y(0, 1, 0);
				MapGIS::Tile::Vector3D normalUp = QuaternionVector3d(rotation, UNIT_Y);
				item.hasNormalUp = true;

				item.normalUp.x = normalUp.x;
				item.normalUp.y = normalUp.y;
				item.normalUp.z = normalUp.z;
				//Right
				MapGIS::Tile::Vector3D UNIT_X(1, 0, 0);
				MapGIS::Tile::Vector3D normalRight = QuaternionVector3d(rotation, UNIT_X);
				item.hasNormalRight = true;
				item.normalRight.x = normalRight.x;
				item.normalRight.y = normalRight.y;
				item.normalRight.z = normalRight.z;
			}

			if (fabs(scale.x - 1) > 1e-7 || fabs(scale.y - 1) > 1e-7 || fabs(scale.z - 1) > 1e-7)
			{
				if (fabs(scale.x - scale.y) <= 1e-7 && fabs(scale.y - scale.z) <= 1e-7)
				{
					item.hasScale = true;
					item.scale = scale.x;
				}
				else
				{
					item.hasScaleNonUniform = true;
					item.scaleNonUniform.x = scale.x;
					item.scaleNonUniform.y = scale.y;
					item.scaleNonUniform.z = scale.z;
				}
			}
			pInstance->emplace_back(item);
		}
	}
}

gisLONG Ci_ModelGltf::To(vector<ContentBase*>*pModel)
{
	if (m_data.size() <= 0 || pModel == NULL)
		return  0;
	string buffer = "";
	buffer.append(m_data.data(), m_data.length());

	std::string strExtension = Microsoft::glTF::KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::gaussiansplattingCompressionSpz2_name;
	size_t index = buffer.find(strExtension);
	if (index != string::npos)
	{
		GaussianContent * contentItem = new  GaussianContent();
		pModel->emplace_back(contentItem);
		contentItem->CreateData(true);
		gisLONG rtn = ToGaussian(contentItem);
		if (rtn <= 0)
		{
			pModel->clear();
			delete contentItem;
		}
		return rtn;
	}
	index = buffer.find("KHR_draco_mesh_compression");
	if (index != string::npos)
	{
		Ci_DracoTool::GeometryDecompression(buffer, buffer);
	}
	else
	{
		index = buffer.find("EXT_meshopt_compression");
		if (index != string::npos)
		{
			MeshOpt_Gltfpack::GeometryDecompression(buffer, buffer);
		}
	}
	int bytesLength = ReadBufferToInt32(buffer, 8);

	Document document;
	stringstream m_istr;
	m_istr.str("");
	m_istr.write((char* const)(&buffer[0]), bytesLength);

	unique_ptr<GLBResourceReader> resourceReader;
	if (!ReadGLTFInfo(m_istr, document, resourceReader, m_pStorage))
		return 0;
	vector<GltfMeshInfo>  meshInfos;
	const auto& gltfScene = document.scenes[0];
	for (const auto& gltfNodeId : gltfScene.nodes)
	{
		vector<NodeTransitionInfo> nodeTransitionInfo;
		StatistMeshInfo(document, *resourceReader.get(), gltfNodeId, nodeTransitionInfo,meshInfos);
	}
	vector<vector<GltfMeshInfo>> groupMeshInfos;
	MeshInfoGroup(meshInfos, groupMeshInfos);
	if (groupMeshInfos.size() > 0)
	{
		for (int i = 0; i < groupMeshInfos.size(); i++)
		{
			if (groupMeshInfos[i].size() <= 0 || groupMeshInfos[i][0].type == GeometryType::None)
				continue;
			GeometryContent * contentItem = new  GeometryContent();
			pModel->emplace_back(contentItem);
			contentItem->CreateData(true, groupMeshInfos[i][0].type, true, true);
			G3DModel* pMode = contentItem->Get3DModel();
			vector<ModelInstance>* pInstance = contentItem->GetModelInstance();
			Matrix4D matrix;
			Matrix4Unit(matrix);
			if (groupMeshInfos[i].size() > 0)
			{
				MakeInstanceAndMatrix(document, *resourceReader.get(), groupMeshInfos[i][0].nodeTransitionInfo, pInstance, matrix);
			}
			for (int j = 0; j < groupMeshInfos[i].size(); j++)
			{
				switch (groupMeshInfos[i][0].type)
				{
				case GeometryType::Point:
					i_To(document, *resourceReader.get(), groupMeshInfos[i][j].meshPrimitive, matrix, *(MapGIS::Tile::PointsModel*)pMode);
					break;
				case GeometryType::Line:
					i_To(document, *resourceReader.get(), groupMeshInfos[i][j].meshPrimitive, matrix, *(MapGIS::Tile::LinesModel*)pMode);
					break;
				case GeometryType::Surface:
					i_To(document, *resourceReader.get(), groupMeshInfos[i][j].meshPrimitive, matrix, *(MapGIS::Tile::SurfacesModel*)pMode, j != 0);
					break;
				case GeometryType::Entity:
					i_To(document, *resourceReader.get(), groupMeshInfos[i][j].meshPrimitive, matrix, *(MapGIS::Tile::EntitiesModel*)pMode, j != 0);
					break;
				default:
					break;
				}
			}
		}
	}
	return 1;
}

gisLONG Ci_ModelGltf::FromGaussian(GaussianContent *pModel, GaussianExtMode mode) 
{
	if (pModel == NULL || pModel->Get3DModel() == NULL || pModel->Get3DModel()->features.size() <= 0)
		return 0;

	MapGIS::Tile::GaussianModel * pGaussianModel =  pModel->Get3DModel();

#ifndef MAPGIS_LINUX
	unique_ptr<iostream>           tempBufferStream = make_unique<stringstream>();
	unique_ptr<GLBResourceWriterMemory> resourceWriter = make_unique<GLBResourceWriterMemory>(move(tempBufferStream));
#else
	std::unique_ptr<std::stringstream> tempBufferStream(new std::stringstream());
	std::unique_ptr<GLBResourceWriterMemory> resourceWriter(new GLBResourceWriterMemory(std::move(tempBufferStream)));
#endif

	GLBResourceWriterMemory                 *glbResourceWriterMemory = NULL;

	Microsoft::glTF::Document                document;
	Microsoft::glTF::Material gltfMaterial;

	string materialId = Ci_GltfSdkTool::AddMaterial(document, gltfMaterial);
	Scene   scene;

	if (mode == GaussianExtMode::KHRGaussianSplattingCompressionSpz2)
	{
		GaussianSplattingBufferBuilder           bufferBuilder(move(resourceWriter));
		bufferBuilder.AddBuffer(GLB_BUFFER_ID);
		glbResourceWriterMemory = dynamic_cast<GLBResourceWriterMemory*>(&bufferBuilder.GetResourceWriter());

		std::string positionId = bufferBuilder.AddAccessor(pGaussianModel->features.size(), { TYPE_VEC3, COMPONENT_FLOAT }).id;
		std::string colorId = bufferBuilder.AddAccessor(pGaussianModel->features.size(), { TYPE_VEC4, COMPONENT_UNSIGNED_BYTE }).id;
		std::string scaleId = bufferBuilder.AddAccessor(pGaussianModel->features.size(), { TYPE_VEC3, COMPONENT_FLOAT }).id;
		std::string rotationId = bufferBuilder.AddAccessor(pGaussianModel->features.size(), { TYPE_VEC4, COMPONENT_FLOAT }).id;

		GaussianFeature& item = pGaussianModel->features[0];

		vector<std::string> shIds;

		if (item.sh.size() >= 3 * 3)
		{
			for (int i = 0; i < 3; i++)
				shIds.push_back(bufferBuilder.AddAccessor(pGaussianModel->features.size(), { TYPE_VEC3, COMPONENT_FLOAT }).id);
		}

		if (item.sh.size() >= (3 + 5) * 3)
		{
			for (int i = 0; i < 5; i++)
				shIds.push_back(bufferBuilder.AddAccessor(pGaussianModel->features.size(), { TYPE_VEC3, COMPONENT_FLOAT }).id);
		}


		if (item.sh.size() >= (3 + 5 + 7) * 3)
		{
			for (int i = 0; i < 7; i++)
				shIds.push_back(bufferBuilder.AddAccessor(pGaussianModel->features.size(), { TYPE_VEC3, COMPONENT_FLOAT }).id);
		}

		MeshPrimitive meshPrimitive;

		meshPrimitive.mode = MESH_POINTS;

		if (!materialId.empty())
		{
			meshPrimitive.materialId = materialId;
		}

		meshPrimitive.attributes[ACCESSOR_POSITION] = positionId;
		meshPrimitive.attributes[ACCESSOR_COLOR_0] = colorId;
		meshPrimitive.attributes["KHR_gaussian_splatting:SCALE"] = scaleId;
		meshPrimitive.attributes["KHR_gaussian_splatting:ROTATION"] = rotationId;

		if (item.sh.size() >= 3 * 3)
		{
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_1_COEF_0"] = shIds[0];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_1_COEF_1"] = shIds[1];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_1_COEF_2"] = shIds[2];
		}


		if (item.sh.size() >= (3 + 5) * 3)
		{
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_0"] = shIds[3];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_1"] = shIds[4];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_2"] = shIds[5];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_3"] = shIds[6];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_4"] = shIds[7];
		}

		if (item.sh.size() >= (3 + 5 + 7) * 3)
		{
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_0"] = shIds[8];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_1"] = shIds[9];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_2"] = shIds[10];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_3"] = shIds[11];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_4"] = shIds[12];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_5"] = shIds[13];
			meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_6"] = shIds[14];
		}
		Ci_SpzGaussianReadWrite spzReadWrite;
		vector<uint8_t> outValue;
		spzReadWrite.Write(*pGaussianModel, outValue);

		std::string bufferViewId = bufferBuilder.AddBufferView(outValue.data(), outValue.size()).id;

		std::string strExtension = Microsoft::glTF::KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::gaussiansplattingCompressionSpz2_name;
		if (!document.IsExtensionUsed(strExtension))
			document.extensionsUsed.insert(strExtension);
		if (!document.IsExtensionRequired(strExtension))
			document.extensionsRequired.insert(strExtension);
		strExtension = Microsoft::glTF::KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::gaussiansplatting_name;
		if (!document.IsExtensionUsed(strExtension))
			document.extensionsUsed.insert(strExtension);
		if (!document.IsExtensionRequired(strExtension))
			document.extensionsRequired.insert(strExtension);


		Microsoft::glTF::KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2 spz2;
		spz2.bufferViewId = bufferViewId;

		ExtensionSerializer extensionSerializer;
		string strGpuInstancing = Microsoft::glTF::KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::SerializeGaussianSplattingCompressionSpz2(spz2, document, extensionSerializer);
		meshPrimitive.extensions[strExtension] = strGpuInstancing;

		Microsoft::glTF::Mesh  gltfMesh;
		gltfMesh.primitives.push_back(std::move(meshPrimitive));

		std::string meshID = Ci_GltfSdkTool::AddMesh(document, gltfMesh);
		Microsoft::glTF::Node  gltfNode;

		Matrix4 matrix;
		matrix.values[0] = 1.0;
		matrix.values[1] = 0;
		matrix.values[2] = 0;
		matrix.values[3] = 0;

		matrix.values[4] = 0;
		matrix.values[5] = 0;
		matrix.values[6] = -1.0;
		matrix.values[7] = 0;

		matrix.values[8] = 0;
		matrix.values[9] = 1.0;
		matrix.values[10] = 0;
		matrix.values[11] = 0;

		matrix.values[12] = 0;
		matrix.values[13] = 0;
		matrix.values[14] = 0;
		matrix.values[15] = 1.0;

		gltfNode.matrix = matrix;
		gltfNode.meshId = meshID;

		std::string nodeID = Ci_GltfSdkTool::AddNode(document, gltfNode);

		scene.nodes.push_back(nodeID);
		bufferBuilder.Output(document);
		document.SetDefaultScene(move(scene), AppendIdPolicy::GenerateOnEmpty);
		string manifest;
		try
		{
			manifest = Serialize(document, SerializeFlags::None);
		}
		catch (const GLTFException& ex)
		{
			stringstream ss;

			ss << "Microsoft::glTF::Serialize failed: ";
			ss << ex.what();

			throw runtime_error(ss.str());
		}
		m_data.clear();
		string buf = "";
		glbResourceWriterMemory->GetGLBInfo(manifest, buf);

		if (m_data.isEmpty())
		{
			m_data.append(buf.c_str(), buf.length());
		}
		return 1;
	}
	//前端不支持不压缩的情况
	//else if (mode == GaussianExtMode::KHR_gaussian_splatting) 
	//{
	//	double  SH_C0 = 0.28209479177387814;
	//	spz::GaussianCloud   result;
	//	int32_t shDegree = 0;
	//	if (pModel->features[0].sh.size()/3 < 3)
	//		shDegree = 0;
	//	else if (pModel->features[0].sh.size()/3 < 8)
	//		shDegree = 1;
	//	else if (pModel->features[0].sh.size()/3 < 15)
	//		shDegree = 2;
	//	else
	//		shDegree = 3;
	//	result.numPoints = pModel->features.size();
	//	result.shDegree = shDegree;
	//	result.positions.reserve(pModel->features.size() * 3);
	//	result.scales.reserve(pModel->features.size() * 3);
	//	result.rotations.reserve(pModel->features.size() * 4);
	//	result.alphas.reserve(pModel->features.size() * 1);
	//	result.colors.reserve(pModel->features.size() * 3);
	//	
	//	if (pModel->features[0].sh.size() > 0)
	//		result.sh.reserve(pModel->features.size() *pModel->features[0].sh.size());
	//	for (int i = 0; i < pModel->features.size(); i++)
	//	{
	//		result.positions.push_back(pModel->features[i].position[0]);
	//		result.positions.push_back(pModel->features[i].position[1]);
	//		result.positions.push_back(pModel->features[i].position[2]);
	//		result.scales.push_back(pModel->features[i].scale[0]);
	//		result.scales.push_back(pModel->features[i].scale[1]);
	//		result.scales.push_back(pModel->features[i].scale[2]);
	//		result.rotations.push_back(pModel->features[i].rotation[0]);
	//		result.rotations.push_back(pModel->features[i].rotation[1]);
	//		result.rotations.push_back(pModel->features[i].rotation[2]);
	//		result.rotations.push_back(pModel->features[i].rotation[3]);
	//		result.colors.push_back(pModel->features[i].color[0]);
	//		result.colors.push_back(pModel->features[i].color[1]);
	//		result.colors.push_back(pModel->features[i].color[2]);
	//		result.alphas.push_back(pModel->features[i].alpha);
	//		for (int j = 0; j < pModel->features[i].sh.size(); j++)
	//		{
	//			result.sh.push_back(pModel->features[i].sh[j]);
	//		}
	//	}
	//	result.convertCoordinates((spz::CoordinateSystem)(int)pModel->coordinateSystem, spz::CoordinateSystem::RBU);
	//	Microsoft::glTF::BufferBuilder           bufferBuilder(move(resourceWriter));
	//	glbResourceWriterMemory = dynamic_cast<GLBResourceWriterMemory*>(&bufferBuilder.GetResourceWriter());
	//	bufferBuilder.AddBuffer(GLB_BUFFER_ID);
	//	vector<float> positions;
	//	positions.reserve(result.numPoints *3);
	//	for (gisLONG k = 0; k <  result.numPoints; k++)
	//	{/*
	//		positions.push_back((float)(result.positions[k * 3 +0]));
	//		positions.push_back((float)(result.positions[k * 3 + 1]));
	//		positions.push_back((float)(result.positions[k * 3 + 2]));*/
	//		positions.push_back((float)(pModel->features[k].position[0]));
	//		positions.push_back((float)(pModel->features[k].position[2]));
	//		positions.push_back((float)(-pModel->features[k].position[1]));
	//	}
	//	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	//	std::string positionId = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT }).id;
	//	vector<float> colors;
	//	colors.reserve(result.numPoints * 4);
	//	for (gisLONG k = 0; k < result.numPoints; k++)
	//	{
	//		colors.push_back((float)((result.colors[k *3 + 0]) * SH_C0 + 0.5));
	//		colors.push_back((float)((result.colors[k * 3 + 1])* SH_C0 + 0.5));
	//		colors.push_back((float)((result.colors[k * 3 + 2])* SH_C0 + 0.5));
	//		colors.push_back((float)((1 / (exp(-result.alphas[k]) + 1))));
	//	}
	//	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	//	std::string colorId = bufferBuilder.AddAccessor(colors, { TYPE_VEC4, COMPONENT_FLOAT }).id;
	//	/*vector<float> scales;
	//	scales.reserve(result.numPoints * 3);
	//	for (gisLONG k = 0; k <  result.numPoints; k++)
	//	{
	//		scales.push_back((float)(result.scales[k * 3 + 0]));
	//		scales.push_back((float)(result.scales[k * 3 + 1]));
	//		scales.push_back((float)(result.scales[k * 3 + 2]));
	//	}*/
	//	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	//	std::string scaleId = bufferBuilder.AddAccessor(result.scales,{ TYPE_VEC3, COMPONENT_FLOAT }).id;
	//  /*vector<float> rotations;
	//	rotations.reserve(result.numPoints * 4);
	//	for (gisLONG k = 0; k < pModel->features.size(); k++)
	//	{
	//		rotations.push_back((float)(result.rotations[k * 4 + 0]));
	//		rotations.push_back((float)(result.rotations[k * 4 + 1]));
	//		rotations.push_back((float)(result.rotations[k * 4 + 2]));
	//		rotations.push_back((float)(result.rotations[k * 4 + 3]));
	//	}*/
	//	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	//	std::string rotationId = bufferBuilder.AddAccessor(result.rotations, { TYPE_VEC4, COMPONENT_FLOAT }).id;
	//	GaussianFeature& item = pModel->features[0];
	//	int num = item.sh.size();
	//	vector<std::string> shIds;
	//	if (item.sh.size() >= 3 * 3)
	//	{
	//		vector<float> shs;
	//		for (int i = 0; i < 3; i++)
	//		{
	//			shs.clear();
	//			shs.reserve(result.numPoints * 3);
	//			for (gisLONG k = 0; k < result.numPoints; k++)
	//			{
	//				shs.push_back((float)(result.sh[k * num + i * 3 + 0]));
	//				shs.push_back((float)(result.sh[k * num + i * 3 + 1]));
	//				shs.push_back((float)(result.sh[k * num + i * 3 + 2]));
	//			}
	//			bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	//			shIds.push_back(bufferBuilder.AddAccessor(shs, { TYPE_VEC3, COMPONENT_FLOAT }).id);
	//		}	
	//	}
	//	if (item.sh.size() >= (3 + 5) * 3)
	//	{
	//		vector<float> shs;
	//		for (int i = 0; i < 5; i++)
	//		{
	//			shs.clear();
	//			shs.reserve(result.numPoints * 3);
	//			for (gisLONG k = 0; k < pModel->features.size(); k++)
	//			{
	//				shs.push_back((float)(result.sh[k * num + (i + 3) * 3 + 0]));
	//				shs.push_back((float)(result.sh[k * num + (i + 3) * 3 + 1]));
	//				shs.push_back((float)(result.sh[k * num + (i + 3) * 3 + 2]));
	//			}
	//			bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	//			shIds.push_back(bufferBuilder.AddAccessor(shs, { TYPE_VEC3, COMPONENT_FLOAT }).id);
	//		}
	//	}
	//	if (item.sh.size() >= (3 + 5 + 7) * 3)
	//	{
	//		vector<float> shs;
	//		for (int i = 0; i < 7; i++)
	//		{
	//			shs.clear();
	//			shs.reserve(pModel->features.size() * 3);
	//			for (gisLONG k = 0; k < pModel->features.size(); k++)
	//			{
	//				shs.push_back((float)(result.sh[k * num + (i + 8) * 3 + 0]));
	//				shs.push_back((float)(result.sh[k * num + (i + 8) * 3 + 1]));
	//				shs.push_back((float)(result.sh[k * num + (i + 8) * 3 + 2]));
	//			}
	//			bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	//			shIds.push_back(bufferBuilder.AddAccessor(shs, { TYPE_VEC3, COMPONENT_FLOAT }).id);
	//		}
	//	}
	//	MeshPrimitive meshPrimitive;
	//	meshPrimitive.mode = MESH_POINTS;
	//	if (!materialId.empty())
	//	{
	//		meshPrimitive.materialId = materialId;
	//	}
	//	meshPrimitive.attributes[ACCESSOR_POSITION] = positionId;
	//	meshPrimitive.attributes[ACCESSOR_COLOR_0] = colorId;
	//	meshPrimitive.attributes["KHR_gaussian_splatting:SCALE"] = scaleId;
	//	meshPrimitive.attributes["KHR_gaussian_splatting:ROTATION"] = rotationId;
	//	if (item.sh.size() >= 3 * 3)
	//	{
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_1_COEF_0"] = shIds[0];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_1_COEF_1"] = shIds[1];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_1_COEF_2"] = shIds[2];
	//	}
	//	if (item.sh.size() >= (3 + 5) * 3)
	//	{
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_0"] = shIds[3];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_1"] = shIds[4];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_2"] = shIds[5];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_3"] = shIds[6];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_2_COEF_4"] = shIds[7];
	//	}
	//	if (item.sh.size() >= (3 + 5 + 7) * 3)
	//	{
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_0"] = shIds[8];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_1"] = shIds[9];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_2"] = shIds[10];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_3"] = shIds[11];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_4"] = shIds[12];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_5"] = shIds[13];
	//		meshPrimitive.attributes["KHR_gaussian_splatting:SH_DEGREE_3_COEF_6"] = shIds[14];
	//	}
	//	/*Ci_SpzGaussianReadWrite spzReadWrite;
	//	vector<uint8_t> outValue;
	//	spzReadWrite.Write(*pModel, outValue);
	//	std::string bufferViewId = bufferBuilder.AddBufferView(outValue.data(), outValue.size()).id;*/
	//	std::string strExtension = Microsoft::glTF::KHR::MeshPrimitiveGaussianSplatting::gaussiansplatting_name;
	//	if (!document.IsExtensionUsed(strExtension))
	//		document.extensionsUsed.insert(strExtension);
	//	if (!document.IsExtensionRequired(strExtension))
	//		document.extensionsRequired.insert(strExtension);
	//	Microsoft::glTF::KHR::MeshPrimitiveGaussianSplatting::GaussianSplatting gaussianSplatting;
	//	//gaussianSplatting.bufferViewId = "0";
	//	ExtensionSerializer extensionSerializer;
	//	string strGpuInstancing = Microsoft::glTF::KHR::MeshPrimitiveGaussianSplatting::SerializeGaussianSplatting(gaussianSplatting, document, extensionSerializer);
	//	meshPrimitive.extensions[strExtension] = strGpuInstancing;
	//	Microsoft::glTF::Mesh  gltfMesh;
	//	gltfMesh.primitives.push_back(std::move(meshPrimitive));
	//	std::string meshID = Ci_GltfSdkTool::AddMesh(document, gltfMesh);
	//	Microsoft::glTF::Node  gltfNode;
	//	Matrix4 matrix;
	//	matrix.values[0] = 1.0;
	//	matrix.values[1] = 0;
	//	matrix.values[2] = 0;
	//	matrix.values[3] = 0;
	//	matrix.values[4] = 0;
	//	matrix.values[5] = 0;
	//	matrix.values[6] = -1.0;
	//	matrix.values[7] = 0;
	//	matrix.values[8] = 0;
	//	matrix.values[9] = 1.0;
	//	matrix.values[10] = 0;
	//	matrix.values[11] = 0;
	//	matrix.values[12] = 0;
	//	matrix.values[13] = 0;
	//	matrix.values[14] = 0;
	//	matrix.values[15] = 1.0;
	//	gltfNode.matrix = matrix;
	//	gltfNode.meshId = meshID;
	//	std::string nodeID = Ci_GltfSdkTool::AddNode(document, gltfNode);
	//	scene.nodes.push_back(nodeID);
	//	bufferBuilder.Output(document);
	//	document.SetDefaultScene(move(scene), AppendIdPolicy::GenerateOnEmpty);
	//	string manifest;
	//	try
	//	{
	//		manifest = Serialize(document, SerializeFlags::None);
	//	}
	//	catch (const GLTFException& ex)
	//	{
	//		stringstream ss;
	//		ss << "Microsoft::glTF::Serialize failed: ";
	//		ss << ex.what();
	//		throw runtime_error(ss.str());
	//	}
	//	m_data.clear();
	//	string buf = "";
	//	glbResourceWriterMemory->GetGLBInfo(manifest, buf);
	//	if (m_data.isEmpty())
	//	{
	//		m_data.append(buf.c_str(), buf.length());
	//	}
	//	return 1;
	//}
	return 0;

}

gisLONG Ci_ModelGltf::ToGaussian(GaussianContent *pModel) 
{
	if (m_data.size() <= 0 || pModel == NULL || pModel->Get3DModel()== NULL)
		return  0;
	string buffer = "";
	buffer.append(m_data.data(), m_data.length());
	int bytesLength = ReadBufferToInt32(buffer, 8);

	Document document;
	stringstream m_istr;
	m_istr.str("");
	m_istr.write((char* const)(&buffer[0]), bytesLength);

	unique_ptr<GLBResourceReader> resourceReader;
	if (!ReadGLTFInfo(m_istr, document, resourceReader, m_pStorage))
		return 0;
	std::string strExtension = Microsoft::glTF::KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::gaussiansplattingCompressionSpz2_name;

	size_t index = buffer.find(strExtension);
	if (index != string::npos)
	{//存在压缩

		size_t bufferSize = 0;
		for (int i = 0; i < document.bufferViews.Elements().size(); i++)
		{
			if (document.bufferViews.Elements()[i].id == "0")
			{

				bufferSize = document.bufferViews[i].byteLength;
			}
		}

		for (int i = 0; i < document.buffers.Elements().size(); i++)
		{
			if (document.buffers.Elements()[i].id == "0")
			{

				std::shared_ptr<std::istream>  stream = resourceReader->GetBinaryStream(document.buffers[i]);

				std::vector<uint8_t> data(bufferSize);

				auto bufferStream = resourceReader->GetBinaryStream(document.buffers[i]);
				auto bufferStreamPos = resourceReader->GetBinaryStreamPos(document.buffers[i]);

				bufferStream->seekg(bufferStreamPos);
				bufferStream->seekg(0, std::ios_base::cur);

				StreamUtils::ReadBinary(*bufferStream, reinterpret_cast<char*>(data.data()), bufferSize);
				Ci_SpzGaussianReadWrite spzReadWrite;
				spzReadWrite.Read(data, *pModel->Get3DModel());
			}
		}
	}
	return 1;
}
