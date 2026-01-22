#pragma once

#include <VyEngine/VK/Device/Device.h>

namespace Vy
{
    enum class VyPipelineFlags
    {
        None       = 0,
        MeshShader = 1 << 0,
    };


    /** 
     * @brief Configuration structure for pipeline layout creation.
     * 
     * This structure holds the descriptor set layouts and push constant ranges
     * used to create a Vulkan pipeline layout.
     */
    struct VyPipelineLayoutConfig
    {
        TVector<VkDescriptorSetLayout> DescriptorSetLayouts;
        TVector<VkPushConstantRange>   PushConstantRanges;
    };

    // ----------------------------------------------------------------------------------------

    /** 
     * @brief Configuration structure for graphics pipeline creation.
     * 
     * This structure holds various pipeline state configurations required
     * to create a Vulkan graphics pipeline.
     */
    struct VyPipelineGraphicsConfig
    {
        VyPipelineGraphicsConfig() = default;

        VyPipelineGraphicsConfig(const VyPipelineGraphicsConfig&)            = delete;
        VyPipelineGraphicsConfig& operator=(const VyPipelineGraphicsConfig&) = delete;

        TVector<VkVertexInputBindingDescription>     BindingDescriptions   {};
        TVector<VkVertexInputAttributeDescription>   AttributeDescriptions {};

        TVector<VkPipelineShaderStageCreateInfo>     ShaderStages          {};
        TVector<VkFormat>                            ColorAttachmentFormats{};

        VkPipelineRenderingCreateInfo                RenderingInfo         {};

        VkPipelineViewportStateCreateInfo            ViewportInfo          {};
        VkPipelineInputAssemblyStateCreateInfo       InputAssemblyInfo     {};
        VkPipelineRasterizationStateCreateInfo       RasterizationInfo     {};
        VkPipelineMultisampleStateCreateInfo         MultisampleInfo       {};
        VkPipelineColorBlendAttachmentState          ColorBlendAttachment  {};
        TVector<VkPipelineColorBlendAttachmentState> ColorBlendAttachments {};
        VkPipelineColorBlendStateCreateInfo          ColorBlendInfo        {};
        VkPipelineDepthStencilStateCreateInfo        DepthStencilInfo      {};
        TVector<VkDynamicState>                      DynamicStateEnables   {};
        VkPipelineDynamicStateCreateInfo             DynamicStateInfo      {};

        VkPipelineLayout                             PipelineLayout = nullptr;
        VkRenderPass                                 RenderPass     = nullptr;
        U32                                          Subpass        = 0;
        VkSpecializationInfo*                        pFragSpecInfo  = nullptr;
        VyPipelineFlags                              Flags          = VyPipelineFlags::None;
    };

    // ----------------------------------------------------------------------------------------

    /** 
     * @brief Configuration structure for compute pipeline creation.
     * 
     * This structure holds the shader stage information required
     * to create a Vulkan compute pipeline.
     */
    struct VyPipelineComputeConfig
    {
        VkPipelineShaderStageCreateInfo ShaderStage;
    };


    class VyPipeline 
    {
    public:

        class  GraphicsBuilder;
        friend GraphicsBuilder;

        class  ComputeBuilder;
        friend ComputeBuilder;

        // ----------------------------------------------------------------------------------------

    public:

        VyPipeline() = default;

        VyPipeline(
            TString                         name,
            const VyPipelineLayoutConfig&   layoutConfig, 
            const VyPipelineGraphicsConfig& graphicsConfig
        );

        VyPipeline(
            TString                         name,
            VkPipelineLayout                vkLayout, 
            const VyPipelineGraphicsConfig& graphicsConfig
        );

        VyPipeline(
            TString                         name,
            const VyPipelineLayoutConfig&   layoutConfig, 
            const VyPipelineComputeConfig&  computeConfig
        );

		VyPipeline(const VyPipeline&) = delete;
		VyPipeline(VyPipeline&& other) noexcept;

        ~VyPipeline();

		VyPipeline& operator=(const VyPipeline&) = delete;
		VyPipeline& operator=(VyPipeline&& other) noexcept;

		operator     VkPipeline()                    const { return m_Pipeline;  }
		VY_NODISCARD VkPipeline          handle()    const { return m_Pipeline;  }
        VY_NODISCARD VkPipelineBindPoint bindPoint() const { return m_BindPoint; }
        // VY_NODISCARD VkPipelineLayout    layout()          { return m_Layout;    }
        VY_NODISCARD VkPipelineLayout layout() const { return m_Layout;    }

