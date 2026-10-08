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
// Command Buffer

#include "CommandBuffer.h"
#include "Queue.h"

namespace VulkanCookbook {

    bool CreateCommandPool( VkDevice                   logical_device,
                            VkCommandPoolCreateFlags   command_pool_flags,
                            uint32_t                   queue_family,
                            VkCommandPool            & command_pool ) {
        VkCommandPoolCreateInfo command_pool_create_info = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,     // VkStructureType              sType
            nullptr,                                        // const void                 * pNext
            command_pool_flags,                             // VkCommandPoolCreateFlags     flags
            queue_family                                    // uint32_t                     queueFamilyIndex
        };

        VkResult result = vkCreateCommandPool( logical_device, &command_pool_create_info, nullptr, &command_pool );
        if( (VK_SUCCESS != result) || (VK_NULL_HANDLE == command_pool) ) {
            std::cout << "Could not create a command pool." << std::endl;
            return false;
        }
        return true;
    }

    bool AllocateCommandBuffers( VkDevice                       logical_device,
                                 VkCommandPool                  command_pool,
                                 VkCommandBufferLevel           level,
                                 uint32_t                       count,
                                 std::vector<VkCommandBuffer> & command_buffers ) {
        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType          sType
            nullptr,                                        // const void             * pNext
            command_pool,                                   // VkCommandPool            commandPool
            level,                                          // VkCommandBufferLevel     level
            count                                           // uint32_t                 commandBufferCount
        };

        command_buffers.resize( count );

        VkResult result = vkAllocateCommandBuffers( logical_device, &command_buffer_allocate_info, command_buffers.data() );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not allocate command buffers." << std::endl;
            return false;
        }
        return true;
    }

    bool BeginCommandBufferRecordingOperation( VkCommandBuffer                  command_buffer,
                                               VkCommandBufferUsageFlags        usage,
                                               VkCommandBufferInheritanceInfo * secondary_command_buffer_info ) {
        VkCommandBufferBeginInfo command_buffer_begin_info = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                        sType
            nullptr,                                        // const void                           * pNext
            usage,                                          // VkCommandBufferUsageFlags              flags
            secondary_command_buffer_info                   // const VkCommandBufferInheritanceInfo * pInheritanceInfo
        };

        VkResult result = vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not begin command buffer recording operation." << std::endl;
            return false;
        }
        return true;
    }

    bool EndCommandBufferRecordingOperation( VkCommandBuffer command_buffer ) {
        VkResult result = vkEndCommandBuffer( command_buffer );
        if( VK_SUCCESS != result ) {
            std::cout << "Error occurred during command buffer recording." << std::endl;
            return false;
        }
        return true;
    }

    bool BeginSingleTimeCommands( VkDevice          logical_device,
                                  VkCommandPool     command_pool,
                                  VkCommandBuffer & command_buffer ) {
        std::vector<VkCommandBuffer> command_buffers;
        if( !AllocateCommandBuffers( logical_device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, command_buffers ) ) {
            return false;
        }
        command_buffer = command_buffers[0];

        if( !BeginCommandBufferRecordingOperation( command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr ) ) {
            vkFreeCommandBuffers( logical_device, command_pool, 1, &command_buffer );
            return false;
        }
        return true;
    }

    bool EndSingleTimeCommands( VkDevice          logical_device,
                                VkCommandPool     command_pool,
                                VkQueue           queue,
                                VkCommandBuffer   command_buffer ) {
        bool success = EndCommandBufferRecordingOperation( command_buffer ) &&
                       SubmitCommandBuffersToQueue( queue, {}, { command_buffer }, {}, VK_NULL_HANDLE );

        // The simplest way to wait for one submission. A fence would allow waiting for several submissions at once.
        if( success && (VK_SUCCESS != vkQueueWaitIdle( queue )) ) {
            std::cout << "Waiting for single time commands failed." << std::endl;
            success = false;
        }

        vkFreeCommandBuffers( logical_device, command_pool, 1, &command_buffer );
        return success;
    }

    bool ResetCommandBuffer( VkCommandBuffer command_buffer,
                             bool            release_resources ) {
        VkResult result = vkResetCommandBuffer( command_buffer, release_resources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0 );
        if( VK_SUCCESS != result ) {
            std::cout << "Error occurred during command buffer reset." << std::endl;
            return false;
        }
        return true;
    }

} // namespace VulkanCookbook
