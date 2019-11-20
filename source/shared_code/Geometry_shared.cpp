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

//Common geometry handling/editing features, shared between Molflow and Synrad

#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "GLApp/MathTools.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLToolkit.h"
#include "SplitFacet.h"
#include "BuildIntersection.h"
#include "MirrorFacet.h"
#include "MirrorVertex.h"
#include "GLApp/GLList.h"

#include "Clipper/clipper.hpp"

#ifdef MOLFLOW
#include "MolFlow.h"
#include "MolflowTypes.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#include "ASELoader.h"
//#include <algorithm>
#include <list>

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

Geometry::Geometry() {
	facets = NULL;
	polyList = 0;
	selectList = 0;
	selectList2 = 0;
	selectList3 = 0;
	arrowList = 0;
	sphereList = 0;

	autoNorme = true;
	centerNorme = true;
	normeRatio = 1.0f;
	texAutoScale = true;
	texLogScale = false;
	texColormap = true;

	sh.nbSuper = 0;
#ifdef MOLFLOW
	textureMode = 0; //PRESSURE
#endif
#ifdef SYNRAD
	textureMode = 1; //FLUX
#endif
	viewStruct = -1;

	strcpy(strPath, "");
}

Geometry::~Geometry() {
	Clear();
}

void Geometry::CheckCollinear() {
	char tmp[256];
	// Check collinear polygon
	int nbCollinear = 0;
	for (int i = 0; i < GetNbFacet(); i++) {
		if (GetFacet(i)->collinear) nbCollinear++;
	}
	bool ok = false;
	if (nbCollinear) {
		sprintf(tmp, "%d null polygon(s) found !\nThese polygons have all vertices on a single line, thus they do nothing.\nDelete them?", nbCollinear);
		ok = GLMessageBox::Display(tmp, "Info", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK;
	}
	if (ok) {
		RemoveCollinear();
		mApp->UpdateModelParams();
	}
}

//Unused
void Geometry::CheckNonSimple() {
	char tmp[256];
	// Check non simple polygon
	int *nonSimpleList = (int *)malloc(GetNbFacet() * sizeof(int));
	int nbNonSimple = 0;
	for (int i = 0; i < GetNbFacet(); i++) {
		if (GetFacet(i)->nonSimple)
			nonSimpleList[nbNonSimple++] = i;
	}
	bool ok = false;
	if (nbNonSimple) {
		sprintf(tmp, "%d non simple (or null) polygon(s) found !\nSome tasks may not work properly\n"
			"Should I try to correct them (vertex shifting)?", nbNonSimple);
		ok = GLMessageBox::Display(tmp, "Warning", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING) == GLDLG_OK;
	}

	if (ok) CorrectNonSimple(nonSimpleList, nbNonSimple);
	SAFE_FREE(nonSimpleList);
}

void Geometry::CheckIsolatedVertex() {
	int nbI = HasIsolatedVertices();
	if (nbI) {
		char tmp[256];
		sprintf(tmp, "Remove %d isolated vertices ?", nbI);
		if (GLMessageBox::Display(tmp, "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK) {
			DeleteIsolatedVertices(false);
			mApp->UpdateModelParams();
		}
	}
}

void Geometry::InitializeGeometry(int facet_number) {

	RecalcBoundingBox(facet_number);

	// Choose an orthogonal (U,V) 2D basis for each facet. (This method can be 
	// improved. stub). The algorithm chooses the longest vedge for the U vector.
	// then it computes V (orthogonal to U and N). Afterwards, U and V are rescaled 
	// so each facet vertex are included in the rectangle defined by uU + vV (0<=u<=1 
	// and 0<=v<=1) of origin f->wp.O, U and V are always orthogonal and (U,V,N) 
	// form a 3D left handed orthogonal basis (not necessary orthonormal).
	// This coordinates system allows to prevent from possible "almost degenerated"
	// basis on fine geometry. It also greatly eases the facet/ray instersection routine 
	// and ref/abs/des hit recording and visualization. In order to ease some calculations, 
	// nU et nV (normalized U et V) are also stored in the Facet structure.
	// The local coordinates of facet vertex are stored in (U,V) coordinates (vertices2).

	
	size_t nbMoments = 1; //Constant flow
#ifdef MOLFLOW
	nbMoments += mApp->worker.moments.size();
#endif
	size_t fOffset = sizeof(GlobalHitBuffer)+nbMoments * mApp->worker.wp.globalHistogramParams.GetDataSize();

	for (int i = 0; i < sh.nbFacet; i++) {
		//initGeoPrg->SetProgress((double)i/(double)wp.nbFacet);
		if ((facet_number == -1) || (i == facet_number)) { //permits to initialize only one facet
														   // Main facet params
			// Current facet
			Facet *f = facets[i];
			CalculateFacetParams(f);

			// Detect non visible edge
			f->InitVisibleEdge();

			// Detect orientation
			//f->DetectOrientation();
			//f->sign = -1;

			if (facet_number == -1) {
				// Hit address
				f->sh.hitOffset = fOffset; //For hits data serialization, not used since 2.7 (multithreaded design)
#ifdef MOLFLOW
				fOffset += f->GetHitsSize(mApp->worker.moments.size());
#endif

#ifdef SYNRAD
				fOffset += f->GetHitsSize();
#endif
			}
		}
	}

	isLoaded = true;
	if (facet_number == -1) {
		BuildGLList();
		mApp->UpdateModelParams();
		mApp->UpdateFacetParams();
	}

	//initGeoPrg->SetVisible(false);
	//SAFE_DELETE(initGeoPrg);
	//_ASSERTE(_CrtCheckMemory());
}

void Geometry::RecalcBoundingBox(int facet_number) {

	if (facet_number == -1) { //bounding box for all vertices
		bb.min.x = 1e100;
		bb.min.y = 1e100;
		bb.min.z = 1e100;
		bb.max.x = -1e100;
		bb.max.y = -1e100;
		bb.max.z = -1e100;

		// Axis Aligned Bounding Box
		for (const Vector3d& p : vertices3) {
			bb.min.x = std::min(bb.min.x, p.x);
			bb.min.y = std::min(bb.min.y, p.y);
			bb.min.z = std::min(bb.min.z, p.z);
			bb.max.x = std::max(bb.max.x, p.x);
			bb.max.y = std::max(bb.max.y, p.y);
			bb.max.z = std::max(bb.max.z, p.z);
		}

#ifdef SYNRAD //Regions
		Worker *worker = &(mApp->worker);
		for (int i = 0; i < (int)worker->regions.size(); i++) {
			if (worker->regions[i].AABBmin.x < bb.min.x) bb.min.x = worker->regions[i].AABBmin.x;
			if (worker->regions[i].AABBmin.y < bb.min.y) bb.min.y = worker->regions[i].AABBmin.y;
			if (worker->regions[i].AABBmin.z < bb.min.z) bb.min.z = worker->regions[i].AABBmin.z;
			if (worker->regions[i].AABBmax.x > bb.max.x) bb.max.x = worker->regions[i].AABBmax.x;
			if (worker->regions[i].AABBmax.y > bb.max.y) bb.max.y = worker->regions[i].AABBmax.y;
			if (worker->regions[i].AABBmax.z > bb.max.z) bb.max.z = worker->regions[i].AABBmax.z;
		}
#endif
	}
	else { //bounding box only for the changed facet
		for (int i = 0; i < facets[facet_number]->sh.nbIndex; i++) {
			const Vector3d& p = vertices3[facets[facet_number]->indices[i]];
			bb.min.x = std::min(bb.min.x, p.x);
			bb.min.y = std::min(bb.min.y, p.y);
			bb.min.z = std::min(bb.min.z, p.z);
			bb.max.x = std::max(bb.max.x, p.x);
			bb.max.y = std::max(bb.max.y, p.y);
			bb.max.z = std::max(bb.max.z, p.z);
		}
	}

	center = 0.5 * (bb.min + bb.max);
}

//Unused
void Geometry::CorrectNonSimple(int *nonSimpleList, int nbNonSimple) {
	mApp->changedSinceSave = true;
	Facet *f;
	for (int i = 0; i < nbNonSimple; i++) {
		f = GetFacet(nonSimpleList[i]);
		if (f->nonSimple) {
			int j = 0;
			while ((j < f->sh.nbIndex) && (f->nonSimple)) {
				f->ShiftVertex();
				InitializeGeometry(nonSimpleList[i]);
				//f->DetectOrientation();
				j++;
			}
		}
	}
	//BuildGLList();
}

size_t Geometry::AnalyzeNeighbors(Worker *work, GLProgress *prg)
{
	size_t i = 0;
	work->abortRequested = false;
	for (i = 0; i < sh.nbFacet; i++) {
		facets[i]->neighbors.clear();
	}
	for (i = 0; !work->abortRequested && i < sh.nbFacet; i++) {
		mApp->DoEvents(); //Cathch possible cancel press
		Facet *f1 = facets[i];
		prg->SetProgress(double(i) / double(sh.nbFacet));
		for (size_t j = i + 1; j < sh.nbFacet; j++) {
			Facet *f2 = facets[j];
			size_t c1, c2, l;
			if (GetCommonEdges(facets[i], facets[j], &c1, &c2, &l)) {
				double dotProduct = Dot(f1->sh.N, f2->sh.N);
				Saturate(dotProduct, -1.0, 1.0); //Rounding errors...
				double angleDiff = fabs(acos(dotProduct));
				NeighborFacet n1, n2;
				n1.id = i;
				n2.id = j;
				n1.angleDiff = n2.angleDiff = angleDiff;
				f1->neighbors.push_back(n2);
				f2->neighbors.push_back(n1);
			}
		}
	}
	return i;
}

std::vector<size_t> Geometry::GetConnectedFacets(size_t sourceFacetId, double maxAngleDiff)
{
	std::vector<size_t> connectedFacets;
	std::vector<bool> alreadyConnected(sh.nbFacet, false);

	std::vector<size_t> toCheck;

	toCheck.push_back(sourceFacetId);
	do {
		//One iteration
		std::vector<size_t> toCheckNext;
		for (auto& facetId : toCheck) {
			for (auto& neighbor : facets[facetId]->neighbors) {
				if (neighbor.id < sh.nbFacet) { //Protect against changed geometry
					if (neighbor.angleDiff <= maxAngleDiff) {
						if (!alreadyConnected[neighbor.id]) toCheckNext.push_back(neighbor.id);
						alreadyConnected[neighbor.id] = true;
					}
				}
			}
		}
		toCheck = toCheckNext;
	} while (toCheck.size() != 0);
	for (size_t index = 0; index < sh.nbFacet; index++) {
		if (alreadyConnected[index] || index == sourceFacetId)
			connectedFacets.push_back(index);
	}
	return connectedFacets;
}
size_t Geometry::GetNbVertex() {
	return sh.nbVertex;
}

Vector3d Geometry::GetFacetCenter(int facet) {

	return facets[facet]->sh.center;

}

size_t Geometry::GetNbStructure() {
	return sh.nbSuper;
}

char *Geometry::GetStructureName(int idx) {
	return strName[idx];
}

void Geometry::AddFacet(const std::vector<size_t>& vertexIds) {
	//Creates a facet connecting vertexIds in order
	//Recalculates geometry after execution, so shouldn't be used repetitively

	//a new facet
	sh.nbFacet++;
	facets = (Facet **)realloc(facets, sh.nbFacet * sizeof(Facet *));
	facets[sh.nbFacet - 1] = new Facet(vertexIds.size());
	if (viewStruct != -1) facets[sh.nbFacet - 1]->sh.superIdx = viewStruct;
	UnselectAll();
	facets[sh.nbFacet - 1]->selected = true;
	for (size_t i = 0; i < vertexIds.size(); i++) {
		facets[sh.nbFacet - 1]->indices[i] = vertexIds[i];
	}

	mApp->changedSinceSave = true;
	InitializeGeometry(); //Need to recalc facet hit offsets
	UpdateSelection();
	mApp->facetList->SetSelectedRow((int)sh.nbFacet - 1);
	mApp->facetList->ScrollToVisible(sh.nbFacet - 1, 1, false);
}

void Geometry::CreatePolyFromVertices_Convex() {
	//creates facet from selected vertices

	auto selectedVertices = GetSelectedVertices();

	if (selectedVertices.size() < 3) {
		char errMsg[512];
		sprintf(errMsg, "Select at least 3 vertices.");
		throw Error(errMsg);
		return;
	}//at least three vertices

	Vector3d U, V, N;
	U = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[1]]).Normalized();

	int i2 = 2;
	do {
		V = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[i2]]).Normalized();
		i2++;
	} while (Dot(U, V) > 0.99 && i2 < selectedVertices.size()); //if U and V are almost the same, the projection would be inaccurate

	//Now we have the U,V plane, let's define it by computing the normal vector:
	N = CrossProduct(V, U).Normalized(); //We have a normal vector
	V = CrossProduct(N, U).Normalized(); //Make V perpendicular to U and N (and still in the U,V plane)

	std::vector<Vector2d> projected;

	//Get coordinates in the U,V system
	for (auto& sel : selectedVertices) {
		projected.push_back(ProjectVertex(vertices3[sel], U, V, vertices3[selectedVertices[0]]));
	}

	//Graham scan here on the projected[] array
	int *returnList = (int *)malloc(selectedVertices.size() * sizeof(int)); //need this intermediary structure for compatibility
	grahamMain(projected.data(), selectedVertices.size(), returnList);
	int ii, loopLength;
	for (ii = 0; ii < selectedVertices.size(); ii++) {
		if (returnList[ii] == returnList[0] && ii > 0) break;
	}
	loopLength = ii;
	//End graham scan

	std::vector<size_t> vertexIds(selectedVertices.size());
	for (size_t i = 0; i < selectedVertices.size(); i++) {
		vertexIds[i] = selectedVertices[returnList[i]];
	}

	SAFE_FREE(returnList);

	AddFacet(vertexIds);
}

void Geometry::CreatePolyFromVertices_Order() {
	//creates facet from selected vertices

	AddFacet(selectedVertexList_ordered);
}

void Geometry::CreateDifference() {
	//creates facet from selected vertices

	mApp->changedSinceSave = true;
	size_t nbSelectedVertex = 0;

	auto selectedFacets = GetSelectedFacets();
	if (selectedFacets.size() != 2) {
		char errMsg[512];
		sprintf(errMsg, "Select exactly 2 facets.");
		throw Error(errMsg);
		return;
	}//at least three vertices

	size_t firstFacet = selectedFacets[0];
	size_t secondFacet = selectedFacets[1];

	//TO DO:
	//swap if normals not collinear
	//shift vertex to make nice cut

	//a new facet
	sh.nbFacet++;
	facets = (Facet **)realloc(facets, sh.nbFacet * sizeof(Facet *));
	facets[sh.nbFacet - 1] = new Facet(facets[firstFacet]->sh.nbIndex + facets[secondFacet]->sh.nbIndex + 2);
	facets[sh.nbFacet - 1]->sh.sticking = 0.0;
	//set selection
	UnselectAll();
	facets[sh.nbFacet - 1]->selected = true;
	selectedFacets = { sh.nbFacet - 1 };
	if (viewStruct != -1) facets[sh.nbFacet - 1]->sh.superIdx = viewStruct;
	//one circle on first facet
	size_t counter = 0;
	for (size_t i = 0; i < facets[firstFacet]->sh.nbIndex; i++)
		facets[sh.nbFacet - 1]->indices[counter++] = facets[firstFacet]->indices[i];
	//close circle by adding the first vertex again
	facets[sh.nbFacet - 1]->indices[counter++] = facets[firstFacet]->indices[0];
	//reverse circle on second facet
	for (int i = (int)facets[secondFacet]->sh.nbIndex - 1; i >= 0; i--)
		facets[sh.nbFacet - 1]->indices[counter++] = facets[secondFacet]->GetIndex(i + 1);
	//close circle by adding the first vertex again
	facets[sh.nbFacet - 1]->indices[counter++] = facets[secondFacet]->indices[0];

	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
	mApp->facetList->SetSelectedRow((int)sh.nbFacet - 1);
	mApp->facetList->ScrollToVisible(sh.nbFacet - 1, 1, false);
}

void Geometry::ClipSelectedPolygons(ClipperLib::ClipType type, int reverseOrder) {
	auto selFacetIds = GetSelectedFacets();
	if (selFacetIds.size() != 2) {
		char errMsg[512];
		sprintf(errMsg, "Select exactly 2 facets.");
		throw Error(errMsg);
		return;
	}//at least three vertices

	size_t firstFacet = selFacetIds[0];
	size_t secondFacet = selFacetIds[1];

	if (reverseOrder == 0) ClipPolygon(firstFacet, secondFacet, type);
	else if (reverseOrder == 1) ClipPolygon(secondFacet, firstFacet, type);
	else {
		//Auto
		ClipperLib::PolyTree solution;
		std::vector<ProjectedPoint> projectedPoints;
		std::vector<std::vector<size_t>> clippingPaths;

		clippingPaths = { facets[secondFacet]->indices };
		if (ExecuteClip(firstFacet, clippingPaths, projectedPoints, solution, type) > 0) {
			ClipPolygon(firstFacet, secondFacet, type);
		}
		else {
			clippingPaths = { facets[firstFacet]->indices };
			if (ExecuteClip(secondFacet, clippingPaths, projectedPoints, solution, type) > 0) {
				ClipPolygon(secondFacet, firstFacet, type);
			}
		}
	}
}

void Geometry::ClipPolygon(size_t id1, std::vector<std::vector<size_t>> clippingPaths, ClipperLib::ClipType type) {
	mApp->changedSinceSave = true;
	ClipperLib::PolyTree solution;
	std::vector<ProjectedPoint> projectedPoints;
	ExecuteClip(id1, clippingPaths, projectedPoints, solution, type); //Returns solution in a polygon/hole list, we have to convert it to a continous outline

	//a new facet
	size_t nbNewFacets = solution.ChildCount(); //Might be more than one if clipping facet splits subject to pieces
	facets = (Facet **)realloc(facets, (sh.nbFacet + nbNewFacets) * sizeof(Facet *));
	//set selection
	UnselectAll();
	std::vector<InterfaceVertex> newVertices;
	for (size_t i = 0; i < nbNewFacets; i++) {
		size_t nbHoles = solution.Childs[i]->ChildCount();
		std::vector<size_t> closestIndexToChild(nbHoles), closestIndexToParent(nbHoles);
		for (size_t holeIndex = 0; holeIndex < nbHoles; holeIndex++) {
			double minDist = 9E99;
			
				for (size_t j = 0; j < solution.Childs[i]->Contour.size(); j++) { //Find closest parent point
					Vector2d vert;
					vert.u = 1E-6*(double)solution.Childs[i]->Contour[j].X;
					vert.v = 1E-6*(double)solution.Childs[i]->Contour[j].Y;
					for (size_t k = 0; k < solution.Childs[i]->Childs[holeIndex]->Contour.size(); k++) {//Find closest child point
						Vector2d childVert;
						childVert.u = 1E-6*(double)solution.Childs[i]->Childs[holeIndex]->Contour[k].X;
						childVert.v = 1E-6*(double)solution.Childs[i]->Childs[holeIndex]->Contour[k].Y;
						double dist = Sqr(facets[id1]->sh.U.Norme() * (vert.u - childVert.u)) + Sqr(facets[id1]->sh.V.Norme() * (vert.v - childVert.v));
						if (dist < minDist) {
							minDist = dist;
							closestIndexToChild[holeIndex] = j;
							closestIndexToParent[holeIndex] = k;
						}
					}
				}
			
		}
		size_t nbRegistered = 0;
		size_t nbVertex = solution.Childs[i]->Contour.size();
		for (size_t holeIndex = 0; holeIndex < nbHoles; holeIndex++) {
			nbVertex += 2 + solution.Childs[i]->Childs[holeIndex]->Contour.size();
		}

		Facet *f = new Facet(nbVertex);
		for (size_t j = 0; j < solution.Childs[i]->Contour.size(); j++) {
			Vector2d vert;
			for (size_t holeIndex = 0; holeIndex < nbHoles; holeIndex++) {
				if (j == closestIndexToChild[holeIndex]) { //Create hole
					vert.u = 1E-6*(double)solution.Childs[i]->Contour[j].X;
					vert.v = 1E-6*(double)solution.Childs[i]->Contour[j].Y;
					RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++);//Register entry from parent
					for (size_t k = 0; k < solution.Childs[i]->Childs[0]->Contour.size(); k++) { //Register hole
						vert.u = 1E-6*(double)solution.Childs[i]->Childs[holeIndex]->Contour[(k + closestIndexToParent[holeIndex]) % solution.Childs[i]->Childs[holeIndex]->Contour.size()].X;
						vert.v = 1E-6*(double)solution.Childs[i]->Childs[holeIndex]->Contour[(k + closestIndexToParent[holeIndex]) % solution.Childs[i]->Childs[holeIndex]->Contour.size()].Y;
						RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++);
					}
					vert.u = 1E-6*(double)solution.Childs[i]->Childs[holeIndex]->Contour[closestIndexToParent[holeIndex]].X;
					vert.v = 1E-6*(double)solution.Childs[i]->Childs[holeIndex]->Contour[closestIndexToParent[holeIndex]].Y;
					RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++); //Re-register hole entry point before exit
				}
			}
			vert.u = 1E-6*(double)solution.Childs[i]->Contour[j].X;
			vert.v = 1E-6*(double)solution.Childs[i]->Contour[j].Y;
			RegisterVertex(f, vert, id1, projectedPoints, newVertices, nbRegistered++);
		}
		f->selected = true;
		if (viewStruct != -1) f->sh.superIdx = viewStruct;
		facets[sh.nbFacet + i] = f;
	}
	sh.nbFacet += nbNewFacets;
	//vertices3 = (InterfaceVertex*)realloc(vertices3, sizeof(InterfaceVertex)*(wp.nbVertex + newVertices.size()));
	vertices3.resize(sh.nbVertex + newVertices.size());
	for (InterfaceVertex newVert : newVertices)
		vertices3[sh.nbVertex++] = newVert;

	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
}

