#include "stdafx.h"
#include "ci_meshopttool.h"
#include "gltf/gltfpack.h"
#include "meshoptimizer.h"

#pragma region gltfpack
static void writeU32(std::string& out, uint32_t data)
{
	out.append((char*)&data, 4);
}
static void writeU32(CGByteArray& out, uint32_t data)
{
	out.append((char*)&data, 4);
}
Settings getSettingsDefaults()
{
	Settings settings = {};
	settings.quantize = false;
	settings.pos_bits = 14;
	settings.tex_bits = 12;
	settings.nrm_bits = 8;
	settings.col_bits = 8;
	settings.trn_bits = 16;
	settings.rot_bits = 12;
	settings.scl_bits = 16;
	settings.anim_freq = 30;
#if MESHOPTIMIZER_VERSION == 200
	settings.simplify_threshold = 1.f;
#else
	settings.mesh_dedup = true ;
	settings.simplify_ratio = 1.f;
	settings.simplify_error = 1e-2f;
#endif

	settings.texture_scale = 1.f;
	for (int kind = 0; kind < TextureKind__Count; ++kind)
		settings.texture_quality[kind] = 8;

	settings.pos_normalized = false;
	settings.pos_float = false;
	settings.tex_float = false;
	//settings.nrm_float = false;
	settings.anim_const = false;

	settings.keep_nodes = false;
	settings.keep_materials = false;
	settings.keep_extras = false;

	settings.mesh_merge = false;
	settings.mesh_instancing = false;

	settings.simplify_aggressive = false;
	settings.simplify_lock_borders = false;
	settings.simplify_debug = 0;

	settings.meshlet_debug = 0;

	settings.texture_ktx2 = false;
	settings.texture_embed = false;
	settings.texture_ref = false;

	settings.texture_pow2 = false;
	settings.texture_flipy = false;
	settings.texture_limit = 0;
	settings.texture_jobs = 0;
	settings.quantize = false;
	settings.compress = true;
	settings.compressmore = true;
	settings.fallback = false;
	settings.verbose = 0;
	return settings;
}

bool MeshOpt_Gltfpack::GeometryCompress(const string& in, string& out)
{
	Settings settings = getSettingsDefaults();
	settings.compress = true;
	settings.compressmore = true;
	const char* error = NULL;
	cgltf_data* data = NULL;
	std::vector<Mesh> meshes;
	std::vector<Animation> animations;
	settings.texture_ktx2 = false;//该功能不支持转换纹理图片
	data = parseGlb(in.c_str(), in.size(), meshes, animations, &error);
	if (error)
		return false;

	std::string json, bin, fallback;
	size_t fallback_size = 0;
	process(data, NULL, NULL, NULL, meshes, animations, settings, json, bin, fallback, fallback_size);
	cgltf_free(data);

	out.clear();
	std::string bufferspec = getBufferSpec(NULL, bin.size(), NULL, fallback_size, settings.compress);
	json.insert(0, "{" + bufferspec + ",");
	json.push_back('}');
	while (json.size() % 4)
		json.push_back(' ');
	while (bin.size() % 4)
		bin.push_back('\0');
	writeU32(out, 0x46546C67);
	writeU32(out, 2);
	writeU32(out, uint32_t(12 + 8 + json.size() + 8 + bin.size()));
	writeU32(out, uint32_t(json.size()));
	writeU32(out, 0x4E4F534A);
	out.append(json.c_str(), json.size());
	writeU32(out, uint32_t(bin.size()));
	writeU32(out, 0x004E4942);
	out.append(bin.c_str(), bin.size());
	return  true;
}

bool MeshOpt_Gltfpack::GeometryDecompression(const string& in, string& out)
{
	Settings settings = getSettingsDefaults();
	settings.compress = false;
	settings.compressmore = false;
	settings.quantize = false;
	const char* error = NULL;
	cgltf_data* data = NULL;
	std::vector<Mesh> meshes;
	std::vector<Animation> animations;
	settings.texture_ktx2 = false;//该功能不支持转换纹理图片
	data = parseGlb(in.c_str(), in.size(), meshes, animations, &error);
	if (error)
		return false;

	std::string json, bin, fallback;
	size_t fallback_size = 0;
	process(data, NULL, NULL, NULL, meshes, animations, settings, json, bin, fallback, fallback_size);
	cgltf_free(data);

	out.clear();
	std::string bufferspec = getBufferSpec(NULL, bin.size(), NULL, fallback_size, settings.compress);
	json.insert(0, "{" + bufferspec + ",");
	json.push_back('}');
	while (json.size() % 4)
		json.push_back(' ');
	while (bin.size() % 4)
		bin.push_back('\0');
	writeU32(out, 0x46546C67);
	writeU32(out, 2);
	writeU32(out, uint32_t(12 + 8 + json.size() + 8 + bin.size()));
	writeU32(out, uint32_t(json.size()));
	writeU32(out, 0x4E4F534A);
	out.append(json.c_str(), json.size());
	writeU32(out, uint32_t(bin.size()));
	writeU32(out, 0x004E4942);
	out.append(bin.c_str(), bin.size());
	return  true;
}

