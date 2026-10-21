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

#include <algorithm>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FloatingObjects.h"

namespace VulkanCookbook {

    namespace {
        const float Gravity             = 9.81f;
        const float WaterDensity        = 1025.0f;  // kg/m^3, sea water
        // Drag relative to the water, per second, for a fully submerged body - slows bobbing and makes boxes drift with the waves
        const float WaterDrag           = 2.5f;
        // Rotation slows down by this fraction per second (water resistance against turning, simplified)
        const float AngularDamping      = 0.8f;
        const float MaxAngularVelocity  = 20.0f;    // rad/s, guards against explosions after hard impacts
        const float SinkingDensity      = 2500.0f;
        const float RemoveDepth         = -15.0f;   // sunk boxes are removed below this

        const float DropInterval        = 2.5f;     // seconds
        const float DropAreaRadius      = 10.0f;
        const float BoatCircleRadius    = 14.0f;
        const float BoatSpeed           = 4.0f;     // m/s
        const float CollisionRestitution = 0.2f;
        const float SplashSpeed         = 2.0f;     // m/s - parts entering the water faster than this throw up spray
        const float BowSpraySpeed       = 2.5f;     // m/s - the boat throws bow spray above this speed

        float Random( std::mt19937 & engine, float min, float max ) {
            return std::uniform_real_distribution<float>( min, max )( engine );
        }

        // Spheres that approximate a box for collisions: a chain along the longest local axis
        void GetCollisionSpheres( glm::vec3 half_extents, std::vector<glm::vec3> & centers, float & radius ) {
            int axis = 0;
            if( half_extents[1] > half_extents[axis] ) axis = 1;
            if( half_extents[2] > half_extents[axis] ) axis = 2;
            float other_half = std::max( half_extents[(axis + 1) % 3], half_extents[(axis + 2) % 3] );
            // Between the inscribed and the bounding sphere of a cube - corners overlap a little, faces a little less
            radius = 1.1f * other_half;
            int count = std::max( 1, static_cast<int>( std::ceil( half_extents[axis] / radius ) ) );
            centers.clear();
            for( int i = 0; i < count; ++i ) {
                glm::vec3 center( 0.0f );
                float span = std::max( half_extents[axis] - radius, 0.0f );
                center[axis] = (count == 1) ? 0.0f : -span + 2.0f * span * i / (count - 1);
                centers.push_back( center );
            }
        }
    }

