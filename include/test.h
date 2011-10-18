/*
 * test.h
 *
 *  Created on: Jul 25, 2010
 *      Author: drogers
 */

#ifndef TEST_H_
#define TEST_H_

#include "poly.h"
#include "vec.h"
#include "particles.h"
#include <vector>

namespace DR {

class GLTest {
public:
	virtual void init() {}
	virtual void render() {}
};

class PolyTest : public GLTest {
public:
	PolyTest();
	virtual ~PolyTest();
	void init();
	void render();

	Triangle *t;
	Quad *q;

	// for testing line intersection, finding centroid, etc
	// pt* are vertices
	Vec pt1, pt2, pt3;
	Vec dir1, dir2, intersect;
	// midpoints
	Vec mid1, mid2;
	bool lines_intersect;
};

}; // namespace DR

#endif /* TEST_H_ */
