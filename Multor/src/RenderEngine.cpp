/// \file RenderEngine.cpp
#include "RenderEngine.h"

namespace Multor
{

Application::Application()
{
    _pLog = std::make_shared<Logging::Logger>();
    try
        {
            pContr_    = std::make_shared<Position_Controller>();
            pWindow_   = std::make_shared<Window>(&signals, pContr_);
            _pRenderer = std::make_shared<VulkanRenderer>(pWindow_, _pLog);

            signals[0] = std::bind(&VulkanRenderer::Update, _pRenderer);
        }
    catch (std::exception err)
        {
            _pLog->Log<Logging::File, Logging::Error>(err.what());
            throw err;
        }
    catch (const char* err)
        {
            _pLog->Log<Logging::File, Logging::Error>(err);
            throw err;
        }
    catch (std::string err)
        {
            _pLog->Log<Logging::File, Logging::Error>(err.c_str());
            throw err;
        }
}

std::shared_ptr<VulkanRenderer> Application::GetRenderer()
{
    return _pRenderer;
}

bool Application::MainLoop()
{
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
    catch (std::exception err)
        {
            _pLog->Log<Logging::Window, Logging::Error>(err.what());
            return false;
        }
    catch (const char* err)
        {
            _pLog->Log<Logging::Window, Logging::Error>(err);
            return false;
        }
    catch (std::string err)
        {
            _pLog->Log<Logging::Window, Logging::Error>(err.c_str());
            return false;
        }
}

double Application::GetTime()
{
    return chron.GetTime();
}

} // namespace Multor