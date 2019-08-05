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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "IntersectAABB_shared.h"
#include "Random.h"
#include "Polygon.h" //IsInPoly
#include "GLApp/MathTools.h"
#include <algorithm> //std::min
#include "Simulation.h"
#include "Worker.h"
#include "SuperFacet.h"
#include <tuple>

// AABB tree stuff

// Minimum number of facet inside a BB
#define MINBB    1
#define MAXDEPTH 50

std::tuple<size_t,size_t,size_t> AABBNODE::FindBestCuttingPlane() {

	// AABB tree balancing

	double centerX = (bb.min.x + bb.max.x) / 2.0;
	double centerY = (bb.min.y + bb.max.y) / 2.0;
	double centerZ = (bb.min.z + bb.max.z) / 2.0;
	size_t rightFromCenterX = 0;
	size_t rightFromCenterY = 0;
	size_t rightFromCenterZ = 0;
	size_t planeType; //1: YZ, 2: XZ, 3: XY
	double best = 1e100;
	size_t nbLeft, nbRight;

	for (const auto& f : facets) {
		if (f->facetRef->sh.center.x > centerX) rightFromCenterZ++;
		if (f->facetRef->sh.center.y > centerY) rightFromCenterY++;
		if (f->facetRef->sh.center.z > centerZ) rightFromCenterX++;
	}

	double deviationFromHalfHalf_X = fabs((double)rightFromCenterZ - (double)(facets.size()) / 2.0);
	if (deviationFromHalfHalf_X < best) {
		best = deviationFromHalfHalf_X;
		nbLeft = facets.size() - rightFromCenterZ;
		nbRight = rightFromCenterZ;
		planeType = 1;
	}
	double deviationFromHalfHalf_Y = fabs((double)rightFromCenterY - (double)(facets.size()) / 2.0);
	if (deviationFromHalfHalf_Y < best) {
		best = deviationFromHalfHalf_Y;
		nbLeft = facets.size() - rightFromCenterY;
		nbRight = rightFromCenterY;
		planeType = 2;
	}
	double deviationFromHalfHalf_Z = fabs((double)rightFromCenterX - (double)(facets.size()) / 2.0);
	if (deviationFromHalfHalf_Z < best) {
		best = deviationFromHalfHalf_Z;
		nbLeft = facets.size() - rightFromCenterX;
		nbRight = rightFromCenterX;
		planeType = 3;
	}

	return { planeType,nbLeft,nbRight };

}

void AABBNODE::ComputeBB() {

	bb.max=Vector3d(-1e100,-1e100,-1e100);
	bb.min=Vector3d(1e100,1e100,1e100);

	for (const auto& f : facets) {
		bb.min.x = std::min(f->facetRef->sh.bb.min.x,bb.min.x);
		bb.min.y = std::min(f->facetRef->sh.bb.min.y, bb.min.y);
		bb.min.z = std::min(f->facetRef->sh.bb.min.z, bb.min.z);
		bb.max.x = std::max(f->facetRef->sh.bb.max.x, bb.max.x);
		bb.max.y = std::max(f->facetRef->sh.bb.max.y, bb.max.y);
		bb.max.z = std::max(f->facetRef->sh.bb.max.z, bb.max.z);
	}

}

AABBNODE *BuildAABBTree(const std::vector<SubprocessFacet*>& facets, const size_t depth,size_t& maxDepth) {

	size_t    nbl = 0, nbr = 0;
	double m;
	
	maxDepth = std::max(depth, maxDepth); //debug
	if (depth >= MAXDEPTH) return NULL;

	AABBNODE *newNode = new AABBNODE();
	newNode->facets = facets;
	newNode->ComputeBB();

	auto [planeType, nbLeft, nbRight] = newNode->FindBestCuttingPlane();

	if (nbLeft >= MINBB && nbRight >= MINBB) {

		// We can cut
		std::vector<SubprocessFacet*> lList(nbLeft);
		std::vector<SubprocessFacet*> rList(nbRight);
		switch (planeType) {

		case 1: // yz
			m = (newNode->bb.min.x + newNode->bb.max.x) / 2.0;
			for (const auto& f : newNode->facets) {
				if (f->facetRef->sh.center.x > m) rList[nbr++] = f;
				else                   lList[nbl++] = f;
			}
			break;

		case 2: // xz
			m = (newNode->bb.min.y + newNode->bb.max.y) / 2.0;
			for (const auto& f : newNode->facets) {
				if (f->facetRef->sh.center.y > m) rList[nbr++] = f;
				else                   lList[nbl++] = f;
			}
			break;

		case 3: // xy
			m = (newNode->bb.min.z + newNode->bb.max.z) / 2.0;
			for (const auto& f : newNode->facets) {
				if (f->facetRef->sh.center.z > m) rList[nbr++] = f;
				else                   lList[nbl++] = f;
			}
			break;

		}
		newNode->left = BuildAABBTree(lList, depth + 1, maxDepth);
		newNode->right = BuildAABBTree(rList, depth + 1, maxDepth);
	}

	return newNode;

}

