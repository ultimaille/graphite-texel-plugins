
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
 

#ifndef H__OGF_INSPECT_COMMANDS_MESH_GROB_CONDITIONAL_FILTER_COMMANDS__H
#define H__OGF_INSPECT_COMMANDS_MESH_GROB_CONDITIONAL_FILTER_COMMANDS__H

#include <OGF/Inspect/common/common.h>
#include <OGF/mesh/commands/mesh_grob_commands.h>

namespace OGF {

    gom_class Inspect_API MeshGrobConditionalFilterCommands : public MeshGrobCommands {
    public:
        MeshGrobConditionalFilterCommands();
        ~MeshGrobConditionalFilterCommands() override;

    gom_slots:

        //   Doxygen comments are parsed and used by Gomgen to
        //     generate tooltips.
        //   In addition to standard Doxygen tags, the following 
	//     tags can be used:
	//
        //      \menu  indicate a menu relative to current menu
        //           (MeshGrobConditionalFilterCommands), or an absolute menu (starting
        //           with a '/') to insert the command in existing
        //           menus (for instance /Surface/Remesh)
	//          
        //      \advanced  all subsequent parameters are in the
        //        advanced section of the command (displayed when
        //        clicking on it)

        enum ComparatorOp { Greater, Lesser, Equal, NotEqual };

        /**
          * \brief Apply a conditional filter on cells

          * \param[in] attr_name Attribute name
          * \param[in] comparator_op The operator used to compare attribute with value
          * \param[in] value Value to compare to attribute
	      * \advanced
          * \menu /Filters
          * \param[in] eps If =, != operators are used, as we can't compare equality between double, we use an epsilon
          */
        gom_arg_attribute(attr_name, handler, "combo_box")
	    gom_arg_attribute(attr_name, values, "$grob.list_attributes('cells','OGF::Numeric::float64','1')")
        void conditional_cell_filter(
            std::string &attr_name,
            ComparatorOp comparator_op,
            double value,
            double eps = 0.0001
        ) ;
    } ;
}

#endif

