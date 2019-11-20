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
#include "Geometry_shared.h"
#include "Worker.h"
#include "GLApp/MathTools.h" //Min max
#include "GLApp/GLToolkit.h"
#include <string.h>
#include <math.h>
#include "GLApp/GLMatrix.h"
#include <tuple>
#ifdef MOLFLOW
#include "MolFlow.h"
#include "Interface.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLList.h"
#include "SmartSelection.h"
#include "FacetCoordinates.h"
#include "VertexCoordinates.h"
#include "Facet_shared.h"

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

void Geometry::SelectFacet(size_t facetId) {
	if (!isLoaded) return;
	Facet *f = facets[facetId];
	f->selected = (viewStruct == -1) || (viewStruct == f->sh.superIdx) || (f->sh.superIdx == -1);
	if (!f->selected) f->UnselectElem();
	nbSelectedHist = 0;
	AddToSelectionHist(facetId);
}

void Geometry::SelectArea(int x1, int y1, int x2, int y2, bool clear, bool unselect, bool vertexBound, bool circularSelection) {

	// Select a set of facet according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float rx, ry, rz, rw, r2;
	int _x1, _y1, _x2, _y2;

	_x1 = Min(x1, x2);
	_x2 = Max(x1, x2);
	_y1 = Min(y1, y2);
	_y2 = Max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}

	GLfloat mProj[16];
	GLfloat mView[16];
	GLVIEWPORT g;

	glGetFloatv(GL_PROJECTION_MATRIX, mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, mView);
	glGetIntegerv(GL_VIEWPORT, (GLint *)&g);

	GLMatrix proj; proj.LoadGL(mProj);
	GLMatrix view; view.LoadGL(mView);
	GLMatrix m; m.Multiply(&proj, &view);

	if (clear && !unselect) UnselectAll();
	nbSelectedHist = 0;
	int lastPaintedProgress = -1;
	char tmp[256];
	int paintStep = (int)((double)sh.nbFacet / 10.0);

	for (int i = 0; i < sh.nbFacet; i++) {
		if (sh.nbFacet > 5000) {
			if ((i - lastPaintedProgress) > paintStep) {
				lastPaintedProgress = i;;
				sprintf(tmp, "Facet search: %d%%", (int)(i*100.0 / (double)sh.nbFacet));
				mApp->SetFacetSearchPrg(true, tmp);
			}
		}
		Facet *f = facets[i];
		if (viewStruct == -1 || f->sh.superIdx == viewStruct || f->sh.superIdx == -1) {

			size_t nb = facets[i]->sh.nbIndex;
			bool isInside = true;
			size_t j = 0;
			bool hasSelectedVertex = false;
			while (j < nb && isInside) {

				size_t idx = f->indices[j];
				m.TransfomVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
					&rx, &ry, &rz, &rw);

				if (rw > 0.0f) {
					int xe = (int)(((rx / rw) + 1.0f) * (float)g.width / 2.0f);
					int ye = (int)(((-ry / rw) + 1.0f) * (float)g.height / 2.0f);
					if (!circularSelection)
						isInside = (xe >= _x1) && (xe <= _x2) && (ye >= _y1) && (ye <= _y2);
					else //circular selection
						isInside = (pow((float)(xe - x1), 2) + pow((float)(ye - y1), 2)) <= r2;
					if (vertices3[idx].selected) hasSelectedVertex = true;
				}
				else {

					isInside = false;
				}
				j++;

			}

			if (isInside && (!vertexBound || hasSelectedVertex)) {
				if (!unselect) {
					f->selected = !unselect;
				}
				else {

					f->selected = !unselect;
				}
			}

		}
	}
	mApp->SetFacetSearchPrg(false, NULL);
	UpdateSelection();
}

