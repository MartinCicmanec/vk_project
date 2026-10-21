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
// Water Renderer

#include <algorithm>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>

#include "WaterRenderer.h"
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
#include "Image.h"

namespace VulkanCookbook {

    namespace {
        // Ocean grid: 512 x 512 cells over 1.2 km. About 0.14 m cells in the center, about 7 m at the edges, where the
        // haze hides the details (and the end of the grid).
        const uint32_t GridCellCount     = 512;
        const float    GridHalfSize      = 600.0f;
        const float    GridCenterDensity = 0.06f;

        // Orbit camera: circles the center at this distance and height, looking at the center
        const float    OrbitRadius       = 28.0f;
        const float    OrbitHeight       = 7.0f;
        const float    OrbitSpeed        = 0.06f;      // radians per second (one circle in about 105 s)

        // Low sun, so the glints are visible from part of the orbit
        const glm::vec3 SunDirection     = glm::normalize( glm::vec3( -0.6f, 0.25f, -0.5f ) );
    }

    bool WaterRenderer::Init( VulkanContext    & context,
                              SwapchainTargets & targets,
                              AppWindow        & window ) {
        Context = &context;
        Targets = &targets;
        TargetWindow = &window;
        Waves = GetDefaultOceanWaves();
        Objects.Init( Waves );

        if( !CreateUniformDescriptorSetLayout() ||
            !CreatePipelines() ||
            !CreateCommandBuffers() ||
            !CreateMeshes() ||
            !CreateUniformBuffers() ||
            !CreateInstanceBuffers() ||
            !CreateRippleField() ||
            !CreateSplashBuffers() ||
            !CreateUniformDescriptorSets() ||
            !CreateSyncObjects() ) {
            return false;
        }

        LastFrameTime = std::chrono::steady_clock::now();
        return true;
    }

