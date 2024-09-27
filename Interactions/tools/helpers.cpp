#include <OGF/Interactions/tools/helpers.h>
#include <geogram/basic/geometry.h>
#include <array>

namespace GEO {

	double ToolHelpers::segment_distance(vec2 a, vec2 b, vec2 c) {
		vec2 ab = b - a;
		vec2 ac = c - a;
		vec2 bc = c - b;

		double proj0 = dot(ab, ac);

		double t = proj0 / (dot(ab, ab) + 1e-10); // add eps to avoid NaN when length = 0

		vec2 closest_point;
		if (t < 0)
			closest_point = a;
		else if (t > 1)
			closest_point = b;
		else 
			closest_point = a + t * ab;
		
		return distance(c, closest_point);
	}

	double tri_area(vec2 A, vec2 B, vec2 C) {
		return .5*((B.y-A.y)*(B.x+A.x) + (C.y-B.y)*(C.x+B.x) + (A.y-C.y)*(A.x+C.x));
	}

	double tri_unsigned_area(vec2 A, vec2 B, vec2 C) {
		return std::abs(tri_area(A, B, C));
	}

	bool ToolHelpers::is_in_2D_convex_quad(std::array<vec2, 4> points, vec2 p) {
		double quad_area = tri_unsigned_area(points[0], points[1], points[2]) + tri_unsigned_area(points[2], points[3], points[0]);

		double area = 0;
		for (int i = 0; i < 4; i++) {
			vec2 A = points[i];
			vec2 B = points[(i + 1) % 4];
			vec2 C = p;

			area += tri_unsigned_area(A, B, C);
		}
		
		return std::abs(area - quad_area) < 1e-4;
	}
	
}