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

#include <algorithm>
#include <cstddef>
#include <glm/gtc/constants.hpp>

#include "Ocean.h"
#include "Buffer.h"

namespace VulkanCookbook {

    std::vector<GerstnerWave> GetDefaultOceanWaves() {
        // Amplitude is about 1% of the wavelength - a calm sea. Shorter waves are steeper and more spread out.
        return {
            //  direction   wavelength  amplitude   steepness   phase
            {     0.0f,     62.0f,      0.60f,      0.35f,      0.0f },
            {    25.0f,     37.0f,      0.38f,      0.40f,      1.7f },
            {   -30.0f,     23.0f,      0.24f,      0.45f,      3.1f },
            {    50.0f,     14.0f,      0.14f,      0.50f,      4.4f },
            {   -55.0f,      8.5f,      0.08f,      0.50f,      0.9f },
            {    15.0f,      5.2f,      0.05f,      0.55f,      2.6f },
            {   -10.0f,      3.1f,      0.03f,      0.55f,      5.3f },
            {    70.0f,      1.9f,      0.018f,     0.60f,      3.8f }
        };
    }

    void PackGerstnerWaves( std::vector<GerstnerWave> const & waves,
                            GpuGerstnerWave                (& gpu_waves)[OCEAN_WAVE_COUNT] ) {
        const float gravity = 9.81f;
        for( uint32_t i = 0; i < OCEAN_WAVE_COUNT; ++i ) {
            // Missing waves stay flat (amplitude 0)
            if( i >= waves.size() ) {
                gpu_waves[i] = { glm::vec4( 1.0f, 0.0f, 1.0f, 0.0f ), glm::vec4( 0.0f ) };
                continue;
            }
            GerstnerWave const & wave = waves[i];
            float angle = glm::radians( wave.DirectionDegrees );
            float k = glm::two_pi<float>() / wave.Wavelength;
            float omega = std::sqrt( gravity * k );
            // Q * k * A summed over all waves stays <= 1 when every Steepness is <= 1
            float q = (wave.Amplitude > 0.0f) ? wave.Steepness / (k * wave.Amplitude * OCEAN_WAVE_COUNT) : 0.0f;

            gpu_waves[i].DirectionWavenumberFrequency = glm::vec4( std::cos( angle ), std::sin( angle ), k, omega );
            gpu_waves[i].AmplitudeSteepnessPhase = glm::vec4( wave.Amplitude, q, wave.Phase, 0.0f );
        }
    }

    namespace {
        // Same as GerstnerPosition in shaders/ocean.glsl, plus the time derivative (the water velocity)
        void EvaluateGerstnerWaves( GpuGerstnerWave const (& waves)[OCEAN_WAVE_COUNT],
                                    glm::vec2               grid_position,
                                    float                   time,
                                    glm::vec3             & position,
                                    glm::vec3             & velocity ) {
            position = glm::vec3( grid_position.x, 0.0f, grid_position.y );
            velocity = glm::vec3( 0.0f );
            for( auto & wave : waves ) {
                glm::vec2 direction( wave.DirectionWavenumberFrequency.x, wave.DirectionWavenumberFrequency.y );
                float k         = wave.DirectionWavenumberFrequency.z;
                float omega     = wave.DirectionWavenumberFrequency.w;
                float amplitude = wave.AmplitudeSteepnessPhase.x;
                float steepness = wave.AmplitudeSteepnessPhase.y;
                float phase     = wave.AmplitudeSteepnessPhase.z;

                float theta = k * glm::dot( direction, grid_position ) - omega * time + phase;
                float cos_theta = std::cos( theta );
                float sin_theta = std::sin( theta );
                position.x += steepness * amplitude * direction.x * cos_theta;
                position.z += steepness * amplitude * direction.y * cos_theta;
                position.y += amplitude * sin_theta;
                // d/dt of the lines above (d theta / dt = -omega)
                velocity.x += steepness * amplitude * direction.x * omega * sin_theta;
                velocity.z += steepness * amplitude * direction.y * omega * sin_theta;
                velocity.y -= amplitude * omega * cos_theta;
            }
        }
    }