void Geometry::Select(int x, int y, bool clear, bool unselect, bool vertexBound, int width, int height) {

	int i;
	if (!isLoaded) return;

	// Select a facet on a mouse click in 3D perspectivce view 
	// (x,y) are in screen coordinates
	// TODO: Handle clipped polygon

	// Check intersection of the facet and a "perspective ray"
	std::vector<int> screenXCoords(sh.nbVertex);
	std::vector<int> screenYCoords(sh.nbVertex);

	// Transform points to screen coordinates
	std::vector<bool> ok(sh.nbVertex);
	std::vector<bool> onScreen(sh.nbVertex);
	for (i = 0; i < sh.nbVertex; i++) {//here we could speed up by choosing visible vertices only?
		if (auto screenCoords = GLToolkit::Get2DScreenCoord(vertices3[i])) {
			ok[i] = true;
			std::tie(screenXCoords[i],screenYCoords[i]) = *screenCoords;
			onScreen[i] = screenXCoords[i] >= 0 && screenYCoords[i] >= 0 && screenXCoords[i] <= width && screenYCoords[i] <= height;
		}
		else {
			ok[i] = false;
			//onScreen[i] = false;
		}
	}

	// Check facets
	bool found = false;
	bool clipped;
	bool hasVertexOnScreen;
	bool hasSelectedVertex;
	i = 0;
	char tmp[256];
	int lastFound = -1;
	int lastPaintedProgress = -1;
	int paintStep = (int)((double)sh.nbFacet / 10.0);

	while (i < sh.nbFacet && !found) {
		if (sh.nbFacet > 5000) {
			if ((i - lastPaintedProgress) > paintStep) {
				lastPaintedProgress = i;;
				sprintf(tmp, "Facet search: %d%%", (int)(i*100.0 / (double)sh.nbFacet));
				mApp->SetFacetSearchPrg(true, tmp);
			}
		}
		if (viewStruct == -1 || facets[i]->sh.superIdx == viewStruct || facets[i]->sh.superIdx == -1) {

			clipped = false;
			hasVertexOnScreen = false;
			hasSelectedVertex = false;
			// Build array of 2D points
			std::vector<Vector2d> v(facets[i]->indices.size());

			for (int j = 0; j < facets[i]->indices.size() && !clipped; j++) {
				size_t idx = facets[i]->indices[j];
				if (ok[idx]) {
					v[j] = Vector2d((double)screenXCoords[idx],(double)screenYCoords[idx]);
					if (onScreen[idx]) hasVertexOnScreen = true;
				}
				else {
					clipped = true;
				}
			}
			if (vertexBound) { //CAPS LOCK on, select facets onyl with at least one seleted vertex
				for (size_t j = 0; j < facets[i]->indices.size() && (!hasSelectedVertex); j++) {
					size_t idx = facets[i]->indices[j];
					if (vertices3[idx].selected) hasSelectedVertex = true;
				}
			}

			if (!clipped && hasVertexOnScreen && (!vertexBound || hasSelectedVertex)) {

				found = IsInPoly(Vector2d((double)x,(double)y), v);

				if (found) {
					if (unselect) {
						if (!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) {
							facets[i]->selected = false;
							found = false; //Continue looking for facets
						}
						else { //Smart selection
							double maxAngleDiff = mApp->smartSelection->GetMaxAngle();
							std::vector<size_t> connectedFacets;
							mApp->SetFacetSearchPrg(true, "Smart selecting...");
							if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(i, maxAngleDiff);
							for (auto& ind : connectedFacets)
								facets[ind]->selected = false;
							mApp->SetFacetSearchPrg(false, "");
						}
					} //end unselect

					if (AlreadySelected(i)) {

						lastFound = i;
						found = false; //Continue looking for facets

					}
				} //end found

			}

		}

		if (!found) i++;

	}
	mApp->SetFacetSearchPrg(false, "");
	if (clear && !unselect) UnselectAll();

	if (!found && lastFound >= 0) {
		if (!unselect) {
			// Restart
			nbSelectedHist = 0;
			AddToSelectionHist(lastFound);
		}
		facets[lastFound]->selected = !unselect;
		if (!unselect) mApp->facetList->ScrollToVisible(lastFound, 0, true); //scroll to selected facet
	}
	else {

		if (found) {
			if (!unselect) AddToSelectionHist(i);
			if (!mApp->smartSelection || !mApp->smartSelection->IsSmartSelection()) {
				facets[i]->selected = !unselect;
			}
			else { //Smart selection
				double maxAngleDiff = mApp->smartSelection->GetMaxAngle();
				std::vector<size_t> connectedFacets;
				mApp->SetFacetSearchPrg(true, "Smart selecting...");
				if (maxAngleDiff >= 0.0) connectedFacets = GetConnectedFacets(i, maxAngleDiff);
				for (auto& ind : connectedFacets)
					facets[ind]->selected = !unselect;
				mApp->SetFacetSearchPrg(false, "");
			}
			if (!unselect) mApp->facetList->ScrollToVisible(i, 0, true); //scroll to selected facet
		}
		else {

			nbSelectedHist = 0;
		}
	}
	UpdateSelection();

}

void Geometry::SelectVertex(int vertexId) {
	//isVertexSelected[vertexId] = (viewStruct==-1) || (viewStruct==f->wp.superIdx);
	//here we should look through facets if vertex is member of any
	//if( !f->selected ) f->UnselectElem();
	if (!isLoaded) return;
	vertices3[vertexId].selected = true;
}

