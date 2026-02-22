/// \file imgui_overlay.cpp

#include "imgui_overlay.h"

#include "../vulkan/renderer.h"
#include "window.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <array>
#include <algorithm>
#include <stdexcept>

namespace Multor
{

namespace
{
} // namespace

ImGuiOverlay::ImGuiOverlay()
{
    backendAvailable_ = true;
    backendStatus_ = "ImGui Vulkan backend enabled";
    lastFrameTime_ = std::chrono::steady_clock::now();
}

ImGuiOverlay::~ImGuiOverlay()
{
    ShutdownBackend();
}

void ImGuiOverlay::AttachWindow(const Window* window)
{
    window_ = window;
}

void ImGuiOverlay::AttachRenderer(const std::shared_ptr<Vulkan::Renderer>& renderer)
{
    renderer_ = renderer;
    if (auto r = renderer_.lock())
        {
            r->SetOverlayDrawCallback([this](VkCommandBuffer cmd)
            {
                if (!initialized_ || drawData_ == nullptr)
                    return;
                ImGui_ImplVulkan_RenderDrawData(
                    static_cast<ImDrawData*>(drawData_), cmd);
            });
        }
}

bool ImGuiOverlay::EnsureInitialized()
{
    if (initialized_)
        return true;
    if (!backendAvailable_)
        return false;

    auto renderer = renderer_.lock();
    if (!renderer || !window_)
        return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    std::array<VkDescriptorPoolSize, 11> poolSizes = {{
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
    }};

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets       = 1000 * static_cast<uint32_t>(poolSizes.size());
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    if (vkCreateDescriptorPool(renderer->GetVkDevice(), &poolInfo, nullptr,
                               &imguiDescriptorPool_) != VK_SUCCESS)
        {
            backendStatus_    = "Failed to create ImGui descriptor pool";
            backendAvailable_ = false;
            return false;
        }

    ImGui_ImplVulkan_InitInfo initInfo {};
    initInfo.Instance       = renderer->GetVkInstance();
    initInfo.PhysicalDevice = renderer->GetVkPhysicalDevice();
    initInfo.Device         = renderer->GetVkDevice();
    initInfo.QueueFamily    = renderer->GetVkGraphicsQueueFamilyIndex();
    initInfo.Queue          = renderer->GetVkGraphicsQueue();
    initInfo.PipelineCache  = VK_NULL_HANDLE;
    initInfo.DescriptorPool = imguiDescriptorPool_;
    initInfo.RenderPass     = renderer->GetVkRenderPass();
    initInfo.Subpass        = 0;
    initInfo.MinImageCount  = renderer->GetMinImageCount();
    initInfo.ImageCount     = renderer->GetSwapchainImageCount();
    initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
    initInfo.UseDynamicRendering = false;
    initInfo.Allocator      = nullptr;
    initInfo.CheckVkResultFn = nullptr;

    if (!ImGui_ImplVulkan_Init(&initInfo))
        {
            backendStatus_    = "ImGui_ImplVulkan_Init failed";
            backendAvailable_ = false;
            return false;
        }

    if (!ImGui_ImplVulkan_CreateFontsTexture())
        {
            backendStatus_    = "ImGui_ImplVulkan_CreateFontsTexture failed";
            backendAvailable_ = false;
            return false;
        }

    initialized_ = true;
    backendStatus_ = "ImGui Vulkan initialized";
    return true;
}

void ImGuiOverlay::ShutdownBackend()
{
    auto renderer = renderer_.lock();
    if (renderer)
        renderer->SetOverlayDrawCallback({});

    if (initialized_)
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui::DestroyContext();
            initialized_ = false;
        }

    if (renderer && imguiDescriptorPool_ != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(renderer->GetVkDevice(), imguiDescriptorPool_,
                                    nullptr);
        }
    imguiDescriptorPool_ = VK_NULL_HANDLE;
}

void ImGuiOverlay::OnSdlEvent(const SDL_Event& e)
{
    if (!initialized_)
        return;

    ImGuiIO& io = ImGui::GetIO();
    switch (e.type)
        {
        case SDL_EVENT_MOUSE_MOTION:
            io.AddMousePosEvent(static_cast<float>(e.motion.x),
                                static_cast<float>(e.motion.y));
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                const bool down = (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
                if (e.button.button == SDL_BUTTON_LEFT)
                    io.AddMouseButtonEvent(0, down);
                if (e.button.button == SDL_BUTTON_RIGHT)
                    io.AddMouseButtonEvent(1, down);
                if (e.button.button == SDL_BUTTON_MIDDLE)
                    io.AddMouseButtonEvent(2, down);
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            io.AddMouseWheelEvent(static_cast<float>(e.wheel.x),
                                  static_cast<float>(e.wheel.y));
            break;
        default:
            break;
        }
}

void ImGuiOverlay::NewFrame()
{
    if (!EnsureInitialized())
        return;

    if (window_)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(static_cast<float>(window_->scrWidth_),
                                    static_cast<float>(window_->scrHeight_));
            auto now = std::chrono::steady_clock::now();
            io.DeltaTime = std::max(
                1.0e-4f,
                std::chrono::duration<float>(now - lastFrameTime_).count());
            lastFrameTime_ = now;
        }

    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
}

void ImGuiOverlay::Draw(const std::shared_ptr<Scene>& scene,
                        const std::shared_ptr<PositionController>& controller,
                        const std::shared_ptr<Vulkan::Renderer>& renderer)
{
    if (!initialized_)
        return;

    if (ImGui::Begin("Stats"))
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            if (controller && controller->cam_)
                {
                    const auto& p = controller->cam_->position_;
                    ImGui::Text("Cam: %.2f %.2f %.2f", p.x, p.y, p.z);
                }
            if (scene)
                {
                    const auto info = scene->GetInfo();
                    ImGui::Text("Scene models: %zu", info.amountModels_);
                    ImGui::Text("Scene meshes: %zu", info.amountMeshes_);
                    ImGui::Text("Scene lights: %zu", info.amountLights_);
                    ImGui::Text("Scene nodes:  %zu", info.amountNodes_);
                }
            if (renderer)
                {
                    ImGui::Text("Frame idx: %zu", renderer->GetCurFrame());
                }
            ImGui::Separator();
            ImGui::TextWrapped("%s", backendStatus_.c_str());
        }
    ImGui::End();
}

void ImGuiOverlay::Render()
{
    if (!initialized_)
        return;
    ImGui::Render();
    drawData_ = ImGui::GetDrawData();
}

bool ImGuiOverlay::IsAvailable() const
{
    return backendAvailable_;
}

const std::string& ImGuiOverlay::BackendStatus() const
{
    return backendStatus_;
}

} // namespace Multor
