
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
 

#ifndef H__OGF_INTERACTIONS_TOOLS_MESH_GROB_TRANSFORM_INTERACTIONS_TOOL__H
#define H__OGF_INTERACTIONS_TOOLS_MESH_GROB_TRANSFORM_INTERACTIONS_TOOL__H

#include <OGF/Interactions/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>
#include <OGF/scene_graph/tools/tool.h>
#include <OGF/scene_graph/tools/tools_manager.h>

namespace OGF {


    // specifies in which box this tool will be added 
    gom_attribute(category,"Interactions") 

    //an icon can be specified for this tool
    //(this example corresponds to GRAPHITE_ROOT/lib/icons/my_icon.xpm)
    gom_attribute(icon,"../../../plugins/OGF/Interactions/Untitle") 

    // specifies the help bubble associated with this tool 
    gom_attribute(help,"TransformInteractions tool") 

    // the message is displayed in the status bar when this
    // tool is selected 
    gom_attribute(message,"insert your message here") 


    gom_class Interactions_API MeshGrobTransformInteractionsTool : public MeshGrobTool {
    public:
        MeshGrobTransformInteractionsTool( ToolsManager* parent ) ;
        virtual ~MeshGrobTransformInteractionsTool() ;

        static void loop(MeshGrobTransformInteractionsTool * tool);

        /**
         * \copydoc Tool::grab()
         */
        void grab(const RayPick& p_ndc) override;
        
        /**
         * \copydoc Tool::drag()
         */
        void drag(const RayPick& p_ndc) override;

        /**
         * \copydoc Tool::release()
         */
        void release(const RayPick& p_ndc) override;

        void paint_overlay(vec2 pdc);
        void paint_transform_interactor();


        

        void unselect_all_verts();
        void select_vert(index_t i);
        vec3 bary_of_selected_verts();
        std::vector<index_t> get_selected_verts();

        std::array<vec2, 4> transform_interactor_axis();
        std::array<vec2, 4> get_x_quad();
        std::array<vec2, 4> get_y_quad();
        std::array<vec2, 4> get_z_quad();
        
        gom_properties:
        /**
         * \brief Move sensitivity
         */
        void set_sensitivity(double _sensitivity) {
            sensitivity = _sensitivity;
        }
         
        double get_sensitivity() const {
	        return sensitivity;
	    }

    protected:

        double sensitivity = 0.5;


        gom_slots:
            void key_down(const std::string& value);
            void key_up(const std::string& value);
            void mouse_down(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, int button, bool control, bool shift);
            void mouse_up(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, int button, bool control, bool shift);
            void mouse_move(RenderingContext* rendering_context, const vec2& point_ndc, const vec2& point_wc, const vec2& delta_ndc, double delta_x_ndc, double delta_y_ndc, const vec2& delta_wc, int button, bool control, bool shift);
            void tool_icon_changed(const std::string& value);

    private:
        double INTERACTOR_AXIS_SIZE;
        static constexpr double FACTOR = 3 / 10.;

        bool is_current_tool = true;

        vec2 m_last_pos;
        index_t selected_vertex;
        GEO::vector<index_t> x_axis_facets;
        GEO::vector<index_t> y_axis_facets;
        GEO::vector<index_t> z_axis_facets;
        vec3 selected_axis;
        // Keyboard / mouse state
        std::string pressedKey;
        int pressed_button;

        vec2 mouse_down_pos;
    } ;

}

#endif

