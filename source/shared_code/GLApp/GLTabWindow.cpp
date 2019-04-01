// Copyright (c) 2011 rubicon IT GmbH
#include "GLTabWindow.h"
#include "GLToolkit.h"
#include "GLComponent.h"
#include "GLFont.h"
#include <cstring>

// Tab bar component

#define TABHEIGHT 18

class TabbedBar : GLComponent {

public:

  TabbedBar(APANEL *panels,int nbPanel):GLComponent(0) {
    this->panels = panels;
    this->nbPanel = nbPanel;
    SetBorder(BORDER_NONE);
    selected = 0;
    nbLine = 1;
    SetTextColor(0,0,0);
    if(nbPanel>0) panels[0].selected = true;
  }

  int GetSelected() {
    return selected;
  }

  void SetTextColor(int r,int g,int b) {
    rText = r/255.0f;
    gText = g/255.0f;
    bText = b/255.0f;
  }

  void Paint() {

    if(!parent) return;
    GLComponent::Paint();
    int nb = 0;

    int sx = 0;
    int sy = (nbLine-1)*TABHEIGHT;
    for(int i=0;i<nbPanel;i++) {
      if(sx+panels[i].width>width) {
        sx  = 0;
        nb++;
        sy -= TABHEIGHT;
      }
      drawTab(i,posX+sx,posY+sy,nb);
      sx += panels[i].width;
      if(i==nbPanel-1 || sx+panels[i+1].width>width) {
        // End the line
        glBegin(GL_LINES);
        _glVertex2i(posX+sx,posY+sy+TABHEIGHT);
        _glVertex2i(posX+width,posY+sy+TABHEIGHT);
        glEnd();
      }
    }

  }

  void ManageEvent(SDL_Event *evt) {

    if( evt->type == SDL_MOUSEBUTTONDOWN ) {

      int ex = evt->button.x - posX;
      int ey = evt->button.y - posY;
      int sx = 0;
      int sy = (nbLine-1)*TABHEIGHT;
      bool found = false;
      int s=0;

      while(s<nbPanel && !found) {
        if(sx+panels[s].width>width) {
          sx  = 0;
          sy -= TABHEIGHT;
        }
        found = ( ex>=sx && ex<=sx+panels[s].width && ey>=sy && ey<=sy+TABHEIGHT );
        sx += panels[s].width;
        if(!found) s++;
      }

      if(found) {
        for(int i=0;i<nbPanel;i++) panels[i].selected = (s==i);
        selected = s;
        parent->ProcessMessage(this,MSG_TAB);
      }

    }

  }

  void Measure(int x,int y,int w,int h) {
    int sx = 0;
    nbLine = 1;
    for(int i=0;i<nbPanel;i++) {
      if(sx+panels[i].width>w) {
        sx  = 0;
        nbLine++;
      }
      sx += panels[i].width;
    }
    SetBounds(x,y,w,nbLine*TABHEIGHT);
  }

private:

  void drawTab(int idx,int x,int y,int curLine) {

    int w = panels[idx].width;
    int h = TABHEIGHT;

    GLFont2D *font;
    if(!panels[idx].selected)
      font = GLToolkit::GetDialogFont();
    else
      font = GLToolkit::GetDialogFontBold();

    int sx = x+(w-font->GetTextWidth(panels[idx].name))/2;
    font->SetTextColor(rText,gText,bText);
    font->DrawText(sx,y+3,panels[idx].name);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);

    glColor3f(0.5f,0.5f,0.5f);
    glBegin(GL_LINE_STRIP);
    glVertex2i(x+0,y+h);
    glVertex2i(x+0,y+5);
    glVertex2i(x+4,y+2);
    glVertex2i(x+8,y+0);
    glVertex2i(x+w-4,y+0);
    glVertex2i(x+w-1,y+2);
    glVertex2i(x+w-1,y+h);
    if(!panels[idx].selected || curLine>0) glVertex2i(x+0,y+h);
    glEnd();
  
  }

  APANEL *panels;
  int nbPanel;
  int nbLine;
  int selected;
  float rText;
  float gText;
  float bText;

};

GLTabWindow::GLTabWindow():GLWindow() {
  nbPanel = 0;
  panels = NULL;
  bar = NULL;
}

GLTabWindow::~GLTabWindow() {
  Clear();
}

void GLTabWindow::Add(int panel,GLComponent *comp) {

  GLWindow::Add(comp);

  APANEL *p = panels + panel;

  if( (p->nbComp % 32)==0 ) {
    GLComponent **news = (GLComponent **)malloc((p->nbComp+32) * sizeof(GLComponent *));
    if(p->nbComp) memcpy(news,p->comp,p->nbComp*sizeof(GLComponent *));
    free(p->comp);
    p->comp = news;
  }
  p->comp[ p->nbComp ] = comp;
  p->nbComp++;

}

void GLTabWindow::SetPanelNumber(int numP) {

  Clear();
  panels = (APANEL *)malloc(numP * sizeof(APANEL));
  memset(panels,0,numP*sizeof(APANEL));
  nbPanel = numP;

}

void GLTabWindow::SetPanelName(int idx,const char *name) {

  panels[idx].name = strdup(name);
  panels[idx].width = GLToolkit::GetDialogFontBold()->GetTextWidth(name)+10;

}

void GLTabWindow::Clear() {

  if( nbPanel ) {
    for(int i=0;i<nbPanel;i++) {
      SAFE_FREE(panels[i].name);
      SAFE_FREE(panels[i].comp);
    }
    SAFE_FREE(panels);
  }
  nbPanel = 0;
  SAFE_DELETE(bar);
  bar = NULL;
  SetMenuBar((GLComponent *)NULL);

}

void GLTabWindow::UpdateBar() {

  if(bar) SAFE_DELETE(bar);
  bar = new TabbedBar(panels,nbPanel);
  SetMenuBar((GLComponent *)bar);
  showHide();

}

void GLTabWindow::SetBounds(int x,int y,int w,int h) {

  if(bar) bar->Measure(x+2,y+20,w-4,h);
  GLWindow::SetBounds(x,y,w,h);

}

void GLTabWindow::ProcessMessage(GLComponent *src,int message) {

  if( message==MSG_TAB ) showHide();
  GLWindow::ProcessMessage(src,message);

}

void GLTabWindow::showHide() {

  int s = bar->GetSelected();
  // Show selected panel
  for(int i=0;i<nbPanel;i++) {
    APANEL *p = panels + i;
    for(int j=0;j<p->nbComp;j++)
      p->comp[j]->SetVisible(i==s);
  }

}

void GLTabWindow::SetTextColor(int r,int g,int b) {
  if(bar) bar->SetTextColor(r,g,b);
}

int GLTabWindow::GetSelectedTabIndex()
{
	return bar->GetSelected();
}
