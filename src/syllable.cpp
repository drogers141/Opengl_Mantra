/*
 * Syllable.cpp
 *
 *  Created on: May 31, 2010
 *      Author: drogers
 */

#include "mygl.h"
#include "face.h"
#include "syllable.h"
#include "dr_util.h"
#include "vec.h"

#include <cstdio>
#include <cassert>
#include <iostream>
#include <ostream>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace DR;


static GLfloat* colors[] = { Util::red, Util::purple, Util::blue, Util::green, Util::yellow,
		Util::grey, Util::magenta, Util::white, Util::cyan, Util::black };

void Syllable2D::InitFromObj(Syllable2D& syllable, const char* objfile, bool unitize) {

	syllable.model = glmReadOBJ ( (char*)objfile );
//	cout << "*** Syllable2D: creating model with glmReadOBJ, model="
//			<< syllable.model << endl;
	if(unitize) {
		glmUnitize ( syllable.model );
	}
	glmFacetNormals ( syllable.model );
	glmVertexNormals ( syllable.model, 90.0 );
}

Syllable2D::~Syllable2D() {
	if(model != NULL) {
//		cout << "*** Syllable2D: destroying model with glmDelete, model="
//				<< model << endl;
		glmDelete(model);
	}
}

GLMmodel* Syllable2D::getModel() {
	return model;
}

//Syllable3D::Syllable3D(char* objfile)
//: base2d() {
//	Syllable2D::InitFromObj(base2d, objfile);
//}

void Syllable3D::initFromObj(const char *objfile, bool unitize) {
	GLdouble start = age();
	Syllable2D::InitFromObj(base2d, objfile, unitize);
	GLdouble end = age();
//	cout << "initFromObj: glm model read in:" << endl
//			<< "elapsed ms: " << (end - start) << endl
//			<< "age now ms: " << end << ",  age started ms: " << start << endl;
}

Syllable3D::Syllable3D()
:  num_vertices(0), num_normals(0), num_normals_sides(0),
   show_normals(false), show_facet_norms(false), show_vert_norms(false),
   base2d(), vertices(NULL), normals(NULL),
   base_face(this), extruded_face(this), side_normals(NULL) {
	start_time = clock();
	srand ( time(NULL) );
//	cout << "start_time: " << start_time << endl;
	// materials
	ambient_diffuse[3] = specular[3] = 1.0f;
	setv(ambient_diffuse, .1, .5, .8);
	setv(specular, 1.0, 1.0, 1.0);
	shininess[0] = 128.0;
	//	setv(specular, 0.0, 0.0, 0.0);
	//	shininess[0] = 0.0;
	// emissive property
	setv(emissive, 0.0, 0.0, 0.0);
	emissive[3] = 0.0f;
	// center is at origin by default
	setv(center, 0, 0, 0);
}

/**
 * Age in milliseconds since constructor called.
 * param: secs: if true, return secs rather than ms
 */
GLdouble Syllable3D::age(bool secs) {
	clock_t now = clock();
	GLdouble elapsed = ((double)(now - start_time)) / (double)CLOCKS_PER_SEC;
	if(!secs) {
		elapsed *= 1000.0;
	}
	return  elapsed;
}

Syllable3D::~Syllable3D() {
	delete [] vertices;
	delete [] normals;
	delete [] side_normals;
	// change this if sides are handled differently
	for (size_t i = 0; i < sides.size(); ++i) {
		delete sides[i];
	}
}

void Syllable3D::render(GLfloat *color, bool no_mat) {
//	render_model();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ambient_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, emissive);

	render_face(base_face, color, false, no_mat);
	render_face(extruded_face, color, false, no_mat);
	render_sides(color, false, no_mat);
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
}

// render using wireframe
// color: defaults to white
void Syllable3D::render_wire(GLfloat *color, bool no_mat, GLfloat line_sz) {
	GLfloat *col = color == NULL ? Util::white : color;
	render_face(base_face, col, true, no_mat, line_sz);
	render_face(extruded_face, col, true, no_mat, line_sz);
	render_sides(col, true, no_mat, line_sz);
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
	//***debug - show syll centers
//	vec3 bc, ec;
//	base_face.get_center(bc);
//	extruded_face.get_center(ec);
//	glPushAttrib(GL_ALL_ATTRIB_BITS);
//	glPointSize(10);
//	glBegin(GL_POINTS);
//	glColor3fv(Util::purple);
//	glVertex3fv(center);
////	glVertex3fv(bc);
////	glVertex3fv(ec);
//	glColor3fv(Util::cyan);
//	glVertex3fv(assigned_center);
//	glEnd();
//	glPopAttrib();
}

void Syllable3D::render_model() {
	//** debug
	static bool print_debug_model = false;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPointSize(4);
	glLineWidth(2);

	int numcolors = Util::numcolors;
	GLMmodel* model = getBase2dModel();
	size_t p;
//	int triangles_per_color = model->numtriangles / numcolors;
	//	printf("triangles_per_color: %d\n", triangles_per_color);

	for (p=0; p < model->numtriangles; p++) {
		GLMtriangle *f=&(model->triangles[p]);
		int i;

		// alternate colors 1 color per triangle
		glColor3fv( colors[p%numcolors]);

		//glColor3f( .5, .5, 1 );

		// alternate colors over entire shape, 1 region per color
		// in order of colors array (starts with red, blue, ..)
		//glColor3fv( colors[p/triangles_per_color]);

		glBegin(GL_TRIANGLES);
//		glBegin(GL_LINES);
		//		glBegin(GL_POINTS);

		glNormal3fv ( &(model->facetnorms[f->findex * 3]) );

		if(print_debug_model) {
			printf("%d\t", p);
		}
		for (i=0; i<3; ++i) {
			GLfloat *pos = & ( model->vertices[f->vindices[i] * 3] );
			glVertex3fv ( pos );
			if(print_debug_model) {
				printf("(%.3f, %.3f, %.3f) ", pos[0], pos[1], pos[2]);
			}
		}
		if(print_debug_model) {
			GLfloat *n = &(model->facetnorms[f->findex * 3]);
			printf("n = (%.3f, %.3f, %.3f) ", n[0], n[1], n[2]);
			printf("\n");
		}
		glEnd();

	}
	print_debug_model = false;

	// debug draw points in order
//	for(size_t i=0; i<model->numtriangles; i++) {
//		GLMtriangle *f=&(model->triangles[i]);
//		glNormal3fv ( &(model->facetnorms[f->findex * 3]) );
//
//		glBegin(GL_POINTS);
//		for (size_t j=0; j<3; ++j) {
//			glColor3fv( colors[(i+j) % numcolors]);
//			GLfloat *pos = & ( model->vertices[f->vindices[j] * 3] );
//			glVertex3fv ( pos );
//			if(print_coords) {
//				printf("(%.3f, %.3f, %.3f) ", pos[0], pos[1], pos[2]);
//			}
//		}
////		GLfloat x, y, z;
////		x = model->vertices[i];
////		y = model->vertices[i+1];
////		y = model->vertices[i+2];
////		glVertex3f(x, y, z);
//		glEnd();
//	}

	glPopAttrib();
}

