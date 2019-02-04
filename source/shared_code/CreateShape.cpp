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
#include "CreateShape.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLToggle.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "GLApp/MathTools.h" //Contains
#include "GLApp/GLIcon.h"

#ifdef MOLFLOW
#include "MolFlow.h"
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

CreateShape::CreateShape(Geometry *g,Worker *w):GLWindow() {

	int wD = 713;
	int hD = 478;
	shapePanel = new GLTitledPanel("Shape");
	shapePanel->SetBounds(8, 3, 694, 247);
	Add(shapePanel);
	positionPanel = new GLTitledPanel("Position");
	positionPanel->SetBounds(8, 256, 694, 94);
	Add(positionPanel);
	sizePanel = new GLTitledPanel("Size");
	sizePanel->SetBounds(8, 356, 694, 66);
	Add(sizePanel);
	racetrackCheckbox = new GLToggle(0, "Racetrack");
	shapePanel->SetCompBounds(racetrackCheckbox, 424, 19, 76, 17);
	shapePanel->Add(racetrackCheckbox);

	ellipseCheckbox = new GLToggle(0, "Circle / Ellipse");
	shapePanel->SetCompBounds(ellipseCheckbox, 308, 19, 93, 17);
	shapePanel->Add(ellipseCheckbox);

	rectangleCheckbox = new GLToggle(0, "Square / Rectangle");
	shapePanel->SetCompBounds(rectangleCheckbox, 182, 19, 120, 17);
	shapePanel->Add(rectangleCheckbox);

	normalStatusLabel = new GLLabel("status");
	positionPanel->SetCompBounds(normalStatusLabel, 594, 68, 35, 13);
	positionPanel->Add(normalStatusLabel);

	normalVertexButton = new GLButton(0, "Center to vertex");
	positionPanel->SetCompBounds(normalVertexButton, 506, 65, 85, 20);
	positionPanel->Add(normalVertexButton);

	facetNormalButton = new GLButton(0, "Facet N");
	positionPanel->SetCompBounds(facetNormalButton, 425, 65, 75, 20);
	positionPanel->Add(facetNormalButton);

	axisStatusLabel = new GLLabel("status");
	positionPanel->SetCompBounds(axisStatusLabel, 594, 42, 35, 13);
	positionPanel->Add(axisStatusLabel);

	axisVertexButton = new GLButton(0, "Center to vertex");
	positionPanel->SetCompBounds(axisVertexButton, 506, 39, 85, 20);
	positionPanel->Add(axisVertexButton);

	axisFacetUButton = new GLButton(0, "Facet U");
	positionPanel->SetCompBounds(axisFacetUButton, 425, 39, 75, 20);
	positionPanel->Add(axisFacetUButton);

	centerStatusLabel = new GLLabel("status");
	positionPanel->SetCompBounds(centerStatusLabel, 594, 16, 35, 13);
	positionPanel->Add(centerStatusLabel);

	centerVertexButton = new GLButton(0, "Vertex");
	positionPanel->SetCompBounds(centerVertexButton, 506, 13, 85, 20);
	positionPanel->Add(centerVertexButton);

	facetCenterButton = new GLButton(0, "Facet center");
	positionPanel->SetCompBounds(facetCenterButton, 425, 13, 75, 20);
	positionPanel->Add(facetCenterButton);

	normalZtext = new GLTextField(0, "1");
	positionPanel->SetCompBounds(normalZtext, 339, 65, 80, 20);
	positionPanel->Add(normalZtext);

	label9 = new GLLabel("Z:");
	positionPanel->SetCompBounds(label9, 322, 68, 17, 13);
	positionPanel->Add(label9);

	normalYtext = new GLTextField(0, "0");
	positionPanel->SetCompBounds(normalYtext, 232, 65, 80, 20);
	positionPanel->Add(normalYtext);

	label10 = new GLLabel("Y:");
	positionPanel->SetCompBounds(label10, 215, 68, 17, 13);
	positionPanel->Add(label10);

	normalXtext = new GLTextField(0, "0");
	positionPanel->SetCompBounds(normalXtext, 128, 65, 80, 20);
	positionPanel->Add(normalXtext);

	label11 = new GLLabel("X:");
	positionPanel->SetCompBounds(label11, 111, 68, 17, 13);
	positionPanel->Add(label11);

	label12 = new GLLabel("Normal direction:");
	positionPanel->SetCompBounds(label12, 6, 68, 86, 13);
	positionPanel->Add(label12);

	axisZtext = new GLTextField(0, "0");
	positionPanel->SetCompBounds(axisZtext, 339, 40, 80, 20);
	positionPanel->Add(axisZtext);

	label5 = new GLLabel("Z:");
	positionPanel->SetCompBounds(label5, 322, 42, 17, 13);
	positionPanel->Add(label5);

	axisYtext = new GLTextField(0, "0");
	positionPanel->SetCompBounds(axisYtext, 232, 39, 80, 20);
	positionPanel->Add(axisYtext);

	label6 = new GLLabel("Y:");
	positionPanel->SetCompBounds(label6, 215, 42, 17, 13);
	positionPanel->Add(label6);

	axisXtext = new GLTextField(0, "1");
	positionPanel->SetCompBounds(axisXtext, 128, 39, 80, 20);
	positionPanel->Add(axisXtext);

	label7 = new GLLabel("X:");
	positionPanel->SetCompBounds(label7, 111, 42, 17, 13);
	positionPanel->Add(label7);

	label8 = new GLLabel("Axis1 direction:");
	positionPanel->SetCompBounds(label8, 6, 42, 78, 13);
	positionPanel->Add(label8);

	centerZtext = new GLTextField(0, "0");
	positionPanel->SetCompBounds(centerZtext, 339, 13, 80, 20);
	positionPanel->Add(centerZtext);

	label4 = new GLLabel("Z:");
	positionPanel->SetCompBounds(label4, 322, 16, 17, 13);
	positionPanel->Add(label4);

	centerYtext = new GLTextField(0, "0");
	positionPanel->SetCompBounds(centerYtext, 232, 13, 80, 20);
	positionPanel->Add(centerYtext);

	label3 = new GLLabel("Y:");
	positionPanel->SetCompBounds(label3, 215, 16, 17, 13);
	positionPanel->Add(label3);

	centerXtext = new GLTextField(0, "0");
	positionPanel->SetCompBounds(centerXtext, 128, 13, 80, 20);
	positionPanel->Add(centerXtext);

	label2 = new GLLabel("X:");
	positionPanel->SetCompBounds(label2, 111, 16, 17, 13);
	positionPanel->Add(label2);

	label1 = new GLLabel("Center:");
	positionPanel->SetCompBounds(label1, 6, 16, 41, 13);
	positionPanel->Add(label1);

	fullCircleButton = new GLButton(0, "Full circle sides");
	sizePanel->SetCompBounds(fullCircleButton, 449, 13, 142, 20);
	sizePanel->Add(fullCircleButton);

	nbstepsText = new GLTextField(0, "10");
	sizePanel->SetCompBounds(nbstepsText, 339, 39, 80, 20);
	sizePanel->Add(nbstepsText);

	label22 = new GLLabel("Steps in arc:");
	sizePanel->SetCompBounds(label22, 229, 42, 66, 13);
	sizePanel->Add(label22);

	label20 = new GLLabel("cm");
	sizePanel->SetCompBounds(label20, 422, 16, 21, 13);
	sizePanel->Add(label20);

	racetrackToplengthText = new GLTextField(0, "");
	sizePanel->SetCompBounds(racetrackToplengthText, 339, 13, 80, 20);
	sizePanel->Add(racetrackToplengthText);

	label21 = new GLLabel("Racetrack top length:");
	sizePanel->SetCompBounds(label21, 229, 16, 110, 13);
	sizePanel->Add(label21);

	label18 = new GLLabel("cm");
	sizePanel->SetCompBounds(label18, 161, 42, 21, 13);
	sizePanel->Add(label18);

	axis2LengthText = new GLTextField(0, "1");
	sizePanel->SetCompBounds(axis2LengthText, 79, 39, 80, 20);
	sizePanel->Add(axis2LengthText);

	label19 = new GLLabel("Axis2 length:");
	sizePanel->SetCompBounds(label19, 6, 42, 67, 13);
	sizePanel->Add(label19);

	label17 = new GLLabel("cm");
	sizePanel->SetCompBounds(label17, 161, 16, 21, 13);
	sizePanel->Add(label17);

	axis1LengthText = new GLTextField(0, "1");
	sizePanel->SetCompBounds(axis1LengthText, 79, 13, 80, 20);
	sizePanel->Add(axis1LengthText);

	label16 = new GLLabel("Axis1 length:");
	sizePanel->SetCompBounds(label16, 6, 16, 67, 13);
	sizePanel->Add(label16);

	createButton = new GLButton(0, "Create facet");
	createButton->SetBounds(327, 431, 100, 20);
	Add(createButton);



	//------------------------------------------------------

	rectangleIcon = new GLIcon("images/edit_rectangle.png");
	rectangleIcon->SetBounds(80, 43, 500, 190);
	rectangleIcon->SetVisible(true);
	Add(rectangleIcon);
	circleIcon = new GLIcon("images/edit_circle.png");
	circleIcon->SetBounds(80, 43, 500, 190);
	circleIcon->SetVisible(false);
	Add(circleIcon);
	racetrackIcon = new GLIcon("images/edit_racetrack.png");
	racetrackIcon->SetBounds(80, 43, 500, 190);
	racetrackIcon->SetVisible(false);
	Add(racetrackIcon);

	SetTitle("Create shape");
	// Center dialog
	//int wS,hS;
	//GLToolkit::GetScreenSize(&wS,&hS);
	int xD = 7; //quite wide window for centering!
	int yD = 29;
	SetBounds(xD,yD,wD,hD);


  RestoreDeviceObjects();

  geom = g;
  work = w;
  mode = MODE_RECTANGLE;
  EnableDisableControls();
  centerStatusLabel->SetText("");
  axisStatusLabel->SetText("");
  normalStatusLabel->SetText("");
}

