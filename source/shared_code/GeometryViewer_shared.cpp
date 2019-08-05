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
#include "GLApp/GLApp.h"
#include "GLApp/GLWindowManager.h"
#include "GeometryViewer.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/MathTools.h" //
#include "GLApp/GLMatrix.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"

#include "Geometry_shared.h"
#include "SDL_SavePNG/savepng.h"

#include <math.h>
//#include <malloc.h>
#include "Facet_shared.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#include "SuperFacet.h"

#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

GeometryViewer::GeometryViewer(int id) :GLComponent(id) {

	work = NULL;

	// Material

	memset(&greenMaterial, 0, sizeof(GLMATERIAL));
	greenMaterial.Diffuse.r = 0.0f;
	greenMaterial.Diffuse.g = 1.0f;
	greenMaterial.Diffuse.b = 0.0f;
	greenMaterial.Ambient.r = 0.0f;
	greenMaterial.Ambient.g = 1.0f;
	greenMaterial.Ambient.b = 0.0f;

	memset(&blueMaterial, 0, sizeof(GLMATERIAL));
	blueMaterial.Diffuse.r = 0.5f;
	blueMaterial.Diffuse.g = 0.6f;
	blueMaterial.Diffuse.b = 1.0f;
	blueMaterial.Ambient.r = 0.5f;
	blueMaterial.Ambient.g = 0.6f;
	blueMaterial.Ambient.b = 1.0f;

	/// Default values
	screenshotStatus.requested = 0;
	draggMode = DRAGG_NONE;
	selected = false;
	view.projMode = ORTHOGRAPHIC_PROJ;
	view.camAngleOx = 0.0;
	view.camAngleOy = 0.0;

	view.camAngleOz = 0.0;

	view.lightAngleOx = 0.0;
	view.lightAngleOy = 0.0;
	view.camDist = 100.0;
	view.camOffset.x = 0.0;
	view.camOffset.y = 0.0;
	view.camOffset.z = 0.0;
	view.vLeft = 0.0;
	view.vRight = 0.0;
	view.vTop = 0.0;
	view.vBottom = 0.0;
	view.name = NULL;
	view.performXY = XYZ_NONE;
	showIndex = false;
	showVertex = false;
	showNormal = false;
	showUV = false;
	showRule = true;
	showLeak = false;
	showHit = false;
	showLine = false;
	showVolume = false;
	showTexture = false;
	showHidden = false;
	showHiddenVertex = true;
	showMesh = false;
	showDir = true;
	autoScaleOn = false;
	mode = MODE_SELECT;
	showBack = SHOW_FRONTANDBACK;
	showFilter = false;
	//showColormap = true;
	hideLot = 500;

	showTP = true;
#ifdef MOLFLOW
	showTime = false;
#endif

#ifdef SYNRAD
	shadeLines = true;
	dispNumTraj = 500;
#endif
	camDistInc = 1.0;
	transStep = 1.0;
	angleStep = 0.005;
	selX1 = 0;
	selY1 = 0;
	selX2 = 0;
	selY2 = 0;
	selectionChange = false;
	vectorLength = 5.0f;
	arrowLength = 1.0;
	dispNumHits = 2048;
	dispNumLeaks = 2048;

	// GL Component default
	SetBorder(BORDER_NONE);
	int bgCol = (false) ? 255 : 0; //not necessary?
	SetBackgroundColor(bgCol, bgCol, bgCol);

	// Components
	toolBack = new GLLabel("");
	toolBack->SetBackgroundColor(220, 220, 220);
	toolBack->SetOpaque(true);
	toolBack->SetBorder(BORDER_BEVEL_IN);
	Add(toolBack);
	coordLab = new GLLabel("");
	coordLab->SetBackgroundColor(220, 220, 220);
	Add(coordLab);

	facetSearchState = new GLLabel("");
	//facetSearchState->SetBackgroundColor(220,220,220);
	facetSearchState->SetVisible(false);
	Add(facetSearchState);

	//debugLabel = new GLLabel("Debug");
	//Add(debugLabel);

	frontBtn = new GLButton(0, "Front");
	frontBtn->SetToggle(true);
	Add(frontBtn);
	topBtn = new GLButton(0, "Top");
	topBtn->SetToggle(true);
	Add(topBtn);
	sideBtn = new GLButton(0, "Side");
	sideBtn->SetToggle(true);
	Add(sideBtn);

	projCombo = new GLCombo(0);
	projCombo->SetSize(2);
	projCombo->SetValueAt(0, "Persp.");
	projCombo->SetValueAt(1, "Ortho.");
	projCombo->SetSelectedIndex(1);
	Add(projCombo);

#ifdef MOLFLOW
	timeLabel = new GLOverlayLabel("");
	timeLabel->SetTextColor(255, 255, 255);
	Add(timeLabel);
#endif

#ifdef SYNRAD
	selTrajBtn = new GLButton(0, "");
	selTrajBtn->SetIcon("images/icon_traj_select.png");
	Add(selTrajBtn);
#endif

	zoomBtn = new GLButton(0, "");
	zoomBtn->SetIcon("images/icon_zoom.png");
	zoomBtn->SetDisabledIcon("images/icon_zoomd.png");
	zoomBtn->SetEnabled(false);
	Add(zoomBtn);
	sysBtn = new GLButton(0, "");
	sysBtn->SetIcon("images/icon_winup.png");
	Add(sysBtn);
	handBtn = new GLButton(0, "");
	handBtn->SetIcon("images/icon_hand.png");
	Add(handBtn);
	selBtn = new GLButton(0, "");
	selBtn->SetIcon("images/icon_arrow.png");
	Add(selBtn);
	selVxBtn = new GLButton(0, "");
	selVxBtn->SetIcon("images/icon_vertex_select.png");
	Add(selVxBtn);

	autoBtn = new GLButton(0, "");
	autoBtn->SetIcon("images/icon_autoscale.png");
	autoBtn->SetToggle(true);
	Add(autoBtn);
		
	capsLockLabel = new GLLabel("CAPS LOCK On: select facets only with selected vertex");
	Add(capsLockLabel);

	hideLotlabel = new GLLabel("Large number of selected facets: normals, \201 \202 and vertices hidden");
	Add(hideLotlabel);

	screenshotLabel = new GLLabel("Screenshot: Draw selection rectangle to capture box. Press CTRL+R again to capture whole scene. ESC to cancel");
	Add(screenshotLabel);

	selectLabel = new GLLabel("Selection mode: hold SPACE to move anchor, hold ALT to use circle, hold TAB to invert facet/vertex mode, hold SHIFT/CTRL to add/remove to existing selection.");
	Add(selectLabel);

	rotateLabel = new GLLabel("Rotation mode: hold SHIFT to slow down rotation, hold CTRL to rotate around the third axis, and hold ALT to rotate lighting direction of volume view");
	Add(rotateLabel);

	panLabel = new GLLabel("Panning mode: hold SHIFT to slow down panning");
	Add(panLabel);

	tabLabel = new GLLabel("TAB key down: facet/vertex selection mode swapped");
	Add(tabLabel);
	
	nonPlanarLabel = new GLLabel("Your geometry has null, non-simple or non-planar facets, causing leaks.");
	Add(nonPlanarLabel);

	UpdateLabelColors();

	// Light
	glShadeModel(GL_SMOOTH);

	GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

	GLfloat ambientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 0.0f };
	GLfloat specularLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	GLfloat ambientLight2[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat diffuseLight2[] = { 0.15f, 0.15f, 0.15f, 0.0f };
	GLfloat specularLight2[] = { 0.3f, 0.3f, 0.3f, 0.3f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glDisable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight2);
	glDisable(GL_LIGHT1);
}

void GeometryViewer::UpdateLabelColors()
{
	int textColor = mApp->whiteBg ? 0 : 255;
	capsLockLabel->SetTextColor(textColor, textColor, textColor);
	hideLotlabel->SetTextColor(textColor, textColor, textColor);
	screenshotLabel->SetTextColor(textColor, textColor, textColor);
	selectLabel->SetTextColor(textColor, textColor, textColor);
	rotateLabel->SetTextColor(textColor, textColor, textColor);
	panLabel->SetTextColor(textColor, textColor, textColor);
	tabLabel->SetTextColor(textColor, textColor, textColor);
	nonPlanarLabel->SetTextColor(255, 0, 255);
}

void GeometryViewer::ToOrigo() {
	//view.projMode = PERSPECTIVE_PROJ;
	view.camAngleOx = 0.0;
	view.camAngleOy = (view.projMode == PERSPECTIVE_PROJ) ? PI : 0.0;
	view.camAngleOz = 0.0;

	view.camDist = 100.0;
	view.camOffset.x = 0.0;
	view.camOffset.y = 0.0;
	view.camOffset.z = 0.0;

	view.vLeft = 0.0;
	view.vRight = 0.0;
	view.vTop = 0.0;
	view.vBottom = 0.0;
}

bool GeometryViewer::IsSelected() {
	return selected;
}

void GeometryViewer::SetSelected(bool s) {
	selected = s;
}

void GeometryViewer::SetFocus(bool focus) {
	if (focus && parent)  parent->ProcessMessage(this, MSG_GEOMVIEWER_SELECT);
	GLComponent::SetFocus(focus);
}

