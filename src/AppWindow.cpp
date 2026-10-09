// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and / or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The below copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Vulkan Cookbook
// ISBN: 9781786468154
// © Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329
//
// Application Window

#if defined VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined VK_USE_PLATFORM_XLIB_KHR
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
    #define GLFW_EXPOSE_NATIVE_WAYLAND
#else
    #error "GLFW window setup is only implemented for the WIN32, XLIB and WAYLAND platforms."
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "AppWindow.h"
#include "Presentation.h"

namespace VulkanCookbook {

    bool AppWindow::Init( uint32_t     width,
                          uint32_t     height,
                          char const * title ) {
        TerminateGlfw.Function = []() { glfwTerminate(); };

        // The surface is created for one specific window system, so GLFW has to use the same one (GLFW 3.4+).
        // Without this an Xlib build started in a Wayland session gets a Wayland window and no X11 handles.
#if defined GLFW_PLATFORM
    #if defined VK_USE_PLATFORM_XLIB_KHR
        glfwInitHint( GLFW_PLATFORM, GLFW_PLATFORM_X11 );
    #elif defined VK_USE_PLATFORM_WAYLAND_KHR
        glfwInitHint( GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND );
    #endif
#endif

        if( glfwInit() != GLFW_TRUE ) {
            std::cout << "Could not initialize GLFW." << std::endl;
            return false;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GlfwWindow = glfwCreateWindow( static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr );
        if( !GlfwWindow ) {
            std::cout << "Could not create GLFW window." << std::endl;
            return false;
        }
        DestroyGlfwWindow.Function = [this]() { glfwDestroyWindow( GlfwWindow ); };

        // GLFW can't call a member function, so the callback gets this object through the window's user pointer
        glfwSetWindowUserPointer( GlfwWindow, this );
        glfwSetFramebufferSizeCallback( GlfwWindow, FramebufferResizeCallback );
        return true;
    }

    void AppWindow::FramebufferResizeCallback( GLFWwindow * window,
                                               int          /*width*/,
                                               int          /*height*/ ) {
        auto app_window = static_cast<AppWindow *>( glfwGetWindowUserPointer( window ) );
        app_window->FramebufferResized = true;
    }

    bool AppWindow::ShouldClose() const {
        return glfwWindowShouldClose( GlfwWindow ) != 0;
    }

    void AppWindow::PollEvents() const {
        glfwPollEvents();
    }

    void AppWindow::WaitEvents() const {
        glfwWaitEvents();
    }

    void AppWindow::GetFramebufferSize( uint32_t & width,
                                        uint32_t & height ) const {
        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize( GlfwWindow, &framebuffer_width, &framebuffer_height );
        width  = static_cast<uint32_t>(framebuffer_width);
        height = static_cast<uint32_t>(framebuffer_height);
    }

    bool AppWindow::WasResized() {
        bool resized = FramebufferResized;
        FramebufferResized = false;
        return resized;
    }

    bool AppWindow::CreateSurface( VkInstance     instance,
                                   VkSurfaceKHR & presentation_surface ) const {
        WindowParameters window_parameters;
#if defined VK_USE_PLATFORM_WIN32_KHR
        window_parameters.HInstance = GetModuleHandle(nullptr);
        window_parameters.HWnd = glfwGetWin32Window(GlfwWindow);
#elif defined VK_USE_PLATFORM_XLIB_KHR
        window_parameters.Dpy = glfwGetX11Display();
        window_parameters.Window = glfwGetX11Window(GlfwWindow);
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
        window_parameters.Display = glfwGetWaylandDisplay();
        window_parameters.Surface = glfwGetWaylandWindow(GlfwWindow);
#endif

        return CreatePresentationSurface( instance, window_parameters, presentation_surface );
    }

} // namespace VulkanCookbook
