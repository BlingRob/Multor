cmake_minimum_required (VERSION 3.8)

#Searching sourse files
file(GLOB_RECURSE SOURCE_FILES
			src/*.c
			src/*.cpp)

file(GLOB_RECURSE HEADER_FILES
			src/*.h
			src/*.hpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

##########################Glm##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/glm/)
##########################STB##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/stb/)
##########################SDL##############################
include_directories(${CMAKE_SOURCE_DIR}/dependencies/sdl/include/)
##########################Vulkan###########################
include_directories(${VULKAN_SDK_INCLUDE_DIR})
link_directories(${VULKAN_SDK_LIBRARY_DIR})


if(WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES} ${HEADER_FILES})
else()
	add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
endif()

# Define the link libraries
target_link_libraries(${PROJECT_NAME} ${LIBS})

if(UNIX)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy 
							${VULKAN_SHARED_LIB}
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libvulkan.so
					COMMENT "Coping shared library file"
	)
elseif(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy 
							${VULKAN_SHARED_LIB}
							${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}
					COMMENT "Coping shared library file"
					)
endif()

message("Install folder with shaders and coping its")
file(COPY ${CMAKE_SOURCE_DIR}/shaders
          DESTINATION ${CMAKE_BINARY_DIR})
