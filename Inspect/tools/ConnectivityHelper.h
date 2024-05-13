#ifndef GEOGRAM_CONN
#define GEOGRAM_CONN


#include <geogram/mesh/mesh.h>

#include <cmath>

namespace GEO {

    // HalfedgeHelper allows to walk on cells and neighbors
    struct HalfedgeHelper {

        // max number of face per cell
        static const index_t max_f = 6;
        // max number of halfedge per face
        static const index_t max_h = 4;

        index_t id(index_t c, index_t cf, index_t cfe) const {
            return max_f*max_h*c + max_h*cf + cfe;
        }

        index_t cell() const {
            return h < 0 ? NO_CELL : index_t(h) / (max_f*max_h);
        }

        index_t cell_facet() const {
            return (index_t(h) % (max_f*max_h)) / max_h;
        }

        index_t cell_facet_edge() const {
            return index_t(h) % max_h;
        }

        index_t cell_vertex() const {
            FOR(lvc, 4) if (org() == m.cells.vertex(cell(), lvc)) 
                return lvc;

            geo_assert_not_reached;

            return NO_CELL;
        }

        index_t facet() const { return m.cells.facet(cell(),cell_facet()); }

        HalfedgeHelper(const Mesh& m, int h = -1);

        HalfedgeHelper& operator=(const int new_id) { h = new_id; return *this; }
        HalfedgeHelper& operator=(const HalfedgeHelper& other) { h = other.h; geo_assert(&m==&(other.m));  return *this; }

        index_t h_from_cell_and_vertices(index_t c, index_t org, index_t dest) const;

        index_t facet_nbv() const {
            return m.cells.facet_nb_vertices(cell(), cell_facet());
        }
        
        bool is_valid() const;

        index_t org() const {
            return m.cells.facet_vertex(cell(), cell_facet(), cell_facet_edge());
        }

        index_t dest() const {
            return m.cells.facet_vertex(cell(), cell_facet(), (cell_facet_edge() + 1) % facet_nbv());
        }
        
        HalfedgeHelper next() const;
        HalfedgeHelper prev() const;
        HalfedgeHelper opp_f() const;
        HalfedgeHelper opp_c() const;
        vector<int> halfedges_around_edge() const;

        protected:
            const Mesh& m;
        public:
            int h;
    };
    
    inline bool operator==(const HalfedgeHelper& A, const HalfedgeHelper& B) { return A.h == B.h; }
    inline bool operator!=(const HalfedgeHelper& A, const HalfedgeHelper& B) { return A.h != B.h; }

}

#endif