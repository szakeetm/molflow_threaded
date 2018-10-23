// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLMenuBar.h"
#include "GLToolkit.h"
#include "GLWindowManager.h"
#include "MathTools.h" //Saturate
#include "GLApp.h"
#include <cstring> //strcpy, etc.
#include <thread> //sleep

extern GLApplication* theApp;

//#include <malloc.h>

#define SIDE_MARGIN 24

GLMenu::GLMenu():GLComponent(0) {
  selMenu = -1;
  nbItem = 0;
  items = NULL;
  pBar = NULL;
  pMenu = NULL;
  rCode = -1;
  hasAcc = false;

  SetBorder(BORDER_BEVEL_OUT);
  SetVisible(false);
  SetFocusable(false);
  SetBackgroundColor(225,220,220);
}

GLMenu::~GLMenu() {
  for(int i=0;i<nbItem;i++) {
    if(!items[i].subMenu->GetParent())
      SAFE_DELETE(items[i].subMenu);
    SAFE_FREE(items[i].accName);
  }
  SAFE_FREE(items);
}

int GLMenu::GetMenu(int mx,int my) {

  bool found = false;
  int i = 0;
  while(!found && i<nbItem) {
    if(!items[i].isSeparator) found = IsInItem(items+i,mx,my);
    if(!found) i++;
  }
  if(found) return i;
  else      return -1;

}

void GLMenu::SetParentMenuBar(GLMenuBar *p) {
  pBar = p;
}

void GLMenu::SetParentMenu(GLMenu *p) {
  pMenu = p;
}

int GLMenu::GetNbItem() {
  return nbItem;
}

void GLMenu::SetCheck(int itemId,bool checked) {

  int i = GetMenu(itemId);
  if( i>=0 ) items[i].checked = checked;

}

void GLMenu::SetIcon(int itemId,int x,int y) {

  int i = GetMenu(itemId);
  if( i>=0 ) {
    items[i].iconX = x;
    items[i].iconY = y;
  }

}

bool GLMenu::GetCheck(int itemId) {

  int i = GetMenu(itemId);
  if( i>=0 )  return items[i].checked;
  else        return false;

}

void  GLMenu::SetEnabled(int itemId,bool enabled) {

  int i = GetMenu(itemId);
  if( i>=0 ) items[i].enabled = enabled;

}

int GLMenu::GetMenu(int id) {

  bool found = false;
  int i=0;
  while(!found && i<nbItem) {
    found = (items[i].itemId == id);
    if(!found) i++;
  }
  if( found ) return i;
  else        return -1;

}

bool GLMenu::IsInItem(MENUITEM *p,int mx,int my) {
    return mx>(p->x) && mx<=(p->x)+width &&
           my>(p->y) && my<=(p->y)+(p->height);
}

void GLMenu::Clear() {

  for(int i=0;i<nbItem;i++) {
    if(!parent) {
      SAFE_DELETE(items[i].subMenu);
    } else {
      parent->PostDelete(items[i].subMenu);
    }
  }
  int size = sizeof(MENUITEM)*MAX_MENU_ITEM;
  if(items) memset(items,0,size);
  nbItem = 0;
  hasAcc = false;

}

GLMenu* GLMenu::Add(const char *itemName,int itemId,int accKeyCode,int accKeyModifier) {

  if(nbItem>=MAX_MENU_ITEM) return NULL;

  if(!items) {
    // Allocate
    int size = sizeof(MENUITEM)*MAX_MENU_ITEM;
    items = (MENUITEM *)malloc(size);
    memset(items,0,size);
  }

  int sx = 3;
  int sy = 3;
  int i;
  for(i=0;i<nbItem;i++) sy+=items[i].height;

  // Accelerators
  if( accKeyCode ) {

    GLWindowManager::RegisterKeyboardShortcut(this,accKeyCode,accKeyModifier,nbItem);
    hasAcc = true;
    items[i].accName = strdup( GLWindowManager::GetAccStr(accKeyCode,accKeyModifier) );
    items[i].accWidth = GLToolkit::GetDialogFont()->GetTextWidth(items[i].accName);

  }

  items[i].itemId = itemId;
  if(itemName) {
    strncpy(items[i].itemName,itemName,MAX_ITEM_LGTH);
    items[i].itemName[MAX_ITEM_LGTH-1]=0;
    GLWindowManager::RemoveAccFromStr(items[i].itemName,&(items[i].shortcut),&(items[i].sctPos),&(items[i].sctWidth));
    items[i].width  = 2*SIDE_MARGIN + GLToolkit::GetDialogFont()->GetTextWidth(items[i].itemName) + items[i].accWidth;
    items[i].height = 17;
    items[i].subMenu = new GLMenu();
  } else {
    items[i].height = 5;
    items[i].isSeparator = true;
  }
  items[i].x = sx;
  items[i].y = sy;
  items[i].enabled = true;

  nbItem++;
  return items[i].subMenu;
}

