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
// Drawing

#ifndef DRAWING
#define DRAWING

#include "Common.h"

namespace VulkanCookbook {

    // The images one frame is rendered with (dynamic rendering - there are no render pass and framebuffer objects)
    struct FrameAttachments {
        VkImage       SwapchainImage;           // Acquired swapchain image: resolve target, presented afterwards
        VkImageView   SwapchainImageView;
        VkImage       ColorImage;               // Multisampled image everything is drawn to, in COLOR_ATTACHMENT_OPTIMAL
        VkImageView   ColorImageView;
        VkImage       DepthImage;               // Multisampled depth image, in DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        VkImageView   DepthImageView;
        VkFormat      DepthFormat;
    };

    // Commands of one frame, with dynamic rendering (Vulkan 1.3):
    //   - barriers: the swapchain image becomes a color attachment, the shared multisampled color and depth images wait
    //     for the previous frame (a render pass did this implicitly through its layouts and subpass dependencies)
    //   - rendering: clear color to black and depth to 1.0 (the far plane), bind the triangle pipeline, the vertex / index
    //     buffers (binding 0, uint32_t indices) and the descriptor set (set 0), set viewport and scissor to the whole
    //     image, draw index_count indices; the multisampled color image is resolved into the swapchain image at the end
    //   - barrier: the swapchain image becomes presentable
    bool RecordTriangleCommandBuffer( VkCommandBuffer            command_buffer,
                                      FrameAttachments const   & attachments,
                                      VkExtent2D        extent,
                                      VkPipeline        graphics_pipeline,
                                      VkBuffer          vertex_buffer,
                                      VkBuffer          index_buffer,
                                      uint32_t          index_count,
                                      VkPipelineLayout  pipeline_layout,
                                      VkDescriptorSet   descriptor_set );

} // namespace VulkanCookbook

#endif // DRAWING
