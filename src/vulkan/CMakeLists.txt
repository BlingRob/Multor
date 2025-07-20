cmake_minimum_required (VERSION 3.10)

project (vulkan_subsystem)

file(GLOB_RECURSE SOURCES
			${CMAKE_CURRENT_SOURCE_DIR}/*.c
			${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${VULKAN_SDK_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${VULKAN_LIBS} quill::quill tomlplusplus::tomlplusplus)

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