    OceanSample SampleOcean( GpuGerstnerWave const (& waves)[OCEAN_WAVE_COUNT],
                             glm::vec2               world_position,
                             float                   time ) {
        // The horizontal displacement is small compared to the wavelengths, so moving the guess by the remaining
        // error converges quickly - 3 steps are well below a millimeter for these waves
        glm::vec2 grid_position = world_position;
        glm::vec3 position;
        glm::vec3 velocity;
        for( int i = 0; i < 3; ++i ) {
            EvaluateGerstnerWaves( waves, grid_position, time, position, velocity );
            grid_position += world_position - glm::vec2( position.x, position.z );
        }
        EvaluateGerstnerWaves( waves, grid_position, time, position, velocity );
        return { position.y, velocity };
    }

    VkVertexInputBindingDescription OceanGridVertex::GetBindingDescription() {
        return {
            0,                                          // uint32_t             binding
            sizeof( OceanGridVertex ),                  // uint32_t             stride
            VK_VERTEX_INPUT_RATE_VERTEX                 // VkVertexInputRate    inputRate
        };
    }

    std::vector<VkVertexInputAttributeDescription> OceanGridVertex::GetAttributeDescriptions() {
        return {
            {
                0,                                      // uint32_t   location
                0,                                      // uint32_t   binding
                VK_FORMAT_R32G32_SFLOAT,                // VkFormat   format
                offsetof( OceanGridVertex, Position )   // uint32_t   offset
            },
            {
                1,                                      // uint32_t   location
                0,                                      // uint32_t   binding
                VK_FORMAT_R32_SFLOAT,                   // VkFormat   format
                offsetof( OceanGridVertex, CellSize )   // uint32_t   offset
            }
        };
    }

    bool CreateOceanGridMesh( VmaAllocator    allocator,
                              VkDevice        logical_device,
                              VkCommandPool   command_pool,
                              VkQueue         queue,
                              uint32_t        cell_count,
                              float           half_size,
                              float           center_density,
                              Mesh          & mesh ) {
        // u goes from -1 to 1 in equal steps; position = half_size * (a u + (1 - a) u^3) with a = center_density.
        // The derivative half_size * (a + 3 (1 - a) u^2) is the spacing: a in the center, up to 3 - 2a at the edges.
        float a = center_density;
        float u_step = 2.0f / cell_count;
        std::vector<float> coordinates;
        std::vector<float> cell_sizes;
        for( uint32_t i = 0; i <= cell_count; ++i ) {
            float u = -1.0f + i * u_step;
            coordinates.push_back( half_size * (a * u + (1.0f - a) * u * u * u) );
            cell_sizes.push_back( half_size * (a + 3.0f * (1.0f - a) * u * u) * u_step );
        }

        uint32_t row_length = cell_count + 1;
        std::vector<OceanGridVertex> vertices;
        vertices.reserve( row_length * row_length );
        for( uint32_t z = 0; z <= cell_count; ++z ) {
            for( uint32_t x = 0; x <= cell_count; ++x ) {
                vertices.push_back( { glm::vec2( coordinates[x], coordinates[z] ), std::max( cell_sizes[x], cell_sizes[z] ) } );
            }
        }

        // Two triangles per cell. Counter-clockwise seen from above (+y), like the front faces of the OBJ models.
        std::vector<uint32_t> indices;
        indices.reserve( cell_count * cell_count * 6 );
        for( uint32_t z = 0; z < cell_count; ++z ) {
            for( uint32_t x = 0; x < cell_count; ++x ) {
                uint32_t i00 = z * row_length + x;          // (x,     z    )
                uint32_t i10 = i00 + 1;                     // (x + 1, z    )
                uint32_t i01 = i00 + row_length;            // (x,     z + 1)
                uint32_t i11 = i01 + 1;                     // (x + 1, z + 1)
                indices.insert( indices.end(), { i00, i01, i10, i10, i01, i11 } );
            }
        }

        mesh.IndexCount = static_cast<uint32_t>( indices.size() );
        if( !CreateDeviceLocalBufferWithData( allocator, logical_device, command_pool, queue,
                                              vertices.data(), sizeof( OceanGridVertex ) * vertices.size(),
                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh.VertexBuffer ) ||
            !CreateDeviceLocalBufferWithData( allocator, logical_device, command_pool, queue,
                                              indices.data(), sizeof( uint32_t ) * indices.size(),
                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mesh.IndexBuffer ) ) {
            return false;
        }
        std::cout << "Ocean grid : " << vertices.size() << " vertices, " << indices.size() / 3 << " triangles" << std::endl;
        return true;
    }

} // namespace VulkanCookbook