size_t Geometry::ExecuteClip(size_t& id1, std::vector<std::vector<size_t>>& clippingPaths, std::vector<ProjectedPoint>& projectedPoints, ClipperLib::PolyTree& solution, ClipperLib::ClipType& type) {
	
	ClipperLib::Paths subj(1), clip(clippingPaths.size());

	for (size_t i1 = 0; i1 < facets[id1]->sh.nbIndex; i1++) {
		subj[0] << ClipperLib::IntPoint((int)(facets[id1]->vertices2[i1].u*1E6), (int)(facets[id1]->vertices2[i1].v*1E6));
	}

	for (size_t i3 = 0; i3 < clippingPaths.size(); i3++) {
		for (size_t i2 = 0; i2 < clippingPaths[i3].size(); i2++) {
			ProjectedPoint proj;
			proj.globalId = clippingPaths[i3][i2];
			proj.vertex2d = ProjectVertex(vertices3[clippingPaths[i3][i2]], facets[id1]->sh.U, facets[id1]->sh.V, facets[id1]->sh.O);
			clip[0] << ClipperLib::IntPoint((int)(proj.vertex2d.u*1E6), (int)(proj.vertex2d.v*1E6));
			projectedPoints.push_back(proj);
		}
	}
	ClipperLib::Clipper c;
	c.AddPaths(subj, ClipperLib::ptSubject, true);
	c.AddPaths(clip, ClipperLib::ptClip, true);
	c.Execute(type, solution, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
	return solution.ChildCount();

	//The code below could identify holes in Molflow logic and convert them to Polygon/hole-subpolygon format.
	//Turns out it wasn't necessary for the Clipper library to recognize holes, so we return to the original code above for simplicty
	/*
	//Identify holes in source facet
	std::vector<size_t> existingVertices;
	std::vector < std::vector<size_t> > holePaths; // list of list of vertices
	bool skipNext = false;
	for (size_t i1 = 0; i1 < facets[id1]->wp.nbIndex; i1++) {
		if (skipNext) {
			skipNext = false;
			continue;
		}
		if (Contains(existingVertices, facets[id1]->indices[i1])) {
			//Identify last occurrence of the same index (beginning of hole)
			size_t holeStartIndex = Previous(i1, facets[id1]->wp.nbIndex);
			while (facets[id1]->indices[holeStartIndex] != facets[id1]->indices[i1]) {
				holeStartIndex = Previous(holeStartIndex, facets[id1]->wp.nbIndex);
			}
			std::vector<size_t> newHolePath;
			for (int i2 = holeStartIndex; i2 != i1; i2 = Next(i2, facets[id1]->wp.nbIndex)) {
				newHolePath.push_back(i2);
			}
			holePaths.push_back(newHolePath);
			skipNext = true;
		}
		existingVertices.push_back(facets[id1]->indices[i1]);
	}


	
	ClipperLib::Paths subj(1+ holePaths.size()), clip(clippingPaths.size());

	size_t lastAdded = -1;
	for (size_t i1 = 0; i1 < facets[id1]->wp.nbIndex; i1++) {
		bool notPartOfHole = true;
		for (auto& path : holePaths) {
			for (auto& v : path) {
				if (facets[id1]->indices[v] == facets[id1]->indices[i1]) {
					notPartOfHole = false;
					break;
				}
			}
		}
		if (notPartOfHole && facets[id1]->indices[i1]!=lastAdded) {
			subj[0] << ClipperLib::IntPoint((int)(facets[id1]->vertices2[i1].u*1E6), (int)(facets[id1]->vertices2[i1].v*1E6));
			lastAdded = facets[id1]->indices[i1];
		}
	}

	for (size_t i1 = 0; i1 < holePaths.size(); i1++) {
		for (size_t i2 : holePaths[i1]) {
			subj[i1+1] << ClipperLib::IntPoint((int)(facets[id1]->vertices2[i2].u*1E6), (int)(facets[id1]->vertices2[i2].v*1E6));
		}
	}

	for (size_t i3 = 0; i3 < clippingPaths.size(); i3++) {
		for (size_t i2 = 0; i2 < clippingPaths[i3].size(); i2++) {
			ProjectedPoint proj;
			proj.globalId = clippingPaths[i3][i2];
			proj.vertex2d = ProjectVertex(vertices3[clippingPaths[i3][i2]], facets[id1]->wp.U, facets[id1]->wp.V, facets[id1]->wp.O);
			clip[0] << ClipperLib::IntPoint((int)(proj.vertex2d.u*1E6), (int)(proj.vertex2d.v*1E6));
			projectedPoints.push_back(proj);
		}
	}
	ClipperLib::Clipper c;
	c.AddPaths(subj, ClipperLib::ptSubject, true);
	c.AddPaths(clip, ClipperLib::ptClip, true);
	c.Execute(type, solution, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
	return solution.ChildCount();
	*/
}

void Geometry::ClipPolygon(size_t id1, size_t id2, ClipperLib::ClipType type) {
	ClipPolygon(id1, { facets[id2]->indices }, type);
}

void Geometry::RegisterVertex(Facet *f, const Vector2d &vert, size_t id1, const std::vector<ProjectedPoint> &projectedPoints, std::vector<InterfaceVertex> &newVertices, size_t registerLocation) {
	int foundId = -1;
	for (size_t k = 0; foundId == -1 && k < facets[id1]->sh.nbIndex; k++) { //Check if part of facet 1
		double dist = (vert - facets[id1]->vertices2[k]).Norme();
		foundId = (dist < 1E-5) ? (int)facets[id1]->indices[k] : -1;
	}
	for (size_t k = 0; foundId == -1 && k < projectedPoints.size(); k++) { //Check if part of facet 2
		double dist = (vert - projectedPoints[k].vertex2d).Norme();
		foundId = (dist < 1E-5) ? (int)projectedPoints[k].globalId : -1;
	}
	if (foundId == -1) { //Create new vertex
		InterfaceVertex newVertex;
		newVertex.selected = true;
		newVertex.SetLocation(facets[id1]->sh.O + vert.u*facets[id1]->sh.U + vert.v*facets[id1]->sh.V);
		f->indices[registerLocation] = sh.nbVertex + newVertices.size();
		newVertices.push_back(newVertex);
	}
	else { //Vertex already exists
		f->indices[registerLocation] = foundId;
	}
}

void Geometry::SelectCoplanar(int width, int height, double tolerance) {

	auto selectedVertices = GetSelectedVertices();
	if (selectedVertices.size() < 3) {
		GLMessageBox::Display("Select at least 3 vertices", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}

	Vector3d U = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[1]]).Normalized();
	Vector3d V = (vertices3[selectedVertices[0]] - vertices3[selectedVertices[2]]).Normalized();
	Vector3d N = CrossProduct(V, U);
	double nN = N.Norme();
	if (nN < 1e-8) {
		GLMessageBox::Display("Sorry, the 3 selected vertices are on a line.", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	N = 1.0 / nN * N; // Normalize N

	// Plane equation
	double A = N.x;
	double B = N.y;
	double C = N.z;
	Vector3d& p0 = vertices3[selectedVertices[0]];
	double D = -Dot(N, p0);

	//double denominator=sqrt(pow(A,2)+pow(B,2)+pow(C,2));
	double distance;

	for (int i = 0; i < sh.nbVertex; i++) {
		Vector3d *v = GetVertex(i);
		if (auto screenCoords = GLToolkit::Get2DScreenCoord(*v)) { //To improve
			auto[outX, outY] = *screenCoords;
			if (outX >= 0 && outY >= 0 && outX <= width && outY <= height) {
				distance = std::abs(A*v->x + B * v->y + C * v->z + D);
				if (distance < tolerance) { //vertex is on the plane
					vertices3[i].selected = true;
				}
				else {
					vertices3[i].selected = false;
				}
			}
		}
		else {
			vertices3[i].selected = false;
		}
	}
}

InterfaceVertex* Geometry::GetVertex(size_t idx) {
	return &(vertices3[idx]);
}

Facet *Geometry::GetFacet(size_t facet) {
	if (facet >= sh.nbFacet || facet < 0) {
		char errMsg[512];
		sprintf(errMsg, "Geometry::GetFacet()\nA process tried to access facet #%zd that doesn't exist.\nAutoSaving and probably crashing...", facet + 1);
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
		mApp->AutoSave(true);
		throw Error(errMsg);
	}
	return facets[facet];
}

size_t Geometry::GetNbFacet() {
	return sh.nbFacet;
}

AxisAlignedBoundingBox Geometry::GetBB() {

	/*if (viewStruct < 0) {

		return bb;

	}
	else {*/
		// BB of selected struture //replaced with all vertices
		AxisAlignedBoundingBox sbb;

		sbb.min.x = 1e100;
		sbb.min.y = 1e100;
		sbb.min.z = 1e100;
		sbb.max.x = -1e100;
		sbb.max.y = -1e100;
		sbb.max.z = -1e100;

		// Axis Aligned Bounding Box
		/*
		for (int i = 0; i < wp.nbFacet; i++) {
			Facet *f = facets[i];
			if (f->wp.superIdx == viewStruct) {
				for (int j = 0; j < f->wp.nbIndex; j++) {
					Vector3d p = vertices3[f->indices[j]];
					if (p.x < sbb.min.x) sbb.min.x = p.x;
					if (p.y < sbb.min.y) sbb.min.y = p.y;
					if (p.z < sbb.min.z) sbb.min.z = p.z;
					if (p.x > sbb.max.x) sbb.max.x = p.x;
					if (p.y > sbb.max.y) sbb.max.y = p.y;
					if (p.z > sbb.max.z) sbb.max.z = p.z;
				}
			}
		}
		*/

		for (size_t i = 0; i < sh.nbVertex; i++) {
			Vector3d* v = &vertices3[i];
			if (v->x < sbb.min.x) sbb.min.x = v->x;
			if (v->y < sbb.min.y) sbb.min.y = v->y;
			if (v->z < sbb.min.z) sbb.min.z = v->z;
			if (v->x > sbb.max.x) sbb.max.x = v->x;
			if (v->y > sbb.max.y) sbb.max.y = v->y;
			if (v->z > sbb.max.z) sbb.max.z = v->z;
		}

#ifdef SYNRAD
		//Regions
		Worker *worker = &(mApp->worker);
		for (int i = 0; i < (int)worker->regions.size(); i++) {
			if (worker->regions[i].AABBmin.x < sbb.min.x) sbb.min.x = worker->regions[i].AABBmin.x;
			if (worker->regions[i].AABBmin.y < sbb.min.y) sbb.min.y = worker->regions[i].AABBmin.y;
			if (worker->regions[i].AABBmin.z < sbb.min.z) sbb.min.z = worker->regions[i].AABBmin.z;
			if (worker->regions[i].AABBmax.x > sbb.max.x) sbb.max.x = worker->regions[i].AABBmax.x;
			if (worker->regions[i].AABBmax.y > sbb.max.y) sbb.max.y = worker->regions[i].AABBmax.y;
			if (worker->regions[i].AABBmax.z > sbb.max.z) sbb.max.z = worker->regions[i].AABBmax.z;
		}
#endif

		//If geometry is empty
		if (sbb.min.x == 1e100) sbb.min.x = -1.0;
		if (sbb.min.y == 1e100) sbb.min.y = -1.0;
		if (sbb.min.z == 1e100) sbb.min.z = -1.0;
		if (sbb.max.x == -1e100) sbb.max.x = 1.0;
		if (sbb.max.y == -1e100) sbb.max.y = 1.0;
		if (sbb.max.z == -1e100) sbb.max.z = 1.0;

		return sbb;
	//}

}

Vector3d Geometry::GetCenter() {

	/*if (viewStruct < 0) {

		return center;

	}
	else {*/

		Vector3d r;
		AxisAlignedBoundingBox sbb = GetBB();

		r.x = (sbb.max.x + sbb.min.x) / 2.0;
		r.y = (sbb.max.y + sbb.min.y) / 2.0;
		r.z = (sbb.max.z + sbb.min.z) / 2.0;

		return r;

	//}
}

int Geometry::AddRefVertex(const InterfaceVertex& p, InterfaceVertex *refs, int *nbRef, double vT) {

	bool found = false;
	int i = 0;
	//Vector3d n;
	double v2 = vT*vT;

	while (i < *nbRef && !found) {
		//Sub(&n,p,refs + i);
		double dx = std::abs(p.x - (refs + i)->x);
		if (dx < vT) {
			double dy = std::abs(p.y - (refs + i)->y);
			if (dy < vT) {
				double dz = std::abs(p.z - (refs + i)->z);
				if (dz < vT) {
					found = (dx*dx + dy*dy + dz*dz < v2);
				}
			}
		}
		if (!found) i++;
	}

	if (!found) {
		// Add a new reference vertex
		refs[*nbRef] = p;
		*nbRef = *nbRef + 1;
	}

	return i;

}

void Geometry::CollapseVertex(Worker *work, GLProgress *prg, double totalWork, double vT) {
	mApp->changedSinceSave = true;
	if (!isLoaded) return;
	// Collapse neighbor vertices
	InterfaceVertex *refs = (InterfaceVertex *)malloc(sh.nbVertex * sizeof(InterfaceVertex));
	if (!refs) throw Error("Out of memory: CollapseVertex");
	int      *idx = (int *)malloc(sh.nbVertex * sizeof(int));
	if (!idx) throw Error("Out of memory: CollapseVertex");
	int       nbRef = 0;

	// Collapse
	prg->SetMessage("Collapsing vertices...");
	for (int i = 0; !work->abortRequested && i < sh.nbVertex; i++) {
		mApp->DoEvents();  //Catch abort request
		prg->SetProgress(((double)i / (double)sh.nbVertex) / totalWork);
		idx[i] = AddRefVertex(vertices3[i], refs, &nbRef, vT);
	}

	if (work->abortRequested) {
		delete refs;
		delete idx;
		return;
	}

	// Create the new vertex array
	vertices3.resize(nbRef); vertices3.shrink_to_fit();

	memcpy(vertices3.data(), refs, nbRef * sizeof(InterfaceVertex));
	sh.nbVertex = nbRef;

	// Update facets indices
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		prg->SetProgress(((double)i / (double)sh.nbFacet) * 0.05 + 0.45);
		for (int j = 0; j < f->sh.nbIndex; j++)
			f->indices[j] = idx[f->indices[j]];
	}

	free(idx);
	free(refs);

}

bool Geometry::GetCommonEdges(Facet *f1, Facet *f2, size_t * c1, size_t * c2, size_t * chainLength) {

	// Detect common edge between facet
	size_t p11, p12, p21, p22, lgth, si, sj;
	size_t maxLength = 0;

	for (size_t i = 0; i < f1->sh.nbIndex; i++) {

		p11 = f1->GetIndex(i);
		p12 = f1->GetIndex(i + 1);

		for (int j = 0; j < f2->sh.nbIndex; j++) { //Cannot be size_t, negative values

			p21 = f2->GetIndex(j);
			p22 = f2->GetIndex(j + 1);

			if (p11 == p22 && p12 == p21) {

				// Common edge found
				si = i;
				sj = j;
				lgth = 1;

				// Loop until the end of the common edge chain
				i += 2;
				j -= 1;
				p12 = f1->GetIndex(i);
				p21 = f2->GetIndex(j);
				bool ok = (p12 == p21);
				while (lgth < f1->sh.nbIndex && lgth < f2->sh.nbIndex && ok) {
					p12 = f1->GetIndex(i);
					p21 = f2->GetIndex(j);
					ok = (p12 == p21);
					if (ok) {
						i++; j--;
						lgth++;
					}
				}

				if (lgth > maxLength) {
					*c1 = si;
					*c2 = sj;
					maxLength = lgth;
				}

			}

		}

	}

	if (maxLength > 0) {
		*chainLength = maxLength;
		return true;
	}

	return false;

}

void Geometry::MoveVertexTo(size_t idx, double x, double y, double z) {
	vertices3[idx].x = x;
	vertices3[idx].y = y;
	vertices3[idx].z = z;

}

void Geometry::SwapNormal() {
	//Default: sweap selected facets
	SwapNormal(GetSelectedFacets());
}

void Geometry::RevertFlippedNormals() {
	std::vector<size_t> flippedFacetList;
	auto selectedFacetList = GetSelectedFacets();
	for (auto i : selectedFacetList) {
		if (facets[i]->normalFlipped) {
			flippedFacetList.push_back(i);
		}
	}
	SwapNormal(flippedFacetList);
}

void Geometry::SwapNormal(const std::vector < size_t>& facetList) { //Swap the normal for a list of facets

	if (!IsLoaded()) {
		GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	mApp->changedSinceSave = true;
	for (auto i:facetList) {
		Facet *f = facets[i];
		f->SwapNormal();
		InitializeGeometry((int)i);
		try {
			SetFacetTexture(i, f->tRatio, f->hasMesh);
		}
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		}	
	}

	DeleteGLLists(true, true);
	BuildGLList();

}

void Geometry::Extrude(int mode, Vector3d radiusBase, Vector3d offsetORradiusdir, bool againstNormal, double distanceORradius, double totalAngle, size_t steps) {

	//creates facet from selected vertices

	mApp->changedSinceSave = true;
	auto selectedFacets = GetSelectedFacets();

	for (auto& sel:selectedFacets)
	{
			size_t sourceFacetId = sel;
			facets[sourceFacetId]->selected = false;
			//Update direction if necessary
			Vector3d dir2, axisBase, axis;

			if (mode == 1) { //Use facet normal to determine offset
				dir2 = facets[sourceFacetId]->sh.N * distanceORradius;
				if (againstNormal) dir2 = -1.0 * dir2;
			}
			else if (mode == 2) { //Use provided offset
				dir2 = offsetORradiusdir;
			}
			else if (mode == 3) {
				offsetORradiusdir = offsetORradiusdir.Normalized();
				axisBase = radiusBase + offsetORradiusdir * distanceORradius;
				axis = CrossProduct(facets[sourceFacetId]->sh.N, offsetORradiusdir).Normalized();
			}

			//Resize vertex and facet arrays
			size_t nbNewVertices = facets[sourceFacetId]->sh.nbIndex;
			if (mode == 3) nbNewVertices *= (steps);
			//vertices3 = (InterfaceVertex*)realloc(vertices3, (wp.nbVertex + nbNewVertices) * sizeof(InterfaceVertex));
			vertices3.resize(sh.nbVertex + nbNewVertices);

			size_t nbNewFacets = facets[sourceFacetId]->sh.nbIndex;
			if (mode == 3) nbNewFacets *= (steps);
			nbNewFacets++; //End cap facet
			facets = (Facet **)realloc(facets, (sh.nbFacet + nbNewFacets) * sizeof(Facet *));

			//create new vertices
			if (mode == 1 || mode == 2) {
				for (size_t j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++) {
					vertices3[sh.nbVertex + j].SetLocation(vertices3[facets[sourceFacetId]->indices[j]] + dir2);
					vertices3[sh.nbVertex + j].selected = false;
				}
			}
			else if (mode == 3) {
				for (size_t step = 0; step < steps; step++) {
					for (size_t j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++) {
						vertices3[sh.nbVertex + step*facets[sourceFacetId]->sh.nbIndex + j].SetLocation(
							Rotate(vertices3[facets[sourceFacetId]->indices[j]], axisBase, axis, (step + 1)*totalAngle*(againstNormal ? -1.0 : 1.0) / (double)steps)
						);
						vertices3[sh.nbVertex + step*facets[sourceFacetId]->sh.nbIndex + j].selected = false;
					}
				}
			}

			//Create end cap
			size_t endCapId = sh.nbFacet + nbNewFacets - 1; //last facet
			facets[endCapId] = new Facet(facets[sourceFacetId]->sh.nbIndex);
			facets[endCapId]->selected = true;
			if (viewStruct != -1) facets[endCapId]->sh.superIdx = viewStruct;
			for (int j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++)
				facets[endCapId]->indices[facets[sourceFacetId]->sh.nbIndex - 1 - j] = sh.nbVertex + j + ((mode == 3) ? (steps - 1)*facets[sourceFacetId]->sh.nbIndex : 0); //assign new vertices to new facet in inverse order

			//Construct sides
			//int direction = 1;
			//if (Dot(&dir2, &facets[sourceFacetId]->wp.N) * distanceORradius < 0.0) direction *= -1; //extrusion towards normal or opposite?
			for (size_t step = 0; step < ((mode == 3) ? steps : 1); step++) {
				for (size_t j = 0; j < facets[sourceFacetId]->sh.nbIndex; j++) {
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j] = new Facet(4);
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[0] = (step == 0) ? facets[sourceFacetId]->indices[j] : sh.nbVertex + (step - 1)*facets[sourceFacetId]->sh.nbIndex + j;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[1] = sh.nbVertex + j + (step)*facets[sourceFacetId]->sh.nbIndex;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[2] = sh.nbVertex + (j + 1) % facets[sourceFacetId]->sh.nbIndex + (step)*facets[sourceFacetId]->sh.nbIndex;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->indices[3] = (step == 0) ? facets[sourceFacetId]->GetIndex(j + 1) : sh.nbVertex + (j + 1) % facets[sourceFacetId]->sh.nbIndex + (step - 1)*facets[sourceFacetId]->sh.nbIndex;
					facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->selected = true;
					if (viewStruct != -1) facets[sh.nbFacet + step*facets[sourceFacetId]->sh.nbIndex + j]->sh.superIdx = viewStruct;
				}
			}
			sh.nbVertex += nbNewVertices; //update number of vertices
			sh.nbFacet += nbNewFacets;
	}
	InitializeGeometry();
	mApp->UpdateFacetParams(true);
	UpdateSelection();
	mApp->facetList->SetSelectedRow((int)sh.nbFacet - 1);
	mApp->facetList->ScrollToVisible(sh.nbFacet - 1, 1, false);
}

