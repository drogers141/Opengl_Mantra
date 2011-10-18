/*
 * vec.cpp
 *
 *	Implementation for stuff in vec.h
 *
 *  Created on: Jun 21, 2010
 *      Author: drogers
 */

#include "vec.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <cmath>


using namespace std;
using namespace DR;

static bool _equal(GLfloat f1, GLfloat f2,
		GLfloat tolerance=FLOAT_TOLERANCE) {
	return (abs(f1 - f2) <= tolerance);
}
/**
 * Equal within tolerance.  == uses DR::FLOAT_TOLERANCE
 * Use this to specify a tolerance.
 */
bool Vec::equal_within(const Vec& other, GLfloat tolerance) {
	return ( _equal(x, other.x, tolerance) &&
			_equal(y, other.y, tolerance) &&
			_equal(z, other.z, tolerance) );
}

ostream& std::operator<<(ostream& out, const Vec& pt) {
	out.flags(ios::fixed);
	out.precision(3);
	out<< "(" << pt.x << ", " << pt.y << ", " << pt.z << ")";
	return out;
}

string Vec::to_string() const {
	stringstream out("");
	out<< "(" << x << ", " << y << ", " << z << ")";
	return out.str();
}

GLfloat Vec::magnitude() const {
	return (GLfloat) sqrt( pow(x, 2) + pow(y, 2) + pow(z, 2));
}

Vec Vec::unit_vec() {
	Vec out;
	GLfloat mag = magnitude();
	if(mag == 0) {
		out = Vec();
	} else {
		out = Vec(x/mag, y/mag, z/mag);
	}
	return out;
}

void Vec::normalize() {
	GLfloat mag = magnitude();
	if(mag == 0) {
		x = y = z = 0;
	} else {
		x /= mag;
		y /= mag;
		z /= mag;
	}
}
// subscript
// calls exit if bad index
GLfloat Vec::operator[](GLint index) {
	switch(index) {
	case 0: return x;
	case 1: return y;
	case 2: return z;
	default: // bad index, don't want to go unnoticed
		cerr << "Vec: subscript out of range: " << index << endl
			<< "vec = " << to_string() << endl;
		exit(1);
	}
}
/**
 * Cross product of this vector and v.
 */
Vec Vec::cross(const Vec& v) const {
	return Vec(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

const Vec& Vec::operator =(const Vec& other_vec) {
	if(&other_vec != this) {
		x = other_vec.x;
		y = other_vec.y;
		z = other_vec.z;
	}
	return *this;
}
const Vec& Vec::operator=(const GLfloat other[3]) {
	x = other[0];
	y = other[1];
	z = other[2];
	return *this;
}
const Vec& Vec::operator +=(const Vec& other_vec) {
	x += other_vec.x;
	y += other_vec.y;
	z += other_vec.z;

	return *this;
}
bool Vec::operator==(const Vec& other) const {
	return ( _equal(x, other.x) && _equal(y, other.y) && _equal(z, other.z) );
}
bool Vec::operator!=(const Vec& other) const {
	return !(*this == other);
}

Vec DR::operator+(Vec vec1, Vec vec2) {
	Vec v = vec1;
	return v += vec2;
}

const Vec& Vec::operator -=(const Vec& other_vec) {
	x -= other_vec.x;
	y -= other_vec.y;
	z -= other_vec.z;
	return *this;
}

Vec DR::operator-(Vec vec1, Vec vec2) {
	Vec v = vec1;
	return v -= vec2;
}

Vec DR::operator*(GLfloat scalar, Vec vec) {
	Vec v = vec;
	v.x *= scalar;
	v.y *= scalar;
	v.z *= scalar;
	return v;
}
Vec DR::operator*(Vec vec, GLfloat scalar) {
	Vec v = vec;
	v.x *= scalar;
	v.y *= scalar;
	v.z *= scalar;
	return v;
}
// transforms Vec by multiplying p by submat[3x3]
Vec DR::operator*(GLfloat matrix[16], Vec p) {
	GLfloat ret[3] = {0, 0, 0};
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			ret[i] += matrix[i+4*j] * p[j];
		}
	}
//	Vec vp = p.vec();
//	Vec mvec[3];
//	for (int i = 0; i < 3; ++i) {
//		mvec[i] = Vec(matrix[i*4], matrix[i*4+1], matrix[i*4+2]);
//	}
//
//	for (int i = 0; i < 3; ++i) {
//		ret[i] = mvec[i].dot(vp);
//	}
	return Vec(ret);

}

GLfloat DR::dist(Vec pt1, Vec pt2) {
	Vec v = pt1 - pt2;
	return v.magnitude();
}

void DR::midpoint(const Vec& v1, const Vec& v2, Vec& midpt) {
	midpt = Vec( (v1.x + v2.x)/2, (v1.y + v2.y)/2, (v1.z + v2.z)/2 );
}

/**
 * Intersection of 2 lines defined by a point and a direction vec.
 * return: true if lines intersect, false otherwise
 * params: pt1, dir1 - point and direction vector for line 1
 * pt2, dir2 - for line 2
 * intersect: point of intersection if return value is true
 */
