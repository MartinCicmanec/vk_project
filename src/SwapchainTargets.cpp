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

#include "SwapchainTargets.h"
#include "AppWindow.h"
#include "VulkanContext.h"
#include "PhysicalDevice.h"
#include "Presentation.h"
#include "Swapchain.h"
#include "Image.h"
#include "CommandBuffer.h"
#include "Synchronization.h"

namespace VulkanCookbook {

    bool SwapchainTargets::Init( VulkanContext & context,
                                 AppWindow     & window ) {
        Context = &context;
        TargetWindow = &window;

        return SelectSwapchainSettings() &&
               SelectDepthFormat() &&
               CreateSwapchainObjects() &&
               CreateColorResources() &&
               CreateDepthResources() &&
               CreateRenderFinishedSemaphores();
    }

    bool SwapchainTargets::Recreate() {
        // A minimized window has a framebuffer size of 0 - pause until it is visible again
        uint32_t framebuffer_width = 0;
        uint32_t framebuffer_height = 0;
        TargetWindow->GetFramebufferSize( framebuffer_width, framebuffer_height );
        while( (framebuffer_width == 0) || (framebuffer_height == 0) ) {
            if( TargetWindow->ShouldClose() ) {
                return true;
            }
            TargetWindow->WaitEvents();
            TargetWindow->GetFramebufferSize( framebuffer_width, framebuffer_height );
        }

        // Objects that are still in use by the GPU must not be destroyed
        Context->WaitIdle();

        // Everything that depends on the swapchain images or their size:
        // depth image -> multisampled color image -> image views -> swapchain.
        // The pipeline is kept - it only depends on the image formats, which are selected once.
        RenderFinishedSemaphores.clear();

        return CreateSwapchainObjects() &&
               CreateColorResources() &&
               CreateDepthResources() &&
               CreateRenderFinishedSemaphores();
    }

    bool SwapchainTargets::SelectSwapchainSettings() {
        if( !SelectDesiredPresentationMode(Context->PhysicalDevice, *Context->PresentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, PresentMode) ) {
            return false;
        }
        std::cout << "Selected present mode : " << PresentMode << std::endl;

        if( !SelectFormatOfSwapchainImages( Context->PhysicalDevice, *Context->PresentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, SurfaceFormat ) ) {
            return false;
        }
        std::cout << "Selected swapchain format : " << SurfaceFormat.format << ", color space : " << SurfaceFormat.colorSpace << std::endl;
        return true;
    }

    bool SwapchainTargets::SelectDepthFormat() {
        // Selected once: the pipeline and the depth image have to use the same format.
        // Most desirable first - no stencil is needed yet.
        if( !SelectSupportedFormat( Context->PhysicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, DepthFormat ) ) {
            return false;
        }
        std::cout << "Selected depth format : " << DepthFormat << std::endl;
        return true;
    }

    bool SwapchainTargets::CreateSwapchainObjects() {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        if( !GetCapabilitiesOfPresentationSurface( Context->PhysicalDevice, *Context->PresentationSurface, surface_capabilities ) ) {
            return false;
        }

        // When the swapchain is recreated, the old one is handed to the new one and destroyed at the end of this function
        VkDestroyer(VkSwapchainKHR) old_swapchain( std::move( Swapchain ) );
        bool first_swapchain = !old_swapchain;

        uint32_t number_of_images;
        SelectNumberOfSwapchainImages(surface_capabilities, number_of_images);
        if( first_swapchain ) {
            std::cout << "Selected number of images : " << number_of_images << std::endl;
        }

        uint32_t framebuffer_width = 0;
        uint32_t framebuffer_height = 0;
        TargetWindow->GetFramebufferSize( framebuffer_width, framebuffer_height );

        SelectSizeOfSwapchainImages( surface_capabilities, framebuffer_width, framebuffer_height, Extent );
        std::cout << (first_swapchain ? "Selected swapchain size : " : "Swapchain recreated : ") << Extent.width << "x" << Extent.height << std::endl;

        InitVkDestroyer( Context->LogicalDevice, Swapchain );
        if( !CreateSwapchain( *Context->LogicalDevice, *Context->PresentationSurface, number_of_images, SurfaceFormat, Extent,
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, surface_capabilities.currentTransform, PresentMode, *old_swapchain, *Swapchain ) ) {
            return false;
        }

        if( !GetHandlesOfSwapchainImages( *Context->LogicalDevice, *Swapchain, Images ) ) {
            return false;
        }
        if( first_swapchain ) {
            std::cout << "Swapchain images : " << Images.size() << std::endl;
        }

        // One image view per swapchain image (views of an old swapchain are destroyed here)
        ImageViews.clear();
        for( auto & image : Images ) {
            ImageViews.emplace_back();
            InitVkDestroyer( Context->LogicalDevice, ImageViews.back() );
            if( !CreateImageView( *Context->LogicalDevice, image, VK_IMAGE_VIEW_TYPE_2D, SurfaceFormat.format,
                                  VK_IMAGE_ASPECT_COLOR_BIT, *ImageViews.back() ) ) {
                return false;
            }
        }
        if( first_swapchain ) {
            std::cout << "Swapchain image views : " << ImageViews.size() << std::endl;
        }
        return true;
    }

