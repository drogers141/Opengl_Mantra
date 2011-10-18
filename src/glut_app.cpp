/*
 * glut_app.cpp
 *
 * Implementation of GlutApp and GlutTrackballApp.  See glut_app.h.
 *
 *  Created on: Aug 8, 2010
 *      Author: drogers
 */

#include "glut_app.h"

#include <cmath>

using namespace std;
using namespace DR;

GlutApp::GlutApp()
: win_width(100), win_height(100),
  frame(0), last_frame(0), framerate(0), show_framerate(false),
  output_file("gl_info.txt"), curr_time_ms(0), start_time_ms(0),
  headsup_announce("Hit k for Keybindings"), update_headsup_freq(0),
  show_keybindings(false), keybindings_width_right_side(0),
  z_start(0), z_zoom(0), z_zoom_delta(0), z_zoom_max(0) {
	setv(light_model_ambient, 0, 0, 0, 1);
}

GlutApp::GlutApp(int window_width, int window_height)
: win_width(window_width), win_height(window_height),
  frame(0), last_frame(0), framerate(0), show_framerate(false),
  output_file("gl_info.txt"), curr_time_ms(0), start_time_ms(0),
  headsup_announce("Hit k for Keybindings"), update_headsup_freq(0),
  show_keybindings(false), keybindings_width_right_side(0),
  z_start(0), z_zoom(0), z_zoom_delta(0), z_zoom_max(0) {
	setv(light_model_ambient, 0, 0, 0, 1);
}

GlutTrackballApp::GlutTrackballApp()
: GlutApp() {
	set_to_ident(trackball_transform_mat);
	setv(mouse_start_point, 0, 0, 0);
}

/**
 * Set the button and modifier controls trackball with.
 * See set_controller for options.
 */
GlutTrackballApp::GlutTrackballApp(int window_width, int window_height,
		int glut_button, int glut_modifier)
: GlutApp(window_width, window_height) {
	set_to_ident(trackball_transform_mat);
	setv(mouse_start_point, 0, 0, 0);
	set_controller(glut_button, glut_modifier);
}

/**
 * Set the button and modifier controls trackball with.
 * params: glut_button: one of { GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON }
 * 		glut_modifier: one of { 0 [default], GLUT_ACTIVE_CTRL , GLUT_ACTIVE_SHIFT,
 * 								GLUT_ACTIVE_ALT }
 * 			if modifier = 0, the button click alone will activate the trackball
 */
void GlutTrackballApp::set_controller(int glut_button, int glut_modifier) {
	mouse_button = glut_button;
	modifier = glut_modifier;
}

/**
 * Call this in the function passed to glutMotionFunc()
 */
void GlutTrackballApp::mouse_motion(int x, int y) {
	if(!trackball_active) {
		return;
	}
	GLfloat new_point[3];
	map_to_sphere(new_point, x, y);

	GLfloat axis[3];
	cross(axis, mouse_start_point, new_point);
	copyv(mouse_start_point, new_point);

	GLfloat len = magnitude(axis);
	normalize(axis);
	GLfloat angle = ((GLfloat)asin(len));
	angle *= 180/PI;

	if(angle > .00001) {
		GLfloat transform[16];
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRotatef(angle, axis[0], axis[1], axis[2]);
		glGetFloatv(GL_MODELVIEW_MATRIX, transform);

		glPopMatrix();

		GLfloat out[16];
		mult_matrixf(out, trackball_transform_mat, transform);
		copym(trackball_transform_mat, out);

		glutPostRedisplay();
	}

}

/**
 * Call this in the function passed to glutMouseFunc()
 */
void GlutTrackballApp::mouse(int button, int state, int x, int y) {
	// set flag off
	trackball_active = false;

	if(state != GLUT_DOWN) {
		return;
	}
	int mod = glutGetModifiers();
	bool correct_mod = mod & modifier;

	if(correct_mod || modifier==0) {
		trackball_active = button == mouse_button;
	}
	if(trackball_active) {
		map_to_sphere(mouse_start_point, x,y );
	}
}

/**
 * Map mouse coords x, y to point on sphere
 */
void GlutTrackballApp::map_to_sphere(GLfloat out[3], int x, int y) {
	GLfloat _x, _y;
	_x = 2 * (x/(GLfloat)win_width) - 1;
	_y = 2 * ((win_height-y)/(GLfloat)win_height) - 1;

	GLfloat p[3] = {_x, _y, 0};
	GLfloat len = magnitude(p);
	GLfloat v[3];
	if(len > 1) {
		copyv(v, p);
		normalize(v);
	} else {
		v[0] = _x;
		v[1] = _y;
		v[2] = sqrt(1 -len*len);
	}
	copyv(out, v);
}
