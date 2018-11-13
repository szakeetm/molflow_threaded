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
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "Simulation.h"
#include "IntersectAABB_shared.h"
#include "Random.h"
#include "GLApp/MathTools.h"
#include <tuple> //std::tie
#include "Worker.h"
#include "MolflowGeometry.h"


// Compute area of all the desorption facet
/*
void Simulation::CalcTotalOutgassing() {
	//float scale_precomputed;

	// Update texture increment for MC
	//scale_precomputed=(float)(40.0/(sqrt(8.0*8.31/(PI*worker->wp.gasMass*0.001))));
	for (size_t j = 0; j < structures.size(); j++) {
		for (SubprocessFacet& f : structures[j].facets) {
			if (f.sh.is2sided) {
				f.fullSizeInc *= 0.5;
				for (auto& inc : f.textureCellIncrements)
					inc *= 0.5;
			}
		}
	}

}
*/

//void PolarToCartesian(SubprocessFacet *iFacet, double theta, double phi, bool reverse) {
//
//	Vector3d U, V, N;
//	double u, v, n;
//
//	// Polar in (nU,nV,N) to Cartesian(x,y,z) transformation  ( nU = U/|U| , nV = V/|V| )
//	// tetha is the angle to the normal of the facet N, phi to U
//	// ! See Geometry::InitializeGeometry() for further informations on the (U,V,N) basis !
//	// (nU,nV,N) and (x,y,z) are both left handed
//
//	//This should be a speed-up routine, but I didn't experience any speed difference so I commented it out. Marton
//	/*#ifdef _WIN32
//	_asm {                    // FPU stack
//	fld qword ptr [theta]
//	fsincos                 // cos(t)        sin(t)
//	fld qword ptr [phi]
//	fsincos                 // cos(p)        sin(p) cos(t) sin(t)
//	fmul st(0),st(3)        // cos(p)*sin(t) sin(p) cos(t) sin(t)
//	fstp qword ptr [u]      // sin(p)        cos(t) sin(t)
//	fmul st(0),st(2)        // sin(p)*sin(t) cos(t) sin(t)
//	fstp qword ptr [v]      // cos(t) sin(t)
//	fstp qword ptr [n]      // sin(t)
//	fstp qword ptr [dummy]  // Flush the sin(t)
//	}
//	#else*/
//	u = sin(theta)*cos(phi);
//	v = sin(theta)*sin(phi);
//	n = cos(theta);
//	//#endif
//
//	// Get the (nU,nV,N) orthonormal basis of the facet
//	U = iFacet->facetRef->sh.nU;
//	V = iFacet->facetRef->sh.nV;
//	N = iFacet->facetRef->sh.N;
//	if (reverse) {
//		N.x = N.x*(-1.0);
//		N.y = N.y*(-1.0);
//		N.z = N.z*(-1.0);
//	}
//
//	// Basis change (nU,nV,N) -> (x,y,z)
//	currentParticle.direction.x = u*U.x + v*V.x + n*N.x;
//	currentParticle.direction.y = u*U.y + v*V.y + n*N.y;
//	currentParticle.direction.z = u*U.z + v*V.z + n*N.z;
//
//}
//
//void CartesianToPolar(SubprocessFacet *iFacet, double *theta, double *phi) {
//
//	// Get polar coordinates of the incoming particule direction in the (U,V,N) facet space.
//	// Note: The facet is parallel to (U,V), we use its (nU,nV,N) orthonormal basis here.
//	// (nU,nV,N) and (x,y,z) are both left handed
//
//	// Cartesian(x,y,z) to polar in (nU,nV,N) transformation
//
//	// Basis change (x,y,z) -> (nU,nV,N)
//	// We use the fact that (nU,nV,N) belongs to SO(3)
//	double u = Dot(currentParticle.direction, iFacet->facetRef->sh.nU);
//	double v = Dot(currentParticle.direction, iFacet->facetRef->sh.nV);
//	double n = Dot(currentParticle.direction, iFacet->facetRef->sh.N);
//
//	/*
//	// (u,v,n) -> (theta,phi)
//	double rho = sqrt(v*v + u*u);
//	*theta = acos(n);              // Angle to normal (PI/2 .. PI for frontal and 0..PI/2 for back incidence)
//	*phi = asin(v / rho);			// Returns -PI/2 ... +PI/2
//	if (u < 0.0) *phi = PI - *phi;  // Angle to U, -PI/2 .. 3PI/2
//	*/
//
//	*theta = acos(n);
//	*phi = atan2(v, u); // -PI..PI
//}

