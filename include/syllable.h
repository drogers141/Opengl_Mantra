/*
 * syllable.h
 *
 * For working with Tibetan syllables.
 * They have been created in Blender and are coming in from obj files.
 * First step is to get them into opengl as 2d in the xz plane with normals
 * = (0, 1, 0).
 *
 *  Created on: May 31, 2010
 *      Author: drogers
 */

#ifndef SYLLABLE_H_
#define SYLLABLE_H_

#include "poly.h"
#include "vec.h"
#include "face.h"
#include "geo.h"

extern "C" {
#include "glm.h"
}
// for linux, need to make sure this is after my headers
// that include mygl.h so glew.h is included before gl.h

#include <vector>
#include <ctime>

using DR::vec3;
using DR::Polygon;
using DR::Triangle;
using DR::Quad;
using DR::IndexedEdge;
using DR::Vec;

using std::vector;


// wrapper around a glm model for 2d model coming in (xz plane up = +y)
class Syllable2D {
public:
	Syllable2D() { model = NULL; }
	~Syllable2D();
	static void InitFromObj(Syllable2D& syllable, const char* objfile, bool unitize=false);
	GLMmodel* getModel();

private:
	GLMmodel 	*model;
};

// Syllable3D:
// Working out representation:
// In general a 3d syllable has 2 faces which are polygon meshes
// these are connected by quads that form the sides (ie thickness)
// the 3d syllable maintains a copy of a 2d syllable which is a wrapper
// around a glm model of the syllable in 2d coming in from an obj file
//
class Syllable3D {
public:
	Syllable3D();
//	Syllable3D(char* objfile);
//	Syllable3D(const Syllable3D& other);
	virtual ~Syllable3D();

	// indicate a face or sides in params
	// (didn't want to have to add an enum)
	static const int BASE = 0;
	static const int EXTRUDED = 1;
	static const int SIDES = 2;

	// render the syllable
	// if color is set and no_mat is true, color will be used
	// no_mat = no materials set
	void render(GLfloat *color=NULL, bool no_mat=false);
	// render using wireframe
	// color: defaults to white
	void render_wire(GLfloat *color=NULL, bool no_mat=false, GLfloat line_sz=1);
	// just draw debug stuff
	void render_debug(GLfloat *color=NULL);

	void initFromObj(const char* objfile, bool unitize=false);

	GLMmodel* getBase2dModel() { return base2d.getModel(); }

	// initialize base_face using base2d model information
	void init_base();
	/**
	 * removes sides and extruded face and resets base_face from model
	 * clears all debug stuff as well
	 */
	void reinit();

	// reset winding for all base polygons
	// for now uses set_winding_from_normal()
	void reset_base_windings();

	// for each face, set all vertex normals to be the average of the facet normals
	// of each polygon the vertex is contained in (within that face, ie don't consider sides)
	void average_vertex_normals();

	// extrudes base_face into 3d syllable, creates extruded face and sides
	// param: thickness, thickness of extrusion
	// param: rev_side_winding [false], if true, will reverse vertex order
	//		of polygons in sides and set facetnorms accordingly
	virtual void extrude(GLfloat thickness, bool rev_side_winding=false);

	// called by extrude to create sides
	// param: rev_winding [false], if true, will reverse vertex order
	//		  and set facetnorm accordingly
	void create_sides(bool rev_winding=false);

	// get a copy of the actual vertex from its index
	void get_vert(GLint index, vec3 out);
	// get a copy of the actual normal from its index
	void get_norm(GLint index, vec3 out);

	// export all perimeters of face to a .node file
	// for input to triangle program
	void export_node_file(const char *nodefile);

	// export all perimeters of face as a pslg to a .poly file
	// for input to triangle program
	void export_poly_file(const char *polyfile);

	// just all purpose testing
	static void test();

