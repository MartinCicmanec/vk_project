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
        std::vector<VkSemaphore>          wait_semaphore_handles;
        std::vector<VkPipelineStageFlags> wait_semaphore_stages;

        for( auto & wait_semaphore_info : wait_semaphore_infos ) {
            wait_semaphore_handles.emplace_back( wait_semaphore_info.Semaphore );
            wait_semaphore_stages.emplace_back( wait_semaphore_info.WaitingStage );
        }

        VkSubmitInfo submit_info = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,                          // VkStructureType                sType
            nullptr,                                                // const void                   * pNext
            static_cast<uint32_t>(wait_semaphore_infos.size()),     // uint32_t                       waitSemaphoreCount
            wait_semaphore_handles.data(),                          // const VkSemaphore            * pWaitSemaphores
            wait_semaphore_stages.data(),                           // const VkPipelineStageFlags   * pWaitDstStageMask
            static_cast<uint32_t>(command_buffers.size()),          // uint32_t                       commandBufferCount
            command_buffers.data(),                                 // const VkCommandBuffer        * pCommandBuffers
            static_cast<uint32_t>(signal_semaphores.size()),        // uint32_t                       signalSemaphoreCount
            signal_semaphores.data()                                // const VkSemaphore            * pSignalSemaphores
        };

        VkResult result = vkQueueSubmit( queue, 1, &submit_info, fence );
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
