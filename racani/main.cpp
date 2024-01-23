#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <iostream>

#include "timing.h"
#include "objloader.h"
#include "spline.h"
#include "part.h"
#include "seaside.h"


constexpr int LAB = 3;

GLuint width = 768, height = 768;
float u = 0;
bspline_t spline;
std::vector<glm::vec3> vertices;
typedef struct _Ociste {
    GLdouble	x;
    GLdouble	y;
    GLdouble	z;
} Ociste;

Ociste	ociste = { 0.0f, 0.0f, 2.0f };

void myDisplay();
void myReshape(int width, int height);
void DrawModel(bool wireframe);
void frame_render();
void idle();
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
glm::vec3 calculate_rot_axis(glm::vec3 starting_orientation);
float calculate_rot_angle(glm::vec3 starting_orientation);
rain_particles_t rain;
balloon_surface_particles_t balloon;
particle_t* cam_particle = nullptr;
glm::vec3 camerashake = { 0,0,0 };
int main(int argc, char** argv)
{
    srand(time(NULL));
    if (LAB == 1) {
		spline.load_points("points.txt");
		spline.generate();
		for (int i = 0; i < spline.control_pts.size(); i++) {
			printf("%f %f %f\n", spline.control_pts[i].x, spline.control_pts[i].y, spline.control_pts[i].z);
		}
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		loadOBJ("model.obj", vertices, uvs, normals);
    }
    else if (LAB == 2) {

    }
    else if (LAB == 3) {
        balloon.generate({0.0f, 3.0f, 0.0f}, 0.3f);
        balloon.water.generate({0.0f, 3.0f, 0.0f}, 0.3f);
        balloon.water.cam_particle = &cam_particle;
        balloon.water.camshake = &camerashake;
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Racani labos");
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_STIPPLE);
   // glEnable(GL_LINE_STIPPLE_PATTERN);
    glutDisplayFunc(myDisplay);
    glutReshapeFunc(myReshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(myKeyboard);
    printf("Tipka: a/d - pomicanje ocista po x os +/-\n");
    printf("Tipka: w/s - pomicanje ocista po y os +/-\n");
    printf("Tipka: r - pocetno stanje\n");
    printf("esc: izlaz iz programa\n");

    glutMainLoop();
    return 0;
}

void myDisplay()
{
    glClearColor(0.0f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    frame_render();
    glutSwapBuffers();
}


void cam_lookat_particle() {
    static glm::vec3 offset = { 0.0, 0.0, 0.0 };
    offset += (camerashake - offset) * dt_get()*2.5f;
    gluLookAt(
        cam_particle->pos.x + offset.x + 0.4,
        cam_particle->pos.y + offset.y + 0.75,
        cam_particle->pos.z + offset.z + 0.1,
        cam_particle->pos.x, cam_particle->pos.y, cam_particle->pos.z,
        0.0, 0.1, 0.0
    );
}

void myReshape(int w, int h)
{
    width = w; height = h;
    //glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-2, 2, -2, 2, 1, 15);
    if (LAB == 3) {
        if (cam_particle != nullptr)
            glFrustum(-.4, .4, -.4, .4, 0.25, 900);
        else
            glFrustum(-1, 1, -1, 1, 1, 900);
    }
    else {
        glFrustum(-1.5, 1.5, -1.5, 1.5, 1.5, 30);
    }
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (cam_particle != nullptr) {
        cam_lookat_particle();
    }
    else {
        gluLookAt(ociste.x, ociste.y, ociste.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);	// ociste x,y,z; glediste x,y,z; up vektor x,y,z
    }
}

void DrawModel(bool wireframe)
{
    glLineWidth(1.0f);
    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < vertices.size(); i++) {
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	}
    glEnd();
}

glm::vec3 calculate_rot_axis(glm::vec3 starting_orientation) {
    return glm::cross(
        starting_orientation, spline.calculate_tangent(u/bspline_t::segment_pointcount)
    );
}

float calculate_rot_angle(glm::vec3 starting_orientation) {
    auto tangent = spline.calculate_tangent(u/bspline_t::segment_pointcount);
    return glm::degrees(glm::acos(
		glm::dot(
            starting_orientation/glm::length(starting_orientation),
            tangent/glm::length(tangent)
        )
      )
    );
}

void DrawFloorGrid() {
    int i;
    glLineWidth(2.0f);
    glColor4f(0.5f, 0.5f, 0.0f,0.1f);
	glBegin(GL_LINES);
    for (i = -10; i <= 10; i++) {
		glVertex3f((float)i, 0.0f, 10.0f); glVertex3f((float)i, 0.0f, -10.0f);
		glVertex3f(10.0f, 0.0f, (float)i); glVertex3f(-10.0f, 0.0f, (float)i);
	}
	glEnd();
}

void DrawFloorWater() {
    //fill the floor
    glColor4f(0.2f, 0.5f, 0.6f, 0.05f);
    constexpr int floor_size = 4;
    glBegin(GL_QUADS);
    glVertex3f(-floor_size, 0.0f, -floor_size);
    glVertex3f(-floor_size, 0.0f, floor_size);
    glVertex3f(floor_size, 0.0f, floor_size);
    glVertex3f(floor_size, 0.0f, -floor_size);
    glEnd();
}

void frame_render()
{

    if (LAB == 1) {
        DrawFloorGrid();
        spline.draw_lines();
        spline.draw_control_pts();
        spline.draw(u, true);
        spline.draw_tangent((float)u, true);
        glPushMatrix();
        glm::vec3 spline_point = spline.curve_pts[u];
        glTranslatef(spline_point.x, spline_point.y, spline_point.z);
        float angle = calculate_rot_angle(glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec3 axis = calculate_rot_axis(glm::vec3(0.0f, 0.0f, 1.0f));
        glRotatef(angle, axis.x, axis.y, axis.z);
        glColor4f(.2f, 0.3f, 0.35f, 0.4f);
        glScalef(0.5f, 0.5f, 0.5f);
        DrawModel(false);
        glColor4f(1.0f, 1.0f, 1.0f, 0.01f);
        DrawModel(true);
        glPopMatrix();
    }
    else if (LAB == 2) {
    //    DrawFloorWater();
        rain.draw();
        rain.ripples.draw();
    } else if (LAB == 3) {
        //draw floor
        glBegin(GL_QUADS);
        glColor4f(0.2f, 0.2f, 0.2f, 1.0f);
        glVertex3f(-5.0f, 0.0f, -5.0f);
        glVertex3f(-5.0f, 0.0f, 5.0f);
        glVertex3f(5.0f, 0.0f, 5.0f);
        glVertex3f(5.0f, 0.0f, -5.0f);
        glEnd();
        balloon.water.flying.draw();
        balloon.water.draw();
		balloon.draw();
	}
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY)
{
    auto timescale = dt_get_timescale();
    switch (theKey)
    {
    case 'a': ociste.x = ociste.x + 0.2f;
        break;
    case 'd': ociste.x = ociste.x - 0.2f;
        break;
    case 'w': ociste.y = ociste.y + 0.2f;
        break;
    case 's': ociste.y = ociste.y - 0.2f;
        break;
    case 'r': ociste.x = 0.0; ociste.y = 0.0;
        break;
    case 27:  exit(0);
        break;
    case 'o':
        rain.ripples.ornamental = !rain.ripples.ornamental;
        break;
    case 'x':
        if (LAB == 3) {
			balloon.drop();
		}
        break;
    case 'p':
        dt_set_timescale(0.0f);
    case 't':
        if (timescale == 1.0f) {
			dt_set_timescale(0.1f);
		} else if (timescale == 0.1f) {
			dt_set_timescale(1.0f);
		}
		break;
    case 'c':
		if (cam_particle == nullptr) {
            cam_particle = &balloon.water.particles[rand()%512];
		} else {
			cam_particle = nullptr;
		}
		break;
    }

    myReshape(width, height);
    glutPostRedisplay();
}

void idle() {
    dt_update();
    if (LAB == 2) {
        int random_chance = rain.ripples.ornamental ? 6 : 25;
        if (rand() % 100 < random_chance) {
            particle_t part;
            part.pos = { (rand() % 100 - 50) / 25.0f, 3, (rand() % 100 - 50) / 25.0f };
            part.vel = { 0.0f, -5.0f, 0.0f };
            part.time_death = glutGet(GLUT_ELAPSED_TIME) + 20000;
            rain.emit(part);
        }
        rain.update();
        rain.ripples.update();
    }
    u++;
    if (u >= spline.curve_pts.size()) u = 0;
    if (LAB == 3) {
        balloon.update();
        balloon.water.update();
        balloon.water.flying.update();
        balloon.update_bb_vectors();
        myReshape(width, height);
    }

    glutPostRedisplay();
}
