/// \file renderer.h

#pragma once

#include "shader_factory.h"
#include "frame_chain.h"
#include "objects/texture.h"
#include "objects/buffer.h"
#include "syncer.h"
#include "structures/light_ubo.h"
#include "structures/shadow_ubo.h"
#include "shadow_resources.h"
#include "shadow_pass.h"
#include "shadow_renderer.h"
#include "../utils/files_tools.h"
#include "../scene_objects/light.h"

#include <vector>
#include <set>
#include <algorithm>
#include <list>
#include <memory>
#include <string_view>
#include <functional>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class Renderer : public FrameChain
{
public:
    Renderer(std::shared_ptr<Window> pWnd);

    ~Renderer();

    std::shared_ptr<Mesh> AddMesh(BaseMesh* mesh);
    std::vector<std::shared_ptr<Mesh> >
    AddMeshes(std::vector<std::unique_ptr<BaseMesh> > meshes);
    void ClearMeshes();
    void AddLight(std::shared_ptr<Multor::BLight> light);
    void SetLights(std::vector<std::shared_ptr<Multor::BLight> > lights);
    void ClearLights();
    void InvalidateShadows();
    void SetLightingEnabled(bool enabled);
    bool IsLightingEnabled() const;
    void SetShadowsEnabled(bool enabled);
    bool IsShadowsEnabled() const;
    const std::vector<std::shared_ptr<Multor::BLight> >& GetLights() const;
    std::shared_ptr<ShaderLayout>
    CreateShaderFromSource(std::string_view vertex, std::string_view fragment,
                           std::string_view geometry = "");
    std::shared_ptr<ShaderLayout>
    CreateShaderFromFiles(std::string_view vertexPath,
                          std::string_view fragmentPath,
                          std::string_view geometryPath = "");
    void UseShader(const std::shared_ptr<ShaderLayout>& shader);
    void SetOverlayDrawCallback(std::function<void(VkCommandBuffer)> callback);

    void Draw();
    void Update();

    size_t GetCurFrame()
    {
        return imageIndex_;
    };
    VkInstance GetVkInstance() const { return instance; }
    VkPhysicalDevice GetVkPhysicalDevice() const { return physicDev; }
    VkDevice GetVkDevice() const { return device; }
    VkQueue GetVkGraphicsQueue() const { return graphicsQueue; }
    uint32_t GetVkGraphicsQueueFamilyIndex() const { return physicDevIndices.graphicsFamily.value(); }
    VkCommandPool GetVkCommandPool() const { return commandPool; }
    VkRenderPass GetVkRenderPass() const { return renderPass_; }
    uint32_t GetSwapchainImageCount() const { return static_cast<uint32_t>(swapChainImages_.size()); }
    uint32_t GetMinImageCount() const { return 2u; }

private:
    void createGraphicsPipeline();
    void createShadowPipeline();
    void createCommandBuffers();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void createUniformBuffers();
    void createSyncObjects();
    void recordCommandBuffer(uint32_t index);

    bool hasStencilComponent(VkFormat format);

    void clearIncludePart();

    void updateMats(uint32_t currentImage);
    void markShadowsDirty();
    void drawShadows();
    void drawDirectionalShadows();
    void drawPointShadows();

private:
    const int maxFramesInFlight_ = 3;
    size_t    currentFrame_      = 0;
    uint32_t  imageIndex_        = 0;

    Logging::Logger& logger_;

    std::unique_ptr<ShaderFactory>              shFactory_;
    std::vector<std::shared_ptr<ShaderLayout> > shaders_;
    std::shared_ptr<ShaderLayout>               activeShader_;
    std::shared_ptr<ShaderLayout>               shadowDirectionalShader_;

    VkPipelineLayout      pipelineLayout_      = VK_NULL_HANDLE;
    VkPipeline            graphicsPipeline_    = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
    VkDescriptorPool      descriptorPool_      = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> commandBuffers_;
    std::vector<VkCommandBuffer> shadowCommandBuffersInFlight_;
    std::vector<Syncer>          syncers_;
    VkFence shadowMapsInFlightFence_ = VK_NULL_HANDLE;
    bool shadowMapsDirty_ = true;
    bool lightingEnabled_ = true;
    bool shadowsEnabled_ = true;

    std::list<std::shared_ptr<Mesh> > meshes_;
    std::vector<std::shared_ptr<Multor::BLight> > lights_;
    std::unique_ptr<LightsUBO> lightsUbo_;
    std::vector<std::unique_ptr<Buffer> > directionalShadowUboBuffers_;
    std::vector<std::unique_ptr<Buffer> > pointShadowUboBuffers_;
    std::unique_ptr<ShadowResources> shadowResources_;
    std::unique_ptr<ShadowPass> shadowPass_;
    std::unique_ptr<ShadowRenderer> shadowRenderer_;
    ShadowMapArray directionalShadowMaps_;
    ShadowMapArray pointShadowMaps_;
    UBOs::ShadowPack shadowPackCache_ {};
    std::function<void(VkCommandBuffer)> overlayDrawCallback_;
};

} // namespace Multor::Vulkan
