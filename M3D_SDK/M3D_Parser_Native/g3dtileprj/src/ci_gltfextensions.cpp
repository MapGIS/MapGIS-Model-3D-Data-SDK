// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#include "stdafx.h"
#include "ci_gltfextensions.h"
#include <GLTFSDK/Document.h>
#include <GLTFSDK/RapidJsonUtils.h>

using namespace Microsoft::glTF;

namespace
{
	void ParseExtensions(const rapidjson::Value& v, glTFProperty& node, const ExtensionDeserializer& extensionDeserializer)
	{
		const auto& extensionsIt = v.FindMember("extensions");
		if (extensionsIt != v.MemberEnd())
		{
			const rapidjson::Value& extensionsObject = extensionsIt->value;
			for (const auto& entry : extensionsObject.GetObject())
			{
				ExtensionPair extensionPair = { entry.name.GetString(), Serialize(entry.value) };

				if (extensionDeserializer.HasHandler(extensionPair.name, node) ||
					extensionDeserializer.HasHandler(extensionPair.name))
				{
					node.SetExtension(extensionDeserializer.Deserialize(extensionPair, node));
				}
				else
				{
					node.extensions.emplace(std::move(extensionPair.name), std::move(extensionPair.value));
				}
			}
		}
	}

	void ParseExtras(const rapidjson::Value& v, glTFProperty& node)
	{
		rapidjson::Value::ConstMemberIterator it;
		if (TryFindMember("extras", v, it))
		{
			const rapidjson::Value& a = it->value;
			node.extras = Serialize(a);
		}
	}

	void ParseProperty(const rapidjson::Value& v, glTFProperty& node, const ExtensionDeserializer& extensionDeserializer)
	{
		ParseExtensions(v, node, extensionDeserializer);
		ParseExtras(v, node);
	}

	void SerializePropertyExtensions(const Document& gltfDocument, const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a, const ExtensionSerializer& extensionSerializer)
	{
		auto registeredExtensions = property.GetExtensions();

		if (!property.extensions.empty() || !registeredExtensions.empty())
		{
			rapidjson::Value& extensions = RapidJsonUtils::FindOrAddMember(propertyValue, "extensions", a);

			// Add registered extensions
			for (const auto& extension : registeredExtensions)
			{
				const auto extensionPair = extensionSerializer.Serialize(extension, property, gltfDocument);

				if (property.HasUnregisteredExtension(extensionPair.name))
				{
					throw GLTFException("Registered extension '" + extensionPair.name + "' is also present as an unregistered extension.");
				}

				if (gltfDocument.extensionsUsed.find(extensionPair.name) == gltfDocument.extensionsUsed.end())
				{
					throw GLTFException("Registered extension '" + extensionPair.name + "' is not present in extensionsUsed");
				}

				const auto d = RapidJsonUtils::CreateDocumentFromString(extensionPair.value);//TODO: validate the returned document against the extension schema!
				rapidjson::Value v(rapidjson::kObjectType);
				v.CopyFrom(d, a);
				extensions.AddMember(RapidJsonUtils::ToStringValue(extensionPair.name, a), v, a);
			}

			// Add unregistered extensions
			for (const auto& extension : property.extensions)
			{
				const auto d = RapidJsonUtils::CreateDocumentFromString(extension.second);
				rapidjson::Value v(rapidjson::kObjectType);
				v.CopyFrom(d, a);
				extensions.AddMember(RapidJsonUtils::ToStringValue(extension.first, a), v, a);
			}
		}
	}

	void SerializePropertyExtras(const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a)
	{
		if (!property.extras.empty())
		{
			auto d = RapidJsonUtils::CreateDocumentFromString(property.extras);
			rapidjson::Value v(rapidjson::kObjectType);
			v.CopyFrom(d, a);
			propertyValue.AddMember("extras", v, a);
		}
	}

	void SerializeProperty(const Document& gltfDocument, const glTFProperty& property, rapidjson::Value& propertyValue, rapidjson::Document::AllocatorType& a, const ExtensionSerializer& extensionSerializer)
	{
		SerializePropertyExtensions(gltfDocument, property, propertyValue, a, extensionSerializer);
		SerializePropertyExtras(property, propertyValue, a);
	}
}
// KHR::TextureBasiuInfos::TextureBasisu

KHR::TextureBasiuInfos::TextureBasisu::TextureBasisu() :
	source("0")
{
}

