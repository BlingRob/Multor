/// \file render_engine.h

#pragma once
#ifndef RENDERENGINE_H
#define RENDERENGINE_H

#include "vulkan/renderer.h"

#include "gui/window.h"
#include "scene_objects/light_manager.h"
#include "transformation.h"
#include "utils/time.h"
#include "logger/logger.h"
#include "configure.h"

#include <array>
#include <functional>
#include <string>

#include <toml.hpp>

namespace Multor
{

class Application
{
public:
    Application();
    ~Application();

    bool                            MainLoop();
    double                          GetTime();
    std::shared_ptr<Vulkan::Renderer> GetRenderer();
    void AddLight(std::shared_ptr<BLight> light);
    void ClearLights();
    void InvalidateShadows();
    //void SetScen(std::unique_ptr<Scene>);
    //Scene* GetScen() const;

private:

    toml::table table_;

    /// @brief Controller
    std::shared_ptr<PositionController> pContr_;
    //Pointer of window class
    std::shared_ptr<Window> pWindow_;
    //Vulkan
    std::shared_ptr<Vulkan::Renderer> pRenderer_;
    //Scene
    std::shared_ptr<LightManager> pLights_;
    //std::shared_ptr<std::unique_ptr<Scene>> _ppScene;
    //Time
    Chronometr chron_;
    //GUI
    //std::unique_ptr<GUI> gui;

    //Resource manager
    //std::unique_ptr<ResourceManager> _pResMgr;

    //SignalsTable
    std::array<std::function<void(void*)>, 5> signals_;

    void SyncLightsToRenderer();
};

} // namespace Multor

#endif // RENDERENGINE_H
