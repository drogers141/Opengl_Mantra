/*
 * poly.h
 *
 * Opengl  - utilities relating to polygons that use indexed vertices, employs namespace DR
 *
 *
 *  Created on: Jun 15, 2010
 *      Author: drogers
 */
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <cerrno>

#include "poly.h"
#include "vec.h"
#include "dr_util.h"

using namespace std;
using namespace DR;

// default ostream just outputs vertices in obj file format
ostream& operator<<(ostream& out, const Polygon& p) {
	out << "f ";
	for (int i = 0; i < p.size; ++i) {
		out << p.verts[i] << " ";
	}
	return out;
}

bool DR::contains(const vector<Polygon*>& v, const Polygon& p) {
	for (size_t i = 0; i < v.size(); ++i) {
		if(p == *v[i]) {
			return true;
		}
	}
	return false;
}
// removes duplicates from v
void DR::uniq(vector<Polygon*>& v) {
	vector<Polygon*> temp;
//	copy(v.begin(), v.end(), back_inserter(temp));
	for (size_t i = 0; i < v.size(); ++i) {
		if(!contains(temp, *v[i])) {
			temp.push_back(v[i]);
		}
	}
	v.clear();
	copy(temp.begin(), temp.end(), back_inserter(v));
}

// puts the intersection of v1 and v2 in intersect
// clears intersect before and removes duplicates from intersect after
void DR::intersection(const vector<Polygon*>& v1, const vector<Polygon*>& v2,
		vector<Polygon*>& intersect) {
	intersect.clear();

	for (size_t i = 0; i < v1.size(); ++i) {
		for (size_t j = 0; j < v2.size(); ++j) {
			if(*v1[i] == *v2[j]) {
				intersect.push_back(v1[i]);
			}
		}
	}
	uniq(intersect);
//	cout << "intersection:  intersect:" << endl;
//	print_polys(intersect, 200, true);
}

// puts elements of the set difference of v1 and v2 in out
// bogus way to do it, but set_difference doesn't like the pointers
// clears out before
void DR::difference(const vector<Polygon*>& v1, const vector<Polygon*>& v2,
		vector<Polygon*>& out) {
	out.clear();
	vector<Polygon*> temp, intersect;
	intersection(v1, v2, intersect);


//	cout << "difference: vector sizes: v1=" << v1.size() << ", v2=" << v2.size() << endl;
	for (size_t i = 0; i < v1.size(); ++i) {
		if(!contains(intersect, *v1[i])) {
			out.push_back(v1[i]);
		}
	}
	for (size_t i = 0; i < v2.size(); ++i) {
		if(!contains(intersect, *v2[i])) {
			out.push_back(v2[i]);
		}
	}
//	cout << "difference: vector sizes: v1=" << v1.size() << ", v2=" << v2.size() << ", intersect=" << intersect.size() << endl;
//	cout << " out=" << out.size() << endl;
}

string IndexedEdge::to_string() {
	stringstream out("");
	out << "(" << u << ", " << v << ")";
	return out.str();
}

Polygon::Polygon()
: size(0), verts(NULL), norms(NULL), edges(NULL),
  actual_verts(NULL), actual_norms(NULL), max_side_length(0) {
	setv(facetnorm, 0, 0, 0);
	setv(center, 0, 0, 0);
}

Polygon::Polygon(GLint sz, vec3 *vertices, vec3 *normals)
: size(sz), max_side_length(0) {
	verts = new GLint[size];
	norms = new GLint[size];
	edges = new IndexedEdge[size];
	actual_verts = vertices;
	actual_norms = normals;
	setv(facetnorm, 0, 0, 0);
	setv(center, 0, 0, 0);
}

