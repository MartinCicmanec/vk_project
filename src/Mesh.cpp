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
// Mesh

#include <chrono>
#include "Mesh.h"
#include "Buffer.h"
#include "Model.h"

namespace VulkanCookbook {

    bool CreateMeshFromObjFile( VmaAllocator        allocator,
                                VkDevice            logical_device,
                                VkCommandPool       command_pool,
                                VkQueue             queue,
                                std::string const & filename,
                                Mesh              & mesh ) {
        // Only needed until the data is uploaded
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;

        auto start = std::chrono::steady_clock::now();
        if( !LoadModelFromObjFile( filename, vertices, indices ) ) {
            return false;
        }
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start ).count();
        std::cout << "Loaded model : " << vertices.size() << " vertices, " << indices.size() << " indices in " << milliseconds << " ms" << std::endl;

        mesh.IndexCount = static_cast<uint32_t>(indices.size());
        return CreateDeviceLocalBufferWithData( allocator, logical_device, command_pool, queue,
                                                vertices.data(), sizeof( vertices[0] ) * vertices.size(),
                                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh.VertexBuffer ) &&
               CreateDeviceLocalBufferWithData( allocator, logical_device, command_pool, queue,
                                                indices.data(), sizeof( indices[0] ) * indices.size(),
                                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mesh.IndexBuffer );
    }

} // namespace VulkanCookbook
