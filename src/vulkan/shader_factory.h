/// \file shader_factory.h.h

#pragma once

#include <vector>
#include <memory>
#include <stdexcept>

#include "shader.h"

namespace Multor::Vulkan
{

class ShaderFactory
{
public:
    ShaderFactory(VkDevice& device);
    std::shared_ptr<ShaderLayout> CreateShader(std::string_view vertex,
                                               std::string_view fragment,
                                               std::string_view geometry = "");
    ~ShaderFactory();

private:
    VkDevice         device_;
    TBuiltInResource glslcResourceLimits_;

    std::unique_ptr<glslang::TShader> createShader(std::string_view,
                                                   EShLanguage type);
    std::unique_ptr<glslang::TProgram>
                              createProgram(std::unique_ptr<glslang::TShader>);
    std::vector<unsigned int> getSPIRV(const glslang::TIntermediate* intr);
    VkShaderModule createModule(const std::vector<unsigned int>& spirv);

    std::vector<VkShaderModule>                 createdModules_;
    std::vector<std::shared_ptr<ShaderLayout> > createdVkShaders_;

    void InitResource();
};

} // namespace Multor::Vulkan