void Geometry::ShiftVertex() {

	if (!IsLoaded()) {
		GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}
	if (GetNbSelectedFacets() <= 0) return;
	mApp->changedSinceSave = true;
	DeleteGLLists(true, true);
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->selected) {
			f->ShiftVertex();
			InitializeGeometry(i);// Reinitialise geom
			try {
				SetFacetTexture(i, f->tRatio, f->hasMesh);
			}
			catch (Error &e) {
				GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			}

		}
	}
	// Delete old resource
	BuildGLList();
}

void Geometry::Merge(size_t nbV, size_t nbF, Vector3d *nV, Facet **nF) {
	mApp->changedSinceSave = true;
	// Merge the current geometry with the specified one
	if (!nbV || !nbF) return;

	// Reallocate mem
	Facet   **nFacets = (Facet **)malloc((sh.nbFacet + nbF) * sizeof(Facet *));
	//InterfaceVertex *nVertices3 = (InterfaceVertex *)malloc((wp.nbVertex + nbV) * sizeof(InterfaceVertex));

	if (sh.nbFacet) memcpy(nFacets, facets, sizeof(Facet *) * sh.nbFacet);
	memcpy(nFacets + sh.nbFacet, nF, sizeof(Facet *) * nbF);

	//if (wp.nbVertex) memcpy(nVertices3, vertices3, sizeof(InterfaceVertex) * wp.nbVertex);
	vertices3.resize(sh.nbVertex + nbV);
	memcpy(&vertices3[sh.nbVertex], nV, sizeof(InterfaceVertex) * nbV);

	SAFE_FREE(facets);
	//SAFE_FREE(vertices3);
	facets = nFacets;
	//vertices3 = nVertices3;
	//UnselectAllVertex();

	// Shift indices
	for (int i = 0; i < nbF; i++) {
		Facet *f = facets[sh.nbFacet + i];
		for (int j = 0; j < f->sh.nbIndex; j++)
			f->indices[j] += sh.nbVertex;
	}

	sh.nbVertex += nbV;
	sh.nbFacet += nbF;

}

int Geometry::HasIsolatedVertices() {

	// Check if there are unused vertices
	int *check = (int *)malloc(sh.nbVertex * sizeof(int));
	memset(check, 0, sh.nbVertex * sizeof(int));

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->sh.nbIndex; j++) {
			check[f->indices[j]]++;
		}
	}

	int nbUnused = 0;
	for (int i = 0; i < sh.nbVertex; i++) {
		if (!check[i]) nbUnused++;
	}

	SAFE_FREE(check);
	return nbUnused;

}

void  Geometry::DeleteIsolatedVertices(bool selectedOnly) {
	mApp->changedSinceSave = true;
	// Remove unused vertices
	std::vector<bool> isUsed(sh.nbVertex);

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->sh.nbIndex; j++) {
			isUsed[f->indices[j]]=true;
		}
	}

	size_t nbUnused = 0;
	for (size_t i = 0; i < sh.nbVertex; i++) {
		if (!isUsed[i] && !(selectedOnly && !vertices3[i].selected)) nbUnused++;
	}

	size_t nbVert = sh.nbVertex - nbUnused;

	/*
	if (nbVert == 0) {
		// Remove all
		Clear();
		return;
	}
	*/

	// Update facet indices
	std::vector<size_t> newIndex(sh.nbVertex);
	for (size_t i = 0, n = 0; i < sh.nbVertex; i++) {
		if (isUsed[i] || (selectedOnly && !vertices3[i].selected)) {
			newIndex[i] = n;
			n++;
		}
	}
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->sh.nbIndex; j++) {
			f->indices[j] = newIndex[f->indices[j]];
		}
	}

	std::vector<InterfaceVertex> nVert(nbVert);

	for (int i = 0, n = 0; i < sh.nbVertex; i++) {
		if (isUsed[i] || (selectedOnly && !vertices3[i].selected)) {
			nVert[n] = vertices3[i];
			n++;
		}
	}

	vertices3.swap(nVert);
	sh.nbVertex = vertices3.size();
}

void Geometry::Clear() {
	viewStruct = -1; //otherwise a nonexistent structure could stay selected
					 // Free memory
	if (facets) {
		for (int i = 0; i < sh.nbFacet; i++)
			SAFE_DELETE(facets[i]);
		free(facets);
	}
	//if (vertices3) free(vertices3);
	vertices3.clear(); vertices3.shrink_to_fit();
	for (int i = 0; i < sh.nbSuper; i++) {
		SAFE_FREE(strName[i]);
		SAFE_FREE(strFileName[i]);
	}
	memset(strName, 0, MAX_SUPERSTR * sizeof(char *));
	memset(strFileName, 0, MAX_SUPERSTR * sizeof(char *));
	DeleteGLLists(true, true);

	if (mApp && mApp->splitFacet) mApp->splitFacet->ClearUndoFacets();
	if (mApp && mApp->buildIntersection) mApp->buildIntersection->ClearUndoFacets();
	if (mApp && mApp->mirrorFacet) mApp->mirrorFacet->ClearUndoVertices();
	if (mApp && mApp->mirrorVertex) mApp->mirrorVertex->ClearUndoVertices();

	// Init default
	facets = NULL;         // Facets array
	sh.nbFacet = 0;        // Number of facets
	sh.nbVertex = 0;       // Number of vertex
	isLoaded = false;      // isLoaded flag
	sh.nbSuper = 0;          // Structure number
	ResetTextureLimits();
	EmptySelectedVertexList();

	memset(lineList, 0, sizeof(lineList));
	//memset(strName, 0, sizeof(strName));
	//memset(strFileName, 0, sizeof(strFileName));

	// Init OpenGL material
	memset(&whiteMaterial, 0, sizeof(GLMATERIAL));
	whiteMaterial.Diffuse.r = 0.9f;
	whiteMaterial.Diffuse.g = 0.9f;
	whiteMaterial.Diffuse.b = 0.9f;
	whiteMaterial.Ambient.r = 0.9f;
	whiteMaterial.Ambient.g = 0.9f;
	whiteMaterial.Ambient.b = 0.9f;

	memset(&fillMaterial, 0, sizeof(GLMATERIAL));

	fillMaterial.Diffuse.r = 0.6f;
	fillMaterial.Diffuse.g = 0.65f;
	fillMaterial.Diffuse.b = 0.65f;
	fillMaterial.Ambient.r = 0.45f;
	fillMaterial.Ambient.g = 0.41f;
	fillMaterial.Ambient.b = 0.41f;

	memset(&arrowMaterial, 0, sizeof(GLMATERIAL));
	arrowMaterial.Diffuse.r = 0.4f;
	arrowMaterial.Diffuse.g = 0.2f;
	arrowMaterial.Diffuse.b = 0.0f;
	arrowMaterial.Ambient.r = 0.6f;
	arrowMaterial.Ambient.g = 0.3f;
	arrowMaterial.Ambient.b = 0.0f;

	nbSelectedHist = 0;    // Selection history
}

void  Geometry::SelectIsolatedVertices() {
	
	std::vector<bool> isolated(sh.nbVertex, true);

	for (size_t i = 0; i < sh.nbFacet; i++) {
		for (const auto& ind : facets[i]->indices) {
			isolated[ind]=false;
		}
	}

	for (size_t i = 0; i < sh.nbVertex; i++) {
		vertices3[i].selected = isolated[i];
	}
}

bool Geometry::RemoveCollinear() {
	std::vector<size_t> facetsToDelete;
	for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->collinear) facetsToDelete.push_back(i);
	RemoveFacets(facetsToDelete);
	return (facetsToDelete.size() > 0);
}

void Geometry::RemoveSelectedVertex() {

	mApp->changedSinceSave = true;

	//Analyze facets
	std::vector<size_t> facetsToRemove, facetsToChange;
	for (int f = 0; f < sh.nbFacet; f++) {
		int nbSelVertex = 0;
		for (int i = 0; i < facets[f]->sh.nbIndex; i++)
			if (vertices3[facets[f]->indices[i]].selected)
				nbSelVertex++;
		if (nbSelVertex) {
			facetsToChange.push_back(f);
			if ((facets[f]->sh.nbIndex - nbSelVertex) <= 2)
				facetsToRemove.push_back(f);
		}
	}

	for (size_t c = 0; c < facetsToChange.size(); c++) {
		Facet* f = facets[facetsToChange[c]];
		size_t nbRemove = 0;
		for (size_t i = 0; (int)i < f->sh.nbIndex; i++) //count how many to remove			
			if (vertices3[f->indices[i]].selected)
				nbRemove++;
		std::vector<size_t> newIndices(f->sh.nbIndex - nbRemove);
		int nb = 0;
		for (size_t i = 0; (int)i < f->sh.nbIndex; i++)
			if (!vertices3[f->indices[i]].selected) newIndices[nb++] = f->indices[i];

		f->indices = newIndices;
		f->sh.nbIndex -= nbRemove;
		f->vertices2.resize(f->sh.nbIndex);
		f->visible.resize(f->sh.nbIndex);
	}

	RemoveFacets(facetsToRemove);
	DeleteIsolatedVertices(true);
}

void Geometry::RemoveFacets(const std::vector<size_t> &facetIdList, bool doNotDestroy) {
	//Think of calling UpdateModelParams() after executed to refresh facet hit list

	if (facetIdList.size() == 0) return;
	mApp->changedSinceSave = true;
	Facet   **f = (Facet **)malloc((sh.nbFacet - facetIdList.size()) * sizeof(Facet *));
	std::vector<bool> facetSelected(sh.nbFacet, false);
	std::vector<int> newRefs(sh.nbFacet, -1);
	for (size_t toRemove : facetIdList) {
		facetSelected[toRemove] = true;
	}

	size_t nb = 0;

	for (size_t i = 0; i < sh.nbFacet; i++) {
		if (facetSelected[i]) {
			if (!doNotDestroy) SAFE_DELETE(facets[i]); //Otherwise it's referenced by an Undo list
		}

		else {
			f[nb] = facets[i];
			newRefs[i] = (int)nb;
			nb++;
		}
	}

	SAFE_FREE(facets);
	facets = f;
	sh.nbFacet = nb;

	mApp->RenumberSelections(newRefs);
	mApp->RenumberFormulas(&newRefs);
	RenumberNeighbors(newRefs);

	// Delete old resources
	DeleteGLLists(true, true);
	BuildGLList();
}

void Geometry::RestoreFacets(std::vector<DeletedFacet> deletedFacetList, bool toEnd) {
	//size_t nbNew = 0;
	std::vector<int> newRefs(sh.nbFacet, -1);
	/*for (auto& restoreFacet : deletedFacetList)
		if (restoreFacet.ori_pos >= wp.nbFacet || toEnd) nbNew++;*/
	Facet** tempFacets = (Facet**)malloc(sizeof(Facet*)*(sh.nbFacet + /*nbNew*/ deletedFacetList.size()));
	size_t pos = 0;
	size_t nbInsert = 0;
	if (toEnd) { //insert to end
		for (size_t insertPos = 0; insertPos < sh.nbFacet; insertPos++) { //Original facets
			tempFacets[insertPos] = facets[insertPos];
			newRefs[insertPos] = (int)insertPos;
		}
		for (auto& restoreFacet : deletedFacetList) {
			tempFacets[sh.nbFacet + nbInsert] = restoreFacet.f;
			tempFacets[sh.nbFacet + nbInsert]->selected = true;
			nbInsert++;
		}
	}
	else { //Insert to original locations
		for (auto& restoreFacet : deletedFacetList) {
			for (size_t insertPos = pos; insertPos < restoreFacet.ori_pos; insertPos++) {
				tempFacets[insertPos] = facets[insertPos - nbInsert];
				newRefs[insertPos - nbInsert] = (int)insertPos;
				pos++;
			}
			//if (restoreFacet.replaceOri) pos--;
			tempFacets[pos] = restoreFacet.f;
			tempFacets[pos]->selected = true;
			pos++;
			if (!restoreFacet.replaceOri) nbInsert++;
		}
		//Remaining facets
		for (size_t insertPos = pos; insertPos < (sh.nbFacet + nbInsert); insertPos++) {
			tempFacets[insertPos] = facets[insertPos - nbInsert];
			newRefs[insertPos - nbInsert] = (int)insertPos;
		}
		//_ASSERTE(_CrtCheckMemory());
		//Renumber things;
		RenumberNeighbors(newRefs);
		mApp->RenumberFormulas(&newRefs);
		mApp->RenumberSelections(newRefs);
	}

	sh.nbFacet += nbInsert;
	SAFE_FREE(facets);
	facets = tempFacets;
	InitializeGeometry();
}

bool Geometry::RemoveNullFacet() {

	// Remove degenerated facet (area~0.0)
	std::vector<size_t> facetsToDelete;

	double areaMin = 1E-20;
	for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->sh.area < areaMin) facetsToDelete.push_back(i);
	RemoveFacets(facetsToDelete);

	return (facetsToDelete.size() > 0);
}

void Geometry::AlignFacets(std::vector<size_t> memorizedSelection, size_t sourceFacetId, size_t destFacetId, size_t anchorSourceVertexId, size_t anchorDestVertexId,
	size_t alignerSourceVertexId, size_t alignerDestVertexId, bool invertNormal, bool invertDir1, bool invertDir2, bool copy, Worker *worker) {

	double counter = 0.0;
	GLProgress *prgAlign = new GLProgress("Aligning facets...", "Please wait");
	prgAlign->SetProgress(0.0);
	prgAlign->SetVisible(true);
	if (!mApp->AskToReset(worker)) return;
	//if (copy) CloneSelectedFacets(); //Causes problems
	std::vector<bool> alreadyMoved(sh.nbVertex, false);

	//Translating facets to align anchors
	size_t temp;
	if (invertDir1) { //change anchor and direction on source
		temp = alignerSourceVertexId;
		alignerSourceVertexId = anchorSourceVertexId;
		anchorSourceVertexId = temp;
	}
	if (invertDir2) { //change anchor and direction on destination
		temp = alignerDestVertexId;
		alignerDestVertexId = anchorDestVertexId;
		anchorDestVertexId = temp;
	}
	Vector3d Translation = vertices3[anchorDestVertexId] - vertices3[anchorSourceVertexId];

	int nb = 0;
	for (const auto& sel : memorizedSelection) {
		counter += 0.333;
		prgAlign->SetProgress(counter / (double)memorizedSelection.size());
		for (const auto& ind: facets[sel]->indices) {
			if (!alreadyMoved[ind]) {
				vertices3[ind].SetLocation(vertices3[ind] + Translation);
				alreadyMoved[ind] = true;
			}
		}
	}

	//Rotating to match normal vectors
	Vector3d Axis;
	Vector3d Normal;
	double angle;
	Normal = facets[destFacetId]->sh.N;
	if (invertNormal) Normal = Normal * -1.0;
	Axis = CrossProduct(facets[sourceFacetId]->sh.N, Normal);
	if (Axis.Norme() < 1e-5) { //The two normals are either collinear or the opposite
		if ((Dot(facets[destFacetId]->sh.N, facets[sourceFacetId]->sh.N) > 0.99999 && (!invertNormal)) ||
			(Dot(facets[destFacetId]->sh.N, facets[sourceFacetId]->sh.N) < 0.00001 && invertNormal)) { //no rotation needed
			Axis.x = 1.0;
			Axis.y = 0.0;
			Axis.z = 0.0;
			angle = 0.0;
		}
		else { //180deg rotation needed
			Axis = CrossProduct(facets[sourceFacetId]->sh.U, facets[sourceFacetId]->sh.N).Normalized();
			angle = PI;
		}
	}
	else {
		Axis = Axis.Normalized();
		angle = acos(Dot(facets[sourceFacetId]->sh.N, facets[destFacetId]->sh.N) / (facets[sourceFacetId]->sh.N.Norme() * facets[destFacetId]->sh.N.Norme()));
		//angle = angle / PI * 180;
		if (invertNormal) angle = PI - angle;
	}

	std::vector<bool> alreadyRotated(sh.nbVertex, false);

	nb = 0;
	for (auto& sel : memorizedSelection) {
		counter += 0.333;
		prgAlign->SetProgress(counter / (double)memorizedSelection.size());
		for (const auto& ind : facets[sel]->indices) {
			if (!alreadyRotated[ind]) {
				//rotation comes here
				vertices3[ind].SetLocation(Rotate(vertices3[ind], vertices3[anchorDestVertexId], Axis, angle));
				alreadyRotated[ind] = true;
			}
		}
	}

	//Rotating to match direction points

	Vector3d Dir1 = vertices3[alignerDestVertexId] - vertices3[anchorDestVertexId];
	Vector3d Dir2 = vertices3[alignerSourceVertexId] - vertices3[anchorSourceVertexId];
	Axis = CrossProduct(Dir2, Dir1);
	if (Axis.Norme() < 1e-5) { //The two directions are either collinear or the opposite
		if (Dot(Dir1, Dir2) > 0.99999) { //no rotation needed
			Axis.x = 1.0;
			Axis.y = 0.0;
			Axis.z = 0.0;
			angle = 0.0;
		}
		else { //180deg rotation needed
			//construct a vector perpendicular to the normal
			Axis = facets[sourceFacetId]->sh.N.Normalized();
			angle = PI;
		}
	}
	else {
		Axis = Axis.Normalized();
		angle = Dot(Dir1, Dir2) / (Dir1.Norme() * Dir2.Norme());
		//bool opposite=(angle<0.0);
		angle = acos(angle);
		//angle = angle / PI * 180;
		//if (invertNormal) angle=180.0-angle;
	}

	std::vector<bool> alreadyRotated2(sh.nbVertex, false);

	nb = 0;
	for (auto& sel : memorizedSelection) {
		counter += 0.333;
		prgAlign->SetProgress(counter / memorizedSelection.size());
		for (const auto& ind : facets[sel]->indices) {
			if (!alreadyRotated2[ind]) {
				//rotation comes here
				vertices3[ind].SetLocation(Rotate(vertices3[ind], vertices3[anchorDestVertexId], Axis, angle));
				alreadyRotated2[ind] = true;
			}
		}
	}

	InitializeGeometry();
	//update textures
	/*try {
		for (int i = 0; i < nbSelected; i++)
			SetFacetTexture(selection[i], facets[selection[i]]->tRatio, facets[selection[i]]->hasMesh);
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}*/
	prgAlign->SetVisible(false);
	SAFE_DELETE(prgAlign);
}

void Geometry::MoveSelectedFacets(double dX, double dY, double dZ, bool towardsDirectionMode, double distance, bool copy) {

	GLProgress *prgMove = new GLProgress("Moving selected facets...", "Please wait");
	prgMove->SetProgress(0.0);
	prgMove->SetVisible(true);
	auto selectedFacets = GetSelectedFacets();

	Vector3d delta = Vector3d(dX, dY, dZ);
	Vector3d translation = towardsDirectionMode ? distance*delta.Normalized() : delta ;

	if (translation.Norme()>0.0) {
		mApp->changedSinceSave = true;
		if (copy) CloneSelectedFacets(); //move
		selectedFacets = GetSelectedFacets(); //Update selection to cloned
		double counter = 1.0;

		std::vector<bool> alreadyMoved(sh.nbVertex, false);

		for (const auto& sel : selectedFacets) {
			counter += 1.0;
			prgMove->SetProgress(counter / selectedFacets.size());
			for (const auto& ind : facets[sel]->indices) {
				if (!alreadyMoved[ind]) {
					vertices3[ind].SetLocation(vertices3[ind] + translation);
					alreadyMoved[ind] = true;
				}
			}
		}

		InitializeGeometry();
		//update textures
		/*try {
			for (int i = 0; i < wp.nbFacet; i++) if (facets[i]->selected) SetFacetTexture(i, facets[i]->tRatio, facets[i]->hasMesh);
		}
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}*/
	}
	prgMove->SetVisible(false);
	SAFE_DELETE(prgMove);
}

std::vector<UndoPoint> Geometry::MirrorProjectSelectedFacets(Vector3d P0, Vector3d N, bool project, bool copy, Worker *worker) {
	std::vector<UndoPoint> undoPoints;
	double counter = 0.0;
	auto selectedFacets = GetSelectedFacets();
	if (selectedFacets.size() == 0) return undoPoints;
	GLProgress *prgMirror = new GLProgress("Mirroring selected facets...", "Please wait");
	prgMirror->SetProgress(0.0);
	prgMirror->SetVisible(true);

	if (!mApp->AskToReset(worker)) return undoPoints;
	int nbSelFacet = 0;
	if (copy) CloneSelectedFacets();
	selectedFacets = GetSelectedFacets(); //Update selection to cloned
	std::vector<bool> alreadyMirrored(sh.nbVertex, false);

	int nb = 0;
	for (const auto& sel : selectedFacets) {
		counter += 1.0;
		prgMirror->SetProgress(counter / selectedFacets.size());
		nbSelFacet++;
		for (const auto& ind : facets[sel]->indices) {
			if (!alreadyMirrored[ind]) {
				Vector3d newPosition;
				if (project) {
					newPosition = Project(vertices3[ind], P0, N);
					if (!copy) {
						UndoPoint oriPoint;
						oriPoint.oriPos = vertices3[ind];
						oriPoint.oriId = ind;
						undoPoints.push_back(oriPoint);
					}
				}
				else {
					//Mirror
					newPosition = Mirror(vertices3[ind], P0, N);
				}
				vertices3[ind].SetLocation(newPosition);
				alreadyMirrored[ind] = true;
			}
		}
	}

	if (nbSelFacet == 0) return undoPoints;
	if (!project) SwapNormal();
	InitializeGeometry();
	//update textures
	/*try {
		for (int i = 0; i < wp.nbFacet; i++) if (facets[i]->selected) SetFacetTexture(i, facets[i]->tRatio, facets[i]->hasMesh);
	}
	catch (Error &e) {
		GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}*/

	prgMirror->SetVisible(false);
	SAFE_DELETE(prgMirror);
	return undoPoints;
}