GLMenu *GLMenu::GetSubMenu(const char *itemName) {

  char tmpName[256];
  strcpy(tmpName,itemName);
  GLWindowManager::RemoveAccFromStr(tmpName);

  bool found = false;
  int i = 0;
  while(!found && i<nbItem) {
    if(!items[i].isSeparator) found = (strcmp(tmpName,items[i].itemName)==0);
    if(!found) i++;
  }
  if(found) {
    items[i].subMenu->SetParentMenuBar(pBar);
    items[i].subMenu->SetParentMenu(pMenu);
    return items[i].subMenu;
  } else
    return NULL;

}

bool GLMenu::HasSub(int s) {
  if(s>=0 && s<nbItem) {
    GLMenu *sub = items[s].subMenu;
    if(sub) return (sub->GetNbItem()>0);
  }
  return false;
}

void GLMenu::ProcessMenuItem(int m) {

  // Menu item
  if( items[m].enabled ) {
    id = items[m].itemId;
    if( pBar )        pBar->Close();
    else if ( pMenu ) pMenu->Close();
    else              Close();
    if( !pMenu ) {
      if( !parent ) {
        GLWindowManager::GetTopLevelWindow()->ProcessMessage(this,MSG_MENU);
      } else {
        parent->ProcessMessage(this,MSG_MENU);
      }
    } else
      pMenu->rCode = id;
  }

}

void GLMenu::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int mx = GetWindow()->GetX(this,evt);
  int my = GetWindow()->GetY(this,evt);

  if(evt->type == SDL_MOUSEMOTION) {
    int newMenu = GetMenu(mx,my);
    if( newMenu>=0 && newMenu!=selMenu ) {
      CloseSub();
      selMenu = newMenu;
      if( HasSub(selMenu) ) DropSub(selMenu);
    }
  }

  if(evt->type == SDL_MOUSEBUTTONUP)
  if(evt->button.button == SDL_BUTTON_LEFT) {
    int newMenu = GetMenu(mx,my);
    if( newMenu>=0 ) {
      selMenu = newMenu;
      if( HasSub(selMenu) ) {
        // Drop sub menu
        DropSub(selMenu);
      } else {
        ProcessMenuItem(selMenu);
      }
    }
  }

  // Active event (Mouse entering-leaving event)
  if (evt->type == SDL_WINDOWEVENT && (evt->window.event == SDL_WINDOWEVENT_ENTER)) {
      if( selMenu>=0 ) {
        if( !items[selMenu].subMenu )
          selMenu = -1;
        else
          if( !items[selMenu].subMenu->IsVisible() )
            selMenu = -1;
      }
  }

  if( nbItem==0 ) return;

  GLMenu *sub = NULL;
  if( selMenu>=0 ) sub = items[selMenu].subMenu;

  if( evt->type == SDL_KEYDOWN ) {
    switch(evt->key.keysym.sym) {
      case SDLK_DOWN:
        if( !(sub && sub->IsVisible()) ) {
          selMenu++;
          if(selMenu>=nbItem) selMenu=0;
          CloseSub(false);
          return;
        }
        break;
      case SDLK_UP:
        if( !(sub && sub->IsVisible()) ) {
          selMenu--;
          if(selMenu<0) selMenu=nbItem-1;
          CloseSub(false);
          return;
        }
        break;
      case SDLK_RETURN:
        if( selMenu>=0 && !HasSub(selMenu) ) {
          ProcessMenuItem(selMenu);
          return;
        } else {
          if(HasSub(selMenu) && !sub->IsVisible()) {
            DropSub(selMenu);
            return;
          }
        }
        break;
      case SDLK_RIGHT:
        if(HasSub(selMenu) && !sub->IsVisible()) {
          DropSub(selMenu);
          sub->selMenu=0;
          return;
        }
        if(!HasSub(selMenu)) {
          if(pBar) pBar->GoRight();
          return;
        }
        break;
      case SDLK_LEFT:
        if(HasSub(selMenu) && sub->IsVisible() ) {
          CloseSub(false);
        } else {
          if(pBar) pBar->GoLeft();
        }
        return;
      default:
        if( ProcessShortcut(evt) ) return;
    }
  }

  // Replay event to sub
  if(sub) sub->ManageEvent(evt);

  if (IsVisible()) {
	  /*GLWindowManager::Repaint();
	  std::this_thread::sleep_for(std::chrono::milliseconds(30));*/
	  theApp->wereEvents = true;
  }
}