/**
 * removes sides and extruded face and resets base_face from model
 * clears all debug stuff as well
 */
void Syllable3D::reinit() {
	extruded_face.clear();
	base_face.clear();
	for (size_t i = 0; i < sides.size(); ++i) {
		delete sides[i];
	}
	sides.clear();

	// clear debug stuff
	debug_edges.clear();
	debug_points.clear();
	debug_polygons.clear();
	debug_verts.clear();

	GLMmodel* model = getBase2dModel();

	for (int i = 0; i < num_vertices; ++i) {
		for (int j = 0; j < 3; ++j) {
			vertices[i][j] = model->vertices[(i+1)*3 + j];
		}
	}
	for (int i = 0; i < num_normals_base; ++i) {
		for (int j = 0; j < 3; ++j) {
			normals[i][j] = model->normals[(i+1)*3 + j];
		}
	}
	int num_triangles = model->numtriangles;
	for (int i = 0; i < num_triangles; ++i) {
		GLMtriangle *f=&(model->triangles[i]);
		Triangle *t = new Triangle(vertices, normals);
		for (int j = 0; j < 3; ++j) {
			t->verts[j] = f->vindices[j] - 1;
			t->norms[j] = f->nindices[j] - 1;
			//cout << "  " << f->vindices[j];
		}
		// this way sets winding from norms[0] and facetnorm from winding
		t->set_winding_from_normal();
		t->set_facetnorm();
		base_face.add_polygon(t);
	}
	cout << "reinit:   " << base_face.polygons.size() << endl;
	base_face.init_regions(0);
}

// copy base2d model vertices to member vertices
// and triangles to base_face, initializes base face polys
// sets base face center
// note -- glm has 1-based arrays for vertices, norms, texcoords
// I'm using zero based
void Syllable3D::init_base() {
	GLdouble start = age();
	GLMmodel* model = getBase2dModel();

	// at this point, vertices and normals will only have
	// enough for the base_face
	num_vertices_base = model->numvertices;
	num_vertices = 2 * num_vertices_base;

	// *** deal with off by 1 when loading our arrays ***
	// double size of vertices and normals for 2nd face
	vertices = new vec3[num_vertices];
	for (int i = 0; i < num_vertices; ++i) {
		for (int j = 0; j < 3; ++j) {
			vertices[i][j] = model->vertices[(i+1)*3 + j];
			// ** debug
//			if(i < 9 || i > num_vertices - 9) {
//				cout << i << " " << j << "\t " << model->vertices[(i+1)*3 + j]
//				     << "   = " << "model->vertices[" << (i+1)*3 + j << "]" << endl;
//
//			}
		}
	}
	num_normals_base = model->numnormals;
	num_normals = 2 * num_normals_base;

	normals = new vec3[num_normals];
	for (int i = 0; i < num_normals_base; ++i) {
		for (int j = 0; j < 3; ++j) {
			normals[i][j] = model->normals[(i+1)*3 + j];
		}

	}
	// convert triangles from model into base_face
	// need to deal with glm's [1] == our [0] here as well
	int num_triangles = model->numtriangles;
	for (int i = 0; i < num_triangles; ++i) {
		GLMtriangle *f=&(model->triangles[i]);
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
//		copyv(t->facetnorm, &(model->facetnorms[f->findex * 3]));

		// this way sets winding from norms[0] and facetnorm from winding
		t->set_winding_from_normal(); //t->facetnorm);
		t->set_facetnorm();

		base_face.add_polygon(t);
	}

//	cout << "base_face polygons: " << base_face.polygons.size() << endl;
//	for (int i = 0; i < base_face.polygons.size(); ++i) {
//		Polygon *p = base_face.polygons[i];
//		cout << *p << endl;
//	}
	// ** debug
//	check_base_model();

	// find regions and perimeters in base face
	base_face.init_regions(0);

	// tell base face to set center polygon
	// so we have it available for later
	base_face.set_center( Vec(0, 0, 1) );

//	Polygon cpoly;
//	base_face.get_center(cpoly);
//	cout << "** init_base: base center after set center:" << endl
//			<< cpoly.to_string() << endl;

	// set syllable's center to the base_face's center
	// until extruded, mapped, etc
	base_face.get_center(center);

//	cout << "** init_base: syll center: " << stringv(center) << endl;

	// debug
	add_all(base_face.debug_polygons, debug_polygons);
	add_all(base_face.debug_points, debug_points);
	add_all(base_face.debug_verts, debug_verts);
	add_all(base_face.debug_edges, debug_edges);
//	cout << "debug_polygons:" << endl;
//	print_polys(debug_polygons);

	//** timing
//	GLdouble end = age();
//	cout << "init_base: converted glm to base_face" << endl
//			<< "elapsed ms: " << (end - start) << endl
//			<< "age now ms: " << end << ",  age started ms: " << start << endl;
}

// reset winding for all base polygons
// for now uses set_winding_from_normal()
//** resets facet normal **
void Syllable3D::reset_base_windings() {
	for (size_t i = 0; i < base_face.polygons.size(); ++i) {
		Polygon *poly = base_face.polygons[i];
		poly->set_winding_from_normal();
		poly->set_facetnorm();
	}
}


