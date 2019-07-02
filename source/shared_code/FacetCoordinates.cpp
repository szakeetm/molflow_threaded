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
#include "FacetCoordinates.h"
#include "Facet_shared.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLInputBox.h"
#include "GLApp/GLComponent.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLList.h"
#include <vector>

#include "Geometry_shared.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

extern GLApplication *theApp;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

static const int   flWidth[] = {35,40,100,100,100};
static const char *flName[] = {"#","Vertex","X","Y","Z"};
static const int   flAligns[] = { ALIGN_CENTER,ALIGN_CENTER,ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };
static const int   fEdits[] = { 0,0,EDIT_NUMBER,EDIT_NUMBER,EDIT_NUMBER };

struct line {
	size_t vertexId;
	Vector3d coord;
};

/**
* \brief Constructor with initialisation for the FacetCoordinates window (Facet/Facet coordinates)
*/
FacetCoordinates::FacetCoordinates():GLWindow() {

  int wD = 405;
  int hD = 400;
  SetIconfiable(true);
  SetResizable(false);

  GLTitledPanel *p = new GLTitledPanel("Insert / Remove vertex");
  p->SetBounds(5,hD-120,wD-10,70);
  Add(p);

  GLLabel *l1 = new GLLabel("Vertex Id to insert:");
  SetCompBoundsRelativeTo(p,l1,20,20,120,19);
  Add(l1);

  insertIdText = new GLTextField(0,"0");
  SetCompBoundsRelativeTo(p,insertIdText,135,20,40,19);
  Add(insertIdText);

  insertLastButton = new GLButton(0,"Insert as last vertex");
  SetCompBoundsRelativeTo(p,insertLastButton,5,45,120,19);
  Add(insertLastButton);

  insertBeforeButton = new GLButton(0,"Insert before sel. row");
  SetCompBoundsRelativeTo(p,insertBeforeButton,135,45,120,19);
  insertBeforeButton->SetEnabled(false);
  Add(insertBeforeButton);

  removePosButton = new GLButton(0,"Remove selected row");;
  SetCompBoundsRelativeTo(p,removePosButton,265,45,120,19);
  removePosButton->SetEnabled(false);
  Add(removePosButton);
  
  updateButton = new GLButton(0,"Apply");
  updateButton->SetBounds(wD-195,hD-43,90,19);
  Add(updateButton);

  setXbutton = new GLButton(0, "X");
  setXbutton->SetBounds(5, hD - 43, 16, 19);
  Add(setXbutton);
  setYbutton = new GLButton(0, "Y");
  setYbutton->SetBounds(27, hD - 43, 16, 19);
  Add(setYbutton);
  setZbutton = new GLButton(0, "Z");
  setZbutton->SetBounds(49, hD - 43, 16, 19);
  Add(setZbutton);

  dismissButton = new GLButton(0,"Dismiss");
  dismissButton->SetBounds(wD-95,hD-43,90,19);
  Add(dismissButton);

  facetListC = new GLList(0);
  facetListC->SetSize(5,1);
  facetListC->SetBounds(5,5,wD-10,hD-130);
  facetListC->SetColumnLabelVisible(true);
  facetListC->SetGrid(true);
  //facetListC->SetSelectionMode(SINGLE_CELL);
  //facetListC->Sortable=true;
  Add(facetListC);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  RestoreDeviceObjects();

}

/**
* \brief Set title of the window with the lowest selected facet id
*/
void FacetCoordinates::GetSelected() {

  selFacet = NULL;
  size_t i = 0;
  Geometry *g = worker->GetGeometry();
  size_t nb = g->GetNbFacet();
  while(!selFacet && i<nb) {
    if( g->GetFacet(i)->selected ) selFacet = g->GetFacet(i);
    if(!selFacet) i++;
  }

  char tmp[32];
  sprintf(tmp,"Facets coordinates #%zd",i+1);
  SetTitle(tmp);

}

/**
* \brief Update table data from selected Facet
*/
void FacetCoordinates::UpdateFromSelection() {

  
  if(!IsVisible()) return;
  GetSelected();
  if(!selFacet) return;

  Geometry *geom = worker->GetGeometry();

  size_t nbIndex = selFacet->sh.nbIndex;

  lines=std::vector<line>();

  for (size_t i=0;i<nbIndex;i++) {
	  line newLine;
	  newLine.coord=*geom->GetVertex(newLine.vertexId=selFacet->indices[i]);
	  lines.push_back(newLine);
  }

  RebuildList();

}

