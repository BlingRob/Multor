/// \file shadow_renderer.cpp

#include "shadow_renderer.h"

#include "objects/vertex.h"

#include <algorithm>
#include <stdexcept>

namespace Multor::Vulkan
{
namespace
{
bool hasStencilComponentShadow(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void recordDepthLayoutTransition(VkCommandBuffer cmd, VkImage image, VkFormat format,
                                 VkImageLayout oldLayout,
                                 VkImageLayout newLayout, uint32_t layerCount)
{
    VkImageMemoryBarrier barrier {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = layerCount;
    if (hasStencilComponentShadow(format))
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    VkPipelineStageFlags sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage           = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage      = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
    else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage           = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage      = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
    else
        {
            throw std::invalid_argument("unsupported shadow depth layout transition");
        }

    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);
}
} // namespace


ShadowRenderer::ShadowRenderer(VkDevice device, VkCommandPool commandPool,
                               VkQueue graphicsQueue,
                               std::shared_ptr<CommandExecuter> executer)
    : device_(device),
      commandPool_(commandPool),
      graphicsQueue_(graphicsQueue),
      executer_(std::move(executer))
{
}

ShadowRenderer::~ShadowRenderer()
{
    DestroyDirectionalPipeline();
}

void ShadowRenderer::DestroyDirectionalPipeline()
{
    if (directionalPipeline_ != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device_, directionalPipeline_, nullptr);
            directionalPipeline_ = VK_NULL_HANDLE;
        }
    if (directionalPipelineLayout_ != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device_, directionalPipelineLayout_, nullptr);
            directionalPipelineLayout_ = VK_NULL_HANDLE;
        }
}

void ShadowRenderer::RecreateDirectionalPipeline(
    const std::shared_ptr<ShaderLayout>& shader, VkRenderPass renderPass)
{
    if (!shader || shader->GetStages()->empty())
        throw std::runtime_error("shadow shader is not initialized");
    if (renderPass == VK_NULL_HANDLE)
        throw std::runtime_error("shadow render pass is null");

    DestroyDirectionalPipeline();

    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions    = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_FRONT_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_TRUE;
    rasterizer.depthBiasConstantFactor = 2.5f;
    rasterizer.depthBiasSlopeFactor    = 10.0f;
    rasterizer.depthBiasClamp          = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable  = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable     = VK_FALSE;

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates    = dynamicStates;

    VkPushConstantRange pushConstant {};
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstant.offset     = 0;
    pushConstant.size       = sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pushConstant;

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr,
                               &directionalPipelineLayout_) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow pipeline layout");

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable   = VK_FALSE;
    colorBlending.attachmentCount = 0;
    colorBlending.pAttachments    = nullptr;

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = static_cast<uint32_t>(shader->GetStages()->size());
    pipelineInfo.pStages             = shader->GetStages()->data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = &depthStencil;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;
    pipelineInfo.layout              = directionalPipelineLayout_;
    pipelineInfo.renderPass          = renderPass;
    pipelineInfo.subpass             = 0;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &directionalPipeline_) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow graphics pipeline");
}

VkCommandBuffer ShadowRenderer::beginOneTimeCommand() const
{
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = commandPool_;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device_, &allocInfo, &cmd) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate shadow command buffer");

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin shadow command buffer");

    return cmd;
}

