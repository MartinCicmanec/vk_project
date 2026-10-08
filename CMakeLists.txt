cmake_minimum_required (VERSION 3.10.1)
#set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "Release" FORCE)
set(CMAKE_VERBOSE_MAKEFILE 0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SUPPRESS_REGENERATION true)
project(vk_project)
# The version number.
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)

if( CMAKE_BUILD_TYPE STREQUAL "" )
	set( CMAKE_BUILD_TYPE "debug" )
endif()

if( NOT DEFINED VK_USE_PLATFORM )
	if( WIN32 )
		set( VK_USE_PLATFORM "WIN32" )
	elseif( UNIX )
		set( VK_USE_PLATFORM "WAYLAND" )
	endif()
endif()

#SET(export_args "VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation:VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation")
#execute_process(COMMAND export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation:VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation )
#add_custom_command(TARGET main PRE_BUILD COMMAND export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation:VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation)
###set(ENV{VK_INSTANCE_LAYERS} VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation:VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation)

#execute_process (
#    COMMAND export "VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation:VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation"
#)
###EXECUTE_PROCESS(COMMAND sh -c "export VK_INSTANCE_LAYERS=VK_LAYER_LUNARG_standard_validation:VK_LAYER_LUNARG_api_dump:VK_LAYER_LUNARG_core_validation")

message("Environment variable: $ENV{VK_INSTANCE_LAYERS}")

if( VK_USE_PLATFORM STREQUAL "WIN32" )
	set( PLATFORM_LIBRARY "" )
elseif( VK_USE_PLATFORM STREQUAL "XCB" )
	set( PLATFORM_LIBRARY dl xcb )
elseif( VK_USE_PLATFORM STREQUAL "XLIB" )
	set( PLATFORM_LIBRARY dl X11 )
elseif( VK_USE_PLATFORM STREQUAL "WAYLAND" )
	set( PLATFORM_LIBRARY dl wayland-client )
endif()

if( MSVC )
	# Sources contain UTF-8 characters (license headers)
	add_compile_options( /utf-8 )
	# Keep Windows.h from defining min/max macros and pulling in rarely used APIs
	add_definitions( -DNOMINMAX -DWIN32_LEAN_AND_MEAN )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "debug" )
	add_definitions(-D_DEBUG)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions( -DVK_NO_PROTOTYPES )
add_definitions( -DVK_USE_PLATFORM_${VK_USE_PLATFORM}_KHR )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

message(STATUS "Debug? ${CMAKE_BUILD_TYPE}")

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found lib: Core=${Vulkan_LIBRARIES} version=${Vulkan_VERSION}")

find_package(glfw3 REQUIRED)

# Header-only vector / matrix math
find_package(glm CONFIG REQUIRED)

# Header-only image loading (stb_image.h). Arch installs it to /usr/include/stb, vcpkg to its include directory.
find_path(STB_INCLUDE_DIR NAMES stb_image.h PATH_SUFFIXES stb)
if( NOT STB_INCLUDE_DIR )
	message(FATAL_ERROR "stb_image.h not found - install the 'stb' package (Arch) or 'stb' with vcpkg (Windows)")
endif()
message(STATUS "glfw3 found lib: Core=${glfw3_LIBRARIES} version=${glfw3_VERSION}")

file(GLOB MAIN_SOURCES "src/*.cpp")
file(GLOB HEADER_FILES "include/*.h" "include/*.inl")

add_executable(main ${MAIN_SOURCES})

# The Vulkan loader is loaded at runtime (LoadLibrary / dlopen), so it is not linked here.
target_link_libraries(main PUBLIC ${PLATFORM_LIBRARY} glfw glm::glm)

# Vulkan's clip space depth range is 0..1, GLM's default (from OpenGL) is -1..1.
# Defined for the whole target, so every file sees the same GLM configuration.
# GLM_ENABLE_EXPERIMENTAL is needed for glm/gtx/hash.hpp (hashing of GLM types for vertex deduplication).
target_compile_definitions(main PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_ENABLE_EXPERIMENTAL)

# Vulkan headers come from the installed Vulkan SDK / vulkan-headers package
target_include_directories(main PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
    ${Vulkan_INCLUDE_DIRS}
    ${STB_INCLUDE_DIR}
)

