/*
 * Noise.h
 *
 * Include file for noise functions from glsldemo by 3dslabs,
 * with modifications.  See Noise.cpp, glsldemo, and the orange book.
 *
 *  Created on: Aug 21, 2010
 *      Author: drogers
 */

#ifndef NOISE_H_
#define NOISE_H_

#include "mygl.h"

//int Noise3DTexSize = 64;
//GLubyte* Noise3DTexPtr;

// don't use this until I know what I'm doing
void CreateNoise3D();
void make3DNoiseTexture();
void init3DNoiseTexture(int texSize, GLubyte* texPtr);

void SetNoiseFrequency(int frequency);
double noise1(double arg);
double noise2(double vec[2]);
double noise3(double vec[3]);

double PerlinNoise1D(double x,double alpha,double beta,int n);
double PerlinNoise2D(double x, double y, double alpha, double beta, int n);
double PerlinNoise3D(double x, double y, double z, double alpha, double beta, int n);



#endif /* NOISE_H_ */
