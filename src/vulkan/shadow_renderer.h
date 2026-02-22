/// \file shadow_renderer.h

#pragma once

#include "command_executer.h"
#include "mesh.h"
#include "shadow_pass.h"
#include "shadow_resources.h"
#include "shader.h"
#include "structures/shadow_ubo.h"

#include <list>
#include <memory>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class ShadowRenderer
{
public:
    ShadowRenderer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
                   std::shared_ptr<CommandExecuter> executer);
    ~ShadowRenderer();

    ShadowRenderer(const ShadowRenderer&) = delete;
    ShadowRenderer& operator=(const ShadowRenderer&) = delete;

    void RecreateDirectionalPipeline(const std::shared_ptr<ShaderLayout>& shader,
                                     VkRenderPass renderPass);
    void DestroyDirectionalPipeline();

    void DrawDirectional(const std::list<std::shared_ptr<Mesh> >& meshes,
                         const ShadowPass& shadowPass,
                         ShadowMapArray& directionalShadowMaps,
                         const UBOs::ShadowPack& shadowPack,
                         uint32_t frameIndex);

    void DrawPoint(const std::list<std::shared_ptr<Mesh> >& meshes,
                   const ShadowPass& shadowPass, ShadowMapArray& pointShadowMaps,
                   const UBOs::ShadowPack& shadowPack, uint32_t frameIndex);

    void DrawAll(const std::list<std::shared_ptr<Mesh> >& meshes,
                 const ShadowPass& shadowPass,
                 ShadowMapArray& directionalShadowMaps,
                 ShadowMapArray& pointShadowMaps,
                 const UBOs::ShadowPack& shadowPack, uint32_t frameIndex);

    VkCommandBuffer BuildShadowCommandBufferAll(
        const std::list<std::shared_ptr<Mesh> >& meshes,
        const ShadowPass& shadowPass, ShadowMapArray& directionalShadowMaps,
        ShadowMapArray& pointShadowMaps, const UBOs::ShadowPack& shadowPack,
        uint32_t frameIndex);
    void FreeCommandBuffer(VkCommandBuffer cmd) const;

private:
    VkCommandBuffer beginOneTimeCommand() const;
    void endOneTimeCommand(VkCommandBuffer cmd) const;

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    std::shared_ptr<CommandExecuter> executer_;

    VkPipelineLayout directionalPipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline directionalPipeline_ = VK_NULL_HANDLE;
};

} // namespace Multor::Vulkan
