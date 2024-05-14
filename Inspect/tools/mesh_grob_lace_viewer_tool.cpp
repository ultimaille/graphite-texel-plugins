
/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: me
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */
 

#include <OGF/Inspect/tools/mesh_grob_lace_viewer_tool.h>
#include <OGF/scene_graph/types/scene_graph_shader_manager.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include "ConnectivityHelper.h"

#include <limits>
#include <chrono>
#include <queue>

namespace OGF {

    index_t cell_edge_to_cell_facet_edge(const MeshGrob *mesh_grob, index_t c, index_t lf, index_t le) {

        index_t v0 = mesh_grob->cells.edge_vertex(c, le, 0);
        index_t v1 = mesh_grob->cells.edge_vertex(c, le, 1);
        std::cout << "---" << std::endl;
        std::cout << "---" << std::endl;
        std::cout << "---" << std::endl;


        std::cout << "cell:" << c << ", f:" << lf << std::endl;
        // std::cout << v0 << "->" << v1 << std::endl;

        // for (index_t fi = 0; fi < mesh_grob->cells.nb_facets(c); fi++) {

            auto nb_verts = mesh_grob->cells.facet_nb_vertices(c, lf);

            for (index_t lv = 0; lv < nb_verts; lv++) {
                auto fv0 = mesh_grob->cells.facet_vertex(c, lf, lv);
                auto fv1 = mesh_grob->cells.facet_vertex(c, lf, (lv + 1) % nb_verts);
                std::cout << fv0 << "->" << fv1 << std::endl;

                if ((v0 == fv0 && v1 == fv1) || (v0 == fv1 && v1 == fv0)) {
                    std::cout << "le: " << le << "," << v0 << "->" << v1 << ", lv:" << lv << std::endl;
                    return lv;
                }
            }

            // for (index_t cur_e = 0; cur_e < mesh_grob->cells.facet_nb_vertices(c, fi); cur_e++) {

            // }

            // for (index_t cur_e = 0; cur_e < mesh_grob->cells.facet_nb_vertices(c, fi); cur_e++) {
                
            //     if (mesh_grob->cells.facet_vertex(c, fi, cur_e) == v && fi == lf) {
            //         return cur_e;
            //     }
            // }
        // }

        return NO_EDGE;
    }

    index_t cell_facet_edge_to_cell_edge(const MeshGrob *mesh_grob, index_t c, index_t lf, index_t e) {

        auto nb_verts = mesh_grob->cells.facet_nb_vertices(c, lf);

        index_t v0 = mesh_grob->cells.facet_vertex(c, lf, e);
        index_t v1 = mesh_grob->cells.facet_vertex(c, lf, (e + 1) % nb_verts);

        for (int cur_e = 0; cur_e < 12; cur_e++) {
            auto cur_v0 = mesh_grob->cells.edge_vertex(c, cur_e, 0);
            auto cur_v1 = mesh_grob->cells.edge_vertex(c, cur_e, 1);
            if ((v0 == cur_v0 && v1 == cur_v1) || (v0 == cur_v1 && v1 == cur_v0))
                return cur_e;
        }

        return NO_EDGE;
    }