std::vector<UndoPoint> Geometry::MirrorProjectSelectedVertices(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, bool project, bool copy, Worker *worker) {
	std::vector<UndoPoint> undoPoints;
	size_t nbVertexOri = sh.nbVertex;
	for (size_t i = 0; i < nbVertexOri; i++) {
		if (vertices3[i].selected) {
			Vector3d newPosition;
			if (!project) {
				newPosition = Mirror(vertices3[i], AXIS_P0, AXIS_DIR);
			}
			else {
				newPosition = Project(vertices3[i], AXIS_P0, AXIS_DIR);
				if (!copy) {
					UndoPoint oriPoint;
					oriPoint.oriPos = vertices3[i];
					oriPoint.oriId = i;
					undoPoints.push_back(oriPoint);
				}
			}
			if (!copy) {
				vertices3[i].SetLocation(newPosition);
			}
			else {
				AddVertex(newPosition);
			}
		}
	}
	InitializeGeometry();
	return undoPoints;
}

void Geometry::RotateSelectedFacets(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker) {

	auto selectedFacets = GetSelectedFacets();
	double counter = 0.0;
	if (selectedFacets.size() == 0) return;
	GLProgress *prgRotate = new GLProgress("Rotating selected facets...", "Please wait");
	prgRotate->SetProgress(0.0);
	prgRotate->SetVisible(true);

	if (theta != 0.0) {
		if (!mApp->AskToReset(worker)) return;
		if (copy) CloneSelectedFacets();
		selectedFacets = GetSelectedFacets(); //Update selection to cloned

		std::vector<bool> alreadyRotated(sh.nbVertex, false);

		int nb = 0;
		for (const auto& sel : selectedFacets) {
			counter += 1.0;
			prgRotate->SetProgress(counter / selectedFacets.size());
			for (const auto& ind : facets[sel]->indices) {
				if (!alreadyRotated[ind]) {
					//rotation comes here
					vertices3[ind].SetLocation(Rotate(vertices3[ind], AXIS_P0, AXIS_DIR, theta));
					alreadyRotated[ind] = true;
				}
			}
		}
	
		InitializeGeometry();
		//update textures
		/*try {
			for (int i = 0; i < wp.nbFacet; i++) if (facets[i]->selected) SetFacetTexture(i, facets[i]->tRatio, facets[i]->hasMesh);

		}
		catch (Error &e) {
			GLMessageBox::Display(e.GetMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}*/

	}
	prgRotate->SetVisible(false);
	SAFE_DELETE(prgRotate);
}

void Geometry::RotateSelectedVertices(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker) {

	if (!copy) { //move
		for (int i = 0; i < sh.nbVertex; i++) {
			if (vertices3[i].selected) {
				vertices3[i].SetLocation(Rotate(vertices3[i], AXIS_P0, AXIS_DIR, theta));
			}
		}
		InitializeGeometry();
	}

	else { //copy
		size_t nbVertexOri = sh.nbVertex;
		for (size_t i = 0; i < nbVertexOri; i++) {
			if (vertices3[i].selected) {
				AddVertex(Rotate(vertices3[i], AXIS_P0, AXIS_DIR, theta));
				//vertices3[i].selected = false; //Unselect original
			}
		}
	}
}

void Geometry::CloneSelectedFacets() { //create clone of selected facets
	auto selectedFacetIds = GetSelectedFacets();
	std::vector<bool> isCopied(sh.nbVertex, false); //we keep log of what has been copied to prevent creating duplicates
	std::vector<InterfaceVertex> newVertices;		//vertices that we create
	std::vector<size_t> newIndices(sh.nbVertex);    //which new vertex was created from this old one

	for (const auto& sel : selectedFacetIds) {
		for (size_t ind = 0; ind < facets[sel]->sh.nbIndex; ind++) {
			size_t vertexId = facets[sel]->indices[ind];
			if (!isCopied[vertexId]) {
				isCopied[vertexId] = true; //mark as copied
				newIndices[vertexId] = sh.nbVertex + newVertices.size(); //remember clone's index
				newVertices.push_back(InterfaceVertex(vertices3[vertexId])); //create clone
			}
		}
	}

	/*
	vertices3 = (InterfaceVertex*)realloc(vertices3, (wp.nbVertex + newVertices.size()) * sizeof(InterfaceVertex)); //Increase vertices3 array

	//fill the new vertices with references to the old ones
	for (size_t newVertexId = 0; newVertexId < newVertices.size(); newVertexId++) {
		vertices3[wp.nbVertex + newVertexId] = newVertices[newVertexId];
	}
	wp.nbVertex += newVertices.size(); //update number of vertices
	*/
	vertices3.insert(vertices3.end(), newVertices.begin(), newVertices.end());
	sh.nbVertex = vertices3.size();

	facets = (Facet **)realloc(facets, (sh.nbFacet + selectedFacetIds.size()) * sizeof(Facet *)); //make space for new facets
	for (size_t i = 0; i < selectedFacetIds.size(); i++) {
		facets[sh.nbFacet + i] = new Facet(facets[selectedFacetIds[i]]->sh.nbIndex); //create new facets
		facets[sh.nbFacet + i]->CopyFacetProperties(facets[selectedFacetIds[i]], false); //get properties
		//replace indices with clones
		for (size_t j = 0; j < facets[selectedFacetIds[i]]->sh.nbIndex; j++) {
			facets[sh.nbFacet + i]->indices[j] = newIndices[facets[selectedFacetIds[i]]->indices[j]];
		}
		facets[sh.nbFacet + i]->selected = true; //Select new facets
		facets[selectedFacetIds[i]]->selected = false; //Deselect original facets
	}
	sh.nbFacet += selectedFacetIds.size();
}

void Geometry::MoveSelectedVertex(double dX, double dY, double dZ, bool towardsDirectionMode, double distance, bool copy) {

	GLProgress *prgMove = new GLProgress("Moving selected vertices...", "Please wait");
	prgMove->SetProgress(0.0);
	prgMove->SetVisible(true);
	auto selectedVertices = GetSelectedVertices();

	Vector3d delta = Vector3d(dX, dY, dZ);
	Vector3d translation = towardsDirectionMode ? distance*delta.Normalized() : delta;

	if (translation.Norme()>0.0) {
		mApp->changedSinceSave = true;
		
		double counter = 1.0;
		for (auto& i:selectedVertices) {
			counter += 1.0;
			prgMove->SetProgress(counter / selectedVertices.size());
			Vector3d newLocation = vertices3[i] + translation;
			if (!copy) {
				vertices3[i].SetLocation(newLocation);
			}
			else {
				AddVertex(newLocation);
			}
		}
		if (!copy) InitializeGeometry(); //Geometry changed
	}
	prgMove->SetVisible(false);
	SAFE_DELETE(prgMove);
}

void Geometry::AddVertex(const Vector3d& location, bool selected) {
	mApp->changedSinceSave = true;

	//a new vertex
	sh.nbVertex++;
	InterfaceVertex newVertex;
	newVertex.SetLocation(location);
	newVertex.selected = selected;
	vertices3.push_back(newVertex);
	/*
	InterfaceVertex *verticesNew = (InterfaceVertex *)malloc(wp.nbVertex * sizeof(InterfaceVertex));
	memcpy(verticesNew, vertices3, (wp.nbVertex - 1) * sizeof(InterfaceVertex)); //copy old vertices
	SAFE_FREE(vertices3);
	verticesNew[wp.nbVertex - 1].SetLocation(location);
	verticesNew[wp.nbVertex - 1].selected = selected;
	vertices3 = verticesNew;
	*/
}

void Geometry::AddVertex(double X, double Y, double Z, bool selected) {
	AddVertex(Vector3d(X, Y, Z), selected);
}

std::vector<size_t> Geometry::GetSelectedFacets() {
	std::vector<size_t> selection;
	for (size_t i = 0; i < sh.nbFacet; i++)
		if (facets[i]->selected) selection.push_back(i);
	return selection;
}

std::vector<size_t> Geometry::GetNonPlanarFacetIds(const double& tolerance) {
	std::vector<size_t> nonPlanar;
	for (size_t i = 0; i < sh.nbFacet; i++)
		if (facets[i]->nonSimple || std::abs(facets[i]->planarityError)>=tolerance) nonPlanar.push_back(i);
	return nonPlanar;
}

size_t Geometry::GetNbSelectedFacets()
{
	size_t nb = 0;
	for (size_t i = 0; i < sh.nbFacet; i++)
		if (facets[i]->selected) nb++;
	return nb;
}

void Geometry::SetSelection(std::vector<size_t> selectedFacets, bool isShiftDown, bool isCtrlDown) {
	if (!isShiftDown && !isCtrlDown) UnselectAll(); //Set selection
	for (auto& sel : selectedFacets) {
		if (sel < sh.nbFacet) facets[sel]->selected = !isCtrlDown;
	}
	UpdateSelection();
	if (selectedFacets.size()) mApp->facetList->ScrollToVisible(selectedFacets.back(), 0, true); //in facet list, select the last facet of selection group
	mApp->UpdateFacetParams(true);
}

void Geometry::AddStruct(const char *name,bool deferDrawing) {
	strName[sh.nbSuper++] = strdup(name);
	if (!deferDrawing) BuildGLList();
}

void Geometry::DelStruct(int numToDel) {

	RemoveFromStruct(numToDel);
	CheckIsolatedVertex();
	mApp->UpdateModelParams();

	for (int i = 0; i < sh.nbFacet; i++) {
		if (facets[i]->sh.superIdx > numToDel) facets[i]->sh.superIdx--;
		if (facets[i]->sh.superDest > numToDel) facets[i]->sh.superDest--;
	}
	SAFE_FREE(strName[numToDel]);
	SAFE_FREE(strFileName[numToDel]);
	for (int j = numToDel; j < (sh.nbSuper - 1); j++)
	{
		//strName[j] = strdup(strName[j + 1]);
		strName[j] = strName[j + 1];
		strFileName[j] = strFileName[j + 1];
	}
	sh.nbSuper--;
	BuildGLList();
}

void Geometry::ScaleSelectedVertices(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker) {

	if (!mApp->AskToReset(worker)) return;
	mApp->changedSinceSave = true;

	size_t nbVertexOri = sh.nbVertex;

	for (size_t i = 0; i < nbVertexOri; i++) {
		if (vertices3[i].selected) {
			Vector3d newPosition;
			newPosition.x = invariant.x + factorX*(vertices3[i].x - invariant.x);
			newPosition.y = invariant.y + factorY*(vertices3[i].y - invariant.y);
			newPosition.z = invariant.z + factorZ*(vertices3[i].z - invariant.z);
			if (!copy) {
				vertices3[i].SetLocation(newPosition); //Move
			}
			else {
				AddVertex(newPosition, true);
			}
		}
	}

	InitializeGeometry();
}

void Geometry::ScaleSelectedFacets(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker) {

	GLProgress *prgMove = new GLProgress("Scaling selected facets...", "Please wait");
	prgMove->SetProgress(0.0);
	prgMove->SetVisible(true);

	if (!mApp->AskToReset(worker)) return;
	if (copy) CloneSelectedFacets();
	auto selectedFacets = GetSelectedFacets(); //Update selection to cloned
	double counter = 1.0;
	double selected = (double)GetNbSelectedFacets();
	if (selected == 0.0) return;

	std::vector<bool> alreadyMoved(sh.nbVertex, false);

	int nb = 0;
	for (auto& i:selectedFacets) {
			counter += 1.0;
			prgMove->SetProgress(counter / selected);
			for (int j = 0; j < facets[i]->sh.nbIndex; j++) {
				if (!alreadyMoved[facets[i]->indices[j]]) {
					vertices3[facets[i]->indices[j]].x = invariant.x + factorX*(vertices3[facets[i]->indices[j]].x - invariant.x);
					vertices3[facets[i]->indices[j]].y = invariant.y + factorY*(vertices3[facets[i]->indices[j]].y - invariant.y);
					vertices3[facets[i]->indices[j]].z = invariant.z + factorZ*(vertices3[facets[i]->indices[j]].z - invariant.z);
					alreadyMoved[facets[i]->indices[j]] = true;
				}
			}
	}

	InitializeGeometry();   

	prgMove->SetVisible(false);
	SAFE_DELETE(prgMove);
}

ClippingVertex::ClippingVertex() {
	visited = false;
	isLink = false;
}

bool operator<(const std::list<ClippingVertex>::iterator& a, const std::list<ClippingVertex>::iterator& b) {
	return (a->distance < b->distance);
}

bool Geometry::IntersectingPlaneWithLine(const Vector3d &P0, const Vector3d &u, const Vector3d &V0, const Vector3d &n, Vector3d *intersectPoint, bool withinSection) {
	//Notations from http://geomalgorithms.com/a05-_intersect-1.html
	//At this point, intersecting ray is L=P0+s*u
	if (IsZero(Dot(n, u))) return false; //Check for parallelness
	Vector3d w = P0 - V0;
	if (IsZero(Dot(n, w))) return false; //Check for inclusion
	//Intersection point: P(s_i)-V0=w+s_i*u -> s_i=(-n*w)/(n*u)
	double s_i = -Dot(n, w) / Dot(n, u);
	if (withinSection && ((s_i < 0) || (s_i > 1.0))) return false;
	//P(s_i)=V0+w+s*u
	*intersectPoint = V0 + w + s_i * u;
	return true;
}

struct IntersectPoint {
	size_t vertexId; //global Id
	size_t withFacetId; //With which facet did it intersect (global Id)
};

struct IntersectFacet {
	size_t id;
	Facet* f;
	//std::vector<std::vector<size_t>> visitedFromThisIndice;
	std::vector<std::vector<IntersectPoint>> intersectionPointId;
	std::vector<IntersectPoint> intersectingPoints; //Intersection points with other facets, not on own edge
};

struct EdgePoint {
	size_t vertexId;
	int onEdge;
	bool visited;
};

std::vector<DeletedFacet> Geometry::BuildIntersection(size_t *nbCreated) {
	mApp->changedSinceSave = true;
	//UnselectAllVertex();
	//std::vector<size_t> result;
	std::vector<InterfaceVertex> newVertices;
	std::vector<IntersectFacet> selectedFacets;
	std::vector<DeletedFacet> deletedFacetList;
	*nbCreated = 0; //Total number of new facets created

	//Populate selected facets
	for (size_t i = 0; i < sh.nbFacet; i++) {
		if (facets[i]->selected) {
			IntersectFacet facet;
			facet.id = i;
			facet.f = facets[i];
			//facet.visitedFromThisIndice.resize(facet.f->wp.nbIndex);
			facet.intersectionPointId.resize(facet.f->sh.nbIndex);
			selectedFacets.push_back(facet);
		}
	}
	for (size_t i = 0; i < selectedFacets.size(); i++) {
		Facet* f1 = selectedFacets[i].f;
		for (size_t j = 0; j < selectedFacets.size(); j++) {
			Facet* f2 = selectedFacets[j].f;
			if (i != j) {
				size_t c1, c2, l;
				if (!GetCommonEdges(f1, f2, &c1, &c2, &l)) {
					for (size_t index = 0; index < f1->sh.nbIndex; index++) { //Go through all indexes of edge-finding facet
						InterfaceVertex intersectionPoint;
						InterfaceVertex base = vertices3[f1->indices[index]];
						Vector3d side = vertices3[f1->GetIndex(index + 1)] - base;

						/*
						//Check if this edge was already checked
						bool found = false;
						for (size_t f_other = 0;found == false && f_other < selectedFacets.size();f_other++) { //Compare with all facets
							if (i != f_other) {
								for (size_t v_other = 0;found == false && v_other < selectedFacets[f_other].f->wp.nbIndex;v_other++) { //Compare with all other facets' all other vertices
									for (size_t visited_v_other = 0;found == false && visited_v_other < selectedFacets[f_other].visitedFromThisIndice[v_other].size();visited_v_other++) //Check if we visited ourselves from an other vertex
										if (selectedFacets[f_other].visitedFromThisIndice[v_other][visited_v_other] == f1->indices[index]) //Found a common point
											found = selectedFacets[f_other].f->indices[v_other] == f1->GetIndex(index + 1);
								}
							}
						}
						if (found) continue; //Skip checking this edge, already checked
						*/
						//selectedFacets[i].visitedFromThisIndice[index].push_back(f1->GetIndex(index + 1));
						if (IntersectingPlaneWithLine(base, side, f2->sh.O, f2->sh.N, &intersectionPoint, true)) {
							Vector2d projected = ProjectVertex(intersectionPoint, f2->sh.U, f2->sh.V, f2->sh.O);
							bool inPoly = IsInPoly(projected, f2->vertices2);
							bool onEdge = IsOnPolyEdge(projected.u, projected.v, f2->vertices2, 1E-6);
							//onEdge = false;
							if (inPoly || onEdge) {
								//Intersection found. First check if we already created this point
								int foundId = -1;
								for (size_t v = 0; foundId == -1 && v < newVertices.size(); v++) {
									if (IsZero((newVertices[v] - intersectionPoint).Norme()))
										foundId = (int)v;
								}
								IntersectPoint newPoint, newPointOtherFacet;
								if (foundId == -1) { //Register new intersection point
									newPoint.vertexId = newPointOtherFacet.vertexId = sh.nbVertex + newVertices.size();

									intersectionPoint.selected = false;
									newVertices.push_back(intersectionPoint);
								}
								else { //Refer to existing intersection point
									newPoint.vertexId = newPointOtherFacet.vertexId = foundId + sh.nbVertex;
								}
								newPoint.withFacetId = j;
								selectedFacets[i].intersectionPointId[index].push_back(newPoint);
								newPointOtherFacet.withFacetId = i; //With my edge
								if (!onEdge) selectedFacets[j].intersectingPoints.push_back(newPointOtherFacet); //Other facet's plane intersected
							}
						}
					}
				}
			}
		}
	}
	/*
	vertices3 = (InterfaceVertex*)realloc(vertices3, sizeof(InterfaceVertex)*(wp.nbVertex + newVertices.size()));
	for (InterfaceVertex vertex : newVertices) {
		vertices3[wp.nbVertex] = vertex;
		//result.push_back(wp.nbVertex);
		wp.nbVertex++;
	}*/
	vertices3.insert(vertices3.end(), newVertices.begin(), newVertices.end());
	sh.nbVertex += newVertices.size();

	UnselectAll();
	for (size_t facetId = 0; facetId < selectedFacets.size(); facetId++) {
		std::vector<std::vector<EdgePoint>> clipPaths;
		Facet *f = selectedFacets[facetId].f;
		for (size_t vertexId = 0; vertexId < selectedFacets[facetId].f->sh.nbIndex; vertexId++) { //Go through indices
			//testPath.push_back(f->indices[vertexId]);
			for (size_t v = 0; v < selectedFacets[facetId].intersectionPointId[vertexId].size(); v++) { //If there are intersection points on this edge, go through them 
				//Check if not the end of an already registered clipping path
				bool found = false;
				for (size_t i = 0; found == false && i < clipPaths.size(); i++) {
					found = clipPaths[i].back().vertexId == selectedFacets[facetId].intersectionPointId[vertexId][v].vertexId;
				}
				if (!found) { //Register a new clip path
					std::vector<EdgePoint> path;
					EdgePoint p;
					p.vertexId = selectedFacets[facetId].intersectionPointId[vertexId][v].vertexId;
					p.onEdge = (int)vertexId;
					path.push_back(p); //Register intersection point
					size_t searchId = selectedFacets[facetId].intersectionPointId[vertexId][v].vertexId; //Current point, by global Id
					size_t searchFacetId = selectedFacets[facetId].intersectionPointId[vertexId][v].withFacetId; // Current facet with which we intersected
					//v++;
					int foundId, foundId2;
					do {
						foundId = -1;
						for (size_t p1 = 0; foundId == -1 && p1 < selectedFacets[facetId].intersectingPoints.size(); p1++) { //Get the next intersection point with same facet
							if (searchId == p1) continue;
							foundId = ((selectedFacets[facetId].intersectingPoints[p1].withFacetId == searchFacetId) && (selectedFacets[facetId].intersectingPoints[p1].vertexId != searchId)) ? (int)p1 : -1;
						}
						if (foundId != -1) {
							EdgePoint p;
							p.vertexId = selectedFacets[facetId].intersectingPoints[foundId].vertexId;
							p.onEdge = -1;
							path.push_back(p);
							//Get next point which is the same
							searchId = selectedFacets[facetId].intersectingPoints[foundId].vertexId;
							searchFacetId = selectedFacets[facetId].intersectingPoints[foundId].withFacetId;
							foundId2 = -1;
							for (size_t p2 = 0; foundId2 == -1 && p2 < selectedFacets[facetId].intersectingPoints.size(); p2++) { //Search next intersection point which is same vertex
								if (p2 == foundId) continue;
								foundId2 = ((selectedFacets[facetId].intersectingPoints[p2].vertexId == searchId) && (selectedFacets[facetId].intersectingPoints[p2].withFacetId != searchFacetId)) ? (int)p2 : -1;
							}
							if (foundId2 != -1) {
								searchFacetId = selectedFacets[facetId].intersectingPoints[foundId2].withFacetId;
								searchId = foundId2;
							}
						}
					} while (foundId != -1 && foundId2 != -1);
					//No more intersection points on the middle of the facet. Need to find closing point, which is on an edge
					foundId = -1;
					for (size_t v2 = v; foundId == -1 && v2 < selectedFacets[facetId].intersectionPointId[vertexId].size(); v2++) { //Check if on same edge
						if (selectedFacets[facetId].intersectionPointId[vertexId][v2].withFacetId == searchFacetId && selectedFacets[facetId].intersectionPointId[vertexId][v2].vertexId != path.front().vertexId) foundId = (int)selectedFacets[facetId].intersectionPointId[vertexId][v2].vertexId;
					}
					if (foundId != -1) {
						EdgePoint p;
						p.vertexId = foundId;
						p.onEdge = (int)vertexId;
						path.push_back(p); //Found on same edge, close
					}
					else { //Search on other edges
						for (size_t v3 = 0; foundId == -1 && v3 < selectedFacets[facetId].f->sh.nbIndex; v3++) {
							if (v3 == vertexId) continue; //Already checked on same edge
							for (size_t v2 = v; foundId == -1 && v2 < selectedFacets[facetId].intersectionPointId[v3].size(); v2++) {
								if (selectedFacets[facetId].intersectionPointId[v3][v2].withFacetId == searchFacetId) {
									foundId = (int)selectedFacets[facetId].intersectionPointId[v3][v2].vertexId;
									EdgePoint p;
									p.vertexId = foundId;
									p.onEdge = (int)v3;
									path.push_back(p);
								}
							}
						}
					}
					if (path.size() > 1) clipPaths.push_back(path);
				}
			}
		}

		if (clipPaths.size() > 0) {
			//Construct clipped facet, having a selected vertex
			std::vector<bool> isIndexSelected(f->sh.nbIndex);
			for (size_t v = 0; v < f->sh.nbIndex; v++) {
				isIndexSelected[v] = vertices3[f->indices[v]].selected; //Make a copy, we don't want to deselect vertices
			}
			size_t nbNewfacet = 0; //Number of new facets created for this particular clipping path
			for (size_t v = 0; v < f->sh.nbIndex; v++) {
				size_t currentVertex = v;
				if (isIndexSelected[currentVertex]) {
					//Restore visited state for all clip paths
					for (size_t i = 0; i < clipPaths.size(); i++) {
						for (size_t j = 0; j < clipPaths[i].size(); j++) {
							clipPaths[i][j].visited = false;
						}
					}
					std::vector<size_t> clipPath;
					nbNewfacet++;
					do { //Build points of facet
						clipPath.push_back(f->indices[currentVertex]);
						isIndexSelected[currentVertex] = false;
						//Get closest path end
						double minDist = 9E99;
						int clipId = -1;
						bool front;

						for (size_t i = 0; i < clipPaths.size(); i++) {
							if (clipPaths[i].front().onEdge == currentVertex && clipPaths[i].back().onEdge != -1) { //If a full clipping path is found on the scanned edge, go through it
								double d = (vertices3[f->indices[currentVertex]] - vertices3[clipPaths[i].front().vertexId]).Norme();
								if (d < minDist) {
									minDist = d;
									clipId = (int)i;
									front = true;
								}
							}
							if (clipPaths[i].back().onEdge == currentVertex && clipPaths[i].front().onEdge != -1) { //If a full clipping path is found on the scanned edge, go through it
								double d = (vertices3[f->indices[currentVertex]] - vertices3[clipPaths[i].back().vertexId]).Norme();
								if (d < minDist) {
									minDist = d;
									clipId = (int)i;
									front = false;
								}
							}
						}
						if (clipId != -1) {
							if ((front && !clipPaths[clipId].front().visited) || (!front && !clipPaths[clipId].back().visited)) {
								for (int cp = front ? 0 : (int)clipPaths[clipId].size() - 1; cp >= 0 && cp < (int)clipPaths[clipId].size(); cp += front ? 1 : -1) {
									clipPath.push_back(clipPaths[clipId][cp].vertexId);
									clipPaths[clipId][cp].visited = true;
								}
								currentVertex = front ? clipPaths[clipId].back().onEdge : clipPaths[clipId].front().onEdge;
							}
						}
						currentVertex = (currentVertex + 1) % f->sh.nbIndex;
					} while (currentVertex != v);
					if (clipPath.size() > 2) {
						Facet *f = new Facet(clipPath.size());
						for (size_t i = 0; i < clipPath.size(); i++)
							f->indices[i] = clipPath[i];
						f->selected = true;

						if (nbNewfacet == 1) {//replace original
							DeletedFacet df;
							df.f = facets[selectedFacets[facetId].id];
							df.ori_pos = selectedFacets[facetId].id;
							df.replaceOri = true;
							deletedFacetList.push_back(df);
							facets[selectedFacets[facetId].id] = f; //replace original
						}
						else {//create new
							facets = (Facet**)realloc(facets, sizeof(Facet*)*(sh.nbFacet + 1));
							facets[sh.nbFacet++] = f;
							(*nbCreated)++;
						}
					}
				}
			}
		}
		/*
		//Clip facet
		std::vector<std::vector<size_t>> clippingPaths;
		for (auto& path : clipPaths) {
			std::vector<size_t> newPath;
			for (auto& point : path) {
				newPath.push_back(point.vertexId);
			}
			clippingPaths.push_back(newPath);
		}
		ClipPolygon(selectedFacets[facetId].id, clippingPaths,ClipperLib::ctIntersection);
		*/
	}

	//Rebuild facet
	/*
	f->wp.nbIndex = (int)testPath.size();
	f->indices = (int*)realloc(f->indices, sizeof(int)*testPath.size());
	f->vertices2 = (Vector2d*)realloc(f->vertices2, sizeof(Vector2d)*testPath.size());
	f->visible = (bool*)realloc(f->visible, sizeof(bool)*testPath.size());
	for (size_t i = 0; i < testPath.size(); i++)
		f->indices[i] = testPath[i];
	Rebuild();
	*/

	/*for (auto& path : clipPaths)
		for (auto& vertexId : path)
			vertices3[vertexId].selected = true;*/

	Rebuild();
	return deletedFacetList;
}

