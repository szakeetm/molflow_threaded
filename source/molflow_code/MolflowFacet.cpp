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
#include "Facet_shared.h"
#include "File.h"
#include "PugiXML/pugixml.hpp"
using namespace pugi;
#include "MolflowTypes.h"
#include <sstream>
#include <iomanip> //setprecision
#include "GLApp/GLToolkit.h"
#include "GLApp/MathTools.h" //IS_ZERO
#include "GLApp/GLMessageBox.h"
#include <cstring>
#include <math.h>
#include <cereal/types/vector.hpp>

// Colormap stuff, defined in GLGradient.cpp
extern std::vector<int> colorMap;


/**
* \brief Function for loading the geometry data of single facets from a GEO file
* \param file filename
* \param version version of the geometry description
* \param nbVertex number of facets contained in the geometry
*/
void Facet::LoadGEO(FileReader *file, int version, size_t nbVertex) {

	file->ReadKeyword("indices"); file->ReadKeyword(":");
	for (int i = 0; i < sh.nbIndex; i++) {
		indices[i] = file->ReadInt() - 1;
		if (indices[i] >= nbVertex)
			throw Error(file->MakeError("Facet index out of bounds"));
	}

	file->ReadKeyword("sticking"); file->ReadKeyword(":");
	sh.sticking = file->ReadDouble();
	file->ReadKeyword("opacity"); file->ReadKeyword(":");
	sh.opacity = file->ReadDouble();
	file->ReadKeyword("desorbType"); file->ReadKeyword(":");
	sh.desorbType = file->ReadInt();
	if (version >= 9) {
		file->ReadKeyword("desorbTypeN"); file->ReadKeyword(":");
		sh.desorbTypeN = file->ReadDouble();
	}
	else {
		ConvertOldDesorbType();
	}
	file->ReadKeyword("reflectType"); file->ReadKeyword(":");
	//Convert old model
	int oldReflType = file->ReadInt();
	if (oldReflType == REFLECTION_DIFFUSE) {
		sh.reflection.diffusePart = 1.0;
		sh.reflection.specularPart = 0.0;
	}
	else if (oldReflType == REFLECTION_SPECULAR) {
		sh.reflection.diffusePart = 0.0;
		sh.reflection.specularPart = 1.0;
	}
	else { //Uniform
		sh.reflection.diffusePart = 0.0;
		sh.reflection.specularPart = 0.0;
		sh.reflection.cosineExponent = 0.0; //Cos^0 = uniform
	}

	file->ReadKeyword("profileType"); file->ReadKeyword(":");
	sh.profileType = file->ReadInt();

	file->ReadKeyword("superDest"); file->ReadKeyword(":");
	sh.superDest = file->ReadInt();
	file->ReadKeyword("superIdx"); file->ReadKeyword(":");
	sh.superIdx = file->ReadInt();
	file->ReadKeyword("is2sided"); file->ReadKeyword(":");
	sh.is2sided = file->ReadInt();
	if (version < 8) {
		file->ReadKeyword("area"); file->ReadKeyword(":");
		sh.area = file->ReadDouble();
	}
	file->ReadKeyword("mesh"); file->ReadKeyword(":");
	hasMesh = file->ReadInt();
	if (version >= 7) {
		file->ReadKeyword("outgassing"); file->ReadKeyword(":");
		sh.outgassing = file->ReadDouble()*0.100; //mbar*l/s -> Pa*m3/s

	}
	file->ReadKeyword("texDimX"); file->ReadKeyword(":");
	sh.texWidthD = file->ReadDouble();

	file->ReadKeyword("texDimY"); file->ReadKeyword(":");
	sh.texHeightD = file->ReadDouble();

	file->ReadKeyword("countDes"); file->ReadKeyword(":");
	sh.countDes = file->ReadInt();
	file->ReadKeyword("countAbs"); file->ReadKeyword(":");
	sh.countAbs = file->ReadInt();

	file->ReadKeyword("countRefl"); file->ReadKeyword(":");
	sh.countRefl = file->ReadInt();

	file->ReadKeyword("countTrans"); file->ReadKeyword(":");
	sh.countTrans = file->ReadInt();

	file->ReadKeyword("acMode"); file->ReadKeyword(":");
	sh.countACD = file->ReadInt();
	file->ReadKeyword("nbAbs"); file->ReadKeyword(":");
	facetHitCache.nbAbsEquiv = file->ReadDouble();

	file->ReadKeyword("nbDes"); file->ReadKeyword(":");
	facetHitCache.nbDesorbed = file->ReadSizeT();

	file->ReadKeyword("nbHit"); file->ReadKeyword(":");

	facetHitCache.nbMCHit = file->ReadSizeT();
	facetHitCache.nbHitEquiv = static_cast<double>(facetHitCache.nbMCHit);
	if (version >= 2) {
		// Added in GEO version 2
		file->ReadKeyword("temperature"); file->ReadKeyword(":");
		sh.temperature = file->ReadDouble();
		file->ReadKeyword("countDirection"); file->ReadKeyword(":");
		sh.countDirection = file->ReadInt();

	}
	if (version >= 4) {
		// Added in GEO version 4
		file->ReadKeyword("textureVisible"); file->ReadKeyword(":");
		textureVisible = file->ReadInt();
		file->ReadKeyword("volumeVisible"); file->ReadKeyword(":");
		volumeVisible = file->ReadInt();
	}

	if (version >= 5) {
		// Added in GEO version 5
		file->ReadKeyword("teleportDest"); file->ReadKeyword(":");
		sh.teleportDest = file->ReadInt();
	}

	if (version >= 13) {
		// Added in GEO version 13
		file->ReadKeyword("accomodationFactor"); file->ReadKeyword(":");
		sh.accomodationFactor = file->ReadDouble();
	}

	UpdateFlags();

}

