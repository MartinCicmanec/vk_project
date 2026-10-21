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
// Floating Objects

#ifndef FLOATING_OBJECTS
#define FLOATING_OBJECTS

#include "Common.h"
#include "Ocean.h"
#include "BoxMesh.h"
#include "Ripples.h"
#include "Splashes.h"
#include <random>
#include <glm/gtc/quaternion.hpp>

namespace VulkanCookbook {

    // Rigid boxes floating on the Gerstner waves, simulated on the CPU:
    //   - buoyancy: every box is split into a grid of sample points; each point below the surface pushes up with the
    //     weight of the water it displaces (Archimedes) and is slowed down relative to the moving water (drag)
    //   - a box is dropped every few seconds; when there are too many, the oldest one gets heavy, sinks and is removed
    //   - one long, light box is a "boat", pushed around a circle by thrust and a steering torque
    //   - boxes push each other apart (approximated by spheres)
    //   - every step, the parts of the boxes that cut through the surface are recorded as ripple sources, and parts
    //     that hit the water fast throw up splash particles (the moving boat also throws a little spray at its bow)
    // The simulation runs in fixed time steps, independent of the frame rate; rendering interpolates between the last
    // two steps. The ocean time comes from the same clock, so the boxes and the rendered waves always match.
    class FloatingObjects {
    public:
        // Simulation step in seconds
        static constexpr float    TIME_STEP = 1.0f / 120.0f;
        // Dropped boxes floating at the same time - more start to sink
        static constexpr uint32_t MAX_DROPPED_BOXES = 14;
        // Upper limit of rendered boxes (boat + floating + sinking)
        static constexpr uint32_t MAX_INSTANCES = 64;

        void Init( std::vector<GerstnerWave> const & waves );
        // Advances the simulation by the real time since the previous frame (long pauses are cut short)
        void Update( float frame_seconds );
        // Ocean time that matches GetInstances - for the shaders
        float GetRenderTime() const;
        // Transforms of all boxes, interpolated between the last two steps; at most MAX_INSTANCES
        void GetInstances( std::vector<BoxInstance> & instances ) const;
        // Moves the ripple sources of the steps since the previous call into `sources`; step_source_counts gets one
        // entry per step (the ripple field is advanced once per step, so both have to match)
        void TakeRippleSources( std::vector<RippleSource> & sources,
                                std::vector<uint32_t>     & step_source_counts );
        // Moves the splash particles created since the previous call into `particles` (at most MAX_SPLASH_SPAWNS_PER_FRAME)
        void TakeSplashParticles( std::vector<SplashParticle> & particles );

    private:
        struct Body {
            glm::vec3   HalfExtents;
            float       Mass = 1.0f;
            glm::vec3   InverseInertia;             // diagonal of the inverse inertia tensor in local space
            glm::vec4   Color;
            glm::ivec3  SampleCounts;               // buoyancy sample points per local axis

            glm::vec3   Position;
            glm::quat   Orientation;
            glm::vec3   Velocity = glm::vec3( 0.0f );
            glm::vec3   AngularVelocity = glm::vec3( 0.0f );
            // State before the last step, for interpolated rendering
            glm::vec3   PreviousPosition;
            glm::quat   PreviousOrientation;

            // Per collision sphere: position (x, z) and depth below the surface in the previous step and the one
            // before, for the ripples
            std::vector<glm::vec3> PreviousRippleState;
            std::vector<glm::vec3> EarlierRippleState;

            bool        IsBoat = false;
            bool        Sinking = false;
        };

        Body CreateBody( glm::vec3 half_extents, float density, glm::vec4 color, glm::vec3 position, glm::quat orientation );
        void SetDensity( Body & body, float density );
        void DropBox();
        void Step();
        // Adds buoyancy and drag; returns the submerged fraction of the body (0 .. 1)
        float AddWaterForces( Body const & body, glm::vec3 & force, glm::vec3 & torque ) const;
        void AddBoatControl( Body const & body, float submerged, glm::vec3 & force, glm::vec3 & torque ) const;
        void ResolveCollisions();
        // Ripple sources and splashes of this step
        void AddSurfaceEffects();
        void AddImpactSplash( glm::vec2 center, float water_height, float radius, float impact_speed );
        void AddBowSpray( Body const & body );

        GpuGerstnerWave             Waves[OCEAN_WAVE_COUNT];
        // Bodies[0] is the boat; dropped boxes follow in the order they were dropped
        std::vector<Body>           Bodies;
        // Fixed seed - every run looks the same, which makes problems reproducible
        std::mt19937                RandomEngine{ 12345 };

        float                       SimulationTime = 0.0f;
        float                       Accumulator = 0.0f;
        float                       TimeSinceLastDrop = 0.0f;

        std::vector<RippleSource>   PendingRippleSources;
        std::vector<uint32_t>       PendingStepSourceCounts;
        std::vector<SplashParticle> PendingSplashParticles;
        // Separate from RandomEngine, so splashes don't change which boxes are dropped where
        std::mt19937                SplashRandomEngine{ 54321 };
    };

} // namespace VulkanCookbook

#endif // FLOATING_OBJECTS
