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
		set( VK_USE_PLATFORM "XCB" )
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

if( VK_USE_PLATFORM STREQUAL "VK_USE_PLATFORM_WIN32_KHR" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO    "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_RELEASE} /MT" )
	set( CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEBUG} /MTd" )
	set( CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} /MT" )
	set( CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} /MTd" )
	set( PLATFORM_LIBRARY                "" )
elseif( VK_USE_PLATFORM STREQUAL "XCB" )
	add_definitions( -std=c++11 )
	set( PLATFORM_LIBRARY dl xcb )
elseif( VK_USE_PLATFORM STREQUAL "XLIB" )
	add_definitions( -std=c++11 )
	set( PLATFORM_LIBRARY dl X11 )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "debug" )
	add_definitions(-D_DEBUG)
endif()

set(CMAKE_CXX_STANDARD 17)

add_definitions( -DVK_NO_PROTOTYPES )
add_definitions( -DVK_USE_PLATFORM_${VK_USE_PLATFORM}_KHR )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

message(STATUS "Debug? ${CMAKE_BUILD_TYPE}")

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan found lib: Core=${Vulkan_LIBRARIES} version=${Vulkan_VERSION}")

find_package(glfw3 REQUIRED)
message(STATUS "glfw3 found lib: Core=${glfw3_LIBRARIES} version=${glfw3_VERSION}")

file(GLOB MAIN_SOURCES "src/*.cpp")
file(GLOB HEADER_FILES "include/*.h" "include/*.inl")

add_executable(main ${MAIN_SOURCES})

if (WIN32)
    target_link_libraries(main PUBLIC )
endif (WIN32)
if (UNIX)
    target_link_libraries(main PUBLIC ${PLATFORM_LIBRARY} vulkan dl X11 glfw)
endif (UNIX)

target_include_directories(main PUBLIC
    ${PROJECT_SOURCE_DIR}/include/
	${PROJECT_SOURCE_DIR}/include/*.inl
)