void Geometry::SelectVertex(int x1, int y1, int x2, int y2, bool shiftDown, bool ctrlDown, bool circularSelection, bool facetBound) {

	// Select a set of vertices according to a 2D bounding rectangle
	// (x1,y1) and (x2,y2) are in viewport coordinates

	float rx, ry, rz, rw, r2;
	int _x1, _y1, _x2, _y2;

	_x1 = Min(x1, x2);
	_x2 = Max(x1, x2);
	_y1 = Min(y1, y2);
	_y2 = Max(y1, y2);

	if (circularSelection) {
		r2 = pow((float)(x1 - x2), 2) + pow((float)(y1 - y2), 2);
	}

	GLfloat mProj[16];
	GLfloat mView[16];
	GLVIEWPORT g;

	glGetFloatv(GL_PROJECTION_MATRIX, mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, mView);
	glGetIntegerv(GL_VIEWPORT, (GLint *)&g);

	GLMatrix proj; proj.LoadGL(mProj);
	GLMatrix view; view.LoadGL(mView);
	GLMatrix m; m.Multiply(&proj, &view);

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	std::vector<bool> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	for (int i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		Vector3d *v = GetVertex(i);
		//if(viewStruct==-1 || f->wp.superIdx==viewStruct) {
		if (true) {

			bool isInside;
			int idx = i;
			m.TransfomVec((float)vertices3[idx].x, (float)vertices3[idx].y, (float)vertices3[idx].z, 1.0f,
				&rx, &ry, &rz, &rw);

			if (rw > 0.0f) {
				int xe = (int)(((rx / rw) + 1.0f) * (float)g.width / 2.0f);
				int ye = (int)(((-ry / rw) + 1.0f) * (float)g.height / 2.0f);
				if (!circularSelection)
					isInside = (xe >= _x1) && (xe <= _x2) && (ye >= _y1) && (ye <= _y2);
				else //circular selection
					isInside = (pow((float)(xe - x1), 2) + pow((float)(ye - y1), 2)) <= r2;
			}
			else {

				isInside = false;
			}

			if (isInside) {
				vertices3[i].selected = !ctrlDown;
				if (ctrlDown) RemoveFromSelectedVertexList(i);
				else {
					AddToSelectedVertexList(i);
					if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(i);
				}
			}
		}
	}

	//UpdateSelectionVertex();
	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void Geometry::SelectVertex(int x, int y, bool shiftDown, bool ctrlDown, bool facetBound) {
	int i;
	if (!isLoaded) return;

	// Select a vertex on a mouse click in 3D perspectivce view 
	// (x,y) are in screen coordinates
	// TODO: Handle clipped polygon

	// Check intersection of the facet and a "perspective ray"
	std::vector<int> allXe(sh.nbVertex);
	std::vector<int> allYe(sh.nbVertex);
	std::vector<bool> ok(sh.nbVertex);

	std::vector<bool> selectedFacetsVertices;
	if (facetBound) selectedFacetsVertices = GetVertexBelongsToSelectedFacet();

	// Transform points to screen coordinates
	for (i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		if (auto screenCoords = GLToolkit::Get2DScreenCoord(vertices3[i])) {
			ok[i] = true;
			std::tie(allXe[i], allYe[i]) = *screenCoords;
		}
		else {
			ok[i] = false;
		}
	}

	//Get Closest Point to click
	double minDist = 9999;
	double distance;
	int minId = -1;
	for (i = 0; i < sh.nbVertex; i++) {
		if (facetBound && !selectedFacetsVertices[i]) continue; //doesn't belong to selected facet
		if (ok[i] && !(allXe[i] < 0) && !(allYe[i] < 0)) { //calculate only for points on screen
			distance = pow((double)(allXe[i] - x), 2) + pow((double)(allYe[i] - y), 2);
			if (distance < minDist) {
				minDist = distance;
				minId = i;
			}
		}
	}

	if (!ctrlDown && !shiftDown) {
		UnselectAllVertex(); EmptySelectedVertexList();
		//nbSelectedHistVertex = 0;
	}

	if (minDist < 250.0) {
		vertices3[minId].selected = !ctrlDown;
		if (ctrlDown) RemoveFromSelectedVertexList(minId);
		else {
			AddToSelectedVertexList(minId);
			if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateId(minId);
			//nbSelectedHistVertex++;
		}
	}

	//UpdateSelection();
	if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
}

void Geometry::AddToSelectionHist(size_t f) {

	if (nbSelectedHist < SEL_HISTORY) {
		selectHist[nbSelectedHist] = f;
		nbSelectedHist++;
	}

}

bool Geometry::AlreadySelected(size_t f) {

	// Check if the facet has already been selected
	bool found = false;
	size_t i = 0;
	while (!found && i < nbSelectedHist) {
		found = (selectHist[i] == f);
		if (!found) i++;
	}
	return found;

}

void Geometry::SelectAll() {
	for (int i = 0; i < sh.nbFacet; i++)
		SelectFacet(i);
	UpdateSelection();
}

void Geometry::EmptySelectedVertexList() {
	selectedVertexList_ordered.clear();
}

void Geometry::RemoveFromSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.erase(std::remove(selectedVertexList_ordered.begin(), selectedVertexList_ordered.end(), vertexId), selectedVertexList_ordered.end());
}

void Geometry::AddToSelectedVertexList(size_t vertexId) {
	selectedVertexList_ordered.push_back(vertexId);
}

void Geometry::SelectAllVertex() {
	for (int i = 0; i < sh.nbVertex; i++)
		SelectVertex(i);
	//UpdateSelectionVertex();
}

size_t Geometry::GetNbSelectedVertex() {
	size_t nbSelectedVertex = 0;
	for (int i = 0; i < sh.nbVertex; i++) {
		if (vertices3[i].selected) nbSelectedVertex++;
	}
	return nbSelectedVertex;
}

void Geometry::UnselectAll() {
	for (int i = 0; i < sh.nbFacet; i++) {
		facets[i]->selected = false;
		facets[i]->UnselectElem();
	}
	UpdateSelection();
}

void Geometry::UnselectAllVertex() {
	for (int i = 0; i < sh.nbVertex; i++) {
		vertices3[i].selected = false;
		//facets[i]->UnselectElem(); //what is this?
	}
	//UpdateSelectionVertex();
}

