/// \file window.cpp

#include "Window.h"

#include <chrono>
#include <thread>
#include <string>

namespace Multor
{

bool Window::InitVulkanContext()
{
#ifdef unix
    return SDL_Vulkan_LoadLibrary("./libvulkan.so");
#elif _WIN32
    return SDL_Vulkan_LoadLibrary("./vulkan-1.dll");
#endif
}

Window::Window(std::array<std::function<void(void*)>, 5>* sigs,
               std::shared_ptr<PositionController>       contr)
{
    pContr_    = contr;
    pSigTable_ = sigs;
    //Mouse setup
    lastX_ = SDL_WINDOWPOS_CENTERED, lastY_ = SDL_WINDOWPOS_CENTERED;
    clicked_    = false;
    firstMouse_ = true;
    //SDL init
    //SDL_SetMainReady();

    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO))
        throw(std::string("Failed SDL init ") + SDL_GetError());

    if (!InitVulkanContext())
        throw(std::string("Vulkan context wasn't loaded:") + SDL_GetError());

    //Creating SDL window
    pWindow_ = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow("Vulkan test window", 800, 800,
                         SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE),
        [](SDL_Window* ptr) { SDL_DestroyWindow(ptr); });
    if (!pWindow_)
        throw std::runtime_error(std::string("Failed to create window!") +
                                 SDL_GetError());

    changedView_ = true;
    changedProj_ = true;
    changeMatrices();
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
            if (eventInterceptor_)
                eventInterceptor_(e);
            //ImGui_ImplSDL2_ProcessEvent(&e);
            switch (e.type)
                {
                        case SDL_EVENT_QUIT: {
                            return false;
                        }
                        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                            if (e.button.button == SDL_BUTTON_LEFT &&
                                keys_[SDL_SCANCODE_LCTRL])
                                {
                                    clicked_ = true;
                                    lastX_   = static_cast<float>(e.button.x);
                                    lastY_   = static_cast<float>(e.button.y);
                                    SDL_SetWindowRelativeMouseMode(pWindow_.get(), true);
                                }
                            break;
                        }
                        case SDL_EVENT_MOUSE_BUTTON_UP: {
                            if (e.button.button == SDL_BUTTON_LEFT)
                                {
                                    clicked_ = false;
                                    SDL_SetWindowRelativeMouseMode(pWindow_.get(), false);
                                }
                            break;
                        }
                        case SDL_EVENT_MOUSE_MOTION: {
                            if (clicked_)
                                {
                                    float xoffset = static_cast<float>(e.motion.x) - lastX_;
                                    float yoffset = lastY_ - static_cast<float>(e.motion.y);

                                    lastX_ = static_cast<float>(e.motion.x);
                                    lastY_ = static_cast<float>(e.motion.y);

                                    pContr_->cam_->ProcessMouseMovement(xoffset,
                                                                       yoffset);
                                    changedView_ = true;
                                    changedProj_ = true;
                                }
                            break;
                        }
                        case SDL_EVENT_MOUSE_WHEEL: {
                            if (clicked_)
                                {
                                    pContr_->cam_->ProcessMouseScroll(
                                        static_cast<float>(e.wheel.y));
                                    changedProj_ = true;
                                }
                            break;
                        }

                        case SDL_EVENT_KEY_DOWN: {
                            if (e.key.scancode == SDL_SCANCODE_ESCAPE)
                                return false;
                            keys_[e.key.scancode] = true;
                            break;
                        }
                        case SDL_EVENT_KEY_UP: {
                            keys_[e.key.scancode] = false;
                            break;
                        }

                    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:

                        scrWidth_  = static_cast<uint32_t>(e.window.data1);
                        scrHeight_ = static_cast<uint32_t>(e.window.data2);
                        ((*pSigTable_)[0])(nullptr);
                        changedProj_ = true;
                        break;
                        //App isn't working, take on sleep mode
                    case SDL_EVENT_WINDOW_MINIMIZED:
                        SDL_PollEvent(&NextEvent);
                        if (NextEvent.type != SDL_EVENT_WINDOW_MAXIMIZED)
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
    doMovement();
    changeMatrices();

    return true;
}

void Window::doMovement()
{
    bool moved = false;
    // Camera controls
    if (keys_[SDL_SCANCODE_W])
        {
            pContr_->cam_->ProcessKeyboard(Camera::Camera_Movement::FORWARD,
                                           pContr_->dt_);
            moved = true;
        }
    if (keys_[SDL_SCANCODE_S])
        {
            pContr_->cam_->ProcessKeyboard(Camera::Camera_Movement::BACKWARD,
                                           pContr_->dt_);
            moved = true;
        }
    if (keys_[SDL_SCANCODE_A])
        {
            pContr_->cam_->ProcessKeyboard(Camera::Camera_Movement::LEFT,
                                           pContr_->dt_);
            moved = true;
        }
    if (keys_[SDL_SCANCODE_D])
        {
            pContr_->cam_->ProcessKeyboard(Camera::Camera_Movement::RIGHT,
                                           pContr_->dt_);
            moved = true;
        }
    if (keys_[SDL_SCANCODE_SPACE])
        {
            pContr_->cam_->ProcessKeyboard(Camera::Camera_Movement::UP,
                                           pContr_->dt_);
            moved = true;
        }
    if (keys_[SDL_SCANCODE_LSHIFT])
        {
            pContr_->cam_->ProcessKeyboard(Camera::Camera_Movement::DOWN,
                                           pContr_->dt_);
            moved = true;
        }

    if (moved)
        changedView_ = true;
}

void Window::changeMatrices()
{
    if (changedProj_)
        {
            pContr_->UpdateProjectionMatrix(static_cast<float>(scrWidth_),
                                            static_cast<float>(scrHeight_));
            changedProj_ = false;
        }
    if (changedView_)
        {
            pContr_->UpdateViewMatrix();
            changedView_   = false;
        }
}

std::pair<int32_t, int32_t> Window::MaxSize() const
{
    const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
    if (dm == nullptr)
        return std::make_pair(800, 800);
    return std::make_pair(dm->w, dm->h);
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
