/*
 * dr_glm.cpp
 *
 *  Created on: Sep 12, 2010
 *      Author: drogers
 */

#include "dr_glm.h"
#include <cassert>

using namespace std;
using namespace DR;

DrGlmModel::DrGlmModel()
: num_vertices(0), num_normals(0),
  vertices(NULL), normals(NULL)  {
	setv(near_white, 0.973, 0.976, 0.957, 1.0);

	copyv(ambient_diffuse, near_white, 4);
	copyv(specular, near_white, 4);
	shininess[0] = 128.0;
	setv(emissive, 0.0, 0.0, 0.0, 1.0);

	show_normals = show_facet_norms = show_vert_norms = false;
	GLfloat draw_normals_length = -1;

	// debug as well
	diff_colors = false;
	colors = Util::colors_vec();

	// debug thing
	poly_skip_factor = 1;
}

DrGlmModel::~DrGlmModel() {
	for (size_t i = 0; i < polygons.size(); ++i) {
		delete polygons[i];
	}
	polygons.clear();
	delete [] vertices;
	delete [] normals;
}

void DrGlmModel::init(GLMmodel *glm_model) {
	assert(glm_model);
	num_vertices = glm_model->numvertices;
	num_normals = glm_model->numnormals;
	// *** deal with off by 1 when loading our arrays ***
	vertices = new vec3[num_vertices];
	for (int i = 0; i < num_vertices; ++i) {
		for (int j = 0; j < 3; ++j) {
			vertices[i][j] = glm_model->vertices[(i+1)*3 + j];
		}
	}
	normals = new vec3[num_normals];
	for (int i = 0; i < num_normals; ++i) {
		for (int j = 0; j < 3; ++j) {
			normals[i][j] = glm_model->normals[(i+1)*3 + j];
		}

	}
	// need to deal with glm's [1] == our [0] here as well
	int num_triangles = glm_model->numtriangles;
	for (int i = 0; i < num_triangles; ++i) {
		GLMtriangle *f=&(glm_model->triangles[i]);
		Triangle *t = new Triangle(vertices, normals);
		//**debug for printing out vindices
		//cout << i;
		for (int j = 0; j < 3; ++j) {
			t->verts[j] = f->vindices[j] - 1;
			t->norms[j] = f->nindices[j] - 1;
			//cout << "  " << f->vindices[j];
		}
		//cout << endl;
		// if we want to use the facetnorms from the model
		copyv(t->facetnorm, &(glm_model->facetnorms[f->findex * 3]));

		// this way sets winding from norms[0] and facetnorm from winding
		t->set_winding_from_normal(t->facetnorm); //t->facetnorm);
//		t->set_facetnorm();

		polygons.push_back(t);
	}
	// set longest and shortest side lengths
	GLfloat minlen = 10000, maxlen = 0;
	for (size_t i = 0; i < polygons.size(); ++i) {
		const Polygon *p = polygons[i];
		//		for (size_t j = 0; j < p->size; ++j) {
		//		}
		vector<Vec> verts;
		p->get_actual_verts(verts);
		for (size_t j = 0; j < verts.size(); ++j) {
			GLfloat sidelen = dist(verts[j], verts[(j+1)%verts.size()]);
			minlen = min(minlen, sidelen);
			maxlen = max(maxlen, sidelen);
		}
	}
	shortest_side_len = minlen;
	longest_side_len = maxlen;
	assert(maxlen > 0 && minlen < 10000);
}
/**
 * Render the model.
 * params: wire - [false] if true render wireframe
 * use_facetnorm - [false] if true use facetnorm instead of vertex norms
 */
void DrGlmModel::render(bool wire, bool use_facetnorm) {
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glLineWidth(1);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambient_diffuse);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambient_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

//	glEnable(GL_NORMALIZE);

//	glScalef(3.0, 3.0, 3.0);

//	glTranslatef(0.0, -3.5, 0.0);

//	glDisable(GL_LIGHTING);
//	glDisable(GL_BLEND);
//	glDisable(GL_TEXTURE_2D);
//	glUseProgram(0);
//	glColor3fv(Util::red);
	if(wire) {
		glDisable(GL_LIGHTING);
		glColor4fv(ambient_diffuse);
	}

	for (size_t i = 0; i < polygons.size(); ++i) {
		const Polygon *p = polygons[i];

		if(wire) {
			glBegin(GL_LINE_LOOP);
			for (int j=0; j<p->size; ++j) {
				glVertex3fv ( vertices[p->verts[j]] );
			}
		} else {
			if(p->size == 3) {
			glBegin(GL_TRIANGLES);
			} else if(p->size == 4) {
				glBegin(GL_QUADS);
			}
			if(use_facetnorm) {
				glNormal3fv(p->facetnorm);
			}
			for (int j=0; j<p->size; ++j) {
				if(!use_facetnorm) {
					glNormal3fv( normals[p->norms[j]] );
				}
				glVertex3fv ( vertices[p->verts[j]] );
			}
		}
		glEnd();
	}
	// draw normals
	if(show_normals) {
		if(show_facet_norms && show_vert_norms) {
			draw_normals('b');
		} else if(show_facet_norms) {
			draw_normals('f');
		} else if(show_vert_norms){
			draw_normals('v');
		}

	}

	glPopAttrib();
	glPopMatrix();

}

void DrGlmModel::render_diff_colors(bool wire) {

}
void DrGlmModel::render_solid_and_wire() {

}
/**
 * Draw polygon normals
 * params: size - (length), if not given, will use average of max/min  poly side lengths
 * which -	'f': default - facetnorms, drawn in magenta
 * 			'v': vertex normals, drawn in cyan
 * 			'b': both
 */
void DrGlmModel::draw_normals(char which, GLfloat size) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glLineWidth(1);
	glPointSize(1);
	glEnable(GL_POINT_SMOOTH);
	if(size < 0) {
		if(draw_normals_length > 0) {
			size = draw_normals_length;
		} else {
			size = (longest_side_len + shortest_side_len) / 2;
		}
	}
	if(which == 'v' || which == 'b') {
		glColor3fv(Util::cyan);
		for (size_t i = 0; i < polygons.size(); ++i) {
			const Polygon *p = polygons[i];

			for (int j = 0; j < p->size; ++j) {
				Vec v(vertices[p->verts[j]]), vn(normals[p->norms[j]]);
				Vec end = v + size * vn;

				glBegin(GL_LINES);
				v.glVertex();
				end.glVertex();
				glEnd();
				glBegin(GL_POINTS);
				end.glVertex();
				glEnd();
			}
		}
	}
	if(which == 'f' || which == 'b') {
		glColor3fv(Util::magenta);
		for (size_t i = 0; i < polygons.size(); ++i) {
			const Polygon *p = polygons[i];
			Vec c(p->center), fn(p->facetnorm);
			Vec end = c + size * fn;

			glBegin(GL_LINES);
			c.glVertex();
			end.glVertex();
			glEnd();
			glBegin(GL_POINTS);
			end.glVertex();
			glEnd();

		}
	}

	glPopAttrib();

}
void DrGlmModel::draw_normals_diff_colors(GLfloat size, bool vertex) {

}
