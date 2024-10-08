#include "FeatureLinesSetup.h"

#include <stack>

namespace OGF {

    // FeatureLinesSetup manager (global to all)
    std::map<std::string, FeatureLinesSetup*> FeatureLinesSetup::feature_lines_setups;

    FeatureLinesSetup::FeatureLinesSetup(MeshGrob* t_mesh_grob) : mesh_grob(t_mesh_grob) {
        feature_lines_setups[mesh_grob->name()] = this;
    }

    FeatureLinesSetup* FeatureLinesSetup::get(MeshGrob* mesh_grob) {
        if (feature_lines_setups[mesh_grob->name()] == NULL) {
            feature_lines_setups[mesh_grob->name()] = new FeatureLinesSetup(mesh_grob);
        }

        return feature_lines_setups[mesh_grob->name()];
    }

    /**
     * \brief Check if mesh grob has some feature lines in attribute "is_feature"
     */
    bool FeatureLinesSetup::has_features() {
        Attribute<Numeric::uint8> is_feature(
            mesh_grob->facet_corners.attributes(), "is_feature"
        );

        for (index_t i = 0; i < is_feature.size(); i++) {
            if (is_feature[i]) {
                return true;
            }
        }

        return false;
    }

    /**
     * \brief Set attribute "is_feature" according to polyline of mesh grob (if a polyline exist and match with the surface)
     */
    void FeatureLinesSetup::setup_attribute() {

        // Get halfedge is_feature attribute
        Attribute<Numeric::uint8> is_feature(
            mesh_grob->facet_corners.attributes(), "is_feature"
        );

        // Record some connectivity info
        // In order to compute link between edges and corners in linear time
        std::vector<std::vector<index_t>> iter_edges(mesh_grob->vertices.nb());
        std::vector<index_t> edge_to(mesh_grob->edges.nb());
        std::vector<std::vector<index_t>> iter_corners(mesh_grob->vertices.nb());
        std::vector<index_t> corner_to(mesh_grob->facet_corners.nb());
        std::vector<index_t> corner_adj(mesh_grob->facet_corners.nb(), NO_CORNER);

        for (index_t e : mesh_grob->edges) {
            index_t f = mesh_grob->edges.vertex(e, 0);
            index_t t = mesh_grob->edges.vertex(e, 1);

            iter_edges[f].push_back(e);
            edge_to[e] = t;
        }

        for (index_t f : mesh_grob->facets) {
            index_t n_corners = mesh_grob->facets.nb_corners(f);

            for (index_t lc = 0; lc < n_corners; lc++) {
                index_t c = mesh_grob->facets.corner(f, lc);
                index_t nc = mesh_grob->facets.corner(f, (lc + 1) % n_corners);
                index_t f = mesh_grob->facet_corners.vertex(c);
                index_t t = mesh_grob->facet_corners.vertex(nc);

                iter_corners[f].push_back(c);
                corner_to[c] = t;

                // Get opposite corner
                index_t adj_f = mesh_grob->facet_corners.adjacent_facet(c);
                
                if (adj_f != NO_FACET) {
                    // Search adjacent corner
                    index_t iv = mesh_grob->facet_corners.vertex(nc);

                    for (index_t lv = 0; lv < mesh_grob->facets.nb_corners(adj_f); lv++) {
                        index_t cur_c = mesh_grob->facets.corner(adj_f, lv);

                        if (mesh_grob->facet_corners.vertex(cur_c) == iv) {
                            corner_adj[c] = cur_c;
                            break;
                            // index_t adj_c = mesh_grob->facets.corner(adj_f, lc);
                        }
                    }

                    
                }
            }
        }

        // Now, we can search for corner that match with edge to mark feature lines
        for (index_t v : mesh_grob->vertices) {
            for (auto c : iter_corners[v]) {
                for (auto e : iter_edges[v]) {
                    auto ve = edge_to[e];
                    auto vc = corner_to[c];

                    if (ve == vc) {
                        is_feature[c] = true;
                        if (corner_adj[c] != NO_CORNER)
                            is_feature[corner_adj[c]] = true;
                    }
                }
            }
        }
    }

