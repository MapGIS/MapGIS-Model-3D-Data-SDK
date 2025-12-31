#ifndef CI_GLTFSDKTOOL_H
#define CI_GLTFSDKTOOL_H

#include <vector>
#include <string>
#include "cgstring.h"
#include "gbytearray.h"
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/Serialize.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/IStreamReader.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>

using namespace std;
using namespace Microsoft::glTF;

//================================================================================
/// glTF SDK工具类
/// 提供glTF文档创建和操作的各种辅助功能
//================================================================================
class Ci_GltfSdkTool
{
public:
    //================================================================================
    /// 默认构造函数
    //================================================================================
    Ci_GltfSdkTool();

    //================================================================================
    /// 析构函数
    //================================================================================
    ~Ci_GltfSdkTool();

    //================================================================================
    /// 添加图像到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] uri 图像URI
    /// @return 图像ID
    //================================================================================
    static std::string AddImage(Document& document, const string& uri);

    //================================================================================
    /// 添加图像到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] bufferViewId 缓冲视图ID
    /// @param [in] mimeType MIME类型
    /// @return 图像ID
    //================================================================================
    static std::string AddImage(Document& document, const string& bufferViewId, const string& mimeType);

    //================================================================================
    /// 添加图像缓冲视图
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] data 图像数据
    /// @return 缓冲视图ID
    //================================================================================
    static std::string AddImageBufferView(BufferBuilder& bufferBuilder, const vector<unsigned char>& data);

    //================================================================================
    /// 添加采样器到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] magFilter 放大滤波模式
    /// @param [in] minFilter 缩小滤波模式
    /// @param [in] wrapS S轴环绕模式
    /// @param [in] wrapT T轴环绕模式
    /// @return 采样器ID
    //================================================================================
    static std::string AddSampler(Document& document, const Microsoft::glTF::MagFilterMode& magFilter, const Microsoft::glTF::MinFilterMode& minFilter,
        const Microsoft::glTF::WrapMode& wrapS, const Microsoft::glTF::WrapMode& wrapT);

    //================================================================================
    /// 添加采样器到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] sampler 采样器对象
    /// @return 采样器ID
    //================================================================================
    static std::string AddSampler(Microsoft::glTF::Document& document, const Microsoft::glTF::Sampler& sampler);

    //================================================================================
    /// 添加纹理到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] imageId 图像ID
    /// @param [in] samplerId 采样器ID
    /// @param [in] mimeType MIME类型
    /// @return 纹理ID
    //================================================================================
    static std::string AddTexture(Microsoft::glTF::Document& document, string& imageId, string& samplerId, string& mimeType);

    //================================================================================
    /// 添加材质到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] material 材质对象
    /// @return 材质ID
    //================================================================================
    static std::string AddMaterial(Microsoft::glTF::Document& document, Microsoft::glTF::Material& material);

    //================================================================================
    /// 添加索引访问器（unsigned char类型）
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] indices 索引数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddIndexAccessor(BufferBuilder& bufferBuilder, const vector<unsigned char>& indices);

    //================================================================================
    /// 添加索引访问器（unsigned short类型）
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] indices 索引数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddIndexAccessor(BufferBuilder& bufferBuilder, const vector<unsigned short>& indices);

    //================================================================================
    /// 添加索引访问器（unsigned int类型）
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] indices 索引数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddIndexAccessor(BufferBuilder& bufferBuilder, const vector<unsigned int>& indices);

    //================================================================================
    /// 添加位置访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] positions 位置数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddPositionsAccessor(BufferBuilder& bufferBuilder, const vector<float>& positions);

    //================================================================================
    /// 添加法线访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] normals 法线数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddNormalsAccessor(BufferBuilder& bufferBuilder, const vector<float>& normals);

    //================================================================================
    /// 添加纹理坐标访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] texCoords 纹理坐标数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddTexCoordsAccessor(BufferBuilder& bufferBuilder, const vector<float>& texCoords);

    //================================================================================
    /// 添加颜色访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] colors 颜色数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddTexColorsAccessor(BufferBuilder& bufferBuilder, const vector<float>& colors);

    //================================================================================
    /// 添加OID访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] OIDs OID数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddOIDsAccessor(BufferBuilder& bufferBuilder, const vector<unsigned int>& OIDs);

    //================================================================================
    /// 添加批次ID访问器（float类型）
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] batchIDs 批次ID数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddBatchIDsAccessor(BufferBuilder& bufferBuilder, const vector<float>& batchIDs);

    //================================================================================
    /// 添加批次ID访问器（unsigned short类型）
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] batchIDs 批次ID数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddBatchIDsAccessor(BufferBuilder& bufferBuilder, const vector<unsigned short>& batchIDs);

    //================================================================================
    /// 添加实例化平移访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] translation 平移数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddInstancingTranslationAccessor(BufferBuilder& bufferBuilder, const vector<float>& translation);

    //================================================================================
    /// 添加实例化旋转访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] rotation 旋转数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddInstancingRotationAccessor(BufferBuilder& bufferBuilder, const vector<float>& rotation);

    //================================================================================
    /// 添加实例化缩放访问器
    ///
    /// @param [in,out] bufferBuilder 缓冲构建器
    /// @param [in] scale 缩放数据
    /// @return 访问器ID
    //================================================================================
    static std::string AddInstancingScaleAccessor(BufferBuilder& bufferBuilder, const vector<float>& scale);

    //================================================================================
    /// 为网格添加图元
    ///
    /// @param [in,out] mesh 网格对象
    /// @param [in] mode 网格模式
    /// @param [in] materialId 材质ID
    /// @param [in] accessorIdIndices 索引访问器ID
    /// @param [in] accessorIdPositions 位置访问器ID
    /// @param [in] accessorIdNormals 法线访问器ID
    /// @param [in] accessorIdTexCoords 纹理坐标访问器ID列表
    /// @param [in] accessorIdColors 颜色访问器ID
    /// @param [in] accessorIdOIDs OID访问器ID
    /// @param [in] accessorIdBatchIDs 批次ID访问器ID
    //================================================================================
    static void MeshAddPrimitive(Microsoft::glTF::Mesh& mesh, Microsoft::glTF::MeshMode& mode, string& materialId, string& accessorIdIndices, string& accessorIdPositions,
        string& accessorIdNormals, vector<string>& accessorIdTexCoords, string& accessorIdColors, string& accessorIdOIDs, string& accessorIdBatchIDs);

    //================================================================================
    /// 添加网格到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] mesh 网格对象
    /// @return 网格ID
    //================================================================================
    static std::string AddMesh(Microsoft::glTF::Document& document, Microsoft::glTF::Mesh& mesh);

    //================================================================================
    /// 为节点添加GPU实例化扩展
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in,out] node 节点对象
    /// @param [in] translationId 平移访问器ID
    /// @param [in] rotationId 旋转访问器ID
    /// @param [in] scaleId 缩放访问器ID
    /// @param [in] batchidId 批次ID访问器ID
    //================================================================================
    static void NodeAddGPUInstancingEx(Microsoft::glTF::Document& document, Microsoft::glTF::Node& node, string& translationId, string& rotationId, string& scaleId, string& batchidId);

    //================================================================================
    /// 添加节点到glTF文档
    ///
    /// @param [in,out] document glTF文档对象
    /// @param [in] node 节点对象
    /// @return 节点ID
    //================================================================================
    static std::string AddNode(Microsoft::glTF::Document& document, Microsoft::glTF::Node& node);

    //================================================================================
    /// 查找采样器索引
    ///
    /// @param [in] document glTF文档对象
    /// @param [in] samplerId 采样器ID
    /// @return 采样器索引，未找到返回-1
    //================================================================================
    static int FindSamplerIndex(const Document& document, const string& samplerId);

    //================================================================================
    /// 查找图像索引
    ///
    /// @param [in] document glTF文档对象
    /// @param [in] imageId 图像ID
    /// @return 图像索引，未找到返回-1
    //================================================================================
    static int FindImageIndex(const Document& document, const string& imageId);

    //================================================================================
    /// 查找纹理索引
    ///
    /// @param [in] document glTF文档对象
    /// @param [in] textureId 纹理ID
    /// @return 纹理索引，未找到返回-1
    //================================================================================
    static int FindTextureIndex(const Document& document, const string& textureId);

    //================================================================================
    /// 查找材质索引
    ///
    /// @param [in] document glTF文档对象
    /// @param [in] materialId 材质ID
    /// @return 材质索引，未找到返回-1
    //================================================================================
    static int FindMaterialIndex(const Document& document, const string& materialId);

    //================================================================================
    /// 查找节点索引
    ///
    /// @param [in] document glTF文档对象
    /// @param [in] nodeId 节点ID
    /// @return 节点索引，未找到返回-1
    //================================================================================
    static int FindNodeIndex(const Document& document, const string& nodeId);

    //================================================================================
    /// 查找网格索引
    ///
    /// @param [in] document glTF文档对象
    /// @param [in] meshesId 网格ID
    /// @return 网格索引，未找到返回-1
    //================================================================================
    static int FindMesheIndex(const Document& document, const string& meshesId);

