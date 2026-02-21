/// \file renderer.h

#include "renderer.h"

#include <chrono>
#include <unordered_map>

namespace Multor::Vulkan
{

Renderer::Renderer(std::shared_ptr<Window> pWnd)
    : FrameChain(std::move(pWnd))
    , logger_(Logging::LoggerFactory::GetLogger("vulkan.log"))
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);
    
    ShFactory    = std::make_unique<ShaderFactory>(device);
    activeShader_ =
        CreateShaderFromFiles("../../shaders/Base.vs", "../../shaders/Base.frag");

    createDescriptorSetLayout();
    createGraphicsPipeline();
    createDescriptorPool();
    createDescriptorSets();
    createUniformBuffers();
    createCommandBuffers();
    createSyncObjects();
}

std::shared_ptr<ShaderLayout> Renderer::CreateShaderFromSource(
    std::string_view vertex, std::string_view fragment, std::string_view geometry)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    auto shader = ShFactory->createShader(vertex, fragment, geometry);
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
    if (shader->getStages()->empty())
        throw std::runtime_error("shader layout has no stages");

    activeShader_ = shader;

    for (auto& mesh : meshes_)
        mesh->sh_ = std::make_shared<Shader>(activeShader_);

    vkDeviceWaitIdle(device);
    clearInlcudePart();

    if (descriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
            descriptorSetLayout = VK_NULL_HANDLE;
        }
    if (graphicsPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device, graphicsPipeline, nullptr);
            graphicsPipeline = VK_NULL_HANDLE;
        }
    if (pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }

    createDescriptorSetLayout();
    createGraphicsPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
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
    depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
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
    pipelineLayoutInfo.pSetLayouts            = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                               &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;
    //pipelineInfo.stageCount = 2;
    //pipelineInfo.pStages = stgs;
    pipelineInfo.stageCount          = activeShader_->getStages()->size();
    pipelineInfo.pStages             = activeShader_->getStages()->data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = pipelineLayout;
    pipelineInfo.renderPass          = renderPass;
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex   = -1;

    //Create pip
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &graphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphics pipeline!");
}

void Renderer::createCommandBuffers()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    commandBuffers.resize(swapChainFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext       = nullptr;
    allocInfo.commandPool = commandPool;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    //Create command buffer for each framebuffer
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

    for (size_t i = 0; i < commandBuffers.size(); i++)
        {
            //writing down commands
            VkCommandBufferBeginInfo beginInfo {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags =
                VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Optional
            beginInfo.pInheritanceInfo = nullptr;             // Optional
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) !=
                VK_SUCCESS)
                throw std::runtime_error(
                    "failed to begin recording command buffer!");

            VkRenderPassBeginInfo renderPassInfo {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.pNext = nullptr;
            renderPassInfo.renderPass        = renderPass;
            renderPassInfo.framebuffer       = swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChainExtent;
            std::array<VkClearValue, 2> clearValues {};
            clearValues[0].color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassInfo.clearValueCount =
                static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();
            VkViewport viewport {};
            viewport.x        = 0.0f;
            viewport.y        = 0.0f;
            viewport.width    = (float)swapChainExtent.width;
            viewport.height   = (float)swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor {};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                                 VK_SUBPASS_CONTENTS_INLINE);

            //SetViewPort
            vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
            //SetScissor
            vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);
            //Include pipline to commandbuffer
            vkCmdBindPipeline(commandBuffers[i],
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              graphicsPipeline);
            //Include drawing operation to commandbuffer
            VkDeviceSize offsets[] = {0};
            for (auto& mesh : meshes_)
                {
                    vkCmdBindVertexBuffers(
                        commandBuffers[i], 0, 1,
                        &mesh->vertBuffer_->pVertBuf_->buffer_, offsets);
                    vkCmdBindIndexBuffer(commandBuffers[i],
                                         mesh->indexBuffer_->buffer_, 0,
                                         VK_INDEX_TYPE_UINT32);
                    vkCmdBindDescriptorSets(commandBuffers[i],
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            pipelineLayout, 0, 1,
                                            &mesh->sh_->DesSet[i], 0, nullptr);
                    vkCmdDrawIndexed(commandBuffers[i],
                                     static_cast<uint32_t>(mesh->indexesSize_),
                                     1, 0, 0, 0);
                }

            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to record command buffer!");
        }
}

void Renderer::Draw()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    //barrier to wait for showing past frame
    vkWaitForFences(device, 1, &syncers[currentFrame].inFlightFences, VK_FALSE,
                    UINT64_MAX);
    //Get next ingex image for redering
    VkResult result =
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                              syncers[currentFrame].imageAvailableSemaphores,
                              VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    //updateUniformBuffer(imageIndex);
    UpdateMats(imageIndex);

    if (syncers[imageIndex].imagesInFlight != VK_NULL_HANDLE)
        vkWaitForFences(device, 1, &syncers[imageIndex].imagesInFlight, VK_TRUE,
                        UINT64_MAX);
    syncers[imageIndex].imagesInFlight = syncers[imageIndex].inFlightFences;

    VkSubmitInfo submitInfo {};
    submitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                  = nullptr;
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores =
        &syncers[currentFrame].imageAvailableSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores =
        &syncers[currentFrame].renderFinishedSemaphores;

    vkResetFences(device, 1, &syncers[currentFrame].inFlightFences);
    //Send command buffer in graphics queue
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                      syncers[currentFrame].inFlightFences) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =
        &syncers[currentFrame].renderFinishedSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains    = &swapChain;
    presentInfo.pImageIndices  = &imageIndex;
    presentInfo.pResults       = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        Update();
    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::createSyncObjects()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    syncers.reserve(MAX_FRAMES_IN_FLIGHT);
    for (uint16_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        syncers.emplace_back(device);
}

