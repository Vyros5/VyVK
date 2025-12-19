#pragma once

#include <Vy/GFX/Backend/Device.h>

namespace Vy 
{
    class VyPipeline 
    {
    public:

		enum class EFlags
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
		struct LayoutConfig
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
		struct GraphicsConfig
		{
			GraphicsConfig() = default;

			GraphicsConfig(const GraphicsConfig&)            = delete;
			GraphicsConfig& operator=(const GraphicsConfig&) = delete;

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
            EFlags                                       Flags          = EFlags::None;
		};

        // ----------------------------------------------------------------------------------------

        /** 
         * @brief Builder class for creating Pipeline instances.
         * 
         * This class provides an interface for configuring and building
         * Pipeline objects with specific layout and graphics settings.
         */
		class GraphicsBuilder
		{
		public:
			GraphicsBuilder();

            GraphicsBuilder(VyPipeline::EFlags flag);

			~GraphicsBuilder();

            // Layout
			GraphicsBuilder& addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
            GraphicsBuilder& addDescriptorSetLayouts(TVector<VkDescriptorSetLayout> descriptorSetLayouts);
            GraphicsBuilder& addPushConstantRange(VkShaderStageFlags stageFlags, U32 size, U32 offset = 0);
			GraphicsBuilder& addShaderStage(VkShaderStageFlagBits stage, const String& shaderPath);

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
            GraphicsBuilder& setLineWidth(float lineWidth);

            // Color Blending
			GraphicsBuilder& addColorAttachment(VkFormat colorFormat, bool alphaBlending = false);

            // Depth Stencil
            GraphicsBuilder& setDepthAttachment(VkFormat depthFormat);
            GraphicsBuilder& setDepthTest(bool enableDepthTest, bool writeDepth, VkCompareOp compareOp = VK_COMPARE_OP_LESS);
			
            // Vertex 
            GraphicsBuilder& setVertexBindingDescriptions(const TVector<VkVertexInputBindingDescription>& bindingDescriptions);
            GraphicsBuilder& setVertexAttributeDescriptions(const TVector<VkVertexInputAttributeDescription>& attributeDescriptions);
            GraphicsBuilder& clearVertexDescriptions();

            // Other
            GraphicsBuilder& setRenderPass(VkRenderPass renderPass);
            GraphicsBuilder& addFlag(VyPipeline::EFlags flag);

            // Build
			Unique<VyPipeline> buildUnique();
            Unique<VyPipeline> buildUnique(VkPipelineLayout layout);
			VyPipeline build();

		private:
			void addShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entryPoint);

			LayoutConfig            m_LayoutConfig;
			GraphicsConfig          m_GraphicsConfig;
			TVector<VkShaderModule> m_ShaderModules;
		};

        // ----------------------------------------------------------------------------------------

        /** 
         * @brief Configuration structure for compute pipeline creation.
         * 
         * This structure holds the shader stage information required
         * to create a Vulkan compute pipeline.
         */
		struct ComputeConfig
		{
			VkPipelineShaderStageCreateInfo ShaderStage;
		};

        // ----------------------------------------------------------------------------------------

        /** 
         * @brief Builder class for creating compute Pipeline instances.
         * 
         * This class provides an interface for configuring and building
         * Pipeline objects specifically for compute pipelines.
         */
		class ComputeBuilder
		{
		public:
			ComputeBuilder() = default;
			~ComputeBuilder();

			ComputeBuilder& addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

			ComputeBuilder& addPushConstantRange(VkShaderStageFlags stageFlags, U32 size);

			ComputeBuilder& setShaderStage(const Path& shaderPath, const char* entryPoint = "main");

			Unique<VyPipeline> buildUnique();
			VyPipeline         build();

		private:
			LayoutConfig  m_LayoutConfig;
			ComputeConfig m_ComputeConfig;
		};

        // ----------------------------------------------------------------------------------------

        VyPipeline() = default;

        VyPipeline(
            const VyPipeline::LayoutConfig&   layoutConfig, 
            const VyPipeline::GraphicsConfig& graphicsConfig
        );

        VyPipeline(
            VkPipelineLayout                  layout, 
            const VyPipeline::GraphicsConfig& graphicsConfig
        );

        VyPipeline(
            const VyPipeline::LayoutConfig&   layoutConfig, 
            const VyPipeline::ComputeConfig&  computeConfig
        );

		VyPipeline(const VyPipeline&) = delete;
		VyPipeline(VyPipeline&& other) noexcept;

        ~VyPipeline();

		VyPipeline& operator=(const VyPipeline&) = delete;
		VyPipeline& operator=(VyPipeline&& other) noexcept;

		operator     VkPipeline()                    const { return m_Pipeline;  }
		VY_NODISCARD VkPipeline          handle()    const { return m_Pipeline;  }
        VY_NODISCARD VkPipelineBindPoint bindPoint() const { return m_BindPoint; }
        VY_NODISCARD VkPipelineLayout    layout()    const { return m_Layout;    }

        VY_NODISCARD bool hasFlag(EFlags flag) const
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
         * @param cmdBuffer     The command buffer to bind the descriptor set to.
         * @param setIndex      The index of the descriptor set.
         * @param descriptorSet The descriptor set to bind.
         */
		void bindDescriptorSet(
            VkCommandBuffer cmdBuffer, 
            SetIndex        setIndex, 
            VkDescriptorSet descriptorSet,
            U32             dynamicOffsetCount = 0,
            const U32*      pDynamicOffsets    = nullptr
        ) const;



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
        void pushConstants(VkCommandBuffer cmdBuffer, VkShaderStageFlags stageFlags, const void* pData, U32 size, U32 offset = 0) const;

        /** 
         * @brief Pushes a typed constant to the pipeline.
         * 
         * @tparam T          Type of the constant data.
         * @param cmdBuffer   The command buffer to push the constant to.
         * @param stageFlags  The shader stages that will use the pushed constant.
         * @param data        Reference to the constant data to push.
         */
		template<typename T>
		void pushConstants(VkCommandBuffer cmdBuffer, VkShaderStageFlags stageFlags, const T& data) const
		{
			pushConstants(cmdBuffer, stageFlags, &data, sizeof(T));
		}

        static void defaultGraphicsPipelineConfig(VyPipeline::GraphicsConfig& config);

        static void defaultMeshPipelineConfig(VyPipeline::GraphicsConfig& config);

    private:

		void createPipelineLayout(const VyPipeline::LayoutConfig& config);

        void createGraphicsPipeline(const VyPipeline::GraphicsConfig& config);
        void createComputePipeline (const VyPipeline::ComputeConfig&  config);

        void destroy();

        VkPipeline          m_Pipeline  = VK_NULL_HANDLE;
        VkPipelineLayout    m_Layout    = VK_NULL_HANDLE;
        VkPipelineBindPoint m_BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        EFlags              m_Flags     = EFlags::None;
    };
}