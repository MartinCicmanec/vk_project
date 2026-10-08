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

#include <algorithm>
#include <limits>
#include "Swapchain.h"

namespace VulkanCookbook {

    bool SelectDesiredPresentationMode( VkPhysicalDevice   physical_device,
                                        VkSurfaceKHR       presentation_surface,
                                        VkPresentModeKHR   desired_present_mode,
                                        VkPresentModeKHR & present_mode ) {
        // Enumerate supported present modes
        uint32_t present_modes_count = 0;
        VkResult result = VK_SUCCESS;

        result = vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, presentation_surface, &present_modes_count, nullptr );
        if( (VK_SUCCESS != result) || (0 == present_modes_count) ) {
            std::cout << "Could not get the number of supported present modes." << std::endl;
            return false;
        }

        std::vector<VkPresentModeKHR> present_modes( present_modes_count );
        result = vkGetPhysicalDeviceSurfacePresentModesKHR( physical_device, presentation_surface, &present_modes_count, present_modes.data() );
        if( (VK_SUCCESS != result) || (0 == present_modes_count) ) {
            std::cout << "Could not enumerate present modes." << std::endl;
            return false;
        }

        // Select present mode
        for( auto & current_present_mode : present_modes ) {
            if( current_present_mode == desired_present_mode ) {
                present_mode = desired_present_mode;
                return true;
            }
        }

        std::cout << "Desired present mode is not supported. Selecting default FIFO mode." << std::endl;
        for( auto & current_present_mode : present_modes ) {
            if( current_present_mode == VK_PRESENT_MODE_FIFO_KHR ) {
                present_mode = VK_PRESENT_MODE_FIFO_KHR;
                return true;
            }
        }

