#pragma once

#include <VyEngine/GFX/Resources/Mesh/Vertex.h>
#include <VyEngine/GFX/Resources/Texture/Texture.h>

#include <VyEngine/VK/Buffer/Buffer.h>
#include <VyEngine/VK/Descriptors/Descriptors.h>

// ASSIMP imports
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

enum aiTextureType;

namespace Vy
{
	// struct MatrixUbo
	// {
	// 	alignas(16) glm::mat4 view;
	// 	alignas(16) glm::mat4 proj;
	// };

	// template<typename T>
	// class UniformBuffer final
	// {
	// public:
	// 	UniformBuffer(U32 count = MAX_FRAMES_IN_FLIGHT)
	// 	{
	// 		VkDeviceSize bufferSize = sizeof(T);

	// 		m_UniformBuffers.resize( count );
	// 		m_BufferInfos   .resize( count );

	// 		for (U32 i = 0; i < count; ++i)
	// 		{
	// 			m_UniformBuffers[i] = std::make_unique<VyBuffer>( VyBuffer::uniformBuffer("template", bufferSize ) ):

	// 			m_BufferInfos[i] = m_UniformBuffers[i]->descriptorBufferInfo();
	// 		}
	// 	}

    //     ~UniformBuffer()
	// 	{
	// 	}

	// 	UniformBuffer(const UniformBuffer&) = delete;
	// 	UniformBuffer& operator=(const UniformBuffer&) = delete;
	// 	UniformBuffer(UniformBuffer&&) = delete;
	// 	UniformBuffer& operator=(UniformBuffer&&) = delete;

	// 	// Methods
	// 	//--------------------
	// 	void update(U32 frameIndex, T& data)
	// 	{
	// 		m_UniformBuffers[ frameIndex ]->write( &data, sizeof(T), 0 );
	// 	}

	// 	// Getters & Setters
	// 	VkBuffer getBuffer(U16 idx) const 
    //     { 
    //         return m_UniformBuffers[idx]->handle(); 
    //     }
		
    //     TVector<VkBuffer> getBuffers()const
	// 	{
	// 		TVector<VkBuffer> buffers;
	// 		for (const auto& buffer : m_UniformBuffers)
	// 		{
	// 			buffers.push_back( buffer->handle() );
	// 		}

	// 		return buffers;
	// 	}

	// 	const TVector<VkDescriptorBufferInfo>& descriptorBufferInfos() const
	// 	{
	// 		return m_BufferInfos;
	// 	}

	// private:

	// 	TVector<Unique<VyBuffer>>       m_UniformBuffers;
	// 	TVector<VkDescriptorBufferInfo> m_BufferInfos;

	// };



    struct alignas( 16 ) MaterialUbo 
    {
        Mat4 ModelMatrix;
        Mat4 ModelInverseMatrix;

        // x: color,
        // y: normal,
        // z: emissive,
        // w: occlusion
        UVec4 AlbedoNormalEmissiveOcclusionTextureHandles{ 0 };

        // x: roughness,
        // y: metallic,
        // z: flags
        // w: ignore
        UVec4 RoughnessMetallicFlags{ 0 };

        // x: normal_scale
        // y: occlusion_factor
        // z: roughness_factor
        // w: metallic_factor
        Vec4 NormalOcclusionRoughnessMetallicFactor{ 1.0f };

        Vec3 EmissiveFactor{ 0.0f };
        Vec4 AlbedoFactor  { 1.0f };
    };


    struct SubMesh
    {
        U32 FirstIndex;
        U32 IndexCount;
        
        U32 MaterialIndex; 
    };

    struct MaterialInfo
    {
        TString AlbedoPath       { "NULL" };
        TString NormalPath       { "NULL" };
        TString MetallicRoughPath{ "NULL" };
        TString OcclusionPath    { "NULL" };
        TString EmissivePath     { "NULL" };

        U32    AlbedoIndex       { UINT32_MAX };
        U32    NormalIndex       { UINT32_MAX };
        U32    MetallicRoughIndex{ UINT32_MAX };
        U32    OcclusionIndex    { UINT32_MAX };
        U32    EmissiveIndex     { UINT32_MAX };

        Vec4  AlbedoFactor   { 1.0f };
        Vec3  EmissiveFactor { 0.0f };
        float MetallicFactor { 1.0f };
        float RoughnessFactor{ 1.0f };
        float OcclusionFactor{ 1.0f }; // Strength
        float NormalScale    { 1.0f };
    };


    // for each mesh
    struct VyMaterialSet 
    {
        U32 AlbedoTextureHandle;
        U32 NormalTextureHandle;
        U32 OcclusionTextureHandle;
        U32 RoughnessMetalnessTextureHandle;
        U32 EmissiveTextureHandle;

        Vec4  AlbedoFactor   { 1.0f };
        Vec3  EmissiveFactor { 0.0f };
        float MetallicFactor { 1.0f };
        float RoughnessFactor{ 1.0f };
        float OcclusionFactor{ 1.0f };
        float NormalScale    { 1.0f };

