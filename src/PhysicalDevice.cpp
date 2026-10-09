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
//
// Physical Device

#include "PhysicalDevice.h"

namespace VulkanCookbook {

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

        /*/
        std::cout << "Devices : " << std::endl;
        for( auto & available_device : physical_devices ) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(available_device, &deviceProperties);
            std::cout << "\t" << deviceProperties.deviceName << std::endl;
        }
        /*/

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

        /*/
        std::cout << "Available device extensions : " << std::endl;
        for (auto & available_extension : available_extensions) {
            std::cout << "\t" << available_extension.extensionName << std::endl;
        }
        /*/

        return true;
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

    bool SelectSupportedFormat( VkPhysicalDevice              physical_device,
                                std::vector<VkFormat> const & candidates,
                                VkImageTiling                 tiling,
                                VkFormatFeatureFlags          features,
                                VkFormat                    & format ) {
        for( auto & candidate : candidates ) {
            VkFormatProperties format_properties;
            vkGetPhysicalDeviceFormatProperties( physical_device, candidate, &format_properties );

            VkFormatFeatureFlags supported_features = (tiling == VK_IMAGE_TILING_LINEAR) ? format_properties.linearTilingFeatures
                                                                                          : format_properties.optimalTilingFeatures;
            if( (supported_features & features) == features ) {
                format = candidate;
                return true;
            }
        }

        std::cout << "Could not find a supported format." << std::endl;
        return false;
    }

    VkSampleCountFlagBits GetMaxUsableSampleCount( VkPhysicalDevice        physical_device,
                                                   VkSampleCountFlagBits   upper_limit ) {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties( physical_device, &device_properties );

        VkSampleCountFlags counts = device_properties.limits.framebufferColorSampleCounts &
                                    device_properties.limits.framebufferDepthSampleCounts;

        // Sample count flags are powers of two (1, 2, 4, ... 64), so they can be compared and halved
        for( uint32_t samples = VK_SAMPLE_COUNT_64_BIT; samples > VK_SAMPLE_COUNT_1_BIT; samples /= 2 ) {
            if( (samples <= static_cast<uint32_t>(upper_limit)) && (counts & samples) ) {
                return static_cast<VkSampleCountFlagBits>(samples);
            }
        }
        return VK_SAMPLE_COUNT_1_BIT;
    }

} // namespace VulkanCookbook
