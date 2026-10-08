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

#if defined VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined VK_USE_PLATFORM_XLIB_KHR
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
    #define GLFW_EXPOSE_NATIVE_WAYLAND
#else
    #error "GLFW window setup is only implemented for the WIN32, XLIB and WAYLAND platforms."
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.h"
#include "Debug.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "Presentation.h"
#include "Swapchain.h"
#include "Image.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Tools.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"
#include "Synchronization.h"
#include "Queue.h"
#include "Drawing.h"
#include "Buffer.h"
#include "Vertex.h"
#include "Descriptor.h"
#include "UniformBufferObject.h"
#include "Sampler.h"
#include "Texture.h"
#include "Model.h"

namespace VulkanCookbook {

    namespace {
        // Paths are relative to the working directory, like the shaders.
        // "Viking room" model by nigelgoh (CC BY 4.0), prepared for vulkan-tutorial.com.
        const std::string ModelPath   = "models/viking_room.obj";
        const std::string TexturePath = "textures/viking_room.png";
    }

    Application::Application( bool enable_validation,
                              bool enable_debug_callbacks ) :
        EnableValidation( enable_validation ),
        EnableDebugCallbacks( enable_debug_callbacks ) {
    }

    Application::~Application() {
        // Runs before the members are destroyed: nothing may be destroyed while the GPU is still using it
        if( LogicalDevice ) {
            vkDeviceWaitIdle( *LogicalDevice );
        }
    }

    bool Application::Init() {
        if( !InitVulkanLoader() ||
            !CreateInstance() ||
            !SetupDebugMessenger() ||
            !ListPhysicalDevices() ) {
            return false;
        }

        CreateGeometryShaderDeviceDemo();

        return InitWindow() &&
               CreateSurface() &&
               PickPhysicalDevice() &&
               CreateDevice() &&
               SelectSwapchainSettings() &&
               SelectDepthFormat() &&
               CreateSwapchainObjects() &&
               CreateTriangleRenderPass() &&
               CreateUniformDescriptorSetLayout() &&
               CreatePipeline() &&
               CreateCommandBuffers() &&
               CreateDepthResources() &&
               CreateFramebuffers() &&
               LoadModel() &&
               CreateVertexAndIndexBuffers() &&
               CreateTexture() &&
               CreateUniformBuffers() &&
               CreateUniformDescriptorSets() &&
               CreateSyncObjects() &&
               CreateRenderFinishedSemaphores();
    }

    bool Application::Run() {
        StartTime = std::chrono::steady_clock::now();
        while( !glfwWindowShouldClose( GlfwWindow ) ) {
            glfwPollEvents();
            if( !DrawFrame() ) {
                return false;
            }
        }
        return true;
    }

    bool Application::InitVulkanLoader() {
        if( !ConnectWithVulkanLoaderLibrary( VulkanLibrary ) ) {
            return false;
        }
        ReleaseVulkanLibrary.Function = [this]() { ReleaseVulkanLoaderLibrary( VulkanLibrary ); };

        if( !LoadFunctionExportedFromVulkanLoaderLibrary( VulkanLibrary ) ) {
            return false;
        }

        if( !LoadGlobalLevelFunctions() ) {
            return false;
        }

        if( vkGetInstanceProcAddr == nullptr ) {
            std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
            return false;
        }

        if((vkEnumerateInstanceExtensionProperties == nullptr) || (vkEnumerateInstanceLayerProperties == nullptr)) {
            std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
            return false;
        }
        return true;
    }

    bool Application::CreateInstance() {
        std::vector<VkLayerProperties> available_layers;
        CheckAvailableInstanceLayers(available_layers);

        InitVkDestroyer(Instance);

        std::vector<char const*> desired_extensions = {
            //"VK_KHR_device_group_creation",
            //"VK_KHR_get_display_properties2",
            //"VK_KHR_get_physical_device_properties2",
            //"VK_KHR_get_surface_capabilities2",
            //"VK_KHR_surface_protected_capabilities",
            "VK_KHR_display",
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
            //"VK_EXT_acquire_drm_display",
            //"VK_EXT_acquire_xlib_display"
            };

        std::vector<char const*> desired_layers = {};

        if (EnableValidation == true) {
            desired_layers.push_back("VK_LAYER_KHRONOS_validation");
        }

        for( auto & layer : desired_layers ) {
            if( !IsLayerSupported( available_layers, layer ) ) {
                std::cout << "Layer named '" << layer << "' is not supported by an Instance object." << std::endl;
                return false;
            }
        }

        if( !CreateVulkanInstanceWithWSIExtensionsEnabled(*Instance, desired_extensions, desired_layers) ) {
            return false;
        }
        return LoadInstanceLevelFunctions(*Instance, desired_extensions);
    }

    bool Application::SetupDebugMessenger() {
        InitVkDestroyer(Instance, DebugMessenger);
        if (EnableDebugCallbacks == true) {
            VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
            messengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            messengerCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            messengerCreateInfo.pfnUserCallback = &MyDebugMessengerCallback;
            messengerCreateInfo.pUserData       = nullptr;

            // Register the callback
            VkResult result = vkCreateDebugUtilsMessengerEXT(*Instance, &messengerCreateInfo, nullptr, &*DebugMessenger);
            if( (result != VK_SUCCESS) || !DebugMessenger ) {
                std::cout << "Could not create debug messenger." << std::endl;
                return false;
            };
        }
        return true;
    }

