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

#include "Pipeline.h"

namespace VulkanCookbook {

    bool CreatePipelineLayout( VkDevice                                   logical_device,
                               std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
                               std::vector<VkPushConstantRange> const   & push_constant_ranges,
                               VkPipelineLayout                         & pipeline_layout ) {
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,              // VkStructureType                  sType
            nullptr,                                                    // const void                     * pNext
            0,                                                          // VkPipelineLayoutCreateFlags      flags
            static_cast<uint32_t>(descriptor_set_layouts.size()),       // uint32_t                         setLayoutCount
            descriptor_set_layouts.data(),                              // const VkDescriptorSetLayout    * pSetLayouts
            static_cast<uint32_t>(push_constant_ranges.size()),         // uint32_t                         pushConstantRangeCount
            push_constant_ranges.data()                                 // const VkPushConstantRange      * pPushConstantRanges
        };

        VkResult result = vkCreatePipelineLayout( logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout );
        if( (VK_SUCCESS != result) || (VK_NULL_HANDLE == pipeline_layout) ) {
            std::cout << "Could not create a pipeline layout." << std::endl;
            return false;
        }
        return true;
    }

    bool CreateGraphicsPipeline( VkDevice                             logical_device,
                                 VkGraphicsPipelineCreateInfo const & graphics_pipeline_create_info,
                                 VkPipeline                         & graphics_pipeline ) {
        VkResult result = vkCreateGraphicsPipelines( logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &graphics_pipeline );
        if( (VK_SUCCESS != result) || (VK_NULL_HANDLE == graphics_pipeline) ) {
            std::cout << "Could not create a graphics pipeline." << std::endl;
            return false;
        }
        return true;
    }

    bool CreateTrianglePipeline( VkDevice                                               logical_device,
                                 VkRenderPass                                           render_pass,
                                 VkPipelineLayout                                       pipeline_layout,
                                 VkShaderModule                                         vertex_shader_module,
                                 VkShaderModule                                         fragment_shader_module,
                                 std::vector<VkVertexInputBindingDescription> const   & vertex_binding_descriptions,
                                 std::vector<VkVertexInputAttributeDescription> const & vertex_attribute_descriptions,
                                 VkPipeline                                           & graphics_pipeline ) {
        // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // VkStructureType                    sType
                nullptr,                                                // const void                       * pNext
                0,                                                      // VkPipelineShaderStageCreateFlags   flags
                VK_SHADER_STAGE_VERTEX_BIT,                             // VkShaderStageFlagBits              stage
                vertex_shader_module,                                   // VkShaderModule                     module
                "main",                                                 // const char                       * pName
                nullptr                                                 // const VkSpecializationInfo       * pSpecializationInfo
            },
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // VkStructureType                    sType
                nullptr,                                                // const void                       * pNext
                0,                                                      // VkPipelineShaderStageCreateFlags   flags
                VK_SHADER_STAGE_FRAGMENT_BIT,                           // VkShaderStageFlagBits              stage
                fragment_shader_module,                                 // VkShaderModule                     module
                "main",                                                 // const char                       * pName
                nullptr                                                 // const VkSpecializationInfo       * pSpecializationInfo
            }
        };

        // Dynamic state - viewport and scissor are set when recording the command buffer
        std::vector<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamic_state = {
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,       // VkStructureType                      sType
            nullptr,                                                    // const void                         * pNext
            0,                                                          // VkPipelineDynamicStateCreateFlags    flags
            static_cast<uint32_t>(dynamic_states.size()),               // uint32_t                             dynamicStateCount
            dynamic_states.data()                                       // const VkDynamicState               * pDynamicStates
        };

        // Vertex input - how vertex buffer data maps to the vertex shader inputs
        VkPipelineVertexInputStateCreateInfo vertex_input_state = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      // VkStructureType                           sType
            nullptr,                                                        // const void                              * pNext
            0,                                                              // VkPipelineVertexInputStateCreateFlags     flags
            static_cast<uint32_t>(vertex_binding_descriptions.size()),      // uint32_t                                  vertexBindingDescriptionCount
            vertex_binding_descriptions.data(),                             // const VkVertexInputBindingDescription   * pVertexBindingDescriptions
            static_cast<uint32_t>(vertex_attribute_descriptions.size()),    // uint32_t                                  vertexAttributeDescriptionCount
            vertex_attribute_descriptions.data()                            // const VkVertexInputAttributeDescription * pVertexAttributeDescriptions
        };

        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, // VkStructureType                          sType
            nullptr,                                                    // const void                             * pNext
            0,                                                          // VkPipelineInputAssemblyStateCreateFlags  flags
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                        // VkPrimitiveTopology                      topology
            VK_FALSE                                                    // VkBool32                                 primitiveRestartEnable
        };

        // Viewport and scissor - only the counts, the values are dynamic
        VkPipelineViewportStateCreateInfo viewport_state = {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,      // VkStructureType                      sType
            nullptr,                                                    // const void                         * pNext
            0,                                                          // VkPipelineViewportStateCreateFlags   flags
            1,                                                          // uint32_t                             viewportCount
            nullptr,                                                    // const VkViewport                   * pViewports
            1,                                                          // uint32_t                             scissorCount
            nullptr                                                     // const VkRect2D                     * pScissors
        };

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterization_state = {
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType                          sType
            nullptr,                                                    // const void                             * pNext
            0,                                                          // VkPipelineRasterizationStateCreateFlags  flags
            VK_FALSE,                                                   // VkBool32                                 depthClampEnable
            VK_FALSE,                                                   // VkBool32                                 rasterizerDiscardEnable
            VK_POLYGON_MODE_FILL,                                       // VkPolygonMode                            polygonMode
            VK_CULL_MODE_BACK_BIT,                                      // VkCullModeFlags                          cullMode
            VK_FRONT_FACE_COUNTER_CLOCKWISE,                            // VkFrontFace                              frontFace (the Y flip in the projection matrix reverses the winding)
            VK_FALSE,                                                   // VkBool32                                 depthBiasEnable
            0.0f,                                                       // float                                    depthBiasConstantFactor
            0.0f,                                                       // float                                    depthBiasClamp
            0.0f,                                                       // float                                    depthBiasSlopeFactor
            1.0f                                                        // float                                    lineWidth
        };

        // Multisampling - disabled
        VkPipelineMultisampleStateCreateInfo multisample_state = {
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,   // VkStructureType                          sType
            nullptr,                                                    // const void                             * pNext
            0,                                                          // VkPipelineMultisampleStateCreateFlags    flags
            VK_SAMPLE_COUNT_1_BIT,                                      // VkSampleCountFlagBits                    rasterizationSamples
            VK_FALSE,                                                   // VkBool32                                 sampleShadingEnable
            1.0f,                                                       // float                                    minSampleShading
            nullptr,                                                    // const VkSampleMask                     * pSampleMask
            VK_FALSE,                                                   // VkBool32                                 alphaToCoverageEnable
            VK_FALSE                                                    // VkBool32                                 alphaToOneEnable
        };

        // Depth test - a fragment is kept (and its depth written) if it is closer than what is in the depth buffer
        VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, // VkStructureType                          sType
            nullptr,                                                    // const void                             * pNext
            0,                                                          // VkPipelineDepthStencilStateCreateFlags   flags
            VK_TRUE,                                                    // VkBool32                                 depthTestEnable
            VK_TRUE,                                                    // VkBool32                                 depthWriteEnable
            VK_COMPARE_OP_LESS,                                         // VkCompareOp                              depthCompareOp
            VK_FALSE,                                                   // VkBool32                                 depthBoundsTestEnable
            VK_FALSE,                                                   // VkBool32                                 stencilTestEnable
            {},                                                         // VkStencilOpState                         front
            {},                                                         // VkStencilOpState                         back
            0.0f,                                                       // float                                    minDepthBounds
            1.0f                                                        // float                                    maxDepthBounds
        };

        // Color blending - disabled, fragment colors are written unmodified
        VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
            VK_FALSE,                                                   // VkBool32                 blendEnable
            VK_BLEND_FACTOR_ONE,                                        // VkBlendFactor            srcColorBlendFactor
            VK_BLEND_FACTOR_ZERO,                                       // VkBlendFactor            dstColorBlendFactor
            VK_BLEND_OP_ADD,                                            // VkBlendOp                colorBlendOp
            VK_BLEND_FACTOR_ONE,                                        // VkBlendFactor            srcAlphaBlendFactor
            VK_BLEND_FACTOR_ZERO,                                       // VkBlendFactor            dstAlphaBlendFactor
            VK_BLEND_OP_ADD,                                            // VkBlendOp                alphaBlendOp
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |       // VkColorComponentFlags    colorWriteMask
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };
        VkPipelineColorBlendStateCreateInfo color_blend_state = {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // VkStructureType                              sType
            nullptr,                                                    // const void                                 * pNext
            0,                                                          // VkPipelineColorBlendStateCreateFlags         flags
            VK_FALSE,                                                   // VkBool32                                     logicOpEnable
            VK_LOGIC_OP_COPY,                                           // VkLogicOp                                    logicOp
            1,                                                          // uint32_t                                     attachmentCount
            &color_blend_attachment_state,                              // const VkPipelineColorBlendAttachmentState  * pAttachments
            { 0.0f, 0.0f, 0.0f, 0.0f }                                  // float                                        blendConstants[4]
        };

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                sType
            nullptr,                                                    // const void                                   * pNext
            0,                                                          // VkPipelineCreateFlags                          flags
            static_cast<uint32_t>(shader_stages.size()),                // uint32_t                                       stageCount
            shader_stages.data(),                                       // const VkPipelineShaderStageCreateInfo        * pStages
            &vertex_input_state,                                        // const VkPipelineVertexInputStateCreateInfo   * pVertexInputState
            &input_assembly_state,                                      // const VkPipelineInputAssemblyStateCreateInfo * pInputAssemblyState
            nullptr,                                                    // const VkPipelineTessellationStateCreateInfo  * pTessellationState
            &viewport_state,                                            // const VkPipelineViewportStateCreateInfo      * pViewportState
            &rasterization_state,                                       // const VkPipelineRasterizationStateCreateInfo * pRasterizationState
            &multisample_state,                                         // const VkPipelineMultisampleStateCreateInfo   * pMultisampleState
            &depth_stencil_state,                                       // const VkPipelineDepthStencilStateCreateInfo  * pDepthStencilState
            &color_blend_state,                                         // const VkPipelineColorBlendStateCreateInfo    * pColorBlendState
            &dynamic_state,                                             // const VkPipelineDynamicStateCreateInfo       * pDynamicState
            pipeline_layout,                                            // VkPipelineLayout                               layout
            render_pass,                                                // VkRenderPass                                   renderPass
            0,                                                          // uint32_t                                       subpass
            VK_NULL_HANDLE,                                             // VkPipeline                                     basePipelineHandle
            -1                                                          // int32_t                                        basePipelineIndex
        };

        return CreateGraphicsPipeline( logical_device, graphics_pipeline_create_info, graphics_pipeline );
    }

} // namespace VulkanCookbook
