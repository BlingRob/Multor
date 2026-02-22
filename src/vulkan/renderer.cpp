/// \file renderer.h

#include "renderer.h"

#include <chrono>
#include <cstring>
#include <functional>
#include <unordered_map>

namespace Multor::Vulkan
{

Renderer::Renderer(std::shared_ptr<Window> pWnd)
    : FrameChain(std::move(pWnd))
    , logger_(Logging::LoggerFactory::GetLogger("vulkan.log"))
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    
    shFactory_   = std::make_unique<ShaderFactory>(device);
    shadowResources_ = std::make_unique<ShadowResources>(device, physicDev);
    directionalShadowMaps_ =
        shadowResources_->CreateDirectionalShadowArray();
    pointShadowMaps_ = shadowResources_->CreatePointShadowCubeArray();
    shadowPass_ =
        std::make_unique<ShadowPass>(device, directionalShadowMaps_.format_);
    shadowRenderer_ =
        std::make_unique<ShadowRenderer>(device, commandPool, graphicsQueue, executer_);
    shadowPass_->BuildFramebuffers(*shadowResources_, directionalShadowMaps_,
                                   pointShadowMaps_);
    executer_->TransitionImageLayoutLayers(
        directionalShadowMaps_.image_, directionalShadowMaps_.format_,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0,
        directionalShadowMaps_.layers_);
    executer_->TransitionImageLayoutLayers(
        pointShadowMaps_.image_, pointShadowMaps_.format_,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0,
        pointShadowMaps_.layers_);
    activeShader_ =
        CreateShaderFromFiles("../../shaders/Base.vs", "../../shaders/Base.frag");
    shadowDirectionalShader_ = CreateShaderFromFiles(
        "../../shaders/ShadowDirectional.vs",
        "../../shaders/ShadowDirectional.frag");

    createDescriptorSetLayout();
    createShadowPipeline();
    createGraphicsPipeline();
    createDescriptorPool();
    createDescriptorSets();
    createUniformBuffers();
    createCommandBuffers();
    createSyncObjects();
    shadowCommandBuffersInFlight_.assign(maxFramesInFlight_, VK_NULL_HANDLE);
}

void Renderer::AddLight(std::shared_ptr<Multor::BLight> light)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    if (!light)
        throw std::runtime_error("light is null");

    light->SetChangedCallback(std::bind(&Renderer::markShadowsDirty, this));
    lights_.push_back(std::move(light));
    markShadowsDirty();
}

void Renderer::SetLights(std::vector<std::shared_ptr<Multor::BLight> > lights)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    for (auto& light : lights_)
        {
            if (light)
                light->SetChangedCallback({});
        }
    for (auto& light : lights)
        {
            if (light)
                light->SetChangedCallback(std::bind(&Renderer::markShadowsDirty, this));
        }
    lights_ = std::move(lights);
    markShadowsDirty();
}

void Renderer::ClearLights()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    for (auto& light : lights_)
        {
            if (light)
                light->SetChangedCallback({});
        }
    lights_.clear();
    markShadowsDirty();
}

void Renderer::InvalidateShadows()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    markShadowsDirty();
}

void Renderer::SetLightingEnabled(bool enabled)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    lightingEnabled_ = enabled;
}

bool Renderer::IsLightingEnabled() const
{
    return lightingEnabled_;
}

void Renderer::SetShadowsEnabled(bool enabled)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    if (shadowsEnabled_ == enabled)
        return;
    shadowsEnabled_ = enabled;
    if (enabled)
        markShadowsDirty();
}

bool Renderer::IsShadowsEnabled() const
{
    return shadowsEnabled_;
}

const std::vector<std::shared_ptr<Multor::BLight> >& Renderer::GetLights() const
{
    return lights_;
}

std::shared_ptr<ShaderLayout> Renderer::CreateShaderFromSource(
    std::string_view vertex, std::string_view fragment, std::string_view geometry)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    auto shader = shFactory_->CreateShader(vertex, fragment, geometry);
    shaders_.push_back(shader);
    return shader;
}

