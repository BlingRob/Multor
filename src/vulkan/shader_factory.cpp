/// \file shader_factory.cpp

#include "shader_factory.h"

namespace Multor::Vulkan
{

ShaderFactory::ShaderFactory(VkDevice& device)
{
    //CreatedModules.reserve(10);
    device_ = device;
    InitResource();
    glslang::InitializeProcess();
}

ShaderFactory::~ShaderFactory()
{
    glslang::FinalizeProcess();
    for (auto& modul : createdModules_)
        vkDestroyShaderModule(device_, modul, nullptr);
}

void ShaderFactory::InitResource()
{
    glslcResourceLimits_ = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
        /* .maxMeshOutputVerticesNV = */ 256,
        /* .maxMeshOutputPrimitivesNV = */ 512,
        /* .maxMeshWorkGroupSizeX_NV = */ 32,
        /* .maxMeshWorkGroupSizeY_NV = */ 1,
        /* .maxMeshWorkGroupSizeZ_NV = */ 1,
        /* .maxTaskWorkGroupSizeX_NV = */ 32,
        /* .maxTaskWorkGroupSizeY_NV = */ 1,
        /* .maxTaskWorkGroupSizeZ_NV = */ 1,
        /* .maxMeshViewCountNV = */ 4,
        /* .maxDualSourceDrawBuffersEXT = */ 1};

    glslcResourceLimits_.limits = /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    };
}

/*VkShaderModule ShaderFactory::CreateModule(std::string_view Source, EShLanguage stage)
{
    if (Source.empty())
        throw(std::string("ERROR::SHADER THERE AREN'T SHADERS\n"));

    std::unique_ptr<glslang::TShader> shader;
    std::unique_ptr<glslang::TProgram> program;
    EShMessages infoMsg = EShMessages::EShMsgDebugInfo;

    shader = createShader(Source, stage);

    //Create glslProgramm
    program = createProgram(std::move(shader));
    std::vector<unsigned int> spirv = getSPIRV(program->getIntermediate(stage));
    //const char* ch = program->getAttributeName(1);
    //program->getIntermediate(stage)->getEntryPointName().c_str();

    //Create VkShaderModul
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.codeSize = spirv.size() * sizeof(unsigned int);
    createInfo.pCode = spirv.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");

    CreatedModules.push_back(shaderModule);

    return CreatedModules.back();
}*/

std::unique_ptr<glslang::TShader>
ShaderFactory::createShader(std::string_view source, EShLanguage type)
{
    EShMessages                       infoMsg = EShMessages::EShMsgDebugInfo;
    std::unique_ptr<glslang::TShader> shader;
    //Set shader parametrs
    shader          = std::make_unique<glslang::TShader>(type);
    const char* str = source.data();
    shader->setStrings(&str, 1);
    shader->setEnvInput(glslang::EShSourceGlsl, type, glslang::EShClientVulkan,
                        100);
    shader->setEnvClient(glslang::EShClientVulkan,
                         glslang::EShTargetVulkan_1_0);
    shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
    shader->parse(&glslcResourceLimits_, 450, false, infoMsg);
    perror(shader->getInfoLog());

    return shader;
}

std::unique_ptr<glslang::TProgram>
ShaderFactory::createProgram(std::unique_ptr<glslang::TShader> shader)
{
    std::unique_ptr<glslang::TProgram> program;
    EShMessages                        infoMsg = EShMessages::EShMsgDebugInfo;
    //Create glslProgramm
    program = std::make_unique<glslang::TProgram>();
    program->addShader(shader.release());
    program->link(infoMsg);
    program->buildReflection();
    perror(program->getInfoLog());

    return program;
}

std::vector<unsigned int>
ShaderFactory::getSPIRV(const glslang::TIntermediate* intr)
{
    //GLSL -> SPIR-V
    spv::SpvBuildLogger logger;
    glslang::SpvOptions spvOptions;
    spvOptions.validate = true;
    std::vector<unsigned int> spirv;

    glslang::GlslangToSpv(*intr, spirv, &logger, &spvOptions);

    return spirv;
}

VkShaderModule
ShaderFactory::createModule(const std::vector<unsigned int>& spirv)
{
    //Create VkShaderModul
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext    = nullptr;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode    = spirv.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");

    return shaderModule;
}

std::shared_ptr<ShaderLayout>
ShaderFactory::CreateShader(std::string_view vertex, std::string_view fragment,
                            std::string_view geometry)
{
    std::shared_ptr<ShaderLayout> _pSh = std::make_shared<ShaderLayout>();
    //if (Source.empty())
    //   throw(std::string("ERROR::SHADER THERE AREN'T SHADERS\n"));

    std::unique_ptr<glslang::TShader>  shader;
    std::unique_ptr<glslang::TProgram> program;
    glslang::TIntermediate*            inter;

    if (!vertex.empty())
        {
            shader  = createShader(vertex, EShLanguage::EShLangVertex);
            program = createProgram(std::move(shader));
            inter   = program->getIntermediate(EShLanguage::EShLangVertex);
            createdModules_.push_back(createModule(getSPIRV(inter)));
            _pSh->AddShaderModule(createdModules_.back(), shader_type::vertex,
                                  std::move(program));
        }

    if (!fragment.empty())
        {
            shader  = createShader(fragment, EShLanguage::EShLangFragment);
            program = createProgram(std::move(shader));
            inter   = program->getIntermediate(EShLanguage::EShLangFragment);
            createdModules_.push_back(createModule(getSPIRV(inter)));
            _pSh->AddShaderModule(createdModules_.back(), shader_type::fragment,
                                  std::move(program));
        }

    if (!geometry.empty())
        {
            shader  = createShader(geometry, EShLanguage::EShLangGeometry);
            program = createProgram(std::move(shader));
            inter   = program->getIntermediate(EShLanguage::EShLangGeometry);
            createdModules_.push_back(createModule(getSPIRV(inter)));
            _pSh->AddShaderModule(createdModules_.back(), shader_type::geometry,
                                  std::move(program));
        }

    createdVkShaders_.push_back(_pSh);

    return createdVkShaders_.back();
}

} // namespace Multor::Vulkan
