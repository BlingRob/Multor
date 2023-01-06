/// \file RenderEngine.cpp
#include "RenderEngine.h"

Application::Application() 
{	
	_pLog = std::make_unique<Logger>();
	try
	{
		_pContr = std::make_shared<Position_Controller>();
		_pWindow = std::make_shared<Window>(&signals,_pContr);
		_pRenderer = std::make_shared<VulkanRenderer>(_pWindow);

		signals[0] = std::bind(&VulkanRenderer::Update, _pRenderer);
	}
	catch (std::exception err)
	{
		_pLog->ExceptionMSG(err);
	}
	catch (const char* err)
	{
		_pLog->ExceptionMSG(err);
	}
	catch (std::string err)
	{
		_pLog->ExceptionMSG(err);
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
		
	}
	catch (std::exception err)
	{
		_pLog->ExceptionMSG(err);
	}
	catch (const char* err)
	{
		_pLog->ExceptionMSG(err);
	}
	catch (std::string err)
	{
		_pLog->ExceptionMSG(err);
	}
	return true;
}

double Application::GetTime()
{
	return chron.GetTime();
}