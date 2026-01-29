// #pragma once

// #include <VyEngine/VK/Buffer/Buffer.h>
// #include <VyEngine/GFX/Resources/Texture/Texture.h>
// #include <VyEngine/GFX/Data/FrameInfo.h>

// #include <Defines/GPUSharedDefines.h>
// #include <VyEngine/GFX/Resources/Animation/Skeleton.h>
// #include <VyEngine/GFX/Resources/Mesh/Vertex.h>

// namespace Vy
// {
//     class UFBXImporter;
//     class MaterialDescriptor;

//     class VyMaterialOLD 
//     {
//     public:
//         enum TextureIndices 
//         {
//             DIFFUSE_MAP_INDEX = 0,
//             NORMAL_MAP_INDEX,
//             ROUGHNESS_MAP_INDEX,
//             METALLIC_MAP_INDEX,
//             ROUGHNESS_METALLIC_MAP_INDEX,
//             EMISSIVE_MAP_INDEX,
//             NUM_TEXTURES
//         };

//         enum MaterialFeatures  // bitset
//         {
//             HAS_DIFFUSE_MAP            = GLSL_HAS_DIFFUSE_MAP,
//             HAS_NORMAL_MAP             = GLSL_HAS_NORMAL_MAP,
//             HAS_ROUGHNESS_MAP          = GLSL_HAS_ROUGHNESS_MAP,
//             HAS_METALLIC_MAP           = GLSL_HAS_METALLIC_MAP,
//             HAS_ROUGHNESS_METALLIC_MAP = GLSL_HAS_ROUGHNESS_METALLIC_MAP,
//             HAS_EMISSIVE_COLOR         = GLSL_HAS_EMISSIVE_COLOR,
//             HAS_EMISSIVE_MAP           = GLSL_HAS_EMISSIVE_MAP
//         };

//         struct PBRMaterial 
//         {
//             // byte 0 to 15
//             U32   Features{0};
//             float Roughness{0.0f};
//             float Metallic{0.0f};
//             float _pad0{0.0f};  // padding

//             // byte 16 to 31
//             Vec4 DiffuseColor{1.0f, 1.0f, 1.0f, 1.0f};

//             // byte 32 to 47
//             Vec3  EmissiveColor{0.0f, 0.0f, 0.0f};
//             float EmissiveStrength{1.0f};

//             // byte 48 to 63
//             float NormalMapIntensity{1.0f};
//             float _pad1{0.0f};  // padding
//             float _pad2{0.0f};  // padding
//             float _pad3{0.0f};  // padding

//             // byte 64 to 128
//             Vec4 _pad4[4];
//         };

//     public:
//         using MaterialTexturesArray = TArray<Shared<VyTexture>, VyMaterialOLD::NUM_TEXTURES>;

//         Shared<VyBuffer>           MaterialBuffer;
//         VyMaterialOLD::PBRMaterial    PbrMaterial;
//         Shared<MaterialDescriptor> MaterialDescriptor;
//         MaterialTexturesArray      MaterialTextures;
//     };



//     class MaterialDescriptor 
//     {
//     public:
//         MaterialDescriptor(VyMaterialOLD& material, VyMaterialOLD::MaterialTexturesArray& textures);
//         MaterialDescriptor(MaterialDescriptor const& other);
//         virtual ~MaterialDescriptor() = default;

//     public:
//         const VkDescriptorSet& descriptorSet() const { return m_DescriptorSet; }

//     private:
//         VkDescriptorSet m_DescriptorSet;
//     };


//     struct Vertex 
//     {
//         Vec3  Position{ 0.0f };
//         Vec4  Color   { 1.0f };
//         Vec3  Normal  {      };
//         Vec2  UV      {      };
//         Vec3  Tangent {      };
//         IVec4 JointIds{      };
//         Vec4  Weights {      };

//         static TVector<VkVertexInputBindingDescription>   bindingDescriptions  ();
//         static TVector<VkVertexInputAttributeDescription> attributeDescriptions();
        
