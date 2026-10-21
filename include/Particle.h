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
// Particle

#ifndef PARTICLE
#define PARTICLE

#include "Common.h"
#include <cstddef>
#include <glm/glm.hpp>

namespace VulkanCookbook {

    // One particle of the compute shader chapter. Matches struct Particle in shaders/particle.comp, whose storage
    // buffers use the std140 layout: vec2, vec2, vec4 gives offsets 0, 8, 16 and a size of 32 - the same as this struct.
    // The same buffer is the vertex buffer for shaders/particle.vert.
    struct Particle {
        glm::vec2 Position;     // layout(location = 0)
        glm::vec2 Velocity;     // only used by the compute shader
        glm::vec4 Color;        // layout(location = 1)

        // One Particle per vertex (one point), all from the buffer bound at binding 0
        static VkVertexInputBindingDescription GetBindingDescription() {
            return {
                0,                                          // uint32_t             binding
                sizeof( Particle ),                         // uint32_t             stride
                VK_VERTEX_INPUT_RATE_VERTEX                 // VkVertexInputRate    inputRate
            };
        }

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
            return {
                {
                    0,                                      // uint32_t   location
                    0,                                      // uint32_t   binding
                    VK_FORMAT_R32G32_SFLOAT,                // VkFormat   format
                    offsetof( Particle, Position )          // uint32_t   offset
                },
                {
                    1,                                      // uint32_t   location
                    0,                                      // uint32_t   binding
                    VK_FORMAT_R32G32B32A32_SFLOAT,          // VkFormat   format
                    offsetof( Particle, Color )             // uint32_t   offset
                }
            };
        }
    };
    static_assert( (sizeof( Particle ) == 32) && (offsetof( Particle, Color ) == 16), "Particle must match the std140 layout in shaders/particle.comp" );

    // Matches the uniform block at binding 0 in shaders/particle.comp
    struct ParticleParameters {
        float DeltaTime;        // milliseconds since the previous frame, times 2 (as in the tutorial)
    };

} // namespace VulkanCookbook

#endif // PARTICLE
