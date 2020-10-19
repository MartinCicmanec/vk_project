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
// Chapter: 01 Instance and Devices
// Recipe:  05 Loading function exported from a Vulkan Loader library

// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers


#include "main.h"
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


namespace VulkanCookbook {

    bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> &available_extensions) {
        uint32_t extensions_count = 0;
        VkResult result = VK_SUCCESS;

        std::cout << "Available instance extensions:" << std::endl;

        result = vkEnumerateInstanceExtensionProperties( NULL, &extensions_count, nullptr );
        if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
            std::cout << "Could not get the number of instance extensions." << std::endl;
            return false;
        };

        available_extensions.resize( extensions_count );
        result = vkEnumerateInstanceExtensionProperties( NULL, &extensions_count, available_extensions.data() );
        if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
            std::cout << "Could not enumerate instance extensions." << std::endl;
            return false;
        };

        for( auto & available_extension : available_extensions ) {
            std::cout << "\t" << available_extension.extensionName << std::endl;
        }

        return true;
    }

    bool CheckAvailableInstanceLayers(std::vector<VkLayerProperties> &available_layers) {
        uint32_t layer_count = 0;
        VkResult result = VK_SUCCESS;

        std::cout << "Available instance layers:" << std::endl;

        result = vkEnumerateInstanceLayerProperties( &layer_count, nullptr );
        if( (result != VK_SUCCESS) || (layer_count == 0) ) {
            std::cout << "Could not get the number of Instance layers." << std::endl;
            return false;
        }

        available_layers.resize(layer_count);

        result = vkEnumerateInstanceLayerProperties( &layer_count, available_layers.data() );
        if ( (result != VK_SUCCESS) || (layer_count == 0) ) {
            std::cout << "Could not enumerate Instance layers." << std::endl;
            return false;
        }

        for ( auto & available_layer : available_layers ) {
            std::cout << "\t" << available_layer.layerName << std::endl;
        }
        return true;
    }

    bool CreateVulkanInstance( std::vector<char const *> const & desired_extensions,
                                char const * const                application_name,
                                VkInstance                      & instance ) {
        
        std::vector<VkExtensionProperties> available_extensions = {};
        if( !CheckAvailableInstanceExtensions( available_extensions ) ) {
            return false;
        }

        for( auto & extension : desired_extensions ) {
            if( !IsExtensionSupported( available_extensions, extension ) ) {
                std::cout << "Extension named '" << extension << "' is not supported by an Instance object." << std::endl;
                return false;
            }
        }

        VkApplicationInfo application_info = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,                 // VkStructureType           sType
            nullptr,                                            // const void              * pNext
            application_name,                                   // const char              * pApplicationName
            VK_MAKE_VERSION( 1, 0, 0 ),                         // uint32_t                  applicationVersion
            "Vulkan Cookbook",                                  // const char              * pEngineName
            VK_MAKE_VERSION( 1, 0, 0 ),                         // uint32_t                  engineVersion
            VK_MAKE_VERSION( 1, 0, 0 )                          // uint32_t                  apiVersion
        };

        VkInstanceCreateInfo instance_create_info = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             // VkStructureType           sType
            nullptr,                                            // const void              * pNext
            0,                                                  // VkInstanceCreateFlags     flags
            &application_info,                                  // const VkApplicationInfo * pApplicationInfo
            0,                                                  // uint32_t                  enabledLayerCount
            nullptr,                                            // const char * const      * ppEnabledLayerNames
            static_cast<uint32_t>(desired_extensions.size()),   // uint32_t                  enabledExtensionCount
            desired_extensions.data()                           // const char * const      * ppEnabledExtensionNames
        };

        VkResult result = vkCreateInstance( &instance_create_info, nullptr, &instance );
        if( (result != VK_SUCCESS) || (instance == VK_NULL_HANDLE) ) {
            std::cout << "Could not create Vulkan instance." << std::endl;
            return false;
        }

        return true;
    }

    bool EnumerateAvailablePhysicalDevices( VkInstance instance, std::vector<VkPhysicalDevice> &physical_devices ) {
        uint32_t devices_count = 0;
        VkResult result = VK_SUCCESS;

        result = vkEnumeratePhysicalDevices( instance, &devices_count, nullptr );
        if( (result != VK_SUCCESS) || (devices_count == 0) ) {
            std::cout << "Could not get the number of available physical devices." << std::endl;
            return false;
        }

        physical_devices.resize( devices_count );
        result = vkEnumeratePhysicalDevices( instance, &devices_count, physical_devices.data() );
        if( (result != VK_SUCCESS) || (devices_count == 0) ) {
            std::cout << "Could not enumerate physical devices." << std::endl;
            return false;
        }
        
        /*std::cout << "Devices : " << std::endl;
        for( auto & available_device : physical_devices ) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(available_device, &deviceProperties);
            std::cout << "\t" << deviceProperties.deviceName << std::endl;
        }*/

        return true;
    }

    bool CheckAvailableDeviceExtensions( VkPhysicalDevice                     physical_device,
                                       std::vector<VkExtensionProperties> & available_extensions ) {
        uint32_t extensions_count = 0;
        VkResult result = VK_SUCCESS;

        result = vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extensions_count, nullptr );
        if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
            std::cout << "Could not get the number of device extensions." << std::endl;
            return false;
        }

        available_extensions.resize( extensions_count );
        result = vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extensions_count, available_extensions.data() );
        if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
            std::cout << "Could not enumerate device extensions." << std::endl;
            return false;
        }
        
        /*
        std::cout << "Available device extensions : " << std::endl;
        for (auto & available_extension : available_extensions) {
            std::cout << "\t" << available_extension.extensionName << std::endl;
        }
        */

        return true;
    }

    void GetFeaturesAndPropertiesOfPhysicalDevice( VkPhysicalDevice             physical_device,
                                                 VkPhysicalDeviceFeatures   & device_features,
                                                 VkPhysicalDeviceProperties & device_properties ) {
        vkGetPhysicalDeviceFeatures( physical_device, &device_features );
        vkGetPhysicalDeviceProperties( physical_device, &device_properties );
    }

    bool CheckAvailableQueueFamiliesAndTheirProperties( VkPhysicalDevice physical_device,
                                                        std::vector<VkQueueFamilyProperties> & queue_families ) {
        uint32_t queue_families_count = 0;

        vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_families_count, nullptr );
        if( queue_families_count == 0 ) {
            std::cout << "Could not get the number of queue families." << std::endl;
            return false;
        }

        queue_families.resize( queue_families_count );
        vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_families_count, queue_families.data() );
        if( queue_families_count == 0 ) {
            std::cout << "Could not acquire properties of queue families." << std::endl;
            return false;
        }

        return true;
    }

    bool SelectIndexOfQueueFamilyWithDesiredCapabilities( VkPhysicalDevice physical_device,
                                                        VkQueueFlags desired_capabilities,
                                                        uint32_t & queue_family_index ) {
        std::vector<VkQueueFamilyProperties> queue_families;
        if( !CheckAvailableQueueFamiliesAndTheirProperties( physical_device, queue_families ) ) {
            return false;
        }

        for( uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index ) {
            if( (queue_families[index].queueCount > 0) && ((queue_families[index].queueFlags & desired_capabilities) == desired_capabilities) ) {
                queue_family_index = index;
                return true;
            }
        }

        return false;
    }

    bool CreateLogicalDevice( VkPhysicalDevice                  physical_device,
                            std::vector<QueueInfo>            queue_infos,
                            std::vector<char const *> const & desired_extensions,
                            VkPhysicalDeviceFeatures        * desired_features,
                            VkDevice                        & logical_device ) {
        
        std::vector<VkExtensionProperties> available_extensions;
        if( !CheckAvailableDeviceExtensions( physical_device, available_extensions ) ) {
            return false;
        }

        for( auto & extension : desired_extensions ) {
            if( !IsExtensionSupported( available_extensions, extension ) ) {
                std::cout << "Extension named '" << extension << "' is not supported by a physical device." << std::endl;
                return false;
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        for( auto & info : queue_infos ) {
            queue_create_infos.push_back( {
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,       // VkStructureType                  sType
                nullptr,                                          // const void                     * pNext
                0,                                                // VkDeviceQueueCreateFlags         flags
                info.FamilyIndex,                                 // uint32_t                         queueFamilyIndex
                static_cast<uint32_t>(info.Priorities.size()),    // uint32_t                         queueCount
                info.Priorities.data()                            // const float                    * pQueuePriorities
            } );
        };

        VkDeviceCreateInfo device_create_info = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,               // VkStructureType                  sType
            nullptr,                                            // const void                     * pNext
            0,                                                  // VkDeviceCreateFlags              flags
            static_cast<uint32_t>(queue_create_infos.size()),   // uint32_t                         queueCreateInfoCount
            queue_create_infos.data(),                          // const VkDeviceQueueCreateInfo  * pQueueCreateInfos
            0,                                                  // uint32_t                         enabledLayerCount
            nullptr,                                            // const char * const             * ppEnabledLayerNames
            static_cast<uint32_t>(desired_extensions.size()),   // uint32_t                         enabledExtensionCount
            desired_extensions.data(),                          // const char * const             * ppEnabledExtensionNames
            desired_features                                    // const VkPhysicalDeviceFeatures * pEnabledFeatures
        };

        VkResult result = vkCreateDevice( physical_device, &device_create_info, nullptr, &logical_device );
        if( (result != VK_SUCCESS) || (logical_device == VK_NULL_HANDLE) ) {
            std::cout << "Could not create logical device." << std::endl;
            return false;
        }

        return true;
    }

} //VulkanCookbook


