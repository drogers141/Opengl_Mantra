/*
 * dr_util.cpp
 *
 *  Created on: Feb 4, 2010
 *      Author: drogers
 */

#include "dr_util.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>

static const GLfloat PI = 3.14159265358979;

using namespace std;
using namespace DR;

//********** misc ************//
/**
 * Exit with message to stderr.
 * exit with status 1
 */
void DR::err_exit(string msg) {
	cerr << msg << endl;
	exit(1);
}

//*********************************************************************************************
//***********
//***********    Opengl and glsl utility stuff
//***********
//*********************************************************************************************

void DR::get_gl_version(int *major, int *minor) {
    const char *verstr = (const char *) glGetString(GL_VERSION);
    if ((verstr == NULL) || (sscanf(verstr,"%d.%d", major, minor) != 2))
    {
        *major = *minor = 0;
        fprintf(stderr, "Invalid GL_VERSION format!!!\n");
    }
}

void DR::get_glsl_version(int *major, int *minor) {
    int gl_major, gl_minor;
    get_gl_version(&gl_major, &gl_minor);

    *major = *minor = 0;
    if(gl_major == 1)
    {
        /* GL v1.x can only provide GLSL v1.00 as an extension */
        const char *extstr = (const char *) glGetString(GL_EXTENSIONS);
        if ((extstr != NULL) &&
            (strstr(extstr, "GL_ARB_shading_language_100") != NULL))
        {
            *major = 1;
            *minor = 0;
        }
    }
    else if (gl_major >= 2)
    {
        /* GL v2.0 and greater must parse the version string */
        const char *verstr =
            (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

        if((verstr == NULL) ||
            (sscanf(verstr, "%d.%d", major, minor) != 2))
        {
            *major = *minor = 0;
            fprintf(stderr,
                "Invalid GL_SHADING_LANGUAGE_VERSION format!!!\n");
        }
    }
}

bool GLInfo::get_info() {
	char* str = 0;
	char* tok = 0;

	// get vendor string
	str = (char*)glGetString(GL_VENDOR);
	if(str)
		this->vendor = str;                  // check NULL return value
	else
		return false;

	// get renderer string
	str = (char*)glGetString(GL_RENDERER);
	if(str)
		this->renderer = str;                // check NULL return value
	else
		return false;

	// get version string
	str = (char*)glGetString(GL_VERSION);
	if(str)
		this->version = str;                 // check NULL return value
	else
		return false;

	// get glsl
	int major, minor;
	get_gl_version( &major, &minor );
	get_glsl_version( &major, &minor );
	char glslstr[80];
	sprintf( glslstr, "%d.%d", major, minor );
	this->glsl_version = glslstr;

	// get all extensions as a string
	str = (char*)glGetString(GL_EXTENSIONS);

	// split extensions
	if(str)
	{
		tok = strtok((char*)str, " ");
		while(tok)
		{
			this->extensions.push_back(tok);    // put a extension into struct
			tok = strtok(0, " ");               // next token
		}
	}
	else
	{
		return false;
	}

	// sort extension by alphabetical order
	std::sort(this->extensions.begin(), this->extensions.end());

	// get number of color bits
	glGetIntegerv(GL_RED_BITS, &this->redBits);
	glGetIntegerv(GL_GREEN_BITS, &this->greenBits);
	glGetIntegerv(GL_BLUE_BITS, &this->blueBits);
	glGetIntegerv(GL_ALPHA_BITS, &this->alphaBits);

	// get depth bits
	glGetIntegerv(GL_DEPTH_BITS, &this->depthBits);

	// get stecil bits
	glGetIntegerv(GL_STENCIL_BITS, &this->stencilBits);

	// get max number of lights allowed
	glGetIntegerv(GL_MAX_LIGHTS, &this->maxLights);

	// get max texture resolution
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->maxTextureSize);

	// get max number of clipping planes
	glGetIntegerv(GL_MAX_CLIP_PLANES, &this->maxClipPlanes);

	// get max modelview and projection matrix stacks
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &this->maxModelViewStacks);
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &this->maxProjectionStacks);
	glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &this->maxAttribStacks);

	// get max texture stacks
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &this->maxTextureStacks);

	return true;
}
/**
 * check if the video card supports a certain extension
 */
