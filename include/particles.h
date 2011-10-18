/*
 * particles.h
 *
 * Particle class - relatively lightweight.  Modeled as a point
 * for now.
 *
 *  Created on: Jul 24, 2010
 *      Author: drogers
 */

#ifndef PARTICLES_H_
#define PARTICLES_H_

#include "dr_util.h"
#include "vec.h"
#include <vector>
#include <stack>

namespace DR {

class Particle {
public:
	Particle();
	Particle(vec4 color, GLfloat size=1.0f, bool alive=true);
	Particle(const Particle& other);
	bool operator==(const Particle& other) const;
	bool operator!=(const Particle& other) const {
		return !(*this == other);
	}

	vec4 color;
	vec3 position;
	vec3 velocity;
	GLfloat size;
	GLfloat age;  // seconds
	bool alive;
};

class LightBeam {
public:
	LightBeam()
	: front(), tail(), velocity(), width(1.0f), age(0),
	  front_life_span(4.0f), max_length(4.0f), tail_free(false), alive(false) {
		for (int i = 0; i < 4; ++i) {
			color[i] = 1.0f;
		}
	}
	LightBeam(const LightBeam& other) {
		front = other.front;
		tail = other.tail;
		velocity = other.velocity;
		copyv(color, other.color, 4);
		width = other.width;
		age = other.age;
		front_life_span = other.front_life_span;
		max_length = other.max_length;
		tail_free = other.tail_free;
		alive = other.alive;
	}
	//	LightBeam(Particle *end, vec3 origin)
//	: front(end), tail(origin), front_life_span(4.0), no_back(true) {}

	// simple light beam is a line connecting 2 points
	// front and tail
	// it is essentially a stretched out particle
	Vec front;
	Vec tail;
	Vec velocity;
	vec4 color;
	GLfloat width;
	GLfloat age; // secs

	// for beams, we want to vary the life span
	// this is named the front life span, because the front
	// of the beam is continually moving while alive, once
	// age hits the front life span, the beam/ray shrinks as
	// the tail catches up at which point the beam really dies
	GLfloat front_life_span;

	// the length of the beam after it has left the source and
	// before it starts to contract after the end of the front
	// life span.  this tells us when the back can start moving
	// according to the front particles velocity vector
	GLfloat max_length;
	// tail free if false while the ray is still connected
	// to the source, which is at member tail's location
	// no_back means that back doesn't get it's position updated
	// yet- the ray is still growing
	bool tail_free;
	// once dead, beam is free
	bool alive;

	/**
	 * Vector that is the front point of the beam minus the tail.
	 */
	void as_vec(Vec& out) const { out = front - tail; }
	/**
	 * Magnitude of as_vec vector.
	 */
	GLfloat length() const { return (front - tail).magnitude(); }
	/**
	 * Kill this beam.  Does not remove.
	 */
	void kill() { alive = false; age = 0.0; }

};

/**
 * Collection of particles, using stl, but keeping particle vector full,
 * and kill particles by setting their alive flag off.  They are then
 * available to whomever wants a new particle.
 */
class ParticleSet {
public:
	ParticleSet()
	: life_span(4.0), old_time_ms(0.0f) {}

	ParticleSet(int num_particles, GLfloat start_time_ms=0.0f);

	void init(int num_particles);

	/**
	 * Add particle to system.
	 */
	void add(const Particle p) { particles.push_back(p); }
	/**
	 * Remove particle from system.
	 */
	void remove(const Particle& p);

	/**
	 * Give particle new life
	 * return: false if no dead particles available
	 */
	bool reincarnate(vec4 color, vec3 pos, vec3 vel, GLfloat size=1.0f);

	virtual void update(GLfloat time_ms);
	void update_w_fade(GLfloat time_ms);
	virtual void render();

	bool is_empty() { return particles.empty(); }
	int size() { return particles.size(); }

	int total_particles() { return particles.size(); }
	int live_particles() { return particles.size() - dead_particles.size(); }

	// life span in secs
	GLfloat life_span;
	GLfloat old_time_ms;


protected:
	std::vector<Particle> particles;
	// stack of indices of dead particles in particles vector
	std::stack<int> dead_particles;

};

class LightBeamSet {
public:
	LightBeamSet()
	: old_time_ms(0.0f) { }

	/**
	 * Initializes base class with num_particles.
	 */
	void init(int num_beams);

	/**
	 * Like reincarnate, but creates a light beam.
	 */
	bool get_beam(vec4 color, vec3 pos, vec3 vel,
			GLfloat life_span,  GLfloat length, GLfloat width=1.0f);
	void update(GLfloat time_ms);
	void render();
	bool is_empty() { return beams.empty(); }
	int size() { return beams.size(); }

	int total_beams() { return beams.size(); }
	int live_beams() { return beams.size() - dead_beams.size(); }
	void print_beam(GLuint index);
	void print_live_beams(GLuint how_many);
	int get_live_beam(GLuint index, LightBeam& out);

	GLfloat old_time_ms;

protected:
	std::vector<LightBeam> beams;
	// stack of indices of dead beams in beams vector
	std::vector<int> dead_beams;

};



} // end namespace DR


namespace std {
ostream& operator<<(ostream&, const DR::LightBeam&);
}



#endif /* PARTICLES_H_ */
