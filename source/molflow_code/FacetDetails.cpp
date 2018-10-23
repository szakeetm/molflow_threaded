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
#include "FacetDetails.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/MathTools.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"

#include "MolFlow.h"
extern MolFlow *mApp;

typedef struct {

  const char *name;
  int   width;
  int   align;
  int   timeDepColor; //Can display time-dependent value, change color accordingly

} COLUMN;

static COLUMN allColumn[] = {
  {"#"             , 40 , ALIGN_CENTER, 0} ,
  {"Sticking"      , 80 , ALIGN_CENTER, 0 } ,
  {"Opacity"       , 80 , ALIGN_CENTER, 0 } ,
  {"Structure"     , 80 , ALIGN_CENTER, 0 } ,
  {"Link"          , 40 , ALIGN_CENTER, 0 } ,
  {"Desorption"    , 80 , ALIGN_CENTER, 0 } ,
  {"Reflection"    , 80 , ALIGN_CENTER, 0 } ,
  {"2 Sided"       , 60 , ALIGN_CENTER, 0 } ,
  {"Vertex"        , 80 , ALIGN_CENTER, 0 } ,
  {"Area"          , 80 , ALIGN_CENTER, 0 } ,
  {"Temperature (K)",80 , ALIGN_CENTER, 0 } ,
  {"Facet 2D Box"  , 100, ALIGN_CENTER, 0 } ,
  {"Texture (u,v)" , 120, ALIGN_CENTER, 0 } ,
  {"Mesh sample/cm", 80 , ALIGN_CENTER, 0 } ,
  {"Count"         , 80 , ALIGN_CENTER, 0 } ,
  {"Memory"        , 80 , ALIGN_CENTER, 0 } ,
  {"Planarity"     , 80 , ALIGN_CENTER, 0 } ,
  {"Profile"       , 80 , ALIGN_CENTER, 0 } ,
  {"Imping.rate"   , 80, ALIGN_CENTER, COLOR_BLUE },
  {"Density [1/m3]", 80, ALIGN_CENTER, COLOR_BLUE },
  {"Density [kg/m3]",80, ALIGN_CENTER, COLOR_BLUE },
  {"Pressure [mbar]",80 , ALIGN_CENTER, COLOR_BLUE } ,
  {"Av.mol.speed[m/s]",80, ALIGN_CENTER, COLOR_BLUE },
  {"MC Hits"           , 80 , ALIGN_CENTER, COLOR_BLUE } ,
  {"Equiv.hits"           , 80 , ALIGN_CENTER, COLOR_BLUE } ,
  {"Des."           , 80 , ALIGN_CENTER, COLOR_BLUE } ,
  {"Equiv.abs."           , 80 , ALIGN_CENTER, COLOR_BLUE } ,
};

static const char *desStr[] = {
  "None",
  "Uniform",
  "Cosine",
  "Cosine^"
};

static const char *profStr[] = {
  "None",
  "Pressure (\201)",
  "Pressure (\202)",
  "Angular",
  "Speed distr.",
  "Ort. velocity",
  "Tan. velocity"
};

static const char *ynStr[] = {
  "No",
  "Yes"
};

