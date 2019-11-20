/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
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

// Temporary transparent hit
#include "Vector.h" //AABB
#include "Simulation.h" //SubprocessFacet

// AABBTree node

class AABBNODE {
public:
	AABBNODE();
	~AABBNODE();
	void ComputeBB();
	std::tuple<size_t, size_t, size_t> FindBestCuttingPlane();
	AxisAlignedBoundingBox             bb;
	AABBNODE *left;
	AABBNODE *right;
	std::vector<SubprocessFacet*> facets;

};

AABBNODE *BuildAABBTree(const std::vector<SubprocessFacet*>& facets,const size_t depth,size_t& maxDepth);

void IntersectTree(Simulation* sHandle, const AABBNODE& node, const Vector3d& rayPos, const Vector3d& rayDirOpposite, SubprocessFacet* const lastHitBefore,
	const bool& nullRx, const bool& nullRy, const bool& nullRz, const Vector3d& inverseRayDir,
	/*std::vector<SubprocessFacet*>& transparentHitFacetPointers,*/ bool& found, SubprocessFacet*& collidedFacet, double& minLength);
std::tuple<bool, SubprocessFacet*, double> Intersect(Simulation* sHandle, const std::vector<SubProcessSuperStructure>& structures, const Vector3d& rayPos, const Vector3d& rayDir);
bool Visible(Simulation* sHandle, const std::vector<SubProcessSuperStructure>& structures, Vector3d *c1,Vector3d *c2,SubprocessFacet *f1,SubprocessFacet *f2);
bool IsInFacet(const SubprocessFacet &f,const double &u,const double &v);
Vector3d PolarToCartesian(SubprocessFacet* const collidedFacet, const double& theta, const double& phi, const bool& reverse); //sets sHandle->currentParticle.direction
std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN);