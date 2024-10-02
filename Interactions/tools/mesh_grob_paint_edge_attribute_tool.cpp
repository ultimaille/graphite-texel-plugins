
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
 

#include <OGF/Interactions/tools/mesh_grob_paint_edge_attribute_tool.h>

#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/reflection/meta.h>

#include <geogram_gfx/third_party/imgui/imgui.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/image/image_library.h>
#include <geogram/image/image_rasterizer.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/basic/algorithm.h>

#include <stack>

namespace OGF {

    /**
     * \brief called a function for each unique picked element in a 
     *  picking image
     * \param[in,out] picking_image the image. It needs to be in RGBA format.
     *  It is modified by the function.
     * \param[in] mask an optional pointer to a mask (or nullptr for no mask).
     * \param[in] doit the function to be called for each unique picked element
     */
    void for_each_picked_element(Image* picking_image, Image* mask, std::function<void(index_t)> doit) {
        geo_assert(picking_image->color_encoding() == Image::RGBA);
        geo_assert(picking_image->component_encoding() == Image::BYTE);

        // Apply mask to picking image
        if(mask != nullptr) {
            geo_assert(mask->color_encoding() == Image::GRAY);
            geo_assert(mask->component_encoding() == Image::BYTE);
            geo_assert(mask->width() == picking_image->width());
            geo_assert(mask->height() == picking_image->height());
            for(index_t y=0; y<picking_image->height(); ++y) {
                for(index_t x=0; x<picking_image->width(); ++x) {
                    // Note: glReadPixels and rasterizer use the opposite
                    // convention for the Y coordinate-------------v
                    if(*mask->pixel_base_byte_ptr(x,mask->height()-y-1) == 0) {
                        *(Numeric::uint32*)picking_image->pixel_base(x,y) =
                            Numeric::uint32(-1);
                    }
                }
            }
        }
        
        // Get all the picked ids, by sorting the pixels of the image by
        // value then using std::unique
        Numeric::uint32* begin = (Numeric::uint32*)(picking_image->base_mem());
        Numeric::uint32* end =
            begin + picking_image->width() * picking_image->height();
        std::sort(begin, end);
        end = std::unique(begin,end);
        for(index_t* p=begin; p!=end; ++p) {
            if(*p != index_t(-1)) {
                doit(*p);
            }
        }
    }

    /**
     * \brief Calls a user-specified function for each edge in a connected
     *  component incident to a given vertex in a feature line
     * \param[in] mesh_grob the mesh
     * \param[in] seed_edge one of the facets of the connected component
     * \param[in] doit the function to be called for each edge of the
     *  connected component incident to \p seed_edge
     */
    void for_each_connected_edge_in_feature_line(
        MeshGrob* mesh_grob, index_t seed_edge,
        std::function<bool(index_t)> doit        
    ) {
        std::vector<bool> visited(mesh_grob->edges.nb(),false);
        std::stack<index_t> S;
        S.push(seed_edge);

        // Search for reverse edge of seed (and propagate to its connected edge through the queue !)
        for (index_t e : mesh_grob->edges) {
            if (mesh_grob->edges.vertex(e, 0) == mesh_grob->edges.vertex(seed_edge, 1) && 
                mesh_grob->edges.vertex(e, 1) == mesh_grob->edges.vertex(seed_edge, 0)) {
                S.push(e);
                visited[e] = true;
                doit(e);
            }
        }

        visited[seed_edge] = true;
        doit(seed_edge);

        while(!S.empty()) {
            index_t e = S.top();
            S.pop();
            
            index_t v[] = {mesh_grob->edges.vertex(e, 0), mesh_grob->edges.vertex(e, 1)};

            // Search for next and prev edges
            int count[2] = {0};
            index_t conn_e[2] = {NO_EDGE};

            for (int i = 0; i < 2; i++) {
                for (index_t ne : mesh_grob->edges) {
                    if (ne == e)
                        continue;

                    // if (mesh_grob->edges.vertex(ne, 1) == v[i] || mesh_grob->edges.vertex(ne, 0) == v[i]) {
                    if (mesh_grob->edges.vertex(ne, i) == v[i]) {
                        count[i]++;
                        conn_e[i] = ne;
                    }

                }

                if (count[i] == 1 && !visited[conn_e[i]]) {
                    if (doit(conn_e[i])) {
                        S.push(conn_e[i]);
                    }
                    visited[conn_e[i]] = true;
                }
            }

        }
    }

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

