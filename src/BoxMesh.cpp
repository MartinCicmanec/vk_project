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
// Box Mesh

#include <cstddef>

#include "BoxMesh.h"
#include "Buffer.h"

namespace VulkanCookbook {

    std::vector<VkVertexInputBindingDescription> GetBoxBindingDescriptions() {
        return {
            {
                0,                                          // uint32_t             binding
                sizeof( BoxVertex ),                        // uint32_t             stride
                VK_VERTEX_INPUT_RATE_VERTEX                 // VkVertexInputRate    inputRate
            },
            {
                1,                                          // uint32_t             binding
                sizeof( BoxInstance ),                      // uint32_t             stride
                VK_VERTEX_INPUT_RATE_INSTANCE               // VkVertexInputRate    inputRate - advances once per box, not per vertex
            }
        };
    }

    std::vector<VkVertexInputAttributeDescription> GetBoxAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributes = {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT,     static_cast<uint32_t>( offsetof( BoxVertex, Position ) ) },
            { 1, 0, VK_FORMAT_R32G32B32_SFLOAT,     static_cast<uint32_t>( offsetof( BoxVertex, Normal ) ) }
        };
        // A mat4 input takes 4 locations, one per column
        for( uint32_t column = 0; column < 4; ++column ) {
            attributes.push_back( { 2 + column, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
                                    static_cast<uint32_t>( offsetof( BoxInstance, Transform ) + column * sizeof( glm::vec4 ) ) } );
        }
        attributes.push_back( { 6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t>( offsetof( BoxInstance, HalfExtents ) ) } );
        attributes.push_back( { 7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, static_cast<uint32_t>( offsetof( BoxInstance, Color ) ) } );
        return attributes;
    }

    bool CreateBoxMesh( VmaAllocator    allocator,
                        VkDevice        logical_device,
                        VkCommandPool   command_pool,
                        VkQueue         queue,
                        Mesh          & mesh ) {
        // For every face: the normal n and two axes u, v with u x v = n. The corners n - u - v, n + u - v, n + u + v,
        // n - u + v are then counter-clockwise seen from outside.
        struct Face { glm::vec3 N; glm::vec3 U; glm::vec3 V; };
        const Face faces[] = {
            { {  1,  0,  0 }, { 0, 1, 0 }, { 0, 0, 1 } },
            { { -1,  0,  0 }, { 0, 0, 1 }, { 0, 1, 0 } },
            { {  0,  1,  0 }, { 0, 0, 1 }, { 1, 0, 0 } },
            { {  0, -1,  0 }, { 1, 0, 0 }, { 0, 0, 1 } },
            { {  0,  0,  1 }, { 1, 0, 0 }, { 0, 1, 0 } },
            { {  0,  0, -1 }, { 0, 1, 0 }, { 1, 0, 0 } }
        };

        std::vector<BoxVertex> vertices;
        std::vector<uint32_t> indices;
        for( auto & face : faces ) {
            uint32_t first = static_cast<uint32_t>( vertices.size() );
            vertices.push_back( { face.N - face.U - face.V, face.N } );
            vertices.push_back( { face.N + face.U - face.V, face.N } );
            vertices.push_back( { face.N + face.U + face.V, face.N } );
            vertices.push_back( { face.N - face.U + face.V, face.N } );
            indices.insert( indices.end(), { first, first + 1, first + 2, first, first + 2, first + 3 } );
        }

        mesh.IndexCount = static_cast<uint32_t>( indices.size() );
        return CreateDeviceLocalBufferWithData( allocator, logical_device, command_pool, queue,
                                                vertices.data(), sizeof( BoxVertex ) * vertices.size(),
                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh.VertexBuffer ) &&
               CreateDeviceLocalBufferWithData( allocator, logical_device, command_pool, queue,
                                                indices.data(), sizeof( uint32_t ) * indices.size(),
                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mesh.IndexBuffer );
    }

} // namespace VulkanCookbook
