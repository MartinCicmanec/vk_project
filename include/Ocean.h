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
// Ocean

#ifndef OCEAN
#define OCEAN

#include "Common.h"
#include "Mesh.h"
#include <glm/glm.hpp>

namespace VulkanCookbook {

    // Must match OCEAN_WAVE_COUNT in shaders/ocean.glsl
    constexpr uint32_t OCEAN_WAVE_COUNT = 8;

    // One Gerstner wave, as it is designed
    struct GerstnerWave {
        float DirectionDegrees;     // direction of travel in the xz plane, 0 = +x, 90 = +z
        float Wavelength;           // meters between two crests
        float Amplitude;            // meters from the rest level to a crest
        float Steepness;            // 0 = round sine wave, 1 = crests just become sharp points (more makes loops)
        float Phase;                // offset, so the waves don't all start at a crest
    };

    // One Gerstner wave, as the shaders use it (struct GerstnerWave in shaders/ocean.glsl, std140)
    struct GpuGerstnerWave {
        alignas(16) glm::vec4 DirectionWavenumberFrequency;
        alignas(16) glm::vec4 AmplitudeSteepnessPhase;
    };

    // Matches the uniform block at binding 0 in shaders/ocean.glsl
    struct OceanUniforms {
        alignas(16) glm::mat4   View;
        alignas(16) glm::mat4   Projection;
        alignas(16) glm::mat4   InverseViewProjection;
        alignas(16) glm::vec4   CameraPositionAndTime;
        alignas(16) glm::vec4   SunDirection;
        alignas(16) glm::vec4   RippleArea;         // x: half size, y: cells per side, z: index of the current height image
        GpuGerstnerWave         Waves[OCEAN_WAVE_COUNT];
    };

    // A calm sea with a wind towards +x: long swells and shorter waves spread around the wind direction
    std::vector<GerstnerWave> GetDefaultOceanWaves();

    // Converts the waves to the form the shaders use: wavenumber k = 2 pi / wavelength, angular frequency from the
    // deep water dispersion relation omega = sqrt(g k) (so long waves are faster, as in a real ocean), and the steepness
    // divided between the waves so that their sum stays below 1 (no loops) - GPU Gems 1, chapter 1.
    void PackGerstnerWaves( std::vector<GerstnerWave> const & waves,
                            GpuGerstnerWave                (& gpu_waves)[OCEAN_WAVE_COUNT] );

    // Water at one point of the surface, computed on the CPU from the same waves as the shaders
    struct OceanSample {
        float       Height;         // y of the surface
        glm::vec3   Velocity;       // velocity of the water there (the orbital motion of the waves)
    };

    // Water height and velocity at the world position (x, z) at the given time - for the physics of floating objects.
    // Gerstner waves also move the grid sideways, so the grid point that ends up at (x, z) is searched first (a few
    // fixed-point steps). Uses the packed waves, so it matches the shaders exactly (without their distance fading).
    OceanSample SampleOcean( GpuGerstnerWave const (& waves)[OCEAN_WAVE_COUNT],
                             glm::vec2               world_position,
                             float                   time );

    // Vertex of the ocean grid. The shaders displace it with the waves.
    struct OceanGridVertex {
        glm::vec2 Position;     // layout(location = 0) - undisplaced position in the xz plane
        float     CellSize;     // layout(location = 1) - distance to the neighboring vertices, for fading out small waves

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };

    // Flat square grid around the origin, from -half_size to half_size on x and z, with cell_count x cell_count cells.
    // The cells are small in the center (center_density times the uniform size) and grow towards the edges
    // (cubic spacing), so vertices are spent where the camera looks. Triangles are counter-clockwise seen from above.
    bool CreateOceanGridMesh( VmaAllocator    allocator,
                              VkDevice        logical_device,
                              VkCommandPool   command_pool,
                              VkQueue         queue,
                              uint32_t        cell_count,
                              float           half_size,
                              float           center_density,
                              Mesh          & mesh );

} // namespace VulkanCookbook

#endif // OCEAN
