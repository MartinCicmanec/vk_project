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
// Model

// The function bodies of tinyobjloader are compiled only in this file
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>
#include "Model.h"

namespace VulkanCookbook {

    bool LoadModelFromObjFile( std::string const     & filename,
                               std::vector<Vertex>   & vertices,
                               std::vector<uint32_t> & indices ) {
        tinyobj::attrib_t                attributes;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warnings;
        std::string                      errors;

        // Faces with more than 3 vertices are triangulated by default
        bool loaded = tinyobj::LoadObj( &attributes, &shapes, &materials, &warnings, &errors, filename.c_str() );
        if( !warnings.empty() ) {
            std::cout << "Warning while loading '" << filename << "': " << warnings << std::endl;
        }
        if( !loaded ) {
            std::cout << "Could not load model '" << filename << "': " << errors << std::endl;
            return false;
        }

        vertices.clear();
        indices.clear();

        // Index of every unique vertex that was already added
        std::unordered_map<Vertex, uint32_t> unique_vertices;

        for( auto & shape : shapes ) {
            for( auto & index : shape.mesh.indices ) {
                Vertex vertex = {};

                vertex.Position = {
                    attributes.vertices[3 * index.vertex_index + 0],
                    attributes.vertices[3 * index.vertex_index + 1],
                    attributes.vertices[3 * index.vertex_index + 2]
                };

                // Negative when the file has no texture coordinates for this vertex
                if( index.texcoord_index >= 0 ) {
                    vertex.TexCoord = {
                        attributes.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                vertex.Color = { 1.0f, 1.0f, 1.0f };

                auto found = unique_vertices.find( vertex );
                if( found == unique_vertices.end() ) {
                    found = unique_vertices.emplace( vertex, static_cast<uint32_t>(vertices.size()) ).first;
                    vertices.push_back( vertex );
                }
                indices.push_back( found->second );
            }
        }

        if( indices.empty() ) {
            std::cout << "Model '" << filename << "' contains no geometry." << std::endl;
            return false;
        }
        return true;
    }

} // namespace VulkanCookbook
