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
// Vulkan Context

#ifndef VULKAN_CONTEXT
#define VULKAN_CONTEXT

#include "Common.h"
#include "ScopeGuard.h"
#include "Allocator.h"

namespace VulkanCookbook {

    class AppWindow;

    // Everything that exists once for the whole run of the program: the Vulkan loader, instance, presentation surface,
    // selected physical device, logical device, its queue and the memory allocator.
    // Other parts of the application get this one object instead of a list of handles. It has to outlive everything
    // that was created with it, and the window has to outlive it (because of the surface).
    class VulkanContext {
    public:
        VulkanContext() = default;
        VulkanContext( VulkanContext const & ) = delete;
        VulkanContext & operator=( VulkanContext const & ) = delete;

        // max_msaa_samples is an upper limit - the device may support less (but always at least 4)
        bool Init( AppWindow const       & window,
                   bool                    enable_validation,
                   bool                    enable_debug_callbacks,
                   VkSampleCountFlagBits   max_msaa_samples );
        // Blocks until the GPU has finished everything that was submitted. Nothing may be destroyed while it is in use.
        void WaitIdle() const;

        // Members are destroyed in reverse order of declaration, so they are declared in the order they are created:
        // transfer command pool -> memory allocator -> device -> surface -> debug messenger -> instance -> Vulkan library
        LIBRARY_TYPE                                    VulkanLibrary = nullptr;
        ScopeGuard                                      ReleaseVulkanLibrary;

        VkDestroyer(VkInstance)                         Instance;
        VkDestroyer(VkDebugUtilsMessengerEXT)           DebugMessenger;
        VkDestroyer(VkSurfaceKHR)                       PresentationSurface;

        VkPhysicalDevice                                PhysicalDevice = VK_NULL_HANDLE;
        uint32_t                                        QueueFamilyIndex = 0;
        VkSampleCountFlagBits                           MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
        // Highest anisotropy the device allows for samplers
        float                                           MaxSamplerAnisotropy = 1.0f;
        // Point sizes other than 1.0 are supported (and enabled on the logical device)
        bool                                            LargePointsSupported = false;

        VkDestroyer(VkDevice)                           LogicalDevice;
        // Queue used for graphics and compute commands and presentation
        VkQueue                                         GraphicsAndPresentQueue = VK_NULL_HANDLE;

        // Memory allocator (VMA) for all buffers and images
        VmaAllocator                                    Allocator = VK_NULL_HANDLE;
        ScopeGuard                                      DestroyVmaAllocator;

        // For short-lived command buffers that upload data or change image layouts
        VkDestroyer(VkCommandPool)                      TransferCommandPool;

    private:
        bool InitVulkanLoader();
        bool CreateInstance( bool enable_validation );
        bool SetupDebugMessenger( bool enable_debug_callbacks );
        bool PickPhysicalDevice( VkSampleCountFlagBits max_msaa_samples );
        bool CreateDevice();
    };

} // namespace VulkanCookbook

#endif // VULKAN_CONTEXT