    /**
     * \brief Gets the visible attribute from the shader of a MeshGrob
     * \param[in] mesh_grob the MeshGrob
     * \param[out] where one of MESH_VERTICES, MESH_FACETS, MESH_CELLS
     * \param[out] attribute_name the base name of the attribute
     * \param[out] component the component index for a vector attribute 
     *             (0 if it is a scalar attribute)
     */
    bool get_visible_attribute(
        MeshGrob* mesh_grob,
        MeshElementsFlags& where,
        std::string& attribute_name,
        index_t& component
    ) {
        MeshGrobShader* shd = dynamic_cast<MeshGrobShader*>(
            mesh_grob->get_shader()
        );
        
        if(shd == nullptr) {
            return false;
        }

        std::string painting;
        shd->get_property("painting",painting);
        if(painting != "ATTRIBUTE") {
            return false;
        }

        std::string full_attribute_name;
        shd->get_property("attribute",full_attribute_name);
        if(!Mesh::parse_attribute_name(
               full_attribute_name,where,attribute_name,component)
          ) {
            return false;
        }
        
        return true;
    }

    /**
     * \brief Paints an attribute value for a given type
     * \tparam T the type of the attribute
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] where one of MESH_VERTICES, MESH_EDGES, MESH_FACETS or 
     *  MESH_CELLS
     * \param[in] name the name of the attribute
     * \param[in] element_id the element to be painted
     * \param[in] component the component of a vector attribute 
     *            (0 for a scalar attribute)
     * \param[in] op one of PAINT_SET, PAINT_RESET, PAINT_INC or PAINT_DEC
     * \param[in] value the value to be painted
     * \retval true if an attribute of the specified type was found 
     * \retval false otherwise
     */
    template <class T> bool paint_attribute_generic(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, PaintOp op, double value
    ) {
        MeshSubElementsStore& elts = mesh_grob->get_subelements_by_type(where);
        if(!Attribute<T>::is_defined(elts.attributes(), name)) {
            return false;
        }
        Attribute<T> attr(elts.attributes(), name);

        switch(op) {
        case PAINT_SET:
            attr[element_id*attr.dimension()+component] = T(value);
            break;
        case PAINT_RESET:
            attr[element_id*attr.dimension()+component] = T(0);
            break;            
        case PAINT_INC:
            attr[element_id*attr.dimension()+component] =
                attr[element_id*attr.dimension()+component] + T(value);
            break;            
        case PAINT_DEC:
            attr[element_id*attr.dimension()+component] =
                attr[element_id*attr.dimension()+component] - T(value);
            break;            
        }
        return true;
    }

    /**
     * \brief Paints an attribute value independently from the type. Tries
     *   int32, uint32, float, double, bool
     * \details The specified value is converted into the type. Floating-point
     *   values are truncated if the attribute has integer type.
     * \param[in] mesh_grob a pointer to the MeshGrob
     * \param[in] where one of MESH_VERTICES, MESH_EDGES, MESH_FACETS or 
     *  MESH_CELLS
     * \param[in] name the name of the attribute
     * \param[in] element_id the element to be painted
     * \param[in] component the component of a vector attribute 
     *            (0 for a scalar attribute)
     * \param[in] op one of PAINT_SET, PAINT_RESET, PAINT_INC or PAINT_DEC
     * \param[in] value the value to be painted
     * \retval true if an attribute could be painted
     * \retval false otherwise
     */
    bool paint_attribute(
        MeshGrob* mesh_grob, MeshElementsFlags where,
        const std::string& name, index_t component,
        index_t element_id, PaintOp op, double value
    ) {
        return paint_attribute_generic<double>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<float>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<Numeric::uint32>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<Numeric::int32>(
                   mesh_grob, where, name, component, element_id, op, value
               ) ||
               paint_attribute_generic<bool>(
                   mesh_grob, where, name, component, element_id, op, value
               );            
    }

    MeshGrobPaintEdgeAttributeTool::MeshGrobPaintEdgeAttributeTool(
        ToolsManager* parent
    ) : MeshGrobTool(parent) {
      width_ = 5;

      value_ = 1.0;
      accumulate_ = false;
      autorange_ = true;
      xray_mode_ = false;
      pick_vertices_only_ = true;
      picked_element_ = index_t(-1);
      stroke_mode_ = true;
    }

    MeshGrobPaintEdgeAttributeTool::~MeshGrobPaintEdgeAttributeTool() { 
    }

