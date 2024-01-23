#pragma once
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <utility>
#include <GL/glut.h>
struct particle_t {
	glm::vec3 pos{};
	glm::vec3 vel{};
	bool alive = false;
	float time_death = 0;
};

constexpr int MAX_PARTICLES = 4096;

struct particle_system_t {
	std::array<particle_t, MAX_PARTICLES> particles;
	int particle_count = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	virtual void _update(float dt, particle_t& part) = 0;
	virtual void _draw(particle_t& part) = 0;
	void update();
	void draw();
	particle_t* emit(particle_t part);
	void destroy(particle_t& part) {
		part.alive = false;
		particle_count--;
	}
	static void update_bb_vectors();
};

struct ripple_particles_t : public particle_system_t {
	bool ornamental = true;
	void _update(float dt, particle_t& part) override;
	void _draw(particle_t& part) override;
};

struct rain_particles_t : public particle_system_t {
	ripple_particles_t ripples;
	void _update(float dt, particle_t& part) override;
	void _draw(particle_t&) override;
};

struct flying_water_particles_t : public particle_system_t {
	void _update(float dt, particle_t& part) override;
	void _draw(particle_t& part) override;
};

struct balloon_water_particles_t : public particle_system_t {
	flying_water_particles_t flying;
	particle_t** cam_particle;
	glm::vec3* camshake;
	bool active = false;
	void generate(glm::vec3 pos, float radius);
	void _update(float dt, particle_t& part) override;
	void _draw(particle_t& part) override;
};

struct balloon_surface_particles_t : public particle_system_t {
	balloon_water_particles_t water;
	bool active = false;
	void generate(glm::vec3 pos, float radius);
	void drop();
	void _update(float dt, particle_t& part) override;
	void _draw(particle_t& part) override;
};