bool GLInfo::is_extension_supported(const char* ext) {
	// search corresponding extension
	std::vector< std::string >::const_iterator iter = this->extensions.begin();
	std::vector< std::string >::const_iterator endIter = this->extensions.end();

	while(iter != endIter)
	{
		if(ext == *iter)
			return true;
		else
			++iter;
	}
	return false;
}

void GLInfo::print(ostream &out)  {
	out << "OpenGL Driver Info" << endl;
	out << "==================" << endl;
	out << "OpenGL Version: " << this->version  << endl;
	out << "GLSL   Version: " << this->glsl_version << endl;
	out << "Vendor: " << this->vendor << endl;
	out << "Renderer: " << this->renderer << endl;

	out << endl;
	out << "Color Bits(R,G,B,A): " << "(" << this->redBits << ", " << this->greenBits << ", "
			<< this->blueBits << ", " << this->alphaBits << ")" << endl;
	out << "Depth Bits: " << this->depthBits << endl;
	out << "Stencil Bits: " << this->stencilBits << endl;

	out << endl;
	out << "Max Texture Size: " << this->maxTextureSize << " x " << this->maxTextureSize << endl;
	out << "Max Lights: " << this->maxLights  << endl;
	out << "Max Clip Planes: " << this->maxClipPlanes << endl;
	out << "Max Modelview Matrix Stacks: " << this->maxModelViewStacks  << endl;
	out << "Max Projection Matrix Stacks: " << this->maxProjectionStacks  << endl;
	out << "Max Attribute Stacks: " << this->maxAttribStacks  << endl;
	out << "Max Texture Stacks: " << this->maxTextureStacks  << endl;

	out << endl;
	out << "Total Number of Extensions: " << this->extensions.size() << endl;
	out << "==============================" << endl;
	for(unsigned int i = 0; i < this->extensions.size(); ++i)
	{
		out << "" << extensions.at(i) << endl;
	}
	out << "======================================================================" << endl;

	out << endl;
}

void GLInfo::print(const char *file) {
	ofstream out;
	out.open(file);
	print(out);
}

void DR::gl_error(string msg) {
	GLenum errCode;
	const GLubyte *errString;
	int i=0;
	while ((errCode = glGetError()) != GL_NO_ERROR) {
		if(i++==0 && msg.size() > 0) cerr << "Error: " << msg << endl;
		errString = gluErrorString(errCode);
//		fprintf (stderr, "OpenGL Error: %s\n", errString);
		cerr << "OpenGL Error: " << errString << endl;
	}
	cerr.flush();
}

void DR::load_shader_src(int prog, const char *filename) {
//#define MAX_STRINGS 10000
//#define MAX_STRING_LEN 132
	const int MAX_STRINGS = 10000;
	const int MAX_STRING_LEN = 132;
	char *strings[MAX_STRINGS];
	int i = 0;
	FILE *f;
	f = fopen ( filename, "r" );
	if ( !f ) {
		printf ("cant open %s\n", filename );
		exit ( 1 );
	}
	int done = 0;
	char c;
	while ( !done ) {
		if ( (c = fgetc ( f )) == EOF ) {
			done = 1;
			continue;
		} else {
			ungetc ( c, f );
		}
		strings[i] = (char *) malloc ( MAX_STRING_LEN );
		fgets ( strings[i], MAX_STRING_LEN, f );
		i++;
	}

	glShaderSource ( prog, i, (const GLchar **)strings, NULL );

	for ( i--; i > 0; i-- ) {
		free ( strings[i] );
	}

}

void DR::compile_shader(int shader) {
	GLint status;
	glCompileShader ( shader );
	glGetShaderiv ( shader, GL_COMPILE_STATUS, &status );
	if ( !status ) {
		printf ( "Shader compile error:\n" );
		print_shader_info_log(shader);
	}
}
/**
 * Print log for shader after compiling.
 */
