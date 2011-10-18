/*
 * glut_app.h
 *
 * Defines a GlutApp and GlutTrackballApp.  Wrappers around glut functionality to
 * help manage the explosion of globals.  Note that you still need the functions
 * that glut uses pointers to to be global, and not a member of a class.
 *
 *  Created on: Aug 8, 2010
 *      Author: drogers
 */

#ifndef GLUT_APP_H_
#define GLUT_APP_H_

#include "mygl.h"
#include "dr_util.h"

#include <vector>

namespace DR {


class GlutApp {
public:
	GlutApp();
	GlutApp(int window_width, int window_height);
	virtual ~GlutApp() {}

	// Glut Callback Functions
	// all are do nothing in base class, override as desired
	// call from glutMotionFunc
	virtual void mouse_motion(int x, int y) {}
	// call from glutMouseFunc
	virtual void mouse(int button, int state, int x, int y) {}
	// call from glutIdleFunc
	virtual void idle() {}
	// call from glutDisplayFunc
	virtual void display() {}
	// call from glutTimerFunc
	virtual void timer() {}
	// call from glutKeyboardFunc
	virtual void keyboard(unsigned char key, int x, int y) {}
	// call from glutReshapeFunc
	virtual void reshape(int w, int h) {}

	// ** register more as needed **


	// screen stuff
	GLint win_width;
	GLint win_height;

	// keep track of framerate
	// overall frame count
	long frame;		// = 0;
	// keep track of frame count at last check
	long last_frame; 		// = 0;
	// updated in idle()
	GLfloat framerate; 		// = 0;
	bool show_framerate; 	// = true;
	// framerate stats, samples of framerate taken regularly
	// for now, just sample it when updating headsup display
	// if sampling
	std::vector<GLfloat> framerate_samples;

	// output file for collecting whatever information
	string output_file;

	// general timing - millisecond accuracy
	// use with glutGet(GLUT_ELAPSED_TIME)
	GLfloat curr_time_ms;
	GLfloat start_time_ms;

	// support for heads-up display
	// prompt/announcement to display when not showing anything else
	std::string headsup_announce;		//"Hit k for Keybindings";
	// how often to update heads up display info - secs
	GLfloat update_headsup_freq;   		// = 1.0;
	// keybindings
	bool show_keybindings; 				// = false;
	// allows for displaying 2 columns of keybindings
	std::vector<const char*> keybindings;
	std::vector<const char*> keybindings_right_side;
	// gives setback value for right side of keybindings x start val
	GLint keybindings_width_right_side;

	// light model - ambient lighting
	vec4	light_model_ambient;

	// global z values for zooming in and out using glTranslate
	// as opposed to gluLookAt() method
	// this is the initial z set back
	// ie we start with eye at z = z_start
	GLfloat z_start; 		// = -12.0;  // orig -2.5;
	// z_zoom will be changed with zoom operation
	GLfloat z_zoom; 	// = 0;
	// how much to change by zooming
	GLfloat z_zoom_delta; 	// = .1;
	// max g_zoom can be
	GLfloat z_zoom_max; 	// = -g_z_start;


};

///**
// * Which mouse button and modifier manipulate the trackball.
// */
//enum Button {
//	LEFT, RIGHT
//};
//enum Modifier {
//	NONE, CTRL, SHIFT, ALT
//};

/**
 * Glut app with a virtual trackball.
 */
class GlutTrackballApp: public GlutApp  {
public:
	GlutTrackballApp();

	/**
	 * Set the button and modifier controls trackball with.
	 * See set_controller for options.
	 */
	GlutTrackballApp(int window_width, int window_height,
			int glut_button, int glut_modifier=0);

	virtual ~GlutTrackballApp() {}
	/**
	 * Set the button and modifier controls trackball with.
	 * params: glut_button: one of { GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON }
	 * 		glut_modifier: one of { 0 [default], GLUT_ACTIVE_CTRL , GLUT_ACTIVE_SHIFT,
	 * 								GLUT_ACTIVE_ALT }
	 * 			if modifier = 0, the button click alone will activate the trackball
	 */
	void set_controller(int glut_button, int glut_modifier=0);

	/**
	 * Call this in the function passed to glutMotionFunc()
	 */
	virtual void mouse_motion(int x, int y);
	/**
	 * Call this in the function passed to glutMouseFunc()
	 */
	virtual void mouse(int button, int state, int x, int y);

	/**
	 * In display function, call glMultMatrixf( app.trackball_transform );
	 * To apply the trackball transformation.
	 */
	GLfloat trackball_transform_mat[16];

protected:
	/**
	 * Map mouse coords x, y to point on sphere
	 */
	void map_to_sphere(GLfloat out[3], int x, int y);

	GLfloat mouse_start_point[3];

	int mouse_button;
	int modifier;

	// if trackball is active, mouse_motion will move the trackball
	bool trackball_active;
	void set_trackball_active();

};







} // end DR namespace

#endif /* GLUT_APP_H_ */
