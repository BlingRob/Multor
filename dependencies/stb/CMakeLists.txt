cmake_minimum_required (VERSION 3.10)

project ("stb")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
############################STB############################
add_library(stb STATIC ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.cpp)
###########################################################