/*
  File:        TestCube.h
  Description: Test cube creator and plotter
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLCombo.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _TESTCUBEH_
#define _TESTCUBEH_

class TestCube : public GLWindow {

public:

  // Construction
  TestCube(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
  Worker	   *work;

  GLButton    *buildButton;
  GLButton    *deleteButton;
  
  GLTextField *fromStr,*toStr;
  GLTextField *posX,*posY,*posZ;
  GLTextField *size;

  GLCombo structureId;

  GLTextField* value1,value2;

  int nbFacetS;

};

#endif /* _TESTCUBEH_ */
