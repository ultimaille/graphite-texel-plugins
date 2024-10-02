
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
 


#include <OGF/Interactions/commands/mesh_grob_feature_commands.h>

namespace OGF {

    MeshGrobFeatureCommands::MeshGrobFeatureCommands() { 
    }
        
    MeshGrobFeatureCommands::~MeshGrobFeatureCommands() { 
    }        

    void MeshGrobFeatureCommands::remove_selected() {
        // Get halfedge is_feature attribute
        Attribute<Numeric::uint8> is_feature(
            mesh_grob()->facet_corners.attributes(), "is_feature"
        );

        // Get halfedge / edge matching
        Attribute<Numeric::uint32> corner_matching(
            mesh_grob()->edges.attributes(), "corner_matching"
        );

        // Get edge selection attribute
        Attribute<Numeric::uint8> edge_selection(
            mesh_grob()->edges.attributes(), "selection"
        );

        for (index_t e : mesh_grob()->edges) {
            if (edge_selection[e]) {
                index_t c = corner_matching[e];
                is_feature[c] = false;
            }
        }
    }       

}
