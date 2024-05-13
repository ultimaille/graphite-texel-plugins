
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
 

#include <OGF/Interactions/tools/mesh_grob_transform_interactions_tool.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/scene_graph/types/scene_graph_shader_manager.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/gom/types/connection.h>

#include <OGF/Interactions/tools/helpers.h>


#include <chrono>
#include <thread>

namespace OGF {

    /**
     * \brief Tests whether a point belongs to a selection
     * \details The selection is a rectangle in device coordinates plus an
     *  optional mask image
     * \param[in] p the device coordinates of the point to be tested
     * \param[in] x0 , y0 , x1 , y1 the device coordinates rectangle 
     * \param[in] mask the optional mask image. Needs to be in GRAYSCALE,
     *  8 bits per pixel, with same dimensions as rectangle.
     */
    bool point_is_selected(
        const vec2& p,                  
        index_t x0, index_t y0, index_t x1, index_t y1,  
        Image* mask = nullptr
    ) {
        if(p.x < double(x0) || p.y < double(y0) ||
           p.x > double(x1) || p.y > double(y1)) {
            return false;
        }
        if(mask != nullptr) {
            if(*mask->pixel_base_byte_ptr(index_t(p.x)-x0,index_t(p.y)-y0)==0){
                return false;
            }
        }
        return true;
    }

