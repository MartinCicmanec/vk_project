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

#ifndef DESCRIPTOR
#define DESCRIPTOR

#include "Common.h"

namespace VulkanCookbook {

    bool CreateDescriptorSetLayout( VkDevice                                          logical_device,
                                    std::vector<VkDescriptorSetLayoutBinding> const & bindings,
                                    VkDescriptorSetLayout                           & descriptor_set_layout );
    bool CreateDescriptorPool( VkDevice                                  logical_device,
                               bool                                      free_individual_sets,
                               uint32_t                                  max_sets_count,
                               std::vector<VkDescriptorPoolSize> const & descriptor_types,
                               VkDescriptorPool                        & descriptor_pool );
    // One descriptor set is allocated for each layout
    bool AllocateDescriptorSets( VkDevice                                   logical_device,
                                 VkDescriptorPool                           descriptor_pool,
                                 std::vector<VkDescriptorSetLayout> const & descriptor_set_layouts,
                                 std::vector<VkDescriptorSet>             & descriptor_sets );
    // Points the uniform buffer descriptor at the given binding of descriptor_sets[i] to buffers[i]
    void UpdateDescriptorSetsWithUniformBuffers( VkDevice                             logical_device,
                                                 std::vector<VkDescriptorSet> const & descriptor_sets,
                                                 uint32_t                             binding,
                                                 std::vector<VkBuffer> const        & buffers,
                                                 VkDeviceSize                         range );
    // Points the storage buffer descriptor at the given binding of descriptor_sets[i] to the whole of buffers[i]
    void UpdateDescriptorSetsWithStorageBuffers( VkDevice                             logical_device,
                                                 std::vector<VkDescriptorSet> const & descriptor_sets,
                                                 uint32_t                             binding,
                                                 std::vector<VkBuffer> const        & buffers );

    // Points the storage image descriptor at the given binding of every set to the same image view
    void UpdateDescriptorSetsWithStorageImage( VkDevice                             logical_device,
                                               std::vector<VkDescriptorSet> const & descriptor_sets,
                                               uint32_t                             binding,
                                               VkImageView                          image_view,
                                               VkImageLayout                        image_layout );

    // Points the combined image sampler descriptor at the given binding of every set to the same image view and sampler
    void UpdateDescriptorSetsWithCombinedImageSampler( VkDevice                             logical_device,
                                                       std::vector<VkDescriptorSet> const & descriptor_sets,
                                                       uint32_t                             binding,
                                                       VkImageView                          image_view,
                                                       VkSampler                            sampler,
                                                       VkImageLayout                        image_layout );

} // namespace VulkanCookbook

#endif // DESCRIPTOR