void GeometryViewer::UpdateMouseCursor(int mode) { //Sets mouse cursor to action

	this->mode = mode;

	if (!parent) return;

	if (draggMode == DRAGG_ROTATE) {
		SetCursor(CURSOR_ROTATE);
	}
	else if (draggMode == DRAGG_MOVE) {
		SetCursor(CURSOR_HAND);
	}
	else {
		if ((mode == MODE_SELECT && !GetWindow()->IsTabDown()) || (mode == MODE_SELECTVERTEX && GetWindow()->IsTabDown())) {
			if (GetWindow()->IsCtrlDown()) {
				SetCursor(CURSOR_SELDEL);
			}
			else if (GetWindow()->IsShiftDown()) {
				SetCursor(CURSOR_SELADD);
			}
			/*else if (GetWindow()->IsAltDown()) {
				SetCursor(CURSOR_HAND);
			}*/ //Disabling ALT-zoom for circular selection
			else {
				SetCursor(CURSOR_DEFAULT);
			}

		}

		else if ((mode == MODE_SELECTVERTEX && !GetWindow()->IsTabDown()) || (mode == MODE_SELECT && GetWindow()->IsTabDown())) {
			if (GetWindow()->IsCtrlDown()) {
				SetCursor(CURSOR_VERTEX_CLR);
			}
			else if (GetWindow()->IsShiftDown()) {
				SetCursor(CURSOR_VERTEX_ADD);
			}
			/*else if (GetWindow()->IsAltDown()) {
				SetCursor(CURSOR_HAND);
			}*/ //Disabling ALT-zoom for circular selection

			else {
				SetCursor(CURSOR_VERTEX);
			}

		}

#ifdef SYNRAD
		else if (mode == MODE_SELECTTRAJ) {
			if (GetWindow()->IsAltDown()) {
				SetCursor(CURSOR_HAND);
			}
			else {
				SetCursor(CURSOR_TRAJ);
			}
		}
#endif

		else if (mode == MODE_ZOOM) {
			SetCursor(CURSOR_ZOOM);
		}
		else if (mode == MODE_MOVE) {
			SetCursor(CURSOR_HAND);
		}
	}
}

bool GeometryViewer::IsDragging() {
	return draggMode != DRAGG_NONE;
}

void GeometryViewer::ToTopView() {

	if (!work) return;
	if (view.projMode == ORTHOGRAPHIC_PROJ) {
		view.camAngleOx = -.5 * PI;
		view.camAngleOy = /*mApp->leftHandedView ? PI :*/ 0.0;
		view.camAngleOz = 0.0;
		view.performXY = XYZ_TOP;
	}
	else { //Perspective
		view.camAngleOx = .5 * PI;
		view.camAngleOy = PI;
		view.camAngleOz = 0.0;
		view.performXY = XYZ_NONE;
	}
	AutoScale();

}

void GeometryViewer::ToSideView() {

	if (!work) return;

	view.camAngleOx = 0.0;
	view.camAngleOy = PI / 2.0;

	view.camAngleOz = 0.0;

	view.performXY = (view.projMode == PERSPECTIVE_PROJ) ? XYZ_NONE : XYZ_SIDE;
	AutoScale();

}

void GeometryViewer::ToFrontView() {
	ToOrigo();
	if (!work) return;
	//view.camAngleOx = 0.0;
	//view.camAngleOy = 0.0;
	view.performXY = (view.projMode == PERSPECTIVE_PROJ) ? XYZ_NONE : XYZ_FRONT;
	AutoScale();

}

void GeometryViewer::UpdateLight() {

	//double handedness = mApp->leftHandedView ? 1.0 : -1.0;

	float ratio = 1.0f;

	// Update lights
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (view.projMode == PERSPECTIVE_PROJ) {
		glRotated(ToDeg(-view.lightAngleOx), 1.0, 0.0, 0.0);
		glRotated(ToDeg(-view.lightAngleOy), 0.0, 1.0, 0.0);
		ratio = 1.0f;
	}
	else { //Ortho
		glScaled(1.0, -1.0, -/*handedness **/ 1.0);
		glRotated(ToDeg(-view.lightAngleOx), 1.0, 0.0, 0.0);
		glRotated(ToDeg(-view.lightAngleOy), 0.0, 1.0, 0.0);
		ratio = (float)view.camDist;
	}

	//ratio*=1.4;

	GLfloat d0[4], d1[4];
	d0[0] = 0.8f * ratio;
	d0[1] = 0.8f * ratio;
	d0[2] = 0.8f * ratio;
	d0[3] = 0.0;

	/*d0[0] = 0.8f;
	d0[1] = 0.8f;
	d0[2] = 0.8f;
	d0[3] = 1.0;*/

	d1[0] = 0.2f * ratio;
	d1[1] = 0.2f * ratio;
	d1[2] = 0.2f * ratio;
	d1[3] = 0.0f;

	if (showBack == SHOW_FRONT) {
		glLightfv(GL_LIGHT0, GL_DIFFUSE, d0);
		glLightfv(GL_LIGHT0, GL_POSITION, positionI);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, d1);
		glLightfv(GL_LIGHT1, GL_POSITION, position);
	}
	else {
		glLightfv(GL_LIGHT0, GL_DIFFUSE, d0);
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, d1);
		glLightfv(GL_LIGHT1, GL_POSITION, positionI);
	}

}

void GeometryViewer::UpdateMatrix() {

	if (!work) return;
	Geometry *geom = work->GetGeometry();
	if (!geom) return;
	double handedness = mApp->leftHandedView ? -1.0 : 1.0;
	double projection = (view.projMode == ORTHOGRAPHIC_PROJ) ? 1.0 : -1.0;
	// Model view matrix ---------------------------------------------------

	// Scale angle in -PI,PI
	view.camAngleOx = RoundAngle(view.camAngleOx);
	view.camAngleOy = RoundAngle(view.camAngleOy);
	view.camAngleOz = RoundAngle(view.camAngleOz);

	// Convert polar coordinates
	Vector3d org = geom->GetCenter();

	/*
	Vector3d X(1.0, 0.0, 0.0);
	Vector3d Y(0.0, 1.0, 0.0);
	Vector3d Z(0.0, 0.0, 1.0);

	camDir = Z;
	camLeft = X * handedness;
	camUp = Y;

	camDir = Rotate(camDir, org, X, -view.camAngleOx);
	camDir = Rotate(camDir, org, Y, -view.camAngleOy);
	camDir = Rotate(camDir, org, Z, -view.camAngleOz);

	camLeft = Rotate(camLeft, org, X, -view.camAngleOx);
	camLeft = Rotate(camLeft, org, Y, -view.camAngleOy);
	camLeft = Rotate(camLeft, org, Z, -view.camAngleOz);

	camUp = Rotate(camUp, org, X, -view.camAngleOx);
	camUp = Rotate(camUp, org, Y, -view.camAngleOy);
	camUp = Rotate(camUp, org, Z, -view.camAngleOz);
	*/

	
	//Original direction towards Z
	double x = -cos(view.camAngleOx) * sin(view.camAngleOy);
	double y = sin(view.camAngleOx);
	double z = cos(view.camAngleOx) * cos(view.camAngleOy);

	//Rotation of cam direction around Z
	camDir.x = x*cos(view.camAngleOz) - y*sin(view.camAngleOz);
	camDir.y = x*sin(view.camAngleOz) + y*cos(view.camAngleOz);
	camDir.z = z;

	//Camleft doesn't take into account camAngleOz...
	camLeft.x = handedness * cos(view.camAngleOy);
	camLeft.y = 0.0;
	camLeft.z = handedness * sin(view.camAngleOy);

	camUp = CrossProduct(camDir, camLeft) * handedness;

	glMatrixMode(GL_MODELVIEW);

	switch (view.projMode) {
	case PERSPECTIVE_PROJ:
	{
		Vector3d camPos = org + view.camOffset;
		GLToolkit::LookAt(camDir * view.camDist + camPos, camPos, camUp, handedness);
		break;
	}
	case ORTHOGRAPHIC_PROJ:
		glLoadIdentity();
		glScaled(-handedness * view.camDist, -view.camDist, -view.camDist);

		glRotated(ToDeg(view.camAngleOx), 1.0, 0.0, 0.0);
		glRotated(ToDeg(view.camAngleOy), 0.0, 1.0, 0.0);
		glRotated(ToDeg(view.camAngleOz), 0.0, 0.0, 1.0);

		glTranslated(-(org.x + view.camOffset.x), -(org.y + view.camOffset.y), -(org.z + view.camOffset.z));
		break;
	}

	glGetFloatv(GL_MODELVIEW_MATRIX, matView);

	// Projection matrix ---------------------------------------------------

	double aspect = (double)width / (double)(height - DOWN_MARGIN);
	ComputeBB(/*true*/);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (view.projMode == PERSPECTIVE_PROJ) {

		double _zNear = Max(zNear, 0.1);
		double _zFar = (_zNear < zFar) ? zFar : _zNear + 1.0;
		GLToolkit::PerspectiveLH(FOV_ANGLE, aspect, _zNear - 0.05, _zFar + 10.0);

	}
	else {

		// 30% margin for extra geometry
		double l = zFar - zNear;
		if ((l > 0.0) && (view.vRight - view.vLeft) > 0.0 && (view.vBottom - view.vTop) > 0.0)
			glOrtho(view.vLeft, view.vRight, view.vBottom, view.vTop, -zFar - l*0.3, -zNear + l*0.3);

	}

	glGetFloatv(GL_PROJECTION_MATRIX, matProj);

}

