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

#ifndef LOADING_FUNCTION_EXPORTED_FROM_A_VULKAN_LOADER_LIBRARY
#define LOADING_FUNCTION_EXPORTED_FROM_A_VULKAN_LOADER_LIBRARY

#include "Common.h"
#include <vector>
#include "ListOfVulkanFunctions.inl"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

bool lf_result = VulkanCookbook::LoadGlobalLevelFunctions();

void* vulkan_library = dlopen("libvulkan.so.1", RTLD_NOW);

bool lfefvll_result = VulkanCookbook::LoadFunctionExportedFromVulkanLoaderLibrary( vulkan_library);

PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) dlsym(vulkan_library, "vkGetInstanceProcAddr");
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) vkGetInstanceProcAddr( nullptr, "vkEnumerateInstanceExtensionProperties" );
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) vkGetInstanceProcAddr( nullptr, "vkEnumerateInstanceLayerProperties" );
PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance) vkGetInstanceProcAddr( nullptr, "vkCreateInstance" );
PFN_vkDestroyInstance vkDestroyInstance = (PFN_vkDestroyInstance) vkGetInstanceProcAddr(nullptr, "vkDestroyInstance");

namespace VulkanCookbook {

    bool EnumerateAvailablePhysicalDevices( VkInstance instance, std::vector<VkPhysicalDevice> & available_devices );
    bool CreateVulkanInstance( std::vector<char const *> const & desired_extensions,
                            char const * const                application_name,
                            VkInstance & instance );

} // namespace VulkanCookbook

bool CheckAvailableInstanceLayers(std::vector<VkLayerProperties> &available_layers);
bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> &available_extensions);


#endif // LOADING_FUNCTION_EXPORTED_FROM_A_VULKAN_LOADER_LIBRARY