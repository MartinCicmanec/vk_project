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

    // Commands from the vulkan-tutorial.com "Command buffers" and "Vertex buffers" chapters:
    // clear color to black and depth to 1.0 (the far plane), bind the triangle pipeline, the vertex / index buffers (binding 0, uint32_t indices) and the descriptor set (set 0),
    // set viewport and scissor to the whole framebuffer, draw index_count indices
    bool RecordTriangleCommandBuffer( VkCommandBuffer   command_buffer,
                                      VkRenderPass      render_pass,
                                      VkFramebuffer     framebuffer,
                                      VkExtent2D        extent,
                                      VkPipeline        graphics_pipeline,
                                      VkBuffer          vertex_buffer,
                                      VkBuffer          index_buffer,
                                      uint32_t          index_count,
                                      VkPipelineLayout  pipeline_layout,
                                      VkDescriptorSet   descriptor_set );

} // namespace VulkanCookbook

#endif // DRAWING
