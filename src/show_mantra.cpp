/*
 * show_mantra.cpp
 *
 * Model of Om ma ni pay me hung mantra (mantra of Chenrezig)
 * encircling the seed syllable Hrih above lotus and moon seat.
 *
 * Left click and drag for virtual trackball.
 * Press k for keybindings.
 *
 *  Created on: May 31, 2010
 *      Author: drogers
 */

#include "show_mantra.h"
#include "dr_util.h"
#include "vec.h"
#include "animated_syllable.h"
#include "cylinder_model.h"
#include "particles.h"
#include "test.h"


#include <cstdio>
//#include <stdlib.h>
#include <cmath>
#include <cstring>
#include <cassert>
//extern "C" {
//#include "glm.h"
//}
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>


using namespace std;
using namespace DR;


/**
 * Global object wraps all glut functionality, see show_mantra.h, glut_app.*
 */
ShowMantraApp app(1200, 800, GLUT_LEFT_BUTTON);

// for testing
PolyTest polytest;
/**
 * Called when running poly tests
 */
void test_display();

ShowMantraApp::ShowMantraApp(int window_width, int window_height,
		int glut_button, int glut_modifier)
: GlutTrackballApp(window_width, window_height,
		glut_button, glut_modifier), NUM_SYLLS(6) {
	// set up light0 and light1
	light0.id = GL_LIGHT0;
	setv(light0.pos, 0.0f, 0.0f, 15.0f, 0.0f);
	setv(light0.ambient, 0.3f, 0.3f, 0.3f, 1.0f);
	setv(light0.diffuse, 0.7f, 0.7f, 0.7f, 1.0f);
	setv(light0.specular, 1.0, 1.0, 1.0, 1.0f);

	// light 1 is a directional light from above the syllables
	// primarily to give more light to the lotus moon
	light1.id = GL_LIGHT1;
	vec3 epos, wpos = { 0.0f, 10.0f, 0.0f};
	world_to_eye(wpos, epos);
	copyv(light1.pos, epos); light1.pos[3] = 0.0f;
	setv(light1.ambient, 0.15f, 0.15f, 0.15f, 1.0f);
	setv(light1.diffuse, 0.35f, 0.35f, 0.35f, 1.0f);
	setv(light1.specular, 1.0, 1.0, 1.0, 1.0f);
//	setv(light1.ambient, 0.3f, 0.3f, 0.3f, 1.0f);
//	setv(light1.diffuse, 0.7f, 0.7f, 0.7f, 1.0f);
//	setv(light1.specular, 1.0, 1.0, 1.0, 1.0f);

	output_file = "show_mantra_glinfo.txt";

	wireframe = false;
	render_debug = false;

	use_debug_syll = false;
	use_debug_shape = false;
	use_single_syll = false;

	show_particle_ct = false;
	show_beam_ct = false;

	cylinder = CylinderModel(3.5, 3);
	show_grid = false;
	do_2d_tweak = true;

	show_normals = false;
	show_facet_norms = true;
	show_vert_norms = false;

	syllnames.push_back("pay");
	syllnames.push_back("ni");
	syllnames.push_back("ma");
	syllnames.push_back("om");
	syllnames.push_back("hung");
	syllnames.push_back("may");
	assert(NUM_SYLLS == (int)syllnames.size());

	max_particles = 10000; //40000;
	ParticleSet particles;
	particle_ct = 0;
	max_beams = 20000;
	LightBeamSet beams;
	beam_ct = 0;
	shader_on = false;
	debug = false;

	glm_lotus_moon = NULL;
	// lotus moon color
	setv(near_white, 0.973, 0.976, 0.957, 1.0);
}

ShowMantraApp::~ShowMantraApp() {
	cleanup();

}

// toggle normal flag for active syllables, and lotus_moon, see keyboard()
// normal_flag:  one of show_normals, show_facet_norms, show_vert_norms
void ShowMantraApp::toggle_normal_display(bool& normal_flag) {
	normal_flag = !normal_flag;
	if(app.use_debug_syll) {
		app.debug_syll->show_normals = app.show_normals;
		app.debug_syll->show_facet_norms = app.show_facet_norms;
		app.debug_syll->show_vert_norms = app.show_vert_norms;
	} else if(app.use_single_syll) {
		// implement if desired
	} else {
		hrih->show_normals = app.show_normals;
		hrih->show_facet_norms = app.show_facet_norms;
		hrih->show_vert_norms = app.show_vert_norms;
		for (size_t i = 0; i < syllables.size(); ++i) {
			syllables[i]->show_normals = app.show_normals;
			syllables[i]->show_facet_norms = app.show_facet_norms;
			syllables[i]->show_vert_norms = app.show_vert_norms;
		}
		lotus_moon.show_normals = app.show_normals;
		lotus_moon.show_facet_norms = app.show_facet_norms;
		lotus_moon.show_vert_norms = app.show_vert_norms;

	}
}

/**
 * Render 2d heads-up display.
 * params: show_keys - if false, show message
 * 					if true, show keybinding
 * with others, nothing else is displayed if false
 */
