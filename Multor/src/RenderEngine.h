/// \file RenderEngine.h

#pragma once
#ifndef RENDERENGINE_H
#define RENDERENGINE_H

#include <array>
#include <functional>
#include "Window.h"
#include "Transformation.h"
#include "VulkanRenderer.h"
#include "Time.h"
#include "Logger.h"

namespace Multor
{

class Application
{
public:
    Application();
    //~Application();

    bool                            MainLoop();
    double                          GetTime();
    std::shared_ptr<VulkanRenderer> GetRenderer();
    //void SetScen(std::unique_ptr<Scene>);
    //Scene* GetScen() const;
private:
    std::shared_ptr<Position_Controller> pContr_;
    //Pointer of window class
    std::shared_ptr<Window> pWindow_;
    //Vulkan
    std::shared_ptr<VulkanRenderer> _pRenderer;
    //Scene
    //std::shared_ptr<std::unique_ptr<Scene>> _ppScene;
    //Time
    Chronometr chron;
    //GUI
    //std::unique_ptr<GUI> gui;
    //Logger
    std::shared_ptr<Logging::Logger> _pLog;
    //Resource manager
    //std::unique_ptr<ResourceManager> _pResMgr;

    //SignalsTable
    std::array<std::function<void(void*)>, 5> signals;
};

} // namespace Multor

#endif // RENDERENGINE_H