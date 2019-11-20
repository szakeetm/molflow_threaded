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
#include "VertexCoordinates.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLList.h"

#include "Geometry_shared.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif
#include "GLApp/GLInputBox.h"

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

static const int   flWidth[] = {40,100,100,100};
static const char *flName[] = {"Vertex#","X","Y","Z"};
static const int   flAligns[] = { ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };
static const int   fEdits[] = { 0,EDIT_NUMBER,EDIT_NUMBER,EDIT_NUMBER };

VertexCoordinates::VertexCoordinates():GLWindow() {

  int wD = 405;
  int hD = 350;
  SetIconfiable(true);
  SetResizable(false);
  /*
  GLTitledPanel *p = new GLTitledPanel("Facet index");
  p->SetBounds(5,hD-145,wD-10,95);
  //Add(p);

  //insert1Button = new GLButton(0,"Insert (New vertex)");
  //SetBounds(p,insert1Button,5,20,120,19);
  //Add(insert1Button);

  insert2Button = new GLButton(0,"Insert (Existing vertex)");
  SetBounds(p,insert2Button,5,45,120,19);
  //Add(insert2Button);

  removeButton = new GLButton(0,"Remove");;
  SetBounds(p,removeButton,5,70,120,19);
  //Add(removeButton);

  GLLabel *l1 = new GLLabel("Insertion position");
  SetBounds(p,l1,130,20,90,19);
  //Add(l1);

  insertPosText = new GLTextField(0,"");
  SetBounds(p,insertPosText,220,20,30,19);
  //Add(insertPosText);

  // TODO
  insert1Button->SetEnabled(false);
  insert2Button->SetEnabled(false);
  removeButton->SetEnabled(false);
  insertPosText->SetEnabled(false);
  */
  setXbutton = new GLButton(0, "X");
  setXbutton->SetBounds(5, hD - 43, 16, 19);
  Add(setXbutton);
  setYbutton = new GLButton(0, "Y");
  setYbutton->SetBounds(27, hD - 43, 16, 19);
  Add(setYbutton);
  setZbutton = new GLButton(0, "Z");
  setZbutton->SetBounds(49, hD - 43, 16, 19);
  Add(setZbutton);

  updateButton = new GLButton(0,"Apply");
  updateButton->SetBounds(wD-195,hD-43,90,19);
  Add(updateButton);

  dismissButton = new GLButton(0,"Dismiss");
  dismissButton->SetBounds(wD-95,hD-43,90,19);
  Add(dismissButton);

  vertexListC = new GLList(0);
  vertexListC->SetBounds(5,5,wD-10,hD-60);
  vertexListC->SetColumnLabelVisible(true);
  vertexListC->SetGrid(true);
  Add(vertexListC);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  RestoreDeviceObjects();

}

/*
void VertexCoordinates::GetSelected() {

  selFacet = NULL;
  int i = 0;
  Geometry *g = worker->GetGeometry();
  int nb = g->GetNbFacet();
  while(!selFacet && i<nb) {
    if( g->GetFacet(i)->selected ) selFacet = g->GetFacet(i);
    if(!selFacet) i++;
  }

  char tmp[32];
  sprintf(tmp,"Facets coordinates #%d",i+1);
  SetTitle(tmp);

}
*/