KHR::TextureBasiuInfos::TextureBasisu::TextureBasisu(const TextureBasisu& textureBasisu) :
	source(textureBasisu.source)
{
}

std::unique_ptr<Extension> KHR::TextureBasiuInfos::TextureBasisu::Clone() const
{
#ifdef _WIN32
	return std::make_unique<TextureBasisu>(*this);
#else
	return std::unique_ptr<Extension>(new TextureBasisu(*this));
#endif
}

bool KHR::TextureBasiuInfos::TextureBasisu::IsEqual(const Extension& rhs) const
{
	const auto other = dynamic_cast<const TextureBasisu*>(&rhs);

	return other != nullptr
		&& glTFProperty::Equals(*this, *other)
		&& this->source == other->source;
}

std::string KHR::TextureBasiuInfos::SerializeTextureBasisu(const TextureBasisu& textureBasisu, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
	rapidjson::Document doc;
	auto& a = doc.GetAllocator();
	rapidjson::Value KHR_textureBasisu(rapidjson::kObjectType);
	{
		if (!textureBasisu.source.empty())
		{
			int nId = atoi(textureBasisu.source.c_str());
			KHR_textureBasisu.AddMember("source", nId, a);
		}

		SerializeProperty(gltfDocument, textureBasisu, KHR_textureBasisu, a, extensionSerializer);
	}

	glTF::rapidjson::StringBuffer buffer;
	glTF::rapidjson::Writer<glTF::rapidjson::StringBuffer> writer(buffer);
	KHR_textureBasisu.Accept(writer);

	return buffer.GetString();
}

std::unique_ptr<Extension> KHR::TextureBasiuInfos::DeserializeTextureBasisu(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
	TextureBasisu textureBasisu;

	auto doc = RapidJsonUtils::CreateDocumentFromString(json);
	const auto sit = doc.GetObject();

	// source
	textureBasisu.source = std::to_string(GetMemberValueOrDefault<std::int32_t>(sit, "source", 0));

	ParseProperty(sit, textureBasisu, extensionDeserializer);

#ifdef _WIN32
	return std::make_unique<TextureBasisu>(textureBasisu);
#else
	return std::unique_ptr<Extension>(new TextureBasisu(textureBasisu));
#endif
}

// Vendor::TextureWebpInfos::TextureWebp

Vendor::TextureWebpInfos::TextureWebp::TextureWebp() :
	source("0")
{
}

Vendor::TextureWebpInfos::TextureWebp::TextureWebp(const TextureWebp& textureWebp) :
	source(textureWebp.source)
{
}

std::unique_ptr<Extension> Vendor::TextureWebpInfos::TextureWebp::Clone() const
{
#ifdef _WIN32
	return std::make_unique<TextureWebp>(*this);
#else
	return std::unique_ptr<Extension>(new TextureWebp(*this));
#endif
}

bool Vendor::TextureWebpInfos::TextureWebp::IsEqual(const Extension& rhs) const
{
	const auto other = dynamic_cast<const TextureWebp*>(&rhs);

	return other != nullptr
		&& glTFProperty::Equals(*this, *other)
		&& this->source == other->source;
}

std::string Vendor::TextureWebpInfos::SerializeTextureWebp(const TextureWebp& textureWebp, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
	rapidjson::Document doc;
	auto& a = doc.GetAllocator();
	rapidjson::Value EXT_textureWebp(rapidjson::kObjectType);
	{
		if (!textureWebp.source.empty())
		{
			int nId = atoi(textureWebp.source.c_str());
			EXT_textureWebp.AddMember("source", nId, a);
		}

		SerializeProperty(gltfDocument, textureWebp, EXT_textureWebp, a, extensionSerializer);
	}

	glTF::rapidjson::StringBuffer buffer;
	glTF::rapidjson::Writer<glTF::rapidjson::StringBuffer> writer(buffer);
	EXT_textureWebp.Accept(writer);

	return buffer.GetString();
}

std::unique_ptr<Extension> Vendor::TextureWebpInfos::DeserializeTextureWebp(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
	TextureWebp textureWebp;

	auto doc = RapidJsonUtils::CreateDocumentFromString(json);
	const auto sit = doc.GetObject();

	// source
	textureWebp.source = std::to_string(GetMemberValueOrDefault<std::int32_t>(sit, "source", 0)) ;

	ParseProperty(sit, textureWebp, extensionDeserializer);

#ifdef _WIN32
	return std::make_unique<TextureWebp>(textureWebp);
#else
	return std::unique_ptr<Extension>(new TextureWebp(textureWebp));
#endif
}

