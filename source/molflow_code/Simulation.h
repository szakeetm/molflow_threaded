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

#include "MolflowTypes.h"
#include "Buffer_shared.h" //Facetproperties
#include "Facet_shared.h"
#include "SMP.h"
#include <vector>
#include "Vector.h"
#include "Parameter.h"
#include <tuple>
#include "Random.h"

#define WAITTIME    100

class GeneratingAnglemap {
public:
	std::vector<size_t>   pdf;		  // Incident angle distribution, phi and theta, not normalized. Used either for recording or for 2nd order interpolation
	std::vector<double>   phi_CDFs;    // A table containing phi distributions for each theta, starting from 0 for every line (1 line = 1 theta value). For speed we keep it in one memory block, 1 pointer
	std::vector<size_t>   phi_CDFsums; // since CDF runs only to the middle of the last segment, for each theta a line sum is stored here. Also a pdf for theta
	std::vector<double>   theta_CDF;	  // Theta CDF, not normalized. nth value is the CDF at the end of region n (beginning of first section is always 0)
	size_t   theta_CDFsum=0; // since theta CDF only runs till the middle of the last segment, the map sum is here

	double GetTheta(const double& thetaIndex, const AnglemapParams& anglemapParams);
	double GetPhi(const double& phiIndex, const AnglemapParams& anglemapParams);
	double GetPhiPdfValue(const double & thetaIndex, const int & phiIndex, const AnglemapParams & anglemapParams);
	double GetPhiCDFValue(const double& thetaIndex, const int& phiIndex, const AnglemapParams& anglemapParams);
	double GetPhiCDFSum(const double & thetaIndex, const AnglemapParams & anglemapParams);
	std::tuple<double, int, double> GenerateThetaFromAngleMap(const AnglemapParams& anglemapParams, MersenneTwister& randomGenerator);
	double GeneratePhiFromAngleMap(const int& thetaLowerIndex, const double& thetaOvershoot, const AnglemapParams& anglemapParams, MersenneTwister& randomGenerator);
};

class Simulation; //Fwd declaration

class SubProcessFacetTempVar {
public:
	// Temporary var (used in Intersect for collision)
	double colDistTranspPass=1E99;
	double colU;
	double colV;
	bool   hitted=false;
};

// Local facet structure
class SubprocessFacet {
public:
	std::vector<double>   textureCellIncrements;              // Texture increment
	std::vector<bool>     largeEnough;      // cells that are NOT too small for autoscaling
	double   fullSizeInc;
	std::vector<double>   outgassingMapCdf; // Cumulative outgassing map when desorption is based on imported file
	double outgassingMapWidthD, outgassingMapHeightD; //Actual width values for faster generation
	GeneratingAnglemap generatingAngleMap;

	// Precalc texture values
	double iw; // 1/textWidthD (1 / actual number of texture cells)
	double ih; // 1/textHeightD (1 / actual number of texture cells)
	double rw; //U.length / iw
	double rh; //V.length / ih	

	size_t globalId; //Global index (to identify when superstructures are present)
	Facet* facetRef = NULL; //Reference to interface facet
	
	void InitializeOnLoad(size_t nbStruct); //Throws exception

	bool InitializeTexture();

	void InitializeAngleMap();

	void InitializeOutgassingMap();

	void InitializeLinkAndVolatile(size_t nbStruct);

};

// Local simulation structure

class AABBNODE;

class SubProcessSuperStructure {
public:
	~SubProcessSuperStructure();
	std::vector<SubprocessFacet>  facets;   // Facet handles
	AABBNODE* aabbTree = NULL; // Structure AABB tree
};

class CurrentParticleStatus {
public:
	Vector3d position;    // Position
	Vector3d direction;    // Direction
	double oriRatio; //Represented ratio of desorbed, used for low flux mode

	//Recordings for histogram
	size_t   nbBounces; // Number of hit (current particle) since desorption
	double   distanceTraveled;
	double   flightTime;

