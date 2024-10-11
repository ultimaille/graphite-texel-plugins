
// /*
//  *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
//  *  Copyright (C) 2000-2015 INRIA - Project ALICE
//  *
//  *  This program is free software; you can redistribute it and/or modify
//  *  it under the terms of the GNU General Public License as published by
//  *  the Free Software Foundation; either version 2 of the License, or
//  *  (at your option) any later version.
//  *
//  *  This program is distributed in the hope that it will be useful,
//  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  *  GNU General Public License for more details.
//  *
//  *  You should have received a copy of the GNU General Public License
//  *  along with this program; if not, write to the Free Software
//  *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//  *
//  *  If you modify this software, you should include a notice giving the
//  *  name of the person performing the modification, the date of modification,
//  *  and the reason for such modification.
//  *
//  *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
//  *  Contact for this Plugin: me
//  *
//  *     Project ALICE
//  *     LORIA, INRIA Lorraine, 
//  *     Campus Scientifique, BP 239
//  *     54506 VANDOEUVRE LES NANCY CEDEX 
//  *     FRANCE
//  *
//  *  Note that the GNU General Public License does not permit incorporating
//  *  the Software into proprietary programs. 
//  *
//  * As an exception to the GPL, Graphite can be linked with the following
//  * (non-GPL) libraries:
//  *     Qt, tetgen, SuperLU, WildMagic and CGAL
//  */
 

// #include <OGF/Interactions/tools/mesh_grob_create_feature_tool.h>
// #include <OGF/renderer/context/rendering_context.h>
// #include <OGF/scene_graph/types/scene_graph_shader_manager.h>
// #include <OGF/mesh/shaders/mesh_grob_shader.h>
// #include <OGF/gom/types/connection.h>
// #include "helpers.h"

// #include <map>

// namespace OGF {

//     MeshGrobCreateFeatureTool::MeshGrobCreateFeatureTool(ToolsManager* parent) : MeshGrobTool(parent) {

//         PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();

//         // Create attribute
//         Attribute<Numeric::uint32> feature_point(
//             mesh_grob()->vertices.attributes(), "feature_point"
//         );

//         // Display vertices
//         auto vs = shader->get_vertices_style();
//         vs.visible = true;
//         shader->set_vertices_style(vs);

//         shader->set_painting(ATTRIBUTE);
//         shader->set_attribute("vertices.feature_point");
//         shader->autorange();

//         // Listen events
//         // auto key_down_listener = new SlotConnection(scene_graph()->get_render_area(), "key_down", this, "key_down");
//     }

//     MeshGrobCreateFeatureTool::~MeshGrobCreateFeatureTool() { 

//     }

//     void MeshGrobCreateFeatureTool::grab(const RayPick& p_ndc) {

//         auto v_idx = pick_vertex(p_ndc);

//         if (v_idx == NO_VERTEX)
//             return;

//         Attribute<Numeric::uint32> feature_point(
//             mesh_grob()->vertices.attributes(), "feature_point"
//         );



//         if (selected_vertices.size() == 2) {
//             feature_parts.push_back(std::tuple<index_t, index_t>(feature_point[0], feature_point[1]));
//         } else if (selected_vertices.size() > 2) {
//             // Search for nearest segment
//             auto p = mesh_grob()->vertices.point(v_idx);

//             double min_dist = std::numeric_limits<double>::max();
//             int min_i = -1;

//             for (int i = 0; i < feature_parts.size(); i++) {
//                 auto s =  feature_parts[i];
//                 auto a = mesh_grob()->vertices.point(std::get<0>(s));
//                 auto b = mesh_grob()->vertices.point(std::get<1>(s));
//                 // Compute distance
//                 double dist = ToolHelpers::segment_distance3(a, b, p);
//                 if (dist < min_dist) {
//                     min_dist = dist;
//                     min_i = i;
//                 }
//             }

//             // Split feature parts in two
//             // Resulting to remove current feature part
//             auto s = feature_parts[min_i];
//             feature_parts.erase(feature_parts.begin() + min_i);
//             // And add two new feature that connect extremities to inserted point
//             feature_parts.insert(feature_parts.begin() + min_i, std::tuple<index_t, index_t>(std::get<0>(s), v_idx));
//             feature_parts.insert(feature_parts.begin() + min_i + 1, std::tuple<index_t, index_t>(v_idx, std::get<1>(s)));

//         }

//         selected_vertices.push_back(v_idx);


//         for (int i = 0; i < feature_parts.size(); i++) {
//             feature_point[std::get<1>(feature_parts[i])] = i + 1;
//         }
//         // if (selected_vertices.size() < 2) {
//         //     feature_point[v_idx] = 1;
//         // } else {
//         //     feature_point[v_idx] = 2;
//         // }


//         PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();
//         shader->autorange();
//         // mesh_grob()->update();

//         MeshGrobTool::grab(p_ndc);       
//     }

//     void MeshGrobCreateFeatureTool::drag(const RayPick& p_ndc) {

//         MeshGrobTool::drag(p_ndc);
//     }

//     void MeshGrobCreateFeatureTool::release(const RayPick& p_ndc) {

//         MeshGrobTool::release(p_ndc);              
//     }

//     void MeshGrobCreateFeatureTool::key_down(const std::string& value) {
//         std::cout << "key down:" << value << std::endl;
//     }

//     std::vector<index_t> MeshGrobCreateFeatureTool::shortest_path(index_t source , index_t target) {

//         // For each vertice compute neighborhood
//         std::map<index_t, std::vector<index_t>> neighborhood;
//         for (auto f : mesh_grob()->facets) {
//             for (auto c : mesh_grob()->facets.corners(f)) {
//                 index_t v = mesh_grob()->facet_corners.vertex(c);
//                 index_t nc = mesh_grob()->facets.next_corner_around_facet(f, c);
//                 index_t pc = mesh_grob()->facets.prev_corner_around_facet(f, c);

//                 neighborhood[v].push_back(nc);
//                 neighborhood[v].push_back(pc);
//             }
//         }

//         std::vector<double> dist(mesh_grob()->vertices.nb());
//         std::vector<int> prev(mesh_grob()->vertices.nb(), -1);
//         std::vector<bool> visited(mesh_grob()->vertices.nb(), false);

//         std::vector<index_t> q;
//         for (auto v : mesh_grob()->vertices) {
//             q.push_back(v);
//         }

//         dist[source] = 0;
//         index_t v = source;

//         while (!q.empty()) {

//             // Loop through neighbors
//             for (auto nv : neighborhood[v]) {

//                 if (visited[nv])
//                     continue;
                
//                 double d = dist[v] + 1;
//                 if (d < dist[nv]) {
//                     dist[nv] = d;
//                     prev[nv] = v;
//                 }

//                 visited[nv] = true;
//             }

//             // Remove current v from queue
//             auto it = std::find(q.begin(), q.end(), v);
//             q.erase(it);

//             // Search min
//             v = *std::min_element(q.begin(), q.end(), [&dist](int a, int b) {
//                 return dist[a] < dist[b];
//             });

//         }

//         // Reconstruct path
//         std::vector<index_t> path;

//         while (v != -1) {
//             path.push_back(v);
//             v = prev[v];
//         }

//         return path;

//     }

// }
