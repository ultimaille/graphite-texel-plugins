
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
 

#ifndef H__OGF_INSPECT_TOOLS_MESH_GROB_LACE_VIEWER_TOOL__H
#define H__OGF_INSPECT_TOOLS_MESH_GROB_LACE_VIEWER_TOOL__H

#include <OGF/Inspect/common/common.h>
#include <OGF/mesh/tools/mesh_grob_tool.h>
#include <OGF/scene_graph/tools/tool.h>
#include <OGF/scene_graph/tools/tools_manager.h>

namespace OGF {


    // specifies in which box this tool will be added 
    gom_attribute(category,"Inspect") 
    gom_attribute(icon,"lace") 
    //an icon can be specified for this tool
    //(this example corresponds to GRAPHITE_ROOT/lib/icons/my_icon.xpm)
    // gom_attribute(icon,"my_icon") 

    // specifies the help bubble associated with this tool 
    gom_attribute(help,"LaceViewer tool") 

    // the message is displayed in the status bar when this
    // tool is selected 
    gom_attribute(message,"insert your message here") 

    gom_class Inspect_API MeshGrobLaceViewerTool : public MeshGrobTool {
    public:
        MeshGrobLaceViewerTool( ToolsManager* parent ) ;
        virtual ~MeshGrobLaceViewerTool() ;

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
        /**
         * \brief Max depth
         */
        void set_value(int value) {
            value_ = value;
        }
         
        int get_value() const {
	        return value_;
	    }

        int get_n_ring_max() const {
            return n_ring_max_;
        }

        void set_n_ring_max(int n_ring_max)  {
            n_ring_max_ = n_ring_max;
        }

        enum ExtractType {LAYER, LACE, RING};

        /**
         * \brief The type of extraction
         */
        void set_extract_type(ExtractType extract_type) {
            extract_type_ = extract_type;
        }
         
        ExtractType get_extract_type() const {
	        return extract_type_;
	    }

    protected:

        int n_ring_max_ = 3;
        int value_ = 1;
        int shrink_value = 1;
        ExtractType extract_type_ = LAYER;

    private:
        index_t pickup_edge(vec3 p0, index_t c_idx);
        std::tuple<index_t, index_t> pickup_facet(vec3 p0, index_t c_idx);
        void bfs_lace_propagate(const OGF::MeshGrob *mesh_grob, index_t c, index_t fi, int max_depth, std::function<void(index_t, int)> f);
        void bfs_layer_propagate(const OGF::MeshGrob *mesh_grob, index_t c, index_t lf, index_t le, int max_depth, std::function<void(index_t, int)> f);
        void bfs_cell_propagate(const OGF::MeshGrob *mesh_grob, index_t c, int max_depth, std::function<void(index_t, int)> f);
    } ;

}

#endif

