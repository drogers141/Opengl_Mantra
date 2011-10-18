/*
 * show_mantra.h
 *
 * Model of Om ma ni pay me hung mantra (mantra of Chenrezig)
 * encircling the seed syllable Hrih above lotus and moon seat.
 *
 * Left click and drag for virtual trackball.
 * Press k for keybindings.
 *  Created on: Aug 8, 2010
 *      Author: drogers
 */

#ifndef SHOW_MANTRA_H_
#define SHOW_MANTRA_H_

#include "mygl.h"
#include "glut_app.h"
#include "animated_syllable.h"
#include "cylinder_model.h"
#include "particles.h"
#include "dr_glm.h"

// glut global functions, can't be in a class
// see show_mantra.cpp for info
void test_display();
int main(int argc, char** argv);

/**
 * Subclass glut trackball app and put whatever globals we can here.
 */
class ShowMantraApp: public DR::GlutTrackballApp {
public:
	ShowMantraApp()
	: NUM_SYLLS(6){}
	ShowMantraApp(int window_width, int window_height,
				int glut_button, int glut_modifier=0);
	~ShowMantraApp();

	//** overridden glut callbacks
//	void mouse_motion(int x, int y);
//	void mouse(int button, int state, int x, int y);
	void idle();
	void display();
	void keyboard(unsigned char key, int x, int y);
	void reshape(int w, int h);

	void init_syllables();
	void map_to_cylinder(bool do_2d_tweak);
	void toggle_normal_display(bool& normal_flag);
	void heads_up_display(bool show_keys, bool show_particles, bool show_framerate);
	void init_syllable(const char *name, Syllable3D*& syll);
	void init();
	// call from exit key - no way to catch if user kills window with regular glut
	void cleanup();
	void test_init();
	void init_shaders();
	void tweak_2d(GLint syll_index, UnitSpace2D& syll_space);
	void draw_seed_syllable(Syllable3D *syll, bool wire=false);
	void draw_debug_syllable(Syllable3D *syll, bool wire);
	void init_single_syll();
	void init_lotus_moon();
	void draw_lotus_moon();

	// lotus and moon seat under syllables
	// glm version
	GLMmodel	*glm_lotus_moon;
	// my version
	DR::DrGlmModel  lotus_moon;

	// color for lotus_moon
	DR::vec4 near_white;

	// first light
	DR::GLlight light0;
	// a second light, maybe not used all the time
	DR::GLlight light1;

	// 'name' of current syllable
	const int NUM_SYLLS; // = 6;
	std::vector<const char*> syllnames; // = { "pay", "ni", "ma", "om", "hung", "may" };

	// syllables on cylinder
	AnimatedSyllable3D  *om, *ma, *ni, *pay, *may, *hung, *hrih;
	vector<Syllable3D*> syllables;


	// syllable rendering
	bool wireframe;
	bool render_debug;

	// debug syllables
	Syllable3D *debug_syll, *single_syll;
	bool use_debug_syll;
	// use a debug shape only
	bool use_debug_shape;
	// debug: just render one syllable
	bool use_single_syll;

	// heads up display
	bool show_particle_ct;
	bool show_beam_ct;

	// Cylinder stuff
	CylinderModel cylinder;
	// debug grid on cylinder -- created in init()
	Grid cyl_grid;
	bool show_grid;
	// tweak the syllables using tweak_2d or not
	bool do_2d_tweak;

	// debug - show normals on all syllables (tweak which normals show
	// in the syllable3d class)
	bool show_normals;
	// if showing normals, show facetnorms,
	bool show_facet_norms;
	// if showing normals, show vert norms
	bool show_vert_norms;

	// particles for animating light from syllables
	int max_particles;
	DR::ParticleSet particles;
	// live particles
	int particle_ct;

	// light beams
	int max_beams;
	DR::LightBeamSet beams;
	// live beams
	int beam_ct;

	// shader stuff
	GLint vert_shader;
	GLint frag_shader;
	GLint shader_prog;
	// uniforms
	GLint loc_front_light_model_product;
	GLfloat front_light_model_product[4];
	// flag to allow disabling of lighting with shader
	GLint loc_disable_lighting;
	//bool disable_lighting = false;
	// flag for turning off shaders
	bool shader_on;

	// not a general thing, just use for when working something out
	// d key toggles
	bool debug;

};


#endif /* SHOW_MANTRA_H_ */
