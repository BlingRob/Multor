/// \file imgui_overlay.cpp

#include "imgui_overlay.h"

#include "../vulkan/renderer.h"
#include "window.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <array>
#include <algorithm>
#include <cstdio>
#include <stdexcept>

namespace Multor
{

namespace
{
const char* LightTypeName(Multor::LightType type)
{
    switch (type)
        {
        case Multor::LightType::Directional:
            return "Directional";
        case Multor::LightType::Point:
            return "Point";
        case Multor::LightType::Spot:
            return "Spot";
        default:
            return "None";
        }
}

bool EditVec3(const char* label, glm::vec3& v, float speed = 0.05f)
{
    float vals[3] = {v.x, v.y, v.z};
    if (!ImGui::DragFloat3(label, vals, speed))
        return false;
    v = glm::vec3(vals[0], vals[1], vals[2]);
    return true;
}

void DrawAxisGizmo(const Multor::Camera& cam)
{
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const ImVec2 c(display.x - 70.0f, display.y - 70.0f);
    const float len = 28.0f;

    auto project = [&](const glm::vec3& dir) -> ImVec2
    {
        // Camera basis projected to screen-space gizmo (x right, y down in ImGui)
        return ImVec2(c.x + dir.x * len, c.y - dir.y * len);
    };

    const glm::vec3 xAxis = cam.right_;
    const glm::vec3 yAxis = cam.up_;
    const glm::vec3 zAxis = -cam.front_;

    dl->AddCircleFilled(c, 3.0f, IM_COL32(230, 230, 230, 220));
    const ImVec2 px = project(xAxis);
    const ImVec2 py = project(yAxis);
    const ImVec2 pz = project(zAxis);
    dl->AddLine(c, px, IM_COL32(220, 60, 60, 255), 2.0f);
    dl->AddLine(c, py, IM_COL32(80, 220, 80, 255), 2.0f);
    dl->AddLine(c, pz, IM_COL32(80, 140, 255, 255), 2.0f);
    dl->AddText(ImVec2(px.x + 4.0f, px.y - 6.0f), IM_COL32(220, 60, 60, 255), "X");
    dl->AddText(ImVec2(py.x + 4.0f, py.y - 6.0f), IM_COL32(80, 220, 80, 255), "Y");
    dl->AddText(ImVec2(pz.x + 4.0f, pz.y - 6.0f), IM_COL32(80, 140, 255, 255), "Z");
}
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

void ImGuiOverlay::SetOpenSceneCallback(std::function<bool(const std::string&)> callback)
{
    openSceneCallback_ = std::move(callback);
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

    bool cameraChanged = false;
    bool lightsChanged = false;

    if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
                        {
                            showOpenSceneWindow_ = true;
                            if (openScenePath_[0] == '\0')
                                std::snprintf(openScenePath_.data(), openScenePath_.size(),
                                              "%s", "../../Res/");
                        }
                    ImGui::EndMenu();
                }

            if (ImGui::BeginMenu("View"))
                {
                    ImGui::MenuItem("Stats", nullptr, &showStatsWindow_);
                    ImGui::MenuItem("Camera", nullptr, &showCameraWindow_);
                    ImGui::MenuItem("Lights", nullptr, &showLightsWindow_);
                    ImGui::MenuItem("Debug", nullptr, &showDebugWindow_);
                    ImGui::Separator();
                    ImGui::MenuItem("Axis Gizmo xOyOz", nullptr, &showAxisGizmo_);
                    ImGui::EndMenu();
                }

            if (ImGui::BeginMenu("Scene"))
                {
                    if (scene)
                        {
                            if (ImGui::MenuItem("Add Directional Light"))
                                {
                                    auto light = std::make_shared<DirectionalLight>(
                                        glm::vec3(0.2f), glm::vec3(0.9f),
                                        glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                                        glm::normalize(glm::vec3(-0.5f, -1.0f, -0.2f)));
                                    light->SetName("imgui_dir_light");
                                    scene->AddLight(light);
                                    if (renderer)
                                        renderer->AddLight(light);
                                    lightsChanged = true;
                                }
                            if (ImGui::MenuItem("Add Point Light"))
                                {
                                    auto light = std::make_shared<PointLight>(
                                        glm::vec3(0.1f), glm::vec3(1.0f), glm::vec3(1.0f),
                                        glm::vec3(1.0f, 0.09f, 0.032f),
                                        glm::vec3(0.0f, 2.0f, 0.0f));
                                    light->SetName("imgui_point_light");
                                    scene->AddLight(light);
                                    if (renderer)
                                        renderer->AddLight(light);
                                    lightsChanged = true;
                                }
                            if (ImGui::MenuItem("Clear Lights"))
                                {
                                    scene->ClearLights();
                                    if (renderer)
                                        renderer->ClearLights();
                                    lightsChanged = true;
                                }
                            ImGui::Separator();
                            glm::vec4 bg = scene->GetBackGround();
                            if (ImGui::ColorEdit4("Background", &bg.x))
                                scene->SetBackGround(bg);
                        }
                    else
                        {
                            ImGui::TextDisabled("No active scene");
                        }
                    ImGui::EndMenu();
                }

