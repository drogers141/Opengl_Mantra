/*
 * dr_util.h
 * Utility stuff for OpenGL. Not object oriented.
 *
 *  Created on: Feb 4, 2010
 *      Author: drogers
 */

#ifndef DR_UTIL_H_
#define DR_UTIL_H_

#include "mygl.h"

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <functional>


namespace DR  {


using std::ostream;
using std::string;

const GLfloat PI = 3.14159265358979;
const GLfloat FLOAT_TOLERANCE = 0.000001; //** orig 0.000001;


// use typedef for normals, vertices, etc.
typedef GLfloat vec3[3];
typedef GLfloat vec4[4];

//********** misc ************//
/**
 * Exit with message to stderr.
 */
void err_exit(string msg);

//*********************************************************************************************
//***********
//***********    Opengl and glsl utility stuff
//***********
//*********************************************************************************************

void get_gl_version(int *major, int *minor);

void get_glsl_version(int *major, int *minor);

void gl_error(string msg="");

void load_shader_src(int prog, const char *filename);

void compile_shader(int shader);

void print_shader_info_log(GLuint shader);

void print_program_info_log(GLuint program);

// move a light source
// params: which: ie GL_LIGHT0, etc
// comp: one of {0, 1, 2} indicating move light in x, y, or z
// delta: how much to move in given direction
// quiet: if false, prints new position
void move_light(GLenum which, GLint comp, GLfloat delta, bool quiet=true);

// information for 1 opengl light source
struct GLlight {
	GLlight() {}
	// eg GL_LIGHT0, ..
	GLenum 	id;
	vec4	pos;
	vec4	ambient;
	vec4	diffuse;
	vec4	specular;

	void set_opengl_state();
	string name();
	string to_string(bool gory_details=false);
};

void move_light(GLlight& which, GLint comp, GLfloat delta, bool quiet=true);
void move_light(GLlight& which, vec4 pos);
/*
 * OpenGL information
 */
struct GLInfo {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string glsl_version;
    std::vector <std::string> extensions;
    GLint redBits;
    GLint greenBits;
    GLint blueBits;
    GLint alphaBits;
    GLint depthBits;
    GLint stencilBits;
    GLint maxTextureSize;
    GLint maxLights;
    GLint maxAttribStacks;
    GLint maxModelViewStacks;
    GLint maxProjectionStacks;
    GLint maxClipPlanes;
    GLint maxTextureStacks;

    GLInfo() : redBits(0), greenBits(0), blueBits(0), alphaBits(0), depthBits(0),
               stencilBits(0), maxTextureSize(0), maxLights(0), maxAttribStacks(0),
               maxModelViewStacks(0), maxClipPlanes(0), maxTextureStacks(0) {}

    /**
     * must be called after GL rendering context opened
     */
    bool get_info();
    /**
     * check if the video card supports a certain extension
     */
    bool is_extension_supported(const char* ext);

