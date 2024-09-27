
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
 

#include <OGF/Interactions/tools/mesh_grob_transform_interactions2_tool.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/scene_graph/types/scene_graph_shader_manager.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/gom/types/connection.h>

#include <OGF/Interactions/tools/helpers.h>


#include <chrono>
#include <thread>

namespace OGF {


   index_t cell_edge_to_cell_facet_edge(const MeshGrob *mesh_grob, index_t c, index_t lf, index_t le) {

      index_t v0 = mesh_grob->cells.edge_vertex(c, le, 0);
      index_t v1 = mesh_grob->cells.edge_vertex(c, le, 1);

      auto nb_verts = mesh_grob->cells.facet_nb_vertices(c, lf);

      for (index_t lv = 0; lv < nb_verts; lv++) {
         auto fv0 = mesh_grob->cells.facet_vertex(c, lf, lv);
         auto fv1 = mesh_grob->cells.facet_vertex(c, lf, (lv + 1) % nb_verts);

         if ((v0 == fv0 && v1 == fv1) || (v0 == fv1 && v1 == fv0)) {
            return lv;
         }
      }


      return NO_EDGE;
   }

    index_t MeshGrobTransformInteractions2Tool::pickup_edge(vec3 p0, index_t c_idx) {
         std::cout << "search for nearest edge" << std::endl;
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

    index_t MeshGrobTransformInteractions2Tool::pickup_facet_edge(vec3 p0, vec3 p01, index_t f_idx) {
        // Search nearest edge
        double min_dist = std::numeric_limits<double>().max();
        index_t e_idx = -1;

        int n_corners = mesh_grob()->facets.nb_corners(f_idx);


      //   std::cout << "search for nearest edge from facet: " << f_idx  << std::endl;
      //   std::cout << "nb corners: " << n_corners << std::endl;
         
        // Search nearest edge
        for (int i = 0; i < n_corners; i++) {
            // Get points from current edge
            index_t v0_idx = mesh_grob()->facet_corners.vertex(i + f_idx * n_corners);
            index_t v1_idx = mesh_grob()->facet_corners.vertex(((i + 1) % n_corners) +  f_idx * n_corners);
            

            // std::cout << "points: " << v0_idx << "," << v1_idx << std::endl;

            // index_t v0_idx = mesh_grob()->facets.edge_vertex(f_idx, i, 0);
            // index_t v1_idx = mesh_grob()->facets.edge_vertex(f_idx, i, 1);
            vec3 &p1 = mesh_grob()->vertices.point(v0_idx);
            vec3 &p2 = mesh_grob()->vertices.point(v1_idx);
            vec3 p_mid = (p1 + p2) * .5; 

            vec3 n0 = normalize(p0 - p01);
            vec3 n1 = normalize(p2 - p1);
            double angle = 1. - abs(dot(n0, n1));

            // Compute dist from mouse point to edge points
            // double dist = length(cross(p0 - p1, p0 - p2)) / length(p2 - p1);
            double dist = distance(p0, p_mid);
            // std::cout << "dist: " << dist << std::endl;
            // Keep min dist
            if (dist < min_dist /*&& dist_mid < 0.005*/ && angle < 0.1) {
                min_dist = dist;
                e_idx = i;
            std::cout << "angle: " << angle << std::endl;

            }
        }

      //   std::cout << "Found nearest edge: " << e_idx << std::endl;

        return e_idx;
    }

    std::tuple<index_t, index_t> MeshGrobTransformInteractions2Tool::pickup_facet(vec3 p0, index_t c_idx) {
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

    MeshGrobTransformInteractions2Tool::MeshGrobTransformInteractions2Tool(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {

      auto key_down_listener = new SlotConnection(scene_graph()->get_render_area(), "key_down", this, "key_down");
      auto key_up_listener = new SlotConnection(scene_graph()->get_render_area(), "key_up", this, "key_up");
      auto mouse_down_listener = new SlotConnection(scene_graph()->get_render_area(), "mouse_down", this, "mouse_down");
      auto mouse_up_listener = new SlotConnection(scene_graph()->get_render_area(), "mouse_up", this, "mouse_up");
      auto mouse_move_listener = new SlotConnection(scene_graph()->get_render_area(), "mouse_move", this, "mouse_move");

      // Listen tools change
      auto interpreter = Interpreter::instance_by_language("Lua");
      auto tools_manager = (ToolsManager*)interpreter->resolve_object("tools");
      auto tool_icon_changed = new SlotConnection(tools_manager, "tool_icon_changed", this, "tool_icon_changed");

      current_interactor = "l";

    }

    MeshGrobTransformInteractions2Tool::~MeshGrobTransformInteractions2Tool() { 
    }

   void MeshGrobTransformInteractions2Tool::tool_icon_changed(const std::string& value) {
      // std::cout << "tool changed !" << value << std::endl;

      // Only trigger events when this tool is selected
      bool will_be_current_tool = value == "tools/../../../plugins/OGF/Interactions/transform2_interactions";

      // Change tool from this to another !
      if (is_current_tool && !will_be_current_tool) {
         
      }
      // Change tool from another to this !
      if (will_be_current_tool) {
         
      }

      is_current_tool = will_be_current_tool;
      std::cout << "interaction 2 is the current tool ? " << is_current_tool << std::endl;
   }

   void MeshGrobTransformInteractions2Tool::key_down(const std::string& value) {
      if (!is_current_tool)
         return;

      //
      if (value == "x") {
         hold_key = value;
      }

      // Select interactor
      if (value == "escape") {
         current_interactor = "";
      }
      else if (value == "t") {
         current_interactor = "t";
         std::cout << "select translate interactor" << std::endl;
      }
      else if (value == "l") {
         current_interactor = "l";
         std::cout << "select loop cut interactor" << std::endl;
      }

      // Set init state of selected interactor
      if (value == "t") {
         auto selected_verts = get_selected_verts();
         for (auto selected_vert : selected_verts) {
            latest_pos[selected_vert] = mesh_grob()->vertices.point(selected_vert);
         }
      }


      paint_overlay(mouse_pos);
      // std::cout << "key down: " << value << std::endl;
   }

   void MeshGrobTransformInteractions2Tool::key_up(const std::string& value) {
      if (!is_current_tool)
         return;

      std::cout << "key up: "<< value << std::endl;

      if (hold_key == "x" && value == "x") {
         std::cout << "up x ?" << std::endl;
         hold_key = "";
      }

      paint_overlay(mouse_pos);



      // std::cout << "key up: " << value << std::endl;
   }

   void MeshGrobTransformInteractions2Tool::mouse_down(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, int button, bool control, bool shift) {
      if (!is_current_tool)
         return;

      is_mousedown = true;

      mouse_pos = ndc_to_dc(point_ndc);

      ref_pos = mouse_pos;


      paint_overlay(mouse_pos);
      std::cout << "mouse down from interaction 2 !" << std::endl;
   }

   void MeshGrobTransformInteractions2Tool::mouse_move(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, const vec2& delta_ndc, double delta_x_ndc, double delta_y_ndc, const vec2& delta_wc, int button, bool control, bool shift) {
      if (!is_current_tool)
         return;
      
      mouse_pos = ndc_to_dc(point_ndc);

      paint_overlay(mouse_pos);
      std::cout << "mouse move from interaction 2 ! " << mouse_pos << std::endl;
   }

   void MeshGrobTransformInteractions2Tool::mouse_up(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, int button, bool control, bool shift) {
      if (!is_current_tool)
         return;
      
      is_mousedown = false;
      mouse_pos = ndc_to_dc(point_ndc);

      if (current_interactor == "t") {
         latest_pos.clear();
         auto selected_verts = get_selected_verts();
         for (auto selected_vert : selected_verts) {
            latest_pos[selected_vert] = mesh_grob()->vertices.point(selected_vert);
         }
      }

      paint_overlay(mouse_pos);
      std::cout << "mouse up from interaction 2 ! " << std::endl;
   }

   std::vector<index_t> MeshGrobTransformInteractions2Tool::get_selected_verts() {
        // Get vertices.selection attribute
        Attribute<Numeric::uint8> vertices_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );

        std::vector<index_t> indexes;

        for (index_t i = 0; i < vertices_selection.size(); i++)
            if (vertices_selection[i])
               indexes.push_back(i);

         return indexes;
   }

   void MeshGrobTransformInteractions2Tool::paint_overlay(vec2 pdc) {
      rendering_context()->overlay().clear();
      
      // Tranform
      if (current_interactor == "t" && is_mousedown) {
         std::cout << "hold key: " << hold_key << std::endl;
         if (hold_key == "") {
            rendering_context()->overlay().segment(
               ref_pos, ref_pos + vec2(10, 0), Color(0.6, 0.6, 0.8, 1.0), 1.
            );
            rendering_context()->overlay().segment(
               ref_pos, ref_pos - vec2(10, 0), Color(0.6, 0.6, 0.8, 1.0), 1.
            );
            rendering_context()->overlay().segment(
               ref_pos, ref_pos + vec2(0, 10), Color(0.6, 0.6, 0.8, 1.0), 1.
            );
            rendering_context()->overlay().segment(
               ref_pos, ref_pos - vec2(0, 10), Color(0.6, 0.6, 0.8, 1.0), 1.
            );
            rendering_context()->overlay().segment(
               ref_pos, mouse_pos, Color(0.6, 0.6, 0.6, 1.0), 1.
            );
         }
         else if (hold_key == "x") {
            rendering_context()->overlay().segment(
               vec2(0, ref_pos.y), vec2(10000, ref_pos.y), Color(0.6, 0.6, 0.6, 1.0), 1.
            );
            rendering_context()->overlay().segment(
               vec2(ref_pos.x, ref_pos.y - 10), vec2(ref_pos.x, ref_pos.y + 10), Color(0.6, 0.6, 0.8, 1.0), 1.
            );
            // rendering_context()->overlay().segment(
            //    ref_pos, mouse_pos, Color(0.6, 0.6, 0.6, 1.0), 1.
            // );
         }

         // Do action !
         auto l = (ref_pos - mouse_pos).length();
         auto selected_verts = get_selected_verts();
         for (index_t selected_vert : selected_verts) {
            mesh_grob()->vertices.point(selected_vert) = latest_pos[selected_vert] + l * vec3(1,0,0) * 0.001;
         }
      }

      if (current_interactor == "l") {

         if (selected_edge != -1 && selected_cell != -1) {

            // auto lfe = cell_edge_to_cell_facet_edge(mesh_grob(), selected_cell, selected_local_facet, selected_edge);
            // std::cout << selected_cell << selected_local_facet << "," << selected_edge << "," << lfe << std::endl;
            index_t v0 = mesh_grob()->cells.edge_vertex(selected_cell, selected_edge, 0);
            index_t v1 = mesh_grob()->cells.edge_vertex(selected_cell, selected_edge, 1);
            // std::cout << "v0:" << v0 << ",v1:" << v1 << std::endl;
            auto p0 = mesh_grob()->vertices.point(v0);
            auto p1 = mesh_grob()->vertices.point(v1);

            auto p02d = project_point(p0);
            auto p12d = project_point(p1);
            rendering_context()->overlay().segment(
               p02d, p12d, Color(0.8, 0.4, 0.2, 1.0), 4.
            );
         }

         if (selected_facet_edges.size() > 0) {
            
            for (auto fe : selected_facet_edges) {
               auto f = std::get<0>(fe);
               auto e = std::get<1>(fe);
               int n_corners = mesh_grob()->facets.nb_corners(f);
               index_t v0 = mesh_grob()->facet_corners.vertex(e + f * n_corners);
               index_t v1 = mesh_grob()->facet_corners.vertex(((e + 1) % n_corners) + f * n_corners);

               // std::cout << "v0:" << v0 << ",v1:" << v1 << std::endl;
               auto p0 = mesh_grob()->vertices.point(v0);
               auto p1 = mesh_grob()->vertices.point(v1);

               auto p02d = project_point(p0);
               auto p12d = project_point(p1);
               rendering_context()->overlay().segment(
                  p02d, p12d, Color(0.8, 0.4, 0.2, 1.0), 4.
               );
            }
         }
      }



      mesh_grob()->update();
      
   }

    void MeshGrobTransformInteractions2Tool::grab(const RayPick& p_ndc) {



      MeshGrobTool::grab(p_ndc);
    }

    void MeshGrobTransformInteractions2Tool::drag(const RayPick& p_ndc) {
       // Some tools may also do special actions
       // behavior when the mouse button is dragged.
       // If not used, you may remove this function as
       // well as its declaration in the header.

      // Loop-cut, find nearest picked edge
      if (current_interactor == "l") {

         selected_cell = pick_cell(p_ndc);
         selected_facet = pick_facet(p_ndc);

         // selected_edge = pickup_edge(picked_point(), selected_cell);

         selected_edge = NO_EDGE;

         if (selected_facet != NO_FACET) {
            auto p_point = picked_point();
            selected_edge = pickup_facet_edge(p_point, latest_pickup_point, selected_facet);
            latest_pickup_point = p_point;
         }
         if (selected_edge != NO_EDGE)
            selected_facet_edges.push_back(std::tuple<index_t, index_t>(selected_facet, selected_edge));

         if (selected_cell != NO_CELL) {
            auto tf = pickup_facet(picked_point(), selected_cell);
            selected_facet = std::get<0>(tf); 
            selected_local_facet = std::get<1>(tf);
         }
      }

       MeshGrobTool::drag(p_ndc);
    }


    void MeshGrobTransformInteractions2Tool::release(const RayPick& p_ndc) {
       // Some tools may also do special actions
       // behavior when the mouse button is dragged.
       // If not used, you may remove this function as
       // well as its declaration in the header.
       MeshGrobTool::release(p_ndc);              
    }

}
