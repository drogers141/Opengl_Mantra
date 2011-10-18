/*
 * particles.cpp
 *
 *  Created on: Jul 24, 2010
 *      Author: drogers
 */

#include "particles.h"
#include <algorithm>
#include <iostream>
#include <cassert>

using namespace std;
using namespace DR;

Particle::Particle()
: size(1.0), age(0), alive(false) {
}

Particle::Particle(vec4 color, GLfloat size, bool alive)
: size(size), age(0), alive(alive) {
	copyv(this->color, color);
}

Particle::Particle(const Particle& other) {
	this->size = other.size;
	this->age = other.age;
	this->alive = other.alive;
	copyv(this->color, other.color);
	copyv(this->position, other.position);
	copyv(this->velocity, other.velocity);

}

bool Particle::operator ==(const Particle& other) const {
	return (this->size == other.size &&
			this->alive == other.alive &&
			almost_equal(this->age, other.age) &&
			equal4(this->color, other.color) &&
			equal(this->position, other.position) &&
			equal(this->velocity, other.velocity)
	);
}

ParticleSet::ParticleSet(int num_particles, GLfloat start_time_ms)
: life_span(4.0), old_time_ms(start_time_ms) {
	Particle p;
	particles.assign(num_particles, p);
	for (int i = num_particles-1; i >= 0 ; --i) {
		dead_particles.push(i);
	}
	//		std::vector<Particle>::iterator it;
	//		for(it=particles.begin; it != particles.end; ++it) {
	//		}
}

void ParticleSet::init(int num_particles) {
	Particle p;
	cout << "particle set init" << endl;
	particles.assign(num_particles, p);
	cout << "num_particles: " << num_particles << endl;
	int j=0;
	for (int i = num_particles-1; i >= 0 ; --i) {
		dead_particles.push(i);
	}
}

void ParticleSet::remove(const Particle& p) {
	particles.erase( std::remove( particles.begin(), particles.end(), p),
			particles.end());
}

/**
 * Give particle new life
 */
bool ParticleSet::reincarnate(vec4 color, vec3 pos, vec3 vel, GLfloat size) {
	if(dead_particles.empty()) {
		return false;
	}
	int i = dead_particles.top();  dead_particles.pop();
	particles[i].alive = true;
	particles[i].age = 0.0f;
	copyv(particles[i].color, color, 4);
	copyv(particles[i].position, pos);
	copyv(particles[i].velocity, vel);
	particles[i].size = size;
	return true;
}

/**
 * Update position of particles, kill particles if past life span
 */
void ParticleSet::update(GLfloat time_ms) {
	GLfloat dt = 0.001 * (time_ms - old_time_ms);
	for (int i = 0; i < (int)particles.size(); ++i) {
		if(!particles[i].alive) continue;
		// kill dead particles
		if(particles[i].age > life_span) {
			particles[i].alive = false;
			dead_particles.push(i);
		} else {
			for (int j = 0; j < 3; ++j) {
				particles[i].position[j] += dt * particles[i].velocity[j];
				// add force effect here

			} // update age
			particles[i].age += dt;
		}
	}
	// reset millisec time
	old_time_ms = time_ms;
	glutPostRedisplay();
}
/**
 * Update position of particles, with alpha fading
 */
void ParticleSet::update_w_fade(GLfloat time_ms) {
	GLfloat dt = 0.001 * (time_ms - old_time_ms);
	for (int i = 0; i < (int)particles.size(); ++i) {
		if(!particles[i].alive) continue;
		// kill dead particles
		if(particles[i].age > life_span) {
			particles[i].alive = false;
			dead_particles.push(i);
		} else {
			for (int j = 0; j < 3; ++j) {
				particles[i].position[j] += dt * particles[i].velocity[j];
				// add force effect here

			} // update age
			particles[i].age += dt;
		}
	}
	// reset millisec time
	old_time_ms = time_ms;
	glutPostRedisplay();
}

void ParticleSet::render() {
	glPushAttrib(GL_POINT_BIT);
	glEnable(GL_POINT_SMOOTH);
	vector<Particle>::iterator it = particles.begin();
	for( ; it != particles.end(); ++it) {
		if(!(it->alive)) {
			continue;
		}
		glPointSize(it->size);
		glBegin(GL_POINTS);
		glColor4fv(it->color);
		glVertex3fv(it->position);
		glEnd();
	}
	glPopAttrib();
}

/**
 * Initializes base class with num_particles as well
 * as initializing number of beams.
 */
void LightBeamSet::init(int num_beams) {
	LightBeam l;
	beams.assign(num_beams, l);
	cout << "LightBeamSet::init: " << num_beams << "  beams" << endl;
	for (int i = num_beams-1; i >= 0 ; --i) {
		dead_beams.push_back(i);
	}
}

/**
 * Like reincarnate, but creates a light beam.
 * The information for the beam is all in the Particle front.
 * The back is just to draw with.  Need to set the age for
 * each beam.  When the age reached, the back travels with the
 * rest of the beam.
 */