    /**
     * \brief Add a polyline to mesh grob following "is_feature" attribute
     */
	void FeatureLinesSetup::setup_polyline() {

		std::cout << "generate" << std::endl;

        // Get halfedge is_feature attribute
        Attribute<Numeric::uint8> is_feature(
            mesh_grob->facet_corners.attributes(), "is_feature"
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

        if(mesh_grob->vertices.dimension() < 3) {
            Logger::err("Mesh") << "Dimension smaller than 3" << std::endl;
            return;
        }

        index_t off = mesh_grob->edges.create_edges(n_edges);

        // Create an attribute on edge, that map edge to halfedge !
        Attribute<Numeric::uint32> corner_matching(
            mesh_grob->edges.attributes(), "corner_matching"
        );

        int n_edge = 0;

        for (index_t f : mesh_grob->facets) {
            for (index_t fc : mesh_grob->facets.corners(f)) {

                index_t nfc = mesh_grob->facets.next_corner_around_facet(f, fc);

                if (is_feature[fc]) {
                    index_t v0 = mesh_grob->facet_corners.vertex(fc);
                    index_t v1 = mesh_grob->facet_corners.vertex(nfc);

                    const index_t k_edge = off + n_edge;
                    mesh_grob->edges.set_vertex(k_edge, 0, v0);
                    mesh_grob->edges.set_vertex(k_edge, 1, v1);
                    
                    // Map edge to halfedge
                    corner_matching[n_edge] = fc;

                    // Add edge
                    edges.push_back(k_edge);

                    n_edge++;
                }
            }
            
        }
  
        m_is_generated = true;
	}

    /**
     * \brief Clean all feature lines on mesh grob
     */
    void FeatureLinesSetup::clean_polyline() {
        vector<index_t> delete_e(mesh_grob->edges.nb(), 1);
        mesh_grob->edges.delete_elements(delete_e, false);
        mesh_grob->update();
    }

    /**
     * \brief Remove a feature line given its edges
     * \param[in] deleted_edges A collection of edge indexes to remove
     */
    void FeatureLinesSetup::remove(GEO::vector<index_t> deleted_edges) {

        // Remove deleted edges to edges
        GEO::vector<index_t> new_edges;

        for (auto e : edges) {
            bool found = false;
            for (auto de : deleted_edges) {
                if (e == de) {
                    found = true;
                    break;
                }
            }

            if (!found)
                new_edges.push_back(e);
        }

        edges = new_edges;

        // Transform edge indexes collection to a map that define
        // if the edges should be deleted or not at index i
        vector<index_t> delete_e(mesh_grob->edges.nb(), 0);
        for (auto e : deleted_edges) {
            delete_e[e] = 1;
        }

        // Batch delete and update mesh grob
        mesh_grob->edges.delete_elements(delete_e, false);
        mesh_grob->update();
    }

    /**
     * \brief Calls a user-specified function for each edge in a connected
     *  component incident to a given vertex in a feature line
     * \param[in] mesh_grob the mesh
     * \param[in] seed_edge one of the facets of the connected component
     * \param[in] doit the function to be called for each edge of the
     *  connected component incident to \p seed_edge
     */
    void FeatureLinesSetup::for_each_connected_edge_in_feature_line(index_t seed_edge, std::function<bool(index_t)> doit) {
        
        std::vector<bool> visited(mesh_grob->edges.nb(), false);
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

                    if (mesh_grob->edges.vertex(ne, i) == v[i]) {
                        count[i]++;
                        conn_e[i] = ne;
                    }

                }
                
                if (count[i] == 1 && !visited[conn_e[i]]) {
                    // Call doit function and push next edge to the stack
                    if (doit(conn_e[i])) {
                        S.push(conn_e[i]);
                    }
                    visited[conn_e[i]] = true;
                }
            }

        }
    }


}
