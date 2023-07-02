# Multor

Multor - Modeling vULkan commoTOR.

This is my project by learning the Vulkan, the computer graphics, visualization interesting physics processes and etc.
It based on the [OpenGL](https://learnopengl.com/) and [Vulkan](https://vulkan-tutorial.com) tutorials. And on this stage it is really similarly them.

The goal this "iteration" is made compilation of shaders in application (without external binaries) and used shaders cache.

## How to build

```
cmake -DCMAKE_BUILD_TYPE={Release|Debug} dir_with_sources 
cmake --build .
```

If CMAKE_BUILD_TYPE wasn't setted, use Debug configuration.

## Format

Change directory on Multor/src and use next command (llvm should be installed):
```
clang-format -style=file -i *.cpp *.h 
```

## Dependencies

All dependencies will be loaded as submodules. Note bene: dependencies demand a lot of addition libraries and drivers. Their installation falls on the user's shoulders. I was really hoping to free the project from manually installing dependencies, but Vulkan is too complicated for that.