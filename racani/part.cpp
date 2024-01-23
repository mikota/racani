#include "part.h"
#include <GL/glut.h>
#include <iostream>
#include "timing.h"

void particle_system_t::update() {
	for (int i = 0; i < MAX_PARTICLES; i++) {
		if (particles[i].alive) {
			_update(dt_get(), particles[i]);
			if (particles[i].time_death < dt_get_time()) {
				destroy(particles[i]);
			}
		}
	}
}

void particle_system_t::draw() {
	for (int i = 0; i < MAX_PARTICLES; i++) {
		if (particles[i].alive) {
			_draw(particles[i]);
		}
	}
}

particle_t* particle_system_t::emit(particle_t part) {
	if (particle_count < MAX_PARTICLES) {
		for (int i = 0; i < MAX_PARTICLES; i++) {
			if (!particles[i].alive) {
				particles[i] = part;
				particles[i].alive = true;
				particle_count++;
				return &particles[i];
				//std::cout << "emitting particle" << std::endl;
			}
		}
	} else {
		return nullptr;
	}
	std::cerr << "No space for new particle but particle count is " << particle_count << std::endl;
	return nullptr;
}


void rain_particles_t::_update(float dt, particle_t& part) {
	part.pos += part.vel * dt;
	if (part.pos.y < 0.0f) {
		destroy(part);
	//	std::cout << "destroying rain" << std::endl;
		int ripple_count = 32 + ripples.ornamental * 200;
		int pattern = rand() % 32 + 16;
		for (int i = 0; i < ripple_count; i++) {
			constexpr double pi = 3.14159265358979323846;
			particle_t ripple;
			ripple.pos = part.pos;
			float angle = 2 * pi * i / ripple_count;
			ripple.vel = {
				glm::cos(angle),
				0.0f,
				glm::sin(angle)
			};
			ripple.vel *= 1 + sin(angle * pattern) * 2;
			if (!ripples.ornamental)
				ripple.vel *= 1 + (rand() % 100 - 50) / 25;
			ripple.vel *= 0.5f;
			ripple.time_death = dt_get_time() + 1000;
			ripples.emit(ripple);
		}
	}
}
void rain_particles_t::_draw(particle_t& part) {
//	std::cout << "drawing rain" << std::endl;
	glColor4f(0.1f, 0.5f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(part.pos.x, part.pos.y, part.pos.z);
	glVertex3f(part.pos.x, part.pos.y-0.1f, part.pos.z);
	glEnd();
}

void ripple_particles_t::_update(float dt, particle_t& part) {
	part.pos += part.vel * dt;
	part.pos.y = 0.02f * sin(part.pos.z * 10.0f + dt_get_time() / 200.0f);
	part.vel += -part.vel * 3.0f * dt;
}

void ripple_particles_t::_draw(particle_t& part) {
	float alpha = (part.time_death - dt_get_time()) / 1000.0f;
	alpha = glm::clamp(alpha, 0.0f, 1.0f);

	glColor4f(0.4f, 0.75f, 1.0f, alpha);
	glBegin(GL_QUADS);
	glVertex3f(part.pos.x - 0.01f, part.pos.y, part.pos.z - 0.01f);
	glVertex3f(part.pos.x + 0.01f, part.pos.y, part.pos.z - 0.01f);
	glVertex3f(part.pos.x + 0.01f, part.pos.y, part.pos.z + 0.01f);
	glVertex3f(part.pos.x - 0.01f, part.pos.y, part.pos.z + 0.01f);
	glEnd();
}


void balloon_surface_particles_t::generate(glm::vec3 pos, float radius) {
	//sphere surface point generation algorithm
	//taken from https://www.cmu.edu/biolphys/deserno/pdf/sphere_equi.pdf
	constexpr int count = 256;
	constexpr float pi = 3.14159265358979323846;
	float alpha = 4.0f * pi * radius * radius / count;
	float d = glm::sqrt(alpha);
	int m_theta = glm::round(pi / d);
	float d_theta = pi / m_theta;
	float d_phi = alpha / d_theta;
	for (int i = 0; i < m_theta; i++) {
		float theta = pi * (i + 0.5f) / m_theta;
		int m_phi = glm::round(2 * pi * glm::sin(theta) / d_phi);
		for (int j = 0; j < m_phi; j++) {
			float phi = 2 * pi * j / m_phi;
			particle_t part;
			part.pos = pos + glm::vec3(
				glm::sin(theta) * glm::cos(phi),
				glm::cos(theta),
				glm::sin(theta) * glm::sin(phi)
			) * radius;
			part.vel = glm::vec3(0.0f, 0.0f, 0.0f);
			part.time_death = dt_get_time() + 1000000;
			emit(part);
		}
	}
}

void balloon_surface_particles_t::_update(float dt, particle_t& part) {
	static bool destruction_started = false;
	if (!active) return;
	part.pos += part.vel * dt;
	part.vel += glm::vec3(0.0f, -1.581f, 0.0f) * dt;
	part.vel.y += glm::sqrt(part.pos.x*part.pos.x + part.pos.z*part.pos.z + part.pos.y*part.pos.y/10) * dt;

	if (part.pos.y < 0.0f) {
		destroy(part);
		//propagate destruction
		if (destruction_started) return;
		//dt_set_timescale(0.1f);
		destruction_started = true;
		for (auto& part : particles) {
			part.time_death = dt_get_time();
			part.time_death += glm::sqrt(glm::abs(part.pos.y + (rand() % 100 - 50.0f) / 1500))*100;
		}
	}
}

static glm::vec3 bb_up, bb_right;
void particle_system_t::update_bb_vectors() {
	static float modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	bb_up = glm::vec3(modelview[1], modelview[5], modelview[9]);
	bb_right = glm::vec3(modelview[0], modelview[4], modelview[8]);
}

void balloon_surface_particles_t::_draw(particle_t& part) {
	glColor4f(part.pos.x+0.25f , 0.1f, 0.1f, 0.25f);
	constexpr float size = 0.02f;
	glBegin(GL_QUADS);
	glVertex3f(part.pos.x - size*(bb_right.x + bb_up.x), part.pos.y - size*(bb_right.y + bb_up.y), part.pos.z - size*(bb_right.z + bb_up.z));
	glVertex3f(part.pos.x + size*(bb_right.x - bb_up.x), part.pos.y + size*(bb_right.y - bb_up.y), part.pos.z + size*(bb_right.z - bb_up.z));
	glVertex3f(part.pos.x + size*(bb_right.x + bb_up.x), part.pos.y + size*(bb_right.y + bb_up.y), part.pos.z + size*(bb_right.z + bb_up.z));
	glVertex3f(part.pos.x - size*(bb_right.x - bb_up.x), part.pos.y - size*(bb_right.y - bb_up.y), part.pos.z - size*(bb_right.z - bb_up.z));
	glEnd();
}

void balloon_surface_particles_t::drop() {
	active = true;
	water.active = true;
}

void balloon_water_particles_t::generate(glm::vec3 pos, float radius) {
	constexpr int count = 1024;
	for (int i = 0; i < count; i++) {
		glm::vec3 r{ rand() % 1024 - 512, rand() % 1024 - 512, rand() % 1024 - 512 };
		r = glm::normalize(r);
		r *= radius * (rand() % 100) / 100.0f;
		particle_t part;
		part.pos = pos + r;
		part.time_death = dt_get_time() + 1000000;
		emit(part);
	}

}

float randf(float a, float b) {
	return a + (b - a) * (rand() % 1000) / 1000.0f;
}
void balloon_water_particles_t::_update(float dt, particle_t& part) {
	if (!active) return;
	part.pos += part.vel * dt;
	part.vel += glm::vec3(0.0f, -1.581f, 0.0f) * dt;
	part.vel.y += glm::sqrt(part.pos.x*part.pos.x + part.pos.z*part.pos.z + part.pos.y*part.pos.y/10) * dt;
	
	if (*cam_particle == &part) {
		static float next_shake = 0.0f;
		if (dt_get_time() > next_shake) {
			next_shake = dt_get_time() + part.pos.y*10;
			float shake_strength = (3 - part.pos.y) + part.vel.y*part.vel.y;
			shake_strength *= 0.05f;
			*camshake = {
				randf(-shake_strength, shake_strength),
				randf(-shake_strength, shake_strength),
				randf(-shake_strength, shake_strength)
			};
		}
	}
	else if (part.pos.y > 0.0f) {
		//vector to center of balloon
		glm::vec3 to_center = -part.pos;
		to_center.y = 0.0f;
		part.vel += dt * 0.1f * (to_center + glm::vec3{randf(-2.0f, 2.0f), randf(-0.25,0.25), randf(-2.0f, 2.0f)});
	}
	if (part.pos.y < 0.0f) {
		part.pos.y = 0.01f;
		part.vel.x = (rand() % (20) - 10) / 50.0f;
		part.vel.z = (rand() % (20) - 10) / 50.0f;
		part.vel.y = (rand() % 1000) / 1000.0f;
		part.vel.x *= 0.1f + 2*sin(2 * 3.14 * part.pos.x);
		part.vel.z *= 0.1f + 2*cos(2 * 3.14 * part.pos.z);
		part.vel.y *= 0.1f + 2*glm::abs((sin(2 * 3.14 * part.pos.x)*cos(2 * 3.14 * part.pos.z)));
		auto flying_part = flying.emit(part);
		if (*cam_particle == &part) {
			*cam_particle = flying_part;
			flying_part->vel.y = 0.25 + (rand()%100)/250.0f;
			flying_part->vel = glm::normalize(flying_part->vel) * (1.0f + (rand()%100)/100.0f*0.75f);
			flying_part->vel.y += 0.05f;
		}
		destroy(part);
	}
}

void balloon_water_particles_t::_draw(particle_t& part) {
	glColor4f(0.3f, 0.7f, 0.95f, 0.7f);
	constexpr float size = 0.015f;
	glBegin(GL_QUADS);
	glVertex3f(part.pos.x - size * (bb_right.x + bb_up.x), part.pos.y - size * (bb_right.y + bb_up.y), part.pos.z - size * (bb_right.z + bb_up.z));
	glVertex3f(part.pos.x + size * (bb_right.x - bb_up.x), part.pos.y + size * (bb_right.y - bb_up.y), part.pos.z + size * (bb_right.z - bb_up.z));
	glVertex3f(part.pos.x + size * (bb_right.x + bb_up.x), part.pos.y + size * (bb_right.y + bb_up.y), part.pos.z + size * (bb_right.z + bb_up.z));
	glVertex3f(part.pos.x - size * (bb_right.x - bb_up.x), part.pos.y - size * (bb_right.y - bb_up.y), part.pos.z - size * (bb_right.z - bb_up.z));
	glEnd();
}

void flying_water_particles_t::_update(float dt, particle_t& part) {
	part.pos += part.vel * dt;
	part.vel += glm::vec3(0.0f, -1.581f, 0.0f) * dt;

	if (part.pos.y < 0.0f) {
		part.pos.y = -0.001f;
		part.vel.y = 0;
		part.vel.x -= part.vel.x * dt * 5;
		part.vel.z -= part.vel.z * dt * 5;
	}
}

void flying_water_particles_t::_draw(particle_t& part) {
	glColor4f(0.3f, 0.7f, 0.95f, 0.7f);
	constexpr float size = 0.015f;
	glBegin(GL_QUADS);
	glVertex3f(part.pos.x - size * (bb_right.x + bb_up.x), part.pos.y - size * (bb_right.y + bb_up.y), part.pos.z - size * (bb_right.z + bb_up.z));
	glVertex3f(part.pos.x + size * (bb_right.x - bb_up.x), part.pos.y + size * (bb_right.y - bb_up.y), part.pos.z + size * (bb_right.z - bb_up.z));
	glVertex3f(part.pos.x + size * (bb_right.x + bb_up.x), part.pos.y + size * (bb_right.y + bb_up.y), part.pos.z + size * (bb_right.z + bb_up.z));
	glVertex3f(part.pos.x - size * (bb_right.x - bb_up.x), part.pos.y - size * (bb_right.y - bb_up.y), part.pos.z - size * (bb_right.z - bb_up.z));
	glEnd();
}
