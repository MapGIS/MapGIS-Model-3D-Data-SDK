#include "stdafx.h"
#include "ci_gltfsdktool.h"
#include "ci_gltfextensions.h"

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/ExtensionsKHR.h>
#include <GLTFSDK/IStreamCache.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/Constants.h>
#include <GLTFSDK/StreamUtils.h>
#include <GLTFSDK/Constants.h>

uint32_t CalculatePadding(size_t byteLength)
{
	const uint32_t alignmentSize = GLB_CHUNK_ALIGNMENT_SIZE;

	const auto padAlign = byteLength % alignmentSize;
	const auto pad = padAlign ? alignmentSize - padAlign : 0U;

	return static_cast<uint32_t>(pad);
}

Ci_GltfSdkTool::Ci_GltfSdkTool()
{
}
Ci_GltfSdkTool::~Ci_GltfSdkTool()
{
}

std::string Ci_GltfSdkTool::AddImage(Document& document, const string& uri)
{
	Microsoft::glTF::Image image;
	image.uri = uri;

	auto imageId = document.images.Append(std::move(image), AppendIdPolicy::GenerateOnEmpty).id;

	return imageId;
}

std::string Ci_GltfSdkTool::AddImage(Document& document, const string& bufferViewId, const string& mimeType)
{
	Microsoft::glTF::Image image;
	image.bufferViewId = bufferViewId;
	image.mimeType = mimeType;

	auto imageId = document.images.Append(std::move(image), AppendIdPolicy::GenerateOnEmpty).id;

	return imageId;
}

std::string Ci_GltfSdkTool::AddImageBufferView(BufferBuilder& bufferBuilder, const vector<unsigned char>& data)
{
	bufferBuilder.AddBufferView(data);

	/*std::ostringstream ostr;
	ostr << bufferBuilder.GetBufferViewCount() - 1;

	return ostr.str();*/
	return std::to_string(bufferBuilder.GetBufferViewCount() - 1);
}

std::string Ci_GltfSdkTool::AddSampler(Document& document, const Microsoft::glTF::MagFilterMode& magFilter, const Microsoft::glTF::MinFilterMode& minFilter,
	const Microsoft::glTF::WrapMode& wrapS, const Microsoft::glTF::WrapMode& wrapT)
{
	Microsoft::glTF::Sampler sampler;
	sampler.magFilter = magFilter;
	sampler.minFilter = minFilter;
	sampler.wrapS = wrapS;
	sampler.wrapT = wrapT;

	auto samplerId = document.samplers.Append(std::move(sampler), AppendIdPolicy::GenerateOnEmpty).id;

	return samplerId;
}

std::string Ci_GltfSdkTool::AddSampler(Microsoft::glTF::Document& document, const Microsoft::glTF::Sampler& sampler)
{
	auto samplerId = document.samplers.Append(std::move(sampler), AppendIdPolicy::GenerateOnEmpty).id;

	return samplerId;
}

std::string Ci_GltfSdkTool::AddTexture(Microsoft::glTF::Document& document, string& imageId, string& samplerId, string& mimeType)
{
	Microsoft::glTF::Texture texture;
	texture.imageId = imageId;
	texture.samplerId = samplerId;

	if (mimeType == "image/ktx2")
	{
		std::string strExtension = KHR::TextureBasiuInfos::TEXTUREBASIU_NAME;
		if (!document.IsExtensionUsed(strExtension))
			document.extensionsUsed.insert(strExtension);
		if (!document.IsExtensionRequired(strExtension))
			document.extensionsRequired.insert(strExtension);

		KHR::TextureBasiuInfos::TextureBasisu textureBasisu;
		textureBasisu.source = imageId;
		ExtensionSerializer extensionSerializer;
		string strTextureBasisu = SerializeTextureBasisu(textureBasisu, document, extensionSerializer);
		texture.extensions[strExtension] = strTextureBasisu;
	}
	else if (mimeType == "image/webp")
	{
		std::string strExtension = Vendor::TextureWebpInfos::TEXTUREWEBP_NAME;
		if (!document.IsExtensionUsed(strExtension))
			document.extensionsUsed.insert(strExtension);
		if (!document.IsExtensionRequired(strExtension))
			document.extensionsRequired.insert(strExtension);

		Vendor::TextureWebpInfos::TextureWebp textureWebp;
		textureWebp.source = imageId;
		ExtensionSerializer extensionSerializer;
		string strTextureWebp = SerializeTextureWebp(textureWebp, document, extensionSerializer);
		texture.extensions[strExtension] = strTextureWebp;
	}

	auto textureId = document.textures.Append(std::move(texture), AppendIdPolicy::GenerateOnEmpty).id;

	return textureId;
}

