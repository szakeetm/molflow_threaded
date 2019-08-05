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
#include "OutgassingMap.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
//#include "GLApp/GLFileBox.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLList.h"
#include "GLApp/GLCombo.h"
#include "MolFlow.h"
#include "MolflowGeometry.h" //Explode command is Molflow-specific
#include "Facet_shared.h"
#include "SuperFacet.h"

extern MolFlow *mApp;

//static const char *fileFilters = "Text files\0*.txt";
//static const int   nbFilter = sizeof(fileFilters) / (2*sizeof(char *));

/**
* \brief Constructor with initialisation for Outgassing map window (Facet/Convert to outgassing map)
*/
OutgassingMap::OutgassingMap():GLWindow() {

  int wD = 600;
  int hD = 300;
  lastUpdate = 0.0f;
  strcpy(currentDir,".");

  SetTitle("Outgassing map");
  SetResizable(true);
  SetIconfiable(true);
  SetMinimumSize(wD,hD);

  mapList = new GLList(0);
  mapList->SetColumnLabelVisible(true);
  mapList->SetRowLabelVisible(true);
  mapList->SetAutoColumnLabel(true);
  mapList->SetAutoRowLabel(true);
  //mapList->SetRowLabelMargin(20);
  mapList->SetGrid(true);
  mapList->SetSelectionMode(SINGLE_CELL);
  mapList->SetCornerLabel("\202\\\201");
  Add(mapList);

  
  desLabel = new GLLabel("Desorption type:");
  Add(desLabel);
  desCombo = new GLCombo(0);
  desCombo->SetSize(3);
  desCombo->SetValueAt(0,"Uniform");
  desCombo->SetValueAt(1,"Cosine");
  desCombo->SetValueAt(2,"Cosine^N");
  
  desCombo->SetSelectedIndex(1);
  Add(desCombo);
  

  /*
  loadButton = new GLButton(0,"Load");
  Add(loadButton);
  */

  sizeButton = new GLButton(0,"Auto size");
  Add(sizeButton);

  explodeButton = new GLButton(0,"Explode");
  Add(explodeButton);

  pasteButton = new GLButton(0,"Paste");
  Add(pasteButton);

  /*
  maxButton = new GLButton(0,"Find Max.");
  Add(maxButton);
  */

  cancelButton = new GLButton(0,"Dismiss");
  Add(cancelButton);

  exponentText = new GLTextField(0,"");
  exponentText->SetEditable(false);
  Add(exponentText);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  RestoreDeviceObjects();

  worker = NULL;

}

/**
* \brief Places all components (buttons, text etc.) at the right position inside the window
*/
void OutgassingMap::PlaceComponents() {

  mapList->SetBounds(5,5,width-15,height-55);
  //saveButton->SetBounds(10,height-45,70,19);
  explodeButton->SetBounds(10,height-45,70,19);
  sizeButton->SetBounds(90,height-45,70,19);
  pasteButton->SetBounds(170,height-45,70,19);
  //maxButton->SetBounds(170,height-45,70,19);
  desLabel->SetBounds(270,height-45,30,19);
  desCombo->SetBounds(370,height-45,80,19);
  cancelButton->SetBounds(width-90,height-45,80,19);
  exponentText->SetBounds(450,height-45,50,19);
}

/**
* \brief Places all components (buttons, text etc.) at the right position inside the window
* \param x x-Coordinate of the window
* \param y y-Coordinate of the window
* \param w width of the window
* \param h height of the window
*/
void OutgassingMap::SetBounds(int x,int y,int w,int h) {

  GLWindow::SetBounds(x,y,w,h);
  PlaceComponents();

}

