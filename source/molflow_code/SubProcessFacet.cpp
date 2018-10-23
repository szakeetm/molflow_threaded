#include "Simulation.h"

void SubprocessFacet::InitializeOnLoad(size_t nbStruct) {

	InitializeLinkAndVolatile(nbStruct);
	InitializeOutgassingMap();
	InitializeAngleMap();
	InitializeTexture();
}

/*
void SubprocessFacet::InitializeHistogram()
{
	FacetHistogramBuffer hist;
	if (sh.facetHistogramParams.recordBounce) hist.nbHitsHistogram.resize(sh.facetHistogramParams.GetBounceHistogramSize());
	if (sh.facetHistogramParams.recordDistance) 	hist.distanceHistogram.resize(sh.facetHistogramParams.GetDistanceHistogramSize());
	if (sh.facetHistogramParams.recordTime) 	hist.timeHistogram.resize(sh.facetHistogramParams.GetTimeHistogramSize());

	tmpHistograms = std::vector<FacetHistogramBuffer>(1 + sHandle->moments.size(), hist);
	sHandle->histogramTotalSize += (1 + sHandle->moments.size()) *
		(sh.facetHistogramParams.GetBouncesDataSize()
		+ sh.facetHistogramParams.GetDistanceDataSize()
		+ sh.facetHistogramParams.GetTimeDataSize());
}

bool SubprocessFacet::InitializeDirectionTexture()
{
	//Direction
	if (sh.countDirection) {
		directionSize = sh.texWidth*sh.texHeight * sizeof(DirectionCell);
		try {
			direction = std::vector<std::vector<DirectionCell>>(1 + sHandle->moments.size(), std::vector<DirectionCell>(sh.texWidth*sh.texHeight));
		}
		catch (...) {
			SetErrorSub("Not enough memory to load direction textures");
			return false;
		}
		sHandle->dirTotalSize += directionSize * (1 + sHandle->moments.size());
	}
	else directionSize = 0;
	return true;
}

bool SubprocessFacet::InitializeProfile()
{
	//Profiles
	if (sh.isProfile) {
		profileSize = PROFILE_SIZE * sizeof(ProfileSlice);
		try {
			profile = std::vector<std::vector<ProfileSlice>>(1 + sHandle->moments.size(), std::vector<ProfileSlice>(PROFILE_SIZE));
		}
		catch (...) {
			SetErrorSub("Not enough memory to load profiles");
			return false;
		}
		sHandle->profTotalSize += profileSize * (1 + sHandle->moments.size());
	}
	else profileSize = 0;
	return true;
}

*/

bool SubprocessFacet::InitializeTexture()
{
	//Textures
	if (facetRef->sh.isTextured) {
		size_t nbE = facetRef->sh.texWidth*facetRef->sh.texHeight;
		largeEnough.resize(nbE);
		textureCellIncrements.resize(nbE);
		double fullCellIncrement = (facetRef->sh.texWidthD * facetRef->sh.texHeightD) / (facetRef->sh.U.Norme() * facetRef->sh.V.Norme());
		for (size_t j = 0; j < nbE; j++) {
			if (facetRef->cellPropertiesIds) {
				double area = facetRef->GetMeshArea(j, true);
				textureCellIncrements[j] = (area > 0.0) ? 1.0 / area : 0.0;
			}
			else {
				textureCellIncrements[j] = fullCellIncrement;
			}
		}
		for (size_t j = 0; j < nbE; j++) { //second pass, filter out very small cells
			largeEnough[j] = textureCellIncrements[j] < (5.0*fullCellIncrement);
		}

		iw = 1.0 / (double)facetRef->sh.texWidthD;
		ih = 1.0 / (double)facetRef->sh.texHeightD;
		rw = facetRef->sh.U.Norme() * iw;
		rh = facetRef->sh.V.Norme() * ih;
	}
	return true;
}

