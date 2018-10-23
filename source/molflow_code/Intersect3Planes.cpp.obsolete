/*
File:        Intersect3Planes.cpp
Description: Define three planes and create a vertex at their intersection
*/

#define MODE_FACET 1
#define MODE_3VERTEX 2
#define MODE_EQUATION 3

#include "Intersect3Planes.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "MolFlow.h"

extern MolFlow *theApp;
int    plane1Mode,plane2Mode,plane3Mode;

Intersect3Planes::Intersect3Planes(Geometry *g,Worker *w):GLWindow() {

	plane1Mode = plane2Mode = plane3Mode = MODE_FACET;
	int wD = 350;
	int hD = 375;

	SetTitle("Find intersection of 3 planes");

	plane1Panel = new GLTitledPanel("Plane 1");
	iPanel->SetBounds(5,5,wD-10,290);
	Add(iPanel);

	l1 = new GLToggle(0,"Axis X");
	l1->SetBounds(10,20,100,18);
	iPanel->Add(l1);

	l2 = new GLToggle(0,"Axis Y");
	l2->SetBounds(10,45,100,18);
	iPanel->Add(l2);

	l3 = new GLToggle(0,"Axis Z");
	l3->SetBounds(10,70,100,18);
	iPanel->Add(l3);

	GLTitledPanel *facetPanel = new GLTitledPanel("From facet");
	facetPanel->SetBounds(10,90,330,90);
	iPanel->Add(facetPanel);

	l4 = new GLToggle(0,"U vector");
	l4->SetBounds(15,105,100,18);
	facetPanel->Add(l4);

	l5 = new GLToggle(0,"V vector");
	l5->SetBounds(15,130,100,18);
	facetPanel->Add(l5);

	l6 = new GLToggle(0,"Normal vector");
	l6->SetBounds(15,155,100,18);
	facetPanel->Add(l6);

	lNum = new GLLabel("of facet #");
	lNum->SetBounds(100,120,80,18);
	iPanel->Add(lNum);

	facetNumber = new GLTextField(0,"0");
	facetNumber->SetBounds(160,120,60,18);
	facetNumber->SetEditable(FALSE);
	iPanel->Add(facetNumber);

	l7 = new GLToggle(0,"Define by 2 selected vertex");
	l7->SetBounds(10,190,100,18);
	iPanel->Add(l7);

	l8 = new GLToggle(0,"Define by equation:");
	l8->SetBounds(10,215,100,18);
	iPanel->Add(l8);

	aLabel = new GLLabel("Point: a:");
	aLabel->SetBounds(10,240,100,18);
	iPanel->Add(aLabel);
	
	aText = new GLTextField(0,"0");
	aText->SetBounds(120,240,30,18);
	aText->SetEditable(FALSE);
	iPanel->Add(aText);
		
	bLabel = new GLLabel("b:");
	bLabel->SetBounds(160,240,20,18);
	iPanel->Add(bLabel);
	
	bText = new GLTextField(0,"0");
	bText->SetBounds(190,240,30,18);
	bText->SetEditable(FALSE);
	iPanel->Add(bText);

	cLabel = new GLLabel("c:");
	cLabel->SetBounds(230,240,20,18);
	iPanel->Add(cLabel);

	cText = new GLTextField(0,"0");
	cText->SetBounds(260,240,30,18);
	cText->SetEditable(FALSE);
	iPanel->Add(cText);

	uLabel = new GLLabel("Direction: u:");
	uLabel->SetBounds(10,265,100,18);
	iPanel->Add(uLabel);

	uText = new GLTextField(0,"0");
	uText->SetBounds(120,265,30,18);
	uText->SetEditable(FALSE);
	iPanel->Add(uText);

	vLabel = new GLLabel("v");
	vLabel->SetBounds(160,265,20,18);
	iPanel->Add(vLabel);

	vText = new GLTextField(0,"0");
	vText->SetBounds(190,265,30,18);
	vText->SetEditable(FALSE);
	iPanel->Add(vText);

	wLabel = new GLLabel("w");
	wLabel->SetBounds(230,265,20,18);
	iPanel->Add(wLabel);

	wText = new GLTextField(0,"0");
	wText->SetBounds(260,265,30,18);
	wText->SetEditable(FALSE);
	iPanel->Add(wText);

	degLabel = new GLLabel("Degrees:");
	degLabel->SetBounds(10,300,100,18);
	Add(degLabel);

	degText = new GLTextField(0,"0");
	degText->SetBounds(105,300,60,18);
	degText->SetEditable(TRUE);
	Add(degText);

	moveButton = new GLButton(0,"Rotate");
	moveButton->SetBounds(5,hD-44,85,21);
	Add(moveButton);

	copyButton = new GLButton(0,"Copy");
	copyButton->SetBounds(95,hD-44,85,21);
	Add(copyButton);

	cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(185,hD-44,85,21);
	Add(cancelButton);

	// Center dialog
	int wS,hS;
	GLToolkit::GetScreenSize(&wS,&hS);
	int xD = (wS-wD)/2;
	int yD = (hS-hD)/2;
	SetBounds(xD,yD,wD,hD);

	RestoreDeviceObjects();

	geom = g;
	work = w;

}