void Renderer::createDescriptorPool()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    std::unordered_map<VkDescriptorType, uint32_t> descriptorsPerSet;
    for (const auto& binding : *activeShader_->getLayoutBindings())
        descriptorsPerSet[binding.descriptorType] += binding.descriptorCount;

    const uint32_t meshCount =
        std::max(1u, static_cast<uint32_t>(meshes_.size()));
    const uint32_t setCount = meshCount * static_cast<uint32_t>(swapChainImages.size());

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

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool!");
}

void Renderer::createDescriptorSetLayout()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    VkDescriptorSetLayoutBinding Layouts[1];
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
    layoutInfo.bindingCount = activeShader_->getLayoutBindings()->size();
    layoutInfo.pBindings = activeShader_->getLayoutBindings()->data(); //Layouts;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                    &descriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor setlayout!");
}

void Renderer::createUniformBuffers()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    for (auto& mesh : meshes_)
        {
            mesh->tr_ = MeshFac->createUBOBuffers(swapChainImages.size());
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

void Renderer::UpdateMats(uint32_t currentImage)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    static auto startTime   = std::chrono::high_resolution_clock::now();
    auto        currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
    proj[1][1] *= -1;

    UBOs::Transform ubo {};
    ubo.model_        = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.PV_           = proj * glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                                           glm::vec3(0.0f, 0.0f, 0.0f),
                                           glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.normalMatrix_ = glm::mat3(glm::transpose(glm::inverse(ubo.model_)));

    for (auto& mesh : meshes_)
        {
            mesh->tr_->updatePV(currentImage, ubo.PV_);
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
            std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(),
                                                       descriptorSetLayout);
            VkDescriptorSetAllocateInfo        allocInfo {};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.pNext = nullptr;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount =
                static_cast<uint32_t>(swapChainImages.size());
            allocInfo.pSetLayouts = layouts.data();
            mesh->sh_->DesSet.resize(swapChainImages.size());

            if (vkAllocateDescriptorSets(
                    device, &allocInfo, mesh->sh_->DesSet.data()) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate descriptor sets!");

            for (size_t i = 0; i < swapChainImages.size(); ++i)
                {
                    std::vector<VkWriteDescriptorSet> descriptorWrites {};
                    for (const auto& layout : *activeShader_->getLayoutBindings())
                        {
                            if (layout.descriptorType ==
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                                {
                                    VkDescriptorBufferInfo bufferInfo {};
                                    bufferInfo.buffer =
                                        mesh->tr_->matrixes_[i]
                                            ->buffer_; // TransformUBO[i]->buffer_;
                                    bufferInfo.offset = 0;
                                    bufferInfo.range  = sizeof(UBOs::Transform);

                                    descriptorWrites.push_back(
                                        {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         nullptr, mesh->sh_->DesSet[i],
                                         layout.binding, 0, 1,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         nullptr, &bufferInfo, nullptr});
                                }

                            if (layout.descriptorType ==
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                                {
                                    if (mesh->textures_.empty())
                                        throw std::runtime_error(
                                            "mesh has no texture for sampler binding");

                                    VkDescriptorImageInfo imageInfo {};
                                    imageInfo.imageLayout =
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                    imageInfo.imageView =
                                        (*mesh->textures_.begin())->view_;
                                    imageInfo.sampler =
                                        (*mesh->textures_.begin())->sampler_;
                                    descriptorWrites.push_back(
                                        {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         nullptr, mesh->sh_->DesSet[i],
                                         layout.binding, 0, 1,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                         &imageInfo, nullptr, nullptr});
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

    clearInlcudePart();

    createUniformBuffers();

    createDescriptorPool();
    createDescriptorSets();
    RecreateSwapChain();
    createCommandBuffers();
}

std::shared_ptr<Mesh> Renderer::AddMesh(BaseMesh* mesh)
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    meshes_.push_back(MeshFac->createMesh(std::unique_ptr<BaseMesh>(mesh)));
    meshes_.back()->sh_ = std::make_shared<Shader>(activeShader_);
    Update();
    return meshes_.back();
}

void Renderer::clearInlcudePart()
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

    vkFreeCommandBuffers(device, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());
    commandBuffers.clear();
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    descriptorPool = VK_NULL_HANDLE;
}

Renderer::~Renderer()
{
    LOG_TRACE_L1(logger_.get(), __FUNCTION__);

    vkDeviceWaitIdle(device);

    syncers.clear();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    descriptorSetLayout = VK_NULL_HANDLE;
    clearInlcudePart();
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    graphicsPipeline = VK_NULL_HANDLE;
    ShFactory.reset();
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    pipelineLayout = VK_NULL_HANDLE;
}

} // namespace Multor::Vulkan
