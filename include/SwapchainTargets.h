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
// Swapchain Targets

#ifndef SWAPCHAIN_TARGETS
#define SWAPCHAIN_TARGETS

#include "Common.h"
#include "Allocator.h"

namespace VulkanCookbook {

    class AppWindow;
    class VulkanContext;

    // Everything a frame is rendered to, and that depends on the size of the window: the swapchain with its image views,
    // the multisampled color image, the depth image and one "render finished" semaphore per swapchain image.
    // All of it is recreated together when the window is resized. The formats and the present mode are selected once.
    class SwapchainTargets {
    public:
        SwapchainTargets() = default;
        SwapchainTargets( SwapchainTargets const & ) = delete;
        SwapchainTargets & operator=( SwapchainTargets const & ) = delete;

        // The context and the window have to outlive this object
        bool Init( VulkanContext & context,
                   AppWindow     & window );
        // Waits until the window is visible (not minimized) and the GPU is idle, then creates everything again for the
        // current size of the window. The old swapchain is handed to the new one.
        bool Recreate();

        VkPresentModeKHR                                PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkSurfaceFormatKHR                              SurfaceFormat = {};
        VkExtent2D                                      Extent = {};
        VkDestroyer(VkSwapchainKHR)                     Swapchain;
        // Swapchain images are owned by the swapchain and destroyed together with it
        std::vector<VkImage>                            Images;
        std::vector<VkDestroyer(VkImageView)>           ImageViews;

        // Multisampled image everything is rendered to - resolved into a swapchain image when rendering ends.
        // One image shared by all frames.
        AllocatedImage                                  ColorImage;
        VkDestroyer(VkImageView)                        ColorImageView;

        // One (multisampled) depth image shared by all frames
        VkFormat                                        DepthFormat = VK_FORMAT_UNDEFINED;
        AllocatedImage                                  DepthImage;
        VkDestroyer(VkImageView)                        DepthImageView;

        // Signaled when rendering to a swapchain image has finished, waited for by its presentation - one per image
        std::vector<VkDestroyer(VkSemaphore)>           RenderFinishedSemaphores;

    private:
        bool SelectSwapchainSettings();
        bool SelectDepthFormat();
        bool CreateSwapchainObjects();
        bool CreateColorResources();
        bool CreateDepthResources();
        bool CreateRenderFinishedSemaphores();

        VulkanContext * Context = nullptr;
        AppWindow     * TargetWindow = nullptr;
    };

} // namespace VulkanCookbook

#endif // SWAPCHAIN_TARGETS
