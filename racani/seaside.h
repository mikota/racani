#pragma once
#include <vector>
#include <array>
#include <glm/glm.hpp>

struct seaside_quad_t {
	std::array<glm::vec3, 4> vertices;
	glm::vec4 color;
};

extern std::vector<seaside_quad_t> seaside_quads;

void draw_mol();