// extrudes base_face into 3d syllable, creates extruded face and sides
// param: thickness, thickness of extrusion
// param: rev_side_winding [false], if true, will reverse vertex order
//		of polygons in sides and set facetnorms accordingly
void Syllable3D::extrude(GLfloat thickness, bool rev_side_winding) {
	// memory is already allocated for vertices and vertex normals
	// need to initialize vertices and vertex normals for extruded face
	// note we rely on the vertex normals having the same index as their vertex
	// for extruded face,
	// new vertex = base_vert + normal * thickness
	// new normal = base_normal
	// also -- negate base normal as now the base face will be pointing
	// the opposite way (the extruded face points the same way as the
	// the base face did
	GLdouble start = age();
	for (int i = num_vertices_base; i < num_vertices; ++i) {
		GLfloat *basev = vertices[i - num_vertices_base];
		GLfloat *basen = normals[i - num_vertices_base];
		Vec basevec(basev), basenorm(basen), newvec;
		newvec = basevec + thickness * basenorm;
		vec3 newv, newn;
		newvec.array_out(newv);
		// copy new vertex
		copyv(vertices[i], newv);
		// copy new normal (same as base vertex normal)
		copyv(normals[i], basen);
		// flip base normal
		Vec newbasenorm = -1 * basenorm;
		newbasenorm.array_out(newn);
		copyv(basen, newn);
	}

	// similar process with copying the polygons to the extruded face
	// except we can't trust the facetnorms to be right on the base face
	// also - copy center poly to extruded face
	GLint base_center_index = base_face.get_center_index();
	bool base_center_found = false;
	for (size_t i = 0; i < base_face.polygons.size(); ++i) {
		Polygon *basep = base_face.polygons[i];
		Triangle *t = new Triangle(vertices, normals);
		for (int j = 0; j < t->size; ++j) {
			t->verts[j] = basep->verts[j] + num_vertices_base;
			t->norms[j] = basep->norms[j] + num_vertices_base;
		}

		// make sure the base facetnorm is right, then
		// set the new facetnorm to it
//		t->set_winding_from_normal();
		// ** assume winding was correct on base face **
		t->set_facetnorm();
		copyv(basep->facetnorm, t->facetnorm);
		// set the edges and center of the new triangle
		t->set_edges();
		t->set_center();
		// add it to the face
		extruded_face.add_polygon(t);

		// check for center poly before modifying base poly
		// when found, set extruded face center to it
		// then set syllable's center as the midpoint
		if(i == (size_t)base_center_index) {
			base_center_found = true;
			Polygon base_center;
			base_face.get_center(base_center);
//			cout << "*** extrude: found center: " << endl
//					<< "i = " << i << ", " << base_center.to_string() << endl;
			extruded_face.set_center(t->center);
			vec3 bc, ec;
			base_face.get_center(bc);
			extruded_face.get_center(ec);
			midpoint(bc, ec, center);
//			cout << "centers: base=" << stringv(bc) << ", extr=" << stringv(ec)
//									<< ", syll=" << stringv(center) << endl;
		}
		// flip the base facetnorm
		Vec bfnorm = -1 * Vec(t->facetnorm);
		bfnorm.array_out(basep->facetnorm);
		// fix winding on base poly
		basep->set_winding_from_normal(basep->facetnorm);
	}
	// getting serious about these centers .. :)
	assert(base_center_found);

	// initialize regions on extruded
//	extruded_face.init_regions(num_vertices_base);
	//** debug
//	goto AFTER_REGIONS_INIT;

	// copy regions and perimeters from base to extruded face
	for (size_t i = 0; i < base_face.regions.size(); ++i) {
		Region *basereg = base_face.regions[i];
		Region *extreg = new Region();
//		cout << "extrude: region " << i << "  basereg->polygons.size(): " << basereg->polygons.size() << endl;
		// copy polygons
		for (size_t j = 0; j < basereg->polygons.size(); ++j) {
			Polygon *bp = basereg->polygons[j];
//			cout << "base poly: " << j << "  "<< bp->to_string() << endl;
			GLuint poly_index = base_face.get_poly_index(bp);
			if(poly_index >= extruded_face.polygons.size()) {
				cout << "***** extrude: poly index = " << poly_index << ", out of range" << endl
						<< "extruded_face.polygons.size(): " << extruded_face.polygons.size() << endl;
				cout << "base_face.polygons.size(): " << base_face.polygons.size() << endl;
			}
			extreg->polygons.push_back( extruded_face.polygons[poly_index]);
//			cout << "extr poly: " << j << "  " << extruded_face.polygons[poly_index]->to_string() << endl;
		}
//		cout << "extreg->polygons.size(): " << extreg->polygons.size() << ", equals base polygons' size? = "
//				<< (extreg->polygons.size() == basereg->polygons.size()) << endl;
		// copy perimeter
		for (size_t j = 0; j < basereg->perimeter.size(); ++j) {
			GLint vert = basereg->perimeter[j] + num_vertices_base;
			extreg->perimeter.push_back(vert);
		}
		// copy inner perimeters
		for (size_t j = 0; j < basereg->inner_perimeters.size(); ++j) {
			vector<GLint> new_inner;
			for (size_t k = 0; k < basereg->inner_perimeters[j].size(); ++k) {
				GLint vert = basereg->inner_perimeters[j][k] + num_vertices_base;
				new_inner.push_back(vert);
			}
			extreg->inner_perimeters.push_back(new_inner);
		}
		extruded_face.regions.push_back(extreg);
	}

	//** debug
//	AFTER_REGIONS_INIT:

	//*** debug
	for (size_t i = 0; i < extruded_face.regions.size(); ++i) {
		Region *r = extruded_face.regions[i];
		for (size_t j = 0; j < r->perimeter.size(); j++) {
			extruded_face.debug_verts.push_back(r->perimeter[j]);
		}
		for (size_t j = 0; j < r->inner_perimeters.size(); ++j) {
			for (size_t k = 0; k < r->inner_perimeters[j].size(); ++k) {
				extruded_face.debug_verts.push_back(r->inner_perimeters[j][k]);
			}
		}
	}
//	cout << "** before adding extruded debug verts, "
//			<< " debug_verts.size(): " << debug_verts.size() << endl;
//
//	// debug
//	add_all(extruded_face.debug_polygons, debug_polygons);
//	add_all(extruded_face.debug_verts, debug_verts);
//	add_all(extruded_face.debug_edges, debug_edges);

//	cout << "*** debug_verts.size(): " << debug_verts.size() << endl;
//
//	cout << "extruded face debug verts: " << extruded_face.debug_verts.size()
//			<< ", base face debug verts: " << base_face.debug_verts.size() << endl;
//	cout << "**** debug verts ****" << endl;
	for (size_t i = 0; i < base_face.debug_verts.size(); ++i) {
		vec3 basev, extrv;
		get_vert(base_face.debug_verts[i], basev);
		get_vert(extruded_face.debug_verts[i], extrv);
//		cout << i << " base: " << stringv(basev) << ", extr: " << stringv(extrv) << endl;
	}
	// check for problems
//	cout << "Checking extruded to base" << endl;
//	check_extruded_to_base();

	// create sides connecting base face and extruded face
	create_sides(rev_side_winding);

	// now that sides have been created, which relied on perimeter
	// vertices mirroring each other, we can fix the base face
	// perimeters to maintain right-handed winding direction
	base_face.reverse_perimeter_windings();

//	cout << endl << "Syllable3D::extrude():  extruded syllable polygons:" << endl;
//	cout << "base face: " << base_face.polygons.size() << endl;
//	cout << "extruded face: " << extruded_face.polygons.size() << endl;
//	cout << "sides: " << sides.size() << endl;
//	int total = base_face.polygons.size() + extruded_face.polygons.size() + sides.size();
//	cout << "total: " << total << endl;

	//** debug
//	for (size_t i = 0; i < sides.size()/4; ++i) {
//		debug_polygons.push_back(sides[i]);
//	}
//	cout << "debug_polygons.size(): " << debug_polygons.size() << endl;

	GLdouble end = age();
//	cout << "extrude(): extruded base into 3d syll" << endl
//			<< "elapsed ms: " << (end - start) << endl
//			<< "age now ms: " << end << ",  age started ms: " << start << endl;

}

