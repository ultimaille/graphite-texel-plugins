#ifndef H__OGF_FEATURE_LINES_SETUP__H
#define H__OGF_FEATURE_LINES_SETUP__H

#include <geogram/mesh/mesh_geometry.h>
#include <OGF/mesh/shaders/mesh_grob_shader.h>
#include <OGF/renderer/context/rendering_context.h>
#include <OGF/gom/reflection/meta.h>

namespace OGF {

	struct FeatureLinesSetup {


    	static std::map<std::string, FeatureLinesSetup*> feature_lines_setups;

		public:

		FeatureLinesSetup(MeshGrob *t_mesh_grob);

		static FeatureLinesSetup* get(MeshGrob* mesh_grob);

		bool has_features();

		// Setup polyline on model following feature lines
		void setup_polyline();
		void setup_attribute();

		void clean_polyline();

		// Remove feature line
		void remove(GEO::vector<index_t> deleted_edges);

		void for_each_connected_edge_in_feature_line(index_t seed_edge, std::function<bool(index_t)> doit);

		inline bool is_generated() { return m_is_generated; }

		private:

		MeshGrob* mesh_grob;
		bool m_is_generated = false;
		GEO::vector<index_t> edges;

	};

}

#endif