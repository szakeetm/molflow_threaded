// Copyright (c) 2011 rubicon IT GmbH
#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#include "GLApp.h"
#include "MathTools.h" //Min, max
#include "GLToolkit.h"
#include "GLWindowManager.h"
#include "GLComponent.h"
#include "GLWindow.h"
#include <math.h>
#include <stdlib.h>
#include <cstring> //strcpy, etc.
#ifndef _WIN32
#include <unistd.h> //_exit()
#endif

GLApplication *theApp=NULL;

#ifdef _WIN32
LARGE_INTEGER perfTickStart; // Fisrt tick
double perfTicksPerSec;      // Performance counter (number of tick per second)
#endif

GLApplication::GLApplication() {

  m_bWindowed = true;
  m_strWindowTitle = "GL application";
  strcpy(m_strFrameStats,"");
  strcpy(m_strEventStats,"");
  m_screenWidth = 640;
  m_screenHeight = 480;
  m_minScreenWidth = 640;
  m_minScreenHeight = 480;
  m_bResizable = false;
  wnd = new GLWindow();
  wnd->SetMaster(true);
  wnd->SetBorder(false);
  wnd->SetBackgroundColor(0,0,0);
  wnd->SetBounds(0,0,m_screenWidth,m_screenHeight);
  wnd->SetVisible(true); // Make top level shell

#ifdef _DEBUG
  nbRestore = 0;
  fPaintTime = 0.0;
  fMoveTime = 0.0;
#endif

#ifdef _WIN32
  m_fscreenWidth = GetSystemMetrics(SM_CXSCREEN);
  m_fscreenHeight = GetSystemMetrics(SM_CYSCREEN);

  LARGE_INTEGER qwTicksPerSec;
  QueryPerformanceFrequency( &qwTicksPerSec );
  QueryPerformanceCounter( &perfTickStart );
  perfTicksPerSec = (double)qwTicksPerSec.QuadPart;

#else
  // TODO
  m_fscreenWidth=1280;
  m_fscreenHeight=1024;
#endif

}

double GLApplication::GetTick() {

	return (double)SDL_GetTicks() / 1000.0;

}

int GLApplication::setUpSDL(bool doFirstInit) {
	int errCode;
	if (doFirstInit) {


		/*
		Uint32 flags;
		flags  = SDL_OPENGL;
		flags |= (m_bWindowed?0:SDL_FULLSCREEN);
		flags |= (m_bResizable?SDL_RESIZABLE:0);

		if( SDL_SetVideoMode( m_screenWidth, m_screenHeight, 0, flags ) == NULL )
		{
		  GLToolkit::Log("GLApplication::setUpSDL SDL_SetVideoMode() failed.");
		  return GL_FAIL;
		}

		SDL_Surface *vSurf = SDL_GetVideoSurface();
		m_bitsPerPixel = vSurf->format->BitsPerPixel;
		*/

		Uint32 flags;
		flags = SDL_WINDOW_OPENGL;
		flags |= (m_bWindowed ? 0 : SDL_WINDOW_FULLSCREEN);
		flags |= (m_bResizable ? SDL_WINDOW_RESIZABLE : 0);

		mainScreen = SDL_CreateWindow("My Game Window",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			m_screenWidth, m_screenHeight,
			flags);

		if (mainScreen == NULL)
		{
			GLToolkit::Log("GLApplication::SDL_CreateWindow() failed.");
			return GL_FAIL;
		}
		mainContext = SDL_GL_CreateContext(mainScreen);
		if (mainContext == NULL) {
			GLToolkit::Log("GLApplication::SDL_GL_CreateContext() failed.");
			return GL_FAIL;
		}

		m_bitsPerPixel = SDL_BITSPERPIXEL(SDL_GetWindowPixelFormat(mainScreen));
	}

	errCode = GLToolkit::RestoreDeviceObjects(m_screenWidth, m_screenHeight);
	if (!errCode) {
		GLToolkit::Log("GLApplication::setUpSDL GLToolkit::RestoreDeviceObjects() failed.");
		return GL_FAIL;
	}
	if (doFirstInit) OneTimeSceneInit();

  GLWindowManager::RestoreDeviceObjects();
#ifdef _DEBUG
  nbRestore++;
#endif
  wnd->SetBounds(0,0,m_screenWidth,m_screenHeight);
  errCode = RestoreDeviceObjects();
  if( !errCode ) {
    GLToolkit::Log("GLApplication::setUpSDL GLApplication::RestoreDeviceObjects() failed.");
    return GL_FAIL;
  }

  return GL_OK;

}