bool IntersectBB_new(const AABBNODE& node,const Vector3d& rayPos,const bool& nullRx,const bool& nullRy,const bool& nullRz,const Vector3d& inverseRayDir) {
	double tNear, tFar;
	//X component
	if (nullRx) {
		if (rayPos.x < node.bb.min.x || rayPos.x > node.bb.max.x) {
			return false;
		} 
		else {
			tNear = -1e100;
			tFar = 1e100;
		}
	}
	else {
		double intersection1 = (node.bb.min.x - rayPos.x) * inverseRayDir.x;
		double intersection2 = (node.bb.max.x - rayPos.x) * inverseRayDir.x;
		tNear = std::min(intersection1, intersection2);
		tFar = std::max(intersection1, intersection2);
		if (tFar < 0.0) return false;
	}

	//Y component
	if (nullRy) {
		if (rayPos.y < node.bb.min.y || rayPos.y > node.bb.max.y) return false;
	}
	else {
		double intersection1 = (node.bb.min.y - rayPos.y) * inverseRayDir.y;
		double intersection2 = (node.bb.max.y - rayPos.y) * inverseRayDir.y;
		tNear = std::max(tNear, std::min(intersection1, intersection2));
		tFar = std::min(tFar, std::max(intersection1, intersection2));
		if (tNear>tFar || tFar<0.0) return false;
	}

	//Z component
	if (nullRz) {
			if (rayPos.z < node.bb.min.z || rayPos.z > node.bb.max.z) return false;   
	}
	else {
		double intersection1 = (node.bb.min.z - rayPos.z) * inverseRayDir.z;
		double intersection2 = (node.bb.max.z - rayPos.z) * inverseRayDir.z;
		tNear = std::max(tNear, std::min(intersection1, intersection2));
		tFar = std::min(tFar, std::max(intersection1, intersection2));
		if (tNear>tFar || tFar<0.0) return false;
	}
	return true;
}

/*
// Ray AABB intersection check (slabs method)
#define IntersectBB(n,lab)                                                   \
	\
	AABBHit = false;                                                           \
	\
	if( nullRx ) {                                                             \
	if( rayPos->x < (n)->bb.min.x || rayPos->x > (n)->bb.max.x ) goto lab;   \
	tNear =  -1e100;                                                         \
	tFar  =   1e100;                                                         \
	} else {                                                                   \
	it1 = ((n)->bb.min.x - rayPos->x)*iRx;                                   \
	it2 = ((n)->bb.max.x - rayPos->x)*iRx;                                   \
	tNear = std::min(it1,it2);												\
	tFar  = std::max(it1,it2);                                              \
	}                                                                          \
	if( tFar<0.0 ) goto lab;                                                   \
	\
	if( nullRy ) {                                                             \
	if( rayPos->y < (n)->bb.min.y || rayPos->y > (n)->bb.max.y ) goto lab;   \
	} else {                                                                   \
	it1 = ((n)->bb.min.y - rayPos->y)*iRy;                                   \
	it2 = ((n)->bb.max.y - rayPos->y)*iRy;                                   \
	tNear = std::max(tNear, std::min(it1,it2));								\
	tFar  = std::min(tFar , std::max(it1,it2));								\                                                                      
	}                                                                          \
	if( tNear>tFar || tFar<0.0 ) goto lab;                                     \
	if( nullRz ) {                                                             \
	if( rayPos->z < (n)->bb.min.z || rayPos->z > (n)->bb.max.z ) goto lab;   \
	}
	else { \
			it1 = ((n)->bb.min.y - rayPos->y)*iRy;                                   \
			it2 = ((n)->bb.max.y - rayPos->y)*iRy;                                   \
			tNear = std::max(tNear, std::min(it1, it2));								\
			tFar = std::min(tFar, std::max(it1, it2));								\
	}                                                                         \
	if( tNear>tFar || tFar<0.0 ) goto lab;                                     \
	AABBHit = true;                                                            \
lab:

*/


