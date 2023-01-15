cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project("Dependencies")

#
# Removes the specified compile flag from the specified target.
#   _target     - The target to remove the compile flag from
#   _flag       - The compile flag to remove
#
# Pre: apply_global_cxx_flags_to_all_targets() must be invoked.
#
macro(remove_flag_from_target _target _flag)
    get_target_property(_target_cxx_flags ${_target} COMPILE_OPTIONS)
    if(_target_cxx_flags)
        list(REMOVE_ITEM _target_cxx_flags ${_flag})
        set_target_properties(${_target} PROPERTIES COMPILE_OPTIONS "${_target_cxx_flags}")
    endif()
endmacro()

###################SDL#################################
add_subdirectory(sdl ${CMAKE_CURRENT_BINARY_DIR}/sdl)
###################STB#################################
add_subdirectory(stb ${CMAKE_CURRENT_BINARY_DIR}/stb)
##########################Assimp build#####################
add_subdirectory(assimp ${CMAKE_CURRENT_BINARY_DIR}/assimp)

set_target_properties(assimp
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/libs"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/libs"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/exe"
    ASSIMP_BUILD_TESTS OFF
    ASSIMP_NO_EXPORT ON
    ASSIMP_BUILD_ASSIMP_TOOLS OFF
    ASSIMP_INSTALL OFF
    ASSIMP_SHARED_LIBS ON
)

############################SQLite############################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sqlite ${CMAKE_CURRENT_BINARY_DIR}/sqlite)
###########################################################

###########################VULKAN##########################
add_subdirectory(VulkanSDK ${CMAKE_CURRENT_BINARY_DIR}/VulkanSDK)
find_package(VulkanSDK REQUIRED HINTS ${CMAKE_CURRENT_BINARY_DIR}/VulkanSDK/cmake/)

# ###########################################################
set_target_properties(${SDL3} stb assimp SQLLite
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/exe 
)

######################Build imgui##########################
#copy configuration
#file(COPY ${CMAKE_SOURCE_DIR}/config/imconfig.h
#     		  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.h
#		  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl.h
#		  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
#		  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl.cpp
#		  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
#collect imgui's source files 
#file(GLOB IMGUI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h)
#file(GLOB IMGUI_SOURCE  ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp)
#create library
#add_library(IMGUI STATIC ${IMGUI_HEADERS} ${IMGUI_SOURCE})
###########################################################

# Put all libraries into a variable
if(UNIX)
    set(LIBS assimp SDL3 SQLLite ${VULKAN_LIBS} ${VULKAN_SHARED_LIBS} stb CACHE STRING "Libraries")
else()
    set(LIBS assimp SDL3 SQLLite ${VULKAN_LIBS} stb CACHE STRING "Libraries")
endif()

if(UNIX)
    list(APPEND CMAKE_CXX_FLAGS "-Wno-error ${CMAKE_CXX_FLAGS}")
    remove_flag_from_target(assimp -Werror)
endif()

