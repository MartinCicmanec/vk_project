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
// Chapter: 01 Instance and Devices
// Recipe:  05 Loading function exported from a Vulkan Loader library

// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers

#include <cstring>
#include <iostream>

#include "Application.h"


int main(int argc, char * argv[]) {

    bool enable_verbose = false;
    bool debug_callbacks = false;
    bool show_particles = false;
    bool show_water = false;
    for ( int i = 0; i < argc; i = i + 1 ){
        if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0)) {
            enable_verbose = true;
            std::cout << "Enabled verbose" << std::endl;
        }
        if ((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--debug") == 0)) {
            debug_callbacks = true;
            std::cout << "Enabled debug callbacks" << std::endl;
        }
        if ((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--particles") == 0)) {
            show_particles = true;
            std::cout << "Showing the compute shader particles" << std::endl;
        }
        if ((strcmp(argv[i], "-w") == 0) || (strcmp(argv[i], "--water") == 0)) {
            show_water = true;
            std::cout << "Showing the water" << std::endl;
        }
    }

    // Only one scene can be shown
    if (show_particles && show_water) {
        std::cout << "-p and -w can't be used together." << std::endl;
        return 1;
    }
    VulkanCookbook::DemoScene scene = VulkanCookbook::DemoScene::Model;
    if (show_particles) {
        scene = VulkanCookbook::DemoScene::Particles;
    } else if (show_water) {
        scene = VulkanCookbook::DemoScene::Water;
    }

    VulkanCookbook::Application application( enable_verbose, debug_callbacks, scene );
    if( !application.Init() || !application.Run() ) {
        return 1;
    }
    return 0;
}
