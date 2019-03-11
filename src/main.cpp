#include <stdio.h>
#include <iostream>
#include <vector>
#include "VulkanFunctions.h"
#include "main.h"
#include "common.h"

namespace VulkanCookbook {

	bool ConnectWithVulkanLoaderLibrary( LIBRARY_TYPE & vulkan_library );
	bool LoadFunctionExportedFromVulkanLoaderLibrary( LIBRARY_TYPE const & vulkan_library );
	bool LoadGlobalLevelFunctions();
	bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> & available_extensions);
	VkResult CreateVulkanInstance( std::vector<char const *> const & desired_extensions, char const * const application_name, VkInstance & instance );
	bool LoadInstanceLevelFunctions( VkInstance instance, std::vector<char const *> const & enabled_extensions );
	bool CheckAvailableDeviceExtensions( VkPhysicalDevice physical_device, std::vector<VkExtensionProperties> & available_extensions );
	VkResult EnumerateAvailablePhysicalDevices( VkInstance instance, std::vector<VkPhysicalDevice> & available_devices );
	void GetFeaturesAndPropertiesOfPhysicalDevice( VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures & device_features, VkPhysicalDeviceProperties & device_properties );
	bool CheckAvailableQueueFamiliesAndTheirProperties( VkPhysicalDevice physical_device, std::vector<VkQueueFamilyProperties> & queue_families );
	bool SelectIndexOfQueueFamilyWithDesiredCapabilities( VkPhysicalDevice physical_device, VkQueueFlags desired_capabilities, uint32_t & queue_family_index );
	bool CreateLogicalDevice( VkPhysicalDevice physical_device, std::vector<QueueInfo> queue_infos, std::vector<char const *> const & desired_extensions,
		VkPhysicalDeviceFeatures * desired_features, VkDevice & logical_device);
	bool LoadDeviceLevelFunctions( VkDevice logical_device, std::vector<char const *> const & enabled_extensions );
	//extern bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> & available_extensions);
	extern bool IsExtensionSupported( std::vector<VkExtensionProperties> const & available_extensions, char const * const extension );
	void GetDeviceQueue( VkDevice logical_device, uint32_t queue_family_index, uint32_t queue_index, VkQueue & queue );
	bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues( VkInstance instance, VkDevice & logical_device, VkQueue & graphics_queue, VkQueue & compute_queue );
	void DestroyLogicalDevice( VkDevice & logical_device );
	void DestroyVulkanInstance( VkInstance & instance );
	void ReleaseVulkanLoaderLibrary( LIBRARY_TYPE & vulkan_library );
	// Chapter 2
	bool CreateVulkanInstanceWithWsiExtensionsEnabled( std::vector<char const *> & desired_extensions, char const * const application_name, VkInstance & instance );

	bool ConnectWithVulkanLoaderLibrary( LIBRARY_TYPE & vulkan_library ) {
		#if defined _WIN32
			vulkan_library = LoadLibrary( "vulkan-1.dll" );
		#elif defined __linux
			vulkan_library = dlopen( "libvulkan.so.1", RTLD_NOW );
		#endif

		if( vulkan_library == nullptr ) {
			std::cout << "Could not connect with a Vulkan Runtime library." << std::endl;
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

		#include "ListOfVulkanFunctions.inl"

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

		#include "ListOfVulkanFunctions.inl"

		return true;
	}

	bool CheckAvailableInstanceExtensions(std::vector<VkExtensionProperties> & available_extensions) {
		uint32_t extensions_count = 0;
		VkResult result = VK_SUCCESS;

		result = VulkanCookbook::vkEnumerateInstanceExtensionProperties( nullptr, &extensions_count, nullptr );
		if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
			std::cout << "Could not get the number of instance extensions." << std::endl;
			return false;
		}

		available_extensions.resize( extensions_count );
		result = VulkanCookbook::vkEnumerateInstanceExtensionProperties( nullptr, &extensions_count, available_extensions.data() );
		if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
			std::cout << "Could not enumerate instance extensions." << std::endl;
			return false;
		}

		return true;
	}

	VkResult CreateVulkanInstance( std::vector<char const *> const & desired_extensions, char const * const application_name, VkInstance & instance ) {
		
		VkResult result = VK_SUCCESS;
		std::vector<VkExtensionProperties> available_extensions = {};
		if(!CheckAvailableInstanceExtensions(available_extensions)) {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		for( auto & extension : available_extensions ) {
			std::cout << "Extension named '" << extension.extensionName << "' is not supported." << std::endl;
		}

		for( auto & extension : desired_extensions ) {
			if( !IsExtensionSupported( available_extensions, extension ) ) {
				std::cout << "Extension named '" << extension << "' is not supported by an Instance object." << std::endl;
				return VK_ERROR_EXTENSION_NOT_PRESENT;
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

		result = vkCreateInstance( &instance_create_info, nullptr, &instance );
		if( (result != VK_SUCCESS) ||
			(instance == VK_NULL_HANDLE) ) {
			std::cout << "Could not create Vulkan instance." << std::endl;
			return result;
		}

		return VK_SUCCESS;
	}

	bool LoadInstanceLevelFunctions( VkInstance instance, std::vector<char const *> const & enabled_extensions ) {
	// Load core Vulkan API instance-level functions
		#define INSTANCE_LEVEL_VULKAN_FUNCTION( name )                                  \
			name = (PFN_##name)vkGetInstanceProcAddr( instance, #name );                \
			if( name == nullptr ) {                                                     \
				std::cout << "Could not load instance-level Vulkan function named: "#name << std::endl;\
				return false;                                                             \
			}

			// Load instance-level functions from enabled extensions
		#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )        \
		for( auto & enabled_extension : enabled_extensions ) {                      \
			if( std::string( enabled_extension ) == std::string( extension ) ) {      \
				name = (PFN_##name)vkGetInstanceProcAddr( instance, #name );            \
				if( name == nullptr ) {                                                 \
					std::cout << "Could not load instance-level Vulkan function named: "#name << std::endl;\
					return false;                                                         \
				}                                                                       \
			}                                                                         \
		}

		#include "ListOfVulkanFunctions.inl"

		return true;
	}

	VkResult EnumerateAvailablePhysicalDevices( VkInstance instance, std::vector<VkPhysicalDevice> & available_devices ) {
		uint32_t devices_count = 0;
		VkResult result = VK_SUCCESS;

		result = vkEnumeratePhysicalDevices( instance, &devices_count, nullptr );
		if( (result != VK_SUCCESS) || (devices_count == 0) ) {
			std::cout << "Could not get the number of available physical devices." << std::endl;
			return result;
		}

		available_devices.resize( devices_count );
		result = vkEnumeratePhysicalDevices( instance, &devices_count, available_devices.data() );
		if( (result != VK_SUCCESS) || (devices_count == 0) ) {
			std::cout << "Could not enumerate physical devices." << std::endl;
			return result;
		}

		return VK_SUCCESS;
	}

	bool CheckAvailableDeviceExtensions( VkPhysicalDevice physical_device, std::vector<VkExtensionProperties> & available_extensions ) {
		uint32_t extensions_count = 0;
		VkResult result = VK_SUCCESS;

		result = vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extensions_count, nullptr );
		if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
			std::cout << "Could not get the number of device extensions." << std::endl;
			return false;
		}

		available_extensions.resize(extensions_count);
		result = vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extensions_count, available_extensions.data() );
		if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
			std::cout << "Could not enumerate device extensions." << std::endl;
			return false;
		}

		return true;
	}

	void GetFeaturesAndPropertiesOfPhysicalDevice( VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures & device_features, VkPhysicalDeviceProperties & device_properties ) {
		vkGetPhysicalDeviceFeatures( physical_device, &device_features );

		vkGetPhysicalDeviceProperties( physical_device, &device_properties );
	}

	bool CheckAvailableQueueFamiliesAndTheirProperties( VkPhysicalDevice physical_device, std::vector<VkQueueFamilyProperties> & queue_families ) {
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

	bool SelectIndexOfQueueFamilyWithDesiredCapabilities( VkPhysicalDevice physical_device, VkQueueFlags desired_capabilities, uint32_t & queue_family_index ) {
		std::vector<VkQueueFamilyProperties> queue_families;
		if( !CheckAvailableQueueFamiliesAndTheirProperties( physical_device, queue_families ) ) {
			return false;
		}

		for( uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index ) {
			if( (queue_families[index].queueCount > 0) &&
				((queue_families[index].queueFlags & desired_capabilities) == desired_capabilities) ) {
				queue_family_index = index;
				return true;
			}
		}
		return false;
	}

	bool CreateLogicalDevice( VkPhysicalDevice physical_device, std::vector<QueueInfo> queue_infos, std::vector<char const *> const & desired_extensions,
		VkPhysicalDeviceFeatures * desired_features, VkDevice & logical_device) {
		
		std::vector<VkExtensionProperties> available_extensions;
		if( !CheckAvailableDeviceExtensions( physical_device, available_extensions )) {
			return false;
		}

		for( auto & extension : desired_extensions ){
			if( !IsExtensionSupported( available_extensions, extension ) ){
				std::cout << "Extension named '" << extension << "' is not supported by a physical device." << std::endl;
				return false;
			}
		}
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
		}

		VkDeviceCreateInfo device_create_info = {
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			nullptr,
			0,
			static_cast<uint32_t>(queue_create_infos.size()),
			queue_create_infos.data(),
			0,
			nullptr,
			static_cast<uint32_t>(desired_extensions.size()),
			desired_extensions.data(),
			desired_features
		};

		VkResult result = vkCreateDevice( physical_device, &device_create_info, nullptr, &logical_device);
		if( (result != VK_SUCCESS) || (logical_device == VK_NULL_HANDLE) ){
			std::cout << "Could not create logical device," << std::endl;
			return false;
		}

		return true;
	}

	bool LoadDeviceLevelFunctions( VkDevice logical_device, std::vector<char const *> const & enabled_extensions ) {
		// Load core Vulkan API device-level functions
		#define DEVICE_LEVEL_VULKAN_FUNCTION( name )	\
			name = (PFN_##name)vkGetDeviceProcAddr( logical_device, #name );	\
			if( name == nullptr ) {													\
				std::cout << "Could not load device-level Vulkan function named: "#name << std::endl;	\
				return false;	\
			}

		// Load device-level functions from enabled extensions
		#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )		\
			for( auto & enabled_extension : enabled_extensions ) {					\
				if( std::string( enabled_extension ) == std::string( extension ) ) {		\
					name = (PFN_##name)vkGetDeviceProcAddr( logical_device, #name );		\
					if( name == nullptr ) {													\
						std::cout << "Could not load device-level Vulkan function named: "#name << std::endl;	\
						return false;				\
					}								\
				}									\
			}

		#include "ListOfVulkanFunctions.inl"
		return true;
	}

	void GetDeviceQueue( VkDevice logical_device, uint32_t queue_family_index, uint32_t queue_index, VkQueue & queue ) {
		vkGetDeviceQueue( logical_device, queue_family_index, queue_index, &queue );
	}

	bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues( VkInstance instance, VkDevice   & logical_device, VkQueue    & graphics_queue, VkQueue    & compute_queue ) {
		std::vector<VkPhysicalDevice> physical_devices;
		EnumerateAvailablePhysicalDevices( instance, physical_devices );

		for( auto & physical_device : physical_devices ) {
			VkPhysicalDeviceFeatures device_features;
			VkPhysicalDeviceProperties device_properties;
			GetFeaturesAndPropertiesOfPhysicalDevice( physical_device, device_features, device_properties );

			if( !device_features.geometryShader ) {
				continue;
			} else {
				device_features = {};
				device_features.geometryShader = VK_TRUE;
			}

			uint32_t graphics_queue_family_index;
			if( !SelectIndexOfQueueFamilyWithDesiredCapabilities( physical_device, VK_QUEUE_GRAPHICS_BIT, graphics_queue_family_index ) ) {
				continue;
			}

			uint32_t compute_queue_family_index;
			if( !SelectIndexOfQueueFamilyWithDesiredCapabilities( physical_device, VK_QUEUE_COMPUTE_BIT, compute_queue_family_index ) ) {
				continue;
			}

			std::vector<QueueInfo> requested_queues = { { graphics_queue_family_index, { 1.0f } } };
			if( graphics_queue_family_index != compute_queue_family_index ) {
				requested_queues.push_back( { compute_queue_family_index, { 1.0f } } );
			}

			if( !CreateLogicalDevice( physical_device, requested_queues, {}, &device_features, logical_device ) ) {
				continue;
			} else {
				if( !LoadDeviceLevelFunctions( logical_device, {} ) ) {
					return false;
				}
				GetDeviceQueue( logical_device, graphics_queue_family_index, 0, graphics_queue );
				GetDeviceQueue( logical_device, compute_queue_family_index, 0, compute_queue );
				return true;
			}
		}
		return false;
	}

	void DestroyLogicalDevice( VkDevice & logical_device ) {
		if( logical_device ) {
			vkDestroyDevice( logical_device, nullptr );
			logical_device = VK_NULL_HANDLE;
		}
	}

	void DestroyVulkanInstance( VkInstance & instance ) {
		if( instance ) {
			vkDestroyInstance( instance, nullptr );
			instance = VK_NULL_HANDLE;
		}
	}

	void ReleaseVulkanLoaderLibrary( LIBRARY_TYPE & vulkan_library ) {
    	if( nullptr != vulkan_library ) {
			#if defined _WIN32
      			FreeLibrary( vulkan_library );
			#elif defined __linux
      			dlclose( vulkan_library );
			#endif
      	vulkan_library = nullptr;
    	}
	}

	// Chapter 2

	bool CreateVulkanInstanceWithWsiExtensionsEnabled( std::vector<char const *> & desired_extensions, char const * const application_name, VkInstance & instance ) {
        desired_extensions.emplace_back( VK_KHR_SURFACE_EXTENSION_NAME );
        desired_extensions.emplace_back(
            #ifdef VK_USE_PLATFORM_WIN32_KHR
                VK_KHR_WIN32_SURFACE_EXTENSION_NAME

            #elif defined VK_USE_PLATFORM_XCB_KHR
                VK_KHR_XCB_SURFACE_EXTENSION_NAME

            #elif defined VK_USE_PLATFORM_XLIB_KHR
                VK_KHR_XLIB_SURFACE_EXTENSION_NAME
            #endif
        );

        return CreateVulkanInstance( desired_extensions, application_name, instance );
    }

} // namespace VulkanCookbook

int main(){

	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;

	vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) VulkanCookbook::vkGetInstanceProcAddr( nullptr, "vkEnumerateInstanceExtensionProperties" );
	vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) VulkanCookbook::vkGetInstanceProcAddr( nullptr, "vkEnumerateInstanceLayerProperties" );

	if (vkEnumerateInstanceExtensionProperties == nullptr) {
			std::cout << "Extension properties failed" << std::endl; 
	}

	if (vkEnumerateInstanceLayerProperties == nullptr) {
			std::cout << "Layer properties failed" << std::endl; 
	}

	/*std::vector<char const *> desired_extensions = {};
	const char * application_name = "Vulkan test";
	VkInstance instance = nullptr;
	VkResult result = VK_SUCCESS;
	result = VulkanCookbook::CreateVulkanInstance(desired_extensions, application_name, instance);
	std::vector<VkPhysicalDevice> available_devices = {};
	result = VulkanCookbook::EnumerateAvailablePhysicalDevices(instance, available_devices);


	VkDevice logical_device;
	//VkPhysicalDeviceProperties properties = {};
	for (auto physical_device : available_devices){
		std::cout << "New device found:" << '\n';
		// Get features and properties

		uint32_t extension_count = 0;
		VulkanCookbook::vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extension_count, nullptr );
		std::vector<VkExtensionProperties> available_extensions = {};
		available_extensions.resize(extension_count);
		VulkanCookbook::vkEnumerateDeviceExtensionProperties( physical_device, nullptr, &extension_count, &available_extensions[0] );
		for (auto extension : available_extensions){
			std::cout << "\t" << extension.extensionName << '\n';
		}

		VkPhysicalDeviceFeatures device_features;
		VkPhysicalDeviceProperties device_properties;
		VulkanCookbook::GetFeaturesAndPropertiesOfPhysicalDevice( physical_device, device_features, device_properties );
		

		std::vector<VkQueueFamilyProperties> queue_families = {};
		VulkanCookbook::CheckAvailableQueueFamiliesAndTheirProperties( physical_device, queue_families);

		uint32_t queue_families_index = 0;
		VkQueueFlags desired_capabilities = {};
		VulkanCookbook::SelectIndexOfQueueFamilyWithDesiredCapabilities( physical_device, desired_capabilities, queue_families_index);

		available_extensions.clear();
		VulkanCookbook::CheckAvailableDeviceExtensions( physical_device, available_extensions );

		std::vector<VulkanCookbook::QueueInfo> requested_queue = { { queue_families_index, {1.0f} } };
				
		
		VulkanCookbook::CreateLogicalDevice(physical_device, requested_queue, {}, &device_features, logical_device);

		for (auto queue_family : queue_families){
			VkQueue queue;
			VulkanCookbook::GetDeviceQueue(logical_device, queue_families_index, queue_families[0].queueCount, queue);
		}

	}

	VkQueue graphics_queue;	
	VkQueue compute_queue;
	VulkanCookbook::CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues( instance, logical_device, graphics_queue, compute_queue );
	
	VulkanCookbook::DestroyLogicalDevice(logical_device);
	VulkanCookbook::DestroyVulkanInstance( instance );*/
	std::cin.ignore();
	// Chapter 2
	//desired_extensions = {};
	//VulkanCookbook::CreateVulkanInstanceWithWsiExtensionsEnabled( desired_extensions, application_name, instance );
	//VulkanCookbook::DestroyVulkanInstance( instance );

	std::cin.ignore();
	return 0;
}