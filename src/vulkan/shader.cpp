/// \file shader.cpp
#include "shader.h"

#include <stdexcept>

namespace Multor::Vulkan
{

ShaderLayout::ShaderLayout(){}

void ShaderLayout::AddShaderModule(VkShaderModule modul, shader_type type,
                                   std::unique_ptr<glslang::TProgram> program)
{
    const auto stageFlags = static_cast<unsigned int>(
        ShaderConverter::convert<VkShaderStageFlagBits>(type));
    auto addOrMergeLayoutBinding =
        [this](VkDescriptorSetLayoutBinding binding)
    {
        for (auto& existing : layouts_)
            {
                if (existing.binding == binding.binding)
                    {
                        if (existing.descriptorType != binding.descriptorType ||
                            existing.descriptorCount != binding.descriptorCount)
                            {
                                // glslang public reflection (without full TType access)
                                // may report non-sampler uniforms in getNumUniformVariables().
                                // Keep the first binding (typically UBO) and skip the conflicting one.
                                return;
                            }
                        existing.stageFlags |= binding.stageFlags;
                        return;
                    }
            }
        layouts_.push_back(binding);
    };

    addPipShStInfo(modul, ShaderConverter::convert<VkShaderStageFlagBits>(type),
                   "main");

    for (std::size_t i{0}; i < program->getNumUniformBlocks(); ++i)
        {
            addOrMergeLayoutBinding(VkDescriptorSetLayoutBinding {
                static_cast<std::uint32_t>(program->getUniformBlock(static_cast<std::int32_t>(i)).getBinding()),
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                stageFlags,
                nullptr});
        }

    for (std::size_t i{0}; i < program->getNumUniformVariables(); ++i)
        {
            const int binding =
                program->getUniform(static_cast<std::int32_t>(i)).getBinding();
            if (binding < 0)
                continue;
            addOrMergeLayoutBinding(VkDescriptorSetLayoutBinding {
                static_cast<std::uint32_t>(binding),
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                stageFlags,
                nullptr});
        }

    prg_ = program.release();
}

void ShaderLayout::addPipShStInfo(VkShaderModule        modul,
                                  VkShaderStageFlagBits stage,
                                  const char*           entryPointName)
{
    VkPipelineShaderStageCreateInfo ShaderStageInfo;

    ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ShaderStageInfo.pNext = nullptr;
    ShaderStageInfo.flags = 0;
    ShaderStageInfo.stage               = stage;
    ShaderStageInfo.module              = modul;
    ShaderStageInfo.pName               = entryPointName;
    ShaderStageInfo.pSpecializationInfo = nullptr;

    units_.push_back(ShaderStageInfo);
}

const std::vector<VkPipelineShaderStageCreateInfo>* ShaderLayout::GetStages()
{
    return &units_;
}

const std::vector<VkDescriptorSetLayoutBinding>*
ShaderLayout::GetLayoutBindings()
{
    return &layouts_;
}

} // namespace Multor::Vulkan
