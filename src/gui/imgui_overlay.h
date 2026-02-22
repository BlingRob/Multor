/// \file imgui_overlay.h

#pragma once
#ifndef MULTOR_IMGUI_OVERLAY_H
#define MULTOR_IMGUI_OVERLAY_H

#include "../scene.h"
#include "../transformation.h"

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <chrono>
#include <functional>
#include <array>

namespace Multor
{

class Window;
namespace Vulkan
{
class Renderer;
}

class ImGuiOverlay
{
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    ImGuiOverlay(const ImGuiOverlay&)            = delete;
    ImGuiOverlay& operator=(const ImGuiOverlay&) = delete;

    void AttachWindow(const Window* window);
    void AttachRenderer(const std::shared_ptr<Vulkan::Renderer>& renderer);
    void SetOpenSceneCallback(std::function<bool(const std::string&)> callback);
    void OnSdlEvent(const SDL_Event& e);

    void NewFrame();
    void Draw(const std::shared_ptr<Scene>& scene,
              const std::shared_ptr<PositionController>& controller,
              const std::shared_ptr<Vulkan::Renderer>& renderer);
    void Render();

    bool IsAvailable() const;
    const std::string& BackendStatus() const;

private:
    bool EnsureInitialized();
    void ShutdownBackend();

    const Window* window_ = nullptr;
    std::weak_ptr<Vulkan::Renderer> renderer_;
    bool backendAvailable_ = false;
    bool initialized_ = false;
    std::string backendStatus_;
    void* drawData_ = nullptr;
    std::chrono::steady_clock::time_point lastFrameTime_ {};
    VkDescriptorPool imguiDescriptorPool_ = VK_NULL_HANDLE;
    bool showAxisGizmo_ = true;
    bool showStatsWindow_ = true;
    bool showCameraWindow_ = true;
    bool showLightsWindow_ = true;
    bool showDebugWindow_ = true;
    bool showOpenSceneWindow_ = false;
    std::function<bool(const std::string&)> openSceneCallback_;
    std::array<char, 512> openScenePath_ {};
    std::string openSceneStatus_;
};

} // namespace Multor

#endif // MULTOR_IMGUI_OVERLAY_H
