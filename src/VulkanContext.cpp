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
// Vulkan Context

#include "VulkanContext.h"
#include "AppWindow.h"
#include "Debug.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "Presentation.h"
#include "CommandBuffer.h"

namespace VulkanCookbook {

    bool VulkanContext::Init( AppWindow const       & window,
                              bool                    enable_validation,
                              bool                    enable_debug_callbacks,
                              VkSampleCountFlagBits   max_msaa_samples ) {
        if( !InitVulkanLoader() ||
            !CreateInstance( enable_validation ) ||
            !SetupDebugMessenger( enable_debug_callbacks ) ) {
            return false;
        }

        // The surface is needed to select a device that can present to it
        InitVkDestroyer( Instance, PresentationSurface );
        if( !window.CreateSurface( *Instance, *PresentationSurface ) ) {
            return false;
        }

        return PickPhysicalDevice( max_msaa_samples ) &&
               CreateDevice();
    }

    void VulkanContext::WaitIdle() const {
        if( LogicalDevice ) {
            vkDeviceWaitIdle( *LogicalDevice );
        }
    }

    bool VulkanContext::InitVulkanLoader() {
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

    bool VulkanContext::CreateInstance( bool enable_validation ) {
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

        if (enable_validation == true) {
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

    bool VulkanContext::SetupDebugMessenger( bool enable_debug_callbacks ) {
        InitVkDestroyer(Instance, DebugMessenger);
        if (enable_debug_callbacks == true) {
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

    bool VulkanContext::PickPhysicalDevice( VkSampleCountFlagBits max_msaa_samples ) {
        std::vector<VkPhysicalDevice> physical_devices;
        if( !EnumerateAvailablePhysicalDevices( *Instance, physical_devices ) ) {
            return false;
        }

        // Use the first physical device that supports Vulkan 1.3 with dynamic rendering and synchronization2,
        // anisotropic filtering, and has a queue family that supports both graphics and presentation to the surface
        for( auto & device : physical_devices ) {
            // The instance version (Instance.cpp) and the device version are separate - an old driver can report less
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties( device, &device_properties );
            if( device_properties.apiVersion < VK_API_VERSION_1_3 ) {
                std::cout << "Skipping '" << device_properties.deviceName << "' : Vulkan 1.3 is not supported (device supports "
                          << VK_API_VERSION_MAJOR( device_properties.apiVersion ) << "." << VK_API_VERSION_MINOR( device_properties.apiVersion ) << ")." << std::endl;
                continue;
            }

            // Features of newer Vulkan versions are queried through structures chained to VkPhysicalDeviceFeatures2
            VkPhysicalDeviceVulkan13Features supported_vulkan13_features = {};
            supported_vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            VkPhysicalDeviceFeatures2 supported_features = {};
            supported_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            supported_features.pNext = &supported_vulkan13_features;
            vkGetPhysicalDeviceFeatures2( device, &supported_features );

            if( !supported_features.features.samplerAnisotropy ||
                !supported_vulkan13_features.dynamicRendering ||
                !supported_vulkan13_features.synchronization2 ) {
                std::cout << "Skipping '" << device_properties.deviceName << "' : a required feature is not supported." << std::endl;
                continue;
            }
            if( SelectQueueFamilyWithGraphicsAndPresentationToSurface(device, *PresentationSurface, QueueFamilyIndex) ) {
                PhysicalDevice = device;
                MsaaSamples = GetMaxUsableSampleCount( device, max_msaa_samples );
                MaxSamplerAnisotropy = device_properties.limits.maxSamplerAnisotropy;
                break;
            }
        }
        if( PhysicalDevice == VK_NULL_HANDLE ) {
            std::cout << "Could not find a physical device with Vulkan 1.3 (dynamic rendering, synchronization2), anisotropic filtering and a queue family that supports graphics and presentation." << std::endl;
            return false;
        }
        std::cout << "Queue Family Index : " << QueueFamilyIndex << std::endl;
        std::cout << "MSAA samples : " << MsaaSamples << std::endl;
        return true;
    }

    bool VulkanContext::CreateDevice() {
        // CreateLogicalDeviceWithWsiExtensionsEnabled adds the swapchain extension to this list
        std::vector<char const*> desired_device_extensions;

        std::vector< QueueInfo > queue_infos;
        QueueInfo queue_info;
        queue_info.FamilyIndex = QueueFamilyIndex;
        queue_info.Priorities = {1.0f};
        queue_infos.push_back(queue_info);
        // Only the features that are actually used - enabling everything the device supports can cost performance
        VkPhysicalDeviceVulkan13Features desired_vulkan13_features = {};
        desired_vulkan13_features.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        desired_vulkan13_features.dynamicRendering = VK_TRUE;
        desired_vulkan13_features.synchronization2 = VK_TRUE;

        VkPhysicalDeviceFeatures2 desired_features = {};
        desired_features.sType                      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        desired_features.pNext                      = &desired_vulkan13_features;
        desired_features.features.samplerAnisotropy = VK_TRUE;

        InitVkDestroyer(LogicalDevice);
        if( !CreateLogicalDeviceWithWsiExtensionsEnabled(PhysicalDevice, queue_infos, desired_device_extensions, &desired_features, *LogicalDevice) ) {
            return false;
        }
        if( !LoadDeviceLevelFunctions(*LogicalDevice, desired_device_extensions) ) {
            return false;
        }

        GetDeviceQueue(*LogicalDevice, QueueFamilyIndex, 0, GraphicsAndPresentQueue);

        // All buffers and images are allocated through it
        if( !CreateAllocator( *Instance, PhysicalDevice, *LogicalDevice, Allocator ) ) {
            return false;
        }
        DestroyVmaAllocator.Function = [this]() { DestroyAllocator( Allocator ); };

        // Upload command buffers are short-lived, which the transient flag tells the driver.
        // Transfers run on the graphics queue - every queue family with graphics support also supports transfers.
        InitVkDestroyer( LogicalDevice, TransferCommandPool );
        return CreateCommandPool( *LogicalDevice, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, QueueFamilyIndex, *TransferCommandPool );
    }

} // namespace VulkanCookbook
