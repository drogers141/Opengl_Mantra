/*
 * cylinder_model.cpp
 *
 *  Created on: Jun 25, 2010
 *      Author: drogers
 */

#include "cylinder_model.h"
#include "dr_util.h"
//#include "vec.h"
#include <cmath>
#include <cassert>

using namespace std;
using namespace DR;

// zero component: 0, 1, 2  for x, y, z whichever should be ignored
// this method should be overridden for different behavior
void Mappable2D::map(int zero_component) {
	int xcomp = zero_component == 0 ? 1 : 0;
	int ycomp = zero_component == 2 ? 1 : 2;
	// this version maps range of vertices from min to max in both dimensions to [0,1]
	GLfloat minx, miny, maxx, maxy, xrange, yrange, x_scale_factor, y_scale_factor;
	minx = miny = 1000.0;
	maxx = maxy = -1000.0;
	for (size_t i = 0; i < verts3d.size(); ++i) {
		Vec vert = verts3d[i];
		minx = min(minx, vert[xcomp]);
		miny = min(miny, vert[ycomp]);
		maxx = max(maxx, vert[xcomp]);
		maxy = max(maxy, vert[ycomp]);
	}
	xrange = maxx - minx;
	yrange = maxy - miny;
	x_scale_factor = 1 / xrange;
	y_scale_factor = 1 / yrange;

	for (size_t i = 0; i < verts3d.size(); ++i) {
		Vec vert3 = verts3d[i];
//		GLfloat *vert2 = new GLfloat[2];
//		vert2[0] = vert3[xcomp] * x_scale_factor + 0.5;
//		vert2[1] = vert3[ycomp] * y_scale_factor + 0.5;
		Vec2 vert2;
		vert2.push_back( vert3[xcomp] * x_scale_factor + 0.5 );
		vert2.push_back( vert3[ycomp] * y_scale_factor + 0.5 );
		verts2d.push_back(vert2);
	}
}

// scale or translate vertices of the element in this space
// param: which - 0 -> scale x only, 1 -> scale y only
//		default == -1, any value other than 0,1 scales both
void UnitSpace2D::scale(GLfloat factor, GLint which) {
	if(which == 0 || which == 1) {
		for (size_t i = 0; i < element.verts2d.size(); ++i) {
			element.verts2d[i][which] *= factor;
		}
	} else {
		for (size_t i = 0; i < element.verts2d.size(); ++i) {
			element.verts2d[i][0] *= factor;
			element.verts2d[i][1] *= factor;
		}
	}
}
void UnitSpace2D::translate(GLfloat x, GLfloat y) {
	for (size_t i = 0; i < element.verts2d.size(); ++i) {
		element.verts2d[i][0] += x;
		element.verts2d[i][1] += y;
	}
}
// invert in x or y direction
	// param: which - 0 -> x, 1 -> y
void UnitSpace2D::flip(GLint which) {
	if(which < 0 || which > 1) {
		cout << "flip: bad param, should be 0,1 for x,y, but = " << which << endl;
		return;
	}
	for (size_t i = 0; i < element.verts2d.size(); ++i) {
		element.verts2d[i][which] = 1.0 - element.verts2d[i][which];
	}
}

// grid is rendered as a line loop of the x_lines
// and a line between the 2 endpoints of each of the y_lines
void Grid::render(GLfloat *color, GLfloat line_width) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	bool lighting_enabled = glIsEnabled(GL_LIGHTING);
	if(lighting_enabled) {
		glDisable(GL_LIGHTING);
	}
	glLineWidth(line_width);
	glColor3fv(color);
	for (size_t i = 0; i < x_lines.size(); ++i) {
		glBegin(GL_LINE_LOOP);
		for (size_t j = 0; j < x_lines[i].size(); ++j) {
			vec3 v;
			x_lines[i][j].array_out(v);
			glVertex3fv(v);
		}
		glEnd();
	}
	for (size_t i = 0; i < y_lines.size(); ++i) {
		if(y_lines[i].size() != 2) {
			cout << "Grid.render:  y_lines should only have 2 points, "
					<< "y_lines[" << i << "].size() = " << y_lines[i].size()
					<< endl << "** exiting **" << endl;
			exit(1);
		}
		glBegin(GL_LINES);
		vec3 v;
		y_lines[i][0].array_out(v);
		glVertex3fv(v);
		y_lines[i][1].array_out(v);
		glVertex3fv(v);
		glEnd();
	}
	if(lighting_enabled) {
		glEnable(GL_LIGHTING);
	}
	glPopAttrib();
}

CylinderModel::~CylinderModel() {

}

