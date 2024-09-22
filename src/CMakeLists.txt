cmake_minimum_required (VERSION 3.8)

project (Multor)
set(CMAKE_CXX_STANDARD 17)

#Searching sourse files
file(GLOB_RECURSE SOURCE_FILES
				${CMAKE_CURRENT_SOURCE_DIR}/*.c
				${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vulkan)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/scene_objects)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gui)

##########################Glm##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glm/)
##########################STB##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/stb/)
##########################SDL##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/sdl/include/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gui)
##########################Vulkan###########################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vulkan)
include_directories(${VULKAN_SDK_INCLUDE_DIR})
message(;rolijgdogjero;gjreorej ${CMAKE_BINARY_DIR}/dependencies/VulkanSDK)
include_directories(${CMAKE_BINARY_DIR}/dependencies/VulkanSDK)

if(WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
else()
	add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

# Define the link libraries
target_link_libraries(${PROJECT_NAME} ${LIBS} vulkan_subsystem)
