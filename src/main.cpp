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

    bool EnumerateAvailablePhysicalDevices( VkInstance instance, std::vector<VkPhysicalDevice> &physical_devices ) {
        uint32_t devices_count = 0;
        VkResult result = VK_SUCCESS;

        result = vkEnumeratePhysicalDevices( NULL, &devices_count, nullptr );
        if( (result != VK_SUCCESS) || (devices_count == 0) ) {
            std::cout << "Could not get the number of available physical devices." << std::endl;
            return false;
        }

        physical_devices.resize( devices_count );
        result = vkEnumeratePhysicalDevices( NULL, &devices_count, physical_devices.data() );
        if( (result != VK_SUCCESS) || (devices_count == 0) ) {
            std::cout << "Could not enumerate physical devices." << std::endl;
            return false;
        }
        
        std::cout << "Devices : " << std::endl;
        for( auto & available_device : physical_devices ) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(available_device, &deviceProperties);
            std::cout << "\t" << deviceProperties.deviceName << std::endl;
            delete &deviceProperties;
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
        result = vkEnumerateInstanceExtensionProperties( NULL, &extensions_count, &available_extensions[0] );
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

        result = vkEnumerateInstanceLayerProperties( &layer_count, &available_layers[0] );
        if ( (result != VK_SUCCESS) || (layer_count == 0) ) {
            std::cout << "Could not enumerate Instance layers." << std::endl;
            return false;
        }

        for ( auto & available_layer : available_layers ) {
            std::cout << "\t" << available_layer.layerName << std::endl;
        }
        return true;
    }

    bool CheckAllDeviceQueueFamilyProperties(VkPhysicalDevice device, std::vector<VkQueueFamilyProperties> &queue_families) {
        uint32_t queue_families_count;
        VkResult result = VK_SUCCESS;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count, nullptr);
        queue_families.resize(queue_families_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_families_count, queue_families.data());
        
        if( queue_families_count == 0 ) {  std::cout << "Could not acquire properties of queue families." <<std::endl;  return false;}

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

    if(VulkanCookbook::vkEnumeratePhysicalDevices == nullptr) {
        std::cout << "Could not load vkEnumeratePhysicalDevices." << std::endl;
    }

    if(VulkanCookbook::vkGetPhysicalDeviceProperties == nullptr) {
        std::cout << "Could not load vkGetPhysicalDeviceProperties." << std::endl;
    }

    if(VulkanCookbook::vkDestroyInstance == nullptr) {
        std::cout << "Could not load vkDestroyInstance." << std::endl;
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
    //EnumerateAvailablePhysicalDevices(instance, physical_devices);
    uint32_t devices_count = 0;
    result = VulkanCookbook::vkEnumeratePhysicalDevices( instance, &devices_count, nullptr );

    std::cout << "device count : " << devices_count << std::endl;
    
    physical_devices.resize( devices_count );
    result = VulkanCookbook::vkEnumeratePhysicalDevices( instance, &devices_count, physical_devices.data() );
    
    VkQueueFlags desired_capabilities = VK_QUEUE_GRAPHICS_BIT || VK_QUEUE_COMPUTE_BIT || VK_QUEUE_TRANSFER_BIT;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    VkPhysicalDevice available_device = physical_devices[0];
    VulkanCookbook::vkGetPhysicalDeviceProperties(available_device, &deviceProperties);
    std::cout << "\t" << deviceProperties.deviceName << std::endl;
    VulkanCookbook::vkGetPhysicalDeviceFeatures( physical_devices[0], &deviceFeatures);

    std::vector<VkQueueFamilyProperties> queue_families;
    VulkanCookbook::CheckAllDeviceQueueFamilyProperties(physical_devices[0], queue_families);
    std::cout << "Available queue families : " << queue_families.size() << std::endl;

    uint32_t queue_family_index;

    for( uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index ) {
        std::cout << "\t" << queue_families[index].queueCount << std::endl;
        if( (queue_families[index].queueCount > 0) && (queue_families[index].queueFlags & desired_capabilities) ) {
            queue_family_index = index;
        }
    }
    //std::vector<VkQueueFamilyProperties> queue_families

    std::vector<QueueInfo> queue_infos;
    QueueInfo qInfo = {
        0,
        {1.0f}
    };
    //qInfo.FamilyIndex = 0;
    //qInfo.Priorities = {1.0f};
    queue_infos.push_back(qInfo);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for( auto & info : queue_infos ) {
        queue_create_infos.push_back( {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            nullptr,
            0,
            info.FamilyIndex,
            static_cast<uint32_t>(info.Priorities.size()),
            info.Priorities.data()
        } );
    };

    VkDeviceCreateInfo device_create_info;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()); //
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.enabledLayerCount = static_cast<uint32_t>(desired_layers.size());
    device_create_info.ppEnabledLayerNames = desired_layers.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(desired_extensions.size());
    device_create_info.ppEnabledExtensionNames = desired_extensions.data();
    device_create_info.pEnabledFeatures = &deviceFeatures;
    

    VkDevice logical_device;
    result = VK_SUCCESS;
    result = VulkanCookbook::vkCreateDevice( physical_devices[0], &device_create_info, nullptr, &logical_device );

    if ((result == VK_SUCCESS) || ((logical_device == VK_NULL_HANDLE))) {
        std::cout << "Created logical device." << std::endl;
    } else {
        std::cout << "Failed to create logical device." << std::endl;
    }

    std::cin.ignore();
    VulkanCookbook::vkDestroyInstance(instance, nullptr);
    return 0;
}