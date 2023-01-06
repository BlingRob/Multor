/// \file VulkanRenderer.h
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <optional>
#include <fstream>
#include <algorithm>
#include <array>
#include <list>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "ImageLoader.h"
#include "Shader.h"
#include "VkFrameChain.h"
#include "VkTexture.h"
#include "FilesTools.h"
#include "VkSync.h"

class VulkanRenderer:public FrameChain
{
public:
	VulkanRenderer(std::shared_ptr<Window> wnd):FrameChain(wnd)
	{
		ShFactory = std::make_unique<ShaderFactory>(device);
		_shaders.push_back(ShFactory->createShader(LoadTextFile("../../shaders/Base.vs"), LoadTextFile("../../shaders/Base.frag")));

		createDescriptorPool();
		createDescriptorSets();
		createDescriptorSetLayout();

		//Include meshes
		//createUniformBuffers();
		//createTextureImage();
		//createTextureImageView();
		//createTextureSampler();
		//createVertexBuffer();
		//createIndexBuffer();
		//Update layout

		createGraphicsPipeline();

		createCommandBuffers();
		createUniformBuffers();
		createSyncObjects();
	}

	~VulkanRenderer();


	std::shared_ptr<VkMesh> AddMesh(BaseMesh* mesh);

	void Draw();
	void Update();

	size_t getCurFrame() { return imageIndex; };

private:
	const int MAX_FRAMES_IN_FLIGHT = 3;
	size_t currentFrame = 0;
	uint32_t imageIndex;

	std::unique_ptr<ShaderFactory> ShFactory;
	std::vector<std::shared_ptr<ShaderLayout>> _shaders;

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSyncer> syncers;

	std::list<std::shared_ptr<VkMesh>> _meshes;

	void createGraphicsPipeline();
	void createCommandBuffers();
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	void createUniformBuffers();
	void createSyncObjects();

	bool hasStencilComponent(VkFormat format);

	void ClearInlcudePart();

	void UpdateMats(uint32_t currentImage);
};