    void MeshGrobPaintEdgeAttributeTool::reset() {
        // Get shader
        PlainMeshGrobShader* shader = (PlainMeshGrobShader*)mesh_grob()->get_shader();

        // Create attribute if doesn't exists
        Attribute<Numeric::uint8> edge_selection(
            mesh_grob()->edges.attributes(), "selection"
        );
        
        // Set shader on edge selection attribute
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        if(!get_visible_attribute(mesh_grob(),where,attribute_name,component) || where != MESH_EDGES){
            shader->set_painting(ATTRIBUTE);
            shader->set_attribute("edges.selection");
            shader->autorange();
            
            auto ps = shader->get_vertices_style();
            ps.visible = false;

            shader->set_vertices_style(ps);
            where = MESH_EDGES;
        }

        // We have two type of CAD inputs:
        // - A surface + polyline (from GMSH) - polyline is feature lines of CAD
        // - A surface in wich each halfedge tagged as is_feature = true is a feature line

        // To match with these two inputs:
        // 1. If it exists a surface + a polyline, check that all polyline segments matches with a halfedge (corner)
        // 2. If yes, feature line are already represented by this polyline, so we just have to tag the matching halfedges with is_feature=true
        // 3. If no, setup a polyline according to is_feature attribute on halfedge

        // Check if a polyline already exist in model
        if (mesh_grob()->edges.nb() > 0) {
            // If yes, we mark halfedges (corners) that match with a edge as a feature line

            // Get halfedge is_feature attribute
            Attribute<Numeric::uint8> is_feature(
                mesh_grob()->facet_corners.attributes(), "is_feature"
            );

            // Check that polyline is a representation of feature lines
            for (index_t e : mesh_grob()->edges) {
                index_t v0 = mesh_grob()->edges.vertex(e, 0);
                index_t v1 = mesh_grob()->edges.vertex(e, 1);
                
                bool found = false;
                for (index_t f : mesh_grob()->facets) {
                    for (index_t fc : mesh_grob()->facets.corners(f)) {
                        index_t nfc = mesh_grob()->facets.next_corner_around_facet(f, fc);
                        index_t vc0 = mesh_grob()->facet_corners.vertex(fc);
                        index_t vc1 = mesh_grob()->facet_corners.vertex(nfc);
                        if (v0 == vc0 && v1 == vc1) {
                            found = true;
                            index_t adj_f = mesh_grob()->facet_corners.adjacent_facet(fc);
                            is_feature[fc] = true;
                            break;
                        }
                    }
                }

                if (!found) {
                    Logger::err("Mesh") << "Mesh polyline is not feature lines. Unable to use this tool on it !" << std::endl;
                    return;
                }

            }

            

        }

        // Generate a polyline that represent feature lines
        // only one time !
        if (!is_poly_generated[mesh_grob()->name()])
            setup_polyline();
    }

    /**
     * Setup a polyline that represent feature lines (needed only for interaction)
     */
    void MeshGrobPaintEdgeAttributeTool::setup_polyline() {

        // Get halfedge is_feature attribute
        Attribute<Numeric::uint8> is_feature(
            mesh_grob()->facet_corners.attributes(), "is_feature"
        );

        std::cout << "is_feature size: " << is_feature.size() << std::endl;

        index_t n_edges = 0;
        for (index_t i = 0; i < is_feature.size(); i++) {
            if (!is_feature[i])
                continue;
            n_edges++;
        }

        std::cout << "n edges: " << n_edges << std::endl;

        // No features found, no polyline to setup
        if (n_edges == 0) {
            return;
        }

        std::cout << "setup poly..." << std::endl;


        MeshGrob& M = *mesh_grob();
        if(M.vertices.dimension() < 3) {
            Logger::err("Mesh") << "Dimension smaller than 3" << std::endl;
            return;
        }

        index_t off = M.edges.create_edges(n_edges);

        // Create an attribute on edge, that map edge to halfedge !
        Attribute<Numeric::uint32> corner_matching(
            mesh_grob()->edges.attributes(), "corner_matching"
        );

        int n_edge = 0;

        for (index_t f : mesh_grob()->facets) {
            for (index_t fc : mesh_grob()->facets.corners(f)) {

                index_t nfc = mesh_grob()->facets.next_corner_around_facet(f, fc);

                if (is_feature[fc]) {
                    index_t v0 = mesh_grob()->facet_corners.vertex(fc);
                    index_t v1 = mesh_grob()->facet_corners.vertex(nfc);

                    M.edges.set_vertex(off + n_edge, 0, v0);
                    M.edges.set_vertex(off + n_edge, 1, v1);
                    
                    // Map edge to halfedge
                    corner_matching[n_edge] = fc;

                    n_edge++;
                }
            }
            
        }
  
        M.update();

        is_poly_generated[mesh_grob()->name()] = true;

    }