int GLApplication::ToggleFullscreen() {

  GLToolkit::InvalidateDeviceObjects();
  GLWindowManager::InvalidateDeviceObjects();
  InvalidateDeviceObjects();

  m_bWindowed = !m_bWindowed;
  m_screenWidth = m_fscreenWidth;
  m_screenHeight = m_fscreenHeight;

  if( setUpSDL() == GL_OK ) {
    GLWindowManager::Resize();
    return GL_OK;
  } else {
    return GL_FAIL;
  }

}

void GLApplication::SetTitle(std::string title) {

  m_strWindowTitle = title;
  SDL_SetWindowTitle(mainScreen, title.c_str());

}

int GLApplication::Create(int width, int height, bool bFullScreen ) {

	theApp=this;
  m_screenWidth = width;
  m_screenHeight = height;
  m_bWindowed = !bFullScreen;

  //Initialize SDL
  if( SDL_Init( SDL_INIT_EVERYTHING ) < 0 )
  {
    GLToolkit::Log("GLApplication::Create SDL_Init() failed.");
    return GL_FAIL;
  }

  //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
  //SDL_EnableUNICODE( 1 );
  //SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
    
  //SDL_WM_SetCaption( m_strWindowTitle.c_str(), NULL ); //To replace

  return setUpSDL(true);

}

void GLApplication::Pause(bool bPause) {
}

int GLApplication::Resize( size_t nWidth, size_t nHeight, bool forceWindowed ) {

  int width  = Max((int)nWidth,m_minScreenWidth);
  int height = Max((int)nHeight,m_minScreenHeight);

  m_screenWidth = width;
  m_screenHeight = height;

  SDL_SetWindowSize(mainScreen, width,height); //Enlarge too small window if needed
  
  GLToolkit::InvalidateDeviceObjects();
  GLWindowManager::InvalidateDeviceObjects();
  InvalidateDeviceObjects();
  
  if( forceWindowed ) m_bWindowed = true;

  if( setUpSDL() == GL_OK ) {
    GLWindowManager::Resize();
    return GL_OK;
  } else {
    return GL_FAIL;
  }
  
  return GL_OK;
}

void GLApplication::Add(GLComponent *comp) {
  wnd->Add(comp);
}

void GLApplication::Exit() {

  char *logs = GLToolkit::GetLogs();
#ifdef _WIN32
  if(logs) {
    strcat(logs,"\nDo you want to exit ?");
    if( MessageBox(NULL,logs,"[Unexpected error]",MB_YESNO)==IDNO ) {
      GLToolkit::ClearLogs();
      return;
    }
  }
#else
  if(logs) {
    printf("[Unexpected error]\n");
    printf("%s",logs);
  }
#endif
  SAFE_FREE(logs);

  GLToolkit::InvalidateDeviceObjects();
  wnd->InvalidateDeviceObjects();
  InvalidateDeviceObjects();
  OnExit();
  SDL_GL_DeleteContext(mainContext);
  SDL_DestroyWindow(mainScreen);
  SDL_Quit();
  _exit(0);

}

void GLApplication::UpdateStats() {

  int fTick = SDL_GetTicks();
  float eps;

  // Update timing
  nbFrame++;
  float fTime = (float)(fTick - lastTick) * 0.001f;
  if( (fTick - lastTick) >= 1000 ) {
     int t0 = fTick;
     int t = t0 - lastTick;
     m_fFPS = (float)(nbFrame*1000) / (float)t;
     eps = (float)(nbEvent*1000) / (float)t;
     nbFrame = 0;
     nbEvent = 0;
     lastTick = t0;
     sprintf(m_strFrameStats,"%.2f fps (%dx%dx%d)   ",m_fFPS,m_screenWidth,m_screenHeight,m_bitsPerPixel);
     sprintf(m_strEventStats,"%.2f eps C:%d W:%d M:%d J:%d K:%d S:%d A:%d R:%d E:%d O:%d   ",
             eps,nbMouse,nbWheel,nbMouseMotion,nbJoystic,nbKey,nbSystem,nbActive,nbResize,nbExpose,nbOther);
  }

  m_fTime = (float) ( fTick - firstTick ) * 0.001f;
  //m_fElapsedTime = (fTick - lastFrTick) * 0.001f;
  //lastFrTick = fTick;

#ifdef _DEBUG
  nbPoly=0;
  nbLine=0;
#endif

}

