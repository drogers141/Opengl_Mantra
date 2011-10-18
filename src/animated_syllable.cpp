/*
 * animated_syllable.cpp
 *
 *  Created on: Aug 2, 2010
 *      Author: drogers
 */

#include "animated_syllable.h"

using namespace std;
using namespace DR;

/**
 * Get light particles from syllable, add to part_set.
 * Actually, reincarnate() is used as a callback.  Particles from
 * part_set are reborn if available.
 * params: num_per_face: if set, will generate this many for each face
 * speed:  magnitude of velocity vector
 * which_surface: which face (or sides can be selected),
 * possible values = -1 -> all, BASE, EXTRUDED, SIDES
 */
void AnimatedSyllable3D::get_particles(ParticleSet& part_set, GLfloat speed,
			int num_per_face, int which_surface) {
	int num_in_face = base_face.polygons.size();
	if(num_per_face == -1) {
		num_per_face = num_in_face/3;
	}
	// assign random polygon indices to get particles from
	// within faces
	vector<int> poly_indices(num_per_face);
	for (int i = 0; i < num_per_face; ++i) {
		poly_indices.push_back( rand() % num_in_face );
	}
	// color
	vec4 col;
	// velocity temps
	Vec vel;
	vec3 vel3;
	// center of syllable
//	Vec syll_center = center;
	Vec syll_center = assigned_center;

	// ray to vertex on face
	Vec ray_from_center;

//	cout << "get_particles: syll_center: " << syll_center << endl;
	copyv(col, ambient_diffuse);
	col[3] = 1.0;
	bool got_particle(false);
//	cout << "get_particles: color: " << stringv(col, 4) << endl;

	// generate particles and add to part_set
	for (size_t i = 0; i < poly_indices.size(); ++i) {
		int index = poly_indices[i];
		Polygon *p;
//		Particle p1(1.0, col), p2(1.0, col);
		p = base_face.polygons[index];
		// use center of poly and facet norm
//		copyv(p1.position, vertices[p->verts[0]]);
//		copyv(p1.position, p->center);

		ray_from_center = Vec(p->center) - syll_center;

		// set direction * speed factor as particle's velocity
		// direction: custom for syllables on cylinder
		// base face normals point inward in xz towards y axis
		Vec n = p->facetnorm;
		// try adding ray from center to the normal and normalizing
		vel = n + ray_from_center;
		vel = vel.unit_vec() * speed;
		vel.array_out(vel3);
//		if(i == 100) cout << "base vel vec: " << vel << endl;

		got_particle = part_set.reincarnate(col, p->center, vel3);

		// do the same for the extruded face
		p = extruded_face.polygons[index];
		ray_from_center = Vec(p->center) - syll_center;
		n = p->facetnorm;
		// try adding ray from center to the normal and normalizing
		vel = n + ray_from_center;
		vel = vel.unit_vec() * speed;
		vel.array_out(vel3);
//		if(i == 100) cout << "extr vel vec: " << vel << endl;

		got_particle = part_set.reincarnate(col, p->center, vel3);

	}
}

/**
 * Like get_particles.
 */
