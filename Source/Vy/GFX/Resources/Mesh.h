#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>
#include <Vy/GFX/Resources/PBRMaterial.h>
#include <Vy/Globals.h>

namespace Vy
{
    struct MeshBuffers
    {
        U64 VertexBufferAddress;
        U64 IndexBufferAddress;
    };

    class Model
    {
    public:
        struct Vertex
        {
            Vec3 Position;
            Vec3 Color;
            Vec3 Normal;
            Vec2 UV;
            int  MaterialId{-1}; // Material index for this vertex

            static TVector<VkVertexInputBindingDescription>   getBindingDescriptions();
            static TVector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            
            bool operator==(const Vertex& other) const 
            { 
                return 
                    Position == other.Position && 
                    Color == other.Color && 
                    Normal == other.Normal && 
                    UV == other.UV; 
                }
        };

        struct MaterialInfo
        {
            String        Name;
            VyPBRMaterial PBRMaterial;
            int           MaterialId; // Index in the materials array

            // Texture paths from MTL file (relative to OBJ file location)
            String DiffuseTexPath;
            String NormalTexPath;
            String RoughnessTexPath;
            String AOTexPath;
            String EmissiveTexPath;
        };

        // Sub-mesh: a portion of the model using one material
        struct SubMesh
        {
            U32 IndexOffset; // Offset into the index buffer
            U32 IndexCount;  // Number of indices for this sub-mesh
            int MaterialId;  // Index into materials array
            U32 MeshletOffset = 0;
            U32 MeshletCount  = 0;
        };

        // Animation structures
        struct AnimationSampler
        {
            enum EInterpolation
            {
                LINEAR,
                STEP,
                CUBICSPLINE
            };

            TVector<float>          Times;        // Keyframe timestamps
            TVector<Vec3>           Translations; // For translation channels
            TVector<Quat>           Rotations;    // For rotation channels
            TVector<Vec3>           Scales;       // For scale channels
            TVector<TVector<float>> MorphWeights; // For morph target weight channels
            EInterpolation          Interpolation = LINEAR;
        };

        struct AnimationChannel
        {
            enum ETargetPath
            {
                TRANSLATION,
                ROTATION,
                SCALE,
                WEIGHTS // Morph target weights
            };

            int         TargetNode; // Index into the node array
            ETargetPath TargetPath;
            int         SamplerIndex;
        };

        struct Animation
        {
            String                    Name;
            float                     Duration = 0.0f; // In seconds
            TVector<AnimationChannel> Channels;
            TVector<AnimationSampler> Samplers;
        };

        struct Node
        {
            String         Name;
            Vec3           Translation = Vec3(0.0f);
            Quat           Rotation    = Quat(1.0f, 0.0f, 0.0f, 0.0f);
            Vec3           Scale       = Vec3(1.0f);
            Mat4           Matrix      = Mat4(1.0f);
            TVector<int>   Children;
            int            Mesh = -1;
            TVector<float> MorphWeights; // Current morph target weights

            Mat4 getLocalTransform() const
            {
                return glm::translate(Mat4(1.0f), Translation) * glm::mat4_cast(Rotation) * glm::scale(Mat4(1.0f), Scale) * Matrix;
            }
        };

        // Morph target (blend shape) data
        struct MorphTarget
        {
            TVector<Vec3> PositionDeltas; // Position offsets from base mesh
            TVector<Vec3> NormalDeltas;   // Normal offsets from base mesh
            String        Name;
        };

        struct MorphTargetSet
        {
            TVector<MorphTarget> Targets;         // All morph targets for this mesh
            TVector<float>       Weights;         // Current blend weights [0-1]
            U32                  VertexOffset;    // Offset into vertex buffer
            U32                  VertexCount;     // Number of vertices affected
            TVector<U32>         PositionIndices; // Mapping from vertex to original glTF position index
        };

