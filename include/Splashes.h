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
// Splashes

#ifndef SPLASHES
#define SPLASHES

#include "Common.h"
#include <glm/glm.hpp>

namespace VulkanCookbook {

    // Spray particles thrown up where floating objects hit the water. Created by the CPU (FloatingObjects), moved by
    // shaders/splash.comp, drawn as soft camera-facing quads by shaders/splash.vert / splash.frag.
    constexpr uint32_t SPLASH_PARTICLE_COUNT            = 16384;    // a multiple of 256 (the compute shader's local size)
    constexpr uint32_t MAX_SPLASH_SPAWNS_PER_FRAME      = 2048;

    // Matches struct SplashParticle in shaders/splash.comp and splash.vert (std430)
    struct SplashParticle {
        glm::vec4 PositionLife;     // xyz: position, w: remaining life in seconds (dead at <= 0)
        glm::vec4 VelocitySize;     // xyz: velocity, w: radius of the quad in meters
    };

    // Push constants of the update. Matches the push constant block in shaders/splash.comp.
    struct SplashUpdateConstants {
        float    DeltaTime;         // seconds since the previous update (simulation clock)
        uint32_t SpawnOffset;       // the new particles go to SpawnOffset, SpawnOffset + 1, ... (wrapping around)
        uint32_t SpawnCount;
        uint32_t ParticleCount;
    };

} // namespace VulkanCookbook

#endif // SPLASHES