std::vector<DeletedFacet> Geometry::SplitSelectedFacets(const Vector3d &base, const Vector3d &normal, size_t *nbCreated,GLProgress *prg) {
	mApp->changedSinceSave = true;
	std::vector<DeletedFacet> deletedFacetList;
	size_t oldNbFacets = sh.nbFacet;
	for (size_t i = 0; i < oldNbFacets; i++) {
		Facet *f = facets[i];
		if (f->selected) {
			if (prg) prg->SetProgress(double(i) / double(sh.nbFacet));
			Vector3d intersectionPoint, intersectLineDir;
			if (!IntersectingPlaneWithLine(f->sh.O, f->sh.U, base, normal, &intersectionPoint))
				if (!IntersectingPlaneWithLine(f->sh.O, f->sh.V, base, normal, &intersectionPoint))
					if (!IntersectingPlaneWithLine(f->sh.O + f->sh.U, -1.0*f->sh.U, base, normal, &intersectionPoint)) //If origin on cutting plane
						if (!IntersectingPlaneWithLine(f->sh.O + f->sh.V, -1.0*f->sh.V, base, normal, &intersectionPoint)) //If origin on cutting plane
						{
							f->selected = false;
							continue;
						}
			//Reduce to a 2D problem in the facet's plane
			Vector2d intPoint2D, intDir2D, intDirOrt2D;
			intPoint2D = ProjectVertex(intersectionPoint, f->sh.U, f->sh.V, f->sh.O);
			intersectLineDir = CrossProduct(normal, f->sh.N);
			intDir2D.u = Dot(f->sh.U, intersectLineDir) / Dot(f->sh.U, f->sh.U);
			intDir2D.v = Dot(f->sh.V, intersectLineDir) / Dot(f->sh.V, f->sh.V);
			//Construct orthogonal vector to decide inside/outside
			intDirOrt2D.u = intDir2D.v;
			intDirOrt2D.v = -intDir2D.u;
			//Do the clipping. Algorithm following pseudocode from Graphic Gems V: "Clipping a Concave Polygon", Andrew S. Glassner
			std::list<ClippingVertex> clipVertices;
			//Assure that we begin on either side of the clipping line
			int currentPos;
			size_t startIndex = 0;
			do {
				double a = Dot(intDirOrt2D, f->vertices2[startIndex] - intPoint2D);
				if (a > 1E-10) {
					currentPos = 1;
				}
				else if (a < -1E-10) {
					currentPos = -1;
				}
				else {
					currentPos = 0;
				}
				startIndex++;
			} while (startIndex < f->sh.nbIndex && currentPos == 0);

			if (startIndex == f->sh.nbIndex) continue; //Whole null facet on clipping line...

			startIndex--; //First vertex not on clipping line. Underrun-safe because it's at least 1
			bool areWeInside = (currentPos == 1);

			size_t v = startIndex;
			do { //Make a circle on the facet
				ClippingVertex V;
				V.vertex = f->vertices2[v];
				V.globalId = f->indices[v];

				double a = Dot(intDirOrt2D, V.vertex - intPoint2D);
				if (a > 1E-10) {
					V.inside = areWeInside = true;
					V.onClippingLine = false;
				}
				else if (a < -1E-10) {
					V.inside = areWeInside = false;
					V.onClippingLine = false;
				}
				else {
					V.inside = areWeInside; //Previous point
					V.onClippingLine = true;
				}
				clipVertices.push_back(V);
				v = (v + 1) % f->sh.nbIndex;
			} while (v != startIndex);
			//At this point the original vertices are prepared
			std::list<std::list<ClippingVertex>::iterator> createdList; //Will contain the clipping points
			std::list<ClippingVertex>::iterator V = clipVertices.begin();
			size_t nbNewPoints = 0;
			do {
				std::list<ClippingVertex>::iterator N = std::next(V);
				if (N == clipVertices.end()) N = clipVertices.begin();
				if (V->inside != N->inside) { //side change, or leaving (not arriving to!) clipping line
					if (V->onClippingLine) {
						createdList.push_back(V); //Just mark V as clipping point, no new vertex required
					}
					else  //New vertex
					{
						//Compute location of intersection point P
						Vector2d v = N->vertex - V->vertex;
						Vector2d w = intPoint2D - V->vertex;
						double s_i = (v.v*w.u - v.u*w.v) / (v.u*intDir2D.v - v.v*intDir2D.u);
						ClippingVertex P;
						P.vertex = intPoint2D + intDir2D*s_i;
						P.globalId = sh.nbVertex + nbNewPoints;
						nbNewPoints++;
						createdList.push_back(clipVertices.insert(N, P)); //Insert P in clippingVertices between V and N
						V++; //iterate to P or end of list
					}
				}
				if (V != clipVertices.end()) V++; //iterate to N
			} while (V != clipVertices.end());
			//Register new vertices and calc distance from clipping line
			if (createdList.size() > 0) { //If there was a cut
				assert(createdList.size() % 2 == 0);
				if (nbNewPoints) vertices3.resize(sh.nbVertex + nbNewPoints);
				for (std::list<std::list<ClippingVertex>::iterator>::iterator newVertexIterator = createdList.begin(); newVertexIterator != createdList.end(); newVertexIterator++) {
					if ((*newVertexIterator)->globalId >= sh.nbVertex) {
						InterfaceVertex newCoord3D;
						newCoord3D.SetLocation(f->sh.O + f->sh.U*(*newVertexIterator)->vertex.u + f->sh.V*(*newVertexIterator)->vertex.v);
						newCoord3D.selected = false;
						vertices3[sh.nbVertex++] = newCoord3D;
					}
					Vector2d diff = (*newVertexIterator)->vertex - intPoint2D;
					(*newVertexIterator)->distance = Dot(diff, intDir2D);
				}
				createdList.sort();
				for (std::list<std::list<ClippingVertex>::iterator>::iterator pairFirst = createdList.begin(); pairFirst != createdList.end(); pairFirst++, pairFirst++) {
					std::list<std::list<ClippingVertex>::iterator>::iterator pairSecond = std::next(pairFirst);
					(*pairFirst)->isLink = (*pairSecond)->isLink = true;
					(*pairFirst)->link = *pairSecond;
					(*pairSecond)->link = *pairFirst;
				}
				std::list<ClippingVertex>::iterator U = clipVertices.begin();
				std::list<std::vector<size_t>> newFacetsIndices;
				do {
					std::vector<size_t> newPolyIndices;
					if (U->visited == false) {
						std::list<ClippingVertex>::iterator V = U;
						do {
							V->visited = true;
							newPolyIndices.push_back(V->globalId);
							if (V->isLink) {
								V = V->link;
								V->visited = true;
								newPolyIndices.push_back(V->globalId);
							}
							V++; if (V == clipVertices.end()) V = clipVertices.begin();
						} while (V != U);
					}
					U++;
					//Register new facet
					if (newPolyIndices.size() > 0) {
						newFacetsIndices.push_back(newPolyIndices);
					}
				} while (U != clipVertices.end());
				if (newFacetsIndices.size() > 0)
					facets = (Facet**)realloc(facets, sizeof(Facet*)*(sh.nbFacet + newFacetsIndices.size()));
				for (auto& newPolyIndices : newFacetsIndices) {
					assert(newPolyIndices.size() >= 3);
					Facet *newFacet = new Facet((int)newPolyIndices.size());
					(*nbCreated)++;
					for (size_t i = 0; i < newPolyIndices.size(); i++) {
						newFacet->indices[i] = newPolyIndices[i];
					}
					newFacet->CopyFacetProperties(f); //Copy physical parameters, structure, etc. - will cause problems with outgassing, though
					CalculateFacetParams(newFacet);
					/*if (f->wp.area > 0.0) {*/
					if (Dot(f->sh.N, newFacet->sh.N) < 0) {
						newFacet->SwapNormal();
					}
					newFacet->selected = true;
					facets[sh.nbFacet] = newFacet;
					sh.nbFacet++;
					/*}*/
				}
				DeletedFacet df;
				df.ori_pos = i;
				df.f = f; //Keep the pointer in memory
				df.replaceOri = false;
				deletedFacetList.push_back(df);
			} //end if there was a cut
		}
		f->selected = false;
	}

	std::vector<size_t> deletedFacetIds;
	for (auto& deletedFacet : deletedFacetList)
		deletedFacetIds.push_back(deletedFacet.ori_pos);
	RemoveFacets(deletedFacetIds, true); //We just renumber, keeping the facets in memory

	// Delete old resources
	DeleteGLLists(true, true);
	InitializeGeometry();
	return deletedFacetList;
}

Facet *Geometry::MergeFacet(Facet *f1, Facet *f2) {
	mApp->changedSinceSave = true;
	// Merge 2 facets into 1 when possible and create a new facet
	// otherwise return NULL.
	size_t  c1,c2,l;
	bool end = false;
	Facet *nF = NULL;

	if (GetCommonEdges(f1, f2, &c1, &c2, &l)) {
		size_t commonNo = f1->sh.nbIndex + f2->sh.nbIndex - 2 * l;
		if (commonNo == 0) { //two identical facets, so return a copy of f1
			nF = new Facet(f1->sh.nbIndex);
			nF->CopyFacetProperties(f1);
			for (int i = 0; i < f1->sh.nbIndex; i++)
				nF->indices[i] = f1->GetIndex(i);
			return nF;
		}

		int nbI = 0;
		nF = new Facet(commonNo);
		// Copy params from f1
		//nF->CopyFacetProperties(f1);
		nF->CopyFacetProperties(f1);

		if (l == f1->sh.nbIndex) {

			// f1 absorbed, copy indices from f2
			for (int i = 0; i < f2->sh.nbIndex - l; i++)
				nF->indices[nbI++] = f2->GetIndex(c2 + 2 + i);

		}
		else if (l == f2->sh.nbIndex) {

			// f2 absorbed, copy indices from f1
			for (int i = 0; i < f1->sh.nbIndex - l; i++)
				nF->indices[nbI++] = f1->GetIndex(c1 + l + i);

		}
		else {

			// Copy indices from f1
			for (int i = 0; i < f1->sh.nbIndex - (l - 1); i++)
				nF->indices[nbI++] = f1->GetIndex(c1 + l + i);
			// Copy indices from f2
			for (int i = 0; i < f2->sh.nbIndex - (l + 1); i++)
				nF->indices[nbI++] = f2->GetIndex(c2 + 2 + i);

		}

	}

	return nF;

}

void Geometry::Collapse(double vT, double fT, double lT, bool doSelectedOnly, Worker *work, GLProgress *prg) {
	mApp->changedSinceSave = true;
	work->abortRequested = false;
	Facet *fi, *fj;
	Facet *merged;

	double totalWork = (1.0 + (double)(fT > 0.0) + (double)(lT > 0.0)); //for progress indicator
																	  // Collapse vertex
	if (vT > 0.0) {
		CollapseVertex(work, prg, totalWork, vT);
		InitializeGeometry(); //Find collinear facets
		if (RemoveCollinear() || RemoveNullFacet()) {
			InitializeGeometry(); //If  facets were removed, update geom.
			mApp->UpdateModelParams();
		}
	}

	if (fT > 0.0 && !work->abortRequested) {

		// Collapse facets
		prg->SetMessage("Collapsing facets...");
		std::vector<int> newRef(sh.nbFacet);
		for (int i = 0; i < sh.nbFacet; i++) {
			newRef[i] = i; //Default: reference doesn't change
		}
		for (int i = 0; !work->abortRequested && i < sh.nbFacet; i++) {
			prg->SetProgress((1.0 + ((double)i / (double)sh.nbFacet)) / totalWork);
			mApp->DoEvents(); //To catch eventual abort button click
			fi = facets[i];
			// Search a coplanar facet
			int j = i + 1;
			while ((!doSelectedOnly || fi->selected) && j < sh.nbFacet) {
				fj = facets[j];
				merged = NULL;
				if ((!doSelectedOnly || fj->selected) && fi->IsCoplanarAndEqual(fj, fT)) {
					// Collapse
					merged = MergeFacet(fi, fj);

					if (merged) {
						// Replace the old 2 facets by the new one
						SAFE_DELETE(fi);
						SAFE_DELETE(fj);
						newRef[i] = newRef[j] = -1;
						for (int k = j; k < sh.nbFacet - 1; k++) {
							facets[k] = facets[k + 1];
						}
						for (int k = j + 1; k < newRef.size(); k++) {
							newRef[k] --; //Renumber references
						}
						sh.nbFacet--;
						facets[i] = merged;
						//InitializeGeometry(i);
						//SetFacetTexture(i,facets[i]->tRatio,facets[i]->hasMesh);  //rebuild mesh
						fi = facets[i];
						j = i + 1;

					}
				}
				if (!merged) j++;
			}
		}
		mApp->RenumberSelections(newRef);
		mApp->RenumberFormulas(&newRef);
		RenumberNeighbors(newRef);
	}
	//Collapse collinear sides. Takes some time, so only if threshold>0
	prg->SetMessage("Collapsing collinear sides...");
	if (lT > 0.0 && !work->abortRequested) {
		for (int i = 0; i < sh.nbFacet; i++) {
			prg->SetProgress((1.0 + (double)(fT > 0.0) + ((double)i / (double)sh.nbFacet)) / totalWork);
			if (!doSelectedOnly || facets[i]->selected)
				MergecollinearSides(facets[i], lT);
		}
	}
	prg->SetMessage("Rebuilding geometry...");
	
	/*
	for (int i = 0; i < sh.nbFacet; i++) {

		Facet *f = facets[i];

		// Revert orientation if normal has been swapped
		// This happens when the second vertex is no longer convex
		Vector3d n, v1, v2;
		double   d;
		size_t i0 = facets[i]->indices[0];
		size_t i1 = facets[i]->indices[1];
		size_t i2 = facets[i]->indices[2];

		v1 = vertices3[i1] - vertices3[i0]; // v1 = P0P1
		v2 = vertices3[i2] - vertices3[i1]; // v2 = P1P2
		n = CrossProduct(v1, v2);           // Cross product
		d = Dot(n, f->sh.N);
		if (d < 0.0) f->SwapNormal();

	}
	*/

	// Delete old resources
	for (int i = 0; i < sh.nbSuper; i++)
		DeleteGLLists(true, true);

	// Reinitialise geom
	InitializeGeometry();

}

void Geometry::RenumberNeighbors(const std::vector<int> &newRefs) {
	for (size_t i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		for (int j = 0; j < f->neighbors.size(); j++) {
			size_t oriId = f->neighbors[j].id;
			if (oriId >= newRefs.size()) { //Refers to a facet that didn't exist even before
				f->neighbors.erase(f->neighbors.begin() + j);
				j--; //Do this index again as it's now the next
			}
			else if (newRefs[oriId] == -1) { //Refers to a facet that we just deleted now
				f->neighbors.erase(f->neighbors.begin() + j);
				j--;  //Do this index again as it's now the next
			}
			else { //Update id
				f->neighbors[j].id = newRefs[oriId];
			}
		}
	}
}

void Geometry::MergecollinearSides(Facet *f, double lT) {
	mApp->changedSinceSave = true;
	bool collinear;
	double linTreshold = cos(lT*PI / 180);
	// Merge collinear sides
	for (int k = 0; (k < f->sh.nbIndex&&f->sh.nbIndex>3); k++) {
		k = k;
		do {
			//collinear=false;
			size_t p0 = f->indices[k];
			size_t p1 = f->indices[(k + 1) % f->sh.nbIndex];
			size_t p2 = f->indices[(k + 2) % f->sh.nbIndex]; //to compare last side with first too
			Vector3d p0p1 = (vertices3[p1] - vertices3[p0]).Normalized();
			Vector3d p0p2 = (vertices3[p2] - vertices3[p1]).Normalized();
			collinear = (Dot(p0p1, p0p2) >= linTreshold);
			if (collinear&&f->sh.nbIndex > 3) { //collinear
				size_t l = (k + 1) % f->sh.nbIndex;
				f->indices.erase(f->indices.begin() + l);
				f->vertices2.erase(f->vertices2.begin() + l);
				f->visible.erase(f->visible.begin() + l);
				if (l <= k) k--;
				f->sh.nbIndex--;
			}
		} while (collinear&&f->sh.nbIndex > 3);
	}
}

