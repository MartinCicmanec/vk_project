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
// Descriptor

#include "Descriptor.h"

namespace VulkanCookbook {

    bool CreateDescriptorSetLayout( VkDevice                                          logical_device,
                                    std::vector<VkDescriptorSetLayoutBinding> const & bindings,
                                    VkDescriptorSetLayout                           & descriptor_set_layout ) {
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,    // VkStructureType                      sType
            nullptr,                                                // const void                         * pNext
            0,                                                      // VkDescriptorSetLayoutCreateFlags     flags
            static_cast<uint32_t>(bindings.size()),                 // uint32_t                             bindingCount
            bindings.data()                                         // const VkDescriptorSetLayoutBinding * pBindings
        };

        VkResult result = vkCreateDescriptorSetLayout( logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create a layout for descriptor sets." << std::endl;
            return false;
        }
        return true;
    }

    bool CreateDescriptorPool( VkDevice                                  logical_device,
                               bool                                      free_individual_sets,
                               uint32_t                                  max_sets_count,
                               std::vector<VkDescriptorPoolSize> const & descriptor_types,
                               VkDescriptorPool                        & descriptor_pool ) {
        VkDescriptorPoolCreateInfo descriptor_pool_create_info = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,          // VkStructureType                sType
            nullptr,                                                // const void                   * pNext
            free_individual_sets ?                                  // VkDescriptorPoolCreateFlags    flags
                VkDescriptorPoolCreateFlags( VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT ) : 0u,
            max_sets_count,                                         // uint32_t                       maxSets
            static_cast<uint32_t>(descriptor_types.size()),         // uint32_t                       poolSizeCount
            descriptor_types.data()                                 // const VkDescriptorPoolSize   * pPoolSizes
        };

        VkResult result = vkCreateDescriptorPool( logical_device, &descriptor_pool_create_info, nullptr, &descriptor_pool );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not create a descriptor pool." << std::endl;
            return false;
        }
        return true;
    }

    bool AllocateDescriptorSets( VkDevice                                   logical_device,
                                 VkDescriptorPool                           descriptor_pool,
                                 std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
                                 std::vector<VkDescriptorSet>             & descriptor_sets ) {
        if( descriptor_set_layouts.empty() ) {
            return false;
        }

        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,         // VkStructureType                  sType
            nullptr,                                                // const void                     * pNext
            descriptor_pool,                                        // VkDescriptorPool                 descriptorPool
            static_cast<uint32_t>(descriptor_set_layouts.size()),   // uint32_t                         descriptorSetCount
            descriptor_set_layouts.data()                           // const VkDescriptorSetLayout    * pSetLayouts
        };

        descriptor_sets.resize( descriptor_set_layouts.size() );

        VkResult result = vkAllocateDescriptorSets( logical_device, &descriptor_set_allocate_info, descriptor_sets.data() );
        if( VK_SUCCESS != result ) {
            std::cout << "Could not allocate descriptor sets." << std::endl;
            return false;
        }
        return true;
    }

    void UpdateDescriptorSetsWithUniformBuffers( VkDevice                             logical_device,
                                                 std::vector<VkDescriptorSet> const & descriptor_sets,
                                                 uint32_t                             binding,
                                                 std::vector<VkBuffer> const        & buffers,
                                                 VkDeviceSize                         range ) {
        // Kept alive until vkUpdateDescriptorSets is called - the writes point into this vector
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        for( auto & buffer : buffers ) {
            buffer_infos.push_back( {
                buffer,                                             // VkBuffer         buffer
                0,                                                  // VkDeviceSize     offset
                range                                               // VkDeviceSize     range
            } );
        }

        std::vector<VkWriteDescriptorSet> descriptor_writes;
        for( size_t i = 0; i < descriptor_sets.size() && i < buffer_infos.size(); ++i ) {
            descriptor_writes.push_back( {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,             // VkStructureType                  sType
                nullptr,                                            // const void                     * pNext
                descriptor_sets[i],                                 // VkDescriptorSet                  dstSet
                binding,                                            // uint32_t                         dstBinding
                0,                                                  // uint32_t                         dstArrayElement
                1,                                                  // uint32_t                         descriptorCount
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                  // VkDescriptorType                 descriptorType
                nullptr,                                            // const VkDescriptorImageInfo    * pImageInfo
                &buffer_infos[i],                                   // const VkDescriptorBufferInfo   * pBufferInfo
                nullptr                                             // const VkBufferView             * pTexelBufferView
            } );
        }

        vkUpdateDescriptorSets( logical_device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr );
    }

    void UpdateDescriptorSetsWithCombinedImageSampler( VkDevice                             logical_device,
                                                       std::vector<VkDescriptorSet> const & descriptor_sets,
                                                       uint32_t                             binding,
                                                       VkImageView                          image_view,
                                                       VkSampler                            sampler,
                                                       VkImageLayout                        image_layout ) {
        VkDescriptorImageInfo image_info = {
            sampler,                                            // VkSampler        sampler
            image_view,                                         // VkImageView      imageView
            image_layout                                        // VkImageLayout    imageLayout
        };

        std::vector<VkWriteDescriptorSet> descriptor_writes;
        for( auto & descriptor_set : descriptor_sets ) {
            descriptor_writes.push_back( {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,         // VkStructureType                  sType
                nullptr,                                        // const void                     * pNext
                descriptor_set,                                 // VkDescriptorSet                  dstSet
                binding,                                        // uint32_t                         dstBinding
                0,                                              // uint32_t                         dstArrayElement
                1,                                              // uint32_t                         descriptorCount
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType                 descriptorType
                &image_info,                                    // const VkDescriptorImageInfo    * pImageInfo
                nullptr,                                        // const VkDescriptorBufferInfo   * pBufferInfo
                nullptr                                         // const VkBufferView             * pTexelBufferView
            } );
        }

        vkUpdateDescriptorSets( logical_device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr );
    }

} // namespace VulkanCookbook