// check all normals are normalized, print out any that are not
void Syllable3D::check_normals() {
//	cout << "check_normals: begin, base_face.polygons.size(): " << base_face.polygons.size()<< endl;
	for (size_t i = 0; i < base_face.polygons.size(); ++i) {
		const Polygon *p = base_face.polygons[i];
//		cout << i << "  " << p->to_string() << endl;
		for (int j = 0; j < p->size; ++j) {
			GLfloat mag = magnitude(normals[p->norms[j]]);
			if(!almost_equal(mag, 1.0f)) {
				cout << "!! normal not unit length:  base_face polygons[" << i << "] norms[" << j << "]" << endl;
				cout << "magnitude: " << mag << endl;
				cout << p->to_string() << endl;
			}
		}
		GLfloat mag = magnitude(p->facetnorm);
		if(!almost_equal(mag, 1.0f)) {
			cout << "!! normal not unit length:  base_face polygons[" << i << "] facetnorm" << endl;
			cout << "magnitude: " << mag << endl;
			cout << p->to_string() << endl;
		}
	}
//	cout << "check_normals: done with base" << endl;
	for (size_t i = 0; i < extruded_face.polygons.size(); ++i) {
		const Polygon *p = extruded_face.polygons[i];
		for (int j = 0; j < p->size; ++j) {
			GLfloat mag = magnitude(normals[p->norms[j]]);
			if(!almost_equal(mag, 1.0f)) {
				cout << "!! normal not unit length:  extruded_face polygons[" << i << "] norms[" << j << "]" << endl;
				cout << "magnitude: " << mag << endl;
				cout << p->to_string() << endl;
			}
		}
		GLfloat mag = magnitude(p->facetnorm);
		if(!almost_equal(mag, 1.0f)) {
			cout << "!! normal not unit length:  extruded_face polygons[" << i << "] facetnorm" << endl;
			cout << "magnitude: " << mag << endl;
			cout << p->to_string() << endl;
		}
	}
//	cout << "check_normals: done with extruded" << endl;
	for (size_t i = 0; i < sides.size(); ++i) {
		const Polygon *p = sides[i];
		for (int j = 0; j < p->size; ++j) {
			GLfloat mag = magnitude(side_normals[p->norms[j]]);
			if(!almost_equal(mag, 1.0f)) {
				cout << "!! normal not unit length:  sides polygons[" << i << "] norms[" << j << "]" << endl;
				cout << "magnitude: " << mag << endl;
				cout << p->to_string() << endl;
			}
		}
		GLfloat mag = magnitude(p->facetnorm);
		if(!almost_equal(mag, 1.0f)) {
			cout << "!! normal not unit length:  sides polygons[" << i << "] facetnorm" << endl;
			cout << "magnitude: " << mag << endl;
			cout << p->to_string() << endl;
		}
	}
	cout << "check_normals: done " << endl;
}

// for each face, set all vertex normals to be the average of the facet normals
// of each polygon the vertex is contained in (within that face, ie don't consider sides)
void Syllable3D::average_vertex_normals() {
	average_vertex_normals(base_face);
	average_vertex_normals(extruded_face);
}

// set all vertex normals in face to be the average of the facet normals
// of each polygon the vertex is contained in
void Syllable3D::average_vertex_normals(Face& face) {
	vector<Polygon *> polys_containing;
	for (size_t i = 0; i < face.polygons.size(); ++i) {
		Polygon *p = face.polygons[i];
		for (int j = 0; j < p->size; ++j) {
			Vec sum;
			face.find_polys_containing(polys_containing, p->verts[j]);
			for (size_t k = 0; k < polys_containing.size(); ++k) {
				Vec fnorm(polys_containing[k]->facetnorm);
				sum += fnorm;
			}
			sum.normalize();
			sum.array_out(normals[p->norms[j]]);
		}
	}
}


// confirm extruded face is properly initialized from base face
// call this before reversing windings on base face
void Syllable3D::check_extruded_to_base() {
	if(base_face.polygons.size() != extruded_face.polygons.size()) {
		cout << "check_base_to_extruded:  base and extruded have different number of polygons, exiting" << endl
				<< "base_face.polygons.size(): " << base_face.polygons.size()
				<< "  extruded_face.polygons.size(): " << extruded_face.polygons.size() << endl;
		exit(1);
	}
	for (size_t i = 0; i < base_face.polygons.size(); ++i) {
		Polygon *bp = base_face.polygons[i];
		Polygon *ep = extruded_face.polygons[i];
		bool eq = true;
		for (int j = 0; j < bp->size; ++j) {
			if(ep->verts[j] != bp->verts[j] + num_vertices_base) {
				eq = false;
				break;
			}
		}
		if( !eq ) {
			cout << "!= : check_base_to_extruded: extruded poly verts != base poly verts + offset, index = " << i << endl;
			cout << "     base poly: " << bp->to_string() << endl;
			cout << "     extr poly: " << ep->to_string() << endl;
			exit(1);
		}
	}
	if(base_face.regions.size() != extruded_face.regions.size()) {
			cout << "check_base_to_extruded:  base and extruded have different number of regions, exiting" << endl
					<< "base_face.regions.size(): " << base_face.regions.size()
					<< "  extruded_face.regions.size(): " << extruded_face.regions.size() << endl;
			exit(1);
		}
	// check perimeters and inner perimeters, iterating over the regions
	for (size_t i = 0; i < base_face.regions.size(); ++i) {
		Region *br = base_face.regions[i];
		Region *er = extruded_face.regions[i];
		cout << "check_base_to_extruded: checking regions, index = " << i << endl;
		if(br->perimeter.size() != er->perimeter.size() ) {
			cout << "!= perimeter size: base = " << br->perimeter.size()
					<< "  extruded = " << er->perimeter.size() << endl;
			exit(1);
		}
		for (size_t j = 0; j < br->perimeter.size(); ++j) {
			if(br->perimeter[j] + num_vertices_base != er->perimeter[j]) {
				cout << "!= base face vs extruded face perimeter vert, index = " << j << endl
						<< "base vert: " << br->perimeter[j]
						<< ", extr vert: " << er->perimeter[j] << endl;
			}
		}
		if(br->inner_perimeters.size() != er->inner_perimeters.size() ) {
			cout << "!= inner_perimeters size: base = " << br->inner_perimeters.size()
									<< "  extruded = " << er->inner_perimeters.size() << endl;
			exit(1);
		}
		for (size_t j = 0; j < br->inner_perimeters.size(); ++j) {
			vector<GLint> bper = br->inner_perimeters[j];
			vector<GLint> eper = er->inner_perimeters[j];
			if(bper.size() != eper.size()) {
				cout << "!= inner_perimeters[" << j << "] size:  base size: "
						<< bper.size() << " extr size: " << eper.size() << endl;
				exit(1);
			}
			for (size_t k = 0; k < bper.size(); ++k) {
				if(bper[k] + num_vertices_base != eper[k]) {
					cout << "!= base face vs extruded face inner perimeter vert " << endl
							<< "inner perimeter: " << j << ", index = " << k << endl
							<< "base vert: " << bper[k] << ", extr vert: " << eper[k] << endl;
				}
			}

		}
		// check polygons of each, the vector of polygons in a region
		// is a vector of pointers, so need to get indices
		// as with the other face members, equivalent polygons
		// should have the same index in each face
		for (size_t j = 0; j < br->polygons.size(); ++j) {
			Polygon *bp = br->polygons[j];
			GLint bp_index = base_face.get_poly_index(bp);
			if(*base_face.polygons[bp_index] != *bp) {
				cout << "base face polygon from pointer not matching to index: bp_index=" << bp_index << endl;
			}
			Polygon *ep = extruded_face.polygons[bp_index];
			bool eq = true;
			for (int k = 0; k < bp->size; ++k) {
				if(ep->verts[k] != bp->verts[k] + num_vertices_base) {
					cout << "ep->verts[k]: " << ep->verts[k] << ", "
							<< "bp->verts[k] + num_vertices_base: "
							<< bp->verts[k] + num_vertices_base << endl;
					eq = false;
					break;
				}
			}
			if( !eq ) {
				cout << "!= : check_base_to_extruded: region = " << i << endl
						<< "extruded poly verts != base poly verts + offset, index = " << i << endl;
				cout << "     base poly: " << bp->to_string() << endl;
				cout << "     extr poly: " << ep->to_string() << endl;
				exit(1);
			}
		}
	}
}



