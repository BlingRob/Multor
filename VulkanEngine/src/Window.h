/// \file Windows.h
#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <functional>
#include <array>
#include <tuple>
#include <chrono>
#include <thread>
#include "Transformation.h"


class Window
{
public:
	Window(std::array<std::function<void(void*)>, 5>* sigs, std::shared_ptr<Position_Controller> contr);

	std::shared_ptr<SDL_Window> GetWindow()
	{
		return _pWindow;
	}

	bool ProcEvents();
	void SwapBuffer();
	SDL_Window* GetWindow() const;
	std::pair<int32_t, int32_t> MaxSize() const;
	//Window(std::shared_ptr<Position_Controller>);

	//Size of window
	uint32_t SCR_WIDTH = 800,
		SCR_HEIGHT = 800;

	~Window();
private:
	//window
	std::shared_ptr<SDL_Window> _pWindow;
	//Signals
	std::array<std::function<void(void*)>, 5>* _pSigTable;
	bool InitVulkanContext();

	//queue of pressed keys
	bool keys[1024] = { false };

	//mouse controll
	float lastX, lastY;
	bool clicked, firstMouse;
	bool ChangedProj = false;
	bool ChangedView = false;
	//callbacks
	inline void do_movement();
	inline void change_matrixes();

	//Data for connect with render engine
	std::shared_ptr<Position_Controller> _pContr;
};