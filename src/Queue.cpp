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
// Queue

#include "Queue.h"

namespace VulkanCookbook {

    bool SubmitCommandBuffersToQueue( VkQueue                                queue,
                                      std::vector<WaitSemaphoreInfo> const & wait_semaphore_infos,
                                      std::vector<VkCommandBuffer> const   & command_buffers,
                                      std::vector<VkSemaphore> const       & signal_semaphores,
                                      VkFence                                fence ) {
        std::vector<VkSemaphoreSubmitInfo> wait_semaphore_submit_infos;
        for( auto & wait_semaphore_info : wait_semaphore_infos ) {
            wait_semaphore_submit_infos.push_back( {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,            // VkStructureType          sType
                nullptr,                                            // const void             * pNext
                wait_semaphore_info.Semaphore,                      // VkSemaphore              semaphore
                0,                                                  // uint64_t                 value (only for timeline semaphores)
                wait_semaphore_info.WaitingStage,                   // VkPipelineStageFlags2    stageMask
                0                                                   // uint32_t                 deviceIndex
            } );
        }

        std::vector<VkCommandBufferSubmitInfo> command_buffer_submit_infos;
        for( auto & command_buffer : command_buffers ) {
            command_buffer_submit_infos.push_back( {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,       // VkStructureType    sType
                nullptr,                                            // const void       * pNext
                command_buffer,                                     // VkCommandBuffer    commandBuffer
                0                                                   // uint32_t           deviceMask
            } );
        }

        // Signaled when all of the submitted commands have finished
        std::vector<VkSemaphoreSubmitInfo> signal_semaphore_submit_infos;
        for( auto & signal_semaphore : signal_semaphores ) {
            signal_semaphore_submit_infos.push_back( {
                VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,            // VkStructureType          sType
                nullptr,                                            // const void             * pNext
                signal_semaphore,                                   // VkSemaphore              semaphore
                0,                                                  // uint64_t                 value
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,               // VkPipelineStageFlags2    stageMask
                0                                                   // uint32_t                 deviceIndex
            } );
        }

        VkSubmitInfo2 submit_info = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO_2,                                // VkStructureType                    sType
            nullptr,                                                        // const void                       * pNext
            0,                                                              // VkSubmitFlags                      flags
            static_cast<uint32_t>(wait_semaphore_submit_infos.size()),      // uint32_t                           waitSemaphoreInfoCount
            wait_semaphore_submit_infos.data(),                             // const VkSemaphoreSubmitInfo      * pWaitSemaphoreInfos
            static_cast<uint32_t>(command_buffer_submit_infos.size()),      // uint32_t                           commandBufferInfoCount
            command_buffer_submit_infos.data(),                             // const VkCommandBufferSubmitInfo  * pCommandBufferInfos
            static_cast<uint32_t>(signal_semaphore_submit_infos.size()),    // uint32_t                           signalSemaphoreInfoCount
            signal_semaphore_submit_infos.data()                            // const VkSemaphoreSubmitInfo      * pSignalSemaphoreInfos
        };

        VkResult result = vkQueueSubmit2( queue, 1, &submit_info, fence );
        if( VK_SUCCESS != result ) {
            std::cout << "Error occurred during command buffer submission." << std::endl;
            return false;
        }
        return true;
    }

    bool PresentImage( VkQueue                          queue,
                       std::vector<VkSemaphore> const & rendering_semaphores,
                       VkSwapchainKHR                   swapchain,
                       uint32_t                         image_index,
                       SwapchainStatus                & status ) {
        VkPresentInfoKHR present_info = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                     // VkStructureType          sType
            nullptr,                                                // const void             * pNext
            static_cast<uint32_t>(rendering_semaphores.size()),     // uint32_t                 waitSemaphoreCount
            rendering_semaphores.data(),                            // const VkSemaphore      * pWaitSemaphores
            1,                                                      // uint32_t                 swapchainCount
            &swapchain,                                             // const VkSwapchainKHR   * pSwapchains
            &image_index,                                           // const uint32_t         * pImageIndices
            nullptr                                                 // VkResult               * pResults
        };

        VkResult result = vkQueuePresentKHR( queue, &present_info );
        switch( result ) {
        case VK_SUCCESS:
            status = SwapchainStatus::Optimal;
            return true;
        case VK_SUBOPTIMAL_KHR:
            status = SwapchainStatus::Suboptimal;
            return true;
        case VK_ERROR_OUT_OF_DATE_KHR:
            status = SwapchainStatus::OutOfDate;
            return true;
        default:
            std::cout << "Could not present an image." << std::endl;
            return false;
        }
    }

} // namespace VulkanCookbook