void SubprocessFacet::InitializeAngleMap()
{
	if (facetRef->sh.desorbType == DES_ANGLEMAP) { //Generate mode
		if (facetRef->angleMapCache.empty()) throw Error(("Facet " + std::to_string(globalId + 1) + ": should generate by angle map but has none recorded.").c_str());
		else generatingAngleMap.pdf = facetRef->angleMapCache; //Copy from interface cache, and now construct generator CDFs
		//Construct CDFs				
		try {
			generatingAngleMap.phi_CDFsums = std::vector<size_t>(facetRef->sh.anglemapParams.thetaLowerRes + facetRef->sh.anglemapParams.thetaHigherRes);
		}
		catch (...) {
			throw Error("Not enough memory to load incident angle map (phi CDF line sums)");
		}
		try {
			generatingAngleMap.theta_CDF.resize(facetRef->sh.anglemapParams.thetaLowerRes + facetRef->sh.anglemapParams.thetaHigherRes);
		}
		catch (...) {
			throw Error("Not enough memory to load incident angle map (line sums, CDF)");
		}
		try {
			generatingAngleMap.phi_CDFs.resize(facetRef->sh.anglemapParams.phiWidth * (facetRef->sh.anglemapParams.thetaLowerRes + facetRef->sh.anglemapParams.thetaHigherRes));
		}
		catch (...) {
			throw Error("Not enough memory to load incident angle map (CDF)");
		}

		//First pass: determine sums
		for (size_t thetaIndex = 0; thetaIndex < (facetRef->sh.anglemapParams.thetaLowerRes + facetRef->sh.anglemapParams.thetaHigherRes); thetaIndex++) {
			for (size_t phiIndex = 0; phiIndex < facetRef->sh.anglemapParams.phiWidth; phiIndex++) {
				generatingAngleMap.phi_CDFsums[thetaIndex] += generatingAngleMap.pdf[thetaIndex*facetRef->sh.anglemapParams.phiWidth + phiIndex];
			}
			generatingAngleMap.theta_CDFsum += generatingAngleMap.phi_CDFsums[thetaIndex];
		}
		if (generatingAngleMap.theta_CDFsum == 0) {
			std::stringstream err; err << "Facet " << globalId + 1 << " has all-zero recorded angle map.";
			throw Error(err.str().c_str());
		}

		//Second pass: write CDFs
		double thetaNormalizingFactor = 1.0 / (double)generatingAngleMap.theta_CDFsum;
		for (size_t thetaIndex = 0; thetaIndex < (facetRef->sh.anglemapParams.thetaLowerRes + facetRef->sh.anglemapParams.thetaHigherRes); thetaIndex++) {
			if (generatingAngleMap.theta_CDFsum == 0) { //no hits in this line, generate CDF of uniform distr.
				generatingAngleMap.theta_CDF[thetaIndex] = (0.5 + (double)thetaIndex) / (double)(facetRef->sh.anglemapParams.thetaLowerRes + facetRef->sh.anglemapParams.thetaHigherRes);
			}
			else {
				if (thetaIndex == 0) {
					//First CDF value, covers half of first segment
					generatingAngleMap.theta_CDF[thetaIndex] = 0.5 * (double)generatingAngleMap.phi_CDFsums[0] * thetaNormalizingFactor;
				}
				else {
					//value covering second half of last segment and first of current segment
					generatingAngleMap.theta_CDF[thetaIndex] = generatingAngleMap.theta_CDF[thetaIndex - 1] + (double)(generatingAngleMap.phi_CDFsums[thetaIndex - 1] + generatingAngleMap.phi_CDFsums[thetaIndex])*0.5*thetaNormalizingFactor;
				}
			}
			double phiNormalizingFactor = 1.0 / (double)generatingAngleMap.phi_CDFsums[thetaIndex];
			for (size_t phiIndex = 0; phiIndex < facetRef->sh.anglemapParams.phiWidth; phiIndex++) {
				size_t index = facetRef->sh.anglemapParams.phiWidth * thetaIndex + phiIndex;
				if (generatingAngleMap.phi_CDFsums[thetaIndex] == 0) { //no hits in this line, create CDF of uniform distr.
					generatingAngleMap.phi_CDFs[index] = (0.5 + (double)phiIndex) / (double)facetRef->sh.anglemapParams.phiWidth;
				}
				else {
					if (phiIndex == 0) {
						//First CDF value, covers half of first segment
						generatingAngleMap.phi_CDFs[index] = 0.5 * (double)generatingAngleMap.pdf[facetRef->sh.anglemapParams.phiWidth * thetaIndex] * phiNormalizingFactor;
					}
					else {
						//value covering second half of last segment and first of current segment
						generatingAngleMap.phi_CDFs[index] = generatingAngleMap.phi_CDFs[facetRef->sh.anglemapParams.phiWidth * thetaIndex + phiIndex - 1] + (double)(generatingAngleMap.pdf[facetRef->sh.anglemapParams.phiWidth * thetaIndex + phiIndex - 1] + generatingAngleMap.pdf[facetRef->sh.anglemapParams.phiWidth * thetaIndex + phiIndex])*0.5*phiNormalizingFactor;
					}
				}
			}
		}
	}
}

void SubprocessFacet::InitializeOutgassingMap()
{
	if (facetRef->sh.useOutgassingFile) {
		//Precalc actual outgassing map width and height for faster generation:
		outgassingMapWidthD = (size_t)(facetRef->sh.U.Norme() * facetRef->sh.outgassingFileRatio);
		outgassingMapHeightD = (size_t)(facetRef->sh.V.Norme() * facetRef->sh.outgassingFileRatio);
		size_t nbE = facetRef->sh.outgassingMapWidth*facetRef->sh.outgassingMapHeight;
		for (size_t i = 1; i < nbE; i++) {
			outgassingMapCdf[i] += outgassingMapCdf[i - 1] + facetRef->outgassingMap[i]; //Convert p.d.f to cumulative distr. 
		}
	}
}

void SubprocessFacet::InitializeLinkAndVolatile(size_t nbStruct)
{
	if (facetRef->sh.superDest) {
		// Link or volatile facet, overides facet settings
		// Must be full opaque and 0 sticking
		// (see SimulationMC.c::PerformBounce)
		//sh.isOpaque = true;
		facetRef->sh.opacity = 1.0;
		facetRef->sh.opacity_paramId = -1;
		facetRef->sh.sticking = 0.0;
		facetRef->sh.sticking_paramId = -1;
		if (((facetRef->sh.superDest - 1) >= nbStruct || facetRef->sh.superDest < 0)) {
			std::ostringstream err;
			err << "Invalid structure (wrong link on F#" << globalId + 1 << ")";
			throw Error(err.str().c_str());
		}
	}
}