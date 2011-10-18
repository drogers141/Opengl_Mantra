/*
 * cylinder_model.h
 *
 *  Created on: Jun 25, 2010
 *      Author: drogers
 */

#ifndef CYLINDER_MODEL_H_
#define CYLINDER_MODEL_H_

#include <iostream>
#include <vector>

#include "vec.h"
#include "geo.h"

using std::cout;
using std::endl;
using std::vector;
using DR::Vec;
using DR::vec3;

// a convenience typdef meant to contain just an x and y
// no actual constraint on size, so ..
//class Vec2: public vector<GLfloat> { };
typedef vector<GLfloat> Vec2;

// anything that can be expressed as a set of vertices in 2d
// assume incoming verts are Vecs (3d) with one component that will be ignored when
// mapping from 3d to 2d
class Mappable2D {
public:
	Mappable2D() { }
	~Mappable2D() {
//		for (int i = 0; i < verts3d.size(); ++i) {
//			delete verts3d[i];
//			delete [] verts2d[i];
//		}
	}
	void add_vert(const Vec& vert) {
//		cout << vert << endl;
//		Vec *v = new Vec(vert);
//		verts3d.push_back(v);
		verts3d.push_back( Vec(vert));
	}
	// zero component: 0, 1, 2  for x, y, z whichever should be ignored
	// this method should be overridden for different behavior
	void map(int zero_component);


	vector<Vec> verts3d;
//	vector<GLfloat*> verts2d;
	vector<Vec2> verts2d;
};

// a unit space is a 2d unit area that will be mapped to a location on the cylinder
class UnitSpace2D {
public:
	UnitSpace2D() { }

	// scale or translate vertices of the element in this space
	// param: which - 0 -> scale x only, 1 -> scale y only
	//		default == -1, any value other than 0,1 scales both
	void scale(GLfloat factor, GLint which=-1);
	void translate(GLfloat x, GLfloat y);
	// invert in x or y direction
	// param: which - 0 -> x, 1 -> y
	void flip(GLint which);

	Mappable2D element;
};

//class VertexSet: public vector<Vec> {};
//class NormalSet: public vector<Vec> {};
typedef vector<Vec> VertexSet;
typedef vector<Vec> NormalSet;

// debug - grid on cylinder
// could be generalized, but for now uses line loop in rendering
class Grid {
public:
	// x lines extend in x direction -- ie latitude
	vector<VertexSet> x_lines;
	// y_lines -- ie longitude
	vector<VertexSet> y_lines;
	void render(GLfloat *color, GLfloat line_width=1.0);
};

// maps vertices from original mappable2d, contained in a unit space
// to a location on a cylinder, along with producing normals for each vertex
// the cylinder circumference is divided evenly among the unit spaces
// the height of the cylinder will be the height of the unit spaces as well
// the incoming vertices to the unit space's mappable will have the same indices
// as the outgoing vertex and normal sets
// the cylinder has the y axis as its axis and is centered at the origin
class CylinderModel {
public:
	CylinderModel() { }
	CylinderModel(GLfloat radius, GLfloat height) {
		this->radius = radius;
		this->height = height;
	}
	~CylinderModel();
	// add a unit space to be mapped
	// return:  index into sets of vertices
	//          and normals produced by mapping
	int add_unit_space(UnitSpace2D unit_space) {
		unit_spaces.push_back(unit_space);
		vertex_sets.push_back(VertexSet());
		normal_sets.push_back(NormalSet());
		return unit_spaces.size() - 1;
	}
	// remove all unit spaces, vertex and normal sets
	void clear() {
		unit_spaces.clear();
		vertex_sets.clear();
		normal_sets.clear();
	}
	// does all the work
	// populates vertex_sets and normal_sets with transformed vertices and normals
	void map();

	/**
	 * map a point in unit space coordinates to the cylinder
	 * params: unit_space - index of unit space on cylinder
	 *     x,y - x and y values over [0, 1] within the unit space
	 */
	void map_point(int unit_space, GLfloat x, GLfloat y, vec3 out);

	// get a mesh grid on the circumference surface of this cylinder
	// latitudes: y values from -1=bottom to 1=top
	// longitudes: theta values, degrees from 0 to 360
	void grid(Grid& out, const vector<GLfloat>& latitudes,
			const vector<GLfloat>& longitudes);
	// divides vertical and horizontal areas evenly using numlats, numlongs
	void grid(Grid& out, GLint num_lats, GLint num_longs);

	GLfloat radius;
	GLfloat height;
	vector<UnitSpace2D> unit_spaces;
	vector<VertexSet>  vertex_sets;
	vector<NormalSet>  normal_sets;

};

#endif /* CYLINDER_MODEL_H_ */