    bool WaterRenderer::CreateUniformDescriptorSetLayout() {
        // One set for everything, used by the graphics and the compute pipelines:
        //   0: uniform buffer - camera, time, sun, ripple area and waves (the splash update needs the water height too)
        //   1, 2: the two ripple height images - written by the ripple shader, read by the water and splash shaders
        //   3: ripple sources of this frame - read by the ripple shader
        //   4: splash particles - updated by the splash compute shader, read by the splash vertex shader
        //   5: splash particles created this frame - read by the splash compute shader
        VkShaderStageFlags all_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, all_stages, nullptr },
            { 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  1, all_stages, nullptr },
            { 2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  1, all_stages, nullptr },
            { 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr },
            { 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT, nullptr },
            { 5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr }
        };

        InitVkDestroyer( Context->LogicalDevice, DescriptorSetLayout );
        return CreateDescriptorSetLayout( *Context->LogicalDevice, bindings, *DescriptorSetLayout );
    }

    bool WaterRenderer::CreatePipelines() {
        // The parameters of the ripple steps and the splash update are push constants - small and different for every
        // dispatch. Both compute shaders use the same range, each with its own block.
        VkPushConstantRange compute_range = {
            VK_SHADER_STAGE_COMPUTE_BIT,                                                    // VkShaderStageFlags   stageFlags
            0,                                                                              // uint32_t             offset
            static_cast<uint32_t>( std::max( sizeof( RippleStepConstants ), sizeof( SplashUpdateConstants ) ) )  // uint32_t size
        };
        InitVkDestroyer( Context->LogicalDevice, PipelineLayout );
        if( !CreatePipelineLayout( *Context->LogicalDevice, { *DescriptorSetLayout }, { compute_range }, *PipelineLayout ) ) {
            return false;
        }

        // Compiled with shaders/compile.sh (or compile.bat); paths are relative to the working directory
        std::vector<unsigned char> water_vertex_code;
        std::vector<unsigned char> water_fragment_code;
        std::vector<unsigned char> sky_vertex_code;
        std::vector<unsigned char> sky_fragment_code;
        std::vector<unsigned char> object_vertex_code;
        std::vector<unsigned char> object_fragment_code;
        std::vector<unsigned char> ripple_compute_code;
        std::vector<unsigned char> splash_compute_code;
        std::vector<unsigned char> splash_vertex_code;
        std::vector<unsigned char> splash_fragment_code;
        if( !GetBinaryFileContents( "shaders/water_vert.spv", water_vertex_code ) ||
            !GetBinaryFileContents( "shaders/water_frag.spv", water_fragment_code ) ||
            !GetBinaryFileContents( "shaders/sky_vert.spv", sky_vertex_code ) ||
            !GetBinaryFileContents( "shaders/sky_frag.spv", sky_fragment_code ) ||
            !GetBinaryFileContents( "shaders/object_vert.spv", object_vertex_code ) ||
            !GetBinaryFileContents( "shaders/object_frag.spv", object_fragment_code ) ||
            !GetBinaryFileContents( "shaders/ripple_comp.spv", ripple_compute_code ) ||
            !GetBinaryFileContents( "shaders/splash_comp.spv", splash_compute_code ) ||
            !GetBinaryFileContents( "shaders/splash_vert.spv", splash_vertex_code ) ||
            !GetBinaryFileContents( "shaders/splash_frag.spv", splash_fragment_code ) ) {
            return false;
        }

        // Shader modules are only needed while creating the pipelines - they are destroyed at the end of this function
        VkDestroyer(VkShaderModule) water_vertex_module;
        InitVkDestroyer( Context->LogicalDevice, water_vertex_module );
        VkDestroyer(VkShaderModule) water_fragment_module;
        InitVkDestroyer( Context->LogicalDevice, water_fragment_module );
        VkDestroyer(VkShaderModule) sky_vertex_module;
        InitVkDestroyer( Context->LogicalDevice, sky_vertex_module );
        VkDestroyer(VkShaderModule) sky_fragment_module;
        InitVkDestroyer( Context->LogicalDevice, sky_fragment_module );
        VkDestroyer(VkShaderModule) object_vertex_module;
        InitVkDestroyer( Context->LogicalDevice, object_vertex_module );
        VkDestroyer(VkShaderModule) object_fragment_module;
        InitVkDestroyer( Context->LogicalDevice, object_fragment_module );
        VkDestroyer(VkShaderModule) ripple_compute_module;
        InitVkDestroyer( Context->LogicalDevice, ripple_compute_module );
        VkDestroyer(VkShaderModule) splash_compute_module;
        InitVkDestroyer( Context->LogicalDevice, splash_compute_module );
        VkDestroyer(VkShaderModule) splash_vertex_module;
        InitVkDestroyer( Context->LogicalDevice, splash_vertex_module );
        VkDestroyer(VkShaderModule) splash_fragment_module;
        InitVkDestroyer( Context->LogicalDevice, splash_fragment_module );
        if( !CreateShaderModule( *Context->LogicalDevice, water_vertex_code, *water_vertex_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, water_fragment_code, *water_fragment_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, sky_vertex_code, *sky_vertex_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, sky_fragment_code, *sky_fragment_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, object_vertex_code, *object_vertex_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, object_fragment_code, *object_fragment_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, ripple_compute_code, *ripple_compute_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, splash_compute_code, *splash_compute_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, splash_vertex_code, *splash_vertex_module ) ||
            !CreateShaderModule( *Context->LogicalDevice, splash_fragment_code, *splash_fragment_module ) ) {
            return false;
        }

        // The water is an ordinary opaque surface: the triangle pipeline (back-face culling, depth test and write) fits
        InitVkDestroyer( Context->LogicalDevice, WaterPipeline );
        if( !CreateTrianglePipeline( *Context->LogicalDevice, Targets->SurfaceFormat.format, Targets->DepthFormat, *PipelineLayout,
                                     *water_vertex_module, *water_fragment_module,
                                     { OceanGridVertex::GetBindingDescription() }, OceanGridVertex::GetAttributeDescriptions(),
                                     Context->MsaaSamples, *WaterPipeline ) ) {
            return false;
        }

        InitVkDestroyer( Context->LogicalDevice, SkyPipeline );
        if( !CreateBackgroundPipeline( *Context->LogicalDevice, Targets->SurfaceFormat.format, Targets->DepthFormat, *PipelineLayout,
                                       *sky_vertex_module, *sky_fragment_module, Context->MsaaSamples, *SkyPipeline ) ) {
            return false;
        }
        // Boxes: also opaque triangles - per-vertex data at binding 0, per-instance data at binding 1
        InitVkDestroyer( Context->LogicalDevice, ObjectPipeline );
        if( !CreateTrianglePipeline( *Context->LogicalDevice, Targets->SurfaceFormat.format, Targets->DepthFormat, *PipelineLayout,
                                     *object_vertex_module, *object_fragment_module,
                                     GetBoxBindingDescriptions(), GetBoxAttributeDescriptions(),
                                     Context->MsaaSamples, *ObjectPipeline ) ) {
            return false;
        }
        InitVkDestroyer( Context->LogicalDevice, RipplePipeline );
        if( !CreateComputePipeline( *Context->LogicalDevice, *PipelineLayout, *ripple_compute_module, *RipplePipeline ) ) {
            return false;
        }
        InitVkDestroyer( Context->LogicalDevice, SplashUpdatePipeline );
        if( !CreateComputePipeline( *Context->LogicalDevice, *PipelineLayout, *splash_compute_module, *SplashUpdatePipeline ) ) {
            return false;
        }
        InitVkDestroyer( Context->LogicalDevice, SplashDrawPipeline );
        if( !CreateBillboardPipeline( *Context->LogicalDevice, Targets->SurfaceFormat.format, Targets->DepthFormat, *PipelineLayout,
                                      *splash_vertex_module, *splash_fragment_module, Context->MsaaSamples, *SplashDrawPipeline ) ) {
            return false;
        }
        std::cout << "Water, sky, object, ripple and splash pipelines created" << std::endl;
        return true;
    }

    bool WaterRenderer::CreateCommandBuffers() {
        // Command buffers are re-recorded every frame, so they have to be resettable individually
        InitVkDestroyer( Context->LogicalDevice, CommandPool );
        if( !CreateCommandPool( *Context->LogicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, Context->QueueFamilyIndex, *CommandPool ) ) {
            return false;
        }

        return AllocateCommandBuffers( *Context->LogicalDevice, *CommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT, CommandBuffers );
    }

    bool WaterRenderer::CreateMeshes() {
        // Uploaded once - the waves are computed by the GPU, the grid itself never changes; the boxes are one mesh,
        // scaled and placed per instance
        return CreateOceanGridMesh( Context->Allocator, *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue,
                                    GridCellCount, GridHalfSize, GridCenterDensity, OceanGrid ) &&
               CreateBoxMesh( Context->Allocator, *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue,
                              BoxMeshData );
    }

    bool WaterRenderer::CreateUniformBuffers() {
        // Updated every frame by the CPU, so no staging buffer - it would only add a copy
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            UniformBuffers.emplace_back();

            // Mapped when it is created and kept mapped until it is destroyed
            if( !CreateBuffer( Context->Allocator, sizeof( OceanUniforms ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               MemoryLocation::HostVisibleMapped, UniformBuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool WaterRenderer::CreateInstanceBuffers() {
        // Written by the CPU every frame and read once by the GPU - host visible like the uniform buffers.
        // One per frame in flight, so the CPU never overwrites data the GPU is still drawing from.
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            InstanceBuffers.emplace_back();
            if( !CreateBuffer( Context->Allocator, sizeof( BoxInstance ) * FloatingObjects::MAX_INSTANCES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               MemoryLocation::HostVisibleMapped, InstanceBuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool WaterRenderer::CreateRippleField() {
        // Two height images. 32 bit float - storage images in this format are supported by every device.
        // Only accessed with imageLoad / imageStore, so no sampler; GENERAL is the layout for storage images.
        for( uint32_t i = 0; i < 2; ++i ) {
            if( !CreateImage( Context->Allocator, RIPPLE_CELL_COUNT, RIPPLE_CELL_COUNT, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_SFLOAT,
                              VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, RippleImages[i] ) ) {
                return false;
            }
            InitVkDestroyer( Context->LogicalDevice, RippleImageViews[i] );
            if( !CreateImageView( *Context->LogicalDevice, *RippleImages[i], VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT,
                                  VK_IMAGE_ASPECT_COLOR_BIT, *RippleImageViews[i] ) ) {
                return false;
            }
        }

        // Calm water at the start: both images in GENERAL and cleared to 0
        VkCommandBuffer command_buffer;
        if( !BeginSingleTimeCommands( *Context->LogicalDevice, *Context->TransferCommandPool, command_buffer ) ) {
            return false;
        }
        VkClearColorValue zero = {};
        VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        for( auto & image : RippleImages ) {
            RecordImageBarrier( command_buffer, *image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
                                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT );
            vkCmdClearColorImage( command_buffer, *image, VK_IMAGE_LAYOUT_GENERAL, &zero, 1, &range );
            RecordImageBarrier( command_buffer, *image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1,
                                VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL,
                                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT );
        }
        if( !EndSingleTimeCommands( *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue, command_buffer ) ) {
            return false;
        }

        // Sources of all steps of a frame, written by the CPU - one buffer per frame in flight
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            RippleSourceBuffers.emplace_back();
            if( !CreateBuffer( Context->Allocator, sizeof( RippleSource ) * MAX_RIPPLE_SOURCES_PER_STEP * MAX_RIPPLE_STEPS_PER_FRAME,
                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, MemoryLocation::HostVisibleMapped, RippleSourceBuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool WaterRenderer::CreateSplashBuffers() {
        // All particles start dead (life 0). Only the GPU touches them afterwards, so device local.
        std::vector<SplashParticle> dead_particles( SPLASH_PARTICLE_COUNT, SplashParticle{ glm::vec4( 0.0f ), glm::vec4( 0.0f ) } );
        if( !CreateDeviceLocalBufferWithData( Context->Allocator, *Context->LogicalDevice, *Context->TransferCommandPool, Context->GraphicsAndPresentQueue,
                                              dead_particles.data(), sizeof( SplashParticle ) * dead_particles.size(),
                                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, SplashParticleBuffer ) ) {
            return false;
        }

        // New particles of a frame, written by the CPU - one buffer per frame in flight
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            SplashSpawnBuffers.emplace_back();
            if( !CreateBuffer( Context->Allocator, sizeof( SplashParticle ) * MAX_SPLASH_SPAWNS_PER_FRAME, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               MemoryLocation::HostVisibleMapped, SplashSpawnBuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool WaterRenderer::CreateUniformDescriptorSets() {
        InitVkDestroyer( Context->LogicalDevice, DescriptorPool );
        if( !CreateDescriptorPool( *Context->LogicalDevice, false, MAX_FRAMES_IN_FLIGHT,
                                   { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
                                     { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2 * MAX_FRAMES_IN_FLIGHT },
                                     { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 * MAX_FRAMES_IN_FLIGHT } }, *DescriptorPool ) ) {
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
        UpdateDescriptorSetsWithUniformBuffers( *Context->LogicalDevice, DescriptorSets, 0, uniform_buffers, sizeof( OceanUniforms ) );
        UpdateDescriptorSetsWithStorageImage( *Context->LogicalDevice, DescriptorSets, 1, *RippleImageViews[0], VK_IMAGE_LAYOUT_GENERAL );
        UpdateDescriptorSetsWithStorageImage( *Context->LogicalDevice, DescriptorSets, 2, *RippleImageViews[1], VK_IMAGE_LAYOUT_GENERAL );
        std::vector<VkBuffer> source_buffers;
        for( auto & source_buffer : RippleSourceBuffers ) {
            source_buffers.push_back( *source_buffer );
        }
        UpdateDescriptorSetsWithStorageBuffers( *Context->LogicalDevice, DescriptorSets, 3, source_buffers );
        UpdateDescriptorSetsWithStorageBuffers( *Context->LogicalDevice, DescriptorSets, 4,
                                                std::vector<VkBuffer>( MAX_FRAMES_IN_FLIGHT, *SplashParticleBuffer ) );
        std::vector<VkBuffer> spawn_buffers;
        for( auto & spawn_buffer : SplashSpawnBuffers ) {
            spawn_buffers.push_back( *spawn_buffer );
        }
        UpdateDescriptorSetsWithStorageBuffers( *Context->LogicalDevice, DescriptorSets, 5, spawn_buffers );
        return true;
    }

    bool WaterRenderer::CreateSyncObjects() {
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

    void WaterRenderer::UpdateObjects( uint32_t frame_index ) {
        auto now = std::chrono::steady_clock::now();
        float frame_seconds = std::chrono::duration<float>( now - LastFrameTime ).count();
        LastFrameTime = now;

        Objects.Update( frame_seconds );
        Objects.GetInstances( Instances );
        InstanceCount = static_cast<uint32_t>( Instances.size() );
        if( InstanceCount > 0 ) {
            CopyDataToBuffer( InstanceBuffers[frame_index], 0, sizeof( BoxInstance ) * InstanceCount, Instances.data() );
        }

        // One ripple step per simulation step of this frame (normally 0 to 2, at most 12 after a long frame)
        Objects.TakeRippleSources( RippleSources, RippleStepSourceCounts );
        if( RippleStepSourceCounts.size() > MAX_RIPPLE_STEPS_PER_FRAME ) {
            RippleStepSourceCounts.resize( MAX_RIPPLE_STEPS_PER_FRAME );
        }
        float time_step = FloatingObjects::TIME_STEP;
        float cell_size = 2.0f * RIPPLE_HALF_SIZE / RIPPLE_CELL_COUNT;
        float courant = RIPPLE_WAVE_SPEED * time_step / cell_size;
        RippleSteps.clear();
        uint32_t source_offset = 0;
        for( uint32_t count : RippleStepSourceCounts ) {
            RippleSteps.push_back( {
                RippleCurrentIndex,
                source_offset,
                count,
                RIPPLE_CELL_COUNT,
                RIPPLE_HALF_SIZE,
                courant * courant,
                std::pow( RIPPLE_DAMPING_PER_SECOND, time_step ),
                RIPPLE_SOURCE_STRENGTH
            } );
            source_offset += count;
            // The step writes the new heights into the other image
            RippleCurrentIndex = 1 - RippleCurrentIndex;
        }
        if( source_offset > 0 ) {
            CopyDataToBuffer( RippleSourceBuffers[frame_index], 0, sizeof( RippleSource ) * source_offset, RippleSources.data() );
        }

        // New splash particles go into the ring after the previous ones; they move on the simulation clock
        Objects.TakeSplashParticles( SplashSpawns );
        uint32_t spawn_count = static_cast<uint32_t>( std::min<size_t>( SplashSpawns.size(), MAX_SPLASH_SPAWNS_PER_FRAME ) );
        if( spawn_count > 0 ) {
            CopyDataToBuffer( SplashSpawnBuffers[frame_index], 0, sizeof( SplashParticle ) * spawn_count, SplashSpawns.data() );
        }
        float render_time = Objects.GetRenderTime();
        SplashUpdate = { std::max( render_time - LastRenderTime, 0.0f ), SplashRingOffset, spawn_count, SPLASH_PARTICLE_COUNT };
        LastRenderTime = render_time;
        SplashRingOffset = (SplashRingOffset + spawn_count) % SPLASH_PARTICLE_COUNT;
    }

    void WaterRenderer::UpdateUniformBuffer( uint32_t frame_index ) {
        // The simulation clock - the waves have to be drawn at the same time the boxes were simulated for
        float time = Objects.GetRenderTime();

        // Y is up in this scene
        float angle = time * OrbitSpeed;
        glm::vec3 camera_position( OrbitRadius * std::cos( angle ), OrbitHeight, OrbitRadius * std::sin( angle ) );

        OceanUniforms uniforms;
        uniforms.View = glm::lookAt( camera_position, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
        // Far plane beyond the end of the grid. Depth range 0..1 because of GLM_FORCE_DEPTH_ZERO_TO_ONE (CMakeLists.txt).
        uniforms.Projection = glm::perspective( glm::radians( 60.0f ), Targets->Extent.width / static_cast<float>( Targets->Extent.height ), 0.5f, 2000.0f );
        // GLM was made for OpenGL, where clip space Y points up - in Vulkan it points down
        uniforms.Projection[1][1] *= -1;
        // For the sky: from a pixel back to a world space direction
        uniforms.InverseViewProjection = glm::inverse( uniforms.Projection * uniforms.View );
        uniforms.CameraPositionAndTime = glm::vec4( camera_position, time );
        uniforms.SunDirection = glm::vec4( SunDirection, 0.0f );
        // After this frame's ripple steps (UpdateObjects runs first)
        uniforms.RippleArea = glm::vec4( RIPPLE_HALF_SIZE, static_cast<float>( RIPPLE_CELL_COUNT ), static_cast<float>( RippleCurrentIndex ), 0.0f );
        PackGerstnerWaves( Waves, uniforms.Waves );

        CopyDataToBuffer( UniformBuffers[frame_index], 0, sizeof( uniforms ), &uniforms );
    }

    bool WaterRenderer::DrawFrame() {
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

        // Safe to overwrite - the fence wait above guarantees the GPU is done with this frame's uniform and instance buffers.
        // Objects first: the uniform buffer uses their clock.
        UpdateObjects( CurrentFrame );
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
        WaterSceneDraw scene = {
            *PipelineLayout,
            DescriptorSets[CurrentFrame],
            *WaterPipeline,
            *OceanGrid.VertexBuffer,
            *OceanGrid.IndexBuffer,
            OceanGrid.IndexCount,
            *SkyPipeline,
            *ObjectPipeline,
            *BoxMeshData.VertexBuffer,
            *BoxMeshData.IndexBuffer,
            BoxMeshData.IndexCount,
            *InstanceBuffers[CurrentFrame],
            InstanceCount,
            *RipplePipeline,
            RippleSteps.data(),
            static_cast<uint32_t>( RippleSteps.size() ),
            *SplashUpdatePipeline,
            SplashUpdate,
            *SplashDrawPipeline
        };
        if( !ResetCommandBuffer( CommandBuffers[CurrentFrame], false ) ||
            !RecordWaterCommandBuffer( CommandBuffers[CurrentFrame], attachments, Targets->Extent, scene ) ) {
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
