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
//
// Image

#include "Image.h"
#include "Buffer.h"

namespace VulkanCookbook {

    bool CreateImage( VkDevice            logical_device,
                      uint32_t            width,
                      uint32_t            height,
                      VkFormat            format,
                      VkImageTiling       tiling,
                      VkImageUsageFlags   usage,
                      VkImage           & image ) {
        VkImageCreateInfo image_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,        // VkStructureType          sType
            nullptr,                                    // const void             * pNext
            0,                                          // VkImageCreateFlags       flags
            VK_IMAGE_TYPE_2D,                           // VkImageType              imageType
            format,                                     // VkFormat                 format
            { width, height, 1 },                       // VkExtent3D               extent
            1,                                          // uint32_t                 mipLevels
            1,                                          // uint32_t                 arrayLayers
            VK_SAMPLE_COUNT_1_BIT,                      // VkSampleCountFlagBits    samples
            tiling,                                     // VkImageTiling            tiling
            usage,                                      // VkImageUsageFlags        usage
            VK_SHARING_MODE_EXCLUSIVE,                  // VkSharingMode            sharingMode
            0,                                          // uint32_t                 queueFamilyIndexCount
            nullptr,                                    // const uint32_t         * pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED                   // VkImageLayout            initialLayout
        };

        VkResult result = vkCreateImage( logical_device, &image_create_info, nullptr, &image );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create an image." << std::endl;
            return false;
        }
        return true;
    }

    bool AllocateAndBindMemoryObjectToImage( VkPhysicalDevice        physical_device,
                                             VkDevice                logical_device,
                                             VkImage                 image,
                                             VkMemoryPropertyFlags   memory_properties,
                                             VkDeviceMemory        & memory_object ) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements( logical_device, image, &memory_requirements );

        uint32_t memory_type_index;
        if( !SelectMemoryType( physical_device, memory_requirements.memoryTypeBits, memory_properties, memory_type_index ) ) {
            return false;
        }

        VkMemoryAllocateInfo image_memory_allocate_info = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // VkStructureType    sType
            nullptr,                                    // const void       * pNext
            memory_requirements.size,                   // VkDeviceSize       allocationSize
            memory_type_index                           // uint32_t           memoryTypeIndex
        };

        VkResult result = vkAllocateMemory( logical_device, &image_memory_allocate_info, nullptr, &memory_object );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not allocate memory for an image." << std::endl;
            return false;
        }

        result = vkBindImageMemory( logical_device, image, memory_object, 0 );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not bind memory object to an image." << std::endl;
            return false;
        }
        return true;
    }

    bool HasStencilComponent( VkFormat format ) {
        return (format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT) || (format == VK_FORMAT_D16_UNORM_S8_UINT);
    }

    bool RecordImageLayoutTransition( VkCommandBuffer   command_buffer,
                                      VkImage           image,
                                      VkFormat          format,
                                      VkImageLayout     old_layout,
                                      VkImageLayout     new_layout ) {
        // A depth/stencil image has to be transitioned with all of its aspects, even if the stencil is not used
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        if( new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ) {
            aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            if( HasStencilComponent( format ) ) {
                aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }

        VkAccessFlags        source_access;
        VkAccessFlags        destination_access;
        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if( (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) && (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) ) {
            // Transfer writes don't have to wait for anything
            source_access      = 0;
            destination_access = VK_ACCESS_TRANSFER_WRITE_BIT;
            source_stage       = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage  = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if( (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ) {
            // Fragment shader reads have to wait for the transfer writes
            source_access      = VK_ACCESS_TRANSFER_WRITE_BIT;
            destination_access = VK_ACCESS_SHADER_READ_BIT;
            source_stage       = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if( (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) && (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ) {
            // The depth buffer is read in the early fragment tests and written in the late ones - the earliest stage is used
            source_access      = 0;
            destination_access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            source_stage       = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            std::cout << "Unsupported image layout transition." << std::endl;
            return false;
        }

        VkImageMemoryBarrier image_memory_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType
            nullptr,                                    // const void               * pNext
            source_access,                              // VkAccessFlags              srcAccessMask
            destination_access,                         // VkAccessFlags              dstAccessMask
            old_layout,                                 // VkImageLayout              oldLayout
            new_layout,                                 // VkImageLayout              newLayout
            VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   srcQueueFamilyIndex (no ownership transfer)
            VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   dstQueueFamilyIndex
            image,                                      // VkImage                    image
            {                                           // VkImageSubresourceRange    subresourceRange
                aspect,                                 // VkImageAspectFlags         aspectMask
                0,                                      // uint32_t                   baseMipLevel
                VK_REMAINING_MIP_LEVELS,                // uint32_t                   levelCount
                0,                                      // uint32_t                   baseArrayLayer
                VK_REMAINING_ARRAY_LAYERS               // uint32_t                   layerCount
            }
        };

        vkCmdPipelineBarrier( command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier );
        return true;
    }

    void RecordCopyBufferToImage( VkCommandBuffer   command_buffer,
                                  VkBuffer          buffer,
                                  VkImage           image,
                                  uint32_t          width,
                                  uint32_t          height ) {
        VkBufferImageCopy region = {
            0,                                          // VkDeviceSize                 bufferOffset
            0,                                          // uint32_t                     bufferRowLength (0 = tightly packed)
            0,                                          // uint32_t                     bufferImageHeight (0 = tightly packed)
            {                                           // VkImageSubresourceLayers     imageSubresource
                VK_IMAGE_ASPECT_COLOR_BIT,              // VkImageAspectFlags           aspectMask
                0,                                      // uint32_t                     mipLevel
                0,                                      // uint32_t                     baseArrayLayer
                1                                       // uint32_t                     layerCount
            },
            { 0, 0, 0 },                                // VkOffset3D                   imageOffset
            { width, height, 1 }                        // VkExtent3D                   imageExtent
        };

        vkCmdCopyBufferToImage( command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
    }

    bool CreateImageView( VkDevice             logical_device,
                          VkImage              image,
                          VkImageViewType      view_type,
                          VkFormat             format,
                          VkImageAspectFlags   aspect,
                          VkImageView        & image_view ) {
        VkImageViewCreateInfo image_view_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType            sType
            nullptr,                                    // const void               * pNext
            0,                                          // VkImageViewCreateFlags     flags
            image,                                      // VkImage                    image
            view_type,                                  // VkImageViewType            viewType
            format,                                     // VkFormat                   format
            {                                           // VkComponentMapping         components
                VK_COMPONENT_SWIZZLE_IDENTITY,          // VkComponentSwizzle         r
                VK_COMPONENT_SWIZZLE_IDENTITY,          // VkComponentSwizzle         g
                VK_COMPONENT_SWIZZLE_IDENTITY,          // VkComponentSwizzle         b
                VK_COMPONENT_SWIZZLE_IDENTITY           // VkComponentSwizzle         a
            },
            {                                           // VkImageSubresourceRange    subresourceRange
                aspect,                                 // VkImageAspectFlags         aspectMask
                0,                                      // uint32_t                   baseMipLevel
                VK_REMAINING_MIP_LEVELS,                // uint32_t                   levelCount
                0,                                      // uint32_t                   baseArrayLayer
                VK_REMAINING_ARRAY_LAYERS               // uint32_t                   layerCount
            }
        };

        VkResult result = vkCreateImageView( logical_device, &image_view_create_info, nullptr, &image_view );
        if( (VK_SUCCESS != result) || (VK_NULL_HANDLE == image_view) ) {
            std::cout << "Could not create an image view." << std::endl;
            return false;
        }
        return true;
    }

} // namespace VulkanCookbook
