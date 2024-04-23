
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
 


#include <OGF/Inspect/commands/mesh_grob_conditional_filter_commands.h>
#include <OGF/scene_graph/types/scene_graph.h>
#include <OGF/scene_graph/types/properties.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/scene_graph/types/scene_graph_shader_manager.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <geogram/basic/file_system.h>
#include <geogram_gfx/mesh/mesh_gfx.h>


namespace OGF {
        enum ComparatorOp { Greater, Lesser, Equal, NotEqual };

    MeshGrobConditionalFilterCommands::MeshGrobConditionalFilterCommands() { 
    }
        
    MeshGrobConditionalFilterCommands::~MeshGrobConditionalFilterCommands() { 
    }        

    void MeshGrobConditionalFilterCommands::conditional_cell_filter(std::string &attr_name, ComparatorOp comparator_op, double value, double eps) {

        std::cout << attr_name << std::endl;
        MeshElementsFlags where;
        index_t dim;

        mesh_grob()->parse_attribute_name(attr_name, where, attr_name, dim);
        // Get cells attr
        Attribute<Numeric::float64> attr(
            mesh_grob()->cells.attributes(), attr_name
        );

        // Get cells.selection attribute
        Attribute<Numeric::uint8> cell_filter(
            mesh_grob()->cells.attributes(), "filter"
        );

        Attribute<Numeric::uint8> vertex_filter(
            mesh_grob()->vertices.attributes(), "filter"
        );

        // Reset
        for (int i = 0; i < cell_filter.nb_elements(); i++)
            cell_filter[i] = false;

        for (int i = 0; i < vertex_filter.nb_elements(); i++)
            vertex_filter[i] = false;

        for (int c = 0; c < mesh_grob()->cells.nb(); c++) {
            switch (comparator_op)
            {
            case Greater:
                cell_filter[c] = attr[c] > value;
                break;
            case Lesser:
                cell_filter[c] = attr[c] < value;
                break;
            case Equal:
                cell_filter[c] = std::abs(attr[c] - value) < eps;
                break;
            case NotEqual:
                cell_filter[c] = std::abs(attr[c] - value) >= eps;
                break;
            }
        }

        // Filter vertices that belong to filtered cells
        for (int i = 0; i < cell_filter.nb_elements(); i++) {
            if (!cell_filter[i])
                continue;
            
            for (int j = 0; j < mesh_grob()->cells.nb_corners(i); j++) {
                index_t v = mesh_grob()->cells.vertex(i, j);
                vertex_filter[v] = true;
            }
        }


        // Get shader
        PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();

        shader->set_painting(ATTRIBUTE);
        shader->set_attribute("cells.filter");
        shader->autorange();

        shader->set_cells_filter(true);
        shader->set_vertices_filter(true);

        // Logger::status() << "Called MeshGrobConditionalFilterCommands::my_function(" 
        //     << my_integer << "," << my_string << "," << my_double << ")" << std::endl ;
    }       

}
