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
const char* PbrDebugViewName(Multor::Vulkan::PbrDebugView view)
{
    switch (view)
        {
        case Multor::Vulkan::PbrDebugView::BaseColor:
            return "Base Color";
        case Multor::Vulkan::PbrDebugView::Normal:
            return "Normal";
        case Multor::Vulkan::PbrDebugView::Metallic:
            return "Metallic";
        case Multor::Vulkan::PbrDebugView::Roughness:
            return "Roughness";
        case Multor::Vulkan::PbrDebugView::AO:
            return "AO";
        case Multor::Vulkan::PbrDebugView::Emissive:
            return "Emissive";
        case Multor::Vulkan::PbrDebugView::EnvDiffuse:
            return "Env Diffuse";
        case Multor::Vulkan::PbrDebugView::EnvSpecular:
            return "Env Specular";
        case Multor::Vulkan::PbrDebugView::GeomNormal:
            return "Geom Normal";
        case Multor::Vulkan::PbrDebugView::Tangent:
            return "Tangent";
        case Multor::Vulkan::PbrDebugView::Bitangent:
            return "Bitangent";
        case Multor::Vulkan::PbrDebugView::NormalDelta:
            return "Normal Delta";
        case Multor::Vulkan::PbrDebugView::Shaded:
        default:
            return "Shaded";
        }
}

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

bool EditVec4(const char* label, glm::vec4& v)
{
    float vals[4] = {v.x, v.y, v.z, v.w};
    if (!ImGui::ColorEdit4(label, vals))
        return false;
    v = glm::vec4(vals[0], vals[1], vals[2], vals[3]);
    return true;
}

bool DrawPbrEnvironmentEditor(Multor::Vulkan::PbrEnvironmentSettings& settings,
                              std::string_view suffix = {})
{
    const std::string suffixStr(suffix);
    bool changed = false;
    changed |= ImGui::SliderFloat(("Ambient Diffuse##" + suffixStr).c_str(),
                                  &settings.diffuseAmbientIntensity_, 0.0f, 1.5f);
    changed |= ImGui::SliderFloat(("Ambient Specular##" + suffixStr).c_str(),
                                  &settings.specularAmbientIntensity_, 0.0f, 2.0f);
    changed |= ImGui::SliderFloat(("Env Fresnel##" + suffixStr).c_str(),
                                  &settings.envFresnelStrength_, 0.0f, 2.0f);
    changed |= ImGui::SliderFloat(("Env Reflection##" + suffixStr).c_str(),
                                  &settings.envReflectionPower_, 0.0f, 2.0f);
    changed |= ImGui::SliderFloat(
        ("Roughness Blur##" + suffixStr).c_str(),
        &settings.roughnessAwareBlurStrength_, 0.0f, 3.0f);
    changed |= ImGui::SliderFloat(
        ("Prefilter Radius##" + suffixStr).c_str(),
        &settings.prefilterSampleRadiusScale_, 0.0f, 3.0f);
    changed |= ImGui::SliderFloat(
        ("Center Weight##" + suffixStr).c_str(),
        &settings.prefilterCenterWeightScale_, 0.1f, 3.0f);
    changed |= ImGui::SliderFloat(
        ("Ring Weight##" + suffixStr).c_str(),
        &settings.prefilterRingWeightScale_, 0.0f, 3.0f);
    bool envUsesGeomNormal = settings.envSpecularUsesGeometricNormal_ >= 0.5f;
    if (ImGui::Checkbox(("Env Uses Geom Normal##" + suffixStr).c_str(),
                        &envUsesGeomNormal))
        {
            settings.envSpecularUsesGeometricNormal_ =
                envUsesGeomNormal ? 1.0f : 0.0f;
            changed = true;
        }
    bool directUsesNormalMap = settings.directLightingUsesNormalMap_ >= 0.5f;
    if (ImGui::Checkbox(("Direct Uses Normal Map##" + suffixStr).c_str(),
                        &directUsesNormalMap))
        {
            settings.directLightingUsesNormalMap_ =
                directUsesNormalMap ? 1.0f : 0.0f;
            changed = true;
        }
    return changed;
}

