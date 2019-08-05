//
// Created by pascal on 8/2/19.
//

#include <cstring>
#include <iomanip> //setprecision
#include <sstream>
#include <math.h>
#define _USE_MATH_DEFINES
#include "SuperFacet.h"

/**
* \brief Fill vertex array
* \param v Array of vertices
*/
void SuperFacet::FillVertexArray(InterfaceVertex *v) {

	int nb = 0;
	for (size_t i = 0;i < sh.texHeight * sh.texWidth; i++) {
		if (cellPropertiesIds[i] != -2) {
			for (size_t j = 0; j < GetMeshNbPoint(i); j++) {
				Vector2d p = GetMeshPoint(i, j);
				v[nb].x = sh.O.x + sh.U.x * p.u + sh.V.x * p.v;
				v[nb].y = sh.O.y + sh.U.y * p.u + sh.V.y * p.v;
				v[nb].z = sh.O.z + sh.U.z * p.u + sh.V.z * p.v;
				nb++;
			}
		}
	}

}

/**
* \brief Get number of texture cells
* \return number of texture cells
*/
size_t SuperFacet::GetNbCell() {
	return sh.texHeight * sh.texWidth;
}

/**
* \brief Get number of cells calculated with a size ratio
* \param ratio ratio used for size conversion
* \return number of texture cells
*/
size_t SuperFacet::GetNbCellForRatio(double ratio) {

	double nU = sh.U.Norme();
	double nV = sh.V.Norme();
	double width = nU*ratio;
	double height = nV*ratio;

	bool dimOK = (width*height > 0.0000001);

	if (dimOK) {
		int iwidth = (int)ceil(width);
		int iheight = (int)ceil(height);
		return iwidth*iheight;
	}
	else {

		return 0;
	}

}

/**
* \brief Detect non visible edge (for polygon which contains holes)
*/
void SuperFacet::InitVisibleEdge() {

	// Detect non visible edge (for polygon which contains holes)
	std::__1::fill(visible.begin(), visible.end(), true);

	for (int i = 0;i < sh.nbIndex; i++) {

		size_t p11 = GetIndex(i);
		size_t p12 = GetIndex(i + 1);

		for (size_t j = i + 1;j < sh.nbIndex; j++) {

			size_t p21 = GetIndex((int)j);
			size_t p22 = GetIndex((int)j + 1);

			if ((p11 == p22 && p12 == p21) || (p11 == p21 && p12 == p22)) {
				// Invisible edge found
				visible[i] = false;
				visible[j] = false;
			}
		}
	}
}

/**
* \brief Get vertex index from buffer for an idx
* \param idx index
* \return vertex index
*/
size_t SuperFacet::GetIndex(int idx) {
	if (idx < 0) {
		return indices[(sh.nbIndex + idx) % sh.nbIndex];
	}
	else {
		return indices[idx % sh.nbIndex];
	}
}

/**
* \brief Get vertex index from buffer for an idx
* \param idx index
* \return vertex index
*/
size_t SuperFacet::GetIndex(size_t idx) {
		return indices[idx % sh.nbIndex];
}

/**
* \brief Calculate mesh area and consider the usage of 2 sided meshes
* \param index cell index
* \param correct2sides if correction for 2 sided meshes should be applied (use factor 2)
* \return mesh area
*/
double SuperFacet::GetMeshArea(size_t index, bool correct2sides) {
	if (!cellPropertiesIds) return -1.0f;
	if (cellPropertiesIds[index] == -1) {
		return ((correct2sides && sh.is2sided) ? 2.0 : 1.0) / (tRatio * tRatio);
	}
	else if (cellPropertiesIds[index] == -2) {
		return 0.0;
	}
	else {
		return ((correct2sides && sh.is2sided) ? 2.0 : 1.0) * meshvector[cellPropertiesIds[index]].area;
	}
}

/**
* \brief Get number of mesh points depending on if its describing a full element, outside of polygon or not
* \param index of mesh
* \return number of mesh points
*/
size_t SuperFacet::GetMeshNbPoint(size_t index) {
	size_t nbPts;
	if (cellPropertiesIds[index] == -1) nbPts = 4;
	else if (cellPropertiesIds[index] == -2) nbPts = 0;
	else nbPts = meshvector[cellPropertiesIds[index]].nbPoints;
	return nbPts;
}

/**
* \brief Get the uv coordinate of a point in a mesh
* \param index of mesh
* \param pointId id of the point in the mesh
* \return Vector for mesh point
*/
Vector2d SuperFacet::GetMeshPoint(size_t index, size_t pointId) {
	Vector2d result;
	if (!cellPropertiesIds) {
		result.u = 0.0;
		result.v = 0.0;
		return result;
	}
	else {
		int id = cellPropertiesIds[index];
		if (id == -2) {
			result.u = 0.0;
			result.v = 0.0;
			return result;
		}
		else if (id != -1) {
			if (pointId < meshvector[id].nbPoints)
				return meshvector[id].points[pointId];
			else {
				result.u = 0.0;
				result.v = 0.0;
				return result;
			}

		}

		else { //full elem
			double iw = 1.0 / (double) sh.texWidthD;
			double ih = 1.0 / (double) sh.texHeightD;
			double sx = (double)(index % sh.texWidth);
			double sy = (double)(index / sh.texWidth);
			if (pointId == 0) {
				double u0 = sx * iw;
				double v0 = sy * ih;
				result.u = u0;
				result.v = v0;
				return result;
			}
			else if (pointId == 1) {
				double u1 = (sx + 1.0) * iw;
				double v0 = sy * ih;
				result.u = u1;
				result.v = v0;
				return result;
			}
			else if (pointId == 2) {
				double u1 = (sx + 1.0) * iw;
				double v1 = (sy + 1.0) * ih;
				result.u = u1;
				result.v = v1;
				return result;
			}
			else if (pointId == 3) {
				double u0 = sx * iw;
				double v1 = (sy + 1.0) * ih;
				result.u = u0;
				result.v = v1;
				return result;
			}
			else {
				result.u = 0.0;
				result.v = 0.0;
				return result;
			}
		}
	}
}

