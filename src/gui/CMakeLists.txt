cmake_minimum_required (VERSION 3.20)

project (gui_subsystem)
set(CMAKE_CXX_STANDARD 23)

SET(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/window.cpp)

##########################Glm##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glm/)
##########################SDL##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/sdl/include/)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