void Geometry::CalculateFacetParams(Facet* f) {
	// Calculate facet normal
	Vector3d p0 = vertices3[f->indices[0]];
	Vector3d v1;
	Vector3d v2;
	bool consecutive = true;
	int ind = 2;

	// TODO: Handle possible collinear consequtive vectors
	size_t i0 = f->indices[0];
	size_t i1 = f->indices[1];
	while (ind < f->sh.nbIndex && consecutive) {
		size_t i2 = f->indices[ind++];

		v1 = vertices3[i1] - vertices3[i0]; // v1 = P0P1
		v2 = vertices3[i2] - vertices3[i1]; // v2 = P1P2
		f->sh.N = CrossProduct(v1, v2);              // Cross product
		consecutive = (f->sh.N.Norme() < 1e-11);
	}
	f->collinear = consecutive; //mark for later that this facet was on a line
	f->sh.N = f->sh.N.Normalized();                  // Normalize

	// Calculate Axis Aligned Bounding Box
	f->sh.bb.min = Vector3d(1e100, 1e100, 1e100);
	f->sh.bb.max = Vector3d(-1e100, -1e100, -1e100);

	for (const auto& i : f->indices) {
		const Vector3d& p = vertices3[i];
		f->sh.bb.min.x = std::min(f->sh.bb.min.x,p.x);
		f->sh.bb.min.y = std::min(f->sh.bb.min.y, p.y);
		f->sh.bb.min.z = std::min(f->sh.bb.min.z, p.z);
		f->sh.bb.max.x = std::max(f->sh.bb.max.x, p.x);
		f->sh.bb.max.y = std::max(f->sh.bb.max.y, p.y);
		f->sh.bb.max.z = std::max(f->sh.bb.max.z, p.z);
	}

	// Facet center (AxisAlignedBoundingBox center)
	f->sh.center = 0.5 * (f->sh.bb.max + f->sh.bb.min);

	// Plane equation
	double A = f->sh.N.x;
	double B = f->sh.N.y;
	double C = f->sh.N.z;
	double D = -Dot(f->sh.N, p0);

	// Facet planarity
	f->planarityError = 0.0;
	for (size_t i = 3; i < f->sh.nbIndex;i++) { //First 3 vertices are by def on a plane
		const Vector3d& p = vertices3[f->indices[i]];
		double d = A * p.x + B * p.y + C * p.z + D;
		f->planarityError = std::max(abs(d), f->planarityError);
	}

	// Plane coef
	f->a = A;
	f->b = B;
	f->c = C;
	f->d = D;

	Vector3d p1 = vertices3[f->indices[1]];

	Vector3d U, V;

	U = (p1 - p0).Normalized(); //First side

	// Construct a normal vector V:
	V = CrossProduct(f->sh.N, U); // |U|=1 and |N|=1 => |V|=1

	// u,v vertices (we start with p0 at 0,0)
	f->vertices2[0].u = 0.0;
	f->vertices2[0].v = 0.0;
	Vector2d BBmin; BBmin.u = 0.0; BBmin.v = 0.0;
	Vector2d BBmax; BBmax.u = 0.0; BBmax.v = 0.0;

	for (size_t j = 1; j < f->sh.nbIndex; j++) {

		Vector3d p = vertices3[f->indices[j]];
		Vector3d v = p - p0;
		f->vertices2[j].u = Dot(U, v);  // Project p on U along the V direction
		f->vertices2[j].v = Dot(V, v);  // Project p on V along the U direction

		// Bounds
		BBmax.u  = std::max(BBmax.u , f->vertices2[j].u);
		BBmax.v = std::max(BBmax.v, f->vertices2[j].v);
		BBmin.u = std::min(BBmin.u, f->vertices2[j].u);
		BBmin.v = std::min(BBmin.v, f->vertices2[j].v);

	}

	// Calculate facet area (Meister/Gauss formula)
	double area = 0.0;
	for (size_t j = 0; j < f->sh.nbIndex; j++) {
		size_t j_next = Next(j,f->sh.nbIndex);
		area += f->vertices2[j].u*f->vertices2[j_next].v - f->vertices2[j_next].u*f->vertices2[j].v; //Equal to Z-component of vectorial product
	}
	if (area > 0.0) {
		//f->sign = -1;
		f->nonSimple = false;
	}
	else if (area < 0.0) {
		//f->sign = -1;
		f->nonSimple = false;
		
		//This is a case where a concave facet doesn't obey the right-hand rule:
		//it happens when the first rotation (usually around the second index) is the opposite as the general outline rotation
		
		//Do a flip
		f->sh.N = -1.0 * f->sh.N;
		f->a = -1.0 * f->a;
		f->b = -1.0 * f->b;
		f->c = -1.0 * f->c;
		f->d = -1.0 * f->d;
		V = -1.0 * V;
		BBmin.v = BBmax.v = 0.0;
		for (auto& v : f->vertices2) {
			v.v = -1.0 * v.v;
			BBmax.v = std::max(BBmax.v, v.v);
			BBmin.v = std::min(BBmin.v, v.v);
		}
		f->normalFlipped = true; //So we can revert it later
	}
	else { //Area==0.0
		//f->sign = 0;
		f->nonSimple = true;
	}

	f->sh.area = std::abs(0.5 * area);

	// Compute the 2D basis (O,U,V)
	double uD = (BBmax.u - BBmin.u);
	double vD = (BBmax.v - BBmin.v);

	// Origin
	f->sh.O = p0 + BBmin.u * U + BBmin.v * V;

	// Rescale U and V vector
	f->sh.nU = U;
	f->sh.U = U * uD;

	f->sh.nV = V;
	f->sh.V = V * vD;

	/*
	//Center might not be on the facet's plane
	Vector2d projectedCenter = ProjectVertex(f->sh.center, f->sh.U, f->sh.V, f->sh.O);
	f->sh.center = f->sh.O + projectedCenter.u*f->sh.U + projectedCenter.v*f->sh.V;
	*/

	f->sh.Nuv = CrossProduct(f->sh.U,f->sh.V); //Not normalized normal vector

	// Rescale u,v coordinates
	for (auto& p : f->vertices2) {
		p.u = (p.u - BBmin.u) / uD;
		p.v = (p.v - BBmin.v) / vD;
	}

#ifdef MOLFLOW
	f->sh.maxSpeed = 4.0 * sqrt(2.0*8.31*f->sh.temperature / 0.001 / mApp->worker.wp.gasMass);
#endif
}

void Geometry::RemoveFromStruct(int numToDel) {
	std::vector<size_t> facetsToDelete;
	for (int i = 0; i < sh.nbFacet; i++)
		if (facets[i]->sh.superIdx == numToDel) facetsToDelete.push_back(i); //Ignore universal (id==-1) facets
	RemoveFacets(facetsToDelete);
}

void Geometry::CreateLoft() {
	//Creates transition between two facets
	struct loftIndex {
		size_t index;
		bool visited;
		bool boundary; //false if in the middle of a series of indices connecting to the same index on the other facet
	};
	//Find first two selected facets
	int nbFound = 0;
	Facet *f1, *f2;
	for (size_t i = 0; nbFound < 2 && i < sh.nbFacet; i++) {
		if (facets[i]->selected) {
			if (nbFound == 0) f1 = facets[i];
			else f2 = facets[i];
			facets[i]->selected = false;
			nbFound++;
		}
	}
	if (!(nbFound == 2)) return;

	std::vector<loftIndex> closestIndices1; closestIndices1.resize(f1->sh.nbIndex); //links starting from the indices of facet 1
	std::vector<loftIndex> closestIndices2; closestIndices2.resize(f2->sh.nbIndex); //links starting from the indices of facet 2
	for (auto& closest : closestIndices1)
		closest.visited = false; //I don't trust C++ default values
	for (auto& closest : closestIndices2)
		closest.visited = false; //I don't trust C++ default values

	double u1Length = f1->sh.U.Norme();
	double v1Length = f1->sh.V.Norme();
	double u2Length = f2->sh.U.Norme();
	double v2Length = f2->sh.V.Norme();

	Vector2d center2Pos = ProjectVertex(f2->sh.center, f1->sh.U, f1->sh.V, f1->sh.O);
	Vector2d center1Pos = ProjectVertex(f1->sh.center, f1->sh.U, f1->sh.V, f1->sh.O);
	Vector2d centerOffset = center1Pos - center2Pos; //the centers will be aligned when looking for closest indices

	//We will loop through the indices on facet 1, and for each, we'll find the closest index on facet 2
	//We have to avoid cross-linking: whenever a link is established to facet 2, we have to reduce the search area on facet 2 to between the last established link and the first established link
	bool normalsAligned = Dot(f1->sh.N, f2->sh.N) > 0.0; //The loop direction on facets 1 and 2 must be the same
	size_t searchBeginIndex=normalsAligned?1:f2->sh.nbIndex-1; //Lower boundary of available region on facet 2 (used to avoid crossing)
	size_t searchEndIndex=0; //Higher boundary of available region on facet 2
	//These two limits cover the whole facet2 in the beginning

	bool needInit = true; //Whole facet2 is available until the first connection is made
	bool leftFirst = false; //Allow to search on full facet2 until only one index is linked on facet 2
	
	for (size_t i1 = 0; i1 < f1->sh.nbIndex; i1++) { //Go through indices on facet 1
		//Find closest point on other facet
		double min = 9E99;
		size_t minPos;
		for (size_t i2 = searchBeginIndex; min>8.9E99 || i2 != (normalsAligned?Next(searchEndIndex,f2->sh.nbIndex):Previous(searchEndIndex,f2->sh.nbIndex)); i2= normalsAligned ? Next(i2, f2->sh.nbIndex) : Previous(i2, f2->sh.nbIndex)) {
			//Loop through the available search area on facet2
			//The loop direction depends on if the normals are pointing the same or the opposite direction
			//Stop when the currently scanned index on facet2 would be the out of the search area (next or previous of searchEndIndex, depending on the loop direction)
			Vector2d projection = ProjectVertex(vertices3[f2->indices[i2]], f1->sh.U, f1->sh.V, f1->sh.O);
			projection = projection + centerOffset;
			double dist = Sqr(u1Length*(projection.u - f1->vertices2[i1].u)) + Sqr(v1Length*(projection.v - f1->vertices2[i1].v)); //We need the absolute distance
			if (dist < min) {
				min = dist;
				minPos = i2;
			}
		}
		//Make pair with the closest index on facet 2
		closestIndices1[i1].index = minPos; //Link from facet1 to facet2
		closestIndices2[minPos].index = i1; //Link from facet2 to facet1
		closestIndices1[i1].visited = closestIndices2[minPos].visited = true; //Pair is complete
		
		if (needInit) { //If this is the first link, set search area from next index on facet 2 to this index on facet 2 (so the whole facet 2 is still searched)
			searchBeginIndex = normalsAligned ? Next(minPos, f2->sh.nbIndex) : Previous(minPos, f2->sh.nbIndex); //Next, depending on loop direction
			searchEndIndex = minPos;
			needInit = false;
		}
		else {
			if (leftFirst || minPos != searchEndIndex) {
				//If it's not just a new link from facet1 to the first link destination on facet2
				searchBeginIndex = minPos; //Reduce search region on facet2 to begin with the last established link
				leftFirst = true;
			}
		}
	}

	//Find boundaries of regions on the first facet that are the closest to the same point on facet 2
	for (size_t i = 0; i < closestIndices1.size(); i++) {
		size_t previousId = Previous(i + closestIndices1.size() , closestIndices1.size());
		size_t nextId = Next(i , closestIndices1.size());
		closestIndices1[i].boundary = (closestIndices1[i].index != closestIndices1[nextId].index) || (closestIndices1[i].index != closestIndices1[previousId].index);
	}

	center2Pos = ProjectVertex(f2->sh.center, f2->sh.U, f2->sh.V, f2->sh.O);
	center1Pos = ProjectVertex(f1->sh.center, f2->sh.U, f2->sh.V, f2->sh.O);
	centerOffset = center2Pos - center1Pos;

	//Revisit those on f2 which aren't linked yet
	for (size_t i2 = 0; i2 < f2->sh.nbIndex; i2++) {
		//Find closest point on other facet
		if (!closestIndices2[i2].visited) {
			double min = 9E99;
			size_t minPos;

			//To avoid crossing, narrow search area on facet2 ranging...
			// ...from the previous facet2 index that is already linked
			// ...to the next facet2 index that is already linked

			//Searching for the previous facet2 index that is already linked
			size_t i2_lookup = i2;
			do {
				i2_lookup = Previous(i2_lookup, f2->sh.nbIndex);
			} while (!closestIndices2[i2_lookup].visited /*&& i2_lookup != i2*/); //Commented out, I don't see how i2_lookup could be the same as i2

			//At this point, we know that closestIndices2[i2_lookup].index is connected to i2_lookup, however we need to find the last facet1 index (in the loop direction) that is connected to the same i2_lookup index (otherwise we would still create a crossing)
			size_t i1index_1 = closestIndices2[i2_lookup].index;
			do {
				i1index_1 = (normalsAligned ? Next(i1index_1,f1->sh.nbIndex) : Previous(i1index_1, f1->sh.nbIndex));
			} while (closestIndices1[i1index_1].index == i2_lookup);

			i1index_1 = (normalsAligned ? Previous(i1index_1, f1->sh.nbIndex) : Next(i1index_1, f1->sh.nbIndex)); //step back one after loop exited

			//Now searching for the next facet2 index that is already linked
			i2_lookup = i2;
			do {
				i2_lookup = Next(i2_lookup, f2->sh.nbIndex); //next index
			} while (!closestIndices2[i2_lookup].visited /*&& i2_lookup != i2*/); //Commented out

			//At this point, we know that closestIndices2[i2_lookup].index is connected to i2_lookup, however we need to find the last facet1 index that is connected to the same i2_lookup index
			size_t i1index_2 = closestIndices2[i2_lookup].index;
			do {
				i1index_2 = (normalsAligned ? Previous(i1index_2, f1->sh.nbIndex) : Next(i1index_2, f1->sh.nbIndex));
			} while (closestIndices1[i1index_2].index == i2_lookup);

			i1index_2 = (normalsAligned ? Next(i1index_2, f1->sh.nbIndex) : Previous(i1index_2, f1->sh.nbIndex)); //step back one after loop exited

			size_t i1index_lower = std::min(i1index_1, i1index_2);
			size_t i1index_higher = std::max(i1index_1, i1index_2);

			//Look for the closest among [i1index_lower .. i1index_higher]. These can be safely connected to, without creating a crossing

			for (size_t i1 = i1index_lower; i1 <= i1index_higher; i1++) {
				Vector2d projection = ProjectVertex(vertices3[f1->indices[i1]], f2->sh.U, f2->sh.V, f2->sh.O);
				projection = projection + centerOffset;
				double dist = pow(u2Length*(projection.u - f2->vertices2[i2].u), 2.0) + pow(v2Length*(projection.v - f2->vertices2[i2].v), 2.0); //We need the absolute distance
				if (!closestIndices1[i1].boundary) dist += 1E6; //penalty -> try to connect with boundaries

				if (dist < min) {
					min = dist;
					minPos = i1;
				}
			}
			//Make pair
			closestIndices2[i2].index = minPos;
			//closestIndices1[minPos].index = i2; //All indices on facet1 already have links
			closestIndices2[i2].visited = true;
		}
	}

	//Detect boundaries on facet 2
	for (size_t i = 0; i < closestIndices2.size(); i++) {
		size_t previousId = Previous(i + closestIndices2.size() , closestIndices2.size());
		size_t nextId = Next(i , closestIndices2.size());
		closestIndices2[i].boundary = (closestIndices2[i].index != closestIndices2[nextId].index) || (closestIndices2[i].index != closestIndices2[previousId].index);
		closestIndices2[i].visited = false; //Reset this flag, will use to make sure we don't miss anything on facet2
	}

	//Links created, now we have to build the facets
	std::vector<Facet*> newFacets;
	for (size_t i1 = 0; i1 < f1->sh.nbIndex; i1++) { //Cycle through all facet1 indices to create triangles and rectangles

		for (size_t i2 = Next(closestIndices1[i1].index , f2->sh.nbIndex); closestIndices2[i2].index == i1; i2 = Next(i2, f2->sh.nbIndex)) {
			//In increasing direction, cycle through those indices on facet2 that are also linked with the actual facet1 index.
			//When two consecutive indices on facet 2 are connected with the same facet1 index, create the following triangle:
			
			Facet *newFacet = new Facet(3);
			newFacet->indices[0] = f1->indices[i1]; //actual facet1 index 
			newFacet->indices[1] = f2->indices[i2]; closestIndices2[i2].visited = true; //next facet2 index (i2 already incremented in for cycle head)
			newFacet->indices[2] = f2->indices[Previous(i2, f2->sh.nbIndex)]; closestIndices2[Previous(i2, f2->sh.nbIndex)].visited = true; //actual facet2 index
			newFacet->selected = true;
			if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
			newFacets.push_back(newFacet);
		}
		

		for (size_t i2 = Previous(closestIndices1[i1].index, f2->sh.nbIndex); closestIndices2[i2].index == i1; i2 = Previous(i2, f2->sh.nbIndex)) {
			//In decreasing direction, cycle through those indices on facet2 that are also linked with the actual facet1 index.
			//When two consecutive indices on facet 2 are connected with the same facet1 index, create the following triangle:

			Facet *newFacet = new Facet(3);
			newFacet->indices[0] = f1->indices[i1]; //actual facet1 index
			newFacet->indices[1] = f2->indices[Next(i2 , f2->sh.nbIndex)]; closestIndices2[Next(i2,f2->sh.nbIndex)].visited = true; //actual facet2 index
			newFacet->indices[2] = f2->indices[i2]; closestIndices2[i2].visited = true; //previous facet2 index (i2 already decremented in for cycle head)
			
			newFacet->selected = true;
			if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
			newFacets.push_back(newFacet);
		}

		//Trivial triangle connections created. Now we're up to create rectangles. Later, if those rectangles wouldn't be planar, we'll split them to two triangles
		bool triangle = closestIndices1[Next(i1,f1->sh.nbIndex)].index == closestIndices1[i1].index; //If the current (i1) and also the next index on facet1 are connected with the same index on facet2, then create a triangle. Else a rectangle.
		Facet *newFacet = new Facet(triangle ? 3 : 4);
		newFacet->indices[0] = f1->indices[i1]; //Actual facet1 index
		newFacet->indices[1] = f1->indices[Next(i1,f1->sh.nbIndex)]; //Next facet1 index
		
		//Find last vertex on other facet that's linked to us
		int incrementDir = (Dot(f1->sh.N, f2->sh.N) > 0) ? -1 : 1; //In some procedures we have to go in the same direction on both facets
		int increment; //not size_t, allow negative values
		for (increment = 0;
			closestIndices2[
				IDX((int)closestIndices1[Next(i1,f1->sh.nbIndex)].index + increment + incrementDir, f2->sh.nbIndex) //Cycles through the indices on facet2
			].index == 
			(Next(i1,f1->sh.nbIndex)); // is connected to the NEXT index on facet1
			increment += incrementDir);
		//Basically, look at the next connection. Fix the connection node as the next index on facet1, then step through all the indices on facet2 that are still connected to the same (the next) index on facet1

		size_t lastIndexOnFacet2ThatConnectsToTheNextOnFacet1 = IDX((int)closestIndices1[Next(i1,f1->sh.nbIndex)].index + increment, f2->sh.nbIndex);
		newFacet->indices[2] = f2->indices[lastIndexOnFacet2ThatConnectsToTheNextOnFacet1];
		closestIndices2[lastIndexOnFacet2ThatConnectsToTheNextOnFacet1].visited = true;

		if (!triangle) {
			size_t nextAfterLastConnected = IDX((int)lastIndexOnFacet2ThatConnectsToTheNextOnFacet1 + incrementDir, f2->sh.nbIndex);
			newFacet->indices[3] = f2->indices[nextAfterLastConnected];
			closestIndices2[nextAfterLastConnected].visited = true;
		}
		if (incrementDir == -1) newFacet->SwapNormal();
		CalculateFacetParams(newFacet);
		if (std::abs(newFacet->planarityError) > 1E-5) {
			//Split to two triangles
			size_t ind4[] = { newFacet->indices[0],newFacet->indices[1], newFacet->indices[2], newFacet->indices[3] };
			delete newFacet;
			newFacet = new Facet(3);
			Vector3d diff_0_2 = vertices3[ind4[0]] - vertices3[ind4[2]];
			Vector3d diff_1_3 = vertices3[ind4[1]] - vertices3[ind4[3]];
			bool connect_0_2 = diff_0_2.Norme() < diff_1_3.Norme(); //Split rectangle to two triangles along shorter side. To do: detect which split would create larger total surface and use that
			newFacet->indices[0] = ind4[0];
			newFacet->indices[1] = ind4[1];
			newFacet->indices[2] = ind4[connect_0_2 ? 2 : 3];
			newFacet->selected = true;
			newFacet->SwapNormal();
			newFacets.push_back(newFacet);

			newFacet = new Facet(3);
			newFacet->indices[0] = ind4[connect_0_2 ? 0 : 1];
			newFacet->indices[1] = ind4[2];
			newFacet->indices[2] = ind4[3];
		}
		newFacet->SwapNormal();
		newFacet->selected = true;
		if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
		newFacets.push_back(newFacet);
	}
	//Go through leftover vertices on facet 2
	for (size_t i2 = 0; i2 < f2->sh.nbIndex; i2++) {
		if (closestIndices2[i2].visited == false) {
			size_t targetIndex = closestIndices2[Previous(i2, f2->sh.nbIndex)].index; //Previous node

			do {
				//Connect with previous
				Facet *newFacet = new Facet(3);
				newFacet->indices[0] = f1->indices[targetIndex];
				newFacet->indices[1] = f2->indices[i2]; closestIndices2[i2].visited = true;
				newFacet->indices[2] = f2->indices[Previous(i2, f2->sh.nbIndex)]; closestIndices2[Previous(i2, f2->sh.nbIndex)].visited = true;
				newFacet->selected = true;
				if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
				newFacets.push_back(newFacet);
				i2 = Next(i2, f2->sh.nbIndex);
			} while (closestIndices2[i2].visited == false);
			//last
				//Connect with next for the last unvisited
			Facet *newFacet = new Facet(3);
			newFacet->indices[0] = f1->indices[targetIndex];
			newFacet->indices[1] = f2->indices[i2]; closestIndices2[i2].visited = true;
			newFacet->indices[2] = f2->indices[Previous(i2, f2->sh.nbIndex)]; closestIndices2[Previous(i2, f2->sh.nbIndex)].visited = true;
			newFacet->selected = true;
			if (viewStruct != -1) newFacet->sh.superIdx = viewStruct;
			newFacets.push_back(newFacet);
		}
	}
	

	//Register new facets
	if (newFacets.size() > 0) facets = (Facet**)realloc(facets, sizeof(Facet*)*(sh.nbFacet + newFacets.size()));
	for (size_t i = 0; i < newFacets.size(); i++)
		facets[sh.nbFacet + i] = newFacets[i];
	sh.nbFacet += newFacets.size();
	InitializeGeometry();
}