std::vector<size_t> Geometry::GetSelectedVertices()
{
	std::vector<size_t> sel;
	for (size_t i = 0; i < sh.nbVertex; i++)
		if (vertices3[i].selected) sel.push_back(i);
	return sel;
}

void Geometry::DrawFacet(Facet *f, bool offset, bool showHidden, bool selOffset) {

	// Render a facet (wireframe)
	size_t nb = f->sh.nbIndex;
	size_t i1;

	if (offset) {

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		if (selOffset) {
			glPolygonOffset(0.0f, 1.0f);
		}
		else {

			glPolygonOffset(0.0f, 5.0f);
		}
		glBegin(GL_POLYGON);
		for (size_t j = 0; j < nb; j++) {
			i1 = f->indices[j];
			glEdgeFlag(f->visible[j] || showHidden);
			glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
		}
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);

	}
	else {

		if (nb < 8) {
			// No hole possible
			glBegin(GL_LINE_LOOP);
			for (size_t j = 0; j < nb; j++) {
				i1 = f->indices[j];
				glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
			}
			glEnd();
		}
		else {

			glBegin(GL_LINES);
			size_t i1, i2, j;
			for (j = 0; j < nb - 1; j++) {
				if (f->visible[j] || showHidden) {
					i1 = f->indices[j];
					i2 = f->indices[j + 1];
					glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
					glVertex3d(vertices3[i2].x, vertices3[i2].y, vertices3[i2].z);
				}
			}
			// Last segment
			if (f->visible[j] || showHidden) {
				i1 = f->indices[j];
				i2 = f->indices[0];
				glVertex3d(vertices3[i1].x, vertices3[i1].y, vertices3[i1].z);
				glVertex3d(vertices3[i2].x, vertices3[i2].y, vertices3[i2].z);
			}
			glEnd();
		}

	}

}

void Geometry::DrawPolys() {

	std::vector<size_t> f3; f3.reserve(sh.nbFacet);
	std::vector<size_t> f4; f4.reserve(sh.nbFacet);
	std::vector<size_t> fp; fp.reserve(sh.nbFacet);

	// Group TRI,QUAD and POLY
	for (size_t i = 0; i < sh.nbFacet; i++) {
		size_t nb = facets[i]->sh.nbIndex;
		if (facets[i]->volumeVisible) {
			if (nb == 3) {
				f3.push_back(i);
			}
			else if (nb == 4) {
				f4.push_back(i);
			}
			else {
				fp.push_back(i);
			}
		}
	}

	// Draw
	glBegin(GL_TRIANGLES);

	// Triangle
	for (const auto& i : f3)
		FillFacet(facets[i], false);

	// Triangulate polygon
	for (const auto& i : fp)
		Triangulate(facets[i], false);

	glEnd();

	// Quads
	glBegin(GL_QUADS);
	for (const auto& i : f4)
		FillFacet(facets[i], false);
	glEnd();

}
void Geometry::SetCullMode(int mode) {

	switch (mode) {
	case 1: // SHOW_FRONT
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case 2: // SHOW_BACK
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	default: //SHOW_FRONTANDBACK
		glDisable(GL_CULL_FACE);
	}

}

void Geometry::ClearFacetTextures()
{
	GLProgress *prg = new GLProgress("Clearing texture", "Frame update");
	prg->SetBounds(5, 28, 300, 90);
	int startTime = SDL_GetTicks();
	for (int i = 0; i<sh.nbFacet; i++) {
		if (!prg->IsVisible() && ((SDL_GetTicks() - startTime) > 500)) {
			prg->SetVisible(true);
		}
		prg->SetProgress((double)i / (double)sh.nbFacet);
		DELETE_TEX(facets[i]->glTex);
		glGenTextures(1, &facets[i]->glTex);
	}
	prg->SetVisible(false);
	SAFE_DELETE(prg);
}

void Geometry::RenderArrow(GLfloat *matView, float dx, float dy, float dz, float px, float py, float pz, float d) {

	if (!arrowList) BuildShapeList();

	// Compute transformation matrix for the arrow
	GLMatrix mView;
	GLMatrix aView;
	GLMatrix mScale;
	GLMatrix mRot;
	GLMatrix mT;
	float v1x, v1y, v1z;

	mView.LoadGL(matView);

	// Direction
	float n = sqrtf(dx*dx + dy*dy + dz*dz);
	if (IsZero(n)) {

		// Isotropic (render a sphere)
		mScale._11 = (d / 4.0f);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);

		mT.Translate(px, py, pz);

		aView.Multiply(&mView);
		aView.Multiply(&mT);
		aView.Multiply(&mScale);

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(aView.GetGL());
		glCallList(sphereList);
		return;

	}

	dx /= n;
	dy /= n;
	dz /= n;
	mRot._11 = dx;
	mRot._21 = dy;
	mRot._31 = dz;

	// A point belonging to the plane
	// normal to the direction vector
	if (!IsZero(dx)) {
		v1x = -dz / dx;
		v1y = 0.0;
		v1z = 1.0;
	}
	else if (!IsZero(dy)) {
		v1x = 0.0;
		v1y = -dz / dy;
		v1z = 1.0;
	}
	else if (!IsZero(dz)) {
		// normal to z
		v1x = 1.0;
		v1y = 0.0;
		v1z = 0.0;
	}
	else {

		// Null vector -> isotropic
	}

	float n1 = sqrtf(v1x*v1x + v1y*v1y + v1z*v1z);
	v1x /= n1;
	v1y /= n1;
	v1z /= n1;
	mRot._12 = v1x;
	mRot._22 = v1y;
	mRot._32 = v1z;

	// Cross product
	mRot._13 = (dy)*(v1z)-(dz)*(v1y);
	mRot._23 = (dz)*(v1x)-(dx)*(v1z);
	mRot._33 = (dx)*(v1y)-(dy)*(v1x);

	// Scale
	if (!autoNorme) {
		mScale._11 = (n*d*normeRatio);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);
	}
	else {

		// Show only direction
		mScale._11 = (d / 1.1f);
		mScale._22 = (d / 4.0f);
		mScale._33 = (d / 4.0f);
	}

	mT.Translate(px, py, pz);

	aView.Multiply(&mView);
	aView.Multiply(&mT);
	aView.Multiply(&mRot);
	aView.Multiply(&mScale);
	if (!centerNorme) {
		mT.Translate(0.5f, 0.0, 0.0);
		aView.Multiply(&mT);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(aView.GetGL());
	glCallList(arrowList);

}