private:
};

//================================================================================
/// GLB内存资源写入器类
/// 用于将glTF数据写入内存中的GLB格式
//================================================================================
class GLBResourceWriterMemory : public GLTFResourceWriter
{
private:
    std::shared_ptr<std::iostream> m_stream;

public:
    //================================================================================
    /// 构造函数
    ///
    /// @param [in] tempBufferStream 临时缓冲流
    //================================================================================
    GLBResourceWriterMemory(std::unique_ptr<std::iostream> tempBufferStream);

    //================================================================================
    /// 获取GLB信息（字符串版本）
    ///
    /// @param [in] manifest 清单数据
    /// @param [out] info GLB信息输出
    //================================================================================
    void GetGLBInfo(const std::string& manifest, std::string& info);

    //================================================================================
    /// 获取GLB信息（字节数组版本）
    ///
    /// @param [in] manifest 清单数据
    /// @param [out] info GLB信息输出
    //================================================================================
    void GetGLBInfo(const std::string& manifest, CGByteArray& info);

private:
    //================================================================================
    /// 内部获取GLB信息实现
    ///
    /// @param [in] manifest 清单数据
    /// @param [out] strstream 字符串流输出
    //================================================================================
    void i_GetGLBInfo(const std::string& manifest, std::stringstream& strstream);

protected:
    //================================================================================
    /// 生成缓冲URI
    ///
    /// @param [in] bufferId 缓冲ID
    /// @return 缓冲URI
    //================================================================================
    std::string GenerateBufferUri(const std::string& bufferId) const;

