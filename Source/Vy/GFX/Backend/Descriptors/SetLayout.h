#pragma once

#include <Vy/GFX/Backend/Device.h>

namespace Vy
{
    using VyLayoutBindingMap      = THashMap<BindingIndex, VkDescriptorSetLayoutBinding>;
    using VyLayoutBindingFlagsMap = THashMap<BindingIndex, VkDescriptorBindingFlags>;
    
    /**
     * @brief Manages a Vulkan descriptor set layout.
     *
     * This class encapsulates the creation and lifetime of a `VkDescriptorSetLayout`,
     * using a builder interface to define descriptor bindings.
     */
    class VyDescriptorSetLayout 
    {
        friend class VyDescriptorWriter;

    public:
		// ----------------------------------------------------------------------------------------

        /**
         * @brief Builder class for creating a VyDescriptorSetLayout.
         *
         * This class allows for a more flexible and readable way to create a VyDescriptorSetLayout
         * by chaining method calls to set various parameters.
         */
        class Builder 
        {
        public:
            Builder() = default;

            /**
             * @brief Adds a descriptor binding to the layout.
             *
             * @param binding        Binding index (must be unique).
             * @param descriptorType Type of descriptor (e.g., uniform buffer, sampler).
             * @param stageFlags     Shader stages that will access the binding.
             * @param count          (Optional) Number of descriptors in the binding (default is 1).
             * 
             * @return Reference to the Builder for chaining.
             *
             * @note Throws an assertion failure if the binding is already in use.
             */
			Builder& addBinding(
				BindingIndex             binding, 
				VkDescriptorType         descriptorType, 
				VkShaderStageFlags       stageFlags,
				U32                      count        = 1,
                VkDescriptorBindingFlags bindingFlags = 0
			);


            Builder& setBindingFlags(BindingIndex binding, VkDescriptorBindingFlags flags);


            Builder& setLayoutFlags(VkDescriptorSetLayoutCreateFlags flags);

            /**
             * @brief Finalizes and builds the VyDescriptorSetLayout.
             *
             * @return A unique pointer to the created VyDescriptorSetLayout.
             * 
             * @throws std::runtime_error if Vulkan layout creation fails.
             */
			VY_NODISCARD
			Unique<VyDescriptorSetLayout> buildUnique() const;

            /**
             * @brief Finalizes and builds the VyDescriptorSetLayout.
             *
             * @return The created VyDescriptorSetLayout.
             * 
             * @throws std::runtime_error if Vulkan layout creation fails.
             */
			VY_NODISCARD
			VyDescriptorSetLayout build() const;

        private:
            VyLayoutBindingMap               m_Bindings    {};
            VyLayoutBindingFlagsMap          m_BindingFlags{};
            VkDescriptorSetLayoutCreateFlags m_LayoutFlags = 0;
        };

		// ----------------------------------------------------------------------------------------

        VyDescriptorSetLayout(VyLayoutBindingMap bindings);

        VyDescriptorSetLayout(
            VyLayoutBindingMap               bindings, 
            VyLayoutBindingFlagsMap          bindingFlags, 
            VkDescriptorSetLayoutCreateFlags layoutFlags
        );
        
		VyDescriptorSetLayout(const VyDescriptorSetLayout&) = delete;
		VyDescriptorSetLayout(VyDescriptorSetLayout&& other) noexcept;
		
		~VyDescriptorSetLayout();

		VyDescriptorSetLayout& operator=(const VyDescriptorSetLayout&) = delete;
		VyDescriptorSetLayout& operator=(VyDescriptorSetLayout&& other) noexcept;

        /**
         * @brief Returns the Vulkan descriptor set layout handle.
         * 
         * @return VkDescriptorSetLayout object.
         */
		VY_NODISCARD
		VkDescriptorSetLayout handle() const { return m_SetLayout; }

		VY_NODISCARD 
        operator VkDescriptorSetLayout() const { return m_SetLayout; }

        VkDescriptorSet allocateDescriptorSet() const;

    private:
		VkDescriptorSetLayout m_SetLayout{ VK_NULL_HANDLE };
		VyLayoutBindingMap    m_Bindings;
    };
}