std::string Ci_GltfSdkTool::AddMaterial(Microsoft::glTF::Document& document, Microsoft::glTF::Material& material) {
	auto materialId = document.materials.Append(std::move(material), AppendIdPolicy::GenerateOnEmpty).id;

	return materialId;
}

std::string Ci_GltfSdkTool::AddIndexAccessor(BufferBuilder& bufferBuilder, const vector<unsigned char>& indices) {
	// Create a BufferView with a target of ELEMENT_ARRAY_BUFFER (as it will reference index
	// data) - it will be the 'current' BufferView that all the Accessors created by this
	// BufferBuilder will automatically reference
	bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

	// Add an Accessor for the indices

	// Copy the Accessor's id - subsequent calls to AddAccessor may invalidate the returned reference
	return bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_BYTE }).id;
}

std::string Ci_GltfSdkTool::AddIndexAccessor(BufferBuilder& bufferBuilder, const vector<unsigned short>& indices) {
	// Create a BufferView with a target of ELEMENT_ARRAY_BUFFER (as it will reference index
	// data) - it will be the 'current' BufferView that all the Accessors created by this
	// BufferBuilder will automatically reference
	bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

	// Add an Accessor for the indices

	// Copy the Accessor's id - subsequent calls to AddAccessor may invalidate the returned reference
	return bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT }).id;
}

std::string Ci_GltfSdkTool::AddIndexAccessor(BufferBuilder& bufferBuilder, const vector<unsigned int>& indices) {
	// Create a BufferView with a target of ELEMENT_ARRAY_BUFFER (as it will reference index
	// data) - it will be the 'current' BufferView that all the Accessors created by this
	// BufferBuilder will automatically reference
	bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

	// Add an Accessor for the indices

	// Copy the Accessor's id - subsequent calls to AddAccessor may invalidate the returned reference
	return bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_INT }).id;
}

std::string Ci_GltfSdkTool::AddPositionsAccessor(BufferBuilder& bufferBuilder, const vector<float>& positions) {
	// Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

	std::vector<float> minValues(3U, (std::numeric_limits<float>::max)());
	std::vector<float> maxValues(3U, (std::numeric_limits<float>::lowest)());

	const size_t positionCount = positions.size();

	// Accessor min/max properties must be set for vertex position data so calculate them here
	for (size_t i = 0U, j = 0U; i < positionCount; ++i, j = (i % 3U))
	{
		minValues[j] = (std::min)(positions[i], minValues[j]);
		maxValues[j] = (std::max)(positions[i], maxValues[j]);
	}

	return bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT, false, std::move(minValues), std::move(maxValues) }).id;
}

std::string Ci_GltfSdkTool::AddNormalsAccessor(BufferBuilder& bufferBuilder, const vector<float>& normals) {
	// Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

	return bufferBuilder.AddAccessor(normals, { TYPE_VEC3, COMPONENT_FLOAT }).id;
}

std::string Ci_GltfSdkTool::AddTexCoordsAccessor(BufferBuilder& bufferBuilder, const vector<float>& texCoords) {
	// Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

	return bufferBuilder.AddAccessor(texCoords, { TYPE_VEC2, COMPONENT_FLOAT }).id;
}

std::string Ci_GltfSdkTool::AddTexColorsAccessor(BufferBuilder& bufferBuilder, const vector<float>& colors) {
	// Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

	return bufferBuilder.AddAccessor(colors, { TYPE_VEC4, COMPONENT_FLOAT }).id;
}

