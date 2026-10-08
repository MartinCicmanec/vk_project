@echo off
rem Clean build: deletes the build folder, configures and builds the project, then compiles the shaders
rem Requires VCPKG_ROOT (GLFW) and VULKAN_SDK (glslc) to be set - see README.md
cd /d "%~dp0"

if not defined VCPKG_ROOT (
    echo VCPKG_ROOT is not set.
    goto :error
)

if exist build rmdir /s /q build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" %*
if errorlevel 1 goto :error
cmake --build build --config Debug
if errorlevel 1 goto :error
call shaders\compile.bat nopause
if errorlevel 1 goto :error

echo Build finished. Run bin\Debug\main.exe from this folder.
pause
exit /b 0

:error
echo Build failed.
pause
exit /b 1