	double   velocity;
	double   expectedDecayMoment; //for radioactive gases
	size_t   structureId;        // Current structure
	int      teleportedFrom;   // We memorize where the particle came from: we can teleport back
	SubprocessFacet *lastHitFacet = NULL;     // Last hitted facet
	std::vector<SubprocessFacet*> transparentHitBuffer; //Storing this buffer simulation-wide is cheaper than recreating it at every Intersect() call
};

class Worker;
class Geometry;

class Simulation { //One per subprocess
public:
	Simulation(Worker* w);

	int prIdx;
	size_t prState;
	size_t prParam;
	size_t prParam2;
	bool end = false;

	MersenneTwister randomGenerator;
	
	std::vector<ParticleLoggerItem> tmpParticleLog; //Recorded particle log since last UpdateMCHits
	size_t myLogTarget = 0;
	GlobalSimuState myTmpResults; //Results recorded since last UpdateMcHits (doesn't include log which is independent)
	std::vector<SubProcessFacetTempVar> myTmpFacetVars; //One per subprocessfacet, for intersect routine
	size_t totalDesorbed = 0;           // Total number of desorptions (for this process, not reset on UpdateMCHits)

	// Geometry
	Worker* worker;
	Geometry* geom;

	OntheflySimulationParams myOtfp; //Copy of worker parameters, to make sure it's updated only on request

	double stepPerSec=0.0;  // Avg number of step per sec
	bool loadOK = false;        // Load OK flag
	bool lastHitUpdateOK;  // Last hit update timeout

	CurrentParticleStatus currentParticle;

	//Control related
	void RecordHitOnTexture(SubprocessFacet *f, double time, bool countHit, double velocity_factor, double ortSpeedFactor);
	void RecordDirectionVector(SubprocessFacet *f, double time);
	void ProfileFacet(SubprocessFacet *f, double time, bool countHit, double velocity_factor, double ortSpeedFactor);
	void LogHit(SubprocessFacet *f);
	void RecordAngleMap(SubprocessFacet* collidedFacet);
	void ClearSimulation();
	void CopyMyStateFromControl();
	size_t GetMyState();
	void CopyStateFromMaster();
	void SetLocalAndMasterState(size_t state, const std::string & status, bool changeState = true, bool changeStatus = true);
	void SetErrorSub(const std::string& msg);
	std::string GetMyStatusAsText();
	void SetReady();
	void SetStatusStringAtMaster(const std::string& status);
	void ResizeTmpLog();
	void ConstructFacetTmpVars();
	int mainLoop(int index);
	bool LoadSimulation();
	bool StartSimulation();
	void ResetSimulation();
	bool SimulationRun();
	bool SimulationMCStep(size_t nbStep);
	void IncreaseDistanceCounters(double d);
	bool StartFromSource();
	void PerformBounce(SubprocessFacet *iFacet);
	void RecordAbsorb(SubprocessFacet *iFacet);
	void RecordHistograms(SubprocessFacet * iFacet);
	void PerformTeleport(SubprocessFacet *iFacet);
	void PerformTransparentPass(SubprocessFacet *iFacet);
	void UpdateLog(size_t timeout);
	void UpdateMCHits(size_t timeout);

	//Simulation related
	void RegisterTransparentPass(SubprocessFacet* f);
	int GetIDId(int paramId);
	void   UpdateVelocity(SubprocessFacet *collidedFacet);
	double GenerateRandomVelocity(int CDFId);
	double GenerateDesorptionTime(SubprocessFacet* src);
	double GetStickingAt(SubprocessFacet *src, double time);
	double GetOpacityAt(SubprocessFacet *src, double time);
	void   IncreaseFacetCounter(SubprocessFacet *f, double time, size_t hit, size_t desorb, size_t absorb, double sum_1_per_v, double sum_v_ort);
	void   TreatMovingFacet();
	void RecordHit(const int& type);
	void RecordLeakPos();
};