// called by extrude to create sides
// must have extruded side created first

// create the sides of the 3d syllable by connecting equivalent vertices
// of the 2 faces using the perimeters of the faces' regions

// for now just stay in object order and the sides will be a flat
// vector of all quads formed
// object order: face -> regions(1 or more) -> outer(1) -> inner(0 or more)
// param: rev_winding [false], if true, will reverse vertex order
//		  and set facetnorm accordingly
void Syllable3D::create_sides(bool rev_winding) {
	for (size_t i = 0; i < base_face.regions.size(); ++i) {
		Region *breg = base_face.regions[i];
		Region *ereg = extruded_face.regions[i];

		// outer perim first
		// set quad vertices with right hand winding
		for (size_t j = 0; j < breg->perimeter.size()-1; ++j) {
			Quad *q = new Quad(vertices, normals);
			q->verts[0] = breg->perimeter[j];
			q->verts[1] = breg->perimeter[j+1];
			q->verts[2] = ereg->perimeter[j+1];
			q->verts[3] = ereg->perimeter[j];
			for (int k = 0; k < 4; ++k) {
				q->norms[k] = -1;
			}
			q->set_edges();
			q->set_center();
			if(rev_winding) {
				q->reverse_winding();
			}
			q->set_facetnorm();
			sides.push_back(q);
		}
		// set last quad
		int last = breg->perimeter.size()-1;
		Quad *q = new Quad(vertices, normals);
		q->verts[0] = breg->perimeter[last];
		q->verts[1] = breg->perimeter[0];
		q->verts[2] = ereg->perimeter[0];
		q->verts[3] = ereg->perimeter[last];
		for (int k = 0; k < 4; ++k) {
			q->norms[k] = -1;
		}
		q->set_edges();
		q->set_center();
		if(rev_winding) {
			q->reverse_winding();
		}
		q->set_facetnorm();
		sides.push_back(q);

		// inner perim(s) if any
		// note windings are reversed of outer perims
		for (size_t p = 0; p < breg->inner_perimeters.size(); ++p) {
			for (size_t j = 0; j < breg->inner_perimeters[p].size()-1; ++j) {
				Quad *q = new Quad(vertices, normals);
				q->verts[0] = breg->inner_perimeters[p][j];
				q->verts[1] = breg->inner_perimeters[p][j+1];
				q->verts[2] = ereg->inner_perimeters[p][j+1];
				q->verts[3] = ereg->inner_perimeters[p][j];
				for (int k = 0; k < 4; ++k) {
					q->norms[k] = -1;
				}
				q->set_edges();
				q->set_center();
				if(!rev_winding) {
					q->reverse_winding();
				}
				q->set_facetnorm();
				sides.push_back(q);

			}
			// set last quad
			last = breg->inner_perimeters[p].size()-1;
			Quad *q = new Quad(vertices, normals);
			q->verts[0] = breg->inner_perimeters[p][last];
			q->verts[1] = breg->inner_perimeters[p][0];
			q->verts[2] = ereg->inner_perimeters[p][0];
			q->verts[3] = ereg->inner_perimeters[p][last];
			for (int k = 0; k < 4; ++k) {
				q->norms[k] = -1;
			}
			q->set_edges();
			q->set_center();
			if(!rev_winding) {
				q->reverse_winding();
			}
			q->set_facetnorm();
			sides.push_back(q);
		}
	}
	// create storage and initialize vertex normals for sides
	// vertex normals are copies of the facet norm at this point
	GLint polysize = sides[0]->size; // could change to triangles if desired
	num_normals_sides = polysize * sides.size();
	side_normals = new vec3[num_normals_sides];
	GLint normindex = 0;
	for (size_t i = 0; i < sides.size(); ++i) {
		Polygon *p = sides[i];
		for (int j = 0; j < p->size; ++j) {
			p->norms[j] = normindex;
			copyv(side_normals[normindex], p->facetnorm);
			normindex++;
		}
		// set polygon's reference to normals' storage
		p->set_normals_storage(side_normals);
	}

}