std::string Ci_GltfSdkTool::AddOIDsAccessor(BufferBuilder& bufferBuilder, const vector<unsigned int>& OIDs) {
	// Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

	vector<unsigned char> OIDArray;

	OIDArray.reserve(OIDs.size() * 4);
	for (int i = 0; i < OIDs.size(); i++)
	{
		unsigned int OID = OIDs[i];

		unsigned char r = (unsigned char)(OID >> 24 & 0xff);
		unsigned char g = (unsigned char)(OID >> 16 & 0xff);
		unsigned char b = (unsigned char)(OID >> 8 & 0xff);
		unsigned char a = (unsigned char)(OID & 0xff);

		OIDArray.push_back(a);
		OIDArray.push_back(b);
		OIDArray.push_back(g);
		OIDArray.push_back(r);
	}

	return bufferBuilder.AddAccessor(OIDArray, { TYPE_VEC4, COMPONENT_UNSIGNED_BYTE }).id;
}

std::string Ci_GltfSdkTool::AddBatchIDsAccessor(BufferBuilder& bufferBuilder, const vector<float>& batchIDs) {
	// Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

	return bufferBuilder.AddAccessor(batchIDs, { TYPE_SCALAR, COMPONENT_FLOAT }).id;
}

std::string Ci_GltfSdkTool::AddBatchIDsAccessor(BufferBuilder& bufferBuilder, const vector<gisUSHORT>& batchIDs) {
	// Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	return bufferBuilder.AddAccessor(batchIDs, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT }).id;
}

std::string Ci_GltfSdkTool::AddInstancingTranslationAccessor(BufferBuilder& bufferBuilder, const vector<float>& translation) {
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	return bufferBuilder.AddAccessor(translation, { TYPE_VEC3, COMPONENT_FLOAT }).id;
}

std::string Ci_GltfSdkTool::AddInstancingRotationAccessor(BufferBuilder& bufferBuilder, const vector<float>& rotation) {
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	return bufferBuilder.AddAccessor(rotation, { TYPE_VEC4, COMPONENT_FLOAT }).id;
}

std::string Ci_GltfSdkTool::AddInstancingScaleAccessor(BufferBuilder& bufferBuilder, const vector<float>& scale) {
	bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);
	return bufferBuilder.AddAccessor(scale, { TYPE_VEC3, COMPONENT_FLOAT }).id;
}

void Ci_GltfSdkTool::MeshAddPrimitive(Microsoft::glTF::Mesh & mesh, Microsoft::glTF::MeshMode& mode, string& materialId, string& accessorIdIndices, string& accessorIdPositions,
	string& accessorIdNormals, vector<string>& accessorIdTexCoords, string& accessorIdColors, string& accessorIdOIDs, string& accessorIdBatchIDs)
{
	MeshPrimitive meshPrimitive;

	meshPrimitive.mode = mode;

	if (!materialId.empty())
	{
		meshPrimitive.materialId = materialId;
	}

	if (!accessorIdIndices.empty())
	{
		meshPrimitive.indicesAccessorId = accessorIdIndices;
	}

	meshPrimitive.attributes[ACCESSOR_POSITION] = accessorIdPositions;

	if (!accessorIdNormals.empty())
	{
		meshPrimitive.attributes[ACCESSOR_NORMAL] = accessorIdNormals;
	}

	if (accessorIdTexCoords.size() > 0)
	{
		for (int i = 0; i < accessorIdTexCoords.size(); i++)
		{
			meshPrimitive.attributes["TEXCOORD_" + std::to_string(i)] = accessorIdTexCoords[i];
		}
	}
	if (!accessorIdColors.empty())
	{
		meshPrimitive.attributes[ACCESSOR_COLOR_0] = accessorIdColors;
	}

	if (!accessorIdOIDs.empty())
	{
		meshPrimitive.attributes[ACCESSOR_OID] = accessorIdOIDs;
	}

	if (!accessorIdBatchIDs.empty())
	{
		meshPrimitive.attributes[ACCESSOR_BatchID] = accessorIdBatchIDs;
	}
	mesh.primitives.push_back(std::move(meshPrimitive));
}