void CreateShape::ProcessMessage(GLComponent *src,int message) {

  switch (message) {
  case MSG_BUTTON:
	  if (src == createButton) {
		  Vector3d center, axisDir, normalDir;
		  double axis1length, axis2length, racetrackTopLength;
		  int nbSteps;

		  if (!centerXtext->GetNumber(&center.x)) {
			  GLMessageBox::Display("Invalid center X coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!centerYtext->GetNumber(&center.y)) {
			  GLMessageBox::Display("Invalid center Y coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!centerZtext->GetNumber(&center.z)) {
			  GLMessageBox::Display("Invalid center Z coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  if (!axisXtext->GetNumber(&axisDir.x)) {
			  GLMessageBox::Display("Invalid Axis1 direction X coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!axisYtext->GetNumber(&axisDir.y)) {
			  GLMessageBox::Display("Invalid Axis1 direction Y coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!axisZtext->GetNumber(&axisDir.z)) {
			  GLMessageBox::Display("Invalid Axis1 direction Z coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  if (!normalXtext->GetNumber(&normalDir.x)) {
			  GLMessageBox::Display("Invalid normal direction X coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!normalYtext->GetNumber(&normalDir.y)) {
			  GLMessageBox::Display("Invalid normal direction Y coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!normalZtext->GetNumber(&normalDir.z)) {
			  GLMessageBox::Display("Invalid normal direction Z coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  if (IsEqual(axisDir.Norme(), 0.0)) {
			  GLMessageBox::Display("Axis1 direction can't be null-vector", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (IsEqual(normalDir.Norme(),0.0)) {
			  GLMessageBox::Display("Normal direction can't be null-vector", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  if (!axis1LengthText->GetNumber(&axis1length) || !(axis1length>0.0)) {
			  GLMessageBox::Display("Invalid axis1 length", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  if (!axis2LengthText->GetNumber(&axis2length) || !(axis2length>0.0)) {
			  GLMessageBox::Display("Invalid axis2 length", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  if (mode == MODE_RACETRACK) {
			  if (!racetrackToplengthText->GetNumber(&racetrackTopLength)) {
				  GLMessageBox::Display("Can't parse racetrack top length", "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
			  if (racetrackTopLength >= axis1length) {
				  GLMessageBox::Display("For a racetrack, the top length must be less than Axis1", "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  } 
			  if (!(racetrackTopLength >= (axis1length - axis2length))) {
				  GLMessageBox::Display("For a racetrack, the top length must be at least (Axis1 - Axis2)", "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
			  if (!(racetrackTopLength > 0.0)) {
				  GLMessageBox::Display("Top length must be positive", "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
		  }
		  if (Contains({ MODE_CIRCLE,MODE_RACETRACK }, mode)) {
			  if (!nbstepsText->GetNumberInt(&nbSteps)) {
				  GLMessageBox::Display("Can't parse number of steps in arc", "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
			  if (!(nbSteps>=2)) {
				  GLMessageBox::Display("Number of arc steps must be at least 2", "Error", GLDLG_OK, GLDLG_ICONERROR);
				  return;
			  }
		  }

		  switch (mode) {
		  case MODE_RECTANGLE:
			  geom->CreateRectangle(center, axisDir, normalDir, axis1length, axis2length);
			  break;
		  case MODE_CIRCLE:
			  geom->CreateCircle(center, axisDir, normalDir, axis1length, axis2length, (size_t)nbSteps);
			  break;
		  case MODE_RACETRACK:
			  geom->CreateRacetrack(center, axisDir, normalDir, axis1length, axis2length, racetrackTopLength, (size_t)nbSteps);
			  break;
		  }
		  work->Reload();
		  mApp->changedSinceSave = true;
		  mApp->UpdateFacetlistSelected();
	  }
	  else if (src == fullCircleButton) {
		  double axis1length, axis2length;
		  if (!axis1LengthText->GetNumber(&axis1length) || !(axis1length>0.0)) {
			  GLMessageBox::Display("Invalid axis1 length", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  if (!axis2LengthText->GetNumber(&axis2length) || !(axis2length>0.0)) {
			  GLMessageBox::Display("Invalid axis2 length", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  racetrackToplengthText->SetText(axis1length-axis2length);
	  }
	  else if (src == facetCenterButton) {
		  auto selFacets = geom->GetSelectedFacets();
		  if (selFacets.size() != 1) {
			  GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  Vector3d facetCenter = geom->GetFacet(selFacets[0])->sh.center;
		  
		  centerXtext->SetText(facetCenter.x);
		  centerYtext->SetText(facetCenter.y);
		  centerZtext->SetText(facetCenter.z);
		  
		  std::stringstream tmp; tmp << "Center of facet " << selFacets[0] + 1;
		  centerStatusLabel->SetText(tmp.str());
	  }
	  else if (src == centerVertexButton) {
		  auto selVertices = geom->GetSelectedVertices();
		  if (selVertices.size() != 1) {
			  GLMessageBox::Display("Select exactly one vertex", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  Vector3d center = *(geom->GetVertex(selVertices[0]));

		  centerXtext->SetText(center.x);
		  centerYtext->SetText(center.y);
		  centerZtext->SetText(center.z);

		  std::stringstream tmp; tmp << "Vertex " << selVertices[0] + 1;
		  centerStatusLabel->SetText(tmp.str());
	  }
	  else if (src == axisFacetUButton) {
		  auto selFacets = geom->GetSelectedFacets();
		  if (selFacets.size() != 1) {
			  GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  Vector3d facetU = geom->GetFacet(selFacets[0])->sh.U;

		  axisXtext->SetText(facetU.x);
		  axisYtext->SetText(facetU.y);
		  axisZtext->SetText(facetU.z);

		  std::stringstream tmp; tmp << "Facet " << selFacets[0] + 1 << " \201";
		  axisStatusLabel->SetText(tmp.str());
	  }
	  else if (src == axisVertexButton) {
		  Vector3d center;
		  if (!centerXtext->GetNumber(&center.x)) {
			  GLMessageBox::Display("Invalid center X coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!centerYtext->GetNumber(&center.y)) {
			  GLMessageBox::Display("Invalid center Y coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!centerZtext->GetNumber(&center.z)) {
			  GLMessageBox::Display("Invalid center Z coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  
		  auto selVertices = geom->GetSelectedVertices();
		  if (selVertices.size() != 1) {
			  GLMessageBox::Display("Select exactly one vertex", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  Vector3d vertexLocation = *(geom->GetVertex(selVertices[0]));

		  Vector3d diff = vertexLocation - center;

		  axisXtext->SetText(diff.x);
		  axisYtext->SetText(diff.y);
		  axisZtext->SetText(diff.z);

		  std::stringstream tmp; tmp << "Center to vertex " << selVertices[0] + 1;
		  axisStatusLabel->SetText(tmp.str());
	  }
	  else if (src == facetNormalButton) {
		  auto selFacets = geom->GetSelectedFacets();
		  if (selFacets.size() != 1) {
			  GLMessageBox::Display("Select exactly one facet", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  Vector3d facetN = geom->GetFacet(selFacets[0])->sh.N;

		  normalXtext->SetText(facetN.x);
		  normalYtext->SetText(facetN.y);
		  normalZtext->SetText(facetN.z);

		  std::stringstream tmp; tmp << "Facet " << selFacets[0] + 1 << " normal";
		  normalStatusLabel->SetText(tmp.str());
	  }
	  else if (src == normalVertexButton) {
		  Vector3d center;
		  if (!centerXtext->GetNumber(&center.x)) {
			  GLMessageBox::Display("Invalid center X coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!centerYtext->GetNumber(&center.y)) {
			  GLMessageBox::Display("Invalid center Y coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  if (!centerZtext->GetNumber(&center.z)) {
			  GLMessageBox::Display("Invalid center Z coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }

		  auto selVertices = geom->GetSelectedVertices();
		  if (selVertices.size() != 1) {
			  GLMessageBox::Display("Select exactly one vertex", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  Vector3d vertexLocation = *(geom->GetVertex(selVertices[0]));

		  Vector3d diff = vertexLocation - center;

		  normalXtext->SetText(diff.x);
		  normalYtext->SetText(diff.y);
		  normalZtext->SetText(diff.z);

		  std::stringstream tmp; tmp << "Center to vertex " << selVertices[0] + 1;
		  normalStatusLabel->SetText(tmp.str());
	  }
	  break;
  case MSG_TOGGLE:
	  if (src == rectangleCheckbox) mode = MODE_RECTANGLE;
	  else if (src == ellipseCheckbox) mode = MODE_CIRCLE;
	  else if (src == racetrackCheckbox) mode = MODE_RACETRACK;

	  EnableDisableControls();

	  break;
  }

  GLWindow::ProcessMessage(src, message);
}

void CreateShape::EnableDisableControls() {
	rectangleCheckbox->SetState((int)(mode == MODE_RECTANGLE));
	rectangleIcon->SetVisible(mode == MODE_RECTANGLE);
	ellipseCheckbox->SetState((int)(mode == MODE_CIRCLE));
	circleIcon->SetVisible(mode == MODE_CIRCLE);
	racetrackCheckbox->SetState((int)(mode == MODE_RACETRACK));
	racetrackToplengthText->SetEditable(mode == MODE_RACETRACK);
	racetrackIcon->SetVisible(mode == MODE_RACETRACK);
	fullCircleButton->SetEnabled(mode == MODE_RACETRACK);
	nbstepsText->SetEditable(Contains({ MODE_CIRCLE,MODE_RACETRACK }, mode));
}