//Unused as of 2017/09/25
bool RaySphereIntersect(Vector3d *center, double radius, Vector3d *rPos, Vector3d *rDir, double *dist) {

	// Perform ray-sphere intersection
	double B, C, D;
	Vector3d s;
	s.x = (rPos->x - center->x);
	s.y = (rPos->y - center->y);
	s.z = (rPos->z - center->z);

	//|rDir|=1 => A=1
	//A = DOT3(rDir->x,rDir->y,rDir->z,rDir->x,rDir->y,rDir->z);
	B = 2.0 * Dot(*rDir, s);
	C = Dot(s, s) - radius*radius;
	D = B*B - 4 * C;

	if (D >= 0.0) {
		double rD = sqrt(D);
		double t0 = (-B - rD) / 2.0;
		double t1 = (-B + rD) / 2.0;
		*dist = Min(t0, t1);
		return (*dist >= 0.0);
	}

	return false;

}


/*std::tuple<bool,SubprocessFacet*,double>*/ void IntersectTree(Simulation* sHandle, const AABBNODE& node, const Vector3d& rayPos, const Vector3d& rayDirOpposite, SubprocessFacet* const lastHitBefore,
	const bool& nullRx, const bool& nullRy, const bool& nullRz, const Vector3d& inverseRayDir,
	/*std::vector<SubprocessFacet*>& transparentHitFacetPointers,*/ bool& found, SubprocessFacet*& collidedFacet, double& minLength) {

	// Returns three values
	// bool: did collision occur?
	// SubprocessFacet* : closest collided facet
	// double: minimum distance

	// Method: 3x3 Sytem solving for ray/rectangle intersection. 
	// Solve the vector equation u*U + v*V + d*D = Z (using Cramer's rule)
	// nuv = u^v (for faster calculation)

	/*bool found = false;
	SubprocessFacet* collidedFacet = lastHitBefore;
	double minLength=minLengthSoFar;*/

	if (node.left == NULL || node.right == NULL) { // Leaf

		for (const auto& f : node.facets) {

			// Do not check last collided facet
			if (f == lastHitBefore)
				continue;

			double det = Dot(f->facetRef->sh.Nuv, rayDirOpposite);
			// Eliminate "back facet"
			if ((f->facetRef->sh.is2sided) || (det > 0.0)) { //If 2-sided or if ray going opposite facet normal

				double u, v, d;
				// Ray/rectangle instersection. Find (u,v,dist) and check 0<=u<=1, 0<=v<=1, dist>=0
				
				if (det != 0.0) {

					double iDet = 1.0 / det;
					Vector3d intZ = rayPos - f->facetRef->sh.O;

					u = iDet * DET33(intZ.x, f->facetRef->sh.V.x, rayDirOpposite.x,
						intZ.y, f->facetRef->sh.V.y, rayDirOpposite.y,
						intZ.z, f->facetRef->sh.V.z, rayDirOpposite.z);

					if (u >= 0.0 && u <= 1.0) {

						v = iDet * DET33(f->facetRef->sh.U.x, intZ.x, rayDirOpposite.x,
							f->facetRef->sh.U.y, intZ.y, rayDirOpposite.y,
							f->facetRef->sh.U.z, intZ.z, rayDirOpposite.z);

						if (v >= 0.0 && v <= 1.0) {

							d = iDet * Dot(f->facetRef->sh.Nuv, intZ);

							if (d>0.0) {

								// Now check intersection with the facet polygon (in the u,v space)
								// This check could be avoided on rectangular facet.
								if (IsInFacet(*f, u, v)) {
									bool hardHit;
#ifdef MOLFLOW
									double time = sHandle->currentParticle.flightTime + d / 100.0 / sHandle->currentParticle.velocity;
									double currentOpacity = sHandle->GetOpacityAt(f, time);
									hardHit = ((currentOpacity == 1.0) || (sHandle->randomGenerator.rnd()<currentOpacity));
#endif

#ifdef SYNRAD
									hardHit = !((f->facetRef->sh.opacity < 0.999999 //Partially transparent facet
										&& rnd()>f->facetRef->sh.opacity)
										|| (f->facetRef->sh.reflectType > 10 //Material reflection
										&& sHandle->materials[f->facetRef->sh.reflectType - 10].hasBackscattering //Has complex scattering
										&& sHandle->materials[f->facetRef->sh.reflectType - 10].GetReflectionType(sHandle->energy,
										acos(Dot(sHandle->direction, f->facetRef->sh.N)) - PI / 2, rnd()) == REFL_TRANS));
#endif
									if (hardHit) {

										// Hard hit
										if (d < minLength) {
											minLength = d;
											collidedFacet = f;
											found = true;
											sHandle->myTmpFacetVars[f->globalId].colU = u;
											sHandle->myTmpFacetVars[f->globalId].colV = v;
										}
									}
									else {
										sHandle->myTmpFacetVars[f->globalId].colDistTranspPass = d;
										sHandle->myTmpFacetVars[f->globalId].colU = u;
										sHandle->myTmpFacetVars[f->globalId].colV = v;
										sHandle->currentParticle.transparentHitBuffer.push_back(f);
									}
								} // IsInFacet
							} // d range
						} // u range
					} // v range
				} // det==0
			} // dot<0
		} // end for

	} /* end Leaf */ else {

		if (IntersectBB_new(*(node.left), rayPos, nullRx, nullRy, nullRz, inverseRayDir)) {
			IntersectTree(sHandle, *(node.left), rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz, inverseRayDir, /*transparentHitFacetPointers,*/ found, collidedFacet, minLength);
		}
		if (IntersectBB_new(*(node.right), rayPos, nullRx, nullRy, nullRz, inverseRayDir)) {
			IntersectTree(sHandle, *(node.right), rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz, inverseRayDir, /*transparentHitFacetPointers,*/ found, collidedFacet, minLength);
		}
	}
}