std::shared_ptr<ShaderLayout> Renderer::CreateShaderFromFiles(
    std::string_view vertexPath, std::string_view fragmentPath,
    std::string_view geometryPath)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    std::string geometry;
    if (!geometryPath.empty())
        geometry = LoadTextFile(geometryPath);

    return CreateShaderFromSource(LoadTextFile(vertexPath),
                                  LoadTextFile(fragmentPath), geometry);
}

void Renderer::UseShader(const std::shared_ptr<ShaderLayout>& shader)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    if (!shader)
        throw std::runtime_error("shader layout is null");
    if (shader->GetStages()->empty())
        throw std::runtime_error("shader layout has no stages");

    activeShader_ = shader;

    for (auto& mesh : meshes_)
        mesh->sh_ = std::make_shared<Shader>(activeShader_);

    vkDeviceWaitIdle(device);
    clearIncludePart();

    if (descriptorSetLayout_ != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout_, nullptr);
            descriptorSetLayout_ = VK_NULL_HANDLE;
        }
    if (graphicsPipeline_ != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device, graphicsPipeline_, nullptr);
            graphicsPipeline_ = VK_NULL_HANDLE;
        }
    if (pipelineLayout_ != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
    createDescriptorSetLayout();
    createShadowPipeline();
    createGraphicsPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

void Renderer::SetOverlayDrawCallback(std::function<void(VkCommandBuffer)> callback)
{
    overlayDrawCallback_ = std::move(callback);
}

void Renderer::createGraphicsPipeline()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext                         = nullptr;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions    = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext                  = nullptr;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = nullptr;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext                   = nullptr;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace =
        VK_FRONT_FACE_CLOCKWISE; // VK_FRONT_FACE_COUNTER_CLOCKWISE;//;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp          = 0.0f;
    rasterizer.depthBiasSlopeFactor    = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext                 = nullptr;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable       = VK_TRUE;
    depthStencil.depthWriteEnable      = VK_TRUE;
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds        = 0.0f; // Optional
    depthStencil.maxDepthBounds        = 1.0f; // Optional
    depthStencil.stencilTestEnable     = VK_FALSE;
    depthStencil.front                 = {}; // Optional
    depthStencil.back                  = {}; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext             = nullptr;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = nullptr;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates    = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.setLayoutCount         = 1;
    pipelineLayoutInfo.pSetLayouts            = &descriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                               &pipelineLayout_) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    //pipelineInfo.stageCount = 2;
    //pipelineInfo.pStages = stgs;
    pipelineInfo.stageCount          = static_cast<std::uint32_t>(activeShader_->GetStages()->size());
    pipelineInfo.pStages             = activeShader_->GetStages()->data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = pipelineLayout_;
    pipelineInfo.renderPass          = renderPass_;
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex   = -1;

    //Create pip
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &graphicsPipeline_) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");
}

void Renderer::createShadowPipeline()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    if (!shadowRenderer_ || !shadowPass_)
        throw std::runtime_error("shadow renderer/pass is not initialized");
    shadowRenderer_->RecreateDirectionalPipeline(shadowDirectionalShader_,
                                                 shadowPass_->GetRenderPass());
}

void Renderer::createCommandBuffers()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    commandBuffers_.resize(swapChainFramebuffers_.size());
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext       = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();
    //Create command buffer for each framebuffer
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers_.data()) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

    for (size_t i = 0; i < commandBuffers_.size(); i++)
        recordCommandBuffer(static_cast<uint32_t>(i));
}

