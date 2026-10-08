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
// Uniform Buffer Object

#ifndef UNIFORM_BUFFER_OBJECT
#define UNIFORM_BUFFER_OBJECT

#include <glm/glm.hpp>

namespace VulkanCookbook {

    // Matches the uniform block at binding 0 in shaders/shader.vert.
    // Alignment is explicit: vec3, vec4 and mat4 members of a uniform block must start at multiples of 16 bytes,
    // and GLM types don't guarantee that on their own (e.g. after a vec2 member or in nested structs).
    struct UniformBufferObject {
        alignas(16) glm::mat4 Model;
        alignas(16) glm::mat4 View;
        alignas(16) glm::mat4 Projection;
    };

} // namespace VulkanCookbook

#endif // UNIFORM_BUFFER_OBJECT
