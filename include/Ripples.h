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
// Ripples

#ifndef RIPPLES
#define RIPPLES

#include "Common.h"
#include <glm/glm.hpp>

namespace VulkanCookbook {

    // Ripple field: a height field around the center, simulated with the wave equation by shaders/ripple.comp and
    // added to the Gerstner waves by the water shaders. Floating objects disturb it (FloatingObjects::Step).
    constexpr uint32_t RIPPLE_CELL_COUNT            = 256;       // cells per side - a multiple of 16 (the compute shader's local size)
    constexpr float    RIPPLE_HALF_SIZE             = 25.6f;     // meters from the center to the edge (0.2 m cells)
    constexpr float    RIPPLE_WAVE_SPEED            = 2.5f;      // m/s
    constexpr float    RIPPLE_DAMPING_PER_SECOND    = 0.6f;      // a ripple keeps this fraction of its height per second
    constexpr float    RIPPLE_SOURCE_STRENGTH       = 0.3f;       // water height change per meter of depth change of a body
    // The physics step is the ripple step; at most 12 steps happen in a frame (FloatingObjects caps long frames)
    constexpr uint32_t MAX_RIPPLE_STEPS_PER_FRAME   = 16;
    constexpr uint32_t MAX_RIPPLE_SOURCES_PER_STEP  = 128;

    // A piece of a body that cuts through the surface, in this step and the two before. The compute shader moves the
    // water surface by the change of the water it displaces (see shaders/ripple.comp - the step two back is needed
    // to add a displacement without also adding a lasting velocity). Matches struct RippleSource in shaders/ripple.comp (std430).
    struct RippleSource {
        glm::vec4 Current;      // xy: position (world x, z), z: radius, w: depth below the surface
        glm::vec4 Previous;     // xy: position in the previous step, z: depth in the previous step, w: unused
        glm::vec4 Earlier;      // xy: position two steps ago, z: depth two steps ago, w: unused
    };

    // Push constants of one compute step. Matches the push constant block in shaders/ripple.comp.
    struct RippleStepConstants {
        uint32_t CurrentIndex;      // which of the two height images holds the current heights (the other one is overwritten)
        uint32_t SourceOffset;      // first source of this step in the source buffer
        uint32_t SourceCount;
        uint32_t CellCount;
        float    HalfSize;
        float    CourantSquared;    // (wave speed * time step / cell size)^2 - stable below 0.5
        float    Damping;           // multiplier per step
        float    SourceStrength;
    };

} // namespace VulkanCookbook

#endif // RIPPLES
