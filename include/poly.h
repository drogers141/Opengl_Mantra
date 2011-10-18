/*
 * poly.h
 *
 * Opengl  - utilities relating to polygons that use indexed vertices, employs namespace DR
 *
 *  Created on: Jun 15, 2010
 *      Author: drogers
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>
#include <string>
#include <vector>

#include "mygl.h"
#include "dr_util.h"
#include "vec.h"

// a namespace for things that might easily clash
// may extend this
namespace DR {

using std::ostream;
using std::string;
using std::vector;

// bidirectional graph edge
struct IndexedEdge {
	GLint u;
	GLint v;
	IndexedEdge() {}
	IndexedEdge(const IndexedEdge&  other) {
		u = other.u;
		v = other.v;
	}
	GLint other_end(GLint vert) {
		return vert == u ? v : u;
	}
	bool operator==(const IndexedEdge& other) const {
		return (other.u == u && other.v == v) || (other.u == v && other.v == u);
	}
	bool operator!=(const IndexedEdge& other) const {
		return !(*this == other);
	}
	string to_string();
	bool contains(GLint vert) {
		return (vert == u || vert == v);
	}
};

// Polygon -- primitive (either quad or triangle)
// assumes access to an array somewhere of vec3 vertices and normals
// that it indexes
class Polygon {
	friend ostream& operator<<(ostream&, const Polygon&);
public:
	Polygon();
	// vertices and normals are the arrays the GLint indices index
	Polygon(GLint size, vec3 *vertices, vec3 *normals);
	Polygon(const Polygon& p);
	virtual ~Polygon();
	// assignment makes a deep copy of everything
	// but actual_verts and actual_norms point to same storage
	const Polygon& operator=(const Polygon& other);
	// at this point == compares verts and norms by index
	// not by actual vertex or normal contents
	bool operator==(const Polygon& other) const;
	bool operator!=(const Polygon& other) const {
		return !(*this == other);
	}
	GLint size;
	GLint *verts;
	GLint *norms;
	vec3 facetnorm;
	// polygons have edges, of course only if you need them ..
	IndexedEdge *edges;
	// what the center is depends on the polygon
	// in the case of irregular quads and triangle,
	// it's the centroid
	vec3 center;

	/**
	 * Length of longest side. Good for various geometric approximations.
	 * Reset whenever resizing.
	 * Called automatically by set_edges, which is called by the winding functions.
	 */
	void set_max_side_length();
	GLfloat max_side_length;
	/**
	 * need to call this after vertices are set or modified
	 * base class prints message to stderr
	 */
	virtual void set_center() { std::cerr << "Polygon base class set_center() called" << std::endl; }
	// call after vertices are set or changed, edges follow order of vertices
	// and connect last back to first
	void set_edges();
	// sets the facet norm, by default uses order of first 3 verts
	// applying cross product: (v0, v1) X (v1, v2)
	// supply vertices[3] and above will be used with param vertices[]
	void set_facetnorm(GLint *vertices=NULL);

	// ensures that polygon has right handed winding based on the normal
	// if no normal is passed in, uses first vertex normal ( actual_norms[norms[0]] )
	void set_winding_from_normal(GLfloat *normal=NULL);

	void reverse_winding();

	bool contains(GLint vert);
	bool contains(const IndexedEdge& edge);
	/**
	 * Note:  depends on DR::FLOAT_TOLERANCE
	 */
	bool contains(const vec3 actual_vert);

