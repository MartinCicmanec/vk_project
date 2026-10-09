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

#ifndef ALLOCATOR
#define ALLOCATOR

#include "Common.h"

// Opaque handles of the Vulkan Memory Allocator library (VMA). They are declared here (identically to vk_mem_alloc.h),
// so only Allocator.cpp has to include that large header.
VK_DEFINE_HANDLE( VmaAllocator )
VK_DEFINE_HANDLE( VmaAllocation )

namespace VulkanCookbook {

    // One allocator per logical device. It allocates a few large blocks of device memory and places buffers and images
    // inside them. It has to be destroyed after all buffers and images created with it, and before the device.
    bool CreateAllocator( VkInstance         instance,
                          VkPhysicalDevice   physical_device,
                          VkDevice           logical_device,
                          VmaAllocator     & allocator );
    void DestroyAllocator( VmaAllocator & allocator );

    // Buffer together with its memory. Both are released when the object is destroyed (or Reset() is called).
    // Can be moved, but not copied. *buffer gives the VkBuffer handle, like with VkDestroyer.
    struct AllocatedBuffer {
        VkBuffer        Handle     = VK_NULL_HANDLE;
        VmaAllocation   Allocation = VK_NULL_HANDLE;
        VmaAllocator    Allocator  = VK_NULL_HANDLE;
        void          * MappedData = nullptr;         // Only for MemoryLocation::HostVisibleMapped

        AllocatedBuffer() = default;
        ~AllocatedBuffer() { Reset(); }
        AllocatedBuffer( AllocatedBuffer && other ) noexcept;
        AllocatedBuffer & operator=( AllocatedBuffer && other ) noexcept;
        AllocatedBuffer( AllocatedBuffer const & ) = delete;
        AllocatedBuffer & operator=( AllocatedBuffer const & ) = delete;

        void Reset();
        VkBuffer operator*() const { return Handle; }
        explicit operator bool() const { return Handle != VK_NULL_HANDLE; }
    };

    // Image together with its memory - see AllocatedBuffer
    struct AllocatedImage {
        VkImage         Handle     = VK_NULL_HANDLE;
        VmaAllocation   Allocation = VK_NULL_HANDLE;
        VmaAllocator    Allocator  = VK_NULL_HANDLE;

        AllocatedImage() = default;
        ~AllocatedImage() { Reset(); }
        AllocatedImage( AllocatedImage && other ) noexcept;
        AllocatedImage & operator=( AllocatedImage && other ) noexcept;
        AllocatedImage( AllocatedImage const & ) = delete;
        AllocatedImage & operator=( AllocatedImage const & ) = delete;

        void Reset();
        VkImage operator*() const { return Handle; }
        explicit operator bool() const { return Handle != VK_NULL_HANDLE; }
    };

    enum class MemoryLocation {
        DeviceLocal,            // Fastest for the GPU, usually not accessible by the CPU
        HostVisibleMapped       // The CPU can write to it - it stays mapped (AllocatedBuffer::MappedData) for its whole lifetime
    };

    // Creates the buffer / image and allocates and binds its memory in one step. The allocator selects the memory type.
    // An object that already holds a resource is reset first.
    bool AllocateBuffer( VmaAllocator               allocator,
                         VkBufferCreateInfo const & buffer_create_info,
                         MemoryLocation             memory_location,
                         AllocatedBuffer          & buffer );
    bool AllocateImage( VmaAllocator              allocator,
                        VkImageCreateInfo const & image_create_info,
                        AllocatedImage          & image );
    // Copies data into the memory of a buffer created with MemoryLocation::HostVisibleMapped. Unlike a plain memcpy to
    // MappedData, this also flushes the memory if it is not host coherent.
    bool CopyDataToBuffer( AllocatedBuffer const & buffer,
                           VkDeviceSize            offset,
                           VkDeviceSize            data_size,
                           void const            * data );

} // namespace VulkanCookbook

#endif // ALLOCATOR
