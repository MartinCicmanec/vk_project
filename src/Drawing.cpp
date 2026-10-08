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

namespace VulkanCookbook {

    bool RecordTriangleCommandBuffer( VkCommandBuffer   command_buffer,
                                      VkRenderPass      render_pass,
                                      VkFramebuffer     framebuffer,
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

        // Same order as the render pass attachments: color, depth
        std::array<VkClearValue, 2> clear_values = {};
        clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clear_values[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo render_pass_begin_info = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,       // VkStructureType        sType
            nullptr,                                        // const void           * pNext
            render_pass,                                    // VkRenderPass           renderPass
            framebuffer,                                    // VkFramebuffer          framebuffer
            { { 0, 0 }, extent },                           // VkRect2D               renderArea
            static_cast<uint32_t>(clear_values.size()),     // uint32_t               clearValueCount
            clear_values.data()                             // const VkClearValue   * pClearValues
        };
        vkCmdBeginRenderPass( command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

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

        vkCmdEndRenderPass( command_buffer );

        return EndCommandBufferRecordingOperation( command_buffer );
    }

} // namespace VulkanCookbook
