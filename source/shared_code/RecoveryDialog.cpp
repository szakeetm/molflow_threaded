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
#include "RecoveryDialog.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLIcon.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/MathTools.h" // MAX

// Construct a message dialog box
RecoveryDialog::RecoveryDialog(const char *message, const char *title, int mode, int icon) :GLWindow() {

  int xD,yD,wD,hD,iW,txtWidth,txtHeight;
  int nbButton=4;

  if(title) SetTitle(title);
  else      SetTitle("Autosave file recovery");

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
  hD = 50+ Max(txtHeight,iW);
  int startX = (wD-totalBW)/2+20;
  int startY = hD - 45;
    
	
		GLButton *btn = new GLButton(GLDLG_LOAD, "Load");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
	startX+=80;
	
		btn = new GLButton(GLDLG_SKIP, "Skip");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
	startX+=120;
	
		btn = new GLButton(GLDLG_DELETE, "Delete");
    btn->SetBounds(startX,startY,75,20);
	btn->SetFontColor(210, 0, 0);
    Add(btn);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( wD > wS ) wD = wS;
  xD = (wS-wD)/2;
  yD = (hS-hD)/2+100;
  SetBounds(xD,yD,wD,hD);

  // Create objects
  RestoreDeviceObjects();

  rCode = GLDLG_CANCEL_R;

}

void RecoveryDialog::ProcessMessage(GLComponent *src, int message) {
  if(message==MSG_BUTTON) {
    rCode = src->GetId();
    GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    return;
  }
  GLWindow::ProcessMessage(src,message);
}

int RecoveryDialog::Display(const char *message, const char *title, int mode, int icon) {

  GLfloat old_mView[16];
  GLfloat old_mProj[16];
  GLint   old_viewport[4];

  // Save current matrix
  glGetFloatv( GL_PROJECTION_MATRIX , old_mProj );
  glGetFloatv( GL_MODELVIEW_MATRIX , old_mView );
  glGetIntegerv( GL_VIEWPORT , old_viewport );

  // Initialise
  RecoveryDialog *dlg = new RecoveryDialog(message, title, mode, icon);
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