void Geometry::SetAutoNorme(bool enable) {
	autoNorme = enable;
}

bool Geometry::GetAutoNorme() {
	return autoNorme;
}

void Geometry::SetCenterNorme(bool enable) {
	centerNorme = enable;
}

bool Geometry::GetCenterNorme() {
	return centerNorme;
}

void Geometry::SetNormeRatio(float r) {
	normeRatio = r;
}

float Geometry::GetNormeRatio() {
	return normeRatio;
}

void Geometry::Rebuild() {

	// Rebuild internal structure on geometry change

	// Remove texture (improvement TODO)
	/*for (int i = 0; i < wp.nbFacet; i++)
		if (facets[i]->wp.isTextured)
			facets[i]->SetTexture(0.0, 0.0, false);*/

			// Delete old resources
	DeleteGLLists(true, true);

	// Reinitialise geom
	InitializeGeometry();

}



void Geometry::SetFacetTexture(size_t facetId, double ratio, bool mesh) {

	Facet *f = facets[facetId];
	double nU = f->sh.U.Norme();
	double nV = f->sh.V.Norme();

	if (!f->SetTexture(nU*ratio, nV*ratio, mesh)) {
		char errMsg[512];
		sprintf(errMsg, "Not enough memory to build mesh on Facet %zd. ", facetId + 1);
		throw Error(errMsg);
	}
	f->tRatio = ratio;
	BuildFacetList(f);

}

// File handling

void Geometry::UpdateName(FileReader *file) {
	UpdateName(file->GetName());
}

std::string Geometry::GetName() {
	return sh.name;
}


void Geometry::UpdateName(const char *fileName) {
	sh.name = FileUtils::GetFilename(fileName);
}

void Geometry::AdjustProfile() {

	// Backward compatibily with TXT profile (To be improved)
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->sh.profileType == PROFILE_U) {
			Vector3d v0 = vertices3[f->indices[1]] - vertices3[f->indices[0]];
			double n0 = v0.Norme();
			double nU = f->sh.U.Norme();
			if (IsZero(n0 - nU)) f->sh.profileType = PROFILE_U; // Select U
			else               f->sh.profileType = PROFILE_V; // Select V
		}
	}

}

void Geometry::ResetTextureLimits() {
#ifdef MOLFLOW
	texture_limits[0].autoscale.min.all = texture_limits[0].autoscale.min.moments_only =
		texture_limits[1].autoscale.min.all = texture_limits[1].autoscale.min.moments_only =
		texture_limits[2].autoscale.min.all = texture_limits[2].autoscale.min.moments_only =
		texture_limits[0].manual.min.all = texture_limits[0].manual.min.moments_only =
		texture_limits[1].manual.min.all = texture_limits[1].manual.min.moments_only =
		texture_limits[2].manual.min.all = texture_limits[2].manual.min.moments_only = 0.0;
	texture_limits[0].autoscale.max.all = texture_limits[0].autoscale.max.moments_only =
		texture_limits[1].autoscale.max.all = texture_limits[1].autoscale.max.moments_only =
		texture_limits[2].autoscale.max.all = texture_limits[2].autoscale.max.moments_only =
		texture_limits[0].manual.max.all = texture_limits[0].manual.max.moments_only =
		texture_limits[1].manual.max.all = texture_limits[1].manual.max.moments_only =
		texture_limits[2].manual.max.all = texture_limits[2].manual.max.moments_only = 1.0;
#endif
#ifdef SYNRAD
	textureMin_auto.count = 0;
	textureMin_auto.flux = 0.0;
	textureMin_auto.power = 0.0;
	textureMin_manual.count = 0;
	textureMin_manual.flux = 0.0;
	textureMin_manual.power = 0.0;
	textureMax_auto.count = 0;
	textureMax_auto.flux = 0.0;
	textureMax_auto.power = 0.0;
	textureMax_manual.count = 0;
	textureMax_manual.flux = 0.0;
	textureMax_manual.power = 0.0;
#endif
}