double GeometryViewer::ToDeg(double radians) {
	return (radians / PI)*180.0f;
}

bool GeometryViewer::SelectionChanged() {
	bool ret = selectionChange;
	selectionChange = false;
	return ret;
}

AVIEW GeometryViewer::GetCurrentView() {
	return view;
}

void GeometryViewer::SetCurrentView(AVIEW v) {

	if (!work) return;
	view = v;

	if (view.projMode == ORTHOGRAPHIC_PROJ) {

		// Rescale viewport (aspect ratio correction + recenter)
		double wA = (double)width / (double)(height - DOWN_MARGIN);
		double vA = (view.vRight - view.vLeft) / (view.vBottom - view.vTop);
		double AA = vA / wA;
		double c = (view.vTop + view.vBottom) / 2.0;
		double l = AA * fabs(view.vTop - view.vBottom) / 2.0;
		view.vTop = c - l;
		view.vBottom = c + l;

	}

	UpdateMatrix();
	projCombo->SetSelectedIndex(view.projMode);
	zoomBtn->SetEnabled(view.performXY != 0);
	//UpdateMouseCursor(MODE_SELECT);
	UpdateMouseCursor(mode);

}

void GeometryViewer::SetProjection(int mode) {
	view.projMode = mode;
	projCombo->SetSelectedIndex(mode);
	ToFrontView();
}

void GeometryViewer::SetWorker(Worker *w) {
	work = w;
	ToFrontView();
	// Auto size vector length (consider Front View)
	Geometry *geom = work->GetGeometry();
	AxisAlignedBoundingBox bb = geom->GetBB();
	vectorLength = Max((bb.max.x - bb.min.x), (bb.max.y - bb.min.y)) / 3.0;
	arrowLength = 10.0 / vectorLength;//Max((bb.max.z-bb.min.z),vectorLength);
}

void GeometryViewer::DrawIndex() {

	char tmp[256];

	// Draw index number
		// Get selected vertex
	Geometry *geom = work->GetGeometry();
	size_t nbVertex = geom->GetNbVertex();
	std::vector<size_t> selectedFacets = geom->GetSelectedFacets();
	if (nbVertex <= 0) return;

	//Mark vertices of selected facets
	std::vector<bool> vertexOnSelectedFacet(nbVertex, false);
	std::vector<size_t> vertexId(nbVertex);
	for (auto& selId:selectedFacets) {
		Facet *f = geom->GetFacet(selId);
			for (size_t i = 0; i < f->sh.nbIndex; i++) {
				vertexOnSelectedFacet[f->indices[i]] = true;
				vertexId[f->indices[i]] = i;
			}
	}

	// Draw dot
	glPointSize(4.0f);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	glColor3f(1.0f, 0.2f, 0.2f);

	glBegin(GL_POINTS);
	for (size_t i = 0; i < nbVertex; i++) {
		if (vertexOnSelectedFacet[i]) {
			Vector3d *v = geom->GetVertex(i);
			glVertex3d(v->x, v->y, v->z);
		}
	}
	glEnd();

	// Save context
	GLToolkit::DrawStringInit();
	GLToolkit::GetDialogFont()->SetTextColor(0.5f, 0.9f, 0.9f);

	// Draw Labels
	for (size_t i = 0; i < nbVertex; i++) {
		if (vertexOnSelectedFacet[i]) {
			if (showIndex && showVertex) {
				sprintf(tmp, "%zd,%zd ", vertexId[i] + 1, i + 1);
			}
			else if (showIndex && !showVertex) {
				sprintf(tmp, "%zd ", vertexId[i] + 1);
			}
			else {
				sprintf(tmp, "%zd ", i + 1);
			}
			Vector3d *v = geom->GetVertex(i);
			GLToolkit::DrawString((float)v->x, (float)v->y, (float)v->z, tmp, GLToolkit::GetDialogFont(), 2, 2);
		}
	}

	//Restore
	GLToolkit::DrawStringRestore();
}

void GeometryViewer::DrawRule() {

	if (showRule) {
		glLineWidth(1.0f);
		// Restore large clipping plane for drawing rules
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (view.projMode == PERSPECTIVE_PROJ) {
			double aspect = (double)width / (double)(height - DOWN_MARGIN);
			GLToolkit::PerspectiveLH(FOV_ANGLE, aspect, 0.05, 1E6);
		}
		else {
			glOrtho(view.vLeft, view.vRight, view.vBottom, view.vTop, -1E6, 1E6);
		}
		glDisable(GL_DEPTH_TEST);
		GLToolkit::SetMaterial(&greenMaterial);
		GLToolkit::DrawRule(vectorLength, false, false, false, arrowLength);
		GLToolkit::GetDialogFontBold()->SetTextColor(0.4f, 0.8f, 0.8f);
		GLToolkit::DrawStringInit();
		GLToolkit::DrawString((float)vectorLength, 0.0f, 0.0f, "x", GLToolkit::GetDialogFontBold());
		GLToolkit::DrawString(0.0f, (float)vectorLength, 0.0f, "y", GLToolkit::GetDialogFontBold());
		GLToolkit::DrawString(0.0f, 0.0f, (float)vectorLength, "z", GLToolkit::GetDialogFontBold());
		GLToolkit::DrawStringRestore();
	}

}

void GeometryViewer::PaintSelectedVertices(bool hiddenVertex) {
	Geometry *geom = work->GetGeometry();
	std::vector<size_t> selectedVertexIds;

	//Populate selected vertices
	for (size_t i = 0; i < geom->GetNbVertex(); i++) {
		if (geom->GetVertex(i)->selected) {
			selectedVertexIds.push_back(i);
		}
	}

	// Draw dot
	if (!mApp->whiteBg) glPointSize(6.0f);
	else glPointSize(7.0f);
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	if (hiddenVertex) glDisable(GL_DEPTH_TEST);
	else glEnable(GL_DEPTH_TEST);
	if (!mApp->whiteBg) glColor3f(1.0f, 0.9f, 0.2f);
	else glColor3f(1.0f, 0.5f, 0.2f);

	glBegin(GL_POINTS);
	for (size_t i : selectedVertexIds) {
		Vector3d *v = geom->GetVertex(i);
		glVertex3d(v->x, v->y, v->z);
	}
	glEnd();

	// Save contexct
	GLToolkit::DrawStringInit();
	if (!mApp->whiteBg) GLToolkit::GetDialogFont()->SetTextColor(1.0f, 0.9f, 0.2f);
	else GLToolkit::GetDialogFont()->SetTextColor(1.0f, 0.2f, 0.0f);

	// Draw Labels
	glEnable(GL_BLEND);
	for (size_t i : selectedVertexIds) {
		Vector3d *v = geom->GetVertex(i);
		GLToolkit::DrawString((float)v->x, (float)v->y, (float)v->z, std::to_string(i + 1).c_str(), GLToolkit::GetDialogFont(), 2, 2);
	}
	glDisable(GL_BLEND);
	//Restore
	GLToolkit::DrawStringRestore();
	if (hiddenVertex) glEnable(GL_DEPTH_TEST);

}

void GeometryViewer::DrawNormal() {
	glLineWidth(2.0f);
	glPointSize(3.0f);
	glColor3f(1.0f, 0.0f, 0.0f);

	Geometry *geom = work->GetGeometry();
	for (int i = 0; i < geom->GetNbFacet(); i++) {
		Facet *f = geom->GetFacet(i);
		if (f->selected) {
			Vector3d v1 = geom->GetFacetCenter(i);
			Vector3d v2 = f->sh.N;
			GLToolkit::SetMaterial(&blueMaterial);
			
			GLToolkit::DrawVector(v1.x, v1.y, v1.z, v1.x + v2.x*vectorLength, v1.y + v2.y*vectorLength, v1.z + v2.z*vectorLength, arrowLength);
			
			glBegin(GL_POINTS);
			glVertex3d(v1.x, v1.y, v1.z);
			glEnd();
		}
	}
	glLineWidth(1.0f);
}