bool DrawNodeMaterialNormalScale(const std::shared_ptr<Node>& node,
                                 const char* label, float minValue = 0.0f,
                                 float maxValue = 3.0f)
{
    if (!node)
        return false;

    auto meshes = node->GetMeshes();
    for (auto it = meshes.first; it != meshes.second; ++it)
        {
            auto mesh = *it;
            if (!mesh || !mesh->GetMaterial())
                continue;

            auto* material = mesh->GetMaterial();
            if (ImGui::SliderFloat(label, &material->normalScale, minValue,
                                   maxValue))
                return true;
            break;
        }
    return false;
}

void DrawMaterialEditorForNode(const std::shared_ptr<Node>& node, bool& materialsChanged)
{
    if (!node)
        return;

    auto meshes = node->GetMeshes();
    int meshIdx = 0;
    for (auto it = meshes.first; it != meshes.second; ++it, ++meshIdx)
        {
            auto mesh = *it;
            if (!mesh || !mesh->GetMaterial())
                continue;

            auto* material = mesh->GetMaterial();
            const std::string meshLabel =
                (mesh->GetName().empty() ? std::string("Mesh") : std::string(mesh->GetName())) +
                "##mat_" + std::string(node->GetName()) + "_" + std::to_string(meshIdx);

            if (!ImGui::TreeNode(meshLabel.c_str()))
                continue;

            bool usePbr = material->IsPBR();
            if (ImGui::Checkbox(("Use PBR##" + meshLabel).c_str(), &usePbr))
                {
                    if (usePbr)
                        material->UseMetallicRoughnessPBR(material->baseColorFactor,
                                                          material->metallicFactor,
                                                          material->roughnessFactor);
                    else
                        material->UseLegacyPhong();
                    materialsChanged = true;
                }

            if (usePbr)
                {
                    glm::vec4 baseColor = material->baseColorFactor;
                    if (EditVec4(("Base Color##" + meshLabel).c_str(), baseColor))
                        {
                            material->baseColorFactor = baseColor;
                            materialsChanged = true;
                        }
                    if (ImGui::SliderFloat(("Metallic##" + meshLabel).c_str(),
                                           &material->metallicFactor, 0.0f, 1.0f))
                        materialsChanged = true;
                    if (ImGui::SliderFloat(("Roughness##" + meshLabel).c_str(),
                                           &material->roughnessFactor, 0.02f, 1.0f))
                        materialsChanged = true;
                    if (ImGui::SliderFloat(("Normal Scale##" + meshLabel).c_str(),
                                           &material->normalScale, 0.0f, 3.0f))
                        materialsChanged = true;
                    if (ImGui::SliderFloat(("AO Strength##" + meshLabel).c_str(),
                                           &material->aoStrength, 0.0f, 2.0f))
                        materialsChanged = true;
                    glm::vec3 emissive = material->emissiveFactor;
                    if (ImGui::ColorEdit3(("Emissive##" + meshLabel).c_str(), &emissive.x))
                        {
                            material->emissiveFactor = emissive;
                            materialsChanged = true;
                        }
                }
            else
                {
                    glm::vec3 ambient = material->ambient;
                    glm::vec3 diffuse = material->diffuse;
                    glm::vec3 specular = material->specular;
                    if (ImGui::ColorEdit3(("Ambient##" + meshLabel).c_str(), &ambient.x))
                        {
                            material->ambient = ambient;
                            materialsChanged = true;
                        }
                    if (ImGui::ColorEdit3(("Diffuse##" + meshLabel).c_str(), &diffuse.x))
                        {
                            material->diffuse = diffuse;
                            materialsChanged = true;
                        }
                    if (ImGui::ColorEdit3(("Specular##" + meshLabel).c_str(), &specular.x))
                        {
                            material->specular = specular;
                            materialsChanged = true;
                        }
                    if (ImGui::SliderFloat(("Shininess##" + meshLabel).c_str(),
                                           &material->shininess, 1.0f, 128.0f))
                        materialsChanged = true;
                }

            ImGui::TextDisabled("Textures");
            ImGui::BulletText("BaseColor: %s", mesh->FindTexture(Texture_Types::Diffuse) ? "yes" : "no");
            ImGui::BulletText("Normal: %s", mesh->FindTexture(Texture_Types::Normal) ? "yes" : "no");
            ImGui::BulletText("Metallic: %s", mesh->FindTexture(Texture_Types::Metallic) ? "yes" : "no");
            ImGui::BulletText("Roughness: %s", mesh->FindTexture(Texture_Types::Roughness) ? "yes" : "no");
            ImGui::BulletText("AO: %s", mesh->FindTexture(Texture_Types::Ambient_occlusion) ? "yes" : "no");
            ImGui::BulletText("Emissive: %s", mesh->FindTexture(Texture_Types::Emissive) ? "yes" : "no");

            ImGui::TreePop();
        }

    auto children = node->GetChildren();
    for (auto it = children.first; it != children.second; ++it)
        DrawMaterialEditorForNode(*it, materialsChanged);
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
                    ImGui::MenuItem("Materials", nullptr, &showMaterialsWindow_);
                    ImGui::MenuItem("Debug", nullptr, &showDebugWindow_);
                    ImGui::MenuItem("PBR Env", nullptr, &showPbrEnvironmentWindow_);
                    ImGui::Separator();
                    ImGui::MenuItem("Axis Gizmo xOyOz", nullptr, &showAxisGizmo_);
                    ImGui::EndMenu();
                }

            if (ImGui::BeginMenu("Scene"))
                {
                    if (scene)
                        {
                            bool physicsEnabled = scene->IsPhysicsEnabled();
                            if (ImGui::MenuItem("Physics Enabled", nullptr, physicsEnabled))
                                scene->SetPhysicsEnabled(!physicsEnabled);
                            ImGui::Separator();
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

                            ImGui::Separator();
                            const Vulkan::PbrDebugView currentView = renderer->GetPbrDebugView();
                            if (ImGui::BeginMenu("PBR Debug View"))
                                {
                                    for (int i = static_cast<int>(Vulkan::PbrDebugView::Shaded);
                                         i <= static_cast<int>(Vulkan::PbrDebugView::NormalDelta); ++i)
                                        {
                                            const auto view = static_cast<Vulkan::PbrDebugView>(i);
                                            const bool selected = (view == currentView);
                                            if (ImGui::MenuItem(PbrDebugViewName(view), nullptr, selected))
                                                renderer->SetPbrDebugView(view);
                                        }
                                    ImGui::EndMenu();
                                }

                            ImGui::Separator();
                            auto pbrSettings = renderer->GetPbrEnvironmentSettings();
                            if (ImGui::BeginMenu("PBR Environment"))
                                {
                                    if (DrawPbrEnvironmentEditor(pbrSettings, "menu"))
                                        renderer->SetPbrEnvironmentSettings(pbrSettings);
                                    if (ImGui::MenuItem("Open Environment Window"))
                                        showPbrEnvironmentWindow_ = true;
                                    ImGui::EndMenu();
                                }
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
                            ImGui::Text("Physics bodies: %zu", info.amountPhysicsBodies_);
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

    if (showPbrEnvironmentWindow_)
        {
            if (ImGui::Begin("PBR Environment", &showPbrEnvironmentWindow_))
                {
                    if (renderer)
                        {
                            auto settings = renderer->GetPbrEnvironmentSettings();
                            if (DrawPbrEnvironmentEditor(settings, "envwindow"))
                                renderer->SetPbrEnvironmentSettings(settings);

                            if (scene)
                                {
                                    bool demoNormalsChanged = false;
                                    ImGui::Separator();
                                    ImGui::TextDisabled("Demo Spheres");
                                    if (auto node =
                                            scene->GetNode("shadow_demo_sphere"))
                                        demoNormalsChanged |=
                                            DrawNodeMaterialNormalScale(
                                                node,
                                                "Primary Sphere Normal");
                                    if (auto node = scene->GetNode(
                                            "shadow_demo_metal_sphere"))
                                        demoNormalsChanged |=
                                            DrawNodeMaterialNormalScale(
                                                node,
                                                "Metal Sphere Normal");
                                    if (demoNormalsChanged)
                                        renderer->InvalidateShadows();
                                }

                            ImGui::Separator();
                            if (environmentPath_[0] == '\0')
                                std::snprintf(environmentPath_.data(),
                                              environmentPath_.size(), "%s",
                                              "../../Res/matrix.jpg");
                            ImGui::InputText("Environment Path",
                                             environmentPath_.data(),
                                             environmentPath_.size());
                            if (ImGui::Button("Load Environment"))
                                {
                                    if (renderer->LoadEnvironmentTexture(
                                            std::string_view(
                                                environmentPath_.data())))
                                        environmentStatus_ =
                                            "Environment texture loaded";
                                    else
                                        environmentStatus_ =
                                            "Failed to load environment texture";
                                }
                            ImGui::SameLine();
                            if (ImGui::Button("Clear Environment"))
                                {
                                    renderer->ClearEnvironmentTexture();
                                    environmentStatus_ =
                                        "Environment reset to neutral";
                                }
                            const std::string currentPath =
                                renderer->HasEnvironmentTexture()
                                    ? std::string(
                                          renderer->GetEnvironmentTexturePath())
                                    : std::string("generated neutral");
                            ImGui::TextWrapped("Current: %s",
                                               currentPath.c_str());
                            if (!environmentStatus_.empty())
                                ImGui::TextWrapped("%s",
                                                   environmentStatus_.c_str());
                            ImGui::Separator();
                            if (irradiancePath_[0] == '\0')
                                std::snprintf(irradiancePath_.data(),
                                              irradiancePath_.size(), "%s",
                                              "../../Res/matrix.jpg");
                            ImGui::InputText("Irradiance Path",
                                             irradiancePath_.data(),
                                             irradiancePath_.size());
                            if (ImGui::Button("Load Irradiance"))
                                {
                                    if (renderer->LoadIrradianceTexture(
                                            std::string_view(
                                                irradiancePath_.data())))
                                        environmentStatus_ =
                                            "Irradiance texture loaded";
                                    else
                                        environmentStatus_ =
                                            "Failed to load irradiance texture";
                                }
                            ImGui::SameLine();
                            if (ImGui::Button("Clear Irradiance"))
                                {
                                    renderer->ClearIrradianceTexture();
                                    environmentStatus_ =
                                        "Irradiance reset to default";
                                }
                            const std::string currentIrradiancePath =
                                renderer->HasIrradianceTexture()
                                    ? std::string(
                                          renderer->GetIrradianceTexturePath())
                                    : std::string("environment fallback");
                            ImGui::TextWrapped("Irradiance: %s",
                                               currentIrradiancePath.c_str());
                            ImGui::Separator();
                            if (prefilteredEnvironmentPath_[0] == '\0')
                                std::snprintf(prefilteredEnvironmentPath_.data(),
                                              prefilteredEnvironmentPath_.size(),
                                              "%s", "../../Res/matrix.jpg");
                            ImGui::InputText("Prefiltered Env Path",
                                             prefilteredEnvironmentPath_.data(),
                                             prefilteredEnvironmentPath_.size());
                            if (ImGui::Button("Load Prefiltered Env"))
                                {
                                    if (renderer->LoadPrefilteredEnvironmentTexture(
                                            std::string_view(
                                                prefilteredEnvironmentPath_.data())))
                                        environmentStatus_ =
                                            "Prefiltered environment loaded";
                                    else
                                        environmentStatus_ =
                                            "Failed to load prefiltered environment";
                                }
                            ImGui::SameLine();
                            if (ImGui::Button("Clear Prefiltered Env"))
                                {
                                    renderer->ClearPrefilteredEnvironmentTexture();
                                    environmentStatus_ =
                                        "Prefiltered environment reset to default";
                                }
                            const std::string currentPrefilteredPath =
                                renderer->HasPrefilteredEnvironmentTexture()
                                    ? std::string(renderer
                                                      ->GetPrefilteredEnvironmentTexturePath())
                                    : std::string("environment fallback");
                            ImGui::TextWrapped("Prefiltered: %s",
                                               currentPrefilteredPath.c_str());
                            ImGui::Separator();
                            if (brdfLutPath_[0] == '\0')
                                std::snprintf(brdfLutPath_.data(),
                                              brdfLutPath_.size(), "%s",
                                              "../../Res/brdf_lut.png");
                            ImGui::InputText("BRDF LUT Path",
                                             brdfLutPath_.data(),
                                             brdfLutPath_.size());
                            if (ImGui::Button("Load BRDF LUT"))
                                {
                                    if (renderer->LoadBrdfLutTexture(
                                            std::string_view(brdfLutPath_.data())))
                                        environmentStatus_ = "BRDF LUT loaded";
                                    else
                                        environmentStatus_ =
                                            "Failed to load BRDF LUT";
                                }
                            ImGui::SameLine();
                            if (ImGui::Button("Clear BRDF LUT"))
                                {
                                    renderer->ClearBrdfLutTexture();
                                    environmentStatus_ =
                                        "BRDF LUT reset to generated default";
                                }
                            const std::string currentBrdfLutPath =
                                renderer->HasBrdfLutTexture()
                                    ? std::string(
                                          renderer->GetBrdfLutTexturePath())
                                    : std::string("generated default");
                            ImGui::TextWrapped("BRDF LUT: %s",
                                               currentBrdfLutPath.c_str());
                        }
                    else
                        {
                            ImGui::TextDisabled("Renderer is not available");
                        }
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

    if (showMaterialsWindow_ && scene)
        {
            bool materialsChanged = false;
            if (ImGui::Begin("Materials", &showMaterialsWindow_))
                {
                    if (renderer)
                        {
                            int debugView = static_cast<int>(renderer->GetPbrDebugView());
                            const char* debugItems[] = {
                                "Shaded", "Base Color", "Normal", "Metallic",
                                "Roughness", "AO", "Emissive",
                                "Env Diffuse", "Env Specular",
                                "Geom Normal", "Tangent",
                                "Bitangent", "Normal Delta"};
                            if (ImGui::Combo("PBR Debug View", &debugView, debugItems,
                                             IM_ARRAYSIZE(debugItems)))
                                renderer->SetPbrDebugView(
                                    static_cast<Vulkan::PbrDebugView>(debugView));
                            auto pbrSettings = renderer->GetPbrEnvironmentSettings();
                            if (DrawPbrEnvironmentEditor(pbrSettings, "materials"))
                                renderer->SetPbrEnvironmentSettings(pbrSettings);
                            ImGui::Separator();
                        }

                    auto nodes = scene->GetNodes();
                    for (auto it = nodes.first; it != nodes.second; ++it)
                        {
                            const auto& node = it->second;
                            if (!node)
                                continue;
                            const std::string nodeLabel =
                                (node->GetName().empty() ? std::string("Node") : std::string(node->GetName())) +
                                "##node_materials";
                            if (ImGui::TreeNode(nodeLabel.c_str()))
                                {
                                    DrawMaterialEditorForNode(node, materialsChanged);
                                    ImGui::TreePop();
                                }
                        }

                    auto models = scene->GetModels();
                    for (auto it = models.first; it != models.second; ++it)
                        {
                            const auto& model = it->second;
                            if (!model || !model->GetRoot())
                                continue;
                            const std::string modelLabel =
                                (model->GetName().empty() ? std::string("Model") : std::string(model->GetName())) +
                                "##model_materials";
                            if (ImGui::TreeNode(modelLabel.c_str()))
                                {
                                    DrawMaterialEditorForNode(model->GetRoot(), materialsChanged);
                                    ImGui::TreePop();
                                }
                        }
                }
            ImGui::End();
            if (renderer && materialsChanged)
                renderer->InvalidateShadows();
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
                            ImGui::Separator();
                            auto pbrSettings = renderer->GetPbrEnvironmentSettings();
                            if (DrawPbrEnvironmentEditor(pbrSettings, "debug"))
                                renderer->SetPbrEnvironmentSettings(pbrSettings);
                        }
                    if (scene)
                        {
                            glm::vec3 gravity = scene->GetPhysicsWorld().GetGravity();
                            if (EditVec3("Gravity", gravity, 0.1f))
                                scene->GetPhysicsWorld().SetGravity(gravity);
                            bool physicsEnabled = scene->IsPhysicsEnabled();
                            if (ImGui::Checkbox("Physics Enabled", &physicsEnabled))
                                scene->SetPhysicsEnabled(physicsEnabled);
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