//****** todo: fix me ***
// debug: confirm things
void Syllable3D::check_base_model() {
	GLMmodel *model = getBase2dModel();

	// check vertices
	cout << "num_vertices: " << num_vertices << ", model: " << model->numvertices << endl;
	for (int i = 0; i < num_vertices; ++i) {
		GLfloat *mv =  &(model->vertices[(i+1)*3]);
		if( !equal(mv, vertices[i]) ) {
			cout << "!=: " << "i = " << i << "  " << vertices[i]
					<< "  "<< stringv(mv) << endl;
		}
	}

	cout << "base polygons: " << base_face.polygons.size()
			<< ", model triangles:  " << model->numtriangles << endl;

	// check vertices in triangles
	for (GLuint i = 0; i < model->numtriangles; ++i) {
		GLMtriangle *f=&(model->triangles[i]);
		for (int j = 0; j < 3; ++j) {
			if( (GLint)f->vindices[j] != base_face.polygons[i]->verts[j] + 1 ) {
				cout << "!=: " << "i = " << i << "  " << "f->vindices[" << j
						<< "] = " << f->vindices[j] << ", "
						<< "base_face.polygons[i]->verts[" << j
						<< "] + 1 = " << base_face.polygons[i]->verts[j] + 1 << endl;
			}
		}
	}
	// check coordinates of vertices in triangles
	for (GLuint i = 0; i < model->numtriangles; ++i) {
		GLMtriangle *f=&(model->triangles[i]);
		for (int j = 0; j < 3; ++j) {
			GLfloat *mvert = & ( model->vertices[f->vindices[j] * 3] );
			GLfloat *pvert = vertices[ base_face.polygons[i]->verts[j] ];

			if( !equal(pvert, mvert) ) {
				cout << "!=: " << "i = " << i << "  " << "poly vert: "
					<< stringv(pvert) << ", model vert:"
					<< stringv(mvert) << endl;
				// more info
				cout << "polygons[" << i << "] verts[" << j  << "] = "
						<< base_face.polygons[i]->verts[j] << endl
						<< "polygons[" << i << "] = " << base_face.polygons[i] << endl;
			}
		}
	}
	// check vertex normals
	for (GLuint i = 0; i < model->numtriangles; ++i) {
		GLMtriangle *f=&(model->triangles[i]);
		for (int j = 0; j < 3; ++j) {
			if( (GLint)f->nindices[j] != base_face.polygons[i]->norms[j] + 1 ) {
				cout << "!=: " << "i = " << i << "  " << "f->nindices[" << j
						<< "] = " << f->nindices[j] << ", "
						<< "base_face.polygons[i]->norms[" << j
						<< "] + 1 = " << base_face.polygons[i]->norms[j] + 1 << endl;
			}
		}
	}
	// check facet normals
	for (GLuint i = 0; i < model->numtriangles; ++i) {
		GLMtriangle *f=&(model->triangles[i]);
		GLfloat *n = &(model->facetnorms[f->findex * 3]);
		if(!equal( n, base_face.polygons[i]->facetnorm ) ) {
			cout << "!=: " << "i = " << i << " facet norm: "
			    << stringv(base_face.polygons[i]->facetnorm)
			    << ",  model:  " << stringv(n) << endl;
		}
	}
}


/**
 * Prints info about the selected range of polygons
 * from the indicated surface.
 * params:
 * which_surface: possible values = -1 -> all, BASE, EXTRUDED, SIDES
 * how_many: [-1] if -1, returns all polys in surface, otherwise
 * starts with start_index, returning next how_many polys
 */
void Syllable3D::print_polygons(int which_surface, int how_many,
		int start_index) {
	vector<Polygon*> polys;
	bool all = (how_many == -1);
	int past_end = start_index + how_many;
	switch(which_surface) {
	case BASE:
		if(all) past_end = base_face.polygons.size();
		for (int i = start_index; i < past_end; ++i) {
			polys.push_back(base_face.polygons[i]);
		}
		break;
	case EXTRUDED:
		if(all) past_end = extruded_face.polygons.size();
		for (int i = start_index; i < past_end; ++i) {
			polys.push_back(extruded_face.polygons[i]);
		}
		break;
	case SIDES:
		if(all) past_end = sides.size();
		for (int i = start_index; i < past_end; ++i) {
			polys.push_back(sides[i]);
		}
		break;
	default:
		cout << "print_polygons: bad surface: " << which_surface << endl;
		return;
	}
	cout << "***** Syllable3D::print_polygons():  *****" << endl;
	for (size_t i = 0; i < polys.size(); ++i) {
		Polygon *p = polys[i];
		cout << i << ": " << p->to_string() << endl;
		cout << "\tcenter: " << stringv(p->center)
				<< ", fnorm: " << stringv(p->facetnorm) << endl;
	}
	cout << "***** end Syllable3D::print_polygons():  *****" << endl;
}


// render an individual polygon
void Syllable3D::render_poly(const Polygon& p, vec3 color) {
	glPushAttrib(GL_COLOR_BUFFER_BIT);
	glNormal3fv(p.facetnorm);
	glColor3fv(color);
	if(p.size == 3) {
		glBegin(GL_TRIANGLES);
		//			glBegin(GL_LINES);
		//			glColor3f( .5, .5, 1 );
	} else if(p.size == 4) {
		glBegin(GL_QUADS);
	}
	for (int i = 0; i < p.size; ++i) {
		glVertex3fv( vertices[ p.verts[i] ] );
	}
	glEnd();
	glPopAttrib();
}

void Syllable3D::render_face(const Face& face, GLfloat *color, bool wire,
		bool no_mat, GLfloat line_sz) {

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	int numcolors = Util::numcolors;

	for (size_t p=0; p < face.polygons.size(); p++) {

		// if no materials set, set color and disable lighting
		if(no_mat) {
			glDisable(GL_LIGHTING);
			if(color != NULL) {
				glColor3fv(color);
			} else {
				// alternate colors 1 color per triangle
				glColor3fv( colors[p%numcolors]);

				// alternate colors over entire shape, 1 region per color
				// in order of colors array (starts with red, blue, ..)
				//			glColor3fv( colors[p/triangles_per_color]);
			}
		}
		Polygon *poly = face.polygons[p];

		if(wire) {
			glLineWidth(line_sz);
			glDisable(GL_LIGHTING);
			if(!no_mat) {
				vec3 col = {ambient_diffuse[0], ambient_diffuse[1], ambient_diffuse[2]};
				glColor3fv(col);
			}
			glBegin(GL_LINE_LOOP);
		} else {
			if(poly->size == 3) {
				glBegin(GL_TRIANGLES);
			} else if(poly->size == 4) {
				glBegin(GL_QUADS);
			}
		}
		for (int i = 0; i < poly->size; ++i) {
			glNormal3fv( normals[ poly->norms[i] ] );
			glVertex3fv( vertices[ poly->verts[i] ] );
		}
		glEnd();

		//*** debug draw normals
//		if(show_normals) {
//			glDisable(GL_LIGHTING);
//			if(show_facet_norms) {
//				//***draw facet normal coming out from center
//				poly->draw_facetnorm(Util::magenta, 1);
//			}
//			if(show_vert_norms) {
//				//**draw all vertex normals
//				poly->draw_normals(Util::magenta, 1);
//			}
//		}
	}
	glPopAttrib();
}

