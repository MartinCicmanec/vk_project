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
// Particle Renderer

#include <limits>
#include <random>
#include <glm/gtc/constants.hpp>

#include "ParticleRenderer.h"
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
#include "Descriptor.h"
#include "Particle.h"

namespace VulkanCookbook {

    bool ParticleRenderer::Init( VulkanContext    & context,
                                 SwapchainTargets & targets,
                                 AppWindow        & window ) {
        Context = &context;
        Targets = &targets;
        TargetWindow = &window;

        // shaders/particle.vert writes gl_PointSize = 14
        if( !Context->LargePointsSupported ) {
            std::cout << "The particle renderer needs the largePoints device feature, which is not supported." << std::endl;
            return false;
        }

        if( !CreateComputeDescriptorSetLayout() ||
            !CreatePipelines() ||
            !CreateCommandBuffers() ||
            !CreateShaderStorageBuffers() ||
            !CreateUniformBuffers() ||
            !CreateComputeDescriptorSets() ||
            !CreateSyncObjects() ) {
            return false;
        }

        LastFrameTime = std::chrono::steady_clock::now();
        return true;
    }

    bool ParticleRenderer::CreateComputeDescriptorSetLayout() {
        // Parameters at binding 0, previous frame's particles at binding 1, this frame's particles at binding 2
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            {
                0,                                              // uint32_t             binding
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,              // VkDescriptorType     descriptorType
                1,                                              // uint32_t             descriptorCount
                VK_SHADER_STAGE_COMPUTE_BIT,                    // VkShaderStageFlags   stageFlags
                nullptr                                         // const VkSampler    * pImmutableSamplers
            },
            {
                1,                                              // uint32_t             binding
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,              // VkDescriptorType     descriptorType
                1,                                              // uint32_t             descriptorCount
                VK_SHADER_STAGE_COMPUTE_BIT,                    // VkShaderStageFlags   stageFlags
                nullptr                                         // const VkSampler    * pImmutableSamplers
            },
            {
                2,                                              // uint32_t             binding
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,              // VkDescriptorType     descriptorType
                1,                                              // uint32_t             descriptorCount
                VK_SHADER_STAGE_COMPUTE_BIT,                    // VkShaderStageFlags   stageFlags
                nullptr                                         // const VkSampler    * pImmutableSamplers
            }
        };

