@echo off
rem Compiles the GLSL shaders in this directory to SPIR-V (uses glslc from the Vulkan SDK)
rem Pass "nopause" to skip the pause at the end (used by the compile.bat in the project root)
cd /d "%~dp0"
"%VULKAN_SDK%\Bin\glslc.exe" shader.vert -o vert.spv
if errorlevel 1 goto :end
"%VULKAN_SDK%\Bin\glslc.exe" shader.frag -o frag.spv

:end
set result=%errorlevel%
if not "%~1"=="nopause" pause
exit /b %result%
