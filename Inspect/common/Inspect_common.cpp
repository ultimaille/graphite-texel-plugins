
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
 
 
#include <OGF/Inspect/common/common.h>
#include <OGF/basic/modules/module.h>
#include <OGF/gom/types/gom_defs.h>
#include <OGF/scene_graph/types/scene_graph_library.h>

#include <OGF/Inspect/tools/mesh_grob_lace_viewer_tool.h>
#include <OGF/Inspect/commands/mesh_grob_conditional_filter_commands.h>
// [includes insertion point] (do not delete this line)

namespace OGF {

    void Inspect_libinit::initialize() {

        Logger::out("Init") << "<Inspect>" << std::endl;

        //**************************************************************

        gom_package_initialize(Inspect) ;

        ogf_register_grob_tool<OGF::MeshGrob,MeshGrobLaceViewerTool>();
        ogf_register_grob_commands<OGF::MeshGrob,OGF::MeshGrobConditionalFilterCommands>();
        // [source insertion point] (do not delete this line)

        // Insert package initialization stuff here ...

        //**************************************************************

        Module* module_info = new Module;
        module_info->set_name("Inspect");
        module_info->set_vendor("me");
        module_info->set_version("3-1.x");
        module_info->set_info(
                "New package, created by Graphite Development Tools"
        );
        Module::bind_module("Inspect", module_info);

        Logger::out("Init") << "</Inspect>" << std::endl;
    }
    
    void Inspect_libinit::terminate() {
        Logger::out("Init") << "<~Inspect>" << std::endl;
	
        //*************************************************************

        // Insert package termination stuff here ...

        //*************************************************************

        Module::unbind_module("Inspect");
        Logger::out("Init") << "</~Inspect>" << std::endl;
    }
    
    Inspect_libinit::Inspect_libinit() {
        increment_users();
    }

    Inspect_libinit::~Inspect_libinit() {
        decrement_users();
    }
    
    void Inspect_libinit::increment_users() {
        // Note that count_ is incremented before calling
        // initialize, else it would still be equal to
        // zero at module initialization time, which 
        // may cause duplicate initialization of libraries.
        count_++;
        if(count_ == 1) {
            initialize();
        }
    }
    
    void Inspect_libinit::decrement_users() {
        count_--;
        if(count_ == 0) {
            terminate();
        }
    }
    
    int Inspect_libinit::count_ = 0;
}

// The initialization and termination functions
// are also declared using C linkage in order to 
// enable dynamic linking of modules.

extern "C" void Inspect_API OGF_Inspect_initialize(void);
extern "C" void Inspect_API OGF_Inspect_initialize() {
    OGF::Inspect_libinit::increment_users();
}

extern "C" void Inspect_API OGF_Inspect_terminate(void);
extern "C" void Inspect_API OGF_Inspect_terminate() {
    OGF::Inspect_libinit::decrement_users();
}
