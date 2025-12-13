#pragma once

#include <Vy/GFX/Backend/Resources/Image.h>
#include <Vy/GFX/Backend/Resources/ImageView.h>
#include <Vy/GFX/Backend/Resources/Sampler.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>

namespace Vy
{
    /**
     * @brief Cubemap texture for skybox rendering
     *
     * Loads 6 face textures (right, left, top, bottom, front, back) into a Vulkan cubemap.
     * Supports JPG, PNG, and other formats via stb_image.
     */
    class Skybox
    {
    public:
        /**
         * @brief Load skybox from 6 separate face images
         * @param device Vulkan device
         * @param facePaths Array of 6 paths: [+X, -X, +Y, -Y, +Z, -Z]
         *                  (right, left, top, bottom, front, back)
         */
        Skybox(const TArray<String, 6>& facePaths);

        /**
         * @brief Load skybox from folder with standard naming
         * @param device Vulkan device
         * @param folderPath Folder containing posx.jpg, negx.jpg, etc.
         * @param extension File extension (default: "jpg")
         */
        static Unique<Skybox> loadFromFolder(const String& folderPath, const String& extension = "jpg");

        ~Skybox();

        Skybox(const Skybox&)            = delete;
        Skybox& operator=(const Skybox&) = delete;
        
        Skybox(Skybox&&)                 = delete;
        Skybox& operator=(Skybox&&)      = delete;

        VkImageView imageView() const { return m_ImageView.handle(); }
        VkSampler   sampler()   const { return m_Sampler.handle(); }

        VkDescriptorImageInfo getDescriptorInfo() const
        {
            return VkDescriptorImageInfo{
                    .sampler     = m_Sampler  .handle(),
                    .imageView   = m_ImageView.handle(),
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        int getSize() const { return m_Size; }

    private:
        void createCubemapImage(const TArray<String, 6>& facePaths);

        void createImageView();
        
        void createSampler();
        
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        
        // void copyBufferToImage(VkBuffer buffer, U32 faceIndex);

        VyImage        m_Image    ;
        VyImageView    m_ImageView;
        VySampler      m_Sampler  ;

        int m_Size = 0; // Width/height of each face (assumed square)
    };
}