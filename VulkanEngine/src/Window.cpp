/// \file Window.cpp
#include "Window.h"

bool Window::InitVulkanContext()
{
	#ifdef unix
		return SDL_Vulkan_LoadLibrary("./libvulkan.so") == 0;
	#elif _WIN32
		return SDL_Vulkan_LoadLibrary("./vulkan-1.dll") == 0;
	#endif
}

Window::Window(std::array<std::function<void(void*)>, 5>* sigs, std::shared_ptr<Position_Controller> contr)
{
	_pContr = contr;
	_pSigTable = sigs;
	//Mouse setup
	lastX = SDL_WINDOWPOS_CENTERED,
		lastY = SDL_WINDOWPOS_CENTERED;
	clicked = false;
	firstMouse = true;
	//SDL init
	SDL_SetMainReady();
	
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) == -1)
		throw(std::string("Failed SDL init ") + SDL_GetError());

	if (!InitVulkanContext())
		throw(std::string("Vulkan context wasn't loaded:") + SDL_GetError());

	//Creating SDL window
	_pWindow = std::shared_ptr<SDL_Window>(SDL_CreateWindow("Vulkan test window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE),
		[](SDL_Window* ptr) {SDL_DestroyWindow(ptr); });
	if (!_pWindow)
		throw std::runtime_error(std::string("Failed to create window!")  + SDL_GetError());
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
		case SDL_QUIT:
		{
			return false;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			if (e.button.button == SDL_BUTTON_LEFT && keys[SDL_SCANCODE_LCTRL])
			{
				clicked = true;
				lastX = static_cast<float>(e.button.x);
				lastY = static_cast<float>(e.button.y);
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				clicked = false;
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			if (clicked)
			{
				float xoffset = e.button.x - lastX;
				float yoffset = lastY - e.button.y;

				lastX = static_cast<float>(e.button.x);
				lastY = static_cast<float>(e.button.y);

				_pContr->cam->ProcessMouseMovement(xoffset, yoffset);
				ChangedView = true;
				ChangedProj = true;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			if (clicked)
				_pContr->cam->ProcessMouseScroll(static_cast<float>(e.wheel.y));
			ChangedProj = true;
			break;
		}

		case SDL_KEYDOWN:
		{
			if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				return false;
			keys[e.key.keysym.scancode] = true;
			break;
		}
		case SDL_KEYUP:
		{
			keys[e.key.keysym.scancode] = false;
			break;
		}

		case SDL_WINDOWEVENT:
		{

			switch (e.window.event)
			{
			case SDL_WINDOWEVENT_SIZE_CHANGED:

				SCR_WIDTH = static_cast<uint32_t>(e.window.data1);
				SCR_HEIGHT = static_cast<uint32_t>(e.window.data2);
				((*_pSigTable)[0])(nullptr);
				ChangedProj = true;
				break;
				//App isn't working, take on sleep mode
			case SDL_WINDOWEVENT_MINIMIZED:
				SDL_PollEvent(&NextEvent);
				if (NextEvent.type == SDL_WINDOWEVENT && NextEvent.window.event != SDL_WINDOWEVENT_MAXIMIZED)
				{
					SDL_PushEvent(&e);
					std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
				}
				break;
			default:
				break;
			}
		}
		}
	}
	//Apply keys
	do_movement();
	change_matrixes();

	return true;
}

void Window::do_movement()
{
	// Camera controls
	if (keys[SDL_SCANCODE_W])
		_pContr->cam->ProcessKeyboard(Camera::Camera_Movement::FORWARD, _pContr->dt);
	if (keys[SDL_SCANCODE_S])
		_pContr->cam->ProcessKeyboard(Camera::Camera_Movement::BACKWARD, _pContr->dt);
	if (keys[SDL_SCANCODE_A])
		_pContr->cam->ProcessKeyboard(Camera::Camera_Movement::LEFT, _pContr->dt);
	if (keys[SDL_SCANCODE_D])
		_pContr->cam->ProcessKeyboard(Camera::Camera_Movement::RIGHT, _pContr->dt);
	ChangedView = true;
}

void Window::change_matrixes()
{
	if (ChangedProj)
	{
		*_pContr->Projection = glm::perspective(glm::radians(_pContr->cam->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 150.0f);
		ChangedProj = false;
	}
	if (ChangedView)
	{
		*_pContr->View = _pContr->cam->GetViewMatrix();
		ChangedView = false;
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
	return _pWindow.get();
}

void Window::SwapBuffer()
{
	SDL_UpdateWindowSurface(_pWindow.get());
}
