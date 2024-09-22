/// \file Windows.h

#pragma once
#ifndef WINDOWS_H
#define WINDOWS_H

#include "../Transformation.h"

#include <memory>
#include <stdexcept>
#include <functional>
#include <array>
#include <tuple>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace Multor
{

class Window
{
public:
    Window(std::array<std::function<void(void*)>, 5>* sigs,
           std::shared_ptr<Position_Controller>       contr);

    std::shared_ptr<SDL_Window> GetWindow()
    {
        return pWindow_;
    }

    bool                        ProcEvents();
    void                        SwapBuffer();
    SDL_Window*                 GetWindow() const;
    std::pair<int32_t, int32_t> MaxSize() const;
    //Window(std::shared_ptr<Position_Controller>);

    //Size of window
    uint32_t SCR_WIDTH = 800, SCR_HEIGHT = 800;

    ~Window();

private:
    //window
    std::shared_ptr<SDL_Window> pWindow_;
    //Signals
    std::array<std::function<void(void*)>, 5>* pSigTable_;
    bool                                       InitVulkanContext();

    //queue of pressed keys_
    bool keys_[1024] = {false};

    //mouse controll
    float lastX_, lastY_;
    bool  clicked_, firstMouse_;
    bool  changedProj_ = false;
    bool  changedView_ = false;
    //callbacks
    inline void do_movement();
    inline void change_matrixes();

    //Data for connect with render engine
    std::shared_ptr<Position_Controller> pContr_;
};

} // namespace Multor

#endif // WINDOWS_H