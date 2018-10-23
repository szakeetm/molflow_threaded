// Copyright (c) 2011 rubicon IT GmbH
#include "GLContainer.h"
#include "GLWindow.h"
#include "GLComponent.h"
#include "GLToolkit.h"
//#include "GLWindowManager.h"
#include "GLApp.h"
//#include <malloc.h>

GLContainer::GLContainer() {

  list = NULL;
  lastFocus = NULL;
  draggedComp = NULL;
  lastClick = 0;
  evtProcessed = false;
  evtCanceled = false;
  parentWin = NULL;
  redirect = NULL;

}

GLContainer::~GLContainer() {
  Clear();
}

void GLContainer::Clear() {

  COMPLINK *node = list;
  COMPLINK *lnode;

  while(node!=NULL) {
    node->comp->InvalidateDeviceObjects();
	node->comp->DestroyComponents();
    SAFE_DELETE(node->comp);
    lnode = node;
    node = node->next;
    SAFE_FREE(lnode);
  }
  list = NULL;

}

void GLContainer::SetWindow(GLWindow *parent) {

  parentWin = parent;
  // Relay to sub
  COMPLINK *node = list;
  while(node!=NULL) {
    node->comp->SetWindow(parent);
    node = node->next;
  }

}

GLWindow *GLContainer::GetWindow() {
  return parentWin;
}

bool GLContainer::IsEventProcessed() {
  return evtProcessed;
}

bool GLContainer::IsEventCanceled() {
  return evtCanceled;
}

bool GLContainer::IsDragging() {
  return (draggedComp!=NULL);
}

void GLContainer::RedirectMessage(GLContainer *cont) {
  redirect = cont;
}

GLContainer *GLContainer::GetRedirect() {
  return redirect;
}

void GLContainer::CancelDrag(SDL_Event *evt) {

  draggedComp=NULL;

}

void GLContainer::RestoreDeviceObjects() {

  COMPLINK *node = list;
  while(node!=NULL) {
    node->comp->RestoreDeviceObjects();
    node = node->next;
  }

}

void GLContainer::InvalidateDeviceObjects() {

  COMPLINK *node = list;
  while(node!=NULL) {
    node->comp->InvalidateDeviceObjects();
    node = node->next;
  }

}

void GLContainer::SetFocus(GLComponent *src) {
  if( lastFocus ) lastFocus->SetFocus(false);
  src->SetFocus(true);
  lastFocus = src;
}

void GLContainer::PostDelete(GLComponent *comp) {

  // Mark for post deletion
  COMPLINK *node = list;
  bool found = false;
  while(!found && node) {
    found = (node->comp == comp);
    if( !found ) node=node->next;
  }

  if( found ) node->postDelete = true;

}

void GLContainer::Remove(GLComponent *comp) {

  COMPLINK *node = list;
  COMPLINK *prevNode = NULL;

  bool found = false;
  while(!found && node) {
    found = (node->comp == comp);
    if( !found ) {
      prevNode = node;
      node=node->next;
    }
  }

  if( found ) {
    if( prevNode ) {
      prevNode->next = node->next;
    } else {
      list = list->next;
    }
    node->comp->SetParent(NULL);
    SAFE_FREE(node);
  }

}

void GLContainer::Add(GLComponent *comp) {

  COMPLINK *node = list;
  GLContainer *cParent = comp->GetParent();

  // Already added
  bool found = false;
  while(!found && node) {
    found = (node->comp == comp);
    if( !found ) node=node->next;
  }
  if( found ) {
    node->canProcess = true;
    return;
  }

  // Remove from other window
  if ( cParent ) cParent->Remove(comp);

  node = list;
  if( !node ) {
    // Create head
    node = (COMPLINK *)malloc(sizeof(COMPLINK));
    node->next = list;
    node->comp = comp;
    node->canProcess = true;
    node->postDelete = false;
    node->comp->SetParent(this);
    list = node;
  } else {
    // Add at the end
    while(node->next) node=node->next; 
    node->next = (COMPLINK *)malloc(sizeof(COMPLINK));
    node->next->next = NULL;
    node->next->comp = comp;
    node->next->postDelete = false;
    node->next->canProcess = true;
    node->next->comp->SetParent(this);
  }

}

void GLContainer::SetCompBoundsRelativeTo(GLComponent * org, GLComponent * src, int dx, int dy, int w, int h) {
	int xc, yc, wc, hc;
	org->GetBounds(&xc, &yc, &wc, &hc);
	src->SetBounds(xc + dx, yc + dy, w, h);
}

void GLContainer::FreezeComp() {

  COMPLINK *node = list;
  while(node) {
    node->canProcess = false;
    node=node->next;
  }

}

