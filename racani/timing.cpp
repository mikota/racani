#include "timing.h"
#include <GL/glut.h>
#include <glm/glm.hpp>
int current_time = 0;
float dt = 0;
float timescale = 1.0f;
float gametime = 0.0f;
float dt_get() {
	return dt;
}
void dt_update() {
	int realtime = glutGet(GLUT_ELAPSED_TIME);
	dt = realtime - current_time;
	dt /= 1000.0f;
	dt *= timescale;
	glm::clamp(dt, 1.0f, 100.0f);
	gametime += dt;
	current_time = realtime;
}
void dt_set_timescale(float new_timescale) {
	timescale = new_timescale;
}
float dt_get_timescale() {
	return timescale;
}

float dt_get_time() {
	return gametime*1000;
}

