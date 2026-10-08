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
//
// Swapchain

#ifndef SWAPCHAIN
#define SWAPCHAIN

#include "Common.h"

namespace VulkanCookbook {

    // Result of acquiring or presenting a swapchain image
    enum class SwapchainStatus {
        Optimal,        // Swapchain matches the surface
        Suboptimal,     // Swapchain can still be used, but no longer matches the surface exactly
        OutOfDate       // Swapchain can't be used any more and has to be recreated
    };

    bool SelectDesiredPresentationMode( VkPhysicalDevice   physical_device,
                                        VkSurfaceKHR       presentation_surface,
                                        VkPresentModeKHR   desired_present_mode,
                                        VkPresentModeKHR & present_mode );
    bool SelectNumberOfSwapchainImages( VkSurfaceCapabilitiesKHR const & surface_capabilities,
                                        uint32_t                       & number_of_images );
    bool SelectFormatOfSwapchainImages( VkPhysicalDevice     physical_device,
                                        VkSurfaceKHR         presentation_surface,
                                        VkSurfaceFormatKHR   desired_surface_format,
                                        VkSurfaceFormatKHR & surface_format );
    bool SelectSizeOfSwapchainImages( VkSurfaceCapabilitiesKHR const & surface_capabilities,
                                      uint32_t                         framebuffer_width,
                                      uint32_t                         framebuffer_height,
                                      VkExtent2D                     & size_of_images );
    bool CreateSwapchain( VkDevice                        logical_device,
                          VkSurfaceKHR                    presentation_surface,
                          uint32_t                        image_count,
                          VkSurfaceFormatKHR              surface_format,
                          VkExtent2D                      image_size,
                          VkImageUsageFlags               image_usage,
                          VkSurfaceTransformFlagBitsKHR   surface_transform,
                          VkPresentModeKHR                present_mode,
                          VkSwapchainKHR                  old_swapchain,
                          VkSwapchainKHR                & swapchain );
    bool GetHandlesOfSwapchainImages( VkDevice               logical_device,
                                      VkSwapchainKHR         swapchain,
                                      std::vector<VkImage> & swapchain_images );
    // Returns false only on a real error. When status is OutOfDate, no image was acquired.
    bool AcquireSwapchainImage( VkDevice           logical_device,
                                VkSwapchainKHR     swapchain,
                                VkSemaphore        semaphore,
                                VkFence            fence,
                                uint32_t         & image_index,
                                SwapchainStatus  & status );

} // namespace VulkanCookbook

#endif // SWAPCHAIN