void DR::print_shader_info_log(GLuint shader) {
	int infologLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	gl_error(); // Check for OpenGL errors
	if (infologLen > 0)
	{
		infoLog = (GLchar*) malloc(infologLen);
		if (infoLog == NULL)
		{
			printf("ERROR: Could not allocate InfoLog buffer\n");
			exit(1);
		}
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
		printf("Shader Log:\n%s\n\n", infoLog);
		free(infoLog);
	}
	gl_error(); // Check for OpenGL errors
}

/**
 * Print log for shader program after linking shaders.
 */
void DR::print_program_info_log(GLuint program) {
	int infologLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLen);
	gl_error(); // Check for OpenGL errors
	if (infologLen > 0)
	{
		infoLog = (GLchar*) malloc(infologLen);
		if (infoLog == NULL)
		{
			printf("ERROR: Could not allocate InfoLog buffer\n");
			exit(1);
		}
		glGetProgramInfoLog(program, infologLen, &charsWritten, infoLog);
		printf("Program Log:\n%s\n\n", infoLog);
		free(infoLog);
	}
	gl_error(); // Check for OpenGL errors
}

void GLlight::set_opengl_state() {
	glLightfv(id, GL_POSITION, pos);
	glLightfv(id, GL_AMBIENT, ambient);
	glLightfv(id, GL_DIFFUSE, diffuse);
	glLightfv(id, GL_SPECULAR, specular);
}
string GLlight::name() {
	string light_name = "";
	switch(id) {
	case GL_LIGHT0: light_name = "0"; break;
	case GL_LIGHT1: light_name = "1"; break;
	case GL_LIGHT2: light_name = "2"; break;
	case GL_LIGHT3: light_name = "3"; break;
	case GL_LIGHT4: light_name = "4"; break;
	case GL_LIGHT5: light_name = "5"; break;
	default: light_name = ">5";
	}
	return light_name;
}
string GLlight::to_string(bool gory_details) {
	if(!gory_details) {
		return "light " + name() + ": " + stringv(pos, 4);
	} else {
		string gory = "light " + name() + ": " + stringv(pos, 4) + "\n";
		gory += "ambient: " + stringv(ambient, 4) + "\n";
		gory += "diffuse: " + stringv(diffuse, 4) + "\n";
		gory += "specular: " + stringv(specular, 4);
		return gory;
	}
}

// move a light source by a delta amount in 1 direction
// params: which: ie GL_LIGHT0, etc
// comp: one of {0, 1, 2} indicating move light in x, y, or z
// delta: how much to move in given direction
void DR::move_light(GLenum which, GLint comp, GLfloat delta, bool quiet) {
	GLfloat pos[4];
	glGetLightfv(which, GL_POSITION, pos);
//	cout << "move_light: pos before: " << stringv(pos, 4) << endl;
//	GLfloat ctm[16];
//	glGetFloatv(GL_MODELVIEW_MATRIX, ctm);
//	print_mat_col(ctm);
//	cout << endl;
	pos[comp] += delta;
	// need this for point light sources
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glLightfv(which, GL_POSITION, pos);
	glPopMatrix();
//	cout << "move_light: pos after: " << stringv(pos, 4) << endl;
	string light_name;
	switch(which) {
	case GL_LIGHT0: light_name = "0"; break;
	case GL_LIGHT1: light_name = "1"; break;
	case GL_LIGHT2: light_name = "2"; break;
	default: light_name = ">2";
	}
	if(!quiet) {
		cout << "Light: " << light_name << " position: " << stringv(pos, 4) << endl;
	}
	gl_error("move_light");
}

void DR::move_light(GLlight& which, GLint comp, GLfloat delta, bool quiet) {
	move_light(which.id, comp, delta, quiet);
	GLfloat pos[4];
	glGetLightfv(which.id, GL_POSITION, pos);
	copyv(which.pos, pos);
	gl_error("move_light (object)");
}

void DR::move_light(GLlight& which, vec4 pos) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glLightfv(which.id, GL_POSITION, pos);
	glPopMatrix();
	copyv(which.pos, pos);
	gl_error("move_light (object)");
}

/**
 * Output a string to the screen using glutBitmapCharacter()
 */
void DR::bitmap_output(GLfloat x, GLfloat y,const char *string, void *font) {
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}

