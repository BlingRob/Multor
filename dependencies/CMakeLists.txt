cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 23)
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

###################STB#################################
add_subdirectory(stb ${CMAKE_CURRENT_BINARY_DIR}/stb)

############################SQLite############################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sqlite ${CMAKE_CURRENT_BINARY_DIR}/sqlite)
###########################################################

###########################VULKAN##########################
add_subdirectory(VulkanSDK ${CMAKE_CURRENT_BINARY_DIR}/VulkanSDK)
find_package(VulkanSDK REQUIRED HINTS ${CMAKE_CURRENT_BINARY_DIR}/VulkanSDK/cmake/)

# ###########################################################
set_target_properties(stb SQLLite
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/exe 
)

# Put all libraries into a variable
if(UNIX)
    set(LIBS SQLLite ${VULKAN_LIBS} ${VULKAN_SHARED_LIBS} stb CACHE STRING "Libraries")
else()
    set(LIBS SQLLite ${VULKAN_LIBS} stb CACHE STRING "Libraries")
endif()

if(UNIX)
    list(APPEND CMAKE_CXX_FLAGS "-Wno-error ${CMAKE_CXX_FLAGS}")
endif()

