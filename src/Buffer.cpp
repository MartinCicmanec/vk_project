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
// Buffer

#include "Buffer.h"
#include "CommandBuffer.h"

namespace VulkanCookbook {

    bool CreateBuffer( VmaAllocator         allocator,
                       VkDeviceSize         size,
                       VkBufferUsageFlags   usage,
                       MemoryLocation       memory_location,
                       AllocatedBuffer    & buffer ) {
        VkBufferCreateInfo buffer_create_info = {
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,           // VkStructureType        sType
            nullptr,                                        // const void           * pNext
            0,                                              // VkBufferCreateFlags    flags
            size,                                           // VkDeviceSize           size
            usage,                                          // VkBufferUsageFlags     usage
            VK_SHARING_MODE_EXCLUSIVE,                      // VkSharingMode          sharingMode
            0,                                              // uint32_t               queueFamilyIndexCount
            nullptr                                         // const uint32_t       * pQueueFamilyIndices
        };

        return AllocateBuffer( allocator, buffer_create_info, memory_location, buffer );
    }

    bool CopyBuffer( VkDevice        logical_device,
                     VkCommandPool   command_pool,
                     VkQueue         queue,
                     VkBuffer        source_buffer,
                     VkBuffer        destination_buffer,
                     VkDeviceSize    size ) {
        VkCommandBuffer command_buffer;
        if( !BeginSingleTimeCommands( logical_device, command_pool, command_buffer ) ) {
            return false;
        }

        VkBufferCopy region = {
            0,                                              // VkDeviceSize   srcOffset
            0,                                              // VkDeviceSize   dstOffset
            size                                            // VkDeviceSize   size
        };
        vkCmdCopyBuffer( command_buffer, source_buffer, destination_buffer, 1, &region );

        return EndSingleTimeCommands( logical_device, command_pool, queue, command_buffer );
    }

    bool CreateDeviceLocalBufferWithData( VmaAllocator         allocator,
                                          VkDevice             logical_device,
                                          VkCommandPool        command_pool,
                                          VkQueue              queue,
                                          void const         * data,
                                          VkDeviceSize         data_size,
                                          VkBufferUsageFlags   usage,
                                          AllocatedBuffer    & buffer ) {
        // Staging buffer in CPU visible memory - destroyed when this function returns
        AllocatedBuffer staging_buffer;
        if( !CreateBuffer( allocator, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryLocation::HostVisibleMapped, staging_buffer ) ||
            !CopyDataToBuffer( staging_buffer, 0, data_size, data ) ) {
            return false;
        }

        // Final buffer in device local memory, usually not accessible by the CPU, but the fastest for the GPU
        if( !CreateBuffer( allocator, data_size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, MemoryLocation::DeviceLocal, buffer ) ) {
            return false;
        }

        return CopyBuffer( logical_device, command_pool, queue, *staging_buffer, *buffer, data_size );
    }

} // namespace VulkanCookbook