void Renderer::recordCommandBuffer(uint32_t i)
{
    if (i >= commandBuffers_.size())
        throw std::out_of_range("command buffer index out of range");

    vkResetCommandBuffer(commandBuffers_[i], 0);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext            = nullptr;
    beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    if (vkBeginCommandBuffer(commandBuffers_[i], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass        = renderPass_;
    renderPassInfo.framebuffer       = swapChainFramebuffers_[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent_;
    std::array<VkClearValue, 2> clearValues {};
    clearValues[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    VkViewport viewport {};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(swapChainExtent_.width);
    viewport.height   = static_cast<float>(swapChainExtent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent_;

    vkCmdBeginRenderPass(commandBuffers_[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffers_[i], 0, 1, &viewport);
    vkCmdSetScissor(commandBuffers_[i], 0, 1, &scissor);
    vkCmdBindPipeline(commandBuffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphicsPipeline_);

    VkDeviceSize offsets[] = {0};
    for (auto& mesh : meshes_)
        {
            vkCmdBindVertexBuffers(commandBuffers_[i], 0, 1,
                                   &mesh->vertBuffer_->pVertBuf_->buffer_,
                                   offsets);
            vkCmdBindIndexBuffer(commandBuffers_[i], mesh->indexBuffer_->buffer_,
                                 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffers_[i],
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout_, 0, 1, &mesh->sh_->desSet_[i],
                                    0, nullptr);
            vkCmdDrawIndexed(commandBuffers_[i],
                             static_cast<uint32_t>(mesh->indexesSize_), 1, 0, 0,
                             0);
        }

    if (overlayDrawCallback_)
        overlayDrawCallback_(commandBuffers_[i]);

    vkCmdEndRenderPass(commandBuffers_[i]);
    if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void Renderer::Draw()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    //barrier to wait for showing past frame
    vkWaitForFences(device, 1, &syncers_[currentFrame_].inFlightFences_, VK_FALSE,
                    UINT64_MAX);
    if (currentFrame_ < shadowCommandBuffersInFlight_.size() &&
        shadowCommandBuffersInFlight_[currentFrame_] != VK_NULL_HANDLE &&
        shadowRenderer_)
        {
            shadowRenderer_->FreeCommandBuffer(
                shadowCommandBuffersInFlight_[currentFrame_]);
            shadowCommandBuffersInFlight_[currentFrame_] = VK_NULL_HANDLE;
        }
    //Get next ingex image for redering
    VkResult result =
        vkAcquireNextImageKHR(device, swapChain_, UINT64_MAX,
                              syncers_[currentFrame_].imageAvailableSemaphores_,
                              VK_NULL_HANDLE, &imageIndex_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    updateMats(imageIndex_);
    if (shadowsEnabled_ && shadowMapsDirty_ && shadowMapsInFlightFence_ != VK_NULL_HANDLE &&
        shadowMapsInFlightFence_ != syncers_[currentFrame_].inFlightFences_)
        {
            vkWaitForFences(device, 1, &shadowMapsInFlightFence_, VK_TRUE,
                            UINT64_MAX);
        }
    VkCommandBuffer shadowCmd = VK_NULL_HANDLE;
    if (shadowsEnabled_ && shadowMapsDirty_ && shadowRenderer_ && shadowPass_)
        {
            shadowCmd = shadowRenderer_->BuildShadowCommandBufferAll(
                meshes_, *shadowPass_, directionalShadowMaps_, pointShadowMaps_,
                shadowPackCache_, imageIndex_);
            if (currentFrame_ < shadowCommandBuffersInFlight_.size())
                shadowCommandBuffersInFlight_[currentFrame_] = shadowCmd;
        }

    if (syncers_[imageIndex_].imagesInFlight_ != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &syncers_[imageIndex_].imagesInFlight_, VK_TRUE,
                        UINT64_MAX);
    syncers_[imageIndex_].imagesInFlight_ = syncers_[currentFrame_].inFlightFences_;
    recordCommandBuffer(imageIndex_);

    VkSubmitInfo submitInfo {};
    submitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                  = nullptr;
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores =
        &syncers_[currentFrame_].imageAvailableSemaphores_;
    submitInfo.pWaitDstStageMask    = waitStages;
    VkCommandBuffer submitCmds[2]   = {shadowCmd, commandBuffers_[imageIndex_]};
    submitInfo.commandBufferCount   =
        (shadowCmd == VK_NULL_HANDLE) ? 1u : 2u;
    submitInfo.pCommandBuffers =
        (shadowCmd == VK_NULL_HANDLE) ? &commandBuffers_[imageIndex_]
                                      : submitCmds;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores =
        &syncers_[currentFrame_].renderFinishedSemaphores_;

    vkResetFences(device, 1, &syncers_[currentFrame_].inFlightFences_);
    //Send command buffer in graphics queue
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                      syncers_[currentFrame_].inFlightFences_) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");
    if (shadowCmd != VK_NULL_HANDLE)
        {
            shadowMapsInFlightFence_ = syncers_[currentFrame_].inFlightFences_;
            shadowMapsDirty_         = false;
        }

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =
        &syncers_[currentFrame_].renderFinishedSemaphores_;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains    = &swapChain_;
    presentInfo.pImageIndices  = &imageIndex_;
    presentInfo.pResults       = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        Update();
    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");

    currentFrame_ = (currentFrame_ + 1) % maxFramesInFlight_;
}

void Renderer::drawDirectionalShadows()
{
    if (!shadowRenderer_ || !shadowPass_)
        return;
    shadowRenderer_->DrawDirectional(meshes_, *shadowPass_, directionalShadowMaps_,
                                     shadowPackCache_, imageIndex_);
}

void Renderer::drawPointShadows()
{
    if (!shadowRenderer_ || !shadowPass_)
        return;
    shadowRenderer_->DrawPoint(meshes_, *shadowPass_, pointShadowMaps_,
                               shadowPackCache_, imageIndex_);
}

void Renderer::drawShadows()
{
    if (!shadowRenderer_ || !shadowPass_)
        return;
    shadowRenderer_->DrawAll(meshes_, *shadowPass_, directionalShadowMaps_,
                             pointShadowMaps_, shadowPackCache_, imageIndex_);
}

void Renderer::createSyncObjects()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    syncers_.reserve(maxFramesInFlight_);
    for (uint16_t i = 0; i < maxFramesInFlight_; ++i)
        syncers_.emplace_back(device);
}

void Renderer::createDescriptorPool()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    std::unordered_map<VkDescriptorType, uint32_t> descriptorsPerSet;
    for (const auto& binding : *activeShader_->GetLayoutBindings())
        descriptorsPerSet[binding.descriptorType] += binding.descriptorCount;

    const uint32_t meshCount =
        std::max(1u, static_cast<uint32_t>(meshes_.size()));
    const uint32_t setCount = meshCount * static_cast<uint32_t>(swapChainImages_.size());

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(descriptorsPerSet.size());
    for (const auto& [type, countPerSet] : descriptorsPerSet)
        {
            poolSizes.push_back(VkDescriptorPoolSize {
                type, countPerSet * setCount});
        }

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext         = nullptr;
    poolInfo.flags         = 0;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = setCount;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool_) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool!");
}

void Renderer::createDescriptorSetLayout()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    // VkDescriptorSetLayoutBinding Layouts[1];
    /*
	//Transformation
	Layouts[0].binding = 0;
	Layouts[0].descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	Layouts[0].descriptorCount = 1;
	Layouts[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	Layouts[0].pImmutableSamplers = nullptr;
	

	//ViewPos
	Layouts[1].binding = 1;
	Layouts[1].descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	Layouts[1].descriptorCount = 1;
	Layouts[1].stageFlags = VK_SHADER_STAGE_ALL;
	Layouts[1].pImmutableSamplers = nullptr;

	//Material
	Layouts[2].binding = 2;
	Layouts[2].descriptorType =
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	Layouts[2].descriptorCount = 1;
	Layouts[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	for (uint16_t i = 3; i < 8; ++i)
	{
		Layouts[i].binding = i;
		Layouts[i].descriptorCount = 1;
		Layouts[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		Layouts[i].pImmutableSamplers = nullptr;
		Layouts[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	}*/
    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.bindingCount = static_cast<uint32_t>(activeShader_->GetLayoutBindings()->size());
    layoutInfo.pBindings = activeShader_->GetLayoutBindings()->data(); //Layouts;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                    &descriptorSetLayout_) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor setlayout!");
}

void Renderer::createUniformBuffers()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    lightsUbo_ = std::make_unique<LightsUBO>(device);
    lightsUbo_->buffers_.clear();
    lightsUbo_->buffers_.reserve(swapChainImages_.size());
    for (size_t i = 0; i < swapChainImages_.size(); ++i)
        lightsUbo_->buffers_.push_back(
            meshFactory_->CreateUniformBuffer(LightsUBO::LightsBufObj));

    directionalShadowUboBuffers_.clear();
    directionalShadowUboBuffers_.reserve(swapChainImages_.size());
    for (size_t i = 0; i < swapChainImages_.size(); ++i)
        directionalShadowUboBuffers_.push_back(meshFactory_->CreateUniformBuffer(
            sizeof(UBOs::DirectionalShadows)));

    pointShadowUboBuffers_.clear();
    pointShadowUboBuffers_.reserve(swapChainImages_.size());
    for (size_t i = 0; i < swapChainImages_.size(); ++i)
        pointShadowUboBuffers_.push_back(meshFactory_->CreateUniformBuffer(
            sizeof(UBOs::PointShadows)));

    for (auto& mesh : meshes_)
        {
            mesh->tr_ = meshFactory_->CreateUBOBuffers(swapChainImages_.size());
            if (mesh->tr_)
                {
                    mesh->tr_->SetModelChangedCallback(
                        std::bind(&Renderer::markShadowsDirty, this));
                }
            /*
		for (size_t i = 0; i < swapChainImages.size(); ++i)
		{
			mesh->matrixes.push_back(MeshFac->createBuffer(TransBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
			/*
			mesh->materialUBO_.push_back(MeshFac->createBuffer(MatBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

			mesh->viewPosUBO_.push_back(MeshFac->createBuffer(ViewBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		}*/
        }
}

void Renderer::updateMats(uint32_t currentImage)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    static auto startTime   = std::chrono::high_resolution_clock::now();
    auto        currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    UBOs::Transform ubo {};
    ubo.model_        = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
    auto controller = _pWnd ? _pWnd->GetController() : nullptr;
    if (!controller || !controller->projection_ || !controller->view_)
        throw std::runtime_error(
            "Renderer::updateMats requires valid PositionController matrices");
    ubo.PV_           = (*controller->projection_) * (*controller->view_);
    ubo.normalMatrix_ = glm::transpose(glm::inverse(ubo.model_));
    const glm::vec3 viewPos = controller->cam_ ? controller->cam_->position_
                                               : glm::vec3(0.0f);

    if (lightsUbo_)
        {
            std::vector<const Multor::BLight*> lightPtrs;
            lightPtrs.reserve(lights_.size());
            for (const auto& light : lights_)
                if (light)
                    lightPtrs.push_back(light.get());
            lightsUbo_->update(currentImage,
                               lightingEnabled_ ? PackLights(lightPtrs)
                                                : UBOs::Lights {});
            shadowPackCache_ = shadowsEnabled_ ? UBOs::PackShadowData(lightPtrs)
                                               : UBOs::ShadowPack {};
            if (currentImage < directionalShadowUboBuffers_.size() &&
                directionalShadowUboBuffers_[currentImage])
                {
                    void* data = nullptr;
                    vkMapMemory(device,
                                directionalShadowUboBuffers_[currentImage]
                                    ->bufferMemory_,
                                0, sizeof(UBOs::DirectionalShadows), 0, &data);
                    std::memcpy(data, &shadowPackCache_.directional_,
                                sizeof(UBOs::DirectionalShadows));
                    vkUnmapMemory(device,
                                  directionalShadowUboBuffers_[currentImage]
                                      ->bufferMemory_);
                }
            if (currentImage < pointShadowUboBuffers_.size() &&
                pointShadowUboBuffers_[currentImage])
                {
                    void* data = nullptr;
                    vkMapMemory(device,
                                pointShadowUboBuffers_[currentImage]
                                    ->bufferMemory_,
                                0, sizeof(UBOs::PointShadows), 0, &data);
                    std::memcpy(data, &shadowPackCache_.point_,
                                sizeof(UBOs::PointShadows));
                    vkUnmapMemory(device,
                                  pointShadowUboBuffers_[currentImage]
                                      ->bufferMemory_);
                }
        }

    for (auto& mesh : meshes_)
        {
            mesh->tr_->updatePV(currentImage, ubo.PV_);
            mesh->tr_->updateView(currentImage, viewPos);
            //mesh->tr_->updateModel(currentImage, glm::mat4(1.0f));
        }
}

