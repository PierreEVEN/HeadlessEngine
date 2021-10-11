# Headless Engine

This is the prototype of a 3D game engine using the Vulkan API. The goal is to offer a straightforward but powerful game engine whith which you can quickly create a scene in a few lines of code.

I'm working on this project in my spare time and currently considere it as a learning project.

## Demo

![unknown](https://user-images.githubusercontent.com/24438631/136795565-0bfdb609-ef96-44de-9579-f8c0b2a79f83.png)
The code of the demo project is [here](src/tests/heGameTest/private/testGameInterface.cpp)

## Documentation

[Documentation](doc/README.md)

*This project is in active development. I'll post more informations about it later.*

## Compilation

*Does not works on linux and macos at the moment. (it should works but I needs to fix somes issues before)*

- Install the last vulkan SDK + CMake and all the MSVC stuffs for C++

- Execute `./Build.sh` (will download dependencies and generate project files)

- Then build & run. (project files are located into the `./temp` directory)

## Third party

[Vulkan](https://www.lunarg.com/vulkan-sdk/)

[Assimp](https://assimp.org/)

[Dear ImGui](https://github.com/ocornut/imgui) 

[ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) 

[glfw](https://www.glfw.org/) 

[glm](https://github.com/g-truc/glm) 

[glslang](https://github.com/KhronosGroup/glslang) 

[Spirv-Cross](https://github.com/KhronosGroup/SPIRV-Cross) 

[stb](https://github.com/nothings/stb) 

[Vulkan memory allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
