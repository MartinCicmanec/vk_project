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
// Application

#ifndef APPLICATION
#define APPLICATION

#include "Common.h"
#include "AppWindow.h"
#include "VulkanContext.h"
#include "SwapchainTargets.h"
#include "Renderer.h"
#include "ParticleRenderer.h"
#include "WaterRenderer.h"

namespace VulkanCookbook {

    // Which renderer the application runs
    enum class DemoScene {
        Model,          // textured Viking room (the main tutorial chapters)
        Particles,      // compute shader particle system
        Water           // ocean with waves
    };

    // Owns the parts of the program and runs the main loop
    class Application {
    public:
        // Upper limit for multisample anti-aliasing - the device may support less (but always at least 4)
        static constexpr VkSampleCountFlagBits MAX_MSAA_SAMPLES = VK_SAMPLE_COUNT_4_BIT;

        Application( bool        enable_validation,
                     bool        enable_debug_callbacks,
                     DemoScene   scene );
        ~Application();

        bool Init();
        bool Run();

    private:
        bool               EnableValidation;
        bool               EnableDebugCallbacks;
        DemoScene          Scene;

        // Members are destroyed in reverse order of declaration, and every part needs the ones before it:
        // renderer -> swapchain targets -> Vulkan context (allocator, device, surface, instance) -> window
        AppWindow          MainWindow;
        VulkanContext      Context;
        SwapchainTargets   Targets;
        // Only one of the renderers is initialized and used - an uninitialized one holds no Vulkan objects
        Renderer           SceneRenderer;
        ParticleRenderer   Particles;
        WaterRenderer      Water;
    };

} // namespace VulkanCookbook

#endif // APPLICATION