    bool SwapchainTargets::CreateColorResources() {
        // Also called when the swapchain is recreated: the old view is destroyed here, the old image (with its memory)
        // when the new one is created
        InitVkDestroyer( Context->LogicalDevice, ColorImageView );

        // Same size and format as the swapchain images, but with several samples per pixel.
        // Transient: its contents are only needed while rendering (they are resolved into the swapchain image).
        if( !CreateImage( Context->Allocator, Extent.width, Extent.height, 1, Context->MsaaSamples, SurfaceFormat.format,
                          VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, ColorImage ) ||
            !CreateImageView( *Context->LogicalDevice, *ColorImage, VK_IMAGE_VIEW_TYPE_2D, SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, *ColorImageView ) ) {
            return false;
        }

        // With dynamic rendering nothing transitions the image automatically - it is put into the layout it is rendered
        // in once, here (RecordTriangleCommandBuffer expects it)
        VkCommandBuffer command_buffer;
        if( !BeginSingleTimeCommands( *Context->LogicalDevice, *Context->TransferCommandPool, command_buffer ) ) {
            return false;
        }
        bool recorded = RecordImageLayoutTransition( command_buffer, *ColorImage, SurfaceFormat.format,
                                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );
        bool submitted = EndSingleTimeCommands( *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue, command_buffer );
        return recorded && submitted;
    }

    bool SwapchainTargets::CreateDepthResources() {
        // Also called when the swapchain is recreated: the old view is destroyed here, the old image (with its memory)
        // when the new one is created
        InitVkDestroyer( Context->LogicalDevice, DepthImageView );

        // Same size as the swapchain images and the same number of samples as the color image
        if( !CreateImage( Context->Allocator, Extent.width, Extent.height, 1, Context->MsaaSamples, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, DepthImage ) ||
            !CreateImageView( *Context->LogicalDevice, *DepthImage, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, *DepthImageView ) ) {
            return false;
        }

        // With dynamic rendering nothing transitions the image automatically - it is put into the layout it is rendered
        // in once, here (RecordTriangleCommandBuffer expects it)
        VkCommandBuffer command_buffer;
        if( !BeginSingleTimeCommands( *Context->LogicalDevice, *Context->TransferCommandPool, command_buffer ) ) {
            return false;
        }
        bool recorded = RecordImageLayoutTransition( command_buffer, *DepthImage, DepthFormat,
                                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
        bool submitted = EndSingleTimeCommands( *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue, command_buffer );
        return recorded && submitted;
    }

    bool SwapchainTargets::CreateRenderFinishedSemaphores() {
        // One "render finished" semaphore per swapchain image (not per frame in flight like in the tutorial):
        // presentation may still wait on a semaphore after the frame's fence is signaled, and it is only safe to reuse
        // once the same swapchain image is acquired again (VUID-vkQueueSubmit-pSignalSemaphores-00067)
        for( size_t i = 0; i < Images.size(); ++i ) {
            RenderFinishedSemaphores.emplace_back();
            InitVkDestroyer( Context->LogicalDevice, RenderFinishedSemaphores.back() );
            if( !CreateVulkanSemaphore( *Context->LogicalDevice, *RenderFinishedSemaphores.back() ) ) {
                return false;
            }
        }
        return true;
    }

} // namespace VulkanCookbook