Polygon::Polygon(const Polygon& p) {
	size = p.size;
	verts = new GLint[size];
	norms = new GLint[size];
	edges = new IndexedEdge[size];
	for (int i = 0; i < size; ++i) {
		verts[i] = p.verts[i];
		norms[i] = p.norms[i];
		edges[i] = p.edges[i];
	}
	copyv(facetnorm, p.facetnorm);
	copyv(center, p.center);
	actual_verts = p.actual_verts;
	actual_norms = p.actual_norms;
	max_side_length = p.max_side_length;
}
// assignment makes a deep copy of everything
// but actual_verts and actual_norms point to same storage
const Polygon& Polygon::operator=(const Polygon& other) {
	if(&other != this) {
		size = other.size;
		verts = new GLint[size];
		norms = new GLint[size];
		edges = new IndexedEdge[size];
		for (int i = 0; i < size; ++i) {
			verts[i] = other.verts[i];
			norms[i] = other.norms[i];
			edges[i] = other.edges[i];
		}
		copyv(facetnorm, other.facetnorm);
		copyv(center, other.center);
		actual_verts = other.actual_verts;
		actual_norms = other.actual_norms;
	}
	return *this;
}

Polygon::~Polygon() {
	delete [] verts;
	delete [] norms;
	delete [] edges;
}

void Polygon::set_edges() {
	for (int i = 0; i < size; ++i) {
		edges[i].u = verts[i];
		edges[i].v = verts[(i+1)%size];
	}
	set_max_side_length();
	init_precomputed();
}

// sets the facet norm, by default uses order of first 3 verts
// applying cross product: (v0, v1) X (v1, v2)
// supply verts[3] and above will be used with param verts[]
void Polygon::set_facetnorm(GLint *vertices) {
	GLfloat *v0, *v1, *v2;
	if(vertices == NULL) {
		v0 = actual_verts[verts[0]];
		v1 = actual_verts[verts[1]];
		v2 = actual_verts[verts[2]];
	} else {
		v0 = actual_verts[vertices[0]];
		v1 = actual_verts[vertices[1]];
		v2 = actual_verts[vertices[2]];
	}
	Vec vec1 =  Vec(v1) - Vec(v0);
	Vec vec2 =  Vec(v2) - Vec(v1);
	Vec n = vec1.cross(vec2);
	n.normalize();
	n.array_out(facetnorm);
}

/**
 * Ensures that polygon has right handed winding based on the parameter
 * Resets edges and center
 * if no normal is passed in, uses first vertex normal (actual_norms[norms[0]])
 * param: normal - vector within 90 degrees of facetnormal that will result from
 * the new winding--ie it doesn't have to be a normal (or normalized), just
 * pointing the right way
 */
void Polygon::set_winding_from_normal(GLfloat *normal) {
	Vec n;
	if(normal==NULL) {
		normalize(actual_norms[norms[0]]); // just to be sure
		n = Vec(actual_norms[norms[0]]);
	} else {
		n = Vec(normal).unit_vec();
	}
	vector<Vec> vts;
	for (int i = 0; i < size; ++i) {
		vts.push_back( Vec(actual_verts[verts[i]]) );
	}
	set_edges();
	set_center();
	// sanity check
	if(!(edges[0].contains(verts[0]) && edges[size-1].contains(verts[0]))) {
		cout << "Polygon::set_winding_from_vert_normal(): vert 0 not contained by first and last edge - exiting" << endl;
		exit(1);
	}
	Vec last_side = vts[1] - vts[0];
	Vec first_side = vts[size-1] - vts[0];
	Vec last_cross_first = last_side.cross(first_side);
	last_cross_first.normalize();
	errno = 0;
	GLfloat cos  = last_cross_first.dot(n);
//	cout << "cos = " << cos << endl;
	if( !( fabs(cos) - DR::FLOAT_TOLERANCE <= 1) ) {
		stringstream s;
		s << "fabs(cos) - DR::FLOAT_TOLERANCE <= 1 failed: cos = " << cos << endl;
		s << "Poly: " << to_string() << endl;
		err_exit(s.str());
	}
	GLfloat angle = (GLfloat)acos(cos);
//	cout << "angle = " << angle << ", errno = " << errno << endl;
	// check for domain error at boundary values
	if(errno == EDOM) {
		errno = 0;
//		cout << "domain error, adjusting dot product" << endl;
		if(cos > 0) {
			cos -= .01;
		} else {
			cos += .01;
		}
		angle = (GLfloat)acos(cos);
//		cout << "angle = " << angle << ", errno = " << errno << endl;
		assert(errno != EDOM);
	}

	if(last_cross_first == n || angle < PI/2) {  // the winding is right
//		cout << "set_winding_from_normal: winding is ok, angle = "
//				<< angle << endl;
		return;
	}
	// reverse winding
//	cout << "set_winding_from_normal: winding is wrong - reversing, angle = "
//				<< angle << endl;
	reverse_winding();

}

