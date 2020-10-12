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
#include "ListOfVulkanFunctions.inl"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>

std::vector<char const*> const validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

namespace VulkanCookbook {

    bool EnumerateAvailablePhysicalDevices( VkInstance instance, std::vector<VkPhysicalDevice> & available_devices ) {
        uint32_t devices_count = 0;
        VkResult result = VK_SUCCESS;

        result = vkEnumeratePhysicalDevices( instance, &devices_count, nullptr );
        if( (result != VK_SUCCESS) || (devices_count == 0) ) {
            std::cout << "Could not get the number of available physical devices." << std::endl;
            return false;
        }

        available_devices.resize( devices_count );
        result = vkEnumeratePhysicalDevices( instance, &devices_count, available_devices.data() );
        if( (result != VK_SUCCESS) || (devices_count == 0) ) {
            std::cout << "Could not enumerate physical devices." << std::endl;
            return false;
        }

        return true;
    }

    bool LoadFunctionExportedFromVulkanLoaderLibrary( LIBRARY_TYPE const & vulkan_library ) {
    #if defined _WIN32
    #define LoadFunction GetProcAddress
    #elif defined __linux
    #define LoadFunction dlsym
    #endif

    #define EXPORTED_VULKAN_FUNCTION( name )                              \
        name = (PFN_##name)LoadFunction( vulkan_library, #name );         \
        if( name == nullptr ) {                                           \
        std::cout << "Could not load exported Vulkan function named: "  \
            #name << std::endl;                                           \
        return false;                                                   \
        }

        return true;
    }

    bool LoadGlobalLevelFunctions() {
    #define GLOBAL_LEVEL_VULKAN_FUNCTION( name )                              \
        name = (PFN_##name)vkGetInstanceProcAddr( nullptr, #name );           \
        if( name == nullptr ) {                                               \
        std::cout << "Could not load global level Vulkan function named: "  \
            #name << std::endl;                                               \
        return false;                                                       \
        }

        return true;
    }

    bool CheckAvailableInstanceExtensions( std::vector<VkExtensionProperties> & available_extensions ) {
        uint32_t extensions_count = 0;
        VkResult result = VK_SUCCESS;

        std::cout << "Middle" << std::endl;
        result = vkEnumerateInstanceExtensionProperties( NULL, &extensions_count, nullptr );
        if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
            std::cout << "Could not get the number of instance extensions." << std::endl;
            return false;
        };
        std::cout << "Middle 2" << std::endl;
        available_extensions.resize( extensions_count );
        result = vkEnumerateInstanceExtensionProperties( NULL, &extensions_count, &available_extensions[0] );
        if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
            std::cout << "Could not enumerate instance extensions." << std::endl;
            return false;
        };

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

    bool checkValidationLayerSupport(std::vector<const char*> & desiredLayers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const auto& layerProperties : availableLayers) {
            std::cout << layerProperties.layerName << std::endl;
        }

        for (const char* layerName : desiredLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                std::cout << "Missing validation layer." << std::endl;
                return false;
            }
        }


        return true;
}

} // namespace VulkanCookbook

int main(){

    VulkanCookbook::LoadGlobalLevelFunctions();

    void* vulkan_library;
    vulkan_library = dlopen("libvulkan.so.1", RTLD_NOW);

    VulkanCookbook::LoadFunctionExportedFromVulkanLoaderLibrary( vulkan_library);

    if( vulkan_library == nullptr ) {
        std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
    }

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) dlsym(vulkan_library, "vkGetInstanceProcAddr");

    if( vkGetInstanceProcAddr == nullptr ) {
        std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
    }

    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;

    PFN_vkCreateInstance vkCreateInstance;
    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) vkGetInstanceProcAddr( nullptr, "vkEnumerateInstanceExtensionProperties" );
    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) vkGetInstanceProcAddr( nullptr, "vkEnumerateInstanceLayerProperties" );
    vkCreateInstance = (PFN_vkCreateInstance) vkGetInstanceProcAddr( nullptr, "vkCreateInstance" );

    if( vkEnumerateInstanceExtensionProperties == nullptr ) {
        std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
    }

    // Checking available Instance extensions
    
    uint32_t extensions_count = 0;
    VkResult result = VK_SUCCESS;

    result = vkEnumerateInstanceExtensionProperties( nullptr, &extensions_count, nullptr );
    if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
        std::cout << "Could not get the number of Instance extensions." << std::endl;
        return false;
    }

    std::vector<VkExtensionProperties> available_extensions;
    available_extensions.resize(extensions_count);

    result = vkEnumerateInstanceExtensionProperties( nullptr, &extensions_count, &available_extensions[0] );
    if ( (result != VK_SUCCESS) || (extensions_count == 0) ) {
        std::cout << "Could not enumerate Instance extensions." << std::endl;
        return false;
    }

    /*for( auto & available_extension : available_extensions ) {
        std::cout << available_extension.extensionName << std::endl;
    }*/


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
        0,                                                  // uint32_t                  enabledLayerCount
        nullptr,                                            // const char * const      * ppEnabledLayerNames
        static_cast<uint32_t>(desired_extensions.size()),   // uint32_t                  enabledExtensionCount
        desired_extensions.data()                           // const char * const      * ppEnabledExtensionNames
    };

    //VulkanCookbook::checkValidationLayerSupport(validationLayers);

    result = vkCreateInstance( &instance_create_info, nullptr, &instance );
    if( (result != VK_SUCCESS) || (instance == VK_NULL_HANDLE) ) {
        std::cout << "Could not create Vulkan instance." << std::endl;
        return false;
    }

    std::cin.ignore();

    //VulkanCookbook::vkDestroyInstance(instance, nullptr);
    return 0;
}