        std::cout << "VK_PRESENT_MODE_FIFO_KHR is not supported though it's mandatory for all drivers!" << std::endl;
        return false;
    }

    bool SelectNumberOfSwapchainImages( VkSurfaceCapabilitiesKHR const & surface_capabilities,
                                        uint32_t                       & number_of_images ) {
        number_of_images = surface_capabilities.minImageCount + 1;
        if( (surface_capabilities.maxImageCount > 0) && (number_of_images > surface_capabilities.maxImageCount) ) {
            number_of_images = surface_capabilities.maxImageCount;
        }
        return true;
    }

    bool SelectFormatOfSwapchainImages( VkPhysicalDevice     physical_device,
                                        VkSurfaceKHR         presentation_surface,
                                        VkSurfaceFormatKHR   desired_surface_format,
                                        VkSurfaceFormatKHR & surface_format ) {
        // Enumerate supported formats
        uint32_t formats_count = 0;
        VkResult result = VK_SUCCESS;

        result = vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, presentation_surface, &formats_count, nullptr );
        if( (VK_SUCCESS != result) || (0 == formats_count) ) {
            std::cout << "Could not get the number of supported surface formats." << std::endl;
            return false;
        }

        std::vector<VkSurfaceFormatKHR> surface_formats( formats_count );
        result = vkGetPhysicalDeviceSurfaceFormatsKHR( physical_device, presentation_surface, &formats_count, surface_formats.data() );
        if( (VK_SUCCESS != result) || (0 == formats_count) ) {
            std::cout << "Could not enumerate supported surface formats." << std::endl;
            return false;
        }

        // Select surface format
        for( auto & current_surface_format : surface_formats ) {
            if( (current_surface_format.format == desired_surface_format.format) &&
                (current_surface_format.colorSpace == desired_surface_format.colorSpace) ) {
                surface_format = current_surface_format;
                return true;
            }
        }

        std::cout << "Desired surface format is not supported. Selecting the first available one." << std::endl;
        surface_format = surface_formats[0];
        return true;
    }

    bool SelectSizeOfSwapchainImages( VkSurfaceCapabilitiesKHR const & surface_capabilities,
                                      uint32_t                         framebuffer_width,
                                      uint32_t                         framebuffer_height,
                                      VkExtent2D                     & size_of_images ) {
        // The surface defines the size - it has to be used
        if( surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ) {
            size_of_images = surface_capabilities.currentExtent;
            return true;
        }

        // The window system lets us choose - use the framebuffer size within the allowed range
        size_of_images.width  = std::clamp( framebuffer_width,  surface_capabilities.minImageExtent.width,  surface_capabilities.maxImageExtent.width );
        size_of_images.height = std::clamp( framebuffer_height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height );
        return true;
    }

    bool CreateSwapchain( VkDevice                        logical_device,
                          VkSurfaceKHR                    presentation_surface,
                          uint32_t                        image_count,
                          VkSurfaceFormatKHR              surface_format,
                          VkExtent2D                      image_size,
                          VkImageUsageFlags               image_usage,
                          VkSurfaceTransformFlagBitsKHR   surface_transform,
                          VkPresentModeKHR                present_mode,
                          VkSwapchainKHR                  old_swapchain,
                          VkSwapchainKHR                & swapchain ) {
        VkSwapchainCreateInfoKHR swapchain_create_info = {
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType                  sType
            nullptr,                                      // const void                     * pNext
            0,                                            // VkSwapchainCreateFlagsKHR        flags
            presentation_surface,                         // VkSurfaceKHR                     surface
            image_count,                                  // uint32_t                         minImageCount
            surface_format.format,                        // VkFormat                         imageFormat
            surface_format.colorSpace,                    // VkColorSpaceKHR                  imageColorSpace
            image_size,                                   // VkExtent2D                       imageExtent
            1,                                            // uint32_t                         imageArrayLayers
            image_usage,                                  // VkImageUsageFlags                imageUsage
            VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode                    imageSharingMode
            0,                                            // uint32_t                         queueFamilyIndexCount
            nullptr,                                      // const uint32_t                 * pQueueFamilyIndices
            surface_transform,                            // VkSurfaceTransformFlagBitsKHR    preTransform
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR      compositeAlpha
            present_mode,                                 // VkPresentModeKHR                 presentMode
            VK_TRUE,                                      // VkBool32                         clipped
            old_swapchain                                 // VkSwapchainKHR                   oldSwapchain
        };

        VkResult result = vkCreateSwapchainKHR( logical_device, &swapchain_create_info, nullptr, &swapchain );
        if( (VK_SUCCESS != result) || (VK_NULL_HANDLE == swapchain) ) {
            std::cout << "Could not create a swapchain." << std::endl;
            return false;
        }
        return true;
    }

    bool GetHandlesOfSwapchainImages( VkDevice               logical_device,
                                      VkSwapchainKHR         swapchain,
                                      std::vector<VkImage> & swapchain_images ) {
        uint32_t images_count = 0;
        VkResult result = VK_SUCCESS;

        result = vkGetSwapchainImagesKHR( logical_device, swapchain, &images_count, nullptr );
        if( (VK_SUCCESS != result) || (0 == images_count) ) {
            std::cout << "Could not get the number of swapchain images." << std::endl;
            return false;
        }

        swapchain_images.resize( images_count );
        result = vkGetSwapchainImagesKHR( logical_device, swapchain, &images_count, swapchain_images.data() );
        if( (VK_SUCCESS != result) || (0 == images_count) ) {
            std::cout << "Could not enumerate swapchain images." << std::endl;
            return false;
        }
        return true;
    }

    bool AcquireSwapchainImage( VkDevice           logical_device,
                                VkSwapchainKHR     swapchain,
                                VkSemaphore        semaphore,
                                VkFence            fence,
                                uint32_t         & image_index,
                                SwapchainStatus  & status ) {
        VkResult result = vkAcquireNextImageKHR( logical_device, swapchain, std::numeric_limits<uint64_t>::max(), semaphore, fence, &image_index );
        switch( result ) {
        case VK_SUCCESS:
            status = SwapchainStatus::Optimal;
            return true;
        case VK_SUBOPTIMAL_KHR:
            status = SwapchainStatus::Suboptimal;
            return true;
        case VK_ERROR_OUT_OF_DATE_KHR:
            status = SwapchainStatus::OutOfDate;
            return true;
        default:
            std::cout << "Could not acquire a swapchain image." << std::endl;
            return false;
        }
    }

} // namespace VulkanCookbook