            if (ImGui::BeginMenu("Render"))
                {
                    if (renderer)
                        {
                            bool lighting = renderer->IsLightingEnabled();
                            if (ImGui::MenuItem("Lighting Enabled", nullptr, lighting))
                                renderer->SetLightingEnabled(!lighting);

                            bool shadows = renderer->IsShadowsEnabled();
                            if (ImGui::MenuItem("Shadows Enabled", nullptr, shadows))
                                renderer->SetShadowsEnabled(!shadows);

                            ImGui::Separator();
                            if (ImGui::MenuItem("Invalidate Shadows"))
                                renderer->InvalidateShadows();
                            if (ImGui::MenuItem("Recreate Renderer Resources"))
                                renderer->Update();
                        }
                    ImGui::EndMenu();
                }

            ImGui::EndMainMenuBar();
        }

    if (showStatsWindow_)
        {
            if (ImGui::Begin("Stats", &showStatsWindow_))
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

    if (showOpenSceneWindow_)
        {
            if (ImGui::Begin("Open Scene", &showOpenSceneWindow_))
                {
                    ImGui::TextWrapped("Load scene file via Application::LoadSceneFromFile().");
                    ImGui::InputText("Path", openScenePath_.data(), openScenePath_.size());

                    const bool canLoad = (openScenePath_[0] != '\0') && static_cast<bool>(openSceneCallback_);
                    if (!openSceneCallback_)
                        ImGui::TextDisabled("OpenScene callback is not connected.");

                    if (!canLoad)
                        ImGui::BeginDisabled();
                    if (ImGui::Button("Load"))
                        {
                            if (openSceneCallback_(std::string(openScenePath_.data())))
                                openSceneStatus_ = "Scene loaded successfully";
                            else
                                openSceneStatus_ = "Failed to load scene";
                        }
                    if (!canLoad)
                        ImGui::EndDisabled();
                    ImGui::SameLine();
                    if (ImGui::Button("Close"))
                        showOpenSceneWindow_ = false;

                    if (!openSceneStatus_.empty())
                        ImGui::TextWrapped("%s", openSceneStatus_.c_str());
                }
            ImGui::End();
        }

    if (showCameraWindow_ && controller && controller->cam_)
        {
            if (ImGui::Begin("Camera", &showCameraWindow_))
                {
                    auto& cam = *controller->cam_;
                    cameraChanged |= EditVec3("Position", cam.position_, 0.1f);

                    float yawDeg   = glm::degrees(cam.yaw_);
                    float pitchDeg = glm::degrees(cam.pitch_);
                    if (ImGui::SliderFloat("Yaw", &yawDeg, -180.0f, 180.0f))
                        {
                            cam.yaw_ = glm::radians(yawDeg);
                            cam.ProcessMouseMovement(0.0f, 0.0f, true);
                            cameraChanged = true;
                        }
                    if (ImGui::SliderFloat("Pitch", &pitchDeg, -89.0f, 89.0f))
                        {
                            cam.pitch_ = glm::radians(pitchDeg);
                            cam.ProcessMouseMovement(0.0f, 0.0f, true);
                            cameraChanged = true;
                        }
                    if (ImGui::SliderFloat("Move Speed", &cam.movementSpeed_, 0.1f, 30.0f))
                        cameraChanged = true;
                    if (ImGui::SliderFloat("Mouse Sens", &cam.mouseSensitivity_, 0.01f, 1.0f))
                        cameraChanged = true;
                    if (ImGui::SliderFloat("Zoom", &cam.zoom_, 1.0f, 90.0f))
                        cameraChanged = true;

                    if (ImGui::Button("Reset Camera"))
                        {
                            cam.position_ = glm::vec3(6.0f, 4.0f, 10.0f);
                            cam.yaw_      = glm::radians(-90.0f);
                            cam.pitch_    = 0.0f;
                            cam.ProcessMouseMovement(0.0f, 0.0f, true);
                            cameraChanged = true;
                        }
                }
            ImGui::End();
        }

    if (showLightsWindow_ && scene)
        {
            if (ImGui::Begin("Lights", &showLightsWindow_))
                {
                    if (ImGui::Button("Add Directional"))
                        {
                            auto light = std::make_shared<DirectionalLight>(
                                glm::vec3(0.2f), glm::vec3(0.9f), glm::vec3(1.0f),
                                glm::vec3(1.0f, 0.0f, 0.0f),
                                glm::normalize(glm::vec3(-0.5f, -1.0f, -0.2f)));
                            light->SetName("imgui_dir_light");
                            scene->AddLight(light);
                            if (renderer)
                                renderer->AddLight(light);
                            lightsChanged = true;
                        }
                    ImGui::SameLine();
                    if (ImGui::Button("Add Point"))
                        {
                            auto light = std::make_shared<PointLight>(
                                glm::vec3(0.1f), glm::vec3(1.0f), glm::vec3(1.0f),
                                glm::vec3(1.0f, 0.09f, 0.032f),
                                glm::vec3(0.0f, 2.0f, 0.0f));
                            light->SetName("imgui_point_light");
                            scene->AddLight(light);
                            if (renderer)
                                renderer->AddLight(light);
                            lightsChanged = true;
                        }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear Lights"))
                        {
                            scene->ClearLights();
                            if (renderer)
                                renderer->ClearLights();
                            lightsChanged = true;
                        }

                    ImGui::Separator();

                    auto lights = scene->GetLights();
                    int idx = 0;
                    for (auto it = lights.first; it != lights.second; ++it, ++idx)
                        {
                            auto light = it->second;
                            if (!light)
                                continue;

                            const std::string header = std::string(LightTypeName(light->GetType())) +
                                                       "##" + std::to_string(idx);
                            if (!ImGui::CollapsingHeader(header.c_str()))
                                continue;

                            glm::vec3 ambient = light->GetAmbient();
                            glm::vec3 diffuse = light->GetDiffuse();
                            glm::vec3 specular = light->GetSpecular();
                            glm::vec3 atten = light->GetAttenuation();

                            if (ImGui::ColorEdit3(("Ambient##" + std::to_string(idx)).c_str(), &ambient.x))
                                { light->SetAmbient(ambient); lightsChanged = true; }
                            if (ImGui::ColorEdit3(("Diffuse##" + std::to_string(idx)).c_str(), &diffuse.x))
                                { light->SetDiffuse(diffuse); lightsChanged = true; }
                            if (ImGui::ColorEdit3(("Specular##" + std::to_string(idx)).c_str(), &specular.x))
                                { light->SetSpecular(specular); lightsChanged = true; }
                            if (EditVec3(("Attenuation##" + std::to_string(idx)).c_str(), atten, 0.01f))
                                { light->SetAttenuation(atten); lightsChanged = true; }

                            if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light);
                                dir && light->GetType() == LightType::Directional)
                                {
                                    glm::vec3 v = dir->GetDir();
                                    if (EditVec3(("Direction##" + std::to_string(idx)).c_str(), v, 0.02f))
                                        {
                                            if (glm::length(v) > 0.0001f)
                                                dir->ChangeDirection(glm::normalize(v));
                                            lightsChanged = true;
                                        }
                                }

                            if (auto point = std::dynamic_pointer_cast<PointLight>(light))
                                {
                                    glm::vec3 p = point->GetPos();
                                    if (EditVec3(("Position##" + std::to_string(idx)).c_str(), p, 0.05f))
                                        {
                                            point->SetPos(p);
                                            lightsChanged = true;
                                        }
                                }

                            if (auto spot = std::dynamic_pointer_cast<SpotLight>(light))
                                {
                                    auto ang = spot->GetAngles();
                                    float outer = ang.first;
                                    float inner = ang.second;
                                    if (ImGui::SliderFloat(("Outer##" + std::to_string(idx)).c_str(), &outer, 1.0f, 89.0f))
                                        {
                                            outer = std::max(outer, inner);
                                            spot->SetAngles({outer, inner});
                                            lightsChanged = true;
                                        }
                                    if (ImGui::SliderFloat(("Inner##" + std::to_string(idx)).c_str(), &inner, 0.1f, 88.0f))
                                        {
                                            inner = std::min(inner, outer);
                                            spot->SetAngles({outer, inner});
                                            lightsChanged = true;
                                        }
                                }
                        }
                }
            ImGui::End();
        }

    if (showDebugWindow_)
        {
            if (ImGui::Begin("Debug Actions", &showDebugWindow_))
                {
                    if (renderer)
                        {
                            bool lighting = renderer->IsLightingEnabled();
                            if (ImGui::Checkbox("Lighting Enabled", &lighting))
                                renderer->SetLightingEnabled(lighting);

                            bool shadows = renderer->IsShadowsEnabled();
                            if (ImGui::Checkbox("Shadows Enabled", &shadows))
                                renderer->SetShadowsEnabled(shadows);
                        }
                    ImGui::Checkbox("Axis Gizmo xOyOz", &showAxisGizmo_);

                    if (renderer && ImGui::Button("Invalidate Shadows"))
                        {
                            renderer->InvalidateShadows();
                        }
                    if (renderer && ImGui::Button("Recreate Renderer Resources"))
                        {
                            renderer->Update();
                        }
                    ImGui::TextWrapped("Use this panel to force shadow/descriptor refresh during porting.");
                }
            ImGui::End();
        }

    if (controller && cameraChanged)
        {
            controller->UpdateViewMatrix();
            if (window_)
                controller->UpdateProjectionMatrix(
                    static_cast<float>(window_->scrWidth_),
                    static_cast<float>(window_->scrHeight_));
        }
    if (renderer && lightsChanged)
        renderer->InvalidateShadows();

    if (showAxisGizmo_ && controller && controller->cam_)
        DrawAxisGizmo(*controller->cam_);
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
