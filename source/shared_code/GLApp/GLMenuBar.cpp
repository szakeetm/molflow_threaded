// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLMenuBar.h"
#include "GLToolkit.h"
#include "GLWindowManager.h"
#include <cstring> //strcpy, etc.
//#include <malloc.h>

#define SIDE_MARGIN 7

GLMenuBar::GLMenuBar(int compId):GLComponent(compId) {
  SetBorder(BORDER_NONE);
  nbItem=0;
  selMenu=-1;
  autoDrop=false;
  memset(items,0,sizeof items);
}

GLMenuBar::~GLMenuBar() {
  for(int i=0;i<nbItem;i++) {
    if(!items[i].subMenu->GetParent())
      SAFE_DELETE(items[i].subMenu);
    SAFE_FREE(items[i].accName);
  }
}

void GLMenuBar::Add(const char *itemName) {

  if(nbItem>=MAX_MENU_ITEM) return;

  int sx = posX+3;
  int sy = posY;
  int i;
  for(i=0;i<nbItem;i++) sx+=items[i].width;

  strncpy(items[i].itemName,itemName,32);
  items[i].itemName[31]=0;
  GLWindowManager::RemoveAccFromStr(items[i].itemName,&(items[i].shortcut),&(items[i].sctPos),&(items[i].sctWidth));
  // Handle key shortcut as accelerator in menubar
  GLWindowManager::RegisterKeyboardShortcut(this,items[i].shortcut,ALT_MODIFIER,i);
  items[i].width  = 2*SIDE_MARGIN + GLToolkit::GetDialogFont()->GetTextWidth(items[i].itemName);
  items[i].height = 17;
  items[i].subMenu = new GLMenu();
  items[i].x = sx;
  items[i].y = sy;

  nbItem++;

}

int GLMenuBar::GetMenu(int mx,int my) {

  bool found = false;
  int i = 0;
  while(!found && i<nbItem) {
    found = IsInItem(items+i,mx,my);
    if(!found) i++;
  }
  if(found) return i;
  else      return -1;

}

bool GLMenuBar::IsInItem(MENUITEM *p,int mx,int my) {
    return mx>=(p->x) && mx<=(p->x)+(p->width) &&
           my>=(p->y) && my<=(p->y)+(p->height);
}

GLMenu *GLMenuBar::GetSubMenu(const char *itemName) {

  char tmpName[256];
  strcpy(tmpName,itemName);
  GLWindowManager::RemoveAccFromStr(tmpName);

  bool found = false;
  int i = 0;
  while(!found && i<nbItem) {
    found = (strcmp(tmpName,items[i].itemName)==0);
    if(!found) i++;
  }
  if(found) {
    items[i].subMenu->SetParentMenuBar(this);
    return items[i].subMenu;
  } else
    return NULL;

}

void GLMenuBar::SetFocus(bool focus) {
  if( !focus ) Close();
  GLComponent::SetFocus(focus);
}

void GLMenuBar::Close() {

  for(int i=0;i<nbItem;i++) {
    GLMenu *sub = items[i].subMenu;
    if(sub) sub->Close();
  }
  autoDrop=false;
  selMenu=-1;

}

void GLMenuBar::Paint() {

  if(!parent) return;
  GLComponent::Paint();

  GLToolkit::GetDialogFont()->SetTextColor(0.0f,0.0f,0.0f);
  for(int i=0;i<nbItem;i++) {
    MENUITEM *p = items + i;
    if(autoDrop) {
      if( selMenu==i )
        GLToolkit::DrawBox(p->x,p->y+1,p->width,p->height,rBack,gBack,bBack,true,true);
    } else {
      if( selMenu==i )
        GLToolkit::DrawBox(p->x,p->y+1,p->width,p->height,rBack,gBack,bBack,true,false);
    }
    GLToolkit::GetDialogFont()->DrawText(p->x+SIDE_MARGIN,p->y+3,p->itemName,false);

    // Shortcut underline
    if( p->shortcut ) {
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      glBegin(GL_LINES);
      _glVertex2i(SIDE_MARGIN+1+p->x+p->sctPos          ,p->y+15);
      _glVertex2i(SIDE_MARGIN+p->x+p->sctPos+p->sctWidth,p->y+15);
      glEnd();
    }

  }
  GLToolkit::CheckGLErrors("GLMenuBar::Paint()");
}

void GLMenuBar::Drop(int sel) {
  Close();
  if( sel>=0 ) {
    MENUITEM *p = items + sel;
    p->subMenu->Drop(parent,p->x,1);
  }
  selMenu = sel;
  autoDrop = true;
}

void  GLMenuBar::GoLeft() {

  selMenu--;
  if(selMenu<0) selMenu=nbItem-1;
  Drop(selMenu);

}

void  GLMenuBar::GoRight() {

  selMenu++;
  if(selMenu>=nbItem) selMenu=0;
  Drop(selMenu); 

}

void GLMenuBar::ProcessKeyboardShortcut(int accId) {
  Drop(accId);
}

void GLMenuBar::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int mx = GetWindow()->GetX(this,evt);
  int my = GetWindow()->GetY(this,evt)+height;
  int s;

  if(evt->type == SDL_MOUSEMOTION) {
    if( autoDrop ) {
      s = GetMenu(mx,my);
      if(s>=0) Drop(s); 
    } else
      selMenu = GetMenu(mx,my);
  }

  if(evt->type == SDL_MOUSEBUTTONDOWN)
  if( evt->button.button == SDL_BUTTON_LEFT ) {
    s = GetMenu(mx,my);
    if( (s>=0) && autoDrop && (selMenu==s) ) { 
      Close();
      selMenu = s;
    } else {
      Drop(s);
    }
  }

  // Active event (Mouse entering-leaving event)
  if (evt->type == SDL_WINDOWEVENT && (evt->window.event == SDL_WINDOWEVENT_ENTER)) {
	  if( !autoDrop ) selMenu = -1;
    }
  

  // Key press
  if( evt->type == SDL_KEYDOWN ) {
    switch(evt->key.keysym.sym) {
      case SDLK_LEFT:
        if( !autoDrop ) {
          selMenu--;
          if(selMenu<0) selMenu=nbItem-1;
          return;
        }
        break;
      case SDLK_RIGHT:
        if( !autoDrop ) { 
          selMenu++;
          if(selMenu>=nbItem) selMenu=0;
          return;
        }
        break;
      case SDLK_RETURN:
       if( (selMenu>=0) && !autoDrop ) { 
         Drop(selMenu);
         return;
       }
       break;
    }
  }

  // Replay event to sub
  if( selMenu>=0 ) {
    GLMenu *sub = items[selMenu].subMenu;
    if(sub) sub->ManageEvent(evt);
  }

}