/**
 * Convert vec3 from eye coordinates to world coordinates.
 */
bool DR::eye_to_world(const vec3 eye, vec3 world) {
	GLdouble ctm[16], inverse[16];
	glMatrixMode(GL_MODELVIEW);
	glGetDoublev(GL_MODELVIEW_MATRIX, ctm);
	bool success = invert(ctm, inverse);
	if(!success) {
		cout << "couldn't invert modelview" << endl;
		return false;
	} else {
//		cout << "inverse:" << endl;
//		print_mat_col(inverse);
		transform_vec(world, eye, inverse);
//		cout << "eye to world: eye: " << stringv(eye) << endl
//				<< "          world: " << stringv(world) << endl;
	}
	return true;
}

/**
 * Convert vec3 from eye coordinates to world coordinates.
 */
bool DR::world_to_eye(const vec3 world, vec3 eye) {
	GLdouble ctm[16];
	glMatrixMode(GL_MODELVIEW);
	glGetDoublev(GL_MODELVIEW_MATRIX, ctm);
	transform_vec(eye, world, ctm);
	return true;
}

//*********************************************************************************************
//***********
//***********    Non Object Oriented Vector Utility Functions
//***********
//*********************************************************************************************

/**
 * Compare floats for equality within tolerance.
 * Default tolerance=0.000001
 */
bool DR::almost_equal(GLfloat val1, GLfloat val2, GLfloat tolerance) {
	return fabs(val1 - val2) <= tolerance;
}

// equal within tolerance
bool DR::equal(const vec3 v1, const vec3 v2, GLfloat tolerance) {
	if( abs(v1[0] - v2[0]) < tolerance &&
			abs(v1[1] - v2[1]) < tolerance &&
			abs(v1[2] - v2[2]) < tolerance ) {
		return true;
	}
	return false;
}
bool DR::equal4(const vec4 v1, const vec4 v2, GLfloat tolerance) {
	if( abs(v1[0] - v2[0]) < tolerance &&
			abs(v1[1] - v2[1]) < tolerance &&
			abs(v1[2] - v2[2]) < tolerance &&
			abs(v1[3] - v2[3]) < tolerance) {
		return true;
	}
	return false;
}


GLfloat DR::degrees(float rads) {
	return rads * 180.0 / PI;
}
GLfloat DR::radians(float degrees) {
	return degrees * PI / 180.0;
}


///******* Vector stuff

/**
 * Returns magnitude or length of vector;
 */
GLfloat DR::magnitude(const GLfloat vec[3]) {
	return (GLfloat) sqrt( pow(vec[0], 2) + pow(vec[1], 2) + pow(vec[2], 2));
}

/**
 * Normalizes vector, ie turns it into a unit vector with the same direction.
 */
void DR::normalize(GLfloat vec[3]) {
	GLfloat mag = magnitude(vec);
	if(mag == 0) {
		vec[0] = vec[1] = vec[2] = 0;
	} else {
		for(int i=0; i<3; i++) {
			vec[i] /= mag;
		}
	}
}

/**
 * Puts the crossproduct of left X right into out.
 */
void DR::cross(GLfloat out[3], GLfloat left[3], GLfloat right[3]) {
	out[0] = left[1]*right[2] - left[2]*right[1]; // y*v.z - z*v.y
	out[1] = left[2]*right[0] - left[0]*right[2];// z*v.x - x*v.z
	out[2] = left[0]*right[1] - left[1]*right[0];// x*v.y - y*v.x
}

/**
 * Returns distance between points pt1 and pt2;
 */
GLfloat DR::dist(const GLfloat pt1[3], const GLfloat pt2[3]) {
	return (GLfloat) sqrt( pow( (pt2[0]-pt1[0]), 2) + pow( (pt2[1]-pt1[1]), 2) +
			pow( (pt2[2]-pt1[2]), 2));
}
GLdouble DR::dist(const GLdouble pt1[3], const GLdouble pt2[3]) {
	return sqrt( pow( (pt2[0]-pt1[0]), 2) + pow( (pt2[1]-pt1[1]), 2) +
			pow( (pt2[2]-pt1[2]), 2));
}