void VertexCoordinates::Update() {

  char tmp[256];
  if(!IsVisible()) return;
  /*
  GetSelected();
  if(!selFacet) return;
  */

  Geometry *s = worker->GetGeometry();
  int count=0;
  vertexListC->SetSize(4,s->GetNbSelectedVertex());
  vertexListC->SetColumnWidths((int*)flWidth);
  vertexListC->SetColumnLabels(flName);
  vertexListC->SetColumnAligns((int *)flAligns);
  vertexListC->SetColumnEditable((int *)fEdits);
  for(size_t i=0;i<s->GetNbVertex();i++) {
	  if(s->GetVertex(i)->selected) {
		  sprintf(tmp,"%zd",i+1);
		  vertexListC->SetValueAt(0,count,tmp);
		  //sprintf(tmp,"%d",idx+1);
		  //vertexListC->SetValueAt(1,i,tmp);
		  Vector3d *v = s->GetVertex(i);
		  sprintf(tmp,"%.10g",v->x);
		  vertexListC->SetValueAt(1,count,tmp);
		  sprintf(tmp,"%.10g",v->y);
		  vertexListC->SetValueAt(2,count,tmp);
		  sprintf(tmp,"%.10g",v->z);
		  vertexListC->SetValueAt(3,count,tmp);
		  count++;
	  }
  }

}

void VertexCoordinates::Display(Worker *w) {
  SetTitle("Vertex coordinates");
  worker = w;
  SetVisible(true);
  Update();

}

void VertexCoordinates::ProcessMessage(GLComponent *src,int message) {

  Geometry *geom = worker->GetGeometry();
  switch(message) {
    case MSG_BUTTON:
      if(src==dismissButton) {
        SetVisible(false);
	  }
	  else if (src == setXbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all X coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int i = 0; i < vertexListC->GetNbRow(); i++) {
			  vertexListC->SetValueAt(1, i, coord);
		  }
	  }
	  else if (src == setYbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all Y coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int i = 0; i < vertexListC->GetNbRow(); i++) {
			  vertexListC->SetValueAt(2, i, coord);
		  }
	  }
	  else if (src == setZbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all Z coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int i = 0; i < vertexListC->GetNbRow(); i++) {
			  vertexListC->SetValueAt(3, i, coord);
		  }
	  }
	  else if (src == updateButton) {
        int rep = GLMessageBox::Display("Apply geometry changes ?","Question",GLDLG_OK|GLDLG_CANCEL,GLDLG_ICONWARNING);
        if( rep == GLDLG_OK ) {
			if (mApp->AskToReset(worker)) {
			//if (worker->isRunning) worker->Stop_Public();
			mApp->changedSinceSave=true;
			for(size_t i=0;i<vertexListC->GetNbRow();i++) {
				double x,y,z;
				size_t id;
				id=vertexListC->GetValueInt(i,0)-1;
				if (!(id>=0 && id<geom->GetNbVertex())) { //wrong coordinates at row
					char tmp[128];
					sprintf(tmp, "Invalid vertex id in row %zd\n Vertex %zd doesn't exist.", i + 1, id+1);
					GLMessageBox::Display(tmp, "Incorrect vertex id", GLDLG_OK, GLDLG_ICONWARNING);
					return;
				}
				bool success = (1==sscanf(vertexListC->GetValueAt(1,i),"%lf",&x));
				success = success && (1==sscanf(vertexListC->GetValueAt(2,i),"%lf",&y));
				success = success && (1==sscanf(vertexListC->GetValueAt(3,i),"%lf",&z));
				if (!success) { //wrong coordinates at row
					char tmp[128];
					sprintf(tmp,"Invalid coordinates in row %zd",i+1);
					GLMessageBox::Display(tmp,"Incorrect vertex",GLDLG_OK,GLDLG_ICONWARNING);
				return;	
				}
				geom->MoveVertexTo(id,x,y,z);
			}
			geom->Rebuild();
          // Send to sub process
          try {
            worker->Reload();
          } catch(Error &e) {
            GLMessageBox::Display(e.GetMsg(),"Error",GLDLG_OK,GLDLG_ICONERROR);
            return;
          }
          //GLWindowManager::FullRepaint();
		 }
        }
      }
    break;
    /*case MSG_LIST:
      if(src==vertexListC) {
        char tmp[32];
        sprintf(tmp,"%d",vertexListC->GetSelectedRow()+1);
        insertPosText->SetText(tmp);
      }
    break;*/

  }

  GLWindow::ProcessMessage(src,message);

}
