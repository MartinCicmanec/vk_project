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
// Renderer

#include <limits>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"
#include "AppWindow.h"
#include "VulkanContext.h"
#include "SwapchainTargets.h"
#include "Swapchain.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Tools.h"
#include "CommandBuffer.h"
#include "Synchronization.h"
#include "Queue.h"
#include "Drawing.h"
#include "Buffer.h"
#include "Vertex.h"
#include "Descriptor.h"
#include "UniformBufferObject.h"

namespace VulkanCookbook {

    namespace {
        // Paths are relative to the working directory, like the shaders.
        // "Viking room" model by nigelgoh (CC BY 4.0), prepared for vulkan-tutorial.com.
        const std::string ModelPath   = "models/viking_room.obj";
        const std::string TexturePath = "textures/viking_room.png";
    }

    bool Renderer::Init( VulkanContext    & context,
                         SwapchainTargets & targets,
                         AppWindow        & window ) {
        Context = &context;
        Targets = &targets;
        TargetWindow = &window;

        if( !CreateUniformDescriptorSetLayout() ||
            !CreatePipeline() ||
            !CreateCommandBuffers() ||
            !LoadScene() ||
            !CreateUniformBuffers() ||
            !CreateUniformDescriptorSets() ||
            !CreateSyncObjects() ) {
            return false;
        }

        // The animation starts when everything is loaded
        StartTime = std::chrono::steady_clock::now();
        return true;
    }

    bool Renderer::CreateUniformDescriptorSetLayout() {
        // Uniform buffer at binding 0 for the vertex shader, texture at binding 1 for the fragment shader
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {
                0,                                              // uint32_t             binding
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,              // VkDescriptorType     descriptorType
                1,                                              // uint32_t             descriptorCount
                VK_SHADER_STAGE_VERTEX_BIT,                     // VkShaderStageFlags   stageFlags
                nullptr                                         // const VkSampler    * pImmutableSamplers
            },
            {
                1,                                              // uint32_t             binding
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType     descriptorType
                1,                                              // uint32_t             descriptorCount
                VK_SHADER_STAGE_FRAGMENT_BIT,                   // VkShaderStageFlags   stageFlags
                nullptr                                         // const VkSampler    * pImmutableSamplers
            }
        };