        Mat4 Transform{};
        Mat4 InverseTransform{};

        VyMaterialSet()  = default;
        ~VyMaterialSet() = default;
    };



    // class Mesh final
    // {
    // public:

    //     struct Material
    //     {
    //         TString AlbedoPath;
    //         TString NormalPath;
    //         TString SpecularPath;

    //         const int Amount = 3;
    //     };


    //     struct RawMeshData
    //     {
    //         TVector<VyVertex> Vertices;
    //         TVector<U32>      Indices;
    //         Mesh::Material    Material;
    //         Mat4              Transform;
    //         bool              IsOpaque = true;
    //     };

    //     Mesh(
    //         UniformBuffer<MatrixUbo>* pUbo,
    //         VyDescriptorSetLayout*    pLayout, 
    //         VyDescriptorPool*         pPool,
    //         const RawMeshData&        meshData
    //     );

    //     ~Mesh();

    //     Mesh(const Mesh&)            = delete;
    //     Mesh& operator=(const Mesh&) = delete;
    //     Mesh(Mesh&&)                 = delete;
    //     Mesh& operator=(Mesh&&)      = delete;

    //     // Methods
    //     //--------------------
    //     void draw(VkCommandBuffer cmdBuffer);
        
    //     void bind(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, U16 idx, bool isDepthPass);

    //     // Getters & Setters
    //     VkBuffer vertexBuffer() const { return m_VertexBuffer->handle(); }
    //     VkBuffer indexBuffer()  const { return m_IndexBuffer->handle(); }

    //     TVector<VyVertex> vertices() const { return m_Vertices; }
    //     TVector<U32>      indices()  const { return m_Indices;  }

    //     const Mat4& transform() const { return m_Transform; }


    // private:
    //     void createVertexBuffer();
    //     void createIndexBuffer();

    //     VkDescriptorSet   m_DescriptorSet;

    //     TVector<VyVertex> m_Vertices;
    //     Unique<VyBuffer>  m_VertexBuffer;
    //     U32               m_VertexCount     { 0 };
    //     U32               m_VertexBufferSize{ 0 };

    //     TVector<U32>      m_Indices;
    //     Unique<VyBuffer>  m_IndexBuffer;
    //     U32               m_IndexCount      { 0 };
    //     U32               m_IndexBufferSize { 0 };
        
    //     bool              m_HasIndexBuffer = false;

    //     TVector<Unique<VyTexture>> m_Textures;

    //     const Mat4 m_Transform = Mat4( 1.0f );

    // };


    class VyModel 
    {
    public:
        struct Data 
        {
            TVector<VyVertex> Vertices{};
            TVector<U32>      Indices {};

			TVector<SubMesh>           Submeshes{};
			TVector<MaterialInfo>      Materials{};
			TVector<Unique<VyTexture>> Textures;

            void load(const TString& path);

            bool loadModel(const TPath& path, bool bAllUniqueVertices = false);

            bool loadModelAssimp(const TPath& path);

            // static Data makeSimpleCube(bool bInvert = false);
            // static void computeTangentBasis(VyVertex& v0, VyVertex& v1, VyVertex& v2, Vec3* pTanOut);

        private:
            void processNode(aiNode* pNode, const aiScene* pScene);
            void processMesh(aiMesh* pMesh, const aiScene* pScene);
            // void loadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const TPath& directory);
            // TVector<VyTexture> loadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const TPath& directory);
        };
        
        VyModel(const VyModel&)            = delete;
        VyModel& operator=(const VyModel&) = delete;
        
        VyModel(const Data& data);
        // VyModel(const Builder& builder);
        ~VyModel();
        
        static Shared<VyModel> loadFromFile(const TPath& filepath);

        static Shared<VyModel> createFromFile(const TString& filepath);

        void bind(VkCommandBuffer cmdBuffer) const;
        void draw(VkCommandBuffer cmdBuffer) const;
        void drawIndexed(VkCommandBuffer cmdBuffer, U32 indexCount, U32 firstIndex) const;

        void createDescriptorSets(VyDescriptorSetLayout& setLayout, VyDescriptorPool& pool);
        
        Data& getData() { return m_Data; };

    private:
        void createVertexBuffer(const TVector<VyVertex>& vertices);
        void createIndexBuffer (const TVector<U32>&      indices );
        
        Unique<VyBuffer> m_VertexBuffer;
        U32              m_VertexCount;

        Unique<VyBuffer> m_IndexBuffer;
        U32              m_IndexCount;
        
        bool m_HasIndexBuffer = false;

        Data m_Data; 

        VkDescriptorSet m_MeshDescriptorSet;
        Unique<VyBuffer> m_MaterialBuffer;
        VyMaterialSet m_MaterialSet;
        U32 m_Flags = 0;

        // TVector<Submesh>          m_Submeshes;
        // TVector<TString>          m_TexturePaths;
        // TMap<MaterialID, TString> m_MaterialIdToTexturePath;

        // BoundingBox m_AABB;
    };
}