bool LightBeamSet::get_beam(vec4 color, vec3 pos, vec3 vel,
		GLfloat life_span,  GLfloat length, GLfloat width) {

	// get a free beam
	if(dead_beams.empty()) {
		cout << "no light beams available" << endl;
		return false;
	}
	int b = dead_beams.back(); dead_beams.pop_back();
	for (int i = 0; i < 4; ++i) {
		beams[b].color[i] = color[i];
	}
	beams[b].front = Vec(pos);
	beams[b].tail = Vec(pos);
	beams[b].velocity = Vec(vel);
	beams[b].front_life_span = life_span;
	beams[b].width = width;
	beams[b].max_length = length;
	beams[b].tail_free = false;
	beams[b].alive = true;
	beams[b].age = 0.0;
	return true;
}

void LightBeamSet::update(GLfloat time_ms) {
	if( almost_equal(0.0, old_time_ms) ) {
		old_time_ms = time_ms;
		return;
	}
	GLfloat dt = 0.001 * (time_ms - old_time_ms);

	for (int i = 0; i < (int)beams.size(); ++i) {
		if(!beams[i].alive) continue;

		Vec pos_delta = dt * beams[i].velocity;
		Vec rayvec; // = beams[i].front - beams[i].tail;
		beams[i].as_vec(rayvec);

		// check if ray/beam is long enough to free tail
		if(rayvec.magnitude() >= beams[i].max_length) {
			beams[i].tail_free = true;
		}

		// if beam is older than front life span
		// front is no longer moving
		if(beams[i].age > beams[i].front_life_span) {
			// check if tail has caught up to front and
			// beam should die
			if(rayvec.dot(beams[i].velocity) < 0) {
				beams[i].kill();
				dead_beams.push_back(i);
				continue;
			}
			// free tail here as well, even though it means the beam
			// didn't live as long as its "maxlength" would allow
			// otherwise beam will get stuck
			beams[i].tail_free = true;

		} else {
			// front is still moving, so move it
			beams[i].front += pos_delta;
		}
		// move tail if free
		if(beams[i].tail_free) {
			beams[i].tail += pos_delta;
		}
		// update age
		beams[i].age += dt;

		// fade effect
		GLfloat alpha_factor = max((GLfloat)(/*1.5**/beams[i].front_life_span-beams[i].age), 0.0f);
		GLfloat alpha = min( (GLfloat)(alpha_factor/beams[i].front_life_span), 1.0f);
		assert(alpha <= 1.0f);
		beams[i].color[3] = alpha;
	}

	// reset millisec time
	old_time_ms = time_ms;
	glutPostRedisplay();
}


void LightBeamSet::render() {
	glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_LINE_STIPPLE);
	GLint factor;
	GLint i=0;
	GLfloat width;
	vec3 v;
	vector<LightBeam>::iterator it = beams.begin();
	for( ; it != beams.end(); ++it) {
		if(!(it->alive)) {
			continue;
		}
		++i;
		factor = rand() % 15 + 5;
		width = (rand() % 6) * 0.5 + 0.5;
		glLineStipple (5, 0x1C47);  //  dash/dot/dash * 5
		glLineWidth(width); //it->width);
		glBegin(GL_LINES);
		glColor4fv(it->color);
		it->tail.array_out(v);
		glVertex3fv(v);
		it->front.array_out(v);
		glVertex3fv(v);
		glEnd();
	}
	glPopAttrib();
}

void LightBeamSet::print_beam(GLuint index) {
	cout << beams[index] << endl;
}
void LightBeamSet::print_live_beams(GLuint how_many) {
	int index = 0;
	for (size_t i = 0; i < how_many; ++i) {
		vector<int>::iterator pos;
		do {
			if(index >= live_beams()) {
				return;
			}
			pos = find(dead_beams.begin(), dead_beams.end(), index++);

		} while( pos != dead_beams.end() );
		cout << "beam " << index << ": " << beams[index] << endl;
	}

}

/**
 * Get live beam [live_index] if exists, if live_index > live_beams()-1
 * then get the last live beam.
 * return: actual index, or -1 if beam found was not alive by the time found
 */
int LightBeamSet::get_live_beam(GLuint live_index, LightBeam& out) {
	GLuint num_live = (GLuint)live_beams();
	live_index = min(num_live-1, live_index);
	int index = 0;
	for (size_t i = 0; i <= live_index; ++i) {
		vector<int>::iterator pos;
		do {
			pos = find(dead_beams.begin(), dead_beams.end(), index++);

		} while( pos != dead_beams.end() );
	}
//	assert(beams[index].alive);
	if(!beams[index].alive) {
		return -1;
	}
	out = LightBeam(beams[index]);
	return index;
}


ostream& std::operator<<(ostream& out, const LightBeam& b) {
	out.flags(ios::fixed);
	out.precision(3);
	out << "front: " << b.front << ", tail: " << b.tail << endl
			<< "velocity: " << b.velocity << ", length: " << b.length() << ", max_length:  " << b.max_length << endl
			<< "tail_free: " << b.tail_free << ", alive: " << b.alive << ", age: " << b.age << endl;
	out << "front_life_span: " << b.front_life_span << ", color: " << stringv(b.color, 4) << endl;
	return out;
}

