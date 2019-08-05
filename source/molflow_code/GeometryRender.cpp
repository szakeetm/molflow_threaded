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
#include "MolflowGeometry.h"
#include "Worker.h"
#include "Facet_shared.h"
//#include <malloc.h>
#include <string.h>
#include <math.h>
#include "GLApp/GLMatrix.h"
#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "SuperFacet.h"

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif


/**
* \brief Processes events like button clicks for the advanced facet parameters panel.
* \param results contains all simulation results/states (like hits)
* \param renderRegularTexture bool value
* \param renderDirectionTexture bool value
* \param sMode which simulation mode was used (monte carlo / angular coefficient)
*/
void MolflowGeometry::BuildFacetTextures(GlobalSimuState& results, bool renderRegularTexture, bool renderDirectionTexture,size_t sMode) {
	//results should already be mutex locked

	Worker *w = &(mApp->worker);

	int nbMoments = (int)mApp->worker.moments.size();
	size_t facetHitsSize = (1 + nbMoments) * sizeof(FacetHitBuffer);

	GLProgress *prg = new GLProgress("Building texture", "Frame update");
	prg->SetBounds(5, 28, 300, 90);
	int startTime = SDL_GetTicks();

	double dCoef_custom[] = { 1.0, 1.0, 1.0 }; //Three coefficients for pressure, imp.rate, density
											   //Autoscaling limits come from the subprocess corrected by "time factor", which makes constant flow and moment values comparable
											   //Time correction factor in subprocess: MoleculesPerTP * nbDesorbed
	double timeCorrection = 1.0;
	double min, max;

	if (renderRegularTexture) {

		switch (sMode) {

		case MC_MODE:

			dCoef_custom[0] = 1E4 / (double)results.globalHits.globalHits.nbDesorbed * mApp->worker.wp.gasMass / 1000 / 6E23*0.0100; //multiplied by timecorr*sum_v_ort_per_area: pressure
			dCoef_custom[1] = 1E4 / (double)results.globalHits.globalHits.nbDesorbed;
			dCoef_custom[2] = 1E4 / (double)results.globalHits.globalHits.nbDesorbed;
			timeCorrection = (mApp->worker.displayedMoment == 0) ? mApp->worker.wp.finalOutgassingRate : mApp->worker.wp.totalDesorbedMolecules / mApp->worker.wp.timeWindowSize;

			for (int i = 0; i < 3; i++) {
				//texture limits already corrected by timeFactor in UpdateMCHits()
				texture_limits[i].autoscale.min.moments_only = results.globalHits.texture_limits[i].min.moments_only*dCoef_custom[i];
				texture_limits[i].autoscale.max.moments_only = results.globalHits.texture_limits[i].max.moments_only*dCoef_custom[i];
				texture_limits[i].autoscale.min.all = results.globalHits.texture_limits[i].min.all*dCoef_custom[i];
				texture_limits[i].autoscale.max.all = results.globalHits.texture_limits[i].max.all*dCoef_custom[i];
			}
			break;
		case AC_MODE:
			texture_limits[0].autoscale.min.all = results.globalHits.texture_limits[0].min.all;
			texture_limits[0].autoscale.max.all = results.globalHits.texture_limits[0].max.all;
			break;
		}

		if (!texAutoScale) { //manual values
			min = texture_limits[textureMode].manual.min.all;
			max = texture_limits[textureMode].manual.max.all;
		}
		else { //autoscale
			min = texAutoScaleIncludeConstantFlow ?
				texture_limits[textureMode].autoscale.min.all
				: texture_limits[textureMode].autoscale.min.moments_only;
			max = texAutoScaleIncludeConstantFlow ?
				texture_limits[textureMode].autoscale.max.all
				: texture_limits[textureMode].autoscale.max.moments_only;
		}
	}

	for (size_t i = 0; i < sh.nbFacet; i++) {
		int time = SDL_GetTicks();
		if (!prg->IsVisible() && ((time - startTime) > 500)) {
			prg->SetVisible(true);
		}
		prg->SetProgress((double)i / (double)sh.nbFacet);
		Facet *f = facets[i];

		size_t profSize = (f->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice)) : 0;
		size_t nbElem = f->sh.texWidth*f->sh.texHeight;
		size_t tSize = nbElem * sizeof(TextureCell);

		if (renderRegularTexture && f->sh.isTextured) {

			GLint max_t;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_t);
			if (f->sh.texHeight > max_t || f->sh.texWidth > max_t) {
				if (!f->textureError) {
					char tmp[1024];
					sprintf(tmp, "Facet #%zd has a texture of %zdx%zd cells.\n"
						"Your video card only supports texture dimensions (width or height) up to %d cells.\n"
						"Texture rendering has been disabled on this facet, but you can still read texture values\n"
						"using the Texture Plotter window. Consider using a smaller mesh resolution, or split the facet\n"
						"into smaller parts. (Use Facet/Explode... command)", i + 1, f->sh.texHeight, f->sh.texWidth, max_t);
					GLMessageBox::Display(tmp, "OpenGL Error", GLDLG_OK, GLDLG_ICONWARNING);
				}
				f->textureError = true;
				return;
			}
			else {
				f->textureError = false;
			}

			// Retrieve texture from shared memory (every seconds)
			//TextureCell *hits_local = (TextureCell *)((BYTE *)shGHit + (f->sh.hitOffset + facetHitsSize + profSize*(1 + nbMoments) + tSize*mApp->worker.displayedMoment));
			f->BuildTexture(results.facetStates[i].momentResults[mApp->worker.displayedMoment].texture, textureMode, min, max, texColormap,
				dCoef_custom[0] * timeCorrection, dCoef_custom[1] * timeCorrection, dCoef_custom[2] * timeCorrection, texLogScale, mApp->worker.displayedMoment);
		}

		if (renderDirectionTexture && f->sh.countDirection && f->dirCache) {
			
			size_t dSize = nbElem * sizeof(DirectionCell);

			/*
			double iDesorbed = 0.0;
			if (results.globalHits.globalHits.nbDesorbed)
			iDesorbed = 1.0 / (double)results.globalHits.globalHits.nbDesorbed;
			*/
						
			//DirectionCell *dirs = (DirectionCell *)((BYTE *)shGHit + (f->sh.hitOffset + facetHitsSize + profSize*(1 + nbMoments) + tSize*(1 + nbMoments) + dSize*mApp->worker.displayedMoment));
			std::vector<DirectionCell>& dirs = results.facetStates[i].momentResults[mApp->worker.displayedMoment].direction;
			for (size_t j = 0; j < nbElem; j++) {
				double denominator = (dirs[j].count > 0) ? 1.0 / dirs[j].count : 1.0;
				f->dirCache[j].dir = dirs[j].dir * denominator;
				f->dirCache[j].count = dirs[j].count;
			}
		}
	}

	prg->SetVisible(false);
	SAFE_DELETE(prg);
}

