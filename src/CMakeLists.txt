cmake_minimum_required (VERSION 3.8)

project (Multor)
set(CMAKE_CXX_STANDARD 23)

find_package(quill REQUIRED CONFIG)
find_package(tomlplusplus REQUIRED CONFIG)
find_package(imgui REQUIRED CONFIG)
find_package(assimp REQUIRED CONFIG)
find_package(SDL3 REQUIRED CONFIG)
find_package(glm)

#Searching sourse files
file(GLOB_RECURSE SOURCE_FILES
				${CMAKE_CURRENT_SOURCE_DIR}/*.c
				${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Conan imgui package exports Vulkan backend files in res/bindings, not in include/.
# Pick them up when available (fallback to vendored Dear ImGui layout if present).
find_path(IMGUI_BACKEND_INCLUDE_DIR
          NAMES imgui_impl_vulkan.h
          HINTS
              ${imgui_PACKAGE_FOLDER_DEBUG}/res/bindings
              ${imgui_PACKAGE_FOLDER_RELEASE}/res/bindings
              ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends)

find_file(IMGUI_IMPL_VULKAN_SOURCE
          NAMES imgui_impl_vulkan.cpp
          HINTS
              ${imgui_PACKAGE_FOLDER_DEBUG}/res/bindings
              ${imgui_PACKAGE_FOLDER_RELEASE}/res/bindings
              ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends)

set(IMGUI_SOURCES)
if(IMGUI_IMPL_VULKAN_SOURCE)
    list(APPEND IMGUI_SOURCES ${IMGUI_IMPL_VULKAN_SOURCE})
else()
    message(WARNING "imgui_impl_vulkan.cpp not found; ImGui Vulkan backend will fail to link")
endif()

##########################STB##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/stb/)
##########################Vulkan###########################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan)
include_directories(${VULKAN_SDK_INCLUDE_DIR})
include_directories(${CMAKE_BINARY_DIR}/dependencies/VulkanSDK)

if(WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${IMGUI_SOURCES})
else()
	add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${IMGUI_SOURCES})
endif()

# Define the link libraries
target_link_libraries(${PROJECT_NAME}
    stb
    SQLLite
    vulkan_subsystem
    quill::quill
    tomlplusplus::tomlplusplus
    imgui::imgui
    assimp::assimp
    SDL3::SDL3
    glm::glm)

if(IMGUI_BACKEND_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_BACKEND_INCLUDE_DIR})
else()
    message(WARNING "imgui_impl_vulkan.h not found; ImGui overlay sources may fail to compile")
endif()
