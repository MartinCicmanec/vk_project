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

#ifndef PARTICLE_RENDERER
#define PARTICLE_RENDERER

#include "Common.h"
#include "Allocator.h"
#include <chrono>

namespace VulkanCookbook {

    class AppWindow;
    class VulkanContext;
    class SwapchainTargets;

    // Particle system of the vulkan-tutorial.com "Compute Shader" chapter: a compute shader moves the particles in
    // shader storage buffers every frame, and the same buffers are drawn as points. Owns everything that is needed for
    // that and doesn't depend on the size of the window - the compute and graphics pipelines, particle and uniform
    // buffers, descriptors, command buffers and the synchronization objects of the frames in flight.
    class ParticleRenderer {
    public:
        // Number of frames the CPU may record ahead of the GPU
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
        // Has to be a multiple of the compute shader's local size (256)
        static constexpr uint32_t PARTICLE_COUNT = 8192;

        ParticleRenderer() = default;
        ParticleRenderer( ParticleRenderer const & ) = delete;
        ParticleRenderer & operator=( ParticleRenderer const & ) = delete;

        // The context, targets and window have to outlive this object. Before it is destroyed, the GPU has to be idle
        // (VulkanContext::WaitIdle).
        bool Init( VulkanContext    & context,
                   SwapchainTargets & targets,
                   AppWindow        & window );
        // Moves the particles, renders and presents one frame. Recreates the swapchain targets when the window was resized.
        bool DrawFrame();

    private:
        bool CreateComputeDescriptorSetLayout();
        bool CreatePipelines();
        bool CreateCommandBuffers();
        bool CreateShaderStorageBuffers();
        bool CreateUniformBuffers();
        bool CreateComputeDescriptorSets();
        bool CreateSyncObjects();
        void UpdateUniformBuffer( uint32_t frame_index );

        VulkanContext    * Context = nullptr;
        SwapchainTargets * Targets = nullptr;
        AppWindow        * TargetWindow = nullptr;

        // Members are destroyed in reverse order of declaration, so they are declared in the order they are created
        VkDestroyer(VkDescriptorSetLayout)              ComputeDescriptorSetLayout;
        VkDestroyer(VkPipelineLayout)                   ComputePipelineLayout;
        VkDestroyer(VkPipeline)                         ComputePipeline;
        // The graphics pipeline uses no descriptors - the particles come in as vertices
        VkDestroyer(VkPipelineLayout)                   GraphicsPipelineLayout;
        VkDestroyer(VkPipeline)                         GraphicsPipeline;

        VkDestroyer(VkCommandPool)                      CommandPool;
        // Command buffers are freed together with the pool - one per frame in flight
        std::vector<VkCommandBuffer>                    CommandBuffers;

        // One particle buffer per frame in flight: frame i reads buffer i - 1 and writes buffer i
        std::vector<AllocatedBuffer>                    ShaderStorageBuffers;
        // One uniform buffer per frame in flight, persistently mapped
        std::vector<AllocatedBuffer>                    UniformBuffers;

        // Descriptor sets are freed together with the pool - one per frame in flight
        VkDestroyer(VkDescriptorPool)                   DescriptorPool;
        std::vector<VkDescriptorSet>                    ComputeDescriptorSets;

        // One per frame in flight
        std::vector<VkDestroyer(VkSemaphore)>           ImageAvailableSemaphores;
        std::vector<VkDestroyer(VkFence)>               InFlightFences;

        uint32_t                                        CurrentFrame = 0;
        // Time of the previous frame, for the particle movement per frame
        std::chrono::steady_clock::time_point           LastFrameTime;
    };

} // namespace VulkanCookbook

#endif // PARTICLE_RENDERER