void Syllable3D::render_sides(GLfloat *color, bool wire,
		bool no_mat, GLfloat line_sz) {

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	int numcolors = Util::numcolors;
	// for using sequence
//	int polys_per_color =  10; //sides.size()/ numcolors;
	// color order: { Util::red, Util::purple, Util::blue, Util::green, Util::yellow,
	//				Util::grey, Util::magenta, Util::white, Util::cyan, Util::black };

	for (size_t p=0; p < sides.size(); p++) {

		// if no materials set, set color and disable lighting
		if(no_mat) {
			glDisable(GL_LIGHTING);
			if(color != NULL) {
				glColor3fv(color);
			} else {
				// alternate colors 1 color per triangle
				glColor3fv( colors[p%numcolors]);

				// alternate colors over entire shape, 1 region per color
				// in order of colors array (starts with red, blue, ..)
				//			glColor3fv( colors[p/triangles_per_color]);
			}
		}
		Polygon *poly = sides[p];
		glNormal3fv(poly->facetnorm);
		if(wire) {
			glLineWidth(line_sz);
			glDisable(GL_LIGHTING);
			if(!no_mat) {
				vec3 col = {ambient_diffuse[0], ambient_diffuse[1], ambient_diffuse[2]};
				glColor3fv(col);
			}
			glBegin(GL_LINE_LOOP);
		} else {
			if(poly->size == 3) {
				glBegin(GL_TRIANGLES);
			} else if(poly->size == 4) {
				glBegin(GL_QUADS);
			}
		}
		for (int i = 0; i < poly->size; ++i) {
			glVertex3fv( vertices[ poly->verts[i] ] );
		}
		glEnd();

		//*** debug draw normals
//		if(show_normals) {  //if(p > 87 && p < 101) {
//			glDisable(GL_LIGHTING);
//			if(show_facet_norms) {
//				//***draw facet normal coming out from center
//				poly->draw_facetnorm(Util::cyan, .5);
//			}
//			if(show_vert_norms) {
//				//**draw all vertex normals
//				poly->draw_normals(Util::cyan, .5);
//			}
//		}
	}
	glPopAttrib();
}
/**
 * Draw all normals the same size, if not given, will use average of max/min base face poly
 * side lengths
 * param: which -	'f': default - facetnorms, drawn in magenta
 * 					'v': vertex normals, drawn in cyan
 * 					'b': both
 */
void Syllable3D::draw_normals(char which, GLfloat size) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glLineWidth(1);
	glPointSize(1);
	glEnable(GL_POINT_SMOOTH);