void Polygon::reverse_winding() {
	GLint tempv[4];
	for (int i = 0; i < size; ++i) {
		tempv[i] = verts[i];
	}
	for (int i = 1; i < size; ++i) {
		verts[i] = tempv[size-i];
	}
	set_edges();
}

bool Polygon::contains(GLint vert) {
	for (int i = 0; i < size; ++i) {
		if ( vert == verts[i] ) {
			return true;
		}
	}
	return false;
}

bool Polygon::contains(const IndexedEdge& edge) {
	for (int i = 0; i < size; ++i) {
		if ( edge == edges[i] ) {
			return true;
		}
	}
	return false;
}

/**
 * Note:  depends on DR::FLOAT_TOLERANCE
 */
bool Polygon::contains(const vec3 actual_vert) {
	for (int i = 0; i < size; ++i) {
		if ( equal(actual_vert ,actual_verts[verts[i]]) ) {
			return true;
		}
	}
	return false;
}
/**
 * RELIES ON FACETNORM BEING SET
 * Returns true if point is above the plane of this polygon,
 * with the facetnorm being considered up.
 */
bool Polygon::is_above_poly(const Vec& point) const {
	Vec vcenter = Vec(center);
	if(Vec(point) == vcenter) {
		return false;
	}
	Vec to_point = point - vcenter;
	to_point.normalize();
	GLfloat dot_ = to_point.dot( Vec(facetnorm) );
	return (dot_ > 0);
}

/**
 * Returns true if a line segment joining points pt1 and pt2
 * penetrates this polygon by at least delta (ie 1 point should not
 * be in the polygon's plane.
 * If true, intersection point will be in intersection
 */
bool Polygon::line_penetrates(const Vec& pt1, const Vec& pt2,
		GLfloat delta, Vec& intersection) const {
	Vec pt1_to_pt2 = pt2 - pt1;
	if(!ray_intersect(intersection, pt1, pt1_to_pt2) ) {
		return false;
	}
	if(dist(intersection, pt1) > delta && dist(intersection, pt2) > delta) {
//		cout << "intersection: " << intersection << ",  delta: " << delta << endl
//				<< "pt1: " << pt1 << ",  pt2: " << pt2 << endl;
//		cout << "pt1 dist: " << dist(intersection, pt1) << ", "
//				<< "pt2 dist: " << dist(intersection, pt2) << endl;
		return true;
	}
	return false;
}

// return string representation, if show_edges is true, adds edges
string Polygon::to_string(bool show_edges) const {
	stringstream out("");
	for (int i = 0; i < size; ++i) {
		out << stringv(actual_verts[verts[i]]) << " ";
	}
	out << endl << "    fnorm: " << stringv(facetnorm) << " "
			<< "  center: " << stringv(center) << "  vert indices: ";
	for (int i = 0; i < size; ++i) {
			out << verts[i] << " ";
		}
	if(show_edges) {
		out << "  ";
		for (int i = 0; i < size; ++i) {
			out << "  " << edges[i].to_string();
		}
	}
	return out.str();
}
// return string representation, if show_edges is true, adds edges
// prefixes output with "Triangle: "
string Triangle::to_string(bool show_edges) const {
	return "Triangle:  " + Polygon::to_string(show_edges);
}
// return string representation, if show_edges is true, adds edges
// prefixes output with "Quad: "
string Quad::to_string(bool show_edges) const {
	return "Quad:  " + Polygon::to_string(show_edges);
}
/**
 * Get a copy of the polys vertices as Vecs to play with.
 */