bool IsInFacet(const SubprocessFacet &f, const double &u, const double &v) {

	/*

	// 2D polygon "is inside" solving
	// Using the "Jordan curve theorem" (we intersect in v direction here)

	int n_updown, n_found, j;
	double x1, x2, y1, y2, a, minx, maxx;

	n_updown = 0;
	n_found = 0;

	for (j = 0; j < f.indices.size() - 1; j++) {

		x1 = f.vertices2[j].u;
		y1 = f.vertices2[j].v;
		x2 = f.vertices2[j + 1].u;
		y2 = f.vertices2[j + 1].v;

		minx = std::min(x1, x2);
		maxx = std::max(x1, x2);

		if (u > minx && u <= maxx) {
			a = (y2 - y1) / (x2 - x1);
			if ((a*(u - x1) + y1) < v) {
				n_updown++;
			}
			else {
				n_updown--;
			}
			n_found++;
		}

	}

	// Last point
	x1 = f.vertices2[j].u;
	y1 = f.vertices2[j].v;
	x2 = f.vertices2[0].u;
	y2 = f.vertices2[0].v;

	minx = std::min(x1, x2);
	maxx = std::max(x1, x2);

	if (u > minx && u <= maxx) {
		a = (y2 - y1) / (x2 - x1);
		if ((a*(u - x1) + y1) < v) {
			n_updown++;
		}
		else {
			n_updown--;
		}
		n_found++;
	}

	if (n_updown<0) n_updown = -n_updown;
	return (((n_found / 2) & 1) ^ ((n_updown / 2) & 1));
	*/

	return IsInPoly(Vector2d(u, v), f.facetRef->vertices2);

}