        struct Meshlet
        {
            U32 VertexOffset;
            U32 TriangleOffset;
            U32 VertexCount;
            U32 TriangleCount;

            float Center[3];
            float Radius;

            float ConeAxis[3];
            float ConeCutoff;
        };

        struct Builder
        {
            TVector<Vertex>         Vertices{};
            TVector<U32>            Indices{};
            TVector<MaterialInfo>   Materials{};       // Materials loaded from MTL file
            TVector<SubMesh>        SubMeshes{};       // Sub-meshes by material
            TVector<Animation>      Animations{};      // Animations from glTF
            TVector<Node>           Nodes{};           // Scene graph nodes
            TVector<MorphTargetSet> MorphTargetSets{}; // Morph targets per mesh
            String                  FilePath{};

            void loadModelFromFile(const String& filepath, bool flipX = false, bool flipY = false, bool flipZ = false);
            void loadModelFromGLTF(const String& filepath, bool flipX = false, bool flipY = false, bool flipZ = false);
        };

        explicit Model(const Builder& builder);
        ~Model();

        // delete copy and move constructors and assignment operators
        Model(const Model&)            = delete;
        Model& operator=(const Model&) = delete;

        static Unique<Model> createModelFromFile(const String& filepath, bool flipX = false, bool flipY = false, bool flipZ = false);
        static Unique<Model> createModelFromGLTF(const String& filepath, bool flipX = false, bool flipY = false, bool flipZ = false);

        void bind(VkCommandBuffer commandBuffer) const;
        void draw(VkCommandBuffer commandBuffer) const;

        // Draw a specific sub-mesh
        void drawSubMesh(VkCommandBuffer commandBuffer, size_t subMeshIndex) const;

        // Get materials loaded from MTL file
        const TVector<MaterialInfo>& getMaterials() const { return m_Materials; }
        TVector<MaterialInfo>&       getMaterials() { return m_Materials; }

        // Get sub-meshes
        const TVector<SubMesh>& getSubMeshes() const { return m_SubMeshes; }

        // Check if model has multiple materials
        bool hasMultipleMaterials() const { return m_SubMeshes.size() > 1; }

        // Animation support
        bool                      hasAnimations() const { return !m_Animations.empty(); }
        const TVector<Animation>& getAnimations() const { return m_Animations; }
        const TVector<Node>&      getNodes() const { return m_Nodes; }
        TVector<Node>&            getNodes() { return m_Nodes; }

        // Morph target support
        bool                               hasMorphTargets() const { return !m_MorphTargetSets.empty(); }
        const TVector<MorphTargetSet>& getMorphTargetSets() const { return m_MorphTargetSets; }
        TVector<MorphTargetSet>&       getMorphTargetSets() { return m_MorphTargetSets; }

        // Buffer access for compute operations
        VkBuffer getVertexBuffer() const { return m_VertexBuffer->handle(); }
        VkBuffer getIndexBuffer() const { return m_IndexBuffer ? m_IndexBuffer->handle() : VK_NULL_HANDLE; }

        U64 getVertexBufferAddress() const { return m_VertexBuffer->deviceAddress(); }
        U64 getIndexBufferAddress() const { return m_IndexBuffer ? m_IndexBuffer->deviceAddress() : 0; }

        void bindAlternateVertexBuffer(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer) const;

        /**
         * @brief Get approximate memory size of this model
         * @return Memory size in bytes (vertex + index buffers)
         */
        size_t getMemorySize() const;

        const String& getFilePath() const { return m_FilePath; }

        void setMeshId(U32 id) { m_MeshId = id; }
        U32 getMeshId() const { return m_MeshId; }

        // Meshlet support
        const TVector<Meshlet>& getMeshlets() const { return m_Meshlets; }
        U64 getMeshletBufferAddress()    const { return m_MeshletBuffer          ? m_MeshletBuffer->deviceAddress()          : 0; }
        U64 getMeshletVerticesAddress()  const { return m_MeshletVerticesBuffer  ? m_MeshletVerticesBuffer->deviceAddress()  : 0; }
        U64 getMeshletTrianglesAddress() const { return m_MeshletTrianglesBuffer ? m_MeshletTrianglesBuffer->deviceAddress() : 0; }
        U32 getMeshletCount() const { return static_cast<U32>(m_Meshlets.size()); }