// Vendor::MeshGpuInstancingInfos::MeshGpuInstancingInfos

Vendor::MeshGpuInstancingInfos::MeshGpuInstancing::MeshGpuInstancing():
	translatton(""), rotation(""), scale(""), batchid("")
{
}

Vendor::MeshGpuInstancingInfos::MeshGpuInstancing::MeshGpuInstancing(const MeshGpuInstancing&instancing) :
	translatton(instancing.translatton), rotation(instancing.rotation), scale(instancing.scale), batchid(instancing.batchid)
{
}

std::unique_ptr<Extension> Vendor::MeshGpuInstancingInfos::MeshGpuInstancing::Clone() const
{
#ifdef _WIN32
	return std::make_unique<MeshGpuInstancing>(*this);
#else
	return std::unique_ptr<Extension>(new MeshGpuInstancing(*this));
#endif
}

bool Vendor::MeshGpuInstancingInfos::MeshGpuInstancing::IsEqual(const Extension& rhs) const
{
	const auto other = dynamic_cast<const MeshGpuInstancing*>(&rhs);

	return other != nullptr
		&& glTFProperty::Equals(*this, *other)
		&& this->translatton == other->translatton
		&& this->rotation == other->rotation
		&& this->scale == other->scale
		&& this->batchid == other->batchid;
}

std::string Vendor::MeshGpuInstancingInfos::SerializeMeshGpuInstancing(const MeshGpuInstancing& instancing, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
	rapidjson::Document doc;
	auto& a = doc.GetAllocator();
	rapidjson::Value EXT_instancing(rapidjson::kObjectType);
	rapidjson::Value attributes(rapidjson::kObjectType);
	{
		if (!instancing.translatton.empty())
		{
			int nId = atoi(instancing.translatton.c_str());
			attributes.AddMember("TRANSLATION", nId, a);
		}
		if (!instancing.rotation.empty())
		{
			int nId = atoi(instancing.rotation.c_str());
			attributes.AddMember("ROTATION", nId, a);
		}
		if (!instancing.scale.empty())
		{
			int nId = atoi(instancing.scale.c_str());
			attributes.AddMember("SCALE", nId, a);
		}
		if (!instancing.batchid.empty())
		{
			int nId = atoi(instancing.batchid.c_str());
			attributes.AddMember("_BATCHID", nId, a);
		}
		EXT_instancing.AddMember("attributes", attributes, a);
	}
	glTF::rapidjson::StringBuffer buffer;
	glTF::rapidjson::Writer<glTF::rapidjson::StringBuffer> writer(buffer);
	EXT_instancing.Accept(writer);
	return buffer.GetString();
}

std::unique_ptr<Extension> Vendor::MeshGpuInstancingInfos::DeserializeMeshGpuInstancing(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
	MeshGpuInstancing instancing;
	auto doc = RapidJsonUtils::CreateDocumentFromString(json);
	const auto& sit = doc.GetObject();
	auto JsonValueToInt = [](const rapidjson::Value& jsVal)
	{
		int val = 0;
		if (jsVal.IsInt())
		{
			val = jsVal.GetInt();
		}
		else if (jsVal.IsNumber())
		{
			double  value = jsVal.GetDouble();
			if (value > -2147483648 - 1E-6 && value < 2147483647 + 1E-6 && fabs(value - (int)value) < 1E-6)
			{
				val = (int)value;
			}
		}
		return val;
	};

	if (sit.HasMember("attributes"))
	{
		const auto& attributes =  sit.FindMember("attributes");

		if (attributes->value.IsObject())
		{
			const auto &objAttributes = attributes->value.GetObject();

			if (objAttributes.HasMember("TRANSLATION"))
			{
				auto it = objAttributes.FindMember("TRANSLATION");
				if (it != objAttributes.MemberEnd())
				{
					instancing.translatton = std::to_string(JsonValueToInt(it->value));
				}
			}
			if (objAttributes.HasMember("ROTATION"))
			{
				auto it = objAttributes.FindMember("ROTATION");
				if (it != objAttributes.MemberEnd())
				{
					instancing.rotation = std::to_string(JsonValueToInt(it->value));
				}
			}
			if (objAttributes.HasMember("SCALE"))
			{
				auto it = objAttributes.FindMember("SCALE");
				if (it != objAttributes.MemberEnd())
				{
					instancing.scale = std::to_string(JsonValueToInt(it->value));
				}
			}
			if (objAttributes.HasMember("_BATCHID"))
			{
				auto it = objAttributes.FindMember("_BATCHID");
				if (it != objAttributes.MemberEnd())
				{
					instancing.batchid = std::to_string(JsonValueToInt(it->value));
				}
			}
		}
	}

#ifdef _WIN32
	return std::make_unique<MeshGpuInstancing>(instancing);
#else
	return std::unique_ptr<Extension>(new MeshGpuInstancing(instancing));
#endif
}



// KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2

KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2() :
	bufferViewId("0")
{
}

KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2(const GaussianSplattingCompressionSpz2& textureBasisu) :
	bufferViewId(textureBasisu.bufferViewId)
{
}


std::unique_ptr<Extension> KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2::Clone() const
{
#ifndef MAPGIS_LINUX
	return std::make_unique<GaussianSplattingCompressionSpz2>(*this);
#else
	return std::unique_ptr<Extension>(new GaussianSplattingCompressionSpz2(*this));
#endif
}


bool KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::GaussianSplattingCompressionSpz2::IsEqual(const Extension& rhs) const
{
	const auto other = dynamic_cast<const GaussianSplattingCompressionSpz2*>(&rhs);

	return other != nullptr
		&& glTFProperty::Equals(*this, *other)
		&& this->bufferViewId == other->bufferViewId;
}



std::string KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::SerializeGaussianSplattingCompressionSpz2(const GaussianSplattingCompressionSpz2& spz2, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer)
{
	rapidjson::Document doc;
	auto& a = doc.GetAllocator();
	rapidjson::Value KHRGaussianSplattingCompressionSpz2(rapidjson::kObjectType);
	rapidjson::Value KHRGaussianSplatting(rapidjson::kObjectType);
	rapidjson::Value KHRGaussianSplattingExtensions(rapidjson::kObjectType);
	{
		int nId = atoi(spz2.bufferViewId.c_str());
		KHRGaussianSplattingCompressionSpz2.AddMember("bufferView", nId, a);
		KHRGaussianSplattingExtensions.AddMember("KHR_gaussian_splatting_compression_spz_2", KHRGaussianSplattingCompressionSpz2, a);
		KHRGaussianSplatting.AddMember("extensions", KHRGaussianSplattingExtensions, a);
	}
	glTF::rapidjson::StringBuffer buffer;
	glTF::rapidjson::Writer<glTF::rapidjson::StringBuffer> writer(buffer);
	KHRGaussianSplatting.Accept(writer);

	//glTF::rapidjson::StringBuffer buffer;
	return buffer.GetString();
}


std::unique_ptr<Extension> KHR::MeshPrimitiveGaussianSplattingCompressionSpz2::DeserializeGaussianSplattingCompressionSpz2(const std::string& json, const ExtensionDeserializer& extensionDeserializer)
{
	GaussianSplattingCompressionSpz2 spz2;
	auto doc = RapidJsonUtils::CreateDocumentFromString(json);
	const auto& sit = doc.GetObject();
	if (sit.HasMember("extensions"))
	{
		const auto& extensions = sit.FindMember("extensions");
		if (extensions->value.IsObject())
		{
			const auto &objExtensions = extensions->value.GetObject();

			if (objExtensions.HasMember("KHR_gaussian_splatting_compression_spz_2"))
			{
				const auto& spz2Extensions = objExtensions.FindMember("KHR_gaussian_splatting_compression_spz_2");

				if (spz2Extensions->value.IsObject())
				{
					const auto &objSpz2Extensions = spz2Extensions->value.GetObject();
					if (objSpz2Extensions.HasMember("bufferView"))
					{
						const auto& bufferView = objSpz2Extensions.FindMember("bufferView");
						if (bufferView != objSpz2Extensions.MemberEnd())
						{
							spz2.bufferViewId = std::to_string(bufferView->value.GetInt());
						}
					}
				}
			}
		}
	}
#ifndef MAPGIS_LINUX
	return std::make_unique<GaussianSplattingCompressionSpz2>(spz2);
#else
	return std::unique_ptr<Extension>(new GaussianSplattingCompressionSpz2(spz2));
#endif
}