    void print(std::ostream &out=std::cout) ;
    void print(const char *file);
};

/**
 * Output a string to the screen using glutBitmapCharacter()
 */
void bitmap_output(GLfloat x, GLfloat y,const char *string, void *font);

/**
 * Convert vec3 from eye coordinates to world coordinates.
 */
bool eye_to_world(const vec3 eye, vec3 world);
/**
 * Convert vec3 from world coordinates to eye coordinates.
 */
bool world_to_eye(const vec3 world, vec3 eye);


// see test.cpp
// in lieu of actually using unit testing
void test();


//*********************************************************************************************
//***********
//***********    Non Object Oriented Vector Utility Functions
//***********
//*********************************************************************************************

/**
 * Comparison functor for vec3 ie, GLfloat *, assuming 3 members.  Can therefore use in sets, etc
 */
struct Vec3Compare: std::binary_function<const vec3, const vec3, bool> {
	bool operator()(const vec3 lhs, const vec3 rhs) const {
		if(lhs[0] < rhs[0]) {
			return true;
		} else if(lhs[1] < rhs[1]) {
			return true;
		} else if(lhs[2] < rhs[2]) {
			return true;
		}
		return false;
	}
};

/**
 * Compare floats for equality within tolerance.
 * Default tolerance=0.000001
 */
bool almost_equal(GLfloat val1, GLfloat val2, GLfloat tolerance=0.000001);

// equal within tolerance
bool equal(const vec3 v1, const vec3 v2, GLfloat tolerance=0.000001);
bool equal4(const vec4 v1, const vec4 v2, GLfloat tolerance=0.000001);


GLfloat degrees(float rads);
GLfloat radians(float degrees);


///******* Vector stuff

/**
 * Returns magnitude or length of vector;
 */
GLfloat magnitude(const GLfloat vec[3]);

/**
 * Normalizes vector, ie turns it into a unit vector with the same direction.
 */
void normalize(GLfloat vec[3]);

/**
 * Puts the crossproduct of left X right into out.
 */
void cross(GLfloat out[3], GLfloat left[3], GLfloat right[3]);

/**
 * Returns distance between points pt1 and pt2;
 */
GLfloat dist(const GLfloat pt1[3], const GLfloat pt2[3]);
GLdouble dist(const GLdouble pt1[3], const GLdouble pt2[3]);

/**
 * Returns midpoint between 2 points
 */
void midpoint(const vec3 pt1, const vec3 pt2, vec3 midpt);

/**
 * Print point or vec to stdout. Uses 3d array by default.
 * Set vec_sz for other size vec.
 */
void printv(GLfloat *pt_or_vec, int vec_sz=3);

/**
 * Returns string version of 3d array by default.
 * Set vec_sz for other size vec.
 */
std::string stringv(const GLfloat *array, int vec_sz=3, bool space_delimited=false);

/**
 * Copies values from copy_from to copy_to.
 * Set size if you want to copy size other than 3.
 */
void copyv(GLfloat *copy_to, const GLfloat *copy_from, int size=3);

/**
 * updates x, y, z values in point or vector,
 * ie indices 0, 1, 2,
 */
void setv(GLfloat *pt_or_vec, GLfloat x, GLfloat y, GLfloat z);
/**
 * Updates x,y,z,w = 0,1,2,3 for array of length 4
 */
void setv(GLfloat *array, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

void draw_point(GLfloat pos[3], GLfloat color[3], GLfloat size=3);

void draw_line(GLfloat from[3], GLfloat to[3], GLfloat color[3],
		GLfloat size=1, bool show_direction=false);

/**
 * Draw coordinate axes from origin in red, blue, green.
 * param: scale_factor - length of each axis is 1 * scale_factor
 */
void draw_axes(GLfloat line_width=1, GLfloat scale_factor=1);

/**
 * Dot product for vectors, vec_sz defaults to 3, specify for other dim.
 */
GLfloat dot(GLfloat *v1, GLfloat *v2, int vec_sz=3);

/**
 * Set values in vec to zero.
 */
void zero(GLfloat vec[3]);


////************************* Matrix Stuff **********************////

/**
 * Sets column-order (ie opengl) matrix m to identity matrix.
 */
void set_to_ident(GLfloat m[16]);
void set_to_ident(GLdouble m[16]);

/**
 * Prints column-order (ie opengl) mat m in row order
 * Formatted so it can be copied and pasted into a numpy array (use eval)
 */
void print_mat(GLfloat m[16]);

/**
 * Prints column-order (ie opengl) mat m in column order
 * Formatted so it can be copied and pasted into a numpy array (use eval)
 */
void print_mat_col(GLfloat m[16]);
void print_mat_col(GLdouble m[16]);

void transpose(GLfloat out[16], const GLfloat in[16]);
void transpose(GLdouble out[16], const GLdouble in[16]);

/**
 * Copies values from copy_from to copy_to.
 */
void copym(GLfloat copy_to[16], GLfloat copy_from[16]);

/**
 * Multiply left * right and put product into out.
 * ie: out <-- left right
 * Does row major order.
 */
void mult_matrixf(GLfloat out[16], GLfloat left[16], GLfloat right[16]);

void transform_vec(vec3 vOut, const vec3 v, const GLfloat m[16]);
void transform_vec(vec3 vOut, const vec3 v, const GLdouble m[16]);

/**
 * Invert an opengl invertible matrix.
 */
void invert(const GLfloat mat[16], GLfloat inverse[16]);
/**
 * Invert from Nate Robins' lightposition.c.
 * returns false if not invertible
 */
GLboolean invert(const GLdouble src[16], GLdouble inverse[16]);

void to2d(const GLfloat matrix[16], GLfloat out[4][4]);
void to1d(const GLfloat matrix[4][4], GLfloat out[16]);

}

////*************************   Colors    **********************////
// Just for getting these colors, etc
// this may go away
class Util {
public:
	// colors
	static GLfloat red[];
	static GLfloat purple[]; // (.5, 0.0, .5)
	static GLfloat blue[];
	static GLfloat green[];
	static GLfloat yellow[]; // (1.0, 1.0, 0.0)
	static GLfloat grey[];
	static GLfloat magenta[];  // (1.0, 0.0, 1.0)
	static GLfloat white[];
	static GLfloat black[];
	static GLfloat cyan[];   // (0.0, 1.0, 1.0)

	static int numcolors;

	// all colors in order above as a vector of vec3's
	static std::vector<GLfloat *> colors_vec();

};


#endif /* DR_UTIL_H_ */
