// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef CI_GLTFEXTENSIONS_H
#define CI_GLTFEXTENSIONS_H

#include <GLTFSDK/ExtensionHandlers.h>
#include <GLTFSDK/Optional.h>

#include <memory>
#include <string>

namespace Microsoft
{
    namespace glTF
    {
        namespace KHR
        {
            namespace TextureBasiuInfos
            {
                constexpr const char* TEXTUREBASIU_NAME = "KHR_texture_basisu";

                //================================================================================
                /// KHR_texture_basisu扩展结构体
                ///
                /// 支持Basis Universal纹理格式的glTF扩展
                /// 继承自Extension和glTFProperty基类
                //================================================================================
                struct TextureBasisu : Extension, glTFProperty
                {
                    //================================================================================
                    /// 默认构造函数
                    //================================================================================
                    TextureBasisu();

                    //================================================================================
                    /// 拷贝构造函数
                    ///
                    /// @param [in] other 源对象
                    //================================================================================
                    TextureBasisu(const TextureBasisu& other);

                    std::string source;

                    //================================================================================
                    /// 克隆扩展对象
                    ///
                    /// @return 返回当前对象的深拷贝副本
                    //================================================================================
                    std::unique_ptr<Extension> Clone() const override;

                    //================================================================================
                    /// 比较两个扩展对象是否相等
                    ///
                    /// @param [in] rhs 右侧比较对象
                    /// @return 相等返回true，否则返回false
                    //================================================================================
                    bool IsEqual(const Extension& rhs) const override;
                };

                //================================================================================
                /// 序列化TextureBasisu对象为JSON字符串
                ///
                /// @param [in] textureBasisu      要序列化的TextureBasisu对象
                /// @param [in] gltfDocument       关联的glTF文档对象
                /// @param [in] extensionSerializer 扩展序列化器
                ///
                /// @return 返回序列化后的JSON字符串
                //================================================================================
                std::string SerializeTextureBasisu(const TextureBasisu& textureBasisu, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer);

                //================================================================================
                /// 从JSON字符串反序列化为TextureBasisu对象
                ///
                /// @param [in] json                  JSON格式的字符串数据
                /// @param [in] extensionDeserializer 扩展反序列化器
                ///
                /// @return 返回反序列化后的TextureBasisu扩展对象
                //================================================================================
                std::unique_ptr<Extension> DeserializeTextureBasisu(const std::string& json, const ExtensionDeserializer& extensionDeserializer);
            }

            namespace MeshPrimitiveGaussianSplattingCompressionSpz2
            {
                constexpr const char* gaussiansplatting_name = "KHR_gaussian_splatting";

                constexpr const char* gaussiansplattingCompressionSpz2_name = "KHR_gaussian_splatting_compression_spz_2";

                //================================================================================
                /// KHR_gaussian_splatting_compression_spz_2扩展结构体
                ///
                /// 支持高斯点阵渲染数据压缩的glTF扩展
                /// 继承自Extension和glTFProperty基类
                //================================================================================
                struct GaussianSplattingCompressionSpz2 : Extension, glTFProperty
                {
                    //================================================================================
                    /// 默认构造函数
                    //================================================================================
                    GaussianSplattingCompressionSpz2();

                    //================================================================================
                    /// 拷贝构造函数
                    ///
                    /// @param [in] other 源对象
                    //================================================================================
                    GaussianSplattingCompressionSpz2(const GaussianSplattingCompressionSpz2& other);

                    std::string bufferViewId;

                    //================================================================================
                    /// 克隆扩展对象
                    ///
                    /// @return 返回当前对象的深拷贝副本
                    //================================================================================
                    std::unique_ptr<Extension> Clone() const override;

                    //================================================================================
                    /// 比较两个扩展对象是否相等
                    ///
                    /// @param [in] rhs 右侧比较对象
                    /// @return 相等返回true，否则返回false
                    //================================================================================
                    bool IsEqual(const Extension& rhs) const override;
                };

                //================================================================================
                /// 序列化GaussianSplattingCompressionSpz2对象为JSON字符串
                ///
                /// @param [in] gaussianSplatting     要序列化的GaussianSplattingCompressionSpz2对象
                /// @param [in] gltfDocument          关联的glTF文档对象
                /// @param [in] extensionSerializer   扩展序列化器
                ///
                /// @return 返回序列化后的JSON字符串
                //================================================================================
                std::string SerializeGaussianSplattingCompressionSpz2(const GaussianSplattingCompressionSpz2& gaussianSplatting, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer);

                //================================================================================
                /// 从JSON字符串反序列化为GaussianSplattingCompressionSpz2对象
                ///
                /// @param [in] json                  JSON格式的字符串数据
                /// @param [in] extensionDeserializer 扩展反序列化器
                ///
                /// @return 返回反序列化后的GaussianSplattingCompressionSpz2扩展对象
                //================================================================================
                std::unique_ptr<Extension> DeserializeGaussianSplattingCompressionSpz2(const std::string& json, const ExtensionDeserializer& extensionDeserializer);
            }
        }