std::tuple<bool, SubprocessFacet*, double> Intersect(Simulation* sHandle, const std::vector<SubProcessSuperStructure>& structures, const Vector3d& rayPos, const Vector3d& rayDir) {
	// Source ray (rayDir vector must be normalized)
	// lastHit is to avoid detecting twice the same collision
	// returns bool found (is there a collision), pointer to collided facet, double d (distance to collision)

	bool nullRx = (rayDir.x == 0.0);
	bool nullRy = (rayDir.y == 0.0);
	bool nullRz = (rayDir.z == 0.0);
	Vector3d inverseRayDir;
	if (!nullRx) inverseRayDir.x = 1.0 / rayDir.x;
	if (!nullRy) inverseRayDir.y = 1.0 / rayDir.y;
	if (!nullRz) inverseRayDir.z = 1.0 / rayDir.z;

	//Global variables, easier for recursion:
	size_t intNbTHits = 0;

	//Output values
	bool found = false;
	SubprocessFacet *collidedFacet = NULL;
	sHandle->currentParticle.transparentHitBuffer.clear();
	double minLength = 1e100;

	IntersectTree(sHandle, *structures[sHandle->currentParticle.structureId].aabbTree, rayPos, -1.0*rayDir, sHandle->currentParticle.lastHitFacet,
		nullRx, nullRy, nullRz, inverseRayDir,
		/*transparentHitFacetPointers,*/ found, collidedFacet, minLength); //output params

	if (found) {

		sHandle->myTmpFacetVars[collidedFacet->globalId].hitted = true;

		// Second pass for transparent hits
		for (const auto& tpFacet: sHandle->currentParticle.transparentHitBuffer){
			if (sHandle->myTmpFacetVars[tpFacet->globalId].colDistTranspPass < minLength) {
				sHandle->RegisterTransparentPass(tpFacet);
			}
		}

		/*
		// Compute intersection with spheric volume element
		if (sHandle->hasDirection) {

		for (j = 0; j < sHandle->sh.nbSuper; j++) {
		for (i = 0; i < sHandle->structures[j].nbFacet; i++) {
		f = sHandle->structures[j].facets[i];
		if (f->direction && f->facetRef->sh.countDirection) {

		int      x, y;
		Vector3d center;
		double   d;
		double   r = f->rw*0.45; // rw/2 - 10% (avoid side FX)

		for (x = 0; x < f->facetRef->sh.texWidth; x++) {
		for (y = 0; y < f->facetRef->sh.texHeight; y++) {
		int add = x + y*f->facetRef->sh.texWidth;
		if (isFull) {

		double uC = ((double)x + 0.5) * f->iw;
		double vC = ((double)y + 0.5) * f->ih;
		center.x = f->facetRef->sh.O.x + f->facetRef->sh.U.x*uC + f->facetRef->sh.V.x*vC;
		center.y = f->facetRef->sh.O.y + f->facetRef->sh.U.y*uC + f->facetRef->sh.V.y*vC;
		center.z = f->facetRef->sh.O.z + f->facetRef->sh.U.z*uC + f->facetRef->sh.V.z*vC;
		if (RaySphereIntersect(&center, r, rPos, rDir, &d)) {
		if (d < intMinLgth) {
		f->direction[add].dir.x += sHandle->currentParticle.direction.x;
		f->direction[add].dir.y += sHandle->currentParticle.direction.y;
		f->direction[add].dir.z += sHandle->currentParticle.direction.z;
		f->direction[add].count++;
		}
		}

		}
		}
		}
		}
		}
		}
		}*/

	}
	return { found, collidedFacet, minLength };

}

