/*
  File:        Intersect3Planes.h
  Description: Define three planes and create a vertex at their intersection
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _Intersect3PlanesH_
#define _Intersect3PlanesH_

class Intersect3Planes : public GLWindow {

public:
  // Construction
  Intersect3Planes(Geometry *geom,Worker *work);
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *plane1Panel,*plane2Panel,*plane3Panel;
  GLButton     *moveButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLToggle     *plane1ModeFacet,*plane2ModeFacet,*plane3ModeFacet;
  GLToggle     *plane1ModeVertex,*plane2ModeVertex,*plane3ModeVertex;
  GLToggle     *plane1ModeEquation,*plane2ModeEquation,*plane3ModeEquation;
  GLButton     *plane1SetFacet,*plane2SetFacet,*plane3SetFacet;
  GLButton     *plane1SetVertices,*plane2SetVertices,*plane3SetVertices;
  GLToggle     *l5;
  GLToggle     *l6;
  GLLabel     *lNum;
  GLToggle     *l7;
  GLToggle     *l8;
  GLTextField *aText;
  GLTextField *bText;
  GLTextField *cText;
  GLTextField *uText;
  GLTextField *vText;
  GLTextField *wText;
  GLTextField *facetNumber;
  GLTextField *degText;
  GLLabel		*aLabel;
  GLLabel		*bLabel;
  GLLabel		*cLabel;
  GLLabel		*uLabel;
  GLLabel		*vLabel;
  GLLabel		*wLabel;
  GLLabel		*degLabel;

  int nbFacetS;

  Geometry     *geom;
  Worker	   *work;

};

#endif /* _Intersect3PlanesH_ */