//         bool operator==(const Vertex& other) const 
//         {
//             return Position == other.Position 
//                 && Color    == other.Color 
//                 && Normal   == other.Normal
//                 && UV       == other.UV
//                 && Tangent  == other.Tangent 
//             ;
//         }
//     };


//     struct VyMesh 
//     {
//         U32 FirstIndex;
//         U32 FirstVertex;
//         U32 IndexCount;
//         U32 VertexCount;
//         U32 InstanceCount;

//         VyMaterialOLD Material;
        
//         Shared<VyBuffer> SkeletonBuffer;
//         VkDescriptorSet  SkeletonDescriptorSet;
//     };


//     class MeshModel 
//     {
//     public:
//         struct Bounds 
//         {
//             Vec3 Lower;
//             Vec3 Upper;
//         };

//     public:
//         MeshModel(const UFBXImporter& importer);
//         ~MeshModel();

//         MeshModel(const MeshModel&)            = delete;
//         MeshModel& operator=(const MeshModel&) = delete;

//         static Unique<MeshModel> createMeshFromFile(TStringView filepath);

//         void updateAnimation(U32 frameCounter);

//         void bind(VkCommandBuffer cmdBuffer);
//         void draw(const VyFrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout);
//         void drawMesh(const VkCommandBuffer& cmdBuffer, const VyMesh& mesh) const;

//         Bounds bounds() const;
//         float  width() const;
//         const TVector<Vertex> vertices() { return m_Vertices; }
//         const TVector<U32>    indices()  { return m_Indices;  }

//         bool                  hasSkeleton() const { return m_Skeleton ? true : false; }
//         Shared<VySkeleton>    skeleton()          { return m_Skeleton; }

//     private:
//         TVector<VyMesh>    m_Meshes{};
//         TMap<TString, I32> m_NodeMap;
//         TVector<Vertex>    m_Vertices;
//         TVector<U32>       m_Indices;

//         Unique<VyBuffer> m_VertexBuffer;
//         U32              m_VertexCount;

//         bool m_HasIndexBuffer = false;
//         Unique<VyBuffer> m_IndexBuffer;
//         U32              m_IndexCount;

//     private:
//         void copyMeshes(const TVector<VyMesh>& meshes);

//         void createVertexBuffer(const TVector<Vertex>& vertices);
//         void createIndexBuffer(const TVector<U32>& indices);

//         void bindDescriptors(const VyFrameInfo& frameInfo, const VkPipelineLayout& pipelineLayout, VyMesh& mesh);

//     private:
//         Shared<VySkeleton> m_Skeleton;
//         Shared<VyBuffer>   m_SkeletonUbo;
//     };




//     // class VyModel 
//     // {
//     // public:
//     //     struct Data 
//     //     {
//     //         TVector<VyVertex> Vertices{};
//     //         TVector<U32>      Indices {};
            
//     //         bool loadModel(const TPath& path, bool bAllUniqueVertices);

//     //         static Data makeSimpleCube(bool bInvert = false);
//     //         static void computeTangentBasis(VyVertex& v0, VyVertex& v1, VyVertex& v2, Vec3* pTanOut);
//     //     };
        
//     //     VyModel(const VyModel&)            = delete;
//     //     VyModel& operator=(const VyModel&) = delete;
        
//     //     VyModel(const Data& data);
//     //     ~VyModel();
        
//     //     static Unique<VyModel> loadFromFile(const TPath& path, bool bAllUniqueVertices = false);
        
//     //     void bind(VkCommandBuffer cmdBuffer) const;
//     //     void draw(VkCommandBuffer cmdBuffer) const;
        
//     // private:
//     //     void createVertexBuffer(const TVector<VyVertex>& vertices);
//     //     void createIndexBuffer(const TVector<U32>& indices);
        
//     //     Unique<VyBuffer> m_VertexBuffer;
//     //     U32              m_VertexCount;

//     //     Unique<VyBuffer> m_IndexBuffer;
//     //     U32              m_IndexCount;
        
//     //     bool m_HasIndexBuffer = false;
//     // };
// }