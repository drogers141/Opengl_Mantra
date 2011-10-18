/*
 * dr_glm.h
 *
 * Wrapper around Nate Robins' glm library using object oriented polygon class.
 * Simplified - assume only 1 group -no materials/texture.
 * Just want to be able to have better control over the polys.
 *
 *  Created on: Sep 12, 2010
 *      Author: drogers
 */

#ifndef DR_GLM_H_
#define DR_GLM_H_

#include "mygl.h"
extern "C" {
#include "glm.h"
}
#include "poly.h"

#include <vector>



namespace DR {


class DrGlmModel {
public:
	DrGlmModel();
	~DrGlmModel();
	void init(GLMmodel *glm_model);

	/**
	 * Render the model.
	 * params: wire - [false] if true render wireframe
	 * use_facetnorm - [false] if true use facetnorm instead of vertex norms
	 */
	void render(bool wire=false, bool use_facetnorm=false);
	void render_diff_colors(bool wire=false);
	void render_solid_and_wire();
	/**
	 * Draw polygon normals
	 * params: size - (length), if not given, will use average of max/min  poly side lengths
	 * which -	'f': default - facetnorms, drawn in magenta
	 * 					'v': vertex normals, drawn in cyan
	 * 					'b': both
	 */
	void draw_normals(char which='f', GLfloat size=-1.0);
	void draw_normals_diff_colors(GLfloat size=0.1, bool vertex=false);

	// show normals controls display of normals at all
	// if it is on, show facet or vert or both based on the other flags
	bool show_normals;
	bool show_facet_norms;
	bool show_vert_norms;
	// length to use for drawing normals if default is uncool
	// overrides default
	GLfloat draw_normals_length;

	// if true render with render_diff_colors
	bool diff_colors;

	GLint num_polygons() { return polygons.size(); }
	// the length of the shortest and
	// longest sides of all polygons, set by init
	GLfloat shortest_side_len;
	GLfloat longest_side_len;

	// material properties
	GLfloat ambient_diffuse[4];
	GLfloat specular[4];
	GLfloat shininess[1];
	GLfloat emissive[4];


private:
	std::vector<Polygon *> polygons;
	GLint num_vertices;
	GLint num_normals;
	vec3 *vertices;
	vec3 *normals;


	GLfloat near_white[4];


	std::vector<GLfloat *> colors;
	// set to 1 for normal display,
	// set > 1 to skip this many polygons while displaying in different colors
	GLint poly_skip_factor;


};

}

#endif /* DR_GLM_H_ */