    void MeshGrobPaintEdgeAttributeTool::for_each_stroke_quad(
        std::function<void(vec2, vec2, vec2, vec2)> doit
    ) {
        for(index_t i=0; i+1<stroke_.size(); ++i) {
            vec2 p1 = stroke_[i];
            vec2 p2 = stroke_[i+1];
            vec2 n1 = p2-p1;
            vec2 n2 = p2-p1;
            if(i > 0) {
                n1 += p1 - stroke_[i-1];
            }
            if(i+2<stroke_.size()) {
                n2 += stroke_[i+2] - p2;
            }
            
            n1 = normalize(vec2(n1.y, -n1.x));
            n2 = normalize(vec2(n2.y, -n2.x));                    
            
            double width = double(width_);
            vec2 q1 = p1-width*n1;
            vec2 q2 = p1+width*n1;
            vec2 q3 = p2-width*n2;
            vec2 q4 = p2+width*n2;
            
            doit(q1,q2,q4,q3);
        }                
    }

    void MeshGrobPaintEdgeAttributeTool::grab(const RayPick& raypick) {

        MeshGrobTool::grab(raypick);

        latest_ndc_ = raypick.p_ndc;

         stroke_.push_back(ndc_to_dc(raypick.p_ndc));
    }

    void MeshGrobPaintEdgeAttributeTool::drag(const RayPick& raypick) {

      if(length(raypick.p_ndc - latest_ndc_) <= 10.0/1024.0) {
         return;
      }

      latest_ndc_ = raypick.p_ndc;
      
      stroke_.push_back(ndc_to_dc(raypick.p_ndc));

      rendering_context()->overlay().clear();
      for(index_t i=0; i<stroke_.size(); ++i) {
            rendering_context()->overlay().fillcircle(
               stroke_[i],double(width_),Color(1.0, 1.0, 1.0, 1.0)  
            );
      }
      for_each_stroke_quad(
            [&](vec2 q1, vec2 q2, vec2 q3, vec2 q4) {
               rendering_context()->overlay().fillquad(
                  q1,q2,q3,q4,Color(1.0, 1.0, 1.0, 1.0) 
               );
            }
      );

       MeshGrobTool::drag(raypick);
    }


    void MeshGrobPaintEdgeAttributeTool::release(const RayPick& raypick) {
        
        if(stroke_mode_ && stroke_.size() != 0) {
            
            // Get the bounding box of the stroke
            int x0 =  65535;
            int y0 =  65535;
            int x1 = -65535;
            int y1 = -65535;
            for_each_stroke_quad(
                [&](vec2 q1, vec2 q2, vec2 q3, vec2 q4) {
                    vec2 t[4] = {q1,q2,q3,q4};
                    for(index_t i=0; i<4; ++i) {
                        x0 = std::min(x0, int(t[i].x));
                        y0 = std::min(y0, int(t[i].y));
                        x1 = std::max(x1, int(t[i].x));
                        y1 = std::max(y1, int(t[i].y));                        
                    }
                }
            );

            // Enlarge the bbox to take into account the stroke width
            x0 -= int(width_);
            y0 -= int(width_);
            x1 += int(width_);
            y1 += int(width_);

            // Clip bounding box
            x0 = std::max(x0,0);
            y0 = std::max(y0,0);
            x1 = std::min(x1,int(rendering_context()->get_width() -1));
            y1 = std::min(y1,int(rendering_context()->get_height()-1));

            if(x1 > x0 && y1 > y0) {

                // Generate mask
                
                Image_var mask = new Image(
                    Image::GRAY, Image::BYTE,
                    index_t(x1-x0+1),
                    index_t(y1-y0+1)
                );
                ImageRasterizer rasterizer(mask);

                Color white(1.0, 1.0, 1.0, 1.0);

                if(stroke_.size() != 0) {
                    double r = double(width_)/double(x1-x0+1);
                    for(index_t i=0; i<stroke_.size(); ++i) {
                        vec2 c = stroke_[i];
                        c -= vec2(double(x0), double(y0));
                        c.x /= double(x1-x0+1);
                        c.y /= double(y1-y0+1);
                        rasterizer.fillcircle(c,r,white);
                    }
                }

                for_each_stroke_quad(
                    [&](vec2 q1, vec2 q2, vec2 q3, vec2 q4) {
                        vec2 t[4] = {q1,q2,q3,q4};
                        for(index_t i=0; i<4; ++i) {
                            t[i] -= vec2(double(x0), double(y0));
                            t[i].x /= double(x1-x0+1);
                            t[i].y /= double(y1-y0+1);
                            t[i].x = std::max(t[i].x,0.0);
                            t[i].y = std::max(t[i].y,0.0);
                            t[i].x = std::min(t[i].x,1.0);
                            t[i].y = std::min(t[i].y,1.0);
                        }
                        rasterizer.triangle(t[0],white,t[1],white,t[2],white);
                        rasterizer.triangle(t[0],white,t[2],white,t[3],white);
                    }
                );

                paint_rect(
                    raypick, index_t(x0), index_t(y0), index_t(x1), index_t(y1),
                    mask
                );


                
            } else {
                paint(raypick);
            }
            
            stroke_.clear();
            rendering_context()->overlay().clear();
        } else {
            paint(raypick);
        }

        if(autorange_) {
            if(mesh_grob()->get_shader() != nullptr) {
                mesh_grob()->get_shader()->invoke_method("autorange");
            }
        }
    }