// Triangulation stuff

int Geometry::FindEar(const GLAppPolygon& p) {

	int i = 0;
	bool earFound = false;
	while (i < p.pts.size() && !earFound) {
		if (IsConvex(p, i))
			earFound = !ContainsConcave(p, i - 1, i, i + 1);
		if (!earFound) i++;
	}

	// REM: Theoritically, it should always find an ear (2-Ears theorem).
	// However on degenerated geometry (flat poly) it may not find one.
	// Returns first point in case of failure.
	if (earFound)
		return i;
	else
		return 0;

}

void Geometry::AddTextureCoord(Facet *f, const Vector2d *p) {

	// Add texture coord with a 1 texel border (for bilinear filtering)
	double uStep = 1.0 / (double)f->texDimW;
	double vStep = 1.0 / (double)f->texDimH;

#if 1
	double fu = f->sh.texWidthD * uStep;
	double fv = f->sh.texHeightD * vStep;
	glTexCoord2f((float)(uStep + p->u*fu), (float)(vStep + p->v*fv));
#else
	// Show border (debugging purpose)
	double fu = (f->sh.texWidthD + 2.0) * uStep;
	double fv = (f->sh.texHeightD + 2.0) * vStep;
	glTexCoord2f((float)(p->u*fu), (float)(p->v*fv));
#endif

}

void Geometry::FillFacet(Facet *f, bool addTextureCoord) {
	//Commented out sections: theoretically in a right-handed system the vertex order is inverse
	//However we'll solve it simpler by inverting the geometry viewer Front/back culling mode setting

	glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
	/*size_t nbDrawn = 0;
	size_t i;
	if (mApp->leftHandedView) {
			i = 0;
			glNormal3d(-f->wp.N.x, -f->wp.N.y, -f->wp.N.z);
	}
	else {
			i = f->wp.nbIndex-1;
			glNormal3d(f->wp.N.x, f->wp.N.y, f->wp.N.z);
	}
	for (; nbDrawn < f->wp.nbIndex; nbDrawn++) {*/
	for (size_t i=0;i<f->sh.nbIndex;i++) {
		size_t idx = f->indices[i];
		if (addTextureCoord) AddTextureCoord(f, &(f->vertices2[i]));
		glVertex3d(vertices3[idx].x, vertices3[idx].y, vertices3[idx].z);
		/*if (mApp->leftHandedView) {
			i++;
		}
		else {
			i--;
		}*/
	}
}

void Geometry::DrawEar(Facet *f, const GLAppPolygon& p, int ear, bool addTextureCoord) {

	//Commented out sections: theoretically in a right-handed system the vertex order is inverse
	//However we'll solve it simpler by inverting the geometry viewer Front/back culling mode setting

	Vector3d  p3D;
	const Vector2d* p1;
	const Vector2d* p2;
	const Vector2d* p3;

	// Follow orientation
	/*double handedness = mApp->leftHandedView ? 1.0 : -1.0;*/
	
	//if (/*handedness * */ p.sign > 0) {
	//	p1 = &(p.pts[Previous(ear, p.pts.size())]);
	//	p2 = &(p.pts[Next(ear, p.pts.size())]);
	//	p3 = &(p.pts[IDX(ear, p.pts.size())]);
	//}
	//else {
		p1 = &(p.pts[Previous(ear, p.pts.size())]);
		p2 = &(p.pts[IDX(ear, p.pts.size())]);
		p3 = &(p.pts[Next(ear, p.pts.size())]);
	//}

	glNormal3d(-f->sh.N.x, -f->sh.N.y, -f->sh.N.z);
	if (addTextureCoord) AddTextureCoord(f, p1);
	f->glVertex2u(p1->u, p1->v);

	//glNormal3d(-f->wp.N.x, -f->wp.N.y, -f->wp.N.z);
	if (addTextureCoord) AddTextureCoord(f, p2);
	f->glVertex2u(p2->u, p2->v);

	//glNormal3d(-f->wp.N.x, -f->wp.N.y, -f->wp.N.z);
	if (addTextureCoord) AddTextureCoord(f, p3);
	f->glVertex2u(p3->u, p3->v);

}

