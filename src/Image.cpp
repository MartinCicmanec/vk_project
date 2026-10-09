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

#include <algorithm>
#include "Image.h"

namespace VulkanCookbook {

    bool CreateImage( VmaAllocator        allocator,
                      uint32_t            width,
                      uint32_t            height,
                      uint32_t            mip_levels,
                      VkSampleCountFlagBits samples,
                      VkFormat            format,
                      VkImageTiling       tiling,
                      VkImageUsageFlags   usage,
                      AllocatedImage    & image ) {
        VkImageCreateInfo image_create_info = {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,        // VkStructureType          sType
            nullptr,                                    // const void             * pNext
            0,                                          // VkImageCreateFlags       flags
            VK_IMAGE_TYPE_2D,                           // VkImageType              imageType
            format,                                     // VkFormat                 format
            { width, height, 1 },                       // VkExtent3D               extent
            mip_levels,                                 // uint32_t                 mipLevels
            1,                                          // uint32_t                 arrayLayers
            samples,                                    // VkSampleCountFlagBits    samples
            tiling,                                     // VkImageTiling            tiling
            usage,                                      // VkImageUsageFlags        usage
            VK_SHARING_MODE_EXCLUSIVE,                  // VkSharingMode            sharingMode
            0,                                          // uint32_t                 queueFamilyIndexCount
            nullptr,                                    // const uint32_t         * pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED                   // VkImageLayout            initialLayout
        };

        return AllocateImage( allocator, image_create_info, image );
    }

    bool HasStencilComponent( VkFormat format ) {
        return (format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format == VK_FORMAT_D24_UNORM_S8_UINT) || (format == VK_FORMAT_D16_UNORM_S8_UINT);
    }

    void RecordImageBarrier( VkCommandBuffer         command_buffer,
                             VkImage                 image,
                             VkImageAspectFlags      aspect,
                             uint32_t                base_mip_level,
                             uint32_t                level_count,
                             VkImageLayout           old_layout,
                             VkImageLayout           new_layout,
                             VkPipelineStageFlags2   source_stage,
                             VkAccessFlags2          source_access,
                             VkPipelineStageFlags2   destination_stage,
                             VkAccessFlags2          destination_access ) {
        VkImageMemoryBarrier2 image_memory_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,   // VkStructureType            sType
            nullptr,                                    // const void               * pNext
            source_stage,                               // VkPipelineStageFlags2      srcStageMask
            source_access,                              // VkAccessFlags2             srcAccessMask
            destination_stage,                          // VkPipelineStageFlags2      dstStageMask
            destination_access,                         // VkAccessFlags2             dstAccessMask
            old_layout,                                 // VkImageLayout              oldLayout
            new_layout,                                 // VkImageLayout              newLayout
            VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   srcQueueFamilyIndex (no ownership transfer)
            VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   dstQueueFamilyIndex
            image,                                      // VkImage                    image
            {                                           // VkImageSubresourceRange    subresourceRange
                aspect,                                 // VkImageAspectFlags         aspectMask
                base_mip_level,                         // uint32_t                   baseMipLevel
                level_count,                            // uint32_t                   levelCount
                0,                                      // uint32_t                   baseArrayLayer
                VK_REMAINING_ARRAY_LAYERS               // uint32_t                   layerCount
            }
        };

        VkDependencyInfo dependency_info = {
            VK_STRUCTURE_TYPE_DEPENDENCY_INFO,          // VkStructureType                  sType
            nullptr,                                    // const void                     * pNext
            0,                                          // VkDependencyFlags                dependencyFlags
            0,                                          // uint32_t                         memoryBarrierCount
            nullptr,                                    // const VkMemoryBarrier2         * pMemoryBarriers
            0,                                          // uint32_t                         bufferMemoryBarrierCount
            nullptr,                                    // const VkBufferMemoryBarrier2   * pBufferMemoryBarriers
            1,                                          // uint32_t                         imageMemoryBarrierCount
            &image_memory_barrier                       // const VkImageMemoryBarrier2    * pImageMemoryBarriers
        };