/**
* \brief Function for loading the geometry data of single facets from a XML file
* \param f xml node for the facet
* \param nbVertex number of facets contained in the geometry
* \param isMolflowFile if the file was generated by molflow (supports all values)
* \param ignoreSumMismatch if total dynamic outgasing can be different from sum of dynamic outgassing cells
* \param vertexOffset offset for the vertex id
*/
void Facet::LoadXML(xml_node f, size_t nbVertex, bool isMolflowFile, bool& ignoreSumMismatch, size_t vertexOffset) {
	int idx = 0;
	int facetId = f.attribute("id").as_int();
	for (xml_node indice : f.child("Indices").children("Indice")) {
		indices[idx] = indice.attribute("vertex").as_int() + vertexOffset;
		if (indices[idx] >= nbVertex) {
			char err[128];
			sprintf(err, "Facet %d refers to vertex %d which doesn't exist", facetId + 1, idx + 1);
			throw Error(err);
		}
		idx++;
	}
	sh.opacity = f.child("Opacity").attribute("constValue").as_double();
	sh.is2sided = f.child("Opacity").attribute("is2sided").as_int();
	sh.superIdx = f.child("Structure").attribute("inStructure").as_int();
	sh.superDest = f.child("Structure").attribute("linksTo").as_int();
	sh.teleportDest = f.child("Teleport").attribute("target").as_int();

	if (isMolflowFile) {
		sh.sticking = f.child("Sticking").attribute("constValue").as_double();
		sh.sticking_paramId = f.child("Sticking").attribute("parameterId").as_int();
		sh.opacity_paramId = f.child("Opacity").attribute("parameterId").as_int();
		sh.outgassing = f.child("Outgassing").attribute("constValue").as_double();
		sh.desorbType = f.child("Outgassing").attribute("desType").as_int();
		sh.desorbTypeN = f.child("Outgassing").attribute("desExponent").as_double();
		sh.outgassing_paramId = f.child("Outgassing").attribute("parameterId").as_int();
		hasOutgassingFile = f.child("Outgassing").attribute("hasOutgassingFile").as_bool();
		sh.useOutgassingFile = f.child("Outgassing").attribute("useOutgassingFile").as_bool();
		sh.temperature = f.child("Temperature").attribute("value").as_double();
		sh.accomodationFactor = f.child("Temperature").attribute("accFactor").as_double();
		xml_node reflNode = f.child("Reflection");
		if (reflNode.attribute("diffusePart") && reflNode.attribute("specularPart")) { //New format
			sh.reflection.diffusePart = reflNode.attribute("diffusePart").as_double();
			sh.reflection.specularPart = reflNode.attribute("specularPart").as_double();
			if (reflNode.attribute("cosineExponent")) {
				sh.reflection.cosineExponent = reflNode.attribute("cosineExponent").as_double();
			}
			else {
				sh.reflection.cosineExponent = 0.0; //uniform
			}
		}
		else { //old XML format: fully diffuse / specular / uniform reflections
			int oldReflType = reflNode.attribute("type").as_int();
			if (oldReflType == REFLECTION_DIFFUSE) {
				sh.reflection.diffusePart = 1.0;
				sh.reflection.specularPart = 0.0;
			}
			else if (oldReflType == REFLECTION_SPECULAR) {
				sh.reflection.diffusePart = 0.0;
				sh.reflection.specularPart = 1.0;
			}
			else { //Uniform
				sh.reflection.diffusePart = 0.0;
				sh.reflection.specularPart = 0.0;
				sh.reflection.cosineExponent = 0.0;
			}
		}
		
		if (reflNode.attribute("enableSojournTime")) {
			sh.enableSojournTime = reflNode.attribute("enableSojournTime").as_bool();
			if (!reflNode.attribute("sojournFreq")) {//Backward compatibility with ver. before 2.6.25
				sh.sojournFreq = 1.0 / reflNode.attribute("sojournTheta0").as_double();
				sh.sojournE = 8.31 * reflNode.attribute("sojournE").as_double();
			}
			else {
				sh.sojournFreq = reflNode.attribute("sojournFreq").as_double();
				sh.sojournE = reflNode.attribute("sojournE").as_double();
			}
		}
		else {
			//Already set to default when calling Molflow::LoadFile()
		}
		sh.isMoving = f.child("Motion").attribute("isMoving").as_bool();
		xml_node recNode = f.child("Recordings");
		sh.profileType = recNode.child("Profile").attribute("type").as_int();
		xml_node incidentAngleNode = recNode.child("IncidentAngleMap");
		if (incidentAngleNode) {
			sh.anglemapParams.record = recNode.child("IncidentAngleMap").attribute("record").as_bool();
			sh.anglemapParams.phiWidth = recNode.child("IncidentAngleMap").attribute("phiWidth").as_ullong();
			sh.anglemapParams.thetaLimit = recNode.child("IncidentAngleMap").attribute("thetaLimit").as_double();
			sh.anglemapParams.thetaLowerRes = recNode.child("IncidentAngleMap").attribute("thetaLowerRes").as_ullong();
			sh.anglemapParams.thetaHigherRes = recNode.child("IncidentAngleMap").attribute("thetaHigherRes").as_ullong();
		}
		xml_node texNode = recNode.child("Texture");
		hasMesh = texNode.attribute("hasMesh").as_bool();
		sh.texWidthD = texNode.attribute("texDimX").as_double();
		sh.texHeightD = texNode.attribute("texDimY").as_double();
		sh.countDes = texNode.attribute("countDes").as_bool() && hasMesh; //Sanitize input
		sh.countAbs = texNode.attribute("countAbs").as_bool() && hasMesh; //Sanitize input
		sh.countRefl = texNode.attribute("countRefl").as_bool() && hasMesh; //Sanitize input
		sh.countTrans = texNode.attribute("countTrans").as_bool() && hasMesh; //Sanitize input
		sh.countDirection = texNode.attribute("countDir").as_bool();
		sh.countACD = texNode.attribute("countAC").as_bool();

		xml_node outgNode = f.child("DynamicOutgassing");
		if ((hasOutgassingFile) && outgNode && outgNode.child("map")) {
			sh.outgassingMapWidth = outgNode.attribute("width").as_int();
			sh.outgassingMapHeight = outgNode.attribute("height").as_int();
			sh.outgassingFileRatio = outgNode.attribute("ratio").as_double();
			totalDose = outgNode.attribute("totalDose").as_double();
			sh.totalOutgassing = outgNode.attribute("totalOutgassing").as_double();
			totalFlux = outgNode.attribute("totalFlux").as_double();

			double sum = 0.0;

			std::stringstream outgText;
			outgText << outgNode.child_value("map");
			std::vector<double>(sh.outgassingMapWidth*sh.outgassingMapHeight).swap(outgassingMap);

			for (int iy = 0; iy < sh.outgassingMapHeight; iy++) {
				for (int ix = 0; ix < sh.outgassingMapWidth; ix++) {
					outgText >> outgassingMap[iy*sh.outgassingMapWidth + ix];
					sum += outgassingMap[iy*sh.outgassingMapWidth + ix];
				}
			}
			if (!ignoreSumMismatch && !IsEqual(sum, sh.totalOutgassing)) {
				std::stringstream msg; msg << std::setprecision(8);
				msg << "Facet " << facetId + 1 << ":\n";
				msg << "The total dynamic outgassing (" << 10.0 * sh.totalOutgassing << " mbar.l/s)\n";
				msg << "doesn't match the sum of the dynamic outgassing cells (" << 10.0 * sum << " mbar.l/s).";
				if (1 == GLMessageBox::Display(msg.str(), "Dynamic outgassing mismatch", { "OK","Ignore rest" },GLDLG_ICONINFO))
					ignoreSumMismatch = true;
			}
		}
		else hasOutgassingFile = sh.useOutgassingFile = 0; //if outgassing map was incorrect, don't use it

		xml_node angleMapNode = f.child("IncidentAngleMap");
		if (angleMapNode && angleMapNode.child("map") && angleMapNode.attribute("angleMapThetaLimit")) {

			sh.anglemapParams.phiWidth = angleMapNode.attribute("angleMapPhiWidth").as_ullong();
			sh.anglemapParams.thetaLimit = angleMapNode.attribute("angleMapThetaLimit").as_double();
			sh.anglemapParams.thetaLowerRes = angleMapNode.attribute("angleMapThetaLowerRes").as_ullong();
			sh.anglemapParams.thetaHigherRes = angleMapNode.attribute("angleMapThetaHigherRes").as_ullong();

			std::stringstream angleText;
			angleText << angleMapNode.child_value("map");
			std::vector<size_t>(sh.anglemapParams.GetMapSize()).swap(angleMapCache);

			for (int iy = 0; iy < (sh.anglemapParams.thetaLowerRes + sh.anglemapParams.thetaHigherRes); iy++) {
				for (int ix = 0; ix < sh.anglemapParams.phiWidth; ix++) {
					angleText >> angleMapCache[iy*sh.anglemapParams.phiWidth + ix];
				}
			}
		}
		else {
			//if angle map was incorrect, don't use it
			if (sh.desorbType == DES_ANGLEMAP) sh.desorbType = DES_NONE;
		}
	} //else use default values at Facet() constructor

	textureVisible = f.child("ViewSettings").attribute("textureVisible").as_bool();
	volumeVisible = f.child("ViewSettings").attribute("volumeVisible").as_bool();

	UpdateFlags();
}

