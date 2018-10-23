// Copyright (c) 2011 rubicon IT GmbH
#include "GLMessageBox.h"
#include "GLButton.h"
#include "GLLabel.h"
#include "GLIcon.h"
#include "GLToolkit.h"
#include "MathTools.h" //Min max

// Construct a message dialog box
GLMessageBox::GLMessageBox(const std::string & message, const std::string & title, const std::vector<std::string> & buttonList, int icon) :GLWindow() {

	int xD, yD, wD, hD, iconWidth, txtWidth, txtHeight;
	int nbButton = 0;

	if (title.length()) SetTitle(title);
	else      SetTitle("Message");

	// Label
	GLLabel  *label = new GLLabel(message.c_str());
	label->GetTextBounds(&txtWidth, &txtHeight);
	iconWidth = (icon == GLDLG_ICONNONE) ? 0 : 64;
	label->SetBounds(iconWidth + 3, 3, txtWidth, txtHeight);
	Add(label);

	// Icon
	GLIcon   *gIcon = NULL;
	switch (icon) {
	case GLDLG_ICONERROR:
		gIcon = new GLIcon("images/icon_error.png");
		break;
	case GLDLG_ICONWARNING:
		gIcon = new GLIcon("images/icon_warning.png");
		break;
	case GLDLG_ICONINFO:
		gIcon = new GLIcon("images/icon_info.png");
		break;
	case GLDGL_ICONDEAD:
		gIcon = new GLIcon("images/icon_dead.png");
		break;
	}
	if (gIcon) {
		gIcon->SetBounds(3, 0, 64, 64);
		Add(gIcon);
	}

	// Buttons
	int totalBW = (int)buttonList.size() * 80;
	wD = Max(txtWidth + iconWidth + 30, totalBW);
	hD = 50 + Max(txtHeight, iconWidth);
	int startX = (wD - totalBW) / 2;
	int startY = hD - 45;

	int compId = 0;
	for (std::string buttonText : buttonList) {
		GLButton *btn = new GLButton(compId++, buttonText.c_str());
		btn->SetBounds(startX, startY, 75, 20);
		Add(btn);
		startX += 80;
	}

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	if (wD > wS) wD = wS;
	xD = (wS - wD) / 2;
	yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	// Create objects
	RestoreDeviceObjects();

	rCode = GLDLG_CANCEL;

}

/*
// Construct a message dialog box
GLMessageBox::GLMessageBox(const char *message,char *title,int mode,int icon):GLWindow() {

  int xD,yD,wD,hD,iconWidth,txtWidth,txtHeight;
  int nbButton=0;

  if(title) SetTitle(title);
  else      SetTitle("Message");

  // Label
  GLLabel  *label = new GLLabel(message);
  label->GetTextBounds(&txtWidth,&txtHeight);
  iconWidth = (icon==GLDLG_ICONNONE)?0:64;
  label->SetBounds(iconWidth+3,3,txtWidth,txtHeight);
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
	case GLDGL_ICONDEAD:
      gIcon = new GLIcon("images/icon_dead.png");
      break;
  }
  if(gIcon) {
    gIcon->SetBounds(3,0,64,64);
    Add(gIcon);
  }

  // Buttons
  if( mode & GLDLG_OK     ) nbButton++;
  if( mode & GLDLG_CANCEL ) nbButton++;
  int totalBW = nbButton * 80;
  wD = Max(txtWidth + iconWidth + 30,totalBW);
  hD = 50 + Max(txtHeight,iconWidth);
  int startX = (wD-totalBW)/2;
  int startY = hD - 45;

  if( mode & GLDLG_OK     ) { 
    GLButton *btn = new GLButton(GLDLG_OK,"OK");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
    startX+=80;
  }
    
  if( mode & GLDLG_CANCEL ) { 
    GLButton *btn = new GLButton(GLDLG_CANCEL,"Cancel");
    btn->SetBounds(startX,startY,75,20);
    Add(btn);
    startX+=80;
  }

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( wD > wS ) wD = wS;
  xD = (wS-wD)/2;
  yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  // Create objects
  RestoreDeviceObjects();

  rCode = GLDLG_CANCEL;

}

*/

void GLMessageBox::ProcessMessage(GLComponent *src,int message) {
  if(message==MSG_BUTTON) {
    rCode = src->GetId();
    GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    return;
  }
  GLWindow::ProcessMessage(src,message);
}

int GLMessageBox::Display(const char *message, const char *title,int mode,int icon) {
	std::vector<std::string> list;
	if (mode & GLDLG_OK) {
		list.push_back("OK");
	}
	if (mode & GLDLG_CANCEL) {
		list.push_back("Cancel");
	}
	std::string titleStr;
	if (title == NULL) titleStr = "";
	else titleStr = title;
	int retCode = Display(message, titleStr, list, icon);
	if (retCode == 0) return GLDLG_OK;
	else return GLDLG_CANCEL;
}

int GLMessageBox::Display(const std::string & message, const std::string & title, const std::vector<std::string>& buttonList, int icon) {
	GLfloat old_mView[16];
	GLfloat old_mProj[16];
	GLint   old_viewport[4];

	// Save current matrix
	glGetFloatv(GL_PROJECTION_MATRIX, old_mProj);
	glGetFloatv(GL_MODELVIEW_MATRIX, old_mView);
	glGetIntegerv(GL_VIEWPORT, old_viewport);

	// Initialise
	GLMessageBox *dlg = new GLMessageBox(message, title, buttonList, icon);
	dlg->DoModal();
	int ret = dlg->rCode;
	delete dlg;

	// Restore matrix
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(old_mProj);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(old_mView);
	glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);

	return ret;
}
