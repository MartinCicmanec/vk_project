#ifndef LOADING_FUNCTION_EXPORTED_FROM_A_VULKAN_LOADER_LIBRARY
#define LOADING_FUNCTION_EXPORTED_FROM_A_VULKAN_LOADER_LIBRARY

#include "common.h"

namespace VulkanCookbook {

  bool LoadFunctionExportedFromVulkanLoaderLibrary( LIBRARY_TYPE const & vulkan_library );
  VkResult CreateVulkanInstance( std::vector<char const *> const & desired_extensions, char const * const application_name, VkInstance & instance );
} // namespace VulkanCookbook

#endif // LOADING_FUNCTION_EXPORTED_FROM_A_VULKAN_LOADER_LIBRARY