/**
* \brief Sets lowest selected facet id in the title
*/
void OutgassingMap::GetSelected() {

  if(!worker) return;

  Geometry *geom = worker->GetGeometry();
  selFacet = NULL;
  int i = 0;
  size_t nb = geom->GetNbFacet();
  while(!selFacet && i<nb) {
    if( geom->GetFacet(i)->selected ) selFacet = geom->GetFacet(i);
    if(!selFacet) i++;
  }

  char tmp[64];
  sprintf(tmp,"Outgassing map for Facet #%d",i+1);
  SetTitle(tmp);

}

/**
* \brief Checks and executes an update if necessary
* \param appTime current time of the application
* \param force if table should be updated no matter what
*/
void OutgassingMap::Update(float appTime,bool force) {

  if(!IsVisible()) return;

  if(force) {
    UpdateTable();
    lastUpdate = appTime;
    return;
  }

  if( (appTime-lastUpdate>1.0f) ) {
    if(worker->isRunning) UpdateTable();
    lastUpdate = appTime;
  }

}

/**
* \brief Updates the values inside the table if a profile is set
*/
void OutgassingMap::UpdateTable() {
	//maxValue=0.0f;
	//double scale;
  GetSelected();
  if( !selFacet ) {
    mapList->Clear();
    return;
  }

  //SHELEM *mesh = selFacet->mesh;
  if( selFacet->cellPropertiesIds ) {

    char tmp[256];
	size_t w = selFacet->sh.texWidth;
	size_t h = selFacet->sh.texHeight;
    mapList->SetSize(w,h);
    mapList->SetAllColumnAlign(ALIGN_CENTER);
	mapList->SetGrid(true);
	mapList->SetColumnLabelVisible(true);
	//mapList->SetSelectionMode(SINGLE_ROW);

    //int mode = viewCombo->GetSelectedIndex();

    
        for(size_t i=0;i<w;i++) { //width (cols)
			*(mapList->cEdits+i)=EDIT_NUMBER;
          for(size_t j=0;j<h;j++) { //height (rows)
			  sprintf(tmp,"0");
			  if (selFacet->GetMeshArea(i+j*w)==0.0) sprintf(tmp,"Outside");
			  mapList->SetValueAt(i,j,tmp);
          }
        }   
    }
  else {
	  mapList->Clear();
	  mapList->SetSize(0,0);
  }
}

/**
* \brief Displays the window
* \param w worker handle
*/
void OutgassingMap::Display(Worker *w) {

  worker = w;
  UpdateTable();
  SetVisible(true);

}

/**
* \brief Close the window
*/
void OutgassingMap::Close() {
  worker = NULL;
  if(selFacet) selFacet->UnselectElem();
  mapList->Clear();
}

