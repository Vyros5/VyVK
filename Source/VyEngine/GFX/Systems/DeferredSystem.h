#pragma once

#include <VyEngine/GFX/Resources/Misc/GBuffer.h>
#include <VyEngine/GFX/Resources/Misc/LightBuffer.h>

#include <VyEngine/VK/Pipeline/Pipeline.h>
#include <VyEngine/GFX/Data/FrameInfo.h>
#include <VyEngine/GFX/Resources/Misc/HDRImage.h>

namespace Vy
{

	struct GeometryPassPush
	{
		Mat4 Transform;       //  64 bytes
		Mat4 ModelMatrix;     //  64 bytes
		U32  AlbedoIndex;      //   4 bytes
		U32  NormalIndex;      //   4 bytes
		U32  MetalRoughIndex;  //   4 bytes
		U32  OcclusionIndex;   //   4 bytes

	};

	struct LightingPassPush 
    {
		Mat4  View;       
		Mat4  Projection;         
		Vec2  Resolution;   
		float _pad0[2];   
		Vec3  CameraPos;    
		U32   LightCount;  
	};

	struct DepthPush
	{
		Mat4 MVP;
		U32  AlbedoIndex;
	};

	enum class LightType : U32 
    { 
        Point = 0,
        Directional = 1 
    };

	struct alignas(16) Light
	{
		Vec3      Position{};
		float     Radius{};
		Vec3      Direction{};
		LightType Type{};
		Vec3      LightColor{}; 
		float     LightIntensity{};
	};

	enum class DebugOutput { 
		Lighting = 0,
		Position,
		Normal,
		Albedo,
		MetalRough,
		Occlusion,
		Depth,
		COUNT
	};

	class DeferredRenderSystem final
	{
	public:
		DeferredRenderSystem(
            VkExtent2D extent, 
            VkFormat swapFormat, 
            Shared<HDRImage>& hdrImage, 
            TVector<Light>& lights
        );
		
        ~DeferredRenderSystem();

		DeferredRenderSystem(const DeferredRenderSystem& other) = delete;
		DeferredRenderSystem& operator=(const DeferredRenderSystem& rhs) = delete;
		DeferredRenderSystem(const DeferredRenderSystem&& other) = delete;
		DeferredRenderSystem& operator=(const DeferredRenderSystem&& rhs) = delete;

		void Initialize(VkExtent2D extent, VkFormat swapFormat); 
		void RenderGeometry(VyFrameInfo& frameInfo);
		// void UpdateGeometry(VyFrameInfo& frameInfo);
		void RenderLighting(VyFrameInfo& frameInfo, VkExtent2D extent);
		void RenderBlit(VkCommandBuffer cmdBuffer); 
		void RenderDepthPrepass(VyFrameInfo& frameInfo);

		void RecreateGBuffer(VkExtent2D extent, VkFormat swapFormat);
		void CycleDebugOutput(); 

		GBuffer& GetGBuffer() { return m_GBuffer;  }
		LightBuffer& GetLightBuffer() { return m_LightingPassBuffer; }

	private:

		void CreateDepthPrepassPipeline();
		// void CreateDepthPrepassPipelineLayout();

		// void CreateGeometryPipelineLayout(); 
		void CreateGeometryPipeline();

		void CreateLightingPipelineLayout();
		void CreateLightingPipeline();
		void CreateLightingDescriptorSet();


		// void CreateBlitPipelineLayout();
		void CreateBlitPipeline(VkFormat swapFormat);
		void CreateBlitDescriptorSet();
		void CreateLightsBuffer(size_t maxLights);

		GBuffer						m_GBuffer;
		LightBuffer					m_LightingPassBuffer;  
		// VkPipelineLayout			m_GeometryPipelineLayout, m_LightPipelineLayout, m_DepthPrepassPipelineLayout, m_BlitPipelineLayout;
		std::unique_ptr<VyPipeline>	m_GeometryPipeline, m_LightPipeline, m_DepthPrepassPipeline, m_BlitPipeline;
		VkDescriptorSet				m_GeometryDescriptorSet, m_LightDescriptorSet, m_BlitDescriptorSet; 
		VkDescriptorSetLayout		m_LightingPassDescriptorSetLayout, m_BlitDescriptorSetLayout; 
		VkDescriptorPool			m_LightingPassDescriptorPool, m_BlitDescriptorPool;

		Unique<VyBuffer>		m_LightsBuffer; 
		// VkDeviceMemory			m_LightsBufferMemory;
		size_t					m_MaxLights = 0; 
		VkDescriptorSetLayout   m_PointLightsDescriptorSetLayout;
		VkDescriptorPool        m_PointLightsDescriptorPool;
		VkDescriptorSet         m_PointLightsDescriptorSet;

		TVector<Light> m_CPULights;

		Shared<HDRImage> m_HDRImage;
		DebugOutput m_DebugOutput{ DebugOutput::Lighting };
	};
}