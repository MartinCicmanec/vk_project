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
// Application

#ifndef APPLICATION
#define APPLICATION

#include "Common.h"
#include "Vertex.h"
#include <chrono>

struct GLFWwindow;

namespace VulkanCookbook {

    // Runs the given function when it is destroyed.
    // Used for non-Vulkan resources, so they are released in the right order relative to the VkDestroyer members.
    struct ScopeGuard {
        std::function<void()> Function;
        ~ScopeGuard() { if( Function ) { Function(); } }
    };

    class Application {
    public:
        // Number of frames the CPU may record ahead of the GPU
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        Application( bool enable_validation,
                     bool enable_debug_callbacks );
        ~Application();

        bool Init();
        bool Run();

    private:
        bool InitVulkanLoader();
        bool CreateInstance();
        bool SetupDebugMessenger();
        bool ListPhysicalDevices();
        void CreateGeometryShaderDeviceDemo();
        bool InitWindow();
        bool CreateSurface();
        bool PickPhysicalDevice();
        bool CreateDevice();
        bool SelectSwapchainSettings();
        bool SelectDepthFormat();
        bool CreateDepthResources();
        bool CreateSwapchainObjects();
        bool CreateTriangleRenderPass();
        bool CreateUniformDescriptorSetLayout();
        bool CreatePipeline();
        bool CreateFramebuffers();
        bool CreateCommandBuffers();
        bool LoadModel();
        bool CreateVertexAndIndexBuffers();
        bool CreateTexture();
        bool CreateUniformBuffers();
        bool CreateUniformDescriptorSets();
        void UpdateUniformBuffer( uint32_t frame_index );
        bool CreateSyncObjects();
        bool CreateRenderFinishedSemaphores();
        bool RecreateSwapchain();
        bool DrawFrame();

        static void FramebufferResizeCallback( GLFWwindow * window,
                                               int          width,
                                               int          height );

        bool                                            EnableValidation;
        bool                                            EnableDebugCallbacks;

        // Members are destroyed in reverse order of declaration, so they are declared in the order they are created:
        // sync objects -> command pool -> framebuffers -> pipeline -> pipeline layout -> render pass -> image views
        // -> swapchain -> device -> surface -> window -> GLFW -> debug messenger -> instance -> Vulkan library
        LIBRARY_TYPE                                    VulkanLibrary = nullptr;
        ScopeGuard                                      ReleaseVulkanLibrary;

        VkDestroyer(VkInstance)                         Instance;
        VkDestroyer(VkDebugUtilsMessengerEXT)           DebugMessenger;
        std::vector<VkPhysicalDevice>                   PhysicalDevices;

        ScopeGuard                                      TerminateGlfw;
        GLFWwindow                                    * GlfwWindow = nullptr;
        ScopeGuard                                      DestroyGlfwWindow;

        VkDestroyer(VkSurfaceKHR)                       PresentationSurface;
        VkPhysicalDevice                                PhysicalDevice = VK_NULL_HANDLE;
        uint32_t                                        QueueFamilyIndex = 0;
        VkDestroyer(VkDevice)                           LogicalDevice;
        // Queue used for both graphics commands and presentation
        VkQueue                                         GraphicsAndPresentQueue = VK_NULL_HANDLE;

        VkPresentModeKHR                                PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkSurfaceFormatKHR                              SwapchainSurfaceFormat = {};
        VkExtent2D                                      SwapchainExtent = {};
        VkDestroyer(VkSwapchainKHR)                     Swapchain;
        // Swapchain images are owned by the swapchain and destroyed together with it
        std::vector<VkImage>                            SwapchainImages;
        std::vector<VkDestroyer(VkImageView)>           SwapchainImageViews;

        // One depth image shared by all framebuffers, recreated together with the swapchain
        VkFormat                                        DepthFormat = VK_FORMAT_UNDEFINED;
        VkDestroyer(VkDeviceMemory)                     DepthImageMemory;
        VkDestroyer(VkImage)                            DepthImage;
        VkDestroyer(VkImageView)                        DepthImageView;

        VkDestroyer(VkRenderPass)                       RenderPass;
        VkDestroyer(VkDescriptorSetLayout)              DescriptorSetLayout;
        VkDestroyer(VkPipelineLayout)                   PipelineLayout;
        VkDestroyer(VkPipeline)                         GraphicsPipeline;
        std::vector<VkDestroyer(VkFramebuffer)>         SwapchainFramebuffers;

        VkDestroyer(VkCommandPool)                      CommandPool;
        // Command buffers are freed together with the pool
        std::vector<VkCommandBuffer>                    CommandBuffers;
        // For short-lived command buffers that upload data (buffer copies, texture uploads)
        VkDestroyer(VkCommandPool)                      TransferCommandPool;

        // Geometry loaded from the model file - 32 bit indices, a model can have more than 65535 vertices
        std::vector<Vertex>                             ModelVertices;
        std::vector<uint32_t>                           ModelIndices;

        // Memory is declared before its buffer, so it is freed after the buffer is destroyed
        VkDestroyer(VkDeviceMemory)                     VertexBufferMemory;
        VkDestroyer(VkBuffer)                           VertexBuffer;
        VkDestroyer(VkDeviceMemory)                     IndexBufferMemory;
        VkDestroyer(VkBuffer)                           IndexBuffer;

        VkDestroyer(VkDeviceMemory)                     TextureImageMemory;
        VkDestroyer(VkImage)                            TextureImage;
        VkDestroyer(VkImageView)                        TextureImageView;
        VkDestroyer(VkSampler)                          TextureSampler;

        // One uniform buffer per frame in flight, persistently mapped
        std::vector<VkDestroyer(VkDeviceMemory)>        UniformBuffersMemory;
        std::vector<VkDestroyer(VkBuffer)>              UniformBuffers;
        std::vector<void *>                             UniformBuffersMapped;

        // Descriptor sets are freed together with the pool - one per frame in flight
        VkDestroyer(VkDescriptorPool)                   DescriptorPool;
        std::vector<VkDescriptorSet>                    DescriptorSets;

        // One per frame in flight
        std::vector<VkDestroyer(VkSemaphore)>           ImageAvailableSemaphores;
        std::vector<VkDestroyer(VkFence)>               InFlightFences;
        // One per swapchain image
        std::vector<VkDestroyer(VkSemaphore)>           RenderFinishedSemaphores;

        uint32_t                                        CurrentFrame = 0;
        // Set by the GLFW callback - not every platform reports a resize through VK_ERROR_OUT_OF_DATE_KHR
        bool                                            FramebufferResized = false;
        // steady_clock is monotonic - high_resolution_clock may be the system clock, which can jump
        std::chrono::steady_clock::time_point           StartTime;
    };

} // namespace VulkanCookbook

#endif // APPLICATION
