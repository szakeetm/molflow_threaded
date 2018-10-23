/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#pragma once
#include "GLApp/GLTypes.h"
#include <stddef.h> //size_t for gcc
//#include "Buffer_shared.h"

// Desorption type
#define DES_NONE    0   // No desorption
#define DES_UNIFORM 1   // Uniform
#define DES_COSINE  2   // cos(theta)
#define DES_COSINE_N 3 // cos(theta)^N
#define DES_ANGLEMAP 4 //imported file

// (Old) Reflection types
#define REFLECTION_DIFFUSE 0   // Diffuse (cosine law)
#define REFLECTION_SPECULAR  1   // Mirror
#define REFLECTION_UNIFORM 2   // Uniform (for testing)

// Profile type
#define PROFILE_NONE       0  // No recording
#define PROFILE_U  1  // Pressure and density profile (U direction)
#define PROFILE_V  2  // Pressure and density profile (V direction)
#define PROFILE_ANGULAR    3  // Angular profile
#define PROFILE_VELOCITY   4 //Velocity distribution
#define PROFILE_ORT_VELOCITY 5 //Orthogonal velocity component
#define PROFILE_TAN_VELOCITY 6 //Tangential velocity (experimental)

// Hit type
#define HIT_DES   1
#define HIT_ABS   2
#define HIT_REF   3
#define HIT_TRANS 4
#define HIT_TELEPORTSOURCE 5
#define HIT_TELEPORTDEST 6
#define HIT_MOVING 7
#define HIT_LAST 10

#define MC_MODE 0         // Monte Carlo simulation mode
#define AC_MODE 1         // Angular coefficient simulation mode

typedef float ACFLOAT;

// Density/Hit field stuff
#define HITMAX 1E38
class ProfileSlice {
public:
	double countEquiv=0.0;
	double sum_v_ort=0.0;
	double sum_1_per_ort_velocity=0.0;
	ProfileSlice& operator+=(const ProfileSlice& rhs);
	ProfileSlice& operator+(const ProfileSlice& rhs);
};

class TextureCell {
public:
	double countEquiv=0.0;
	double sum_v_ort_per_area=0.0;
	double sum_1_per_ort_velocity=0.0;
	TextureCell& operator+=(const TextureCell& rhs);
	TextureCell& operator+(const TextureCell& rhs);
};

//Texture limit types
typedef struct {
	double all;
	double moments_only;
} TEXTURE_MOMENT_TYPE;

typedef struct {
	TEXTURE_MOMENT_TYPE min;
	TEXTURE_MOMENT_TYPE max;
} TEXTURE_MIN_MAX;

typedef struct {
	TEXTURE_MIN_MAX manual;
	TEXTURE_MIN_MAX autoscale;
} TEXTURE_SCALE_TYPE;

class AnglemapParams {
public:
	bool   record; // Record incident angle 2-dim distribution
	//bool hasRecorded; //Replaced with !angleMapCache.empty()
	size_t phiWidth; //resolution between -PI and +PI
	double thetaLimit; //angle map can have a different resolution under and over the limit. Must be between 0 and PI/2
	size_t thetaLowerRes; //resolution between 0 and angleMapThetaLimit
	size_t thetaHigherRes; //resolution between angleMapThetaLimit and PI/2
	
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(
			   record, // Record incident angle 2-dim distribution
		 /*hasRecorded,*/
		 phiWidth, //resolution between -PI and +PI
		 thetaLimit, //angle map can have a different resolution under and over the limit. Must be between 0 and PI/2
		 thetaLowerRes, //resolution between 0 and angleMapThetaLimit
		 thetaHigherRes //resolution between angleMapThetaLimit and PI/2
		);
	}

	size_t GetMapSize() {
		return phiWidth * (thetaLowerRes + thetaHigherRes);
	}
	/*
	size_t GetRecordedMapSize() {
		if (!hasRecorded) return 0;
		else return GetMapSize();
	}
	*/
	size_t GetDataSize() {
		return sizeof(size_t)*GetMapSize();
	}
	/*
	size_t GetRecordedDataSize() {
		return sizeof(size_t)*GetRecordedMapSize();
	}
	*/
};

class Reflection {
public:
	double diffusePart;
	double specularPart;
	double cosineExponent; //Cos^N part: 1-diffuse-specular
	
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(diffusePart, specularPart, cosineExponent);
	}
};

//Just for AC matrix calculation in Molflow, old mesh structure:
typedef struct {

	float   area;     // Area of element
	float   uCenter;  // Center coordinates
	float   vCenter;  // Center coordinates
	int     elemId;   // Element index (MESH array)
	bool    full;     // Element is full

} SHELEM_OLD;