        vkCmdPipelineBarrier2( command_buffer, &dependency_info );
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

        if( (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) && (new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) ) {
            // Transfer writes don't have to wait for anything
            RecordImageBarrier( command_buffer, image, aspect, 0, VK_REMAINING_MIP_LEVELS, old_layout, new_layout,
                                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT );
        } else if( (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) ) {
            // Fragment shader reads have to wait for the transfer writes
            RecordImageBarrier( command_buffer, image, aspect, 0, VK_REMAINING_MIP_LEVELS, old_layout, new_layout,
                                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT );
        } else if( (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) && (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ) {
            // The depth buffer is read in the early fragment tests and written in the late ones
            RecordImageBarrier( command_buffer, image, aspect, 0, VK_REMAINING_MIP_LEVELS, old_layout, new_layout,
                                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT );
        } else if( (old_layout == VK_IMAGE_LAYOUT_UNDEFINED) && (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) ) {
            RecordImageBarrier( command_buffer, image, aspect, 0, VK_REMAINING_MIP_LEVELS, old_layout, new_layout,
                                VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
                                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT );
        } else {
            std::cout << "Unsupported image layout transition." << std::endl;
            return false;
        }
        return true;
    }

    uint32_t CalculateMipLevelCount( uint32_t width,
                                     uint32_t height ) {
        // How many times the largest dimension can be halved, plus one for the original image
        uint32_t levels = 1;
        for( uint32_t size = std::max( width, height ); size > 1; size /= 2 ) {
            ++levels;
        }
        return levels;
    }

    bool IsMipmapGenerationSupported( VkPhysicalDevice physical_device,
                                      VkFormat         format ) {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties( physical_device, format, &format_properties );

        VkFormatFeatureFlags required_features = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                                                 VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                                                 VK_FORMAT_FEATURE_BLIT_DST_BIT;
        return (format_properties.optimalTilingFeatures & required_features) == required_features;
    }

    void RecordMipmapGeneration( VkCommandBuffer   command_buffer,
                                 VkImage           image,
                                 uint32_t          width,
                                 uint32_t          height,
                                 uint32_t          mip_levels ) {
        int32_t mip_width  = static_cast<int32_t>(width);
        int32_t mip_height = static_cast<int32_t>(height);

        for( uint32_t level = 1; level < mip_levels; ++level ) {
            // The previous level was filled (by the buffer copy or the previous blit) - make it the source of this blit
            RecordImageBarrier( command_buffer, image, VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 1,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT );

            // A dimension that reached 1 stays 1 (images that are not square)
            int32_t next_width  = (mip_width  > 1) ? (mip_width  / 2) : 1;
            int32_t next_height = (mip_height > 1) ? (mip_height / 2) : 1;

            VkImageBlit blit = {
                { VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 0, 1 },         // VkImageSubresourceLayers    srcSubresource
                { { 0, 0, 0 }, { mip_width, mip_height, 1 } },          // VkOffset3D                  srcOffsets[2]
                { VK_IMAGE_ASPECT_COLOR_BIT, level, 0, 1 },             // VkImageSubresourceLayers    dstSubresource
                { { 0, 0, 0 }, { next_width, next_height, 1 } }         // VkOffset3D                  dstOffsets[2]
            };
            vkCmdBlitImage( command_buffer,
                            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            1, &blit, VK_FILTER_LINEAR );

            // The previous level is finished - it will only be sampled from now on
            RecordImageBarrier( command_buffer, image, VK_IMAGE_ASPECT_COLOR_BIT, level - 1, 1,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT );

            mip_width  = next_width;
            mip_height = next_height;
        }

        // The last level is never a blit source, so it is still in the transfer destination layout
        RecordImageBarrier( command_buffer, image, VK_IMAGE_ASPECT_COLOR_BIT, mip_levels - 1, 1,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT );
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
