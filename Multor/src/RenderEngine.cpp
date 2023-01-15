/// \file RenderEngine.cpp
#include "RenderEngine.h"

namespace Multor
{

Application::Application() 
{	
	_pLog = std::make_shared<Logging::Logger>();
	try
	{
		_pContr = std::make_shared<Position_Controller>();
		_pWindow = std::make_shared<Window>(&signals, _pContr);
		_pRenderer = std::make_shared<VulkanRenderer>(_pWindow, _pLog);

		signals[0] = std::bind(&VulkanRenderer::Update, _pRenderer);
	}
	catch (std::exception err)
	{
		_pLog->Log<Logging::File, Logging::Error>(err.what());
	}
	catch (const char* err)
	{
		_pLog->Log<Logging::File, Logging::Error>(err);
	}
	catch (std::string err)
	{
		_pLog->Log<Logging::File, Logging::Error>(err.c_str());
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
		_pContr->dt = static_cast<float>(chron());
		if (!_pWindow->ProcEvents())
			return false;
		_pWindow->SwapBuffer();
		//(*_ppScene)->Draw();
		//*_pContr->View = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//*_pContr->Projection = glm::perspective(glm::radians(45.0f), 800 / (float)800, 0.1f, 10.0f);
		//_pRenderer->UpdatePV(*_pContr->Projection * (*_pContr->View));
		_pRenderer->Draw();

		//gui->Draw();

		return true;
		
	}
	catch (std::exception err)
	{
		_pLog->Log<Logging::Window, Logging::Error>(err.what());
	}
	catch (const char* err)
	{
		_pLog->Log<Logging::Window, Logging::Error>(err);
	}
	catch (std::string err)
	{
		_pLog->Log<Logging::Window, Logging::Error>(err.c_str());
	}
}

double Application::GetTime()
{
	return chron.GetTime();
}

} // namespace Multor