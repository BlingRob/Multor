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
            pWindow_   = std::make_shared<Window>(&signals_, pContr_);
            pRenderer_ = std::make_shared<Vulkan::Renderer>(pWindow_);
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

void Application::AddLight(std::shared_ptr<BLight> light)
{
    if (!pLights_)
        throw std::runtime_error("light manager is not initialized");
    pLights_->Add(std::move(light));
    SyncLightsToRenderer();
}

void Application::ClearLights()
{
    if (!pLights_)
        throw std::runtime_error("light manager is not initialized");
    pLights_->Clear();
    SyncLightsToRenderer();
}

void Application::SyncLightsToRenderer()
{
    if (!pRenderer_)
        throw std::runtime_error("renderer is not initialized");
    if (!pLights_)
        throw std::runtime_error("light manager is not initialized");
    pRenderer_->SetLights(pLights_->GetAll());
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

} // namespace Multor