//	virtual void get_actual_verts(GLfloat **out);

	// return string representation, if show_edges is true, adds edges
	virtual string to_string(bool show_edges=false) const;

	/**
	 * Get a copy of the polys vertices as Vecs to play with.
	 */
	void get_actual_verts(vector<Vec>& out) const ;

	/**
	 * render facet normal using a colored line drawn from the center of the poly
	 * assumes center has been set
	 * param: scale - scale factor multiplied by length of a side to create line length
	 */
	void draw_facetnorm(const GLfloat *color, GLfloat scale=2) const;
	// same as above but for each norm, at the vertex
	void draw_normals(GLfloat *color, GLfloat scale=2);
	// set this polygon's pointer to the array holding its vertex normals
	void set_normals_storage(vec3 *normals_storage) { actual_norms = normals_storage; }

	virtual void render(GLfloat *color=NULL, bool use_fnorm=true);
	/**
	 * Disables gl_lighting
	 * wire: draw wireframe
	 * line_sz: if wire, line size [1]
	 */
	virtual void render_no_lighting(const GLfloat *color, bool wire=false, GLfloat line_sz=1) const;

	/**
	 * Returns true if ray intersects this poly.
	 * If found, intersecting point will be put in out.
	 * The ray is defined by a Vec, ray0, and a direction vector, ray_dir
	 * The plane is defined by a Vec, plane0, and its normal, plane_norm
	 * Base class prints error and returns false;
	 */
	virtual bool ray_intersect(Vec& out, Vec ray0, Vec ray_dir) const {
		std::cerr << "Polygon base class ray_intersect() called" << std::endl;
		return false;
	}

	/**
	 * Returns true if a line segment joining points pt1 and pt2
	 * penetrates this polygon by at least delta (ie 1 point should not
	 * be in the polygon's plane.
	 * If true, intersection point will be in intersection
	 */
	bool line_penetrates(const Vec& pt1, const Vec& pt2,
			GLfloat delta, Vec& intersection) const;

	/**
	 * RELIES ON FACETNORM BEING SET
	 * Returns true if point is above the plane of this polygon,
	 * with the facetnorm being considered up.
	 */
	bool is_above_poly(const Vec& point) const;

	/**
	 * Init precomputed stuff for ray intersection.
	 * Called by set_edges, so shouldn't have to worry.
	 */
	virtual void init_precomputed() {
		std::cerr << "Polygon base class init_precomputed() called" << std::endl;
	}
	/**
	 * precomputed info used in testing if points are in a triangle
	 * quad will use 2 of these
	 */
	struct TrianglePrecompute{
		GLfloat d;
		Vec u_beta;
		Vec u_gamma;
	};
	TrianglePrecompute precomputed;

protected:
	// references to vertex and normal storage
	vec3 *actual_verts;
	vec3 *actual_norms;


//	virtual void render();
};

class Triangle: public Polygon {
public:
	Triangle(vec3 *vertices, vec3 *normals)
	: Polygon(3, vertices, normals) {}
	Triangle(const Triangle& t): Polygon(t) {}
	void set_center();
	/**
	 * Returns true if ray intersects this poly.
	 * If found, intersecting point will be put in out.
	 * The ray is defined by a Vec, ray0, and a direction vector, ray_dir
	 */
	bool ray_intersect(Vec& out, Vec ray0, Vec ray_dir) const;
	void init_precomputed();
	// return string representation, if show_edges is true, adds edges
	// prefixes output with "Triangle: "
	string to_string(bool show_edges=false) const;


private:
	Triangle() {}
};

class Quad: public Polygon {
public:
	Quad(vec3 *vertices, vec3 *normals)
	: Polygon(4, vertices, normals) {}
	Quad(const Quad& q): Polygon(q) {}
	void set_center();
	/**
	 * Returns true if ray intersects this poly.
	 * If found, intersecting point will be put in out.
	 * The ray is defined by a Vec, ray0, and a direction vector, ray_dir
	 */
	bool ray_intersect(Vec& out, Vec ray0, Vec ray_dir) const;
	void init_precomputed();
	// data for a second triangle test for intersections
	TrianglePrecompute precomputed2;
	// return string representation, if show_edges is true, adds edges
	// prefixes output with "Quad: "
	string to_string(bool show_edges=false) const;

private:
	Quad() {}
};

/**
 * Comparison operator for poly pointers, compares by index verts and facetnorm
 */
struct PolygonCompare: std::binary_function<const Polygon *, const Polygon *, bool> {
		bool operator()(const Polygon *lhs, const Polygon *rhs) const;
};

// print out vector of polys with indices of vector
// set a limit for printing if desired, -1 -> no limit
// print_p: print the pointer as well
void print_polys(const std::vector<Polygon*>& polys, int limit=-1, bool print_p=false);

//*** note these implementations could be done somehow with stl,
// but since I'm using pointers, and I don't want to compare pointers
// it's a thing

// puts the intersection of v1 and v2 in intersect
// clears intersect before and removes duplicates from intersect after
void intersection(const vector<Polygon*>& v1, const vector<Polygon*>& v2,
		vector<Polygon*>& intersect);

// puts elements of the set difference of v1 and v2 in out
void difference(const vector<Polygon*>& v1, const vector<Polygon*>& v2,
		vector<Polygon*>& out);

// removes duplicates from v
void uniq(vector<Polygon*>& v);

// returns true if v contains p (not pointer, actual poly)
bool contains(const vector<Polygon*>& v, const Polygon& p);

// todo ** fix this - unnecessary after better acquainted with stl
// add all to the back of another vector
template<class T>
void add_all(const vector<T>& from, vector<T>& to) {
	copy(from.begin(), from.end(), back_inserter(to));
}

} // ** end namespace DR **


#endif /* UTIL_H_ */
