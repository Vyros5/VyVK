// #pragma once

// #include <VyEngine/VK/Pipeline/ShaderReflection.h>

// namespace Vy
// {
//     enum class ShaderType
//     {
//         Vertex,
//         Fragment,
//         Compute,
//         Geometry,
//         TessellationControl,
//         TessellationEvaluation
//     };

//     struct ShaderDescriptorInfo
//     {
//         struct DescriptorInfo
//         {
//             VkDescriptorType   Type;
//             U32                Count;
//             VkShaderStageFlags StageFlags;
//             U32                Binding;
//         };

//         TMap<U32, TVector<DescriptorInfo>> SetDescriptors;
//         TMap<VkDescriptorType, U32>        TotalDescriptorCounts;
//         TSet<U32>                          UniqueSets;

//         void addShaderReflection(const VyShaderReflection& reflection, VkShaderStageFlags stage)
//         {
//             for (const auto& [set, resources] : reflection.getDescriptorSets())
//             {
//                 UniqueSets.insert(set);

//                 for (const auto& resource : resources)
//                 {
//                     auto& descriptors = SetDescriptors[set];

//                     auto it = std::find_if(
//                         descriptors.begin(), descriptors.end(),
//                         [&](const DescriptorInfo& info)
//                         {
//                             return info.Type == resource.DescriptorType && info.Binding == resource.Binding;
//                         }
//                     );
                    
//                     if (it != descriptors.end())
//                     {
//                         it->StageFlags |= stage;
//                         it->Count = std::max(it->Count, resource.ArraySize);
//                     }
//                     else
//                     {
//                         descriptors.push_back({
//                             .Type       = resource.DescriptorType,
//                             .Count      = resource.ArraySize,
//                             .StageFlags = stage,
//                             .Binding    = resource.Binding
//                         });
//                     }
//                     TotalDescriptorCounts[resource.DescriptorType] += resource.ArraySize;
//                 }
//             }
//         }

//         U32 getTotalUniqueSetCount() const
//         {
//             return UniqueSets.size();
//         }
//     };



//     class VyShader
//     {
//     public:
//         VyShader(TString filePath, ShaderType type);
//         ~VyShader();

//         VyShaderReflection&   getReflection()   const { return *m_Reflection; }
//         VkShaderModule        getShaderModule() const { return m_ShaderModule; }
//         VkShaderStageFlagBits getShaderStage()  const;

//     private:
//         void load();
//         TVector<U32> compile();
//         void createShaderModule(const TVector<U32>& code);

//     private:
//         TString        m_FilePath;
//         ShaderType     m_Type;
//         VkShaderModule m_ShaderModule = VK_NULL_HANDLE;
//         TString        m_ShaderSource;

//         Shared<VyShaderReflection> m_Reflection;
//     };
// }