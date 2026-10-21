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

#ifndef BOX_MESH
#define BOX_MESH

#include "Common.h"
#include "Mesh.h"
#include <glm/glm.hpp>

namespace VulkanCookbook {

    // Vertex of the unit box (-1 .. 1 on every axis). Matches the per-vertex inputs of shaders/object.vert.
    struct BoxVertex {
        glm::vec3 Position;     // layout(location = 0)
        glm::vec3 Normal;       // layout(location = 1)
    };

    // Per-instance data of one box (binding 1, one per instance). Matches the per-instance inputs of shaders/object.vert.
    struct BoxInstance {
        glm::mat4 Transform;    // layout(location = 2..5) - rotation and translation only, so normals stay correct
        glm::vec4 HalfExtents;  // layout(location = 6)    - xyz: the unit box is scaled by these in the vertex shader
        glm::vec4 Color;        // layout(location = 7)    - rgb, linear
    };

    // Binding 0: BoxVertex per vertex, binding 1: BoxInstance per instance
    std::vector<VkVertexInputBindingDescription> GetBoxBindingDescriptions();
    std::vector<VkVertexInputAttributeDescription> GetBoxAttributeDescriptions();

    // Unit box with flat normals: 24 vertices (4 per face, so each face has its own normal), 36 indices.
    // Faces are counter-clockwise seen from outside.
    bool CreateBoxMesh( VmaAllocator    allocator,
                        VkDevice        logical_device,
                        VkCommandPool   command_pool,
                        VkQueue         queue,
                        Mesh          & mesh );

} // namespace VulkanCookbook

#endif // BOX_MESH
