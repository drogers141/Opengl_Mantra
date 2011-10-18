/*
 * face.cpp
 *
 *	Implementation of faces and regions, see face.h
 *
 *  Created on: Jun 23, 2010
 *      Author: drogers
 */

#include "dr_util.h"
#include "vec.h"
//#include "geo.h"
#include "face.h"
#include "syllable.h"

#include <cstdio>
#include <cassert>
#include <iostream>
#include <ostream>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace DR;


bool Region::contains(const Polygon *poly) {
	return DR::contains(polygons, *poly);
}

bool Region::contains(GLint vert) {
	for (size_t i = 0; i < polygons.size(); ++i) {
		if(polygons[i]->contains(vert)) {
			return true;
		}
	}
	return false;
}

Face::Face() {
	parent = NULL;
	center_index = -1;
}
Face::Face(Syllable3D *syll) {
	parent = syll;
	center_index = -1;
}

// assumes that the face is centered at the origin
// and is in the xz plane with up being positive y
// creates perimeters starting with minvert being the vertex
// with the minimum x value, and the perimeter wrapping in a right hand
// manner
// so if there is more than 1 perimeter, they will be placed in perimeters
// in this manner, it is up to the Face to decide what perimeter is the outside,
// etc.
void Face::create_perimeters(Region& region,
		vector<vector<GLint> >& perimeters, bool debug) {
	vector<GLint> remaining, working, perim;
	vector<IndexedEdge> edges, used_edges, remaining_edges, temp_edges;
	vector<IndexedEdge>::iterator edge_it;
	vector<GLint>::iterator glint_it;
	vector<Polygon*> polys, temp_polys;
	GLint curr_vert;
	IndexedEdge curr_edge;

//	for (size_t i = 0; i < region.polygons.size(); ++i) {
//		Polygon *poly = region.polygons[i];
//		cout << i << "  " << poly->to_string() << endl;
//	}
//	cout << "*********************************************************************" << endl;

	// check all edges in region for those contained by only one polygon
	// this is the unordered set of edges on all perimeters
	for (size_t i = 0; i < region.polygons.size(); ++i) {
		Polygon *poly = region.polygons[i];
//		cout << i << "  " << poly->to_string() << endl;
		for (int j = 0; j < poly->size; ++j) {
//			GLint vert = poly->verts[j];
			IndexedEdge edge = poly->edges[j];
			find_polys_containing(polys, edge);

//			cout << "edge: " << edge.to_string() << "  polys " << polys.size() << "  ";
//			cout << "poly: " << poly->to_string() << endl;

			if(polys.size() == 1) {
				// checked insert -- we don't have uniq for this
				edge_it = find(edges.begin(), edges.end(), edge);
				if(edge_it == edges.end()) {
					edges.push_back(edge);
				}
				// debug
//				temp_polys.push_back(region.polygons[i]);

//				glint_it = find(remaining.begin(), remaining.end(), vert);
//				if(glint_it == remaining.end()) {
//					remaining.push_back(vert);
//				}
			}
		}
//		cout << endl;
	}
//	uniq(temp_polys);


	// edges contains all the perimeter edges in region
	// remaining edges will be used to show other perimeters in region
//	remaining_edges.clear();
	remaining_edges = edges;

	bool done = false;
	while(!done) {
		// ******* find a perimeter *********

		// start with vert with min x and find perimeter with
		// right hand winding
		GLfloat minx = 10000;
		GLint minvert = 10000;
		for (size_t i = 0; i < remaining_edges.size(); ++i) {
			vec3 vert;
			IndexedEdge e = remaining_edges[i];
			parent->get_vert(e.u, vert);
			if(vert[0] < minx) {
				minx = vert[0];
				minvert = e.u;
			}
			parent->get_vert(e.v, vert);
			if(vert[0] < minx) {
				minx = vert[0];
				minvert = e.v;
			}
		}
		if(debug) cout << "minx: " << minx << endl;
		perim.clear();
		perim.push_back(minvert);

		// determine right hand winding direction
		IndexedEdge e1, e2;
		GLint vert1, vert2;
		vec3 minv, v1, v2;
		temp_edges.clear();
		for (size_t i = 0; i < edges.size(); ++i) {
			if(edges[i].u == minvert || edges[i].v == minvert) {
				temp_edges.push_back(edges[i]);
			}
		}
		if(debug) cout << "temp_edges.size(): " << temp_edges.size() << endl;
		if(temp_edges.size() != 2) {
			cout << "create_perimeters: minx vert not in 2 edges, exiting " << endl;
			exit(1);
		}
		parent->get_vert(minvert, minv);
		e1 = temp_edges[0];
		e2 = temp_edges[1];
		vert1 = e1.u == minvert ? e1.v : e1.u;
		vert2 = e2.u == minvert ? e2.v : e2.u;
		parent->get_vert(vert1, v1);
		parent->get_vert(vert2, v2);
		if(debug) cout << "minv: " << stringv(minv) << "  v1: " << stringv(v1) << " v2: " << stringv(v2) << endl;
		if(debug) cout << "minvert: " << minvert << "  vert1: " << vert1 << "  vert2: " << vert2 << endl;

		Vec min_pt(minv), pt1(v1), pt2(v2);
		Vec vec1, vec2, cross;
		vec1 = pt1 - min_pt;
		vec2 = pt2 - min_pt;
		cross = vec1.cross(vec2);
		if(debug) cout << "v1 cross v2 = " << cross.to_string() << endl;
		if(cross.y > 0) {
			curr_edge = e1;
			curr_vert = vert1;
		} else {
			curr_edge = e2;
			curr_vert = vert2;
		}
		cross = vec2.cross(vec1);
		if(debug) cout << "v2 cross v1 = " << cross.to_string() << endl;

		//	cout << "curr_edge: " << curr_edge.to_string() << "  curr_vert: " << curr_vert << endl;

		// we should be off to a good start, so iterate around perim
		// ** note ** wish above statement were true, but not always, so
		// adding escape so program can continue and debug can be displayed
		used_edges.clear();
		while(curr_vert != minvert) {
			perim.push_back(curr_vert);
			used_edges.push_back(curr_edge);
			IndexedEdge next;
			bool found = false;
			for (size_t i = 0; i < edges.size(); ++i) {
				if(edges[i].contains(curr_vert)) {
					if(edges[i] != curr_edge) {
						next = edges[i];
						found = true;
						break;
					}
				}
			}
			if(!found) {
				cout << "create_perimeter: next edge not found !  exiting .." << endl;
				exit(1);
			}
			if(!next.contains(curr_vert)) {
				cout << "create_perimeter: next edge doesn't contain curr_vert !  exiting .." << endl;
				exit(1);
			}
			curr_vert = next.other_end(curr_vert);
			curr_edge = next;
			//*** debug
			//			vec3 tempv; parent->get_vert(curr_vert, tempv);
			//			cout << "curr_edge: " << curr_edge.to_string() << "  curr_vert: " << curr_vert
			//					<< "   " << stringv(tempv) << endl;

			// escape clause
			if(used_edges.size() > remaining_edges.size() * 1.1) {
				cout << "create_perimeter: used edges bigger than remaining edges " << endl
						<< "the perimeter search is not stopping back at the original vertex" << endl
						<< "breaking out .." << endl;
				done = true;
				break;
			}

		}
		// account for last edge, so it can be removed from remaining edges
		IndexedEdge last;
		last.u = perim[perim.size()-1];
		last.v = minvert;
		used_edges.push_back(last);

		// copy perimeter to outgoing perimeters
		vector<GLint> perim_copy;
		add_all(perim, perim_copy);
		perimeters.push_back(perim_copy);

		if(debug) cout << "perim.size(): " << perim.size() << endl;
		// ** debug - these are added at the end of init_regions,
		// so not necessary even for debug, unless we want to check something
		// else out
		//		for (size_t i = 0; /*i < 200 &&*/ i < perim.size(); ++i) {
		//			debug_verts.push_back(perim[i]);
		//		}
		if(debug) cout << "used_edges.size(): " << used_edges.size() << endl;
		if(debug) cout << "remaining_edges.size(): " << remaining_edges.size() << " before removing used edges " << endl;
		//******* end finding perimeter
		// reset used_edges for possible next perimeter after removing
		// all edges in it from remaining_edges -- which are the edges
		// remaining in the region
		for (size_t i = 0; i < used_edges.size(); ++i) {
			/*edge_it = find(remaining_edges.begin(), remaining_edges.end(), used_edges[i]);
						if(edge_it != remaining_edges.end()) {
							remaining_edges.erase(edge_it);
						}*/
			remaining_edges.erase( remove(remaining_edges.begin(), remaining_edges.end(), used_edges[i]),
							remaining_edges.end());
		}
		if(debug) cout << "remaining_edges.size(): " << remaining_edges.size() << " after " << endl;
		used_edges.clear();
		if(remaining_edges.empty()) {
			done = true;
		}
//		static int tempcount = 0;
//		if(tempcount++ > 1) done = true;
	}

//	cout << "edges.size(): " << edges.size() << endl;

//	for (size_t i = 0; i < edges.size(); ++i) {
//		debug_edges.push_back(edges[i]);
//		debug_points.push_back(edges[i].u);
//		debug_points.push_back(edges[i].v);
//	}

//	cout << "polys with edges: " << temp_polys.size() << endl;
//	print_polys(temp_polys);

//	cout << "remaining.size(): " << remaining.size() << endl;
//	for (size_t i = 0; i < remaining.size(); ++i) {
//		debug_points.push_back(remaining[i]);
//		cout << remaining[i] << "  ";
//		if(i % 4 == 3) {
//			cout << endl;
//		}
//	}


}

