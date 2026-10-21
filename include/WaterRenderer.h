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

#ifndef WATER_RENDERER
#define WATER_RENDERER

#include "Common.h"
#include "Allocator.h"
#include "Mesh.h"
#include "Ocean.h"
#include "FloatingObjects.h"
#include "Ripples.h"
#include "Splashes.h"
#include <chrono>

namespace VulkanCookbook {

    class AppWindow;
    class VulkanContext;
    class SwapchainTargets;

    // Ocean scene: a grid displaced by Gerstner waves in the vertex shader, lit per pixel with Fresnel sky reflections and
    // sun glints, under a procedural sky, seen from a camera that slowly orbits the center. Boxes float on the waves
    // (FloatingObjects, simulated on the CPU) and are drawn instanced. Their ripples and wakes are simulated on the GPU
    // (shaders/ripple.comp) in a height field around the center and added to the waves; where they hit the water fast,
    // spray particles fly (shaders/splash.comp, drawn as soft quads). Owns everything that is needed for that and doesn't
    // depend on the size of the window - the pipelines, meshes, uniform and instance buffers, descriptors, command
    // buffers and the synchronization objects of the frames in flight.
    class WaterRenderer {
    public:
        // Number of frames the CPU may record ahead of the GPU
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        WaterRenderer() = default;
        WaterRenderer( WaterRenderer const & ) = delete;
        WaterRenderer & operator=( WaterRenderer const & ) = delete;

        // The context, targets and window have to outlive this object. Before it is destroyed, the GPU has to be idle
        // (VulkanContext::WaitIdle).
        bool Init( VulkanContext    & context,
                   SwapchainTargets & targets,
                   AppWindow        & window );
        // Renders and presents one frame. Recreates the swapchain targets when the window was resized.
        bool DrawFrame();

    private:
        bool CreateUniformDescriptorSetLayout();
        bool CreatePipelines();
        bool CreateCommandBuffers();
        bool CreateMeshes();
        bool CreateUniformBuffers();
        bool CreateInstanceBuffers();
        bool CreateRippleField();
        bool CreateSplashBuffers();
        bool CreateUniformDescriptorSets();
        bool CreateSyncObjects();
        void UpdateUniformBuffer( uint32_t frame_index );
        // Advances the floating objects, writes their transforms to this frame's instance buffer and their ripple
        // sources to this frame's source buffer, and prepares the ripple steps (RippleSteps)
        void UpdateObjects( uint32_t frame_index );

        VulkanContext    * Context = nullptr;
        SwapchainTargets * Targets = nullptr;
        AppWindow        * TargetWindow = nullptr;

        // Members are destroyed in reverse order of declaration, so they are declared in the order they are created
        VkDestroyer(VkDescriptorSetLayout)              DescriptorSetLayout;
        // Shared by the water and the sky pipeline
        VkDestroyer(VkPipelineLayout)                   PipelineLayout;
        VkDestroyer(VkPipeline)                         WaterPipeline;
        VkDestroyer(VkPipeline)                         SkyPipeline;
        VkDestroyer(VkPipeline)                         ObjectPipeline;
        VkDestroyer(VkPipeline)                         RipplePipeline;
        VkDestroyer(VkPipeline)                         SplashUpdatePipeline;
        VkDestroyer(VkPipeline)                         SplashDrawPipeline;

        VkDestroyer(VkCommandPool)                      CommandPool;
        // Command buffers are freed together with the pool - one per frame in flight
        std::vector<VkCommandBuffer>                    CommandBuffers;

        Mesh                                            OceanGrid;
        Mesh                                            BoxMeshData;
        std::vector<GerstnerWave>                       Waves;
        FloatingObjects                                 Objects;

        // One uniform buffer per frame in flight, persistently mapped
        std::vector<AllocatedBuffer>                    UniformBuffers;
        // One box instance buffer per frame in flight (FloatingObjects::MAX_INSTANCES boxes), persistently mapped
        std::vector<AllocatedBuffer>                    InstanceBuffers;
        std::vector<BoxInstance>                        Instances;
        uint32_t                                        InstanceCount = 0;

        // Ripple field: two height images that take turns (RippleCurrentIndex is the one with the newest heights),
        // shared by all frames; ripple sources per frame in flight, persistently mapped
        AllocatedImage                                  RippleImages[2];
        VkDestroyer(VkImageView)                        RippleImageViews[2];
        uint32_t                                        RippleCurrentIndex = 0;
        std::vector<AllocatedBuffer>                    RippleSourceBuffers;
        std::vector<RippleSource>                       RippleSources;
        std::vector<uint32_t>                           RippleStepSourceCounts;
        std::vector<RippleStepConstants>                RippleSteps;

        // Splash particles: one ring buffer on the GPU, shared by all frames; new particles per frame in flight
        AllocatedBuffer                                 SplashParticleBuffer;
        std::vector<AllocatedBuffer>                    SplashSpawnBuffers;
        std::vector<SplashParticle>                     SplashSpawns;
        uint32_t                                        SplashRingOffset = 0;
        SplashUpdateConstants                           SplashUpdate = {};
        float                                           LastRenderTime = 0.0f;

        // Descriptor sets are freed together with the pool - one per frame in flight
        VkDestroyer(VkDescriptorPool)                   DescriptorPool;
        std::vector<VkDescriptorSet>                    DescriptorSets;

        // One per frame in flight
        std::vector<VkDestroyer(VkSemaphore)>           ImageAvailableSemaphores;
        std::vector<VkDestroyer(VkFence)>               InFlightFences;

        uint32_t                                        CurrentFrame = 0;
        // Real time of the previous frame - the simulation advances by the difference.
        // steady_clock is monotonic - high_resolution_clock may be the system clock, which can jump.
        std::chrono::steady_clock::time_point           LastFrameTime;
    };

} // namespace VulkanCookbook

#endif // WATER_RENDERER