Vector3d PolarToCartesian(SubprocessFacet* const collidedFacet, const double& theta, const double& phi, const bool& reverse) {

	//returns sHandle->currentParticle.direction

	//Vector3d U, V, N;
	//double u, v, n;

	// Polar in (nU,nV,N) to Cartesian(x,y,z) transformation  ( nU = U/|U| , nV = V/|V| )
	// tetha is the angle to the normal of the facet N, phi to U
	// ! See Geometry::InitializeGeometry() for further informations on the (U,V,N) basis !
	// (nU,nV,N) and (x,y,z) are both left handed

	/*#ifdef _WIN32
	_asm {                    // FPU stack
	fld qword ptr [theta]
	fsincos                 // cos(t)        sin(t)
	fld qword ptr [phi]
	fsincos                 // cos(p)        sin(p) cos(t) sin(t)
	fmul st(0),st(3)        // cos(p)*sin(t) sin(p) cos(t) sin(t)
	fstp qword ptr [u]      // sin(p)        cos(t) sin(t)
	fmul st(0),st(2)        // sin(p)*sin(t) cos(t) sin(t)
	fstp qword ptr [v]      // cos(t) sin(t)
	fstp qword ptr [n]      // sin(t)
	fstp qword ptr [dummy]  // Flush the sin(t)
	}
	#else*/
	double u = sin(theta)*cos(phi);
	double v = sin(theta)*sin(phi);
	double n = cos(theta);
	//#endif

	// Get the (nU,nV,N) orthonormal basis of the facet
	Vector3d U = collidedFacet->facetRef->sh.nU;
	Vector3d V = collidedFacet->facetRef->sh.nV;
	Vector3d N = collidedFacet->facetRef->sh.N;
	if (reverse) {
		N = -1.0 * N;
	}
	// Basis change (nU,nV,N) -> (x,y,z)
	return u*U + v*V + n*N;
}

std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN) {

	//input vectors need to be normalized

	// Get polar coordinates of the incoming particule direction in the (U,V,N) facet space.
	// Note: The facet is parallel to (U,V), we use its (nU,nV,N) orthonormal basis here.
	// (nU,nV,N) and (x,y,z) are both left handed

	// Cartesian(x,y,z) to polar in (nU,nV,N) transformation

	// Basis change (x,y,z) -> (nU,nV,N)
	// We use the fact that (nU,nV,N) belongs to SO(3)
	double u = Dot(incidentDir, normU);
	double v = Dot(incidentDir, normV);
	double n = Dot(incidentDir, normN);
	Saturate(n, -1.0, 1.0); //sometimes rounding errors do occur, 'acos' function would return no value for theta

							// (u,v,n) -> (theta,phi)
	
	double inTheta = acos(n);              // Angle to normal (PI/2 => PI
	//double rho = sqrt(v*v + u*u);
	//double inPhi = asin(v / rho);     //At this point, -PI/2 < inPhi < PI/2
	//if (u < 0.0) inPhi = PI - inPhi;  // Angle to U
	double inPhi = atan2(v, u); //-PI .. PI, and the angle is 0 when pointing towards u
	return { inTheta, inPhi };
}

bool Visible(Simulation* sHandle, const std::vector<SubProcessSuperStructure>& structures, Vector3d *c1, Vector3d *c2, SubprocessFacet *f1, SubprocessFacet *f2) {
	//For AC matrix calculation, used only in MolFlow

	Vector3d rayPos = *c1;
	Vector3d rayDir = *c2 - *c1;

	bool nullRx = (rayDir.x == 0.0);
	bool nullRy = (rayDir.y == 0.0);
	bool nullRz = (rayDir.z == 0.0);
	Vector3d inverseRayDir;
	if (!nullRx) inverseRayDir.x = 1.0 / rayDir.x;
	if (!nullRy) inverseRayDir.y = 1.0 / rayDir.y;
	if (!nullRz) inverseRayDir.z = 1.0 / rayDir.z;

	//Global variables, easier for recursion:
	size_t intNbTHits = 0;

	//Output values
	bool found;
	SubprocessFacet *collidedFacet;
	double minLength;

	//std::vector<SubprocessFacet*> transparentHitFacetPointers;
	sHandle->currentParticle.transparentHitBuffer.clear();

	IntersectTree(sHandle, *structures[0].aabbTree, rayPos, -1.0*rayDir,
		f1, nullRx, nullRy, nullRz, inverseRayDir, /*transparentHitFacetPointers,*/ found, collidedFacet, minLength);

	if (found) {
		if (collidedFacet != f2) {
			// Obstacle found
			return false;
		}
	}

	return true;
}

AABBNODE::AABBNODE()
{
	left = right = NULL;
}

AABBNODE::~AABBNODE()
{
	SAFE_DELETE(left);
	SAFE_DELETE(right);
}
