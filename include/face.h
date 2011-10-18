/*
 * face.h
 *
 * Defines Region, and Face classes that are hierarchical
 * groupings of polygons making  up a bounded surface that
 * can have holes.
 *
 *  Created on: Aug 2, 2010
 *      Author: drogers
 */

#ifndef FACE_H_
#define FACE_H_

#include "poly.h"
#include "vec.h"

#include <vector>

using DR::vec3;
using DR::Polygon;
using DR::Triangle;
using DR::Quad;
using DR::IndexedEdge;
using DR::Vec;

using std::vector;

// A region is a connected group of polygons
// bounded by an outer perimeter,
// and possible a number of inner perimeters (edges of holes)
// in general, start each perimeter with vertex having min x value
// and keep in order using right hand winding (ie cross product
// of successive edges in a perimeter points up, however defined)
struct Region {
	vector<GLint> perimeter;
	vector<vector<GLint> > inner_perimeters;
	vector<Polygon *> polygons;
	bool contains(const Polygon *poly);
	bool contains(GLint vert);
};

class Syllable3D;

//// Vertices that define the boundaries of a face
//// plus the center - just for general orientation
////
//struct FaceBounds {
//	Vec up;		// direction of up
//	Vec left;	// vertex highest up
//	Vec right;	// vertex furthest from center along direction -90 degrees from up
//	Vec top;
//	Vec bottom;
//	Vec center;
//	FaceBounds() {}
//};

// A face of a 3d syllable,
// consists of Polygons, divided into regions
class Face {

public:
	vector<Region *> regions;
	vector<Polygon *> polygons;
	Face();
	Face(Syllable3D *syll);
	~Face();
	// add polygon created with new
	void add_polygon(Polygon *p);
	// remove all polygons and regions from this face, clearing storage
	void clear();
	// overall initialization and setup of regions
	void init_regions(GLint start_vert);
	// expand neighbors of start until region is defined
	void grow_region(Region& region, Polygon *start);
	// returns polygon containing all verts
	void get_poly(vector<GLint> verts, Polygon& out);
	// returns index of poly in face.polygons vector
	// returns -1 if poly not found
	GLint get_poly_index(const Polygon *poly) const;
	void create_perimeters(Region& region, vector<vector<GLint> >& perimeters, bool debug=false);
	// perimeters are always (we hope) in right handed winding order with respect to the outside of the face
	// so if you flip the face, reverse the windings if desired (for instance, create_sides depends on face winding)
	// reverses all perimeter windings
	void reverse_perimeter_windings();
	void find_polys_containing(vector<Polygon *>& out, GLint vert);
	void find_polys_containing(vector<Polygon *>& out, const IndexedEdge& edge);
	void get_neighbors(vector<Polygon *>& out, const Polygon *poly);
	/**
	 * Get center of center polygon.
	 * Asserts center poly is set.
	 */
	void get_center(GLfloat *out);
	/**
	 * Get copy of center poly, asserts that there is one.
	 */
	void get_center(Polygon& out);
	/**
	 * Get the index of center poly.
	 * Asserts center poly is set.
	 */
	GLint get_center_index();
	// tell face to decide which is center poly. for simplicity,
	// assume syllable is flat on xz plane as it first
	// comes in (ie this will assert up = Vec(0, 0, 1)
	void set_center(Vec up);
	// if you have a center point, this will set the
	// center poly with it
	void set_center(vec3 centerpt);

	// just a metric if desired, the length of the shortest and
	// longest sides of all polygons, set by init_regions
	GLfloat shortest_side_len;
	GLfloat longest_side_len;

	vector<Polygon*> debug_polygons;
	vector<IndexedEdge> debug_edges;
	vector<GLint> debug_verts;
	vector<Vec> debug_points;

private:

	Syllable3D *parent;
	// index of center poly
	GLint center_index;
};



#endif /* FACE_H_ */
