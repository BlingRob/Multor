/// \file RenderEngine.cpp

#include "application.h"

#include "configure.h"

#include <filesystem>
#include <chrono>
#include <thread>

namespace Multor
{

Application::Application()
{
    if (std::filesystem::exists(CFG_DEFAULT_FILE))
        {
            table_ = toml::parse_file(CFG_DEFAULT_FILE);
        }
    else if (std::filesystem::exists("config/config.toml"))
        {
            table_ = toml::parse_file("config/config.toml");
        }

    Logging::LoggerFactory::Init(table_);

    auto logger = Logging::LoggerFactory::GetLogger(table_["logging"]["filename"].value_or(DEFAULT_LOG_FILE));
    
    LOG_INFO(logger.get(), "Application initialization...");

    try
        {
            pContr_    = std::make_shared<PositionController>();
            pLights_   = std::make_shared<LightManager>();
            pScene_    = std::make_shared<Scene>(pContr_);
            pWindow_   = std::make_shared<Window>(&signals_, pContr_);
            pRenderer_ = std::make_shared<Vulkan::Renderer>(pWindow_);
            pGui_      = std::make_unique<ImGuiOverlay>();
            pGui_->AttachWindow(pWindow_.get());
            pGui_->AttachRenderer(pRenderer_);
            pGui_->SetOpenSceneCallback([this](const std::string& path)
            {
                return this->LoadSceneFromFile(path);
            });
            LOG_INFO(logger.get(), "GUI overlay status: {}", pGui_->BackendStatus());
            pWindow_->SetEventInterceptor([this](const SDL_Event& e)
            {
                if (pGui_)
                    pGui_->OnSdlEvent(e);
            });
            SyncLightsToRenderer();

            signals_[0] = std::bind(&Vulkan::Renderer::Update, pRenderer_);

            LOG_INFO(logger.get(), "Application was initializated");
        }
    catch (const std::exception& err)
        {
            LOG_CRITICAL(logger.get(), "Fail start application with error: {}", err.what());
            throw err;
        }
    catch (const std::string& err)
        {
            LOG_CRITICAL(logger.get(), "Fail start application with error: {}", err);
            throw err;
        }
}

Application::~Application(){}

std::shared_ptr<Vulkan::Renderer> Application::GetRenderer()
{
    return pRenderer_;
}

std::shared_ptr<Scene> Application::GetScene()
{
    return pScene_;
}

void Application::SetScene(std::shared_ptr<Scene> scene)
{
    pScene_ = std::move(scene);
    if (pScene_ && !pScene_->GetController())
        pScene_->SetController(pContr_);
    if (pLights_)
        {
            pLights_->Clear();
            if (pScene_)
                {
                    auto lights = pScene_->GetLights();
                    for (auto it = lights.first; it != lights.second; ++it)
                        if (it->second)
                            pLights_->Add(it->second);
                }
        }
    SyncSceneToRenderer();
}

void Application::AddLight(std::shared_ptr<BLight> light)
{
    if (!pLights_)
        throw std::runtime_error("light manager is not initialized");
    pLights_->Add(std::move(light));
    if (pScene_)
        pScene_->AddLight(pLights_->GetAll().back());
    SyncLightsToRenderer();
}

void Application::ClearLights()
{
    if (!pLights_)
        throw std::runtime_error("light manager is not initialized");
    pLights_->Clear();
    if (pScene_)
        pScene_->ClearLights();
    SyncLightsToRenderer();
}

void Application::InvalidateShadows()
{
    if (!pRenderer_)
        throw std::runtime_error("renderer is not initialized");
    pRenderer_->InvalidateShadows();
}

void Application::SyncLightsToRenderer()
{
    if (!pRenderer_)
        throw std::runtime_error("renderer is not initialized");
    if (!pLights_)
        throw std::runtime_error("light manager is not initialized");
    pRenderer_->SetLights(pLights_->GetAll());
}

void Application::SyncSceneToRenderer()
{
    if (!pRenderer_)
        throw std::runtime_error("renderer is not initialized");

    sceneMeshBindings_.clear();

    if (!pScene_)
        return;

    if (pScene_->GetController() == nullptr)
        pScene_->SetController(pContr_);

    pRenderer_->ClearMeshes();
    pRenderer_->ClearLights();
    auto lights = pScene_->GetLights();
    for (auto it = lights.first; it != lights.second; ++it)
        if (it->second)
            pRenderer_->AddLight(it->second);

    std::vector<std::shared_ptr<Node> > sceneNodesForMeshes;
    std::vector<std::unique_ptr<BaseMesh> > meshClones;

    auto models = pScene_->GetModels();
    std::function<void(const std::shared_ptr<Node>&)> collectNodeBindings;
    collectNodeBindings = [this, &collectNodeBindings, &sceneNodesForMeshes,
                           &meshClones](const std::shared_ptr<Node>& node)
    {
        if (!node)
            return;

        auto meshes = node->GetMeshes();
        for (auto mit = meshes.first; mit != meshes.second; ++mit)
            {
                if (!(*mit))
                    continue;
                auto clone = (*mit)->Clone();
                if (!clone)
                    continue;
                sceneNodesForMeshes.push_back(node);
                meshClones.push_back(std::move(clone));
            }

        auto children = node->GetChildren();
        for (auto cit = children.first; cit != children.second; ++cit)
            collectNodeBindings(*cit);
    };

    for (auto it = models.first; it != models.second; ++it)
        {
            if (!it->second || !it->second->GetRoot())
                continue;
            collectNodeBindings(it->second->GetRoot());
        }

    auto vkMeshes = pRenderer_->AddMeshes(std::move(meshClones));
    const std::size_t bindCount =
        std::min(sceneNodesForMeshes.size(), vkMeshes.size());
    sceneMeshBindings_.reserve(bindCount);
    for (std::size_t i = 0; i < bindCount; ++i)
        {
            if (sceneNodesForMeshes[i] && vkMeshes[i])
                sceneMeshBindings_.push_back({sceneNodesForMeshes[i], vkMeshes[i]});
        }
}

void Application::UpdateSceneBindings()
{
    if (!pRenderer_)
        return;

    const std::size_t frame = pRenderer_->GetCurFrame();
    for (auto& binding : sceneMeshBindings_)
        {
            auto node = binding.node_.lock();
            if (!node || !binding.vkMesh_ || !binding.vkMesh_->tr_)
                continue;
            binding.vkMesh_->tr_->updateModel(frame, node->GetTransform());
        }
}

bool Application::MainLoop()
{
    static auto logger{Logging::LoggerFactory::GetLogger(table_["logging"]["filename"].value_or(DEFAULT_LOG_FILE))};
    try
        {
            using clock = std::chrono::high_resolution_clock;
            const int maxFps = table_["rendering"]["max_fps"].value_or(30);
            const auto frameStart = clock::now();

            pContr_->dt_ = static_cast<float>(chron_());
            if (!pWindow_->ProcEvents())
                return false;
            pWindow_->SwapBuffer();
            UpdateSceneBindings();
            if (pGui_)
                {
                    pGui_->NewFrame();
                    pGui_->Draw(pScene_, pContr_, pRenderer_);
                    pGui_->Render();
                }
            pRenderer_->Draw();

            if (maxFps > 0)
                {
                    const auto frameTarget =
                        std::chrono::duration<double>(1.0 / static_cast<double>(maxFps));
                    const auto frameTime = clock::now() - frameStart;
                    if (frameTime < frameTarget)
                        std::this_thread::sleep_for(frameTarget - frameTime);
                }


            return true;
        }
    catch (const std::exception& err)
        {
            LOG_CRITICAL(logger.get(), "Fail start application with error: {}", err.what());
            return false;
        }
    catch (const std::string& err)
        {
            LOG_CRITICAL(logger.get(), "Fail start application with error: {}", err);
            throw err;
        }
}

double Application::GetTime()
{
    return chron_.GetTime();
}

bool Application::LoadSceneFromFile(std::string_view path)
{
    SceneLoader loader;
    if (!loader.LoadScene(path))
        return false;

    auto scene = loader.GetScene(pContr_);
    if (!scene)
        return false;

    SetScene(std::move(scene));
    return true;
}

} // namespace Multor