void AnimatedSyllable3D::get_beams(LightBeamSet& beam_set, GLfloat speed,
		int num_per_face, int which_surface) {
	GLfloat min_lifetime = 1;
	GLfloat max_lifetime = 4;
	// length of beam
	GLfloat max_len = 20, min_len = 5;
	// beam width
	GLfloat width = 1.0;

	int num_in_face = base_face.polygons.size();
	if(num_per_face == -1) {
		num_per_face = num_in_face/4;
	}

	// assign random polygon indices to get particles from
	// within faces
	vector<int> poly_indices(num_per_face);
	for (int i = 0; i < num_per_face; ++i) {
		int poss_index = rand() % num_in_face;

		poly_indices.push_back( poss_index );
	}
	// color of beam (all same for now)
	vec4 color;
	// velocity temps
	Vec vel;
	vec3 vel3;
	// center of syllable
	//	Vec syll_center = center;
	Vec syll_center = assigned_center;

	// ray to vertex on face
	Vec ray_from_center;

	//	cout << "get_particles: syll_center: " << syll_center << endl;
	copyv(color, ambient_diffuse);
	color[3] = 1.0;
	//	cout << "get_particles: color: " << stringv(col, 4) << endl;

	// generate beams
	for (size_t i = 0; i < poly_indices.size(); ++i) {
		int index = poly_indices[i];
		Polygon *p;
		p = base_face.polygons[index];

		ray_from_center = Vec(p->center) - syll_center;

		// set direction * speed factor as particle's velocity
		// direction: custom for syllables on cylinder
		// base face normals point inward in xz towards y axis
		Vec n = p->facetnorm;
		// try adding ray from center to the normal and normalizing
		vel = n + ray_from_center;
		vel = vel.unit_vec() * speed;

		// just radiate from center
//		vel = ray_from_center.unit_vec() * speed;

		vel.array_out(vel3);
		//		if(i == 100) cout << "base vel vec: " << vel << endl;

		GLfloat life_span = rand() % (int)(max_lifetime-min_lifetime) + min_lifetime;
		GLfloat len = rand() % (int)(max_len-min_len) + min_len;
		beam_set.get_beam(color, p->center, vel3, life_span, len, width);

		// do the same for the extruded face
		p = extruded_face.polygons[index];
		ray_from_center = Vec(p->center) - syll_center;
		n = p->facetnorm;
		// try adding ray from center to the normal and normalizing
		vel = n + ray_from_center;
		vel = vel.unit_vec() * speed;

		// just radiate from center
//		vel = ray_from_center.unit_vec() * speed;

		vel.array_out(vel3);
		//		if(i == 100) cout << "extr vel vec: " << vel << endl;

		beam_set.get_beam(color, p->center, vel3, life_span, len, width);

	}

	// get beams from sides
	poly_indices.clear();
	// the sides are quads, possibly with bigger area
	int num_in_sides = sides.size();
	int num_for_sides = (int)(num_in_sides * .75);

	for (int i = 0; i < num_for_sides; ++i) {
		int poss_index = rand() % num_in_sides;

		poly_indices.push_back( poss_index );
	}

	for (size_t i = 0; i < poly_indices.size(); ++i) {
		int index = poly_indices[i];
		Polygon *p;
		p = sides[index];

		ray_from_center = Vec(p->center) - syll_center;

		// set direction * speed factor as particle's velocity
		// direction: custom for syllables on cylinder
		// base face normals point inward in xz towards y axis
//		Vec n = p->facetnorm;
		// try adding ray from center to the normal and normalizing
//		vel = n + ray_from_center;
//		vel = vel.unit_vec() * speed;

		// just radiate from center
		vel = ray_from_center.unit_vec() * speed;
		vel.array_out(vel3);
		//		if(i == 100) cout << "base vel vec: " << vel << endl;

		GLfloat life_span = rand() % (int)(max_lifetime-min_lifetime) + min_lifetime;
		GLfloat len = rand() % (int)(max_len-min_len) + min_len;
		beam_set.get_beam(color, p->center, vel3, life_span, len, width);
	}

//	cout << "live beams: " << beam_set.live_beams() << endl;
//	cout << "first couple of live beams: " << endl;
//	beam_set.print_live_beams(2);


}

//void AnimatedSyllable3D::get_particles_verts(ParticleSet& part_set, GLfloat speed) {
////	int num_in_face = base_face.polygons.size();
////	if(num_per_face == -1) {
////		num_per_face = num_in_face/3;
////	}
//	int num_verts = num_vertices_base / 3;
//	// assign random polygon indices to get particles from
//	// within faces
//	vector<int> poly_indices(num_verts);
//	for (int i = 0; i < num_verts; ++i) {
//		poly_indices.push_back( rand() % num_vertices_base );
//	}
//	cout << "implement me" << endl;
//	return;
//
//	// color
//	vec4 col;
//	// velocity temps
//	Vec vel;
//	vec3 vel3;
//	// center of syllable
////	Vec syll_center = center;
//	Vec syll_center = assigned_center;
//
//	// ray to vertex on face
//	Vec ray_from_center;
//
////	cout << "get_particles: syll_center: " << syll_center << endl;
//	copyv(col, ambient_diffuse);
//	col[3] = 1.0;
//	bool got_particle(false);
////	cout << "get_particles: color: " << stringv(col, 4) << endl;
//
//	// generate particles and add to part_set
//	for (size_t i = 0; i < poly_indices.size(); ++i) {
//		int index = poly_indices[i];
//		Polygon *p;
////		Particle p1(1.0, col), p2(1.0, col);
//		p = base_face.polygons[index];
//		// use center of poly and facet norm
////		copyv(p1.position, vertices[p->verts[0]]);
////		copyv(p1.position, p->center);
//
//		ray_from_center = Vec(p->center) - syll_center;
//
//		// set direction * speed factor as particle's velocity
//		// direction: custom for syllables on cylinder
//		// base face normals point inward in xz towards y axis
//		Vec n = p->facetnorm;
//		// try adding ray from center to the normal and normalizing
//		vel = n + ray_from_center;
//		vel = vel.unit_vec() * speed;
//		vel.array_out(vel3);
////		if(i == 100) cout << "base vel vec: " << vel << endl;
//
//		got_particle = part_set.reincarnate(col, p->center, vel3);
//
//		// do the same for the extruded face
//		p = extruded_face.polygons[index];
//		ray_from_center = Vec(p->center) - syll_center;
//		n = p->facetnorm;
//		// try adding ray from center to the normal and normalizing
//		vel = n + ray_from_center;
//		vel = vel.unit_vec() * speed;
//		vel.array_out(vel3);
////		if(i == 100) cout << "extr vel vec: " << vel << endl;
//
//		got_particle = part_set.reincarnate(col, p->center, vel3);
//
//	}
//}