void Polygon::get_actual_verts(vector<Vec>& out) const {
	if(!out.empty()) {
		out.clear();
	}
	for (int i = 0; i < size; ++i) {
		out.push_back( actual_verts[verts[i]] );
	}
}

// at this point == compares verts and norms by index
// not by actual vertex or normal contents
bool Polygon::operator==(const Polygon& other) const {

	for (int i = 0; i < size; ++i) {
		if(verts[i] != other.verts[i]) return false;
		if(norms[i] != other.norms[i]) return false;
	}
	if(!equal(facetnorm, other.facetnorm)) return false;
	return true;
}

bool PolygonCompare::operator()(const Polygon *lhs, const Polygon *rhs) const {
	if(lhs->size != rhs->size) {
		return (lhs->size < rhs->size);
	}
	for (int i = 0; i < lhs->size; ++i) {
		if(lhs->verts[i] < rhs->verts[i]) {
			return true;
		}
	}
	Vec3Compare vec3less;
	if( vec3less(lhs->facetnorm, rhs->facetnorm) ) {
		return true;
	}
}

/**
 * Triangle finds centroid by getting line intersection
 * of lines made from first 2 verts and their opposing midpoints
 */
void Triangle::set_center() {
	Vec mid1, mid2, dir0, dir1, vts[3], c;
	for (int i = 0; i < 3; ++i) {
		vts[i] = Vec(actual_verts[verts[i]]);
	}
	midpoint(vts[1], vts[2], mid1);
	midpoint(vts[0], vts[2], mid2);
	dir0 = mid1 - vts[0];
	dir1 = mid2 - vts[1];

	bool intersect_found;
	intersect_found = line_intersect(vts[0], dir0, vts[1], dir1, c);
	c.array_out(center);
	if(!intersect_found) {
		cout << "Triangle::set_center(): can't find center, c = " << c << endl;
		cout << this->to_string() << endl;
	}
}
/**
 * Length of longest side. Good for various geometric approximations.
 * Reset whenever resizing.
 * Called automatically by set_edges, which is called by the winding functions.
 */
void Polygon::set_max_side_length() {
	GLfloat m = 0;
	for (int i = 0; i < size; ++i) {
		GLfloat len = dist( actual_verts[verts[i]], actual_verts[verts[(i+1)%size]]);
		m = max(m, len);
	}
	max_side_length = m;
}

/**
 * Init precomputed stuff for ray intersection.
 * Called by set_edges, so shouldn't have to worry.
 * See 3D Comp Graphics a Math. Intro for this algorithm
 */
void Triangle::init_precomputed() {
	GLfloat a, b, c, A, B, C, D;
	Vec v0(actual_verts[verts[0]]);
	Vec e1 = Vec(actual_verts[verts[1]]) - v0;
	Vec e2 = Vec(actual_verts[verts[2]]) - v0;
	precomputed.d = Vec(facetnorm).dot(v0);
	a = e1.dot(e1);
	b = e1.dot(e2);
	c = e2.dot(e2);
	D = a * c - b*b;
	A = a / D;
	B = b / D;
	C = c / D;
	precomputed.u_beta = C * e1 - B * e2;
	precomputed.u_gamma = A * e2 - B * e1;
}

/**
 * Returns true if ray intersects this poly.
 * If found, intersecting point will be put in out.
 * The ray is defined by a point Vec, ray0, and a direction vector, ray_dir
 */
bool Triangle::ray_intersect(Vec& out, Vec ray0, Vec ray_dir) const {
	Vec n(facetnorm), v0(actual_verts[verts[0]]);
	bool inplane = DR::ray_intersect(out, ray0, ray_dir, n, v0);
	if(!inplane) {
		return false;
	}
	// easy optimization
	if(dist(center, out) > max_side_length) {
		return false;
	}
	Vec r = out - v0;
	GLfloat beta = precomputed.u_beta.dot(r);
	if(beta < 0) {
		return false;
	}
	GLfloat gamma = precomputed.u_gamma.dot(r);
	if(gamma < 0) {
		return false;
	}
	GLfloat alpha = 1 - beta - gamma;
	if(alpha < 0) {
		return false;
	}
	return true;

}
/**
 * Returns true if ray intersects this poly.
 * If found, intersecting point will be put in out.
 * The ray is defined by a point Vec, ray0, and a direction vector, ray_dir
 */