    bool MeshGrobPaintEdgeAttributeTool::get_painting_parameters(
        const RayPick& raypick,
        PaintOp& op,
        MeshElementsFlags& where,
        std::string& attribute_name,
        index_t& component
    ) {
        if(!get_visible_attribute(mesh_grob(),where,attribute_name,component)){
            return false;
        }
        op = PAINT_SET;
        if(accumulate_) {
            op = (raypick.button==MOUSE_BUTTON_LEFT) ? PAINT_INC : PAINT_DEC;
        } else {
            op = (raypick.button==MOUSE_BUTTON_LEFT) ? PAINT_SET : PAINT_RESET;
        }
        return true;
    }

    void MeshGrobPaintEdgeAttributeTool::paint(const RayPick& raypick) {

        PaintOp op = PAINT_SET;
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        
        if(!get_painting_parameters(raypick,op,where,attribute_name,component)){
            return;
        }
        
        index_t picked_element = pick(raypick,where);

        // Paint the picked element
        if(picked_element != index_t(-1)) {
            paint_attribute(
                mesh_grob(), where,
                attribute_name, component,
                picked_element, op, value_
            );
        } else if(where == MESH_VERTICES && !pick_vertices_only_) {

            // If painting vertices and no vertex was picked, try to
            // pick a facet or a cell, and paint its vertices.
            
            index_t f = pick_facet(raypick);
            if(f != index_t(-1)) {
                picked_element_ = f;
                for(index_t lv = 0;
                    lv<mesh_grob()->facets.nb_vertices(f); ++lv
                ) {
                    index_t v = mesh_grob()->facets.vertex(f,lv);
                    paint_attribute(
                        mesh_grob(), where,
                        attribute_name, component,
                        v, op, value_
                    );
                } 
            } else {
                index_t c = pick_cell(raypick);
                picked_element_ = c;
                if(c != index_t(-1)) {
                    for(index_t lv = 0;
                        lv<mesh_grob()->cells.nb_vertices(c); ++lv
                    ) {
                        index_t v = mesh_grob()->cells.vertex(c,lv);
                        paint_attribute(
                            mesh_grob(), where,
                            attribute_name, component,
                            v, op, value_
                        );
                    }
                }
            }
        }

        if(picked_element != picked_element_) {
            if(autorange_) {
                if(mesh_grob()->get_shader() != nullptr) {
                    mesh_grob()->get_shader()->invoke_method("autorange");
                }
            }
            mesh_grob()->update();
            picked_element_ = picked_element;
        }
    }

