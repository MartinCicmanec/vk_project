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
    // depth test and write (closer fragments win), given number of samples (without sample shading), no blending.
    // For dynamic rendering (Vulkan 1.3): instead of a render pass, the pipeline is created for the formats of the
    // attachments it will be used with - one color attachment and a depth attachment, both with the given sample count.
    bool CreateTrianglePipeline( VkDevice                                               logical_device,
                                 VkFormat                                               color_attachment_format,
                                 VkFormat                                               depth_attachment_format,
                                 VkPipelineLayout                                       pipeline_layout,
                                 VkShaderModule                                         vertex_shader_module,
                                 VkShaderModule                                         fragment_shader_module,
                                 std::vector<VkVertexInputBindingDescription> const   & vertex_binding_descriptions,
                                 std::vector<VkVertexInputAttributeDescription> const & vertex_attribute_descriptions,
                                 VkSampleCountFlagBits                                  samples,
                                 VkPipeline                                           & graphics_pipeline );
    // Pipeline for the particles of the "Compute Shader" chapter: given vertex input, point list (the vertex shader sets
    // gl_PointSize), dynamic viewport and scissor, no culling, no depth, alpha blending, given number of samples.
    // For dynamic rendering with one color attachment of the given format and sample count, and no depth attachment.
    bool CreateParticlePipeline( VkDevice                                               logical_device,
                                 VkFormat                                               color_attachment_format,
                                 VkPipelineLayout                                       pipeline_layout,
                                 VkShaderModule                                         vertex_shader_module,
                                 VkShaderModule                                         fragment_shader_module,
                                 std::vector<VkVertexInputBindingDescription> const   & vertex_binding_descriptions,
                                 std::vector<VkVertexInputAttributeDescription> const & vertex_attribute_descriptions,
                                 VkSampleCountFlagBits                                  samples,
                                 VkPipeline                                           & graphics_pipeline );
    // Pipeline for a background drawn behind everything (the sky of the water scene): no vertex input, the vertex shader
    // makes one full-screen triangle at depth 1.0 from 3 vertices; no culling; depth test LESS_OR_EQUAL without writing,
    // so after the scene only the pixels nothing was drawn to are shaded; no blending; given number of samples.
    // For dynamic rendering with one color and one depth attachment.
    bool CreateBackgroundPipeline( VkDevice                logical_device,
                                   VkFormat                color_attachment_format,
                                   VkFormat                depth_attachment_format,
                                   VkPipelineLayout        pipeline_layout,
                                   VkShaderModule          vertex_shader_module,
                                   VkShaderModule          fragment_shader_module,
                                   VkSampleCountFlagBits   samples,
                                   VkPipeline            & graphics_pipeline );
    // Pipeline for transparent camera-facing quads built by the vertex shader (the splashes of the water scene): no vertex
    // input, triangle list, no culling, depth test LESS without writing (hidden behind solid objects, but not hiding
    // each other), alpha blending, given number of samples. For dynamic rendering with one color and one depth attachment.
    bool CreateBillboardPipeline( VkDevice                logical_device,
                                  VkFormat                color_attachment_format,
                                  VkFormat                depth_attachment_format,
                                  VkPipelineLayout        pipeline_layout,
                                  VkShaderModule          vertex_shader_module,
                                  VkShaderModule          fragment_shader_module,
                                  VkSampleCountFlagBits   samples,
                                  VkPipeline            & graphics_pipeline );
    // Compute pipeline with the "main" entry point of the given compute shader
    bool CreateComputePipeline( VkDevice           logical_device,
                                VkPipelineLayout   pipeline_layout,
                                VkShaderModule     compute_shader_module,
                                VkPipeline       & compute_pipeline );

} // namespace VulkanCookbook

#endif // PIPELINE
