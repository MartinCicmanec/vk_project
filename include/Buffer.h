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

#ifndef BUFFER
#define BUFFER

#include "Common.h"
#include "Allocator.h"

namespace VulkanCookbook {

    // Buffer with its memory. For MemoryLocation::HostVisibleMapped the CPU can write to it (CopyDataToBuffer).
    bool CreateBuffer( VmaAllocator         allocator,
                       VkDeviceSize         size,
                       VkBufferUsageFlags   usage,
                       MemoryLocation       memory_location,
                       AllocatedBuffer    & buffer );
    // Records, submits and waits for a one-time copy command buffer allocated from command_pool
    bool CopyBuffer( VkDevice        logical_device,
                     VkCommandPool   command_pool,
                     VkQueue         queue,
                     VkBuffer        source_buffer,
                     VkBuffer        destination_buffer,
                     VkDeviceSize    size );
    // Creates a buffer in device local memory and fills it with data through a temporary staging buffer.
    // usage gets VK_BUFFER_USAGE_TRANSFER_DST_BIT added automatically.
    bool CreateDeviceLocalBufferWithData( VmaAllocator         allocator,
                                          VkDevice             logical_device,
                                          VkCommandPool        command_pool,
                                          VkQueue              queue,
                                          void const         * data,
                                          VkDeviceSize         data_size,
                                          VkBufferUsageFlags   usage,
                                          AllocatedBuffer    & buffer );

} // namespace VulkanCookbook

#endif // BUFFER
