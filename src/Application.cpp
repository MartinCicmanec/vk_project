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
// Application

#include "Application.h"

namespace VulkanCookbook {

    Application::Application( bool        enable_validation,
                              bool        enable_debug_callbacks,
                              DemoScene   scene ) :
        EnableValidation( enable_validation ),
        EnableDebugCallbacks( enable_debug_callbacks ),
        Scene( scene ) {
    }

    Application::~Application() {
        // Runs before the members are destroyed: nothing may be destroyed while the GPU is still using it
        Context.WaitIdle();
    }

    bool Application::Init() {
        // Every part needs the ones before it
        if( !MainWindow.Init( 800, 600, "Vulkan" ) ||
            !Context.Init( MainWindow, EnableValidation, EnableDebugCallbacks, MAX_MSAA_SAMPLES ) ||
            !Targets.Init( Context, MainWindow ) ) {
            return false;
        }
        switch( Scene ) {
            case DemoScene::Particles:  return Particles.Init( Context, Targets, MainWindow );
            case DemoScene::Water:      return Water.Init( Context, Targets, MainWindow );
            case DemoScene::Model:      break;
        }
        return SceneRenderer.Init( Context, Targets, MainWindow );
    }

    bool Application::Run() {
        while( !MainWindow.ShouldClose() ) {
            MainWindow.PollEvents();
            bool frame_drawn = false;
            switch( Scene ) {
                case DemoScene::Model:      frame_drawn = SceneRenderer.DrawFrame(); break;
                case DemoScene::Particles:  frame_drawn = Particles.DrawFrame(); break;
                case DemoScene::Water:      frame_drawn = Water.DrawFrame(); break;
            }
            if( !frame_drawn ) {
                return false;
            }
        }
        return true;
    }

} // namespace VulkanCookbook