        namespace Vendor
        {
            namespace TextureWebpInfos
            {
                constexpr const char* TEXTUREWEBP_NAME = "EXT_texture_webp";

                //================================================================================
                /// EXT_texture_webp扩展结构体
                ///
                /// 支持WebP纹理格式的glTF扩展
                /// 继承自Extension和glTFProperty基类
                //================================================================================
                struct TextureWebp : Extension, glTFProperty
                {
                    //================================================================================
                    /// 默认构造函数
                    //================================================================================
                    TextureWebp();

                    //================================================================================
                    /// 拷贝构造函数
                    ///
                    /// @param [in] other 源对象
                    //================================================================================
                    TextureWebp(const TextureWebp& other);

                    std::string source;

                    //================================================================================
                    /// 克隆扩展对象
                    ///
                    /// @return 返回当前对象的深拷贝副本
                    //================================================================================
                    std::unique_ptr<Extension> Clone() const override;

                    //================================================================================
                    /// 比较两个扩展对象是否相等
                    ///
                    /// @param [in] rhs 右侧比较对象
                    /// @return 相等返回true，否则返回false
                    //================================================================================
                    bool IsEqual(const Extension& rhs) const override;
                };

                //================================================================================
                /// 序列化TextureWebp对象为JSON字符串
                ///
                /// @param [in] textureWebp           要序列化的TextureWebp对象
                /// @param [in] gltfDocument          关联的glTF文档对象
                /// @param [in] extensionSerializer   扩展序列化器
                ///
                /// @return 返回序列化后的JSON字符串
                //================================================================================
                std::string SerializeTextureWebp(const TextureWebp& textureWebp, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer);

                //================================================================================
                /// 从JSON字符串反序列化为TextureWebp对象
                ///
                /// @param [in] json                  JSON格式的字符串数据
                /// @param [in] extensionDeserializer 扩展反序列化器
                ///
                /// @return 返回反序列化后的TextureWebp扩展对象
                //================================================================================
                std::unique_ptr<Extension> DeserializeTextureWebp(const std::string& json, const ExtensionDeserializer& extensionDeserializer);
            }

            namespace MeshGpuInstancingInfos
            {
                constexpr const char* MESH_GPU_INSTANCING_NAME = "EXT_mesh_gpu_instancing";

                //================================================================================
                /// EXT_mesh_gpu_instancing扩展类
                ///
                /// 支持GPU实例化渲染的glTF扩展
                /// 继承自Extension和glTFProperty基类
                //================================================================================
                class MeshGpuInstancing : public Extension, glTFProperty
                {
                public:
                    //================================================================================
                    /// 默认构造函数
                    //================================================================================
                    MeshGpuInstancing();

                    //================================================================================
                    /// 拷贝构造函数
                    ///
                    /// @param [in] other 源对象
                    //================================================================================
                    MeshGpuInstancing(const MeshGpuInstancing& other);

                    std::string translatton;
                    std::string rotation;
                    std::string scale;
                    std::string batchid;

                    //================================================================================
                    /// 克隆扩展对象
                    ///
                    /// @return 返回当前对象的深拷贝副本
                    //================================================================================
                    std::unique_ptr<Extension> Clone() const override;

                    //================================================================================
                    /// 比较两个扩展对象是否相等
                    ///
                    /// @param [in] rhs 右侧比较对象
                    /// @return 相等返回true，否则返回false
                    //================================================================================
                    bool IsEqual(const Extension& rhs) const override;
                };

                //================================================================================
                /// 序列化MeshGpuInstancing对象为JSON字符串
                ///
                /// @param [in] meshGpuInstancing     要序列化的MeshGpuInstancing对象
                /// @param [in] gltfDocument          关联的glTF文档对象
                /// @param [in] extensionSerializer   扩展序列化器
                ///
                /// @return 返回序列化后的JSON字符串
                //================================================================================
                std::string SerializeMeshGpuInstancing(const MeshGpuInstancing& meshGpuInstancing, const Document& gltfDocument, const ExtensionSerializer& extensionSerializer);

                //================================================================================
                /// 从JSON字符串反序列化为MeshGpuInstancing对象
                ///
                /// @param [in] json                  JSON格式的字符串数据
                /// @param [in] extensionDeserializer 扩展反序列化器
                ///
                /// @return 返回反序列化后的MeshGpuInstancing扩展对象
                //================================================================================
                std::unique_ptr<Extension> DeserializeMeshGpuInstancing(const std::string& json, const ExtensionDeserializer& extensionDeserializer);
            }
        }
    }
}

#endif