//	static GLfloat t = 0;
//	GLfloat a = age(true);
//	if(t < a - 1.0) {
//		cout << "show_normals: " << show_normals << ", show_facet_norms: "
//				<< show_facet_norms << ", show_vert_norms: " << show_vert_norms << endl;
//		t = a;
//	}
	if(size < 0) {
		size = (base_face.longest_side_len + base_face.shortest_side_len) / 2;
	}
	// do faces
	PolygonArray polys;
	polys.insert(polys.end(), base_face.polygons.begin(), base_face.polygons.end());
	polys.insert(polys.end(), extruded_face.polygons.begin(), extruded_face.polygons.end());
	if(which == 'v' || which == 'b') {
		glColor3fv(Util::cyan);
		for (size_t i = 0; i < polys.size(); ++i) {
			const Polygon *p = polys[i];

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
		for (size_t i = 0; i < polys.size(); ++i) {
			const Polygon *p = polys[i];
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
	// do sides separately
	if(which == 'v' || which == 'b') {
		glColor3fv(Util::cyan);
		for (size_t i = 0; i < sides.size(); ++i) {
			const Polygon *p = sides[i];

			for (int j = 0; j < p->size; ++j) {
				Vec v(vertices[p->verts[j]]), vn(side_normals[p->norms[j]]);
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
		for (size_t i = 0; i < sides.size(); ++i) {
			const Polygon *p = sides[i];
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
// just draw debug stuff
void Syllable3D::render_debug(GLfloat *color) {
	bool draw_edges = true;
	bool draw_points = true;
	bool draw_verts = true;
	bool draw_polys = true;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	if( glIsEnabled(GL_LIGHTING) ) glDisable(GL_LIGHTING);
	glPointSize(4);
	glLineWidth(2);

	int numcolors = Util::numcolors;
	if(draw_points) {
		glBegin(GL_POINTS);
		for (size_t i = 0; i < debug_points.size(); ++i) {
			if(color != NULL) {
				glColor3fv(color);
			} else {
				glColor3fv(colors[i%numcolors]);
			}
			vec3 vert;
			debug_points[i].array_out(vert);
			glVertex3fv(vert);
		}
		glEnd();
	}
// color order: { Util::red, Util::purple, Util::blue, Util::green, Util::yellow,
//				Util::grey, Util::magenta, Util::white, Util::cyan, Util::black };

	if(draw_verts) {
		// to show sequence easier, show this many verts in order in each color
		int verts_per_color = 20; //debug_verts.size()/20;
				// for all:  debug_verts.size()/numcolors;
		glBegin(GL_POINTS);
		for (size_t i = 0; i < debug_verts.size()/3; ++i) {
			if(color != NULL) {
				glColor3fv(color);
			} else {
//				glColor3fv(colors[i%numcolors]);
				// alternate colors over entire shape, 1 region per color
				glColor3fv( colors[ (i/verts_per_color) % numcolors]);
			}
			glVertex3fv(vertices[debug_verts[i]]);
		}
		glEnd();
	}
	if(draw_polys) {
//		cout << "debug_polygons.size(): " << debug_polygons.size() << endl;
		int polys_per_color = 20;
		for (size_t i = 0; i < debug_polygons.size(); ++i) {
			Polygon *p = debug_polygons[i];
			GLfloat *color = colors[ (i/polys_per_color) % numcolors];
			render_poly(*p, color);
		}
	}
	if(draw_edges) {
		for (size_t i = 0; i < debug_edges.size(); ++i) {
			IndexedEdge e = debug_edges[i];
			glBegin(GL_LINES);
			if(color == NULL) {
				glColor3f(0, 0, 1);
			} else {
				glColor3fv(colors[i%numcolors]);
			}
			glVertex3fv(vertices[e.u]);
			glVertex3fv(vertices[e.v]);
			glEnd();
		}
	}
	// debug polygons if we need to see something
//	GLfloat color[3] = { 5., 0, .5 };
//	for (size_t i = 0; i < debug_polygons.size(); ++i) {
//		render_poly(*debug_polygons[i], color);
//	}
	if( glIsEnabled(GL_LIGHTING) ) glEnable(GL_LIGHTING);
	glPopAttrib();
}

void Syllable3D::print_verts(int start, int end) {
	if(end == -1) {
		end = num_vertices - 1;
	}
	for (int i = 0; i <= end; ++i) {
		cout << i << "\t" << stringv(vertices[i], 3, true) << endl;
	}
}
/**
 * Returns vector of pointers to all polygons
 */
PolygonArray Syllable3D::get_all_polys() {
	PolygonArray polys;
	polys.insert(polys.end(), base_face.polygons.begin(), base_face.polygons.end());
	polys.insert(polys.end(), extruded_face.polygons.begin(), extruded_face.polygons.end());
	polys.insert(polys.end(), sides.begin(), sides.end());
	return polys;
}

// get a copy of the actual vertex from its index
void Syllable3D::get_vert(GLint index, vec3 out) {
	copyv(out, vertices[index]);
}
// get a copy of the actual normal from its index
void Syllable3D::get_norm(GLint index, vec3 out) {
	copyv(out, normals[index]);
}

void Syllable3D::get_center(vec3 out) {
	copyv(out, center);
}

// export all perimeters of base face to a .node file
// assumes vertices are in xz plane
void Syllable3D::export_node_file(const char *nodefile) {
	ofstream out(nodefile);
	vector<Vec> all_verts;
	for (size_t i = 0; i < base_face.regions.size(); ++i) {
		Region *reg = base_face.regions[i];
		// outer perimeter
		for (size_t j = 0; j < reg->perimeter.size(); ++j) {
			Vec vert = Vec(vertices[reg->perimeter[j]]);
			all_verts.push_back(vert);
		}
		for (size_t j = 0; j < reg->inner_perimeters.size(); ++j) {
			for (size_t k = 0; k < reg->inner_perimeters[j].size(); ++k) {
				Vec vert = Vec(vertices[reg->inner_perimeters[j][k]]);
				all_verts.push_back(vert);
			}
		}
	}
	int num_verts = all_verts.size();
	cout << "export_node_file: exporting " << num_verts
			<< " verts,  from " << base_face.regions.size() << " regions" << endl;
	// header .node file
	out << num_verts << "  2  0  0 " << endl;
	for (size_t i = 0; i < all_verts.size(); ++i) {
		Vec v = all_verts[i];
		out << i << "  " << v.x << "  " << v.z << endl;
	}
	out << endl;
}

// export all perimeters of face as a pslg to a .poly file
// assumes 2d in xz plane,
// assumes only one hole per region, and assumes that hole is convex (1 midpoint taken)
void Syllable3D::export_poly_file(const char *polyfile) {
	ofstream out(polyfile);
	vector<Vec> all_verts;
	vector<vector<int> > segments; // indexed to all_verts - inner vector has size 2
	vector<Vec> holes; // one point per hole

	for (size_t i = 0; i < base_face.regions.size(); ++i) {
		Region *reg = base_face.regions[i];
		// outer perimeter
		// first vert
		all_verts.push_back( Vec(vertices[reg->perimeter[0]]) );
		int first_index = all_verts.size()-1;
		for (size_t j = 1; j < reg->perimeter.size(); ++j) {
			Vec vert = Vec(vertices[reg->perimeter[j]]);
			all_verts.push_back(vert);
			int curr_index = all_verts.size()-1;
			vector<int> seg;  seg.push_back(curr_index-1); seg.push_back(curr_index);
			segments.push_back(seg);
		}
		// last segment
		vector<int> seg;  seg.push_back(all_verts.size()-1); seg.push_back(first_index);
		segments.push_back(seg);

		// inner perimeters -- assume only 1 !
		// need to specify holes with a point inside the whole
		if( reg->inner_perimeters.size() > 1 ) {
			cout << "reg->inner_perimeters.size() > 1:  exiting " << endl;
			exit(1);
		}
		for (size_t j = 0; j < reg->inner_perimeters.size(); ++j) {
			// first vert
			Vec first = Vec(vertices[reg->inner_perimeters[j][0]]);
			Vec mid;
			all_verts.push_back( first );
			first_index = all_verts.size()-1;
			for (size_t k = 1; k < reg->inner_perimeters[j].size(); ++k) {
				Vec vert = Vec(vertices[reg->inner_perimeters[j][k]]);
				all_verts.push_back(vert);
				// get middle vertex for defining hole
				if( k == reg->inner_perimeters[j].size()/ 2) {
					mid = Vec(vert);
				}
				int curr_index = all_verts.size()-1;
				vector<int> segm;  segm.push_back(curr_index-1); segm.push_back(curr_index);
				segments.push_back(segm);
			}
			// last segment
			vector<int> segm;  segm.push_back(all_verts.size()-1); segm.push_back(first_index);
			segments.push_back(segm);
			// define hole
			Vec pt_in_hole;
			midpoint(first, mid, pt_in_hole);
			holes.push_back(pt_in_hole);
		}
	}
	int num_verts = all_verts.size();
	cout << "export_poly_file: export file: " << polyfile << endl
			<< " exporting " << num_verts
			<< " verts,  from " << base_face.regions.size() << " regions" << endl;
	cout << "segments:  " << segments.size() << ",  holes:  " << holes.size() << endl;
	// header # vertices, etc
	out << "# vertices " << endl;
	out << num_verts << "  2  0  0 " << endl;
	for (size_t i = 0; i < all_verts.size(); ++i) {
		Vec v = all_verts[i];
		out << i << "  " << v.x << "  " << v.z << endl;
	}
	out << endl;
	out << "# segments " << endl;
	out << segments.size() << "  0 " << endl;
	for (size_t i = 0; i < segments.size(); ++i) {
		out << i << "  " << segments[i][0] << "  " << segments[i][1] << endl;
	}
	out << endl;
	out << "# holes " << endl;
	out << holes.size() << endl;
	for (size_t i = 0; i < holes.size(); ++i) {
		Vec v = holes[i];
		out << i << "  " << v.x << "  " << v.z << endl;
	}
}

void Syllable3D::test() {
	Syllable3D syll3d;
	syll3d.initFromObj("data/om.obj");
	syll3d.init_base();

	cout <<  "\n******************** Syllable3D::test() **************************"
			<< endl;
//	cout->flush();
	// test polygon == and !=
	for (size_t i = 0; i < syll3d.base_face.polygons.size(); ++i) {
		Polygon *bp = syll3d.base_face.polygons[i];
		Polygon p(*bp);
		if(!(p == *bp)) {
			cout << "!=: polygon == : " << i << "  " << bp->to_string() << endl;
		}
		if(p != *bp) {
			cout << "!=: polygon != : " << i << "  " << bp->to_string() << endl;
		}
	}

	for (size_t i = 0; i < syll3d.base_face.polygons.size(); ++i) {
		Polygon *bp = syll3d.base_face.polygons[i];
		// just print some polys
//		if(i < 100) cout << bp->to_string() << endl;
		for (int j = 0; j < bp->size; ++j) {
			IndexedEdge e = bp->edges[j];
			GLint vert = bp->verts[j];
			// test verts are in proper range
			if(vert < 0 ||vert >= syll3d.num_vertices) {
				cout << "!=: vert: verts[" << j << "] = " << vert
						<< "  poly: " << bp->to_string() << endl;
			}
			// test edges
			if(e.u != vert || e.v != bp->verts[(j+1)%bp->size]) {
				cout << "!=: edge: " << e.to_string() << "  poly: " << bp->to_string() << endl;

			}
		}

	}



	cout << "\n***************** Done:  Syllable3D::test() ***********************"
			<< endl;
}