bool MeshOpt_Gltfpack::GeometryCompress(const string& in, CGByteArray& out)
{
	Settings settings = getSettingsDefaults();
	settings.compress = true;
	settings.compressmore = true;
	const char* error = NULL;
	cgltf_data* data = NULL;
	std::vector<Mesh> meshes;
	std::vector<Animation> animations;
	settings.texture_ktx2 = false;//该功能不支持转换纹理图片
	data = parseGlb(in.c_str(), in.size(), meshes, animations, &error);
	if (error)
		return false;

	std::string json, bin, fallback;
	size_t fallback_size = 0;
	process(data, NULL, NULL, NULL, meshes, animations, settings, json, bin, fallback, fallback_size);
	cgltf_free(data);

	out.clear();
	std::string bufferspec = getBufferSpec(NULL, bin.size(), NULL, fallback_size, settings.compress);
	json.insert(0, "{" + bufferspec + ",");
	json.push_back('}');
	while (json.size() % 4)
		json.push_back(' ');
	while (bin.size() % 4)
		bin.push_back('\0');
	writeU32(out, 0x46546C67);
	writeU32(out, 2);
	writeU32(out, uint32_t(12 + 8 + json.size() + 8 + bin.size()));
	writeU32(out, uint32_t(json.size()));
	writeU32(out, 0x4E4F534A);
	out.append(json.c_str(), json.size());
	writeU32(out, uint32_t(bin.size()));
	writeU32(out, 0x004E4942);
	out.append(bin.c_str(), bin.size());
	return  true;
}

bool MeshOpt_Gltfpack::GeometryDecompression(const string& in, CGByteArray& out)
{
	Settings settings = getSettingsDefaults();
	settings.compress = false;
	settings.compressmore = false;
	settings.quantize = false;
	const char* error = NULL;
	cgltf_data* data = NULL;
	std::vector<Mesh> meshes;
	std::vector<Animation> animations;
	settings.texture_ktx2 = false;//该功能不支持转换纹理图片
	data = parseGlb(in.c_str(), in.size(), meshes, animations, &error);
	if (error)
		return false;

	std::string json, bin, fallback;
	size_t fallback_size = 0;
	process(data, NULL, NULL, NULL, meshes, animations, settings, json, bin, fallback, fallback_size);
	cgltf_free(data);

	out.clear();
	std::string bufferspec = getBufferSpec(NULL, bin.size(), NULL, fallback_size, settings.compress);
	json.insert(0, "{" + bufferspec + ",");
	json.push_back('}');
	while (json.size() % 4)
		json.push_back(' ');
	while (bin.size() % 4)
		bin.push_back('\0');
	writeU32(out, 0x46546C67);
	writeU32(out, 2);
	writeU32(out, uint32_t(12 + 8 + json.size() + 8 + bin.size()));
	writeU32(out, uint32_t(json.size()));
	writeU32(out, 0x4E4F534A);
	out.append(json.c_str(), json.size());
	writeU32(out, uint32_t(bin.size()));
	writeU32(out, 0x004E4942);
	out.append(bin.c_str(), bin.size());
	return  true;
}

bool MeshOpt_Gltfpack::GeometryCompress(const CGByteArray& in, CGByteArray& out)
{
	Settings settings = getSettingsDefaults();
	settings.compress = true;
	settings.compressmore = true;
	const char* error = NULL;
	cgltf_data* data = NULL;
	std::vector<Mesh> meshes;
	std::vector<Animation> animations;
	settings.texture_ktx2 = false;//该功能不支持转换纹理图片
	data = parseGlb(in.data(), in.size(), meshes, animations, &error);
	if (error)
		return false;

	std::string json, bin, fallback;
	size_t fallback_size = 0;
	process(data, NULL, NULL, NULL, meshes, animations, settings, json, bin, fallback, fallback_size);
	cgltf_free(data);

	out.clear();
	std::string bufferspec = getBufferSpec(NULL, bin.size(), NULL, fallback_size, settings.compress);
	json.insert(0, "{" + bufferspec + ",");
	json.push_back('}');
	while (json.size() % 4)
		json.push_back(' ');
	while (bin.size() % 4)
		bin.push_back('\0');
	writeU32(out, 0x46546C67);
	writeU32(out, 2);
	writeU32(out, uint32_t(12 + 8 + json.size() + 8 + bin.size()));
	writeU32(out, uint32_t(json.size()));
	writeU32(out, 0x4E4F534A);
	out.append(json.c_str(), json.size());
	writeU32(out, uint32_t(bin.size()));
	writeU32(out, 0x004E4942);
	out.append(bin.c_str(), bin.size());
	return  true;
}

bool MeshOpt_Gltfpack::GeometryDecompression(const CGByteArray& in, CGByteArray& out)
{
	Settings settings = getSettingsDefaults();
	settings.compress = false;
	settings.compressmore = false;
	settings.quantize = false;
	const char* error = NULL;
	cgltf_data* data = NULL;
	std::vector<Mesh> meshes;
	std::vector<Animation> animations;
	settings.texture_ktx2 = false;//该功能不支持转换纹理图片
	data = parseGlb(in.data(), in.size(), meshes, animations, &error);
	if (error)
		return false;

	std::string json, bin, fallback;
	size_t fallback_size = 0;
	process(data, NULL, NULL, NULL, meshes, animations, settings, json, bin, fallback, fallback_size);
	cgltf_free(data);

	out.clear();
	std::string bufferspec = getBufferSpec(NULL, bin.size(), NULL, fallback_size, settings.compress);
	json.insert(0, "{" + bufferspec + ",");
	json.push_back('}');
	while (json.size() % 4)
		json.push_back(' ');
	while (bin.size() % 4)
		bin.push_back('\0');
	writeU32(out, 0x46546C67);
	writeU32(out, 2);
	writeU32(out, uint32_t(12 + 8 + json.size() + 8 + bin.size()));
	writeU32(out, uint32_t(json.size()));
	writeU32(out, 0x4E4F534A);
	out.append(json.c_str(), json.size());
	writeU32(out, uint32_t(bin.size()));
	writeU32(out, 0x004E4942);
	out.append(bin.c_str(), bin.size());
	return  true;
}
#pragma endregion