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

#ifndef IMAGE
#define IMAGE

#include "Common.h"
#include "Allocator.h"

namespace VulkanCookbook {

    // 2D image with one layer and its (device local) memory, initially in VK_IMAGE_LAYOUT_UNDEFINED.
    // Images with more than one sample per texel can only have one mip level.
    bool CreateImage( VmaAllocator        allocator,
                      uint32_t            width,
                      uint32_t            height,
                      uint32_t            mip_levels,
                      VkSampleCountFlagBits samples,
                      VkFormat            format,
                      VkImageTiling       tiling,
                      VkImageUsageFlags   usage,
                      AllocatedImage    & image );
    // Records a pipeline barrier for a range of mip levels of an image (Vulkan 1.3 synchronization2): operations in
    // destination_stage with destination_access wait for the operations in source_stage with source_access, and the
    // layout is changed in between. Use VK_PIPELINE_STAGE_2_NONE / VK_ACCESS_2_NONE when there is nothing to wait for.
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
                             VkAccessFlags2          destination_access );
    // True for depth formats that also contain a stencil component
    bool HasStencilComponent( VkFormat format );
    // Convenience version of RecordImageBarrier for all mip levels of an image. Supported transitions:
    //   UNDEFINED            -> TRANSFER_DST_OPTIMAL
    //   TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL (read in the fragment shader)
    //   UNDEFINED            -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL (format decides if the stencil aspect is included)
    //   UNDEFINED            -> COLOR_ATTACHMENT_OPTIMAL
    bool RecordImageLayoutTransition( VkCommandBuffer   command_buffer,
                                      VkImage           image,
                                      VkFormat          format,
                                      VkImageLayout     old_layout,
                                      VkImageLayout     new_layout );
    // Number of levels in a full mip chain: every level is half the size of the previous one, down to 1x1
    uint32_t CalculateMipLevelCount( uint32_t width,
                                     uint32_t height );
    // True if mip levels of an optimally tiled image with this format can be generated with RecordMipmapGeneration:
    // the format has to support linear filtering and being the source and the destination of a blit
    bool IsMipmapGenerationSupported( VkPhysicalDevice physical_device,
                                      VkFormat         format );
    // Fills the mip levels 1 .. mip_levels - 1 of a color image by repeatedly downscaling the previous level.
    // Expects level 0 to be filled and all levels to be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL (the image needs the
    // TRANSFER_SRC and TRANSFER_DST usages) and leaves all levels in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
    // Must be submitted to a queue with graphics support.
    void RecordMipmapGeneration( VkCommandBuffer   command_buffer,
                                 VkImage           image,
                                 uint32_t          width,
                                 uint32_t          height,
                                 uint32_t          mip_levels );
    // Copies to mip level 0. The image must be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, the buffer must contain tightly packed texels
    void RecordCopyBufferToImage( VkCommandBuffer   command_buffer,
                                  VkBuffer          buffer,
                                  VkImage           image,
                                  uint32_t          width,
                                  uint32_t          height );

    bool CreateImageView( VkDevice             logical_device,
                          VkImage              image,
                          VkImageViewType      view_type,
                          VkFormat             format,
                          VkImageAspectFlags   aspect,
                          VkImageView        & image_view );

} // namespace VulkanCookbook

#endif // IMAGE