    //================================================================================
    /// 获取缓冲流
    ///
    /// @param [in] bufferId 缓冲ID
    /// @return 缓冲流指针
    //================================================================================
    std::ostream* GetBufferStream(const std::string& bufferId) override;
};

//================================================================================
/// 流读取器类
/// 实现glTF SDK的IStreamReader接口，用于读取文件流
//================================================================================

//================================================================================
/// 流读取器类
/// 实现glTF SDK的IStreamReader接口，用于读取文件流
//================================================================================
class StreamReader : public  IStreamReader
{
public:
    //================================================================================
    /// 构造函数
    ///
    /// @param [in] pathBase 基础路径
    //================================================================================
    StreamReader(std::string pathBase) : m_pathBase(std::move(pathBase))
    {
    }

    //================================================================================
    /// 获取输入流
    ///
    /// @param [in] filename 文件名
    /// @return 输入流共享指针
    //================================================================================
    std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const override
    {
        // In order to construct a valid stream:
        // 1. The filename argument will be encoded as UTF-8 so use filesystem::u8path to
        //    correctly construct a path instance.
        // 2. Generate an absolute path by concatenating m_pathBase with the specified filename
        //    path. The filesystem::operator/ uses the platform's preferred directory separator
        //    if appropriate.
        // 3. Always open the file stream in binary mode. The glTF SDK will handle any text
        //    encoding issues for us.

        CGString tempStr = CGString::UTF8ToGB18030(filename);
        //std::string streamPath = m_pathBase / std::experimental::filesystem::u8path(filename);
        std::string streamPath = m_pathBase;
        
        if(streamPath[streamPath.size()-1] == '\\' || streamPath[streamPath.size()-1] == '/')
            streamPath += tempStr.CStr();
        else 
            streamPath += "/" + tempStr.StdString();

#ifdef WIN32

        std::replace(streamPath.begin(), streamPath.end(), '/', '\\');
        auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::binary);

#else
        std::replace(streamPath.begin(), streamPath.end(), '\\', '/');
        streamPath = CGString::GB18030ToUTF8(streamPath);
        auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::binary);
#endif

        // Check if the stream has no errors and is ready for I/O operations
        if (!stream || !(*stream))
        {
            throw std::runtime_error("Unable to create a valid input stream for uri: " + filename);
        }

        return stream;
    }