void Geometry::Triangulate(Facet *f, bool addTextureCoord) {

	// Triangulate a facet (rendering purpose)
	// The facet must have at least 3 points
	// Use the very simple "Two-Ears" theorem. It computes in O(n^2).

	if (f->nonSimple) {
		// Not a simple polygon
		// Abort triangulation
		return;
	}

	// Build a Polygon
	GLAppPolygon p;
	p.pts = f->vertices2;
	//p.sign = f->sign;
	
	// Perform triangulation
	while (p.pts.size() > 3) {
		int e = FindEar(p);
		DrawEar(f, p, e, addTextureCoord);
		// Remove the ear
		p.pts.erase(p.pts.begin() + e);
	}

	// Draw the last ear
	DrawEar(f, p, 0, addTextureCoord);

}

void Geometry::Render(GLfloat *matView, bool renderVolume, bool renderTexture, int showMode, bool filter, bool showHidden, bool showMesh, bool showDir) {

	if (!isLoaded) return;

	// Render the geometry
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	// Render Volume
	if (renderVolume) {
		glPolygonOffset(1.0f, 4.0f);
		SetCullMode(showMode);
		GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHTING);
		GLToolkit::SetMaterial(&fillMaterial);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glCallList(polyList);
		glDisable(GL_POLYGON_OFFSET_FILL);
		GLToolkit::SetMaterial(&whiteMaterial);
		glDisable(GL_LIGHTING);
	}
	else {

		// Default material
		GLToolkit::SetMaterial(&whiteMaterial);

		// Draw lines
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

		float color = (mApp->whiteBg) ? 0.0f : 1.0f; //whitebg here
		if (viewStruct == -1) {
			glColor4f(color, color, color, 0.5f);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			}
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			for (int i = 0;i < sh.nbSuper;i++)
				glCallList(lineList[i]);
			glDisable(GL_POLYGON_OFFSET_LINE);
			glDisable(GL_BLEND);
			glDisable(GL_LINE_SMOOTH);
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		else {

			// Draw non selectable facet in dark grey
			glColor3f(0.2f, 0.2f, 0.2f);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
			}
			for (int i = 0;i < sh.nbSuper;i++)
				if (viewStruct != i)
					glCallList(lineList[i]);
			// Selectable in white
			glColor3f(color, color, color);
			glPolygonOffset(1.0f, 5.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glCallList(lineList[viewStruct]);
			glDisable(GL_POLYGON_OFFSET_LINE);
			if (mApp->antiAliasing) {
				glDisable(GL_BLEND);
				glDisable(GL_LINE_SMOOTH);
			}
		}
	}

	// Paint texture
	if (renderTexture) {
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 3.0f);
		for (size_t i = 0;i < sh.nbFacet && renderTexture;i++) {
			Facet *f = facets[i];
			bool paintRegularTexture = f->sh.isTextured && f->textureVisible && (f->sh.countAbs || f->sh.countRefl || f->sh.countTrans);
#ifdef MOLFLOW
			paintRegularTexture = paintRegularTexture || (f->sh.isTextured && f->textureVisible && (f->sh.countACD || f->sh.countDes));
#endif
			if (paintRegularTexture) {
				if (f->sh.is2sided)   glDisable(GL_CULL_FACE);
				else                   SetCullMode(showMode);
				glBindTexture(GL_TEXTURE_2D, f->glTex);
				if (filter) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
				else {

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				}
				
				glCallList(f->glList);
			}
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	}

	// Paint mesh
	if (showMesh) {
		glColor4f(0.7f, 0.7f, 0.7f, 0.3f);
		if (mApp->antiAliasing) {
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (int i = 0; i < sh.nbFacet;i++) {

			Facet *f = facets[i];
			if (f->cellPropertiesIds  && f->textureVisible) {
				if (!f->glElem) f->BuildMeshGLList();

				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(1.0f, 2.0f);
				glCallList(f->glElem);
				glDisable(GL_POLYGON_OFFSET_LINE);
			}
		}
		if (mApp->antiAliasing) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
		}
	}

	// Paint direction fields
	if (showDir) {

		GLToolkit::SetMaterial(&arrowMaterial);
		for (int i = 0;i < sh.nbFacet;i++) {
			Facet *f = facets[i];
			if (f->sh.countDirection && f->dirCache) {
				double iw = 1.0 / (double)f->sh.texWidthD;
				double ih = 1.0 / (double)f->sh.texHeightD;
				double rw = f->sh.U.Norme() * iw;
				for (int x = 0;x < f->sh.texWidth;x++) {
					for (int y = 0;y < f->sh.texHeight;y++) {
						size_t add = x + y*f->sh.texWidth;
						if (f->GetMeshArea(add) > 0.0) {
							double uC = ((double)x + 0.5) * iw;
							double vC = ((double)y + 0.5) * ih;
							float xc = (float)(f->sh.O.x + f->sh.U.x*uC + f->sh.V.x*vC);
							float yc = (float)(f->sh.O.y + f->sh.U.y*uC + f->sh.V.y*vC);
							float zc = (float)(f->sh.O.z + f->sh.U.z*uC + f->sh.V.z*vC);

							RenderArrow(matView,
								(float)f->dirCache[add].dir.x,
								(float)f->dirCache[add].dir.y,
								(float)f->dirCache[add].dir.z,
								xc, yc, zc, (float)rw); // dircache already normalized
						}
					}
				}
			}
		}

		// Restore default matrix
		glLoadMatrixf(matView);
	}


	// Paint non-planar and selected facets
	//if (GetNbSelectedFacets()>0) {
		if (mApp->antiAliasing) {
			glEnable(GL_BLEND);
			glEnable(GL_LINE_SMOOTH);
		}
		glBlendFunc(GL_ONE, GL_ZERO);
		if (mApp->highlightNonplanarFacets) {
			glColor3f(1.0f, 0.0f, 1.0f);    //purple
			glCallList(nonPlanarList);
		}
		glColor3f(1.0f, 0.0f, 0.0f);    //red
		if (showHidden) {
			glDisable(GL_DEPTH_TEST);
			glCallList(selectList3);
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glCallList(selectList3);
		}
		if (mApp->antiAliasing) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
		}

	//}

	// Paint selected cell on mesh
	for (int i = 0; i < sh.nbFacet;i++) {
		Facet *f = facets[i];
		f->RenderSelectedElem();
	}

}