bool Quad::ray_intersect(Vec& out, Vec ray0, Vec ray_dir) const {
	Vec n(facetnorm), v0(actual_verts[verts[0]]);
	bool in_first_triangle = true;
	bool inplane;

	while(in_first_triangle) {
		// first triangle: 0, 1, 2
		inplane = DR::ray_intersect(out, ray0, ray_dir, n, v0);
		if(!inplane) {
			in_first_triangle = false;
			break;
		}
		// easy optimization
		if(dist(center, out) > max_side_length) {
			in_first_triangle = false;
			break;
		}
		Vec r = out - v0;
		GLfloat beta = precomputed.u_beta.dot(r);
		if(beta < 0) {
			in_first_triangle = false;
			break;
		}
		GLfloat gamma = precomputed.u_gamma.dot(r);
		if(gamma < 0) {
			in_first_triangle = false;
			break;
		}
		GLfloat alpha = 1 - beta - gamma;
		if(alpha < 0) {
			in_first_triangle = false;
			break;
		}
		return true;
	}
	assert(!in_first_triangle);

	// second triangle
	inplane = DR::ray_intersect(out, ray0, ray_dir, n, v0);
	if(!inplane) {
		return false;
	}
	// easy optimization
	if(dist(center, out) > max_side_length) {
		return false;
	}
	Vec r = out - v0;
	GLfloat beta = precomputed2.u_beta.dot(r);
	if(beta < 0) {
		return false;
	}
	GLfloat gamma = precomputed2.u_gamma.dot(r);
	if(gamma < 0) {
		return false;
	}
	GLfloat alpha = 1 - beta - gamma;
	if(alpha < 0) {
		return false;
	}
	return true;

}
/**
 * Init precomputed stuff for ray intersection.
 * For the quad, I'm just going to treat it as 2 triangles.
 * Quad bisected by line between verts 0, 2
 * So to keep right-handed, triangles are:
 * 0, 1, 2;  0, 2, 3
 */
void Quad::init_precomputed() {
	GLfloat a, b, c, A, B, C, D;
	// first triangle
	// verts: 0, 1, 2
	Vec v0(actual_verts[verts[0]]);
	Vec e1 = Vec(actual_verts[verts[1]]) - v0;
	Vec e2 = Vec(actual_verts[verts[2]]) - v0;
	precomputed.d = Vec(facetnorm).dot(v0);
	a = e1.dot(e1);
	b = e1.dot(e2);
	c = e2.dot(e2);
	D = a * c - b*b;
	A = a / D;
	B = b / D;
	C = c / D;
	precomputed.u_beta = C * e1 - B * e2;
	precomputed.u_gamma = A * e2 - B * e1;
	// second triangle
	// verts: 0, 2, 3
	e1 = Vec(actual_verts[verts[2]]) - v0;
	e2 = Vec(actual_verts[verts[3]]) - v0;
	precomputed2.d = Vec(facetnorm).dot(v0);
	a = e1.dot(e1);
	b = e1.dot(e2);
	c = e2.dot(e2);
	D = a * c - b*b;
	A = a / D;
	B = b / D;
	C = c / D;
	precomputed2.u_beta = C * e1 - B * e2;
	precomputed2.u_gamma = A * e2 - B * e1;
}

/**
 * Quad finds centroid by getting line intersection
 * of lines made from opposing midpoints
 */
