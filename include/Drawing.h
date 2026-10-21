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
#include "Ripples.h"
#include "Splashes.h"

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

    // Commands of one frame of the "Compute Shader" chapter:
    //   - compute: bind the compute pipeline and descriptor set (set 0), dispatch one invocation per particle
    //     (particle_count has to be a multiple of the shader's local size, 256)
    //   - barrier: drawing (vertex input) and the next frame's dispatch wait for the particle buffer writes
    //   - barriers: the swapchain image becomes a color attachment, the shared multisampled color image waits for the
    //     previous frame (the depth image of the attachments is not used)
    //   - rendering: clear to black, draw particle_count points from particle_buffer (binding 0), resolve into the
    //     swapchain image
    //   - barrier: the swapchain image becomes presentable
    // Everything runs on one queue, so a pipeline barrier replaces the tutorial's separate compute submission and semaphore.
    bool RecordParticleCommandBuffer( VkCommandBuffer            command_buffer,
                                      FrameAttachments const   & attachments,
                                      VkExtent2D                 extent,
                                      VkPipeline                 compute_pipeline,
                                      VkPipelineLayout           compute_pipeline_layout,
                                      VkDescriptorSet            compute_descriptor_set,
                                      VkPipeline                 graphics_pipeline,
                                      VkBuffer                   particle_buffer,
                                      uint32_t                   particle_count );

    // What one frame of the water scene draws. All pipelines use the same layout and descriptor set (set 0).
    struct WaterSceneDraw {
        VkPipelineLayout   PipelineLayout;
        VkDescriptorSet    DescriptorSet;
        VkPipeline         WaterPipeline;
        VkBuffer           GridVertexBuffer;
        VkBuffer           GridIndexBuffer;
        uint32_t           GridIndexCount;
        VkPipeline         SkyPipeline;
        // Floating boxes: one instanced draw of the box mesh, InstanceCount may be 0
        VkPipeline         ObjectPipeline;
        VkBuffer           BoxVertexBuffer;
        VkBuffer           BoxIndexBuffer;
        uint32_t           BoxIndexCount;
        VkBuffer           InstanceBuffer;
        uint32_t           InstanceCount;
        // Ripple field: one compute dispatch per step, with these push constants; the count may be 0
        VkPipeline                      RipplePipeline;
        RippleStepConstants const     * RippleSteps;
        uint32_t                        RippleStepCount;
        // Splash particles: updated by one compute dispatch every frame, drawn as 6 vertices (a quad) per particle
        VkPipeline                      SplashUpdatePipeline;
        SplashUpdateConstants           SplashUpdate;
        VkPipeline                      SplashDrawPipeline;
    };

    // Commands of one frame of the water scene:
    //   - compute: advance the ripple field by RippleStepCount steps (RIPPLE_CELL_COUNT^2 cells, 16 x 16 per work group),
    //     then update the splash particles (they read the ripples) - with barriers so that every step sees the previous
    //     one and the drawing sees the results
    //   - barriers: as in RecordTriangleCommandBuffer (swapchain image, shared multisampled color and depth images)
    //   - rendering: clear color and depth, draw the boxes (instanced: vertices at binding 0, one BoxInstance per box at
    //     binding 1), the ocean grid (uint32_t indices) - it hides the submerged parts - then the sky as one full-screen
    //     triangle (3 vertices, no vertex buffer) - only the pixels without water run its shader - and the transparent
    //     splash quads last (after the sky, which would otherwise cover them);
    //     the multisampled color image is resolved into the swapchain image at the end
    //   - barrier: the swapchain image becomes presentable
    bool RecordWaterCommandBuffer( VkCommandBuffer            command_buffer,
                                   FrameAttachments const   & attachments,
                                   VkExtent2D                 extent,
                                   WaterSceneDraw const     & scene );

} // namespace VulkanCookbook

#endif // DRAWING
