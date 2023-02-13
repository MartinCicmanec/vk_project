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

#ifndef MY_TEST
#define MY_TEST

#include "Common.h"
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

struct QueueInfo {
    uint32_t FamilyIndex;
    std::vector<float> Priorities;
};

namespace VulkanCookbook {
    bool CheckAvailableInstanceLayers(std::vector<VkLayerProperties> &available_layers);
    bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> &available_extensions);
    bool CreateVulkanInstance( std::vector<char const *> const & desired_extensions,
                            char const * const                application_name,
                            VkInstance & instance);
    bool EnumerateAvailablePhysicalDevices( VkInstance instance, std::vector<VkPhysicalDevice> &available_devices);
    bool CheckAvailableDeviceExtensions( VkPhysicalDevice physical_device, std::vector<VkExtensionProperties> & available_extensions);
    void GetFeaturesAndPropertiesOfPhysicalDevice( VkPhysicalDevice physical_device,
                                                 VkPhysicalDeviceFeatures & device_features,
                                                 VkPhysicalDeviceProperties & device_properties);
    bool CheckAvailableQueueFamiliesAndTheirProperties( VkPhysicalDevice physical_device, std::vector<VkQueueFamilyProperties> & queue_families);
    bool SelectIndexOfQueueFamilyWithDesiredCapabilities( VkPhysicalDevice physical_device,
                                                        VkQueueFlags desired_capabilities,
                                                        uint32_t & queue_family_index );
    bool CreateLogicalDevice( VkPhysicalDevice physical_device,
                            std::vector<QueueInfo> queue_infos,
                            std::vector<char const *> const & desired_extensions,
                            VkPhysicalDeviceFeatures * desired_features,
                            VkDevice & logical_device );
    void GetDeviceQueue( VkDevice logical_device, uint32_t queue_family_index, uint32_t queue_index, VkQueue & queue );
    bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues( VkInstance   instance,
                                                                            VkDevice   & logical_device,
                                                                            VkQueue    & graphics_queue,
                                                                            VkQueue    & compute_queue );
    void DestroyLogicalDevice( VkDevice & logical_device );
    void DestroyVulkanInstance( VkInstance & instance );
    // Capter 2
    bool CreateVulkanInstanceWithWSIExtensionsEnabled( VkInstance & instance,
                                                        std::vector<char const *> & desired_extensions);
    bool CreatePresentationSurface( VkInstance         instance,
                                  WindowParameters   window_parameters,
                                  VkSurfaceKHR     & presentation_surface );
    bool SelectQueueFamilyWithPresentationToSurface( VkPhysicalDevice    physical_device,
                                                     VkSurfaceKHR        presentation_surface,
                                                     uint32_t          & queue_family_index );
    bool CreateLogicalDeviceWithWsiExtensionsEnabled( VkPhysicalDevice          physical_device,
                                                      std::vector< QueueInfo >    queue_infos,
                                                      std::vector<char const *> & desired_extensions,
                                                      VkPhysicalDeviceFeatures  * desired_features,
                                                      VkDevice                  & logical_device );
    bool SelectDesiredPresentationMode( VkPhysicalDevice   physical_device,
                                        VkSurfaceKHR       presentation_surface,
                                        VkPresentModeKHR   desired_present_mode,
                                        VkPresentModeKHR & present_mode );
    bool GetCapabilitiesOfPresentationSurface( VkPhysicalDevice           physical_device,
                                               VkSurfaceKHR               presentation_surface,
                                               VkSurfaceCapabilitiesKHR & surface_capabilities );
    bool SelectNumberOfSwapchainImages( VkSurfaceCapabilitiesKHR const & surface_capabilities,
                                        uint32_t                       & number_of_images );
} //namespace

#endif