        VY_NODISCARD bool hasFlag(VyPipelineFlags flag) const
        {
            return (static_cast<U32>(m_Flags) & static_cast<U32>(flag)) != 0;
        }

        /** 
         * @brief Binds the pipeline to the given command buffer.
         * 
         * @param cmdBuffer The command buffer to bind the pipeline to.
         */
		void bind(VkCommandBuffer cmdBuffer) const;


        /** 
         * @brief Binds a descriptor set to the pipeline.
         * 
         * @param cmdBuffer          The command buffer to bind the descriptor set to.
         * @param setIndex           The index of the descriptor set.
         * @param descriptorSet      The descriptor set to bind.
         * @param dynamicOffsetCount (Optional) The number of dynamic offsets in the pDynamicOffsets array.
         * @param pDynamicOffsets    (Optional) A pointer to an array of U32 values specifying dynamic offsets.
         * 
         */
		void bindDescriptorSet(
            VkCommandBuffer cmdBuffer, 
            SetIndex        setIndex, 
            VkDescriptorSet descriptorSet,
            U32             dynamicOffsetCount = 0,
            const U32*      pDynamicOffsets    = nullptr
        ) const;


        // https://docs.vulkan.org/guide/latest/descriptor_dynamic_offset.html

        /**
         * @brief Binds multiple descriptor sets to the pipeline.
         * 
         * @param cmdBuffer          The command buffer to bind the descriptor set to.
         * @param setIndex           The index of the descriptor set.
         * @param descriptorSets     The descriptors set to bind.
         * @param dynamicOffsetCount (Optional) The number of dynamic offsets in the pDynamicOffsets array.
         * @param pDynamicOffsets    (Optional) A pointer to an array of U32 values specifying dynamic offsets.
         */
	    void bindDescriptorSets(
            VkCommandBuffer          cmdBuffer, 
            SetIndex                 setIndex, 
            TVector<VkDescriptorSet> descriptorSets,
            U32                      dynamicOffsetCount = 0,
            const U32*               pDynamicOffsets    = nullptr
        ) const;

        /** 
         * @brief Pushes constants to the pipeline.
         * 
         * @param cmdBuffer  The command buffer to push the constants to.
         * @param stageFlags The shader stages that will use the pushed constants.
         * @param pData      Pointer to the constant data to push.
         * @param size       Size of the constant data in bytes.
         * @param offset     Offset into the push constant range. (Default is 0)
         */
        void pushConstants(
            VkCommandBuffer    cmdBuffer, 
            VkShaderStageFlags stageFlags, 
            const void*        pData, 
            U32                size, 
            U32                offset = 0
        ) const;

        /** 
         * @brief Pushes a typed constant to the pipeline.
         * 
         * @tparam T          Type of the constant data.
         * @param cmdBuffer   The command buffer to push the constant to.
         * @param stageFlags  The shader stages that will use the pushed constant.
         * @param data        Reference to the constant data to push.
         */
		template<typename T>
		void pushConstants(
            VkCommandBuffer    cmdBuffer, 
            VkShaderStageFlags stageFlags, 
            const T&           data,
            U32                offset = 0) const
		{
			pushConstants(cmdBuffer, stageFlags, &data, sizeof(T), offset);
		}
        

        static void defaultGraphicsPipelineConfig(VyPipelineGraphicsConfig& config);

        static void defaultMeshPipelineConfig(VyPipelineGraphicsConfig& config);

        static VkPipelineColorBlendAttachmentState colorBlendAttachmentState();
        static VkPipelineColorBlendAttachmentState colorBlendAttachmentStateNoMask();
        static VkPipelineColorBlendAttachmentState alphaBlendAttachmentState();
    private:

		void createPipelineLayout(const VyPipelineLayoutConfig& config);

        void createGraphicsPipeline(const VyPipelineGraphicsConfig& config);
        void createComputePipeline (const VyPipelineComputeConfig&  config);

        void destroy();

        VkPipeline          m_Pipeline  = VK_NULL_HANDLE;
        VkPipelineLayout    m_Layout    = VK_NULL_HANDLE;
        VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        VyPipelineFlags     m_Flags     = VyPipelineFlags::None;
        TString             m_DebugName{ "unnamed" };
    };



    /** 
     * @brief Builder class for creating Pipeline instances.
     * 
     * This class provides an interface for configuring and building
     * Pipeline objects with specific layout and graphics settings.
     */
    class VyPipeline::GraphicsBuilder
    {
    public:
        GraphicsBuilder();

        GraphicsBuilder(VyPipelineFlags flag);

        ~GraphicsBuilder();

        GraphicsBuilder& setName(TString name);

