/// \file VkShaderFactory.h

#pragma once
#ifndef VKSHADERFACTORY_H
#define VKSHADERFACTORY_H

#include <vector>
#include <memory>
#include <stdexcept>

#include "VkShader.h"

namespace Multor
{

class ShaderFactory
{
public:
	ShaderFactory(VkDevice& device);
	std::shared_ptr<ShaderLayout> createShader(std::string_view vertex, std::string_view fragment, std::string_view geometry = "");
	~ShaderFactory();

private:
	VkDevice _device;
	TBuiltInResource glslc_resource_limits;

	std::unique_ptr<glslang::TShader>  createShader(std::string_view, EShLanguage type);
	std::unique_ptr<glslang::TProgram> createProgram(std::unique_ptr<glslang::TShader>);
	std::vector<unsigned int>		   getSPIRV(const glslang::TIntermediate* intr);
	VkShaderModule					   createModule(const std::vector<unsigned int>& spirv);

	std::vector<VkShaderModule> CreatedModules;
	std::vector<std::shared_ptr<ShaderLayout>> createdVkShaders;

	void InitResource();
};

} // namespace Multor

#endif // VKSHADERFACTORY_H