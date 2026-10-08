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
// Vertex

#ifndef VERTEX
#define VERTEX

#include "Common.h"
#include <cstddef>
#include <glm/glm.hpp>
// Hash functions for the GLM types - a GLM "experimental" extension, enabled by GLM_ENABLE_EXPERIMENTAL in CMakeLists.txt
#include <glm/gtx/hash.hpp>

namespace VulkanCookbook {

    // Interleaved vertex data matching the inputs of shaders/shader.vert
    struct Vertex {
        glm::vec3 Position;     // layout(location = 0)
        glm::vec3 Color;        // layout(location = 1)
        glm::vec2 TexCoord;     // layout(location = 2)

        bool operator==( Vertex const & other ) const {
            return (Position == other.Position) && (Color == other.Color) && (TexCoord == other.TexCoord);
        }

        // All vertex data comes from one buffer bound at binding 0, one Vertex per vertex
        static VkVertexInputBindingDescription GetBindingDescription() {
            return {
                0,                                          // uint32_t             binding
                sizeof( Vertex ),                           // uint32_t             stride
                VK_VERTEX_INPUT_RATE_VERTEX                 // VkVertexInputRate    inputRate
            };
        }

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() {
            return {
                {
                    0,                                      // uint32_t   location
                    0,                                      // uint32_t   binding
                    VK_FORMAT_R32G32B32_SFLOAT,             // VkFormat   format
                    offsetof( Vertex, Position )            // uint32_t   offset
                },
                {
                    1,                                      // uint32_t   location
                    0,                                      // uint32_t   binding
                    VK_FORMAT_R32G32B32_SFLOAT,             // VkFormat   format
                    offsetof( Vertex, Color )               // uint32_t   offset
                },
                {
                    2,                                      // uint32_t   location
                    0,                                      // uint32_t   binding
                    VK_FORMAT_R32G32_SFLOAT,                // VkFormat   format
                    offsetof( Vertex, TexCoord )            // uint32_t   offset
                }
            };
        }
    };

} // namespace VulkanCookbook

// Lets Vertex be used as a key in std::unordered_map (for removing duplicate vertices)
namespace std {
    template<> struct hash<VulkanCookbook::Vertex> {
        size_t operator()( VulkanCookbook::Vertex const & vertex ) const {
            return ((hash<glm::vec3>()( vertex.Position ) ^
                    (hash<glm::vec3>()( vertex.Color ) << 1)) >> 1) ^
                    (hash<glm::vec2>()( vertex.TexCoord ) << 1);
        }
    };
}

#endif // VERTEX