bool DR::line_intersect(const Vec& pt1, const Vec& dir1,
		const Vec& pt2, const Vec& dir2, Vec& intersect) {
	// solving equation: a ( v1 X v2 ) = (pt2 - pt1) X v2  ; where v1, v2 = dir1, dir2
	// a = magnitude( (pt2 - pt1) X v2 ) / magnitude( v1 X v2 )
	GLfloat a;
	Vec diff, num, denom;
	diff = pt1 - pt2;
	num = diff.cross(dir2);
	denom = dir1.cross(dir2);
//	cout << "num = " << num << ", denom = " << denom << endl;
//	cout << denom.magnitude() << endl;
	// if left and right side vecs of equation are parallel, lines intersect
	Vec udenom = denom.unit_vec(), unum = num.unit_vec();
	if( _equal(denom.magnitude(), 0.0) ) {
		cout << "denom magnitude equals 0: " << denom.magnitude() << endl;
		cout << "line_intersect: numerator vec: " << unum << ", denominator vec: "
				<< udenom << endl;
		cout << "(-1 * num) = " << (-1 * unum) << endl;
		return false;
	}
//	cout << "u1 = " << u1 << ", u2 = " << u2 << endl;
	GLfloat tolerance = 0.0001;
	if( udenom.equal_within(unum, tolerance) ||
			udenom.equal_within( (-1 * unum), tolerance ) ) {
		a = num.magnitude() / denom.magnitude();
	} else {
		cout << "line_intersect: numerator vec: " << unum << ", denominator vec: "
				<< udenom << endl;
		cout << "(-1 * num) = " << (-1 * unum) << endl;
		return false;
	}
	// find point of intersection using scalar a
	intersect = pt1 + a * dir1;
	return true;
}

void DR::draw_line(const Vec& pt1, const Vec& pt2, const GLfloat *color, GLfloat linesz, bool disable_lighting) {
	glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_ENABLE_BIT);
	if(disable_lighting) {
		glDisable(GL_LIGHTING);
	}
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(linesz);
	glColor3fv(color);
	glBegin(GL_LINES);
	GLfloat v[3];
	pt1.array_out(v);
	glVertex3fv(v);
	pt2.array_out(v);
	glVertex3fv(v);
	glEnd();
	glPopAttrib();
}

/**
 * Draws a line with a point for the tip of the vector, ie the direction it is pointing.
 * Later, fix this if possible to use a cone or arrows, but non-trivial, and
 * possibly dependent on how the world coordinates are set up--ie trackball, etc.
 */
