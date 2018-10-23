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

#include "Geometry_shared.h"
#include <cereal/archives/xml.hpp>

#define TEXTURE_MODE_PRESSURE 0
#define TEXTURE_MODE_IMPINGEMENT 1
#define TEXTURE_MODE_DENSITY 2



#define SYNVERSION 10

class Worker;

class MolflowGeometry: public Geometry {

public:

	// Constructor/Destructor
	MolflowGeometry();

	// Load
	void LoadGEO(FileReader *file, GLProgress *prg, int *version, Worker *worker);
	void LoadSYN(FileReader *file, GLProgress *prg, int *version, Worker *worker);
	bool LoadTexturesGEO(FileReader *file, GLProgress *prg, /*Dataport *dpHit*/ GlobalSimuState& results, int version);
	//void ImportDesorption_DES(FileReader *file); //Deprecated
	void ImportDesorption_SYN(FileReader *synFile, const size_t &source, const double &time,
		const size_t &mode, const double &eta0, const double &alpha, const double &cutoffdose,
		const std::vector<std::pair<double, double>> &convDistr,
		GLProgress *prg);
	void AnalyzeSYNfile(FileReader *f, GLProgress *progressDlg, size_t *nbNewFacet,
		size_t *nbTextured, size_t *nbDifferent, GLProgress *prg);

	// Insert
	void InsertSYN(FileReader *file, GLProgress *prg, bool newStr);

	// Save
	void SaveTXT(FileWriter *file, /*Dataport *dpHit*/ GlobalSimuState& results, bool saveSelected);
	void ExportTextures(FILE *file, int grouping, int mode, /*Dataport *dpHit*/ GlobalSimuState& results, bool saveSelected, size_t sMode);
	void ExportProfiles(FILE *file, int isTXT,/*Dataport *dpHit*/ GlobalSimuState& results, Worker *worker);
	void SaveGEO(FileWriter *file, GLProgress *prg, /*Dataport *dpHit*/ GlobalSimuState& results, Worker *worker,
		bool saveSelected, bool crashSave = false);
	
	void SaveXML_geometry(pugi::xml_node saveDoc, Worker *work, GLProgress *prg, bool saveSelected);
	bool SaveXML_simustate(pugi::xml_node saveDoc, Worker *work, GlobalSimuState& results, GLProgress *prg, bool saveSelected);
	void LoadXML_geom(pugi::xml_node loadXML, Worker *work, GLProgress *progressDlg);
	void InsertXML(pugi::xml_node loadXML, Worker *work, GLProgress *progressDlg, bool newStr);
	bool LoadXML_simustate(pugi::xml_node loadXML, /*Dataport *dpHit*/ GlobalSimuState& results, Worker *work, GLProgress *progressDlg);

	// Geometry
	void     BuildPipe(double L, double R, double s, int step);
	void     LoadProfileGEO(FileReader *file, /*Dataport *dpHit*/ GlobalSimuState& results, int version);

	// Memory usage (in bytes)
	size_t GetGeometrySize();
	size_t GetHitsSize(std::vector<double> *moments);

	// Raw data buffer (geometry)
	void CopyGeometryBuffer(BYTE *buffer,const OntheflySimulationParams& ontheflyParams);

	// AC matrix
	size_t GetMaxElemNumber();

	// Texture scaling
	//TEXTURE_SCALE_TYPE texture_limits[3];   // Min/max values for texture scaling: Pressure/Impingement rate/Density
	bool  texAutoScaleIncludeConstantFlow;  // Include constant flow when calculating autoscale values

	

#pragma region GeometryRender.cpp
	void BuildFacetTextures(GlobalSimuState& results,bool renderRegularTexture,bool renderDirectionTexture,size_t sMode);
	void BuildFacetDirectionTextures(GlobalSimuState& results);
#pragma endregion

	void SerializeForLoader(cereal::BinaryOutputArchive&);
	/*
	template <class Archive> void serialize(Archive & archive) {
		archive(
			CEREAL_NVP(sh),
			CEREAL_NVP(vertices3)
		);
		
		for (size_t i = 0; i < sh.nbFacet; i++) {
			archive(
				CEREAL_NVP(*(facets[i]))
			);
		}
	}
	*/

private:

	void InsertSYNGeom(FileReader *file, size_t strIdx = 0, bool newStruct = false);
	void SaveProfileGEO(FileWriter *file, /*Dataport *dpHit*/ GlobalSimuState& results, int super = -1, bool saveSelected = false, bool crashSave = false);

};