    bool Application::ListPhysicalDevices() {
        if( !EnumerateAvailablePhysicalDevices(*Instance, PhysicalDevices) ) {
            return false;
        }

        std::cout << "Found devices:" << std::endl;
        for (auto & physical_device : PhysicalDevices) {
            std::vector<VkExtensionProperties> available_device_extensions;
            CheckAvailableDeviceExtensions(physical_device, available_device_extensions);

            VkPhysicalDeviceFeatures deviceFeatures;
            VkPhysicalDeviceProperties deviceProperties;
            GetFeaturesAndPropertiesOfPhysicalDevice(physical_device, deviceFeatures, deviceProperties);
            std::cout << "\t" << deviceProperties.deviceName << std::endl;

            std::vector<VkQueueFamilyProperties> queue_families;
            CheckAvailableQueueFamiliesAndTheirProperties(physical_device, queue_families);

            std::cout << "\t\tAvailable queue families : " << queue_families.size() << std::endl;

            uint32_t queue_family_index = 0;
            VkQueueFlags desired_capabilities = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

            if( SelectIndexOfQueueFamilyWithDesiredCapabilities(physical_device, desired_capabilities, queue_family_index) ) {
                std::cout << "\t\tQueue family index : " << queue_family_index << std::endl;
            } else {
                std::cout << "\t\tNo queue family with graphics, compute and transfer support" << std::endl;
            }
        }
        return true;
    }

    void Application::CreateGeometryShaderDeviceDemo() {
        // Chapter 1 recipe: device with geometry shaders and graphics + compute queues.
        // Only a demonstration - it is destroyed at the end of this function and a device with swapchain support is created later.
        VkDestroyer(VkDevice) geometry_device;
        InitVkDestroyer(geometry_device);
        VkQueue graphics_queue = VK_NULL_HANDLE;
        VkQueue compute_queue = VK_NULL_HANDLE;
        if( !CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(*Instance, *geometry_device, graphics_queue, compute_queue) ) {
            std::cout << "Could not create a logical device with geometry shaders, graphics and compute queues." << std::endl;
        }
    }

