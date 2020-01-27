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
#include "MolflowGeometry.h"
#include "MolFlow.h"
#include "Facet_shared.h"
#include "GLApp/MathTools.h"
#include "ProfilePlotter.h"
#include <iomanip>

#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

/*
//Leak detection
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
*/

using namespace pugi;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

/**
* \brief Basic constructor that initializes a clean (none) geometry
*/
MolflowGeometry::MolflowGeometry() {

	texAutoScaleIncludeConstantFlow = true;

	Clear(); //Contains resettexturelimits

}

/**
* \brief Calculates the memory size for the whole geometry
* \return calculated memory usage for the whole geometry
*/
size_t MolflowGeometry::GetGeometrySize() {

	Worker  *work = &mApp->worker;

	// Compute number of bytes allocated
	size_t memoryUsage = 0;
	memoryUsage += sizeof(GeomProperties);
	memoryUsage += sizeof(OntheflySimulationParams);
	memoryUsage += sh.nbVertex * sizeof(Vector3d);
	for (int i = 0; i < sh.nbFacet; i++)
		memoryUsage += facets[i]->GetGeometrySize();

	//CDFs
	memoryUsage += sizeof(size_t); //number of CDFs
	for (auto& i : work->CDFs) {
		memoryUsage += sizeof(size_t); //CDF size
		memoryUsage += i.size() * 2 * sizeof(double);
	}

	//IDs
	memoryUsage += sizeof(size_t); //number of IDs
	for (auto& i : work->IDs) {

		memoryUsage += sizeof(size_t); //ID size
		memoryUsage += i.size() * 2 * sizeof(double);
	}

	//Parameters
	memoryUsage += sizeof(size_t); //number of parameters
	for (auto& i : work->parameters) {

		memoryUsage += sizeof(size_t); //parameter size

		memoryUsage += i.GetSize() * 2 * sizeof(double);
	}
	memoryUsage += sizeof(size_t); //number of temperatures
	memoryUsage += sizeof(double)*(int)(work->temperatures).size(); //temperatures

	//moments size already passed
	memoryUsage += sizeof(double)*(int)(work->moments).size(); //moments

	memoryUsage += sizeof(size_t); //number of desparamIDs
	memoryUsage += sizeof(size_t)*(int)(work->desorptionParameterIDs).size(); //desparamIDs
	return memoryUsage;
}

/**
* \brief Builds a shared buffer for the geometry and copies necessary values into it (TODO: check if it unused)
* \param buffer buffer
* \param ontheflyParams parameters that can be changed without restarting the simulation
*/
void MolflowGeometry::CopyGeometryBuffer(BYTE *buffer, const OntheflySimulationParams& ontheflyParams) {

	// Build shared buffer for geometry (see Shared.h)
	// Basically we serialize all data and let the subprocesses read them

	/*
	Memory map:

	-->bufferStart
	SHGEOM (nbFacets, time-dep parameters, gas mass, etc.)
	OntheflyParams
	vertices3 (nbVertex times Vector3d struct)
	FOR EACH FACET
	SHFACET
	indices (nbIndex times int)
	vertices2 (nbIndex times Vector2d struct)
	[outgassingMap (height*width*double)]
	[angleMap (height*width*size_t)]
	[inc Map: for each facet with texture, height*width*double]
	-->globalBuff
	CDFs.size()
	CDFs
	IDs.size()
	[IDs]
	parameters.size()
	[parameters]
	temperatures.size()
	temperatures

	*/

	Worker *w = &mApp->worker;
	WRITEBUFFER(sh, GeomProperties);
	WRITEBUFFER(w->wp, WorkerParams);
	/*GeomProperties *shGeom = (GeomProperties *)buffer;
	memcpy(shGeom, &(this->wp), sizeof(GeomProperties));
	buffer += sizeof(GeomProperties);*/
	WRITEBUFFER(ontheflyParams, OntheflySimulationParams);
	memcpy(buffer, vertices3.data(), sizeof(Vector3d)*sh.nbVertex);
	buffer += sizeof(Vector3d)*sh.nbVertex;

	size_t fOffset = sizeof(GlobalHitBuffer) + (1 + mApp->worker.moments.size())*mApp->worker.wp.globalHistogramParams.GetDataSize(); //calculating offsets for all facets for the hits dataport during the simulation
	for (size_t i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		f->sh.hitOffset = fOffset; //Marking the offsets for the hits, but here we don't actually send any hits.
		fOffset += f->GetHitsSize(mApp->worker.moments.size());
		WRITEBUFFER(f->sh, FacetProperties);
		memcpy(buffer, f->indices.data(), sizeof(size_t)*f->sh.nbIndex);
		buffer += sizeof(size_t)*f->sh.nbIndex;
		memcpy(buffer, f->vertices2.data(), sizeof(Vector2d)*f->sh.nbIndex);
		buffer += sizeof(Vector2d)*f->sh.nbIndex;
		if (f->sh.useOutgassingFile) {
			memcpy(buffer, f->outgassingMap.data(), sizeof(double)*f->sh.outgassingMapWidth*f->sh.outgassingMapHeight);
			buffer += sizeof(double)*f->sh.outgassingMapWidth*f->sh.outgassingMapHeight;
		}
		//if (f->wp.anglemapParams.hasRecorded) { //Check not necessary, commented out
		memcpy(buffer, f->angleMapCache.data(), sizeof(size_t)*f->angleMapCache.size());
		buffer += sizeof(size_t)*f->angleMapCache.size();
		//}

		// Add surface elements area (reciprocal)
		if (f->sh.isTextured) {
			if (f->cellPropertiesIds) {
				size_t add = 0;
				for (int j = 0; j < f->sh.texHeight; j++) {
					for (int i = 0; i < f->sh.texWidth; i++) {
						double area = f->GetMeshArea(add, true);

						if (area > 0.0) {
							// Use the sign bit to store isFull flag
							WRITEBUFFER(1.0 / area, double);
						}
						else {
							WRITEBUFFER(0.0, double);
						}
						add++;
					}
				}
			}
			else {

				double rw = f->sh.U.Norme() / (double)(f->sh.texWidthD);
				double rh = f->sh.V.Norme() / (double)(f->sh.texHeightD);
				double area = rw * rh;

				for (int j = 0; j < f->sh.texHeight; j++) {
					for (int i = 0; i < f->sh.texWidth; i++) {
						if (area > 0.0) {
							WRITEBUFFER(1.0 / area, double);
						}
						else {
							WRITEBUFFER(0.0, double);
						}
					}
				}
			}
		}
	}

	//CDFs
	WRITEBUFFER(w->CDFs.size(), size_t);
	for (size_t i = 0; i < w->CDFs.size(); i++) {
		WRITEBUFFER(w->CDFs[i].size(), size_t);
		for (size_t j = 0; j < w->CDFs[i].size(); j++) {
			WRITEBUFFER(w->CDFs[i][j].first, double);
			WRITEBUFFER(w->CDFs[i][j].second, double);
		}
	}

	//IDs
	WRITEBUFFER(w->IDs.size(), size_t);
	for (size_t i = 0; i < w->IDs.size(); i++) {
		WRITEBUFFER(w->IDs[i].size(), size_t);
		for (size_t j = 0; j < w->IDs[i].size(); j++) {
			WRITEBUFFER(w->IDs[i][j].first, double);
			WRITEBUFFER(w->IDs[i][j].second, double);
		}
	}

	//Parameters
	/*WRITEBUFFER(w->parameters.size(), size_t);
	  for (size_t i = 0; i < w->parameters.size(); i++) {
	  WRITEBUFFER(w->parameters[i].values.size(), size_t);
	  for (size_t j=0;j<w->parameters[i].values.size();j++) {
	  WRITEBUFFER(w->parameters[i].values[j].first, double);
	  WRITEBUFFER(w->parameters[i].values[j].second, double);
	  }
	  }*/
	WRITEBUFFER(w->parameters.size(), size_t);
	for (auto& par : w->parameters) {
		size_t paramSize = par.GetSize();
		WRITEBUFFER(paramSize, size_t);
		for (int j = 0; j < paramSize; j++) {
			WRITEBUFFER(par.GetX(j), double);
			WRITEBUFFER(par.GetY(j), double);
		}
	}

	//Temperatures
	WRITEBUFFER(w->temperatures.size(), size_t);
	for (size_t i = 0; i < w->temperatures.size(); i++) {
		WRITEBUFFER(w->temperatures[i], double);
	}

	//Time moments
	//WRITEBUFFER(w->moments.size(), size_t); //nbMoments already passed
	for (size_t i = 0; i < w->moments.size(); i++) {
		WRITEBUFFER(w->moments[i], double);
	}

	//Desorption parameter IDs
	WRITEBUFFER(w->desorptionParameterIDs.size(), size_t);
	for (size_t i = 0; i < w->desorptionParameterIDs.size(); i++) {
		WRITEBUFFER(w->desorptionParameterIDs[i], size_t);
	}
}

/**
* \brief Serializes data of the complete geometry into a cereal binary archive
* \param outputarchive reference to the binary archive
*/
void MolflowGeometry::SerializeForLoader(cereal::BinaryOutputArchive& outputArchive) {
	outputArchive(
		CEREAL_NVP(sh),
		CEREAL_NVP(vertices3)
	);

	size_t fOffset = sizeof(GlobalHitBuffer) + (1 + mApp->worker.moments.size())*mApp->worker.wp.globalHistogramParams.GetDataSize(); //calculating offsets for all facets for the hits dataport during the simulation

	for (size_t i = 0; i < sh.nbFacet; i++) {
		facets[i]->sh.hitOffset = fOffset; //Marking the offsets for the hits, but here we don't actually send any hits.
		fOffset += facets[i]->GetHitsSize(mApp->worker.moments.size());
		facets[i]->SerializeForLoader(outputArchive);
	}
}

/**
* \brief Compute number of bytes allocated from the hits size of all facets
* \param moments vector containing all moments
* \return calculated size of memory usage from all facet hits in the geometry
*/
size_t MolflowGeometry::GetHitsSize(std::vector<double> *moments) {

	// Compute number of bytes allocated
	size_t memoryUsage = 0;
	memoryUsage += sizeof(GlobalHitBuffer) + (1 + moments->size())*mApp->worker.wp.globalHistogramParams.GetDataSize();
	for (int i = 0; i < sh.nbFacet; i++) {
		memoryUsage += facets[i]->GetHitsSize(moments->size());
	}

	return memoryUsage;
}

/**
* \brief Compute the maximal (surface) element number (TODO: check if unused)
* \return max element number
*/
size_t MolflowGeometry::GetMaxElemNumber() {

	size_t nbElem = 0;
	for (size_t i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->cellPropertiesIds) nbElem += f->sh.texWidth*f->sh.texHeight;
		else          return 0;
	}
	return nbElem;

}

/*void MolflowGeometry::CopyElemBuffer(BYTE *buffer) {

	int idx = 0;
	for (int i = 0; i < wp.nbFacet; i++) {
		Facet *f = facets[i];

		//int sz = f->wp.texWidth * f->wp.texHeight * sizeof(SHELEM);
		//memcpy(buffer + idx, f->mesh, sz);
		//idx += sz;

		//To fix
	}

}*/

/**
* \brief Testing purpose function, construct a PIPE
* \param L length
* \param R radius
* \param s sticking value
* \param step number of facets used to construct the circular hull
*/
void  MolflowGeometry::BuildPipe(double L, double R, double s, int step) {
	Clear();

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();

	int nbDecade = 0;
	int nbTF = 9 * nbDecade;
	int nbTV = 4 * nbTF;

	sh.nbVertex = 2 * step + nbTV;
	std::vector<InterfaceVertex>(sh.nbVertex).swap(vertices3);

	sh.nbFacet = step + 2 + nbTF;


	sh.nbSuper = 1;
	strName[0] = strdup("Pipe");
	//strFileName[0] = strdup("pipe.txt");

	if (!(facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *))))
		throw Error("Couldn't allocate memory for facets");
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));

	// Vertices
	for (int i = 0; i < step; i++) {
		double angle = (double)i / (double)step * 2 * PI;
		vertices3[2 * i + nbTV].x = R * cos(angle);
		vertices3[2 * i + nbTV].y = R * sin(angle);
		vertices3[2 * i + nbTV].z = 0.0;
		vertices3[2 * i + 1 + nbTV].x = R * cos(angle);
		vertices3[2 * i + 1 + nbTV].y = R * sin(angle);
		vertices3[2 * i + 1 + nbTV].z = L;
	}

	try {
		// Cap facet
		facets[0 + nbTF] = new Facet(step);
		facets[0 + nbTF]->sh.sticking = 1.0;
		facets[0 + nbTF]->sh.desorbType = DES_COSINE;
		facets[0 + nbTF]->sh.outgassing = 1.0;
		for (int i = 0; i < step; i++)
			facets[0 + nbTF]->indices[i] = 2 * i + nbTV;

		facets[1 + nbTF] = new Facet(step);
		facets[1 + nbTF]->sh.sticking = 1.0;
		facets[1 + nbTF]->sh.desorbType = DES_NONE;
		for (int i = 0; i < step; i++)
			facets[1 + nbTF]->indices[step - i - 1] = 2 * i + 1 + nbTV;

		// Wall facet
		for (int i = 0; i < step; i++) {
			facets[i + 2 + nbTF] = new Facet(4);
			//facets[i + 2 + nbTF]->wp.reflection.diffusePart = 1.0; //constructor does this already
			//facets[i + 2 + nbTF]->wp.reflection.specularPart = 0.0; //constructor does this already
			facets[i + 2 + nbTF]->sh.sticking = s;
			facets[i + 2 + nbTF]->indices[0] = 2 * i + nbTV;
			facets[i + 2 + nbTF]->indices[1] = 2 * i + 1 + nbTV;
			if (i < step - 1) {
				facets[i + 2 + nbTF]->indices[2] = 2 * (i + 1) + 1 + nbTV;
				facets[i + 2 + nbTF]->indices[3] = 2 * (i + 1) + nbTV;
			}
			else {

				facets[i + 2 + nbTF]->indices[2] = 1 + nbTV;
				facets[i + 2 + nbTF]->indices[3] = 0 + nbTV;
			}
		}
	}
	catch (std::bad_alloc) {
		Clear();
		throw Error("Couldn't reserve memory for the facets");
	}
	catch (...) {
		throw Error("Unspecified Error while building pipe");
	}
	InitializeGeometry();
	//isLoaded = true; //InitializeGeometry() sets to true

}

/**
* \brief File handling for inserting a SYN geometry + initialisation
* \param file name of the input file
* \param prg GLProgress (TODO: which is never used)
* \param newStr newStructure if a super structure will be used or not
*/
void MolflowGeometry::InsertSYN(FileReader *file, GLProgress *prg, bool newStr) {

	int structId = viewStruct;
	if (structId == -1) structId = 0;
	InsertSYNGeom(file, structId, newStr);
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	//AdjustProfile();

}

