// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLLABELH_
#define _GLLABELH_

#include "GLComponent.h"
#include <string>

class GLFont2D;

class GLLabel : public GLComponent {

public:

  // Construction
  GLLabel(const char *text);
  ~GLLabel();

  // Component method
  void SetText(std::string text);
  void SetText(const char *text);
  void SetTextColor(int r,int g,int b);
  void GetTextBounds(int *w, int *h);
  void Clear();
  char* GetText();

  // Implementation
  void Paint();
  void RestoreDeviceObjects();

protected:

  char *mText;
  char *lines[64];
  int  nbLine;
  int  txtWidth;
  int  txtHeight;
  float rText;
  float gText;
  float bText;
  GLFont2D *font; 

};

class GLOverlayLabel : public GLLabel {
public:
	GLOverlayLabel(const char *text);
	void SetBackgroundColor(float r,float g,float b);
	void Paint();
	void SetTextSize(float s);
	bool paintBg;
private:
	float rBack;
	float gBack;
	float bBack;
	float sizeFactor;
};
#endif /* _GLLABELH_ */