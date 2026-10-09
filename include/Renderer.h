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

#ifndef RENDERER
#define RENDERER

#include "Common.h"
#include "Allocator.h"
#include "Mesh.h"
#include "Texture.h"
#include <chrono>

namespace VulkanCookbook {

    class AppWindow;
    class VulkanContext;
    class SwapchainTargets;

    // Draws the scene: one textured mesh that rotates. Owns everything that is needed for that and doesn't depend on
    // the size of the window - the pipeline, descriptors, mesh, texture, uniform buffers, command buffers and the
    // synchronization objects of the frames in flight.
    class Renderer {
    public:
        // Number of frames the CPU may record ahead of the GPU
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        Renderer() = default;
        Renderer( Renderer const & ) = delete;
        Renderer & operator=( Renderer const & ) = delete;

        // The context, targets and window have to outlive this object. Before it is destroyed, the GPU has to be idle
        // (VulkanContext::WaitIdle).
        bool Init( VulkanContext    & context,
                   SwapchainTargets & targets,
                   AppWindow        & window );
        // Renders and presents one frame. Recreates the swapchain targets when the window was resized.
        bool DrawFrame();

    private:
        bool CreateUniformDescriptorSetLayout();
        bool CreatePipeline();
        bool CreateCommandBuffers();
        bool LoadScene();
        bool CreateUniformBuffers();
        bool CreateUniformDescriptorSets();
        bool CreateSyncObjects();
        void UpdateUniformBuffer( uint32_t frame_index );

        VulkanContext    * Context = nullptr;
        SwapchainTargets * Targets = nullptr;
        AppWindow        * TargetWindow = nullptr;

        // Members are destroyed in reverse order of declaration, so they are declared in the order they are created
        VkDestroyer(VkDescriptorSetLayout)              DescriptorSetLayout;
        VkDestroyer(VkPipelineLayout)                   PipelineLayout;
        VkDestroyer(VkPipeline)                         GraphicsPipeline;

        VkDestroyer(VkCommandPool)                      CommandPool;
        // Command buffers are freed together with the pool - one per frame in flight
        std::vector<VkCommandBuffer>                    CommandBuffers;

        Mesh                                            SceneMesh;
        Texture                                         SceneTexture;

        // One uniform buffer per frame in flight, persistently mapped
        std::vector<AllocatedBuffer>                    UniformBuffers;

        // Descriptor sets are freed together with the pool - one per frame in flight
        VkDestroyer(VkDescriptorPool)                   DescriptorPool;
        std::vector<VkDescriptorSet>                    DescriptorSets;

        // One per frame in flight
        std::vector<VkDestroyer(VkSemaphore)>           ImageAvailableSemaphores;
        std::vector<VkDestroyer(VkFence)>               InFlightFences;

        uint32_t                                        CurrentFrame = 0;
        // steady_clock is monotonic - high_resolution_clock may be the system clock, which can jump
        std::chrono::steady_clock::time_point           StartTime;
    };

} // namespace VulkanCookbook

#endif // RENDERER