    private:
        String m_FilePath;
        U32    m_MeshId = 0;

        Unique<VyBuffer> m_VertexBuffer;
        U32              m_VertexCount = 0;

        Unique<VyBuffer> m_IndexBuffer;
        U32              m_IndexCount = 0;

        // Meshlet buffers
        TVector<Meshlet> m_Meshlets;
        Unique<VyBuffer> m_MeshletBuffer;
        Unique<VyBuffer> m_MeshletVerticesBuffer;
        Unique<VyBuffer> m_MeshletTrianglesBuffer;

        TVector<MaterialInfo>   m_Materials;       // Materials from MTL file
        TVector<SubMesh>        m_SubMeshes;       // Sub-meshes by material
        TVector<Animation>      m_Animations;      // Animations from glTF
        TVector<Node>           m_Nodes;           // Scene graph nodes
        TVector<MorphTargetSet> m_MorphTargetSets; // Morph targets

        void createVertexBuffers(const TVector<Vertex>& vertices);
        void createIndexBuffers(const TVector<U32>& indices);
        void generateMeshlets(const TVector<Vertex>& vertices, const TVector<U32>& indices);
    };
}





















// #include <Vy/GFX/Backend/Pipeline.h>
// #include <Vy/GFX/Resources/Texture.h>
// #include <Vy/Asset/Asset.h>

// #include <VyLib/Util/Hash.h>

// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/hash.hpp>

// // Forward declarations for Assimp
// struct aiNode;
// struct aiScene;
// struct aiMesh;
// struct aiMaterial;

// namespace Vy
// {
// 	struct MaterialData
// 	{
// 		VyAssetHandle AlbedoMap = VyAssetHandle::Invalid();
// 		VyAssetHandle NormalMap = VyAssetHandle::Invalid();

// 		VyAssetHandle MetallicRoughnessMap = VyAssetHandle::Invalid();
// 		VyAssetHandle AOMap = VyAssetHandle::Invalid();
// 		VyAssetHandle EmissiveMap = VyAssetHandle::Invalid();

// 		Vec4 AlbedoColor = Vec4(1.0f);
// 		float Metallic = 0.0f;
// 		float Roughness = 0.0f;
// 		Vec3 EmissiveFactor = Vec3(0.0f);


// 		Json::Value serialize() const;
// 		static Optional<MaterialData> deserialize(const Json::Value& root);
// 	};


// 	class Material : public VyAsset
// 	{
// 	public:
// 		class Builder {
// 		public:
// 			explicit Builder() = default;

// 			Builder& setAssetHandle(const VyAssetHandle& handle) { m_Handle = handle; return *this; }
// 			Builder& setAlbedoMap(const VyAssetHandle& handle) { m_MaterialData.AlbedoMap = handle; return *this; }
// 			Builder& setNormalMap(const VyAssetHandle& handle) { m_MaterialData.NormalMap = handle; return *this; }
// 			Builder& setMetallicRoughnessMap(const VyAssetHandle& handle) { m_MaterialData.MetallicRoughnessMap = handle; return *this; }
// 			Builder& setAOMap(const VyAssetHandle& handle) { m_MaterialData.AOMap = handle; return *this; }
// 			Builder& setEmissiveMap(const VyAssetHandle& handle) { m_MaterialData.EmissiveMap = handle; return *this; }

// 			Builder& setAlbedoColor(const Vec4& color) { m_MaterialData.AlbedoColor = color; return *this; }
// 			Builder& setMetallicFactor(float metallic) { m_MaterialData.Metallic = metallic; return *this; }
// 			Builder& setRoughnessFactor(float roughness) { m_MaterialData.Roughness = roughness; return *this; }
// 			Builder& setEmissiveFactor(Vec3 emissive) { m_MaterialData.EmissiveFactor = emissive; return *this; }