void Simulation::UpdateMCHits(size_t timeout) {

	SetLocalAndMasterState(0, "Waiting for 'hits' dataport access...", false, true);
	lastHitUpdateOK = LockMutex(worker->results.mutex, timeout);
	SetLocalAndMasterState(0, "Updating MC hits...", false, true);
	if (!lastHitUpdateOK) return; //Timeout, will try again later

	// Global hits and leaks: adding local hits to shared memory
	worker->results.globalHits.globalHits += myTmpResults.globalHits.globalHits;
	//gHits->globalHits.nbMCHit += myTmpResults.globalHits.globalHits.nbMCHit;
	//gHits->globalHits.nbHitEquiv += myTmpResults.globalHits.globalHits.nbHitEquiv;
	//gHits->globalHits.nbAbsEquiv += myTmpResults.globalHits.globalHits.nbAbsEquiv;
	//gHits->globalHits.nbDesorbed += myTmpResults.globalHits.globalHits.nbDesorbed;
	worker->results.globalHits.distTraveled_total += myTmpResults.globalHits.distTraveled_total;
	worker->results.globalHits.distTraveledTotal_fullHitsOnly += myTmpResults.globalHits.distTraveledTotal_fullHitsOnly;

	//Memorize current limits, then do a min/max search
	TEXTURE_MIN_MAX texture_limits_old[3];
	for (size_t i = 0; i < 3; i++) {
		texture_limits_old[i] = worker->results.globalHits.texture_limits[i];
		worker->results.globalHits.texture_limits[i].min.all = worker->results.globalHits.texture_limits[i].min.moments_only = HITMAX;
		worker->results.globalHits.texture_limits[i].max.all = worker->results.globalHits.texture_limits[i].max.moments_only = 0;
	}

	// Leak
	for (size_t leakIndex = 0; leakIndex < myTmpResults.globalHits.leakCacheSize; leakIndex++)
		worker->results.globalHits.leakCache[(leakIndex + worker->results.globalHits.lastLeakIndex) % LEAKCACHESIZE] = myTmpResults.globalHits.leakCache[leakIndex];
	worker->results.globalHits.nbLeakTotal += myTmpResults.globalHits.nbLeakTotal;
	worker->results.globalHits.lastLeakIndex = (worker->results.globalHits.lastLeakIndex + myTmpResults.globalHits.leakCacheSize) % LEAKCACHESIZE;
	worker->results.globalHits.leakCacheSize = Min(LEAKCACHESIZE, worker->results.globalHits.leakCacheSize + myTmpResults.globalHits.leakCacheSize);

	// HHit (Only prIdx 0)
	if (prIdx == 0) {
		for (size_t hitIndex = 0; hitIndex < myTmpResults.globalHits.hitCacheSize; hitIndex++)
			worker->results.globalHits.hitCache[(hitIndex + worker->results.globalHits.lastHitIndex) % HITCACHESIZE] = myTmpResults.globalHits.hitCache[hitIndex];

		if (myTmpResults.globalHits.hitCacheSize > 0) {
			worker->results.globalHits.lastHitIndex = (worker->results.globalHits.lastHitIndex + myTmpResults.globalHits.hitCacheSize) % HITCACHESIZE;
			worker->results.globalHits.hitCache[worker->results.globalHits.lastHitIndex].type = HIT_LAST; //Penup (border between blocks of consecutive hits in the hit cache)
			worker->results.globalHits.hitCacheSize = Min(HITCACHESIZE, worker->results.globalHits.hitCacheSize + myTmpResults.globalHits.hitCacheSize);
		}
	}

	//Global histograms
	worker->results.globalHistograms += myTmpResults.globalHistograms;
	/*
		for (int m = 0; m < (1 + worker->moments.size()); m++) {
			BYTE *histCurrentMoment = buffer + sizeof(GlobalHitBuffer) + m * worker->wp.globalHistogramParams.GetDataSize();
			if (worker->wp.globalHistogramParams.recordBounce) {
				double* nbHitsHistogram = (double*)histCurrentMoment;
				for (size_t i = 0; i < worker->wp.globalHistogramParams.GetBounceHistogramSize(); i++) {
					nbHitsHistogram[i] += sHandle->tmpGlobalHistograms[m].nbHitsHistogram[i];
				}
			}
			if (worker->wp.globalHistogramParams.recordDistance) {
				double* distanceHistogram = (double*)(histCurrentMoment + worker->wp.globalHistogramParams.GetBouncesDataSize());
				for (size_t i = 0; i < (worker->wp.globalHistogramParams.GetDistanceHistogramSize()); i++) {
					distanceHistogram[i] += sHandle->tmpGlobalHistograms[m].distanceHistogram[i];
				}
			}
			if (worker->wp.globalHistogramParams.recordTime) {
				double* timeHistogram = (double*)(histCurrentMoment + worker->wp.globalHistogramParams.GetBouncesDataSize() + worker->wp.globalHistogramParams.GetDistanceDataSize());
				for (size_t i = 0; i < (worker->wp.globalHistogramParams.GetTimeHistogramSize()); i++) {
					timeHistogram[i] += sHandle->tmpGlobalHistograms[m].timeHistogram[i];
				}
			}
		}
		*/

	worker->results.facetStates += myTmpResults.facetStates;

	//Manual texture min/max search
	for (auto& s : worker->subprocessStructures) {
		for (auto& f : s.facets) {
			if (myTmpFacetVars[f.globalId].hitted) {
				if (f.facetRef->sh.isTextured) {
					for (int m = 0; m < (1 + worker->moments.size()); m++) {
						double timeCorrection = m == 0 ? worker->wp.finalOutgassingRate : (worker->wp.totalDesorbedMolecules) / worker->wp.timeWindowSize;
						//Timecorrection is required to compare constant flow texture values with moment values (for autoscaling)

						const auto& texture = worker->results.facetStates[f.globalId].momentResults[m].texture;
						size_t textureSize = texture.size();

						for (size_t t = 0; t < textureSize; t++) {
							double val[3];  //pre-calculated autoscaling values (Pressure, imp.rate, density)

							val[0] = texture[t].sum_v_ort_per_area*timeCorrection; //pressure without dCoef_pressure
							val[1] = texture[t].countEquiv*f.textureCellIncrements[t] * timeCorrection; //imp.rate without dCoef
							val[2] = f.textureCellIncrements[t] * texture[t].sum_1_per_ort_velocity* timeCorrection; //particle density without dCoef

						//Global autoscale
							for (int v = 0; v < 3; v++) {
								if (f.largeEnough[t])
									worker->results.globalHits.texture_limits[v].max.all = std::max(worker->results.globalHits.texture_limits[v].max.all, val[v]);

								if (val[v] > 0.0 && val[v] < worker->results.globalHits.texture_limits[v].min.all && f.largeEnough[t])
									worker->results.globalHits.texture_limits[v].min.all = val[v];

								//Autoscale ignoring constant flow (moments only)
								if (m != 0) {
									if (f.largeEnough[t])
										worker->results.globalHits.texture_limits[v].max.moments_only = std::max(worker->results.globalHits.texture_limits[v].max.moments_only, val[v]);

									if (val[v] > 0.0 && val[v] < worker->results.globalHits.texture_limits[v].min.moments_only && f.largeEnough[t])
										worker->results.globalHits.texture_limits[v].min.moments_only = val[v];
								}
							}
						}
					}
				}
			}
		}
	}

		/*
		size_t facetHitsSize = (1 + nbMoments) * sizeof(FacetHitBuffer);
		// Facets
		for (s = 0; s < structures.size(); s++) {
			for (SubprocessFacet& f : structures[s].facets) {
				if (f.hitted) {

					for (int m = 0; m < (1 + nbMoments); m++) {
						FacetHitBuffer *facetHitBuffer = (FacetHitBuffer *)(buffer + f.sh.hitOffset + m * sizeof(FacetHitBuffer));
						facetHitBuffer->nbAbsEquiv += f.tmpCounter[m].nbAbsEquiv;
						facetHitBuffer->nbDesorbed += f.tmpCounter[m].nbDesorbed;
						facetHitBuffer->nbMCHit += f.tmpCounter[m].nbMCHit;
						facetHitBuffer->nbHitEquiv += f.tmpCounter[m].nbHitEquiv;
						facetHitBuffer->sum_1_per_ort_velocity += f.tmpCounter[m].sum_1_per_ort_velocity;
						facetHitBuffer->sum_v_ort += f.tmpCounter[m].sum_v_ort;
						facetHitBuffer->sum_1_per_velocity += f.tmpCounter[m].sum_1_per_velocity;
					}

					if (f.sh.isProfile) {
						for (int m = 0; m < (1 + nbMoments); m++) {
							ProfileSlice *shProfile = (ProfileSlice *)(buffer + f.sh.hitOffset + facetHitsSize + m * f.profileSize);
							for (j = 0; j < PROFILE_SIZE; j++) {
								shProfile[j] += f.profile[m][j];
							}
						}
					}

					if (f.sh.isTextured) {
						for (int m = 0; m < (1 + nbMoments); m++) {
							TextureCell *shTexture = (TextureCell *)(buffer + (f.sh.hitOffset + facetHitsSize + f.profileSize*(1 + nbMoments) + m * f.textureSize));
							//double dCoef = gHits->globalHits.nbDesorbed * 1E4 * worker->wp.gasMass / 1000 / 6E23 * MAGIC_CORRECTION_FACTOR;  //1E4 is conversion from m2 to cm2
							double timeCorrection = m == 0 ? worker->wp.finalOutgassingRate : (worker->wp.totalDesorbedMolecules) / worker->wp.timeWindowSize;
							//Timecorrection is required to compare constant flow texture values with moment values (for autoscaling)

							for (y = 0; y < f.sh.texHeight; y++) {
								for (x = 0; x < f.sh.texWidth; x++) {
									size_t add = x + y * f.sh.texWidth;

									//Add temporary hit counts
									shTexture[add] += f.texture[m][add];

									double val[3];  //pre-calculated autoscaling values (Pressure, imp.rate, density)

									val[0] = shTexture[add].sum_v_ort_per_area*timeCorrection; //pressure without dCoef_pressure
									val[1] = shTexture[add].countEquiv*f.textureCellIncrements[add] * timeCorrection; //imp.rate without dCoef
									val[2] = f.textureCellIncrements[add] * shTexture[add].sum_1_per_ort_velocity* timeCorrection; //particle density without dCoef

									//Global autoscale
									for (int v = 0; v < 3; v++) {
										if (val[v] > gHits->texture_limits[v].max.all && f.largeEnough[add])
											gHits->texture_limits[v].max.all = val[v];

										if (val[v] > 0.0 && val[v] < gHits->texture_limits[v].min.all && f.largeEnough[add])
											gHits->texture_limits[v].min.all = val[v];

										//Autoscale ignoring constant flow (moments only)
										if (m != 0) {
											if (val[v] > gHits->texture_limits[v].max.moments_only && f.largeEnough[add])
												gHits->texture_limits[v].max.moments_only = val[v];

											if (val[v] > 0.0 && val[v] < gHits->texture_limits[v].min.moments_only && f.largeEnough[add])
												gHits->texture_limits[v].min.moments_only = val[v];
										}
									}
								}
							}
						}
					}

					if (f.sh.countDirection) {
						for (int m = 0; m < (1 + nbMoments); m++) {
							DirectionCell *shDir = (DirectionCell *)(buffer + (f.sh.hitOffset + facetHitsSize + f.profileSize*(1 + nbMoments) + f.textureSize*(1 + nbMoments) + f.directionSize*m));
							for (y = 0; y < f.sh.texHeight; y++) {
								for (x = 0; x < f.sh.texWidth; x++) {
									size_t add = x + y * f.sh.texWidth;
									shDir[add].dir.x += f.direction[m][add].dir.x;
									shDir[add].dir.y += f.direction[m][add].dir.y;
									shDir[add].dir.z += f.direction[m][add].dir.z;
									//shDir[add].sumSpeed += f.direction[m][add].sumSpeed;
									shDir[add].count += f.direction[m][add].count;
								}
							}
						}
					}

					if (f.sh.anglemapParams.record) {
						size_t *shAngleMap = (size_t *)(buffer + f.sh.hitOffset + facetHitsSize + f.profileSize*(1 + nbMoments) + f.textureSize*(1 + nbMoments) + f.directionSize*(1 + nbMoments));
						for (y = 0; y < (f.sh.anglemapParams.thetaLowerRes + f.sh.anglemapParams.thetaHigherRes); y++) {
							for (x = 0; x < f.sh.anglemapParams.phiWidth; x++) {
								size_t add = x + y * f.sh.anglemapParams.phiWidth;
								shAngleMap[add] += f.angleMap.pdf[add];
							}
						}
					}

					//Facet histograms

						for (int m = 0; m < (1 + nbMoments); m++) {
							BYTE *histCurrentMoment = buffer + f.sh.hitOffset + facetHitsSize + f.profileSize*(1 + nbMoments) + f.textureSize*(1 + nbMoments) + f.directionSize*(1 + nbMoments) + f.sh.anglemapParams.GetRecordedDataSize() + m * f.sh.facetHistogramParams.GetDataSize();
							if (f.sh.facetHistogramParams.recordBounce) {
								double* nbHitsHistogram = (double*)histCurrentMoment;
								for (size_t i = 0; i < f.sh.facetHistogramParams.GetBounceHistogramSize(); i++) {
									nbHitsHistogram[i] += f.tmpHistograms[m].nbHitsHistogram[i];
								}
							}
							if (f.sh.facetHistogramParams.recordDistance) {
								double* distanceHistogram = (double*)(histCurrentMoment + f.sh.facetHistogramParams.GetBouncesDataSize());
								for (size_t i = 0; i < (f.sh.facetHistogramParams.GetDistanceHistogramSize()); i++) {
									distanceHistogram[i] += f.tmpHistograms[m].distanceHistogram[i];
								}
							}
							if (f.sh.facetHistogramParams.recordTime) {
								double* timeHistogram = (double*)(histCurrentMoment + f.sh.facetHistogramParams.GetBouncesDataSize() + f.sh.facetHistogramParams.GetDistanceDataSize());
								for (size_t i = 0; i < (f.sh.facetHistogramParams.GetTimeHistogramSize()); i++) {
									timeHistogram[i] += f.tmpHistograms[m].timeHistogram[i];
								}
							}
						}

				} // End if(hitted)
			} // End nbFacet
		} // End nbSuper
		*/

		//if there were no textures:
		for (int v = 0; v < 3; v++) {
			if (worker->results.globalHits.texture_limits[v].min.all == HITMAX) worker->results.globalHits.texture_limits[v].min.all = texture_limits_old[v].min.all;
			if (worker->results.globalHits.texture_limits[v].min.moments_only == HITMAX) worker->results.globalHits.texture_limits[v].min.moments_only = texture_limits_old[v].min.moments_only;
			if (worker->results.globalHits.texture_limits[v].max.all == 0.0) worker->results.globalHits.texture_limits[v].max.all = texture_limits_old[v].max.all;
			if (worker->results.globalHits.texture_limits[v].max.moments_only == 0.0) worker->results.globalHits.texture_limits[v].max.moments_only = texture_limits_old[v].max.moments_only;
		}

		ReleaseMutex(worker->results.mutex);

		myTmpResults.Reset();
		SetLocalAndMasterState(0, GetMyStatusAsText(), false, true);

}




void Simulation::UpdateLog(size_t timeout)
{
	if (tmpParticleLog.size()) {

		SetLocalAndMasterState(0, "Waiting for 'dpLog' dataport access...", false, true);
		if (!LockMutex(worker->logMutex, timeout)) return;
		SetLocalAndMasterState(0, "Updating Log...", false, true);
		size_t writeNb = myOtfp.logLimit - worker->log.size();
		Saturate(writeNb, 0, tmpParticleLog.size());
		worker->log.insert(worker->log.begin(), tmpParticleLog.begin(), tmpParticleLog.begin() + writeNb);
		myLogTarget = (myOtfp.logLimit - worker->log.size()) / myOtfp.nbProcess + 1; //+1 to avoid all threads rounding down
		ReleaseMutex(worker->logMutex);
		tmpParticleLog.clear();
		SetLocalAndMasterState(0, GetMyStatusAsText(), false, true);
	}
}

// Compute particle teleport