void ShadowRenderer::endOneTimeCommand(VkCommandBuffer cmd) const
{
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        throw std::runtime_error("failed to end shadow command buffer");

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = 0;

    VkFence submitFence = VK_NULL_HANDLE;
    if (vkCreateFence(device_, &fenceInfo, nullptr, &submitFence) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow submit fence");

    VkSubmitInfo submitInfo {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmd;

    if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, submitFence) != VK_SUCCESS)
        {
            vkDestroyFence(device_, submitFence, nullptr);
            throw std::runtime_error("failed to submit shadow command buffer");
        }
    if (vkWaitForFences(device_, 1, &submitFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
        {
            vkDestroyFence(device_, submitFence, nullptr);
            throw std::runtime_error("failed waiting for shadow submit fence");
        }
    vkDestroyFence(device_, submitFence, nullptr);

    vkFreeCommandBuffers(device_, commandPool_, 1, &cmd);
}

void ShadowRenderer::FreeCommandBuffer(VkCommandBuffer cmd) const
{
    if (cmd == VK_NULL_HANDLE)
        return;
    vkFreeCommandBuffers(device_, commandPool_, 1, &cmd);
}

VkCommandBuffer ShadowRenderer::BuildShadowCommandBufferAll(
    const std::list<std::shared_ptr<Mesh> >& meshes, const ShadowPass& shadowPass,
    ShadowMapArray& directionalShadowMaps, ShadowMapArray& pointShadowMaps,
    const UBOs::ShadowPack& shadowPack, uint32_t frameIndex)
{
    if (directionalPipeline_ == VK_NULL_HANDLE)
        return VK_NULL_HANDLE;

    const bool hasDirectional = shadowPack.directional_.counts_.x > 0;
    const bool hasPoint       = shadowPack.point_.counts_.x > 0;
    if (!hasDirectional && !hasPoint)
        return VK_NULL_HANDLE;

    VkCommandBuffer cmd = beginOneTimeCommand();
    if (hasDirectional)
        {
            recordDepthLayoutTransition(
                cmd, directionalShadowMaps.image_, directionalShadowMaps.format_,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                directionalShadowMaps.layers_);
        }
    if (hasPoint)
        {
            recordDepthLayoutTransition(
                cmd, pointShadowMaps.image_, pointShadowMaps.format_,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                pointShadowMaps.layers_);
        }

    if (hasDirectional)
        {
            VkViewport viewport {};
            viewport.width    = static_cast<float>(directionalShadowMaps.width_);
            viewport.height   = static_cast<float>(directionalShadowMaps.height_);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor {};
            scissor.extent = {directionalShadowMaps.width_, directionalShadowMaps.height_};

            const auto& framebuffers = shadowPass.GetDirectionalFramebuffers();
            for (int idx = 0; idx < shadowPack.directional_.counts_.x; ++idx)
                {
                    const auto& entry =
                        shadowPack.directional_.entries_[static_cast<std::size_t>(idx)];
                    if (entry.meta_.z == 0 || entry.meta_.x < 0)
                        continue;

                    const uint32_t shadowId = static_cast<uint32_t>(entry.meta_.x);
                    if (shadowId >= framebuffers.size())
                        continue;

                    VkClearValue clearValue {};
                    clearValue.depthStencil = {1.0f, 0};

                    VkRenderPassBeginInfo rpInfo {};
                    rpInfo.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    rpInfo.renderPass = shadowPass.GetRenderPass();
                    rpInfo.framebuffer = framebuffers[shadowId];
                    rpInfo.renderArea.extent = {directionalShadowMaps.width_,
                                                directionalShadowMaps.height_};
                    rpInfo.clearValueCount = 1;
                    rpInfo.pClearValues    = &clearValue;

                    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      directionalPipeline_);
                    vkCmdSetViewport(cmd, 0, 1, &viewport);
                    vkCmdSetScissor(cmd, 0, 1, &scissor);

                    VkDeviceSize offsets[] = {0};
                    for (auto& mesh : meshes)
                        {
                            if (!mesh || !mesh->tr_ || mesh->tr_->modelCache_.empty())
                                continue;

                            const std::size_t modelIdx = std::min<std::size_t>(
                                frameIndex, mesh->tr_->modelCache_.size() - 1);
                            const glm::mat4 lightMvp =
                                entry.lightSpace_ * mesh->tr_->modelCache_[modelIdx];

                            vkCmdPushConstants(cmd, directionalPipelineLayout_,
                                               VK_SHADER_STAGE_VERTEX_BIT, 0,
                                               sizeof(glm::mat4), &lightMvp);
                            vkCmdBindVertexBuffers(
                                cmd, 0, 1, &mesh->vertBuffer_->pVertBuf_->buffer_,
                                offsets);
                            vkCmdBindIndexBuffer(cmd, mesh->indexBuffer_->buffer_, 0,
                                                 VK_INDEX_TYPE_UINT32);
                            vkCmdDrawIndexed(cmd, mesh->indexesSize_, 1, 0, 0, 0);
                        }

                    vkCmdEndRenderPass(cmd);
                }
        }

    if (hasPoint)
        {
            VkViewport viewport {};
            viewport.width    = static_cast<float>(pointShadowMaps.width_);
            viewport.height   = static_cast<float>(pointShadowMaps.height_);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor {};
            scissor.extent = {pointShadowMaps.width_, pointShadowMaps.height_};

            const auto& framebuffers = shadowPass.GetPointFramebuffers();
            for (int idx = 0; idx < shadowPack.point_.counts_.x; ++idx)
                {
                    const auto& entry =
                        shadowPack.point_.entries_[static_cast<std::size_t>(idx)];
                    if (entry.meta_.z == 0 || entry.meta_.x < 0)
                        continue;

                    const uint32_t shadowId = static_cast<uint32_t>(entry.meta_.x);
                    for (uint32_t face = 0; face < 6; ++face)
                        {
                            const uint32_t layerIndex = shadowId * 6u + face;
                            if (layerIndex >= framebuffers.size())
                                continue;

                            VkClearValue clearValue {};
                            clearValue.depthStencil = {1.0f, 0};

                            VkRenderPassBeginInfo rpInfo {};
                            rpInfo.sType      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                            rpInfo.renderPass = shadowPass.GetRenderPass();
                            rpInfo.framebuffer = framebuffers[layerIndex];
                            rpInfo.renderArea.extent = {pointShadowMaps.width_,
                                                        pointShadowMaps.height_};
                            rpInfo.clearValueCount = 1;
                            rpInfo.pClearValues    = &clearValue;

                            vkCmdBeginRenderPass(cmd, &rpInfo,
                                                 VK_SUBPASS_CONTENTS_INLINE);
                            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                              directionalPipeline_);
                            vkCmdSetViewport(cmd, 0, 1, &viewport);
                            vkCmdSetScissor(cmd, 0, 1, &scissor);

                            VkDeviceSize offsets[] = {0};
                            for (auto& mesh : meshes)
                                {
                                    if (!mesh || !mesh->tr_ ||
                                        mesh->tr_->modelCache_.empty())
                                        continue;

                                    const std::size_t modelIdx = std::min<std::size_t>(
                                        frameIndex,
                                        mesh->tr_->modelCache_.size() - 1);
                                    const glm::mat4 lightMvp =
                                        entry.shadowMatrices_[face] *
                                        mesh->tr_->modelCache_[modelIdx];

                                    vkCmdPushConstants(
                                        cmd, directionalPipelineLayout_,
                                        VK_SHADER_STAGE_VERTEX_BIT, 0,
                                        sizeof(glm::mat4), &lightMvp);
                                    vkCmdBindVertexBuffers(
                                        cmd, 0, 1,
                                        &mesh->vertBuffer_->pVertBuf_->buffer_,
                                        offsets);
                                    vkCmdBindIndexBuffer(
                                        cmd, mesh->indexBuffer_->buffer_, 0,
                                        VK_INDEX_TYPE_UINT32);
                                    vkCmdDrawIndexed(cmd, mesh->indexesSize_, 1, 0,
                                                     0, 0);
                                }

                            vkCmdEndRenderPass(cmd);
                        }
                }
        }

    if (hasDirectional)
        {
            recordDepthLayoutTransition(
                cmd, directionalShadowMaps.image_, directionalShadowMaps.format_,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                directionalShadowMaps.layers_);
        }
    if (hasPoint)
        {
            recordDepthLayoutTransition(
                cmd, pointShadowMaps.image_, pointShadowMaps.format_,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                pointShadowMaps.layers_);
        }

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        {
            vkFreeCommandBuffers(device_, commandPool_, 1, &cmd);
            throw std::runtime_error("failed to end shadow command buffer");
        }

    return cmd;
}