// reverses all perimeter windings
void Face::reverse_perimeter_windings() {
	for (size_t i = 0; i < regions.size(); ++i) {
		Region *r = regions[i];
		reverse(r->perimeter.begin() + 1, r->perimeter.end());

		for (size_t j = 0; j < r->inner_perimeters.size(); ++j) {
			reverse(r->inner_perimeters[j].begin() + 1, r->inner_perimeters[j].end());
		}
	}
}

Face::~Face() {
	for (size_t i = 0; i < polygons.size(); ++i) {
		delete polygons[i];
	}
	polygons.clear();
	for (size_t i = 0; i < regions.size(); ++i) {
		delete regions[i];
	}
	regions.clear();
}

void Face::add_polygon(Polygon *p) {
	polygons.push_back(p);
}

// remove all polygons and regions from this face, clearing storage
void Face::clear() {
	for (size_t i = 0; i < polygons.size(); ++i) {
		delete polygons[i];
	}
	polygons.clear();
	regions.clear();

	debug_edges.clear();
	debug_polygons.clear();
	debug_verts.clear();
}

void Face::find_polys_containing(vector<Polygon *>& out, GLint vert) {
	out.clear();
	for (size_t i = 0; i < polygons.size(); ++i) {
		Polygon *p = polygons[i];
		if(p->contains(vert)) {
			out.push_back(polygons[i]);
		}
	}
	uniq(out);
}
// returns polygon containing all verts
// if not found or error, does nothing
void Face::get_poly(vector<GLint> verts, Polygon& out) {
	if((GLint)verts.size() != out.size) {
		return;
	}
	for (size_t i = 0; i < verts.size(); ++i) {
		bool found = true;
		for (size_t j = 0; j < polygons.size(); ++j) {
			Polygon *p = polygons[i];
			for (size_t k = 0; k < verts.size(); ++k) {
				if(!(p->contains(verts[k])) ) {
					found = false;
					break;
				}
			}
			if(found) {
				out = *p;
				return;
			}
		}
	}
}
// returns index of poly in face.polygons vector
// returns -1 if poly not found
GLint Face::get_poly_index(const Polygon *poly) const {
	GLint index = -1;
	for (size_t i = 0; i < polygons.size(); ++i) {
		if(*poly == *polygons[i]) {
			index = i;
			break;
		}
	}
	return index;
}

