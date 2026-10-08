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
// Sampler

#include "Sampler.h"

namespace VulkanCookbook {

    bool CreateSampler( VkDevice               logical_device,
                        VkFilter               filter,
                        VkSamplerAddressMode   address_mode,
                        bool                   anisotropy_enable,
                        float                  max_anisotropy,
                        VkSampler            & sampler ) {
        VkSamplerCreateInfo sampler_create_info = {
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,      // VkStructureType          sType
            nullptr,                                    // const void             * pNext
            0,                                          // VkSamplerCreateFlags     flags
            filter,                                     // VkFilter                 magFilter
            filter,                                     // VkFilter                 minFilter
            VK_SAMPLER_MIPMAP_MODE_LINEAR,              // VkSamplerMipmapMode      mipmapMode
            address_mode,                               // VkSamplerAddressMode     addressModeU
            address_mode,                               // VkSamplerAddressMode     addressModeV
            address_mode,                               // VkSamplerAddressMode     addressModeW
            0.0f,                                       // float                    mipLodBias
            anisotropy_enable ? VK_TRUE : VK_FALSE,     // VkBool32                 anisotropyEnable
            anisotropy_enable ? max_anisotropy : 1.0f,  // float                    maxAnisotropy
            VK_FALSE,                                   // VkBool32                 compareEnable
            VK_COMPARE_OP_ALWAYS,                       // VkCompareOp              compareOp
            0.0f,                                       // float                    minLod
            0.0f,                                       // float                    maxLod
            VK_BORDER_COLOR_INT_OPAQUE_BLACK,           // VkBorderColor            borderColor
            VK_FALSE                                    // VkBool32                 unnormalizedCoordinates
        };

        VkResult result = vkCreateSampler( logical_device, &sampler_create_info, nullptr, &sampler );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create a sampler." << std::endl;
            return false;
        }
        return true;
    }

} // namespace VulkanCookbook