void ShowMantraApp::heads_up_display(bool show_keys, bool show_particles,
		bool show_framerate) {
// this setup puts the text up in the top left corner
// with generous spacing given the font,
// allowing (height / line_height) = 21 lines with margin

	GLfloat width = 10.0, height = 10.0;
	GLfloat margin = .2;
	GLfloat line_height = .45;
	GLfloat right_start = 7.0;
	GLint numlines = (int)floor(height / line_height);

	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

//	if(frame % 150 == 0) {
//		cout << "shader_on = " << shader_on << endl;
//	}
	glDisable(GL_LIGHTING);
	// disable shader
	if(shader_on) {
		glUseProgram(0);
	}
	// disable lighting in shader
//	glUniform1i(loc_disable_lighting, 1);
	glDisable(GL_DEPTH_TEST);
    glLineWidth(3);
    glColor3f(1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glTranslatef(margin, height-(margin+line_height), 0);

    glMatrixMode(GL_MODELVIEW);
    // keybinding display
    if(show_keys) {
    	// left side
    	for(size_t i=0; i<app.keybindings.size(); i++) {
    		bitmap_output(0, (-line_height)*i, app.keybindings[i],
    				GLUT_BITMAP_TIMES_ROMAN_24);
    	}
    	// right side
    	for(size_t i=0; i<app.keybindings_right_side.size(); i++) {
    		bitmap_output(right_start, (-line_height)*i, app.keybindings_right_side[i],
    				GLUT_BITMAP_TIMES_ROMAN_24);
    	}

    } else {
    	bitmap_output(0, 0, app.headsup_announce.c_str(),
    			GLUT_BITMAP_TIMES_ROMAN_24);
    }
    // particle/beam count and framerate
    if(show_particles) {
    	// either show particles or beams for now - occupy same text space
    	if(app.show_particle_ct) {
    		char plabel[40];
    		sprintf(plabel, "Particles: %d", particle_ct);
    		bitmap_output(right_start, (-line_height)*(numlines-4), plabel,
    				GLUT_BITMAP_TIMES_ROMAN_24);
    	} else if(app.show_beam_ct) {
    		char blabel[40];
    		sprintf(blabel, "Beams: %d", beam_ct);
    		bitmap_output(right_start, (-line_height)*(numlines-4), blabel,
    				GLUT_BITMAP_TIMES_ROMAN_24);
    	}
    }
    if(show_framerate) {
    	char flabel[40];
    	sprintf(flabel, "Framerate: %.2f", app.framerate);
    	bitmap_output(right_start, (-line_height)*(numlines-3), flabel,
    			GLUT_BITMAP_TIMES_ROMAN_24);
    }

    if(shader_on) {
    	// reenable shader
    	glUseProgram(shader_prog);
    }
	// reenable lighting in shader
//	glUniform1i(loc_disable_lighting, 0);
//	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

// apply tweaks to 2d unit space for each syllable
// ie scale, translate, etc
void ShowMantraApp::tweak_2d(GLint syll_index, UnitSpace2D& syll_space) { // { "pay", "ni", "ma", "om", "hung", "may" };
	switch(syll_index) {
	case 4: // hung
		// compress in x dir -- ie squeeze it
		syll_space.scale(.95, 0);
		syll_space.translate(0.15, -.075);
		break;
	case 5: // may
		syll_space.scale(.72);
		syll_space.translate(0.17, 0.18);
		break;
	case 0: // pay
		syll_space.scale(.9);
		syll_space.translate(0.085, -0.22);
		break;
	case 1: // ni
		// compress in x dir
		syll_space.scale(.75, 0);
		// stretch in y a bit
		syll_space.scale(1.3, 1);
		syll_space.translate(0.16, -0.3);
		break;
	case 2: // ma
		syll_space.scale(.68);
		syll_space.translate(0.17, -0.0);
		break;
	case 3: // om
		syll_space.scale(.95);
		syll_space.translate(0.0, 0.12);
		break;
	}
}

/**
 * initialize the lotus flower and moon seat below mantra
 */
void ShowMantraApp::init_lotus_moon() {
	char lotus_moon_obj[80] = "data/lotus_moon_seat.obj";
	glm_lotus_moon = glmReadOBJ(lotus_moon_obj);
//	glmUnitize(lotus_seat_model);
	glmFacetNormals(glm_lotus_moon);
	glmVertexNormals(glm_lotus_moon, 90.0);
	// init my model
	lotus_moon.init(glm_lotus_moon);
	copyv(lotus_moon.ambient_diffuse, near_white, 4);
	copyv(lotus_moon.specular, near_white, 4);
	lotus_moon.shininess[0] = 128.0;
//	copyv(lotus_moon.emissive, near_white, 4);
	setv(lotus_moon.emissive, 0.0, 0.0, 0.0, 1.0);
	// set length to draw normals for the lotus moon
	// (default is too long because of huge triangles in model)
	lotus_moon.draw_normals_length = .1;
}
/**
 * render the lotus flower and moon seat below mantra
 */
void ShowMantraApp::draw_lotus_moon() {
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_NORMALIZE);
	glPushMatrix();
	glTranslatef(0.0, -2.5, 0.0);
	glRotatef(30, 0, 1, 0);
	glScalef(1.6, 1.6, 1.6);

	if(wireframe) {
		GLint curr_prog;
		glGetIntegerv(GL_CURRENT_PROGRAM, &curr_prog);
		glUseProgram(0);
		lotus_moon.render(wireframe);
		glUseProgram(curr_prog);
	} else {
		lotus_moon.render(wireframe);
	}

	glPopAttrib();
	glPopMatrix();

	// if we want to check out rendering raw glm model
//	glPushMatrix();
//	glPushAttrib(GL_ALL_ATTRIB_BITS);
//	glLineWidth(1);
//	GLfloat a[4 ]= {1.0, 1.0, 1.0, 1.0};
//	GLfloat d[4] = {1.0, 1.0, 1.0, 1.0};
//	GLfloat s[4] = {1.0, 1.0, 1.0, 1.0};
//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, a);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, d);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
//
//	//	glScalef(3.0, 3.0, 3.0);
//	glTranslatef(0.0, -3.5, 0.0);
//
//	for (GLuint p=0; p < glm_lotus_moon->numtriangles; p++) {
//		GLMtriangle *f=&(glm_lotus_moon->triangles[p]);
//
//		glBegin(GL_TRIANGLES);
////		glBegin(GL_LINE_LOOP);
//
//		glNormal3fv ( &(glm_lotus_moon->facetnorms[f->findex * 3]) );
//
//		for (int i=0; i<3; ++i) {
//			GLfloat *pos = & ( glm_lotus_moon->vertices[f->vindices[i] * 3] );
//			GLfloat *n = & ( glm_lotus_moon->vertices[f->nindices[i] * 3] );
////			glNormal3fv(n);
//			glVertex3fv ( pos );
//		}
//
//		glEnd();
//	}
//	glPopAttrib();
//	glPopMatrix();
}

// initialize a syllable to syllable 'name'
// name: { om, ma, ni, pay, may, hung, hrih }
void ShowMantraApp::init_syllable(const char *name, Syllable3D* &syll) {
	char filename[80];
	sprintf(filename, "data/%s.obj", name);
	cout << "** init_syllable: creating " << name << " from " << filename << endl;
	if(syll != NULL) {
		delete syll;
	}
	syll = new Syllable3D();
	syll->initFromObj(filename);
	syll->init_base();
	syll->extrude(0.2, true);
}

// initialize the syllables, transforming the mantra syllables
// with a cylinder model
void ShowMantraApp::init_syllables() {
	if(!syllables.empty() ) {
		for (size_t i = 0; i < syllables.size(); ++i) {
			delete syllables[i];
		}
		syllables.clear();
	}
	// adjustable colors for syllables
	vec3 white, green, yellow, blue, red, black;
	GLfloat *colors[] = {blue, yellow, green, white, black, red};
	setv(white, 0.973, 0.976, 0.957); // 0.973, 0.961, 0.949  1.0, 1.0, 1.0);
	setv(green, .388, .718, .267);  // 0.0, 1.0, 0.0);
	setv(yellow, .984, .796, .235);  // 1.0, 1.0, 0.0);
	setv(blue, .169, .318, .616);  // 0.0, 0.0, 1.0);
	setv(red, 0.929, 0.184, 0.141); // 1.0, .047, .071);  // 1.0, 0.0, 0.0);
	setv(black, .114, .047, .071);  // 0.0, 0.0, 0.0);

	// seed syllable is not part of the cylinder
	delete hrih;
	hrih = new AnimatedSyllable3D();
	copyv(hrih->ambient_diffuse, white);
	copyv(hrih->specular, white);
//	copyv(hrih->emissive, white);
	cout << "********* seed syllable: hrih **********" << endl;
	hrih->initFromObj("data/hrih.obj");
	hrih->init_base();
//	cout << "done hrih init_base" << endl;
	//***debug
//	cout << "******* hrih after init_base ************" << endl;
//	hrih->print_polygons(Syllable3D::BASE, 30);
//	cout << "******* end hrih after init_base ************" << endl;

	hrih->extrude(0.25, true);
	hrih->check_normals();
	cout << "** hrih: polygons: " << hrih->num_polygons() << endl;

	// create mantra syllables
	om = new AnimatedSyllable3D(); ma = new AnimatedSyllable3D(); ni = new AnimatedSyllable3D();
	pay = new AnimatedSyllable3D(); may = new AnimatedSyllable3D(); hung = new AnimatedSyllable3D();
	// want this order: { "pay", "ni", "ma", "om", "hung", "may" };
	syllables.push_back(pay);
	syllables.push_back(ni);
	syllables.push_back(ma);
	syllables.push_back(om);
	syllables.push_back(hung);
	syllables.push_back(may);

	assert(NUM_SYLLS == (int)syllables.size());

	// set material properties and add to syllables[]
	for (int i = 0; i < NUM_SYLLS; ++i) {
		Syllable3D *s = syllables[i];
		GLfloat *c = colors[i];
		// in this case, add a specular component that is the
		// same color as the letter
		copyv(s->ambient_diffuse, c);
		copyv(s->specular, c);
//		copyv(s->emissive, c);
	}

	for (size_t s = 0; s < syllables.size(); ++s) {
		Syllable3D *syll = syllables[s];
//		cout << "pointer before: " << s << "  " << syll << endl;
	}
	char filename[80];
	for (size_t s = 0; s < syllables.size(); ++s) {
		Syllable3D *syll = syllables[s];
//		cout << "pointer syll: " << syll << endl;
		cout << "***** syllable: " << syllnames[s] << " *****" << endl;
		// the name of the syllable indicates the correct file
		sprintf(filename, "data/%s.obj", syllnames[s]);
		syll->initFromObj(filename, true);
		syll->init_base();
	}
	// ready to map sylls to cylinder
	map_to_cylinder(do_2d_tweak);

}

/**
 * transforms syllables in mantra by mapping them
 * to their location on the cylinder, then extruding
 * param: do_2d_tweak - if true calls tweak_2d() for sylls
 * pre: syllables have initialized their bases
 */
void ShowMantraApp::map_to_cylinder(bool do_2d_tweak) {
	cylinder.clear();
	for (size_t s = 0; s < syllables.size(); ++s) {
		Syllable3D *syll = syllables[s];
//		cout << "mapping to cylinder:  pointer syll: " << syll << endl;
//		cout << "***** syllable: " << g_syllnames[s] << " *****" << endl;

		int num_vertices = syll->num_vertices_base;
		UnitSpace2D syll_space;
		//** debug
//		vector<UnitSpace2D> empty_spaces(5);
		for (int i = 0; i < /*100;*/ num_vertices; ++i) {
			vec3 v3;
			syll->get_vert(i, v3);
//			cout << "vec3 = " << stringv(v3) << endl;
			syll_space.element.add_vert( Vec(v3) );
		}
		syll_space.element.map(1);
		// flip in y direction
		syll_space.flip(1);
		// apply whatever other tweaks to unit space for
		// the specific syllable
		if(do_2d_tweak) {
			tweak_2d(s, syll_space);
		}
//		cout << "num_vertices: " << num_vertices << endl;
//		cout << "syll_space.element.verts2d.size(): "
//				<< syll_space.element.verts2d.size() << endl;
		// syll_index is the same as the syllable's index in syllables
		// so don't need it for now
		int syll_index = cylinder.add_unit_space(syll_space);
	}
//	cout << "syllables.size(): " << syllables.size() << endl;
	// map the new 3d verts and normals
	cylinder.map();
//	cout << "cylinder map done" << endl;
	// copy them to the syllables and extrude
	for (size_t s = 0; s < syllables.size(); ++s) {
		Syllable3D *syll = syllables[s];
		int num_vertices = syll->num_vertices_base;

		// copy transformed vertices and normals into syllable
		vec3 *syll_verts = NULL, *syll_norms = NULL;
		syll_verts = syll->get_vertices();
		syll_norms = syll->get_normals();
		//** debug
//		int printed_syll = 0;
//		if(s == printed_syll) {
//			cout << "**** syll: " << g_syllnames[s] << " *****" << endl;
//			cout << " first 50 norms and verts from cyl model: " << endl;
//		}
		for (int i = 0; i < num_vertices; ++i) {
			Vec vert = cylinder.vertex_sets[s][i];
			Vec norm = cylinder.normal_sets[s][i];
			//**debug continued from above
//			if(s == printed_syll && i < 50) {
//				cout << i << " vert: " << vert << "  norm: " << norm << endl;
//			}
			vert.array_out(syll_verts[i]);
			norm.array_out(syll_norms[i]);

		}
		//**debug continued from above
//		if(s == printed_syll) {
//			cout << " first 50 norms and verts actual: " << endl;
//			for (int i = 0; i < 50; ++i) {
//				GLfloat *v, *n;
//				v = syll_verts[i];
//				n = syll_norms[i];
//				cout << i << " vert: " << stringv(v) << "  norm: "
//						<< stringv(n) << endl;
//			}
//		}
		syll->reset_base_windings();
//		//***debug
//		continue;
		GLfloat thickness = 0.25;
		syll->extrude(thickness, false);
		// adjust vertex normals
		syll->average_vertex_normals();
		// sanity check on normals
		syll->check_normals();

		// ad hoc - set another center
		vec3 c;
		cylinder.map_point(s, .5, .5, c);
		Vec cent(c);
		Vec radial(cent.x, 0, cent.z);
		cent = cent + radial.unit_vec() * ( .5 * thickness);
		cent.array_out(c);
		copyv(syll->assigned_center, c);

		cout << "** finished: " << syllnames[s] << ": polygons: " <<
				syll->num_polygons() << endl;// << "**" << endl;
	}
}
// debug - work with single syllable on cylinder
void ShowMantraApp::init_single_syll() {
	single_syll = new AnimatedSyllable3D();
	single_syll->initFromObj("data/om.obj", false);
	single_syll->init_base();
//	return;

	int num_vertices = single_syll->num_vertices_base;
	CylinderModel cylinder(3, 3);
	UnitSpace2D syll_space;
	vector<UnitSpace2D> empty_spaces(6);
	for (int i = 0; i < /*100;*/ num_vertices; ++i) {
		vec3 v3;
		single_syll->get_vert(i, v3);
//		cout << "vec3 = " << stringv(v3) << endl;
		syll_space.element.add_vert( Vec(v3) );
	}
	syll_space.element.map(1);
	// invert in y direction
	syll_space.flip(1);

	cout << "num_vertices: " << num_vertices << endl;
	cout << "syll_space.element.verts2d.size(): "
			<< syll_space.element.verts2d.size() << endl;
//	for (int i = 0; i < syll_space.element.verts2d.size(); ++i) {
//		cout << i << "  " << stringv(syll_space.element.verts2d[i], 2) << endl;
//	}

	//	for
//	for (int i = 0; i < syll_space.element.verts3d.size(); ++i) {
//		cout << i << "  " << *syll_space.element.verts3d[i] << endl;
//	}
//	return;

//	g_render_debug = true;
	int syll_index;
	for (size_t i = 0; i < empty_spaces.size(); ++i) {
		if(i == 4) {
			syll_index = cylinder.add_unit_space(syll_space);
		} else {
			cylinder.add_unit_space(empty_spaces[i]);
		}
	}
//	return;
	cylinder.map();

	// copy transformed vertices and normals into syllable
	vec3 *syll_verts = NULL, *syll_norms = NULL;
	syll_verts = single_syll->get_vertices();
	syll_norms = single_syll->get_normals();
	for (int i = 0; i < num_vertices; ++i) {
		Vec vert = cylinder.vertex_sets[syll_index][i];
		Vec norm = cylinder.normal_sets[syll_index][i];
//		cout << i << " vert: " << vert << "  norm: " << norm << endl;

		vert.array_out(syll_verts[i]);
		norm.array_out(syll_norms[i]);

	}
	cout << "**************** actual vertices ***************" << endl;
	for (int i = 0; i < num_vertices; ++i) {
		vec3 v;
		single_syll->get_vert(i, v);
//		cout << i << "  " << stringv(v) << endl;
	}

	single_syll->extrude(0.25);
}

void ShowMantraApp::init_shaders() {
	vert_shader = glCreateShader(GL_VERTEX_SHADER);
	frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	string shader_dir = "shaders/";
	string vert_name, frag_name;
	// shaders for 1 light
//	vert_name = shader_dir + "vert.fp";
//	frag_name = shader_dir + "frag.fp";

	// shaders for 2 directional lights
//	vert_name = shader_dir + "directional_lights.vp";
//	frag_name = shader_dir + "directional_lights.fp";

	// shaders for 2 directional lights with per pixel lighting
	vert_name = shader_dir + "directional_lights_per_pixel.vp";
	frag_name = shader_dir + "directional_lights_per_pixel.fp";

	load_shader_src(vert_shader, vert_name.c_str() );
	load_shader_src(frag_shader, frag_name.c_str() );
	compile_shader(vert_shader);
	compile_shader(frag_shader);

	shader_prog = glCreateProgram();
	glAttachShader(shader_prog, vert_shader);
	glAttachShader(shader_prog, frag_shader);
	glLinkProgram(shader_prog);

	GLint status;
	glGetProgramiv(shader_prog, GL_LINK_STATUS, &status);
	if(!status) {
		cout << "Shader link error.." << endl;
	}

	// uniforms
	loc_front_light_model_product = glGetUniformLocation(shader_prog,
			"FrontLightModelProduct");
	// to turn off lighting in shader, use the actual location of
	// the boolean flag in the shader itself
	loc_disable_lighting = glGetUniformLocation(shader_prog,
			"DisableLighting");
	glUseProgram(shader_prog);
	shader_on = true;
}

void ShowMantraApp::init() {

	time_t now;
	time(&now);
	cout << "Start time: " << ctime(&now) << endl;
	// glut app init

	// zoom controls
	z_start = -12;
	z_zoom = 0;
	z_zoom_delta = .1;
	z_zoom_max = -z_start;

	// global ambient light
	setv(light_model_ambient, .2, .2, .2, 1);

	// start the clock
	curr_time_ms = start_time_ms = glutGet(GLUT_ELAPSED_TIME);

	// heads up display init
	show_keybindings = false;
	show_framerate = true;
	headsup_announce = "Hit k for Keybindings";
	// how often to update framerate, etc
	update_headsup_freq = 1.0;

	// init keybindings string
	keybindings.push_back("Key Bindings:");
	keybindings.push_back("esc  quit");
	keybindings.push_back("k    toggle showing this message" );
	keybindings.push_back("+/-  zoom in/out");
	keybindings.push_back("w    toggle using wireframe");
	keybindings.push_back("g    toggle showing grid");
	keybindings.push_back("t    toggle showing syllables on");
	keybindings.push_back("     cylinder without 2d tweaks");
	keybindings.push_back("n    toggle showing normals");
	keybindings.push_back("If normals are shown:");
	keybindings.push_back("f    toggle showing facet normals [default]");
	keybindings.push_back("v    toggle showing vertex normals");
//	keybindings.push_back("Move light in x, y or z direction:");
//	keybindings.push_back("x,y,z    positive");
//	keybindings.push_back("X,Y,Z    negative");
	keybindings.push_back("s    toggle shader off/on");
	keybindings.push_back("shader is:");
	// default: shader is on
	keybindings.push_back("on");

	keybindings_right_side.push_back("p    emit particles from");
	keybindings_right_side.push_back("     selected syllable");
	keybindings_right_side.push_back("0-5  select syllable");

	int major, minor;
	get_gl_version( &major, &minor );
	printf ( "OpenGL version %d.%d\n", major, minor );
	get_glsl_version( &major, &minor );
	printf ( "OpenGL Shading Language version %d.%d\n\n", major, minor );
	cout << "CLOCKS_PER_SEC: " << CLOCKS_PER_SEC << endl;

	GLInfo gl_info;
	gl_info.get_info();
	gl_info.print(output_file.c_str());
	cout << "*** OpenGL info written to " << output_file << endl;

//	glClearColor ( 0.0, 0.0, 0.0, 1.0);
	glClearColor ( .2, .2, .2, 1.0);
//	glClearColor ( .3, .3, .3, 1.0);
	
	glEnable ( GL_DEPTH_TEST );
	
	glEnable(GL_POINT_SMOOTH);
//	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_NORMALIZE);
	
	//*** lighting taken from interactive instances program to get started
	glEnable(GL_LIGHTING);
//	glEnable(GL_BLEND);
	// set ambient lighting
//	setv(g_ambient, .2, .2, .2); //1, 1, 1); //.2, .2, .2);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);

//	glShadeModel(GL_SMOOTH);

	// Setup and enable lights 0 and 1
	glEnable(light0.id);
	light0.set_opengl_state();
	glEnable(light1.id);
	light1.set_opengl_state();

	// build grid from cylinder, available via keyboard
	cylinder.grid(cyl_grid, 4, 12);

	// initialize particles' size
//	particles.init(max_particles);

	// init light beams
	beams.init(max_beams);


	// ** normal execution - create seed syllable and mantra syllables
	// initialize syllables
	GLfloat before_ms = glutGet(GLUT_ELAPSED_TIME);
	init_syllables();

	GLfloat after_ms = glutGet(GLUT_ELAPSED_TIME);
	cout << "**** Syllables Initialized: ****" << endl;
	GLfloat elapsed = 0.001 * (after_ms - before_ms);
	cout << "**   " << elapsed << " s  **" << endl << endl;

	//** debug - just create and display one syllable on cylinder
//	init_single_syll();
//	use_single_syll = true;

//	g_render_debug = true;

	init_shaders();
	cout << "** init shaders" << endl;

	// init the extra light model stuff for the shader
	// this is taken from the approach used in the orange book
//#   This gets the contribution of material + environment not
//#   obtained from the other FrontMaterial properties
//#   FrontMaterial.emissive + FrontMaterial.ambient * LightModel.ambient
//    frontLightModelProduct = (0 + can.material_props['GL_AMBIENT']
//                              * glGetFloatv(GL_LIGHT_MODEL_AMBIENT))

	//**** big assumption:  this is consistent for all syllables
	//** for now this is true, otherwise have to update as necessary
	//** at render time
	//*** UPDATED NOTE
	//*** need to fix the whole front light model product deal -- adds too much light
	//*** for now it's disregarded in shader anyway
//	GLfloat *front_emissive = om->emissive;
//	GLfloat *front_ambient = om->ambient_diffuse;
	vec4 front_emissive = {0, 0, 0, 1};
	vec4 front_ambient = {1, 1, 1, 1};
	GLfloat *ambient = light_model_ambient;
	for (int i = 0; i < 4; ++i) {
		front_light_model_product[i] = front_emissive[i] +
				front_ambient[i] * ambient[i];
	}
	cout << "front_light_model_product: "
			<< stringv(front_light_model_product, 4) << endl;
	glUniform4fv(loc_front_light_model_product, 1,
			front_light_model_product);

	// set lighting enabled
	glUniform1i(loc_disable_lighting, 0);

	// set starting time for particles
	particles.old_time_ms = glutGet(GLUT_ELAPSED_TIME);

	init_lotus_moon();
	cout << "*** end init" << endl;
}

/**
 * Things to do after glut main loop.
 */
void ShowMantraApp::cleanup() {
	// get statistics from framerate distribution
	if(framerate_samples.empty()) {
		cout << "no stats to report" << endl;
		return;
	}
	vector<GLfloat>::iterator it;
	GLint numsamples = framerate_samples.size();
	sort(framerate_samples.begin(), framerate_samples.end());
	GLfloat minfr, maxfr, avgfr, sumfr;
	minfr = framerate_samples[0];
	maxfr = framerate_samples[numsamples-1];
	sumfr = accumulate(framerate_samples.begin(), framerate_samples.end(), 0);
	avgfr = sumfr/numsamples;
	cout << "******* Framerate Stats ********" << endl;
	cout << "min: " << minfr << endl << "max: " << maxfr << endl << "avg: " << avgfr << endl;
	cout << "num samples: " << numsamples << endl;

	// simple histogram of distribution
	int numbins = 5;
	vector<GLint> bins(numbins, 0);
	int which = 0;
	GLfloat bin_spread = (maxfr - minfr)/ numbins;
//	cout << "bin_spread: " << bin_spread << endl;
	for (int i = 0; i < numsamples; ++i) {
		if( framerate_samples[i] > minfr + (which+1)*bin_spread ) {
			which++;
		}
		bins[which]++;
	}
	// calculate ratio of each bin to sum for cheap graphics
	GLfloat total = (GLfloat)accumulate(bins.begin(), bins.end(), 0);
//	cout << "total = " << total << endl;
//	cout << "bins:" << endl;
//	for (size_t i = 0; i < bins.size(); ++i) {
//		cout << i << ": " << bins[i] << endl;
//	}
	cout << "Distribution:" << endl
			<< setw(10) << left << "Range" << "Count   Graph" << endl;
	cout << fixed << right;
	cout << setw(6) << "0.0" << endl;
	for (size_t i = 0; i < bins.size(); ++i) {
		GLfloat bin_part = (GLfloat)bins[i] / total;
		cout << setw(6) << setprecision(1) << minfr + (i+1)*bin_spread
				<< setw(8) << bins[i] << "    ";
		// output horizontal bar graph of chars
		// 50 chars would equal 100 %
		GLint numchars = (GLint)(bin_part * 50);
		for (int j = 0; j < numchars; ++j) {
			cout << '#';
		}
		cout << endl;
	}
	cout << endl;
	time_t now;
	time(&now);
	cout << "End time: " << ctime(&now) << endl;

	// append all to output_file
	ofstream out;
	out.open(output_file.c_str(), ios_base::app);
	out << endl;
	out << "******* Framerate Stats ********" << endl;
	out << "min: " << minfr << endl << "max: " << maxfr << endl << "avg: " << avgfr << endl;
	out << "num samples: " << numsamples << endl;
	out << "Distribution:" << endl
			<< setw(10) << left << "Range" << "Count" << endl;
	out << fixed << right;
	out << setw(6) << "0.0" << endl;
	for (size_t i = 0; i < bins.size(); ++i) {
		GLfloat bin_part = (GLfloat)bins[i] / total;
		out << setw(6) << setprecision(1) << minfr + (i+1)*bin_spread
				<< setw(8) << bins[i] << "    ";
		GLint numchars = (GLint)(bin_part * 50);
		for (int j = 0; j < numchars; ++j) {
			out << '#';
		}
		out << endl;
	}
	out << endl;
	out << "End time: " << ctime(&now) << endl;
	out.close();
}

// draw the seed syllable in the center on a vertical plane facing first
// letter of mantra (om in this case)
// param: wire - [false] if true render wireframe
void ShowMantraApp::draw_seed_syllable(Syllable3D *syll, bool wire) {
	GLfloat *color = Util::white;
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
//	glTranslatef(0.25, 0.2, 0.0); // orig
	glTranslatef(0.25, 0.2, -0.5);
	glRotatef(-30, 0, 0, 1);

	glScalef(2.5, 2.5, 2.5);

//	g_render_debug = false;
	if(render_debug) {
		syll->render_debug();
	} else {
		if(wire) {
			// disable lighting in shader
//			glUniform1i(loc_disable_lighting, 1);
			GLint curr_prog;
			glGetIntegerv(GL_CURRENT_PROGRAM, &curr_prog);
			glUseProgram(0);
			syll->render_wire(color);
//			glUniform1i(loc_disable_lighting, 0);
			glUseProgram(curr_prog);
		} else {
			syll->render(color);
		}
	}

	glPopMatrix();
}

void ShowMantraApp::draw_debug_syllable(Syllable3D *syll, bool wire) {
	GLfloat *color = Util::white;
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
//	glRotatef(-20, 0, 0, 1);
//	glTranslatef(0.5, 0, 0);
	glScalef(3.0, 3.0, 3.0);
	if(render_debug) {
		syll->render_debug();
	} else {
		if(wire) {
			syll->render_wire(color);
		} else {
			syll->render(color);
		}
	}

	glPopMatrix();
}

// unit box in xz plane (ie between -1 and 1 in x,z)
//void draw_xz_box() {
//	glPushAttrib(GL_ALL_ATTRIB_BITS);
//	glColor3fv(Util::blue);
//	glBegin(GL_LINE_LOOP);
//		glVertex3f(1, 0, 1);
//		glVertex3f(1, 0, -1);
//		glVertex3f(-1, 0, -1);
//		glVertex3f(-1, 0, 1);
//	glEnd();
//	glPopAttrib();
//}

/**
 * idle func to be called by glut's idle()
 */
void ShowMantraApp::idle() {
	frame++;
	GLfloat time_ms = glutGet(GLUT_ELAPSED_TIME);
	GLfloat elapsed_secs = 0.001 * (time_ms - curr_time_ms);
//	if (frame % 100 == 0) {
//		cout << "time: " << time_ms << ", elapsed: " << elapsed_secs << ", frame: " << frame << endl;
//	}

	if(elapsed_secs >= update_headsup_freq) {
		// update framerate and live particle count
		long elapsed_frames = frame - last_frame;
		framerate = elapsed_frames/elapsed_secs;

		particle_ct = particles.live_particles();
		beam_ct = beams.live_beams();

		curr_time_ms = time_ms;
		last_frame = frame;
		// sample framerate starting from 5 secs after prog start
		if(time_ms > 5000) {
			framerate_samples.push_back(framerate);
		}
	}

	// particles stuff
	if( !particles.is_empty() ) {
		particles.update( glutGet(GLUT_ELAPSED_TIME) );
	}
	if( !beams.is_empty() ) {
		beams.update( glutGet(GLUT_ELAPSED_TIME));
	}
	// debug
	if(debug && frame % (long)framerate == 0) {
//		cout << "live beams: " << beams.live_beams() << endl;
//		beams.print_live_beams(1);
		if(beams.live_beams() > 0) {
			LightBeam b;
			int index = beams.get_live_beam(0, b);
			if(index > -1) {
				cout << "beams[" << index << "]: " << b << endl
						<< "color: " << stringv(b.color, 4) << endl;
			}
		}
	}
	glutPostRedisplay();
}

void ShowMantraApp::display() {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glPushMatrix();

	// apply zoom, trackball transformation
	glTranslatef(0, 0, z_zoom);
	glMultMatrixf(trackball_transform_mat);

	// this takes it from obj up to opengl up
//	glRotatef ( 90, 1, 0, 0 );
//	glRotatef(180, 0, 0, 1);

	// should not be nec
	GLfloat *color = Util::white;

	// various render possibilities -- mostly debug
	if(use_debug_shape) {
		// just show a glut shape
//		glColor3f(1.0, 0.0, 0.0);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pay->ambient_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, pay->specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, pay->shininess);
		glutSolidTorus(1.5, 3.0, 12, 24);
	} else if(use_debug_syll) {
//		if(frame % 100 == 0) cout << "display debug syll" << endl;
		// just display debug_syll as seed
		// disable lighting in shader
//		glUniform1i(loc_disable_lighting, 1);
//		draw_debug_syllable(debug_syll, wireframe);
//		glUniform1i(loc_disable_lighting, 0);

	} else if(use_single_syll) {		//** debug - individual rendering
		if(render_debug) {
			single_syll->render_debug();
		} else if(wireframe) {
			//		syll3d->render_wire();
			single_syll->render_wire();
		} else {
			//		syll3d->render();
			single_syll->render();
		}
	} else { 	// normal execution
		draw_lotus_moon();
//		goto PastDrawing;
		draw_seed_syllable(hrih, wireframe);
//		cout << "rendered hrih" << endl;
		for (size_t i = 0; i < syllables.size(); ++i) {
			Syllable3D *syll = syllables[i];

			if(render_debug) {
				syll->render_debug();
			} else if(wireframe) {
				// disable lighting in shader
				if(shader_on) glUseProgram(0);
//				glUniform1i(loc_disable_lighting, 1);
				syll->render_wire(Util::white);
//				glUniform1i(loc_disable_lighting, 0);
				if(shader_on) glUseProgram(shader_prog);
			} else {
				syll->render(Util::white);
			}
		}
	}
	if(show_grid) {
		if(shader_on) glUseProgram(0);
//		glUniform1i(loc_disable_lighting, 1);
		cyl_grid.render(Util::magenta, 2.0);
		if(shader_on) glUseProgram(shader_prog);
//		glUniform1i(loc_disable_lighting, 0);
	}

	// render any light particles
	if(!particles.is_empty()) {
		if(shader_on) glUseProgram(0);
//		glUniform1i(loc_disable_lighting, 1);
		particles.render();
		if(shader_on) glUseProgram(shader_prog);
//		glUniform1i(loc_disable_lighting, 0);
	}
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// render any light beams
	if(!beams.is_empty()) {
		if(shader_on) glUseProgram(0);
//		glUniform1i(loc_disable_lighting, 1);
		beams.render();
		if(shader_on) glUseProgram(shader_prog);
//		glUniform1i(loc_disable_lighting, 0);
	}
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

//	if(beams.live_beams() > 2) {
//		beams.print_live_beams(2);
//	}
//	draw_xz_box();
	
//	PastDrawing:

	glPopMatrix();

	// keybindings
	heads_up_display(show_keybindings, show_beam_ct,
			show_framerate);

	glutSwapBuffers();
}

void ShowMantraApp::keyboard(unsigned char key, int x, int y) {
	static int particle_syll = 2;
	GLfloat eye_z;
	// how much to move a light source each time
	GLfloat light_delta = .1;
	bool moved_light = false;
	switch (key) {
		case 27: /* ESC */
//			cleanup();
			exit(0);
			break;
		case 'p': {// do particles for a syllable
			AnimatedSyllable3D *syll = (AnimatedSyllable3D *)syllables[particle_syll];
//			syll->get_particles(particles, 3.0);
//			cout << "particles: " << particles.live_particles() << endl;

			syll->get_beams(beams, 10.0);
//			cout << "Adding beams: count = " << beams.live_beams() << endl;
			break;
		}
		case '0': particle_syll = 0; break;
		case '1': particle_syll = 1; break;
		case '2': particle_syll = 2; break;
		case '3': particle_syll = 3; break;
		case '4': particle_syll = 4; break;
		case '5': particle_syll = 5; break;

		case 'k': // toggle display of keybindings
			show_keybindings = !show_keybindings;
			break;
		case 'w': // toggle wireframe
			wireframe = !wireframe;
			break;
		case '+':  case '=':  // zoom in
			z_zoom = min(z_zoom + z_zoom_delta, z_zoom_max);
			eye_z = z_start + z_zoom;
			// set lights z to eye_z
//			g_light0_pos[2] = g_light1_pos[2] = eye_z;
			cout << "z_zoom: " << z_zoom << endl;
			cout << "eye z = " << eye_z << endl;
			break;
		case '-': // zoom out
			z_zoom -= z_zoom_delta;
			eye_z = z_start + z_zoom;
			// set lights z to eye_z
//			g_light0_pos[2] = g_light1_pos[2] = eye_z;
			cout << "z_zoom: " << z_zoom << endl;
			cout << "eye z = " << eye_z << endl;
			break;
		case 't': // toggle showing syllables with 2d tweaking
					// by default tweaking is on
			do_2d_tweak = ! do_2d_tweak;
			for (size_t i = 0; i < syllables.size(); ++i) {
				Syllable3D *s = syllables[i];
				s->reinit();
			}
			map_to_cylinder(do_2d_tweak);
			break;
		case 'g': // toggle showing cylinder grid
			show_grid = !show_grid;
			break;
		case 'n': // toggle showing normals
				// this will toggle the show_normals flag, by default
				// the show_facet_norms is true, so they will be displayed
			toggle_normal_display(show_normals);
			cout << "show_normals: " << show_normals << ", show_facet_norms: "
					<< show_facet_norms << ", show_vert_norms: " << show_vert_norms << endl;
			break;
		case 'f': // if showing normals, show facet normals
			toggle_normal_display(show_facet_norms);
			cout << "show_normals: " << show_normals << ", show_facet_norms: "
					<< show_facet_norms << ", show_vert_norms: " << show_vert_norms << endl;
			break;
		case 'v': // if showing normals, show vertex normals
			toggle_normal_display(show_vert_norms);
			cout << "show_normals: " << show_normals << ", show_facet_norms: "
					<< show_facet_norms << ", show_vert_norms: " << show_vert_norms << endl;
			break;

		case 'm': {// show modelview and trackball transform matrices
				GLfloat ctm[16];
				glGetFloatv(GL_MODELVIEW_MATRIX, ctm);

				cout << "ModelView Row Order:" << endl;
				print_mat(ctm);
				cout << "ModelView Col Order:" << endl;
				print_mat_col(ctm);
				cout << endl;

				copym(ctm, trackball_transform_mat);
				cout << "Trackball Transform Row Order:" << endl;
				print_mat(ctm);
				cout << "Trackball Transform Col Order:" << endl;
				print_mat_col(ctm);
				cout << endl;
				break;
		}
		case 's': {// toggle shader off/on
			shader_on = !shader_on;
			if(shader_on) {
				glUseProgram(shader_prog);
			} else {
				glUseProgram(0);
			}
			const char *strval = shader_on ? "on" : "off";
			keybindings.pop_back();
			keybindings.push_back(strval);
			break;
		}
//		case 'x': // move light in positive x direction
//			move_light(GL_LIGHT0, 0, light_delta);
//			moved_light = true;
//			break;
//		case 'X': // move light in negative x direction
//			move_light(GL_LIGHT0, 0, -light_delta);
//			moved_light = true;
//			break;
//		case 'y': // move light in positive y direction
//			move_light(GL_LIGHT0, 1, light_delta);
//			moved_light = true;
//			break;
//		case 'Y': // move light in negative y direction
//			move_light(GL_LIGHT0, 1, -light_delta);
//			moved_light = true;
//			break;
//		case 'z': // move light in positive z direction
//			move_light(GL_LIGHT0, 2, light_delta);
//			moved_light = true;
//			break;
//		case 'Z': // move light in negative z direction
//			move_light(GL_LIGHT0, 2, -light_delta);
//			moved_light = true;
//			break;

		case 'd': { // debug action
			debug = !debug;
			if(debug) {
				cout << " ******* doing debug *******" << endl;
				use_debug_shape = true;
			} else {
				cout << " ******* end debug *******" << endl;
				use_debug_shape = false;
			}
			break;
		}
//		case 'l': {// toggle light1
//			bool enabled1 = glIsEnabled(GL_LIGHT1);
//			if(enabled1) {
//				glDisable(GL_LIGHT1);
//				cout << "disabled light1" << endl;
//			} else {
//				glEnable(GL_LIGHT1);
//				cout << "enabled light1" << endl;
//			}
//			break;
//		}

		default:
			break;
	}
//	if(moved_light) {
//		GLfloat pos[4], epos[3], wpos[3], epos2[3];
//		glGetLightfv(GL_LIGHT0, GL_POSITION, pos);
//		copyv(epos, pos);
//		eye_to_world(epos, wpos);
//		cout << "Light 0: eye: " << stringv(pos, 4) << endl
//				<< "world: " << stringv(wpos) << endl ;
////		world_to_eye(wpos, epos2);
////		cout << "confirm eye: " << stringv(epos2) << endl;
//	}
	glutPostRedisplay();
}

void ShowMantraApp::reshape(int w, int h) {
	win_width = w;
	win_height = h;
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
//	gluPerspective(65.0, (GLfloat) w/(GLfloat) h, 5.0, 50.0);
	gluPerspective(65.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef (0.0, 0.0, z_start);
	glLightfv(light0.id, GL_POSITION, light0.pos);
	glLightfv(light1.id, GL_POSITION, light1.pos);
	//cout << "reshape" << endl
	//		<< "light 0: " << stringv(light0.pos, 4) << endl;;
//	glLightfv(GL_LIGHT1, GL_POSITION, g_light1_pos);

//	glTranslatef( 0.0, 0.25, 0);
}

//**** test stuff ***
void ShowMantraApp::test_init() {
	glClearColor ( .2, .2, .2, 1.0);
//	glClearColor ( .3, .3, .3, 1.0);
	glEnable ( GL_DEPTH_TEST );
	glEnable(GL_POINT_SMOOTH);
	// for trackball
//	set_to_ident(trackball_transform);
	// test init
	polytest.init();
	glutReshapeWindow(600, 400);
}

/**
 * Called when running poly tests
 */
void test_display() {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glPushMatrix();
	glTranslatef(0, 0, app.z_zoom);
	glMultMatrixf(app.trackball_transform_mat);

	polytest.render();
	glPopMatrix();

	// heads-up display - 2d
	app.heads_up_display(app.show_keybindings, app.show_particle_ct,
			app.show_framerate);

	glutSwapBuffers();
}

// *** glut required callbacks call into our classes ***
void reshape (int w, int h) {
	app.reshape(w, h);
}
void mouse(int button, int state, int x, int y) {
	app.mouse(button, state, x, y);
}
void mouse_motion(int x, int y) {
	app.mouse_motion(x, y);
}
void display(void) {
	app.display();
}
void idle() {
	app.idle();
}
void keyboard(unsigned char key, int x, int y) {
	app.keyboard(key, x, y);
}

// ** usage for main
void usage(string progname) {
	cout << "Usage: " << progname << " [width height]" << endl
			<< "\twidth, height: screen size " << endl;
}
int main(int argc, char** argv)
{
	// set to true to run test_init and test_display with polytest
	bool test_only = false;

	if(argc == 3) {
		// width and height params
		int w = atoi(argv[1]), h = atoi(argv[2]);
		if(w > 100 && h > 100) {
			app.win_width = w;
			app.win_height = h;
		}
	}
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize (app.win_width, app.win_height);
	glutInitWindowPosition (0, 0);
	glutCreateWindow ( "Working Mantra");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	  return 1;
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	// regular execution
	if(!test_only) {
		app.init ();
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutMouseFunc (mouse);
		glutMotionFunc(mouse_motion);
		glutIdleFunc(idle);
		glutMainLoop();

	} else { // test only
		app.test_init();
		glutDisplayFunc(test_display);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutMouseFunc (mouse);
		glutMotionFunc(mouse_motion);
		glutIdleFunc(test_display);
		glutMainLoop();
	}
	return 0;
}