FacetDetails::FacetDetails():GLWindow() {

  int wD = 502;
  int hD = 400;
  worker = NULL;

  SetTitle("Facets details");
  SetIconfiable(true);
  SetResizable(true);
  SetMinimumSize(502,200);

  checkAllButton = new GLButton(0,"Check All");
  Add(checkAllButton);
  uncheckAllButton = new GLButton(0,"Uncheck All");
  Add(uncheckAllButton);
  updateButton = new GLButton(0,"Update");
  Add(updateButton);
  dismissButton = new GLButton(0,"Dismiss");
  Add(dismissButton);

  facetListD = new GLList(0);
  facetListD->SetColumnLabelVisible(true);
  facetListD->SetGrid(true);
  //facetListD->Sortable=true;
  Add(facetListD);

  sPanel = new GLTitledPanel("Show column");
  sPanel->SetClosable(true);
  Add(sPanel);

  show[0] = new GLToggle(0,"#");
  show[0]->SetState(true);  // Always visible (not displayed)

  show[1] = new GLToggle(1,"Sticking");
  show[1]->SetState(true);
  sPanel->Add(show[1]);
  show[2] = new GLToggle(2,"Opacity");
  show[2]->SetState(true);
  sPanel->Add(show[2]);
  show[3] = new GLToggle(3,"Structure");
  show[3]->SetState(true);
  sPanel->Add(show[3]);
  show[4] = new GLToggle(4,"Link");
  show[4]->SetState(true);
  sPanel->Add(show[4]);
  show[5] = new GLToggle(5,"Desorption");
  show[5]->SetState(true);
  sPanel->Add(show[5]);
  show[6] = new GLToggle(6,"Reflection");
  show[6]->SetState(true);
  sPanel->Add(show[6]);
  show[7] = new GLToggle(7,"2 Sided");
  show[7]->SetState(true);
  sPanel->Add(show[7]);
  show[8] = new GLToggle(8,"Vertex nb");
  show[8]->SetState(true);
  sPanel->Add(show[8]);
  show[9] = new GLToggle(9,"Area");
  show[9]->SetState(true);
  sPanel->Add(show[9]);
  show[10] = new GLToggle(10, "Temperature");
  show[10]->SetState(true);
  sPanel->Add(show[10]);
  show[11] = new GLToggle(11,"2D Box");
  show[11]->SetState(true);
  sPanel->Add(show[11]);
  show[12] = new GLToggle(12,"Texture UV");
  show[12]->SetState(true);
  sPanel->Add(show[12]);
  show[13] = new GLToggle(13,"Mesh sample/cm");
  show[13]->SetState(true);
  sPanel->Add(show[13]);
  show[14] = new GLToggle(14,"Count mode");
  show[14]->SetState(true);
  sPanel->Add(show[14]);
  show[15] = new GLToggle(15,"Memory");
  show[15]->SetState(true);
  sPanel->Add(show[15]);
  show[16] = new GLToggle(16,"Planarity");
  show[16]->SetState(true);
  sPanel->Add(show[16]);
  show[17] = new GLToggle(17,"Profile");
  show[17]->SetState(true);
  sPanel->Add(show[17]);
  show[18] = new GLToggle(18, "Imping.rate");
  show[18]->SetState(true);
  sPanel->Add(show[18]);
  show[19] = new GLToggle(19, "Density[1/m3]");
  show[19]->SetState(true);
  sPanel->Add(show[19]);
  show[20] = new GLToggle(20, "Density[kg/m3]");
  show[20]->SetState(true);
  sPanel->Add(show[20]);
  show[21] = new GLToggle(21, "Pressure");
  show[21]->SetState(true);
  sPanel->Add(show[21]);
  show[22] = new GLToggle(22, "Mol.speed");
  show[22]->SetState(true);
  sPanel->Add(show[22]);
  show[23] = new GLToggle(23,"MC Hits");
  show[23]->SetState(true);
  sPanel->Add(show[23]);
  show[24] = new GLToggle(24, "Equiv.hits");
  show[24]->SetState(true);
  sPanel->Add(show[24]);
  show[25] = new GLToggle(25,"Des.");
  show[25]->SetState(true);
  sPanel->Add(show[25]);
  show[26] = new GLToggle(26,"Equiv.Abs.");
  show[26]->SetState(true);
  sPanel->Add(show[26]);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  RestoreDeviceObjects();

}

void FacetDetails::PlaceComponents() {

  // Show toggle panel
  int nbW = (width-20)/80;
  int nbL = (NB_FDCOLUMN-1)/nbW + (((NB_FDCOLUMN-1)%nbW)?1:0);
  int hp;
  if(!sPanel->IsClosed())
    hp = 20*(nbL+1);
  else
    hp = 20;
  sPanel->SetBounds(5,height-(hp+52),width-10,hp);
  for(size_t i=0;i<NB_FDCOLUMN;i++)
    sPanel->SetCompBounds(show[i],5+80*(((int)i-1)%nbW),18+20*(((int)i-1)/nbW),85,19);

  facetListD->SetBounds(5,5,width-10,height-(62+hp));

  checkAllButton->SetBounds(5,height-45,90,19);
  uncheckAllButton->SetBounds(100,height-45,90,19);
  updateButton->SetBounds(195,height-45,90,19);
  dismissButton->SetBounds(width-100,height-45,90,19);

}

void FacetDetails::SetBounds(int x,int y,int w,int h) {

  GLWindow::SetBounds(x,y,w,h);
  PlaceComponents();

}

