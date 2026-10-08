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
// Pipeline

#ifndef PIPELINE
#define PIPELINE

#include "Common.h"

namespace VulkanCookbook {

    bool CreatePipelineLayout( VkDevice                                   logical_device,
                               std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
                               std::vector<VkPushConstantRange> const   & push_constant_ranges,
                               VkPipelineLayout                         & pipeline_layout );
    bool CreateGraphicsPipeline( VkDevice                             logical_device,
                                 VkGraphicsPipelineCreateInfo const & graphics_pipeline_create_info,
                                 VkPipeline                         & graphics_pipeline );
    // Pipeline from the vulkan-tutorial.com "Graphics pipeline basics" chapter:
    // given vertex input, triangle list, dynamic viewport and scissor, back-face culling with counter-clockwise front faces,
    // depth test and write (closer fragments win), no multisampling, no blending.
    // The render pass must have a depth attachment.
    bool CreateTrianglePipeline( VkDevice                                               logical_device,
                                 VkRenderPass                                           render_pass,
                                 VkPipelineLayout                                       pipeline_layout,
                                 VkShaderModule                                         vertex_shader_module,
                                 VkShaderModule                                         fragment_shader_module,
                                 std::vector<VkVertexInputBindingDescription> const   & vertex_binding_descriptions,
                                 std::vector<VkVertexInputAttributeDescription> const & vertex_attribute_descriptions,
                                 VkPipeline                                           & graphics_pipeline );

} // namespace VulkanCookbook

#endif // PIPELINE
