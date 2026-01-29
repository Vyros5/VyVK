#pragma once


#include <VyEngine/VK/Core/VKCore.h>

namespace Vy
{
    class VyTextureObject;

    class TextureBuilder 
    {
        enum class ESourceType 
        { 
            None, 
            Stb, 
            Hdr, 
            Ktx2, 
            Ktx1, 
            FloatArray, 
            RawBuffer 
        };

        template <size_t W, size_t H, size_t D>
        using TextureArray = TArray<TArray<TArray<float, D>, W>, H>;

    public:
        explicit TextureBuilder();

        //// Input sources ////
        TextureBuilder& fromFile(const TString& path);
        TextureBuilder& fromKTX2(const TString& path);
        TextureBuilder& fromKTX (const TString& path);
        TextureBuilder& fromHDR (const TString& path);
        TextureBuilder& fromSTB (const TString& path);

        template <size_t W, size_t H, size_t D>
        TextureBuilder& fromArray(const TextureArray<W, H, D>& textureArray);

        TextureBuilder& fromVector(const TVector<TVector<TVector<float>>>& textureArray);
        TextureBuilder& fromCharBuffer(TVector<unsigned char> buffer, const size_t width, const size_t height, const size_t channel, const size_t mipLevel);

        //// Texture options ////
        TextureBuilder& withSRGB   (bool enable);
        TextureBuilder& withMipmaps(bool enable);
        TextureBuilder& asCubemap  (bool enable = true);

        //// Sampler options ////
        TextureBuilder& withMinFilter(VkFilter f);
        TextureBuilder& withMagFilter(VkFilter f);
        TextureBuilder& withWrap     (VkSamplerAddressMode mode);

        //// Build ////
        Unique<VyTextureObject> build();

        //// from existing texture ////
        // Unique<VyTextureObject> fromTextureInfo(
        //     VkImageCreateInfo     imageInfo, 
        //     VkImageViewCreateInfo viewInfo, 
        //     VkSamplerCreateInfo   samplerInfo, 
        //     VkImageLayout         initImageLayout, 
        //     U32                   layerCount = 1
        // );

    private:

        //// Build helpers ////
        Unique<VyTextureObject> build2D();
        Unique<VyTextureObject> buildCubemap();
        Unique<VyTextureObject> buildFromArray();
        Unique<VyTextureObject> buildFromCharBuffer();

    private:
        TString m_Path;

        // From Array
        U32            m_ArrayW         = 0;
        U32            m_ArrayH         = 0;
        U32            m_ArrayD         = 0;
        U32            m_ArrayMipLevels = 1;
        TVector<float> m_ArrayPixels;

        // From Char buffer
        TVector<unsigned char> m_CharBuffer;

        bool m_ForceCubemap = false;
        bool m_UseSRGB      = false;
        bool m_UseMipmaps   = false;

        VkFilter             m_MinFilter = VK_FILTER_LINEAR;
        VkFilter             m_MagFilter = VK_FILTER_LINEAR;
        VkSamplerAddressMode m_WrapMode  = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        // Selected decoder type
        ESourceType m_Source = ESourceType::None;
    };

    
    template<size_t W, size_t H, size_t D>
    inline TextureBuilder& TextureBuilder::fromArray(const TextureArray<W, H, D>& textureArray)
    {
        m_Source = ESourceType::Array;

        m_ArrayW = static_cast<U32>(W);
        m_ArrayH = static_cast<U32>(H);
        m_ArrayD = static_cast<U32>(D);

        m_ArrayPixels.resize(W * H * D);

        size_t index = 0;
        for (size_t y = 0; y < H; ++y) 
        {
            for (size_t x = 0; x < W; ++x) 
            {
                for (size_t c = 0; c < D; ++c) 
                {
                    m_ArrayPixels[ index++ ] = textureArray[y][x][c];
                }
            }
        }

        return *this;
    }
}