void ShadowRenderer::DrawAll(const std::list<std::shared_ptr<Mesh> >& meshes,
                             const ShadowPass& shadowPass,
                             ShadowMapArray& directionalShadowMaps,
                             ShadowMapArray& pointShadowMaps,
                             const UBOs::ShadowPack& shadowPack,
                             uint32_t frameIndex)
{
    VkCommandBuffer cmd = BuildShadowCommandBufferAll(
        meshes, shadowPass, directionalShadowMaps, pointShadowMaps, shadowPack,
        frameIndex);
    if (cmd == VK_NULL_HANDLE)
        return;
    endOneTimeCommand(cmd);
}

void ShadowRenderer::DrawDirectional(const std::list<std::shared_ptr<Mesh> >& meshes,
                                     const ShadowPass& shadowPass,
                                     ShadowMapArray& directionalShadowMaps,
                                     const UBOs::ShadowPack& shadowPack,
                                     uint32_t frameIndex)
{
    if (directionalPipeline_ == VK_NULL_HANDLE)
        return;
    if (shadowPack.directional_.counts_.x <= 0)
        return;

    executer_->TransitionImageLayoutLayers(
        directionalShadowMaps.image_, directionalShadowMaps.format_,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0,
        directionalShadowMaps.layers_);

    VkCommandBuffer cmd = beginOneTimeCommand();

    VkViewport viewport {};
    viewport.width    = static_cast<float>(directionalShadowMaps.width_);
    viewport.height   = static_cast<float>(directionalShadowMaps.height_);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.extent = {directionalShadowMaps.width_, directionalShadowMaps.height_};

    const auto& framebuffers = shadowPass.GetDirectionalFramebuffers();
    for (int idx = 0; idx < shadowPack.directional_.counts_.x; ++idx)
        {
            const auto& entry = shadowPack.directional_.entries_[static_cast<std::size_t>(idx)];
            if (entry.meta_.z == 0 || entry.meta_.x < 0)
                continue;

            const uint32_t shadowId = static_cast<uint32_t>(entry.meta_.x);
            if (shadowId >= framebuffers.size())
                continue;

            VkClearValue clearValue {};
            clearValue.depthStencil = {1.0f, 0};

            VkRenderPassBeginInfo rpInfo {};
            rpInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpInfo.renderPass        = shadowPass.GetRenderPass();
            rpInfo.framebuffer       = framebuffers[shadowId];
            rpInfo.renderArea.extent = {directionalShadowMaps.width_, directionalShadowMaps.height_};
            rpInfo.clearValueCount   = 1;
            rpInfo.pClearValues      = &clearValue;

            vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, directionalPipeline_);
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            VkDeviceSize offsets[] = {0};
            for (auto& mesh : meshes)
                {
                    if (!mesh || !mesh->tr_ || mesh->tr_->modelCache_.empty())
                        continue;

                    const std::size_t modelIdx = std::min<std::size_t>(
                        frameIndex, mesh->tr_->modelCache_.size() - 1);
                    const glm::mat4 lightMvp =
                        entry.lightSpace_ * mesh->tr_->modelCache_[modelIdx];

                    vkCmdPushConstants(cmd, directionalPipelineLayout_,
                                       VK_SHADER_STAGE_VERTEX_BIT, 0,
                                       sizeof(glm::mat4), &lightMvp);
                    vkCmdBindVertexBuffers(cmd, 0, 1,
                                           &mesh->vertBuffer_->pVertBuf_->buffer_, offsets);
                    vkCmdBindIndexBuffer(cmd, mesh->indexBuffer_->buffer_, 0,
                                         VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed(cmd, mesh->indexesSize_, 1, 0, 0, 0);
                }

            vkCmdEndRenderPass(cmd);
        }

    endOneTimeCommand(cmd);

    executer_->TransitionImageLayoutLayers(
        directionalShadowMaps.image_, directionalShadowMaps.format_,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, directionalShadowMaps.layers_);
}

