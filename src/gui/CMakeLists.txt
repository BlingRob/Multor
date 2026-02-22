cmake_minimum_required (VERSION 3.20)

project (gui_subsystem)
set(CMAKE_CXX_STANDARD 23)

SET(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_overlay.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_vulkan.cpp)

##########################Glm##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glm/)
##########################SDL##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/sdl/include/)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/imgui/)
include_directories(${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
