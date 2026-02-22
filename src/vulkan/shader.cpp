/// \file VkShader.cpp
#include "shader.h"

// #include <glslang/MachineIndependent/localintermediate.h>
#include <D:/programming/c++/Multor/build/dependencies/VulkanSDK/glslang/glslang/MachineIndependent/localintermediate.h>


namespace Multor::Vulkan
{

ShaderLayout::ShaderLayout(){}

void ShaderLayout::AddShaderModule(VkShaderModule modul, shader_type type,
                                   std::unique_ptr<glslang::TProgram> program)
{
    glslang::TIntermediate* inter =
        program->getIntermediate(ShaderConverter::convert<EShLanguage>(type));

    addPipShStInfo(modul, ShaderConverter::convert<VkShaderStageFlagBits>(type),
                   inter->getEntryPointName().c_str());

    /*int var1 = program->getNumUniformBlocks();
    int var2 = program->getNumUniformVariables();
    const char* name = program->getUniformBlock(0).name.c_str();
    const char* name1 = program->getUniform(0).name.c_str();
    int inn = program->getUniform(0).offset;
    bool t = program->getUniform(0).getType()->isMatrix();
    int name2 = program->getPipeInput(0).badReflection().getBinding();*/

    //const char* name1 = program->getUniform(0).getType()->getTypeName().c_str();

    for (size_t i = 0; i < program->getNumUniformBlocks(); ++i)
        {
            layouts_.push_back(VkDescriptorSetLayoutBinding {
                static_cast<uint32_t>(program->getUniformBlock(i).getBinding()),
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                static_cast<unsigned int>(
                    ShaderConverter::convert<VkShaderStageFlagBits>(type)),
                nullptr});
        }

    for (size_t i = 0; i < program->getNumUniformVariables(); ++i)
        {
            if (program->getUniform(i).getType()->isTexture())
                {
                    layouts_.push_back(VkDescriptorSetLayoutBinding {
                        static_cast<uint32_t>(
                            program->getUniform(i).getBinding()),
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                        static_cast<unsigned int>(
                            ShaderConverter::convert<VkShaderStageFlagBits>(
                                type)),
                        nullptr});
                }
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