void Geometry::LoadASE(FileReader *file, GLProgress *prg) {

	Clear();

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	ASELoader ase(file);
	ase.Load();

	// Compute total of facet
	sh.nbFacet = 0;
	for (int i = 0; i < ase.nbObj; i++) sh.nbFacet += ase.OBJ[i].nb_face;

	// Allocate mem
	sh.nbVertex = 3 * sh.nbFacet;
	facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));
	/*vertices3 = (InterfaceVertex *)malloc(wp.nbVertex * sizeof(InterfaceVertex));
	memset(vertices3, 0, wp.nbVertex * sizeof(InterfaceVertex));*/
	std::vector<InterfaceVertex>(sh.nbVertex).swap(vertices3);

	// Fill 
	int nb = 0;
	for (int i = 0; i < ase.nbObj; i++) {

		for (int j = 0; j < ase.OBJ[i].nb_face; j++) {
			vertices3[3 * nb + 0].SetLocation(ase.OBJ[i].pts[ase.OBJ[i].face[j].v1]);
			vertices3[3 * nb + 1].SetLocation(ase.OBJ[i].pts[ase.OBJ[i].face[j].v2]);
			vertices3[3 * nb + 2].SetLocation(ase.OBJ[i].pts[ase.OBJ[i].face[j].v3]);
			facets[nb] = new Facet(3);
			facets[nb]->indices[0] = 3 * nb + 0;
			facets[nb]->indices[1] = 3 * nb + 1;
			facets[nb]->indices[2] = 3 * nb + 2;
			nb++;
		}

	}

	UpdateName(file);
	sh.nbSuper = 1;
	strName[0] = strdup(sh.name.c_str());
	strFileName[0] = strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadSTR(FileReader *file, GLProgress *prg) {

	char nPath[512];
	char fPath[512];
	char fName[1028];
	char sName[512];
	/*size_t nF, nV;
	Facet **F;
	InterfaceVertex *V;*/
	FileReader *fr;

	Clear();

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	// Load multiple structure file
	sh.nbSuper = file->ReadInt();

	strcpy(fPath, file->ReadLine());
	strcpy(nPath, FileUtils::GetPath(file->GetName()).c_str());

	for (int n = 0; n < sh.nbSuper; n++) {

		int i1 = file->ReadInt();
		int i2 = file->ReadInt();
		fr = NULL;
		strcpy(sName, file->ReadWord());
		strName[n] = strdup(sName);
		char *e = strrchr(strName[n], '.');
		if (e) *e = 0;

		sprintf(fName, "%s%s", nPath, sName);
		if (FileUtils::Exist(fName)) {
			fr = new FileReader(fName);
			strcpy(strPath, nPath);

		}
		else {

			sprintf(fName, "%s\\%s", fPath, sName);
			if (FileUtils::Exist(fName)) {
				fr = new FileReader(fName);
				strcpy(strPath, fPath);
			}
		}

		if (!fr) {
			char errMsg[560];
			sprintf(errMsg, "Cannot find %s", sName);
			throw Error(errMsg);
		}

		strFileName[n] = strdup(sName);
		//LoadTXTGeom(fr, n);
		InsertTXTGeom(fr, n, true);
		/*
		Merge(nV, nF, V, F);
		SAFE_FREE(V);
		SAFE_FREE(F);
		*/
		delete fr;

	}

	UpdateName(file);
	InitializeGeometry();
	AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadSTL(FileReader *file, GLProgress *prg, double scaleFactor) {
	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	char *w;

	prg->SetMessage("Clearing current geometry...");
	Clear();

	// First pass
	prg->SetMessage("Counting facets in STL file...");
	//file->ReadKeyword("solid");
	file->ReadLine(); // solid name
	w = file->ReadWord();
	while (strcmp(w, "facet") == 0) {
		sh.nbFacet++;
		file->JumpSection("endfacet");
		w = file->ReadWord();
	}
	if (strcmp(w, "endsolid") != 0) throw Error("Unexpected or not supported STL keyword, 'endsolid' required\nMaybe the STL file was saved in binary instead of ASCII format?");

	// Allocate mem
	sh.nbVertex = 3 * sh.nbFacet;
	facets = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	if (!facets) throw Error("Out of memory: LoadSTL");
	memset(facets, 0, sh.nbFacet * sizeof(Facet *));

	std::vector<InterfaceVertex>(sh.nbVertex).swap(vertices3);

	// Second pass
	prg->SetMessage("Reading facets...");
	file->SeekStart();
	//file->ReadKeyword("solid");
	file->ReadLine();
	for (int i = 0; i < sh.nbFacet; i++) {

		double p = (double)i / (double)(sh.nbFacet);
		prg->SetProgress(p);

		file->ReadKeyword("facet");
		file->ReadKeyword("normal");
		file->ReadDouble();
		file->ReadDouble();
		file->ReadDouble();
		file->ReadKeyword("outer");
		file->ReadKeyword("loop");

		file->ReadKeyword("vertex");
		vertices3[3 * i + 0].x = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 0].y = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 0].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		vertices3[3 * i + 1].x = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 1].y = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 1].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		vertices3[3 * i + 2].x = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 2].y = file->ReadDouble()*scaleFactor;
		vertices3[3 * i + 2].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("endloop");
		file->ReadKeyword("endfacet");

		try {
			facets[i] = new Facet(3);
		}
		catch (...) {
			throw Error("Out of memory");
		}
		facets[i]->indices[0] = 3 * i + 0;
		facets[i]->indices[1] = 3 * i + 2;
		facets[i]->indices[2] = 3 * i + 1;

	}

	sh.nbSuper = 1;
	UpdateName(file);
	strName[0] = strdup(sh.name.c_str());
	strFileName[0] = strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	prg->SetMessage("Initializing geometry...");
	InitializeGeometry();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadTXT(FileReader *file, GLProgress *prg, Worker* worker) {

	//mApp->ClearAllSelections();
	//mApp->ClearAllViews();
	Clear();
	LoadTXTGeom(file, worker);
	UpdateName(file);
	sh.nbSuper = 1;
	strName[0] = strdup(sh.name.c_str());
	strFileName[0] = strdup(file->GetName());

	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::InsertTXT(FileReader *file, GLProgress *prg, bool newStr) {

	//Clear();
	int structId = viewStruct;
	if (structId == -1) structId = 0;
	InsertTXTGeom(file, structId, newStr);
	//UpdateName(file);
	//wp.nbSuper = 1;
	//strName[0] = strdup(wp.name);
	//strFileName[0] = strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::InsertSTL(FileReader *file, GLProgress *prg, double scaleFactor, bool newStr) {

	//Clear();
	int structId = viewStruct;
	if (structId == -1) structId = 0;
	InsertSTLGeom(file, structId, scaleFactor, newStr);
	//UpdateName(file);
	//wp.nbSuper = 1;
	//strName[0] = strdup(wp.name);
	//strFileName[0] = strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::InsertGEO(FileReader *file, GLProgress *prg, bool newStr) {

	//Clear();
	int structId = viewStruct;
	if (structId == -1) structId = 0;
	InsertGEOGeom(file, structId, newStr);
	//UpdateName(file);
	//wp.nbSuper = 1;
	//strName[0] = strdup(wp.name);
	//strFileName[0] = strdup(file->GetName());
	char *e = strrchr(strName[0], '.');
	if (e) *e = 0;
	InitializeGeometry();
	//AdjustProfile();
	//isLoaded = true; //InitializeGeometry() sets to true

}

void Geometry::LoadTXTGeom(FileReader *file, Worker* worker, size_t strIdx) {

	file->ReadInt(); // Unused
	worker->globalHitCache.globalHits.nbMCHit = file->ReadSizeT();
	worker->globalHitCache.globalHits.nbHitEquiv = (double)worker->globalHitCache.globalHits.nbMCHit; //Backward comp
	worker->globalHitCache.nbLeakTotal = file->ReadSizeT();
	worker->globalHitCache.globalHits.nbDesorbed = file->ReadSizeT();
	worker->ontheflyParams.desorptionLimit = file->ReadSizeT();

	sh.nbVertex = file->ReadInt();
	sh.nbFacet = file->ReadInt();

	// Allocate memory
	Facet   **f = (Facet **)malloc(sh.nbFacet * sizeof(Facet *));
	memset(f, 0, sh.nbFacet * sizeof(Facet *));
	
	std::vector<InterfaceVertex>(sh.nbVertex).swap(vertices3);
	/*
	InterfaceVertex *v = (InterfaceVertex *)malloc(nV * sizeof(InterfaceVertex));
	memset(v, 0, nV * sizeof(InterfaceVertex)); //avoid selected flag
	*/

	// Read geometry vertices
	for (int i = 0; i < sh.nbVertex; i++) {
		vertices3[i].x = file->ReadDouble();
		vertices3[i].y = file->ReadDouble();
		vertices3[i].z = file->ReadDouble();
	}

	// Read geometry facets (indexed from 1)
	for (int i = 0; i < sh.nbFacet; i++) {
		int nb = file->ReadInt();
		f[i] = new Facet(nb);
		for (int j = 0; j < nb; j++) {
			f[i]->indices[j] = file->ReadInt() - 1;
			if (f[i]->indices[j] >= sh.nbVertex) {
				std::string errMsg = "Facet " + std::to_string(i + 1) + " refers to non-existent vertex (" + std::to_string(f[i]->indices[j]+1)  + ")";
				throw Error(errMsg.c_str());
			}
		}
	}

	// Read facets params
	for (int i = 0; i < sh.nbFacet; i++) {
		f[i]->LoadTXT(file);
		while ((f[i]->sh.superDest) > sh.nbSuper) { //If facet refers to a structure that doesn't exist, create it
			AddStruct("TXT linked",true);
		}
		f[i]->sh.superIdx = static_cast<int>(strIdx);
	}

	
	facets = f;

}

void Geometry::InsertTXTGeom(FileReader *file, size_t strIdx, bool newStruct) {

	UnselectAll();

	//loaded_nbMCHit = file->ReadSizeT();
	//loaded_nbLeak = file->ReadInt();
	//loaded_nbDesorption = file->ReadSizeT();
	//loaded_desorptionLimit = file->ReadSizeT(); 
	for (int i = 0; i < 5; i++) file->ReadInt(); //leading lines

	int nbNewVertex = file->ReadInt();
	int nbNewFacets = file->ReadInt();

	// Allocate memory
	facets = (Facet **)realloc(facets, (nbNewFacets + sh.nbFacet) * sizeof(Facet **));
	memset(facets + sh.nbFacet, 0, nbNewFacets * sizeof(Facet *));
	//vertices3 = (Vector3d*)realloc(vertices3,(nbNewVertex+wp.nbVertex) * sizeof(Vector3d));
	
	/*
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + wp.nbVertex) * sizeof(InterfaceVertex));
	memmove(tmp_vertices3, vertices3, (wp.nbVertex) * sizeof(InterfaceVertex));
	memset(tmp_vertices3 + wp.nbVertex, 0, nbNewVertex * sizeof(InterfaceVertex));
	SAFE_FREE(vertices3);
	vertices3 = tmp_vertices3;
	*/
	vertices3.resize(sh.nbVertex + nbNewVertex);

	// Read geometry vertices
	for (size_t i = sh.nbVertex; i < (sh.nbVertex + nbNewVertex); i++) {
		vertices3[i].x = file->ReadDouble();
		vertices3[i].y = file->ReadDouble();
		vertices3[i].z = file->ReadDouble();
		vertices3[i].selected = false;
	}

	// Read geometry facets (indexed from 1)
	for (size_t i = sh.nbFacet; i < (sh.nbFacet + nbNewFacets); i++) {
		size_t nb = file->ReadSizeT();
		*(facets + i) = new Facet(nb);
		(facets)[i]->selected = true;
		for (size_t j = 0; j < nb; j++)
			(facets)[i]->indices[j] = file->ReadSizeT() - 1 + sh.nbVertex;
	}

	// Read facets params
	for (size_t i = sh.nbFacet; i < (sh.nbFacet + nbNewFacets); i++) {
		(facets)[i]->LoadTXT(file);
		while (((facets)[i]->sh.superDest) > sh.nbSuper) { //If facet refers to a structure that doesn't exist, create it
			AddStruct("TXT linked");
		}
		if (newStruct) {
			(facets)[i]->sh.superIdx = static_cast<int>(sh.nbSuper);
		}
		else {

			(facets)[i]->sh.superIdx = static_cast<int>(strIdx);
		}
	}

	sh.nbVertex += nbNewVertex;
	sh.nbFacet += nbNewFacets;
	if (newStruct) AddStruct("Inserted TXT file");

}

void Geometry::InsertGEOGeom(FileReader *file, size_t strIdx, bool newStruct) {

	UnselectAll();

	file->ReadKeyword("version"); file->ReadKeyword(":");
	int version2;
	version2 = file->ReadInt();
	if (version2 > GEOVERSION) {
		char errMsg[512];
		sprintf(errMsg, "Unsupported GEO version V%d", version2);
		throw Error(errMsg);
	}

	file->ReadKeyword("totalHit"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("totalDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("totalLeak"); file->ReadKeyword(":");
	file->ReadSizeT();
	if (version2 >= 12) {
		file->ReadKeyword("totalAbs"); file->ReadKeyword(":");
		file->ReadSizeT();
		if (version2 >= 15) {
			file->ReadKeyword("totalDist_total");
		}
		else { //between versions 12 and 15
			file->ReadKeyword("totalDist");
		}
		file->ReadKeyword(":");
		file->ReadDouble();
		if (version2 >= 15) {
			file->ReadKeyword("totalDist_fullHitsOnly"); file->ReadKeyword(":");
			file->ReadDouble();
		}
	}
	file->ReadKeyword("maxDes"); file->ReadKeyword(":");
	file->ReadSizeT();
	file->ReadKeyword("nbVertex"); file->ReadKeyword(":");
	int nbNewVertex = file->ReadInt();
	file->ReadKeyword("nbFacet"); file->ReadKeyword(":");
	int nbNewFacets = file->ReadInt();
	file->ReadKeyword("nbSuper"); file->ReadKeyword(":");
	int nbNewSuper = file->ReadInt();
	int nbF = 0; std::vector<std::vector<string>> loadFormulas;
	int nbV = 0;
	if (version2 >= 2) {
		file->ReadKeyword("nbFormula"); file->ReadKeyword(":");
		nbF = file->ReadInt();
		file->ReadKeyword("nbView"); file->ReadKeyword(":");
		nbV = file->ReadInt();
	}
	int nbS = 0;
	if (version2 >= 8) {
		file->ReadKeyword("nbSelection"); file->ReadKeyword(":");
		nbS = file->ReadInt();
	}
	if (version2 >= 7) {
		file->ReadKeyword("gasMass"); file->ReadKeyword(":");
		/*wp.gasMass = */file->ReadDouble();
	}
	if (version2 >= 10) { //time-dependent version
		file->ReadKeyword("userMoments"); file->ReadKeyword("{");
		file->ReadKeyword("nb"); file->ReadKeyword(":");
		int nb = file->ReadInt();

		for (int i = 0; i < nb; i++)
			file->ReadString();
		file->ReadKeyword("}");
	}
	if (version2 >= 11) { //gas pulse parameters
		file->ReadKeyword("desorptionStart"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("desorptionStop"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("timeWindow"); file->ReadKeyword(":");
		file->ReadDouble();
		file->ReadKeyword("useMaxwellian"); file->ReadKeyword(":");
		file->ReadInt();
	}

	if (version2 >= 12) { //2013.aug.22
		file->ReadKeyword("calcConstantFlow"); file->ReadKeyword(":");
		file->ReadInt();
	}

	if (version2 >= 2) {
		file->ReadKeyword("formulas"); file->ReadKeyword("{");
		for (int i = 0; i < nbF; i++) {
			char tmpName[256];
			char tmpExpr[512];
			strcpy(tmpName, file->ReadString());
			strcpy(tmpExpr, file->ReadString());
			//mApp->OffsetFormula(tmpExpr, wp.nbFacet);
			//mApp->AddFormula(tmpName, tmpExpr); //parse after selection groups are loaded
#ifdef MOLFLOW
			std::vector<string> newFormula;
			newFormula.push_back(tmpName);
			mApp->OffsetFormula(tmpExpr, (int)sh.nbFacet); //offset formula
			newFormula.push_back(tmpExpr);
			loadFormulas.push_back(newFormula);
#endif
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

	if (version2 >= 8) {
		file->ReadKeyword("selections"); file->ReadKeyword("{");
		for (int i = 0; i < nbS; i++) {
			SelectionGroup s;
			char tmpName[256];
			strcpy(tmpName, file->ReadString());
			s.name = strdup(tmpName);
			int nbFS = file->ReadInt();

			for (int j = 0; j < nbFS; j++) {
				s.selection.push_back((size_t)file->ReadInt() + sh.nbFacet); //offset facet number by current number of facets
			}
			mApp->AddSelection(s);
		}
		file->ReadKeyword("}");
	}
#ifdef MOLFLOW
	for (int i = 0; i < nbF; i++) { //parse formulas now that selection groups are loaded
		mApp->AddFormula(loadFormulas[i][0].c_str(), loadFormulas[i][1].c_str());
	}
#endif
	file->ReadKeyword("structures"); file->ReadKeyword("{");
	for (int i = 0; i < nbNewSuper; i++) {
		strName[sh.nbSuper + i] = strdup(file->ReadString());
		// For backward compatibilty with STR
		/* //Commented out for GEO
		sprintf(tmp, "%s.txt", strName[i]);
		strFileName[i] = strdup(tmp);
		*/
	}
	file->ReadKeyword("}");

	// Reallocate memory
	facets = (Facet **)realloc(facets, (nbNewFacets + sh.nbFacet) * sizeof(Facet **));
	memset(*facets + sh.nbFacet, 0, nbNewFacets * sizeof(Facet *));
	/*
	//vertices3 = (Vector3d*)realloc(vertices3,(nbNewVertex+*nbVertex) * sizeof(Vector3d));
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + *nbVertex) * sizeof(InterfaceVertex));
	if (!tmp_vertices3) throw Error("Out of memory: InsertGEOGeom");
	memmove(tmp_vertices3, vertices3, (*nbVertex) * sizeof(InterfaceVertex));
	memset(tmp_vertices3 + *nbVertex, 0, nbNewVertex * sizeof(InterfaceVertex));
	SAFE_FREE(vertices3);
	vertices3 = tmp_vertices3;
	*/
	vertices3.resize(sh.nbVertex + nbNewVertex);

	// Read geometry vertices
	file->ReadKeyword("vertices"); file->ReadKeyword("{");
	for (size_t i = sh.nbVertex; i < (sh.nbVertex + nbNewVertex); i++) {
		// Check idx
		size_t idx = file->ReadSizeT();
		if (idx != i - sh.nbVertex + 1) throw Error(file->MakeError("Wrong vertex index !"));
		vertices3[i].x = file->ReadDouble();
		vertices3[i].y = file->ReadDouble();
		vertices3[i].z = file->ReadDouble();
		vertices3[i].selected = false;
	}
	file->ReadKeyword("}");

	if (version2 >= 6) {
		// Read leaks
		file->ReadKeyword("leaks"); file->ReadKeyword("{");
		file->ReadKeyword("nbLeak"); file->ReadKeyword(":");
		int nbleak2 = file->ReadInt();
		for (int i = 0; i < nbleak2; i++) {
			int idx = file->ReadInt();
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
		int nbHHit2 = file->ReadInt();
		for (int i = 0; i < nbHHit2; i++) {
			int idx = file->ReadInt();
			//if( idx != i ) throw Error(file->MakeError("Wrong hit cache index !"));
			file->ReadDouble();
			file->ReadDouble();
			file->ReadDouble();

			file->ReadInt();
		}
		file->ReadKeyword("}");
	}

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
		facets[i]->LoadGEO(file, version2, nbNewVertex);
		facets[i]->selected = true;
		for (int j = 0; j < nb; j++)
			facets[i]->indices[j] += sh.nbVertex;
		file->ReadKeyword("}");
		if (newStruct) {
			facets[i]->sh.superIdx += static_cast<int>(sh.nbSuper);
			if (facets[i]->sh.superDest > 0) facets[i]->sh.superDest += sh.nbSuper;
		}
		else {

			facets[i]->sh.superIdx += static_cast<int>(strIdx);
			if (facets[i]->sh.superDest > 0) facets[i]->sh.superDest += strIdx;
		}
	}

	sh.nbVertex += nbNewVertex;
	sh.nbFacet += nbNewFacets;
	if (newStruct) sh.nbSuper += nbNewSuper;
	else if (sh.nbSuper < strIdx + nbNewSuper) sh.nbSuper = strIdx + nbNewSuper;

}

void Geometry::InsertSTLGeom(FileReader *file, size_t strIdx, double scaleFactor, bool newStruct) {

	UnselectAll();
	char *w;

	int nbNewFacets = 0;
	// First pass
	file->ReadKeyword("solid");
	file->ReadLine(); // solid name
	w = file->ReadWord();
	while (strcmp(w, "facet") == 0) {
		nbNewFacets++;
		file->JumpSection("endfacet");
		w = file->ReadWord();
	}
	if (strcmp(w, "endsolid") != 0) throw Error("Unexpected or not supported STL keyword, 'endsolid' required");

	// Allocate memory
	int nbNewVertex = 3 * nbNewFacets;
	facets = (Facet **)realloc(facets, (nbNewFacets + sh.nbFacet) * sizeof(Facet **));
	memset(facets + sh.nbFacet, 0, nbNewFacets * sizeof(Facet *));
	
	/*
	//vertices3 = (Vector3d*)realloc(vertices3,(nbNewVertex+wp.nbVertex) * sizeof(Vector3d));
	InterfaceVertex *tmp_vertices3 = (InterfaceVertex *)malloc((nbNewVertex + wp.nbVertex) * sizeof(InterfaceVertex));
	memmove(tmp_vertices3, vertices3, (wp.nbVertex) * sizeof(InterfaceVertex));
	memset(tmp_vertices3 + wp.nbVertex, 0, nbNewVertex * sizeof(InterfaceVertex));
	SAFE_FREEvertices3;
	vertices3 = tmp_vertices3;
	*/
	vertices3.resize(nbNewVertex + sh.nbVertex);

	// Second pass
	file->SeekStart();
	file->ReadKeyword("solid");
	file->ReadLine();
	for (int i = 0; i < nbNewFacets; i++) {

		file->ReadKeyword("facet");
		file->ReadKeyword("normal");
		file->ReadDouble(); //ignoring normal vector, will be calculated from triangle orientation
		file->ReadDouble();
		file->ReadDouble();
		file->ReadKeyword("outer");
		file->ReadKeyword("loop");

		file->ReadKeyword("vertex");
		vertices3[sh.nbVertex + 3 * i + 0].x = file->ReadDouble()*scaleFactor;
		vertices3[sh.nbVertex + 3 * i + 0].y = file->ReadDouble()*scaleFactor;
		vertices3[sh.nbVertex + 3 * i + 0].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		vertices3[sh.nbVertex + 3 * i + 1].x = file->ReadDouble()*scaleFactor;
		vertices3[sh.nbVertex + 3 * i + 1].y = file->ReadDouble()*scaleFactor;
		vertices3[sh.nbVertex + 3 * i + 1].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("vertex");
		vertices3[sh.nbVertex + 3 * i + 2].x = file->ReadDouble()*scaleFactor;
		vertices3[sh.nbVertex + 3 * i + 2].y = file->ReadDouble()*scaleFactor;
		vertices3[sh.nbVertex + 3 * i + 2].z = file->ReadDouble()*scaleFactor;

		file->ReadKeyword("endloop");
		file->ReadKeyword("endfacet");

		*(facets + i + sh.nbFacet) = new Facet(3);
		facets[i + sh.nbFacet]->selected = true;
		facets[i + sh.nbFacet]->indices[0] = sh.nbVertex + 3 * i + 0;
		facets[i + sh.nbFacet]->indices[1] = sh.nbVertex + 3 * i + 1;
		facets[i + sh.nbFacet]->indices[2] = sh.nbVertex + 3 * i + 2;

		if (newStruct) {
			facets[i + sh.nbFacet]->sh.superIdx = static_cast<int>(sh.nbSuper);
		}
		else {
			facets[i + sh.nbFacet]->sh.superIdx = static_cast<int>(strIdx);
		}
	}

	sh.nbVertex += nbNewVertex;
	sh.nbFacet += nbNewFacets;
	if (newStruct) AddStruct("Inserted STL file");
}

void Geometry::SaveSTR(/*Dataport *dpHit*/ GlobalSimuState& results, bool saveSelected) {

	if (!IsLoaded()) throw Error("Nothing to save !");
	if (sh.nbSuper < 1) throw Error("Cannot save single structure in STR format");

	// Block dpHit during the whole disc writting

	for (int i = 0; i < sh.nbSuper; i++)
		SaveSuper(i);

}

void Geometry::SaveSuper(int s) {

	char fName[512+128+1];
	sprintf(fName, "%s/%s", strPath, strFileName[s]);
	FileWriter *file = new FileWriter(fName);

	// Unused
	file->Write(0, "\n");

	//Extract data of the specified super structure
	size_t totHit = 0;
	size_t totAbs = 0;
	size_t totDes = 0;
	int *refIdx = (int *)malloc(sh.nbVertex * sizeof(int));
	memset(refIdx, 0xFF, sh.nbVertex * sizeof(int));
	int nbV = 0;
	int nbF = 0;

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		if (f->sh.superIdx == s) {
			for (int j = 0; j < f->sh.nbIndex; j++)
				refIdx[f->indices[j]] = 1;
			nbF++;
		}
	}

	for (int i = 0; i < sh.nbVertex; i++) {
		if (refIdx[i] >= 0) {
			refIdx[i] = nbV;
			nbV++;
		}
	}

	file->Write(0, "\n");
	file->Write(0, "\n");
	file->Write(0, "\n");
	file->Write(0, "\n");

	file->Write(nbV, "\n");
	file->Write(nbF, "\n");

	// Read geometry vertices
	for (int i = 0; i < sh.nbVertex; i++) {
		if (refIdx[i] >= 0) {
			file->Write(vertices3[i].x, " ");
			file->Write(vertices3[i].y, " ");
			file->Write(vertices3[i].z, "\n");
		}
	}

	// Facets
	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		int j;
		if (f->sh.superIdx == s) {
			file->Write(f->sh.nbIndex, " ");
			for (j = 0; j < f->sh.nbIndex - 1; j++)
				file->Write(refIdx[f->indices[j]] + 1, " ");
			file->Write(refIdx[f->indices[j]] + 1, "\n");
		}
	}

	// Params
	for (int i = 0; i < sh.nbFacet; i++) {

		// Update facet hits from shared mem
		Facet *f = facets[i];
		if (f->sh.superIdx == s) {
			f->SaveTXT(file);
		}

	}

	SaveProfileTXT(file);

	SAFE_DELETE(file);
	free(refIdx);

}

void Geometry::SaveProfileTXT(FileWriter *file) {
	// Profiles
	for (int j = 0; j < PROFILE_SIZE; j++)
		file->Write("\n");
}

bool Geometry::IsLoaded() {
	return isLoaded;
}

void Geometry::CreateRectangle(const Vector3d& center, const Vector3d& axis1Dir, const Vector3d& normalDir, const double& axis1Length, const double& axis2Length) {
	
	std::vector<size_t> vertexIds;
	Vector3d axis1half = axis1Dir.Normalized()*0.5*axis1Length;
	Vector3d axis2half = CrossProduct(normalDir.Normalized()*(axis2Length/axis1Length), axis1half); //left-hand coordinate system, V = N x U

	AddVertex(center - axis1half - axis2half, false);
	vertexIds.push_back(sh.nbVertex - 1);
	AddVertex(center + axis1half - axis2half, false);
	vertexIds.push_back(sh.nbVertex - 1);
	AddVertex(center + axis1half + axis2half, false);
	vertexIds.push_back(sh.nbVertex - 1);
	AddVertex(center - axis1half + axis2half, false);
	vertexIds.push_back(sh.nbVertex - 1);

	AddFacet(vertexIds);
}

void Geometry::CreateCircle(const Vector3d & center, const Vector3d & axis1Dir, const Vector3d & normalDir, const double & axis1Length, const double & axis2Length, const size_t & nbSteps)
{	
	std::vector<size_t> vertexIds;
	Vector3d axis1half = axis1Dir.Normalized()*0.5*axis1Length;
	Vector3d axis2half = CrossProduct(normalDir.Normalized()*(axis2Length / axis1Length), axis1half); //left-hand coordinate system, V = N x U
	for (double angle = 0.0; angle < (2 * PI); angle += ((2 * PI / ((double)nbSteps)))) {
		double u = cos(angle);
		double v = sin(angle);
		AddVertex(center + u*axis1half + v*axis2half,false);
		vertexIds.push_back(sh.nbVertex - 1);
	}
	AddFacet(vertexIds);
}

void Geometry::CreateRacetrack(const Vector3d & center, const Vector3d & axis1Dir, const Vector3d & normalDir, const double & axis1Length, const double & axis2Length, const double & topLength, const size_t & nbSteps)
{
	//Set of 3 equations:
	// r: radius of racetrack arc
	// alpha: half-angle of racetrack arc
	// delta: arc center offset from shape center

	//Equation1: axis1L == 2*(r + delta)
	//Equation2: axis2L == 2*r*sin(alpha)
	//Equation2: topLength == 2*(r*cos(alpha)+delta)

	//Solution by Wolfram Alpha:
	double r = (Sqr(axis1Length) - 2 * axis1Length*topLength + Sqr(axis2Length) + Sqr(topLength)) / (4 * (axis1Length - topLength));
	double delta = axis1Length / 2 - r;
	double alpha = acos((topLength / 2 - delta) / r);

	std::vector<size_t> vertexIds;
	Vector3d i = axis1Dir.Normalized();
	Vector3d j = CrossProduct(normalDir, i).Normalized(); //left-hand coordinate system, V = N x U

	for (size_t step = 0; step < (nbSteps + 1);step++) {  //Right side arc
		double angle = 0.0-alpha + (double)step / (double)nbSteps*2.0*alpha;
		AddVertex(center + delta*i + r*cos(angle)*i + r*sin(angle)*j,false);
		vertexIds.push_back(sh.nbVertex - 1);
	}

	for (size_t step = 0; step < (nbSteps + 1); step++) {  //Left side arc
		double angle = PI-alpha + (double)step / (double)nbSteps*2.0*alpha;
		AddVertex(center - delta*i + r*cos(angle)*i + r*sin(angle)*j,false);
		vertexIds.push_back(sh.nbVertex - 1);
	}

	std::rotate(vertexIds.begin(), vertexIds.begin()+vertexIds.size()-1, vertexIds.end()); //Shift vertex back so U is aligned with the bottom

	AddFacet(vertexIds);
}

int  Geometry::ExplodeSelected(bool toMap, int desType, double exponent, double* values){

	auto selectedFacets = GetSelectedFacets();
	if (selectedFacets.size() == 0) return -1;

	// Check that all facet has a mesh
	bool ok = true;
	int idx = 0;
	while (ok && idx < sh.nbFacet) {
		if (facets[idx]->selected)
			ok = facets[idx]->hasMesh;
		idx++;
	}
	if (!ok) return -2;

	mApp->changedSinceSave = true;

	size_t nb = 0;
	size_t FtoAdd = 0;
	size_t VtoAdd = 0;

	std::vector<FacetGroup> blocks;

	for (auto& sel : selectedFacets) {
		blocks.push_back(facets[sel]->Explode());
		FtoAdd += blocks[nb].facets.size();
		VtoAdd += blocks[nb].nbV;
		nb++;
	}

	// Update vertex array
	InterfaceVertex *ptrVert;
	size_t       vIdx;
	/*
	InterfaceVertex *nVert = (InterfaceVertex *)malloc((wp.nbVertex + VtoAdd) * sizeof(InterfaceVertex));
	memcpy(nVert, vertices3, wp.nbVertex * sizeof(InterfaceVertex));*/
	vertices3.resize(sh.nbVertex + VtoAdd);

	ptrVert = &(vertices3[sh.nbVertex]);
	vIdx = sh.nbVertex;
	nb = 0;
	for (int i = 0; i < sh.nbFacet; i++) {
		if (facets[i]->selected) {
			facets[i]->FillVertexArray(ptrVert);
			for (auto& f : blocks[nb].facets) {
				for (int k = 0; k < f->sh.nbIndex; k++) {
					f->indices[k] = vIdx + k;
				}
				vIdx += f->sh.nbIndex;
			}
			ptrVert += blocks[nb].nbV;
			nb++;
		}
	}
	/*SAFE_FREE(vertices3);
	vertices3 = nVert;*/

	for (size_t i = sh.nbVertex; i < sh.nbVertex + VtoAdd; i++)
		vertices3[i].selected = false;
	sh.nbVertex += VtoAdd;

	// Update facet
	Facet   **f = (Facet **)malloc((sh.nbFacet + FtoAdd - selectedFacets.size()) * sizeof(Facet *));

	auto nbS = selectedFacets.size(); //to remember it after RemveSelected() routine
									  // Delete selected facets
	RemoveFacets(selectedFacets);

	//Fill old facets
	for (nb = 0; nb < sh.nbFacet; nb++)
		f[nb] = facets[nb];

	// Add new facets
	int count = 0;
	for (int i = 0; i < nbS; i++) {
		for (auto& fac : blocks[i].facets) {
			f[nb++] = fac;
#ifdef MOLFLOW
			if (toMap) { //set outgassing values
				f[nb - 1]->sh.outgassing = *(values + count++) *0.100; //0.1: mbar*l/s->Pa*m3/s
				if (f[nb - 1]->sh.outgassing > 0.0) {
					f[nb - 1]->sh.desorbType = desType + 1;
					f[nb - 1]->selected = true;
					if (f[nb - 1]->sh.desorbType == DES_COSINE_N) f[nb - 1]->sh.desorbTypeN = exponent;
				}
				else {
					f[nb - 1]->sh.desorbType = DES_NONE;
					f[nb - 1]->selected = false;
				}
			}
#endif
		}
	}

	/*
	// Free allocated memory
	for (int i = 0; i < nbS; i++) {
	SAFE_FREE(blocks[i].facets);
	}
	SAFE_FREE(blocks);
	*/

	SAFE_FREE(facets);
	facets = f;
	sh.nbFacet = nb;

	// Delete old resources
	DeleteGLLists(true, true);

	InitializeGeometry();

	return 0;

}

void  Geometry::EmptyGeometry() {
	Clear(); //Clear everything first

	//Initialize a default structure:
	sh.nbSuper = 1;
	strName[0] = strdup("");
	//Do rest of init:
	InitializeGeometry(); //sets isLoaded to true

}

PhysicalValue Geometry::GetPhysicalValue(Facet* f, const PhysicalMode& mode, const double& moleculesPerTP, const double& densityCorrection, const double& gasMass, const int& index) {

	//if x==y==-1 and buffer=NULL then returns facet value, otherwise texture cell [x,y] value
	//buff is either NULL or a (BYTE*) pointer to texture or direction buffer, must be locked by AccessDataport before call
	//texture position must be calculated before call (index=i+j*w)

	PhysicalValue result;

	if (index == -1) { //Facet mode

	}
	else { //Texture cell mode
		switch (mode) {
		case PhysicalMode::CellArea:
			result.value = f->GetMeshArea(index);
			break;
		}
	}
	return result;
}

PhysicalValue Geometry::GetPhysicalValue(Facet* f, const PhysicalMode& mode, const double& moleculesPerTP, const double& densityCorrection, const double& gasMass, const int& index, const std::vector<TextureCell>& facetTexture) {
																																	  
	//if x==y==-1 and buffer=NULL then returns facet value, otherwise texture cell [x,y] value
	//buff is either NULL or a (BYTE*) pointer to texture or direction buffer, must be locked by AccessDataport before call
	//texture position must be calculated before call (index=i+j*w)

	PhysicalValue result;

	if (index == -1) { //Facet mode

	}
	else { //Texture cell mode
		switch (mode) {
		case PhysicalMode::CellArea:
			result.value = f->GetMeshArea(index);
			break;
		case PhysicalMode::MCHits:
			result.value = facetTexture[index].countEquiv;
			break;
		case PhysicalMode::ImpingementRate:
		{
			double area = (f->GetMeshArea(index, true));
			if (area == 0.0) area = 1.0;
			result.value = facetTexture[index].countEquiv / (area * 1E-4) * moleculesPerTP;
			break;
		}
		case PhysicalMode::ParticleDensity:
		{
			double density = facetTexture[index].sum_1_per_ort_velocity / (f->GetMeshArea(index, true) * 1E-4) * moleculesPerTP * densityCorrection;
			result.value = density;
			break;
		}
		case PhysicalMode::GasDensity:
		{
			double density = facetTexture[index].sum_1_per_ort_velocity / (f->GetMeshArea(index, true) * 1E-4) * moleculesPerTP * densityCorrection;
			result.value = density * gasMass / 1000.0 / 6E23;
			break;
		}
		case PhysicalMode::Pressure:
			result.value = facetTexture[index].sum_v_ort_per_area * 1E4 * (gasMass / 1000 / 6E23) * 0.0100 * moleculesPerTP;  //1E4 is conversion from m2 to cm2; 0.01 is Pa->mbar
			break;
		case PhysicalMode::AvgGasVelocity:
			result.value = 4.0*facetTexture[index].countEquiv / facetTexture[index].sum_1_per_ort_velocity;
			break;
		}
	}

	return result;
}

	PhysicalValue Geometry::GetPhysicalValue(Facet* f, const PhysicalMode& mode, const double& moleculesPerTP, const double& densityCorrection, const double& gasMass, const int& index, const std::vector<DirectionCell>& facetDirection) {

		//if x==y==-1 and buffer=NULL then returns facet value, otherwise texture cell [x,y] value
		//buff is either NULL or a (BYTE*) pointer to texture or direction buffer, must be locked by AccessDataport before call
		//texture position must be calculated before call (index=i+j*w)

		PhysicalValue result;

		if (index == -1) { //Facet mode

		}
		else { //Texture cell mode
			switch (mode) {

				case PhysicalMode::GasVelocityVector:
				{
					double denominator = (facetDirection[index].count > 0) ? (1.0 / facetDirection[index].count) : 1.0;
					result.vect = facetDirection[index].dir * denominator;
					break;
				}
				case PhysicalMode::NbVelocityVectors:
					result.count = facetDirection[index].count;
					break;
			}

		}
		return result;
}