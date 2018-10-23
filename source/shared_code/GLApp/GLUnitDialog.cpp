// Copyright (c) 2011 rubicon IT GmbH
#include "GLUnitDialog.h"
#include "GLButton.h"
#include "GLLabel.h"
#include "GLIcon.h"
#include "GLToolkit.h"
#include "MathTools.h" //Min max

// Construct a message dialog box
GLUnitDialog::GLUnitDialog(const char *message, const char *title,int mode,int icon):GLWindow() {

  int xD,yD,wD,hD,iW,txtWidth,txtHeight;
  int nbButton=5;

  if(title) SetTitle(title);
  else      SetTitle("Unit choice");

  // Label
  GLLabel  *label = new GLLabel(message);
  label->GetTextBounds(&txtWidth,&txtHeight);
  iW = (icon==GLDLG_ICONNONE)?0:64;
  label->SetBounds(iW+3,3,txtWidth,txtHeight);
  Add(label);

  // Icon
  GLIcon   *gIcon = NULL;
  switch(icon) {
    case GLDLG_ICONERROR:
      gIcon = new GLIcon("images/icon_error.png");
      break;
    case GLDLG_ICONWARNING:
      gIcon = new GLIcon("images/icon_warning.png");
      break;
    case GLDLG_ICONINFO:
      gIcon = new GLIcon("images/icon_info.png");
      break;
  }
  if(gIcon) {
    gIcon->SetBounds(3,0,64,64);
    Add(gIcon);
  }

  // Buttons
  int totalBW = nbButton * 80;
  wD = Max(txtWidth + iW + 30,totalBW);
  hD = 75+ Max(txtHeight,iW);
  int startX = (wD-totalBW)/2;
  int startY = hD - 70;
    
	if (true){
    GLButton *btn = new GLButton(GLDLG_MM,"Millimeters");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
	startX+=80;}
	if (true){
    GLButton *btn = new GLButton(GLDLG_CM,"Centimeters");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
	startX+=80;}
	if (true){
	GLButton *btn = new GLButton(GLDLG_M,"Meters");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
	startX+=80;}
	if (true){
	GLButton *btn = new GLButton(GLDLG_INCH,"Inches");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
	startX+=80;}
	if (true){
	GLButton *btn = new GLButton(GLDLG_FOOT,"Feet");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
	startX+=80;}
    if (true){
    GLButton *btn = new GLButton(GLDLG_CANCEL_U,"Cancel");
    btn->SetBounds(startX-240,startY+25,75,20);
	Add(btn);}

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( wD > wS ) wD = wS;
  xD = (wS-wD)/2;
  yD = (hS-hD)/2+100;
  SetBounds(xD,yD,wD,hD);

  // Create objects
  RestoreDeviceObjects();

  rCode = GLDLG_CANCEL_U;

}

void GLUnitDialog::ProcessMessage(GLComponent *src,int message) {
  if(message==MSG_BUTTON) {
    rCode = src->GetId();
    GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    return;
  }
  GLWindow::ProcessMessage(src,message);
}

int GLUnitDialog::Display(const char *message, const char *title,int mode,int icon) {

  GLfloat old_mView[16];
  GLfloat old_mProj[16];
  GLint   old_viewport[4];

  // Save current matrix
  glGetFloatv( GL_PROJECTION_MATRIX , old_mProj );
  glGetFloatv( GL_MODELVIEW_MATRIX , old_mView );
  glGetIntegerv( GL_VIEWPORT , old_viewport );

  // Initialise
  GLUnitDialog *dlg = new GLUnitDialog(message,title,mode,icon);
  dlg->DoModal();
  int ret = dlg->rCode;
  delete dlg;

  // Restore matrix
  glMatrixMode( GL_PROJECTION );
  glLoadMatrixf(old_mProj);
  glMatrixMode( GL_MODELVIEW );
  glLoadMatrixf(old_mView);
  glViewport(old_viewport[0],old_viewport[1],old_viewport[2],old_viewport[3]);

  return ret;

}