void GeometryViewer::DrawUV() {
	Geometry *geom = work->GetGeometry();
	for (int i = 0; i < geom->GetNbFacet(); i++) {
		Facet *f = geom->GetFacet(i);
		if (f->selected) {
			Vector3d O = f->sh.O;
			Vector3d U = f->sh.U;
			Vector3d V = f->sh.V;
			GLToolkit::SetMaterial(&blueMaterial);
			if (mApp->antiAliasing) {
				glEnable(GL_LINE_SMOOTH);
				//glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				//glColor4f(0.0f,0.0f,1.0f,0.5f);
				glEnable(GL_DEPTH_TEST);
			}
			glLineWidth(1.0f);
			GLToolkit::DrawVector(O.x, O.y, O.z, O.x + U.x, O.y + U.y, O.z + U.z, arrowLength);
			GLToolkit::DrawVector(O.x, O.y, O.z, O.x + V.x, O.y + V.y, O.z + V.z, arrowLength);
			if (mApp->antiAliasing) glDisable(GL_LINE_SMOOTH);
			glPointSize(3.0f);
			glColor3f(0.5f, 1.0f, 1.0f);
			glBegin(GL_POINTS);
			glVertex3d(O.x, O.y, O.z);
			glEnd();
			//glEnable(GL_BLEND);
			GLToolkit::GetDialogFont()->SetTextColor(0.5f, 0.6f, 1.0f);
			GLToolkit::DrawStringInit();
			GLToolkit::DrawString((float)(O.x + U.x), (float)(O.y + U.y), (float)(O.z + U.z), "\201", GLToolkit::GetDialogFont());
			GLToolkit::DrawString((float)(O.x + V.x), (float)(O.y + V.y), (float)(O.z + V.z), "\202", GLToolkit::GetDialogFont());
			GLToolkit::DrawStringRestore();
			//glDisable(GL_BLEND);
		}
	}
}

void GeometryViewer::DrawLeak() {

	// Draw leak
	if (showLeak) {

		glPointSize(4.0f);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glEnable(GL_LINE_SMOOTH);
		for (size_t i = 0; i < Min(dispNumLeaks,mApp->worker.globalHitCache.leakCacheSize); i++) {

			Vector3d p = mApp->worker.globalHitCache.leakCache[i].pos;
			Vector3d d = mApp->worker.globalHitCache.leakCache[i].dir;

			glColor3f(0.9f, 0.2f, 0.5f);
			glBegin(GL_POINTS);
			glVertex3d(p.x, p.y, p.z);
			glEnd();

			GLToolkit::DrawVector(p.x, p.y, p.z,
				p.x + d.x*vectorLength, p.y + d.y*vectorLength, p.z + d.z*vectorLength, arrowLength);

		}
		glDisable(GL_LINE_SMOOTH);
	}

}

void GeometryViewer::AutoScale(bool reUpdateMouseCursor) {

	if (!work) return;
	Geometry *geom = work->GetGeometry();
	if (!geom) return;

	double aspect = (double)width / (double)(height - DOWN_MARGIN);
	if (aspect == 0.0) aspect = 1.0; //To avoid division by zero
	Vector3d org = geom->GetCenter();

	// Reset offset, zoom
	view.camOffset.x = 0.0;
	view.camOffset.y = 0.0;
	view.camOffset.z = 0.0;
	view.camDist = 1.0f;
	UpdateMatrix();

	// Get geometry transformed BB
	ComputeBB(/*false*/);

	Vector3d v;
	v.x = xMax - org.x;
	v.y = yMax - org.y;
	v.z = zFar - org.z;
	camDistInc = v.Norme() / 100.0;
	view.camOffset.x = 0.0;
	view.camOffset.y = 0.0;
	view.camOffset.z = 0.0;
	if (view.projMode == PERSPECTIVE_PROJ) {

		// Autoscale dist, at least try to ;) .stub.
		double a = 0.5 / tan((FOV_ANGLE / 360.0)*PI);
		view.camDist = Max((xMax - xMin) / aspect,
			(yMax - yMin)) * a
			+ (zFar - zNear) / 1.9;

	}
	else {

		// Scale
		view.camDist = 1.0;

		double mDist = Max((xMax - xMin), (yMax - yMin)*aspect);
		mDist *= 1.1; // 10% margin
		double dx = (xMax + xMin) / 2.0;
		double dy = (yMax + yMin) / 2.0;
		view.vLeft = dx - mDist / 2.0;
		view.vRight = dx + mDist / 2.0;
		view.vTop = dy - mDist / (2.0*aspect);
		view.vBottom = dy + mDist / (2.0*aspect);

	}

	UpdateMatrix();
	zoomBtn->SetEnabled(view.performXY != XYZ_NONE);
	//if(reUpdateMouseCursor) UpdateMouseCursor(MODE_SELECT);
	if (reUpdateMouseCursor) UpdateMouseCursor(mode);

}

void GeometryViewer::Zoom() {

	if (std::abs(selX1 - selX2) >= 2 && std::abs(selY1 - selY2) >= 2 && work) {

		int wS = std::abs(selX1 - selX2);
		int hS = std::abs(selY1 - selY2);
		double sAspect = (double)wS / (double)hS;
		double aspect = (double)width / (double)(height - DOWN_MARGIN);
		double x0, y0, w0, h0;
		double dx = 0.0, dy = 0.0, dz = 0.0;
		Vector3d org = work->GetGeometry()->GetCenter();
		double z;

		if (hS > wS) {
			w0 = (double)hS*aspect;
			h0 = (double)hS;
			x0 = ((double)wS - w0) / 2.0f + (double)Min(selX1, selX2) + w0 / 2.0;
			y0 = (double)Min(selY1, selY2) + h0 / 2.0;
			z = (double)(height - DOWN_MARGIN) / h0;
		}
		else {
			w0 = (double)wS;
			h0 = (double)wS / aspect;
			x0 = (double)Min(selX1, selX2) + w0 / 2.0;
			y0 = ((double)hS - h0) / 2.0f + (double)Min(selY1, selY2) + h0 / 2.0;
			z = (double)width / w0;
		}

		x0 -= (double)posX;
		y0 -= (double)posY;
		
		double handedness = mApp->leftHandedView ? 1.0 : -1.0;
		switch (view.performXY) {
			
		case XYZ_TOP: // TopView
			dx = -handedness*(0.5 - x0 / (double)width)  * (view.vRight - view.vLeft);
			dz = (0.5 - y0 / (double)(height - DOWN_MARGIN)) * (view.vBottom - view.vTop);
			break;
		case XYZ_SIDE: // Side View
			dz = -handedness*(0.5 - x0 / (double)width) * (view.vRight - view.vLeft);
			dy = (0.5 - y0 / (double)(height - DOWN_MARGIN)) * (view.vBottom - view.vTop);
			break;
		case XYZ_FRONT: // Front View
			dx = handedness*(-0.5 + x0 / (double)width)  * (view.vRight - view.vLeft);
			dy = (0.5 - y0 / (double)(height - DOWN_MARGIN)) * (view.vBottom - view.vTop);
			break;
		}

		view.camOffset.x += dx / view.camDist;
		view.camOffset.y += dy / view.camDist;
		view.camOffset.z += dz / view.camDist;
		view.camDist *= z;

		UpdateMatrix();

	}

}

void GeometryViewer::Paint() {
	
	char tmp[256];

	if (!parent) return;
	GLComponent::Paint();

	//Background gradient
	int x, y, width, height;
	((GLComponent*)this)->GetBounds(&x, &y, &width, &height);
	
		glBegin(GL_QUADS);
		if (mApp->whiteBg) {
			glColor3f(1.0f, 1.0f, 1.0f);
		}
		else {
#ifdef MOLFLOW
			glColor3f(0.3f, 0.5f, 0.7f); //blue top
#endif

#ifdef SYNRAD
			glColor3f(0.7f, 0.4f, 0.3f); //red top
#endif
		}
		glVertex2i(x, y);
		glVertex2i(x + width, y);

		if (!mApp->whiteBg) {
			glColor3f(0.05f, 0.05f, 0.05f); //grey bottom
		}
		glVertex2i(x + width, y + height);
		glVertex2i(x, y + height);

		glEnd();
	

	if (!work) return;
	Geometry *geom = work->GetGeometry();
	if (!geom->IsLoaded()) {
		PaintCompAndBorder();
		return;
	}
	sprintf(tmp, "%s", "");
	topBtn->SetState(false);
	frontBtn->SetState(false);
	sideBtn->SetState(false);
	if (view.performXY) {
		// Draw coordinates on screen when aligned
		Vector3d org = geom->GetCenter();
		double x, y, z;
		double handedness = mApp->leftHandedView ? 1.0 : -1.0;
		switch (view.performXY) {
		case XYZ_TOP: // TopView
			x = -handedness * (-view.vLeft - (1.0 - (double)mXOrg / (double)width) * (view.vRight - view.vLeft) + (handedness * org.x + view.camOffset.x)*view.camDist);
			z = -view.vTop - ((double)mYOrg / (double)(height - DOWN_MARGIN)) * (view.vBottom - view.vTop) + (org.z + view.camOffset.z)*view.camDist;
			sprintf(tmp, "X=%g, Z=%g", -x / view.camDist, z / view.camDist);
			topBtn->SetState(true);
			break;
		case XYZ_SIDE: // Side View
			z = view.vLeft + ((double)mXOrg / (double)width) * (view.vRight - view.vLeft) + (org.z + view.camOffset.z)*view.camDist;
			y = -view.vTop - ((double)mYOrg / (double)(height - DOWN_MARGIN)) * (view.vBottom - view.vTop) + (org.y + view.camOffset.y)*view.camDist;
			sprintf(tmp, "Z=%g, Y=%g", z / view.camDist, y / view.camDist);
			sideBtn->SetState(true);
			break;
		case XYZ_FRONT: // Front View
			x =  handedness * (-view.vLeft - (1.0 - (double)mXOrg / (double)width) * (view.vRight - view.vLeft) + (org.x + view.camOffset.x)*view.camDist);
			y = -view.vTop - ((double)mYOrg / (double)(height - DOWN_MARGIN)) * (view.vBottom - view.vTop) + (org.y + view.camOffset.y)*view.camDist;
			sprintf(tmp, "X=%g, Y=%g", x / view.camDist, y / view.camDist);
			frontBtn->SetState(true);
			break;
		}
	}
	coordLab->SetText(tmp);

	// Clipping and projection matrix
	GetWindow()->Clip(this, 0, 0, 0, DOWN_MARGIN);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(matProj);
	UpdateLight();
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(matView);
	glDisable(GL_BLEND);

	// Draw geometry
	if (showVolume || showTexture) glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_LEQUAL);
	
	/*if (view.projMode == ORTHOGRAPHIC_PROJ) {
		if (mApp->leftHandedView) {
			//Ortho + left-handed
			
			//glDepthRange(0, 1); //chg recently
		}
		else {
			//Ortho + right-handed
			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);
			//glDepthRange(0, 1);
			glDepthFunc(GL_LEQUAL);
		}
	}
	else {
		if (mApp->leftHandedView) {
			//Persp + left-handed
			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_LEQUAL);
			//glDepthRange(1, 0);
		}
		else {
			//Persp + right-handed
			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_LEQUAL);
		}
	}*/
	
	/*
	// Draw geometry
if( showVolume || showTexture ) {

  glEnable(GL_DEPTH_TEST);

  if(view.projMode==PERSPECTIVE_PROJ) {
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	//glPolygonOffset( -0.5f, -0.002f );
  } else {
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	//glDepthFunc(GL_GEQUAL);
	//glPolygonOffset( 0.5f, 0.002f );
  }

} else {

  glDisable(GL_DEPTH_TEST);

}*/

	int bgCol = (mApp->whiteBg) ? 255 : 0;
	SetBackgroundColor(bgCol, bgCol, bgCol);
	DrawLinesAndHits();
	int cullMode;
	if (showBack != SHOW_FRONTANDBACK && !mApp->leftHandedView) {
		//Right-handed coord system: front and back inverse
		if (showBack == SHOW_BACK) cullMode = SHOW_FRONT;
		else cullMode = SHOW_BACK;
	} else cullMode = showBack;
	geom->Render((GLfloat *)matView, showVolume, showTexture, cullMode, showFilter, showHidden, showMesh, showDir);
