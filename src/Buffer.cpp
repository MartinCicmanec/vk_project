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

    bool CreateBuffer( VkDevice             logical_device,
                       VkDeviceSize         size,
                       VkBufferUsageFlags   usage,
                       VkBuffer           & buffer ) {
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

        VkResult result = vkCreateBuffer( logical_device, &buffer_create_info, nullptr, &buffer );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create a buffer." << std::endl;
            return false;
        }
        return true;
    }

    bool SelectMemoryType( VkPhysicalDevice        physical_device,
                           uint32_t                memory_type_bits,
                           VkMemoryPropertyFlags   memory_properties,
                           uint32_t              & memory_type_index ) {
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
        vkGetPhysicalDeviceMemoryProperties( physical_device, &physical_device_memory_properties );

        for( uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type ) {
            if( (memory_type_bits & (1u << type)) &&
                ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memory_properties) == memory_properties) ) {
                memory_type_index = type;
                return true;
            }
        }

        std::cout << "Could not find a suitable memory type." << std::endl;
        return false;
    }

    bool AllocateAndBindMemoryObjectToBuffer( VkPhysicalDevice        physical_device,
                                              VkDevice                logical_device,
                                              VkBuffer                buffer,
                                              VkMemoryPropertyFlags   memory_properties,
                                              VkDeviceMemory        & memory_object ) {
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements( logical_device, buffer, &memory_requirements );

        uint32_t memory_type_index;
        if( !SelectMemoryType( physical_device, memory_requirements.memoryTypeBits, memory_properties, memory_type_index ) ) {
            return false;
        }

        VkMemoryAllocateInfo buffer_memory_allocate_info = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,         // VkStructureType    sType
            nullptr,                                        // const void       * pNext
            memory_requirements.size,                       // VkDeviceSize       allocationSize
            memory_type_index                               // uint32_t           memoryTypeIndex
        };

        VkResult result = vkAllocateMemory( logical_device, &buffer_memory_allocate_info, nullptr, &memory_object );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not allocate memory for a buffer." << std::endl;
            return false;
        }

        result = vkBindBufferMemory( logical_device, buffer, memory_object, 0 );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not bind memory object to a buffer." << std::endl;
            return false;
        }
        return true;
    }

    bool MapUpdateAndUnmapHostVisibleMemory( VkDevice         logical_device,
                                             VkDeviceMemory   memory_object,
                                             VkDeviceSize     offset,
                                             VkDeviceSize     data_size,
                                             void const     * data ) {
        void * local_pointer;
        VkResult result = vkMapMemory( logical_device, memory_object, offset, data_size, 0, &local_pointer );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not map memory object." << std::endl;
            return false;
        }

        std::memcpy( local_pointer, data, static_cast<size_t>(data_size) );

        vkUnmapMemory( logical_device, memory_object );
        return true;
    }

    bool CreateHostVisibleBuffer( VkPhysicalDevice     physical_device,
                                  VkDevice             logical_device,
                                  VkDeviceSize         size,
                                  VkBufferUsageFlags   usage,
                                  VkBuffer           & buffer,
                                  VkDeviceMemory     & memory_object ) {
        return CreateBuffer( logical_device, size, usage, buffer ) &&
               AllocateAndBindMemoryObjectToBuffer( physical_device, logical_device, buffer,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory_object );
    }

    bool MapMemory( VkDevice         logical_device,
                    VkDeviceMemory   memory_object,
                    VkDeviceSize     offset,
                    VkDeviceSize     size,
                    void         * & pointer ) {
        VkResult result = vkMapMemory( logical_device, memory_object, offset, size, 0, &pointer );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not map memory object." << std::endl;
            return false;
        }
        return true;
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

    bool CreateDeviceLocalBufferWithData( VkPhysicalDevice     physical_device,
                                          VkDevice             logical_device,
                                          VkCommandPool        command_pool,
                                          VkQueue              queue,
                                          void const         * data,
                                          VkDeviceSize         data_size,
                                          VkBufferUsageFlags   usage,
                                          VkBuffer           & buffer,
                                          VkDeviceMemory     & memory_object ) {
        // Staging buffer in CPU visible memory - destroyed when this function returns.
        // Memory is declared first, so it is freed after the buffer is destroyed.
        VkDestroyer(VkDeviceMemory) staging_memory;
        InitVkDestroyer( logical_device, staging_memory );
        VkDestroyer(VkBuffer) staging_buffer;
        InitVkDestroyer( logical_device, staging_buffer );

        if( !CreateHostVisibleBuffer( physical_device, logical_device, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *staging_buffer, *staging_memory ) ||
            !MapUpdateAndUnmapHostVisibleMemory( logical_device, *staging_memory, 0, data_size, data ) ) {
            return false;
        }

        // Final buffer in device local memory, usually not accessible by the CPU, but the fastest for the GPU
        if( !CreateBuffer( logical_device, data_size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer ) ||
            !AllocateAndBindMemoryObjectToBuffer( physical_device, logical_device, buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object ) ) {
            return false;
        }

        return CopyBuffer( logical_device, command_pool, queue, *staging_buffer, buffer, data_size );
    }

} // namespace VulkanCookbook