/**
* \brief Function for loading the geometry data of single facets from a SYN file
* \param file filename of the SYN file
* \param version version of the syn description
* \param nbVertex number of facets contained in the geometry
*/
void Facet::LoadSYN(FileReader *file, int version, size_t nbVertex) {

	file->ReadKeyword("indices"); file->ReadKeyword(":");
	for (size_t i = 0; i < sh.nbIndex; i++) {
		indices[i] = file->ReadInt() - 1;
		if (indices[i] >= nbVertex) {
			throw Error(file->MakeError("Facet index out of bounds"));
		}
	}

	if (version >= 9) { //new reflection model
		file->ReadKeyword("reflectType"); file->ReadKeyword(":");
		sh.reflection.diffusePart = 1.0;
		sh.reflection.specularPart = 0.0;
		int reflType = file->ReadInt(); //Discard Synrad diffuse
		file->ReadKeyword("sticking"); file->ReadKeyword(":");
		sh.sticking = 0; file->ReadDouble(); //Discard Synrad sticking

		if (reflType >= 2) { //Material reflection: update index from the material's name
			file->ReadKeyword("materialName"); file->ReadKeyword(":"); file->ReadWord();
		}
		file->ReadKeyword("doScattering"); file->ReadKeyword(":");
		file->ReadInt();
		file->ReadKeyword("rmsRoughness"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("autoCorrLength"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("opacity"); file->ReadKeyword(":");
		sh.opacity = file->ReadDouble();
	}
	else { //legacy reflection model
		file->ReadKeyword("sticking"); file->ReadKeyword(":");
		sh.sticking = 0; file->ReadDouble(); //Discard Synrad sticking
		if (version >= 4) {
			file->ReadKeyword("roughness"); file->ReadKeyword(":");
			file->ReadDouble(); //roughness
		}
		file->ReadKeyword("opacity"); file->ReadKeyword(":");
		sh.opacity = file->ReadDouble();
		file->ReadKeyword("reflectType"); file->ReadKeyword(":");
		sh.reflection.diffusePart = 1.0;
		sh.reflection.specularPart = 0.0;
		file->ReadInt(); //Discard Synrad diffuse
	}

	file->ReadKeyword("profileType"); file->ReadKeyword(":");
	sh.profileType = 0; file->ReadInt(); //Discard Synrad profile
	file->ReadKeyword("hasSpectrum"); file->ReadKeyword(":");
	file->ReadInt();
	file->ReadKeyword("superDest"); file->ReadKeyword(":");
	sh.superDest = file->ReadInt();
	file->ReadKeyword("superIdx"); file->ReadKeyword(":");
	sh.superIdx = file->ReadInt();
	file->ReadKeyword("is2sided"); file->ReadKeyword(":");
	sh.is2sided = file->ReadInt();
	file->ReadKeyword("mesh"); file->ReadKeyword(":");
	hasMesh = false; file->ReadInt(); //Discard synrad texture
	file->ReadKeyword("texDimX"); file->ReadKeyword(":");
	sh.texWidthD = 0.0; file->ReadDouble();
	file->ReadKeyword("texDimY"); file->ReadKeyword(":");
	sh.texHeightD = 0.0; file->ReadDouble();
	if (version < 3) {
		file->ReadKeyword("countDes"); file->ReadKeyword(":");
		file->ReadInt();
	}
	file->ReadKeyword("countAbs"); file->ReadKeyword(":");
	sh.countAbs = false; file->ReadInt();
	file->ReadKeyword("countRefl"); file->ReadKeyword(":");
	sh.countRefl = false; file->ReadInt();
	file->ReadKeyword("countTrans"); file->ReadKeyword(":");
	sh.countTrans = false; file->ReadInt();
	if (version >= 10) file->ReadKeyword("nbAbsEquiv");
	else file->ReadKeyword("nbAbs"); file->ReadKeyword(":");
	facetHitCache.nbAbsEquiv = 0; file->ReadSizeT();
	if (version < 3) {
		file->ReadKeyword("nbDes"); file->ReadKeyword(":");
		facetHitCache.nbDesorbed = 0;
		file->ReadSizeT();
	}
	file->ReadKeyword("nbHit"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version >= 10) {
		file->ReadKeyword("nbHitEquiv"); file->ReadKeyword(":");file->ReadSizeT();
	}
	facetHitCache.nbMCHit = 0; facetHitCache.nbHitEquiv = 0.0; 
	if (version >= 3) {
		file->ReadKeyword("fluxAbs"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("powerAbs"); file->ReadKeyword(":");
		file->ReadDouble();
	}
	file->ReadKeyword("countDirection"); file->ReadKeyword(":");
	sh.countDirection = false; file->ReadInt();
	file->ReadKeyword("textureVisible"); file->ReadKeyword(":");
	textureVisible = file->ReadInt();
	file->ReadKeyword("volumeVisible"); file->ReadKeyword(":");
	volumeVisible = file->ReadInt();
	file->ReadKeyword("teleportDest"); file->ReadKeyword(":");
	sh.teleportDest = file->ReadInt();

	UpdateFlags();

}

/**
* \brief Function for loading the geometry data of single facets from a TXT file
* \param file filename of the TXT file
*/
void Facet::LoadTXT(FileReader *file) {

	// Opacity parameters descripton (TXT format)
	// -4    => Pressure profile (1 sided)
	// -3    => Desorption distribution
	// -2    => Angular profile
	// -1    => Pressure profile (2 sided)
	// [0,1] => Partial opacity (1 sided)
	// [1,2] => Partial opacity (2 sided)

	// Read facet parameters from TXT format
	sh.sticking = file->ReadDouble();
	double o = file->ReadDouble();
	/*wp.area =*/ file->ReadDouble();
	facetHitCache.nbDesorbed = (size_t)(file->ReadDouble() + 0.5);
	facetHitCache.nbMCHit = (size_t)(file->ReadDouble() + 0.5);
	facetHitCache.nbHitEquiv = static_cast<double>(facetHitCache.nbMCHit);
	facetHitCache.nbAbsEquiv = (double)(size_t)(file->ReadDouble() + 0.5);
	sh.desorbType = (int)(file->ReadDouble() + 0.5);

	// Convert opacity
	sh.profileType = PROFILE_NONE;
	if (o < 0.0) {

		sh.opacity = 0.0;
		if (IsZero(o + 1.0)) {
			sh.profileType = PROFILE_U;
			sh.is2sided = true;
		}
		if (IsZero(o + 2.0))
			sh.profileType = PROFILE_ANGULAR;
		if (IsZero(o + 4.0)) {
			sh.profileType = PROFILE_U;
			sh.is2sided = false;
		}

	}
	else {

		if (o >= 1.0000001) {
			sh.opacity = o - 1.0;
			sh.is2sided = true;
		}
		else

			sh.opacity = o;
	}

	// Convert desorbType
	switch (sh.desorbType) {
	case 0:
		sh.desorbType = DES_COSINE;
		break;
	case 1:
		sh.desorbType = DES_UNIFORM;
		break;
	case 2:
	case 3:
	case 4:
		sh.desorbType = sh.desorbType + 1; // cos^n
		break;
	}
	ConvertOldDesorbType();
	int reflectType = (int)(file->ReadDouble() + 0.5);

	// Convert reflectType
	switch (reflectType) {
	case REFLECTION_SPECULAR:
		sh.reflection.diffusePart = 0.0;
		sh.reflection.specularPart = 1.0;
		break;
	case REFLECTION_DIFFUSE:
		sh.reflection.diffusePart = 1.0;
		sh.reflection.specularPart = 0.0;
		break;
	default:
		sh.reflection.diffusePart = 0.0;
		sh.reflection.specularPart = 0.0;
		sh.reflection.cosineExponent = 0.0;
		break;
	}

	file->ReadDouble(); // Unused

	if (facetHitCache.nbDesorbed == 0)
		sh.desorbType = DES_NONE;

	if (IsTXTLinkFacet()) {
		sh.superDest = (int)(sh.sticking + 0.5);
		sh.sticking = 0;
	}

	UpdateFlags();

}

/**
* \brief Function for saving the geometry data of single facets into a TXT file
* \param file filename of the TXT file
*/
void Facet::SaveTXT(FileWriter *file) {

	if (!sh.superDest)
		file->Write(sh.sticking, "\n");
	else {
		file->Write((double)sh.superDest, "\n");
		sh.opacity = 0.0;
	}

	if (sh.is2sided)
		file->Write(sh.opacity + 1.0, "\n");
	else
		file->Write(sh.opacity, "\n");

	file->Write(sh.area, "\n");

	if (sh.desorbType != DES_NONE)
		file->Write(1.0, "\n");
	else
		file->Write(0.0, "\n");
	file->Write(0.0, "\n"); //nbMCHit
	file->Write(0.0, "\n"); //nbAbsEquiv

	file->Write(0.0, "\n"); //no desorption

	if (sh.reflection.diffusePart > 0.99) {
		file->Write((double)REFLECTION_DIFFUSE, "\n");
	}
	else if (sh.reflection.specularPart > 0.99) {
		file->Write((double)REFLECTION_SPECULAR, "\n");
	} 
	else
		file->Write((double)REFLECTION_UNIFORM, "\n");

	file->Write(0.0, "\n"); // Unused
}

/**
* \brief Function for saving the geometry data of single facets into a GEO file
* \param file filename of the GEO file
* \param idx index of the facet
*/
void Facet::SaveGEO(FileWriter *file, int idx) {

	char tmp[256];

	sprintf(tmp, "facet %d {\n", idx + 1);
	file->Write(tmp);
	file->Write("  nbIndex:"); file->Write(sh.nbIndex, "\n");
	file->Write("  indices:\n");
	for (int i = 0; i < sh.nbIndex; i++) {
		file->Write("    ");
		file->Write(indices[i] + 1, "\n");
	}
	//file->Write("\n");
	file->Write("  sticking:"); file->Write(sh.sticking, "\n");
	file->Write("  opacity:"); file->Write(sh.opacity, "\n");
	file->Write("  desorbType:"); file->Write(sh.desorbType, "\n");
	file->Write("  desorbTypeN:"); file->Write(sh.desorbTypeN, "\n");
	file->Write("  reflectType:"); 
	//Convert to old reflection type
	if (sh.reflection.diffusePart > 0.99) {
		file->Write(REFLECTION_DIFFUSE, "\n");
	}
	else if (sh.reflection.specularPart > 0.99) {
		file->Write(REFLECTION_SPECULAR, "\n");
	}
	else
		file->Write(REFLECTION_UNIFORM, "\n");
	
	file->Write("  profileType:"); file->Write(sh.profileType, "\n");

	file->Write("  superDest:"); file->Write(sh.superDest, "\n");
	file->Write("  superIdx:"); file->Write(sh.superIdx, "\n");
	file->Write("  is2sided:"); file->Write(sh.is2sided, "\n");
	file->Write("  mesh:"); file->Write((cellPropertiesIds != NULL), "\n");

	file->Write("  outgassing:"); file->Write(sh.outgassing*10.00, "\n"); //Pa*m3/s -> mbar*l/s for compatibility with old versions
	file->Write("  texDimX:"); file->Write(sh.texWidthD, "\n");
	file->Write("  texDimY:"); file->Write(sh.texHeightD, "\n");

	file->Write("  countDes:"); file->Write(sh.countDes, "\n");
	file->Write("  countAbs:"); file->Write(sh.countAbs, "\n");
	file->Write("  countRefl:"); file->Write(sh.countRefl, "\n");
	file->Write("  countTrans:"); file->Write(sh.countTrans, "\n");
	file->Write("  acMode:"); file->Write(sh.countACD, "\n");
	file->Write("  nbAbs:"); file->Write((size_t)facetHitCache.nbAbsEquiv, "\n");
	file->Write("  nbDes:"); file->Write(facetHitCache.nbDesorbed, "\n");
	file->Write("  nbHit:"); file->Write((size_t)facetHitCache.nbMCHit, "\n");

	// Version 2
	file->Write("  temperature:"); file->Write(sh.temperature, "\n");
	file->Write("  countDirection:"); file->Write(sh.countDirection, "\n");

	// Version 4
	file->Write("  textureVisible:"); file->Write(textureVisible, "\n");
	file->Write("  volumeVisible:"); file->Write(volumeVisible, "\n");

	// Version 5
	file->Write("  teleportDest:"); file->Write(sh.teleportDest, "\n");

	// Version 13
	file->Write("  accomodationFactor:"); file->Write(sh.accomodationFactor, "\n");

	file->Write("}\n");
}

/**
* \brief Calculates the geometry size for a single facet which is necessary for loader dataport
* \return calculated size of the facet geometry
*/
size_t Facet::GetGeometrySize()  { //for loader dataport

	size_t s = sizeof(FacetProperties)
		+ (sh.nbIndex * sizeof(size_t)) //indices
		+ (sh.nbIndex * sizeof(Vector2d));

	// Size of the 'element area' array passed to the geometry buffer
	if (sh.isTextured) s += sizeof(double)*sh.texWidth*sh.texHeight; //incbuff
	if (sh.useOutgassingFile ) s += sizeof(double)*sh.outgassingMapWidth*sh.outgassingMapHeight;
	s += sizeof(size_t)*angleMapCache.size();
	return s;

}

/**
* \brief Calculates the hits size for a single facet which is necessary for hits dataport
* \param nbMoments amount of moments
* \return calculated size of the facet hits
*/
size_t Facet::GetHitsSize(size_t nbMoments)  { //for hits dataport

	return   (1 + nbMoments)*(
		sizeof(FacetHitBuffer) +
		+(sh.texWidth*sh.texHeight * sizeof(TextureCell))
		+ (sh.isProfile ? (PROFILE_SIZE * sizeof(ProfileSlice)) : 0)
		+ (sh.countDirection ? (sh.texWidth*sh.texHeight * sizeof(DirectionCell)) : 0)
		+ sh.facetHistogramParams.GetDataSize()
		) + sizeof(size_t)*angleMapCache.size();

}

/**
* \brief Calculates the RAM size for the texture of a single facet
* \param nbMoments amount of moments
* \return calculated size of the texture RAM usage
*/
size_t Facet::GetTexRamSize(size_t nbMoments)  {
	//Values
	size_t sizePerCell = sizeof(TextureCell)*nbMoments; //TextureCell: long + 2*double
	if (sh.countDirection) sizePerCell += sizeof(DirectionCell)*nbMoments; //DirectionCell: Vector3d + long
	//Mesh
	sizePerCell += sizeof(int); //CellPropertiesIds
	size_t sizePerMeshElement = sizeof(CellProperties);
	sizePerMeshElement += 4 * sizeof(Vector2d); //Estimate: most mesh elements have 4 points
	return sh.texWidth*sh.texHeight*sizePerCell + meshvectorsize*sizePerMeshElement;
}

/**
* \brief Calculates the RAM size for the texture of a single facet by using a ratio for actual size conversation
* \param ratio ratio to calculate actual width/height of the facet
* \param useMesh unused paramter TODO: remove
* \param countDir unused paramter TODO: remove
* \param nbMoments amount of moments
* \return calculated size of the texture RAM usage
*/
size_t Facet::GetTexRamSizeForRatio(double ratio, bool useMesh, bool countDir, size_t nbMoments)  {
	double nU = sh.U.Norme();
	double nV = sh.V.Norme();
	double width = nU*ratio;
	double height = nV*ratio;

	bool dimOK = (width*height > 0.0000001);

	if (dimOK) {
		int iwidth = (int)ceil(width);
		int iheight = (int)ceil(height);

		//Values
		size_t sizePerCell = sizeof(TextureCell)*nbMoments; //TextureCell: long + 2*double
		if (sh.countDirection) sizePerCell += sizeof(DirectionCell)*nbMoments; //DirectionCell: Vector3d + long
		//Mesh
		sizePerCell += sizeof(int); //CellPropertiesIds
		size_t sizePerMeshElement = sizeof(CellProperties);
		sizePerMeshElement += 4 * sizeof(Vector2d); //Estimate: most mesh elements have 4 points
		return iwidth*iheight*(sizePerCell + sizePerMeshElement); //Conservative: assuming all cells are non-full
	}
	else {
		return 0;
	}
}

/**
* \brief Calculates a smoothing factor for a facet TODO: was used to perform edge smoothing, remove?
* \param i index of neighbor in u direction
* \param j index of neighbor in v direction
* \param texBuffer texture buffer
* \param textureMode which mode for the texture was used
* \param scaleF scaling factor
* \return smoothing factor
*/
double Facet::GetSmooth(int i, int j, TextureCell *texBuffer, int textureMode, double scaleF) {

	double W = 0.0;
	double sum = 0.0;
	int w = (int)sh.texWidth - 1;
	int h = (int)sh.texHeight - 1;

	//Diagonal neighbors, weight 1
	Sum_Neighbor(i - 1, j - 1, 1.0, texBuffer, textureMode, scaleF, &sum, &W);
	Sum_Neighbor(i - 1, j + 1, 1.0, texBuffer, textureMode, scaleF, &sum, &W);
	Sum_Neighbor(i + 1, j - 1, 1.0, texBuffer, textureMode, scaleF, &sum, &W);
	Sum_Neighbor(i + 1, j + 1, 1.0, texBuffer, textureMode, scaleF, &sum, &W);
	//Side neighbors, weight 2
	Sum_Neighbor(i, j - 1, 2.0, texBuffer, textureMode, scaleF, &sum, &W);
	Sum_Neighbor(i, j + 1, 2.0, texBuffer, textureMode, scaleF, &sum, &W);
	Sum_Neighbor(i - 1, j, 2.0, texBuffer, textureMode, scaleF, &sum, &W);
	Sum_Neighbor(i + 1, j, 2.0, texBuffer, textureMode, scaleF, &sum, &W);

	if (W == 0.0)
		return 0.0;
	else
		return sum / W;
}

/**
* \brief Adds texture value from a neighboring facet to a sum, used for texture smoothing
* \param i index of neighbor in u direction
* \param j index of neighbor in v direction
* \param weight weight factor for the memory requirement
* \param texBuffer texture buffer
* \param textureMode which mode for the texture was used
* \param scaleF scaling factor
* \param sum pointer to an existing sum counter
* \param totalWeight pointer to an existing counter for the total weight
*/
void Facet::Sum_Neighbor(const int& i, const int& j, const double& weight, TextureCell *texBuffer, const int& textureMode, const double& scaleF, double *sum, double *totalWeight) {
												
	if( i>=0 && i<sh.texWidth && j>=0 && j<sh.texHeight ) {								
		size_t add = (size_t)i+(size_t)j*sh.texWidth;												
		if( GetMeshArea(add)>0.0 ) {											
			if (textureMode==0)													
				*sum += weight*(texBuffer[add].countEquiv*scaleF);					
			else if (textureMode==1)											
				*sum += weight*(texBuffer[add].sum_1_per_ort_velocity*scaleF);   
			else if (textureMode==2)											
				*sum += weight*(texBuffer[add].sum_v_ort_per_area*scaleF);       
			*totalWeight+=weight;															
		}																		
	}
}

#define LOG10(x) log10f((float)x)


/**
* \brief Adds texture value from a neighboring facet to a sum, used for texture smoothing
* \param texBuffer vector of the texture buffer for each pixel
* \param textureMode which mode for the texture was used
* \param min min value for color scaling
* \param max max value for color scaling
* \param useColorMap if a 16bit high color map should be used (rainbow)
*/
void Facet::BuildTexture(std::vector<TextureCell>& texBuffer, int textureMode, double min, double max, bool useColorMap,
	double dCoeff1, double dCoeff2, double dCoeff3, bool doLog, size_t m) {
	size_t size = sh.texWidth*sh.texHeight;
	size_t tSize = texDimW*texDimH;
	if (size == 0 || tSize == 0) return;

	double scaleFactor = 1.0;
	int val;

	glBindTexture(GL_TEXTURE_2D, glTex);

		
		// 16 Bit rainbow colormap
		

		// Scale
		if (min < max) {
			if (doLog) {
				if (min < 1e-20) min = 1e-20;
				scaleFactor = (useColorMap ? 65534.0 : 255.0) / (log10(max) - log10(min)); // -1 for saturation color
			}
			else {

				scaleFactor = (useColorMap ? 65534.0 : 255.0) / (max - min); // -1 for saturation color
			}
		}
		else {
			doLog = false;
			min = 0;
		}

		unsigned char *buff8;
		int *buff32;

		if (useColorMap) {
			buff32 = (int *)calloc(tSize, sizeof(int)); //Color
			if (!buff32) throw Error("Cannot allocate memory for texture buffer");
		}
		else {
			buff8 = (unsigned char *)calloc(tSize, sizeof(unsigned char)); //Greyscale
			if (!buff8) throw Error("Cannot allocate memory for texture buffer");
		}

		for (size_t j = 0; j < sh.texHeight; j++) {
			for (size_t i = 0; i < sh.texWidth; i++) {
				size_t idx = i + j*sh.texWidth;
				double physicalValue;
				switch (textureMode) {
				case 0: //pressure
					physicalValue = texBuffer[idx].sum_v_ort_per_area*dCoeff1;
					break;
				case 1: //impingement rate
					physicalValue = texBuffer[idx].countEquiv / this->GetMeshArea(idx, true)*dCoeff2;
					break;
				case 2: //particle density
					physicalValue = DensityCorrection() * texBuffer[idx].sum_1_per_ort_velocity / this->GetMeshArea(idx, true)*dCoeff3;
					break;
				}
				if (doLog) {
					val = (int)((log10(physicalValue) - log10(min))*scaleFactor + 0.5);
				}
				else {

					val = (int)((physicalValue - min)*scaleFactor + 0.5);
				}
				Saturate(val, 0, useColorMap ? 65535 : 255);
				if (useColorMap) {
					buff32[(i + 1) + (j + 1)*texDimW] = colorMap[val];
					if (IsEqual(texBuffer[idx].countEquiv, 0.0)) buff32[(i + 1) + (j + 1)*texDimW] = (unsigned long)(65535 + 256 + 1); //show unset value as white
				}
				else {
					buff8[(i + 1) + (j + 1)*texDimW] = val;
				}
			}
		}

		/*
		// Perform edge smoothing (only with mesh)
		if( mesh ) {
		for(int j=-1;j<=wp.texHeight;j++) {
		for(int i=-1;i<=wp.texWidth;i++) {
		bool doSmooth = (i<0) || (i>=wp.texWidth) ||
		(j<0) || (j>=wp.texHeight) ||
		mesh[i+j*wp.texWidth].area==0.0f;
		if( doSmooth ) {
		if( doLog ) {
		val = (int)((log10(GetSmooth(i,j,texBuffer,dCoeff))-log10(min))*scaleFactor+0.5f);
		} else {
		val = (int)((GetSmooth(i,j,texBuffer,dCoeff)-min)*scaleFactor+0.5f);
		}
		Saturate(val,0,65535);
		buff32[(i+1) + (j+1)*texDimW] = colorMap[val];
		}
		}
		}
		}
		*/

		GLint width, height, format;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		if (format == (useColorMap ? GL_RGBA : GL_LUMINANCE) && width == texDimW && height == texDimH) {
			//Update texture
			glTexSubImage2D(
				GL_TEXTURE_2D,       // Type
				0,                   // No Mipmap
				0,					// X offset
				0,					// Y offset
				(int)texDimW,             // Width
				(int)texDimH,             // Height
				(useColorMap ? GL_RGBA : GL_LUMINANCE),             // Format RGBA
				GL_UNSIGNED_BYTE,    // 8 Bit/pixel
				(useColorMap ? (void*)buff32 : (void*)buff8)              // Data
			);
		}
		else {
			//Rebuild texture
			glTexImage2D(
				GL_TEXTURE_2D,       // Type
				0,                   // No Mipmap
				(useColorMap ? GL_RGBA : GL_LUMINANCE),             // Format RGBA or LUMINANCE
				(int)texDimW,             // Width
				(int)texDimH,             // Height
				0,                   // Border
				(useColorMap ? GL_RGBA : GL_LUMINANCE),             // Format RGBA or LUMINANCE
				GL_UNSIGNED_BYTE,    // 8 Bit/pixel
				(useColorMap ? (void*)buff32 : (void*)buff8)              // Data
			);
		}
		free(useColorMap ? (void*)buff32 : (void*)buff8);
		GLToolkit::CheckGLErrors("Facet::BuildTexture()");

}

/**
* \brief Converts the desorption type of a facet if it's from a particular type (TODO: check if this implies unneeded backwards compatibility)
*/
void Facet::ConvertOldDesorbType() {
	if (sh.desorbType >= 3 && sh.desorbType <= 5) {
		sh.desorbTypeN = (double)(sh.desorbType - 1);
		sh.desorbType = DES_COSINE_N;
	}
}

/**
* \brief To save facet data for the geometry in XML
* \param f XML node representing a facet
*/
void  Facet::SaveXML_geom(pugi::xml_node f) {
	xml_node e = f.append_child("Sticking");
	e.append_attribute("constValue") = sh.sticking;
	e.append_attribute("parameterId") = sh.sticking_paramId;

	e = f.append_child("Opacity");
	e.append_attribute("constValue") = sh.opacity;
	e.append_attribute("parameterId") = sh.opacity_paramId;
	e.append_attribute("is2sided") = (int)sh.is2sided; //backward compatibility: 0 or 1

	e = f.append_child("Outgassing");
	e.append_attribute("constValue") = sh.outgassing;
	e.append_attribute("parameterId") = sh.outgassing_paramId;
	e.append_attribute("desType") = sh.desorbType;
	e.append_attribute("desExponent") = sh.desorbTypeN;
	e.append_attribute("hasOutgassingFile") = (int)hasOutgassingFile; //backward compatibility: 0 or 1
	e.append_attribute("useOutgassingFile") = (int)sh.useOutgassingFile; //backward compatibility: 0 or 1

	e = f.append_child("Temperature");
	e.append_attribute("value") = sh.temperature;
	e.append_attribute("accFactor") = sh.accomodationFactor;

	e = f.append_child("Reflection");

	e.append_attribute("diffusePart") = sh.reflection.diffusePart;
	e.append_attribute("specularPart") = sh.reflection.specularPart;
	e.append_attribute("cosineExponent") = sh.reflection.cosineExponent;

	//For backward compatibility
	if (sh.reflection.diffusePart > 0.99) {
		e.append_attribute("type") = REFLECTION_DIFFUSE;
	}
	else if (sh.reflection.specularPart > 0.99) {
		e.append_attribute("type") = REFLECTION_SPECULAR;
	}
	else
		e.append_attribute("type") = REFLECTION_UNIFORM;

	e.append_attribute("enableSojournTime") = (int)sh.enableSojournTime; //backward compatibility: 0 or 1
	e.append_attribute("sojournFreq") = sh.sojournFreq;
	e.append_attribute("sojournE") = sh.sojournE;

	e = f.append_child("Structure");
	e.append_attribute("inStructure") = sh.superIdx;
	e.append_attribute("linksTo") = sh.superDest;

	e = f.append_child("Teleport");
	e.append_attribute("target") = sh.teleportDest;

	e = f.append_child("Motion");
	e.append_attribute("isMoving") = (int)sh.isMoving; //backward compatibility: 0 or 1

	e = f.append_child("Recordings");
	xml_node t = e.append_child("Profile");
	t.append_attribute("type") = sh.profileType;
	switch (sh.profileType) {
	case 0:
		t.append_attribute("name") = "none";
		break;
	case 1:
		t.append_attribute("name") = "pressure u";
		break;
	case 2:
		t.append_attribute("name") = "pressure v";
		break;
	case 3:
		t.append_attribute("name") = "angular";
		break;
	case 4:
		t.append_attribute("name") = "speed";
		break;
	case 5:
		t.append_attribute("name") = "ortho.v";
		break;
	case 6:
		t.append_attribute("name") = "tang.v.";
		break;
	}
	t = e.append_child("Texture");
	assert(!(cellPropertiesIds == NULL && (sh.countAbs || sh.countDes || sh.countRefl || sh.countTrans)); //Count texture on non-existent texture

	t.append_attribute("hasMesh") = cellPropertiesIds != NULL;
	t.append_attribute("texDimX") = sh.texWidthD;
	t.append_attribute("texDimY") = sh.texHeightD;
	t.append_attribute("countDes") = (int)sh.countDes; //backward compatibility: 0 or 1
	t.append_attribute("countAbs") = (int)sh.countAbs; //backward compatibility: 0 or 1
	t.append_attribute("countRefl") = (int)sh.countRefl; //backward compatibility: 0 or 1
	t.append_attribute("countTrans") = (int)sh.countTrans; //backward compatibility: 0 or 1
	t.append_attribute("countDir") = (int)sh.countDirection; //backward compatibility: 0 or 1
	t.append_attribute("countAC") = (int)sh.countACD; //backward compatibility: 0 or 1

	if (sh.anglemapParams.record) {
		t = e.append_child("IncidentAngleMap");
		t.append_attribute("record") = sh.anglemapParams.record;
		t.append_attribute("phiWidth") = sh.anglemapParams.phiWidth;
		t.append_attribute("thetaLimit") = sh.anglemapParams.thetaLimit;
		t.append_attribute("thetaLowerRes") = sh.anglemapParams.thetaLowerRes;
		t.append_attribute("thetaHigherRes") = sh.anglemapParams.thetaHigherRes;
	}

	e = f.append_child("ViewSettings");

	e.append_attribute("textureVisible") = (int)textureVisible; //backward compatibility: 0 or 1
	e.append_attribute("volumeVisible") = (int)volumeVisible; //backward compatibility: 0 or 1

	f.append_child("Indices").append_attribute("nb") = sh.nbIndex;
	for (size_t i = 0; i < sh.nbIndex; i++) {
		xml_node indice = f.child("Indices").append_child("Indice");
		indice.append_attribute("id") = i;
		indice.append_attribute("vertex") = indices[i];
	}

	if (hasOutgassingFile) {
		xml_node textureNode = f.append_child("DynamicOutgassing");
		textureNode.append_attribute("width") = sh.outgassingMapWidth;
		textureNode.append_attribute("height") = sh.outgassingMapHeight;
		textureNode.append_attribute("ratio") = sh.outgassingFileRatio;
		textureNode.append_attribute("totalDose") = totalDose;
		textureNode.append_attribute("totalOutgassing") = sh.totalOutgassing;
		textureNode.append_attribute("totalFlux") = totalFlux;

		std::stringstream outgText; outgText << std::setprecision(8);
		outgText << '\n'; //better readability in file
		for (int iy = 0; iy < sh.outgassingMapHeight; iy++) {
			for (int ix = 0; ix < sh.outgassingMapWidth; ix++) {
				outgText << outgassingMap[iy*sh.outgassingMapWidth + ix] << '\t';
			}
			outgText << '\n';
		}
		textureNode.append_child("map").append_child(node_cdata).set_value(outgText.str().c_str());

	} //end texture

	if (!angleMapCache.empty()) {
		xml_node textureNode = f.append_child("IncidentAngleMap");
		textureNode.append_attribute("angleMapPhiWidth") = sh.anglemapParams.phiWidth;
		textureNode.append_attribute("angleMapThetaLimit") = sh.anglemapParams.thetaLimit;
		textureNode.append_attribute("angleMapThetaLowerRes") = sh.anglemapParams.thetaLowerRes;
		textureNode.append_attribute("angleMapThetaHigherRes") = sh.anglemapParams.thetaHigherRes;

		std::stringstream angleText;
		angleText << '\n'; //better readability in file
		for (int iy = 0; iy < (sh.anglemapParams.thetaLowerRes + sh.anglemapParams.thetaHigherRes); iy++) {
			for (int ix = 0; ix < sh.anglemapParams.phiWidth; ix++) {
				angleText << angleMapCache[iy*sh.anglemapParams.phiWidth + ix] << '\t';
			}
			angleText << '\n';
		}
		textureNode.append_child("map").append_child(node_cdata).set_value(angleText.str().c_str());

	} //end angle map
}


/**
* \brief Function that retrieves the angle map as a String
* \param formatId ID that describes the seperator for the angle map string
* \return string describing the angle map
*/
std::string Facet::GetAngleMap(size_t formatId)
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
		result << -PI + (0.5 + (double)i) / ((double)sh.anglemapParams.phiWidth)*2.0*PI << separator;
	result << "\n";

	//Actual table
	for (size_t row = 0; row < (sh.anglemapParams.thetaLowerRes + sh.anglemapParams.thetaHigherRes); row++) {
		//First column: theta label
		if (row < sh.anglemapParams.thetaLowerRes)
			result << ((double)row + 0.5) / (double)sh.anglemapParams.thetaLowerRes*sh.anglemapParams.thetaLimit << separator;
		else
			result << sh.anglemapParams.thetaLimit + (0.5 + (double)(row-sh.anglemapParams.thetaLowerRes)) / (double)sh.anglemapParams.thetaHigherRes *(PI/2.0-sh.anglemapParams.thetaLimit) << separator;
		//Value
		for (size_t col = 0; col < sh.anglemapParams.phiWidth; col++) {
			result << angleMapCache[row * sh.anglemapParams.phiWidth + col] << separator;
		}
		result << "\n";
	}
	return result.str();
}

/**
* \brief Function that imports an angle map from a table
* \param table reference of a 2D vector structure of strings
*/
void Facet::ImportAngleMap(const std::vector<std::vector<std::string>>& table)
{
	size_t phiWidth, thetaLowerRes, thetaHigherRes;
	double thetaLimit;

	if (table[0][0] == "" || beginsWith(table[0][0], "Theta")) { //asume there is a header
		//looking at header values, try to determine theta resolution and limit
		phiWidth = table[0].size()-1; //row width minus first header column
		size_t spacingTypes = 1;
		double currentSpacing;
		double previousVal;
		for (size_t i = 1; i < table.size(); i++) { //skip first header row
			double val; size_t sz;
			try {
				val = std::stod(table[i][0], &sz); //convert to double
			}
			catch (...) {
					std::stringstream err;
					err << "Can't convert row " << i + 1 << " first cell to a double\nCell content: " << table[i][0];
					throw Error(err.str().c_str());
			}
			if (sz != table[i][0].size()) {
				std::stringstream err;
				err << "Can't convert row " << i+1 << " first cell to a double\nCell content: " << table[i][0];
				throw Error(err.str().c_str());
			}
			if (i == 1) currentSpacing = val*2.0;
			else if (!IsEqual(currentSpacing, val - previousVal, 1E-3)) {
				spacingTypes++;
				if (spacingTypes > 2) {
					std::stringstream err;
					err << "Row  " << i+1 << ": more than two types of theta difference\nCell content: " << table[i][0];
					throw Error(err.str().c_str());
				}
				//Just switched to next range
				thetaLowerRes = i-1; //subtract header
				thetaLimit = ((double)(i - 1)) * currentSpacing;
				currentSpacing = 2.0 * (val - thetaLimit);
			}
			previousVal = val;
		}
		if (spacingTypes == 1) {
			//might fill whole 0..PI/2 range, but maybe only lower or only upper res
			double lastTheta = currentSpacing * (table.size() - 1);
			if (IsEqual(lastTheta, PI / 2, 1E-3)) { //fills whole range
				thetaLimit = PI / 2;
				thetaLowerRes = table.size() - 1;
				thetaHigherRes = 0;
			}
			else if (lastTheta < (PI / 2)) { //fills from 0 to thetaLimit then no higher values
				thetaLimit = lastTheta;
				thetaLowerRes = table.size() - 1;
				thetaHigherRes = 0;
			}
			else { //no lower values then fills from thetaLimit to PI/2
				currentSpacing = 2.0*(PI / 2 - previousVal);
				thetaLimit = PI / 2 - (table.size() - 1)*currentSpacing;
				thetaLowerRes = 0;
				thetaHigherRes = table.size() - 1;
			}
		}
		else thetaHigherRes = table.size() - 1 - thetaLowerRes;

		//Fill table
		angleMapCache.resize(phiWidth * (thetaLowerRes + thetaHigherRes)); //Will be filled with values

		for (int iy = 0; iy < (thetaLowerRes + thetaHigherRes); iy++) {
			for (int ix = 0; ix < phiWidth; ix++) {
				size_t cellSize;
				try {
					angleMapCache[iy*phiWidth + ix] = std::stoi(table[iy+1][ix+1], &cellSize); //convert to double
				}
				catch (...) {
					std::stringstream err;
					err << "Can't convert cell row " << iy + 1 << " col " << ix + 1 << " to an integer\nCell content: " << table[iy+1][ix+1];
					angleMapCache.clear();
					throw Error(err.str().c_str());
				}
				if (cellSize != table[iy+1][ix+1].size()) {
					std::stringstream err;
					err << "Can't convert cell row " << iy + 1 << " col " << ix + 1 << " to an integer\nCell content: " << table[iy+1][ix+1];
					angleMapCache.clear();
					throw Error(err.str().c_str());
				}
			}
		}
	}
	else { //no header, equal spacing
		thetaLowerRes = table.size();
		thetaHigherRes = 0;
		phiWidth = table[0].size(); //row width
		thetaLimit = PI/2.0;

		//Fill table
		angleMapCache.resize(phiWidth * (thetaLowerRes + thetaHigherRes)); //Will be filled with values

		for (int iy = 0; iy < (thetaLowerRes + thetaHigherRes); iy++) {
			for (int ix = 0; ix < phiWidth; ix++) {
				size_t cellSize;
				try {
					angleMapCache[iy*phiWidth + ix] = std::stoi(table[iy][ix], &cellSize); //convert to double
				}
				catch (...) {
					std::stringstream err;
					err << "Can't convert cell row " << iy + 1 << " col " << ix + 1 << " to an integer\nCell content: " << table[iy][ix];
					angleMapCache.clear();
					throw Error(err.str().c_str());
				}
				if (cellSize != table[iy][ix].size()) {
					std::stringstream err;
					err << "Can't convert cell row " << iy+1 << " col " << ix+1 << " to an integer\nCell content: " << table[iy][ix];
					angleMapCache.clear();
					throw Error(err.str().c_str());
				}
			}
		}
	}

	//No errors, apply values
	sh.anglemapParams.phiWidth = phiWidth;
	sh.anglemapParams.record = false;
	sh.anglemapParams.thetaHigherRes = thetaHigherRes;
	sh.anglemapParams.thetaLimit = thetaLimit;
	sh.anglemapParams.thetaLowerRes = thetaLowerRes;
}

/**
* \brief Function that calculates a density correction factor [0..1] (with 1.0 = no correction)
* \return correction factor value [0..1]
*/
double Facet::DensityCorrection() {
	//Correction for double-density effect (measuring density on desorbing/absorbing facets):

	//Normally a facet only sees half of the particles (those moving towards it). So it multiplies the "seen" density by two.
	//However, in case of desorption or sticking, the real density is not twice the "seen" density, but a bit less, therefore this reduction factor
	//If only desorption, or only absorption, the correction factor is 0.5, if no des/abs, it's 1.0, and in between, see below

	if (facetHitCache.nbMCHit > 0 || facetHitCache.nbDesorbed > 0) {
		if (facetHitCache.nbAbsEquiv > 0.0 || facetHitCache.nbDesorbed > 0) {//otherwise save calculation time
			return 1.0 - (facetHitCache.nbAbsEquiv + (double)facetHitCache.nbDesorbed) / (facetHitCache.nbHitEquiv + (double)facetHitCache.nbDesorbed) / 2.0;
		}
		else return 1.0;
	}
	else return 1.0;
}

/**
* \brief Serializes data from facet into a cereal binary archive
* \param outputarchive reference to the binary archive
*/
void Facet::SerializeForLoader(cereal::BinaryOutputArchive& outputarchive) {

		//std::vector<double> outgMapVector(sh.useOutgassingFile ? sh.outgassingMapWidth*sh.outgassingMapHeight : 0);
		//memcpy(outgMapVector.data(), outgassingMap, sizeof(double)*(sh.useOutgassingFile ? sh.outgassingMapWidth*sh.outgassingMapHeight : 0));
		std::vector<double> textIncVector;

		// Add surface elements area (reciprocal)
		if (sh.isTextured) {
			textIncVector.resize(sh.texHeight*sh.texWidth);
			if (cellPropertiesIds) {
				size_t add = 0;
				for (size_t j = 0; j < sh.texHeight; j++) {
					for (size_t i = 0; i < sh.texWidth; i++) {
						double area = GetMeshArea(add, true);

						if (area > 0.0) {
							// Use the sign bit to store isFull flag
							textIncVector[add] = 1.0 / area;
						}
						else {
							textIncVector[add] = 0.0;
						}
						add++;
					}
				}
			}
			else {

				double rw = sh.U.Norme() / (double)(sh.texWidthD);
				double rh = sh.V.Norme() / (double)(sh.texHeightD);
				double area = rw * rh;
				size_t add = 0;
				for (int j = 0; j < sh.texHeight; j++) {
					for (int i = 0; i < sh.texWidth; i++) {
						if (area > 0.0) {
							textIncVector[add] = 1.0 / area;
						}
						else {
							textIncVector[add] = 0.0;
						}
						add++;
					}
				}
			}
		}

		outputarchive(
			CEREAL_NVP(sh), //Contains anglemapParams
			CEREAL_NVP(indices),
			CEREAL_NVP(vertices2)
#ifdef MOLFLOW
			, CEREAL_NVP(outgassingMap)
			, CEREAL_NVP(angleMapCache)
			, CEREAL_NVP(textIncVector)
#endif
		);
	
}