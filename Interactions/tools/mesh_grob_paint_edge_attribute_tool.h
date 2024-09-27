
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
 

#ifndef H__OGF_INTERACTIONS_TOOLS_MESH_GROB_PAINT_EDGE_ATTRIBUTE_TOOL__H
#define H__OGF_INTERACTIONS_TOOLS_MESH_GROB_PAINT_EDGE_ATTRIBUTE_TOOL__H

#include <OGF/Interactions/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>
#include <OGF/scene_graph/tools/tool.h>
#include <OGF/scene_graph/tools/tools_manager.h>

namespace OGF {

    /**
     * \brief A painting operation.
     */
    enum PaintOp {
        PAINT_SET,   /**< sets attribute value */
        PAINT_RESET, /**< resets attribute value to zero */
        PAINT_INC,   /**< adds to attribute value */
        PAINT_DEC    /**< subtracts from attribute value */
    };

    // specifies in which box this tool will be added 
    gom_attribute(category,"Interactions") 

    //an icon can be specified for this tool
    //(this example corresponds to GRAPHITE_ROOT/lib/icons/my_icon.xpm)
    // gom_attribute(icon,"my_icon") 

    // specifies the help bubble associated with this tool 
    gom_attribute(help,"PaintEdgeAttribute tool") 

    // the message is displayed in the status bar when this
    // tool is selected 
    gom_attribute(message,"insert your message here") 

    gom_class Interactions_API MeshGrobPaintEdgeAttributeTool : public MeshGrobTool {
    public:
        MeshGrobPaintEdgeAttributeTool( ToolsManager* parent ) ;
        virtual ~MeshGrobPaintEdgeAttributeTool() ;

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

        gom_properties:

        void set_width(index_t value) {
            width_ = value;
        }

        index_t get_width() const {
            return width_;
        }
        
        void set_stroke_mode(bool value) {
            stroke_mode_ = value;
        }
        
        bool get_stroke_mode() const {
            return stroke_mode_;
        }



    protected:

        /**
         * \brief Calls a user function for each quad of the stroke.
         * \param[in] doit the function to be called. It takes as arguments
         *  the four vertices of the quad, in device coordinates.
         */
        void for_each_stroke_quad(std::function<void(vec2, vec2, vec2, vec2)> doit);

         bool get_painting_parameters(
             const RayPick& raypick,
             PaintOp& op,
             MeshElementsFlags& where,
             std::string& attribute_name,
             index_t& component
         );

        /**
         * \brief Paints a rectangle
         * \param[in] raypick 
         * \param[in] x0 , y0 , x1 , y1 image bounds (device coordinates)
         * \param[in] mask an optional mask. Black pixels do not belong to
         *  the selection, non-zero pixels belong to selection. Image is
         *  supposed to be grayscale, 8 bits per pixels. Size is supposed to
         *  be (x1-x0+1) x (y1-y0+1)
         */
        void paint_rect(const RayPick& raypick, index_t x0, index_t y0, index_t x1, index_t y1, Image* mask = nullptr);

        void paint(const RayPick& p_ndc);

        void setup_polyline();

        /**
         * \copydoc MeshGrobTool::reset()
         */
        void reset() override;

        double value_;
        bool accumulate_;
        bool autorange_;
        bool pick_vertices_only_;
        bool xray_mode_;
        
        index_t picked_element_;
        index_t width_;
        vec2 latest_ndc_;
        bool stroke_mode_;
        vector<vec2> stroke_;

        std::map<std::string, bool> is_poly_generated;
    } ;

}

#endif

