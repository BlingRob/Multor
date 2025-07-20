/// \file shader.h

#pragma once

#include <vector>
#include <memory>
#include <set>
#include <tuple>

#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

const int32_t NSupportedShaderTypes = 3;

enum shader_type : int32_t
{
    vertex = 0,
    fragment,
    geometry
};

class ShaderConverter
{
public:
    template <typename retT, typename inT>
    static retT convert(inT in)
    {
        int32_t typeNum = typeToNum<inT>::value;
        for (size_t i = 0; i < NSupportedShaderTypes; ++i)
            if (typeMatr[typeNum][i] == in)
                return static_cast<retT>(typeMatr[typeToNum<retT>::value][i]);
        return static_cast<retT>(-1);
    }

private:
    template <typename type>
    struct typeToNum
    {
        static inline int32_t value =
            (std::is_same_v<type, EShLanguage>)             ? 0
            : (std::is_same_v<type, VkShaderStageFlagBits>) ? 1
            : (std::is_same_v<type, shader_type>)           ? 2
                                                            : -1;
    };

    static inline const int32_t
        typeMatr[NSupportedShaderTypes][NSupportedShaderTypes] = {
            {EShLanguage::EShLangVertex, EShLanguage::EShLangFragment,
             EShLanguage::EShLangGeometry},
            {VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
             VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
             VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT},
            {shader_type::vertex, shader_type::fragment,
             shader_type::geometry}};
};

class ShaderLayout
{
public:
    ShaderLayout();

    void addShaderModule(VkShaderModule modul, shader_type type,
                         std::unique_ptr<glslang::TProgram> programm);

    const std::vector<VkPipelineShaderStageCreateInfo>* getStages();
    const std::vector<VkDescriptorSetLayoutBinding>*    getLayoutBindings();

private:
    glslang::TProgram* prg;

    void addPipShStInfo(VkShaderModule modul, VkShaderStageFlagBits stage,
                        const char* entryPointName);
    //getShaderVariables();

    std::vector<VkPipelineShaderStageCreateInfo> units;
    std::vector<VkDescriptorSetLayoutBinding>    Layouts;
};

class Shader
{
public:
    Shader(std::shared_ptr<ShaderLayout> layout) : m_layout(layout)
    {
    }
    std::vector<VkDescriptorSet> DesSet;

private:
    const std::shared_ptr<ShaderLayout>             m_layout;
    std::set<std::pair<uint32_t, VkDescriptorSet> > m_bind_VMemP;
};

} // namespace Multor::Vulkan