void Quad::set_center() {
	Vec mids[4], dir0, dir1, vts[4], c;
	for (int i = 0; i < 4; ++i) {
		vts[i] = Vec(actual_verts[verts[i]]);
	}
	for (int i = 0; i < 4; ++i) {
		int j = (i+1) % 4;
		midpoint(vts[i], vts[j], mids[i]);
	}
	dir0 = mids[2] - mids[0];
	dir1 = mids[3] - mids[1];
	bool intersect_found;
	intersect_found = line_intersect(mids[0], dir0, mids[1], dir1, c);
	c.array_out(center);
	if(!intersect_found) {
		cout << "Quad::set_center(): can't find center" << endl
				<< "line 1: pt=" << mids[0] << ", dir=" << dir0 << ";  "
				<< "line 2: pt=" << mids[1] << ", dir=" << dir1 << endl;
	}
}

/**
 * render facet normal using a colored line drawn from the center of the poly
 * assumes center has been set
 * param: scale - scale factor multiplied by length of a side to create line length
 */
void Polygon::draw_facetnorm(const GLfloat *color, GLfloat scale) const {
		Vec vts[2];
		for (int i = 0; i < 2; ++i) {
			vts[i] = Vec(actual_verts[verts[i]]);
		}
		GLfloat side_len = dist(vts[0], vts[1]);
		Vec end = center + Vec(facetnorm) * side_len * scale;
		draw_line(center, end, color, 2);
}

// same as above but for each norm, at the vertex
void Polygon::draw_normals(GLfloat *color, GLfloat scale) {
	vector<Vec> vts;
	for (int i = 0; i < size; ++i) {
		vts.push_back( Vec(actual_verts[verts[i]]) );
	}
	GLfloat side_len = dist(vts[0], vts[1]);
	for (int i = 0; i < size; ++i) {
		Vec norm = Vec(actual_norms[norms[i]]);
		Vec end = vts[i] + norm * side_len * scale;
		draw_line(vts[i], end, color, 2);
	}
}

/**
 * Render polygon.
 * params: color: [NULL] if not NULL, use color, else doesn't set color
 * use_fnorm: [true], if true use facet norm, else use vert norms
 */
void Polygon::render(GLfloat *color, bool use_fnorm) {
	glPushAttrib(GL_CURRENT_BIT);
	if(color) {
		glColor3fv(color);
	}
	if(use_fnorm) {
		glNormal3fv(facetnorm);
	}
	if(size == 3) {
		glBegin(GL_TRIANGLES);
		//			glBegin(GL_LINES);
		//			glColor3f( .5, .5, 1 );
	} else if(size == 4) {
		glBegin(GL_QUADS);
	} else { // unknown poly type
		cout << "Polygon::render(): can't render, size = " << size << endl;
		return;
	}
	for (int i = 0; i < size; ++i) {
		if(!use_fnorm) {
			glNormal3fv( actual_norms[ norms[i] ] );
		}
		glVertex3fv( actual_verts[ verts[i] ] );
	}
	glEnd();
	glPopAttrib();
}

/**
 * Disables gl_lighting
 * wire: draw wireframe
 * line_sz: if wire, line size [1]
 */
void Polygon::render_no_lighting(const GLfloat *color, bool wire, GLfloat line_sz) const {
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_LIGHTING);
	glColor3fv(color);
	if(wire) {
		glLineWidth(line_sz);
		glBegin(GL_LINE_LOOP);
	} else if(size == 3) {
		glBegin(GL_TRIANGLES);
	} else if(size == 4) {
		glBegin(GL_QUADS);
	} else { // unknown poly type
		cout << "Polygon::render(): can't render, size = " << size << endl;
		return;
	}
	for (int i = 0; i < size; ++i) {
		glVertex3fv( actual_verts[ verts[i] ] );
	}
	glEnd();
	glPopAttrib();
}

// print out vector of polys with indices of vector
// set a limit for printing if desired, -1 -> no limit
// print_p: print the pointer as well
void DR::print_polys(const std::vector<Polygon*>& polys, int limit, bool print_p) {
	int num_polys = polys.size();
	if(limit != -1) {
		num_polys = min(num_polys, limit);
	}
	for (int i = 0; i < num_polys; ++i) {
		if(print_p) {
			cout << polys[i] << i << "  " << polys[i]->to_string() << endl;
		} else {
			cout << i << "  " << polys[i]->to_string() << endl;
		}
	}
}