/**
 * Returns midpoint between 2 points
 */
void DR::midpoint(const vec3 pt1, const vec3 pt2, vec3 midpt) {
	for (int i = 0; i < 3; ++i) {
		midpt[i] = (pt1[i] + pt2[i]) / 2.0;
	}
}

/**
 * Print point or vec to stdout. Uses 3d array by default.
 * Set vec_sz for other size vec.
 */
void DR::printv(GLfloat *pt_or_vec, int vec_sz) {
	cout << stringv(pt_or_vec, vec_sz);
}

/**
 * Returns string version of 3d array by default.
 * Set vec_sz for other size vec.
 */
string DR::stringv(const GLfloat *array, int vec_sz, bool space_delimited) {
	stringstream out;
	string ret;
	out.flags(ios::fixed);
	out.precision(3);
	if(!space_delimited) {
		out << "(";
		for (int i = 0; i < vec_sz; ++i) {
			out << array[i] << ", ";
		}
		ret = out.str();
		ret = ret.substr( 0, ret.size()-2 ) + ")";
	} else {
		for (int i = 0; i < vec_sz; ++i) {
			out << array[i] << "  ";
		}
		ret = out.str();
	}
	return ret;
}
/**
 * Copies values from copy_from to copy_to.
 */
void DR::copyv(GLfloat *copy_to, const GLfloat *copy_from, int size) {
	for (int i = 0; i < size; ++i) {
		copy_to[i] = copy_from[i];
	}
}

/**
 * updates x, y, z values in point or vector,
 * ie indices 0, 1, 2
 */
void DR::setv(GLfloat *vec, GLfloat x, GLfloat y, GLfloat z) {
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
}
/**
 * Updates x,y,z,w = 0,1,2,3 for array of length 4
 */
void DR::setv(GLfloat *array, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	array[0] = x;
	array[1] = y;
	array[2] = z;
	array[3] = w;
}

