#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <utility>
struct bspline_t {
	static int segment_pointcount;
	std::vector<glm::vec3> control_pts;
	int segment_n;
	std::vector<glm::vec3> curve_pts;
	std::vector<glm::vec3> curve_tangents;
	bool load_points(const char* path);
	void draw_control_pts();
	void draw_lines();
	void draw(float u, bool color_segments=false);
	void draw_tangent(float u, bool color_segments=false);
	void generate();
	glm::vec3 calculate_point(float u);
	glm::vec3 calculate_tangent(float u);
};
