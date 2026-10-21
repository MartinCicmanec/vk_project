@echo off
rem Compiles the GLSL shaders in this directory to SPIR-V (uses glslc from the Vulkan SDK)
rem Pass "nopause" to skip the pause at the end (used by the compile.bat in the project root)
cd /d "%~dp0"
"%VULKAN_SDK%\Bin\glslc.exe" shader.vert -o vert.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" shader.frag -o frag.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" particle.comp -o particle_comp.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" particle.vert -o particle_vert.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" particle.frag -o particle_frag.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" water.vert -o water_vert.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" water.frag -o water_frag.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" sky.vert -o sky_vert.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" sky.frag -o sky_frag.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" object.vert -o object_vert.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" object.frag -o object_frag.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" ripple.comp -o ripple_comp.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" splash.comp -o splash_comp.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" splash.vert -o splash_vert.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" splash.frag -o splash_frag.spv

:end
set result=%errorlevel%
if not "%~1"=="nopause" pause
exit /b %result%