void DR::draw_point(GLfloat pos[3], GLfloat color[3], GLfloat size) {
	glPushAttrib(GL_POINT_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(size);
	glColor3fv(color);
	glBegin(GL_POINTS);
	glVertex3fv(pos);
	glEnd();
	glPopAttrib();
}

void DR::draw_line(GLfloat from[3], GLfloat to[3], GLfloat color[3],
		GLfloat size, bool show_direction) {
	bool lighting = glIsEnabled(GL_LIGHTING);
	glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(size);
	glColor3fv(color);
	glBegin(GL_LINES);
	glVertex3fv(from);
	glVertex3fv(to);
	glEnd();
	if(show_direction) {
		glPointSize(2 * size);
		glBegin(GL_POINTS);
		glVertex3fv(to);
		glEnd();
	}
	glPopAttrib();
}

/**
 * Draw coordinate axes from origin in red, blue, green.
 * param: scale_factor - length of each axis is 1 * scale_factor
 */
void DR::draw_axes(GLfloat line_width, GLfloat scale_factor) {
	GLfloat from[3] = {0.0, 0.0, 0.0};
	GLfloat x[] = {1.0 * scale_factor, 0.0, 0.0};
	GLfloat y[] = {0.0, 1.0 * scale_factor, 0.0};
	GLfloat z[] = {0.0, 0.0, 1.0 * scale_factor};
	GLfloat r[] = {1.0, 0.0, 0.0};
	GLfloat g[] = {0.0, 1.0, 0.0};
	GLfloat b[] = {0.0, 0.0, 1.0};
	draw_line(from, x, r, line_width);
	draw_line(from, y, g, line_width);
	draw_line(from, z, b, line_width);
}

/**
 * Set values in vec to zero.
 */
void DR::zero(GLfloat vec[3]) {
	vec[0] = 0;
	vec[1] = 0;
	vec[2] = 0;
}

/**
 * Dot product for vectors, vec_sz defaults to 3, specify for other dim.
 */
GLfloat DR::dot(GLfloat *v1, GLfloat *v2, int vec_sz) {
	GLfloat dot = 0;
	for (int i = 0; i < vec_sz; ++i) {
		dot += v1[i]*v2[i];
	}
	return dot;
}


////********************* Matrix Stuff ***********************////
void DR::set_to_ident(GLfloat m[16]) {
	for (int i = 0; i < 16; ++i) {
		m[i] = 0;
	}
	m[0] = 1;
	m[5] = 1;
	m[10] = 1;
	m[15] = 1;
}
void DR::set_to_ident(GLdouble m[16]) {
	for (int i = 0; i < 16; ++i) {
		m[i] = 0;
	}
	m[0] = 1;
	m[5] = 1;
	m[10] = 1;
	m[15] = 1;
}

/**
 * Prints mat m in row order
 * Formatted so it can be copied and
 * pasted into a numpy array (use eval)
 */
void DR::print_mat(GLfloat m[16]) {
	printf("[");
	for (int i = 0; i < 4; ++i) {
		printf("[");
		for (int j = 0; j < 4; ++j) {
			printf("%f,  ", m[i*4 + j]);
		}
		if(i<3) {
			printf("],\n ");
		} else {
			printf("]]\n");
		}
	}
}

/**
 * Prints column-order (ie opengl) mat m in column order
 * Formatted so it can be copied and pasted into a numpy array (use eval)
 */
void DR::print_mat_col(GLfloat m[16]) {
	printf("[");
	for (int i = 0; i < 4; ++i) {
		printf("[");
		for (int j = 0; j < 4; ++j) {
			printf("%f,  ", m[i + 4*j]);
		}
		if(i<3) {
			printf("],\n ");
		} else {
			printf("]]\n");
		}
	}
}
void DR::print_mat_col(GLdouble m[16]) {
	printf("[");
	for (int i = 0; i < 4; ++i) {
		printf("[");
		for (int j = 0; j < 4; ++j) {
			printf("%f,  ", m[i + 4*j]);
		}
		if(i<3) {
			printf("],\n ");
		} else {
			printf("]]\n");
		}
	}
}
void DR::transpose(GLfloat out[16], const GLfloat in[16]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			out[i*4 + j] = in[i + 4*j];
		}
	}
}
void DR::transpose(GLdouble out[16], const GLdouble in[16]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			out[i*4 + j] = in[i + 4*j];
		}
	}
}

/**
 * Copies values from copy_from to copy_to.
 */
void DR::copym(GLfloat copy_to[16], GLfloat copy_from[16]) {
	for (int i = 0; i < 16; ++i) {
		copy_to[i] = copy_from[i];
	}
}
void DR::transform_vec(vec3 vOut, const vec3 v, const GLfloat m[16]) {
	vOut[0] = m[0] * v[0] + m[4] * v[1] + m[8] *  v[2] + m[12];// * v[3];
	vOut[1] = m[1] * v[0] + m[5] * v[1] + m[9] *  v[2] + m[13];// * v[3];
	vOut[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];// * v[3];
	//vOut[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
}
void DR::transform_vec(vec3 vOut, const vec3 v, const GLdouble m[16]) {
	vOut[0] = m[0] * v[0] + m[4] * v[1] + m[8] *  v[2] + m[12];// * v[3];
	vOut[1] = m[1] * v[0] + m[5] * v[1] + m[9] *  v[2] + m[13];// * v[3];
	vOut[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];// * v[3];
	//vOut[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
}

/**
 * Multiply left * right and put product into out.
 * ie: out <-- left right
 */
void DR::mult_matrixf(GLfloat out[16], GLfloat left[16], GLfloat right[16]) {
	GLfloat rows[4][4], cols[4][4];
	for (int i = 0; i < 4; ++i) {

		for (int j = 0; j < 4; ++j) {
			rows[i][j] = left[i*4 + j];
			cols[i][j] = right[i + 4*j];
		}
	}
//	cout << "left:" << endl;
//	cout << "[";
//	for (int i = 0; i < 4; ++i) {
//		cout << "[";
//		for (int j = 0; j < 4; ++j) {
//			cout << rows[i][j] << ", ";
//		}
//		cout << "]" << endl;
//	}
//	cout << "right:" << endl;
//	cout << "[";
//	for (int i = 0; i < 4; ++i) {
//		cout << "[";
//		for (int j = 0; j < 4; ++j) {
//			cout << cols[j][i] << ", ";
//		}
//		cout << "]" << endl;
//	}

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			out[i*4 + j] = dot(rows[i], cols[j], 4);
		}
	}

}

