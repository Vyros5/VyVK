// #pragma once

// // #include <ufbx/ufbx.h>
// #include <VyLib/Include/ufbx/ufbx.h>
// #include <VyEngine/GFX/Resources/Model/Model.h>
// #include <VyEngine/GFX/Resources/Animation/Animations.h>

// namespace Vy
// {
//     // struct VySkeleton;
    
//     class UFBXImporter 
//     {
//     public:
//         TVector<U32>        Indices{};
//         TVector<Vertex>     Vertices{};
//         TVector<VyMesh>     Meshes{};
//         TVector<VyMaterialOLD> Materials{};
//         TMap<TString, I32>  NodeMap;
//         TMap<TString, U32>  BoneMap;

//     public:
//         UFBXImporter() = delete;
//         UFBXImporter(const TString& filePath);
//         ~UFBXImporter() = default;

//         bool loadModel(const U32 instanceCount = 1);

//     private:
//         TString                m_Filepath;
//         TString                m_Path;
//         ufbx_scene*            m_ModelScene = nullptr;
//         THashMap<TString, U32> m_MaterialNameToIndex;

//         U32 m_InstanceCount = 1;
//         U32 m_InstanceIndex = 0;

//         bool m_FBXNoTangents = false;

//     private:
//         void loadMaterials();
//         void loadMaterial(const ufbx_material* pFBXMaterial, ufbx_material_pbr_map materialProperty, int materialIndex);
//         Shared<VyTexture> loadTexture(ufbx_material_map const& materialMap, bool useSRGB);
//         void loadNode(const ufbx_node* pFBXNode);
//         void loadMesh(const ufbx_node* pFBXNode, const U32 meshIndex);
//         void assignMaterial(VyMesh& submesh, int const materialIndex);

//         void calculateTangentsFromIndexBuffer(const TVector<U32>& indices);
//         void calculateTangents();

//         Mat4 ufbxToglm(const ufbx_matrix& ufbxMat);
//         Vec3 ufbxToglm(const ufbx_vec3& ufbxVec3);
//         Quat ufbxToglm(const ufbx_quat& ufbxQuat);

//     public:
//         Shared<VySkeleton>   Skeleton;
//         Shared<VyBuffer>     SkeletonUbo;
//         Unique<VyAnimations> Animations;

//     public:
//         bool loadAnimations();
//         bool addAnimation();

//     private:
//         void loadSkeletons();
//         void loadAnimationClips();
//         // void addAnimationClip();
//     };
// }