
#include "ConnectivityHelper.h"

namespace GEO {

    HalfedgeHelper::HalfedgeHelper(const Mesh& m, int h) : m(m), h(h) {}

    index_t HalfedgeHelper::h_from_cell_and_vertices(index_t c, index_t org, index_t dest) const {
	    FOR(cf, m.cells.nb_facets(c)) FOR(cfh, m.cells.facet_nb_vertices(c, cf)) {
            if (m.cells.facet_vertex(c, cf, cfh) == org && m.cells.facet_vertex(c, cf, (cfh + 1) % m.cells.facet_nb_vertices(c, cf)) == dest)
		        return id(c, cf, cfh);
        }
        geo_assert_not_reached;
    }

    bool HalfedgeHelper::is_valid() const {
        if (h < 0) return false;
        if (cell() >= m.cells.nb()) return false;
        if (cell_facet() >= m.cells.nb_facets(cell())) return false;
        if (cell_facet_edge() >= facet_nbv()) return false;

        return true;
    }

    HalfedgeHelper HalfedgeHelper::next() const {
        if (cell_facet_edge() + 1 < facet_nbv()) return HalfedgeHelper(m, h + 1);
        return HalfedgeHelper(m, h - int(cell_facet_edge()));
    }

    HalfedgeHelper HalfedgeHelper::prev() const {
        if (cell_facet_edge() != 0) return HalfedgeHelper(m, h - 1);
        return HalfedgeHelper(m, h + int(facet_nbv()) - 1);
    }

    HalfedgeHelper HalfedgeHelper::opp_f() const {
	    return HalfedgeHelper(m, int(h_from_cell_and_vertices(cell(), dest(), org())));
    }

    HalfedgeHelper HalfedgeHelper::opp_c() const {
        index_t opp_cell = m.cells.adjacent(cell(), cell_facet());

        if (opp_cell == NO_CELL) 
            return HalfedgeHelper(m, -1);

        return HalfedgeHelper(m, int(h_from_cell_and_vertices(opp_cell, dest(), org())));
    }

    vector<int> HalfedgeHelper::halfedges_around_edge() const {

        vector<int> result;
        HalfedgeHelper n_e = *this;
        
        // boundary ? rewind !
        do {
            result.push_back(n_e.h);

            if (!n_e.opp_c().is_valid()) 
                break;

            n_e = n_e.opp_c().opp_f();

        } while (n_e != *this);

        if (n_e.opp_c().is_valid() && n_e == *this) 
            return result;

        //iterate if the edge is on border
        result.clear();

        do {
            result.push_back(n_e.h);
            n_e = n_e.opp_f().opp_c();
        } while (n_e.is_valid());

        return result;

    }

}