std::string  Ci_GltfSdkTool::AddMesh(Microsoft::glTF::Document& document,  Microsoft::glTF::Mesh & mesh)
{
	string meshId = document.meshes.Append(std::move(mesh), AppendIdPolicy::GenerateOnEmpty).id;

	return meshId;
}
void Ci_GltfSdkTool::NodeAddGPUInstancingEx(Microsoft::glTF::Document& document, Microsoft::glTF::Node & node, string &translationId, string &rotationId, string &scaleId, string &batchidId)
{
	if (!translationId.empty() && !rotationId.empty() && !scaleId.empty())
	{
		std::string strExtension = Vendor::MeshGpuInstancingInfos::MESH_GPU_INSTANCING_NAME;
		if (!document.IsExtensionUsed(strExtension))
			document.extensionsUsed.insert(strExtension);
		if (!document.IsExtensionRequired(strExtension))
			document.extensionsRequired.insert(strExtension);
		Vendor::MeshGpuInstancingInfos::MeshGpuInstancing gpuInstancing;
		gpuInstancing.translatton = translationId;
		gpuInstancing.rotation = rotationId;
		gpuInstancing.scale = scaleId;
		gpuInstancing.batchid = batchidId;
		ExtensionSerializer extensionSerializer;
		string strGpuInstancing = Vendor::MeshGpuInstancingInfos::SerializeMeshGpuInstancing(gpuInstancing, document, extensionSerializer);
		node.extensions[strExtension] = strGpuInstancing;
	}
}

std::string Ci_GltfSdkTool::AddNode(Microsoft::glTF::Document& document, Microsoft::glTF::Node & node)
{
	auto nodeId = document.nodes.Append(std::move(node), AppendIdPolicy::GenerateOnEmpty).id;
	return nodeId;
}
int Ci_GltfSdkTool::FindSamplerIndex(const Document& document, const string& samplerId)
{
	for (int i = 0; i < document.samplers.Elements().size(); i++)
	{
		if (document.samplers.Elements()[i].id == samplerId)
		{
			return i;
		}
	}
	return -1;
}

int Ci_GltfSdkTool::FindImageIndex(const Document& document, const string& imageId)
{
	for (int i = 0; i < document.images.Elements().size(); i++)
	{
		if (document.images.Elements()[i].id == imageId)
		{
			return i;
		}
	}
	return -1;
}

int Ci_GltfSdkTool::FindTextureIndex(const Document& document, const string& textureId)
{
	for (int i = 0; i < document.textures.Elements().size(); i++)
	{
		if (document.textures.Elements()[i].id == textureId)
		{
			return i;
		}
	}
	return -1;
}

int Ci_GltfSdkTool::FindMaterialIndex(const Document& document, const string& materialId)
{
	for (int i = 0; i < document.materials.Elements().size(); i++)
	{
		if (document.materials.Elements()[i].id == materialId)
		{
			return i;
		}
	}
	return -1;
}
int Ci_GltfSdkTool::FindNodeIndex(const Document& document, const string& nodeId)
{
	for (gisLONG i = 0; i < document.nodes.Elements().size(); i++)
	{
		if (document.nodes.Elements()[i].id == nodeId)
		{
			return i;
		}
	}
	return -1;
}

