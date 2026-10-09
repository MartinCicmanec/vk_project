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

#ifndef APP_WINDOW
#define APP_WINDOW

#include "Common.h"
#include "ScopeGuard.h"

struct GLFWwindow;

namespace VulkanCookbook {

    // The GLFW window the application renders to. Named AppWindow because X11 already has a global type called Window.
    // It has to outlive the presentation surface created for it.
    class AppWindow {
    public:
        AppWindow() = default;
        // GLFW keeps a pointer to this object for its callbacks, so it must not be copied or moved
        AppWindow( AppWindow const & ) = delete;
        AppWindow & operator=( AppWindow const & ) = delete;

        bool Init( uint32_t     width,
                   uint32_t     height,
                   char const * title );

        bool ShouldClose() const;
        void PollEvents() const;
        // Blocks until an event arrives (used while the window is minimized)
        void WaitEvents() const;
        // Size in pixels, which can differ from the size in screen coordinates on high DPI displays. 0 when minimized.
        void GetFramebufferSize( uint32_t & width,
                                 uint32_t & height ) const;
        // True once after the framebuffer size has changed. Not every platform reports a resize through
        // VK_ERROR_OUT_OF_DATE_KHR (Wayland doesn't), so this is the reliable signal.
        bool WasResized();
        // Creates a presentation surface for the window system this program was built for
        bool CreateSurface( VkInstance     instance,
                            VkSurfaceKHR & presentation_surface ) const;

    private:
        static void FramebufferResizeCallback( GLFWwindow * window,
                                               int          width,
                                               int          height );

        ScopeGuard     TerminateGlfw;
        GLFWwindow   * GlfwWindow = nullptr;
        ScopeGuard     DestroyGlfwWindow;
        bool           FramebufferResized = false;
    };

} // namespace VulkanCookbook

#endif // APP_WINDOW