int main(int argc, char * argv[]) {

    bool enable_verbose = false;
    for ( int i = 0; i < argc; i = i + 1 ){
        if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0)) {
            enable_verbose = true;
            std::cout << "Enabled verbose" << std::endl;
        }
    }


    void* vulkan_library = dlopen("libvulkan.so", RTLD_NOW);
    bool lfefvll_result = VulkanCookbook::LoadFunctionExportedFromVulkanLoaderLibrary( vulkan_library);

    bool lglf = VulkanCookbook::LoadGlobalLevelFunctions();
    
    if( vulkan_library == nullptr ) {
        std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
    }
    if( VulkanCookbook::vkGetInstanceProcAddr == nullptr ) {
        std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
    }
    
    if((VulkanCookbook::vkEnumerateInstanceExtensionProperties == nullptr) || (VulkanCookbook::vkEnumerateInstanceLayerProperties == nullptr)) {
        std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
    }

    VkResult result = VK_SUCCESS;
    
    std::vector<VkExtensionProperties> available_extensions;
    VulkanCookbook::CheckAvailableInstanceExtensions(available_extensions);

    std::vector<VkLayerProperties> available_layers;
    VulkanCookbook::CheckAvailableInstanceLayers(available_layers);

    VkInstance instance = NULL;
    
    std::vector<char const*> const desired_extensions = {
        "VK_KHR_device_group_creation",
        "VK_KHR_get_display_properties2",
        "VK_KHR_get_physical_device_properties2",
        "VK_KHR_get_surface_capabilities2",
        "VK_KHR_surface",
        "VK_KHR_display",
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils"
        };


    for( auto & extension : desired_extensions ) {
        if( !VulkanCookbook::IsExtensionSupported( available_extensions, extension ) ) {
            std::cout << "Extension named '" << extension << "' is not supported by an Instance object." << std::endl;
            return false;
        }
    }

    std::vector<char const*> desired_layers = {
        "VK_LAYER_LUNARG_monitor"
    };

    if (enable_verbose == true) {
        desired_layers.push_back("VK_LAYER_KHRONOS_validation");
        desired_layers.push_back("VK_LAYER_LUNARG_api_dump");
    }


    for( auto & layer : desired_layers ) {
        if( !VulkanCookbook::IsLayerSupported( available_layers, layer ) ) {
            std::cout << "Layer named '" << layer << "' is not supported by an Instance object." << std::endl;
            return false;
        }
    }

    VkApplicationInfo application_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,                 // VkStructureType           sType
        nullptr,                                            // const void              * pNext
        "Test",                                             // const char              * pApplicationName
        VK_MAKE_VERSION( 1, 0, 0 ),                         // uint32_t                  applicationVersion
        "Vulkan Cookbook",                                  // const char              * pEngineName
        VK_MAKE_VERSION( 1, 0, 0 ),                         // uint32_t                  engineVersion
        VK_MAKE_VERSION( 1, 0, 0 )                          // uint32_t                  apiVersion
    };

    VkInstanceCreateInfo instance_create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             // VkStructureType           sType
        nullptr,                                            // const void              * pNext
        0,                                                  // VkInstanceCreateFlags     flags
        &application_info,                                  // const VkApplicationInfo * pApplicationInfo
        static_cast<uint32_t>(desired_layers.size()),       // uint32_t                  enabledLayerCount
        desired_layers.data(),                              // const char * const      * ppEnabledLayerNames
        static_cast<uint32_t>(desired_extensions.size()),   // uint32_t                  enabledExtensionCount
        desired_extensions.data()                           // const char * const      * ppEnabledExtensionNames
    };

    result = VulkanCookbook::vkCreateInstance( &instance_create_info, nullptr, &instance );
    if( (result != VK_SUCCESS) || (instance == VK_NULL_HANDLE) ) {
        std::cout << "Could not create Vulkan instance." << std::endl;
        return false;
    };

    bool lilf = VulkanCookbook::LoadInstanceLevelFunctions(instance, desired_extensions);

    
    if(VulkanCookbook::vkEnumeratePhysicalDevices == nullptr) {
        std::cout << "Could not load vkEnumeratePhysicalDevices." << std::endl;
    }

    if(VulkanCookbook::vkGetPhysicalDeviceProperties == nullptr) {
        std::cout << "Could not load vkGetPhysicalDeviceProperties." << std::endl;
    }

    if(VulkanCookbook::vkDestroyInstance == nullptr) {
        std::cout << "Could not load vkDestroyInstance." << std::endl;
    }


    bool debug_callbacks = false;
    if (debug_callbacks == true) {
        VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
        callbackCreateInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        callbackCreateInfo.pNext       = nullptr;
        callbackCreateInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                        VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
        callbackCreateInfo.pUserData   = nullptr;

        /* Register the callback */
        VkDebugReportCallbackEXT callback;
        result = VulkanCookbook::vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);
        if( (result != VK_SUCCESS) || (instance == VK_NULL_HANDLE) ) {
            std::cout << "Could not create debug callback." << std::endl;
            return false;
        };
    }
    
    
    std::vector<VkPhysicalDevice> physical_devices;
    VulkanCookbook::EnumerateAvailablePhysicalDevices(instance, physical_devices);
    /*uint32_t devices_count = 0;
    result = VulkanCookbook::vkEnumeratePhysicalDevices( instance, &devices_count, nullptr );

    std::cout << "device count : " << devices_count << std::endl;
    
    physical_devices.resize( devices_count );
    result = VulkanCookbook::vkEnumeratePhysicalDevices( instance, &devices_count, physical_devices.data() );
    

    */
    
    std::vector<VkExtensionProperties> available_device_extensions;
    VulkanCookbook::CheckAvailableDeviceExtensions(physical_devices[0], available_device_extensions);

    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceProperties deviceProperties;
    VulkanCookbook::GetFeaturesAndPropertiesOfPhysicalDevice(physical_devices[0], deviceFeatures, deviceProperties);

    std::vector<VkQueueFamilyProperties> queue_families;
    VulkanCookbook::CheckAvailableQueueFamiliesAndTheirProperties(physical_devices[0], queue_families);
    
    std::cout << "Available queue families : " << queue_families.size() << std::endl;

    uint32_t queue_family_index;
    VkQueueFlags desired_capabilities = VK_QUEUE_GRAPHICS_BIT || VK_QUEUE_COMPUTE_BIT || VK_QUEUE_TRANSFER_BIT;
    
    VulkanCookbook::SelectIndexOfQueueFamilyWithDesiredCapabilities(physical_devices[0], desired_capabilities, queue_family_index);
    //std::vector<VkQueueFamilyProperties> queue_families

    std::vector<char const*> const desired_device_extensions = {
        "VK_KHR_8bit_storage",
        "VK_KHR_16bit_storage",
        "VK_KHR_bind_memory2",
        "VK_KHR_buffer_device_address",
        "VK_KHR_create_renderpass2",
        "VK_KHR_dedicated_allocation",
        "VK_KHR_depth_stencil_resolve",
        "VK_KHR_descriptor_update_template",
        "VK_KHR_device_group",
        "VK_KHR_draw_indirect_count",
        "VK_KHR_driver_properties",
        "VK_KHR_external_fence",
        "VK_KHR_external_fence_fd",
        "VK_KHR_external_memory",
        "VK_KHR_external_memory_fd",
        "VK_KHR_external_semaphore",
        "VK_KHR_external_semaphore_fd",
        "VK_KHR_get_memory_requirements2",
        "VK_KHR_image_format_list",
        "VK_KHR_imageless_framebuffer",
        "VK_KHR_incremental_present",
        "VK_KHR_maintenance1",
        "VK_KHR_maintenance2",
        "VK_KHR_maintenance3",
        "VK_KHR_multiview",
        "VK_KHR_pipeline_executable_properties",
        "VK_KHR_push_descriptor",
        "VK_KHR_relaxed_block_layout",
        "VK_KHR_sampler_mirror_clamp_to_edge",
        "VK_KHR_sampler_ycbcr_conversion",
        "VK_KHR_separate_depth_stencil_layouts",
        "VK_KHR_shader_atomic_int64",
        "VK_KHR_shader_clock",
        "VK_KHR_shader_draw_parameters",
        "VK_KHR_shader_float16_int8",
        "VK_KHR_shader_float_controls",
        "VK_KHR_shader_subgroup_extended_types",
        "VK_KHR_spirv_1_4",
        "VK_KHR_storage_buffer_storage_class",
        "VK_KHR_swapchain",
        "VK_KHR_swapchain_mutable_format",
        "VK_KHR_timeline_semaphore",
        "VK_KHR_uniform_buffer_standard_layout",
        "VK_KHR_variable_pointers",
        "VK_KHR_vulkan_memory_model",
        "VK_EXT_buffer_device_address",
        "VK_EXT_calibrated_timestamps",
        "VK_EXT_conditional_rendering",
        "VK_EXT_depth_clip_enable",
        "VK_EXT_descriptor_indexing",
        "VK_EXT_display_control",
        "VK_EXT_external_memory_dma_buf",
        "VK_EXT_external_memory_host",
        "VK_EXT_fragment_shader_interlock",
        "VK_EXT_global_priority",
        "VK_EXT_host_query_reset",
        "VK_EXT_index_type_uint8",
        "VK_EXT_inline_uniform_block",
        "VK_EXT_line_rasterization",
        "VK_EXT_memory_budget",
        "VK_EXT_pci_bus_info",
        "VK_EXT_pipeline_creation_feedback",
        "VK_EXT_post_depth_coverage",
        "VK_EXT_sampler_filter_minmax",
        "VK_EXT_scalar_block_layout",
        "VK_EXT_separate_stencil_usage",
        "VK_EXT_shader_demote_to_helper_invocation",
        "VK_EXT_shader_stencil_export",
        "VK_EXT_shader_subgroup_ballot",
        "VK_EXT_shader_subgroup_vote",
        "VK_EXT_shader_viewport_index_layer",
        "VK_EXT_subgroup_size_control",
        "VK_EXT_texel_buffer_alignment",
        "VK_EXT_transform_feedback",
        "VK_EXT_vertex_attribute_divisor",
        "VK_EXT_ycbcr_image_arrays",
        "VK_GOOGLE_decorate_string",
        "VK_GOOGLE_hlsl_functionality1",
        "VK_GOOGLE_user_type",
        "VK_INTEL_shader_integer_functions2",
        "VK_NV_compute_shader_derivatives"
    };

    std::vector<QueueInfo> queue_infos;
    QueueInfo qInfo = {
        0,
        {1.0f}
    };

    queue_infos.push_back(qInfo);

    VkDevice logical_device;
    result = VK_SUCCESS;
    VulkanCookbook::CreateLogicalDevice(physical_devices[0], queue_infos, desired_device_extensions, &deviceFeatures, logical_device);

    if ((result == VK_SUCCESS) || ((logical_device == VK_NULL_HANDLE))) {
        std::cout << "Created logical device." << std::endl;
    } else {
        std::cout << "Failed to create logical device." << std::endl;
    }
    VulkanCookbook::LoadDeviceLevelFunctions(logical_device, desired_device_extensions);



    std::cin.ignore();
    VulkanCookbook::vkDestroyInstance(instance, nullptr);
    return 0;
}