#ifdef SYNRAD
	for (size_t i = 0; i < work->regions.size(); i++)
		work->regions[i].Render((int)i, dispNumTraj, &blueMaterial, vectorLength);
#endif

	bool detailsSuppressed = hideLot != -1 && (geom->GetNbSelectedFacets() > hideLot);
	bool displayWarning = (showIndex || showVertex || showNormal || showUV) && detailsSuppressed;
	if ((showIndex || showVertex) && (!detailsSuppressed)) DrawIndex();
	if (showNormal && (!detailsSuppressed)) DrawNormal();
	if (showUV && (!detailsSuppressed)) DrawUV();
	DrawLeak();
	GLToolkit::CheckGLErrors("GLLabel::Paint()");
	DrawRule();
	GLToolkit::CheckGLErrors("GLLabel::Paint()");
	PaintSelectedVertices(showHiddenVertex);
	//DrawBB();
	// Restore old transformation/viewport
	GetWindow()->ClipToWindow();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	GLWindowManager::SetDefault();
	// Draw selection rectangle or circle
	bool displaySelectionRectangle = (draggMode == DRAGG_SELECT || draggMode == DRAGG_SELECTVERTEX)
		&& (mode == MODE_SELECT || mode == MODE_SELECTVERTEX || mode == MODE_ZOOM)
		&& (selX1 != selX2) && (selY1 != selY2);
	if (displaySelectionRectangle) {
		bool circleMode = GetWindow()->IsAltDown();
		GLushort dashPattern = 0xCCCC;

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glColor3f(1.0f, 0.8f, 0.9f);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, dashPattern);

		if (!circleMode) { //normal rectangle
			glBegin(GL_LINE_LOOP);
			_glVertex2i(selX1, selY1);
			_glVertex2i(selX1, selY2);
			_glVertex2i(selX2, selY2);
			_glVertex2i(selX2, selY1);
			glEnd();
		}
		else { //draw circle
			glBegin(GL_POINTS);
			glVertex2i(selX1, selY1);
			glEnd();
			glBegin(GL_LINE_LOOP);
			glVertex2i(selX1, selY1);
			glVertex2i(selX2, selY2);
			glEnd();
			glBegin(GL_LINE_LOOP);
			float DEG2RAD = (float)(3.14159 / 180.0);
			float radius = sqrt(pow((float)(selX1 - selX2), 2) + pow((float)(selY1 - selY2), 2));

			for (int i = 0; i <= 360; i += 2) {
				float degInRad = i * DEG2RAD;
				glVertex2f(selX1 + cos(degInRad)*radius, selY1 + sin(degInRad)*radius);
			}
			glEnd();
		}

		glDisable(GL_LINE_STIPPLE);

	}

	//Status labels
	//From bottom to up
	bool displayHideLotLabel = displayWarning;
	bool displayCapsLockLabel = GetWindow()->IsCapsLockOn();
	bool displayRotateLabel = draggMode == DRAGG_ROTATE;
	bool displayScreenshotLabel = screenshotStatus.requested > 0;
	bool displaySelectionLabel = displaySelectionRectangle;
	bool displayPanLabel = draggMode == DRAGG_MOVE;
	bool displayTabLabel = GetWindow()->IsTabDown();
	bool displayNonPlanarLabel = geom->hasNonPlanar;
	int offsetCount = 0;
	hideLotlabel->SetBounds(posX + 10, posY + height - 47 - 20*offsetCount, 0, 19); offsetCount += (int)displayHideLotLabel;hideLotlabel->SetVisible(displayHideLotLabel);
	capsLockLabel->SetBounds(posX + 10, posY + height - 47 - 20 * offsetCount, 0, 19); offsetCount += (int)displayCapsLockLabel;capsLockLabel->SetVisible(displayCapsLockLabel);
	rotateLabel->SetBounds(posX + 10, posY + height - 47 - 20 * offsetCount, 0, 19); offsetCount += (int)displayRotateLabel;rotateLabel->SetVisible(displayRotateLabel);
	screenshotLabel->SetBounds(posX + 10, posY + height - 47 - 20 * offsetCount, 0, 19); offsetCount += (int)displayScreenshotLabel;screenshotLabel->SetVisible(displayScreenshotLabel);
	selectLabel->SetBounds(posX + 10, posY + height - 47 - 20 * offsetCount, 0, 19); offsetCount += (int)displaySelectionLabel;selectLabel->SetVisible(displaySelectionLabel);
	panLabel->SetBounds(posX + 10, posY + height - 47 - 20 * offsetCount, 0, 19); offsetCount += (int)displayPanLabel;panLabel->SetVisible(displayPanLabel);
	tabLabel->SetBounds(posX + 10, posY + height - 47 - 20 * offsetCount, 0, 19); offsetCount += (int)displayTabLabel;tabLabel->SetVisible(displayTabLabel);
	nonPlanarLabel->SetBounds(posX + 10, posY + height - 47 - 20 * offsetCount, 0, 19); offsetCount += (int)displayNonPlanarLabel;nonPlanarLabel->SetVisible(displayNonPlanarLabel);

#ifdef MOLFLOW
	if (work->displayedMoment)
		sprintf(tmp, "t= %g s", work->moments[work->displayedMoment - 1]);
	else
		sprintf(tmp, "Const. flow");
	timeLabel->SetText(tmp);
	timeLabel->SetVisible(showTime);
#endif
	
	if (screenshotStatus.requested >= 2) {
		Screenshot();
	}
	PaintCompAndBorder();
	
}