void GLMenu::ProcessKeyboardShortcut(int accId) {

  if(accId>=nbItem) return;

  if( !HasSub(accId) ) {
    ProcessMenuItem(accId);
  } else {
    if(HasSub(accId) && !items[accId].subMenu->IsVisible()) {
      DropSub(accId);
    }
  }

}

bool GLMenu::ProcessShortcut(SDL_Event *evt) {

  if(evt->key.keysym.sym ==0 ) return false;

  bool found = false;
  int i = 0;
  while(!found && i<nbItem) {
    found = (evt->key.keysym.sym ==items[i].shortcut);
    if(!found) i++;
  }

  if(found) ProcessKeyboardShortcut(i);

  return found;

}

void GLMenu::Close() {

  CloseSub();
  SetVisible(false);

}

void GLMenu::Paint() {

  if(!parent) return;
  GLComponent::Paint();
  GLToolkit::DrawHGradientBox(posX+1,posY+2,20,height-3);

  GLFont2D *font = GLToolkit::GetDialogFont();

  for(int i=0;i<nbItem;i++) {

    MENUITEM *p = items + i;

    if( p->isSeparator ) {

      // Etched colors
      float rL = (float)rBack / 100.0f;
      float gL = (float)gBack / 100.0f;
      float bL = (float)bBack / 100.0f;
      Saturate(rL,0.0f,1.0f);
      Saturate(gL,0.0f,1.0f);
      Saturate(bL,0.0f,1.0f);
      float rD = (float)rBack / 500.0f;
      float gD = (float)gBack / 500.0f;
      float bD = (float)bBack / 500.0f;
      Saturate(rD,0.0f,1.0f);
      Saturate(gD,0.0f,1.0f);
      Saturate(bD,0.0f,1.0f);

      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      glColor3f(rD,gD,bD);
      glBegin(GL_LINES);
      _glVertex2i(posX+p->x+SIDE_MARGIN-4,posY+p->y+3);
      _glVertex2i(posX+p->x+width-4,posY+p->y+3);
      glEnd();
      glColor3f(rL,gL,bL);
      glBegin(GL_LINES);
      _glVertex2i(posX+p->x+SIDE_MARGIN-4,posY+p->y+4);
      _glVertex2i(posX+p->x+width-4,posY+p->y+4);
      glEnd();

    } else {

      if( p->enabled ) {

        if( selMenu==i ) {
          GLToolkit::DrawBox(posX+p->x+SIDE_MARGIN-4,posY+p->y,width-2-SIDE_MARGIN,p->height+1,60,60,200);
          font->SetTextColor(1.0f,1.0f,1.0f);
        } else {
          font->SetTextColor(0.0f,0.0f,0.0f);
        }
        font->DrawText(posX+4+p->x+SIDE_MARGIN,posY+p->y+3,p->itemName,false);

        // Underline shortcut char
        if( p->shortcut ) {
          glDisable(GL_BLEND);
          glDisable(GL_TEXTURE_2D);
          glBegin(GL_LINES);
          _glVertex2i(SIDE_MARGIN+5+posX+p->x+p->sctPos,posY+p->y+15);
          _glVertex2i(SIDE_MARGIN+4+posX+p->x+p->sctPos+p->sctWidth,posY+p->y+15);
          glEnd();
        }

      } else {

        font->SetTextColor(1.0f,1.0f,1.0f);
        font->DrawText(posX+p->x+SIDE_MARGIN+5,posY+p->y+4,p->itemName,false);
        font->SetTextColor(0.4f,0.4f,0.4f);
        font->DrawText(posX+p->x+SIDE_MARGIN+4,posY+p->y+3,p->itemName,false);

      }

      GLMenu *sub = p->subMenu;
      if( sub && sub->GetNbItem() ) {
        font->SetTextColor(0.0f,0.0f,0.0f);
        font->DrawText(posX+p->x+width-15,posY+p->y+2,"\213",false);
      }

      if( !HasSub(i) && p->accName ) {
        font->SetTextColor(0.0f,0.0f,0.0f);
        font->DrawText(posX+p->x+width-items[i].accWidth-10,posY+p->y+3,p->accName,false);
      }

      if( p->checked ) {
        font->SetTextColor(0.0f,0.0f,0.0f);
        font->DrawText(posX+SIDE_MARGIN-1,posY+p->y+3,"\215",false);
      }

      if( p->iconX || p->iconY ) {
        GLToolkit::Draw16x16(posX+2,posY+p->y+1,p->iconX,p->iconY);
      }
    
    }

  }
  GLToolkit::CheckGLErrors("GLMenu::Paint()");
}