        // Layout
        GraphicsBuilder& addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
        GraphicsBuilder& addDescriptorSetLayouts(TVector<VkDescriptorSetLayout> descriptorSetLayouts);
        GraphicsBuilder& addPushConstantRange(VkShaderStageFlags stageFlags, U32 size, U32 offset = 0);
        GraphicsBuilder& addShaderStage(VkShaderStageFlagBits stage, const TString& shaderPath);

        GraphicsBuilder& setSubpass(U32 subpass);

        // Specialization Info
        GraphicsBuilder& setFragSpecialization(VkSpecializationInfo* specInfo);

        // Rendering
        GraphicsBuilder& setStencilFormat(VkFormat stencilFormat);
        
        // Input Assembly
        GraphicsBuilder& setTopology(VkPrimitiveTopology topology);

        // Rasterization
        GraphicsBuilder& setDepthClampEnable(bool depthClampEnable);
        GraphicsBuilder& setPolygonMode(VkPolygonMode polygonMode);
        GraphicsBuilder& setCullMode(VkCullModeFlags cullMode);
        GraphicsBuilder& setFrontFace(VkFrontFace frontFace);
        GraphicsBuilder& setDepthBias(float constantFactor, float clamp, float slopeFactor);
        GraphicsBuilder& setDepthBiasEnable(bool enable);
        GraphicsBuilder& setDepthBiasConstantFactor(float constantFactor);
        GraphicsBuilder& setDepthBiasClamp(float clamp);
        GraphicsBuilder& setDepthBiasSlopeFactor(float slopeFactor);
        GraphicsBuilder& setLineWidth(float lineWidth);

        // Multisampling
        GraphicsBuilder& setRasterizationSamples(VkSampleCountFlagBits samples);

        // Color Blending
        GraphicsBuilder& addColorAttachment(VkFormat colorFormat, bool bAlphaBlending = false);
        GraphicsBuilder& addColorAttachment(const VkPipelineColorBlendAttachmentState& attachment);
        GraphicsBuilder& addColorAttachments(VkFormat colorFormat, U32 count, bool bAlphaBlending = false);
        GraphicsBuilder& clearColorAttachments();

        // Depth Stencil
        GraphicsBuilder& setDepthAttachment(VkFormat depthFormat);
        GraphicsBuilder& setDepthTest(bool enableDepthTest, bool writeDepth, VkCompareOp compareOp = VK_COMPARE_OP_LESS);
        GraphicsBuilder& setDepthTestEnable(bool enable);
        GraphicsBuilder& setDepthWriteEnable(bool enable);
        GraphicsBuilder& setDepthCompareOp(VkCompareOp compareOp = VK_COMPARE_OP_LESS);
        
        // Vertex 
        GraphicsBuilder& setVertexBindingDescriptions(const TVector<VkVertexInputBindingDescription>& bindingDescriptions);
        GraphicsBuilder& setVertexAttributeDescriptions(const TVector<VkVertexInputAttributeDescription>& attributeDescriptions);
        GraphicsBuilder& clearVertexDescriptions();

        // Other
        GraphicsBuilder& setRenderPass(VkRenderPass renderPass);
        GraphicsBuilder& addFlag(VyPipelineFlags flag);

        // Build
        Unique<VyPipeline> buildPtr();
        Unique<VyPipeline> buildPtr(VkPipelineLayout layout);
        VyPipeline         build();

    private:
        void addShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entryPoint);

        VyPipelineLayoutConfig   m_LayoutConfig;
        VyPipelineGraphicsConfig m_GraphicsConfig;
        TVector<VkShaderModule>  m_ShaderModules;
        TString                  m_Name{ "unnamed" };
    };

    // ----------------------------------------------------------------------------------------

    /** 
     * @brief Builder class for creating compute Pipeline instances.
     * 
     * This class provides an interface for configuring and building
     * Pipeline objects specifically for compute pipelines.
     */
    class VyPipeline::ComputeBuilder
    {
    public:
        ComputeBuilder() = default;
        ~ComputeBuilder();

        ComputeBuilder& setName(TString name);

        ComputeBuilder& addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

        ComputeBuilder& addPushConstantRange(VkShaderStageFlags stageFlags, U32 size);

        ComputeBuilder& setShaderStage(const TPath& shaderPath, const char* entryPoint = "main");

        Unique<VyPipeline> buildPtr();
        VyPipeline         build();

    private:
        VyPipelineLayoutConfig  m_LayoutConfig;
        VyPipelineComputeConfig m_ComputeConfig;
        TString                 m_Name{ "unnamed" };
    };
}