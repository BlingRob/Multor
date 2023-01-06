/// \file VkFrameChain.h
#pragma once
#include <vulkan/vulkan.h>
#include "VkGeneralOptions.h"
#include <algorithm>
#include <vector>
#include <array>
#include "Mesh.h"

class FrameChain:public VkBaseStructs
{
public:
	FrameChain(std::shared_ptr<Window> wnd):VkBaseStructs(wnd)
	{
		_executer = std::make_shared<CommandExecuter>(device, commandPool, graphicsQueue);
		MeshFac = std::make_unique<VkMeshFactory>(device, physicDev, _executer);
		createSwapChain();
		createImageViews();
		createRenderPass();
		DepthImg = MeshFac->createDepthTexture(swapChainExtent.width, swapChainExtent.height);
		createFramebuffers();
	}
	~FrameChain() 
	{
		MeshFac.reset();
		_executer.reset();
		cleanUpSwapChain();
	}
	void cleanUpSwapChain();
	void recreateSwapChain();
protected:

	VkRenderPass renderPass;
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	std::unique_ptr<VkTexture> DepthImg;
	
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	std::shared_ptr<CommandExecuter> _executer;
	std::unique_ptr<VkMeshFactory> MeshFac;

	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createFramebuffers();
};