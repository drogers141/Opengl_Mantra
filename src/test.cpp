/*
 * test.cpp
 *
 *  Created on: Jul 25, 2010
 *      Author: drogers
 */

#include "test.h"
#include "syllable.h"
//#include "dr_util.h"

using namespace std;
using namespace DR;

vec3 vecs[] = { {0, 4, 0}, {3, 0, 0}, {0, 0, 5},
			{-4, 2, 1}, {-6, -2, 1}, {2, -2, 1}, {2, 2, 1} };
vec3 norms[] = { {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
		{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0} };

PolyTest::PolyTest() {
	t = NULL;
	q = NULL;
}
PolyTest::~PolyTest() {
	delete t;
}
void PolyTest::init() {
	vec3 *v = vecs;
	vec3 *n = norms;
	t = new Triangle(v, n);
	for (int i = 0; i < 3; ++i) {
		t->verts[i] = i;
		t->norms[i] = i;
	}
	cout << "Polytest: " << endl;

	// test set winding from vector that is not exact normal
	// vector should just need to be facing correct direction
	vec3 dir = { -1, -1, -1 };
	t->set_winding_from_normal(dir);

//	t->set_edges();
//	t->reverse_winding();
//	t->set_center();
	t->set_facetnorm();
	for (int i = 0; i < 3; ++i) {
		copyv(norms[t->norms[i]], t->facetnorm);
	}
	cout << "triangle: " << t->to_string(true) << endl;
	cout << "\tfnorm: " << stringv(t->facetnorm) << ", norms[0]: "
			<< stringv(norms[t->norms[0]]) << endl;
	cout << "\tcenter: " << stringv(t->center) << endl;

	t->set_winding_from_normal();
	cout << "after set_winding_from_normal() default:" << endl;
	cout << "triangle: " << t->to_string(true) << endl;
	cout << "\tfnorm: " << stringv(t->facetnorm) << ", norms[0]: "
			<< stringv(norms[t->norms[0]]) << endl;
	cout << "\tcenter: " << stringv(t->center) << endl;

	t->set_winding_from_normal(t->facetnorm);
	cout << "after set_winding_from_normal() with facetnorm:" << endl;
	cout << "triangle: " << t->to_string(true) << endl;
	cout << "\tfnorm: " << stringv(t->facetnorm) << ", norms[0]: "
			<< stringv(norms[t->norms[0]]) << endl;
	cout << "\tcenter: " << stringv(t->center) << endl;


	// test intersection of lines with verts 0, 1 and opposing midpts
	pt1 = Vec(vecs[t->verts[0]]);
	pt2 = Vec(vecs[t->verts[1]]);
	pt3 = Vec(vecs[t->verts[2]]);
	midpoint(pt2, pt3, mid1);
	midpoint(pt1, pt3, mid2);
	dir1 = mid1 - pt1;
	dir2 = mid2 - pt2;

	lines_intersect = line_intersect(pt1, dir1, pt2, dir2, intersect);
	if(!lines_intersect) {
		cout << "Triangle lines not intersecting" << endl;
	}

	q = new Quad(v, n);
	for (int i = 0; i < 4; ++i) {
		q->verts[i] = i + 3;
		q->norms[i] = i + 3;
	}
	q->set_edges();
	q->set_center();
	q->set_facetnorm();
	for (int i = 0; i < 4; ++i) {
		copyv(norms[q->norms[i]], q->facetnorm);
	}
	cout << "quad: " << q->to_string(true) << endl;
	cout << "\tfnorm: " << stringv(q->facetnorm) << ", norms[0]: "
				<< stringv(norms[q->norms[0]]) << endl;
	cout << "\tcenter: " << stringv(q->center) << endl;

	q->set_winding_from_normal();
	cout << "after set_winding_from_normal() default:" << endl;
	cout << "quad: " << q->to_string(true) << endl;
	cout << "\tfnorm: " << stringv(q->facetnorm) << ", norms[0]: "
			<< stringv(norms[q->norms[0]]) << endl;
	cout << "\tcenter: " << stringv(q->center) << endl;

	q->set_winding_from_normal(q->facetnorm);
	cout << "after set_winding_from_normal() with facetnorm:" << endl;
	cout << "quad: " << q->to_string(true) << endl;
	cout << "\tfnorm: " << stringv(q->facetnorm) << ", norms[0]: "
			<< stringv(norms[q->norms[0]]) << endl;
	cout << "\tcenter: " << stringv(q->center) << endl;

}

void PolyTest::render() {
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POINT_BIT);
	glPointSize(5);
//	glColor3f(1, 1, 1);
//	glBegin(GL_POINTS);
//	glVertex3f(0, 0, 0);
//	glEnd;
//	glPopAttrib();
	draw_axes(2, 8);
	t->render(Util::grey);
	t->draw_facetnorm(Util::magenta, 1);
	t->draw_normals(Util::cyan, 1);

	draw_line(pt1, mid1, Util::yellow, 3);
	draw_line(pt2, mid2, Util::yellow, 3);

	vec3 vert;
	glBegin(GL_POINTS);
	glColor3fv(Util::green);
	mid1.array_out(vert);
	glVertex3fv(vert);

	glColor3fv(Util::blue);
	mid2.array_out(vert);
	glVertex3fv(vert);

	glColor3fv(Util::red);
	intersect.array_out(vert);
	glVertex3fv(vert);
	glEnd();

	q->render(Util::purple);
	q->draw_facetnorm(Util::yellow, 1);
	q->draw_normals(Util::green, 1);
	glBegin(GL_POINTS);
	glColor3fv(Util::red);
	glVertex3fv(q->center);
	glEnd();

	glPopAttrib();


}