void ShadowRenderer::DrawPoint(const std::list<std::shared_ptr<Mesh> >& meshes,
                               const ShadowPass& shadowPass, ShadowMapArray& pointShadowMaps,
                               const UBOs::ShadowPack& shadowPack, uint32_t frameIndex)
{
    if (directionalPipeline_ == VK_NULL_HANDLE)
        return;
    if (shadowPack.point_.counts_.x <= 0)
        return;

    executer_->TransitionImageLayoutLayers(
        pointShadowMaps.image_, pointShadowMaps.format_,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, pointShadowMaps.layers_);

    VkCommandBuffer cmd = beginOneTimeCommand();

    VkViewport viewport {};
    viewport.width    = static_cast<float>(pointShadowMaps.width_);
    viewport.height   = static_cast<float>(pointShadowMaps.height_);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.extent = {pointShadowMaps.width_, pointShadowMaps.height_};

    const auto& framebuffers = shadowPass.GetPointFramebuffers();
    for (int idx = 0; idx < shadowPack.point_.counts_.x; ++idx)
        {
            const auto& entry = shadowPack.point_.entries_[static_cast<std::size_t>(idx)];
            if (entry.meta_.z == 0 || entry.meta_.x < 0)
                continue;

            const uint32_t shadowId = static_cast<uint32_t>(entry.meta_.x);
            for (uint32_t face = 0; face < 6; ++face)
                {
                    const uint32_t layerIndex = shadowId * 6u + face;
                    if (layerIndex >= framebuffers.size())
                        continue;

                    VkClearValue clearValue {};
                    clearValue.depthStencil = {1.0f, 0};

                    VkRenderPassBeginInfo rpInfo {};
                    rpInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    rpInfo.renderPass        = shadowPass.GetRenderPass();
                    rpInfo.framebuffer       = framebuffers[layerIndex];
                    rpInfo.renderArea.extent = {pointShadowMaps.width_, pointShadowMaps.height_};
                    rpInfo.clearValueCount   = 1;
                    rpInfo.pClearValues      = &clearValue;

                    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, directionalPipeline_);
                    vkCmdSetViewport(cmd, 0, 1, &viewport);
                    vkCmdSetScissor(cmd, 0, 1, &scissor);

                    VkDeviceSize offsets[] = {0};
                    for (auto& mesh : meshes)
                        {
                            if (!mesh || !mesh->tr_ || mesh->tr_->modelCache_.empty())
                                continue;

                            const std::size_t modelIdx = std::min<std::size_t>(
                                frameIndex, mesh->tr_->modelCache_.size() - 1);
                            const glm::mat4 lightMvp =
                                entry.shadowMatrices_[face] *
                                mesh->tr_->modelCache_[modelIdx];

                            vkCmdPushConstants(cmd, directionalPipelineLayout_,
                                               VK_SHADER_STAGE_VERTEX_BIT, 0,
                                               sizeof(glm::mat4), &lightMvp);
                            vkCmdBindVertexBuffers(cmd, 0, 1,
                                                   &mesh->vertBuffer_->pVertBuf_->buffer_, offsets);
                            vkCmdBindIndexBuffer(cmd, mesh->indexBuffer_->buffer_, 0,
                                                 VK_INDEX_TYPE_UINT32);
                            vkCmdDrawIndexed(cmd, mesh->indexesSize_, 1, 0, 0, 0);
                        }

                    vkCmdEndRenderPass(cmd);
                }
        }

    endOneTimeCommand(cmd);

    executer_->TransitionImageLayoutLayers(
        pointShadowMaps.image_, pointShadowMaps.format_,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, pointShadowMaps.layers_);
}

} // namespace Multor::Vulkan
