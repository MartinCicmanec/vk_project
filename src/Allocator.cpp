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
// Allocator

// Vulkan functions are loaded at runtime in this project (VK_NO_PROTOTYPES), so VMA can't link to them statically.
// It loads them itself through vkGetInstanceProcAddr and vkGetDeviceProcAddr, which are passed to it in CreateAllocator.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
// The function bodies of VMA are compiled only in this file
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "Allocator.h"

namespace VulkanCookbook {

    bool CreateAllocator( VkInstance         instance,
                          VkPhysicalDevice   physical_device,
                          VkDevice           logical_device,
                          VmaAllocator     & allocator ) {
        VmaVulkanFunctions vulkan_functions = {};
        vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vulkan_functions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocator_create_info = {};
        allocator_create_info.physicalDevice   = physical_device;
        allocator_create_info.device           = logical_device;
        allocator_create_info.instance         = instance;
        allocator_create_info.pVulkanFunctions = &vulkan_functions;
        // Has to match the version the instance was created with (Instance.cpp)
        allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_3;

        VkResult result = vmaCreateAllocator( &allocator_create_info, &allocator );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create a memory allocator." << std::endl;
            return false;
        }
        return true;
    }

    void DestroyAllocator( VmaAllocator & allocator ) {
        if( allocator ) {
            vmaDestroyAllocator( allocator );
            allocator = VK_NULL_HANDLE;
        }
    }

    AllocatedBuffer::AllocatedBuffer( AllocatedBuffer && other ) noexcept :
        Handle( other.Handle ),
        Allocation( other.Allocation ),
        Allocator( other.Allocator ),
        MappedData( other.MappedData ) {
        other.Handle     = VK_NULL_HANDLE;
        other.Allocation = VK_NULL_HANDLE;
        other.Allocator  = VK_NULL_HANDLE;
        other.MappedData = nullptr;
    }

    AllocatedBuffer & AllocatedBuffer::operator=( AllocatedBuffer && other ) noexcept {
        if( this != &other ) {
            Reset();
            std::swap( Handle, other.Handle );
            std::swap( Allocation, other.Allocation );
            std::swap( Allocator, other.Allocator );
            std::swap( MappedData, other.MappedData );
        }
        return *this;
    }

    void AllocatedBuffer::Reset() {
        if( Handle ) {
            vmaDestroyBuffer( Allocator, Handle, Allocation );
        }
        Handle     = VK_NULL_HANDLE;
        Allocation = VK_NULL_HANDLE;
        Allocator  = VK_NULL_HANDLE;
        MappedData = nullptr;
    }

    AllocatedImage::AllocatedImage( AllocatedImage && other ) noexcept :
        Handle( other.Handle ),
        Allocation( other.Allocation ),
        Allocator( other.Allocator ) {
        other.Handle     = VK_NULL_HANDLE;
        other.Allocation = VK_NULL_HANDLE;
        other.Allocator  = VK_NULL_HANDLE;
    }

    AllocatedImage & AllocatedImage::operator=( AllocatedImage && other ) noexcept {
        if( this != &other ) {
            Reset();
            std::swap( Handle, other.Handle );
            std::swap( Allocation, other.Allocation );
            std::swap( Allocator, other.Allocator );
        }
        return *this;
    }

    void AllocatedImage::Reset() {
        if( Handle ) {
            vmaDestroyImage( Allocator, Handle, Allocation );
        }
        Handle     = VK_NULL_HANDLE;
        Allocation = VK_NULL_HANDLE;
        Allocator  = VK_NULL_HANDLE;
    }

    bool AllocateBuffer( VmaAllocator               allocator,
                         VkBufferCreateInfo const & buffer_create_info,
                         MemoryLocation             memory_location,
                         AllocatedBuffer          & buffer ) {
        buffer.Reset();

        // VMA_MEMORY_USAGE_AUTO: the memory type is selected from the usage of the buffer and these flags
        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
        if( memory_location == MemoryLocation::HostVisibleMapped ) {
            // Written sequentially by the CPU (never read back) and kept mapped
            allocation_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VmaAllocationInfo allocation_info = {};
        VkResult result = vmaCreateBuffer( allocator, &buffer_create_info, &allocation_create_info, &buffer.Handle, &buffer.Allocation, &allocation_info );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create a buffer and allocate its memory." << std::endl;
            buffer.Handle = VK_NULL_HANDLE;
            buffer.Allocation = VK_NULL_HANDLE;
            return false;
        }
        buffer.Allocator  = allocator;
        buffer.MappedData = allocation_info.pMappedData;
        return true;
    }

    bool AllocateImage( VmaAllocator              allocator,
                        VkImageCreateInfo const & image_create_info,
                        AllocatedImage          & image ) {
        image.Reset();

        // Images are never accessed by the CPU directly, so they end up in device local memory
        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;

        VkResult result = vmaCreateImage( allocator, &image_create_info, &allocation_create_info, &image.Handle, &image.Allocation, nullptr );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create an image and allocate its memory." << std::endl;
            image.Handle = VK_NULL_HANDLE;
            image.Allocation = VK_NULL_HANDLE;
            return false;
        }
        image.Allocator = allocator;
        return true;
    }

    bool CopyDataToBuffer( AllocatedBuffer const & buffer,
                           VkDeviceSize            offset,
                           VkDeviceSize            data_size,
                           void const            * data ) {
        VkResult result = vmaCopyMemoryToAllocation( buffer.Allocator, data, buffer.Allocation, offset, data_size );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not copy data to buffer memory." << std::endl;
            return false;
        }
        return true;
    }

} // namespace VulkanCookbook