void GLMenu::DropSub(int s) {

  CloseSub();
  if( s>=0 ) {
    MENUITEM *p = items + s;
    p->subMenu->SetParentMenu(pMenu);
    p->subMenu->Drop(parent,posX+width+1,posY+p->y);
  }
  selMenu = s;

}

void GLMenu::CloseSub(bool resetSel) {

  for(int i=0;i<nbItem;i++) {
    GLMenu *sub = items[i].subMenu;
    if(sub) sub->Close();
  }
  if(resetSel) selMenu = -1;

}

int GLMenu::Track(GLWindow *parent,int x,int y) {

  if(!parent) parent = GLWindowManager::GetTopLevelWindow();

  // Measure menu
  int menuWidth  = 0;
  int menuHeight = 0;
  for(int i=0;i<nbItem;i++) { 
    MENUITEM *p = items + i;
    menuWidth=Max(menuWidth,p->width);
    menuHeight += p->height;
  }
  // margin
  menuHeight += 6;
  menuWidth  += 0;

  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( x+menuWidth  > wS    ) x -= menuWidth;
  if( y+menuHeight > hS-30 ) y -= menuHeight;
  SetBounds(x,y,menuWidth,menuHeight);
  parent->FreezeComp();
  parent->AddMenu(this);
  SetParentMenu(this);
  SetVisible(true);
  rCode = -1;

  // Modal Loop
  SDL_Event evt;
  while( IsVisible() )
  {

    //While there are events to handle
    while( SDL_PollEvent( &evt ) ) {
      parent->ManageMenu(&evt);
    }

    if(!parent->IsEventProcessed()) {
      if( evt.type==SDL_MOUSEBUTTONDOWN )
        // Click outside
        Close();
    }

    if(evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_EXPOSED)
      GLWindowManager::FullRepaint();

    // Close modal window window on [ESC]
    if( evt.type == SDL_KEYDOWN ) {
      if(evt.key.keysym.sym == SDLK_ESCAPE ) SetVisible(false);
    }

	if( IsVisible() ) {
      GLWindowManager::Repaint();
	  std::this_thread::sleep_for(std::chrono::milliseconds(30));
		//theApp->wereEvents = true;
    }

  }

  parent->UnfreezeComp();
  //GLWindowManager::FullRepaint();
  theApp->wereEvents = true;
  return rCode;

}

void GLMenu::Drop(GLContainer *parent,int x,int y) {
  
  if(!parent) parent = GLWindowManager::GetTopLevelWindow();

  // Measure menu
  int menuWidth  = 0;
  int menuHeight = 0;
  for(int i=0;i<nbItem;i++) { 
    MENUITEM *p = items + i;
    menuWidth=Max(menuWidth,p->width);
    menuHeight += p->height;
  }
  // margin
  menuHeight += 6;
  menuWidth  += 40;

  SetBounds(x,y,menuWidth,menuHeight);
  parent->GetWindow()->AddMenu(this);
  SetVisible(true);

}

