
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

#include "../tools/FeatureLinesSetup.h"

namespace OGF {

    MeshGrobFeatureCommands::MeshGrobFeatureCommands() { 
    }
        
    MeshGrobFeatureCommands::~MeshGrobFeatureCommands() { 
    }        

    /**
     * \brief Remove the selected feature lines
     */
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

        // As a feature line is composed by many edge
        // We search selected edges, in order to remove them
        GEO::vector<index_t> selected_edges;

        for (index_t e : mesh_grob()->edges) {
            // If edge is selected, 
            if (edge_selection[e]) {
                // Get corresponding corner
                index_t c = corner_matching[e];
                // Set is_feature attribute of the corresponding corner to false
                is_feature[c] = false;
                // Add edge index to selected_edges
                selected_edges.push_back(e);
            }
        }
        

        // Remove feature line visually (remove polyline edges) thanks to feature line manager
        auto featurelines_setup = FeatureLinesSetup::get(mesh_grob());
        featurelines_setup->remove(selected_edges);
    }       

}
