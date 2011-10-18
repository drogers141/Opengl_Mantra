/*
 * animated_syllable.h
 *
 * Subclass (es?) of Syllable3D that can emit particles, and
 * whatever else we come up with.
 *
 *  Created on: Aug 2, 2010
 *      Author: drogers
 */

#ifndef ANIMATED_SYLLABLE_H_
#define ANIMATED_SYLLABLE_H_

#include "syllable.h"
#include "particles.h"
#include <map>


class AnimatedSyllable3D: public Syllable3D {
public:
	AnimatedSyllable3D()
	: Syllable3D() { }
//	AnimatedSyllable3D(char *objfile)
//	: Syllable3D(objfile) { }
//	AnimatedSyllable3D()
//	: Syllable3D() { }
	~AnimatedSyllable3D() { }

	/**
	 * Get light particles from syllable, add to part_set.
	 * params: num_per_face: if set, will generate this many for each face
	 * speed:  magnitude of velocity vector
	 * which_surface: which face (or sides can be selected),
	 * possible values = -1 -> all, BASE, EXTRUDED, SIDES
	 */
	void get_particles(DR::ParticleSet& part_set, GLfloat speed=1.0,
			int num_per_face=-1, int which_surface=-1);

//	void get_particles_verts(DR::ParticleSet& part_set, GLfloat speed=1.0);

	void get_beams(DR::LightBeamSet& beam_set, GLfloat speed=1.0,
			int num_per_face=-1, int which_surface=-1);

protected:
	// when generating a new beam, mark the index of the
	// poly (or vertex if we go that way) used for its starting point
	// by putting it in this vector, the index gets removed after the
	// beams' particles' age is past its life span and the back of the
	// beam is no longer at the source
	vector<int> used_indices;


	// key: index of poly, vertex, whatever used to generate
	// beam origin
	// value: ms value of age when index will be open (ie a beam will
	// not be emerging from the vertex or poly center)
	std::map<int, double> waitlist;

};

#endif /* ANIMATED_SYLLABLE_H_ */
