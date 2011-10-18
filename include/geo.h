/*
 * geo.h
 *
 *  Created on: Sep 6, 2010
 *      Author: drogers
 */

#ifndef GEO_H_
#define GEO_H_

#include "dr_util.h"
#include "vec.h"
#include "poly.h"

#include <iterator>
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>


namespace DR  {


typedef std::set< const DR::Polygon *, DR::PolygonCompare> PolygonSet;
typedef std::set<DR::Vec, DR::VecCompare>  VecSet;
typedef std::set<DR::vec3, DR::Vec3Compare>  Vec3Set;

typedef std::vector<const DR::Polygon *> PolygonArray;
typedef std::vector<DR::Vec>  VecArray;
typedef std::vector<DR::vec3>  Vec3Array;

typedef std::map<DR::Vec, DR::Polygon *, DR::VecCompare> VecPolyMap;

template <typename T>
inline void print_elements (const T& coll, string sep="\n", string head="")
{
	typename T::const_iterator pos;
	std::cout << head << std::endl;
	for (pos=coll.begin(); pos!=coll.end(); ++pos) {
		std::cout << *pos << sep;
	}
	std::cout << std::endl;
}

/**
 * Returns closest point to orig from collection of Vecs coll.
 * Puts closest in out param closest.
 * Returns distance from closest.
 */
template <typename T>
GLfloat closest (const T& coll, const Vec& orig, Vec& closest)
{
	typename T::const_iterator pos;
	GLfloat _dist, min_dist = 10000;
	for (pos=coll.begin(); pos!=coll.end(); ++pos) {
		Vec v = *pos;
		_dist = dist(orig, v);
		if(_dist < min_dist) {
			min_dist = _dist;
			closest = v;
		}
	}
	return min_dist;
}




} // end namespace DR

#endif /* GEO_H_ */
