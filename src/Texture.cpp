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
// Texture

// The function bodies of stb_image are compiled only in this file
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Image.h"

namespace VulkanCookbook {

    bool CreateTextureImageFromFile( VkPhysicalDevice   physical_device,
                                     VkDevice           logical_device,
                                     VkCommandPool      command_pool,
                                     VkQueue            queue,
                                     char const       * filename,
                                     VkImage          & image,
                                     VkDeviceMemory   & memory_object ) {
        // STBI_rgb_alpha adds an alpha channel if the file has none, so every texel is 4 bytes
        int width = 0;
        int height = 0;
        int channels = 0;
        std::unique_ptr<stbi_uc, void( * )( void * )> pixels( stbi_load( filename, &width, &height, &channels, STBI_rgb_alpha ), stbi_image_free );
        if( !pixels ) {
            std::cout << "Could not load texture image '" << filename << "': " << stbi_failure_reason() << std::endl;
            return false;
        }
        VkDeviceSize image_size = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * 4;

        // Staging buffer in CPU visible memory - destroyed when this function returns
        VkDestroyer(VkDeviceMemory) staging_memory;
        InitVkDestroyer( logical_device, staging_memory );
        VkDestroyer(VkBuffer) staging_buffer;
        InitVkDestroyer( logical_device, staging_buffer );
        if( !CreateHostVisibleBuffer( physical_device, logical_device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *staging_buffer, *staging_memory ) ||
            !MapUpdateAndUnmapHostVisibleMemory( logical_device, *staging_memory, 0, image_size, pixels.get() ) ) {
            return false;
        }
        pixels.reset();

        // Optimal tiling - texels are stored in an implementation defined order that is fastest for sampling
        if( !CreateImage( logical_device, static_cast<uint32_t>(width), static_cast<uint32_t>(height), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, image ) ||
            !AllocateAndBindMemoryObjectToImage( physical_device, logical_device, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_object ) ) {
            return false;
        }

        // Transition, copy and transition are recorded into one command buffer and submitted once
        VkCommandBuffer command_buffer;
        if( !BeginSingleTimeCommands( logical_device, command_pool, command_buffer ) ) {
            return false;
        }
        bool recorded = RecordImageLayoutTransition( command_buffer, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
        if( recorded ) {
            RecordCopyBufferToImage( command_buffer, *staging_buffer, image, static_cast<uint32_t>(width), static_cast<uint32_t>(height) );
            recorded = RecordImageLayoutTransition( command_buffer, image, VK_FORMAT_R8G8B8A8_SRGB,
                                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
        }
        // Always ended, so the command buffer is freed even if recording failed
        bool submitted = EndSingleTimeCommands( logical_device, command_pool, queue, command_buffer );
        return recorded && submitted;
    }

} // namespace VulkanCookbook