// 			Material build();

// 		private:
// 			MaterialData m_MaterialData;
// 			VyAssetHandle m_Handle = VyAssetHandle::Invalid();
// 		};

// 	public:
// 		Material(MaterialData materialData, VyAssetHandle assetHandle);
		
//         void bind(const VyPipeline& pipeline, VkCommandBuffer cmdBuffer) const;
		
//         void reload() override;
		
//         bool checkForDirtyInDependencies() override;

// 	private:
// 		void createDescriptorSet();
// 		MaterialData m_MaterialData;
// 		Unique<VyDescriptorSetLayout> m_DescriptorSetLayout;
//         VkDescriptorSet m_DescriptorSet;
// 		Unique<VyBuffer> m_MaterialBuffer;
// 	};





// 	struct Vertex
// 	{
// 		Vec3 Position;
// 		Vec3 Normal;
// 		Vec2 TextureCoord;
// 		Vec3 Tangent;

// 		static TVector<VkVertexInputBindingDescription>   getBindingDescriptions();
// 		static TVector<VkVertexInputAttributeDescription> getAttributeDescriptions();
// 	};




// 	class Mesh
// 	{
// 	public:
// 		Mesh() = default;

// 		TVector<Vertex>& getVertices() { return  m_Vertices; }
// 		TVector<U32>&    getIndices()  { return  m_Indices; }
// 		U32              getMaterialIndex() const { return m_MaterialIndex; }

// 	private:
// 		friend class Model;
// 		U32 m_MaterialIndex = 0;
// 		TVector<Vertex> m_Vertices;
// 		TVector<U32>    m_Indices;
// 	};



// 	class Model
// 	{
// 	public:
// 		Model() = default;

// 		      TVector<Mesh>&          getMeshes()                { return m_Meshes; }
// 		const TVector<Mesh>&          getMeshes()          const { return m_Meshes; }
// 		      TVector<VyAssetHandle>& getMaterialHandles()       { return m_MaterialHandles; }
// 		const TVector<VyAssetHandle>& getMaterialHandles() const { return m_MaterialHandles; }

// 		static Model fromFile(const Path& absolutePath);
// 	private:
// 		TVector<VyAssetHandle> m_MaterialHandles {};
// 		TVector<Mesh>          m_Meshes{};
// 	};



// 	class SubMeshRenderer
// 	{
// 	public:
// 		SubMeshRenderer(Mesh& mesh);

// 		~SubMeshRenderer();
		
//         SubMeshRenderer(SubMeshRenderer&& other) noexcept;

// 		SubMeshRenderer(const SubMeshRenderer&) = delete;
// 		SubMeshRenderer &operator=(const SubMeshRenderer&) = delete;

// 		void submitSubMesh(Mesh& subMesh);
// 		void bind(VkCommandBuffer cmdBuffer) const;
// 		void draw(VkCommandBuffer cmdBuffer) const;
// 	private:
// 		U32 m_VertexCount;
// 		U32 m_IndexCount;
// 		VyBuffer m_VertexBuffer;
// 		VyBuffer m_IndexBuffer;
// 	};


// 	class MeshRenderer : public VyAsset
// 	{
// 	public:
// 		explicit MeshRenderer(VyAssetHandle handle, Model model);

// 		const Model& getModel() const { return m_Model; }
		
//         void draw(const VyPipeline& pipeline, VkCommandBuffer cmdBuffer) const;

// 		void reload() override;
	
//         bool checkForDirtyInDependencies() override { return false; }
	
//     private:

// 		void init();

// 		Model m_Model {};
// 		TVector<Shared<Material>> m_Materials {};
// 		TVector<SubMeshRenderer> m_SubMeshRenderers {};
// 	};
// }