/*
void OutgassingMap::SaveFile() {

  if(!selFacet) return;

  FILENAME *fn = GLFileBox::SaveFile(currentDir,NULL,"Save File",fileFilters,nbFilter);

  if( fn ) {

    int u,v,wu,wv;
    if( !mapList->GetSelectionBox(&u,&v,&wu,&wv) ) {
      u=0;
      v=0;
      wu = mapList->GetNbRow();
      wv = mapList->GetNbColumn();
    }

    // Save tab separated text
    FILE *f = fopen(fn->fullName,"w");

    if( f==NULL ) {
      char errMsg[512];
      sprintf(errMsg,"Cannot open file\nFile:%s",fn->fullName);
      GLMessageBox::Display(errMsg,"Error",GLDLG_OK,GLDLG_ICONERROR);
      return;
    }

    for(int i=u;i<u+wu;i++) {
      for(int j=v;j<v+wv;j++) {
        char *str = mapList->GetValueAt(j,i);
        if( str ) fprintf(f,"%s",str);
        if( j<v+wv-1 ) 
          fprintf(f,"\t");
      }
      fprintf(f,"\r\n");
    }
    fclose(f);

  }

}
*/

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void OutgassingMap::ProcessMessage(GLComponent *src,int message) {

  switch(message) {

    case MSG_CLOSE:
      Close();
      break;

    case MSG_BUTTON:
      if(src==cancelButton) {
        Close();
        GLWindow::ProcessMessage(NULL,MSG_CLOSE);
      } else if(src==pasteButton) {
		  mapList->PasteClipboardText(false,false);
      } else if (src==sizeButton) {
        mapList->AutoSizeColumn();
      } else if (src==explodeButton) {
		  if (worker->GetGeometry()->GetNbSelectedFacets()!=1) {
			  GLMessageBox::Display("Exactly one facet has to be selected","Error",GLDLG_OK,GLDLG_ICONERROR);
			  return;
		  }
		  if (!selFacet->hasMesh) {
			  GLMessageBox::Display("Selected facet must have a mesh","Error",GLDLG_OK,GLDLG_ICONERROR);
			  return;
		  }
		  
		  size_t w = mapList->GetNbColumn(); //width
		  size_t h = mapList->GetNbRow(); //height

		  double *values = (double *)malloc(w*h*sizeof(double));
		  size_t count=0;
		  for(size_t j=0;j<h;j++) { //height (rows)
			  for(size_t i=0;i<w;i++) { //width (cols)
				  if (selFacet->GetMeshArea(i+j*w)>0.0) {
					  char *str = mapList->GetValueAt(i,j);
					  int conv = sscanf(str,"%lf",values+count);
					  if(!conv || !(*(values+count++)>=0.0)) {
						  mapList->SetSelectedCell(i,j);
						  char tmp[256];
						  sprintf(tmp,"Invalid outgassing number at Cell(%zd,%zd)",i,j);
						  GLMessageBox::Display(tmp,"Error",GLDLG_OK,GLDLG_ICONERROR);
						  return;
					  }
				  }
			  }
		  }
		  double desorbTypeN;
		  if (desCombo->GetSelectedIndex()==2) {
			  exponentText->GetNumber(&desorbTypeN) ;
				  if( !(desorbTypeN>1.0) ) {
					  exponentText->SetFocus(true);
					  GLMessageBox::Display("Desorption type exponent must be greater than 1.0","Error",GLDLG_OK,GLDLG_ICONERROR);
					  return;
				  }
			  
		  }
		  if( GLMessageBox::Display("Explode selected facet?","Question",GLDLG_OK|GLDLG_CANCEL,GLDLG_ICONINFO)==GLDLG_OK ) {	

			  if (mApp->AskToReset()) {
				  mApp->changedSinceSave=true;
				  try { 
					  worker->GetMolflowGeometry()->ExplodeSelected(true,desCombo->GetSelectedIndex(),desorbTypeN,values);
					  SAFE_FREE(values);
					  mApp->UpdateModelParams();
					  mApp->UpdateFacetParams(true);
					  worker->CalcTotalOutgassing();
					  // Send to sub process
					  worker->Reload(); } catch(Error &e) {
						  GLMessageBox::Display(e.GetMsg(),"Error exploding facet (not enough memory?)",GLDLG_OK,GLDLG_ICONERROR);
					}
				}
				        Close();
        GLWindow::ProcessMessage(NULL,MSG_CLOSE);
		  }
      }/*else if (src==saveButton) {
        SaveFile();
      } else if (src==maxButton) {
		         int u,v,wu,wv;
		  mapList->SetSelectedCell(maxX,maxY);
		  if( mapList->GetSelectionBox(&v,&u,&wv,&wu) )
          selFacet->SelectElem(u,v,wu,wv);
      }
	  */
      break;

    case MSG_LIST:
      if(src==mapList) {
        size_t u,v,wu,wv;
        if( mapList->GetSelectionBox(&v,&u,&wv,&wu) )
          selFacet->SelectElem(u,v,wu,wv);
      }
      break;

    case MSG_COMBO:
      if(src==desCombo) {
		  bool cosineN = desCombo->GetSelectedIndex()==2;
		  exponentText->SetEditable(cosineN);
		  if (!cosineN) exponentText->SetText("");
      }
      break;

  }

  GLWindow::ProcessMessage(src,message);
}