        InitVkDestroyer( Context->LogicalDevice, DescriptorSetLayout );
        return CreateDescriptorSetLayout( *Context->LogicalDevice, bindings, *DescriptorSetLayout );
    }

    bool Renderer::CreatePipeline() {
        // Descriptor set 0 is the uniform buffer, no push constants yet
        InitVkDestroyer( Context->LogicalDevice, PipelineLayout );
        if( !CreatePipelineLayout( *Context->LogicalDevice, { *DescriptorSetLayout }, {}, *PipelineLayout ) ) {
            return false;
        }

        // Compiled with shaders/compile.sh (or compile.bat); paths are relative to the working directory
        std::vector<unsigned char> vertex_shader_code;
        std::vector<unsigned char> fragment_shader_code;
        if( !GetBinaryFileContents( "shaders/vert.spv", vertex_shader_code ) ||
            !GetBinaryFileContents( "shaders/frag.spv", fragment_shader_code ) ) {
            return false;
        }

        // Shader modules are only needed while creating the pipeline - they are destroyed at the end of this function
        VkDestroyer(VkShaderModule) vertex_shader_module;
        InitVkDestroyer( Context->LogicalDevice, vertex_shader_module );
        if( !CreateShaderModule( *Context->LogicalDevice, vertex_shader_code, *vertex_shader_module ) ) {
            return false;
        }

        VkDestroyer(VkShaderModule) fragment_shader_module;
        InitVkDestroyer( Context->LogicalDevice, fragment_shader_module );
        if( !CreateShaderModule( *Context->LogicalDevice, fragment_shader_code, *fragment_shader_module ) ) {
            return false;
        }

        InitVkDestroyer( Context->LogicalDevice, GraphicsPipeline );
        if( !CreateTrianglePipeline( *Context->LogicalDevice, Targets->SurfaceFormat.format, Targets->DepthFormat, *PipelineLayout,
                                     *vertex_shader_module, *fragment_shader_module,
                                     { Vertex::GetBindingDescription() }, Vertex::GetAttributeDescriptions(), Context->MsaaSamples, *GraphicsPipeline ) ) {
            return false;
        }
        std::cout << "Graphics pipeline created" << std::endl;
        return true;
    }

    bool Renderer::CreateCommandBuffers() {
        // Command buffers are re-recorded every frame, so they have to be resettable individually
        InitVkDestroyer( Context->LogicalDevice, CommandPool );
        if( !CreateCommandPool( *Context->LogicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, Context->QueueFamilyIndex, *CommandPool ) ) {
            return false;
        }

        return AllocateCommandBuffers( *Context->LogicalDevice, *CommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT, CommandBuffers );
    }

    bool Renderer::LoadScene() {
        return CreateMeshFromObjFile( Context->Allocator, *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue,
                                      ModelPath, SceneMesh ) &&
               CreateTextureFromFile( Context->PhysicalDevice, Context->Allocator, *Context->LogicalDevice, *Context->TransferCommandPool,
                                      Context->GraphicsAndPresentQueue, TexturePath.c_str(), Context->MaxSamplerAnisotropy, SceneTexture );
    }

    bool Renderer::CreateUniformBuffers() {
        // Updated every frame by the CPU, so no staging buffer - it would only add a copy
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            UniformBuffers.emplace_back();

            // Mapped when it is created and kept mapped until it is destroyed
            if( !CreateBuffer( Context->Allocator, sizeof( UniformBufferObject ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               MemoryLocation::HostVisibleMapped, UniformBuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool Renderer::CreateUniformDescriptorSets() {
        InitVkDestroyer( Context->LogicalDevice, DescriptorPool );
        if( !CreateDescriptorPool( *Context->LogicalDevice, false, MAX_FRAMES_IN_FLIGHT,
                                   { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
                                     { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT } }, *DescriptorPool ) ) {
            return false;
        }

        // One set per frame in flight, all with the same layout
        std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, *DescriptorSetLayout );
        if( !AllocateDescriptorSets( *Context->LogicalDevice, *DescriptorPool, layouts, DescriptorSets ) ) {
            return false;
        }

        std::vector<VkBuffer> uniform_buffers;
        for( auto & uniform_buffer : UniformBuffers ) {
            uniform_buffers.push_back( *uniform_buffer );
        }
        UpdateDescriptorSetsWithUniformBuffers( *Context->LogicalDevice, DescriptorSets, 0, uniform_buffers, sizeof( UniformBufferObject ) );
        UpdateDescriptorSetsWithCombinedImageSampler( *Context->LogicalDevice, DescriptorSets, 1, *SceneTexture.ImageView, *SceneTexture.Sampler,
                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
        return true;
    }

    void Renderer::UpdateUniformBuffer( uint32_t frame_index ) {
        float time = std::chrono::duration<float>( std::chrono::steady_clock::now() - StartTime ).count();

        UniformBufferObject ubo;
        // 90 degrees per second around the Z axis
        ubo.Model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        // Looking at the origin from above at a 45 degree angle
        ubo.View = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        // Aspect ratio from the current swapchain size, so it stays correct after a resize.
        // Depth range is 0..1 because of GLM_FORCE_DEPTH_ZERO_TO_ONE (set in CMakeLists.txt).
        ubo.Projection = glm::perspective( glm::radians( 45.0f ), Targets->Extent.width / static_cast<float>( Targets->Extent.height ), 0.1f, 10.0f );
        // GLM was made for OpenGL, where clip space Y points up - in Vulkan it points down
        ubo.Projection[1][1] *= -1;

        CopyDataToBuffer( UniformBuffers[frame_index], 0, sizeof( ubo ), &ubo );
    }

    bool Renderer::CreateSyncObjects() {
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            ImageAvailableSemaphores.emplace_back();
            InitVkDestroyer( Context->LogicalDevice, ImageAvailableSemaphores.back() );
            InFlightFences.emplace_back();
            InitVkDestroyer( Context->LogicalDevice, InFlightFences.back() );

            // Fences start signaled, so waiting for the "previous" frame doesn't block on the first frames
            if( !CreateVulkanSemaphore( *Context->LogicalDevice, *ImageAvailableSemaphores.back() ) ||
                !CreateFence( *Context->LogicalDevice, true, *InFlightFences.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool Renderer::DrawFrame() {
        // Wait until the GPU has finished the frame that last used this frame's command buffer and semaphores
        if( !WaitForFences( *Context->LogicalDevice, { *InFlightFences[CurrentFrame] }, VK_TRUE, std::numeric_limits<uint64_t>::max() ) ) {
            return false;
        }

        uint32_t image_index;
        SwapchainStatus swapchain_status;
        if( !AcquireSwapchainImage( *Context->LogicalDevice, *Targets->Swapchain, *ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, image_index, swapchain_status ) ) {
            return false;
        }
        // Nothing was acquired - recreate the swapchain and try again in the next frame.
        // A suboptimal swapchain still delivered an image, so that frame is drawn and the swapchain is recreated after presenting.
        if( swapchain_status == SwapchainStatus::OutOfDate ) {
            return Targets->Recreate();
        }

        // Reset only after acquiring - if acquiring fails and the frame is skipped, the fence must stay signaled
        if( !ResetFences( *Context->LogicalDevice, { *InFlightFences[CurrentFrame] } ) ) {
            return false;
        }

        // Safe to overwrite - the fence wait above guarantees the GPU is done with this frame's uniform buffer
        UpdateUniformBuffer( CurrentFrame );

        FrameAttachments attachments = {
            Targets->Images[image_index],
            *Targets->ImageViews[image_index],
            *Targets->ColorImage,
            *Targets->ColorImageView,
            *Targets->DepthImage,
            *Targets->DepthImageView,
            Targets->DepthFormat
        };
        if( !ResetCommandBuffer( CommandBuffers[CurrentFrame], false ) ||
            !RecordTriangleCommandBuffer( CommandBuffers[CurrentFrame], attachments,
                                          Targets->Extent, *GraphicsPipeline, *SceneMesh.VertexBuffer, *SceneMesh.IndexBuffer,
                                          SceneMesh.IndexCount,
                                          *PipelineLayout, DescriptorSets[CurrentFrame] ) ) {
            return false;
        }

        if( !SubmitCommandBuffersToQueue( Context->GraphicsAndPresentQueue,
                                          { { *ImageAvailableSemaphores[CurrentFrame], VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT } },
                                          { CommandBuffers[CurrentFrame] },
                                          { *Targets->RenderFinishedSemaphores[image_index] },
                                          *InFlightFences[CurrentFrame] ) ) {
            return false;
        }

        if( !PresentImage( Context->GraphicsAndPresentQueue, { *Targets->RenderFinishedSemaphores[image_index] }, *Targets->Swapchain, image_index, swapchain_status ) ) {
            return false;
        }

        // Checked after presenting, so the semaphores are in a consistent state
        bool window_resized = TargetWindow->WasResized();
        if( (swapchain_status != SwapchainStatus::Optimal) || window_resized ) {
            if( !Targets->Recreate() ) {
                return false;
            }
        }

        CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return true;
    }

} // namespace VulkanCookbook