void Geometry::DeleteGLLists(bool deletePoly, bool deleteLine) {
	if (deleteLine) {
		for (int i = 0; i < sh.nbSuper; i++)
			DELETE_LIST(lineList[i]);
	}
	if (deletePoly) DELETE_LIST(polyList);
	DELETE_LIST(selectList);
	DELETE_LIST(selectList2);
	DELETE_LIST(selectList3);
}

std::vector<bool> Geometry::GetVertexBelongsToSelectedFacet() {
	std::vector<bool> result(sh.nbVertex, false);
	std::vector<size_t> selFacetIds = GetSelectedFacets();
	for (auto& facetId : selFacetIds) {
		Facet* f = facets[facetId];
		for (size_t i = 0; i < f->sh.nbIndex; i++)
			result[f->indices[i]] = true;
	}
	return result;
}

void Geometry::BuildShapeList() {

	// Shapes used for direction field rendering

	// 3D arrow (direction field)
	int nbDiv = 10;
	double alpha = 2.0*PI / (double)nbDiv;

	arrowList = glGenLists(1);
	glNewList(arrowList, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_TRIANGLES);

	// Arrow
	for (int i = 0; i < nbDiv; i++) {

		double y1 = sin(alpha*(double)i);
		double z1 = cos(alpha*(double)i);
		double y2 = sin(alpha*(double)((i + 1) % nbDiv));
		double z2 = cos(alpha*(double)((i + 1) % nbDiv));

		glNormal3d(0.0, y1, z1);
		glVertex3d(-0.5, 0.5*y1, 0.5*z1);
		glNormal3d(1.0, 0.0, 0.0);
		glVertex3d(0.5, 0.0, 0.0);
		glNormal3d(0.0, y2, z2);
		glVertex3d(-0.5, 0.5*y2, 0.5*z2);

	}

	// Cap facets
	for (int i = 0; i < nbDiv; i++) {

		double y1 = sin(alpha*(double)i);
		double z1 = cos(alpha*(double)i);
		double y2 = sin(alpha*(double)((i + 1) % nbDiv));
		double z2 = cos(alpha*(double)((i + 1) % nbDiv));

		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.5*y1, 0.5*z1);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.5*y2, 0.5*z2);
		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3d(-0.5, 0.0, 0.0);

	}

	glEnd();
	glEndList();

	// Shpere list (isotropic case)
	int nbPhi = 16;
	int nbTetha = 7;
	double dphi = 2.0*PI / (double)(nbPhi);
	double dtetha = PI / (double)(nbTetha + 1);

	sphereList = glGenLists(1);
	glNewList(sphereList, GL_COMPILE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_TRIANGLES);

	for (int i = 0; i <= nbTetha; i++) {
		for (int j = 0; j < nbPhi; j++) {

			Vector3d v1, v2, v3, v4;

			v1.x = sin(dtetha*(double)i)*cos(dphi*(double)j);
			v1.y = sin(dtetha*(double)i)*sin(dphi*(double)j);
			v1.z = cos(dtetha*(double)i);

			v2.x = sin(dtetha*(double)(i + 1))*cos(dphi*(double)j);
			v2.y = sin(dtetha*(double)(i + 1))*sin(dphi*(double)j);
			v2.z = cos(dtetha*(double)(i + 1));

			v3.x = sin(dtetha*(double)(i + 1))*cos(dphi*(double)(j + 1));
			v3.y = sin(dtetha*(double)(i + 1))*sin(dphi*(double)(j + 1));
			v3.z = cos(dtetha*(double)(i + 1));

			v4.x = sin(dtetha*(double)i)*cos(dphi*(double)(j + 1));
			v4.y = sin(dtetha*(double)i)*sin(dphi*(double)(j + 1));
			v4.z = cos(dtetha*(double)i);

			if (i < nbTetha) {
				glNormal3d(v1.x, v1.y, v1.z);
				glVertex3d(v1.x, v1.y, v1.z);
				glNormal3d(v2.x, v2.y, v2.z);
				glVertex3d(v2.x, v2.y, v2.z);
				glNormal3d(v3.x, v3.y, v3.z);
				glVertex3d(v3.x, v3.y, v3.z);
			}

			if (i > 0) {
				glNormal3d(v1.x, v1.y, v1.z);
				glVertex3d(v1.x, v1.y, v1.z);
				glNormal3d(v3.x, v3.y, v3.z);
				glVertex3d(v3.x, v3.y, v3.z);
				glNormal3d(v4.x, v4.y, v4.z);
				glVertex3d(v4.x, v4.y, v4.z);
			}

		}
	}

	glEnd();
	glEndList();

}

