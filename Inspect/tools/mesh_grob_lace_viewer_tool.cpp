
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

#include <limits>
#include <chrono>
#include <queue>

namespace OGF {

    MeshGrobLaceViewerTool::MeshGrobLaceViewerTool(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
    }

    MeshGrobLaceViewerTool::~MeshGrobLaceViewerTool() { 
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
                dist[adj_c] = Numeric::uint32(depth);
            }

        }
    }

    std::tuple<index_t, index_t> MeshGrobLaceViewerTool::pickup_facet(vec3 p0, index_t c_idx) {
        // Search if point is on facet
        double min_dist = std::numeric_limits<double>().max();
        index_t f_idx = -1;
        index_t lf_idx = -1;

        std::cout << "p0: " << p0 << std::endl;

        std::cout << mesh_grob()->cells.nb_facets(c_idx) << std::endl;

        Attribute<Numeric::uint8> visited(
            mesh_grob()->cells.attributes(), "facetus"
        );

        std::cout << "facet attribute: " << visited.size() << std::endl;

        // Search nearest edge
        for (int lf = 0; lf < mesh_grob()->cells.nb_facets(c_idx); lf++) {
            
            index_t fi = mesh_grob()->cells.facet(c_idx, lf);
            
            std::cout << "found global fi: " << fi << std::endl;

            visited[mesh_grob()->facets.adjacent(fi, 0)] = OGF::Numeric::uint8(1);

            // std::cout << "nb corners: " << mesh_grob()->facets.nb_corners(fi) << std::endl;
            // vec3 g;
            
            // for (int lc = 0; lc < mesh_grob()->facets.nb_corners(fi); lc++) {
            //     index_t v_idx = mesh_grob()->facets.corner(fi, lc);
            //     std::cout << v_idx << std::endl;
            //     g += mesh_grob()->vertices.point(v_idx);
            // }
            // g /= mesh_grob()->facets.nb_corners(fi);

            // // Compute dist from mouse point to bary
            // double dist = g.distance2(p0);
            // std::cout << "dist: " << dist << ", for: " << lf << std::endl;
            
            // // Keep min dist
            // if (dist < min_dist) {
            //     min_dist = dist;
            //     lf_idx = lf;
            //     f_idx = fi;
            // }
        }

        std::cout << "Found nearest facet: " << f_idx << std::endl;

        return std::tuple<index_t, index_t>(f_idx, lf_idx);
    }

    // Propagate an hex lace following normal of a given facet (Breadth-First-Search)
    void MeshGrobLaceViewerTool::bfs_cell_propagate2(const OGF::MeshGrob *mesh_grob, index_t c, index_t lf, int max_depth, std::function<void(index_t, int)> f) {

        Attribute<Numeric::uint8> visited(
            mesh_grob->cells.attributes(), "filter"
        );
        

        std::queue<int> q;
        std::queue<int> d;
        q.push(c);
        d.push(0);
        visited[c] = true;


        while (!q.empty()) {

            int depth = d.front();

            if (depth >= max_depth)
                break;

            int c = q.front();
            d.pop();        
            
            q.pop();
            f(c, depth);

            
            index_t adj_c = mesh_grob->cells.adjacent(c, lf);
            // // Next cell
            // index_t nc = mesh_grob->facets.next_corner_around_facet(fi, 0);
            // index_t nf = mesh_grob->facet_corners.adjacent_facet(nc);
            // nc = mesh_grob->facets.next_corner_around_facet(nf, 0);
            // nf = mesh_grob->facet_corners.adjacent_facet(nc);
            // index_t adj_c = mesh_grob->cell_facets.adjacent_cell(nf);

            if (adj_c == NO_CELL)
                continue;



            q.push(adj_c);
            d.push(depth + 1);
            visited[adj_c] = Numeric::uint8(1);

        }
    }

    void MeshGrobLaceViewerTool::grab(const RayPick& p_ndc) {
        index_t c_idx = NO_CELL;

        // Get shader
        PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();

        mesh_grob()->cells.connect();

        if (p_ndc.button == MOUSE_BUTTON_RIGHT) {
            shader->set_cells_filter(!shader->get_cells_filter());
            

            if (shader->get_cells_filter()) {
                shader->set_shrink(shrink_value);
            }
            else 
                shader->set_shrink(0);

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
            auto f_idx = pick_facet(p_ndc);



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


        // Put an attribute on cells 
        Attribute<Numeric::uint8> cell_filter(
            mesh_grob()->cells.attributes(), "filter"
        );

        // Reset filter attribute
        for (int i = 0; i < cell_filter.size(); i++)
            cell_filter[i] = false;



        // Set selection on picked cell
        cell_selection[c_idx] = true;

        index_t n_facets = mesh_grob()->cells.nb_facets(c_idx);

        bfs_cell_propagate(mesh_grob(), c_idx, get_value(), [](index_t _, int b) {

        });

        Attribute<Numeric::uint32> dist(
            mesh_grob()->cells.attributes(), "distance"
        );

        for (int i = 0; i < mesh_grob()->cells.nb(); i++) {
            if (dist[i] <= get_value())
                cell_filter[i] = true;
        }

        // Paint layer attribute
        if (p_ndc.button == MOUSE_BUTTON_LEFT) {
            shader->set_painting(ATTRIBUTE);
            shader->set_attribute("cells.distance");
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