void GeometryViewer::PaintCompAndBorder() {
	// Components
	PaintComponents();
	// Border
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	if (selected) {

		glColor3f(0.5f, 0.5f, 1.0f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		_glVertex2i(posX, posY);
		_glVertex2i(posX + width, posY);
		_glVertex2i(posX, posY);
		_glVertex2i(posX, posY + height);
		_glVertex2i(posX + width, posY + height);
		_glVertex2i(posX, posY + height);
		_glVertex2i(posX + width, posY + height);
		_glVertex2i(posX + width, posY - 1);
		glEnd();
		glLineWidth(1.0f);

	}
	else {

		glColor3f(0.5f, 0.5f, 0.5f);
		glBegin(GL_LINES);
		_glVertex2i(posX, posY);
		_glVertex2i(posX + width, posY);
		_glVertex2i(posX, posY);
		_glVertex2i(posX, posY + height);
		glEnd();
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		_glVertex2i(posX + width, posY + height);
		_glVertex2i(posX, posY + height);
		_glVertex2i(posX + width, posY + height);
		_glVertex2i(posX + width, posY - 1);
		glEnd();

	}

}

void GeometryViewer::TranslateScale(double diff) {

	// Translate or Scale view according to the projection mode
	if (view.projMode == PERSPECTIVE_PROJ) {
		view.camDist += diff * camDistInc;
		if (view.camDist < 0.01) view.camDist = 0.01;
	}
	else {
		double r = 1.0 - (diff / 100.0);
		if (r > 0.01) view.camDist *= r;
	}
	UpdateMatrix();

}

void GeometryViewer::ManageEvent(SDL_Event *evt)
{

	if (!work) return;
	Geometry *geom = work->GetGeometry();
	// Key pressed
	if (evt->type == SDL_KEYDOWN) {
		int unicode = /*(evt->key.keysym.unicode & 0x7F);
		if (!unicode) unicode =*/ evt->key.keysym.sym;
		
		double handedness = mApp->leftHandedView ? 1.0 : -1.0;
		if (unicode == SDLK_UP) {
			if (GetWindow()->IsShiftDown()) {
				view.camAngleOx += angleStep;
				view.performXY = XYZ_NONE;
			}
			else if (GetWindow()->IsCtrlDown()) {
				// Up
				view.camOffset.x += transStep * camUp.x;
				view.camOffset.y += transStep * camUp.y;
				view.camOffset.z += transStep * camUp.z;
			}
			else {
				if (view.projMode == PERSPECTIVE_PROJ) {
					// Forward
					view.camOffset.x -= transStep * camDir.x;
					view.camOffset.y -= transStep * camDir.y;
					view.camOffset.z -= transStep * camDir.z;
				}
				else {
					// Up
					
					view.camOffset.x += transStep * camUp.x;
					view.camOffset.y += transStep * camUp.y;
					view.camOffset.z += handedness * transStep * camUp.z;
				}
			}
			UpdateMatrix();
			autoScaleOn = false;
			autoBtn->SetState(false);
		}

		if (unicode == SDLK_DOWN) {
			if (GetWindow()->IsShiftDown()) {
				view.camAngleOx -= angleStep;
				view.performXY = XYZ_NONE;
			}
			else if (GetWindow()->IsCtrlDown()) {
				// Down
				view.camOffset.x -= transStep * camUp.x;
				view.camOffset.y -= transStep * camUp.y;
				view.camOffset.z -= transStep * camUp.z;
			}
			else {
				if (view.projMode == PERSPECTIVE_PROJ) {
					// Backward
					view.camOffset.x += transStep * camDir.x;
					view.camOffset.y += transStep * camDir.y;
					view.camOffset.z += transStep * camDir.z;
				}
				else {
					// Down
					view.camOffset.x -= transStep * camUp.x;
					view.camOffset.y -= transStep * camUp.y;
					view.camOffset.z -= handedness * transStep * camUp.z;
				}
			}
			UpdateMatrix();
			autoScaleOn = false;
			autoBtn->SetState(false);
		}

		if (unicode == SDLK_LEFT) {
			double handedness = mApp->leftHandedView ? 1.0 : -1.0;
			double projection = (view.projMode == ORTHOGRAPHIC_PROJ) ? 1.0 : -1.0;
			if (GetWindow()->IsShiftDown()) {
				view.camAngleOy += angleStep * handedness * projection;
				view.performXY = XYZ_NONE;
			}
			else {
				// Strafe left
				view.camOffset.x += transStep * projection * camLeft.x;
				view.camOffset.y += transStep * projection * camLeft.y;
				view.camOffset.z += transStep * projection * camLeft.z;
			}
			UpdateMatrix();
		}

		if (unicode == SDLK_RIGHT) {
			double handedness = mApp->leftHandedView ? 1.0 : -1.0;
			double projection = (view.projMode == ORTHOGRAPHIC_PROJ) ? 1.0 : -1.0;
			if (GetWindow()->IsShiftDown()) {
				view.camAngleOy -= angleStep * handedness * projection;
				view.performXY = XYZ_NONE;
			}
			else {
				// Strafe right
				view.camOffset.x -= transStep * projection * camLeft.x;
				view.camOffset.y -= transStep * projection * camLeft.y;
				view.camOffset.z -= transStep * projection * camLeft.z;
			}
			UpdateMatrix();
			autoScaleOn = false;
			autoBtn->SetState(false);
		}

		if (unicode == SDLK_LCTRL || unicode == SDLK_RCTRL) {
			//UpdateMouseCursor(MODE_SELECT);
			UpdateMouseCursor(mode);
		}

		else if (unicode == SDLK_LSHIFT || unicode == SDLK_RSHIFT) {
			//UpdateMouseCursor(MODE_SELECT);
			UpdateMouseCursor(mode);
		}
		else if (unicode == SDLK_LALT || unicode == SDLK_RALT) {
			//UpdateMouseCursor(MODE_SELECT);
			UpdateMouseCursor(mode);
		}
		else if (unicode == SDLK_TAB) {
			UpdateMouseCursor(mode);
		}
		else if (unicode == SDLK_ESCAPE) {
			screenshotStatus.requested = 0;
		}

		return;
	}

	if (evt->type == SDL_KEYUP) {

		int unicode = evt->key.keysym.sym;

		if (unicode == SDLK_LCTRL || unicode == SDLK_RCTRL) {
			//UpdateMouseCursor(MODE_SELECT);
			UpdateMouseCursor(mode);
		}
		else if (unicode == SDLK_LSHIFT || unicode == SDLK_RSHIFT) {
			//UpdateMouseCursor(MODE_SELECT);
			UpdateMouseCursor(mode);
		}
		else if (unicode == SDLK_LALT || unicode == SDLK_RALT) {
			//UpdateMouseCursor(MODE_SELECT);
			UpdateMouseCursor(mode);
		}
		else if (unicode == SDLK_TAB) {
			UpdateMouseCursor(mode);
		}

		return;

	}

	if (!draggMode) {
		GLContainer::ManageEvent(evt);
		GLContainer::RelayEvent(evt);
		if (evtProcessed) {
			// Latch active cursor
			SetCursor(GLToolkit::GetCursor());
			return;
		}
		else {
			UpdateMouseCursor(mode);
		}
	}

	// (mX,mY) in window coorinates
	int mX = GetWindow()->GetX(this, evt) + posX;
	int mY = GetWindow()->GetY(this, evt) + posY;

	// Handle mouse events
	if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONDBLCLICK) {
		mXOrg = mX;
		mYOrg = mY;

		if (evt->button.button == SDL_BUTTON_LEFT) {
			// Selection dragging
			selX1 = selX2 = mX;
			selY1 = selY2 = mY;
			if (mode == MODE_ZOOM) draggMode = DRAGG_SELECT;
			else if (mode == MODE_SELECT) {
				if (!GetWindow()->IsTabDown()) draggMode = DRAGG_SELECT;
				else draggMode = DRAGG_SELECTVERTEX;
			}
			else if (mode == MODE_SELECTVERTEX) {
				if (!GetWindow()->IsTabDown()) draggMode = DRAGG_SELECTVERTEX;
				else draggMode = DRAGG_SELECT;
			}
#ifdef SYNRAD
			else if (mode == MODE_SELECTTRAJ) draggMode = DRAGG_SELECTTRAJ;
#endif
			else if (mode == MODE_MOVE) draggMode = DRAGG_MOVE;

		}
		if (evt->button.button == SDL_BUTTON_MIDDLE) {
			// Camera translational dragging
			draggMode = DRAGG_MOVE;
			//UpdateMouseCursor(MODE_MOVE);
		}
		if (evt->button.button == SDL_BUTTON_RIGHT) {
			// Camera rotating
			draggMode = DRAGG_ROTATE;
			//UpdateMouseCursor(MODE_MOVE);
		}

		UpdateMouseCursor(mode);
	}

	if (evt->type == SDL_MOUSEWHEEL) {
#ifdef __APPLE__
		int appleInversionFactor = -1; //Invert mousewheel on Apple devices
#else
		int appleInversionFactor = 1;
#endif
		if (evt->wheel.y !=0) { //Vertical scroll
			if (GetWindow()->IsShiftDown()) {
				TranslateScale(-2.0 * evt->wheel.y * appleInversionFactor); //Zoom slower when SHIFT is pressed
			}
			else {
				TranslateScale(-20.0 * evt->wheel.y * appleInversionFactor);
			}
			autoScaleOn = false;
			autoBtn->SetState(false);
		}
	}

	if (evt->type == SDL_MOUSEBUTTONUP) {

		switch (draggMode) {

		case DRAGG_SELECT:
		case DRAGG_SELECTVERTEX:
#ifdef SYNRAD
		case DRAGG_SELECTTRAJ:
#endif

			if (mode == MODE_ZOOM) {
				Zoom();
				autoScaleOn = false;
				autoBtn->SetState(false);
			}
			else if ((mode == MODE_SELECT && !GetWindow()->IsTabDown()) || (mode == MODE_SELECTVERTEX && GetWindow()->IsTabDown())) {
				GetWindow()->Clip(this, 0, 0, 0, DOWN_MARGIN);
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(matProj);
				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixf(matView);
				selectionChange = true;
				if (std::abs(selX1 - selX2) <= 1 && std::abs(selY1 - selY2) <= 1) {
					// Simple click, select/unselect facet
					//SetCursor(CURSOR_BUSY);
					GLToolkit::SetCursor(CURSOR_BUSY);
					geom->Select(mX - posX, mY - posY, !GetWindow()->IsShiftDown(), GetWindow()->IsCtrlDown(), GetWindow()->IsCapsLockOn(), this->width, this->height);
					//UpdateMouseCursor(mode);
				}
				else {
					// Select region
					if (screenshotStatus.requested == 1) {
						int wx, wy, ww, wh;
						this->GetBounds(&wx, &wy, &ww, &wh); wh -= 28;//Toolbar height
						int sx = Min(selX1, selX2)/* - wx*/; sx = Max(sx, 0);
						int sy = Min(selY1, selY2)/* - wy*/; sy = Max(sy, 0);
						int sw = std::abs(selX2 - selX1); sw = Min(sw, ww - sx);
						int sh = std::abs(selY2 - selY1); sh = Min(sh, wh - sy);
						screenshotStatus.x = sx;
						screenshotStatus.y = sy;
						screenshotStatus.w = sw;
						screenshotStatus.h = sh;
						screenshotStatus.requested = 2;
					}
					else {
						geom->SelectArea(selX1 - posX, selY1 - posY, selX2 - posX, selY2 - posY,
							!GetWindow()->IsShiftDown(), GetWindow()->IsCtrlDown(), GetWindow()->IsCapsLockOn(), GetWindow()->IsAltDown());
					}
				}
			}
			else if ((mode == MODE_SELECTVERTEX && !GetWindow()->IsTabDown()) || (mode == MODE_SELECT && GetWindow()->IsTabDown())) {
				GetWindow()->Clip(this, 0, 0, 0, DOWN_MARGIN);
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(matProj);
				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixf(matView);
				//selectionChange = true;
				if (std::abs(selX1 - selX2) <= 1 && std::abs(selY1 - selY2) <= 1) {
					// Simple click, select/unselect vertex
					geom->SelectVertex(mX - posX, mY - posY, GetWindow()->IsShiftDown(), GetWindow()->IsCtrlDown(), GetWindow()->IsCapsLockOn());
					//select closest vertex
				}
				else {
					// Select region
					geom->SelectVertex(selX1 - posX, selY1 - posY, selX2 - posX, selY2 - posY,
						GetWindow()->IsShiftDown(), GetWindow()->IsCtrlDown(), GetWindow()->IsAltDown(), GetWindow()->IsCapsLockOn());
				}
			}
#ifdef SYNRAD
			else if (mode == MODE_SELECTTRAJ) {
				GetWindow()->Clip(this, 0, 0, 0, DOWN_MARGIN);
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(matProj);
				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixf(matView);
				for (size_t i = 0; i < work->regions.size(); i++)
					work->regions[i].SelectTrajPoint(mX - posX, mY - posY, i);
			}
#endif

		}
		draggMode = DRAGG_NONE;
		UpdateMouseCursor(mode); //Sets cursor
	}

	if (evt->type == SDL_MOUSEMOTION) {

		/*
		//debug
		std::stringstream tmp;
		tmp << "MxOrg:" << mXOrg << " MyOrg:" << mYOrg << "Mx:" << mX << " My:" << mY << " SelX1:" << selX1 << " SelY1:" << selY1 << " SelX2:" << selX2 << " SelY2:" << selY2 << "type=mousemotion, mode="<<draggMode<<"\n"
			<< " ALT=" << GetWindow()->IsAltDown() << " CTRL="<<GetWindow()->IsCtrlDown()  << " SHIFT=" << GetWindow()->IsShiftDown() << " CAPS=" << GetWindow()->IsCapsLockOn() << " SPC=" << GetWindow()->IsSpaceDown()<<" modstate="<<GetWindow()->GetModState();
		debugLabel->SetText(tmp.str());
		*/

		int diffX = (mX - mXOrg);
		int diffY = (mY - mYOrg);
		mXOrg = mX;
		mYOrg = mY;

		UpdateMouseCursor(mode);
		double handedness = mApp->leftHandedView ? 1.0 : -1.0;
		switch (draggMode) {

		case DRAGG_NONE:
			// performXY
			mXOrg = GetWindow()->GetX(this, evt);
			mYOrg = GetWindow()->GetY(this, evt);
			break;

		case DRAGG_SELECTVERTEX:
		case DRAGG_SELECT:
		{

			
			if (GetWindow()->IsSpaceDown()) { //Move origin
				selX1 += diffX;
				selX2 += diffX;
				selY1 += diffY;
				selY2 += diffY;
			}
			else {
				selX2 = mX;
				selY2 = mY;
			}
		}
			break;

		case DRAGG_MOVE:

			if (view.projMode == PERSPECTIVE_PROJ) {
				double factor = GetWindow()->IsShiftDown() ? 0.05 : 1.0;
				double tv = factor*diffX / (double)width * view.camDist * 0.75;
				double tu = factor*diffY / (double)(height - DOWN_MARGIN) * view.camDist * 0.75;
				view.camOffset.x += tu * camUp.x - tv * camLeft.x;
				view.camOffset.y += tu * camUp.y - tv * camLeft.y;
				view.camOffset.z += tu * camUp.z - tv * camLeft.z;
			}
			else {
				double factor = GetWindow()->IsShiftDown() ? 0.05 : 1.0;
				double tv = factor*(diffX / (double)width)  * (view.vRight - view.vLeft) / view.camDist;
				double tu = factor*(diffY / (double)(height - DOWN_MARGIN)) * (view.vBottom - view.vTop) / view.camDist;
				view.camOffset.x += tu * camUp.x + tv * camLeft.x;
				view.camOffset.y += tu * camUp.y + tv * camLeft.y;
				view.camOffset.z += tu * camUp.z + tv * camLeft.z;
			}

			UpdateMatrix();
			autoScaleOn = false;
			autoBtn->SetState(false);
			break;

		case DRAGG_ROTATE:

			if (fabs(diffX) > 1.9 || fabs(diffY) > 1.9) {
				double factor = GetWindow()->IsShiftDown() ? 0.05 : 1.0;
				double handedness = mApp->leftHandedView ? 1.0 : -1.0;
				if (GetWindow()->IsCtrlDown()) {
					//Z axis rotation
					//TranslateScale(diffY);
					view.camAngleOz += diffX*angleStep*factor;
				}
				else {
					// Rotate view

					if (GetWindow()->IsAltDown()) {            //Lights direction rotation
						view.lightAngleOx += diffY * angleStep*factor;
						view.lightAngleOy += diffX * angleStep*factor;
					}
					else {                                  //Camera angle rotation
						if (view.projMode == PERSPECTIVE_PROJ) {
							view.camAngleOx += diffY * angleStep*factor;
							view.camAngleOy -= diffX * angleStep*factor*handedness;
						}
						else {
							view.camAngleOx -= diffY * angleStep*factor;
							view.camAngleOy -= diffX * angleStep*factor*handedness;
						}
					}
				}
				view.performXY = XYZ_NONE;
				zoomBtn->SetEnabled(false);
				if (mode == MODE_ZOOM) UpdateMouseCursor(MODE_SELECT);
				UpdateMatrix();
				if (autoScaleOn) (AutoScale(false));

			}

			//UpdateMatrix();
			break;
		}
	}

}