void Face::find_polys_containing(vector<Polygon *>& out, const IndexedEdge& edge) {
	out.clear();
	for (size_t i = 0; i < polygons.size(); ++i) {
		if(polygons[i]->contains(edge)) {
			out.push_back(polygons[i]);
		}
	}
	uniq(out);
}

// put neighboring polys in out
// runs uniq on out and removes poly from it
void Face::get_neighbors(vector<Polygon *>& out, const Polygon *poly) {
	vector<Polygon *> working, temp;
	out.clear();
	for (int i = 0; i < poly->size; ++i) {
		GLint vert = poly->verts[i];
		find_polys_containing(working, vert);
		for (size_t j = 0; j < working.size(); ++j) {
			out.push_back(working[j]);
		}
		working.clear();
	}
	uniq(out);
	// remove original poly from neighbors
	for (size_t i = 0; i < out.size(); ++i) {
		if(*out[i] == *poly) {
			out.erase(out.begin()+i);
		}
	}
}
void Face::grow_region(Region& region, Polygon *start) {
	vector<Polygon*> fringe, neighbors, temp_polys, temp_polys2;
	vector<Polygon*>::iterator poly_it;
	get_neighbors(fringe, start);
//	int i=0;
	do {
//		cout << "i = " << i++ << "  fringe.size(): " << fringe.size() << endl;
//		cout << "grow_region: polys in fringe: " << endl;
//		print_polys(fringe, 100);
//		cout << "************************ end fringe ************" << endl;

		// remove duplicate polys from the fringe
		uniq(fringe);
		// add the fringe to the region, uniq it
		add_all(fringe, region.polygons);
		uniq(region.polygons);
//		copy(fringe.begin(), fringe.end(), back_inserter(region.polygons));
		// find neighbors of all the fringe
		neighbors.clear();
		for (size_t i = 0; i < fringe.size(); ++i) {
			get_neighbors(temp_polys, fringe[i]);
			add_all(temp_polys, neighbors);
			uniq(neighbors);
		}
		temp_polys.clear();
		// the difference between the old fringe and the neighbors
		// is the new fringe
		difference(fringe, neighbors, temp_polys);
		fringe.clear();
		add_all(temp_polys, fringe);
		// subtract fringe intersection with region
		intersection(fringe, region.polygons, temp_polys);
		difference(fringe, temp_polys, temp_polys2);
		fringe.clear();
		add_all(temp_polys2, fringe);

//		cout << "neighbors.size(): " << neighbors.size() << endl;

	} while(!fringe.empty());

//	cout << "region: polys: " << endl;
//	print_polys(region.polygons);
//	cout << "************************ end region **********************" << endl;
}


