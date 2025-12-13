// #pragma once

// #include <Vy/Systems/Rendering/IRenderSystem.h>

// #include <Vy/GFX/Backend/Descriptors.h>
// #include <Vy/GFX/Backend/Device.h>
// #include <Vy/GFX/Backend/Resources/Buffer.h>
// #include <Vy/GFX/Backend/Pipeline.h>

// namespace Vy
// {
//     class VyShadowSystem;
//     class IBLSystem;

//     struct MaterialUniformData
//     {
//         Vec4 Albedo{1.0f};
//         F32  Metallic{0.0f};
//         F32  Roughness{0.5f};
//         F32  AO{1.0f};
//         F32  IsSelected{0.0f};
//         F32  Clearcoat{0.0f};
//         F32  ClearcoatRoughness{0.03f};
//         F32  Anisotropic{0.0f};
//         F32  AnisotropicRotation{0.0f};
//         F32  Transmission{0.0f};
//         F32  IOR{1.5f};
//         F32  Iridescence{0.0f};
//         F32  IridescenceIOR{1.3f};
//         F32  IridescenceThickness{100.0f};
//         U32  TextureFlags{0};
//         F32  UVScale{1.0f};
//         F32  AlphaCutoff{0.5f};
//         U32  AlphaMode{0};
//         U32  AlbedoIndex{0};
//         U32  NormalIndex{0};
//         U32  MetallicIndex{0};
//         U32  RoughnessIndex{0};
//         U32  AOIndex{0};
//         U32  EmissiveIndex{0};
//         U32  _padding{0};
//         Vec4 EmissiveInfo{0.0f, 0.0f, 0.0f, 1.0f}; // rgb: color, a: strength
//     };

//     class MeshRenderSystem
//     {
//     public:
//         MeshRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout bindlessSetLayout);

//         ~MeshRenderSystem();

//         MeshRenderSystem(const MeshRenderSystem&)            = delete;
//         MeshRenderSystem& operator=(const MeshRenderSystem&) = delete;

//         void render(VyFrameInfo& frameInfo);

//         void setShadowSystem(VyShadowSystem* shadowSystem);
//         void setIBLSystem(IBLSystem* iblSystem);

//     private:
//         void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout bindlessSetLayout);
//         void createPipeline(VkRenderPass renderPass);
//         void createShadowDescriptorResources();
//         void createIBLDescriptorResources();
//         void createMaterialDescriptorResources();

//         Unique<VyPipeline> m_Pipeline;
//         Unique<VyPipeline> m_TransparentPipeline;
//         VkPipelineLayout   m_PipelineLayout;

//         VyShadowSystem* m_CurrentShadowSystem{nullptr};
//         IBLSystem*    m_CurrentIBLSystem{nullptr};

//         VkDescriptorSetLayout    m_ShadowDescriptorSetLayout;
//         VkDescriptorPool         m_ShadowDescriptorPool;
//         TVector<VkDescriptorSet> m_ShadowDescriptorSets;

//         VkDescriptorSetLayout    m_IBLDescriptorSetLayout;
//         VkDescriptorPool         m_IBLDescriptorPool;
//         TVector<VkDescriptorSet> m_IBLDescriptorSets;

//         VkDescriptorSetLayout     m_MaterialDescriptorSetLayout;
//         VkDescriptorPool          m_MaterialDescriptorPool;
//         TVector<VkDescriptorSet>  m_MaterialDescriptorSets;
//         TVector<Unique<VyBuffer>> m_MaterialBuffers;
//     };
// }