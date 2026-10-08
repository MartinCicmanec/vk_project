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

#ifndef TEXTURE
#define TEXTURE

#include "Common.h"

namespace VulkanCookbook {

    // Loads an image file (PNG, JPEG, BMP, ...) as 8 bit RGBA, uploads it through a staging buffer into a
    // device local VK_FORMAT_R8G8B8A8_SRGB image and leaves it in VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    bool CreateTextureImageFromFile( VkPhysicalDevice   physical_device,
                                     VkDevice           logical_device,
                                     VkCommandPool      command_pool,
                                     VkQueue            queue,
                                     char const       * filename,
                                     VkImage          & image,
                                     VkDeviceMemory   & memory_object );

} // namespace VulkanCookbook

#endif // TEXTURE
