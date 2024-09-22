/// \file VulkanRenderer.h

#pragma once
#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "vk_shader_factory.h"
#include "VkFrameChain.h"
#include "objects/vk_texture.h"
#include "vk_sync.h"
#include "../utils/files_tools.h"

#include <vector>
#include <set>
#include <algorithm>
#include <list>
#include <memory>

#include <vulkan/vulkan.h>

namespace Multor
{

class VulkanRenderer : public FrameChain
{
public:
    VulkanRenderer(std::shared_ptr<Window>          pWnd,
                   std::shared_ptr<Logging::Logger> pLog)
        : FrameChain(std::move(pWnd), std::move(pLog))
    {
        ShFactory = std::make_unique<VkShaderFactory>(device);
        shaders_.push_back(
            ShFactory->createShader(LoadTextFile("../../shaders/Base.vs"),
                                    LoadTextFile("../../shaders/Base.frag")));

        createDescriptorPool();
        createDescriptorSets();
        createDescriptorSetLayout();

        //Include meshes
        //createUniformBuffers();
        //createTextureImage();
        //createTextureImageView();
        //createTextureSampler();
        //createVertexBuffer();
        //createIndexBuffer();
        //Update layout

        createGraphicsPipeline();

        createCommandBuffers();
        createUniformBuffers();
        createSyncObjects();
    }

    ~VulkanRenderer();

    std::shared_ptr<VkMesh> AddMesh(BaseMesh* mesh);

    void Draw();
    void Update();

    size_t getCurFrame()
    {
        return imageIndex;
    };

private:
    const int MAX_FRAMES_IN_FLIGHT = 3;
    size_t    currentFrame         = 0;
    uint32_t  imageIndex;

    std::unique_ptr<VkShaderFactory>              ShFactory;
    std::vector<std::shared_ptr<ShaderLayout> > shaders_;

    VkPipelineLayout      pipelineLayout;
    VkPipeline            graphicsPipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool      descriptorPool;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSyncer>        syncers;

    std::list<std::shared_ptr<VkMesh> > meshes_;

    void createGraphicsPipeline();
    void createCommandBuffers();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void createUniformBuffers();
    void createSyncObjects();

    bool hasStencilComponent(VkFormat format);

    void ClearInlcudePart();

    void UpdateMats(uint32_t currentImage);
};

} // namespace Multor

#endif // VULKANRENDERER_H