    bool Application::InitWindow() {
        TerminateGlfw.Function = []() { glfwTerminate(); };

        // The surface is created for one specific window system, so GLFW has to use the same one (GLFW 3.4+).
        // Without this an Xlib build started in a Wayland session gets a Wayland window and no X11 handles.
#if defined GLFW_PLATFORM
    #if defined VK_USE_PLATFORM_XLIB_KHR
        glfwInitHint( GLFW_PLATFORM, GLFW_PLATFORM_X11 );
    #elif defined VK_USE_PLATFORM_WAYLAND_KHR
        glfwInitHint( GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND );
    #endif
#endif

        if( glfwInit() != GLFW_TRUE ) {
            std::cout << "Could not initialize GLFW." << std::endl;
            return false;
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GlfwWindow = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
        if( !GlfwWindow ) {
            std::cout << "Could not create GLFW window." << std::endl;
            return false;
        }
        DestroyGlfwWindow.Function = [this]() { glfwDestroyWindow( GlfwWindow ); };

        // GLFW can't call a member function, so the callback gets the Application through the window's user pointer
        glfwSetWindowUserPointer( GlfwWindow, this );
        glfwSetFramebufferSizeCallback( GlfwWindow, FramebufferResizeCallback );
        return true;
    }

    void Application::FramebufferResizeCallback( GLFWwindow * window,
                                                 int          /*width*/,
                                                 int          /*height*/ ) {
        auto application = static_cast<Application *>( glfwGetWindowUserPointer( window ) );
        application->FramebufferResized = true;
    }

    bool Application::CreateSurface() {
        WindowParameters window_parameters;
#if defined VK_USE_PLATFORM_WIN32_KHR
        window_parameters.HInstance = GetModuleHandle(nullptr);
        window_parameters.HWnd = glfwGetWin32Window(GlfwWindow);
#elif defined VK_USE_PLATFORM_XLIB_KHR
        window_parameters.Dpy = glfwGetX11Display();
        window_parameters.Window = glfwGetX11Window(GlfwWindow);
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
        window_parameters.Display = glfwGetWaylandDisplay();
        window_parameters.Surface = glfwGetWaylandWindow(GlfwWindow);
#endif

        InitVkDestroyer(Instance, PresentationSurface);
        return CreatePresentationSurface(*Instance, window_parameters, *PresentationSurface);
    }

    bool Application::PickPhysicalDevice() {
        // Use the first physical device that supports anisotropic filtering and has a queue family that supports both
        // graphics and presentation to the surface
        for( auto & device : PhysicalDevices ) {
            VkPhysicalDeviceFeatures supported_features;
            vkGetPhysicalDeviceFeatures( device, &supported_features );
            if( !supported_features.samplerAnisotropy ) {
                continue;
            }
            if( SelectQueueFamilyWithGraphicsAndPresentationToSurface(device, *PresentationSurface, QueueFamilyIndex) ) {
                PhysicalDevice = device;
                break;
            }
        }
        if( PhysicalDevice == VK_NULL_HANDLE ) {
            std::cout << "Could not find a physical device with anisotropic filtering and a queue family that supports graphics and presentation." << std::endl;
            return false;
        }
        std::cout << "Queue Family Index : " << QueueFamilyIndex << std::endl;
        return true;
    }

    bool Application::CreateDevice() {
        std::vector<char const*> desired_device_extensions;
        /*/
        = {
            "VK_KHR_8bit_storage",
            "VK_KHR_16bit_storage",
            "VK_KHR_acceleration_structure",
            "VK_KHR_bind_memory2",
            "VK_KHR_buffer_device_address",
            "VK_KHR_copy_commands2",
            "VK_KHR_create_renderpass2",
            "VK_KHR_dedicated_allocation",
            "VK_KHR_deferred_host_operations",
            "VK_KHR_depth_stencil_resolve",
            "VK_KHR_descriptor_update_template",
            "VK_KHR_device_group",
            "VK_KHR_draw_indirect_count",
            "VK_KHR_driver_properties",
            "VK_KHR_dynamic_rendering",
            "VK_KHR_external_fence",
            "VK_KHR_external_fence_fd",
            "VK_KHR_external_memory",
            "VK_KHR_external_memory_fd",
            "VK_KHR_external_semaphore",
            "VK_KHR_external_semaphore_fd",
            "VK_KHR_format_feature_flags2",
            "VK_KHR_fragment_shading_rate",
            "VK_KHR_get_memory_requirements2",
            "VK_KHR_image_format_list",
            "VK_KHR_imageless_framebuffer",
            "VK_KHR_incremental_present",
            "VK_KHR_maintenance1",
            "VK_KHR_maintenance2",
            "VK_KHR_maintenance3",
            "VK_KHR_maintenance4",
            "VK_KHR_multiview",
            "VK_KHR_performance_query",
            "VK_KHR_pipeline_executable_properties",
            "VK_KHR_pipeline_library",
            "VK_KHR_push_descriptor",
            "VK_KHR_ray_query",
            "VK_KHR_ray_tracing_maintenance1",
            "VK_KHR_relaxed_block_layout",
            "VK_KHR_sampler_mirror_clamp_to_edge",
            "VK_KHR_sampler_ycbcr_conversion",
            "VK_KHR_separate_depth_stencil_layouts",
            "VK_KHR_shader_atomic_int64",
            "VK_KHR_shader_clock",
            "VK_KHR_shader_draw_parameters",
            "VK_KHR_shader_float16_int8",
            "VK_KHR_shader_float_controls",
            "VK_KHR_shader_integer_dot_product",
            "VK_KHR_shader_non_semantic_info",
            "VK_KHR_shader_subgroup_extended_types",
            "VK_KHR_shader_subgroup_uniform_control_flow",
            "VK_KHR_shader_terminate_invocation",
            "VK_KHR_spirv_1_4",
            "VK_KHR_storage_buffer_storage_class",
            "VK_KHR_swapchain",
            "VK_KHR_swapchain_mutable_format",
            "VK_KHR_synchronization2",
            "VK_KHR_timeline_semaphore",
            "VK_KHR_uniform_buffer_standard_layout",
            "VK_KHR_variable_pointers",
            "VK_KHR_vulkan_memory_model",
            "VK_KHR_workgroup_memory_explicit_layout",
            "VK_KHR_zero_initialize_workgroup_memory",
            "VK_EXT_4444_formats",
            "VK_EXT_attachment_feedback_loop_layout",
            "VK_EXT_border_color_swizzle",
            "VK_EXT_buffer_device_address",
            "VK_EXT_calibrated_timestamps",
            "VK_EXT_color_write_enable",
            "VK_EXT_conditional_rendering",
            "VK_EXT_conservative_rasterization",
            "VK_EXT_custom_border_color",
            "VK_EXT_depth_clip_control",
            "VK_EXT_depth_clip_enable",
            "VK_EXT_depth_range_unrestricted",
            "VK_EXT_descriptor_indexing",
            "VK_EXT_discard_rectangles",
            "VK_EXT_display_control",
            "VK_EXT_extended_dynamic_state",
            "VK_EXT_extended_dynamic_state2",
            "VK_EXT_external_memory_dma_buf",
            "VK_EXT_external_memory_host",
            "VK_EXT_global_priority",
            "VK_EXT_global_priority_query",
            "VK_EXT_host_query_reset",
            "VK_EXT_image_2d_view_of_3d",
            "VK_EXT_image_drm_format_modifier",
            "VK_EXT_image_robustness",
            "VK_EXT_image_view_min_lod",
            "VK_EXT_index_type_uint8",
            "VK_EXT_inline_uniform_block",
            "VK_EXT_line_rasterization",
            "VK_EXT_memory_budget",
            "VK_EXT_memory_priority",
            "VK_EXT_multi_draw",
            "VK_EXT_non_seamless_cube_map",
            "VK_EXT_pci_bus_info",
            "VK_EXT_physical_device_drm",
            "VK_EXT_pipeline_creation_cache_control",
            "VK_EXT_pipeline_creation_feedback",
            "VK_EXT_post_depth_coverage",
            "VK_EXT_primitive_topology_list_restart",
            "VK_EXT_primitives_generated_query",
            "VK_EXT_private_data",
            "VK_EXT_provoking_vertex",
            "VK_EXT_queue_family_foreign",
            "VK_EXT_robustness2",
            "VK_EXT_sampler_filter_minmax",
            "VK_EXT_scalar_block_layout",
            "VK_EXT_separate_stencil_usage",
            "VK_EXT_shader_atomic_float",
            "VK_EXT_shader_atomic_float2",
            "VK_EXT_shader_demote_to_helper_invocation",
            "VK_EXT_shader_image_atomic_int64",
            "VK_EXT_shader_module_identifier",
            "VK_EXT_shader_stencil_export",
            "VK_EXT_shader_subgroup_ballot",
            "VK_EXT_shader_subgroup_vote",
            "VK_EXT_shader_viewport_index_layer",
            "VK_EXT_subgroup_size_control",
            "VK_EXT_texel_buffer_alignment",
            "VK_EXT_transform_feedback",
            "VK_EXT_vertex_attribute_divisor",
            "VK_EXT_vertex_input_dynamic_state",
            "VK_EXT_ycbcr_image_arrays",
            "VK_AMD_buffer_marker",
            "VK_AMD_device_coherent_memory",
            "VK_AMD_draw_indirect_count",
            "VK_AMD_gcn_shader",
            "VK_AMD_gpu_shader_half_float",
            "VK_AMD_gpu_shader_int16",
            "VK_AMD_memory_overallocation_behavior",
            "VK_AMD_mixed_attachment_samples",
            "VK_AMD_shader_ballot",
            "VK_AMD_shader_core_properties",
            "VK_AMD_shader_core_properties2",
            "VK_AMD_shader_explicit_vertex_parameter",
            "VK_AMD_shader_fragment_mask",
            "VK_AMD_shader_image_load_store_lod",
            "VK_AMD_shader_trinary_minmax",
            "VK_AMD_texture_gather_bias_lod",
            "VK_GOOGLE_decorate_string",
            "VK_GOOGLE_hlsl_functionality1",
            "VK_GOOGLE_user_type",
            "VK_INTEL_shader_integer_functions2",
            "VK_NV_compute_shader_derivatives",
            "VK_VALVE_mutable_descriptor_type"
        };
        /*/

        std::vector< QueueInfo > queue_infos;
        QueueInfo queue_info;
        queue_info.FamilyIndex = QueueFamilyIndex;
        queue_info.Priorities = {1.0f};
        queue_infos.push_back(queue_info);
        // Only the features that are actually used - enabling everything the device supports can cost performance
        VkPhysicalDeviceFeatures desired_features = {};
        desired_features.samplerAnisotropy = VK_TRUE;

        InitVkDestroyer(LogicalDevice);
        if( !CreateLogicalDeviceWithWsiExtensionsEnabled(PhysicalDevice, queue_infos, desired_device_extensions, &desired_features, *LogicalDevice) ) {
            return false;
        }
        if( !LoadDeviceLevelFunctions(*LogicalDevice, desired_device_extensions) ) {
            return false;
        }

        GetDeviceQueue(*LogicalDevice, QueueFamilyIndex, 0, GraphicsAndPresentQueue);
        return true;
    }

    bool Application::SelectSwapchainSettings() {
        if( !SelectDesiredPresentationMode(PhysicalDevice, *PresentationSurface, VK_PRESENT_MODE_MAILBOX_KHR, PresentMode) ) {
            return false;
        }
        std::cout << "Selected present mode : " << PresentMode << std::endl;

        if( !SelectFormatOfSwapchainImages( PhysicalDevice, *PresentationSurface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, SwapchainSurfaceFormat ) ) {
            return false;
        }
        std::cout << "Selected swapchain format : " << SwapchainSurfaceFormat.format << ", color space : " << SwapchainSurfaceFormat.colorSpace << std::endl;
        return true;
    }

    bool Application::SelectDepthFormat() {
        // Selected once: the render pass and the depth image have to use the same format.
        // Most desirable first - no stencil is needed yet.
        if( !SelectSupportedFormat( PhysicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                                    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, DepthFormat ) ) {
            return false;
        }
        std::cout << "Selected depth format : " << DepthFormat << std::endl;
        return true;
    }

    bool Application::CreateDepthResources() {
        // Also called when the swapchain is recreated: the old view, image and memory are destroyed here, in that order
        InitVkDestroyer( LogicalDevice, DepthImageView );
        InitVkDestroyer( LogicalDevice, DepthImage );
        InitVkDestroyer( LogicalDevice, DepthImageMemory );

        // Same size as the swapchain images
        if( !CreateImage( *LogicalDevice, SwapchainExtent.width, SwapchainExtent.height, DepthFormat, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, *DepthImage ) ||
            !AllocateAndBindMemoryObjectToImage( PhysicalDevice, *LogicalDevice, *DepthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *DepthImageMemory ) ||
            !CreateImageView( *LogicalDevice, *DepthImage, VK_IMAGE_VIEW_TYPE_2D, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, *DepthImageView ) ) {
            return false;
        }

        // Not strictly needed - the render pass transitions the image from UNDEFINED as well - but done explicitly
        // for completeness, like in the tutorial
        VkCommandBuffer command_buffer;
        if( !BeginSingleTimeCommands( *LogicalDevice, *TransferCommandPool, command_buffer ) ) {
            return false;
        }
        bool recorded = RecordImageLayoutTransition( command_buffer, *DepthImage, DepthFormat,
                                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
        bool submitted = EndSingleTimeCommands( *LogicalDevice, *TransferCommandPool, GraphicsAndPresentQueue, command_buffer );
        return recorded && submitted;
    }

    bool Application::CreateSwapchainObjects() {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        if( !GetCapabilitiesOfPresentationSurface( PhysicalDevice, *PresentationSurface, surface_capabilities ) ) {
            return false;
        }

        // When the swapchain is recreated, the old one is handed to the new one and destroyed at the end of this function
        VkDestroyer(VkSwapchainKHR) old_swapchain( std::move( Swapchain ) );
        bool first_swapchain = !old_swapchain;

        uint32_t number_of_images;
        SelectNumberOfSwapchainImages(surface_capabilities, number_of_images);
        if( first_swapchain ) {
            std::cout << "Selected number of images : " << number_of_images << std::endl;
        }

        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize( GlfwWindow, &framebuffer_width, &framebuffer_height );

        SelectSizeOfSwapchainImages( surface_capabilities, static_cast<uint32_t>(framebuffer_width), static_cast<uint32_t>(framebuffer_height), SwapchainExtent );
        std::cout << (first_swapchain ? "Selected swapchain size : " : "Swapchain recreated : ") << SwapchainExtent.width << "x" << SwapchainExtent.height << std::endl;

        InitVkDestroyer( LogicalDevice, Swapchain );
        if( !CreateSwapchain( *LogicalDevice, *PresentationSurface, number_of_images, SwapchainSurfaceFormat, SwapchainExtent,
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, surface_capabilities.currentTransform, PresentMode, *old_swapchain, *Swapchain ) ) {
            return false;
        }

        if( !GetHandlesOfSwapchainImages( *LogicalDevice, *Swapchain, SwapchainImages ) ) {
            return false;
        }
        if( first_swapchain ) {
            std::cout << "Swapchain images : " << SwapchainImages.size() << std::endl;
        }

        // One image view per swapchain image (views of an old swapchain are destroyed here)
        SwapchainImageViews.clear();
        for( auto & image : SwapchainImages ) {
            SwapchainImageViews.emplace_back();
            InitVkDestroyer( LogicalDevice, SwapchainImageViews.back() );
            if( !CreateImageView( *LogicalDevice, image, VK_IMAGE_VIEW_TYPE_2D, SwapchainSurfaceFormat.format,
                                  VK_IMAGE_ASPECT_COLOR_BIT, *SwapchainImageViews.back() ) ) {
                return false;
            }
        }
        if( first_swapchain ) {
            std::cout << "Swapchain image views : " << SwapchainImageViews.size() << std::endl;
        }
        return true;
    }

    bool Application::CreateTriangleRenderPass() {
        // Render pass with a color attachment (a swapchain image) that is cleared, rendered to and presented,
        // and a depth attachment that is cleared and only needed while rendering
        std::vector<VkAttachmentDescription> attachments_descriptions = {
            {
                0,                                                  // VkAttachmentDescriptionFlags     flags
                SwapchainSurfaceFormat.format,                      // VkFormat                         format
                VK_SAMPLE_COUNT_1_BIT,                              // VkSampleCountFlagBits            samples
                VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp               loadOp
                VK_ATTACHMENT_STORE_OP_STORE,                       // VkAttachmentStoreOp              storeOp
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,                    // VkAttachmentLoadOp               stencilLoadOp
                VK_ATTACHMENT_STORE_OP_DONT_CARE,                   // VkAttachmentStoreOp              stencilStoreOp
                VK_IMAGE_LAYOUT_UNDEFINED,                          // VkImageLayout                    initialLayout
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR                     // VkImageLayout                    finalLayout
            },
            {
                0,                                                  // VkAttachmentDescriptionFlags     flags
                DepthFormat,                                        // VkFormat                         format
                VK_SAMPLE_COUNT_1_BIT,                              // VkSampleCountFlagBits            samples
                VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp               loadOp
                VK_ATTACHMENT_STORE_OP_DONT_CARE,                   // VkAttachmentStoreOp              storeOp (not used after drawing)
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,                    // VkAttachmentLoadOp               stencilLoadOp
                VK_ATTACHMENT_STORE_OP_DONT_CARE,                   // VkAttachmentStoreOp              stencilStoreOp
                VK_IMAGE_LAYOUT_UNDEFINED,                          // VkImageLayout                    initialLayout
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL    // VkImageLayout                    finalLayout
            }
        };
        VkAttachmentReference depth_attachment_reference = {
            1,                                                      // uint32_t                         attachment
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL        // VkImageLayout                    layout
        };
        VkAttachmentReference color_attachment_reference = {
            0,                                                      // uint32_t                         attachment
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                // VkImageLayout                    layout
        };
        std::vector<VkSubpassDescription> subpass_descriptions = {
            {
                0,                                                  // VkSubpassDescriptionFlags        flags
                VK_PIPELINE_BIND_POINT_GRAPHICS,                    // VkPipelineBindPoint              pipelineBindPoint
                0,                                                  // uint32_t                         inputAttachmentCount
                nullptr,                                            // const VkAttachmentReference    * pInputAttachments
                1,                                                  // uint32_t                         colorAttachmentCount
                &color_attachment_reference,                        // const VkAttachmentReference    * pColorAttachments
                nullptr,                                            // const VkAttachmentReference    * pResolveAttachments
                &depth_attachment_reference,                        // const VkAttachmentReference    * pDepthStencilAttachment
                0,                                                  // uint32_t                         preserveAttachmentCount
                nullptr                                             // const uint32_t                 * pPreserveAttachments
            }
        };

        // Delay the layout transition at the start of the render pass until the swapchain image is acquired
        // (the image available semaphore is waited on in the color attachment output stage).
        // The depth image is shared by all frames: its clear (early fragment tests) has to wait for the depth writes
        // of the previous frame (late fragment tests).
        std::vector<VkSubpassDependency> subpass_dependencies = {
            {
                VK_SUBPASS_EXTERNAL,                                // uint32_t                         srcSubpass
                0,                                                  // uint32_t                         dstSubpass
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |     // VkPipelineStageFlags             srcStageMask
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |     // VkPipelineStageFlags             dstStageMask
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,       // VkAccessFlags                    srcAccessMask
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |              // VkAccessFlags                    dstAccessMask
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                0                                                   // VkDependencyFlags                dependencyFlags
            }
        };

        InitVkDestroyer( LogicalDevice, RenderPass );
        return CreateRenderPass( *LogicalDevice, attachments_descriptions, subpass_descriptions, subpass_dependencies, *RenderPass );
    }

    bool Application::CreateUniformDescriptorSetLayout() {
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

        InitVkDestroyer( LogicalDevice, DescriptorSetLayout );
        return CreateDescriptorSetLayout( *LogicalDevice, bindings, *DescriptorSetLayout );
    }

    bool Application::CreatePipeline() {
        // Descriptor set 0 is the uniform buffer, no push constants yet
        InitVkDestroyer( LogicalDevice, PipelineLayout );
        if( !CreatePipelineLayout( *LogicalDevice, { *DescriptorSetLayout }, {}, *PipelineLayout ) ) {
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
        InitVkDestroyer( LogicalDevice, vertex_shader_module );
        if( !CreateShaderModule( *LogicalDevice, vertex_shader_code, *vertex_shader_module ) ) {
            return false;
        }

        VkDestroyer(VkShaderModule) fragment_shader_module;
        InitVkDestroyer( LogicalDevice, fragment_shader_module );
        if( !CreateShaderModule( *LogicalDevice, fragment_shader_code, *fragment_shader_module ) ) {
            return false;
        }

        InitVkDestroyer( LogicalDevice, GraphicsPipeline );
        if( !CreateTrianglePipeline( *LogicalDevice, *RenderPass, *PipelineLayout,
                                     *vertex_shader_module, *fragment_shader_module,
                                     { Vertex::GetBindingDescription() }, Vertex::GetAttributeDescriptions(), *GraphicsPipeline ) ) {
            return false;
        }
        std::cout << "Graphics pipeline created" << std::endl;
        return true;
    }

    bool Application::CreateFramebuffers() {
        // One framebuffer per swapchain image view, all sharing the depth image view.
        // The order has to match the render pass attachments: color, depth.
        for( auto & image_view : SwapchainImageViews ) {
            SwapchainFramebuffers.emplace_back();
            InitVkDestroyer( LogicalDevice, SwapchainFramebuffers.back() );
            if( !CreateFramebuffer( *LogicalDevice, *RenderPass, { *image_view, *DepthImageView }, SwapchainExtent.width, SwapchainExtent.height, 1,
                                    *SwapchainFramebuffers.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool Application::CreateCommandBuffers() {
        // Command buffers are re-recorded every frame, so they have to be resettable individually
        InitVkDestroyer( LogicalDevice, CommandPool );
        if( !CreateCommandPool( *LogicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, QueueFamilyIndex, *CommandPool ) ) {
            return false;
        }

        if( !AllocateCommandBuffers( *LogicalDevice, *CommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_FRAMES_IN_FLIGHT, CommandBuffers ) ) {
            return false;
        }

        // Upload command buffers are short-lived, which the transient flag tells the driver
        InitVkDestroyer( LogicalDevice, TransferCommandPool );
        return CreateCommandPool( *LogicalDevice, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, QueueFamilyIndex, *TransferCommandPool );
    }

    bool Application::LoadModel() {
        auto start = std::chrono::steady_clock::now();
        if( !LoadModelFromObjFile( ModelPath, ModelVertices, ModelIndices ) ) {
            return false;
        }
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start ).count();
        std::cout << "Loaded model : " << ModelVertices.size() << " vertices, " << ModelIndices.size() << " indices in " << milliseconds << " ms" << std::endl;
        return true;
    }

    bool Application::CreateVertexAndIndexBuffers() {
        // Transfers run on the graphics queue - every queue family with graphics support also supports transfers
        InitVkDestroyer( LogicalDevice, VertexBufferMemory );
        InitVkDestroyer( LogicalDevice, VertexBuffer );
        InitVkDestroyer( LogicalDevice, IndexBufferMemory );
        InitVkDestroyer( LogicalDevice, IndexBuffer );
        return CreateDeviceLocalBufferWithData( PhysicalDevice, *LogicalDevice, *TransferCommandPool, GraphicsAndPresentQueue,
                                                ModelVertices.data(), sizeof( ModelVertices[0] ) * ModelVertices.size(),
                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, *VertexBuffer, *VertexBufferMemory ) &&
               CreateDeviceLocalBufferWithData( PhysicalDevice, *LogicalDevice, *TransferCommandPool, GraphicsAndPresentQueue,
                                                ModelIndices.data(), sizeof( ModelIndices[0] ) * ModelIndices.size(),
                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, *IndexBuffer, *IndexBufferMemory );
    }

    bool Application::CreateTexture() {
        InitVkDestroyer( LogicalDevice, TextureImageMemory );
        InitVkDestroyer( LogicalDevice, TextureImage );
        if( !CreateTextureImageFromFile( PhysicalDevice, *LogicalDevice, *TransferCommandPool, GraphicsAndPresentQueue,
                                         TexturePath.c_str(), *TextureImage, *TextureImageMemory ) ) {
            return false;
        }

        InitVkDestroyer( LogicalDevice, TextureImageView );
        if( !CreateImageView( *LogicalDevice, *TextureImage, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, *TextureImageView ) ) {
            return false;
        }

        // Linear filtering, repeating addressing, the highest anisotropy the device allows
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties( PhysicalDevice, &device_properties );

        InitVkDestroyer( LogicalDevice, TextureSampler );
        return CreateSampler( *LogicalDevice, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, true,
                              device_properties.limits.maxSamplerAnisotropy, *TextureSampler );
    }

    bool Application::CreateUniformBuffers() {
        // Updated every frame by the CPU, so no staging buffer - it would only add a copy
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            UniformBuffersMemory.emplace_back();
            InitVkDestroyer( LogicalDevice, UniformBuffersMemory.back() );
            UniformBuffers.emplace_back();
            InitVkDestroyer( LogicalDevice, UniformBuffers.back() );
            UniformBuffersMapped.push_back( nullptr );

            // Mapped once and kept mapped - freeing the memory at the end unmaps it implicitly
            if( !CreateHostVisibleBuffer( PhysicalDevice, *LogicalDevice, sizeof( UniformBufferObject ), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                          *UniformBuffers.back(), *UniformBuffersMemory.back() ) ||
                !MapMemory( *LogicalDevice, *UniformBuffersMemory.back(), 0, sizeof( UniformBufferObject ), UniformBuffersMapped.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool Application::CreateUniformDescriptorSets() {
        InitVkDestroyer( LogicalDevice, DescriptorPool );
        if( !CreateDescriptorPool( *LogicalDevice, false, MAX_FRAMES_IN_FLIGHT,
                                   { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
                                     { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT } }, *DescriptorPool ) ) {
            return false;
        }

        // One set per frame in flight, all with the same layout
        std::vector<VkDescriptorSetLayout> layouts( MAX_FRAMES_IN_FLIGHT, *DescriptorSetLayout );
        if( !AllocateDescriptorSets( *LogicalDevice, *DescriptorPool, layouts, DescriptorSets ) ) {
            return false;
        }

        std::vector<VkBuffer> uniform_buffers;
        for( auto & uniform_buffer : UniformBuffers ) {
            uniform_buffers.push_back( *uniform_buffer );
        }
        UpdateDescriptorSetsWithUniformBuffers( *LogicalDevice, DescriptorSets, 0, uniform_buffers, sizeof( UniformBufferObject ) );
        UpdateDescriptorSetsWithCombinedImageSampler( *LogicalDevice, DescriptorSets, 1, *TextureImageView, *TextureSampler,
                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
        return true;
    }

    void Application::UpdateUniformBuffer( uint32_t frame_index ) {
        float time = std::chrono::duration<float>( std::chrono::steady_clock::now() - StartTime ).count();

        UniformBufferObject ubo;
        // 90 degrees per second around the Z axis
        ubo.Model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        // Looking at the origin from above at a 45 degree angle
        ubo.View = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        // Aspect ratio from the current swapchain size, so it stays correct after a resize.
        // Depth range is 0..1 because of GLM_FORCE_DEPTH_ZERO_TO_ONE (set in CMakeLists.txt).
        ubo.Projection = glm::perspective( glm::radians( 45.0f ), SwapchainExtent.width / static_cast<float>( SwapchainExtent.height ), 0.1f, 10.0f );
        // GLM was made for OpenGL, where clip space Y points up - in Vulkan it points down
        ubo.Projection[1][1] *= -1;

        std::memcpy( UniformBuffersMapped[frame_index], &ubo, sizeof( ubo ) );
    }

    bool Application::CreateSyncObjects() {
        for( uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i ) {
            ImageAvailableSemaphores.emplace_back();
            InitVkDestroyer( LogicalDevice, ImageAvailableSemaphores.back() );
            InFlightFences.emplace_back();
            InitVkDestroyer( LogicalDevice, InFlightFences.back() );

            // Fences start signaled, so waiting for the "previous" frame doesn't block on the first frames
            if( !CreateVulkanSemaphore( *LogicalDevice, *ImageAvailableSemaphores.back() ) ||
                !CreateFence( *LogicalDevice, true, *InFlightFences.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool Application::CreateRenderFinishedSemaphores() {
        // One "render finished" semaphore per swapchain image (not per frame in flight like in the tutorial):
        // presentation may still wait on a semaphore after the frame's fence is signaled, and it is only safe to reuse
        // once the same swapchain image is acquired again (VUID-vkQueueSubmit-pSignalSemaphores-00067)
        for( size_t i = 0; i < SwapchainImages.size(); ++i ) {
            RenderFinishedSemaphores.emplace_back();
            InitVkDestroyer( LogicalDevice, RenderFinishedSemaphores.back() );
            if( !CreateVulkanSemaphore( *LogicalDevice, *RenderFinishedSemaphores.back() ) ) {
                return false;
            }
        }
        return true;
    }

    bool Application::RecreateSwapchain() {
        // A minimized window has a framebuffer size of 0 - pause until it is visible again
        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize( GlfwWindow, &framebuffer_width, &framebuffer_height );
        while( (framebuffer_width == 0) || (framebuffer_height == 0) ) {
            if( glfwWindowShouldClose( GlfwWindow ) ) {
                return true;
            }
            glfwWaitEvents();
            glfwGetFramebufferSize( GlfwWindow, &framebuffer_width, &framebuffer_height );
        }

        // Objects that are still in use by the GPU must not be destroyed
        vkDeviceWaitIdle( *LogicalDevice );

        // Everything that depends on the swapchain images or their size:
        // framebuffers -> depth image -> image views -> swapchain.
        // The render pass and pipeline are kept - they only depend on the image formats, which are selected once.
        SwapchainFramebuffers.clear();
        RenderFinishedSemaphores.clear();

        return CreateSwapchainObjects() &&
               CreateDepthResources() &&
               CreateFramebuffers() &&
               CreateRenderFinishedSemaphores();
    }

    bool Application::DrawFrame() {
        // Wait until the GPU has finished the frame that last used this frame's command buffer and semaphores
        if( !WaitForFences( *LogicalDevice, { *InFlightFences[CurrentFrame] }, VK_TRUE, std::numeric_limits<uint64_t>::max() ) ) {
            return false;
        }

        uint32_t image_index;
        SwapchainStatus swapchain_status;
        if( !AcquireSwapchainImage( *LogicalDevice, *Swapchain, *ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, image_index, swapchain_status ) ) {
            return false;
        }
        // Nothing was acquired - recreate the swapchain and try again in the next frame.
        // A suboptimal swapchain still delivered an image, so that frame is drawn and the swapchain is recreated after presenting.
        if( swapchain_status == SwapchainStatus::OutOfDate ) {
            return RecreateSwapchain();
        }

        // Reset only after acquiring - if acquiring fails and the frame is skipped, the fence must stay signaled
        if( !ResetFences( *LogicalDevice, { *InFlightFences[CurrentFrame] } ) ) {
            return false;
        }

        // Safe to overwrite - the fence wait above guarantees the GPU is done with this frame's uniform buffer
        UpdateUniformBuffer( CurrentFrame );

        if( !ResetCommandBuffer( CommandBuffers[CurrentFrame], false ) ||
            !RecordTriangleCommandBuffer( CommandBuffers[CurrentFrame], *RenderPass, *SwapchainFramebuffers[image_index],
                                          SwapchainExtent, *GraphicsPipeline, *VertexBuffer, *IndexBuffer,
                                          static_cast<uint32_t>( ModelIndices.size() ),
                                          *PipelineLayout, DescriptorSets[CurrentFrame] ) ) {
            return false;
        }

        if( !SubmitCommandBuffersToQueue( GraphicsAndPresentQueue,
                                          { { *ImageAvailableSemaphores[CurrentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT } },
                                          { CommandBuffers[CurrentFrame] },
                                          { *RenderFinishedSemaphores[image_index] },
                                          *InFlightFences[CurrentFrame] ) ) {
            return false;
        }

        if( !PresentImage( GraphicsAndPresentQueue, { *RenderFinishedSemaphores[image_index] }, *Swapchain, image_index, swapchain_status ) ) {
            return false;
        }

        // Checked after presenting, so the semaphores are in a consistent state
        if( (swapchain_status != SwapchainStatus::Optimal) || FramebufferResized ) {
            FramebufferResized = false;
            if( !RecreateSwapchain() ) {
                return false;
            }
        }

        CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return true;
    }

} // namespace VulkanCookbook