void Geometry::BuildSelectList() {

	selectList = glGenLists(1);
	glNewList(selectList, GL_COMPILE);
	/*
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	if (antiAliasing){
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	//glBlendFunc(GL_ONE,GL_ZERO);
	}
	glLineWidth(2.0f);

	for(int i=0;i<wp.nbFacet;i++ ) {
	Facet *f = facets[i];
	if( f->selected ) {
	//DrawFacet(f,false);
	DrawFacet(f,1,1,1);
	nbSelected++;
	}
	}
	glLineWidth(1.0f);
	if (antiAliasing) {
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	}*/
	glEndList();

	// Second list for usage with POLYGON_OFFSET
	selectList2 = glGenLists(1);
	glNewList(selectList2, GL_COMPILE);
	/*
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	if (antiAliasing){
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	for(int i=0;i<wp.nbFacet;i++ ) {
	Facet *f = facets[i];
	if( f->selected )
	{
	//DrawFacet(f,true,false,true);
	DrawFacet(f,1,1,1);
	}
	}
	glLineWidth(1.0f);
	if (antiAliasing) {
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	}*/
	glEndList();

	// Third list with hidden (hole join) edge visible
	selectList3 = glGenLists(1);
	glNewList(selectList3, GL_COMPILE);

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	if (mApp->antiAliasing) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	auto selectedFacets = GetSelectedFacets();
	for (auto& sel : selectedFacets) {
		Facet *f = facets[sel];
		//DrawFacet(f,false,true,true);
		DrawFacet(f, false, true, false); //Faster than true true true, without noticeable glitches
	}
	glLineWidth(1.0f);
	if (mApp->antiAliasing) {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	glEndList();

}


void Geometry::BuildNonPlanarList() {

	
	nonPlanarList = glGenLists(1);
	glNewList(nonPlanarList, GL_COMPILE);

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	if (mApp->antiAliasing) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}
	glLineWidth(2.0f);

	auto nonPlanarFacetIds = GetNonPlanarFacetIds();
	hasNonPlanar = nonPlanarFacetIds.size() > 0;
	for (const auto& np : nonPlanarFacetIds) {
		Facet *f = facets[np];
		//DrawFacet(f,false,true,true);
		DrawFacet(f, false, true, false); //Faster than true true true, without noticeable glitches
	}
	glLineWidth(1.0f);
	if (mApp->antiAliasing) {
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}
	glEndList();

}


void Geometry::UpdateSelection() {

	DeleteGLLists();
	BuildSelectList();

}

void Geometry::BuildGLList() {

	// Compile geometry for OpenGL
	for (int j = 0; j < sh.nbSuper; j++) {
		lineList[j] = glGenLists(1);
		glNewList(lineList[j], GL_COMPILE);
		for (int i = 0; i < sh.nbFacet; i++) {
			if (facets[i]->sh.superIdx == j || facets[i]->sh.superIdx == -1)
				DrawFacet(facets[i], false, true, false);
		}
		glEndList();
	}

	polyList = glGenLists(1);
	glNewList(polyList, GL_COMPILE);
	DrawPolys();
	glEndList();

	BuildNonPlanarList();
	BuildSelectList();

}

int Geometry::InvalidateDeviceObjects() {

	DeleteGLLists(true, true);
	DELETE_LIST(arrowList);
	DELETE_LIST(sphereList);
	for (int i = 0; i < sh.nbFacet; i++)
		facets[i]->InvalidateDeviceObjects();

	return GL_OK;

}

int Geometry::RestoreDeviceObjects() {

	if (!IsLoaded()) return GL_OK;

	for (int i = 0; i < sh.nbFacet; i++) {
		Facet *f = facets[i];
		f->RestoreDeviceObjects();
		BuildFacetList(f);
	}

	BuildGLList();

	return GL_OK;

}

void Geometry::BuildFacetList(Facet *f) {

	// Rebuild OpenGL geometry with texture

	if (f->sh.isTextured) {

		// Facet geometry
		glNewList(f->glList, GL_COMPILE);
		if (f->sh.nbIndex == 3) {
			glBegin(GL_TRIANGLES);
			FillFacet(f, true);
			glEnd();
		}
		else if (f->sh.nbIndex == 4) {

			glBegin(GL_QUADS);
			FillFacet(f, true);
			glEnd();
		}
		else {

			glBegin(GL_TRIANGLES);
			Triangulate(f, true);
			glEnd();
		}
		glEndList();
	}
}