void DR::to2d(const GLfloat matrix[16], GLfloat out[4][4]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			out[i][j] = matrix[i + j*4];
		}

	}
}
void DR::to1d(const GLfloat matrix[4][4], GLfloat out[16]) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			out[i + j*4] = matrix[i][j];
		}

	}
}

//********* inversion of matrices and related *************//
/**
 * rotate vec with 3x3
 */
void _vec_rotate (const GLfloat vin[3], const GLfloat mtx[4][4], GLfloat vout[3])
{
	vout[0] = vin[0]*mtx[0][0] + vin[1]*mtx[1][0] + vin[2]*mtx[2][0];
	vout[1] = vin[0]*mtx[0][1] + vin[1]*mtx[1][1] + vin[2]*mtx[2][1];
	vout[2] = vin[0]*mtx[0][2] + vin[1]*mtx[1][2] + vin[2]*mtx[2][2];
}
/**
 * Invert the rotation submatrix in an opengl mat. by transposing.
 */
void _mat_inv_rotate(const GLfloat mtxin[4][4], GLfloat mtxout[4][4])
{
  for(int i=0 ; i<3; i++) {
    for(int j=0; j<3; j++) {
      mtxout[j][i] = mtxin[i][j];
    }
  }
}

/**
 * Invert from Nate Robins' lightposition.c.
 * returns false if not invertible
 */
GLboolean DR::invert(const GLdouble src[16], GLdouble inverse[16]) {
    double t;
    int i, j, k, swap;
    GLdouble tmp[4][4];

    set_to_ident(inverse);

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            tmp[i][j] = src[i*4+j];
        }
    }

    for (i = 0; i < 4; i++) {
        /* look for largest element in column. */
        swap = i;
        for (j = i + 1; j < 4; j++) {
            if (fabs(tmp[j][i]) > fabs(tmp[i][i])) {
                swap = j;
            }
        }

        if (swap != i) {
            /* swap rows. */
            for (k = 0; k < 4; k++) {
                t = tmp[i][k];
                tmp[i][k] = tmp[swap][k];
                tmp[swap][k] = t;

                t = inverse[i*4+k];
                inverse[i*4+k] = inverse[swap*4+k];
                inverse[swap*4+k] = t;
            }
        }

        if (tmp[i][i] == 0) {
        /* no non-zero pivot.  the matrix is singular, which
           shouldn't happen.  This means the user gave us a bad
            matrix. */
            return GL_FALSE;
        }

        t = tmp[i][i];
        for (k = 0; k < 4; k++) {
            tmp[i][k] /= t;
            inverse[i*4+k] /= t;
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                t = tmp[j][i];
                for (k = 0; k < 4; k++) {
                    tmp[j][k] -= tmp[i][k]*t;
                    inverse[j*4+k] -= inverse[i*4+k]*t;
                }
            }
        }
    }
    return GL_TRUE;
}

/**
 * Invert an opengl invertible matrix.
 */
void DR::invert(const GLfloat matrix[16], GLfloat inverse[16]) {
	GLfloat mat[4][4];
	GLfloat inv[4][4];
	to2d(matrix, mat);
//	to2d(inverse, inv);

	//Step 1. Transpose the 3x3 rotation portion of the 4x4 matrix to get the inverse rotation
	_mat_inv_rotate(mat, inv);

	//Step 2. negate the translation vector portion of the 4x4 matrix and then rotate it using the newly constructed 3x3 rotation matrix
	GLfloat vTmp[3], vTmp2[3];

	vTmp[0] = -mat[3][0];
	vTmp[1] = -mat[3][1];
	vTmp[2] = -mat[3][2];

	_vec_rotate(vTmp, inv, vTmp2);

	//Step 3. put the new translation vector into the new 4x4 matrix

	inv[3][0] = vTmp2[0];
	inv[3][1] = vTmp2[1];
	inv[3][2] = vTmp2[2];


	//Step 4. do some house cleaning so that the matrix can be used with OpenGL

	inv[0][3] = inv[1][3] = inv[2][3] = 0.0f;
	inv[3][3] = 1.0f;

	to1d(inv, inverse);
}