    MeshGrobTransformInteractionsTool::MeshGrobTransformInteractionsTool(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {

         PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();

         selected_axis = vec3(0, 0, 0);

        // Display vertices
        auto vs = shader->get_vertices_style();
        vs.visible = true;
        shader->set_vertices_style(vs);

      //   auto interpreter = Interpreter::instance_by_language("Lua");
      //   auto render_area = interpreter->resolve_object("main.render_area", false);

        auto key_down_listener = new SlotConnection(scene_graph()->get_render_area(), "key_down", this, "key_down");
        auto key_up_listener = new SlotConnection(scene_graph()->get_render_area(), "key_up", this, "key_up");
        auto mouse_down_listener = new SlotConnection(scene_graph()->get_render_area(), "mouse_down", this, "mouse_down");
        auto mouse_up_listener = new SlotConnection(scene_graph()->get_render_area(), "mouse_up", this, "mouse_up");
        auto mouse_move_listener = new SlotConnection(scene_graph()->get_render_area(), "mouse_move", this, "mouse_move");
        
    }

    MeshGrobTransformInteractionsTool::~MeshGrobTransformInteractionsTool() { 

    }

   void MeshGrobTransformInteractionsTool::mouse_down(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, int button, bool control, bool shift) {
      std::cout << "button: " << button << std::endl;

      pressed_button = button;
      mouse_down_pos = ndc_to_dc(point_ndc);

      // Call grab, select multiple points !
      if (pressedKey == "shift" && button == MOUSE_BUTTON_LEFT)
         grab(RayPick(point_ndc, button));

   }

   void MeshGrobTransformInteractionsTool::mouse_up(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, int button, bool control, bool shift) {
      pressed_button = MOUSE_BUTTON_NONE;

      if (pressedKey == "control")
         release(RayPick(point_ndc, button));
   }

   void MeshGrobTransformInteractionsTool::mouse_move(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, const vec2& delta_ndc, double delta_x_ndc, double delta_y_ndc, const vec2& delta_wc, int button, bool control, bool shift) {
      // Update paint interactor on mouse move (when moving camera)
      // if (pressedKey == "control") {
      //    paint_transform_interactor();
      // }

      // // Draw rect for selection
      // if (pressedKey == "shift" && button == MOUSE_BUTTON_LEFT) {

      //    rendering_context->overlay().clear();
            
      //    auto pdc = ndc_to_dc(point_ndc);
      //    rendering_context->overlay().fillrect(
      //       mouse_down_pos, pdc, Color(.5, .5, .5, 0.3)
      //    );

         
      // }

      if (pressedKey == "control")
         paint_overlay(ndc_to_dc(point_ndc));
   }

   
   void MeshGrobTransformInteractionsTool::key_down(const std::string& value) {
      pressedKey = value;
      std::cout << "key down:" << value << std::endl;
   }

   void MeshGrobTransformInteractionsTool::key_up(const std::string& value) {
      pressedKey = "";
      std::cout << "key up: " << value << std::endl;
   }

   void MeshGrobTransformInteractionsTool::grab(const RayPick& p_ndc) {

      m_last_pos = p_ndc.p_ndc;
      

      auto axis = transform_interactor_axis();

      vec2 p_dc = ndc_to_dc(p_ndc.p_ndc);

      bool is_grab_interactor = false;

      // Get distance between drawing interactor segment and mouse 
      double distx = ToolHelpers::segment_distance(axis[0], axis[1], p_dc);
      double disty = ToolHelpers::segment_distance(axis[0], axis[2], p_dc);
      double distz = ToolHelpers::segment_distance(axis[0], axis[3], p_dc);

      // Get selected axis / plane
      if (!get_selected_verts().empty() && ToolHelpers::is_in_2D_convex_quad(get_x_quad(INTERACTOR_QUADS_SIZE), p_dc)) {
         selected_axis = vec3(0, 1, 1);
         is_grab_interactor = true;
         std::cout << "is in quad x" << std::endl;
      }
      else if (!get_selected_verts().empty() && ToolHelpers::is_in_2D_convex_quad(get_y_quad(INTERACTOR_QUADS_SIZE), p_dc)) {
         selected_axis = vec3(1, 0, 1);
         is_grab_interactor = true;
         std::cout << "is in quad y" << std::endl;

      }
      else if (!get_selected_verts().empty() && ToolHelpers::is_in_2D_convex_quad(get_z_quad(INTERACTOR_QUADS_SIZE), p_dc)) {
         selected_axis = vec3(1, 1, 0);
         is_grab_interactor = true;
         std::cout << "is in quad z" << std::endl;
      }
      // Select axis with minimal distance
      else if (distx < disty && distx < distz)
         selected_axis = vec3(1, 0, 0);
      else if (disty < distx && disty < distz)
         selected_axis = vec3(0, 1, 0);
      else if (distz < distx && distz < disty)
         selected_axis = vec3(0, 0, 1);

      // Test if we grab the interactor axis or planes
      is_grab_interactor |= distx < 6 || disty < 6 || distz < 6;

      index_t v_idx = pick_vertex(p_ndc);

      // If a vertex was picked, change selected vertex and repaint interactor
      if (v_idx != NO_VERTEX && !is_grab_interactor) {

         if (pressedKey != "shift")
            unselect_all_verts();
         
         select_vert(v_idx);
      }

      // Unselect
      if (p_ndc.button == MOUSE_BUTTON_RIGHT) {
         unselect_all_verts();
      }

      // paint_transform_interactor();
      paint_overlay(p_dc);


      MeshGrobTool::grab(p_ndc);
   }

    void MeshGrobTransformInteractionsTool::drag(const RayPick& p_ndc) {
      
      // Get mouse move delta
      vec2 m_delta = p_ndc.p_ndc - m_last_pos;

      // Get interactor axis
      auto axis = transform_interactor_axis();

      // 2D - ortho basis
      vec2 i = normalize(axis[1] - axis[0]);
      vec2 j = normalize(axis[2] - axis[0]);
      vec2 k = normalize(axis[3] - axis[0]);

      // Displacement is proportional to the direction of the mouse along the axis (see use of dot)
      vec3 delta = vec3{dot(i, m_delta), dot(j, m_delta), dot(k, m_delta)} * 0.5;

      for (index_t selected_vertex : get_selected_verts())
         mesh_grob()->vertices.point(selected_vertex) += vec3(selected_axis.x * delta.x, selected_axis.y * delta.y, selected_axis.z * delta.z);

      mesh_grob()->update();


      m_last_pos = p_ndc.p_ndc;

      // paint_transform_interactor();
      paint_overlay(ndc_to_dc(p_ndc.p_ndc));



       // Some tools may also do special actions
       // behavior when the mouse button is dragged.
       // If not used, you may remove this function as
       // well as its declaration in the header.
       MeshGrobTool::drag(p_ndc);


    }

    std::array<vec2, 4> MeshGrobTransformInteractionsTool::transform_interactor_axis() {

      if (get_selected_verts().empty())
         return  std::array<vec2, 4>({vec2(0,0), vec2(0,0), vec2(0,0), vec2(0,0)});

      vec3 vp = bary_of_selected_verts();
      vec2 origin = project_point(vp);
      vec2 xp = project_point(vp + vec3(0.05, 0, 0));
      vec2 yp = project_point(vp + vec3(0, 0.05, 0));
      vec2 zp = project_point(vp + vec3(0, 0, 0.05));
      
      return std::array<vec2, 4>({origin, xp, yp, zp});
    }

   std::array<vec2, 4> MeshGrobTransformInteractionsTool::get_x_quad(double size = INTERACTOR_QUADS_SIZE) { 
      vec3 vp = bary_of_selected_verts();

      
      vec2 origin = project_point(vp);
      vec2 yp = project_point(vp + vec3(0, size, 0));
      vec2 zp = project_point(vp + vec3(0, 0, size));
      vec2 yzp = project_point(vp + vec3(0, size, size));

      return { origin, yp, yzp, zp };
   }

   std::array<vec2, 4> MeshGrobTransformInteractionsTool::get_y_quad(double size = INTERACTOR_QUADS_SIZE) { 
      vec3 vp = bary_of_selected_verts();
      
      vec2 origin = project_point(vp);
      vec2 xp = project_point(vp + vec3(size, 0, 0));
      vec2 zp = project_point(vp + vec3(0, 0, size));
      vec2 xzp = project_point(vp + vec3(size, 0, size));

      return { origin, xp, xzp, zp };
   }

   std::array<vec2, 4> MeshGrobTransformInteractionsTool::get_z_quad(double size = INTERACTOR_QUADS_SIZE) { 
      vec3 vp = bary_of_selected_verts();
      
      vec2 origin = project_point(vp);
      vec2 xp = project_point(vp + vec3(size, 0, 0));
      vec2 yp = project_point(vp + vec3(0, size, 0));
      vec2 xyp = project_point(vp + vec3(size, size, 0));

      return { origin, xp, xyp, yp };
   }

   void MeshGrobTransformInteractionsTool::paint_overlay(vec2 pdc) {

      rendering_context()->overlay().clear();

      if (pressed_button == MOUSE_BUTTON_LEFT && pressedKey == "shift") {
         rendering_context()->overlay().fillrect(
            mouse_down_pos, pdc, Color(.5, .5, .5, 0.3)

         );
      }

      paint_transform_interactor();
   }

    void MeshGrobTransformInteractionsTool::paint_transform_interactor() {

      if (get_selected_verts().empty())
         return;   

      // World coordinate system
      auto coord_system = transform_interactor_axis();
      vec2 origin = coord_system[0];
      vec2 xp = coord_system[1];
      vec2 yp = coord_system[2];
      vec2 zp = coord_system[3];

      vec3 col =  selected_axis * 0.2;

      // X-Axis
      rendering_context()->overlay().segment(
         origin, xp, Color(.8 + selected_axis.x * .2, 0, 0, 1.0), 1.
      );
      rendering_context()->overlay().fillcircle(xp, 5., Color(.8 + selected_axis.x * .2, 0, 0, 1.0));

      // Y-Axis
      rendering_context()->overlay().segment(
         origin, yp, Color(0, .8 + selected_axis.y * .2, 0, 1.0), 1.
      );
      rendering_context()->overlay().fillcircle(yp, 5., Color(0, .8 + selected_axis.y * .2, 0, 1.0));

      // Z-Axis
      rendering_context()->overlay().segment(
         origin, zp, Color(0, 0, .8 + + selected_axis.z * .2, 1.0), 1.
      );
      rendering_context()->overlay().fillcircle(zp, 5., Color(0, 0, .8 + selected_axis.z * .2, 1.0));


      // Plane highlight colors
      double r = selected_axis.y > 0 && selected_axis.z > 0 ? 1. : 0.8;
      double g = selected_axis.x > 0 && selected_axis.z > 0 ? 1. : 0.8;
      double b = selected_axis.y > 0 && selected_axis.z > 0 ? 1. : 0.8;

      // X-Plane
      auto x_quad = get_x_quad();

      rendering_context()->overlay().fillquad(
         x_quad[0], x_quad[1], x_quad[2], x_quad[3], Color(r, 0, 0, 0.3)
      );

      // Y-Plane
      auto y_quad = get_y_quad();

      rendering_context()->overlay().fillquad(
         y_quad[0], y_quad[1], y_quad[2], y_quad[3], Color(0, g, 0, 0.3)
      );

      // Z-Plane
      auto z_quad = get_z_quad();
      
      rendering_context()->overlay().fillquad(
         z_quad[0], z_quad[1], z_quad[2], z_quad[3], Color(0, 0, b, 0.3)
      );

    }


   void MeshGrobTransformInteractionsTool::release(const RayPick& p_ndc) {

      index_t v_idx = pick_vertex(p_ndc);

      // No axis selected
      selected_axis = vec3(0, 0, 0);

      paint_overlay(ndc_to_dc(p_ndc.p_ndc));


      MeshGrobTool::release(p_ndc);
   }


   void MeshGrobTransformInteractionsTool::unselect_all_verts() {
        
        // Get vertices.selection attribute
        Attribute<Numeric::uint8> vertices_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );

        for (int i = 0; i < vertices_selection.size(); i++)
            vertices_selection[i] = false;

   }

   void MeshGrobTransformInteractionsTool::select_vert(index_t i) {

        // Get vertices.selection attribute
        Attribute<Numeric::uint8> vertices_selection(
            mesh_grob()->vertices.attributes(), "selection"
        );

        vertices_selection[i] = true;

   }

   std::vector<index_t> MeshGrobTransformInteractionsTool::get_selected_verts() {
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

   vec3 MeshGrobTransformInteractionsTool::bary_of_selected_verts() {
      auto selected_vertices = get_selected_verts();
      vec3 bary = {0,0,0};
      for (auto v : selected_vertices)
         bary += mesh_grob()->vertices.point(v);

      return bary / selected_vertices.size();
   }

}