/**
* \brief Get the uv coordinate of the central point in a mesh
* \param index of mesh
* \return Vector for point in the center of a mesh
*/
Vector2d SuperFacet::GetMeshCenter(size_t index) {
	Vector2d result;
	if (!cellPropertiesIds) {
		result.u = 0.0;
		result.v = 0.0;
		return result;
	}
	if (cellPropertiesIds[index] != -1) {
		if (cellPropertiesIds[index] == -2) {
			result.u = 0.0;
			result.v = 0.0;
			return result;
		}
		else {
			result.u = meshvector[cellPropertiesIds[index]].uCenter;
			result.v = meshvector[cellPropertiesIds[index]].vCenter;
			return result;
		}
	}
	else {
		double iw = 1.0 / (double) sh.texWidthD;
		double ih = 1.0 / (double) sh.texHeightD;
		double sx = (double)(index % sh.texWidth);
		double sy = (double)(index / sh.texWidth);
		double u0 = sx * iw;
		double v0 = sy * ih;
		double u1 = (sx + 1.0) * iw;
		double v1 = (sy + 1.0) * ih;
		result.u = (float)(u0 + u1) / 2.0f;
		result.v = (float)(v0 + v1) / 2.0f;
		return result;
	}
}

/**
* \brief Get calculated area of a facet (depends on one or double sided)
* \return facet area
*/
double SuperFacet::GetArea() {
	return sh.area * (sh.is2sided ? 2.0 : 1.0);
}

/**
* \brief Calculates the geometry size for a single facet which is necessary for loader dataport
* \return calculated size of the facet geometry
*/
size_t SuperFacet::GetGeometrySize()  { //for loader dataport

	size_t s = sizeof(FacetProperties)
               + (sh.nbIndex * sizeof(size_t)) //indices
		+ (sh.nbIndex * sizeof(Vector2d));

	// Size of the 'element area' array passed to the geometry buffer
	if (sh.isTextured) s += sizeof(double) * sh.texWidth * sh.texHeight; //incbuff
	if (sh.useOutgassingFile ) s += sizeof(double) * sh.outgassingMapWidth * sh.outgassingMapHeight;
	s += sizeof(size_t) * angleMapCache.size();
	return s;

}

/**
* \brief Calculates the hits size for a single facet which is necessary for hits dataport
* \param nbMoments amount of moments
* \return calculated size of the facet hits
*/
size_t SuperFacet::GetHitsSize(size_t nbMoments)  { //for hits dataport

	return   (1 + nbMoments)*(
		sizeof(FacetHitBuffer) +
		+(sh.texWidth * sh.texHeight * sizeof(TextureCell))
		+ (sh.isProfile ? (PROFILE_SIZE * sizeof(ProfileSlice)) : 0)
		+ (sh.countDirection ? (sh.texWidth * sh.texHeight * sizeof(DirectionCell)) : 0)
		+ sh.facetHistogramParams.GetDataSize()
		) + sizeof(size_t) * angleMapCache.size();

}

/**
* \brief Function that retrieves the angle map as a String
* \param formatId ID that describes the seperator for the angle map string
* \return string describing the angle map
*/
std::string SuperFacet::GetAngleMap(size_t formatId)
{
	std::stringstream result; result << std::setprecision(8);
	char separator;
	if (formatId == 1)
		separator = ',';
	else if (formatId == 2)
		separator = '\t';
	else return "";
	//First row: phi labels
	result << "Theta below / Phi to the right" << separator; //A1 cell
	for (size_t i = 0; i < sh.anglemapParams.phiWidth; i++)
		result << -M_PI + (0.5 + (double)i) / ((double) sh.anglemapParams.phiWidth) * 2.0 * M_PI << separator;
	result << "\n";

	//Actual table
	for (size_t row = 0; row < (sh.anglemapParams.thetaLowerRes + sh.anglemapParams.thetaHigherRes); row++) {
		//First column: theta label
		if (row < sh.anglemapParams.thetaLowerRes)
			result << ((double)row + 0.5) / (double) sh.anglemapParams.thetaLowerRes * sh.anglemapParams.thetaLimit << separator;
		else
			result << sh.anglemapParams.thetaLimit + (0.5 + (double)(row -
                                                                            sh.anglemapParams.thetaLowerRes)) / (double) sh.anglemapParams.thetaHigherRes * (M_PI / 2.0 -
                                                                                                                                                                           sh.anglemapParams.thetaLimit) << separator;
		//Value
		for (size_t col = 0; col < sh.anglemapParams.phiWidth; col++) {
			result << angleMapCache[row * sh.anglemapParams.phiWidth + col] << separator;
		}
		result << "\n";
	}
	return result.str();
}