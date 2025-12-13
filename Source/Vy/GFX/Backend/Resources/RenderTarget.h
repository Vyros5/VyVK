#pragma once

#include <Vy/GFX/Resources/Texture.h>

namespace Vy
{
    class RenderTarget 
    {
    public:
        Shared<VyTexture> ColorTexture{};  // Shared pointer to color texture
        Shared<VyTexture> DepthTexture{};  // Shared pointer to depth texture

        enum class AttachmentType 
        {
            COLOR,
            DEPTH,
            BOTH
        };

        enum class TargetType 
        {
            TEXTURE_2D,
            CUBEMAP
        };

        RenderTarget(
            U32               width, 
            U32               height,
            VkFormat          colorFormat, 
            VkImageUsageFlags colorUsage,
            VkFormat          depthFormat, 
            VkImageUsageFlags depthUsage,
            AttachmentType    attachmentType = AttachmentType::BOTH,
            TargetType        targetType     = TargetType::TEXTURE_2D
        ) : 
            m_Target(targetType) 
        {
            if (attachmentType == AttachmentType::COLOR || 
                attachmentType == AttachmentType::BOTH) 
            {
                ColorTexture = createColorTexture(targetType, width, height, colorFormat, colorUsage);
            }

            if (attachmentType == AttachmentType::DEPTH || 
                attachmentType == AttachmentType::BOTH) 
            {
                DepthTexture = createDepthTexture(width, height, depthFormat, depthUsage);
            }
        }


        static Shared<VyTexture> createColorTexture(
            TargetType            target,
            U32                   width, 
            U32                   height,
            VkFormat              format, 
            VkImageUsageFlags     usage, 
            VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
        {
            Shared<VyTexture> texture;

            if (target == TargetType::CUBEMAP) 
            {
                texture = VyTexture::Builder()
                    .size(width, height)
                    .format(format)
                    .usage(usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
                    .asCubemap()
                    .build();
            } 
            else 
            {
                texture = VyTexture::Builder()
                    .size(width, height)
                    .format(format)
                    .usage(usage)
                    .sampleCount(samples)
                    .build();
            }
            return texture;
        }


        static Shared<VyTexture> createDepthTexture(
            U32               width, 
            U32               height, 
            VkFormat          format, 
            VkImageUsageFlags usage)
        {
            Shared<VyTexture> texture;

            texture = VyTexture::Builder()
                .size(width, height)
                .format(format)
                .usage(usage)
                .build();

            return texture;
        }

    private:
        TargetType m_Target;
    };
}