    void FloatingObjects::Init( std::vector<GerstnerWave> const & waves ) {
        PackGerstnerWaves( waves, Waves );

        // Boat: long and light, on the circle, heading along it (counter-clockwise seen from above).
        // Its local +x is the bow; rotating -90 degrees around y turns +x into +z, the direction of travel at (r, 0, 0).
        Body boat = CreateBody( glm::vec3( 2.0f, 0.45f, 0.8f ), 350.0f, glm::vec4( 0.75f, 0.12f, 0.08f, 1.0f ),
                                glm::vec3( BoatCircleRadius, 0.0f, 0.0f ), glm::angleAxis( glm::radians( -90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ) );
        boat.IsBoat = true;
        Bodies.push_back( boat );

        // The first box is dropped right away
        TimeSinceLastDrop = DropInterval;
    }

    FloatingObjects::Body FloatingObjects::CreateBody( glm::vec3 half_extents, float density, glm::vec4 color, glm::vec3 position, glm::quat orientation ) {
        Body body;
        body.HalfExtents = half_extents;
        body.Color = color;
        // About 0.4 m between sample points, at least 2 per axis (so tilting creates a torque)
        glm::vec3 size = 2.0f * half_extents;
        body.SampleCounts = glm::clamp( glm::ivec3( glm::ceil( size / 0.4f ) ), glm::ivec3( 2 ), glm::ivec3( 6 ) );
        body.Position = position;
        body.Orientation = orientation;
        body.PreviousPosition = position;
        body.PreviousOrientation = orientation;
        SetDensity( body, density );
        return body;
    }

    void FloatingObjects::SetDensity( Body & body, float density ) {
        glm::vec3 size = 2.0f * body.HalfExtents;
        body.Mass = density * size.x * size.y * size.z;
        // Solid box: I = m / 12 * (b^2 + c^2) for each axis
        glm::vec3 squared = size * size;
        glm::vec3 inertia = body.Mass / 12.0f * glm::vec3( squared.y + squared.z, squared.x + squared.z, squared.x + squared.y );
        body.InverseInertia = 1.0f / inertia;
    }

    void FloatingObjects::DropBox() {
        // Wooden crates in a few colors
        const glm::vec4 colors[] = {
            { 0.45f, 0.28f, 0.12f, 1.0f },
            { 0.55f, 0.38f, 0.18f, 1.0f },
            { 0.30f, 0.18f, 0.08f, 1.0f },
            { 0.15f, 0.30f, 0.55f, 1.0f },
            { 0.70f, 0.60f, 0.20f, 1.0f }
        };
        glm::vec3 half_extents( Random( RandomEngine, 0.3f, 0.8f ), Random( RandomEngine, 0.3f, 0.8f ), Random( RandomEngine, 0.3f, 0.8f ) );
        float density = Random( RandomEngine, 350.0f, 700.0f );
        glm::vec4 color = colors[std::uniform_int_distribution<int>( 0, 4 )( RandomEngine )];

        // Anywhere in a disc around the center (sqrt: evenly over the area), a few meters above the water, randomly turned
        float angle = Random( RandomEngine, 0.0f, glm::two_pi<float>() );
        float distance = DropAreaRadius * std::sqrt( Random( RandomEngine, 0.0f, 1.0f ) );
        glm::vec3 position( distance * std::cos( angle ), Random( RandomEngine, 5.0f, 9.0f ), distance * std::sin( angle ) );
        glm::vec3 axis = glm::normalize( glm::vec3( Random( RandomEngine, -1.0f, 1.0f ), Random( RandomEngine, -1.0f, 1.0f ), Random( RandomEngine, -1.0f, 1.0f ) ) + glm::vec3( 0.0f, 0.001f, 0.0f ) );
        glm::quat orientation = glm::angleAxis( Random( RandomEngine, 0.0f, glm::two_pi<float>() ), axis );

        Body box = CreateBody( half_extents, density, color, position, orientation );
        box.AngularVelocity = glm::vec3( Random( RandomEngine, -1.5f, 1.5f ), Random( RandomEngine, -1.5f, 1.5f ), Random( RandomEngine, -1.5f, 1.5f ) );
        Bodies.push_back( box );

        // Too many floating boxes - the oldest one sinks
        uint32_t floating = 0;
        for( auto & body : Bodies ) {
            if( !body.IsBoat && !body.Sinking ) {
                ++floating;
            }
        }
        if( floating > MAX_DROPPED_BOXES ) {
            for( auto & body : Bodies ) {
                if( !body.IsBoat && !body.Sinking ) {
                    body.Sinking = true;
                    SetDensity( body, SinkingDensity );
                    break;
                }
            }
        }
    }

    void FloatingObjects::Update( float frame_seconds ) {
        // After a long pause (window minimized, resizing) the simulation continues where it stopped instead of catching up
        Accumulator += std::min( frame_seconds, 0.1f );
        while( Accumulator >= TIME_STEP ) {
            Step();
            Accumulator -= TIME_STEP;
        }
    }

    float FloatingObjects::GetRenderTime() const {
        // Between the previous step (SimulationTime - TIME_STEP) and the last one, like the box transforms
        return SimulationTime - TIME_STEP + Accumulator;
    }

    void FloatingObjects::GetInstances( std::vector<BoxInstance> & instances ) const {
        float alpha = Accumulator / TIME_STEP;
        instances.clear();
        for( auto & body : Bodies ) {
            if( instances.size() >= MAX_INSTANCES ) {
                break;
            }
            glm::vec3 position = glm::mix( body.PreviousPosition, body.Position, alpha );
            glm::quat orientation = glm::slerp( body.PreviousOrientation, body.Orientation, alpha );
            instances.push_back( {
                glm::translate( glm::mat4( 1.0f ), position ) * glm::mat4_cast( orientation ),
                glm::vec4( body.HalfExtents, 0.0f ),
                body.Color
            } );
        }
    }

    float FloatingObjects::AddWaterForces( Body const & body, glm::vec3 & force, glm::vec3 & torque ) const {
        glm::mat3 rotation = glm::mat3_cast( body.Orientation );
        glm::vec3 size = 2.0f * body.HalfExtents;
        glm::vec3 cell = size / glm::vec3( body.SampleCounts );
        float cell_volume = cell.x * cell.y * cell.z;
        // Each point stands for a cube-ish piece of the box: it is partly submerged within +-half its size around the surface
        float cell_size = std::cbrt( cell_volume );
        int point_count = body.SampleCounts.x * body.SampleCounts.y * body.SampleCounts.z;

        float submerged = 0.0f;
        for( int x = 0; x < body.SampleCounts.x; ++x ) {
            for( int y = 0; y < body.SampleCounts.y; ++y ) {
                for( int z = 0; z < body.SampleCounts.z; ++z ) {
                    glm::vec3 local = -body.HalfExtents + (glm::vec3( x, y, z ) + 0.5f) * cell;
                    glm::vec3 arm = rotation * local;
                    glm::vec3 point = body.Position + arm;

                    OceanSample water = SampleOcean( Waves, glm::vec2( point.x, point.z ), SimulationTime );
                    float fraction = glm::clamp( (water.Height - point.y) / cell_size + 0.5f, 0.0f, 1.0f );
                    if( fraction <= 0.0f ) {
                        continue;
                    }
                    float displaced_mass = WaterDensity * cell_volume * fraction;

                    // Archimedes: up with the weight of the displaced water. Drag: towards the velocity of the water.
                    glm::vec3 point_velocity = body.Velocity + glm::cross( body.AngularVelocity, arm );
                    glm::vec3 point_force = glm::vec3( 0.0f, displaced_mass * Gravity, 0.0f ) -
                                            displaced_mass * WaterDrag * (point_velocity - water.Velocity);

                    force += point_force;
                    torque += glm::cross( arm, point_force );
                    submerged += fraction / point_count;
                }
            }
        }
        return submerged;
    }

    void FloatingObjects::AddBoatControl( Body const & body, float submerged, glm::vec3 & force, glm::vec3 & torque ) const {
        // The propeller and rudder only work in the water
        float control = std::min( 1.0f, submerged * 4.0f );
        if( control <= 0.0f ) {
            return;
        }

        // Heading in the xz plane (the boat may pitch and roll)
        glm::vec3 bow = glm::mat3_cast( body.Orientation ) * glm::vec3( 1.0f, 0.0f, 0.0f );
        glm::vec2 heading( bow.x, bow.z );
        if( glm::length( heading ) < 1e-3f ) {
            return;
        }
        heading = glm::normalize( heading );

        // Desired direction: along the circle (counter-clockwise from above), steering back towards it when off
        glm::vec2 radial( body.Position.x, body.Position.z );
        float distance = std::max( glm::length( radial ), 1e-3f );
        radial /= distance;
        glm::vec2 tangent( -radial.y, radial.x );
        glm::vec2 desired = glm::normalize( tangent + 4.0f * (BoatCircleRadius - distance) / BoatCircleRadius * radial );

        // Thrust: what the water drag takes away at the target speed, plus a correction towards it. Without the first
        // part the boat would settle where the correction equals the drag - far below the target speed.
        float forward_speed = glm::dot( glm::vec2( body.Velocity.x, body.Velocity.z ), heading );
        float acceleration = glm::clamp( WaterDrag * BoatSpeed + 1.5f * (BoatSpeed - forward_speed), 0.0f, 15.0f );
        force += control * body.Mass * acceleration * glm::vec3( heading.x, 0.0f, heading.y );

        // Keel: sideways motion is damped much more than forward motion
        glm::vec2 side( -heading.y, heading.x );
        float side_speed = glm::dot( glm::vec2( body.Velocity.x, body.Velocity.z ), side );
        force -= control * body.Mass * 3.0f * side_speed * glm::vec3( side.x, 0.0f, side.y );

        // Rudder: a rotation by +angle around y turns +x towards -z, so the sign of the turn towards `desired` is
        // heading.y * desired.x - heading.x * desired.y. Going counter-clockwise (seen from above) around the circle
        // needs a yaw rate of -speed / radius - the controller aims for that rate, corrected by the heading error.
        // Without the rate target, the boat would need a constant heading error to keep turning and drift outwards.
        float turn = heading.y * desired.x - heading.x * desired.y;
        if( glm::dot( heading, desired ) < 0.0f ) {
            turn = (turn >= 0.0f) ? 1.0f : -1.0f;       // facing away - turn at full rate
        }
        float circle_yaw_rate = -forward_speed / BoatCircleRadius;
        float yaw_inertia = 1.0f / body.InverseInertia.y;
        torque.y += control * yaw_inertia * (4.0f * turn + 3.0f * (circle_yaw_rate - body.AngularVelocity.y));
    }

    void FloatingObjects::Step() {
        TimeSinceLastDrop += TIME_STEP;
        if( TimeSinceLastDrop >= DropInterval ) {
            TimeSinceLastDrop = 0.0f;
            DropBox();
        }

        for( auto & body : Bodies ) {
            body.PreviousPosition = body.Position;
            body.PreviousOrientation = body.Orientation;

            glm::vec3 force( 0.0f, -body.Mass * Gravity, 0.0f );
            glm::vec3 torque( 0.0f );
            float submerged = AddWaterForces( body, force, torque );
            if( body.IsBoat ) {
                AddBoatControl( body, submerged, force, torque );
            }

            // Semi-implicit Euler: velocities first, then positions with the new velocities (stable for these springs)
            body.Velocity += force / body.Mass * TIME_STEP;

            glm::mat3 rotation = glm::mat3_cast( body.Orientation );
            glm::mat3 inverse_inertia_world = rotation * glm::mat3( glm::vec3( body.InverseInertia.x, 0.0f, 0.0f ),
                                                                    glm::vec3( 0.0f, body.InverseInertia.y, 0.0f ),
                                                                    glm::vec3( 0.0f, 0.0f, body.InverseInertia.z ) ) * glm::transpose( rotation );
            body.AngularVelocity += inverse_inertia_world * torque * TIME_STEP;
            body.AngularVelocity *= std::exp( -AngularDamping * TIME_STEP );
            float angular_speed = glm::length( body.AngularVelocity );
            if( angular_speed > MaxAngularVelocity ) {
                body.AngularVelocity *= MaxAngularVelocity / angular_speed;
            }

            body.Position += body.Velocity * TIME_STEP;
            // dq/dt = 1/2 * (0, omega) * q
            glm::quat spin( 0.0f, body.AngularVelocity.x, body.AngularVelocity.y, body.AngularVelocity.z );
            body.Orientation = glm::normalize( body.Orientation + (0.5f * TIME_STEP) * (spin * body.Orientation) );
        }

        ResolveCollisions();
        AddSurfaceEffects();

        // Sunk boxes are gone (the water is opaque, so they disappear before this)
        Bodies.erase( std::remove_if( Bodies.begin(), Bodies.end(), []( Body const & body ) {
            return body.Sinking && (body.Position.y < RemoveDepth);
        } ), Bodies.end() );

        SimulationTime += TIME_STEP;
    }

    void FloatingObjects::TakeRippleSources( std::vector<RippleSource> & sources,
                                             std::vector<uint32_t>     & step_source_counts ) {
        sources.swap( PendingRippleSources );
        step_source_counts.swap( PendingStepSourceCounts );
        PendingRippleSources.clear();
        PendingStepSourceCounts.clear();
    }

    void FloatingObjects::TakeSplashParticles( std::vector<SplashParticle> & particles ) {
        particles.swap( PendingSplashParticles );
        PendingSplashParticles.clear();
    }

    void FloatingObjects::AddImpactSplash( glm::vec2 center, float water_height, float radius, float impact_speed ) {
        // More and higher spray for faster and bigger impacts; a box falling from 9 m hits with about 13 m/s
        float speed = std::min( impact_speed, 14.0f );
        int count = std::min( 30, static_cast<int>( speed * radius * radius * 12.0f ) );
        for( int i = 0; i < count; ++i ) {
            if( PendingSplashParticles.size() >= MAX_SPLASH_SPAWNS_PER_FRAME ) {
                return;
            }
            // From a ring around the impact, outwards and up
            float angle = Random( SplashRandomEngine, 0.0f, glm::two_pi<float>() );
            glm::vec2 direction( std::cos( angle ), std::sin( angle ) );
            glm::vec2 start = center + direction * radius * Random( SplashRandomEngine, 0.6f, 1.1f );
            glm::vec2 outward = direction * Random( SplashRandomEngine, 0.5f, 2.0f ) * (0.15f * speed);
            float upward = Random( SplashRandomEngine, 0.4f, 1.0f ) * 0.7f * speed;
            PendingSplashParticles.push_back( {
                glm::vec4( start.x, water_height + Random( SplashRandomEngine, 0.0f, 0.3f ), start.y, Random( SplashRandomEngine, 1.0f, 2.0f ) ),
                glm::vec4( outward.x, upward, outward.y, Random( SplashRandomEngine, 0.04f, 0.12f ) )
            } );
        }
    }

    void FloatingObjects::AddBowSpray( Body const & body ) {
        glm::vec2 velocity( body.Velocity.x, body.Velocity.z );
        float speed = glm::length( velocity );
        if( speed < BowSpraySpeed ) {
            return;
        }
        glm::mat3 rotation = glm::mat3_cast( body.Orientation );
        glm::vec3 bow = body.Position + rotation * glm::vec3( body.HalfExtents.x, 0.0f, 0.0f );
        glm::vec3 side = rotation * glm::vec3( 0.0f, 0.0f, 1.0f );
        float water_height = SampleOcean( Waves, glm::vec2( bow.x, bow.z ), SimulationTime ).Height;
        // Only while the bow is in the water
        if( std::abs( bow.y - water_height ) > body.HalfExtents.y + 0.2f ) {
            return;
        }
        // About 1 particle per step at 4 m/s, thrown to both sides
        int count = (Random( SplashRandomEngine, 0.0f, 1.0f ) < (speed - BowSpraySpeed) / 1.5f) ? 2 : 1;
        for( int i = 0; i < count; ++i ) {
            if( PendingSplashParticles.size() >= MAX_SPLASH_SPAWNS_PER_FRAME ) {
                return;
            }
            float side_sign = (i == 0) ? 1.0f : -1.0f;
            glm::vec3 start = bow + side * side_sign * body.HalfExtents.z * 0.8f;
            glm::vec3 spray = 0.5f * glm::vec3( velocity.x, 0.0f, velocity.y ) +
                              side * side_sign * Random( SplashRandomEngine, 1.0f, 2.5f ) +
                              glm::vec3( 0.0f, Random( SplashRandomEngine, 1.0f, 2.5f ), 0.0f );
            PendingSplashParticles.push_back( {
                glm::vec4( start.x, water_height + 0.05f, start.z, Random( SplashRandomEngine, 0.6f, 1.2f ) ),
                glm::vec4( spray, Random( SplashRandomEngine, 0.05f, 0.12f ) )
            } );
        }
    }

    void FloatingObjects::AddSurfaceEffects() {
        uint32_t count = 0;
        for( auto & body : Bodies ) {
            std::vector<glm::vec3> local_centers;
            float radius;
            GetCollisionSpheres( body.HalfExtents, local_centers, radius );
            glm::mat3 rotation = glm::mat3_cast( body.Orientation );
            // Vertical half size of the rotated box
            float half_height = glm::dot( glm::abs( glm::vec3( rotation[0][1], rotation[1][1], rotation[2][1] ) ), body.HalfExtents );

            // Position (x, z) and depth of every sphere
            std::vector<glm::vec3> state;
            std::vector<float> water_heights;
            for( auto & local_center : local_centers ) {
                glm::vec3 center = body.Position + rotation * local_center;
                float water_height = SampleOcean( Waves, glm::vec2( center.x, center.z ), SimulationTime ).Height;
                float bottom = center.y - half_height;
                float top = center.y + half_height;
                // How deep the body reaches into the water here. Fades out once it is fully under water - a body
                // deep below doesn't move the surface.
                float depth = glm::clamp( water_height - bottom, 0.0f, 2.0f * half_height ) *
                              glm::clamp( 1.0f - (water_height - top) / half_height, 0.0f, 1.0f );
                state.push_back( glm::vec3( center.x, center.z, depth ) );
                water_heights.push_back( water_height );
            }

            // A new body starts without previous states - it is treated as if it had been there all along
            if( body.PreviousRippleState.size() != state.size() ) {
                body.PreviousRippleState = state;
                body.EarlierRippleState = state;
            }

            for( size_t i = 0; i < state.size(); ++i ) {
                glm::vec3 now = state[i];
                glm::vec3 previous = body.PreviousRippleState[i];
                glm::vec3 earlier = body.EarlierRippleState[i];
                // Nothing to add when the body stood still for the last two steps (the second difference is 0)
                bool changed = (glm::length( now - previous ) > 1e-4f) || (glm::length( previous - earlier ) > 1e-4f);
                bool touches_water = (now.z > 0.0f) || (previous.z > 0.0f) || (earlier.z > 0.0f);
                if( changed && touches_water && (count < MAX_RIPPLE_SOURCES_PER_STEP) ) {
                    PendingRippleSources.push_back( {
                        glm::vec4( now.x, now.y, radius, now.z ),
                        glm::vec4( previous, 0.0f ),
                        glm::vec4( earlier, 0.0f )
                    } );
                    ++count;
                }

                // Splash: this part pushes into the water fast (sinking boxes are already under water)
                float impact_speed = (now.z - previous.z) / TIME_STEP;
                if( (impact_speed > SplashSpeed) && !body.Sinking ) {
                    AddImpactSplash( glm::vec2( now.x, now.y ), water_heights[i], radius, impact_speed );
                }
            }
            if( body.IsBoat ) {
                AddBowSpray( body );
            }
            body.EarlierRippleState = body.PreviousRippleState;
            body.PreviousRippleState = state;
        }
        PendingStepSourceCounts.push_back( count );
    }

    void FloatingObjects::ResolveCollisions() {
        // Spheres in world space for every body; sinking boxes don't collide (they fall away under the others)
        struct Spheres { std::vector<glm::vec3> Centers; float Radius; };
        std::vector<Spheres> spheres( Bodies.size() );
        for( size_t i = 0; i < Bodies.size(); ++i ) {
            std::vector<glm::vec3> local_centers;
            GetCollisionSpheres( Bodies[i].HalfExtents, local_centers, spheres[i].Radius );
            glm::mat3 rotation = glm::mat3_cast( Bodies[i].Orientation );
            for( auto & center : local_centers ) {
                spheres[i].Centers.push_back( Bodies[i].Position + rotation * center );
            }
        }

        // All pairs - a few dozen bodies, so this is cheap. Only linear velocities are changed (no spin from collisions).
        for( size_t i = 0; i < Bodies.size(); ++i ) {
            for( size_t j = i + 1; j < Bodies.size(); ++j ) {
                Body & a = Bodies[i];
                Body & b = Bodies[j];
                if( a.Sinking || b.Sinking ) {
                    continue;
                }
                float inverse_mass_a = 1.0f / a.Mass;
                float inverse_mass_b = 1.0f / b.Mass;
                float min_distance = spheres[i].Radius + spheres[j].Radius;
                for( auto & center_a : spheres[i].Centers ) {
                    for( auto & center_b : spheres[j].Centers ) {
                        glm::vec3 offset = center_b - center_a;
                        float distance = glm::length( offset );
                        if( (distance >= min_distance) || (distance < 1e-4f) ) {
                            continue;
                        }
                        glm::vec3 normal = offset / distance;

                        // Push apart, the lighter body more
                        float penetration = min_distance - distance;
                        float total_inverse_mass = inverse_mass_a + inverse_mass_b;
                        a.Position -= normal * penetration * inverse_mass_a / total_inverse_mass;
                        b.Position += normal * penetration * inverse_mass_b / total_inverse_mass;

                        // Bounce if they move towards each other
                        float approach_speed = glm::dot( b.Velocity - a.Velocity, normal );
                        if( approach_speed < 0.0f ) {
                            float impulse = -(1.0f + CollisionRestitution) * approach_speed / total_inverse_mass;
                            a.Velocity -= impulse * inverse_mass_a * normal;
                            b.Velocity += impulse * inverse_mass_b * normal;
                        }
                    }
                }
            }
        }
    }

} // namespace VulkanCookbook
