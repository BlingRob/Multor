/// \file RenderEngine.cpp

#include "application.h"

#include "configure.h"

#include <filesystem>

namespace Multor
{

Application::Application()
{
    if(std::filesystem::exists(CFG_DEFAULT_FILE))
    {
        table_ = toml::parse_file(CFG_DEFAULT_FILE);
    }

    Logging::LoggerFactory::Init(table_);

    auto logger = Logging::LoggerFactory::GetLogger(table_["logging"]["filename"].value_or(DEFAULT_LOG_FILE));
    
    LOG_INFO(logger.get(), "Application initialization...");

    try
        {
            pContr_    = std::make_shared<Position_Controller>();
            pWindow_   = std::make_shared<Window>(&signals, pContr_);
            _pRenderer = std::make_shared<Vulkan::Renderer>(pWindow_);

            signals[0] = std::bind(&Vulkan::Renderer::Update, _pRenderer);

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
    return _pRenderer;
}

bool Application::MainLoop()
{
    static auto logger{Logging::LoggerFactory::GetLogger(table_["logging"]["filename"].value_or(DEFAULT_LOG_FILE))};
    try
        {
            pContr_->dt = static_cast<float>(chron());
            if (!pWindow_->ProcEvents())
                return false;
            pWindow_->SwapBuffer();
            //(*_ppScene)->Draw();
            //*pContr_->View = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            //*pContr_->Projection = glm::perspective(glm::radians(45.0f), 800 / (float)800, 0.1f, 10.0f);
            //_pRenderer->UpdatePV(*pContr_->Projection * (*pContr_->View));
            _pRenderer->Draw();

            //gui->Draw();

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
    return chron.GetTime();
}

} // namespace Multor