/**
* \brief Displays window with selected facet data
* \param w Worker handle
*/
void FacetCoordinates::Display(Worker *w) {

  worker = w;
  SetVisible(true);
  UpdateFromSelection();

}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void FacetCoordinates::ProcessMessage(GLComponent *src,int message) {

  Geometry *geom = worker->GetGeometry();
  switch(message) {
    case MSG_BUTTON:
      if(src==dismissButton) {
        SetVisible(false);
	  } else if (src==insertLastButton) {
		  int vertexId;
		  size_t rowId=facetListC->GetNbRow();
		  if (!(insertIdText->GetNumberInt(&vertexId)) || !(vertexId>=1 && vertexId<=geom->GetNbVertex())) {
			  GLMessageBox::Display("Wrong vertex Id entered","Wrong number",GLDLG_OK,GLDLG_ICONWARNING);
			  break;
		  }
		  InsertVertex(rowId,vertexId-1);
	  } else if (src==insertBeforeButton) {
		  int vertexId;
		  int rowId=facetListC->GetSelectedRow();
		  if (!(insertIdText->GetNumberInt(&vertexId)) || !(vertexId>=1 && vertexId<=geom->GetNbVertex())) {
			  GLMessageBox::Display("Wrong vertex Id entered","Wrong number",GLDLG_OK,GLDLG_ICONWARNING);
			  break;
		  }
		  InsertVertex(rowId,vertexId-1);
	  } else if (src==removePosButton) {
		  RemoveRow(facetListC->GetSelectedRow());
	  } else if(src==updateButton) {
		  int rep = GLMessageBox::Display("Apply geometry changes ?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
		  if (rep == GLDLG_OK) {
			  ApplyChanges();
		  }
		  break;
	  }
	  else if (src == setXbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all X coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int row=0;row<(int)lines.size();row++) {
			  lines[row].coord.x=coordValue;
		  }
		  RebuildList();
	  }
	  else if (src == setYbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all Y coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int row = 0; row<(int)lines.size(); row++) {
			  lines[row].coord.y = coordValue;
		  }
		  RebuildList();
	  }
	  else if (src == setZbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all Z coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int row = 0; row<(int)lines.size(); row++) {
			  lines[row].coord.z = coordValue;
		  }
		  RebuildList();
	  }
    case MSG_LIST:
      if(src==facetListC) {
        int selRow=facetListC->GetSelectedRow()+1;
		insertBeforeButton->SetEnabled(selRow);
		removePosButton->SetEnabled(selRow);
      }
    break;

  }

  GLWindow::ProcessMessage(src,message);

}

/**
* \brief Update ID of the vertex to be inserted to vertexId
* \param vertexId ID of the vertex
*/
void FacetCoordinates::UpdateId(int vertexId) {
	char tmp[64];
	sprintf(tmp,"%d",vertexId+1);
	insertIdText->SetText(tmp);
}

/**
* \brief Rebuild list with the buffered data
*/
void FacetCoordinates::RebuildList() {
	facetListC->SetSize(5,(int)lines.size());
	
	facetListC->SetColumnWidths((int*)flWidth);
	facetListC->SetColumnLabels(flName);
	facetListC->SetColumnAligns((int *)flAligns);
	facetListC->SetColumnEditable((int *)fEdits);
	
	char tmp[128];

	for(size_t i=0;i<lines.size();i++) {
		
		sprintf(tmp,"%zd",i+1);
		facetListC->SetValueAt(0,i,tmp);
		
		sprintf(tmp,"%zd",lines[i].vertexId+1);
		facetListC->SetValueAt(1,i,tmp);
    
		sprintf(tmp,"%.10g",lines[i].coord.x);
		facetListC->SetValueAt(2,i,tmp);

		sprintf(tmp,"%.10g",lines[i].coord.y);
		facetListC->SetValueAt(3,i,tmp);

		sprintf(tmp,"%.10g",lines[i].coord.z);
		facetListC->SetValueAt(4,i,tmp);
  }
	int selRow=facetListC->GetSelectedRow()+1;
		insertBeforeButton->SetEnabled(selRow);
		removePosButton->SetEnabled(selRow);
}

