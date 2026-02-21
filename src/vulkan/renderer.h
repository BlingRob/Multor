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

    size_t getCurFrame()
    {
        return imageIndex;
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

    void clearInlcudePart();

    void UpdateMats(uint32_t currentImage);

private:
    const int MAX_FRAMES_IN_FLIGHT = 3;
    size_t    currentFrame         = 0;
    uint32_t  imageIndex;

    Logging::Logger& logger_;

    std::unique_ptr<ShaderFactory>              ShFactory;
    std::vector<std::shared_ptr<ShaderLayout> > shaders_;
    std::shared_ptr<ShaderLayout>               activeShader_;

    VkPipelineLayout      pipelineLayout;
    VkPipeline            graphicsPipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool      descriptorPool;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<Syncer>          syncers;

    std::list<std::shared_ptr<Mesh> > meshes_;
};

} // namespace Multor::Vulkan