int Ci_GltfSdkTool::FindMesheIndex(const Document& document, const string& meshesId)
{
	for (gisLONG i = 0; i < document.meshes.Elements().size(); i++)
	{
		if (document.meshes.Elements()[i].id == meshesId)
		{
			return i;
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLBResourceWriterMemory::GLBResourceWriterMemory(std::unique_ptr<std::iostream> tempBufferStream) :
	GLTFResourceWriter(std::move((std::unique_ptr<IStreamWriterCache>)nullptr)), m_stream(std::move(tempBufferStream))
{
}
void GLBResourceWriterMemory::i_GetGLBInfo(const std::string& manifest, std::stringstream& strstream)
{
	uint32_t jsonChunkLength = static_cast<uint32_t>(manifest.length());
	const uint32_t jsonPaddingLength = CalculatePadding(jsonChunkLength);
	jsonChunkLength += jsonPaddingLength;

	std::string bufferId = GLB_BUFFER_ID;
	std::streamoff  off = GetBufferOffset(bufferId);
	uint32_t binaryChunkLength = static_cast<uint32_t>(off);
	const uint32_t binaryPaddingLength = CalculatePadding(binaryChunkLength);

	binaryChunkLength += binaryPaddingLength;

	const uint32_t length = GLB_HEADER_BYTE_SIZE // 12 bytes (GLB header) + 8 bytes (JSON header)
		+ jsonChunkLength
		+ sizeof(binaryChunkLength) + GLB_CHUNK_TYPE_SIZE // 8 bytes (BIN header)
		+ binaryChunkLength;

	// Write GLB header (12 bytes)
	StreamUtils::WriteBinary(strstream, GLB_HEADER_MAGIC_STRING, GLB_HEADER_MAGIC_STRING_SIZE);
	StreamUtils::WriteBinary(strstream, GLB_HEADER_VERSION_2);
	StreamUtils::WriteBinary(strstream, length);

	// Write JSON header (8 bytes)
	StreamUtils::WriteBinary(strstream, jsonChunkLength);
	StreamUtils::WriteBinary(strstream, GLB_CHUNK_TYPE_JSON, GLB_CHUNK_TYPE_SIZE);

	// Write JSON (indeterminate length)
	StreamUtils::WriteBinary(strstream, manifest);

	if (jsonPaddingLength > 0)
	{
		// GLB spec requires the JSON chunk to be padded with trailing space characters (0x20) to satisfy alignment requirements
		StreamUtils::WriteBinary(strstream, std::string(jsonPaddingLength, ' '));
	}

	// Write BIN header (8 bytes)
	StreamUtils::WriteBinary(strstream, binaryChunkLength);
	StreamUtils::WriteBinary(strstream, GLB_CHUNK_TYPE_BIN, GLB_CHUNK_TYPE_SIZE);

	// Write BIN contents (indeterminate length) - copy the temporary buffer's contents to the output stream
	if (binaryChunkLength > 0)
	{
		strstream << m_stream->rdbuf();
	}

	if (binaryPaddingLength > 0)
	{
		// GLB spec requires the BIN chunk to be padded with trailing zeros (0x00) to satisfy alignment requirements
		StreamUtils::WriteBinary(strstream, std::vector<uint8_t>(binaryPaddingLength, 0));
	}
}
void  GLBResourceWriterMemory::GetGLBInfo(const std::string& manifest, CGByteArray &info)
{
	std::stringstream tempBufferStream;
	GLBResourceWriterMemory::i_GetGLBInfo(manifest, tempBufferStream);
	string value = tempBufferStream.rdbuf()->str();
	info.clear();
	info.append(value[0], value.length());
}

void GLBResourceWriterMemory::GetGLBInfo(const std::string& manifest, std::string& info)
{
	std::stringstream tempBufferStream;
	GLBResourceWriterMemory::i_GetGLBInfo(manifest, tempBufferStream);
	info = tempBufferStream.rdbuf()->str();
}

std::string GLBResourceWriterMemory::GenerateBufferUri(const std::string& bufferId)  const
{
	std::string bufferUri;

	// Return an empty uri string when passed the GLB buffer id
	if (bufferId != GLB_BUFFER_ID)
	{
		bufferUri = GLTFResourceWriter::GenerateBufferUri(bufferId);
	}
	return bufferUri;
}

std::ostream* GLBResourceWriterMemory::GetBufferStream(const std::string& bufferId)
{
	std::ostream* stream = m_stream.get();
	if (bufferId != GLB_BUFFER_ID)
	{
		stream = NULL;
	}
	return stream;
}

GaussianSplattingBufferBuilder::GaussianSplattingBufferBuilder(std::unique_ptr<ResourceWriter>&& resourceWriter) : m_resourceWriter(std::move(resourceWriter))
{
}

const Buffer& GaussianSplattingBufferBuilder::AddBuffer(const char* bufferId)
{
	Buffer buffer;
	if (bufferId)
	{
		buffer.id = bufferId;
	}
	buffer.byteLength = 0U;// The buffer's length is updated whenever an Accessor or BufferView is added (and data is written to the underlying buffer)
	auto& bufferRef = m_buffers.Append(std::move(buffer), AppendIdPolicy::GenerateOnEmpty);
	if (m_resourceWriter)
		bufferRef.uri = m_resourceWriter->GenerateBufferUri(bufferRef.id);
	return bufferRef;
}

const BufferView& GaussianSplattingBufferBuilder::AddBufferView(const void* data, size_t byteLength, Optional<size_t> byteStride, Optional<BufferViewTarget> target)
{
	Buffer& buffer = m_buffers.Back();
	BufferView bufferView;

	bufferView.bufferId = buffer.id;
	bufferView.byteOffset = buffer.byteLength;
	bufferView.byteLength = byteLength;
	bufferView.byteStride = byteStride;
	bufferView.target = target;

	buffer.byteLength = bufferView.byteOffset + bufferView.byteLength;

	if (m_resourceWriter)
	{
		m_resourceWriter->Write(bufferView, data);
	}

	return m_bufferViews.Append(std::move(bufferView), AppendIdPolicy::GenerateOnEmpty);
}

const Buffer& GaussianSplattingBufferBuilder::GetCurrentBuffer() const
{
	return m_buffers.Back();
}

const BufferView& GaussianSplattingBufferBuilder::GetCurrentBufferView() const
{
	return m_bufferViews.Back();
}

const Accessor& GaussianSplattingBufferBuilder::GetCurrentAccessor() const
{
	return m_accessors.Back();
}

size_t GaussianSplattingBufferBuilder::GetBufferCount() const
{
	return m_buffers.Size();
}

size_t GaussianSplattingBufferBuilder::GetBufferViewCount() const
{
	return m_bufferViews.Size();
}

size_t GaussianSplattingBufferBuilder::GetAccessorCount() const
{
	return m_accessors.Size();
}

ResourceWriter& GaussianSplattingBufferBuilder::GetResourceWriter()
{
	return *m_resourceWriter;
}

const ResourceWriter& GaussianSplattingBufferBuilder::GetResourceWriter() const
{
	return *m_resourceWriter;
}

const Accessor& GaussianSplattingBufferBuilder::AddAccessor(size_t count, AccessorDesc desc)
{
	//Buffer& buffer = m_buffers.Back();
	//BufferView& bufferView = m_bufferViews.Back();


	if (count == 0)
	{
		throw GLTFException("Invalid accessor count: 0");
	}

	if (desc.accessorType == TYPE_UNKNOWN)
	{
		throw GLTFException("Invalid accessorType: TYPE_UNKNOWN");
	}

	if (desc.componentType == COMPONENT_UNKNOWN)
	{
		throw GLTFException("Invalid componentType: COMPONENT_UNKNOWN");
	}

	const auto accessorTypeSize = Accessor::GetTypeCount(desc.accessorType);
	size_t componentTypeSize = Accessor::GetComponentTypeSize(desc.componentType);

	// Only check for a valid number of min and max values if they exist
	if ((!desc.minValues.empty() || !desc.maxValues.empty()) &&
		((desc.minValues.size() != accessorTypeSize) || (desc.maxValues.size() != accessorTypeSize)))
	{
		throw InvalidGLTFException("the number of min and max values must be equal to the number of elements to be stored in the accessor");
	}

	if (desc.byteOffset % componentTypeSize != 0)
	{
		throw InvalidGLTFException("asccessor offset within buffer view must be a multiple of the component size");
	}

	Accessor accessor;


	accessor.count = count;
	accessor.byteOffset = desc.byteOffset;
	accessor.type = desc.accessorType;
	accessor.componentType = desc.componentType;
	accessor.normalized = desc.normalized;

	// TODO: make accessor min & max members be vectors of doubles
	accessor.min = desc.minValues;
	accessor.max = desc.maxValues;

	return m_accessors.Append(std::move(accessor), AppendIdPolicy::GenerateOnEmpty);
}