// does all the work
void CylinderModel::map() {
	GLfloat theta = 0;
	// arc length around  circumference of width of one unit space
	GLfloat unit_width = 2 * PI / (GLfloat)unit_spaces.size();
	// angle to beginning of current unit space (theta is taken parallel to z plane)
	GLfloat base_theta;
	// angle offset into unit space
	GLfloat offset_theta;

	for (size_t i = 0; i < unit_spaces.size(); ++i) {
		UnitSpace2D unit_space = unit_spaces[i];
		base_theta = i * unit_width;

		cout << "***********  cylinder map: starting unit space: "
						<< i << "  **********" << endl;
		cout << "unit_space.element.verts2d.size(): "
				<< unit_space.element.verts2d.size() << endl;
		for (size_t index = 0; index < unit_space.element.verts2d.size(); ++index) {
			GLfloat x2d = unit_space.element.verts2d[index][0];
			GLfloat y2d = unit_space.element.verts2d[index][1];
			GLfloat x3d, y3d, z3d;

//			cout << "x2d=" << x2d << "   y2d=" << y2d << endl;

			offset_theta = x2d;
			theta = base_theta + offset_theta;
			x3d = radius * sin(theta);
			z3d = radius * cos(theta);

			// the top of the cylinder is at y = height/2, bottom at y = -height/2
			// the unit space y is mapped over [0,1]
			y3d = (y2d - 0.5) * height;

			Vec vert = Vec(x3d, y3d, z3d);
//			cout << "cylinder map: vert: " << index << " " << vert << endl;
			vertex_sets[i].push_back(vert);

			// normal:  for this mapping, the normal is just radiating out from the y axis
			// could be changed ..
			Vec norm = Vec(x3d, 0, z3d).unit_vec();
//			cout << "map: norm: " << norm << endl;
			normal_sets[i].push_back(norm);
		}

//		cout << "***********  cylinder map: done with unit space: "
//				<< i << "**********" << endl;

	}
}

/**
 * map a point in unit space coordinates to the cylinder
 * params: unit_space - index of unit space on cylinder
 *     x,y - x and y values over [0, 1] within the unit space
 */
void CylinderModel::map_point(int unit_space, GLfloat x, GLfloat y, vec3 out) {
	assert(unit_space < (int)unit_spaces.size() && unit_space >= 0);
	assert( x >= 0 && x <= 1 &&  y >= 0 && y <= 1 );

	GLfloat theta = 0;
	// arc length around  circumference of width of one unit space
	GLfloat unit_width = 2 * PI / (GLfloat)unit_spaces.size();
	// angle to beginning of unit space (theta is taken parallel to z plane)
	GLfloat base_theta;
	// angle offset into unit space
	GLfloat offset_theta;
	GLfloat x3d, y3d, z3d;

	base_theta = unit_space * unit_width;
	offset_theta = x;
	theta = base_theta + offset_theta;
	x3d = radius * sin(theta);
	z3d = radius * cos(theta);

	// the top of the cylinder is at y = height/2, bottom at y = -height/2
	// the unit space y is mapped over [0,1]
	y3d = (y - 0.5) * height;

	setv(out, x3d, y3d, z3d);
}


// get a mesh grid on the circumference surface of this cylinder
// latitudes: y values from -1=bottom to 1=top
// longitudes: theta values, degrees from 0 to 360
void CylinderModel::grid(Grid& out, const vector<GLfloat>& latitudes,
		const vector<GLfloat>& longitudes) {
	out.x_lines.clear();
	out.y_lines.clear();
	// latitude requires series of vertices
	int numverts = 36;
	GLfloat theta;
	for (size_t i = 0; i < latitudes.size(); ++i) {
		vector<Vec> lat;
		for (int j = 0; j < numverts; ++j) {
			theta = j * 2*PI / numverts;
			GLfloat x = radius * sin(theta);
			GLfloat z = radius * cos(theta);
			GLfloat y = latitudes[i] * (height / 2.0);
			lat.push_back(Vec(x, y, z));
		}
		out.x_lines.push_back( lat );
	}

	// longitudes require only 2 vertices
	for (size_t i = 0; i < longitudes.size(); ++i) {
		GLfloat theta = radians(longitudes[i]);
		VertexSet l;
		for (int j = 0; j < 2; ++j) {
			GLfloat x = radius * sin(theta);
			GLfloat z = radius * cos(theta);
			GLfloat y = j == 0 ? -(height / 2.0) : (height / 2.0);
			l.push_back(Vec(x, y, z));
		}
		out.y_lines.push_back(l);
	}
}

// divides vertical and horizontal areas evenly using numlats, numlongs
void CylinderModel::grid(Grid& out, GLint num_lats, GLint num_longs) {
	vector<GLfloat> lats, longs;
	// longitudes
	for (int i = 0; i < num_longs; ++i) {
		GLfloat l = 360.0f / num_longs * i;
		longs.push_back(l);
	}
	for (int i = 0; i < num_lats; ++i) {
		GLfloat lat = -1.0 + (2.0/(num_lats-1)) * i;
		lats.push_back(lat);
	}
	lats.push_back(1.0f);
	grid(out, lats, longs);
}