void Intersect3Planes::ProcessMessage(GLComponent *src,int message) {
	MolFlow *mApp = (MolFlow *)theApp;
	double a,b,c,u,v,w,deg;
	int facetNum;

	switch(message) {
		
	case MSG_TOGGLE:
		UpdateToggle(src);
		break;

	case MSG_BUTTON:

		if(src==cancelButton) {

			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==moveButton || src==copyButton) {
			if (geom->GetNbSelected()==0) {
				GLMessageBox::Display("No facets selected","Nothing to mirror",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			//Calculate the plane
			VERTEX3D AXIS_P0,AXIS_DIR;
			int nbSelectedVertex;
			int *vIdx = (int *)malloc(geom->GetNbVertex()*sizeof(int));
			memset(vIdx,0xFF,geom->GetNbVertex()*sizeof(int));

			if (!(degText->GetNumber(&deg))) {
				GLMessageBox::Display("Invalid degree","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			switch (axisMode) {
			case XMODE:
				AXIS_P0.x=0.0;AXIS_P0.y=0.0;AXIS_P0.z=0.0;
				AXIS_DIR.x=1.0;AXIS_DIR.y=0.0;AXIS_DIR.z=0.0;
				break;
			case YMODE:
				AXIS_P0.x=0.0;AXIS_P0.y=0.0;AXIS_P0.z=0.0;
				AXIS_DIR.x=0.0;AXIS_DIR.y=1.0;AXIS_DIR.z=0.0;
				break;
			case ZMODE:
				AXIS_P0.x=0.0;AXIS_P0.y=0.0;AXIS_P0.z=0.0;
				AXIS_DIR.x=0.0;AXIS_DIR.y=0.0;AXIS_DIR.z=1.0;
				break;
			case FACETUMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				AXIS_P0=geom->GetFacet(facetNum-1)->sh.O;
				AXIS_DIR=geom->GetFacet(facetNum-1)->sh.U;
				break;
			case FACETVMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				AXIS_P0=geom->GetFacet(facetNum-1)->sh.O;
				AXIS_DIR=geom->GetFacet(facetNum-1)->sh.V;
				break;
			case FACETNMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				AXIS_P0=geom->GetFacet(facetNum-1)->sh.center;
				AXIS_DIR=geom->GetFacet(facetNum-1)->sh.N;
				break;
			case TWOVERTEXMODE:
				if (geom->GetNbSelectedVertex()!=2) {
					GLMessageBox::Display("Select exactly 2 vertices","Can't define axis",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				nbSelectedVertex = 0;

				for(int i=0;i<geom->GetNbVertex()&&nbSelectedVertex<geom->GetNbSelectedVertex();i++ ) {
					//VERTEX3D *v = GetVertex(i);
					if( geom->GetVertex(i)->selected ) {
						vIdx[nbSelectedVertex] = i;
						nbSelectedVertex++;
					}
				}

				AXIS_DIR.x = geom->GetVertex(vIdx[1])->x - geom->GetVertex(vIdx[0])->x;
				AXIS_DIR.y = geom->GetVertex(vIdx[1])->y - geom->GetVertex(vIdx[0])->y;
				AXIS_DIR.z = geom->GetVertex(vIdx[1])->z - geom->GetVertex(vIdx[0])->z;

				AXIS_P0.x = geom->GetVertex(vIdx[0])->x;
				AXIS_P0.y = geom->GetVertex(vIdx[0])->y;
				AXIS_P0.z = geom->GetVertex(vIdx[0])->z;

				break;
			case EQMODE:
				if( !(aText->GetNumber(&a)) ) {
					GLMessageBox::Display("Invalid a coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(bText->GetNumber(&b)) ) {
					GLMessageBox::Display("Invalid b coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(cText->GetNumber(&c)) ) {
					GLMessageBox::Display("Invalid c coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(uText->GetNumber(&u)) ) {
					GLMessageBox::Display("Invalid u coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(vText->GetNumber(&v)) ) {
					GLMessageBox::Display("Invalid v coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(wText->GetNumber(&w)) ) {
					GLMessageBox::Display("Invalid w coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}

				if ((u==0.0)&&(v==0.0)&&(w==0.0)) {
					GLMessageBox::Display("u, v, w are all zero. That's not a vector.","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}

				AXIS_P0.x=a;AXIS_P0.y=b;AXIS_P0.z=c;
				AXIS_DIR.x=u;AXIS_DIR.y=v;AXIS_DIR.z=w;
				break;
			default:
				GLMessageBox::Display("Select an axis definition mode.","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			SAFE_FREE(vIdx);
			if (mApp->AskToReset()) {
				geom->RotateSelectedFacets(AXIS_P0,AXIS_DIR,deg,src==copyButton,work);
				//theApp->UpdateModelParams();
				work->Reload(); 
				theApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
				//GLWindowManager::FullRepaint();
			}
		}
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

void Intersect3Planes::UpdateToggle(GLComponent *src) {
	l1->SetCheck(FALSE);
	l2->SetCheck(FALSE);
	l3->SetCheck(FALSE);
	l4->SetCheck(FALSE);
	l5->SetCheck(FALSE);
	l6->SetCheck(FALSE);
	l7->SetCheck(FALSE);
	l8->SetCheck(FALSE);

	GLToggle *toggle=(GLToggle*)src;
	toggle->SetCheck(TRUE);

	facetNumber->SetEditable(src==l4||src==l5||src==l6);
	aText->SetEditable(src==l8);
	bText->SetEditable(src==l8);
	cText->SetEditable(src==l8);
	uText->SetEditable(src==l8);
	vText->SetEditable(src==l8);
	wText->SetEditable(src==l8);

	if (src==l1) axisMode=XMODE;
	if (src==l2) axisMode=YMODE;
	if (src==l3) axisMode=ZMODE;
	if (src==l4) axisMode=FACETUMODE;
	if (src==l5) axisMode=FACETVMODE;
	if (src==l6) axisMode=FACETNMODE;
	if (src==l7) axisMode=TWOVERTEXMODE;
	if (src==l8) axisMode=EQMODE;

}