    MeshGrobLaceViewerTool::MeshGrobLaceViewerTool(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {

    }

    MeshGrobLaceViewerTool::~MeshGrobLaceViewerTool() { 
    }

    index_t MeshGrobLaceViewerTool::pickup_edge(vec3 p0, index_t c_idx) {
        // Search nearest edge
        double min_dist = std::numeric_limits<double>().max();
        index_t e_idx = -1;

        // Search nearest edge
        for (int i = 0; i < mesh_grob()->cells.nb_edges(c_idx); i++) {
            // Get points from current edge
            index_t v0_idx = mesh_grob()->cells.edge_vertex(c_idx, i, 0);
            index_t v1_idx = mesh_grob()->cells.edge_vertex(c_idx, i, 1);
            vec3 &p1 = mesh_grob()->vertices.point(v0_idx);
            vec3 &p2 = mesh_grob()->vertices.point(v1_idx);

            // Compute dist from mouse point to edge points
            double dist = length(cross(p0 - p1, p0 - p2)) / length(p2 - p1);
            // std::cout << "dist: " << dist << std::endl;
            // Keep min dist
            if (dist < min_dist) {
                min_dist = dist;
                e_idx = i;
            }
        }

        std::cout << "Found nearest edge: " << e_idx << std::endl;

        return e_idx;
    }

    std::tuple<index_t, index_t> MeshGrobLaceViewerTool::pickup_facet(vec3 p0, index_t c_idx) {
        // Search if point is on facet
        double min_dist = std::numeric_limits<double>().max();
        index_t f_idx = NO_FACET;
        index_t lf_idx = NO_FACET;

        // std::cout << "p0: " << p0 << std::endl;
        // std::cout << "c_idx: " << c_idx << std::endl;

        // std::cout << "n facet: " << mesh_grob()->cells.nb_facets(c_idx) << std::endl;
        // std::cout << "n verts: " << mesh_grob()->cells.nb_vertices(c_idx) << std::endl;

        for (index_t lf = 0; lf < mesh_grob()->cells.nb_facets(c_idx); lf++) {
            // std::cout << "n verts in facet: " << mesh_grob()->cells.facet_nb_vertices(c_idx, lf) << std::endl;

            auto a = mesh_grob()->vertices.point(mesh_grob()->cells.facet_vertex(c_idx, lf, 0));
            auto b = mesh_grob()->vertices.point(mesh_grob()->cells.facet_vertex(c_idx, lf, 1));
            auto c = mesh_grob()->vertices.point(mesh_grob()->cells.facet_vertex(c_idx, lf, 2));
            auto d = mesh_grob()->vertices.point(mesh_grob()->cells.facet_vertex(c_idx, lf, 3));

            auto n = normalize(cross(b - a, c - b));
            // double dist = dot(p0 - d, n);
            double dist = dot(p0 - a, n);
            // std::cout << "DIST:" << dot(p0 - d, n) << std::endl;
            
            if (dist < min_dist) {
                min_dist = dist;
                f_idx = mesh_grob()->cells.facet(c_idx, lf);
                lf_idx = lf;
            }
        }

        // std::cout << "Found nearest facet: " << f_idx << std::endl;
        // std::cout << "Found nearest facet: " << lf_idx << std::endl;

        return std::tuple<index_t, index_t>(f_idx, lf_idx);
    }

    // Propagate an hex layer perpendicular to a given halfedge (Breadth-First-Search)
    void MeshGrobLaceViewerTool::bfs_layer_propagate(const OGF::MeshGrob *mesh_grob, index_t c, index_t lf, index_t le, int max_depth, std::function<void(index_t, int)> f) {

        
        std::vector<bool> visited(mesh_grob->cells.nb());

        Attribute<Numeric::uint32> dist(
            mesh_grob->cells.attributes(), "distance"
        );

        HalfedgeHelper chw(*mesh_grob);

        std::queue<index_t> q;
        std::queue<int> d;

        auto lfe = cell_edge_to_cell_facet_edge(mesh_grob, c, lf, le);

        q.push(chw.id(c, lf, lfe));

        d.push(0);
        visited[c] = true;
        dist[c] = Numeric::uint32(0);

        while (!q.empty()) {

            int depth = d.front();

            HalfedgeHelper chw(*mesh_grob, q.front());
            d.pop(); 
            q.pop();

            f(c, depth);

            // TODO ugly code repeated, refactor !
            if (chw.opp_c().cell() != NO_CELL) {
                auto chw1 = chw.opp_c().opp_f().next().next().opp_f();
                auto opp_c1 = chw1.cell();

                if (!visited[opp_c1]) {
                    q.push(chw1.h);
                    d.push(depth + 1);
                    dist[opp_c1] = Numeric::uint32(depth+1);
                    visited[opp_c1] = true;
                }
            }

            if (chw.opp_f().opp_c().cell() != NO_CELL) {
                auto chw2 = chw.opp_f().opp_c().opp_f().next().next();
                auto opp_c2 = chw2.cell();

                if (!visited[opp_c2]) {
                    q.push(chw2.h);
                    d.push(depth + 1);
                    dist[opp_c2] = Numeric::uint32(depth+1);
                    visited[opp_c2] = true;
                }

            }

            if (chw.next().next().opp_f().opp_c().cell() != NO_CELL) {
                auto chw3 = chw.next().next().opp_f().opp_c().opp_f();
                auto opp_c3 = chw3.cell();

                if (!visited[opp_c3]) {
                    q.push(chw3.h);
                    d.push(depth + 1);
                    dist[opp_c3] = Numeric::uint32(depth+1);
                    visited[opp_c3] = true;
                }

            }

            if (chw.opp_f().next().next().opp_f().opp_c().cell() != NO_CELL) {
                auto chw4 = chw.opp_f().next().next().opp_f().opp_c();
                auto opp_c4 = chw4.cell();

                if (!visited[opp_c4]) {
                    q.push(chw4.h);
                    d.push(depth + 1);
                    dist[opp_c4] = Numeric::uint32(depth+1);
                    visited[opp_c4] = true;
                }

            }

        }

    }

    // Propagate an hex layer perpendicular to a given halfedge (Breadth-First-Search)
    void MeshGrobLaceViewerTool::bfs_lace_propagate(const OGF::MeshGrob *mesh_grob, index_t c, index_t lf, int max_depth, std::function<void(index_t, int)> f) {


        // std::vector<bool> visited(mesh_grob->cells.nb());

        Attribute<Numeric::uint8> visited(
            mesh_grob->cells.attributes(), "filter"
        );

        Attribute<Numeric::uint32> dist(
            mesh_grob->cells.attributes(), "distance"
        );

        std::queue<index_t> q;
        std::queue<int> d;
        q.push(c);
        d.push(0);
        visited[c] = true;
        dist[c] = Numeric::uint32(0);


        // index_t nlf = 2;
        index_t nlf = lf;

        while (!q.empty()) {

            int depth = d.front();

            index_t c = q.front();
            d.pop();        
            
            q.pop();
            f(c, depth);

            HalfedgeHelper chw(*mesh_grob);
            chw = chw.id(c, nlf, 0);
            chw = chw.opp_c();
            index_t adj_c = chw.cell();
            nlf = chw.cell_facet();
            std::cout << "nlf bef:" << nlf << std::endl;

            // Get opposite facet
            if (nlf % 2 == 0)
                nlf += 1;
            else 
                nlf -= 1;
            
            std::cout << "nlf aft:" << nlf << std::endl;
            
            if (adj_c == NO_CELL || visited[adj_c])
                continue;
            
            q.push(adj_c);
            d.push(depth + 1);
            visited[adj_c] = true;
            dist[adj_c] = Numeric::uint32(depth + 1);

        }

        q.push(c);
        d.push(0);

        // nlf = 3;
        nlf = lf % 2 == 0 ? lf + 1 : lf - 1;
        while (!q.empty()) {

            int depth = d.front();

            index_t c = q.front();
            d.pop();        
            
            q.pop();
            f(c, depth);

            HalfedgeHelper chw(*mesh_grob);
            chw = chw.id(c, nlf, 0);
            chw = chw.opp_c();
            index_t adj_c = chw.cell();
            nlf = chw.cell_facet();
            std::cout << "nlf bef:" << nlf << std::endl;

            // Get opposite facet
            if (nlf % 2 == 0)
                nlf += 1;
            else 
                nlf -= 1;
            
            std::cout << "nlf aft:" << nlf << std::endl;
            
            if (adj_c == NO_CELL || visited[adj_c])
                continue;
            
            q.push(adj_c);
            d.push(depth + 1);
            visited[adj_c] = true;
            dist[adj_c] = Numeric::uint32(depth + 1);

        }
    }

    // Propagate an hex layer perpendicular to a given halfedge (Breadth-First-Search)
    void MeshGrobLaceViewerTool::bfs_cell_propagate(const OGF::MeshGrob *mesh_grob, index_t c, int max_depth, std::function<void(index_t, int)> f) {


        std::vector<bool> visited(mesh_grob->cells.nb());

        Attribute<Numeric::uint32> dist(
            mesh_grob->cells.attributes(), "distance"
        );

        std::queue<int> q;
        std::queue<int> d;
        q.push(c);
        d.push(0);
        visited[c] = true;
        dist[c] = Numeric::uint32(0);


        while (!q.empty()) {

            int depth = d.front();

            int c = q.front();
            d.pop();        
            
            q.pop();
            f(c, depth);

            index_t n_facets = mesh_grob->cells.nb_facets(c);

            for (index_t lf = 0; lf < n_facets; lf++) {
                index_t adj_c = mesh_grob->cells.adjacent(c, lf);
                
                if (adj_c == NO_CELL || visited[adj_c])
                    continue;
                
                q.push(adj_c);
                d.push(depth + 1);
                visited[adj_c] = Numeric::uint8(1);
                dist[adj_c] = Numeric::uint32(depth + 1);
            }

        }
    }


    void MeshGrobLaceViewerTool::grab(const RayPick& p_ndc) {
        index_t c_idx = NO_CELL;

        // Get shader
        PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();

        mesh_grob()->cells.connect();
        mesh_grob()->facets.connect();
        

        if (p_ndc.button == MOUSE_BUTTON_RIGHT) {
            bool is_filter = !shader->get_cells_filter();
            shader->set_cells_filter(is_filter);
            shader->set_vertices_filter(is_filter);
            
            shader->set_shrink(shader->get_cells_filter() ? shrink_value : 0);
            shader->autorange();
        }

        if (p_ndc.button == MOUSE_BUTTON_WHEEL_DOWN && get_value() > 0) {
            set_value(get_value() - 1);
        }
        else if (p_ndc.button == MOUSE_BUTTON_WHEEL_UP && get_value() < n_ring_max_ - 1) {
            set_value(get_value() + 1);
        }

        // Get cells.selection attribute
        Attribute<Numeric::uint8> cell_selection(
            mesh_grob()->cells.attributes(), "selection"
        );

        // Get picked cell index
        if (p_ndc.button == MOUSE_BUTTON_LEFT) {

            c_idx = pick_cell(p_ndc);
            // auto f_idx = pick_facet(p_ndc);

            // Reset selection attribute
            for (int i = 0; i < cell_selection.size(); i++)
                cell_selection[i] = false;
        } else {

            // Search first selected cell
            for (int i = 0; i < cell_selection.size(); i++) {
                if (cell_selection[i])
                {
                    c_idx = i;
                    break;
                }
            }

        }
        
        // // Get 3D picked point on cell
        // vec3 p0 = picked_point();

        // No cell
        if (c_idx == NO_CELL)
            return;



        // Put a filter on cells / vertices
        Attribute<Numeric::uint8> cell_filter(
            mesh_grob()->cells.attributes(), "filter"
        );

        Attribute<Numeric::uint8> vertices_filter(
            mesh_grob()->vertices.attributes(), "filter"
        );

        Attribute<Numeric::uint32> dist(
            mesh_grob()->cells.attributes(), "distance"
        );

        // Reset filter attribute
        for (int i = 0; i < cell_filter.size(); i++)
            cell_filter[i] = false;

        for (int i = 0; i < vertices_filter.size(); i++)
            vertices_filter[i] = false;

        // Reset dist
        for (int i = 0; i < dist.size(); i++)
            dist[i] = std::numeric_limits<uint>().max();

        // Set selection on picked cell
        cell_selection[c_idx] = true;

        index_t n_facets = mesh_grob()->cells.nb_facets(c_idx);

        auto [ff, lff] = pickup_facet(picked_point(), c_idx);



        auto le = pickup_edge(picked_point(), c_idx);
       


        switch (extract_type_)
        {
        case LACE:
            bfs_lace_propagate(mesh_grob(), c_idx, lff, get_value(), [](index_t _, int b) {

            });

            break;
        case LAYER:
            bfs_layer_propagate(mesh_grob(), c_idx, lff, le, get_value(), [](index_t _, int b) {

            });
            break;
        case RING:
        {

            bfs_cell_propagate(mesh_grob(), c_idx, get_value(), [](index_t _, int b) {

            });

        }

            break;
        default:
            break;
        }




        for (int i = 0; i < mesh_grob()->cells.nb(); i++) {
            if (dist[i] <= get_value()) {
                cell_filter[i] = true;

                for (int lv = 0; lv < mesh_grob()->cells.nb_vertices(i); lv++) {
                    vertices_filter[mesh_grob()->cells.vertex(i, lv)] = true;
                }
            }
        }


        // Paint layer attribute
        if (p_ndc.button == MOUSE_BUTTON_LEFT) {
            shader->set_painting(ATTRIBUTE);
            shader->set_attribute("cells.selection");
            shader->autorange();
        }

        

        // Set grid
        auto es = shader->get_mesh_style();
        es.visible = true;
        es.width = 2;
        shader->set_mesh_style(es);

        MeshGrobTool::grab(p_ndc);       
    }


    void MeshGrobLaceViewerTool::drag(const RayPick& p_ndc) {
       MeshGrobTool::drag(p_ndc);

        if (p_ndc.button != MOUSE_BUTTON_MIDDLE)
            return;

        // Get shader
        PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();

        vec2 center(0,0);
        
        shrink_value = (int)(center.distance(p_ndc.p_ndc) * 10.);
        if (shader->get_cells_filter())
            shader->set_shrink(shrink_value);
        else 
            shader->set_shrink(0);

    }


    void MeshGrobLaceViewerTool::release(const RayPick& p_ndc) {
        // if (p_ndc.button == MOUSE_BUTTON_MIDDLE)
        //     lp = p_ndc.p_ndc;

        MeshGrobTool::release(p_ndc);              
    }

}