void Simulation::PerformTeleport(SubprocessFacet *iFacet) {


	//Search destination
	SubprocessFacet *destination;
	bool found = false;
	bool revert = false;
	int destIndex;
	if (iFacet->facetRef->sh.teleportDest == -1) {
		destIndex = currentParticle.teleportedFrom;
		if (destIndex == -1) {
			/*char err[128];
			sprintf(err, "Facet %d tried to teleport to the facet where the particle came from, but there is no such facet.", iFacet->globalId + 1);
			SetErrorSub(err);*/
			RecordHit(HIT_REF);
			currentParticle.lastHitFacet = iFacet;
			return; //LEAK
		}
	}
	else destIndex = iFacet->facetRef->sh.teleportDest - 1;

	//Look in which superstructure is the destination facet:
	for (size_t i = 0; i < worker->subprocessStructures.size() && (!found); i++) {
		for (size_t j = 0; j < worker->subprocessStructures[i].facets.size() && (!found); j++) {
			if (destIndex == worker->subprocessStructures[i].facets[j].globalId) {
				destination = &(worker->subprocessStructures[i].facets[j]);
				if (destination->facetRef->sh.superIdx != -1) {
					currentParticle.structureId = destination->facetRef->sh.superIdx; //change current superstructure, unless the target is a universal facet
				}
				currentParticle.teleportedFrom = (int)iFacet->globalId; //memorize where the particle came from
				found = true;
			}
		}
	}
	if (!found) {
		/*char err[128];
		sprintf(err, "Teleport destination of facet %d not found (facet %d does not exist)", iFacet->globalId + 1, iFacet->facetRef->sh.teleportDest);
		SetErrorSub(err);*/
		RecordHit(HIT_REF);
		currentParticle.lastHitFacet = iFacet;
		return; //LEAK
	}
	// Count this hit as a transparent pass
	RecordHit(HIT_TELEPORTSOURCE);
	if (/*iFacet->texture && */iFacet->facetRef->sh.countTrans) RecordHitOnTexture(iFacet, currentParticle.flightTime, true, 2.0, 2.0);
	if (/*iFacet->direction && */iFacet->facetRef->sh.countDirection) RecordDirectionVector(iFacet, currentParticle.flightTime);
	ProfileFacet(iFacet, currentParticle.flightTime, true, 2.0, 2.0);
	LogHit(iFacet);
	if (iFacet->facetRef->sh.anglemapParams.record) RecordAngleMap(iFacet);

	// Relaunch particle from new facet
	auto[inTheta, inPhi] = CartesianToPolar(currentParticle.direction, iFacet->facetRef->sh.nU, iFacet->facetRef->sh.nV, iFacet->facetRef->sh.N);
	PolarToCartesian(destination, inTheta, inPhi, false);
	// Move particle to teleport destination point
	double u = myTmpFacetVars[iFacet->globalId].colU;
	double v = myTmpFacetVars[iFacet->globalId].colV;
	currentParticle.position = destination->facetRef->sh.O + u * destination->facetRef->sh.U + v * destination->facetRef->sh.V;
	RecordHit(HIT_TELEPORTDEST);
	int nbTry = 0;
	if (!IsInFacet(*destination, u, v)) { //source and destination facets not the same shape, would generate leak
		// Choose a new starting point
		RecordHit(HIT_ABS);
		bool found = false;
		while (!found && nbTry < 1000) {
			u = randomGenerator.rnd();
			v = randomGenerator.rnd();
			if (IsInFacet(*destination, u, v)) {
				found = true;
				currentParticle.position = destination->facetRef->sh.O + u * destination->facetRef->sh.U + v * destination->facetRef->sh.V;
				RecordHit(HIT_DES);
			}
		}
		nbTry++;
	}

	currentParticle.lastHitFacet = destination;

	//Count hits on teleport facets
	/*iFacet->facetRef->sh.tmpCounter.nbAbsEquiv++;
	destination->sh.tmpCounter.nbDesorbed++;*/

	double ortVelocity = currentParticle.velocity*std::abs(Dot(currentParticle.direction, iFacet->facetRef->sh.N));
	//We count a teleport as a local hit, but not as a global one since that would affect the MFP calculation
	/*iFacet->facetRef->sh.tmpCounter.nbMCHit++;
	iFacet->facetRef->sh.tmpCounter.sum_1_per_ort_velocity += 2.0 / ortVelocity;
	iFacet->facetRef->sh.tmpCounter.sum_v_ort += 2.0*(worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity;*/
	IncreaseFacetCounter(iFacet, currentParticle.flightTime, 1, 0, 0, 2.0 / ortVelocity, 2.0*(worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity);
	myTmpFacetVars[iFacet->globalId].hitted = true;
	/*destination->sh.tmpCounter.sum_1_per_ort_velocity += 2.0 / currentParticle.velocity;
	destination->sh.tmpCounter.sum_v_ort += currentParticle.velocity*std::abs(DOT3(
	currentParticle.direction.x, currentParticle.direction.y, currentParticle.direction.z,
	destination->sh.N.x, destination->sh.N.y, destination->sh.N.z));*/
}

// Perform nbStep simulation steps (a step is a bounce)

bool Simulation::SimulationMCStep(size_t nbStep) {

	// Perform simulation steps
	for (size_t i = 0; i < nbStep; i++) {

		//Prepare output values
		auto[found, collidedFacet, d] = Intersect(this, worker->subprocessStructures, currentParticle.position, currentParticle.direction);

		if (found) {

			// Move particle to intersection point
			currentParticle.position = currentParticle.position + d * currentParticle.direction;
			//currentParticle.distanceTraveled += d;

			double lastFLightTime = currentParticle.flightTime; //memorize for partial hits
			currentParticle.flightTime += d / 100.0 / currentParticle.velocity; //conversion from cm to m

			if ((!worker->wp.calcConstantFlow && (currentParticle.flightTime > worker->wp.latestMoment))
				|| (worker->wp.enableDecay && (currentParticle.expectedDecayMoment < currentParticle.flightTime))) {
				//hit time over the measured period - we create a new particle
				//OR particle has decayed
				double remainderFlightPath = currentParticle.velocity*100.0*
					Min(worker->wp.latestMoment - lastFLightTime, currentParticle.expectedDecayMoment - lastFLightTime); //distance until the point in space where the particle decayed
				myTmpResults.globalHits.distTraveled_total += remainderFlightPath * currentParticle.oriRatio;
				RecordHit(HIT_LAST);
				//sHandle->distTraveledSinceUpdate += currentParticle.distanceTraveled;
				if (!StartFromSource())
					// desorptionLimit reached
					return false;
			}
			else { //hit within measured time, particle still alive
				if (collidedFacet->facetRef->sh.teleportDest != 0) { //Teleport
					IncreaseDistanceCounters(d * currentParticle.oriRatio);
					PerformTeleport(collidedFacet);
				}
				/*else if ((GetOpacityAt(collidedFacet, currentParticle.flightTime) < 1.0) && (randomGenerator.rnd() > GetOpacityAt(collidedFacet, currentParticle.flightTime))) {
					//Transparent pass
					myTmpResults.globalHits.distTraveled_total += d;
					PerformTransparentPass(collidedFacet);
				}*/
				else { //Not teleport
					IncreaseDistanceCounters(d * currentParticle.oriRatio);
					double stickingProbability = GetStickingAt(collidedFacet, currentParticle.flightTime);
					if (!myOtfp.lowFluxMode) { //Regular stick or bounce
						if (stickingProbability == 1.0 || ((stickingProbability > 0.0) && (randomGenerator.rnd() < (stickingProbability)))) {
							//Absorbed
							RecordAbsorb(collidedFacet);
							//sHandle->distTraveledSinceUpdate += currentParticle.distanceTraveled;
							if (!StartFromSource())
								// desorptionLimit reached
								return false;
						}
						else {
							//Reflected
							PerformBounce(collidedFacet);
						}
					}
					else { //Low flux mode
						if (stickingProbability > 0.0) {
							double oriRatioBeforeCollision = currentParticle.oriRatio; //Local copy
							currentParticle.oriRatio *= (stickingProbability); //Sticking part
							RecordAbsorb(collidedFacet);
							currentParticle.oriRatio = oriRatioBeforeCollision * (1.0 - stickingProbability); //Reflected part
						}
						else
							currentParticle.oriRatio *= (1.0 - stickingProbability);
						if (currentParticle.oriRatio > myOtfp.lowFluxCutoff) {
							PerformBounce(collidedFacet);
						}
						else { //eliminate remainder and create new particle
							if (!StartFromSource())
								// desorptionLimit reached
								return false;
						}
					}
				}
			} //end hit within measured time
		} //end intersection found
		else {
			// No intersection found: Leak
			myTmpResults.globalHits.nbLeakTotal++;
			RecordLeakPos();
			if (!StartFromSource())
				// desorptionLimit reached
				return false;
		}
	}
	return true;
}

void Simulation::IncreaseDistanceCounters(double distanceIncrement)
{
	myTmpResults.globalHits.distTraveled_total += distanceIncrement;
	myTmpResults.globalHits.distTraveledTotal_fullHitsOnly += distanceIncrement;
	currentParticle.distanceTraveled += distanceIncrement;
}

// Launch a ray from a source facet. The ray 
// direction is chosen according to the desorption type.

bool Simulation::StartFromSource() {
	bool found = false;
	bool foundInMap = false;
	bool reverse;
	size_t mapPositionW, mapPositionH;
	SubprocessFacet *src = NULL;
	double srcRnd;
	double sumA = 0.0;
	int i = 0, j = 0;
	int nbTry = 0;

	// Check end of simulation
	if (myOtfp.desorptionLimit > 0) {
		if (totalDesorbed >= myOtfp.desorptionLimit / myOtfp.nbProcess) {
			currentParticle.lastHitFacet = NULL;
			return false;
		}
	}

	// Select source
	srcRnd = randomGenerator.rnd() * worker->wp.totalDesorbedMolecules;

	while (!found && j < worker->subprocessStructures.size()) { //Go through superstructures
		i = 0;
		while (!found && i < worker->subprocessStructures[j].facets.size()) { //Go through facets in a structure
			SubprocessFacet& f = worker->subprocessStructures[j].facets[i];
			if (f.facetRef->sh.desorbType != DES_NONE) { //there is some kind of outgassing
				if (f.facetRef->sh.useOutgassingFile) { //Using SynRad-generated outgassing map
					if (f.facetRef->sh.totalOutgassing > 0.0) {
						found = (srcRnd >= sumA) && (srcRnd < (sumA + worker->wp.latestMoment * f.facetRef->sh.totalOutgassing / (1.38E-23*f.facetRef->sh.temperature)));
						if (found) {
							//look for exact position in map
							double rndRemainder = (srcRnd - sumA) / worker->wp.latestMoment*(1.38E-23*f.facetRef->sh.temperature); //remainder, should be less than f.facetRef->sh.totalOutgassing
							/*double sumB = 0.0;
							for (w = 0; w < f.facetRef->sh.outgassingMapWidth && !foundInMap; w++) {
								for (h = 0; h < f.facetRef->sh.outgassingMapHeight && !foundInMap; h++) {
									double cellOutgassing = f.outgassingMap[h*f.facetRef->sh.outgassingMapWidth + w];
									if (cellOutgassing > 0.0) {
										foundInMap = (rndRemainder >= sumB) && (rndRemainder < (sumB + cellOutgassing));
										if (foundInMap) mapPositionW = w; mapPositionH = h;
										sumB += cellOutgassing;
									}
								}
							}*/
							double lookupValue = rndRemainder;
							int outgLowerIndex = my_lower_bound(lookupValue, f.outgassingMapCdf); //returns line number AFTER WHICH LINE lookup value resides in ( -1 .. size-2 )
							outgLowerIndex++;
							mapPositionH = (size_t)((double)outgLowerIndex / (double)f.facetRef->sh.outgassingMapWidth);
							mapPositionW = (size_t)outgLowerIndex - mapPositionH * f.facetRef->sh.outgassingMapWidth;
							foundInMap = true;
							/*if (!foundInMap) {
								SetErrorSub("Starting point not found in imported desorption map");
								return false;
							}*/
						}
						sumA += worker->wp.latestMoment * f.facetRef->sh.totalOutgassing / (1.38E-23*f.facetRef->sh.temperature);
					}
				} //end outgassing file block
				else { //constant or time-dependent outgassing
					double facetOutgassing =
						(f.facetRef->sh.outgassing_paramId >= 0)
						? worker->IDs[f.facetRef->sh.IDid].back().second / (1.38E-23*f.facetRef->sh.temperature)
						: worker->wp.latestMoment*f.facetRef->sh.outgassing / (1.38E-23*f.facetRef->sh.temperature);
					found = (srcRnd >= sumA) && (srcRnd < (sumA + facetOutgassing));
					sumA += facetOutgassing;
				} //end constant or time-dependent outgassing block
			} //end 'there is some kind of outgassing'
			if (!found) i++;
			if (f.facetRef->sh.is2sided) reverse = randomGenerator.rnd() > 0.5;
			else reverse = false;
		}
		if (!found) j++;
	}
	if (!found) {
		SetErrorSub("No starting point, aborting");
		return false;
	}
	src = &(worker->subprocessStructures[j].facets[i]);

	currentParticle.lastHitFacet = src;
	//currentParticle.distanceTraveled = 0.0;  //for mean free path calculations
	//currentParticle.flightTime = sHandle->desorptionStartTime + (sHandle->desorptionStopTime - sHandle->desorptionStartTime)*randomGenerator.rnd();
	currentParticle.flightTime = GenerateDesorptionTime(src);
	if (worker->wp.useMaxwellDistribution) currentParticle.velocity = GenerateRandomVelocity(src->facetRef->sh.CDFid);
	else currentParticle.velocity = 145.469*sqrt(src->facetRef->sh.temperature / worker->wp.gasMass);  //sqrt(8*R/PI/1000)=145.47
	currentParticle.oriRatio = 1.0;
	if (worker->wp.enableDecay) { //decaying gas
		currentParticle.expectedDecayMoment = currentParticle.flightTime + worker->wp.halfLife*1.44269*-log(randomGenerator.rnd()); //1.44269=1/ln2
		//Exponential distribution PDF: probability of 't' life = 1/TAU*exp(-t/TAU) where TAU = half_life/ln2
		//Exponential distribution CDF: probability of life shorter than 't" = 1-exp(-t/TAU)
		//Equation: randomGenerator.rnd()=1-exp(-t/TAU)
		//Solution: t=TAU*-log(1-randomGenerator.rnd()) and 1-randomGenerator.rnd()=randomGenerator.rnd() therefore t=half_life/ln2*-log(randomGenerator.rnd())
	}
	else {
		currentParticle.expectedDecayMoment = 1e100; //never decay
	}
	//sHandle->temperature = src->facetRef->sh.temperature; //Thermalize particle
	currentParticle.nbBounces = 0;
	currentParticle.distanceTraveled = 0;

	found = false; //Starting point within facet

	// Choose a starting point
	while (!found && nbTry < 1000) {
		double u, v;

		if (foundInMap) {
			if (mapPositionW < (src->facetRef->sh.outgassingMapWidth - 1)) {
				//Somewhere in the middle of the facet
				u = ((double)mapPositionW + randomGenerator.rnd()) / src->outgassingMapWidthD;
			}
			else {
				//Last element, prevent from going out of facet
				u = ((double)mapPositionW + randomGenerator.rnd() * (src->outgassingMapWidthD - (src->facetRef->sh.outgassingMapWidth - 1))) / src->outgassingMapWidthD;
			}
			if (mapPositionH < (src->facetRef->sh.outgassingMapHeight - 1)) {
				//Somewhere in the middle of the facet
				v = ((double)mapPositionH + randomGenerator.rnd()) / src->outgassingMapHeightD;
			}
			else {
				//Last element, prevent from going out of facet
				v = ((double)mapPositionH + randomGenerator.rnd() * (src->outgassingMapHeightD - (src->facetRef->sh.outgassingMapHeight - 1))) / src->outgassingMapHeightD;
			}
		}
		else {
			u = randomGenerator.rnd();
			v = randomGenerator.rnd();
		}
		if (IsInFacet(*src, u, v)) {

			// (U,V) -> (x,y,z)
			currentParticle.position = src->facetRef->sh.O + u * src->facetRef->sh.U + v * src->facetRef->sh.V;
			myTmpFacetVars[src->globalId].colU = u;
			myTmpFacetVars[src->globalId].colV = v;
			found = true;

		}
		nbTry++;
	}

	if (!found) {
		// Get the center, if the center is not included in the facet, a leak is generated.
		if (foundInMap) {
			//double uLength = sqrt(pow(src->facetRef->sh.U.x, 2) + pow(src->facetRef->sh.U.y, 2) + pow(src->facetRef->sh.U.z, 2));
			//double vLength = sqrt(pow(src->facetRef->sh.V.x, 2) + pow(src->facetRef->sh.V.y, 2) + pow(src->facetRef->sh.V.z, 2));
			double u = ((double)mapPositionW + 0.5) / src->outgassingMapWidthD;
			double v = ((double)mapPositionH + 0.5) / src->outgassingMapHeightD;
			currentParticle.position = src->facetRef->sh.O + u * src->facetRef->sh.U + v * src->facetRef->sh.V;
			myTmpFacetVars[src->globalId].colU = u;
			myTmpFacetVars[src->globalId].colV = v;
		}
		else {
			myTmpFacetVars[src->globalId].colU = 0.5;
			myTmpFacetVars[src->globalId].colV = 0.5;
			currentParticle.position = worker->subprocessStructures[j].facets[i].facetRef->sh.center;
		}

	}

	if (src->facetRef->sh.isMoving && worker->wp.motionType) RecordHit(HIT_MOVING);
	else RecordHit(HIT_DES); //create blue hit point for created particle

	//See docs/theta_gen.png for further details on angular distribution generation
	switch (src->facetRef->sh.desorbType) {
	case DES_UNIFORM:
		currentParticle.direction = PolarToCartesian(src, acos(randomGenerator.rnd()), randomGenerator.rnd()*2.0*PI, reverse);
		break;
	case DES_NONE: //for file-based
	case DES_COSINE:
		currentParticle.direction = PolarToCartesian(src, acos(sqrt(randomGenerator.rnd())), randomGenerator.rnd()*2.0*PI, reverse);
		break;
	case DES_COSINE_N:
		currentParticle.direction = PolarToCartesian(src, acos(pow(randomGenerator.rnd(), 1.0 / (src->facetRef->sh.desorbTypeN + 1.0))), randomGenerator.rnd()*2.0*PI, reverse);
		break;
	case DES_ANGLEMAP:
	{
		auto [theta, thetaLowerIndex, thetaOvershoot] = src->generatingAngleMap.GenerateThetaFromAngleMap(src->facetRef->sh.anglemapParams, randomGenerator);
		auto phi = src->generatingAngleMap.GeneratePhiFromAngleMap(thetaLowerIndex, thetaOvershoot, src->facetRef->sh.anglemapParams, randomGenerator);
		/*

		size_t angleMapSum = src->angleMapLineSums[(src->facetRef->sh.anglemapParams.thetaLowerRes + src->facetRef->sh.anglemapParams.thetaHigherRes) - 1];
		if (angleMapSum == 0) {
			std::stringstream tmp;
			tmp << "Facet " << src->globalId + 1 << ": angle map has all 0 values";
			SetErrorSub(tmp.str().c_str());
			return false;
		}
		double lookupValue = randomGenerator.rnd()*(double)angleMapSum; //last element of cumulative distr. = sum
		int thetaLowerIndex = my_lower_bound(lookupValue, src->angleMapLineSums, (src->facetRef->sh.anglemapParams.thetaLowerRes + src->facetRef->sh.anglemapParams.thetaHigherRes)); //returns line number AFTER WHICH LINE lookup value resides in ( -1 .. size-2 )
		double thetaOvershoot;
		double theta_a, theta_b, theta_c, theta_cumulative_A, theta_cumulative_B, theta_cumulative_C;
		bool theta_hasThreePoints;
		if (thetaLowerIndex == -1) {//In the first line
			thetaOvershoot = lookupValue / (double)src->angleMapLineSums[0];
		}
		else {
			//(lower index can't be last element)
			thetaOvershoot = (lookupValue - (double)src->angleMapLineSums[thetaLowerIndex]) / (double)(src->angleMapLineSums[thetaLowerIndex + 1] - src->angleMapLineSums[thetaLowerIndex]);
		}

		theta_a = GetTheta(src, (double)thetaLowerIndex); theta_cumulative_A = (thetaLowerIndex == -1) ? 0.0 : (double)src->angleMapLineSums[thetaLowerIndex + 0];
		theta_b = GetTheta(src, (double)(thetaLowerIndex+1)); theta_cumulative_B = (double)src->angleMapLineSums[thetaLowerIndex + 1];
		if ((thetaLowerIndex + 2)<(src->facetRef->sh.anglemapParams.thetaLowerRes + src->facetRef->sh.anglemapParams.thetaHigherRes)) {
			theta_c = GetTheta(src, (double)(thetaLowerIndex+2)); theta_cumulative_C = (double)src->angleMapLineSums[thetaLowerIndex + 2];
			theta_hasThreePoints = true;
		}
		else {
			theta_hasThreePoints = false;
		}

		double theta;
		theta_hasThreePoints = false; //debug
		if (!theta_hasThreePoints) theta = GetTheta(src,thetaLowerIndex + thetaOvershoot);
		else theta = InverseQuadraticInterpolation(lookupValue, theta_a, theta_b, theta_c, theta_cumulative_A, theta_cumulative_B, theta_cumulative_C);

		//Theta determined, let's find phi
		double weigh;
		size_t distr1index, distr2index;
		if (thetaOvershoot < 0.5) {
			distr1index = thetaLowerIndex;
			distr2index = thetaLowerIndex + 1;
			weigh = thetaOvershoot + 0.5;
		}
		else {
			distr1index = thetaLowerIndex + 1;
			distr2index = thetaLowerIndex + 2;
			weigh = thetaOvershoot - 0.5;
		}
		if (distr1index == -1) distr1index++; //In case we interpolate in the first theta range and thetaOvershoot<0.5
		if (distr2index == (src->facetRef->sh.anglemapParams.thetaLowerRes + src->facetRef->sh.anglemapParams.thetaHigherRes)) distr2index--; //In case we interpolate in the last theta range and thetaOvershoot>=0.5

		size_t distr1sum = src->angleMapLineSums[distr1index]-((distr1index!=distr2index && distr1index>0)?src->angleMapLineSums[distr1index-1]:0);
		size_t distr2sum = src->angleMapLineSums[distr2index] - ((distr1index != distr2index) ? src->angleMapLineSums[distr2index - 1] : 0);
		double weighedSum=Weigh((double)distr1sum, (double)distr2sum, weigh);
		double phiLookup = randomGenerator.rnd() * weighedSum;
		int phiLowerIndex = weighed_lower_bound_X(phiLookup, weigh, &(src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index]), &(src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index]), src->facetRef->sh.anglemapParams.phiWidth);

		////////////////

		double phiOvershoot;
		double phi_a, phi_b, phi_c, phi_cumulative_A, phi_cumulative_B, phi_cumulative_C;
		bool phi_hasThreePoints;
		if (phiLowerIndex == -1) {
			phiOvershoot = phiLookup / Weigh((double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index], (double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index],weigh);
		}
		else {
			//(lower index can't be last element)
			phiOvershoot = (phiLookup - Weigh((double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index + phiLowerIndex], (double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index+phiLowerIndex], weigh))
				/ (Weigh((double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index + phiLowerIndex + 1], (double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index + phiLowerIndex + 1], weigh)
				- Weigh((double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index + phiLowerIndex], (double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index + phiLowerIndex], weigh));
		}

		phi_a = 2.0*PI*(double)(phiLowerIndex + 1) / (double)src->facetRef->sh.anglemapParams.phiWidth - PI; phi_cumulative_A = (phiLowerIndex == -1) ? 0.0 : Weigh((double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index + phiLowerIndex], (double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index + phiLowerIndex], weigh);
		phi_b = 2.0*PI*(double)(phiLowerIndex + 2) / (double)src->facetRef->sh.anglemapParams.phiWidth - PI; phi_cumulative_B = Weigh((double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index + phiLowerIndex + 1], (double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index + phiLowerIndex + 1], weigh);
		if ((phiLowerIndex + 2)<(src->facetRef->sh.anglemapParams.phiWidth)) {
			phi_c = 2.0*PI*(double)(phiLowerIndex + 3) / (double)src->facetRef->sh.anglemapParams.phiWidth - PI; phi_cumulative_C = Weigh((double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr1index + phiLowerIndex + 2], (double)src->angleMap_pdf[src->facetRef->sh.anglemapParams.phiWidth*distr2index + phiLowerIndex + 2], weigh);
			phi_hasThreePoints = true;
		}
		else {
			phi_hasThreePoints = false;
		}

		double phi;
		phi_hasThreePoints = false; //debug
		if (!phi_hasThreePoints) phi = 2.0*PI*((double)(phiLowerIndex + 1) + phiOvershoot) / (double)src->facetRef->sh.anglemapParams.phiWidth - PI;
		else phi = InverseQuadraticInterpolation(phiLookup, phi_a, phi_b, phi_c, phi_cumulative_A, phi_cumulative_B, phi_cumulative_C);

		/////////////////////////////
		*/
		currentParticle.direction = PolarToCartesian(src, PI - theta, phi, false); //angle map contains incident angle (between N and source dir) and theta is dir (between N and dest dir)
		//_ASSERTE(currentParticle.direction.x == currentParticle.direction.x);

	}
	}

	// Current structure
	if (src->facetRef->sh.superIdx == -1) {
		std::ostringstream out;
		out << "Facet " << (src->globalId + 1) << " is in all structures, it shouldn't desorb.";
		SetErrorSub(out.str().c_str());
		return false;
	}
	currentParticle.structureId = src->facetRef->sh.superIdx;
	currentParticle.teleportedFrom = -1;

	// Count

	myTmpFacetVars[src->globalId].hitted = true;
	totalDesorbed++;
	myTmpResults.globalHits.globalHits.nbDesorbed++;
	//sHandle->nbPHit = 0;

	if (src->facetRef->sh.isMoving) {
		TreatMovingFacet();
	}

	double ortVelocity = currentParticle.velocity*std::abs(Dot(currentParticle.direction, src->facetRef->sh.N));
	/*src->facetRef->sh.tmpCounter.nbDesorbed++;
	src->facetRef->sh.tmpCounter.sum_1_per_ort_velocity += 2.0 / ortVelocity; //was 2.0 / ortV
	src->facetRef->sh.tmpCounter.sum_v_ort += (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity;*/
	IncreaseFacetCounter(src, currentParticle.flightTime, 0, 1, 0, 2.0 / ortVelocity, (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity);
	//Desorption doesn't contribute to angular profiles, nor to angle maps
	ProfileFacet(src, currentParticle.flightTime, false, 2.0, 1.0); //was 2.0, 1.0
	LogHit(src);
	if (/*src->texture && */src->facetRef->sh.countDes) RecordHitOnTexture(src, currentParticle.flightTime, true, 2.0, 1.0); //was 2.0, 1.0
	//if (src->direction && src->facetRef->sh.countDirection) RecordDirectionVector(src, currentParticle.flightTime);

	found = false;
	return true;
}

std::tuple<double, int, double> GeneratingAnglemap::GenerateThetaFromAngleMap(const AnglemapParams& anglemapParams, MersenneTwister& randomGenerator)
{
	double lookupValue = randomGenerator.rnd();
	int thetaLowerIndex = my_lower_bound(lookupValue, theta_CDF); //returns line number AFTER WHICH LINE lookup value resides in ( -1 .. size-2 )
	double theta, thetaOvershoot;

	if (thetaLowerIndex == -1) { //first half section
		thetaOvershoot = 0.5 + 0.5 * lookupValue / theta_CDF[0]; //between 0.5 and 1
		theta = GetTheta((double)thetaLowerIndex + 0.5 + thetaOvershoot, anglemapParams); //between 0 and the first section end
		return { theta, thetaLowerIndex, thetaOvershoot };
	}
	else if (thetaLowerIndex == (anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes - 1)) { //last half section //can this happen?
		thetaOvershoot = 0.5 * (lookupValue - theta_CDF[thetaLowerIndex])
			/ (1.0 - theta_CDF[thetaLowerIndex]); //between 0 and 0.5
		theta = GetTheta((double)thetaLowerIndex + 0.5 + thetaOvershoot, anglemapParams); //between 0 and the first section end
		return { theta, thetaLowerIndex, thetaOvershoot };
	}
	else { //regular section
		if (/*true || */phi_CDFsums[thetaLowerIndex] == phi_CDFsums[thetaLowerIndex + 1]) {
			//The pdf's slope is 0, linear interpolation
			thetaOvershoot = (lookupValue - theta_CDF[thetaLowerIndex]) / (theta_CDF[thetaLowerIndex + 1] - theta_CDF[thetaLowerIndex]);
			theta = GetTheta((double)thetaLowerIndex + 0.5 + thetaOvershoot, anglemapParams);
		}
		else {
			//2nd degree interpolation
			// y(x) = ax^2 + bx + c
			// c: CDF value at lower index
			// b: pdf value at lower index
			// a: pdf slope at lower index / 2
			// dy := y - c
			// dx := x - [x at lower index]
			// dy = ax^2 + bx
			// dx = ( -b + sqrt(b^2 +4*a*dy) ) / (2a)
			double thetaStep = GetTheta((double)thetaLowerIndex + 1.5, anglemapParams) - GetTheta((double)thetaLowerIndex + 0.5, anglemapParams);
			double c = theta_CDF[thetaLowerIndex]; //CDF value at lower index
			double b = (double)phi_CDFsums[thetaLowerIndex] / (double)theta_CDFsum / thetaStep; //pdf value at lower index
			double a = 0.5 * ((double)(phi_CDFsums[thetaLowerIndex + 1]) - (double)phi_CDFsums[thetaLowerIndex]) / (double)theta_CDFsum / Sqr(thetaStep); //pdf slope at lower index
			double dy = lookupValue - c;

			double dx = (-b + sqrt(Sqr(b) + 4 * a*dy)) / (2 * a); //Since b>=0 it's the + branch of the +- that is valid for us

			thetaOvershoot = dx / thetaStep;
			theta = GetTheta((double)thetaLowerIndex + 0.5 + thetaOvershoot, anglemapParams);
		}
	}
	assert(theta == theta);
	return { theta, thetaLowerIndex, thetaOvershoot };
}

double GeneratingAnglemap::GeneratePhiFromAngleMap(const int & thetaLowerIndex, const double & thetaOvershoot, const AnglemapParams & anglemapParams, MersenneTwister& randomGenerator)
{
	double lookupValue = randomGenerator.rnd();
	if (anglemapParams.phiWidth == 1) return -PI + 2.0 * PI * lookupValue; //special case, uniform phi distribution
	int phiLowerIndex;
	double weigh; //0: take previous theta line, 1: take next theta line, 0..1: interpolate in-between
	if (thetaLowerIndex == -1) { //first theta half section
		lookupValue += phi_CDFs[0]; //periodic BCs over -PI...PI, can be larger than 1
		phiLowerIndex = my_lower_bound(lookupValue, &phi_CDFs[0], anglemapParams.phiWidth); //take entirely the phi ditro belonging to first theta
		weigh = thetaOvershoot; // [0.5 - 1], will subtract 0.5 when evaluating thetaIndex
	}
	else if (thetaLowerIndex == (anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes - 1)) { //last theta half section
		lookupValue += phi_CDFs[thetaLowerIndex*anglemapParams.phiWidth]; //periodic BCs over -PI...PI, can be larger than 1
		phiLowerIndex = my_lower_bound(lookupValue, &phi_CDFs[thetaLowerIndex*anglemapParams.phiWidth], anglemapParams.phiWidth); //take entirely the phi ditro belonging to latest theta
		weigh = thetaOvershoot; // [0 - 0.5], will add 0.5 when evaluating thetaIndex
	}
	else {
		//Here we do a weighing both by the hit sum of the previous and next lines (w1 and w2) and also the weighs of the two lines based on thetaOvershoot (w3 and w4)
		// w1: sum of hits in previous line
		// w2: sum of hits in next line
		// w3: weigh of previous line (1 - thetaOvershoot)
		// w4: weigh of next line     (thetaOvershoot)
		// result: previous value weight: w1*w3 / (w1*w3 + w2*w4)
		//         next     value weight: w2*w4 / (w1*w3 + w2*w4) <- this will be the input for weighed_lower_bound

		double div, weigh;
		div = ((double)phi_CDFsums[thetaLowerIndex] * (1.0 - thetaOvershoot) + (double)phi_CDFsums[thetaLowerIndex + 1] * thetaOvershoot); // (w1*w3 + w2*w4)
		if (div > 0.0) {
			weigh = (thetaOvershoot * (double)phi_CDFsums[thetaLowerIndex + 1]) / div;    //      w2*w4 / (w1*w3 + w2*w4)
		}
		else {
			weigh = thetaOvershoot;
		}
		lookupValue += Weigh((double)phi_CDFs[thetaLowerIndex*anglemapParams.phiWidth], (double)phi_CDFs[(thetaLowerIndex + 1) * anglemapParams.phiWidth], weigh);
		phiLowerIndex = weighed_lower_bound_X(lookupValue, weigh, &phi_CDFs[thetaLowerIndex*anglemapParams.phiWidth], &phi_CDFs[(thetaLowerIndex + 1) * anglemapParams.phiWidth], anglemapParams.phiWidth);
	}

	double phi, phiOvershoot;
	double thetaIndex = (double)thetaLowerIndex + 0.5 + weigh;
	if (phiLowerIndex == -1) { //first half section
		DEBUG_BREAK;
		phiOvershoot = 0.5 + 0.5 * lookupValue / GetPhiCDFValue(thetaIndex, 0, anglemapParams); //between 0.5 and 1
		phi = GetPhi((double)phiLowerIndex + 0.5 + phiOvershoot, anglemapParams); //between 0 and the first section end
	}
	/*else if (phiLowerIndex == (anglemapParams.phiWidth - 1)) { //last half section
		phiOvershoot = 0.5 * (lookupValue - GetPhiCDFValue(thetaIndex, phiLowerIndex, anglemapParams) )
			/ (1.0 - GetPhiCDFValue(thetaIndex, phiLowerIndex, anglemapParams)); //between 0 and 0.5
		phi = GetPhi((double)phiLowerIndex + 0.5 + phiOvershoot, anglemapParams); //between 0 and the first section end
	}*/
	else { //regular or last section 
		if (/*true ||*/ GetPhiPdfValue(thetaIndex, phiLowerIndex, anglemapParams) == GetPhiPdfValue(thetaIndex, phiLowerIndex + 1, anglemapParams)) {
			//The pdf's slope is 0, linear interpolation
			phiOvershoot = (lookupValue - GetPhiCDFValue(thetaIndex, phiLowerIndex, anglemapParams))
				/ (GetPhiCDFValue(thetaIndex, phiLowerIndex + 1, anglemapParams) - GetPhiCDFValue(thetaIndex, phiLowerIndex, anglemapParams));
			phi = GetPhi((double)phiLowerIndex + 0.5 + phiOvershoot, anglemapParams);
		}
		else {

			//2nd degree interpolation
			// y(x) = ax^2 + bx + c
			// c: CDF value at lower index
			// b: pdf value at lower index
			// a: pdf slope at lower index / 2
			// dy := y - c
			// dx := x - [x at lower index]
			// dy = ax^2 + bx
			// dx = ( -b + sqrt(b^2 +4*a*dy) ) / (2a)
			double phiStep = 2.0 * PI / (double)anglemapParams.phiWidth;
			double c = GetPhiCDFValue(thetaIndex, phiLowerIndex, anglemapParams); //CDF value at lower index
			double b = GetPhiPdfValue(thetaIndex, phiLowerIndex, anglemapParams) / GetPhiCDFSum(thetaIndex, anglemapParams) / phiStep; //pdf value at lower index
			double a = 0.5 * (GetPhiPdfValue(thetaIndex, phiLowerIndex + 1, anglemapParams) - GetPhiPdfValue(thetaIndex, phiLowerIndex, anglemapParams)) / GetPhiCDFSum(thetaIndex, anglemapParams) / Sqr(phiStep); //pdf slope at lower index
			double dy = lookupValue - c;

			double D = Sqr(b) + 4 * a*dy; //Discriminant. In rare cases it might be slightly negative, then fall back to linear interpolation:
			if (D < 0) {
				phiOvershoot = (lookupValue - GetPhiCDFValue(thetaIndex, phiLowerIndex, anglemapParams))
					/ (GetPhiCDFValue(thetaIndex, (int)IDX(phiLowerIndex + 1, anglemapParams.phiWidth), anglemapParams) - GetPhiCDFValue(thetaIndex, phiLowerIndex, anglemapParams));
			}
			else {
				double dx = (-b + sqrt(Sqr(b) + 4 * a*dy)) / (2 * a); //Since b>=0 it's the + branch of the +- that is valid for us
				phiOvershoot = dx / phiStep;
			}
			phi = GetPhi((double)phiLowerIndex + 0.5 + phiOvershoot, anglemapParams);
		}
	}
	assert(phi == phi);
	assert(phi > -PI && phi < PI);
	return phi;
}

double GeneratingAnglemap::GetTheta(const double& thetaIndex, const AnglemapParams& anglemapParams)
{
	if ((size_t)(thetaIndex) < anglemapParams.thetaLowerRes) { // 0 < theta < limit
		return anglemapParams.thetaLimit * (thetaIndex) / (double)anglemapParams.thetaLowerRes;
	}
	else { // limit < theta < PI/2
		return anglemapParams.thetaLimit + (PI / 2.0 - anglemapParams.thetaLimit) * (thetaIndex - (double)anglemapParams.thetaLowerRes) / (double)anglemapParams.thetaHigherRes;
	}
}

double GeneratingAnglemap::GetPhi(const double & phiIndex, const AnglemapParams & anglemapParams)
//makes phiIndex circular and converts from index to -pi...pi
{
	double width = (double)anglemapParams.phiWidth;
	double correctedIndex = (phiIndex < width) ? phiIndex : phiIndex - width;
	return -PI + 2.0 * PI * correctedIndex / width;
}

double GeneratingAnglemap::GetPhiPdfValue(const double & thetaIndex, const int & phiLowerIndex, const AnglemapParams& anglemapParams)
//phiLowerIndex is circularized
{
	if (thetaIndex < 0.5) {
		return (double)pdf[IDX(phiLowerIndex, anglemapParams.phiWidth)];
	}
	else if (thetaIndex > (double)(anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes) - 0.5) {
		return (double)pdf[anglemapParams.phiWidth * (anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes - 1) + IDX(phiLowerIndex, anglemapParams.phiWidth)];
	}
	else {
		size_t thetaLowerIndex = (size_t)(thetaIndex - 0.5);
		double thetaOvershoot = thetaIndex - 0.5 - (double)thetaLowerIndex;
		double valueFromLowerpdf = (double)pdf[anglemapParams.phiWidth * thetaLowerIndex + IDX(phiLowerIndex, anglemapParams.phiWidth)];
		double valueFromHigherpdf = (double)pdf[anglemapParams.phiWidth * (thetaLowerIndex + 1) + IDX(phiLowerIndex, anglemapParams.phiWidth)];
		return Weigh(valueFromLowerpdf, valueFromHigherpdf, thetaOvershoot);
	}
}

double GeneratingAnglemap::GetPhiCDFValue(const double & thetaIndex, const int & phiLowerIndex, const AnglemapParams& anglemapParams)
{
	if (thetaIndex < 0.5) {
		return (phiLowerIndex < anglemapParams.phiWidth) ? phi_CDFs[phiLowerIndex] : 1.0 + phi_CDFs[0];
	}
	else if (thetaIndex > (double)(anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes) - 0.5) {
		return (phiLowerIndex < anglemapParams.phiWidth) ? phi_CDFs[anglemapParams.phiWidth * (anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes - 1) + phiLowerIndex] : 1.0 + phi_CDFs[anglemapParams.phiWidth * (anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes - 1)];
	}
	else {
		size_t thetaLowerIndex = (size_t)(thetaIndex - 0.5);
		double thetaOvershoot = thetaIndex - 0.5 - (double)thetaLowerIndex;
		double valueFromLowerCDF = (phiLowerIndex < anglemapParams.phiWidth) ? phi_CDFs[anglemapParams.phiWidth * thetaLowerIndex + phiLowerIndex] : 1.0 + phi_CDFs[anglemapParams.phiWidth * (thetaLowerIndex)];
		double valueFromHigherCDF = (phiLowerIndex < anglemapParams.phiWidth) ? phi_CDFs[anglemapParams.phiWidth * (thetaLowerIndex + 1) + phiLowerIndex] : 1.0 + phi_CDFs[anglemapParams.phiWidth * (thetaLowerIndex + 1)];
		return Weigh(valueFromLowerCDF, valueFromHigherCDF, thetaOvershoot);
	}

}

double GeneratingAnglemap::GetPhiCDFSum(const double & thetaIndex, const AnglemapParams& anglemapParams)
{
	if (thetaIndex < 0.5) {
		return (double)phi_CDFsums[0];
	}
	else if (thetaIndex > (double)(anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes) - 0.5) {
		return (double)phi_CDFsums[anglemapParams.thetaLowerRes + anglemapParams.thetaHigherRes - 1];
	}
	else {
		size_t thetaLowerIndex = (size_t)(thetaIndex - 0.5);
		double thetaOvershoot = thetaIndex - 0.5 - (double)thetaLowerIndex;
		double valueFromLowerSum = (double)phi_CDFsums[thetaLowerIndex];
		double valueFromHigherSum = (double)phi_CDFsums[thetaLowerIndex + 1];
		return Weigh(valueFromLowerSum, valueFromHigherSum, thetaOvershoot);
	}
}

/*inline*/ void Simulation::PerformBounce(SubprocessFacet *iFacet) {

	bool revert = false;

	myTmpResults.globalHits.globalHits.nbMCHit++; //global
	myTmpResults.globalHits.globalHits.nbHitEquiv += currentParticle.oriRatio;

	// Handle super structure link facet. Can be 
	if (iFacet->facetRef->sh.superDest) {
		IncreaseFacetCounter(iFacet, currentParticle.flightTime, 1, 0, 0, 0, 0);
		currentParticle.structureId = iFacet->facetRef->sh.superDest - 1;
		if (iFacet->facetRef->sh.isMoving) { //A very special case where link facets can be used as transparent but moving facets
			RecordHit(HIT_MOVING);
			TreatMovingFacet();
		}
		else {
			// Count this hit as a transparent pass
			RecordHit(HIT_TRANS);
		}
		LogHit(iFacet);
		ProfileFacet(iFacet, currentParticle.flightTime, true, 2.0, 2.0);
		if (iFacet->facetRef->sh.anglemapParams.record) RecordAngleMap(iFacet);
		if (/*iFacet->texture &&*/ iFacet->facetRef->sh.countTrans) RecordHitOnTexture(iFacet, currentParticle.flightTime, true, 2.0, 2.0);
		if (/*iFacet->direction &&*/ iFacet->facetRef->sh.countDirection) RecordDirectionVector(iFacet, currentParticle.flightTime);

		return;

	}

	if (iFacet->facetRef->sh.is2sided) {
		// We may need to revert normal in case of 2 sided hit
		revert = Dot(currentParticle.direction, iFacet->facetRef->sh.N) > 0.0;
	}

	//Texture/Profile incoming hit


	//Register (orthogonal) velocity
	double ortVelocity = currentParticle.velocity*std::abs(Dot(currentParticle.direction, iFacet->facetRef->sh.N));

	/*iFacet->facetRef->sh.tmpCounter.nbMCHit++; //hit facet
	iFacet->facetRef->sh.tmpCounter.sum_1_per_ort_velocity += 1.0 / ortVelocity;
	iFacet->facetRef->sh.tmpCounter.sum_v_ort += (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity;*/

	IncreaseFacetCounter(iFacet, currentParticle.flightTime, 1, 0, 0, 1.0 / ortVelocity, (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity);
	currentParticle.nbBounces++;
	if (/*iFacet->texture &&*/ iFacet->facetRef->sh.countRefl) RecordHitOnTexture(iFacet, currentParticle.flightTime, true, 1.0, 1.0);
	if (/*iFacet->direction &&*/ iFacet->facetRef->sh.countDirection) RecordDirectionVector(iFacet, currentParticle.flightTime);
	LogHit(iFacet);
	ProfileFacet(iFacet, currentParticle.flightTime, true, 1.0, 1.0);
	if (iFacet->facetRef->sh.anglemapParams.record) RecordAngleMap(iFacet);

	// Relaunch particle
	UpdateVelocity(iFacet);
	//Sojourn time
	if (iFacet->facetRef->sh.enableSojournTime) {
		double A = exp(-iFacet->facetRef->sh.sojournE / (8.31*iFacet->facetRef->sh.temperature));
		currentParticle.flightTime += -log(randomGenerator.rnd()) / (A*iFacet->facetRef->sh.sojournFreq);
	}

	if (iFacet->facetRef->sh.reflection.diffusePart > 0.999999) { //Speedup branch for most common, diffuse case
		currentParticle.direction = PolarToCartesian(iFacet, acos(sqrt(randomGenerator.rnd())), randomGenerator.rnd()*2.0*PI, revert);
	}
	else {
		double reflTypeRnd = randomGenerator.rnd();
		if (reflTypeRnd < iFacet->facetRef->sh.reflection.diffusePart)
		{
			//diffuse reflection
			//See docs/theta_gen.png for further details on angular distribution generation
			currentParticle.direction = PolarToCartesian(iFacet, acos(sqrt(randomGenerator.rnd())), randomGenerator.rnd()*2.0*PI, revert);
		}
		else  if (reflTypeRnd < (iFacet->facetRef->sh.reflection.diffusePart + iFacet->facetRef->sh.reflection.specularPart))
		{
			//specular reflection
			auto [inTheta, inPhi] = CartesianToPolar(currentParticle.direction, iFacet->facetRef->sh.nU, iFacet->facetRef->sh.nV, iFacet->facetRef->sh.N);
			currentParticle.direction = PolarToCartesian(iFacet, PI - inTheta, inPhi, false);

		}
		else {
			//Cos^N reflection
			currentParticle.direction = PolarToCartesian(iFacet, acos(pow(randomGenerator.rnd(), 1.0 / (iFacet->facetRef->sh.reflection.cosineExponent + 1.0))), randomGenerator.rnd()*2.0*PI, revert);
		}
	}

	if (iFacet->facetRef->sh.isMoving) {
		TreatMovingFacet();
	}

	//Texture/Profile outgoing particle
	//Register outgoing velocity
	ortVelocity = currentParticle.velocity*std::abs(Dot(currentParticle.direction, iFacet->facetRef->sh.N));

	/*iFacet->facetRef->sh.tmpCounter.sum_1_per_ort_velocity += 1.0 / ortVelocity;
	iFacet->facetRef->sh.tmpCounter.sum_v_ort += (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity;*/
	IncreaseFacetCounter(iFacet, currentParticle.flightTime, 0, 0, 0, 1.0 / ortVelocity, (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity);
	if (/*iFacet->texture &&*/ iFacet->facetRef->sh.countRefl) RecordHitOnTexture(iFacet, currentParticle.flightTime, false, 1.0, 1.0); //count again for outward velocity
	ProfileFacet(iFacet, currentParticle.flightTime, false, 1.0, 1.0);
	//no direction count on outgoing, neither angle map

	if (iFacet->facetRef->sh.isMoving && worker->wp.motionType) RecordHit(HIT_MOVING);
	else RecordHit(HIT_REF);
	currentParticle.lastHitFacet = iFacet;
	//sHandle->nbPHit++;
}

void Simulation::PerformTransparentPass(SubprocessFacet *iFacet) { //disabled, caused finding hits with the same facet
	/*double directionFactor = std::abs(DOT3(
		currentParticle.direction.x, currentParticle.direction.y, currentParticle.direction.z,
		iFacet->facetRef->sh.N.x, iFacet->facetRef->sh.N.y, iFacet->facetRef->sh.N.z));
	iFacet->facetRef->sh.tmpCounter.nbMCHit++;
	iFacet->facetRef->sh.tmpCounter.sum_1_per_ort_velocity += 2.0 / (currentParticle.velocity*directionFactor);
	iFacet->facetRef->sh.tmpCounter.sum_v_ort += 2.0*(worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*currentParticle.velocity*directionFactor;
	iFacet->hitted = true;
	if (iFacet->texture && iFacet->facetRef->sh.countTrans) RecordHitOnTexture(iFacet, currentParticle.flightTime + iFacet->colDistTranspPass / 100.0 / currentParticle.velocity,
		true, 2.0, 2.0);
	if (iFacet->direction && iFacet->facetRef->sh.countDirection) RecordDirectionVector(iFacet, currentParticle.flightTime + iFacet->colDistTranspPass / 100.0 / currentParticle.velocity);
	ProfileFacet(iFacet, currentParticle.flightTime + iFacet->colDistTranspPass / 100.0 / currentParticle.velocity,
		true, 2.0, 2.0);
	RecordHit(HIT_TRANS);
	sHandle->lastHit = iFacet;*/
}

void Simulation::RecordAbsorb(SubprocessFacet *iFacet) {
	myTmpResults.globalHits.globalHits.nbMCHit++; //global	
	myTmpResults.globalHits.globalHits.nbHitEquiv += currentParticle.oriRatio;
	myTmpResults.globalHits.globalHits.nbAbsEquiv += currentParticle.oriRatio;

	RecordHistograms(iFacet);

	RecordHit(HIT_ABS);
	double ortVelocity = currentParticle.velocity*std::abs(Dot(currentParticle.direction, iFacet->facetRef->sh.N));
	IncreaseFacetCounter(iFacet, currentParticle.flightTime, 1, 0, 1, 2.0 / ortVelocity, (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity);
	LogHit(iFacet);
	ProfileFacet(iFacet, currentParticle.flightTime, true, 2.0, 1.0); //was 2.0, 1.0
	if (iFacet->facetRef->sh.anglemapParams.record) RecordAngleMap(iFacet);
	if (/*iFacet->texture &&*/ iFacet->facetRef->sh.countAbs) RecordHitOnTexture(iFacet, currentParticle.flightTime, true, 2.0, 1.0); //was 2.0, 1.0
	if (/*iFacet->direction &&*/ iFacet->facetRef->sh.countDirection) RecordDirectionVector(iFacet, currentParticle.flightTime);
}

void Simulation::RecordHistograms(SubprocessFacet * iFacet)
{
	//Record in global and facet histograms
	for (size_t m = 0; m <= worker->moments.size(); m++) {
		if (m == 0 || std::abs(currentParticle.flightTime - worker->moments[m - 1]) < worker->wp.timeWindowSize / 2.0) {
			size_t binIndex;
			if (worker->wp.globalHistogramParams.recordBounce) {
				binIndex = Min(currentParticle.nbBounces / worker->wp.globalHistogramParams.nbBounceBinsize, worker->wp.globalHistogramParams.GetBounceHistogramSize() - 1);
				myTmpResults.globalHistograms[m].nbHitsHistogram[binIndex] += currentParticle.oriRatio;
			}
			if (worker->wp.globalHistogramParams.recordDistance) {
				binIndex = Min(static_cast<size_t>(currentParticle.distanceTraveled / worker->wp.globalHistogramParams.distanceBinsize), worker->wp.globalHistogramParams.GetDistanceHistogramSize() - 1);
				myTmpResults.globalHistograms[m].distanceHistogram[binIndex] += currentParticle.oriRatio;
			}
			if (worker->wp.globalHistogramParams.recordTime) {
				binIndex = Min(static_cast<size_t>(currentParticle.flightTime / worker->wp.globalHistogramParams.timeBinsize), worker->wp.globalHistogramParams.GetTimeHistogramSize() - 1);
				myTmpResults.globalHistograms[m].timeHistogram[binIndex] += currentParticle.oriRatio;
			}
			if (iFacet->facetRef->sh.facetHistogramParams.recordBounce) {
				binIndex = Min(currentParticle.nbBounces / iFacet->facetRef->sh.facetHistogramParams.nbBounceBinsize, iFacet->facetRef->sh.facetHistogramParams.GetBounceHistogramSize() - 1);
				myTmpResults.facetStates[iFacet->globalId].momentResults[m].histogram.nbHitsHistogram[binIndex] += currentParticle.oriRatio;
			}
			if (iFacet->facetRef->sh.facetHistogramParams.recordDistance) {
				binIndex = Min(static_cast<size_t>(currentParticle.distanceTraveled / iFacet->facetRef->sh.facetHistogramParams.distanceBinsize), iFacet->facetRef->sh.facetHistogramParams.GetDistanceHistogramSize() - 1);
				myTmpResults.facetStates[iFacet->globalId].momentResults[m].histogram.distanceHistogram[binIndex] += currentParticle.oriRatio;
			}
			if (iFacet->facetRef->sh.facetHistogramParams.recordTime) {
				binIndex = Min(static_cast<size_t>(currentParticle.flightTime / iFacet->facetRef->sh.facetHistogramParams.timeBinsize), iFacet->facetRef->sh.facetHistogramParams.GetTimeHistogramSize() - 1);
				myTmpResults.facetStates[iFacet->globalId].momentResults[m].histogram.timeHistogram[binIndex] += currentParticle.oriRatio;
			}
		}
	}
}

Simulation::Simulation(Worker*  w)
{
	worker = w;
	geom = w->GetMolflowGeometry();
}

void Simulation::RecordHitOnTexture(SubprocessFacet *f, double time, bool countHit, double velocity_factor, double ortSpeedFactor) {

	size_t tu = (size_t)(myTmpFacetVars[f->globalId].colU * f->facetRef->sh.texWidthD);
	size_t tv = (size_t)(myTmpFacetVars[f->globalId].colV * f->facetRef->sh.texHeightD);
	size_t add = tu + tv * (f->facetRef->sh.texWidth);
	double ortVelocity = (worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*currentParticle.velocity*std::abs(Dot(currentParticle.direction, f->facetRef->sh.N)); //surface-orthogonal velocity component

	for (size_t m = 0; m <= worker->moments.size(); m++)
		if (m == 0 || std::abs(time - worker->moments[m - 1]) < worker->wp.timeWindowSize / 2.0) {
			if (countHit) myTmpResults.facetStates[f->globalId].momentResults[m].texture[add].countEquiv += currentParticle.oriRatio;
			myTmpResults.facetStates[f->globalId].momentResults[m].texture[add].sum_1_per_ort_velocity += currentParticle.oriRatio * velocity_factor / ortVelocity;
			myTmpResults.facetStates[f->globalId].momentResults[m].texture[add].sum_v_ort_per_area += currentParticle.oriRatio * ortSpeedFactor*ortVelocity*f->textureCellIncrements[add]; // sum ortho_velocity[m/s] / cell_area[cm2]
		}
}

void Simulation::RecordDirectionVector(SubprocessFacet *f, double time) {
	size_t tu = (size_t)(myTmpFacetVars[f->globalId].colU * f->facetRef->sh.texWidthD);
	size_t tv = (size_t)(myTmpFacetVars[f->globalId].colV * f->facetRef->sh.texHeightD);
	size_t add = tu + tv * (f->facetRef->sh.texWidth);

	for (size_t m = 0; m <= worker->moments.size(); m++) {
		if (m == 0 || std::abs(time - worker->moments[m - 1]) < worker->wp.timeWindowSize / 2.0) {
			myTmpResults.facetStates[f->globalId].momentResults[m].direction[add].dir += currentParticle.oriRatio * currentParticle.direction * currentParticle.velocity;
			myTmpResults.facetStates[f->globalId].momentResults[m].direction[add].count++;
		}
	}

}

void Simulation::ProfileFacet(SubprocessFacet *f, double time, bool countHit, double velocity_factor, double ortSpeedFactor) {

	size_t nbMoments = worker->moments.size();

	if (countHit && f->facetRef->sh.profileType == PROFILE_ANGULAR) {
		double dot = Dot(f->facetRef->sh.N, currentParticle.direction);
		double theta = acos(std::abs(dot));     // Angle to normal (PI/2 => PI)
		size_t pos = (size_t)(theta / (PI / 2)*((double)PROFILE_SIZE)); // To Grad
		Saturate(pos, 0, PROFILE_SIZE - 1);
		for (size_t m = 0; m <= nbMoments; m++) {
			if (m == 0 || std::abs(time - worker->moments[m - 1]) < worker->wp.timeWindowSize / 2.0) {
				myTmpResults.facetStates[f->globalId].momentResults[m].profile[pos].countEquiv += currentParticle.oriRatio;
			}
		}
	}
	else if (f->facetRef->sh.profileType == PROFILE_U || f->facetRef->sh.profileType == PROFILE_V) {
		size_t pos = (size_t)((f->facetRef->sh.profileType == PROFILE_U ? myTmpFacetVars[f->globalId].colU : myTmpFacetVars[f->globalId].colV)*(double)PROFILE_SIZE);
		if (pos >= 0 && pos < PROFILE_SIZE) {
			for (size_t m = 0; m <= nbMoments; m++) {
				if (m == 0 || std::abs(time - worker->moments[m - 1]) < worker->wp.timeWindowSize / 2.0) {
					if (countHit) myTmpResults.facetStates[f->globalId].momentResults[m].profile[pos].countEquiv += currentParticle.oriRatio;
					double ortVelocity = currentParticle.velocity*std::abs(Dot(f->facetRef->sh.N, currentParticle.direction));
					myTmpResults.facetStates[f->globalId].momentResults[m].profile[pos].sum_1_per_ort_velocity += currentParticle.oriRatio * velocity_factor / ortVelocity;
					myTmpResults.facetStates[f->globalId].momentResults[m].profile[pos].sum_v_ort += currentParticle.oriRatio * ortSpeedFactor*(worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*ortVelocity;
				}
			}
		}
	}
	else if (countHit && (f->facetRef->sh.profileType == PROFILE_VELOCITY || f->facetRef->sh.profileType == PROFILE_ORT_VELOCITY || f->facetRef->sh.profileType == PROFILE_TAN_VELOCITY)) {
		double dot;
		if (f->facetRef->sh.profileType == PROFILE_VELOCITY) {
			dot = 1.0;
		}
		else if (f->facetRef->sh.profileType == PROFILE_ORT_VELOCITY) {
			dot = std::abs(Dot(f->facetRef->sh.N, currentParticle.direction));  //cos(theta) as "dot" value
		}
		else { //Tangential
			dot = sqrt(1 - Sqr(std::abs(Dot(f->facetRef->sh.N, currentParticle.direction))));  //tangential
		}
		size_t pos = (size_t)(dot*currentParticle.velocity / f->facetRef->sh.maxSpeed*(double)PROFILE_SIZE); //"dot" default value is 1.0
		if (pos >= 0 && pos < PROFILE_SIZE) {
			for (size_t m = 0; m <= nbMoments; m++) {
				if (m == 0 || std::abs(time - worker->moments[m - 1]) < worker->wp.timeWindowSize / 2.0) {
					myTmpResults.facetStates[f->globalId].momentResults[m].profile[pos].countEquiv += currentParticle.oriRatio;
				}
			}
		}
	}
}

void Simulation::LogHit(SubprocessFacet * f)
{
	if (myOtfp.enableLogging &&
		myOtfp.logFacetId == f->globalId &&
		tmpParticleLog.size() < myLogTarget) {
		ParticleLoggerItem log;
		log.facetHitPosition = Vector2d(myTmpFacetVars[f->globalId].colU, myTmpFacetVars[f->globalId].colV);
		std::tie(log.hitTheta, log.hitPhi) = CartesianToPolar(currentParticle.direction, f->facetRef->sh.nU, f->facetRef->sh.nV, f->facetRef->sh.N);
		log.oriRatio = currentParticle.oriRatio;
		log.particleDecayMoment = currentParticle.expectedDecayMoment;
		log.time = currentParticle.flightTime;
		log.velocity = currentParticle.velocity;
		tmpParticleLog.push_back(log);
	}
}

void Simulation::RecordAngleMap(SubprocessFacet* collidedFacet) {
	auto[inTheta, inPhi] = CartesianToPolar(currentParticle.direction, collidedFacet->facetRef->sh.nU, collidedFacet->facetRef->sh.nV, collidedFacet->facetRef->sh.N);
	if (inTheta > PI / 2.0) inTheta = std::abs(PI - inTheta); //theta is originally respective to N, but we'd like the angle between 0 and PI/2
	bool countTheta = true;
	size_t thetaIndex;
	if (inTheta < collidedFacet->facetRef->sh.anglemapParams.thetaLimit) {
		if (collidedFacet->facetRef->sh.anglemapParams.thetaLowerRes > 0) {
			thetaIndex = (size_t)(inTheta / collidedFacet->facetRef->sh.anglemapParams.thetaLimit*(double)collidedFacet->facetRef->sh.anglemapParams.thetaLowerRes);
		}
		else {
			countTheta = false;
		}
	}
	else {
		if (collidedFacet->facetRef->sh.anglemapParams.thetaHigherRes > 0) {
			thetaIndex = collidedFacet->facetRef->sh.anglemapParams.thetaLowerRes + (size_t)((inTheta - collidedFacet->facetRef->sh.anglemapParams.thetaLimit)
				/ (PI / 2.0 - collidedFacet->facetRef->sh.anglemapParams.thetaLimit)*(double)collidedFacet->facetRef->sh.anglemapParams.thetaHigherRes);
		}
		else {
			countTheta = false;
		}
	}
	if (countTheta) {
		size_t phiIndex = (size_t)((inPhi + 3.1415926) / (2.0*PI)*(double)collidedFacet->facetRef->sh.anglemapParams.phiWidth); //Phi: -PI..PI , and shifting by a number slightly smaller than PI to store on interval [0,2PI[
		myTmpResults.facetStates[collidedFacet->globalId].recordedAngleMapPdf[thetaIndex*collidedFacet->facetRef->sh.anglemapParams.phiWidth + phiIndex]++;
		//collidedFacet->angleMap.pdf[thetaIndex*collidedFacet->facetRef->sh.anglemapParams.phiWidth + phiIndex]++;
	}
}

/*inline*/ void Simulation::UpdateVelocity(SubprocessFacet *collidedFacet) {
	if (collidedFacet->facetRef->sh.accomodationFactor > 0.9999) { //speedup for the most common case: perfect thermalization
		if (worker->wp.useMaxwellDistribution) currentParticle.velocity = GenerateRandomVelocity(collidedFacet->facetRef->sh.CDFid);
		else currentParticle.velocity = 145.469*sqrt(collidedFacet->facetRef->sh.temperature / worker->wp.gasMass);
	}
	else {
		double oldSpeed2 = pow(currentParticle.velocity, 2);
		double newSpeed2;
		if (worker->wp.useMaxwellDistribution) newSpeed2 = pow(GenerateRandomVelocity(collidedFacet->facetRef->sh.CDFid), 2);
		else newSpeed2 = /*145.469*/ 29369.939*(collidedFacet->facetRef->sh.temperature / worker->wp.gasMass);
		//sqrt(29369)=171.3766= sqrt(8*R*1000/PI)*3PI/8, that is, the constant part of the v_avg=sqrt(8RT/PI/m/0.001)) found in literature, multiplied by
		//the corrective factor of 3PI/8 that accounts for moving from volumetric speed distribution to wall collision speed distribution
		currentParticle.velocity = sqrt(oldSpeed2 + (newSpeed2 - oldSpeed2)*collidedFacet->facetRef->sh.accomodationFactor);
	}
}

/*inline*/ double Simulation::GenerateRandomVelocity(int CDFId) {
	//return FastLookupY(randomGenerator.rnd(),worker->CDFs[CDFId],false);
	double r = randomGenerator.rnd();
	double v = InterpolateX(r, worker->CDFs[CDFId], false, true); //Allow extrapolate
	return v;
}

double Simulation::GenerateDesorptionTime(SubprocessFacet *src) {
	if (src->facetRef->sh.outgassing_paramId >= 0) { //time-dependent desorption
		return InterpolateX(randomGenerator.rnd()*worker->IDs[src->facetRef->sh.IDid].back().second, worker->IDs[src->facetRef->sh.IDid], false, true); //allow extrapolate
	}
	else {
		return randomGenerator.rnd()*worker->wp.latestMoment; //continous desorption between 0 and latestMoment
	}
}

double Simulation::GetStickingAt(SubprocessFacet *f, double time) {
	if (f->facetRef->sh.sticking_paramId == -1) //constant sticking
		return f->facetRef->sh.sticking;
	else return worker->parameters[f->facetRef->sh.sticking_paramId].InterpolateY(time, false);
}

double Simulation::GetOpacityAt(SubprocessFacet *f, double time) {
	if (f->facetRef->sh.opacity_paramId == -1) //constant sticking
		return f->facetRef->sh.opacity;
	else return worker->parameters[f->facetRef->sh.opacity_paramId].InterpolateY(time, false);
}

void Simulation::TreatMovingFacet() {
	Vector3d localVelocityToAdd;
	if (worker->wp.motionType == 1) {
		localVelocityToAdd = worker->wp.motionVector2;
	}
	else if (worker->wp.motionType == 2) {
		Vector3d distanceVector = 0.01*(currentParticle.position - worker->wp.motionVector1); //distance from base, with cm->m conversion
		localVelocityToAdd = CrossProduct(worker->wp.motionVector2, distanceVector);
	}
	Vector3d oldVelocity, newVelocity;
	oldVelocity = currentParticle.direction*currentParticle.velocity;
	newVelocity = oldVelocity + localVelocityToAdd;
	currentParticle.direction = newVelocity.Normalized();
	currentParticle.velocity = newVelocity.Norme();
}

void Simulation::IncreaseFacetCounter(SubprocessFacet *f, double time, size_t hit, size_t desorb, size_t absorb, double sum_1_per_v, double sum_v_ort) {
	size_t nbMoments = worker->moments.size();
	for (size_t m = 0; m <= nbMoments; m++) {
		if (m == 0 || std::abs(time - worker->moments[m - 1]) < worker->wp.timeWindowSize / 2.0) {
			myTmpResults.facetStates[f->globalId].momentResults[m].hits.nbMCHit += hit;
			double hitEquiv = static_cast<double>(hit)*currentParticle.oriRatio;
			myTmpResults.facetStates[f->globalId].momentResults[m].hits.nbHitEquiv += hitEquiv;
			myTmpResults.facetStates[f->globalId].momentResults[m].hits.nbDesorbed += desorb;
			myTmpResults.facetStates[f->globalId].momentResults[m].hits.nbAbsEquiv += static_cast<double>(absorb)*currentParticle.oriRatio;
			myTmpResults.facetStates[f->globalId].momentResults[m].hits.sum_1_per_ort_velocity += currentParticle.oriRatio * sum_1_per_v;
			myTmpResults.facetStates[f->globalId].momentResults[m].hits.sum_v_ort += currentParticle.oriRatio * sum_v_ort;
			myTmpResults.facetStates[f->globalId].momentResults[m].hits.sum_1_per_velocity += (hitEquiv + static_cast<double>(desorb)) / currentParticle.velocity;
		}
	}
}

void Simulation::ResetSimulation() {
	currentParticle.lastHitFacet = NULL;
	totalDesorbed = 0;
	myTmpResults.Reset();
	tmpParticleLog.clear();
	ConstructFacetTmpVars(); //Reset "hitted" property of facets
	myLogTarget = 0;
}

int Simulation::GetIDId(int paramId) {

	int i;
	for (i = 0; i < (int)worker->desorptionParameterIDs.size() && (paramId != worker->desorptionParameterIDs[i]); i++); //check if we already had this parameter Id
	if (i >= (int)worker->desorptionParameterIDs.size()) i = -1; //not found
	return i;
}

void Simulation::RecordHit(const int &type) {
	
	if (myTmpResults.globalHits.hitCacheSize < HITCACHESIZE) {
		myTmpResults.globalHits.hitCache[myTmpResults.globalHits.hitCacheSize].pos = currentParticle.position;
		myTmpResults.globalHits.hitCache[myTmpResults.globalHits.hitCacheSize].type = type;
		myTmpResults.globalHits.hitCacheSize++;
	}
}

void Simulation::RecordLeakPos() {
	// Source region check performed when calling this routine 
	// Record leak for debugging
	RecordHit(HIT_REF);
	RecordHit(HIT_LAST);
	if (myTmpResults.globalHits.leakCacheSize < LEAKCACHESIZE) {
		myTmpResults.globalHits.leakCache[myTmpResults.globalHits.leakCacheSize].pos = currentParticle.position;
		myTmpResults.globalHits.leakCache[myTmpResults.globalHits.leakCacheSize].dir = currentParticle.direction;
		myTmpResults.globalHits.leakCacheSize++;
	}
}