bool Renderer::hasStencilComponent(VkFormat format)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Renderer::createDescriptorSets()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    for (auto& mesh : meshes_)
        {
            std::vector<VkDescriptorSetLayout> layouts(swapChainImages_.size(),
                                                       descriptorSetLayout_);
            VkDescriptorSetAllocateInfo        allocInfo {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = nullptr;
            allocInfo.descriptorPool = descriptorPool_;
            allocInfo.descriptorSetCount =
                static_cast<uint32_t>(swapChainImages_.size());
            allocInfo.pSetLayouts = layouts.data();
            mesh->sh_->desSet_.resize(swapChainImages_.size());

            if (vkAllocateDescriptorSets(
                    device, &allocInfo, mesh->sh_->desSet_.data()) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate descriptor sets!");

            for (size_t i = 0; i < swapChainImages_.size(); ++i)
                {
                    std::vector<VkWriteDescriptorSet> descriptorWrites {};
                    std::vector<VkDescriptorBufferInfo> descriptorBufferInfos {};
                    std::vector<VkDescriptorImageInfo> descriptorImageInfos {};
                    const auto bindingCount =
                        activeShader_->GetLayoutBindings()->size();
                    descriptorWrites.reserve(bindingCount);
                    descriptorBufferInfos.reserve(bindingCount);
                    descriptorImageInfos.reserve(bindingCount);
                    for (const auto& layout : *activeShader_->GetLayoutBindings())
                        {
                            if (layout.descriptorType ==
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                                {
                                    VkDescriptorBufferInfo bufferInfo {};
                                    if (layout.binding == 0)
                                        {
                                            bufferInfo.buffer =
                                                mesh->tr_->matrixes_[i]->buffer_;
                                            bufferInfo.range =
                                                sizeof(UBOs::Transform);
                                        }
                                    else if (layout.binding == 1 && lightsUbo_ &&
                                             i < lightsUbo_->buffers_.size())
                                        {
                                            bufferInfo.buffer =
                                                lightsUbo_->buffers_[i]->buffer_;
                                            bufferInfo.range =
                                                LightsUBO::LightsBufObj;
                                        }
                                    else if (layout.binding == 2 &&
                                             i < mesh->tr_->viewPosUBO_.size())
                                        {
                                            bufferInfo.buffer =
                                                mesh->tr_->viewPosUBO_[i]->buffer_;
                                            bufferInfo.range =
                                                sizeof(UBOs::ViewPosition);
                                        }
                                    else if (layout.binding == 4 &&
                                             i < directionalShadowUboBuffers_.size())
                                        {
                                            bufferInfo.buffer =
                                                directionalShadowUboBuffers_[i]
                                                    ->buffer_;
                                            bufferInfo.range =
                                                sizeof(UBOs::DirectionalShadows);
                                        }
                                    else if (layout.binding == 6 &&
                                             i < pointShadowUboBuffers_.size())
                                        {
                                            bufferInfo.buffer =
                                                pointShadowUboBuffers_[i]
                                                    ->buffer_;
                                            bufferInfo.range =
                                                sizeof(UBOs::PointShadows);
                                        }
                                    else
                                        {
                                            throw std::runtime_error(
                                                "unsupported uniform buffer binding");
                                        }
                                    bufferInfo.offset = 0;
                                    descriptorBufferInfos.push_back(bufferInfo);

                                    descriptorWrites.push_back(
                                        {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         nullptr, mesh->sh_->desSet_[i],
                                         layout.binding, 0, 1,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         nullptr,
                                         &descriptorBufferInfos.back(),
                                         nullptr});
                                }

                            if (layout.descriptorType ==
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                                {
                                    VkDescriptorImageInfo imageInfo {};
                                    if (layout.binding == 3)
                                        {
                                            if (mesh->textures_.empty())
                                                throw std::runtime_error(
                                                    "mesh has no texture for sampler binding");
                                            imageInfo.imageLayout =
                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                            imageInfo.imageView =
                                                (*mesh->textures_.begin())->view_;
                                            imageInfo.sampler =
                                                (*mesh->textures_.begin())->sampler_;
                                        }
                                    else if (layout.binding == 5)
                                        {
                                            imageInfo.imageLayout =
                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                            imageInfo.imageView =
                                                directionalShadowMaps_.view_;
                                            imageInfo.sampler =
                                                directionalShadowMaps_.sampler_;
                                        }
                                    else if (layout.binding == 7)
                                        {
                                            imageInfo.imageLayout =
                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                            imageInfo.imageView =
                                                pointShadowMaps_.view_;
                                            imageInfo.sampler =
                                                pointShadowMaps_.sampler_;
                                        }
                                    else
                                        {
                                            throw std::runtime_error(
                                                "unsupported sampler binding");
                                        }
                                    descriptorImageInfos.push_back(imageInfo);
                                    descriptorWrites.push_back(
                                        {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         nullptr, mesh->sh_->desSet_[i],
                                         layout.binding, 0, 1,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                         &descriptorImageInfos.back(),
                                         nullptr, nullptr});
                                }
                        }
                    /*
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mesh->tr_->matrixes[i]->buffer_;// TransformUBO[i]->buffer_;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UBOs::Transform);
			uint32_t bindpoint = 0;
			std::vector<VkWriteDescriptorSet> descriptorWrites{};
			descriptorWrites.push_back(
				{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				mesh->DesSet[i],
				bindpoint,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfo,
				nullptr
				});*/
                    /*
			bufferInfo.buffer = mesh->viewPosUBO_[i]->buffer_;//viewPosUBO_[i]->buffer_;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UBOs::ViewPosition);

			++bindpoint;
			descriptorWrites.push_back(
				{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				mesh->DesSet[i],
				bindpoint,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfo,
				nullptr
				});

			bufferInfo.buffer = mesh->materialUBO_[i]->buffer_;// materialUBO_[i]->buffer_;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Material);

			++bindpoint;
			descriptorWrites.push_back(
				{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				nullptr,
				mesh->DesSet[i],
				bindpoint,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&bufferInfo,
				nullptr
				});

			//bind texture with set
			for (auto& tex : mesh->textures)
			{
				bindpoint++;
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = tex->view_;
				imageInfo.sampler = tex->sampler_;

				descriptorWrites.push_back
				(
					{
					VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					nullptr,
					mesh->DesSet[i],
					bindpoint,
					0,
					1,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					&imageInfo,
					nullptr,
					nullptr
					}
				);
				
			}*/

                    vkUpdateDescriptorSets(
                        device, static_cast<uint32_t>(descriptorWrites.size()),
                        descriptorWrites.data(), 0, nullptr);
                }
        }
}

void Renderer::Update()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    vkDeviceWaitIdle(device);

    clearIncludePart();

    createUniformBuffers();

    createDescriptorPool();
    createDescriptorSets();
    RecreateSwapChain();
    createCommandBuffers();
}

std::shared_ptr<Mesh> Renderer::AddMesh(BaseMesh* mesh)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    meshes_.push_back(meshFactory_->CreateMesh(std::unique_ptr<BaseMesh>(mesh)));
    meshes_.back()->sh_ = std::make_shared<Shader>(activeShader_);
    markShadowsDirty();
    Update();
    return meshes_.back();
}

std::vector<std::shared_ptr<Mesh> >
Renderer::AddMeshes(std::vector<std::unique_ptr<BaseMesh> > meshes)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    std::vector<std::shared_ptr<Mesh> > result;
    result.reserve(meshes.size());

    bool addedAny = false;
    for (auto& mesh : meshes)
        {
            if (!mesh)
                continue;
            meshes_.push_back(meshFactory_->CreateMesh(std::move(mesh)));
            meshes_.back()->sh_ = std::make_shared<Shader>(activeShader_);
            result.push_back(meshes_.back());
            addedAny = true;
        }

    if (addedAny)
        {
            markShadowsDirty();
            Update();
        }

    return result;
}