//The VectorRotate code is straight forward:
//
//void VectorRotate (const msVec3 vin, const glMatrix mtx, msVec3 vout)
//{
//	vout[0] = vin[0]*mtx[0][0] + vin[1]*mtx[1][0] + vin[2]*mtx[2][0];
//	vout[1] = vin[0]*mtx[0][1] + vin[1]*mtx[1][1] + vin[2]*mtx[2][1];
//	vout[2] = vin[0]*mtx[0][2] + vin[1]*mtx[1][2] + vin[2]*mtx[2][2];
//}

// **** took this from gamedev.net forum ******
//Here is another technique that I use for finding the inverse of a 4x4 matrix.
//Note: the matrices used are the OpenGL Column major format
//
//Step 1. Transpose the 3x3 rotation portion of the 4x4 matrix to get the inverse rotation
//
//    MatrixInvRotate(mtxin, mtxout);
//
//
//the MatrixInvRotate code is:
//
//void MatrixInvRotate(glMatrix mtxin, glMatrix mtxout)
//{
//  for(int i=0 ; i<3; i++) {
//    for(int j=0; j<3; j++) {
//      mtxout[j][i] = mtxin[i][j];
//    }
//  }
//}
//
//
//Step 2. negate the translation vector portion of the 4x4 matrix and then rotate it using the newly constructed 3x3 rotation matrix
//
//  msVec3 vTmp, vTmp2;
//
//  vTmp[0] = -mtxin[3][0];
//  vTmp[1] = -mtxin[3][1];
//  vTmp[2] = -mtxin[3][2];
//
//  VectorRotate(vTmp, mtxout, vTmp2);
//
//
//
//The VectorRotate code is straight forward:
//
//void VectorRotate (const msVec3 vin, const glMatrix mtx, msVec3 vout)
//{
//	vout[0] = vin[0]*mtx[0][0] + vin[1]*mtx[1][0] + vin[2]*mtx[2][0];
//	vout[1] = vin[0]*mtx[0][1] + vin[1]*mtx[1][1] + vin[2]*mtx[2][1];
//	vout[2] = vin[0]*mtx[0][2] + vin[1]*mtx[1][2] + vin[2]*mtx[2][2];
//}
//
//
//Step 3. put the new translation vector into the new 4x4 matrix
//
//  mtxout[3][0] = vTmp2[0];
//  mtxout[3][1] = vTmp2[1];
//  mtxout[3][2] = vTmp2[2];
//
//
//Step 4. do some house cleaning so that the matrix can be used with OpenGL
//
//  mtxout[0][3] = mtxout[1][3] = mtxout[2][3] = 0.0f;
//  mtxout[3][3] = 1.0f;
//
//
//

//******************************  Colors   *************************************//
GLfloat Util::red[] = {1.0, 0.0, 0.0};
GLfloat Util::purple[] = {.5, 0.0, .5};
GLfloat Util::blue[] = {0.0, 0.0, 1.0};
GLfloat Util::green[] = {0.0, 1.0, 0.0};
GLfloat Util::yellow[] = {1.0, 1.0, 0.0};
GLfloat Util::grey[] = {0.5, 0.5, 0.5};
GLfloat Util::magenta[] = {1.0, 0.0, 1.0};
GLfloat Util::white[] = {1.0, 1.0, 1.0};
GLfloat Util::cyan[] = {0.0, 1.0, 1.0};
GLfloat Util::black[] = {0.0, 0.0, 0.0};

int Util::numcolors = 10;

// all colors in order above as a vector of vec3's
vector<GLfloat *> Util::colors_vec() {
	vector<GLfloat *> v;
	v.push_back(red); v.push_back(purple); v.push_back(blue); v.push_back(green);
	v.push_back(yellow); v.push_back(grey); v.push_back(magenta); v.push_back(white);
	v.push_back(cyan); v.push_back(black);
	return v;
}