/**
* \brief Remove row from table with ID rowId
* \param rowId ID of the row to be removed
*/
void FacetCoordinates::RemoveRow(size_t rowId){
	if (rowId < lines.size()) {
		lines.erase(lines.begin() + rowId);
		RebuildList();
		if (rowId < facetListC->GetNbRow()) {
			facetListC->SetSelectedRow((int)rowId);
		}
		else { //Select last
			facetListC->SetSelectedRow((int)facetListC->GetNbRow()-1);
		}
		int selRow = facetListC->GetSelectedRow() + 1;
		insertBeforeButton->SetEnabled(selRow);
		removePosButton->SetEnabled(selRow);
	}
}

/**
* \brief Insert vertex data into the table
* \param rowId ID of the row where new vertex should be added
* \param vertexId ID of the vertex to be added
*/
void FacetCoordinates::InsertVertex(size_t rowId,size_t vertexId){
	line newLine;
	newLine.vertexId=vertexId;
	newLine.coord=*(worker->GetGeometry()->GetVertex(vertexId));
	lines.insert(lines.begin()+rowId,newLine);
	RebuildList();

	facetListC->SetSelectedRow((int)rowId+1);
	int selRow=facetListC->GetSelectedRow()+1;
	insertBeforeButton->SetEnabled(selRow);
	removePosButton->SetEnabled(selRow);
	facetListC->ScrollToVisible(rowId,0);
}

/**
* \brief Apply changes to the geometry made in the table
*/
void FacetCoordinates::ApplyChanges(){
	
	Geometry *geom = worker->GetGeometry();
	
	if (facetListC->GetNbRow()<3) {
		GLMessageBox::Display("A facet must have at least 3 vertices","Not enough vertices",GLDLG_OK,GLDLG_ICONWARNING);
		return;
	}
	
	//validate user inputs
	for (int row=0;row<(int)lines.size();row++) {
		double x,y,z;
		if (!(lines[row].vertexId >= 0 && lines[row].vertexId<geom->GetNbVertex())) { //wrong coordinates at row
			char tmp[128];
			sprintf(tmp, "Invalid vertex id in row %d\n Vertex %zd doesn't exist.", row + 1, lines[row].vertexId + 1);
			GLMessageBox::Display(tmp, "Incorrect vertex id", GLDLG_OK, GLDLG_ICONWARNING);
			return;
		}
		bool success = (1==sscanf(facetListC->GetValueAt(2,row),"%lf",&x));
		success = success && (1==sscanf(facetListC->GetValueAt(3,row),"%lf",&y));
		success = success && (1==sscanf(facetListC->GetValueAt(4,row),"%lf",&z));
		if (!success) { //wrong coordinates at row
				char tmp[128];
				sprintf(tmp,"Invalid coordinates in row %d",row+1);
				GLMessageBox::Display(tmp,"Incorrect vertex",GLDLG_OK,GLDLG_ICONWARNING);
				return;
		}
		lines[row].coord.x=x;
		lines[row].coord.y=y;
		lines[row].coord.z=z;
	}

	//int rep = GLMessageBox::Display("Apply geometry changes ?","Question",GLDLG_OK|GLDLG_CANCEL,GLDLG_ICONWARNING);
	//if( rep == GLDLG_OK ) {

		if (mApp->AskToReset(worker)) {
			mApp->changedSinceSave=true;

			//Change number of vertices
			selFacet->sh.nbIndex = (int)lines.size();
			selFacet->indices.resize(selFacet->sh.nbIndex);
			selFacet->vertices2.resize(selFacet->sh.nbIndex);
			selFacet->visible.resize(selFacet->sh.nbIndex);

			for(size_t i=0;i<lines.size();i++) {
				geom->MoveVertexTo(lines[i].vertexId,lines[i].coord.x,lines[i].coord.y,lines[i].coord.z);
				selFacet->indices[i]=lines[i].vertexId;
			}
			geom->Rebuild(); //Will recalculate facet parameters

			// Send to sub process
			try {
				worker->Reload();
			} catch(Error &e) {
				GLMessageBox::Display(e.GetMsg(),"Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			//GLWindowManager::FullRepaint();
		}
	//}
}