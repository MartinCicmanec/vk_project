# vk_project
A dabble into the world of Vulkan API with the help of the [Vulkan Cookbook](https://www.oreilly.com/library/view/vulkan-cookbook/9781786468154/).
Another resource [Vulkan Programming Guide](https://www.amazon.com/Vulkan-Programming-Guide-Official-Learning/dp/0134464540/ref=sr_1_1?crid=3AAXAG6X6UFC7&keywords=vulkan+for+beginners&qid=1663762161&s=digital-text&sprefix=vulkan+for+beginners%2Cdigital-text%2C129&sr=1-1-catcorr)

## Dependencies
sudo apt-get install libvulkan-dev vulkan-validationlayers-dev libglfw3-dev vulkan-sdk
yay -S vulkan-icd-loader vulkan-headers vulkan-validation-layers glfw shaderc glm stb vulkan-memory-allocator

https://vulkan.lunarg.com/sdk/home#linux

### Building on Linux
`./compile.sh` does a clean build (deletes `build/`, runs the commands below and compiles the shaders). Extra arguments are passed to CMake, e.g. `./compile.sh -DVK_USE_PLATFORM=XLIB`. Manual steps:
```
cmake -S . -B build
cmake --build build
./shaders/compile.sh
./bin/main
```
Run `./shaders/compile.sh` again after changing a shader. The program loads `shaders/*.spv` relative to the working directory, so start it from the project root.
The window system defaults to Wayland. For an X11 session configure with `-DVK_USE_PLATFORM=XLIB`.

### Usage
```
./bin/main [-v] [-d] [-p | -w]
```
| Option | Description |
|---|---|
| `-v`, `--verbose` | Enable the `VK_LAYER_KHRONOS_validation` layer (requires the validation layers to be installed). |
| `-d`, `--debug` | Register a debug report callback that prints layer messages. Use together with `-v`. |
| `-p`, `--particles` | Show the particle system of the "Compute Shader" chapter instead of the model. |
| `-w`, `--water` | Show the ocean scene instead of the model. Can't be combined with `-p`. |

## Windows

### Dependencies
1. [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload (includes MSVC and CMake).
2. [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) - the installer sets the `VULKAN_SDK` environment variable (needed by CMake) and provides the validation layers.
3. An up to date GPU driver - it provides `vulkan-1.dll`, which the program loads at runtime.
4. [vcpkg](https://github.com/microsoft/vcpkg) for GLFW, GLM, stb and Vulkan Memory Allocator.

### Environment setup
In a *Developer PowerShell for VS 2022*:
```
git clone https://github.com/microsoft/vcpkg C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
setx VCPKG_ROOT C:\vcpkg
C:\vcpkg\vcpkg install glfw3:x64-windows glm:x64-windows stb:x64-windows vulkan-memory-allocator:x64-windows
```
Open a new terminal afterwards so `VCPKG_ROOT` is picked up.

### Building on Windows
`compile.bat` does a clean build (deletes `build\`, runs the commands below and compiles the shaders). Manual steps:
```
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Debug
.\shaders\compile.bat
.\bin\Debug\main.exe
```
Run `shaders\compile.bat` again after changing a shader, and start the program from the project root.
The vcpkg toolchain copies `glfw3.dll` next to the executable.

## Assets
The model and its texture are not part of this repository (they are in `.gitignore`). Download them once from the project root before running the program:

Linux:
```
mkdir -p models textures
curl -L -o models/viking_room.obj https://vulkan-tutorial.com/resources/viking_room.obj
curl -L -o textures/viking_room.png https://vulkan-tutorial.com/resources/viking_room.png
```

Windows (PowerShell):
```
New-Item -ItemType Directory -Force models, textures
curl.exe -L -o models\viking_room.obj https://vulkan-tutorial.com/resources/viking_room.obj
curl.exe -L -o textures\viking_room.png https://vulkan-tutorial.com/resources/viking_room.png
```

- `models/viking_room.obj` (about 480 KB) and `textures/viking_room.png` (about 960 KB): [Viking room](https://sketchfab.com/3d-models/viking-room-a49f1b8e4f5c4ecf9e1fe7d81915ad38) by [nigelgoh](https://sketchfab.com/nigelgoh), licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), in the version adapted for [vulkan-tutorial.com](https://vulkan-tutorial.com/Loading_models). Without them the program stops with "Could not load model".
- `textures/texture.png`: generated test texture for checking texture orientation (part of the repository, currently unused).

## Third-party code
- `include/tiny_obj_loader.h`: [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) v2.0.0rc13 (MIT).

## Reference
[VKSpec](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#memory-allocation)