private:
    std::string m_pathBase;
};

//================================================================================
/// 高斯点阵缓冲构建器类
/// 专门用于构建高斯点阵数据的缓冲区
//================================================================================
class GaussianSplattingBufferBuilder final
{
public:
    //================================================================================
    /// 构造函数
    ///
    /// @param [in] resourceWriter 资源写入器
    //================================================================================
    GaussianSplattingBufferBuilder(std::unique_ptr<ResourceWriter>&& resourceWriter);

    //================================================================================
    /// 添加缓冲区
    ///
    /// @param [in] bufferId 缓冲区ID（可选）
    /// @return 缓冲区引用
    //================================================================================
    const Buffer& AddBuffer(const char* bufferId = nullptr);

    //================================================================================
    /// 添加缓冲视图
    ///
    /// @param [in] data 数据指针
    /// @param [in] byteLength 数据字节长度
    /// @param [in] byteStride 字节步长（可选）
    /// @param [in] target 缓冲视图目标（可选）
    /// @return 缓冲视图引用
    //================================================================================
    const BufferView& AddBufferView(const void* data, size_t byteLength, Optional<size_t> byteStride = {}, Optional<BufferViewTarget> target = {});

    //================================================================================
    /// 添加访问器
    ///
    /// @param [in] count 元素数量
    /// @param [in] accessorDesc 访问器描述
    /// @return 访问器引用
    //================================================================================
    const Accessor& AddAccessor(size_t count, AccessorDesc accessorDesc);

    // This method moved from the .cpp to the header because
    // When this library is built with VS2017 and used in an executable built with VS2019
    // an unordered_map issue ( see https://docs.microsoft.com/en-us/cpp/overview/cpp-conformance-improvements?view=msvc-160 )
    // makes the destruction to perform an underflow.
    // To fix the issue, the code here is inlined so it gets compiled and linked with VS2019.
    // So only 1 version of std::unordered_map binary code is generated.
    //================================================================================
    /// 输出到glTF文档
    ///
    /// @param [in,out] gltfDocument glTF文档对象
    //================================================================================
    void Output(Document& gltfDocument)
    {
        for (auto& buffer : m_buffers.Elements())
        {
            gltfDocument.buffers.Append(std::move(buffer), AppendIdPolicy::ThrowOnEmpty);
        }

        m_buffers.Clear();

        for (auto& bufferView : m_bufferViews.Elements())
        {
            gltfDocument.bufferViews.Append(std::move(bufferView), AppendIdPolicy::ThrowOnEmpty);
        }

        m_bufferViews.Clear();

        for (auto& accessor : m_accessors.Elements())
        {
            gltfDocument.accessors.Append(std::move(accessor), AppendIdPolicy::ThrowOnEmpty);
        }
        m_accessors.Clear();
    }

    //================================================================================
    /// 获取当前缓冲区
    ///
    /// @return 当前缓冲区引用
    //================================================================================
    const Buffer& GetCurrentBuffer() const;

    //================================================================================
    /// 获取当前缓冲视图
    ///
    /// @return 当前缓冲视图引用
    //================================================================================
    const BufferView& GetCurrentBufferView() const;

    //================================================================================
    /// 获取当前访问器
    ///
    /// @return 当前访问器引用
    //================================================================================
    const Accessor& GetCurrentAccessor() const;

    //================================================================================
    /// 获取缓冲区数量
    ///
    /// @return 缓冲区数量
    //================================================================================
    size_t GetBufferCount() const;

    //================================================================================
    /// 获取缓冲视图数量
    ///
    /// @return 缓冲视图数量
    //================================================================================
    size_t GetBufferViewCount() const;

    //================================================================================
    /// 获取访问器数量
    ///
    /// @return 访问器数量
    //================================================================================
    size_t GetAccessorCount() const;

    //================================================================================
    /// 获取资源写入器
    ///
    /// @return 资源写入器引用
    //================================================================================
    ResourceWriter& GetResourceWriter();

    //================================================================================
    /// 获取资源写入器（常量版本）
    ///
    /// @return 资源写入器常量引用
    //================================================================================
    const ResourceWriter& GetResourceWriter() const;

private:
    std::unique_ptr<ResourceWriter> m_resourceWriter;

    IndexedContainer<Buffer>     m_buffers;
    IndexedContainer<BufferView> m_bufferViews;
    IndexedContainer<Accessor>   m_accessors;
};

#endif