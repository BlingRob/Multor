/// \file Window.cpp

#include "Window.h"

#include <chrono>
#include <thread>
#include <string>

namespace Multor
{

bool Window::InitVulkanContext()
{
#ifdef unix
    return SDL_Vulkan_LoadLibrary("./libvulkan.so") == 0;
#elif _WIN32
    return SDL_Vulkan_LoadLibrary("./vulkan-1.dll") == 0;
#endif
}

Window::Window(std::array<std::function<void(void*)>, 5>* sigs,
               std::shared_ptr<Position_Controller>       contr)
{
    pContr_    = contr;
    pSigTable_ = sigs;
    //Mouse setup
    lastX_ = SDL_WINDOWPOS_CENTERED, lastY_ = SDL_WINDOWPOS_CENTERED;
    clicked_    = false;
    firstMouse_ = true;
    //SDL init
    //SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) == -1)
        throw(std::string("Failed SDL init ") + SDL_GetError());

    if (!InitVulkanContext())
        throw(std::string("Vulkan context wasn't loaded:") + SDL_GetError());

    //Creating SDL window
    pWindow_ = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow("Vulkan test window", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 800, 800,
                         SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE),
        [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); });
    if (!pWindow_)
        throw std::runtime_error(std::string("Failed to create window!") +
                                 SDL_GetError());
}

Window::~Window()
{
    SDL_Vulkan_UnloadLibrary();
}

bool Window::ProcEvents()
{
    SDL_Event e, NextEvent;
    while (SDL_PollEvent(&e))
        {
            //ImGui_ImplSDL2_ProcessEvent(&e);
            switch (e.type)
                {
                        case SDL_QUIT: {
                            return false;
                        }
                        case SDL_MOUSEBUTTONDOWN: {
                            if (e.button.button == SDL_BUTTON_LEFT &&
                                keys_[SDL_SCANCODE_LCTRL])
                                {
                                    clicked_ = true;
                                    lastX_   = static_cast<float>(e.button.x);
                                    lastY_   = static_cast<float>(e.button.y);
                                    SDL_SetRelativeMouseMode(SDL_TRUE);
                                }
                            break;
                        }
                        case SDL_MOUSEBUTTONUP: {
                            if (e.button.button == SDL_BUTTON_LEFT)
                                {
                                    clicked_ = false;
                                    SDL_SetRelativeMouseMode(SDL_FALSE);
                                }
                            break;
                        }
                        case SDL_MOUSEMOTION: {
                            if (clicked_)
                                {
                                    float xoffset = e.button.x - lastX_;
                                    float yoffset = lastY_ - e.button.y;

                                    lastX_ = static_cast<float>(e.button.x);
                                    lastY_ = static_cast<float>(e.button.y);

                                    pContr_->cam->ProcessMouseMovement(xoffset,
                                                                       yoffset);
                                    changedView_ = true;
                                    changedProj_ = true;
                                }
                            break;
                        }
                        case SDL_MOUSEWHEEL: {
                            if (clicked_)
                                pContr_->cam->ProcessMouseScroll(
                                    static_cast<float>(e.wheel.y));
                            changedProj_ = true;
                            break;
                        }

                        case SDL_KEYDOWN: {
                            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                                return false;
                            keys_[e.key.keysym.scancode] = true;
                            break;
                        }
                        case SDL_KEYUP: {
                            keys_[e.key.keysym.scancode] = false;
                            break;
                        }

                    case SDL_WINDOWEVENT_SIZE_CHANGED:

                        SCR_WIDTH  = static_cast<uint32_t>(e.window.data1);
                        SCR_HEIGHT = static_cast<uint32_t>(e.window.data2);
                        ((*pSigTable_)[0])(nullptr);
                        changedProj_ = true;
                        break;
                        //App isn't working, take on sleep mode
                    case SDL_WINDOWEVENT_MINIMIZED:
                        SDL_PollEvent(&NextEvent);
                        if (NextEvent.type != SDL_WINDOWEVENT_MAXIMIZED)
                            {
                                SDL_PushEvent(&e);
                                std::this_thread::sleep_for(
                                    std::chrono::duration<double, std::milli>(
                                        100));
                            }
                        break;
                    default:
                        break;
                }
        }
    //Apply keys_
    do_movement();
    change_matrixes();

    return true;
}

void Window::do_movement()
{
    // Camera controls
    if (keys_[SDL_SCANCODE_W])
        pContr_->cam->ProcessKeyboard(Camera::Camera_Movement::FORWARD,
                                      pContr_->dt);
    if (keys_[SDL_SCANCODE_S])
        pContr_->cam->ProcessKeyboard(Camera::Camera_Movement::BACKWARD,
                                      pContr_->dt);
    if (keys_[SDL_SCANCODE_A])
        pContr_->cam->ProcessKeyboard(Camera::Camera_Movement::LEFT,
                                      pContr_->dt);
    if (keys_[SDL_SCANCODE_D])
        pContr_->cam->ProcessKeyboard(Camera::Camera_Movement::RIGHT,
                                      pContr_->dt);
    changedView_ = true;
}

void Window::change_matrixes()
{
    if (changedProj_)
        {
            *pContr_->Projection = glm::perspective(
                glm::radians(pContr_->cam->Zoom),
                (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 150.0f);
            changedProj_ = false;
        }
    if (changedView_)
        {
            *pContr_->View = pContr_->cam->GetViewMatrix();
            changedView_   = false;
        }
}

std::pair<int32_t, int32_t> Window::MaxSize() const
{
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    return std::make_pair(DM.w, DM.h);
}

SDL_Window* Window::GetWindow() const
{
    return pWindow_.get();
}

void Window::SwapBuffer()
{
    SDL_UpdateWindowSurface(pWindow_.get());
}

} // namespace Multor