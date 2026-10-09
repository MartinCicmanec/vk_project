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

#include "CommandBuffer.h"
#include "Drawing.h"
#include "Image.h"

namespace VulkanCookbook {

    bool RecordTriangleCommandBuffer( VkCommandBuffer            command_buffer,
                                      FrameAttachments const   & attachments,
                                      VkExtent2D        extent,
                                      VkPipeline        graphics_pipeline,
                                      VkBuffer          vertex_buffer,
                                      VkBuffer          index_buffer,
                                      uint32_t          index_count,
                                      VkPipelineLayout  pipeline_layout,
                                      VkDescriptorSet   descriptor_set ) {
        if( !BeginCommandBufferRecordingOperation( command_buffer, 0, nullptr ) ) {
            return false;
        }

        // The swapchain image was just acquired - its old contents don't matter (UNDEFINED), it is overwritten by the resolve.
        // The source stage is the stage the image available semaphore is waited in, so the transition happens after the wait.
        RecordImageBarrier( command_buffer, attachments.SwapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE,
                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT );

        // The multisampled color and depth images are shared by all frames: this frame's clears and writes have to wait
        // for the writes of the previous frame. The layouts stay the same - these barriers only synchronize.
        RecordImageBarrier( command_buffer, attachments.ColorImage, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT );

        VkImageAspectFlags depth_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        if( HasStencilComponent( attachments.DepthFormat ) ) {
            depth_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        RecordImageBarrier( command_buffer, attachments.DepthImage, depth_aspect, 0, 1,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT );

        VkClearValue color_clear_value = {};
        color_clear_value.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        VkClearValue depth_clear_value = {};
        depth_clear_value.depthStencil = { 1.0f, 0 };

        // Drawn to the multisampled image, which is resolved (averaged) into the swapchain image when rendering ends.
        // Only the resolved image is kept.
        VkRenderingAttachmentInfo color_attachment = {
            VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,    // VkStructureType          sType
            nullptr,                                        // const void             * pNext
            attachments.ColorImageView,                     // VkImageView              imageView
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,       // VkImageLayout            imageLayout
            VK_RESOLVE_MODE_AVERAGE_BIT,                    // VkResolveModeFlagBits    resolveMode
            attachments.SwapchainImageView,                 // VkImageView              resolveImageView
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,       // VkImageLayout            resolveImageLayout
            VK_ATTACHMENT_LOAD_OP_CLEAR,                    // VkAttachmentLoadOp       loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,               // VkAttachmentStoreOp      storeOp
            color_clear_value                               // VkClearValue             clearValue
        };

        // Only needed while rendering
        VkRenderingAttachmentInfo depth_attachment = {
            VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,        // VkStructureType          sType
            nullptr,                                            // const void             * pNext
            attachments.DepthImageView,                         // VkImageView              imageView
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   // VkImageLayout            imageLayout
            VK_RESOLVE_MODE_NONE,                               // VkResolveModeFlagBits    resolveMode
            VK_NULL_HANDLE,                                     // VkImageView              resolveImageView
            VK_IMAGE_LAYOUT_UNDEFINED,                          // VkImageLayout            resolveImageLayout
            VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp       loadOp
            VK_ATTACHMENT_STORE_OP_DONT_CARE,                   // VkAttachmentStoreOp      storeOp
            depth_clear_value                                   // VkClearValue             clearValue
        };

        VkRenderingInfo rendering_info = {
            VK_STRUCTURE_TYPE_RENDERING_INFO,               // VkStructureType                    sType
            nullptr,                                        // const void                       * pNext
            0,                                              // VkRenderingFlags                   flags
            { { 0, 0 }, extent },                           // VkRect2D                           renderArea
            1,                                              // uint32_t                           layerCount
            0,                                              // uint32_t                           viewMask
            1,                                              // uint32_t                           colorAttachmentCount
            &color_attachment,                              // const VkRenderingAttachmentInfo  * pColorAttachments
            &depth_attachment,                              // const VkRenderingAttachmentInfo  * pDepthAttachment
            nullptr                                         // const VkRenderingAttachmentInfo  * pStencilAttachment
        };
        vkCmdBeginRendering( command_buffer, &rendering_info );

        vkCmdBindPipeline( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline );

        VkDeviceSize vertex_buffer_offset = 0;
        vkCmdBindVertexBuffers( command_buffer, 0, 1, &vertex_buffer, &vertex_buffer_offset );
        vkCmdBindIndexBuffer( command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32 );

        // Viewport and scissor are dynamic state in the triangle pipeline
        VkViewport viewport = {
            0.0f,                                           // float    x
            0.0f,                                           // float    y
            static_cast<float>(extent.width),               // float    width
            static_cast<float>(extent.height),              // float    height
            0.0f,                                           // float    minDepth
            1.0f                                            // float    maxDepth
        };
        vkCmdSetViewport( command_buffer, 0, 1, &viewport );

        VkRect2D scissor = { { 0, 0 }, extent };
        vkCmdSetScissor( command_buffer, 0, 1, &scissor );

        vkCmdBindDescriptorSets( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr );

        vkCmdDrawIndexed( command_buffer, index_count, 1, 0, 0, 0 );

        vkCmdEndRendering( command_buffer );

        // Nothing in this command buffer uses the image afterwards - presentation waits for the render finished semaphore
        RecordImageBarrier( command_buffer, attachments.SwapchainImage, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                            VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE );

        return EndCommandBufferRecordingOperation( command_buffer );
    }

} // namespace VulkanCookbook
