/*
 * vec.h
 *
 *	An obj. oriented point or vector class, 3d. All are Opengl GLfloat related.
 *
 *  Created on: Jun 21, 2010
 *      Author: drogers
 */

#ifndef VEC_H_
#define VEC_H_

#include <iostream>
#include <string>
#include <set>

#include "mygl.h"
#include "dr_util.h"


namespace DR  {


using std::ostream;
using std::string;


/**
 * Simple 3d vector class
 * Note:  be careful with storage, to streamline arithmetic operations, many functions
 * and operators return local copies of a Vec.  This way they can be used in chains.
 * Since there is an assignment operator and copy constructor, as long as an expression
 * is assigned to a Vec, then you have that Vec available in whatever scope.
 */
class Vec {
public:
//	friend ostream& operator<<(ostream&, const Vec&);
public:
	GLfloat x, y, z;

	Vec(GLfloat _x=0, GLfloat _y=0, GLfloat _z=0)
	: x(_x), y(_y), z(_z) {	}

	Vec(const Vec& other)
	: x(other.x), y(other.y), z(other.z) { }

	Vec(const GLfloat other[3])
	: x(other[0]), y(other[1]), z(other[2]) { }

	string to_string() const;

	/**
	 * Calls glVertex3f(x, y, z)
	 */
	void glVertex() const {
		glVertex3f(x, y, z);
	}

	/**
	 *  use this if you want an outgoing array copy
	 */
	void array_out(GLfloat out[3]) const {
		out[0] = x;
		out[1] = y;
		out[2] = z;
	}
	GLfloat magnitude() const;

	Vec unit_vec();
	void normalize();
	/**
	 * Cross product of this vector and v.
	 */
	Vec cross(const Vec& v) const;

	/**
	 * Dot product
	 */
	GLfloat dot(const Vec& v) const {
		return v.x * x + v.y * y + v.z * z;
	}

	/**
	 * Equal within tolerance.  == uses DR::FLOAT_TOLERANCE
	 * Use this to specify a tolerance.
	 */
	bool equal_within(const Vec& other, GLfloat tolerance);

	// subscript
	// calls exit if bad index
	GLfloat operator[](GLint index);
	const Vec& operator=(const Vec& other_vec);
	const Vec& operator=(const GLfloat other[3]);
	const Vec& operator+=(const Vec& other_vec);
	const Vec& operator-=(const Vec& other_vec);
	bool operator==(const Vec& other) const;
	bool operator!=(const Vec& other) const;

};

Vec operator+(Vec vec1, Vec vec2);
Vec operator*(GLfloat scalar, Vec vec);
Vec operator*(Vec vec, GLfloat scalar);
Vec operator-(Vec vec1, Vec vec2);

Vec operator*(GLfloat matrix[16], Vec p);

/**
 * Comparison functor for Vec objects.  Can therefore use in sets, etc
 */
struct VecCompare: std::binary_function<Vec, Vec, bool> {
	bool operator()(const Vec& lhs, const Vec& rhs) const {
		if(lhs.x < rhs.x) {
			return true;
		} else if(lhs.y < rhs.y) {
			return true;
		} else if(lhs.z < rhs.z) {
			return true;
		}
		return false;
	}
};

GLfloat dist(Vec pt1, Vec pt2);

void midpoint(const Vec& v1, const Vec& v2, Vec& midpt);

/**
 * Intersection of 2 lines defined by a point and a direction vec.
 * return: true if lines intersect, false otherwise
 * params: pt1, dir1 - point and direction vector for line 1
 * pt2, dir2 - for line 2
 * intersect: point of intersection if return value is true
 */
bool line_intersect(const Vec& pt1, const Vec& dir1,
		const Vec& pt2, const Vec& dir2, Vec& intersect);

// draw line between 2 points
void draw_line(const Vec& pt1, const Vec& pt2, const GLfloat *color,
		GLfloat linesz=1, bool disable_lighting=true);

class LineSeg {
public:
	Vec start;
	Vec end;
	vec3 color;
	GLfloat width;
	LineSeg() {}
	LineSeg(const Vec& start, const Vec& end, const GLfloat *color=Util::grey, GLfloat width=1)
	: start(start), end(end), width(width) {
		copyv(this->color, color);
	}
	LineSeg(const LineSeg& other)
	: start(other.start), end(other.end), width(other.width) {
		copyv(this->color, other.color);
	}
	/**
	 * Calls draw_line() - ie sets up opengl state.
	 */
	void draw() const {
		draw_line(start, end, color, width);
	}
	/**
	 * Only calls glColor and glVertex.
	 */
	void draw_gl() const {
		glColor3fv(color);
		start.glVertex();
		end.glVertex();
	}
};

struct LineSegCompare: std::binary_function<LineSeg, LineSeg, bool> {
	VecCompare vecless;
	Vec3Compare vec3less;
	bool operator()(const LineSeg& lhs, const LineSeg& rhs) const {
		if( vecless(lhs.start, rhs.start) ) {
			return true;
		} else if( vecless(lhs.end, rhs.end) ) {
			return true;
		} else if( lhs.width < rhs.width ) {
			return true;
		} else if( vec3less(lhs.color, rhs.color) ) {
			return true;
		}
		return false;
	}
};

typedef std::set<LineSeg, LineSegCompare> LineSet;

/**
 * Draws a line with a point for the tip of the vector, ie the direction it is pointing.
 * Later, fix this if possible to use a cone or arrows, but non-trivial, and
 * possibly dependent on how the world coordinates are set up--ie trackball, etc.
 */
void draw_vector(Vec tail, Vec tip, const GLfloat *color, const GLfloat *tipcolor,
		GLfloat linesz=1, bool disable_lighting=true);

void draw_point(Vec pos, const GLfloat color[3], GLfloat size=1);

/**
 * Checks for intersection of a ray and a plane.
 * The ray is defined by a Vec, ray0, and a direction vector, ray_dir
 * The plane is defined by a Vec, plane0, and its normal, plane_norm
 * If the return value is true, the intersection Vec will be in out
 */
bool ray_intersect(Vec& out, Vec ray0, Vec ray_dir,
		Vec plane_norm, Vec plane0);


////************************* Matrix Stuff **********************////
/**
 * Pull out the (transformed) Vec from a row order mat
 */
//void getVec(Vec& out, GLfloat m[16]);


}  // **** end DR namespace

namespace std {
ostream& operator<<(ostream&, const DR::Vec&);
}

#endif /* VEC_H_ */