    void MeshGrobPaintEdgeAttributeTool::paint_rect(
        const RayPick& raypick,
        index_t x0, index_t y0, index_t x1, index_t y1,
        Image* mask
    ) {
        if(x1 == x0 || y1 == y0) {
            return;
        }

        if(mask != nullptr) {
            geo_assert(mask->color_encoding() == Image::GRAY);
            geo_assert(mask->component_encoding() == Image::BYTE);
            geo_assert(mask->width() == x1-x0+1);
            geo_assert(mask->height() == y1-y0+1);
        }
        
        PaintOp op = PAINT_SET;
        MeshElementsFlags where;
        std::string attribute_name;
        index_t component;
        if(
            !get_painting_parameters(
                raypick, op, where, attribute_name, component
            )
        ) {
            return;
        }
        
        // Pick the elements, and copy the selected rect in an image
        index_t width  = x1-x0+1;
        index_t height = y1-y0+1;
        Image_var picking_image = new Image;
        // We need 32-bit pixel values (default is 24-bit)
        picking_image->initialize(Image::RGBA, Image::BYTE, width, height);


        // // In xray mode, test for each element whether its center falls in the
        // // selection. 
        // if(xray_mode_) {
        //     switch(where) {
        //     case MESH_VERTICES: {
        //         for(index_t v: mesh_grob()->vertices) {
        //             vec2 p = project_point(
        //                 vec3(mesh_grob()->vertices.point_ptr(v))
        //             );
        //             if(point_is_selected(p,x0,y0,x1,y1,mask)) {
        //                 paint_attribute(
        //                     mesh_grob(), where,
        //                     attribute_name, component,
        //                     v, op, value_
        //                 );
        //             }
        //         }
        //     } break;
        //     case MESH_FACETS: {
        //         for(index_t f: mesh_grob()->facets) {
        //             vec2 p = project_point(
        //                 Geom::mesh_facet_center(*mesh_grob(),f)
        //             );
        //             if(point_is_selected(p,x0,y0,x1,y1,mask)) {
        //                 paint_attribute(
        //                     mesh_grob(), where,
        //                     attribute_name, component,
        //                     f, op, value_
        //                 );
        //             }
        //         }
        //     } break;
        //     case MESH_CELLS: {
        //         for(index_t c: mesh_grob()->cells) {
        //             vec2 p = project_point(
        //                 Geom::mesh_cell_center(*mesh_grob(),c)
        //             );
        //             if(point_is_selected(p,x0,y0,x1,y1,mask)) {
        //                 paint_attribute(
        //                     mesh_grob(), where,
        //                     attribute_name, component,
        //                     c, op, value_
        //                 );
        //             }
        //         }
        //     } break;
        //     default: {
        //     } break;
        //     }
        // } else {

            // In standard mode, get picking image, apply the (optional) mask
            // and find all the picked elements
            
            // Damnit, glReadPixels and ImageRasterizer use the
            // opposite convention for Y coordinate.
            // It has an importance here because we got a mask,
            // so we flip y0 so that mask and picking image
            // have the same orientation.
            y0 = rendering_context()->get_height()-height-1-y0;
        
            pick(raypick, where, picking_image, x0, y0, width, height);

            for_each_picked_element(picking_image, mask, [&](index_t picked_element) {

                    paint_attribute(
                        mesh_grob(), where,
                        attribute_name, component,
                        picked_element, op, value_
                    );

                    if (where == MeshElementsFlags::MESH_EDGES) {

                        // Paint all connected edges that form a feature line
                        for_each_connected_edge_in_feature_line(mesh_grob(), picked_element, [&](index_t connected_e)->bool {

                            paint_attribute(
                                mesh_grob(), where,
                                attribute_name, component,
                                connected_e, op, value_
                            );

                            return true;
                        });
                    }


                }
            );

            // If painting vertices and no vertex was picked, try to
            // pick a facet or a cell, and paint its vertices.
            
            // if(!pick_vertices_only_ && where == MESH_VERTICES) {

                // pick(raypick,MESH_FACETS, picking_image, x0, y0, width, height);
                // for_each_picked_element(
                //     picking_image, mask,
                //     [&](index_t f) {
                //         for(index_t lv = 0;
                //             lv<mesh_grob()->facets.nb_vertices(f); ++lv
                //            ) {
                //             index_t v = mesh_grob()->facets.vertex(f,lv);
                //             paint_attribute(
                //                 mesh_grob(), where,
                //                 attribute_name, component,
                //                 v, op, value_
                //             );
                //         }
                //     }
                // );

                // pick(raypick, MESH_CELLS, picking_image, x0, y0, width, height);
                // for_each_picked_element(
                //     picking_image, mask,
                //     [&](index_t c) {
                //         for(index_t lv = 0;
                //             lv<mesh_grob()->cells.nb_vertices(c); ++lv
                //         ) {
                //             index_t v = mesh_grob()->cells.vertex(c,lv);
                //             paint_attribute(
                //                 mesh_grob(), where,
                //                 attribute_name, component,
                //                 v, op, value_
                //             );
                //         }
                //     }
                // );
            // }
        // }

        if(autorange_) {
            if(mesh_grob()->get_shader() != nullptr) {
                mesh_grob()->get_shader()->invoke_method("autorange");
            }
        }
    }

}