void Renderer::ClearMeshes()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    meshes_.clear();
    markShadowsDirty();
    Update();
}

void Renderer::markShadowsDirty()
{
    shadowMapsDirty_ = true;
}

void Renderer::clearIncludePart()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    //materialUBO_.clear();
    //TransformUBO.clear();
    //viewPosUBO_.clear();

    for (auto& mesh : meshes_)
        {
            mesh->tr_.reset();
            /*
		for (size_t i = 0; i < swapChainImages.size(); ++i)
		{
			mesh->materialUBO_.clear();
			mesh->viewPosUBO_.clear();
			mesh->matrixes.clear();
        }*/
        }
    lightsUbo_.reset();
    directionalShadowUboBuffers_.clear();
    pointShadowUboBuffers_.clear();
    if (shadowRenderer_)
        {
            for (auto& cmd : shadowCommandBuffersInFlight_)
                {
                    if (cmd != VK_NULL_HANDLE)
                        shadowRenderer_->FreeCommandBuffer(cmd);
                    cmd = VK_NULL_HANDLE;
                }
        }
    shadowCommandBuffersInFlight_.assign(maxFramesInFlight_, VK_NULL_HANDLE);
    shadowMapsInFlightFence_ = VK_NULL_HANDLE;
    shadowMapsDirty_ = true;

    vkFreeCommandBuffers(device, commandPool,
                         static_cast<uint32_t>(commandBuffers_.size()),
                         commandBuffers_.data());
    commandBuffers_.clear();
    vkDestroyDescriptorPool(device, descriptorPool_, nullptr);
    descriptorPool_ = VK_NULL_HANDLE;
}

Renderer::~Renderer()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    vkDeviceWaitIdle(device);

    for (auto& light : lights_)
        {
            if (light)
                light->SetChangedCallback({});
        }

    syncers_.clear();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout_, nullptr);
    descriptorSetLayout_ = VK_NULL_HANDLE;
    clearIncludePart();
    vkDestroyPipeline(device, graphicsPipeline_, nullptr);
    graphicsPipeline_ = VK_NULL_HANDLE;
    shadowRenderer_.reset();
    shadowPass_.reset();
    shadowResources_.reset();
    pointShadowMaps_ = {};
    directionalShadowMaps_ = {};
    shFactory_.reset();
    vkDestroyPipelineLayout(device, pipelineLayout_, nullptr);
    pipelineLayout_ = VK_NULL_HANDLE;
}

} // namespace Multor::Vulkan
