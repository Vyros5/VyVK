// #pragma once

// #include <VyEngine/VK/Core/VKCore.h>

// #include <spirv_cross/spirv_cross.hpp>

// namespace Vy
// {
//     class VyShaderReflection
//     {
//     public:
//         struct ShaderResource
//         {
//             TString            Name;
//             U32                Binding;
//             U32                Set;
//             VkDescriptorType   DescriptorType;
//             VkShaderStageFlags StageFlags;
//             U32                ArraySize;
//             bool               IsReadOnly;
//             bool               IsWriteOnly;

//             TString toString() const;
//         };

//         struct VertexInputBinding
//         {
//             U32               Binding;
//             U32               Stride;
//             VkVertexInputRate InputRate;
//         };

//         struct VertexInputAttribute
//         {
//             U32      Location;
//             U32      Binding;
//             VkFormat Format;
//             U32      Offset;
//             TString  Name;
//         };

//         struct ShaderOutput
//         {
//             TString  Name;
//             U32      Location;
//             VkFormat Format;
//         };

//         struct PushConstantRange
//         {
//             TString            Name;
//             U32                Offset;
//             U32                Size;
//             VkShaderStageFlags StageFlags;
//         };

//         struct SpecializationConstant
//         {
//             U32     ID;
//             TString Name;
//             U32     Size;
//         };

//         VyShaderReflection(const TVector<U32>& spirvCode, VkShaderStageFlagBits stage);

//         const TString&                            getEntryPoint() const { return m_EntryPoint; }
//         const TVector<ShaderResource>&            getResources() const { return m_Resources; }
//         const TMap<U32, TVector<ShaderResource>>& getDescriptorSets() const { return m_DescriptorSets; }
//         U32                                       getDescriptorSetCount() const { return m_DescriptorSets.size(); }
//         U32                                       getTotalDescriptorCountAcrossAllSets() const;
//         const TVector<VertexInputBinding>&        getVertexInputBindings() const { return m_VertexInputBindings; }
//         const TVector<VertexInputAttribute>&      getVertexInputAttributes() const { return m_VertexInputAttributes; }
//         const TVector<ShaderOutput>&              getOutputs() const { return m_Outputs; }
//         const TVector<PushConstantRange>&         getPushConstantRanges() const { return m_PushConstantRanges; }
//         const TVector<SpecializationConstant>&    getSpecializationConstants() const { return m_SpecializationConstants; }

//     private:
//         void ReflectEntryPoint(const spirv_cross::Compiler& compiler);
//         void ReflectDescriptors(const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources);
//         void ReflectVertexInputs(const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources);
//         void ReflectOutputs(const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources);
//         void ReflectPushConstants(const spirv_cross::Compiler& compiler, const spirv_cross::ShaderResources& resources);
//         void ReflectSpecializationConstants(const spirv_cross::Compiler& compiler);

//         static VkFormat SPIRTypeToVkFormat(const spirv_cross::SPIRType& type);
//         static U32      getFormatSize(VkFormat format);
//         static TString  getShaderStageName(VkShaderStageFlagBits stage);
//         static TString  getDescriptorTypeName(VkDescriptorType type);
//         static TString  getVkFormatName(VkFormat format);

//         VkShaderStageFlagBits m_ShaderStage;
//         TString m_EntryPoint;
//         TVector<ShaderResource> m_Resources;
//         TVector<VertexInputBinding> m_VertexInputBindings;
//         TVector<VertexInputAttribute> m_VertexInputAttributes;
//         TVector<ShaderOutput> m_Outputs;
//         TVector<PushConstantRange> m_PushConstantRanges;
//         TVector<SpecializationConstant> m_SpecializationConstants;
//         TMap<U32, TVector<ShaderResource>> m_DescriptorSets;
//     };
// }