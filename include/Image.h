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

namespace VulkanCookbook {

    // 2D image with one mip level and one layer, initially in VK_IMAGE_LAYOUT_UNDEFINED
    bool CreateImage( VkDevice            logical_device,
                      uint32_t            width,
                      uint32_t            height,
                      VkFormat            format,
                      VkImageTiling       tiling,
                      VkImageUsageFlags   usage,
                      VkImage           & image );
    bool AllocateAndBindMemoryObjectToImage( VkPhysicalDevice        physical_device,
                                             VkDevice                logical_device,
                                             VkImage                 image,
                                             VkMemoryPropertyFlags   memory_properties,
                                             VkDeviceMemory        & memory_object );
    // True for depth formats that also contain a stencil component
    bool HasStencilComponent( VkFormat format );
    // Records a pipeline barrier that changes the layout of an image. Supported transitions:
    //   UNDEFINED            -> TRANSFER_DST_OPTIMAL
    //   TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL (read in the fragment shader)
    //   UNDEFINED            -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL (format decides if the stencil aspect is included)
    bool RecordImageLayoutTransition( VkCommandBuffer   command_buffer,
                                      VkImage           image,
                                      VkFormat          format,
                                      VkImageLayout     old_layout,
                                      VkImageLayout     new_layout );
    // The image must be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, the buffer must contain tightly packed texels
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
