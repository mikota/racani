#include "spline.h"
#include <GL/glut.h>
#include <array>
bool bspline_t::load_points(const char* path) {
	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Couldn't find file.\n");
		return false;
	}

	while (true) {
		glm::vec3 vertex;
		int res = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
		if (res == EOF)
			break;
		control_pts.push_back(vertex);
	}
}

constexpr float stepvalue = 1 / 200.0f;
int bspline_t::segment_pointcount = 200;

constexpr static std::array<glm::vec3, 5> colors{
	glm::vec3{1.0f, 0.2f, 0.2f},
	glm::vec3{0.2f, 1.0f, 0.2f},
	glm::vec3{0.2f, 0.2f, 1.0f},
	glm::vec3{0.2f, 1.0f, 1.0f},
	glm::vec3{1.0f, 0.2f, 1.0f}
};

void bspline_t::generate() {
	int segment_n = control_pts.size() - 3;
	constexpr float stepvalue = 1/200.0f;
	float u = 0.0f;
	while (true) {
		auto p1 = calculate_point(u);
		curve_pts.push_back(p1);
		u += stepvalue;
		if (u > segment_n) {
			break;
		}
	}
}

static constexpr glm::mat4x4 basis_matrix() {
	return glm::mat4x4{ -1, 3, -3, 1, 3, -6, 0, 4, -3, 3, 3, 1, 1, 0, 0, 0 };
}

static constexpr glm::mat4x3 basis_matrix_dt() {
	return glm::mat4x3{ -1, 2, -1, 3, -4, 0, -3, 2, 1, 1, 0, 0};
}

glm::vec3 bspline_t::calculate_point(float u) {
	int segment_index = std::floor(u) + 1;
	float t = u - std::floor(u);
	glm::vec4 T {t*t*t, t*t, t, 1};
	static constexpr glm::mat4x4 B = basis_matrix();
	glm::mat4x3 R = {
		control_pts[segment_index - 1],
		control_pts[segment_index],
		control_pts[segment_index + 1],
		control_pts[segment_index + 2]
	};
	return T / 6.0f * B * glm::transpose(R);
}

glm::vec3 bspline_t::calculate_tangent(float u) {
	int segment_index = std::floor(u) + 1;
	float t = u - std::floor(u);
	static constexpr auto B_dt = basis_matrix_dt();
	glm::vec3 T{ t * t, t, 1 };
	glm::mat4x3 R = {
		control_pts[segment_index - 1],
		control_pts[segment_index],
		control_pts[segment_index + 1],
		control_pts[segment_index + 2]
	};
	return T / 2.0f * B_dt * glm::transpose(R);
}

void bspline_t::draw_control_pts() {
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < control_pts.size(); i++) {
		glVertex3f(control_pts[i].x, control_pts[i].y, control_pts[i].z);
	}
	glEnd();
}

void bspline_t::draw_lines() {
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glLineStipple(10, 0xAAAA);
	glLineWidth(0.5f);
	glBegin(GL_LINES);
	for (int i = 0; i < control_pts.size() - 1; i++) {
		glVertex3f(control_pts[i].x, control_pts[i].y, control_pts[i].z);
		glVertex3f(control_pts[i + 1].x, control_pts[i + 1].y, control_pts[i + 1].z);
	}
	glEnd();
}

void bspline_t::draw(float u, bool color_segments) {
	u /= segment_pointcount;
	int u_segment_index = std::floor(u);

	glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(2.0f);
	glLineStipple(0, 0xFFFF);
	glBegin(GL_LINES);
	for (int i = 0; i < curve_pts.size() - 1; i++) {
		int segment_i = i/segment_pointcount;
		auto color = colors[segment_i%colors.size()];
		float alpha = 0.25f;
		if (color_segments && segment_i == u_segment_index) {
			alpha = 1.0f;
		}
		if (color_segments) glColor4f(color.x,color.y,color.z,alpha);
		glVertex3f(curve_pts[i].x, curve_pts[i].y, curve_pts[i].z);
		glVertex3f(curve_pts[i+1].x, curve_pts[i+1].y, curve_pts[i+1].z);
	}
	glEnd();
}

void bspline_t::draw_tangent(float u, bool color_segments) {
	u /= segment_pointcount;
	int u_segment_index = std::floor(u);
	auto tangent = calculate_tangent(u);
	auto point = calculate_point(u);
	glLineWidth(2.0f);
	glLineStipple(0, 0xFFFF);
	if (color_segments) {
		auto color = colors[u_segment_index % colors.size()];
		glColor4f(color.x, color.y, color.z, 1.0f);
	}
	glBegin(GL_LINES);
	glVertex3f(point.x, point.y, point.z);
	glVertex3f(point.x + tangent.x, point.y + tangent.y, point.z + tangent.z);
	glEnd();
}