	int num_polygons() {
		int total = base_face.polygons.size() + extruded_face.polygons.size() + sides.size();
		return total;
	}
	// check all normals are normalized, print out any that are not
	void check_normals();

	/**
	 * Prints info about the selected range of polygons
	 * from the indicated surface.
	 * params:
	 * which_surface: possible values = -1 -> all, BASE, EXTRUDED, SIDES
	 * how_many: [-1] if -1, returns all polys in surface, otherwise
	 * starts with start_index, returning next how_many polys
	 */
	void print_polygons(int which_surface, int how_many=-1,
			int start_index=0);

	vector<Polygon*> debug_polygons;
	vector<IndexedEdge> debug_edges;
	vector<GLint> debug_verts;
	vector<Vec> debug_points;

	// num_vertices is the total number of vertices
	GLint num_vertices;
	// number of vertices in base_face
	GLint num_vertices_base;
	// same as above, for vertex normals
	GLint num_normals;
	GLint num_normals_base;
	// number of vertex normals in sides
	GLint num_normals_sides;

	// show normals controls display of normals at all
	// if it is on, show facet or vert or both based on the other flags
	bool show_normals;
	bool show_facet_norms;
	bool show_vert_norms;

	vec3* get_vertices() {
		return vertices;
	}
	vec3* get_normals() {
		return normals;
	}

	// center is set during init_base, then during extrude
	// so after one of these, you should get something of interest
	void get_center(vec3 out);

	// material properties
	// ambient and diffuse
	GLfloat ambient_diffuse[4];
	GLfloat specular[4];
	GLfloat shininess[1];
	GLfloat emissive[4];

	// point to set manually however looks good
	vec3 assigned_center;

	/**
	 * Age in milliseconds since constructor called.
	 * param: secs: if true, return secs rather than ms
	 */
	GLdouble age(bool secs=false);

protected:
	Syllable2D 	base2d;
	// if color is supplied it will be used when rendering if no_mat is true
	// (no materials set), otherwise materials for the syllable will override color
	// if wire -> render with wireframe, using line_sz
	void render_face(const Face& face, GLfloat *color=NULL,
			bool wire=false, bool no_mat=false, GLfloat line_sz=1);
	// same rules as with faces
	void render_sides(GLfloat *color=NULL, bool wire=false,
			bool no_mat=false, GLfloat line_sz=1);
	/**
	 * Draw all normals the same size, if not given, will use average of max/min base face poly
	 * side lengths
	 * param: which -	'f': default - facetnorms, drawn in magenta
	 * 					'v': vertex normals, drawn in cyan
	 * 					'b': both
	 */
	void draw_normals(char which='f', GLfloat size=-1.0);
	// render the glm model that we started from
	void render_model();
	// render an individual polygon
	void render_poly(const Polygon& p, vec3 color);

	// set all vertex normals in face to be the average of the facet normals
	// of each polygon the vertex is contained in
	void average_vertex_normals(Face& face);

	// checks base face to glmModel
	void check_base_model();
	// checks extruded face to base face
	void check_extruded_to_base();
	void print_verts(int start=0, int end=-1);
	/**
	 * Returns vector of pointers to all polygons
	 */
	DR::PolygonArray get_all_polys();

	// all vertices, first half should be from base2d
	vec3 *vertices;
	// normals match up with vertices
	vec3 *normals;

	// base face is made up of base2d vertices
	// but normals will be opposite
	Face base_face;
	// this face is created from extruding into 3d
	Face extruded_face;

	// sides are quads connecting the 2 faces generated from perimeters of each region
	// the vertices of each quad are indices into the existing vertices array,
	// however new storage is created for the vertex normals
	vector<Polygon*> sides;

	// storage for vertex normals for side polygons
	vec3 *side_normals;

	// center of syllable, be wary,
	// initialized to set to {0, 0, 0}, reset by init_base, extrude
	vec3 center;

	clock_t start_time;
};



#endif /* SYLLABLE_H_ */