void GeometryViewer::SelectCoplanar(double tolerance) {
	if (!work) return;
	Geometry *geom = work->GetGeometry();
	GetWindow()->Clip(this, 0, 0, 0, DOWN_MARGIN);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(matProj);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(matView);
	selectionChange = true;
	geom->SelectCoplanar(this->width, this->height, tolerance);
}

void GeometryViewer::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:
		if (src == topBtn) {
			ToTopView();
		}
		else if (src == sideBtn) {
			ToSideView();
		}
		else if (src == frontBtn) {
			ToFrontView();
		}
		else if (src == zoomBtn) {
			UpdateMouseCursor(MODE_ZOOM);
		}
		else if (src == sysBtn) {
			GetParent()->ProcessMessage(this, MSG_GEOMVIEWER_MAXIMISE);
		}
		else if (src == selBtn) {
			UpdateMouseCursor(MODE_SELECT);
		}
		else if (src == selVxBtn) {
			UpdateMouseCursor(MODE_SELECTVERTEX);
		}
#ifdef SYNRAD
		else if (src == selTrajBtn) {
			UpdateMouseCursor(MODE_SELECTTRAJ);
		}
#endif
		else if (src == autoBtn) {
			autoScaleOn = !autoScaleOn;
			//autoBtn->SetState(autoScaleOn);
			if (autoScaleOn) AutoScale(false);
		}
		else if (src == handBtn) {
			UpdateMouseCursor(MODE_MOVE);
		}
		break;
	case MSG_COMBO:
		if (src == projCombo) {
			view.projMode = projCombo->GetSelectedIndex();
			ToFrontView();
		}
		break;
	}

}

#define TRANSFORMBB( X,Y,Z )                                                \
	mv.TransfomVec((float)bbO.X,(float)bbO.Y,(float)bbO.Z,1.0f,&rx,&ry,&rz,&rw);\
	dx = (double)rx;                                                            \
	dy = (double)ry;                                                            \
	dz = (double)rz;                                                            \
	if( dx < xMin ) xMin = dx;                                                  \
	if( dy < yMin ) yMin = dy;                                                  \
	if( dz < zNear) zNear = dz;                                                 \
	if( dx > xMax ) xMax = dx;                                                  \
	if( dy > yMax ) yMax = dy;                                                  \
	if( dz > zFar ) zFar = dz;

