#ifndef OGF_Interactions_HELPERS
#define OGF_Interactions_HELPERS
#include <geogram/basic/geometry.h>
#include <array>

namespace GEO {
	struct ToolHelpers {
		static double segment_distance(vec2 a, vec2 b, vec2 c);
		static bool is_in_2D_convex_quad(std::array<vec2, 4> points, vec2 p);
	};
}

#endif