void DR::draw_vector(Vec tail, Vec tip, const GLfloat *color, const GLfloat *tipcolor,
		GLfloat linesz, bool disable_lighting) {

	draw_line(tail, tip, color, linesz, disable_lighting);
	draw_point(tip, tipcolor, 4*linesz);

//	GLint viewport[4];
//	GLdouble mvmatrix[16], projmatrix[16];
//	GLdouble wx, wy, wz, wx1, wy1, wz1; /* returned world x, y, z coords */
//
//	GLdouble x0, y0, x1, y1;
//
//	glGetIntegerv(GL_VIEWPORT, viewport);
//	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
//	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
//	// get center points in screen space, 1 pixel apart in x direction
//	x0 = (viewport[0] + viewport[2])/2.0;
//	y0 = (viewport[1] + viewport[3])/2.0;
//	x1 = x0 + 1; y1 = y0;
//
//	gluUnProject(x0, y0, 0.5, mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
//	gluUnProject(x1, y1, 0.5, mvmatrix, projmatrix, viewport, &wx1, &wy1, &wz1);
//
//	GLdouble w1[3] = {wx, wy, wz}, w2[3] = {wx1, wy1, wz1};
//	// roughly the size of one pixel in world coordinates
//	GLfloat world_pixel = (GLfloat)dist(w1, w2);
////	cout << "world_pixel = " << world_pixel << endl;
//
//
//	GLfloat w_linesz = world_pixel * linesz;
//	GLfloat arrow_width = 100 * w_linesz, arrow_len = 2 * arrow_width;
//	Vec line_dir = tip - tail;
//	// this footwork is just to ensure that a fat line won't protrude outside
//	// the surface of the arrow -- I'm using arrow here, meaning only the arrow head
//	GLfloat len_actual = line_dir.magnitude();
//	line_dir.normalize();
//	Vec line_end = tail + line_dir * (len_actual - .5 * arrow_len);
//	draw_line(tail, line_end, color, linesz, disable_lighting);
//	Vec arrow_start = tail + line_dir * (len_actual - arrow_len);
//	Vec perp_vec = Vec( (-line_dir.y-line_dir.z)/line_dir.x, 1, 1 ).unit_vec();
//	Vec perp_vec2 = line_dir.cross(perp_vec).unit_vec();
//	// draw the arrow head as 4 lines, so should appear roughly consistent
//	// regardless of orientation
//	Vec pt1 = arrow_start + perp_vec * .5*arrow_width, pt2  = arrow_start - perp_vec * .5*arrow_width;
//	Vec pt3 = arrow_start + perp_vec2 * .5*arrow_width, pt4 = arrow_start - perp_vec2 * .5*arrow_width;
//
//	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
//	if(disable_lighting) {
//		glDisable(GL_LIGHTING);
//	}
//	glColor3fv(tipcolor);
//	GLfloat v[3];
//	glBegin(GL_TRIANGLES);
//	pt1.array_out(v);
//	glVertex3fv(v);
//	tip.array_out(v);
//	glVertex3fv(v);
//	pt2.array_out(v);
//	glVertex3fv(v);
//
//	pt3.array_out(v);
//	glVertex3fv(v);
//	tip.array_out(v);
//	glVertex3fv(v);
//	pt4.array_out(v);
//	glVertex3fv(v);
//
//	glEnd();
//	glPopAttrib();
////	draw_point(pt1, tipcolor, linesz);
////	draw_point(pt2, tipcolor, linesz);
////	draw_point(pt3, tipcolor, linesz);
////	draw_point(pt4, tipcolor, linesz);
//
////	draw_line(pt1, tip, tipcolor, linesz);
////	draw_line(pt2, tip, tipcolor, linesz);
////	draw_line(pt3, tip, tipcolor, linesz);
////	draw_line(pt4, tip, tipcolor, linesz);
//
//
////	GLfloat cone_ht_to_linesz = 3;
////	GLfloat cone_ht = cone_ht_to_linesz * linesz;
////	GLfloat cone_r = linesz;
////	Vec line_dir = tip - tail;
////	// this footwork is just to ensure that a fat line won't protrude outside
////	// the surface of the cone
////	GLfloat len_actual = line_dir.magnitude();
////	line_dir.normalize();
////	Vec line_end = tail + line_dir * (len_actual - .5 * cone_ht);
////	draw_line(tail, line_end, color, linesz, disable_lighting);
////	Vec c = tail + line_dir * (len_actual - cone_ht);
////	// cone in world is too much, how about arrow head with linds?
//
////	glPushMatrix();
////	glTranslatef(c.x, c.y, c.z);
////	GLfloat to_degrees = 180 / PI;
////	glRotatef(acos(line_dir.x) * to_degrees, 1, 0, 0);
////	glRotatef(acos(line_dir.y) * to_degrees, 0, 1, 0);
////	glRotatef(acos(line_dir.z) * to_degrees, 0, 0, 1);
////	glutSolidCone(cone_r, cone_ht, 12, 12);
////	glPopMatrix();
}

void DR::draw_point(Vec pos, const GLfloat color[3], GLfloat size) {
	glPushAttrib(GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(size);
	glColor3fv(color);
	vec3 p;
	pos.array_out(p);
	glBegin(GL_POINTS);
	glVertex3fv(p);
	glEnd();
	glPopAttrib();
}

/**
 * Checks for intersection of a ray and a plane.
 * The ray is defined by a Vec, ray0, and a direction vector, ray_dir
 * The plane is defined by a Vec, plane0, and its normal, plane_norm
 * If the return value is true, the intersection Vec will be in out
 * Note:******
 * Modified this so it only returns true if the ray intersects the
 * plane given its direction, before it would return true if the negative
 * direction intersected, so the only time it was false was if the ray
 * was parallel
 */
bool DR::ray_intersect(Vec& out, Vec ray0, Vec ray_dir,
		Vec plane_norm, Vec plane0) {
//	GLfloat denom = plane_norm.dot(ray_dir);
//	// check for parallel
//	if(denom == 0) {
//		return false;
//	}
//	// d as in ax + by + cz + d = 0
//	GLfloat d = -plane0.dot(plane_norm);
//	// param to substitute in
//	GLfloat t = -(plane_norm.dot(ray0) + d) / denom;
//
//	out = ray0 + t * ray_dir;

	// above method is fine, but may be misleading as the intersection
	// may involve the negative direction of the ray
	plane_norm.normalize();
	GLfloat c = plane_norm.dot(ray_dir);
	// parallel
	if(c == 0) {
		return false;
	}
	// These variables come from 3D Computer Graphics, a Math. Intro w Opengl
	// let the plane be defined by  X dot N = d   (d, a scalar)
	// let the ray be defined by point p, and unit vec u, direction
	// then set:  alpha = (d - p dot n)/(u dot n)
	// c is the denominator, as denom above
	// let q be the intersection point if it exists
	// then alpha is the distance from p to q, if it's negative,
	// the ray points away, this is what was missing above
	// plane0 is X, ray_dir is u, ray0 is p
	GLfloat d = plane0.dot(plane_norm);

	GLfloat alpha = (d - ray0.dot(plane_norm)) / c;
	if(alpha < 0) {
		return false;
	}
	// q = p + alpha(u) is the intersection
	out = ray0 + alpha * ray_dir;

	return true;
}

