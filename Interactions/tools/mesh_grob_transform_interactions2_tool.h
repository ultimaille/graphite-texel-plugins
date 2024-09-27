
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
 

#ifndef H__OGF_INTERACTIONS_TOOLS_MESH_GROB_TRANSFORM_INTERACTIONS2_TOOL__H
#define H__OGF_INTERACTIONS_TOOLS_MESH_GROB_TRANSFORM_INTERACTIONS2_TOOL__H

#include <OGF/Interactions/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>
#include <OGF/scene_graph/tools/tool.h>
#include <OGF/scene_graph/tools/tools_manager.h>

namespace OGF {


    // specifies in which box this tool will be added 
    gom_attribute(category, "Interactions") 

    //an icon can be specified for this tool
    //(this example corresponds to GRAPHITE_ROOT/lib/icons/my_icon.xpm)
    gom_attribute(icon,"../../../plugins/OGF/Interactions/transform2_interactions") 

    // specifies the help bubble associated with this tool 
    gom_attribute(help,"TransformInteractions2 tool") 

    // the message is displayed in the status bar when this
    // tool is selected 
    gom_attribute(message,"insert your message here") 

    gom_class Interactions_API MeshGrobTransformInteractions2Tool : public MeshGrobTool {
    public:
        MeshGrobTransformInteractions2Tool( ToolsManager* parent ) ;
        virtual ~MeshGrobTransformInteractions2Tool() ;

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

        std::vector<index_t> get_selected_verts();
        void paint_overlay(vec2 pdc);

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

        index_t pickup_edge(vec3 p0, index_t c_idx);
        index_t pickup_facet_edge(vec3 p0, vec3 p01, index_t f_idx);
        std::tuple<index_t, index_t> pickup_facet(vec3 p0, index_t c_idx);

        bool is_mousedown = false;
        bool is_current_tool = true;
        std::map<index_t, vec3> latest_pos;

        std::string current_interactor;
        std::string hold_key;

        vec2 ref_pos;
        vec2 mouse_pos;
        index_t selected_edge = -1;
        index_t selected_cell = -1;
        index_t selected_facet = -1;
        index_t selected_local_facet = -1;


        std::vector<std::tuple<index_t, index_t>> selected_facet_edges;
        vec3 latest_pickup_point;

    } ;

}

#endif