// find regions of connected polygons in this face
// initialize the regions with outer and possibly inner perimeters
void Face::init_regions(GLint start_vert) {
	vector<Polygon*> in_region, temp_polys;
	Polygon *start;
	bool all_polys = false;

	// start with connected polygons sharing starting vertex
	find_polys_containing(in_region, start_vert);
	if(in_region.empty()) {
		cout << "init_regions: no polys containing start vert: " << start_vert << endl;
		return;
	}
//	cout << "init regions: polys containing vert: " << start_vert << in_region.size() << endl;

	start = in_region[0];

	int rcount = 0;
	while(!all_polys) {
		Region *r = new Region();
		grow_region(*r, start);
		//** debug
//		cout << "region " << rcount++ <<  " polys: " << r->polygons.size() << endl;
//		add_all(r->polygons, debug_polygons);

		regions.push_back(r);

		// account for all polys already in a region
		in_region.clear();
		for (size_t i = 0; i < regions.size(); ++i) {
			add_all(regions[i]->polygons, in_region);
		}
		uniq(in_region); // shouldn't be nec.


		// if any polys not in a region, start another with the first
		difference(in_region, polygons, temp_polys);
		if(temp_polys.empty()) {
			all_polys = true;
		} else {
			start = temp_polys[0];
		}
	}

	// initialize perimeters in regions
	// if more than one perimeter is found, the first is
	// assumed to be the 'perimeter' -- ie outer, the rest (assume only 1 for
	// current scenario) are assumed to be inner
	for (size_t i = 0; i < regions.size(); ++i) {
		Region *r = regions[i];
		vector< vector<GLint> > perimeters;

//		cout << "**** Region: " << i << " creating perimeters ****" << endl;
		create_perimeters(*r, perimeters);
		//****debug version
//		create_perimeters(*r, perimeters, true);
//		cout << "**** Region: " << i << " done creating perimeters ****" << endl;
		if(perimeters.empty()) {
			cout << "Face.init_regions: no perimeters for region: " << i << endl;
			exit(1);
		}
		add_all(perimeters[0], r->perimeter);
		for (size_t j = 1; j < perimeters.size(); ++j) {
			vector<GLint> inner_p;
			r->inner_perimeters.push_back(inner_p);
			add_all(perimeters[j], r->inner_perimeters[j-1]);
		}
	}
	// set longest and shortest poly side lengths
	// **** todo: refactor this, creating an init that calls init_regions, then this
	GLfloat minlen = 10000, maxlen = 0;
	for (size_t i = 0; i < polygons.size(); ++i) {
		const Polygon *p = polygons[i];
//		for (size_t j = 0; j < p->size; ++j) {
//		}
		vector<Vec> verts;
		p->get_actual_verts(verts);
		for (size_t j = 0; j < verts.size(); ++j) {
			GLfloat sidelen = dist(verts[j], verts[(j+1)%verts.size()]);
			minlen = min(minlen, sidelen);
			maxlen = max(maxlen, sidelen);
		}
	}
	shortest_side_len = minlen;
	longest_side_len = maxlen;
	assert(maxlen > 0 && minlen < 10000);

	//** debug
//	cout << "regions.size() " << regions.size() << endl;
//	for (size_t i = 0; i < regions.size(); ++i) {
//		cout << "region " << i << ": perimeters: " << endl;
//		Region *r = regions[i];
//		cout << "r->perimeter.size(): " << r->perimeter.size() << endl;
//		cout << "inner perimeters: " << endl;
//		for (size_t j = 0; j < r->inner_perimeters.size(); ++j) {
//			cout << j << " perim size = " << r->inner_perimeters[j].size() << endl;
//		}
//	}
	//** debug
	// put all the perimeter vertices into debug_verts
	for (size_t i = 0; i < regions.size(); ++i) {
		Region *r = regions[i];
		for (size_t j = 0; j < r->perimeter.size(); j++) {
			debug_verts.push_back(r->perimeter[j]);
		}
		for (size_t j = 0; j < r->inner_perimeters.size(); ++j) {
			for (size_t k = 0; k < r->inner_perimeters[j].size(); ++k) {
				debug_verts.push_back(r->inner_perimeters[j][k]);
			}
		}
	}
//	for (size_t i = 0; i < regions.size(); ++i) {
//		cout << "region "<< i << " polys: " << endl;
//		print_polys(regions[i]->polygons, 100);
//		cout << "************************ end region **********************" << endl;
//	}
}