        InitVkDestroyer( Context->LogicalDevice, ComputeDescriptorSetLayout );
        return CreateDescriptorSetLayout( *Context->LogicalDevice, bindings, *ComputeDescriptorSetLayout );
    }

    bool ParticleRenderer::CreatePipelines() {
        // Compiled with shaders/compile.sh (or compile.bat); paths are relative to the working directory
        std::vector<unsigned char> compute_shader_code;
        std::vector<unsigned char> vertex_shader_code;
        std::vector<unsigned char> fragment_shader_code;
        if( !GetBinaryFileContents( "shaders/particle_comp.spv", compute_shader_code ) ||
            !GetBinaryFileContents( "shaders/particle_vert.spv", vertex_shader_code ) ||
            !GetBinaryFileContents( "shaders/particle_frag.spv", fragment_shader_code ) ) {
            return false;
        }

        // Shader modules are only needed while creating the pipelines - they are destroyed at the end of this function
        VkDestroyer(VkShaderModule) compute_shader_module;
        InitVkDestroyer( Context->LogicalDevice, compute_shader_module );
        VkDestroyer(VkShaderModule) vertex_shader_module;
        InitVkDestroyer( Context->LogicalDevice, vertex_shader_module );
        VkDestroyer(VkShaderModule) fragment_shader_module;
        InitVkDestroyer( Context->LogicalDevice, fragment_shader_module );
        if( !CreateShaderModule( *Context->LogicalDevice, compute_shader_code, *compute_shader_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, vertex_shader_code, *vertex_shader_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, fragment_shader_code, *fragment_shader_module ) ) {
            return false;
        }

        InitVkDestroyer( Context->LogicalDevice, ComputePipelineLayout );
        if( !CreatePipelineLayout( *Context->LogicalDevice, { *ComputeDescriptorSetLayout }, {}, *ComputePipelineLayout ) ) {
            return false;
        }
        InitVkDestroyer( Context->LogicalDevice, ComputePipeline );
        if( !CreateComputePipeline( *Context->LogicalDevice, *ComputePipelineLayout, *compute_shader_module, *ComputePipeline ) ) {
            return false;
        }

        InitVkDestroyer( Context->LogicalDevice, GraphicsPipelineLayout );
        if( !CreatePipelineLayout( *Context->LogicalDevice, {}, {}, *GraphicsPipelineLayout ) ) {
            return false;
        }
        InitVkDestroyer( Context->LogicalDevice, GraphicsPipeline );
        if( !CreateParticlePipeline( *Context->LogicalDevice, Targets->SurfaceFormat.format, *GraphicsPipelineLayout,
                                     *vertex_shader_module, *fragment_shader_module,
                                     { Particle::GetBindingDescription() }, Particle::GetAttributeDescriptions(), Context->MsaaSamples, *GraphicsPipeline ) ) {
            return false;
        }
        std::cout << "Compute and particle pipelines created" << std::endl;
        return true;
    }

    bool ParticleRenderer::CreateCommandBuffers() {
        // Command buffers are re-recorded every frame, so they have to be resettable individually
        InitVkDestroyer( Context->LogicalDevice, CommandPool );
        if( !CreateCommandPool( *Context->LogicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, Context->QueueFamilyIndex, *CommandPool ) ) {
            return false;
        }

        return AllocateCommandBuffers( *Context->LogicalDevice, *CommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT, CommandBuffers );
    }

    bool ParticleRenderer::CreateShaderStorageBuffers() {
        // Random particles in a circle around the center, moving outwards
        std::default_random_engine random_engine( static_cast<unsigned>( std::chrono::steady_clock::now().time_since_epoch().count() ) );
        std::uniform_real_distribution<float> random_distribution( 0.0f, 1.0f );

        // Positions are in clip space (-1 .. 1 on both axes), so x is scaled by the aspect ratio to get a circle.
        // The ratio of the first swapchain size is used - after a resize the circle is stretched (as in the tutorial).
        float aspect_correction = Targets->Extent.height / static_cast<float>( Targets->Extent.width );

        std::vector<Particle> particles( PARTICLE_COUNT );
        for( auto & particle : particles ) {
            // sqrt spreads the particles evenly over the area of the circle instead of crowding the center
            float r = 0.25f * std::sqrt( random_distribution( random_engine ) );
            float theta = random_distribution( random_engine ) * glm::two_pi<float>();
            float x = r * std::cos( theta ) * aspect_correction;
            float y = r * std::sin( theta );
            particle.Position = glm::vec2( x, y );
            particle.Velocity = glm::normalize( glm::vec2( x, y ) ) * 0.00025f;
            particle.Color = glm::vec4( random_distribution( random_engine ), random_distribution( random_engine ), random_distribution( random_engine ), 1.0f );
        }

        // Written and read by the compute shader (storage), drawn as vertices (vertex), filled once from the CPU.
        // All buffers start with the same particles - the first frame reads the last buffer.
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            ShaderStorageBuffers.emplace_back();
            if( !CreateDeviceLocalBufferWithData( Context->Allocator, *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue,
                                                  particles.data(), sizeof( Particle ) * particles.size(),
                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, ShaderStorageBuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool ParticleRenderer::CreateUniformBuffers() {
        // Updated every frame by the CPU, so no staging buffer - it would only add a copy
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            UniformBuffers.emplace_back();

            // Mapped when it is created and kept mapped until it is destroyed
            if( !CreateBuffer( Context->Allocator, sizeof( ParticleParameters ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               MemoryLocation::HostVisibleMapped, UniformBuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool ParticleRenderer::CreateComputeDescriptorSets() {
        InitVkDestroyer( Context->LogicalDevice, DescriptorPool );
        if( !CreateDescriptorPool( *Context->LogicalDevice, false, MAX_FRAMES_IN_FLIGHT,
                                   { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
                                     { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2 * MAX_FRAMES_IN_FLIGHT } }, *DescriptorPool ) ) {
            return false;
        }

        // One set per frame in flight, all with the same layout
        std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, *ComputeDescriptorSetLayout );
        if( !AllocateDescriptorSets( *Context->LogicalDevice, *DescriptorPool, layouts, ComputeDescriptorSets ) ) {
            return false;
        }

        // Set i: parameters i, particles of the previous frame (i - 1, wrapping around) and of this frame (i)
        std::vector<VkBuffer> uniform_buffers;
        std::vector<VkBuffer> previous_frame_buffers;
        std::vector<VkBuffer> current_frame_buffers;
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            uniform_buffers.push_back( *UniformBuffers[i] );
            previous_frame_buffers.push_back( *ShaderStorageBuffers[(i + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT] );
            current_frame_buffers.push_back( *ShaderStorageBuffers[i] );
        }
        UpdateDescriptorSetsWithUniformBuffers( *Context->LogicalDevice, ComputeDescriptorSets, 0, uniform_buffers, sizeof( ParticleParameters ) );
        UpdateDescriptorSetsWithStorageBuffers( *Context->LogicalDevice, ComputeDescriptorSets, 1, previous_frame_buffers );
        UpdateDescriptorSetsWithStorageBuffers( *Context->LogicalDevice, ComputeDescriptorSets, 2, current_frame_buffers );
        return true;
    }

    bool ParticleRenderer::CreateSyncObjects() {
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

    void ParticleRenderer::UpdateUniformBuffer( uint32_t frame_index ) {
        auto now = std::chrono::steady_clock::now();
        float frame_time_ms = std::chrono::duration<float, std::milli>( now - LastFrameTime ).count();
        LastFrameTime = now;

        // Velocities are per millisecond; the factor 2 is the tutorial's speed-up
        ParticleParameters parameters;
        parameters.DeltaTime = frame_time_ms * 2.0f;

        CopyDataToBuffer( UniformBuffers[frame_index], 0, sizeof( parameters ), &parameters );
    }

    bool ParticleRenderer::DrawFrame() {
        // Wait until the GPU has finished the frame that last used this frame's command buffer, buffers and semaphores
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
            !RecordParticleCommandBuffer( CommandBuffers[CurrentFrame], attachments, Targets->Extent,
                                          *ComputePipeline, *ComputePipelineLayout, ComputeDescriptorSets[CurrentFrame],
                                          *GraphicsPipeline, *ShaderStorageBuffers[CurrentFrame], PARTICLE_COUNT ) ) {
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
