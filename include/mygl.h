/*
 * mygl.h
 * Header to include before any others for opengl + glut + glew projects
 * For cross platform development, hopefully.
 * Note that Windows Visual Studio requires that glut.h comes after stdlib.h 
 * due to the redefinition of exit(), but even with precautions it is still a problem.
 * Using only this header may help, however best to add GLUT_BUILDING_LIB to
 * Properties/Configuration Properties/C/C++/Preprocessor/
 *
 *  Created on: Feb 4, 2010
 *      Author: drogers
 */

#ifndef MYGL_H
#define MYGL_H

#include <cstdlib>


#ifdef __APPLE__
#include <GL/glew.h>
#include <GLUT/glut.h>
#else

#include <GL/glew.h>
#include <GL/glut.h>

#endif


#endif  // MYGL_H