/**
 * Get center of center polygon.
 * Sets out to {0, 0, 0} if no center poly is set.
 */
void Face::get_center(GLfloat *out) {
	assert(center_index != -1);
	copyv(out, polygons[center_index]->center);
}

/**
 * Get copy of center poly, asserts that there is one.
 */
void Face::get_center(Polygon& out) {
	assert(center_index != -1);
	out = *polygons[center_index];
}
/**
 * Get the index of center poly.
 * Asserts center poly is set.
 */
GLint Face::get_center_index() {
	assert(center_index != -1);
	return center_index;
}

void Face::set_center(vec3 centerpt) {
	GLfloat mindist = 10000, dist_center;
	GLint cindex = -1;
	for (size_t i = 0; i < polygons.size(); ++i) {
		Polygon *p = polygons[i];
		dist_center = dist(centerpt, p->center);
		if(dist_center < mindist) {
			cindex = i;
			mindist = dist_center;
		}
	}
	assert(cindex > -1);
	center_index = cindex;
}


// decide which is center poly. for simplicity,
// assume syllable is flat on xz plane as it first
// comes in (ie assert up = Vec(0, 0, 1)
void Face::set_center(Vec up) {
	assert(up == Vec(0, 0, 1));
	// for default implementation, just find center of bounding
	// box, and figure which poly contains it
	GLfloat minx = 100000, maxx = -100000, minz = 100000, maxz = -100000;
	// use perimeters
	vec3 vert;
	for (size_t i = 0; i < regions.size(); ++i) {
		Region *r = regions[i];
		for (size_t j = 0; j < r->perimeter.size(); ++j) {
			GLint vert_index = r->perimeter[j];
			parent->get_vert(vert_index, vert);
			minx = min(minx, vert[0]);
			maxx = max(maxx, vert[0]);
			minz = min(minz, vert[2]);
			maxz = max(maxz, vert[2]);
		}
	}
//	cout << "***** Face::set_center ***** " << endl;
//	cout << "minx, maxx: " << minx << ", " << maxx <<endl;
//	cout << "minz, maxz: " << minz << ", " << maxz <<endl;
	GLfloat centerx = (minx + maxx)/2.0;
	GLfloat centerz = (minz + maxz)/2.0;
//	cout << "centerx: " << centerx << ",  centerz: " << centerz << endl;
	// debug
	debug_points.push_back( Vec(minx, 0, minz) );
	debug_points.push_back( Vec(minx, 0, maxz) );
	debug_points.push_back( Vec(maxx, 0, minz) );
	debug_points.push_back( Vec(maxx, 0, maxz) );
	debug_points.push_back( Vec(centerx, 0, centerz) );

	vec3 centerpt = {centerx, 0, centerz};
	GLfloat mindist = 10000, dist_center;

	GLint cindex = -1;
	for (size_t i = 0; i < polygons.size(); ++i) {
		Polygon *p = polygons[i];
		dist_center = dist(centerpt, p->center);
		if(dist_center < mindist) {
			cindex = i;
			mindist = dist_center;
		}
	}
	assert(cindex > -1);
	center_index = cindex;

}