#define TRANSFORMVERTEX( X,Y,Z )                                  \
	mv.TransfomVec((float)X,(float)Y,(float)Z,1.0f,&rx,&ry,&rz,&rw);  \
	dx = (double)rx;                                                  \
	dy = (double)ry;                                                  \
	dz = (double)rz;                                                  \
	if( dx < xMin ) xMin = dx;                                        \
	if( dy < yMin ) yMin = dy;                                        \
	if( dz < zNear) zNear = dz;                                       \
	if( dx > xMax ) xMax = dx;                                        \
	if( dy > yMax ) yMax = dy;                                        \
	if( dz > zFar ) zFar = dz;

/*

void GeometryViewer::DrawBB() {
if( showLeak ) {
glDisable(GL_TEXTURE_2D);
glDisable(GL_LIGHTING);
glDisable(GL_BLEND);
glDisable(GL_CULL_FACE);
glColor3f(1.0f,1.0f,0.0f);
glBegin(GL_LINES);
DrawBB(geom->aabbTree);
glEnd();
}
}

void GeometryViewer::DrawBB(AABBNODE *node) {

if( node ) {

if( node->left==NULL && node->right==NULL ) {

// Leaf
glVertex3d(node->bb.min.x, node->bb.min.y, node->bb.min.z);
glVertex3d(node->bb.max.x, node->bb.min.y, node->bb.min.z);

glVertex3d(node->bb.max.x, node->bb.min.y, node->bb.min.z);
glVertex3d(node->bb.max.x, node->bb.min.y, node->bb.max.z);

glVertex3d(node->bb.max.x, node->bb.min.y, node->bb.max.z);
glVertex3d(node->bb.min.x, node->bb.min.y, node->bb.max.z);

glVertex3d(node->bb.min.x, node->bb.min.y, node->bb.max.z);
glVertex3d(node->bb.min.x, node->bb.min.y, node->bb.min.z);

glVertex3d(node->bb.min.x, node->bb.min.y, node->bb.min.z);
glVertex3d(node->bb.min.x, node->bb.max.y, node->bb.min.z);

glVertex3d(node->bb.max.x, node->bb.min.y, node->bb.min.z);
glVertex3d(node->bb.max.x, node->bb.max.y, node->bb.min.z);

glVertex3d(node->bb.max.x, node->bb.min.y, node->bb.max.z);
glVertex3d(node->bb.max.x, node->bb.max.y, node->bb.max.z);

glVertex3d(node->bb.min.x, node->bb.min.y, node->bb.max.z);
glVertex3d(node->bb.min.x, node->bb.max.y, node->bb.max.z);

glVertex3d(node->bb.min.x, node->bb.max.y, node->bb.min.z);
glVertex3d(node->bb.max.x, node->bb.max.y, node->bb.min.z);

glVertex3d(node->bb.max.x, node->bb.max.y, node->bb.min.z);
glVertex3d(node->bb.max.x, node->bb.max.y, node->bb.max.z);

glVertex3d(node->bb.max.x, node->bb.max.y, node->bb.max.z);
glVertex3d(node->bb.min.x, node->bb.max.y, node->bb.max.z);

glVertex3d(node->bb.min.x, node->bb.max.y, node->bb.max.z);
glVertex3d(node->bb.min.x, node->bb.max.y, node->bb.min.z);

} else {
DrawBB(node->left);
DrawBB(node->right);
}

}

}
*/

void GeometryViewer::ComputeBB(/*bool getAll*/) {

	Geometry *geom = work->GetGeometry();

	GLMatrix mv;
	float rx, ry, rz, rw;
	double dx, dy, dz;
	xMin = 1e100;
	yMin = 1e100;
	zNear = 1e100;
	xMax = -1e100;
	yMax = -1e100;
	zFar = -1e100;
	mv.LoadGL(matView);

	//Transform the AxisAlignedBoundingBox (fast method, but less accurate 
	//than full vertex transform)
	//AxisAlignedBoundingBox bbO = geom->GetBB();
	//TRANSFORMBB(min.x,min.y,min.z);
	//TRANSFORMBB(max.x,min.y,min.z);
	//TRANSFORMBB(max.x,min.y,max.z);
	//TRANSFORMBB(min.x,min.y,max.z);
	//TRANSFORMBB(min.x,max.y,min.z);
	//TRANSFORMBB(max.x,max.y,min.z);
	//TRANSFORMBB(max.x,max.y,max.z);
	//TRANSFORMBB(min.x,max.y,max.z);

	size_t nbV = geom->GetNbVertex();

	if (true /*geom->viewStruct < 0 || getAll*/) {

		/*
		for (int i = 0; i < nbV; i++) {
			Vector3d *p = geom->GetVertex(i);
			TRANSFORMVERTEX(p->x, p->y, p->z);
		}*/

//#ifdef SYNRAD		
		//regions included
		AxisAlignedBoundingBox bb = geom->GetBB();
		TRANSFORMVERTEX(bb.min.x, bb.min.y, bb.min.z);
		TRANSFORMVERTEX(bb.max.x, bb.min.y, bb.min.z);
		TRANSFORMVERTEX(bb.min.x, bb.max.y, bb.min.z);
		TRANSFORMVERTEX(bb.min.x, bb.min.y, bb.max.z);
		TRANSFORMVERTEX(bb.min.x, bb.max.y, bb.max.z);
		TRANSFORMVERTEX(bb.max.x, bb.min.y, bb.max.z);
		TRANSFORMVERTEX(bb.max.x, bb.max.y, bb.min.z);
		TRANSFORMVERTEX(bb.max.x, bb.max.y, bb.max.z);
//#endif

	}
	else { //ignored

		bool *refIdx = (bool *)malloc(nbV * sizeof(bool));
		memset(refIdx, 0, nbV * sizeof(bool));

		// Get facet of the selected structure
		size_t nbF = geom->GetNbFacet();
		for (int i = 0; i < nbF; i++) {
			Facet *f = geom->GetFacet(i);
			if (f->sh.superIdx == geom->viewStruct || f->sh.superIdx == -1) {
				for (int j = 0; j < f->sh.nbIndex; j++) refIdx[f->indices[j]] = true;
			}
		}

		// Transform vertex
		for (size_t i = 0; i < nbV; i++) {
			if (refIdx[i]) {
				Vector3d *p = geom->GetVertex(i);
				TRANSFORMVERTEX(p->x, p->y, p->z);
			}
		}

		free(refIdx);

	}

}

void Geometry::ClearFacetMeshLists()
{
	GLProgress *prg = new GLProgress("Please wait...", "Clearing facet meshes...");
	prg->SetVisible(true);
	size_t nbFacet = mApp->worker.GetGeometry()->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		prg->SetProgress((double)i / (double)nbFacet);
		DELETE_LIST(mApp->worker.GetGeometry()->GetFacet(i)->glElem);
	}
	prg->SetVisible(false);
	SAFE_DELETE(prg);
}

void Geometry::BuildFacetMeshLists()
{
	GLProgress *prg = new GLProgress("Please wait...", "Building facet meshes...");
	prg->SetVisible(true);
	size_t nbFacet = mApp->worker.GetGeometry()->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		prg->SetProgress((double)i / (double)nbFacet);
		mApp->worker.GetGeometry()->GetFacet(i)->BuildMeshGLList();

	}
	prg->SetVisible(false);
	SAFE_DELETE(prg);

}

void GeometryViewer::Screenshot() {

	screenshotStatus.requested = 0;

	SDL_Surface * image = SDL_CreateRGBSurface(SDL_SWSURFACE, screenshotStatus.w, screenshotStatus.h, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);

	glReadPixels(screenshotStatus.x, this->GetHeight() - screenshotStatus.y - screenshotStatus.h , screenshotStatus.w, screenshotStatus.h, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

	//Vertical flip
	int index;
	void* temp_row;
	int height_div_2;

	temp_row = (void *)malloc(image->pitch);
	if (NULL == temp_row)
	{
		throw Error("Not enough memory for image inversion");
	}
	height_div_2 = (int)(image->h * .5);
	for (index = 0; index < height_div_2; index++)
	{
		memcpy((Uint8 *)temp_row, (Uint8 *)(image->pixels) + image->pitch * index, image->pitch);
		memcpy((Uint8 *)(image->pixels) + image->pitch * index, (Uint8 *)(image->pixels) + image->pitch * (image->h - index - 1), image->pitch);
		memcpy((Uint8 *)(image->pixels) + image->pitch * (image->h - index - 1), temp_row, image->pitch);
	}
	free(temp_row);
	SDL_SavePNG_RW(image, SDL_RWFromFile(screenshotStatus.fileName.c_str(), "wb"), 1);
	//SDL_SavePNG(image, screenshotStatus.fileName.c_str());
	SDL_FreeSurface(image);

	screenshotLabel->SetVisible(false);
	//mApp->wereEvents = true; //Hide screensot status label
}

void GeometryViewer::RequestScreenshot(std::string fileName, int x, int y, int w, int h) {
	//Set area as whole screen by default
	screenshotStatus.x = x;
	screenshotStatus.y = y;
	screenshotStatus.w = w;
	screenshotStatus.h = h;
	screenshotStatus.fileName = fileName;
	if (screenshotStatus.requested<2) screenshotStatus.requested++;
}