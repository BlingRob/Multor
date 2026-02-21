/// \file renderer.h

#pragma once

#include "shader_factory.h"
#include "frame_chain.h"
#include "objects/texture.h"
#include "syncer.h"
#include "../utils/files_tools.h"

#include <vector>
#include <set>
#include <algorithm>
#include <list>
#include <memory>
#include <string_view>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class Renderer : public FrameChain
{
public:
    Renderer(std::shared_ptr<Window> pWnd);

    ~Renderer();

    std::shared_ptr<Mesh> AddMesh(BaseMesh* mesh);
    std::shared_ptr<ShaderLayout>
    CreateShaderFromSource(std::string_view vertex, std::string_view fragment,
                           std::string_view geometry = "");
    std::shared_ptr<ShaderLayout>
    CreateShaderFromFiles(std::string_view vertexPath,
                          std::string_view fragmentPath,
                          std::string_view geometryPath = "");
    void UseShader(const std::shared_ptr<ShaderLayout>& shader);

    void Draw();
    void Update();

    size_t GetCurFrame()
    {
        return imageIndex_;
    };

private:
    void createGraphicsPipeline();
    void createCommandBuffers();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void createUniformBuffers();
    void createSyncObjects();

    bool hasStencilComponent(VkFormat format);

    void clearIncludePart();

    void updateMats(uint32_t currentImage);

private:
    const int maxFramesInFlight_ = 3;
    size_t    currentFrame_      = 0;
    uint32_t  imageIndex_        = 0;

    Logging::Logger& logger_;

    std::unique_ptr<ShaderFactory>              shFactory_;
    std::vector<std::shared_ptr<ShaderLayout> > shaders_;
    std::shared_ptr<ShaderLayout>               activeShader_;

    VkPipelineLayout      pipelineLayout_      = VK_NULL_HANDLE;
    VkPipeline            graphicsPipeline_    = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
    VkDescriptorPool      descriptorPool_      = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> commandBuffers_;
    std::vector<Syncer>          syncers_;

    std::list<std::shared_ptr<Mesh> > meshes_;
};

} // namespace Multor::Vulkan