/**
* \brief Inserting the SYN geometry
* \param file name of the input file
* \param strIdx struct ID
* \param newStruct if a super structure will be used or not
*/
void MolflowGeometry::InsertSYNGeom(FileReader *file, size_t strIdx, bool newStruct) {

	UnselectAll();

	file->ReadKeyword("version"); file->ReadKeyword(":");
	int version2;
	version2 = file->ReadInt();
	if (version2 > SYNVERSION) {
		char errMsg[512];
		sprintf(errMsg, "Unsupported SYN version V%d", version2);
		throw Error(errMsg);
	}

	file->ReadKeyword("totalHit"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("totalDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version2 >= 6) {
		file->ReadKeyword("no_scans"); file->ReadKeyword(":");
		/*loaded_no_scans = */file->ReadDouble();
	}

	file->ReadKeyword("totalLeak"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version2 > 2) {
		file->ReadKeyword("totalFlux"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("totalPower"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("maxDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("nbVertex"); file->ReadKeyword(":");
	size_t nbNewVertex = file->ReadSizeT();
	file->ReadKeyword("nbFacet"); file->ReadKeyword(":");
	size_t nbNewFacets = file->ReadSizeT();
	file->ReadKeyword("nbSuper"); file->ReadKeyword(":");
	size_t nbNewSuper = file->ReadSizeT();
	file->ReadKeyword("nbFormula"); file->ReadKeyword(":");
	size_t nbF = file->ReadSizeT();
	file->ReadKeyword("nbView"); file->ReadKeyword(":");
	size_t nbV = file->ReadSizeT();
	file->ReadKeyword("nbSelection"); file->ReadKeyword(":");
	size_t nbS = file->ReadSizeT();
	if (version2 > 1) {
		file->ReadKeyword("nbRegions"); file->ReadKeyword(":");
		size_t nbR = file->ReadSizeT();
		file->ReadKeyword("PARfiles"); file->ReadKeyword("{");
		for (size_t i = 0; i < nbR; i++) {
			file->ReadString();
		}
		file->ReadKeyword("}");
	}

	file->ReadKeyword("formulas"); file->ReadKeyword("{");
	for (size_t i = 0; i < nbF; i++) {
		char tmpName[256];
		char tmpExpr[512];
		strcpy(tmpName, file->ReadString());
		strcpy(tmpExpr, file->ReadString());
		//mApp->AddFormula(tmpName,tmpExpr); //we don't add SynRad formulas to MolFlow
	}
	file->ReadKeyword("}");

	file->ReadKeyword("views"); file->ReadKeyword("{");
	for (size_t i = 0; i < nbV; i++) {
		char tmpName[256];
		AVIEW v;
		strcpy(tmpName, file->ReadString());
		v.projMode = file->ReadInt();
		v.camAngleOx = file->ReadDouble();
		v.camAngleOy = file->ReadDouble();
		v.camAngleOz = 0.0; //No support for Z angle in current SYN version
		v.camDist = file->ReadDouble();
		v.camOffset.x = file->ReadDouble();
		v.camOffset.y = file->ReadDouble();
		v.camOffset.z = file->ReadDouble();
		v.performXY = file->ReadInt();

		v.vLeft = file->ReadDouble();
		v.vRight = file->ReadDouble();
		v.vTop = file->ReadDouble();
		v.vBottom = file->ReadDouble();
		mApp->AddView(tmpName, v);
	}
	file->ReadKeyword("}");

	file->ReadKeyword("selections"); file->ReadKeyword("{");
	for (size_t i = 0; i < nbS; i++) {
		SelectionGroup s;
		char tmpName[256];
		strcpy(tmpName, file->ReadString());
		s.name = strdup(tmpName);
		size_t nbSel = file->ReadSizeT();
		for (size_t j = 0; j < nbSel; j++) {
			s.selection.push_back(file->ReadInt() + sh.nbFacet);
		}
		mApp->AddSelection(s);
	}
	file->ReadKeyword("}");

	file->ReadKeyword("structures"); file->ReadKeyword("{");
	for (size_t i = 0; i < nbNewSuper; i++) {
		strName[sh.nbSuper + i] = strdup(file->ReadString());
	}
	file->ReadKeyword("}");

	// Reallocate memory
	facets = (Facet **)realloc(facets, (nbNewFacets + sh.nbFacet) * sizeof(Facet **));
	memset(facets + sh.nbFacet, 0, nbNewFacets * sizeof(Facet *));

	/*
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + wp.nbVertex) * sizeof(InterfaceVertex));
	memmove(tmp_vertices3, *vertices3, (wp.nbVertex)*sizeof(Vector3d));
	memset(tmp_vertices3 + wp.nbVertex, 0, nbNewVertex * sizeof(Vector3d));
	SAFE_FREE(*vertices3);
	*vertices3 = tmp_vertices3;
	*/
	vertices3.resize(nbNewVertex + sh.nbVertex);

	// Read geometry vertices
	file->ReadKeyword("vertices"); file->ReadKeyword("{");
	for (size_t i = sh.nbVertex; i < (sh.nbVertex + nbNewVertex); i++) {
		// Check idx
		int idx = file->ReadInt();
		if (idx != i - sh.nbVertex + 1) throw Error(file->MakeError("Wrong vertex index !"));
		vertices3[i].x = file->ReadDouble();
		vertices3[i].y = file->ReadDouble();
		vertices3[i].z = file->ReadDouble();
		vertices3[i].selected = false;
	}
	file->ReadKeyword("}");

	// Read leaks
	file->ReadKeyword("leaks"); file->ReadKeyword("{");
	file->ReadKeyword("nbLeak"); file->ReadKeyword(":");
	size_t nbleak_local = file->ReadSizeT();
	for (size_t i = 0; i < nbleak_local; i++) {
		size_t idx = file->ReadSizeT();
		//if( idx != i ) throw Error(file->MakeError("Wrong leak index !"));
		file->ReadDouble();
		file->ReadDouble();
		file->ReadDouble();

		file->ReadDouble();
		file->ReadDouble();
		file->ReadDouble();
	}
	file->ReadKeyword("}");
	// Read hit cache

	file->ReadKeyword("hits"); file->ReadKeyword("{");
	file->ReadKeyword("nbHHit"); file->ReadKeyword(":");
	size_t nbHHit_local = file->ReadSizeT();
	for (size_t i = 0; i < nbHHit_local; i++) {
		size_t idx = file->ReadSizeT();
		//if( idx != i ) throw Error(file->MakeError("Wrong hit cache index !"));
		file->ReadDouble(); //x
		file->ReadDouble(); //y
		file->ReadDouble(); //z
		file->ReadDouble(); //dF
		file->ReadDouble(); //dP
		file->ReadInt();    //type
	}
	file->ReadKeyword("}");

	// Read geometry facets (indexed from 1)
	for (size_t i = sh.nbFacet; i < (sh.nbFacet + nbNewFacets); i++) {
		file->ReadKeyword("facet");
		// Check idx
		size_t idx = file->ReadSizeT();
		if (idx != i + 1 - sh.nbFacet) throw Error(file->MakeError("Wrong facet index !"));
		file->ReadKeyword("{");
		file->ReadKeyword("nbIndex");
		file->ReadKeyword(":");
		size_t nb = file->ReadSizeT();

		if (nb < 3) {
			char errMsg[512];
			sprintf(errMsg, "Facet %zd has only %zd vertices. ", i, nb);
			throw Error(errMsg);
		}

		facets[i] = new Facet(nb);
		facets[i]->LoadSYN(file, version2, nbNewVertex);
		facets[i]->selected = true;
		for (size_t j = 0; j < nb; j++)
			facets[i]->indices[j] += sh.nbVertex;
		file->ReadKeyword("}");
		if (newStruct) {
			if (facets[i]->sh.superIdx != -1) //-1 = facet member of all structures
				facets[i]->sh.superIdx += static_cast<int>(sh.nbSuper);
			if (facets[i]->sh.superDest > 0) facets[i]->sh.superDest += sh.nbSuper;
		}
		else {
			if (facets[i]->sh.superIdx != -1) //-1 = facet member of all structures
				facets[i]->sh.superIdx += static_cast<int>(strIdx);
			if (facets[i]->sh.superDest > 0) facets[i]->sh.superDest += strIdx;
		}
	}

	sh.nbVertex += nbNewVertex;
	sh.nbFacet += nbNewFacets;
	if (newStruct) sh.nbSuper += nbNewSuper;
	else if (sh.nbSuper < strIdx + nbNewSuper) sh.nbSuper = strIdx + nbNewSuper;
	//return result;
}

/**
* \brief For saving profile data (simulation) into GEO format
* \param file name of the output file
* \param results results from the simulation
* \param super TODO: check if truly unused
* \param saveSelected prevents profile from being saved?
* \param crashSave prevents profile from being saved?
* TODO: Function doesn't seem to cancel properly
*/
void MolflowGeometry::SaveProfileGEO(FileWriter *file, GlobalSimuState& results, int super, bool saveSelected, bool crashSave) {

	file->Write("profiles {\n");
	// Profiles
	int nbProfile = 0;
	int *profileFacet = (int *)malloc((sh.nbFacet) * sizeof(int));
	for (int i = 0; i < sh.nbFacet; i++)
		if ((!saveSelected && !crashSave) && facets[i]->sh.isProfile)
			profileFacet[nbProfile++] = i;

	file->Write(" number: "); file->Write(nbProfile, "\n");
	file->Write(" facets: ");
	for (int i = 0; i < nbProfile; i++) //doesn't execute when crashSave or saveSelected...
		file->Write(profileFacet[i], "\t");

	file->Write("\n");
	size_t facetHitsSize = (1 + mApp->worker.moments.size()) * sizeof(FacetHitBuffer);
	for (size_t m = 0; (m <= mApp->worker.moments.size()) || (m == 0); m++) {
		char tmp[128];
		sprintf(tmp, " moment %zd {\n", m);
		file->Write(tmp);

		for (int j = 0; j < PROFILE_SIZE; j++) {
			for (int i = 0; i < nbProfile; i++) { //doesn't execute when crashSave or saveSelected...
				Facet *f = GetFacet(profileFacet[i]);
				std::vector<ProfileSlice>& pr = results.facetStates[profileFacet[i]].momentResults[m].profile;
				//char tmp2[128];
				file->Write(static_cast<size_t>(pr[j].countEquiv), "\t"); //Backwards compatibility
				file->Write(pr[j].sum_1_per_ort_velocity, "\t");
				file->Write(pr[j].sum_v_ort);
				file->Write("\t");
			}

			if (nbProfile > 0) file->Write("\n");
		}
		file->Write(" }\n");
	}
	file->Write("}\n");
	SAFE_FREE(profileFacet);
}

/**
* \brief For loading profile data (simulation) from GEO format
* \param file name of the input file
* \param results results from the simulation
* \param version version of the GEO description
*/
void MolflowGeometry::LoadProfileGEO(FileReader *file, GlobalSimuState& results, int version) {

	LockMutex(results.mutex);
	file->ReadKeyword("profiles"); file->ReadKeyword("{");
	// Profiles
	int nbProfile;
	file->ReadKeyword("number"); file->ReadKeyword(":"); nbProfile = file->ReadInt();
	file->ReadKeyword("facets"); file->ReadKeyword(":");
	std::vector<int> profileFacet(nbProfile);
	for (int i = 0; i < nbProfile; i++)
		profileFacet[i] = file->ReadInt();
	size_t facetHitsSize = (1 + mApp->worker.moments.size()) * sizeof(FacetHitBuffer);
	for (size_t m = 0; m <= mApp->worker.moments.size() || (version < 10 && m == 0); m++) {
		if (version >= 10) {
			file->ReadKeyword("moment");
			if (m != file->ReadInt()) {
				throw Error("Unexpected profile moment");
				break;
			}
			file->ReadKeyword("{");
		}

		for (int j = 0; j < PROFILE_SIZE; j++) {
			for (int i = 0; i < nbProfile; i++) {
				Facet *f = GetFacet(profileFacet[i]);
				std::vector<ProfileSlice>& pr = results.facetStates[profileFacet[i]].momentResults[m].profile;
				pr[j].countEquiv = static_cast<double>(file->ReadSizeT());
				if (version >= 13) pr[j].sum_1_per_ort_velocity = file->ReadDouble();
				if (version >= 13) pr[j].sum_v_ort = file->ReadDouble();
			}
		}
		if (version >= 10) file->ReadKeyword("}");
	}
	ReleaseMutex(results.mutex);
}

/**
* \brief For loading geometry data from GEO format
* \param file name of the input file
* \param prg GLProgress window
* \param version version of the GEO description
* \param worker thread worker that executes the task
*/
void MolflowGeometry::LoadGEO(FileReader *file, GLProgress *prg, int *version, Worker *worker) {

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	prg->SetMessage("Clearing current geometry...");
	Clear();
	//mApp->ClearFormulas();

	// Globals
	char tmp[512];
	prg->SetMessage("Reading GEO file header...");
	file->ReadKeyword("version"); file->ReadKeyword(":");
	*version = file->ReadInt();
	if (*version > GEOVERSION) {
		char errMsg[512];
		sprintf(errMsg, "Unsupported GEO version V%d", *version);
		throw Error(errMsg);
	}

	file->ReadKeyword("totalHit"); file->ReadKeyword(":");
	worker->globalHitCache.globalHits.nbMCHit = file->ReadSizeT();
	worker->globalHitCache.globalHits.nbHitEquiv = static_cast<double>(worker->globalHitCache.globalHits.nbMCHit);

	file->ReadKeyword("totalDes"); file->ReadKeyword(":");
	worker->globalHitCache.globalHits.nbDesorbed = file->ReadSizeT();

	file->ReadKeyword("totalLeak"); file->ReadKeyword(":");
	worker->globalHitCache.nbLeakTotal = file->ReadSizeT();

	if (*version >= 12) {
		file->ReadKeyword("totalAbs"); file->ReadKeyword(":");
		worker->globalHitCache.globalHits.nbAbsEquiv = (double)file->ReadSizeT();
		if (*version >= 15) {
			file->ReadKeyword("totalDist_total");
		}
		else { //between versions 12 and 15
			file->ReadKeyword("totalDist");
		}
		file->ReadKeyword(":");
		worker->globalHitCache.distTraveled_total = file->ReadDouble();
		if (*version >= 15) {
			file->ReadKeyword("totalDist_fullHitsOnly"); file->ReadKeyword(":");
			worker->globalHitCache.distTraveledTotal_fullHitsOnly = file->ReadDouble();
		}
	}
	else {
		worker->globalHitCache.globalHits.nbAbsEquiv = 0.0;
		worker->globalHitCache.distTraveled_total = 0.0;
		worker->globalHitCache.distTraveledTotal_fullHitsOnly = 0.0;
	}
	file->ReadKeyword("maxDes"); file->ReadKeyword(":");
	worker->ontheflyParams.desorptionLimit = file->ReadSizeT();
	file->ReadKeyword("nbVertex"); file->ReadKeyword(":");
	sh.nbVertex = file->ReadInt();
	file->ReadKeyword("nbFacet"); file->ReadKeyword(":");
	sh.nbFacet = file->ReadInt();
	file->ReadKeyword("nbSuper"); file->ReadKeyword(":");
	sh.nbSuper = file->ReadInt();
	int nbF = 0; std::vector<std::vector<string>> loadFormulas;
	int nbV = 0;
	if (*version >= 2) {
		file->ReadKeyword("nbFormula"); file->ReadKeyword(":");
		nbF = file->ReadInt(); loadFormulas.reserve(nbF);
		file->ReadKeyword("nbView"); file->ReadKeyword(":");
		nbV = file->ReadInt();
	}
	int nbS = 0;
	if (*version >= 8) {
		file->ReadKeyword("nbSelection"); file->ReadKeyword(":");
		nbS = file->ReadInt();
	}
	if (*version >= 7) {
		file->ReadKeyword("gasMass"); file->ReadKeyword(":");
		worker->wp.gasMass = file->ReadDouble();
	}
	if (*version >= 10) { //time-dependent version
		file->ReadKeyword("userMoments"); file->ReadKeyword("{");
		file->ReadKeyword("nb"); file->ReadKeyword(":");
		int nb = file->ReadInt();

		for (int i = 0; i < nb; i++) {
			char tmpExpr[512];
			strcpy(tmpExpr, file->ReadString());
			worker->userMoments.push_back(tmpExpr);
			worker->AddMoment(mApp->worker.ParseMoment(tmpExpr));
		}
		file->ReadKeyword("}");

		/*for (size_t i = 0; i < wp.nbFacet;i++)
			facets[i]->ResizeCounter(wp.nbMoments); //Initialize hits counters for facets*/
	}
	if (*version >= 11) { //pulse version
		file->ReadKeyword("desorptionStart"); file->ReadKeyword(":");
		/*worker->desorptionStartTime =*/ file->ReadDouble();

		file->ReadKeyword("desorptionStop"); file->ReadKeyword(":");
		/*worker->desorptionStopTime =*/ file->ReadDouble();

		file->ReadKeyword("timeWindow"); file->ReadKeyword(":");
		worker->wp.timeWindowSize = file->ReadDouble();

		file->ReadKeyword("useMaxwellian"); file->ReadKeyword(":");
		worker->wp.useMaxwellDistribution = file->ReadInt();

	}
	if (*version >= 12) { //2013.aug.22
		file->ReadKeyword("calcConstantFlow"); file->ReadKeyword(":");
		worker->wp.calcConstantFlow = file->ReadInt();

	}
	if (*version >= 2) {
		file->ReadKeyword("formulas"); file->ReadKeyword("{");
		for (int i = 0; i < nbF; i++) {
			char tmpName[256];
			char tmpExpr[512];
			strcpy(tmpName, file->ReadString());
			strcpy(tmpExpr, file->ReadString());
			//mApp->AddFormula(tmpName, tmpExpr); //parse after selection groups are loaded

			std::vector<string> newFormula;
			newFormula.push_back(tmpName);
			newFormula.push_back(tmpExpr);
			loadFormulas.push_back(newFormula);
		}
		file->ReadKeyword("}");

		file->ReadKeyword("views"); file->ReadKeyword("{");
		for (int i = 0; i < nbV; i++) {
			char tmpName[256];
			AVIEW v;
			strcpy(tmpName, file->ReadString());
			v.projMode = file->ReadInt();
			v.camAngleOx = file->ReadDouble();
			v.camAngleOy = file->ReadDouble();
			v.camAngleOz = 0.0; //No support for Z angle in current GEO version
			v.camDist = file->ReadDouble();
			v.camOffset.x = file->ReadDouble();
			v.camOffset.y = file->ReadDouble();
			v.camOffset.z = file->ReadDouble();
			v.performXY = file->ReadInt();

			v.vLeft = file->ReadDouble();
			v.vRight = file->ReadDouble();
			v.vTop = file->ReadDouble();
			v.vBottom = file->ReadDouble();
			mApp->AddView(tmpName, v);
		}
		file->ReadKeyword("}");
	}

	if (*version >= 8) {
		file->ReadKeyword("selections"); file->ReadKeyword("{");
		for (int i = 0; i < nbS; i++) {
			SelectionGroup s;
			char tmpName[256];
			strcpy(tmpName, file->ReadString());
			s.name = strdup(tmpName);
			int nbSel = file->ReadInt();

			for (int j = 0; j < nbSel; j++) {
				s.selection.push_back(file->ReadInt());
			}
			mApp->AddSelection(s);
		}
		file->ReadKeyword("}");
	}

	for (int i = 0; i < nbF; i++) { //parse formulas now that selection groups are loaded
		mApp->AddFormula(loadFormulas[i][0].c_str(), loadFormulas[i][1].c_str());
	}

	file->ReadKeyword("structures"); file->ReadKeyword("{");
	for (int i = 0; i < sh.nbSuper; i++) {
		strName[i] = strdup(file->ReadString());
		// For backward compatibilty with STR
		sprintf(tmp, "%s.txt", strName[i]);
		strFileName[i] = strdup(tmp);
	}
	file->ReadKeyword("}");

	// Allocate memory
	facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));
	std::vector<InterfaceVertex>(sh.nbVertex).swap(vertices3);

	// Read vertices
	prg->SetMessage("Reading vertices...");
	file->ReadKeyword("vertices"); file->ReadKeyword("{");
	for (int i = 0; i < sh.nbVertex; i++) {
		// Check idx
		int idx = file->ReadInt();
		if (idx != i + 1) throw Error(file->MakeError("Wrong vertex index !"));
		vertices3[i].x = file->ReadDouble();
		vertices3[i].y = file->ReadDouble();
		vertices3[i].z = file->ReadDouble();
		vertices3[i].selected = false;
	}
	file->ReadKeyword("}");

	if (*version >= 6) {
		prg->SetMessage("Reading leaks and hits...");
		// Read leaks
		file->ReadKeyword("leaks"); file->ReadKeyword("{");
		file->ReadKeyword("nbLeak"); file->ReadKeyword(":");
		worker->globalHitCache.leakCacheSize = file->ReadInt();
		for (int i = 0; i < worker->globalHitCache.leakCacheSize; i++) {
			int idx = file->ReadInt();
			if (idx != i) throw Error(file->MakeError("Wrong leak index !"));
			if (i < LEAKCACHESIZE) {
				worker->globalHitCache.leakCache[i].pos.x = file->ReadDouble();
				worker->globalHitCache.leakCache[i].pos.y = file->ReadDouble();
				worker->globalHitCache.leakCache[i].pos.z = file->ReadDouble();

				worker->globalHitCache.leakCache[i].dir.x = file->ReadDouble();
				worker->globalHitCache.leakCache[i].dir.y = file->ReadDouble();
				worker->globalHitCache.leakCache[i].dir.z = file->ReadDouble();
			}
			else { //Saved file has more leaks than we could load
				for (int i = 0; i < 6; i++)
					file->ReadDouble();
			}
		}
		file->ReadKeyword("}");

		// Read hit cache
		file->ReadKeyword("hits"); file->ReadKeyword("{");
		file->ReadKeyword("nbHHit"); file->ReadKeyword(":");
		worker->globalHitCache.hitCacheSize = file->ReadInt();
		for (int i = 0; i < worker->globalHitCache.hitCacheSize; i++) {
			int idx = file->ReadInt();
			if (idx != i) throw Error(file->MakeError("Wrong hit cache index !"));
			if (i < HITCACHESIZE) {
				worker->globalHitCache.hitCache[i].pos.x = file->ReadDouble();
				worker->globalHitCache.hitCache[i].pos.y = file->ReadDouble();
				worker->globalHitCache.hitCache[i].pos.z = file->ReadDouble();

				worker->globalHitCache.hitCache[i].type = file->ReadInt();
			}
			else { //Saved file has more hits than we could load
				for (int i = 0; i < 3; i++)
					file->ReadDouble();
				file->ReadInt();
			}
		}
		file->ReadKeyword("}");
	}

	// Read facets
	prg->SetMessage("Reading facets...");
	for (int i = 0; i < sh.nbFacet; i++) {
		file->ReadKeyword("facet");
		// Check idx
		int idx = file->ReadInt();
		if (idx != i + 1) throw Error(file->MakeError("Wrong facet index !"));
		file->ReadKeyword("{");
		file->ReadKeyword("nbIndex");
		file->ReadKeyword(":");
		int nbI = file->ReadInt();
		if (nbI < 3) {
			char errMsg[512];
			sprintf(errMsg, "Facet %d has only %d vertices. ", i + 1, nbI);
			throw Error(errMsg);
		}
		prg->SetProgress((float)i / sh.nbFacet);
		facets[i] = new Facet(nbI);
		facets[i]->LoadGEO(file, *version, sh.nbVertex);
		file->ReadKeyword("}");
	}

	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true
	UpdateName(file);

	// Update mesh
	prg->SetMessage("Building mesh...");
	for (int i = 0; i < sh.nbFacet; i++) {
		double p = (double)i / (double)sh.nbFacet;
		prg->SetProgress(p);
		Facet *f = facets[i];
		if (!f->SetTexture(f->sh.texWidthD, f->sh.texHeightD, f->hasMesh)) {
			char errMsg[512];
			sprintf(errMsg, "Not enough memory to build mesh on Facet %d. ", i + 1);
			throw Error(errMsg);
		}
		BuildFacetList(f);
		double nU = f->sh.U.Norme();
		f->tRatio = f->sh.texWidthD / nU;
	}

}

/**
* \brief For loading geometry data from SYN format
* \param file name of the input file
* \param prg GLProgress window
* \param version version of the SYN description
* \param worker thread worker that executes the task
*/
void MolflowGeometry::LoadSYN(FileReader *file, GLProgress *prg, int *version, Worker *worker) {

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	prg->SetMessage("Clearing current geometry...");
	Clear();
	//mApp->ClearFormulas();

	// Globals
	char tmp[512];
	prg->SetMessage("Reading SYN file header...");
	file->ReadKeyword("version"); file->ReadKeyword(":");
	*version = file->ReadInt();
	if (*version > SYNVERSION) {
		char errMsg[512];
		sprintf(errMsg, "Unsupported SYN version V%d", *version);
		throw Error(errMsg);
	}
	file->ReadKeyword("totalHit"); file->ReadKeyword(":");
	worker->globalHitCache.globalHits.nbMCHit = 0;
	worker->globalHitCache.globalHits.nbHitEquiv = 0.0;
	file->ReadSizeT();
	if (*version >= 10) {
		file->ReadKeyword("totalHitEquiv"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("totalDes"); file->ReadKeyword(":");
	worker->globalHitCache.globalHits.nbDesorbed = 0; file->ReadSizeT();
	if (*version >= 6) {
		file->ReadKeyword("no_scans"); file->ReadKeyword(":");
		/*loaded_no_scans = */file->ReadDouble();
	}
	file->ReadKeyword("totalLeak"); file->ReadKeyword(":");
	worker->globalHitCache.nbLeakTotal = 0; file->ReadSizeT();
	if (*version > 2) {
		file->ReadKeyword("totalFlux"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("totalPower"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	if (*version >= 10) {
		file->ReadKeyword("totalAbsEquiv"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("totalDist"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("maxDes"); file->ReadKeyword(":");
	worker->ontheflyParams.desorptionLimit = 0; file->ReadSizeT();
	file->ReadKeyword("nbVertex"); file->ReadKeyword(":");
	sh.nbVertex = file->ReadInt();
	file->ReadKeyword("nbFacet"); file->ReadKeyword(":");
	sh.nbFacet = file->ReadInt();
	file->ReadKeyword("nbSuper"); file->ReadKeyword(":");
	sh.nbSuper = file->ReadInt();
	int nbF = 0;
	int nbV = 0;

	file->ReadKeyword("nbFormula"); file->ReadKeyword(":");
	nbF = file->ReadInt();
	file->ReadKeyword("nbView"); file->ReadKeyword(":");
	nbV = file->ReadInt();
	int nbS = 0;
	file->ReadKeyword("nbSelection"); file->ReadKeyword(":");
	nbS = file->ReadInt();

	if (*version > 1) {
		file->ReadKeyword("nbRegions"); file->ReadKeyword(":");
		int nbR = file->ReadInt();
		//result=PARfileList(nbR);

		file->ReadKeyword("PARfiles"); file->ReadKeyword("{");
		for (int i = 0; i < nbR; i++) {
			/*char tmp[512];
			strcpy(tmp,file->ReadString());
			result.fileNames[i]=strdup(tmp);*/
			file->ReadString();
		}
		file->ReadKeyword("}");
	}

	file->ReadKeyword("formulas"); file->ReadKeyword("{");
	for (int i = 0; i < nbF; i++) {
		char tmpName[256];
		char tmpExpr[512];
		strcpy(tmpName, file->ReadString());
		strcpy(tmpExpr, file->ReadString());
		//mApp->AddFormula(tmpName, tmpExpr);
	}
	file->ReadKeyword("}");

	file->ReadKeyword("views"); file->ReadKeyword("{");
	for (int i = 0; i < nbV; i++) {
		char tmpName[256];
		AVIEW v;
		strcpy(tmpName, file->ReadString());
		v.projMode = file->ReadInt();
		v.camAngleOx = file->ReadDouble();
		v.camAngleOy = file->ReadDouble();
		v.camAngleOz = 0.0; //No support for Z angle in current SYN version
		v.camDist = file->ReadDouble();
		v.camOffset.x = file->ReadDouble();
		v.camOffset.y = file->ReadDouble();
		v.camOffset.z = file->ReadDouble();
		v.performXY = file->ReadInt();
		v.vLeft = file->ReadDouble();
		v.vRight = file->ReadDouble();
		v.vTop = file->ReadDouble();
		v.vBottom = file->ReadDouble();
		mApp->AddView(tmpName, v);
	}
	file->ReadKeyword("}");

	file->ReadKeyword("selections"); file->ReadKeyword("{");
	for (int i = 0; i < nbS; i++) {
		SelectionGroup s;
		char tmpName[256];
		strcpy(tmpName, file->ReadString());
		s.name = strdup(tmpName);
		int nbSel = file->ReadInt();

		for (int j = 0; j < nbSel; j++) {
			s.selection.push_back(file->ReadInt());
		}
		mApp->AddSelection(s);
	}
	file->ReadKeyword("}");

	file->ReadKeyword("structures"); file->ReadKeyword("{");
	for (int i = 0; i < sh.nbSuper; i++) {
		strName[i] = strdup(file->ReadString());
		// For backward compatibilty with STR
		sprintf(tmp, "%s.txt", strName[i]);
		strFileName[i] = strdup(tmp);
	}
	file->ReadKeyword("}");

	// Allocate memory
	facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));
	vertices3.resize(sh.nbVertex); vertices3.shrink_to_fit();

	// Read vertices
	prg->SetMessage("Reading vertices...");
	file->ReadKeyword("vertices"); file->ReadKeyword("{");
	for (int i = 0; i < sh.nbVertex; i++) {
		// Check idx
		int idx = file->ReadInt();
		if (idx != i + 1) throw Error(file->MakeError("Wrong vertex index !"));
		vertices3[i].x = file->ReadDouble();
		vertices3[i].y = file->ReadDouble();
		vertices3[i].z = file->ReadDouble();
		vertices3[i].selected = false;
	}
	file->ReadKeyword("}");
	prg->SetMessage("Reading leaks and hits...");
	// Read leaks
	file->ReadKeyword("leaks"); file->ReadKeyword("{");
	file->ReadKeyword("nbLeak"); file->ReadKeyword(":");

	int nbleak_local = file->ReadInt();
	for (int i = 0; i < nbleak_local; i++) {
		int idx = file->ReadInt();
		if (idx != i) throw Error(file->MakeError("Wrong leak index !"));
		//(pleak+i)->pos.x = 
		file->ReadDouble();
		//(pleak+i)->pos.y = 
		file->ReadDouble();
		//(pleak+i)->pos.z = 
		file->ReadDouble();

		//(pleak+i)->dir.x = 
		file->ReadDouble();
		//(pleak+i)->dir.y = 
		file->ReadDouble();
		//(pleak+i)->dir.z = 
		file->ReadDouble();
	}
	file->ReadKeyword("}");

	// Read hit cache
	file->ReadKeyword("hits"); file->ReadKeyword("{");
	file->ReadKeyword("nbHHit"); file->ReadKeyword(":");

	int nbHHit_local = file->ReadInt();
	for (int i = 0; i < nbHHit_local; i++) {
		int idx = file->ReadInt();
		if (idx != i) throw Error(file->MakeError("Wrong hit cache index !"));
		//(hitCache+i)->pos.x = 
		file->ReadDouble();
		//(hitCache+i)->pos.y = 
		file->ReadDouble();
		//(hitCache+i)->pos.z = 
		file->ReadDouble();
		//(hitCache+i)->dF = 
		file->ReadDouble();
		//(hitCache+i)->dP = 
		file->ReadDouble();
		//(hitCache+i)->type = 
		file->ReadInt();
	}
	file->ReadKeyword("}");
	// Read facets
	prg->SetMessage("Reading facets...");
	for (int i = 0; i < sh.nbFacet; i++) {
		file->ReadKeyword("facet");
		// Check idx
		int idx = file->ReadInt();
		if (idx != i + 1) throw Error(file->MakeError("Wrong facet index !"));
		file->ReadKeyword("{");
		file->ReadKeyword("nbIndex");
		file->ReadKeyword(":");
		int nbI = file->ReadInt();
		if (nbI < 3) {
			char errMsg[512];
			sprintf(errMsg, "Facet %d has only %d vertices. ", i, nbI);
			throw Error(errMsg);
		}
		prg->SetProgress((float)i / sh.nbFacet);
		facets[i] = new Facet(nbI);
		facets[i]->LoadSYN(file, *version, sh.nbVertex);
		file->ReadKeyword("}");
	}

	prg->SetMessage("Initalizing geometry and building mesh...");
	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true
	UpdateName(file);

	// Update mesh
	prg->SetMessage("Drawing textures...");
	for (int i = 0; i < sh.nbFacet; i++) {
		double p = (double)i / (double)sh.nbFacet;
		prg->SetProgress(p);
		Facet *f = facets[i];
		//f->SetTexture(f->wp.texWidthD,f->wp.texHeightD,f->hasMesh);
		BuildFacetList(f);
		//double nU = &(f->wp.U).Norme();
		//f->tRatio = f->wp.texWidthD / nU;
	}
	//return result;

}

/**
* \brief For loading texture data from GEO format
* \param file name of the input file
* \param prg GLProgress window
* \param results simulation results describing the texture
* \param version version of the GEO description
*/
bool MolflowGeometry::LoadTexturesGEO(FileReader *file, GLProgress *prg, GlobalSimuState& results, int version) {

	if (file->SeekFor("{textures}")) {
		char tmp[256];
		//versions 3+
		// Block dpHit during the whole disc reading

		LockMutex(results.mutex);
		//TextureCell readVal;

		// Globals
		GlobalHitBuffer& gHits = results.globalHits;

		/*gHits->globalHits.nbMCHit = loaded_nbMCHit;
		gHits->globalHits.nbDesorbed = loaded_nbDesorption;
		gHits->globalHits.nbAbsEquiv = loaded_nbAbsEquiv;
		gHits->nbLeakTotal = loaded_nbLeak;
		gHits->distTraveled_total = distTraveled_total;

		gHits->distTraveledTotal_fullHitsOnly = distTraveledTotal_fullHitsOnly;*/

		// Read facets
		if (version >= 13) {
			file->ReadKeyword("min_pressure_all"); file->ReadKeyword(":");
			gHits.texture_limits[0].min.all = file->ReadDouble();
			file->ReadKeyword("min_pressure_moments_only"); file->ReadKeyword(":");
			gHits.texture_limits[0].min.moments_only = file->ReadDouble();
			file->ReadKeyword("max_pressure_all"); file->ReadKeyword(":");
			gHits.texture_limits[0].max.all = file->ReadDouble();
			file->ReadKeyword("max_pressure_moments_only"); file->ReadKeyword(":");
			gHits.texture_limits[0].max.moments_only = file->ReadDouble();

			file->ReadKeyword("min_impingement_all"); file->ReadKeyword(":");
			gHits.texture_limits[1].min.all = file->ReadDouble();
			file->ReadKeyword("min_impingement_moments_only"); file->ReadKeyword(":");
			gHits.texture_limits[1].min.moments_only = file->ReadDouble();
			file->ReadKeyword("max_impingement_all"); file->ReadKeyword(":");
			gHits.texture_limits[1].max.all = file->ReadDouble();
			file->ReadKeyword("max_impingement_moments_only"); file->ReadKeyword(":");
			gHits.texture_limits[1].max.moments_only = file->ReadDouble();

			file->ReadKeyword("min_density_all"); file->ReadKeyword(":");

			gHits.texture_limits[2].min.all = file->ReadDouble();
			file->ReadKeyword("min_density_moments_only"); file->ReadKeyword(":");

			gHits.texture_limits[2].min.moments_only = file->ReadDouble();
			file->ReadKeyword("max_density_all"); file->ReadKeyword(":");

			gHits.texture_limits[2].max.all = file->ReadDouble();
			file->ReadKeyword("max_density_moments_only"); file->ReadKeyword(":");

			gHits.texture_limits[2].max.moments_only = file->ReadDouble();

			size_t facetHitsSize = (1 + mApp->worker.moments.size()) * sizeof(FacetHitBuffer);
			for (size_t m = 0; m <= mApp->worker.moments.size() || (m == 0 /*&& version<10*/); m++) {
				//if (version>=10) {
				file->ReadKeyword("moment");
				if (m != file->ReadInt()) {
					throw Error("Unexpected profile moment");
					break;
				}
				file->ReadKeyword("{");
				//}

				for (int i = 0; i < sh.nbFacet; i++) {
					Facet *f = facets[i];
					if (f->hasMesh/* || version<8*/) {
						prg->SetProgress((double)(i + m * sh.nbFacet) / (double)(mApp->worker.moments.size()*sh.nbFacet)*0.33 + 0.66);
						file->ReadKeyword("texture_facet");
						// Check idx
						int idx = file->ReadInt();

						if (idx != i + 1) {
							sprintf(tmp, "Wrong facet index. Expected %d, read %d.", i + 1, idx);
							throw Error(file->MakeError(tmp));
						}

						file->ReadKeyword("{");

						int ix, iy;

						///Load textures, for GEO file version 3+

						size_t profSize = (f->sh.isProfile) ? ((1 + mApp->worker.moments.size())*(PROFILE_SIZE * sizeof(ProfileSlice))) : 0;
						size_t h = f->sh.texHeight;
						size_t w = f->sh.texWidth;

						//TextureCell *texture = (TextureCell *)((BYTE *)gHits + (f->sh.hitOffset + facetHitsSize + profSize + m * w*h * sizeof(TextureCell)));
						std::vector<TextureCell>& texture = results.facetStates[i].momentResults[m].texture;

						size_t texWidth_file, texHeight_file;
						//In case of rounding errors, the file might contain different texture dimensions than expected.
						if (version >= 14) {
							file->ReadKeyword("width"); file->ReadKeyword(":"); texWidth_file = file->ReadInt();
							file->ReadKeyword("height"); file->ReadKeyword(":"); texHeight_file = file->ReadInt();
						}
						else {
							texWidth_file = f->sh.texWidth;
							texHeight_file = f->sh.texHeight;
						}

						for (iy = 0; iy < (Min(f->sh.texHeight, texHeight_file)); iy++) { //MIN: If stored texture is larger, don't read extra cells
							for (ix = 0; ix < (Min(f->sh.texWidth, texWidth_file)); ix++) { //MIN: If stored texture is larger, don't read extra cells
								size_t index = iy * f->sh.texWidth + ix;
								texture[index].countEquiv = static_cast<double>(file->ReadSizeT());
								texture[index].sum_1_per_ort_velocity = file->ReadDouble();
								texture[index].sum_v_ort_per_area = file->ReadDouble();

							}
							for (size_t ie = 0; ie < texWidth_file - f->sh.texWidth; ie++) {//Executed if file texture is bigger than expected texture
								//Read extra cells from file without doing anything
								file->ReadSizeT();
								file->ReadDouble();
								file->ReadDouble();
							}
						}
						for (size_t ie = 0; ie < texHeight_file - f->sh.texHeight; ie++) {//Executed if file texture is bigger than expected texture
							//Read extra cells ffrom file without doing anything
							for (int iw = 0; iw < texWidth_file; iw++) {
								file->ReadSizeT();

								file->ReadDouble();
								file->ReadDouble();
							}
						}
						file->ReadKeyword("}");
					}
				}
				/*if (version>=10)*/ file->ReadKeyword("}");
			}
		}

		ReleaseMutex(results.mutex);
		return true;

	}
	else
	{
		//old versions
		return false;
	}

}

/**
* \brief For saving the geometry data into GEO format
* \param file name of the output file
* \param prg GLProgress window
* \param results simulation results describing the texture
* \param worker thread worker handling this task
* \param saveSelected if a selection is to be saved
* \param crashSave if crash save is enabled
*/

void MolflowGeometry::SaveGEO(FileWriter *file, GLProgress *prg, GlobalSimuState& results, Worker *worker,
	bool saveSelected, bool crashSave) {

	prg->SetMessage("Counting hits...");
	if (!IsLoaded()) throw Error("Nothing to save !");

	// Block dpHit during the whole disc writing
	if (!crashSave && !saveSelected) LockMutex(results.mutex);

	// Globals
	GlobalHitBuffer *gHits;
	if (!crashSave && !saveSelected) gHits = &(results.globalHits);

	double dCoef = 1.0;
	int ix, iy;

	/*switch(gHits->mode) {

	case MC_MODE:
	if( gHits->globalHits.nbDesorbed>0 ) {
	dCoef = (float)totalOutgassing / (float)gHits->globalHits.nbDesorbed / 8.31 * wp.gasMass / 100;
	texMinAutoscale = gHits->minHit * dCoef;
	texMaxAutoscale = gHits->maxHit * dCoef;
	} else {
	texMinAutoscale = gHits->minHit;
	texMaxAutoscale = gHits->maxHit;
	}
	break;

	case AC_MODE:
	texMinAutoscale = gHits->minHit;
	texMaxAutoscale = gHits->maxHit;
	break;

	}*/

	prg->SetMessage("Writing geometry details...");
	file->Write("version:"); file->Write(GEOVERSION, "\n");
	file->Write("totalHit:"); file->Write((!crashSave && !saveSelected) ? gHits->globalHits.nbMCHit : 0, "\n");
	file->Write("totalDes:"); file->Write((!crashSave && !saveSelected) ? gHits->globalHits.nbDesorbed : 0, "\n");
	file->Write("totalLeak:"); file->Write((!crashSave && !saveSelected) ? gHits->nbLeakTotal : 0, "\n");
	file->Write("totalAbs:"); file->Write((!crashSave && !saveSelected) ? (size_t)gHits->globalHits.nbAbsEquiv : 0, "\n");
	file->Write("totalDist_total:"); file->Write((!crashSave && !saveSelected) ? gHits->distTraveled_total : 0, "\n");
	file->Write("totalDist_fullHitsOnly:"); file->Write((!crashSave && !saveSelected) ? gHits->distTraveledTotal_fullHitsOnly : 0, "\n");
	file->Write("maxDes:"); file->Write((!crashSave && !saveSelected) ? worker->ontheflyParams.desorptionLimit : 0, "\n");

	auto selectedFacets = GetSelectedFacets();
	file->Write("nbVertex:"); file->Write(sh.nbVertex, "\n");
	file->Write("nbFacet:"); file->Write(saveSelected ? selectedFacets.size() : sh.nbFacet, "\n");
	file->Write("nbSuper:"); file->Write(sh.nbSuper, "\n");
	file->Write("nbFormula:"); file->Write((!saveSelected) ? mApp->formulas_n.size() : 0, "\n");

	file->Write("nbView:"); file->Write(mApp->nbView, "\n");
	file->Write("nbSelection:"); file->Write((!saveSelected) ? mApp->selections.size() : 0, "\n");

	file->Write("gasMass:"); file->Write(worker->wp.gasMass, "\n");

	file->Write("userMoments {\n");
	file->Write(" nb:"); file->Write((int)worker->userMoments.size());
	for (size_t u = 0; u < worker->userMoments.size(); u++) {
		file->Write("\n \"");
		file->Write(worker->userMoments[u].c_str());
		file->Write("\"");
	}
	file->Write("\n}\n");

	file->Write("desorptionStart:"); file->Write(/*worker->desorptionStartTime*/0.0, "\n");
	file->Write("desorptionStop:"); file->Write(/*worker->desorptionStopTime*/1.0, "\n");
	file->Write("timeWindow:"); file->Write(worker->wp.timeWindowSize, "\n");
	file->Write("useMaxwellian:"); file->Write(worker->wp.useMaxwellDistribution, "\n");
	file->Write("calcConstantFlow:"); file->Write(worker->wp.calcConstantFlow, "\n");

	file->Write("formulas {\n");
	if (!saveSelected) {
		for (auto& f : mApp->formulas_n) {
			file->Write("  \"");
			file->Write(f->GetName());
			file->Write("\" \"");
			file->Write(f->GetExpression());
			file->Write("\"\n");
		}
	}
	file->Write("}\n");

	file->Write("views {\n");
	for (int i = 0; i < mApp->nbView; i++) {
		file->Write("  \"");
		file->Write(mApp->views[i].name);
		file->Write("\"\n");
		file->Write(mApp->views[i].projMode, " ");
		file->Write(mApp->views[i].camAngleOx, " ");
		file->Write(mApp->views[i].camAngleOy, " ");
		file->Write(mApp->views[i].camDist, " ");
		file->Write(mApp->views[i].camOffset.x, " ");
		file->Write(mApp->views[i].camOffset.y, " ");
		file->Write(mApp->views[i].camOffset.z, " ");
		file->Write(mApp->views[i].performXY, " ");
		file->Write(mApp->views[i].vLeft, " ");
		file->Write(mApp->views[i].vRight, " ");
		file->Write(mApp->views[i].vTop, " ");
		file->Write(mApp->views[i].vBottom, "\n");
	}
	file->Write("}\n");

	file->Write("selections {\n");
	for (size_t i = 0; (i < mApp->selections.size()) && !saveSelected; i++) { //don't save selections when exporting part of the geometry (saveSelected)

		file->Write("  \"");
		file->Write(mApp->selections[i].name);
		file->Write("\"\n ");
		file->Write(mApp->selections[i].selection.size(), "\n");
		for (auto& sel : mApp->selections[i].selection) {
			file->Write("  ");
			file->Write(sel, "\n");
		}
		//file->Write("\n");
	}
	file->Write("}\n");

	file->Write("structures {\n");
	for (size_t i = 0; i < sh.nbSuper; i++) {
		file->Write("  \"");
		file->Write(strName[i]);
		file->Write("\"\n");
	}
	file->Write("}\n");
	//vertices
	prg->SetMessage("Writing vertices...");
	file->Write("vertices {\n");
	for (size_t i = 0; i < sh.nbVertex; i++) {
		prg->SetProgress(0.33*((double)i / (double)sh.nbVertex));
		file->Write("  ");
		file->Write(i + 1, " ");
		file->Write(vertices3[i].x, " ");
		file->Write(vertices3[i].y, " ");
		file->Write(vertices3[i].z, "\n");
	}
	file->Write("}\n");

	//leaks
	prg->SetMessage("Writing leaks...");
	file->Write("leaks {\n");
	file->Write("  nbLeak:"); file->Write((!crashSave && !saveSelected) ? worker->globalHitCache.leakCacheSize : 0, "\n");
	for (int i = 0; (i < worker->globalHitCache.leakCacheSize) && (!crashSave && !saveSelected); i++) {

		file->Write("  ");
		file->Write(i, " ");
		file->Write(worker->globalHitCache.leakCache[i].pos.x, " ");
		file->Write(worker->globalHitCache.leakCache[i].pos.y, " ");
		file->Write(worker->globalHitCache.leakCache[i].pos.z, " ");

		file->Write(worker->globalHitCache.leakCache[i].dir.x, " ");
		file->Write(worker->globalHitCache.leakCache[i].dir.y, " ");
		file->Write(worker->globalHitCache.leakCache[i].dir.z, "\n");
	}

	file->Write("}\n");

	//hit cache (lines and dots)
	prg->SetMessage("Writing hit cache...");

	file->Write("hits {\n");
	file->Write("  nbHHit:"); file->Write((!crashSave && !saveSelected) ? worker->globalHitCache.hitCacheSize : 0, "\n");
	for (int i = 0; (i < worker->globalHitCache.hitCacheSize) && (!crashSave && !saveSelected); i++) {

		file->Write("  ");
		file->Write(i, " ");
		file->Write(worker->globalHitCache.hitCache[i].pos.x, " ");
		file->Write(worker->globalHitCache.hitCache[i].pos.y, " ");
		file->Write(worker->globalHitCache.hitCache[i].pos.z, " ");

		file->Write(worker->globalHitCache.hitCache[i].type, "\n");
	}

	file->Write("}\n");

	//facets

	prg->SetMessage("Writing facets...");

	for (int i = 0, k = 0; i < sh.nbFacet; i++) {
		prg->SetProgress(0.33 + ((double)i / (double)sh.nbFacet) *0.33);

		if (!saveSelected || facets[i]->selected) facets[i]->SaveGEO(file, k++);

	}

	prg->SetMessage("Writing profiles...");
	SaveProfileGEO(file, results, -1, saveSelected, crashSave);

	///Save textures, for GEO file version 3+

	char tmp[256];
	file->Write("{textures}\n");

	file->Write("min_pressure_all:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[0].min.all : 0, "\n");
	file->Write("min_pressure_moments_only:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[0].min.moments_only : 0, "\n");
	file->Write("max_pressure_all:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[0].max.all : 1, "\n");
	file->Write("max_pressure_moments_only:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[0].max.moments_only : 1, "\n");

	file->Write("min_impingement_all:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[1].min.all : 0, "\n");
	file->Write("min_impingement_moments_only:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[1].min.moments_only : 0, "\n");
	file->Write("max_impingement_all:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[1].max.all : 1, "\n");
	file->Write("max_impingement_moments_only:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[1].max.moments_only : 1, "\n");

	file->Write("min_density_all:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[2].min.all : 0, "\n");
	file->Write("min_density_moments_only:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[2].min.moments_only : 0, "\n");
	file->Write("max_density_all:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[2].max.all : 1, "\n");
	file->Write("max_density_moments_only:"); file->Write(
		(!crashSave && !saveSelected) ? gHits->texture_limits[2].max.moments_only : 1, "\n");

	//Selections
	//SaveSelections();

	prg->SetMessage("Writing textures...");
	size_t facetHitsSize = (1 + mApp->worker.moments.size()) * sizeof(FacetHitBuffer);
	for (size_t m = 0; m <= mApp->worker.moments.size(); m++) {
		sprintf(tmp, "moment %zd {\n", m);
		file->Write(tmp);
		for (size_t i = 0; i < sh.nbFacet; i++) {
			prg->SetProgress((double)(i + m * sh.nbFacet) / (double)(mApp->worker.moments.size()*sh.nbFacet)*0.33 + 0.66);
			Facet *f = facets[i];
			if (f->hasMesh) {
				size_t h = f->sh.texHeight;
				size_t w = f->sh.texWidth;
				size_t profSize = (f->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice)*(1 + (int)mApp->worker.moments.size())) : 0;
				std::vector<TextureCell>* texture;
				if (!crashSave && !saveSelected) texture = &(results.facetStates[i].momentResults[m].texture);

				//char tmp[256];
				sprintf(tmp, " texture_facet %zd {\n", i + 1);

				file->Write(tmp);
				file->Write("width:"); file->Write(f->sh.texWidth); file->Write(" height:"); file->Write(f->sh.texHeight); file->Write("\n");
				for (iy = 0; iy < h; iy++) {
					for (ix = 0; ix < w; ix++) {
						file->Write((!crashSave && !saveSelected) ? static_cast<size_t>((*texture)[iy*f->sh.texWidth + ix].countEquiv) : 0, "\t");
						file->Write((!crashSave && !saveSelected) ? (*texture)[iy*f->sh.texWidth + ix].sum_1_per_ort_velocity : 0, "\t");
						file->Write((!crashSave && !saveSelected) ? (*texture)[iy*f->sh.texWidth + ix].sum_v_ort_per_area : 0, "\t");
					}
					file->Write("\n");
				}

				file->Write(" }\n");
			}
		}
		file->Write("}\n");
	}
	if (!crashSave && !saveSelected) ReleaseMutex(results.mutex);

}

/**
* \brief For saving the geometry data into TXT format
* \param file name of the output file
* \param results simulation results describing the texture
* \param saveSelected if a selection is to be saved
*/
void MolflowGeometry::SaveTXT(FileWriter *file, GlobalSimuState& results, bool saveSelected) {

	if (!IsLoaded()) throw Error("Nothing to save !");
	
	// Block dpHit during the whole disc writing
	if (!LockMutex(results.mutex)) return;
	
	// Unused
	file->Write(0, "\n");

	// Globals
	GlobalHitBuffer& gHits = results.globalHits;

	// Unused
	file->Write(gHits.globalHits.nbMCHit, "\n");
	file->Write(gHits.nbLeakTotal, "\n");

	file->Write(gHits.globalHits.nbDesorbed, "\n");
	file->Write(0, "\n"); //Desorption limit

	file->Write(sh.nbVertex, "\n");
	file->Write(saveSelected ? GetNbSelectedFacets() : sh.nbFacet, "\n");

	// Read geometry vertices
	for (int i = 0; i < sh.nbVertex; i++) {
		file->Write(vertices3[i].x, " ");
		file->Write(vertices3[i].y, " ");
		file->Write(vertices3[i].z, "\n");
	}

	// Facets
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		int j;
		if (saveSelected) {
			if (f->selected) {
				file->Write(f->sh.nbIndex, " ");
				for (j = 0; j < f->sh.nbIndex - 1; j++)
					file->Write(f->indices[j] + 1, " ");
				file->Write(f->indices[j] + 1, "\n");
			}
		}
		else {
			file->Write(f->sh.nbIndex, " ");
			for (j = 0; j < f->sh.nbIndex - 1; j++)
				file->Write(f->indices[j] + 1, " ");
			file->Write(f->indices[j] + 1, "\n");
		}
	}

	// Params
	for (int i = 0; i < sh.nbFacet; i++) {

		// Update facet hits from shared mem
		Facet *f = facets[i];
		/*FacetHitBuffer *shF = (FacetHitBuffer *)(buffer + f->wp.hitOffset);
		memcpy(&(f->wp.tmpCounter), shF, sizeof(FacetHitBuffer));*/
		if (saveSelected) {
			if (f->selected) f->SaveTXT(file);
		}
		else {

			f->SaveTXT(file);
		}

	}

	SaveProfileTXT(file);

	ReleaseMutex(results.mutex);

}

/**
* \brief For exporting textures depending on the texture mode
* \param file name of the output file
* \param grouping if facets should be grouped for the output
* \param mode texture mode; which type of data describes it
* \param results simulation results describing the texture
* \param saveSelected if a selection is to be saved (TODO: chefk if actually used)
* \param sMode simulation mode
*/
void MolflowGeometry::ExportTextures(FILE *file, int grouping, int mode, GlobalSimuState& results, bool saveSelected, size_t sMode) {



	// Block dpHit during the whole disc writing
	if (!LockMutex(results.mutex)) return;

	if (grouping == 1) fprintf(file, "X_coord_cm\tY_coord_cm\tZ_coord_cm\tValue\t\n"); //mode 10: special ANSYS export

	size_t facetHitsSize = (1 + mApp->worker.moments.size()) * sizeof(FacetHitBuffer);
	for (size_t m = 0; m <= mApp->worker.moments.size(); m++) {
		if (m == 0) fprintf(file, " moment 0 (Constant Flow){\n");
		else fprintf(file, " moment %zd (%g s){\n", m, mApp->worker.moments[m - 1]);
		// Facets
		for (int i = 0; i < sh.nbFacet; i++) {
			Facet *f = facets[i];

			if (f->selected) {
				if (grouping == 0) fprintf(file, "FACET%d\n", i + 1); //mode 10: special ANSYS export
				TextureCell *texture = NULL;
				DirectionCell *dirs = NULL;

				if (f->cellPropertiesIds || f->sh.countDirection) {

					char tmp[256];
					char out[512];
					double dCoef = 1.0;
					GlobalHitBuffer& shGHit = results.globalHits;
					size_t nbMoments = mApp->worker.moments.size();
					size_t profSize = (f->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice)*(1 + nbMoments)) : 0;
					size_t w = f->sh.texWidth;
					size_t h = f->sh.texHeight;
					size_t tSize = w * h * sizeof(TextureCell);
					size_t dSize = w * h * sizeof(DirectionCell);
					std::vector<TextureCell>& texture = results.facetStates[i].momentResults[m].texture;
					std::vector<DirectionCell>& dirs = results.facetStates[i].momentResults[m].direction;

					for (size_t i = 0; i < w; i++) {
						for (size_t j = 0; j < h; j++) {
							size_t index = i + j * w;
							tmp[0] = out[0] = 0;
							switch (mode) {

							case 0: // Element area
								sprintf(tmp, "%g", f->GetMeshArea(index));
								break;

							case 1: //MC Hits
								if (!grouping || texture[index].countEquiv > 0.0) sprintf(tmp, "%g", texture[index].countEquiv);
								break;

							case 2: //Impingement rate
								dCoef = 1E4; //1E4: conversion m2->cm2
								if (sMode == MC_MODE) dCoef *= mApp->worker.GetMoleculesPerTP(m);
								if (!grouping || texture[index].countEquiv > 0.0) sprintf(tmp, "%g", texture[i + j * w].countEquiv / f->GetMeshArea(i + j * w, true)*dCoef);
								break;

							case 3: //Particle density
							{
								dCoef = 1E4; //1E4: conversion m2->cm2
								if (sMode == MC_MODE) dCoef *= mApp->worker.GetMoleculesPerTP(m);
								double v_ort_avg = 2.0*texture[index].countEquiv / texture[index].sum_1_per_ort_velocity;
								double imp_rate = texture[index].countEquiv / f->GetMeshArea(index, true)*dCoef;
								double rho = 2.0*imp_rate / v_ort_avg;
								if (!grouping || texture[index].countEquiv > 0.0) sprintf(tmp, "%g", rho);
								break;
							}
							case 4: //Gas density
							{
								dCoef = 1E4; //1E4: conversion m2->cm2
								if (sMode == MC_MODE) dCoef *= mApp->worker.GetMoleculesPerTP(m);
								double v_ort_avg = 2.0*texture[index].countEquiv / texture[index].sum_1_per_ort_velocity;
								double imp_rate = texture[index].countEquiv / f->GetMeshArea(index, true)*dCoef;
								double rho = 2.0*imp_rate / v_ort_avg;
								double rho_mass = rho * mApp->worker.wp.gasMass / 1000.0 / 6E23;
								if (!grouping || texture[index].countEquiv > 0.0) sprintf(tmp, "%g", rho_mass);
								break;
							}
							case 5:  // Pressure [mbar]

								// Lock during update
								dCoef = 1E4 * (mApp->worker.wp.gasMass / 1000 / 6E23) *0.0100;  //1E4 is conversion from m2 to cm2, 0.01: Pa->mbar
								if (sMode == MC_MODE) dCoef *= mApp->worker.GetMoleculesPerTP(m);
								if (!grouping || texture[index].sum_v_ort_per_area) sprintf(tmp, "%g", texture[index].sum_v_ort_per_area*dCoef);
								break;

							case 6: // Average velocity
								if (!grouping || texture[index].countEquiv > 0.0) sprintf(tmp, "%g", 2.0*texture[i + j * w].countEquiv / texture[i + j * w].sum_1_per_ort_velocity);
								break;

							case 7: // Velocity vector
								if (f->sh.countDirection) {
									sprintf(tmp, "%g,%g,%g",
										dirs[i + j * w].dir.x,
										dirs[i + j * w].dir.y,
										dirs[i + j * w].dir.z);
								}
								else {
									sprintf(tmp, "Direction not recorded");
								}
								break;

							case 8: // Velocity vector Count
								if (f->sh.countDirection) {
									sprintf(tmp, "%zd", dirs[i + j * w].count);
								}
								else {

									sprintf(tmp, "None");
								}
								break;
							} //end switch

							if (grouping == 1 && tmp && tmp[0]) {
								Vector2d facetCenter = f->GetMeshCenter(index);
								sprintf(out, "%g\t%g\t%g\t%s\t\n",
									f->sh.O.x + facetCenter.u*f->sh.U.x + facetCenter.v*f->sh.V.x,
									f->sh.O.y + facetCenter.u*f->sh.U.y + facetCenter.v*f->sh.V.y,
									f->sh.O.z + facetCenter.u*f->sh.U.z + facetCenter.v*f->sh.V.z,
									tmp);
							}
							else sprintf(out, "%s", tmp);

							if (out) fprintf(file, "%s", out);
							if (j < w - 1 && grouping == 0)
								fprintf(file, "\t");
						} //h
						if (grouping == 0) fprintf(file, "\n");
					} //w
				} //if mesh
				else {
					fprintf(file, "No mesh.\n");
				}
				if (grouping == 0) fprintf(file, "\n"); //Current facet exported. 
			} //if selected

		} //end facet
		fprintf(file, " }\n");

	} //end moment

	ReleaseMutex(results.mutex);

}

/**
* \brief For exporting profile data (simulation)
* \param file name of the output file
* \param isTXT if TXT output will be used
* \param results simulation results describing the texture
* \param worker thread worker handling the task
*/
void MolflowGeometry::ExportProfiles(FILE *file, int isTXT, GlobalSimuState& results, Worker *worker) {

	char sep = isTXT ? '\t' : ',';
	//if(!IsLoaded()) throw Error("Nothing to save !");

	// Block dpHit during the whole disc writing
	if (!LockMutex(results.mutex)) return;

	extern const char* profType[];

	// Globals
	//BYTE *buffer = (BYTE *)dpHit->buff;
	//SHGHITS *gHits = (SHGHITS *)buffer;
	std::ostringstream header;
	header << "Facet number" << sep << "Profile_type" << sep << "O_x" << sep << "O_y" << sep << "O_z" << sep << "U_x" << sep << "U_y" << sep << "U_z" << sep;
	header << "V_x" << sep << "V_y" << sep << "V_z" << sep << "U_length" << sep << "V_length" << sep << "Center_x" << sep << "Center_y" << sep << "Center_z" << sep << "V_max" << sep << "MC Hits" << sep << "Hit equiv." << sep;
	for (int i = 0; i < PROFILE_SIZE; i++)
		header << i + 1 << sep;
	header << '\n';

	fputs(header.str().c_str(), file);

	size_t facetHitsSize = (1 + mApp->worker.moments.size()) * sizeof(FacetHitBuffer);
	for (size_t m = 0; m <= mApp->worker.moments.size(); m++) {
		if (m == 0) fputs(" moment 0 (Constant Flow){\n", file);
		else fprintf(file, " moment %zd (%g s){\n", m, mApp->worker.moments[m - 1]);
		// Facets

		for (int i = 0; i < sh.nbFacet; i++) {
			Facet *f = facets[i];

			if (f->selected) {
				std::ostringstream line;

				line << i + 1 << sep << profType[f->sh.profileType] << sep << f->sh.O.x << sep << f->sh.O.y << sep << f->sh.O.z << sep << f->sh.U.x << sep << f->sh.U.y << sep << f->sh.U.z << sep;
				line << f->sh.V.x << sep << f->sh.V.y << sep << f->sh.V.z << sep << f->sh.U.Norme() << sep << f->sh.V.Norme() << sep << f->sh.center.x << sep << f->sh.center.y << sep << f->sh.center.z << sep << f->sh.maxSpeed << sep << f->facetHitCache.nbMCHit << sep << f->facetHitCache.nbHitEquiv << sep;

				if (f->sh.isProfile) {

					double dCoef = 1.0;

					GlobalHitBuffer& shGHit = results.globalHits;
					double nbDes = (shGHit.globalHits.nbDesorbed > 0) ? (double)shGHit.globalHits.nbDesorbed : 1.0;
					std::vector<ProfileSlice>& prof = results.facetStates[i].momentResults[m].profile;
					double scaleX, scaleY;
					switch (f->sh.profileType) {
					case PROFILE_U:
					case PROFILE_V:
						scaleY = 1.0 / (f->GetArea() / (double)PROFILE_SIZE*1E-4)* worker->wp.gasMass / 1000 / 6E23 * 0.0100; //0.01: Pa->mbar
						scaleY *= worker->GetMoleculesPerTP(m);

						for (int j = 0; j < PROFILE_SIZE; j++)
							line << prof[j].sum_v_ort*scaleY << sep;
						break;
					case PROFILE_VELOCITY:
					case PROFILE_ORT_VELOCITY:
					case PROFILE_TAN_VELOCITY:
						scaleX = f->sh.maxSpeed / (double)PROFILE_SIZE;
						for (int j = 0; j < PROFILE_SIZE; j++)
							line << prof[j].countEquiv / (f->facetHitCache.nbHitEquiv + static_cast<double>(f->facetHitCache.nbDesorbed)) << sep;
						break;
					case PROFILE_ANGULAR:
						scaleX = 90.0 / (double)PROFILE_SIZE;
						for (int j = 0; j < PROFILE_SIZE; j++)
							line << prof[j].countEquiv / (f->facetHitCache.nbHitEquiv + static_cast<double>(f->facetHitCache.nbDesorbed)) << sep;
						break;
					}
				}

				else {
					line << "No profile.";
				}
				line << '\n';
				fputs(line.str().c_str(), file);

			}

		}
		fputs(" }\n", file);

	}
	ReleaseMutex(results.mutex);
}

/* Commenting out as deprecated
void MolflowGeometry::ImportDesorption_DES(FileReader *file) {

	//if(!IsLoaded()) throw Error("Nothing to save !");

	// Block dpHit during the whole disc writing

	for (int i = 0; i < wp.nbFacet; i++) { //clear previous desorption maps
		facets[i]->hasOutgassingFile = false;
		facets[i]->wp.useOutgassingFile = false;
		facets[i]->wp.desorbType = DES_NONE; //clear previously set desorptions
		facets[i]->selected = false;
		facets[i]->UnselectElem();
	}
	// Facets
	while (!file->IsEof()) {
		file->ReadKeyword("facet");
		int facetId = file->ReadInt() - 1;
		file->ReadKeyword("{");
		if (!(facetId >= 0 && facetId < wp.nbFacet)) {
			file->MakeError("Invalid facet Id (loaded desorption file for a different geometry?)");
			return;
		}

		Facet *f = facets[facetId];
		f->hasOutgassingFile = true;
		f->wp.useOutgassingFile = true; //turn on file usage by default
		f->wp.desorbType = DES_COSINE; //auto-set to cosine
		SelectFacet(facetId);
		file->ReadKeyword("cell_size_cm"); file->ReadKeyword(":");
		double ratio = f->wp.outgassingFileRatio = file->ReadDouble();
		if (f->wp.outgassingFileRatio != 0.0) {
			f->wp.outgassingFileRatio = 1.0 / f->wp.outgassingFileRatio; //cell size -> samples per cm
			ratio = f->wp.outgassingFileRatio;
		}
		double nU = f->wp.U.Norme();
		double nV = f->wp.V.Norme();
		size_t w = f->wp.outgassingMapWidth = (size_t)ceil(nU*ratio); //double precision written to file
		size_t h = f->wp.outgassingMapHeight = (size_t)ceil(nV*ratio); //double precision written to file
		f->outgassingMap = (double*)malloc(w*h*sizeof(double));
		if (!f->outgassingMap) throw Error("Not enough memory to store outgassing map.");
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				f->outgassingMap[i + j*w] = file->ReadDouble();
			}
		}
		file->ReadKeyword("}");
	}
	UpdateSelection();
	//InitializeGeometry();

	//Debug memory check
	//_ASSERTE (!_CrtDumpMemoryLeaks());;
	_ASSERTE(_CrtCheckMemory());

}
*/

/**
* \brief For importing desorption data from a SYN file
* \param file name of the input file
* \param source what the source to calculate the dose is
* \param time time to calculate the dose
* \param mode mode used for outgassing calculation
* \param eta0 coefficient for outgassing calculation in mode==1
* \param alpha exponent for outgassing calculation in mode==1
* \param cutoffdose cutoff dose for outgassing calculation in mode==1
* \param convDistr distribution for outgassing calculation in mode==2
* \param prg GLProgress window where visualising of the import progress is shown
*/
void MolflowGeometry::ImportDesorption_SYN(
	FileReader *file, const size_t &source, const double &time,
	const size_t &mode, const double &eta0, const double &alpha, const double &cutoffdose,
	const std::vector<std::pair<double, double>> &convDistr,
	GLProgress *prg) {

	//UnselectAll();
	char tmp[512];
	std::vector<double> xdims, ydims;
	double no_scans = 1.0;

	file->ReadKeyword("version"); file->ReadKeyword(":");
	int version = file->ReadInt();
	if (version > SYNVERSION) {
		char errMsg[512];
		sprintf(errMsg, "Unsupported SYN version V%d", version);
		throw Error(errMsg);

	}

	//now read number of facets
	file->ReadKeyword("totalHit"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version >= 10) {
		file->ReadKeyword("totalHitEquiv"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("totalDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version >= 6) {
		file->ReadKeyword("no_scans"); file->ReadKeyword(":");
		no_scans = file->ReadDouble();
	}
	file->ReadKeyword("totalLeak"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version > 2) {
		file->ReadKeyword("totalFlux"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("totalPower"); file->ReadKeyword(":");
		file->ReadDouble();

	}
	if (version >= 10) {
		file->ReadKeyword("totalAbsEquiv"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("totalDist"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("maxDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("nbVertex"); file->ReadKeyword(":");
	file->ReadInt();
	file->ReadKeyword("nbFacet"); file->ReadKeyword(":");
	size_t nbNewFacet = file->ReadInt(); //gotcha! :)
	xdims.reserve(nbNewFacet);
	ydims.reserve(nbNewFacet);

	//now go for the facets to get their texture ratio
	for (size_t i = 0; i < Min(nbNewFacet, GetNbFacet()); i++) {
		prg->SetProgress(0.5*(double)i / (double)Min(nbNewFacet, GetNbFacet()));
		file->JumpSection("facet");
		// Check idx
		int idx = file->ReadInt();
		if (idx != i + 1) throw Error(file->MakeError("Wrong facet index !"));

		file->JumpSection("texDimX"); file->ReadKeyword(":");
		xdims.push_back(file->ReadDouble());
		//if (!IsZero(xdims[i])) SelectFacet(GetFacet(i));
		file->ReadKeyword("texDimY"); file->ReadKeyword(":");
		ydims.push_back(file->ReadDouble());

	}

	//now read actual textures
	//read header
	file->SeekFor("{textures}");
	file->ReadKeyword("minHit_MC"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("maxHit_MC"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("minHit_flux"); file->ReadKeyword(":");
	file->ReadDouble();
	file->ReadKeyword("maxHit_flux"); file->ReadKeyword(":");
	file->ReadDouble();
	file->ReadKeyword("minHit_power"); file->ReadKeyword(":");
	file->ReadDouble();
	file->ReadKeyword("maxHit_power"); file->ReadKeyword(":");
	file->ReadDouble();

	//read texture values
	for (size_t i = 0; i < Min(nbNewFacet, GetNbFacet()); i++) {
		prg->SetProgress(0.5 + 0.5*(double)i / (double)Min(nbNewFacet, GetNbFacet()));
		if (!IsZero(xdims[i])) { //has texture
			Facet *f = GetFacet(i);

			if (f->selected) {
				f->hasOutgassingFile = true;
				f->sh.useOutgassingFile = true; //turn on file usage by default
				f->sh.desorbType = DES_COSINE; //auto-set to cosine
			}
			//SelectFacet(f);
			file->ReadKeyword("texture_facet");
			// Check idx
			int idx = file->ReadInt();

			if (idx != i + 1) {
				sprintf(tmp, "Wrong facet index. Expected %zd, read %d.", i + 1, idx);
				throw Error(file->MakeError(tmp));
			}

			//Now load values
			file->ReadKeyword("{");

			size_t ix, iy;
			f->sh.outgassingMapWidth = (size_t)ceil(xdims[i] * 0.9999999);
			f->sh.outgassingMapHeight = (size_t)ceil(ydims[i] * 0.9999999);

			if (f->selected) {
				f->sh.outgassingFileRatio = xdims[i] / f->sh.U.Norme();
				try {
					std::vector<double>(f->sh.outgassingMapWidth*f->sh.outgassingMapHeight).swap(f->outgassingMap);
				}
				catch (...) {
					throw Error("Not enough memory to store outgassing map.");
				}
				f->totalDose = f->sh.totalOutgassing = f->totalFlux = 0.0;
			}

			size_t texWidth_file, texHeight_file;
			//In case of rounding errors, the file might contain different texture dimensions than expected.
			if (version >= 8) {
				file->ReadKeyword("width"); file->ReadKeyword(":"); texWidth_file = file->ReadInt();
				file->ReadKeyword("height"); file->ReadKeyword(":"); texHeight_file = file->ReadInt();
			}
			else {
				texWidth_file = f->sh.outgassingMapWidth;
				texHeight_file = f->sh.outgassingMapHeight;
			}

			for (iy = 0; iy < (Min(f->sh.outgassingMapHeight, texHeight_file)); iy++) { //MIN: If stored texture is larger, don't read extra cells
				for (ix = 0; ix < (Min(f->sh.outgassingMapWidth, texWidth_file)); ix++) { //MIN: If stored texture is larger, don't read extra cells
					size_t index = iy * f->sh.outgassingMapWidth + ix;
					//Read original values
					size_t MC = file->ReadSizeT();
					double cellArea = 1.0;
					if (version >= 7) cellArea = file->ReadDouble();
					if (cellArea < 1E-10) cellArea = 1.0; //to avoid division by zero
					double flux = file->ReadDouble() / no_scans; //not normalized by cell area
					double power = file->ReadDouble() / no_scans; //not normalized by cell area

					if (f->selected) {
						//Calculate dose
						double dose;
						if (source == 0) dose = (double)MC*time;
						else if (source == 1) dose = flux * time / cellArea;
						else if (source == 2) dose = power * time / cellArea;

						double outgassing;
						if (dose == 0) outgassing = 0; //to avoid division by zero later
						else {
							//Convert to outgassing
							if (mode == 0) {
								if (source == 0) outgassing = (double)MC * 0.100 / 1.38E-23 / f->sh.temperature;
								else if (source == 1) outgassing = flux * 0.100 / 1.38E-23 / f->sh.temperature; //Division by 10 because the user will want to see the same outgassing in mbar*l/s
								else if (source == 2) outgassing = power * 0.100 / 1.38E-23 / f->sh.temperature; //(Outgassing is stored internally in Pa*m3/s, for consistent SI unit calculations)
							}
							else if (mode == 1) {
								double moleculePerPhoton = eta0 * pow(Max(1.0, dose / cutoffdose), alpha);
								outgassing = flux * moleculePerPhoton;
							}
							else if (mode == 2) {
								double moleculePerPhoton = InterpolateY(dose, convDistr, false, true);
								outgassing = flux * moleculePerPhoton;
							}
						}
						//Apply outgassing
						//f->outgassingMap[index] = outgassing *0.100; //0.1: mbar*l/s->Pa*m3/s
						f->outgassingMap[index] = outgassing * 1.38E-23 * f->sh.temperature; //1[Pa*m3/s] = kT [particles/sec]

						//Facet diagnostic info
						f->totalDose += flux * time;
						f->totalFlux += flux;
						f->sh.totalOutgassing += f->outgassingMap[index];

					} //if selected
				}
				for (size_t ie = 0; ie < texWidth_file - f->sh.outgassingMapWidth; ie++) {//Executed if file texture is bigger than expected texture
					//Read extra cells from file without doing anything
					//Read original values
					file->ReadSizeT(); //MC
					if (version >= 7) file->ReadDouble(); //area
					file->ReadDouble(); //flux
					file->ReadDouble(); //power
				}
			}
			for (size_t ie = 0; ie < texHeight_file - f->sh.outgassingMapHeight; ie++) {//Executed if file texture is bigger than expected texture
				//Read extra cells ffrom file without doing anything
				for (size_t iw = 0; iw < texWidth_file; iw++) {
					//Read original values
					file->ReadSizeT(); //MC
					if (version >= 7) file->ReadDouble(); //area
					file->ReadDouble(); //flux
					file->ReadDouble(); //power
				}
			}
			file->ReadKeyword("}");
		} //if has texture
	}
	//end
	//UpdateSelection();
}

/**
* \brief To analyse desorption data from a SYN file
* \param file name of the input file
* \param progressDlg GLProgress dialog (TODO: but is it ever used?)
* \param nbNewFacet number of facets in the file
* \param nbTextured number of textured facets in the file
* \param nbDifferent number that is only set to 0 but never used (TODO: check usage)
* \param prg GLProgress window where visualising of the analysation progress is shown
*/
void MolflowGeometry::AnalyzeSYNfile(FileReader *file, GLProgress *progressDlg, size_t *nbNewFacet,
	size_t *nbTextured, size_t *nbDifferent, GLProgress *prg) {
	//init
	*nbTextured = 0;
	*nbNewFacet = 0;
	*nbDifferent = 0;

	UnselectAll();
	//char tmp[512];

	file->ReadKeyword("version"); file->ReadKeyword(":");
	int version;
	version = file->ReadInt();
	if (version > SYNVERSION) {
		char errMsg[512];
		sprintf(errMsg, "Unsupported SYN version V%d", version);
		throw Error(errMsg);

	}

	//now read number of facets
	file->ReadKeyword("totalHit"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version >= 10) {
		file->ReadKeyword("totalHitEquiv"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("totalDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version >= 6) {
		file->ReadKeyword("no_scans"); file->ReadKeyword(":");
		/*no_scans = */file->ReadDouble();

	}
	file->ReadKeyword("totalLeak"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version > 2) {
		file->ReadKeyword("totalFlux"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("totalPower"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	if (version >= 10) {
		file->ReadKeyword("totalAbsEquiv"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("totalDist"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("maxDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("nbVertex"); file->ReadKeyword(":");
	file->ReadInt();
	file->ReadKeyword("nbFacet"); file->ReadKeyword(":");
	*nbNewFacet = file->ReadInt(); //gotcha! :)

	//now go for the facets to get their texture ratio, etc.
	for (size_t i = 0; i < *nbNewFacet && i < GetNbFacet(); i++) {
		prg->SetProgress((double)i / (double)Min(*nbNewFacet, GetNbFacet()));
		file->JumpSection("facet");
		// Check idx
		int idx = file->ReadInt();
		if (idx != i + 1) throw Error(file->MakeError("Wrong facet index !"));

		file->JumpSection("mesh"); file->ReadKeyword(":");
		if (file->ReadInt()) { //has mesh
			(*nbTextured)++;
			SelectFacet(i);
			/*file->ReadKeyword("texDimX");file->ReadKeyword(":");
			if ((this->GetFacet(i)->wp.texWidthD-file->ReadDouble())>1E-8) {
			(*nbDifferent)++;
			continue;

			}
			file->ReadKeyword("texDimY");file->ReadKeyword(":");
			if ((this->GetFacet(i)->wp.texHeightD-file->ReadDouble())>1E-8) {
			(*nbDifferent)++;
			}*/

		}
	}
	UpdateSelection();

}

/**
* \brief To save geometry data into a XML file
* \param saveDoc xml output file
* \param work thread worker handling the task
* \param prg GLProgress window where visualising of the export progress is shown
* \param saveSelected saveSelected if a selection is to be saved
*/
void MolflowGeometry::SaveXML_geometry(pugi::xml_node saveDoc, Worker *work, GLProgress *prg, bool saveSelected) {
	//TiXmlDeclaration* decl = new TiXmlDeclaration("1.0")="")="");
	//saveDoc->LinkEndChild(decl);

	xml_node geomNode = saveDoc.append_child("Geometry");

	prg->SetMessage("Writing vertices...");
	geomNode.append_child("Vertices").append_attribute("nb") = sh.nbVertex; //creates Vertices node, adds nb attribute and sets its value to wp.nbVertex
	for (int i = 0; i < sh.nbVertex; i++) {
		prg->SetProgress(0.166*((double)i / (double)sh.nbVertex));
		xml_node v = geomNode.child("Vertices").append_child("Vertex");
		v.append_attribute("id") = i;
		v.append_attribute("x") = vertices3[i].x;
		v.append_attribute("y") = vertices3[i].y;
		v.append_attribute("z") = vertices3[i].z;
	}

	prg->SetMessage("Writing facets...");
	geomNode.append_child("Facets");
	geomNode.child("Facets").append_attribute("nb") = sh.nbFacet;
	for (int i = 0, k = 0; i < sh.nbFacet; i++) {
		prg->SetProgress(0.166 + ((double)i / (double)sh.nbFacet) *0.166);
		if (!saveSelected || facets[i]->selected) {
			xml_node f = geomNode.child("Facets").append_child("Facet");
			f.append_attribute("id") = i;
			facets[i]->SaveXML_geom(f);
		}
	}

	prg->SetMessage("Writing model details...");
	geomNode.append_child("Structures").append_attribute("nb") = sh.nbSuper;

	for (int i = 0, k = 0; i < sh.nbSuper; i++) {
		xml_node s = geomNode.child("Structures").append_child("Structure");
		s.append_attribute("id") = i;
		s.append_attribute("name") = (strName) ? strName[i] : "";

	}
	xml_node interfNode = saveDoc.append_child("Interface");

	xml_node selNode = interfNode.append_child("Selections");
	selNode.append_attribute("nb") = (!saveSelected)*(mApp->selections.size());
	for (size_t i = 0; (i < mApp->selections.size()) && !saveSelected; i++) { //don't save selections when exporting part of the geometry (saveSelected)
		xml_node newSel = selNode.append_child("Selection");
		newSel.append_attribute("id") = i;
		newSel.append_attribute("name") = mApp->selections[i].name.c_str();
		newSel.append_attribute("nb") = mApp->selections[i].selection.size();
		for (int j = 0; j < mApp->selections[i].selection.size(); j++) {
			xml_node newItem = newSel.append_child("selItem");
			newItem.append_attribute("id") = j;
			newItem.append_attribute("facet") = mApp->selections[i].selection[j];
		}
	}

	xml_node viewNode = interfNode.append_child("Views");
	viewNode.append_attribute("nb") = (!saveSelected)*(mApp->nbView);
	for (int i = 0; (i < mApp->nbView) && !saveSelected; i++) { //don't save views when exporting part of the geometry (saveSelected)
		xml_node newView = viewNode.append_child("View");
		newView.append_attribute("id") = i;
		newView.append_attribute("name") = mApp->views[i].name;
		newView.append_attribute("projMode") = mApp->views[i].projMode;
		newView.append_attribute("camAngleOx") = mApp->views[i].camAngleOx;
		newView.append_attribute("camAngleOy") = mApp->views[i].camAngleOy;
		newView.append_attribute("camAngleOz") = mApp->views[i].camAngleOz;
		newView.append_attribute("camDist") = mApp->views[i].camDist;
		newView.append_attribute("camOffset.x") = mApp->views[i].camOffset.x;
		newView.append_attribute("camOffset.y") = mApp->views[i].camOffset.y;
		newView.append_attribute("camOffset.z") = mApp->views[i].camOffset.z;
		newView.append_attribute("performXY") = mApp->views[i].performXY;
		newView.append_attribute("vLeft") = mApp->views[i].vLeft;
		newView.append_attribute("vRight") = mApp->views[i].vRight;
		newView.append_attribute("vTop") = mApp->views[i].vTop;
		newView.append_attribute("vBottom") = mApp->views[i].vBottom;
	}

	xml_node formulaNode = interfNode.append_child("Formulas");
	formulaNode.append_attribute("nb") = (!saveSelected)*(mApp->formulas_n.size());
	if (!saveSelected) { //don't save formulas when exporting part of the geometry (saveSelected)
		for (size_t i = 0; i < mApp->formulas_n.size(); i++) {
			xml_node newFormula = formulaNode.append_child("Formula");
			newFormula.append_attribute("id") = i;
			newFormula.append_attribute("name") = mApp->formulas_n[i]->GetName();
			newFormula.append_attribute("expression") = mApp->formulas_n[i]->GetExpression();
		}
	}

	if (mApp->profilePlotter) {
		std::vector<int> ppViews = mApp->profilePlotter->GetViews();
		xml_node profilePlotterNode = interfNode.append_child("ProfilePlotter");
		profilePlotterNode.append_child("Parameters").append_attribute("logScale") = (int)mApp->profilePlotter->IsLogScaled(); //backward compatibility: 0 or 1
		xml_node viewsNode = profilePlotterNode.append_child("Views");
		for (int v : ppViews) {
			xml_node view = viewsNode.append_child("View");
			view.append_attribute("facetId") = v;
		}
	}

	xml_node simuParamNode = saveDoc.append_child("MolflowSimuSettings");

	simuParamNode.append_child("Gas").append_attribute("mass") = work->wp.gasMass;
	simuParamNode.child("Gas").append_attribute("enableDecay") = (int)work->wp.enableDecay; //backward compatibility: 0 or 1
	simuParamNode.child("Gas").append_attribute("halfLife") = work->wp.halfLife;

	xml_node timeSettingsNode = simuParamNode.append_child("TimeSettings");

	xml_node userMomentsNode = timeSettingsNode.append_child("UserMoments");
	userMomentsNode.append_attribute("nb") = work->userMoments.size();
	for (size_t i = 0; i < work->userMoments.size(); i++) {
		xml_node newUserEntry = userMomentsNode.append_child("UserEntry");
		newUserEntry.append_attribute("id") = i;
		newUserEntry.append_attribute("content") = work->userMoments[i].c_str();
	}

	timeSettingsNode.append_attribute("timeWindow") = work->wp.timeWindowSize;
	timeSettingsNode.append_attribute("useMaxwellDistr") = (int)work->wp.useMaxwellDistribution; //backward compatibility: 0 or 1
	timeSettingsNode.append_attribute("calcConstFlow") = (int)work->wp.calcConstantFlow; //backward compatibility: 0 or 1

	xml_node motionNode = simuParamNode.append_child("Motion");
	motionNode.append_attribute("type") = work->wp.motionType;
	if (work->wp.motionType == 1) { //fixed motion
		xml_node v = motionNode.append_child("VelocityVector");
		v.append_attribute("vx") = work->wp.motionVector2.x;
		v.append_attribute("vy") = work->wp.motionVector2.y;
		v.append_attribute("vz") = work->wp.motionVector2.z;
	}
	else if (work->wp.motionType == 2) { //rotation
		xml_node v = motionNode.append_child("AxisBasePoint");
		v.append_attribute("x") = work->wp.motionVector1.x;
		v.append_attribute("y") = work->wp.motionVector1.y;
		v.append_attribute("z") = work->wp.motionVector1.z;
		xml_node v2 = motionNode.append_child("RotationVector");
		v2.append_attribute("x") = work->wp.motionVector2.x;
		v2.append_attribute("y") = work->wp.motionVector2.y;
		v2.append_attribute("z") = work->wp.motionVector2.z;
	}

	xml_node paramNode = simuParamNode.append_child("Parameters");
	size_t nonCatalogParameters = 0;
	
	for (size_t i = 0; i < work->parameters.size(); i++) {
		if (work->parameters[i].fromCatalog == false) { //Don't save catalog parameters
			xml_node newParameter = paramNode.append_child("Parameter");
			newParameter.append_attribute("id") = nonCatalogParameters;
			newParameter.append_attribute("name") = work->parameters[i].name.c_str();
			newParameter.append_attribute("nbMoments") = (int)work->parameters[i].GetSize();
			for (size_t m = 0; m < work->parameters[i].GetSize(); m++) {
				xml_node newMoment = newParameter.append_child("Moment");
				newMoment.append_attribute("id") = m;
				newMoment.append_attribute("t") = work->parameters[i].GetX(m);
				newMoment.append_attribute("value") = work->parameters[i].GetY(m);
			}
			nonCatalogParameters++;
		}
	}
	paramNode.append_attribute("nb") = nonCatalogParameters;
}

/**
* \brief To save simulation data into a XML file
* \param saveDoc xml output file
* \param work thread worker handling the task
* \param results simulation results
* \param prg GLProgress window where visualising of the export progress is shown
* \param saveSelected saveSelected if a selection is to be saved (TODO: check if necessary)
* \return bool if saving is successfull (always is here)
*/
bool MolflowGeometry::SaveXML_simustate(xml_node saveDoc, Worker *work, GlobalSimuState& results, GLProgress *prg, bool saveSelected) {
	//results is already mutex locked
	xml_node resultNode = saveDoc.append_child("MolflowResults");
	prg->SetMessage("Writing simulation results...");
	xml_node momentsNode = resultNode.append_child("Moments");
	momentsNode.append_attribute("nb") = work->moments.size() + 1;
	size_t facetHitsSize = (1 + mApp->worker.moments.size()) * sizeof(FacetHitBuffer);
	GlobalHitBuffer& gHits = results.globalHits;
	for (size_t m = 0; m <= mApp->worker.moments.size(); m++) {
		prg->SetProgress(0.5 + 0.5*(double)m / (1.0 + (double)mApp->worker.moments.size()));
		xml_node newMoment = momentsNode.append_child("Moment");
		newMoment.append_attribute("id") = m;
		if (m == 0)
			newMoment.append_attribute("time") = "Constant flow";
		else
			newMoment.append_attribute("time") = work->moments[m - 1];

		if (m == 0) { //Write global results. Later these results will probably be time-dependent as well.
			xml_node globalNode = newMoment.append_child("Global");

			xml_node hitsNode = globalNode.append_child("Hits");
			hitsNode.append_attribute("totalHit") = gHits.globalHits.nbMCHit;
			hitsNode.append_attribute("totalHitEquiv") = gHits.globalHits.nbHitEquiv;
			hitsNode.append_attribute("totalDes") = gHits.globalHits.nbDesorbed;
			hitsNode.append_attribute("totalAbsEquiv") = gHits.globalHits.nbAbsEquiv;
			hitsNode.append_attribute("totalDist_total") = gHits.distTraveled_total;
			hitsNode.append_attribute("totalDist_fullHitsOnly") = gHits.distTraveledTotal_fullHitsOnly;
			hitsNode.append_attribute("totalLeak") = gHits.nbLeakTotal;
			hitsNode.append_attribute("maxDesorption") = work->ontheflyParams.desorptionLimit;

			xml_node hitCacheNode = globalNode.append_child("Hit_Cache");
			hitCacheNode.append_attribute("nb") = work->globalHitCache.hitCacheSize;

			for (int i = 0; i < work->globalHitCache.hitCacheSize; i++) {
				xml_node newHit = hitCacheNode.append_child("Hit");
				newHit.append_attribute("id") = i;
				newHit.append_attribute("posX") = work->globalHitCache.hitCache[i].pos.x;
				newHit.append_attribute("posY") = work->globalHitCache.hitCache[i].pos.y;
				newHit.append_attribute("posZ") = work->globalHitCache.hitCache[i].pos.z;
				newHit.append_attribute("type") = work->globalHitCache.hitCache[i].type;
			}

			xml_node leakCacheNode = globalNode.append_child("Leak_Cache");
			leakCacheNode.append_attribute("nb") = work->globalHitCache.leakCacheSize;
			for (int i = 0; i < work->globalHitCache.leakCacheSize; i++) {
				xml_node newLeak = leakCacheNode.append_child("Leak");
				newLeak.append_attribute("id") = i;
				newLeak.append_attribute("posX") = work->globalHitCache.leakCache[i].pos.x;
				newLeak.append_attribute("posY") = work->globalHitCache.leakCache[i].pos.y;
				newLeak.append_attribute("posZ") = work->globalHitCache.leakCache[i].pos.z;
				newLeak.append_attribute("dirX") = work->globalHitCache.leakCache[i].dir.x;
				newLeak.append_attribute("dirY") = work->globalHitCache.leakCache[i].dir.y;
				newLeak.append_attribute("dirZ") = work->globalHitCache.leakCache[i].dir.z;
			}
		} //end global node

		xml_node facetResultsNode = newMoment.append_child("FacetResults");

		for (int i = 0; i < sh.nbFacet; i++) {
			Facet *f = GetFacet(i);
			xml_node newFacetResult = facetResultsNode.append_child("Facet");
			newFacetResult.append_attribute("id") = i;

			xml_node facetHitNode = newFacetResult.append_child("Hits");
			FacetHitBuffer& facetCounter = results.facetStates[i].momentResults[m].hits;
			facetHitNode.append_attribute("nbHit") = facetCounter.nbMCHit;
			facetHitNode.append_attribute("nbHitEquiv") = facetCounter.nbHitEquiv;
			facetHitNode.append_attribute("nbDes") = facetCounter.nbDesorbed;
			facetHitNode.append_attribute("nbAbsEquiv") = facetCounter.nbAbsEquiv;
			facetHitNode.append_attribute("sum_v_ort") = facetCounter.sum_v_ort;
			facetHitNode.append_attribute("sum_1_per_v") = facetCounter.sum_1_per_ort_velocity;
			facetHitNode.append_attribute("sum_v") = facetCounter.sum_1_per_velocity;

			if (f->sh.isProfile) {
				xml_node profileNode = newFacetResult.append_child("Profile");
				profileNode.append_attribute("size") = PROFILE_SIZE;
				std::vector<ProfileSlice>& pr = results.facetStates[i].momentResults[m].profile;
				for (int p = 0; p < PROFILE_SIZE; p++) {
					xml_node slice = profileNode.append_child("Slice");
					slice.append_attribute("id") = p;
					slice.append_attribute("countEquiv") = pr[p].countEquiv;
					slice.append_attribute("sum_1_per_v") = pr[p].sum_1_per_ort_velocity;
					slice.append_attribute("sum_v_ort") = pr[p].sum_v_ort;
				}
			}

			size_t profSize = (f->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice)*(1 + mApp->worker.moments.size())) : 0;
			size_t h = f->sh.texHeight;
			size_t w = f->sh.texWidth;

			if (f->hasMesh) {
				xml_node textureNode = newFacetResult.append_child("Texture");
				textureNode.append_attribute("width") = f->sh.texWidth;
				textureNode.append_attribute("height") = f->sh.texHeight;

				std::vector<TextureCell>& texture = results.facetStates[i].momentResults[m].texture;
				std::stringstream countText, sum1perText, sumvortText;
				countText << '\n'; //better readability in file
				sum1perText << std::setprecision(8) << '\n';
				sumvortText << std::setprecision(8) << '\n';

				for (size_t iy = 0; iy < h; iy++) {
					for (size_t ix = 0; ix < w; ix++) {
						countText << texture[iy*f->sh.texWidth + ix].countEquiv << '\t';
						sum1perText << texture[iy*f->sh.texWidth + ix].sum_1_per_ort_velocity << '\t';
						sumvortText << texture[iy*f->sh.texWidth + ix].sum_v_ort_per_area << '\t';

					}
					countText << '\n';
					sum1perText << '\n';
					sumvortText << '\n';
				}
				textureNode.append_child("count").append_child(node_cdata).set_value(countText.str().c_str());
				textureNode.append_child("sum_1_per_v").append_child(node_cdata).set_value(sum1perText.str().c_str());
				textureNode.append_child("sum_v_ort").append_child(node_cdata).set_value(sumvortText.str().c_str());

			} //end texture

			if (f->sh.countDirection && f->dirCache) {
				xml_node dirNode = newFacetResult.append_child("Directions");
				dirNode.append_attribute("width") = f->sh.texWidth;
				dirNode.append_attribute("height") = f->sh.texHeight;

				std::vector<DirectionCell>& dirs = results.facetStates[i].momentResults[m].direction;
				std::stringstream dirText, dirCountText;
				dirText << std::setprecision(8) << '\n'; //better readability in file
				dirCountText << '\n';

				for (size_t iy = 0; iy < h; iy++) {
					for (size_t ix = 0; ix < w; ix++) {
						dirText << dirs[iy*f->sh.texWidth + ix].dir.x << ",";
						dirText << dirs[iy*f->sh.texWidth + ix].dir.y << ",";
						dirText << dirs[iy*f->sh.texWidth + ix].dir.z << "\t";
						dirCountText << dirs[iy*f->sh.texWidth + ix].count << "\t";

					}
					dirText << "\n";
					dirCountText << "\n";
				}
				dirNode.append_child("vel.vectors").append_child(node_cdata).set_value(dirText.str().c_str());
				dirNode.append_child("count").append_child(node_cdata).set_value(dirCountText.str().c_str());
			} //end directions
		}
	}

	//Texture Min/Max
	xml_node minMaxNode = resultNode.append_child("TextureMinMax");
	minMaxNode.append_child("With_constant_flow").append_child("Pressure").append_attribute("min") = gHits.texture_limits[0].min.all;
	minMaxNode.child("With_constant_flow").child("Pressure").append_attribute("max") = gHits.texture_limits[0].max.all;
	minMaxNode.child("With_constant_flow").append_child("Density").append_attribute("min") = gHits.texture_limits[1].min.all;
	minMaxNode.child("With_constant_flow").child("Density").append_attribute("max") = gHits.texture_limits[1].max.all;
	minMaxNode.child("With_constant_flow").append_child("Imp.rate").append_attribute("min") = gHits.texture_limits[2].min.all;
	minMaxNode.child("With_constant_flow").child("Imp.rate").append_attribute("max") = gHits.texture_limits[2].max.all;

	minMaxNode.append_child("Moments_only").append_child("Pressure").append_attribute("min") = gHits.texture_limits[0].min.moments_only;
	minMaxNode.child("Moments_only").child("Pressure").append_attribute("max") = gHits.texture_limits[0].max.moments_only;
	minMaxNode.child("Moments_only").append_child("Density").append_attribute("min") = gHits.texture_limits[1].min.moments_only;
	minMaxNode.child("Moments_only").child("Density").append_attribute("max") = gHits.texture_limits[1].max.moments_only;
	minMaxNode.child("Moments_only").append_child("Imp.rate").append_attribute("min") = gHits.texture_limits[2].min.moments_only;
	minMaxNode.child("Moments_only").child("Imp.rate").append_attribute("max") = gHits.texture_limits[2].max.moments_only;

	return true;
}

/**
* \brief To load geometry data from a XML file
* \param loadXML xml input file
* \param work thread worker handling the task
* \param progressDlg GLProgress window where visualising of the import progress is shown
*/
void MolflowGeometry::LoadXML_geom(pugi::xml_node loadXML, Worker *work, GLProgress *progressDlg) {
	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	//mApp->ClearFormulas();
	Clear();
	xml_node geomNode = loadXML.child("Geometry");

	//Vertices
	sh.nbVertex = geomNode.child("Vertices").select_nodes("Vertex").size();

	vertices3.resize(sh.nbVertex); vertices3.shrink_to_fit();
	size_t idx = 0;
	for (xml_node vertex : geomNode.child("Vertices").children("Vertex")) {
		vertices3[idx].x = vertex.attribute("x").as_double();
		vertices3[idx].y = vertex.attribute("y").as_double();
		vertices3[idx].z = vertex.attribute("z").as_double();
		vertices3[idx].selected = false;
		idx++;

	}

	//Structures
	sh.nbSuper = geomNode.child("Structures").select_nodes("Structure").size();
	idx = 0;
	for (xml_node structure : geomNode.child("Structures").children("Structure")) {
		strName[idx] = strdup(structure.attribute("name").value());
		// For backward compatibilty with STR
		char tmp[256];
		sprintf(tmp, "%s.txt", strName[idx]);
		strFileName[idx] = strdup(tmp);
		idx++;
	}

	//Parameters (needs to precede facets)
	xml_node simuParamNode = loadXML.child("MolflowSimuSettings");
	bool isMolflowFile = (simuParamNode != NULL); //if no "MolflowSimuSettings" node, it's a Synrad file

	{
		std::vector<Parameter> loadedParams;
		if (isMolflowFile) {
			xml_node paramNode = simuParamNode.child("Parameters");
			for (xml_node newParameter : paramNode.children("Parameter")) {
				Parameter newPar;
				newPar.name = newParameter.attribute("name").as_string();
				for (xml_node newMoment : newParameter.children("Moment")) {
					newPar.AddPair(std::make_pair(newMoment.attribute("t").as_double(),
						newMoment.attribute("value").as_double()));
				}
				loadedParams.push_back(newPar);
			}
		}
		work->InsertParametersBeforeCatalog(loadedParams);
	}

	//Facets
	sh.nbFacet = geomNode.child("Facets").select_nodes("Facet").size();
	facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));
	idx = 0;
	bool ignoreSumMismatch = false;
	for (xml_node facetNode : geomNode.child("Facets").children("Facet")) {
		size_t nbIndex = facetNode.child("Indices").select_nodes("Indice").size();
		if (nbIndex < 3) {
			char errMsg[128];
			sprintf(errMsg, "Facet %zd has only %zd vertices. ", idx + 1, nbIndex);
			throw Error(errMsg);
		}

		facets[idx] = new Facet(nbIndex);
		facets[idx]->LoadXML(facetNode, sh.nbVertex, isMolflowFile, ignoreSumMismatch);

		if (isMolflowFile) {
			//Set param names for interface
			if (facets[idx]->sh.sticking_paramId > -1) facets[idx]->userSticking = work->parameters[facets[idx]->sh.sticking_paramId].name;
			if (facets[idx]->sh.opacity_paramId > -1) facets[idx]->userOpacity = work->parameters[facets[idx]->sh.opacity_paramId].name;
			if (facets[idx]->sh.outgassing_paramId > -1) facets[idx]->userOutgassing = work->parameters[facets[idx]->sh.outgassing_paramId].name;
		}
		idx++;
	}

	xml_node interfNode = loadXML.child("Interface");

	xml_node selNode = interfNode.child("Selections");
	//int nbS = selNode.select_nodes("Selection").size();

	for (xml_node sNode : selNode.children("Selection")) {
		SelectionGroup s;
		s.name = strdup(sNode.attribute("name").as_string());
		s.selection.reserve(sNode.select_nodes("selItem").size());
		for (xml_node iNode : sNode.children("selItem"))
			s.selection.push_back(iNode.attribute("facet").as_llong());
		mApp->AddSelection(s);
	}

	xml_node viewNode = interfNode.child("Views");
	for (xml_node newView : viewNode.children("View")) {
		AVIEW v;
		v.name = strdup(newView.attribute("name").as_string());
		v.projMode = newView.attribute("projMode").as_int();
		v.camAngleOx = newView.attribute("camAngleOx").as_double();
		v.camAngleOy = newView.attribute("camAngleOy").as_double();
		if (newView.attribute("camAngleOz")) {
			v.camAngleOz = newView.attribute("camAngleOz").as_double();
		}
		else {
			v.camAngleOz = 0.0; //Otherwise RoundAngle() routine hangs for unitialized value
		}
		v.camDist = newView.attribute("camDist").as_double();
		v.camOffset.x = newView.attribute("camOffset.x").as_double();
		v.camOffset.y = newView.attribute("camOffset.y").as_double();
		v.camOffset.z = newView.attribute("camOffset.z").as_double();
		v.performXY = newView.attribute("performXY").as_int();
		v.vLeft = newView.attribute("vLeft").as_double();
		v.vRight = newView.attribute("vRight").as_double();
		v.vTop = newView.attribute("vTop").as_double();
		v.vBottom = newView.attribute("vBottom").as_double();
		mApp->AddView(v.name, v);

	}

	if (isMolflowFile) {
		xml_node formulaNode = interfNode.child("Formulas");
		for (xml_node newFormula : formulaNode.children("Formula")) {
			mApp->AddFormula(newFormula.attribute("name").as_string(),
				newFormula.attribute("expression").as_string());
		}

		xml_node ppNode = interfNode.child("ProfilePlotter");
		if (ppNode) {
			if (!mApp->profilePlotter) mApp->profilePlotter = new ProfilePlotter(); mApp->profilePlotter->SetWorker(work);
			xml_node paramsNode = ppNode.child("Parameters");
			if (paramsNode && paramsNode.attribute("logScale"))
				mApp->profilePlotter->SetLogScaled(paramsNode.attribute("logScale").as_bool());
			xml_node viewsNode = ppNode.child("Views");
			if (viewsNode) {
				std::vector<int> views;
				for (xml_node view : viewsNode.children("View"))
					views.push_back(view.attribute("facetId").as_int());
				mApp->profilePlotter->SetViews(views);
			}
		}

		work->wp.gasMass = simuParamNode.child("Gas").attribute("mass").as_double();
		work->wp.halfLife = simuParamNode.child("Gas").attribute("halfLife").as_double();
		if (simuParamNode.child("Gas").attribute("enableDecay")) {
			work->wp.enableDecay = simuParamNode.child("Gas").attribute("enableDecay").as_bool();
		}
		else {
			work->wp.enableDecay = work->wp.halfLife < 1e100;
		}

		xml_node timeSettingsNode = simuParamNode.child("TimeSettings");

		xml_node userMomentsNode = timeSettingsNode.child("UserMoments");
		for (xml_node newUserEntry : userMomentsNode.children("UserEntry")) {
			char tmpExpr[512];
			strcpy(tmpExpr, newUserEntry.attribute("content").as_string());
			work->userMoments.push_back(tmpExpr);
			work->AddMoment(mApp->worker.ParseMoment(tmpExpr));
		}

		/*
		//Initialize facet counters
		for (size_t i = 0;i < wp.nbFacet;i++) {
			memset(&(facets[i]->facetHitCache), 0, sizeof(FacetHitBuffer));
		}
		*/

		work->wp.timeWindowSize = timeSettingsNode.attribute("timeWindow").as_double();
		work->wp.useMaxwellDistribution = timeSettingsNode.attribute("useMaxwellDistr").as_bool();
		work->wp.calcConstantFlow = timeSettingsNode.attribute("calcConstFlow").as_bool();

		xml_node motionNode = simuParamNode.child("Motion");
		work->wp.motionType = motionNode.attribute("type").as_int();
		if (work->wp.motionType == 1) { //fixed motion
			xml_node v = motionNode.child("VelocityVector");
			work->wp.motionVector2.x = v.attribute("vx").as_double();
			work->wp.motionVector2.y = v.attribute("vy").as_double();
			work->wp.motionVector2.z = v.attribute("vz").as_double();
		}
		else if (work->wp.motionType == 2) { //rotation
			xml_node v = motionNode.child("AxisBasePoint");
			work->wp.motionVector1.x = v.attribute("x").as_double();
			work->wp.motionVector1.y = v.attribute("y").as_double();
			work->wp.motionVector1.z = v.attribute("z").as_double();
			xml_node v2 = motionNode.child("RotationVector");
			work->wp.motionVector2.x = v2.attribute("x").as_double();
			work->wp.motionVector2.y = v2.attribute("y").as_double();
			work->wp.motionVector2.z = v2.attribute("z").as_double();
		}
	}

	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

	// Update mesh
	progressDlg->SetMessage("Building mesh...");
	for (size_t i = 0; i < sh.nbFacet; i++) {
		double p = (double)i / (double)sh.nbFacet;

		progressDlg->SetProgress(p);
		Facet *f = facets[i];
		if (!f->SetTexture(f->sh.texWidthD, f->sh.texHeightD, f->hasMesh)) {
			char errMsg[512];
			sprintf(errMsg, "Not enough memory to build mesh on Facet %zd. ", i + 1);
			throw Error(errMsg);
		}
		BuildFacetList(f);
		double nU = f->sh.U.Norme();
		f->tRatio = f->sh.texWidthD / nU;
	}
}

/**
* \brief To load geometry data from a XML file and insert into an existing structure
* \param loadXML xml input file
* \param work thread worker handling the task
* \param progressDlg GLProgress window where visualising of the insert progress is shown
* \param newStr if a new super structure is to be used
*/
void MolflowGeometry::InsertXML(pugi::xml_node loadXML, Worker *work, GLProgress *progressDlg, bool newStr) {
	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	//mApp->ClearFormulas();
	//Clear();
	int structId = viewStruct;
	if (structId == -1) structId = 0;
	UnselectAll();

	xml_node geomNode = loadXML.child("Geometry");
	//Vertices
	size_t nbNewVertex = geomNode.child("Vertices").select_nodes("Vertex").size();
	size_t nbNewFacets = geomNode.child("Facets").select_nodes("Facet").size();

	// reallocate memory
	facets = (Facet **)realloc(facets, (nbNewFacets + sh.nbFacet) * sizeof(Facet **));
	memset(facets + sh.nbFacet, 0, nbNewFacets * sizeof(Facet *));

	/*
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + wp.nbVertex) * sizeof(InterfaceVertex));
	memmove(tmp_vertices3, vertices3, (wp.nbVertex)*sizeof(InterfaceVertex));
	memset(tmp_vertices3 + wp.nbVertex, 0, nbNewVertex * sizeof(InterfaceVertex));
	SAFE_FREE(vertices3);
	vertices3 = tmp_vertices3;
	*/
	vertices3.resize(nbNewVertex + sh.nbVertex);

	// Read geometry vertices
	size_t idx = sh.nbVertex;
	for (xml_node vertex : geomNode.child("Vertices").children("Vertex")) {
		vertices3[idx].x = vertex.attribute("x").as_double();
		vertices3[idx].y = vertex.attribute("y").as_double();
		vertices3[idx].z = vertex.attribute("z").as_double();
		vertices3[idx].selected = false;
		idx++;

	}

	//Structures
	size_t nbNewSuper = geomNode.child("Structures").select_nodes("Structure").size();
	idx = 0;
	for (xml_node structure : geomNode.child("Structures").children("Structure")) {
		strName[sh.nbSuper + idx] = strdup(structure.attribute("name").value());
		// For backward compatibilty with STR
		char tmp[256];
		sprintf(tmp, "%s.txt", strName[idx]);
		strFileName[sh.nbSuper + idx] = strdup(tmp);
		idx++;
	}

	//Parameters (needs to precede facets)
	xml_node simuParamNode = loadXML.child("MolflowSimuSettings");
	bool isMolflowFile = (simuParamNode != NULL); //if no "MolflowSimuSettings" node, it's a Synrad XML file

	{
		std::vector<Parameter> loadedParams;
		if (isMolflowFile) {
			xml_node paramNode = simuParamNode.child("Parameters");
			for (xml_node newParameter : paramNode.children("Parameter")) {
				Parameter newPar;
				newPar.name = newParameter.attribute("name").as_string();
				for (xml_node newMoment : newParameter.children("Moment")) {
					newPar.AddPair(std::make_pair(newMoment.attribute("t").as_double(),
						newMoment.attribute("value").as_double()));
				}
				loadedParams.push_back(newPar);
			}
		}
		work->InsertParametersBeforeCatalog(loadedParams);
	}

	//Facets
	idx = sh.nbFacet;
	bool ignoreSumMismatch = false;
	for (xml_node facetNode : geomNode.child("Facets").children("Facet")) {
		size_t nbIndex = facetNode.child("Indices").select_nodes("Indice").size();
		if (nbIndex < 3) {
			char errMsg[128];
			sprintf(errMsg, "Facet %zd has only %zd vertices. ", idx + 1, nbIndex);
			throw Error(errMsg);
		}
		facets[idx] = new Facet(nbIndex);
		facets[idx]->LoadXML(facetNode, sh.nbVertex + nbNewVertex, isMolflowFile, ignoreSumMismatch, sh.nbVertex);
		facets[idx]->selected = true;

		if (newStr) {
			if (facets[idx]->sh.superIdx != -1) //-1 = facet member of all structures
				facets[idx]->sh.superIdx += static_cast<int>(sh.nbSuper); //offset structure
			if (facets[idx]->sh.superDest > 0) facets[idx]->sh.superDest += sh.nbSuper;
		}
		else {
			if (facets[idx]->sh.superIdx != -1) //-1 = facet member of all structures
				facets[idx]->sh.superIdx += structId; //offset structure
			if (facets[idx]->sh.superDest > 0) facets[idx]->sh.superDest += structId;
		}

		if (isMolflowFile) {
			//Set param names for interface
			if (facets[idx]->sh.sticking_paramId > -1) facets[idx]->userSticking = work->parameters[facets[idx]->sh.sticking_paramId].name;
			if (facets[idx]->sh.opacity_paramId > -1) facets[idx]->userOpacity = work->parameters[facets[idx]->sh.opacity_paramId].name;
			if (facets[idx]->sh.outgassing_paramId > -1) facets[idx]->userOutgassing = work->parameters[facets[idx]->sh.outgassing_paramId].name;
		}
		idx++;
	}

	xml_node interfNode = loadXML.child("Interface");
	xml_node selNode = interfNode.child("Selections");
	//int nbS = selNode.select_nodes("Selection").size();

	for (xml_node sNode : selNode.children("Selection")) {
		SelectionGroup s;
		s.name = strdup(sNode.attribute("name").as_string());
		size_t nbSel = sNode.select_nodes("selItem").size();
		for (xml_node iNode : sNode.children("selItem"))
			s.selection.push_back(iNode.attribute("facet").as_int() + sh.nbFacet); //offset selection numbers
		mApp->AddSelection(s);
	}

	xml_node viewNode = interfNode.child("Views");
	for (xml_node newView : selNode.children("View")) {
		AVIEW v;
		v.name = strdup(newView.attribute("name").as_string());
		v.projMode = newView.attribute("projMode").as_int();
		v.camAngleOx = newView.attribute("camAngleOx").as_double();
		v.camAngleOy = newView.attribute("camAngleOy").as_double();
		if (newView.attribute("camAngleOz")) {
			v.camAngleOz = newView.attribute("camAngleOz").as_double();
		}
		else {
			v.camAngleOz = 0.0; //Otherwise RoundAngle() routine hangs for unitialized value
		}
		v.camDist = newView.attribute("camDist").as_double();
		v.camOffset.x = newView.attribute("camOffset.x").as_double();
		v.camOffset.y = newView.attribute("camOffset.y").as_double();
		v.camOffset.z = newView.attribute("camOffset.z").as_double();
		v.performXY = newView.attribute("performXY").as_int();
		v.vLeft = newView.attribute("vLeft").as_double();
		v.vRight = newView.attribute("vRight").as_double();
		v.vTop = newView.attribute("vTop").as_double();
		v.vBottom = newView.attribute("vBottom").as_double();
		mApp->AddView(v.name, v);
	}

	sh.nbVertex += nbNewVertex;
	sh.nbFacet += nbNewFacets; //formulas can refer to newly inserted facets

	if (isMolflowFile) {
		xml_node formulaNode = interfNode.child("Formulas");
		for (xml_node newFormula : formulaNode.children("Formula")) {
			char tmpExpr[512];
			strcpy(tmpExpr, newFormula.attribute("expression").as_string());
			mApp->OffsetFormula(tmpExpr, (int)sh.nbFacet);
			mApp->AddFormula(newFormula.attribute("name").as_string(),
				tmpExpr);
		}
	}

	/*work->wp.gasMass = simuParamNode.child("Gas").attribute("mass").as_double();
	work->wp.halfLife = simuParamNode.child("Gas").attribute("wp.halfLife").as_double();*/

	/*
	xml_node timeSettingsNode = simuParamNode.child("TimeSettings");

	xml_node userMomentsNode = timeSettingsNode.child("UserMoments");
	for (xml_node newUserEntry : userMomentsNode.children("UserEntry")) {
	char tmpExpr[512];
	strcpy(tmpExpr, newUserEntry.attribute("content").as_string());
	work->userMoments.push_back(tmpExpr);
	work->AddMoment(mApp->worker.ParseMoment(tmpExpr));

	}
	work->wp.wp.timeWindowSize = timeSettingsNode.attribute("timeWindow").as_double();
	work->wp.useMaxwellDistribution = timeSettingsNode.attribute("useMaxwellDistr").as_int();
	work->wp.calcConstantFlow = timeSettingsNode.attribute("calcConstFlow").as_int();
	*/

	if (newStr) sh.nbSuper += nbNewSuper;
	else if (sh.nbSuper < structId + nbNewSuper) sh.nbSuper = structId + nbNewSuper;
	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

	// Update mesh for newly inserted facets
	progressDlg->SetMessage("Building mesh...");
	for (size_t i = sh.nbFacet - nbNewFacets; i < sh.nbFacet; i++) {
		double p = (double)(sh.nbFacet-i) / (double)nbNewFacets;

		progressDlg->SetProgress(p);
		Facet *f = facets[i];
		if (!f->SetTexture(f->sh.texWidthD, f->sh.texHeightD, f->hasMesh)) {
			char errMsg[512];
			sprintf(errMsg, "Not enough memory to build mesh on Facet %zd. ", i + 1);
			throw Error(errMsg);
		}
		BuildFacetList(f);
		double nU = f->sh.U.Norme();
		f->tRatio = f->sh.texWidthD / nU;
	}
}

/**
* \brief To load simulation data from a XML file
* \param loadXML xml input file
* \param results current simulation results
* \param work thread worker handling the task
* \param progressDlg GLProgress window where visualising of the load progress is shown
* \return bool showing if loading was successful
*/
bool MolflowGeometry::LoadXML_simustate(pugi::xml_node loadXML, GlobalSimuState& results, Worker *work, GLProgress *progressDlg) {
	if (!loadXML.child("MolflowResults")) return false; //simu state not saved with file
	LockMutex(results.mutex);
	GlobalHitBuffer &gHits = results.globalHits;
	xml_node resultNode = loadXML.child("MolflowResults");
	xml_node momentsNode = resultNode.child("Moments");
	size_t nbMoments = momentsNode.select_nodes("Moment").size(); //Contains constant flow!
	size_t facetHitsSize = (nbMoments) * sizeof(FacetHitBuffer);
	size_t m = 0;
	for (xml_node newMoment : momentsNode.children("Moment")) {
		progressDlg->SetProgress((double)m / (double)nbMoments);
		if (m == 0) { //read global results
			xml_node globalNode = newMoment.child("Global");
			xml_node hitsNode = globalNode.child("Hits");
			work->globalHitCache.globalHits.nbMCHit = hitsNode.attribute("totalHit").as_llong();
			if (hitsNode.attribute("totalHitEquiv")) {
				work->globalHitCache.globalHits.nbHitEquiv = hitsNode.attribute("totalHitEquiv").as_double();
			}
			else {
				//Backward compatibility
				work->globalHitCache.globalHits.nbHitEquiv = static_cast<double>(work->globalHitCache.globalHits.nbMCHit);
			}
			work->globalHitCache.globalHits.nbDesorbed = hitsNode.attribute("totalDes").as_llong();
			if (hitsNode.attribute("totalAbsEquiv")) {
				work->globalHitCache.globalHits.nbAbsEquiv = hitsNode.attribute("totalAbsEquiv").as_double();
			}
			else {
				//Backward compatibility
				work->globalHitCache.globalHits.nbAbsEquiv = hitsNode.attribute("totalAbs").as_double();
			}
			if (hitsNode.attribute("totalDist_total")) { //if it's in the new format where total/partial are separated
				work->globalHitCache.distTraveled_total = hitsNode.attribute("totalDist_total").as_double();
				work->globalHitCache.distTraveledTotal_fullHitsOnly = hitsNode.attribute("totalDist_fullHitsOnly").as_double();
			}
			else
				work->globalHitCache.distTraveled_total = work->globalHitCache.distTraveledTotal_fullHitsOnly = hitsNode.attribute("totalDist").as_double();
			work->globalHitCache.nbLeakTotal = hitsNode.attribute("totalLeak").as_llong();
			//work->desorptionLimit=hitsNode.attribute("maxDesorption").as_llong();

			work->globalHitCache.hitCacheSize = 0;
			xml_node hitCacheNode = globalNode.child("Hit_Cache");
			for (xml_node newHit : hitCacheNode.children("Hit")) {
				if (work->globalHitCache.hitCacheSize < HITCACHESIZE) {
					work->globalHitCache.hitCache[work->globalHitCache.hitCacheSize].pos.x = newHit.attribute("posX").as_double();
					work->globalHitCache.hitCache[work->globalHitCache.hitCacheSize].pos.y = newHit.attribute("posY").as_double();
					work->globalHitCache.hitCache[work->globalHitCache.hitCacheSize].pos.z = newHit.attribute("posZ").as_double();
					work->globalHitCache.hitCache[work->globalHitCache.hitCacheSize].type = newHit.attribute("type").as_int();
					work->globalHitCache.hitCacheSize++;
				}
			}

			work->globalHitCache.leakCacheSize = 0;
			xml_node leakCacheNode = globalNode.child("Leak_Cache");
			for (xml_node newLeak : leakCacheNode.children("Leak")) {
				if (work->globalHitCache.leakCacheSize < LEAKCACHESIZE) {
					work->globalHitCache.leakCache[work->globalHitCache.leakCacheSize].pos.x = newLeak.attribute("posX").as_double();
					work->globalHitCache.leakCache[work->globalHitCache.leakCacheSize].pos.y = newLeak.attribute("posY").as_double();
					work->globalHitCache.leakCache[work->globalHitCache.leakCacheSize].pos.z = newLeak.attribute("posZ").as_double();
					work->globalHitCache.leakCache[work->globalHitCache.leakCacheSize].dir.x = newLeak.attribute("dirX").as_double();
					work->globalHitCache.leakCache[work->globalHitCache.leakCacheSize].dir.y = newLeak.attribute("dirY").as_double();
					work->globalHitCache.leakCache[work->globalHitCache.leakCacheSize].dir.z = newLeak.attribute("dirZ").as_double();
					work->globalHitCache.leakCacheSize++;
				}
			}
		} //end global node

		xml_node facetResultsNode = newMoment.child("FacetResults");
		for (xml_node newFacetResult : facetResultsNode.children("Facet")) {
			int facetId = newFacetResult.attribute("id").as_int();
			Facet* f = GetFacet(facetId);
			xml_node facetHitNode = newFacetResult.child("Hits");
			FacetHitBuffer& facetHits = results.facetStates[facetId].momentResults[m].hits;
			if (facetHitNode) { //If there are hit results for the current moment	
				facetHits.nbMCHit = facetHitNode.attribute("nbHit").as_llong();
				if (facetHitNode.attribute("nbHitEquiv")) {
					facetHits.nbHitEquiv = facetHitNode.attribute("nbHitEquiv").as_double();
				}
				else {
					//Backward compatibility
					facetHits.nbHitEquiv = static_cast<double>(facetHits.nbMCHit);
				}
				facetHits.nbDesorbed = facetHitNode.attribute("nbDes").as_llong();
				if (facetHitNode.attribute("nbAbsEquiv")) {
					facetHits.nbAbsEquiv = facetHitNode.attribute("nbAbsEquiv").as_double();
				}
				else {
					//Backward compatibility
					facetHits.nbAbsEquiv = facetHitNode.attribute("nbAbs").as_double();
				}
				facetHits.sum_v_ort = facetHitNode.attribute("sum_v_ort").as_double();
				facetHits.sum_1_per_ort_velocity = facetHitNode.attribute("sum_1_per_v").as_double();
				if (facetHitNode.attribute("sum_v")) {
					facetHits.sum_1_per_velocity = facetHitNode.attribute("sum_v").as_double();
				}
				else {
					//Backward compatibility
					facetHits.sum_1_per_velocity = 4.0 * Sqr(facetHits.nbHitEquiv + static_cast<double>(facetHits.nbDesorbed)) / facetHits.sum_1_per_ort_velocity;
				}

				if (work->displayedMoment == m) { //For immediate display in facet hits list and facet counter
					f->facetHitCache = facetHits;
				}
			}
			else { //No hit information, so set to 0
				facetHits.nbMCHit =
					facetHits.nbDesorbed =
					0;
				facetHits.sum_v_ort =
					facetHits.nbHitEquiv =
					facetHits.sum_1_per_ort_velocity =
					facetHits.sum_1_per_velocity =
					facetHits.nbAbsEquiv =
					0.0;
			}

			//Profiles
			if (f->sh.isProfile) {
				xml_node profileNode = newFacetResult.child("Profile");
				//ProfileSlice *profilePtr = (ProfileSlice *)(buffer + f->sh.hitOffset + facetHitsSize + m * sizeof(ProfileSlice)*PROFILE_SIZE);
				std::vector<ProfileSlice>& profilePtr = results.facetStates[facetId].momentResults[m].profile;
				size_t id = 0;
				for (xml_node slice : profileNode.children("Slice")) {
					if (slice.attribute("countEquiv")) {
						profilePtr[id].countEquiv = slice.attribute("countEquiv").as_double();
					}
					else {
						//Old format before low-flux
						profilePtr[id].countEquiv = static_cast<double>(slice.attribute("count").as_llong());
					}
					profilePtr[id].sum_1_per_ort_velocity = slice.attribute("sum_1_per_v").as_double();
					profilePtr[id].sum_v_ort = slice.attribute("sum_v_ort").as_double();
					id++;
				}
			}

			//Textures
			int ix, iy;

			if (f->hasMesh) {
				xml_node textureNode = newFacetResult.child("Texture");
				size_t texWidth_file = textureNode.attribute("width").as_llong();
				size_t texHeight_file = textureNode.attribute("height").as_llong();

				/*if (textureNode.attribute("width").as_int() != f->wp.texWidth ||
					textureNode.attribute("height").as_int() != f->wp.texHeight) {
					std::stringstream msg;
					msg << "Texture size mismatch on facet " << facetId + 1 << ".\nExpected: " << f->wp.texWidth << "x" << f->wp.texHeight << "\n"
					<< "In file: " << textureNode.attribute("width").as_int() << "x" << textureNode.attribute("height").as_int();
					throw Error(msg.str().c_str());
					}*/ //We'll treat texture size mismatch, see below

				std::vector<TextureCell>& texture = results.facetStates[facetId].momentResults[m].texture;
				std::stringstream countText, sum1perText, sumvortText;
				if (textureNode.child("countEquiv")) {
					countText << textureNode.child_value("countEquiv");
				}
				else {
					countText << textureNode.child_value("count");
				}
				sum1perText << textureNode.child_value("sum_1_per_v");
				sumvortText << textureNode.child_value("sum_v_ort");

				for (iy = 0; iy < (Min(f->sh.texHeight, texHeight_file)); iy++) { //MIN: If stored texture is larger, don't read extra cells
					for (ix = 0; ix < (Min(f->sh.texWidth, texWidth_file)); ix++) { //MIN: If stored texture is larger, don't read extra cells
						countText >> texture[iy*f->sh.texWidth + ix].countEquiv;
						sum1perText >> texture[iy*f->sh.texWidth + ix].sum_1_per_ort_velocity;
						sumvortText >> texture[iy*f->sh.texWidth + ix].sum_v_ort_per_area;

					}
					for (int ie = 0; ie < texWidth_file - f->sh.texWidth; ie++) {//Executed if file texture is bigger than expected texture
						//Read extra cells from file without doing anything
						size_t dummy_ll;
						double dummy_d;
						countText >> dummy_ll;
						sum1perText >> dummy_d;
						sumvortText >> dummy_d;

					}
				}
				for (int ie = 0; ie < texHeight_file - f->sh.texHeight; ie++) {//Executed if file texture is bigger than expected texture
					//Read extra cells ffrom file without doing anything
					for (int iw = 0; iw < texWidth_file; iw++) {
						size_t dummy_ll;
						double dummy_d;
						countText >> dummy_ll;
						sum1perText >> dummy_d;
						sumvortText >> dummy_d;
					}

				}
			} //end texture

			if (f->sh.countDirection && f->dirCache) {
				xml_node dirNode = newFacetResult.child("Directions");
				if (dirNode.attribute("width").as_int() != f->sh.texWidth ||
					dirNode.attribute("height").as_int() != f->sh.texHeight) {
					std::stringstream msg;
					msg << "Direction texture size mismatch on facet " << facetId + 1 << ".\nExpected: " << f->sh.texWidth << "x" << f->sh.texHeight << "\n"
						<< "In file: " << dirNode.attribute("width").as_int() << "x" << dirNode.attribute("height").as_int();
					throw Error(msg.str().c_str());

				}
				std::vector<DirectionCell>& dirs = results.facetStates[facetId].momentResults[m].direction;

				std::stringstream dirText, dirCountText;
				dirText << dirNode.child_value("vel.vectors");
				dirCountText << dirNode.child_value("count");

				for (int iy = 0; iy < f->sh.texHeight; iy++) {
					for (int ix = 0; ix < f->sh.texWidth; ix++) {
						std::string component;
						std::getline(dirText, component, ',');
						dirs[iy*f->sh.texWidth + ix].dir.x = std::stod(component);
						std::getline(dirText, component, ',');
						dirs[iy*f->sh.texWidth + ix].dir.y = std::stod(component);
						dirText >> dirs[iy*f->sh.texWidth + ix].dir.z;
						dirCountText >> dirs[iy*f->sh.texWidth + ix].count;
					}
				}
			} //end directions
		} //end facetResult
		m++;
	} //end moment

	/*
	//Send angle maps //Commented out: CopyGeometryBuffer will send it after LoadXML_geom
	for (size_t i = 0; i < wp.nbFacet; i++) {
		Facet* f = facets[i];
		int profSize = (f->wp.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice)*(1 + (int)mApp->worker.moments.size())) : 0;
		size_t *angleMap = (size_t *)((BYTE *)gHits + f->wp.hitOffset + facetHitsSize
			+ profSize + (1 + (int)work->moments.size())*f->wp.texWidth*f->wp.texHeight * sizeof(TextureCell)
			+ (1 + (int)work->moments.size())*f->wp.texWidth*f->wp.texHeight * sizeof(DirectionCell));
		memcpy(angleMap, f->angleMapCache, f->wp.anglemapParams.phiWidth*(f->wp.anglemapParams.thetaLowerRes + f->wp.anglemapParams.thetaHigherRes) * sizeof(size_t));
	}
	*/

	xml_node minMaxNode = resultNode.child("TextureMinMax");
	/* //First write to worker->globalhitcache, then sync it to ghits(dphit) with SendToHitBuffer()
	gHits->texture_limits[0].min.all = minMaxNode.child("With_constant_flow").child("Pressure").attribute("min").as_double();
	gHits->texture_limits[0].max.all = minMaxNode.child("With_constant_flow").child("Pressure").attribute("max").as_double();
	gHits->texture_limits[1].min.all = minMaxNode.child("With_constant_flow").child("Density").attribute("min").as_double();
	gHits->texture_limits[1].max.all = minMaxNode.child("With_constant_flow").child("Density").attribute("max").as_double();
	gHits->texture_limits[2].min.all = minMaxNode.child("With_constant_flow").child("Imp.rate").attribute("min").as_double();
	gHits->texture_limits[2].max.all = minMaxNode.child("With_constant_flow").child("Imp.rate").attribute("max").as_double();
	gHits->texture_limits[0].min.moments_only = minMaxNode.child("Moments_only").child("Pressure").attribute("min").as_double();
	gHits->texture_limits[0].max.moments_only = minMaxNode.child("Moments_only").child("Pressure").attribute("max").as_double();
	gHits->texture_limits[1].min.moments_only = minMaxNode.child("Moments_only").child("Density").attribute("min").as_double();
	gHits->texture_limits[1].max.moments_only = minMaxNode.child("Moments_only").child("Density").attribute("max").as_double();
	gHits->texture_limits[2].min.moments_only = minMaxNode.child("Moments_only").child("Imp.rate").attribute("min").as_double();
	gHits->texture_limits[2].max.moments_only = minMaxNode.child("Moments_only").child("Imp.rate").attribute("max").as_double();
	*/

	work->globalHitCache.texture_limits[0].min.all = minMaxNode.child("With_constant_flow").child("Pressure").attribute("min").as_double();
	work->globalHitCache.texture_limits[0].max.all = minMaxNode.child("With_constant_flow").child("Pressure").attribute("max").as_double();
	work->globalHitCache.texture_limits[1].min.all = minMaxNode.child("With_constant_flow").child("Density").attribute("min").as_double();
	work->globalHitCache.texture_limits[1].max.all = minMaxNode.child("With_constant_flow").child("Density").attribute("max").as_double();
	work->globalHitCache.texture_limits[2].min.all = minMaxNode.child("With_constant_flow").child("Imp.rate").attribute("min").as_double();
	work->globalHitCache.texture_limits[2].max.all = minMaxNode.child("With_constant_flow").child("Imp.rate").attribute("max").as_double();
	work->globalHitCache.texture_limits[0].min.moments_only = minMaxNode.child("Moments_only").child("Pressure").attribute("min").as_double();
	work->globalHitCache.texture_limits[0].max.moments_only = minMaxNode.child("Moments_only").child("Pressure").attribute("max").as_double();
	work->globalHitCache.texture_limits[1].min.moments_only = minMaxNode.child("Moments_only").child("Density").attribute("min").as_double();
	work->globalHitCache.texture_limits[1].max.moments_only = minMaxNode.child("Moments_only").child("Density").attribute("max").as_double();
	work->globalHitCache.texture_limits[2].min.moments_only = minMaxNode.child("Moments_only").child("Imp.rate").attribute("min").as_double();
	work->globalHitCache.texture_limits[2].max.moments_only = minMaxNode.child("Moments_only").child("Imp.rate").attribute("max").as_double();

	ReleaseMutex(results.mutex);
	return true;
}