char *FacetDetails::GetCountStr(Facet *f) {
  static char ret[128];
  strcpy(ret,"");
  if(f->sh.countDes) strcat(ret,"DES");
  if(f->sh.countAbs) if(strlen(ret)==0) strcat(ret,"ABS"); else strcat(ret,"+ABS");
  if(f->sh.countRefl) if(strlen(ret)==0) strcat(ret,"REFL"); else strcat(ret,"+REFL");
  if(f->sh.countTrans) if(strlen(ret)==0) strcat(ret,"TRANS"); else strcat(ret,"+TRANS");
  return ret;
}

char *FacetDetails::FormatCell(size_t idx,Facet *f,size_t mode) {
  static char ret[512];
  strcpy(ret,"");

  switch(mode) {
    case 0:
      sprintf(ret,"%zd",idx+1);
      break;
    case 1:
      sprintf(ret,"%g",f->sh.sticking);
      break;
    case 2:
      sprintf(ret,"%g",f->sh.opacity);
      break;
    case 3:
	{
		std::ostringstream out;
		if (f->sh.superIdx == -1) out << "All";
		else out << (f->sh.superIdx + 1);
		sprintf(ret, "%s", out.str().c_str());
		break;
	}
    case 4:
      sprintf(ret,"%zd",f->sh.superDest);
      break;
    case 5:
	  if (f->sh.desorbType == DES_COSINE_N)
	  {
		  sprintf(ret, "%s%g", desStr[f->sh.desorbType], f->sh.desorbTypeN); //append exponent
	  }
	  else
	  {
		  sprintf(ret, "%s", desStr[f->sh.desorbType]);
	  }
      break;
    case 6:
      sprintf(ret,"%g diff. %g spec. %g cos^%g",f->sh.reflection.diffusePart,f->sh.reflection.specularPart,1.0-f->sh.reflection.diffusePart-f->sh.reflection.specularPart,f->sh.reflection.cosineExponent);
      break;
    case 7:
      sprintf(ret,"%s",ynStr[f->sh.is2sided]);      
      break;
    case 8:
      sprintf(ret,"%zd",f->sh.nbIndex);
      break;
    case 9:
		if (f->sh.is2sided) sprintf(ret,"2*%g",f->sh.area);
		else sprintf(ret,"%g",f->sh.area);
      break;
	case 10:
		sprintf(ret, "%g", f->sh.temperature);
		break;
    case 11:
      sprintf(ret,"%g x %g",f->sh.U.Norme(),f->sh.V.Norme());
      break;
    case 12:
      if( f->sh.isTextured ) {
        sprintf(ret,"%zdx%zd (%g x %g)",f->sh.texWidth,f->sh.texHeight,f->sh.texWidthD,f->sh.texHeightD);
      } else {
        sprintf(ret,"None");
      }
      break;
    case 13:
      sprintf(ret,"%g",f->tRatio);
      break;
    case 14:
      sprintf(ret,"%s",GetCountStr(f));
      break;
    case 15:
		sprintf(ret,"%s",FormatMemory(f->GetTexRamSize(1+worker->moments.size())));
      break;
    case 16:
      sprintf(ret,"%f",f->planarityError);
      break;
    case 17:
		sprintf(ret,"%s",profStr[f->sh.profileType]);
		break;
	case 18: //imp.rate
	{
	double dCoef = 1E4 * worker->GetMoleculesPerTP(worker->displayedMoment);  //1E4 is conversion from m2 to cm2; 0.01 is Pa->mbar
	sprintf(ret, "%g", f->facetHitCache.nbHitEquiv / f->GetArea()*dCoef);
	//11.77=sqrt(8*8.31*293.15/3.14/0.028)/4/10
	break; }
	case 19: //particle density
	{
	double dCoef = 1E4 * worker->GetMoleculesPerTP(worker->displayedMoment)*f->DensityCorrection();  //1E4 is conversion from m2 to cm2; 0.01 is Pa->mbar	
	
	sprintf(ret, "%g", f->facetHitCache.sum_1_per_ort_velocity / f->GetArea()*dCoef);

	break; }
	case 20: //gas density
	{
	double dCoef =  1E4 * worker->GetMoleculesPerTP(worker->displayedMoment)*f->DensityCorrection();  //1E4 is conversion from m2 to cm2; 0.01 is Pa->mbar
	
	sprintf(ret, "%g", f->facetHitCache.sum_1_per_ort_velocity / f->GetArea()*dCoef*mApp->worker.wp.gasMass / 1000.0 / 6E23);
	break; }
	case 21: //avg.pressure
	{
	double dCoef = 1E4 * worker->GetMoleculesPerTP(worker->displayedMoment) * (worker->wp.gasMass / 1000 / 6E23) * 0.0100;  //1E4 is conversion from m2 to cm2; 0.01 is Pa->mbar
	
	sprintf(ret, "%g", f->facetHitCache.sum_v_ort*dCoef / f->GetArea());
	break; }
	case 22: //avg. gas speed (estimate)
		/*sprintf(ret, "%g", 4.0*(double)(f->facetHitCache.nbMCHit+f->facetHitCache.nbDesorbed) / f->facetHitCache.sum_1_per_ort_velocity);*/
		sprintf(ret, "%g", (f->facetHitCache.nbHitEquiv + static_cast<double>(f->facetHitCache.nbDesorbed)) / f->facetHitCache.sum_1_per_velocity);
		//<v_surf>=2*<v_surf_ort>
		//<v_gas>=1/<1/v_surf>
		break;
	case 23:
		sprintf(ret,"%zd",f->facetHitCache.nbMCHit);
		break;
	case 24:
		sprintf(ret, "%g", f->facetHitCache.nbHitEquiv);
		break;
	case 25:
		sprintf(ret,"%zd",f->facetHitCache.nbDesorbed);
		break;
	case 26:
		sprintf(ret,"%g",f->facetHitCache.nbAbsEquiv);
		break;
  }

  return ret;

}