void GLContainer::UnfreezeComp() {

  COMPLINK *node = list;
  while(node) {
    node->canProcess = true;
    node=node->next;
  }

}

void GLContainer::ManageEvent(SDL_Event *evt) {

  evtProcessed = false;
  evtCanceled = false;

  // Cancel dragging
  if( evt->type == SDL_MOUSEBUTTONUP ) {
    if(draggedComp) {
      ManageComp(draggedComp,evt);
      DoPostDelete();
    }
    CancelDrag(evt);
  }

}

GLComponent* GLContainer::GetFirstChildComp() {
	return (list->next->comp);
}

void GLContainer::RelayEvent(SDL_Event *evt) {

  if( parentWin ) {
    RelayEventReverse(list,evt);
    DoPostDelete();
  }

}

void GLContainer::RelayEventReverse(COMPLINK *lst,SDL_Event *evt) {
  if( lst ) {
    RelayEventReverse(lst->next,evt);
    if(!lst->postDelete && lst->canProcess) RelayEvent(lst->comp,evt);
  }
}

void GLContainer::DoPostDelete() {

  COMPLINK *node = list;
  COMPLINK *prevNode = NULL;
  COMPLINK *toFree = NULL;

  while(node) {
    if (node->postDelete) {
      if( prevNode==NULL )
        // Remove head
        list=list->next;
      else
        prevNode->next = node->next;
      node->comp->InvalidateDeviceObjects();
      if(node->comp==lastFocus) lastFocus = NULL;
      SAFE_DELETE(node->comp);
      toFree=node;
    } else {
      prevNode = node;
    }
    node=node->next;
    SAFE_FREE(toFree);
  }

}

void GLContainer::ManageComp(GLComponent *comp,SDL_Event *evt) {

  comp->ManageEvent(evt);
  if(!comp->IsEventProcessed()) GLToolkit::SetCursor(comp->GetCursor());
  evtProcessed = true;

}

void GLContainer::RelayEvent(GLComponent *comp,SDL_Event *evt,int ox,int oy) {

  if(comp->IsEnabled() && comp->IsVisible() && (!evtProcessed)) {
	  
    // Focus
    if( evt->type == SDL_MOUSEBUTTONDOWN ) {
      if(parentWin->IsInComp(comp,evt->button.x+ox,evt->button.y+oy) && comp->IsFocusable()) {
        if( comp != lastFocus ) {
          comp->SetFocus(true);
          if( lastFocus ) lastFocus->SetFocus(false);
          lastFocus = comp;
          lastClick = SDL_GetTicks();
        } else {
          // Emulate double click
          int t = SDL_GetTicks();
          if( lastClick && evt->button.button==SDL_BUTTON_LEFT ) {
            if( t-lastClick<250 ) {
              t = 0;
              evt->type = SDL_MOUSEBUTTONDBLCLICK;
            }
          }
          lastClick = t;
        }
        draggedComp = comp;
      }
    }
	
    // Relay events
    if( evt->type == SDL_MOUSEBUTTONUP || evt->type == SDL_MOUSEBUTTONDOWN ) {
      if(parentWin->IsInComp(comp,evt->button.x+ox,evt->button.y+oy)) {
        ManageComp(comp,evt);
      }
	}
	else if (evt->type ==  SDL_MOUSEWHEEL) {
		int x, y;
		SDL_GetMouseState(&x, &y);
		if (parentWin->IsInComp(comp, x, y)) {
			ManageComp(comp,evt);
		}
	} else if (evt->type == SDL_MOUSEMOTION) {
      if( draggedComp ) {
        if(draggedComp==comp) ManageComp(comp,evt);
      } else {
        if(parentWin->IsInComp(comp,evt->motion.x+ox,evt->motion.y+oy)) {
          ManageComp(comp,evt);
        }
      }
    } else if( evt->type == SDL_WINDOWEVENT && (evt->window.event == SDL_WINDOWEVENT_ENTER)) {
      ManageComp(comp,evt);
    } else {
      if( comp->HasFocus() ) {
        ManageComp(comp,evt);
      }
    }
	
  }

}

void GLContainer::ProcessKeyboardShortcut(int accId) {
}

void GLContainer::ProcessMessage(GLComponent *src,int message) {
  if(redirect) redirect->ProcessMessage(src,message);
}

void GLContainer::PaintComponents() {
  COMPLINK *node = list;
  while(node!=NULL) {
    if(node->comp->IsVisible()) node->comp->Paint();
	GLToolkit::CheckGLErrors("GLComponent:Paint()");
    node = node->next;
  }
}