void GLApplication::UpdateEventCount(SDL_Event *evt) {

  switch(evt->type) {		

      case SDL_KEYDOWN:
      case SDL_KEYUP:
        nbKey++;
        break;

      case SDL_MOUSEMOTION:
        nbMouseMotion++;
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
          nbMouse++;
        break;

	  case SDL_MOUSEWHEEL:
		nbWheel++;
		break;

      case SDL_JOYAXISMOTION:
      case SDL_JOYBALLMOTION:
      case SDL_JOYHATMOTION:
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        nbJoystic++;
        break;

      case SDL_QUIT:
      case SDL_SYSWMEVENT:
        nbSystem++;
        break;

	  case SDL_WINDOWEVENT:
		  switch (evt->window.event) {
		  case SDL_WINDOWEVENT_RESIZED:
			  nbResize++;
			  break;

		  case SDL_WINDOWEVENT_EXPOSED:
			  nbExpose++;
			  break;
		  case SDL_WINDOWEVENT_ENTER:
		  case SDL_WINDOWEVENT_FOCUS_GAINED:
			  nbActive++;
			  break;
		  }
		  break;

      default:
        nbOther++;
        break;

  }

  if(evt->type) nbEvent++;

}

void GLApplication::Run() {
	#ifdef MOLFLOW
	extern MolFlow *mApp;
	#endif

	#ifdef SYNRAD
	extern SynRad*mApp;
	#endif
  SDL_Event sdlEvent;

  bool   quit = false;
  int    ok;
  GLenum glError;
//#ifdef _DEBUG
  double t1,t0;
//#endif

  // Stats
  m_fTime        = 0.0f;
  //m_fElapsedTime = 0.0f;
  m_fFPS         = 0.0f;
  fMoveTime =			
	  fPaintTime = 0.0;
  nbFrame        = 0;
  nbEvent        = 0;
  nbMouse        = 0;
  nbMouseMotion  = 0;
  nbKey          = 0;
  nbSystem       = 0;
  nbActive       = 0;
  nbResize       = 0;
  nbJoystic      = 0;
  nbOther        = 0;
  nbExpose       = 0;
  nbWheel        = 0;

  //lastTick = lastFrTick = firstTick = SDL_GetTicks();
  lastTick  = firstTick = SDL_GetTicks();

  mApp->CheckForRecovery();
  wereEvents = false;
				
  //Wait for user exit
  while( !quit )
  {
        
     //While there are events to handle
     while( !quit && SDL_PollEvent( &sdlEvent ) )
     {

		if (sdlEvent.type!=SDL_MOUSEMOTION || sdlEvent.motion.state!=0) wereEvents = true;

       UpdateEventCount(&sdlEvent);
       switch( sdlEvent.type ) {

         case SDL_QUIT:
           if (mApp->AskToSave()) quit = true;
           break;

         case SDL_WINDOWEVENT:
           if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED) Resize(sdlEvent.window.data1,sdlEvent.window.data2);
           break;

         case SDL_SYSWMEVENT:
           break;

         default:

           if(GLWindowManager::ManageEvent(&sdlEvent)) {
             // Relay to GLApp EventProc
             EventProc(&sdlEvent);
           }

       }
     }

     if( quit ) {
		 Exit();
       return;
     }

     glError = glGetError();
     if( glError!=GL_NO_ERROR ) {
       GLToolkit::Log("GLApplication::ManageEvent() failed.");
       GLToolkit::printGlError(glError); 
       Exit();
     }

     UpdateStats();

	 Uint32 flags = SDL_GetWindowFlags(mainScreen);
     if (flags && (SDL_WINDOW_SHOWN & flags)) { //Application visible

//#ifdef _DEBUG
       t0 = GetTick();
//#endif
       // Call FrameMove
       ok = FrameMove();
//#ifdef _DEBUG
       t1 = GetTick();
       fMoveTime = 0.9*fMoveTime + 0.1*(t1 - t0); //Moving average over 10 FrameMoves
//#endif
       glError = glGetError();
       if( !ok || glError!=GL_NO_ERROR ) {
         GLToolkit::Log("GLApplication::FrameMove() failed.");
         GLToolkit::printGlError(glError); 
         Exit();
       }

       // Repaint
	   if (wereEvents) {
		   GLWindowManager::Repaint();
		   wereEvents = false;
	   }

	   GLToolkit::CheckGLErrors("GLApplication::Paint()");
     
     } else {

       SDL_Delay(100);

     }
      
  }
  
  //Clean up and exit
  Exit();
  
}