void FacetDetails::UpdateTable() {

  Geometry *geom = worker->GetGeometry();
  auto selectedFacets = geom->GetSelectedFacets();
  static char ret[256];
  strcpy(ret,"");

  const char *tmpName[NB_FDCOLUMN];
  int  tmpWidth[NB_FDCOLUMN];
  int  tmpAlign[NB_FDCOLUMN];
  int  tmpColor[NB_FDCOLUMN];

  size_t nbCol = 0;

  for(size_t i=0;i<NB_FDCOLUMN;i++) {
    if(i==0 || show[i]->GetState()) {
      tmpName[nbCol]  = allColumn[i].name;
      tmpWidth[nbCol] = allColumn[i].width;
      tmpAlign[nbCol] = allColumn[i].align;
	  tmpColor[nbCol] = allColumn[i].timeDepColor;
      shown[nbCol] = i;
      nbCol++;
    }
  }

  facetListD->SetSize(nbCol,selectedFacets.size());
  facetListD->SetColumnWidths(tmpWidth);
  facetListD->SetColumnLabels(tmpName);
  facetListD->SetColumnAligns(tmpAlign);
  
  if (worker->displayedMoment == 0)
	  facetListD->SetAllColumnColors(COLOR_BLACK);
  else 
	facetListD->SetColumnColors(tmpColor);
  

  size_t nbS = 0;
  for(auto& sel:selectedFacets) {
    Facet *f = geom->GetFacet(sel);
    for(size_t j=0;j<nbCol;j++)
        facetListD->SetValueAt(j,nbS,FormatCell(sel,f,shown[j]));
	nbS++;
	}
}

void FacetDetails::Update() {

  if(!worker) return;
  if(!IsVisible()) return;

  Geometry *s = worker->GetGeometry();
  size_t nbS = s->GetNbSelectedFacets();
  
  if(nbS==0) {
    facetListD->Clear();
    return;
  }

  UpdateTable();

}

void FacetDetails::Display(Worker *w) {

  worker = w;
  SetVisible(true);
  Update();

}

void FacetDetails::ProcessMessage(GLComponent *src,int message) {

  switch(message) {

    case MSG_BUTTON:
      if(src==dismissButton) {
        SetVisible(false);
      } else if (src==checkAllButton) {
        for(size_t i=0;i<NB_FDCOLUMN;i++) show[i]->SetState(true);
        UpdateTable();
      } else if (src==uncheckAllButton) {
        for(size_t i=0;i<NB_FDCOLUMN;i++) show[i]->SetState(false);
        UpdateTable();
	  }	else if (src==updateButton) {
        UpdateTable();
      }
      break;

    case MSG_TOGGLE:
      UpdateTable();
      break;

    case MSG_LIST_COL:
      if( src==facetListD ) {
        // Save column width
        int c = facetListD->GetDraggedCol();
        allColumn[shown[c]].width = facetListD->GetColWidth(c);
      }
      break;

    case MSG_PANELR:
      PlaceComponents();
      break;

  }

  GLWindow::ProcessMessage(src,message);

}

