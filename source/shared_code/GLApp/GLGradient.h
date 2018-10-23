// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLGRADIENTH_
#define _GLGRADIENTH_

#include "GLComponent.h"
#include "GLChart/GLChartConst.h" //linear, log scale constants
#include <vector>
class GLAxis;
class GLLabel;

#define GRADIENT_BW    0
#define GRADIENT_COLOR 1

class GLGradient : public GLComponent {

public:

	// Construction
	GLGradient(int compId);
	~GLGradient();

	static std::vector<int> GenerateColorMap();

	// Component methods
	void SetMinMax(double min, double max);
	void SetType(int type);
	void SetScale(int scale);
	int  GetScale();
	void SetMouseCursor(bool enable);

	// Implementation
	void Paint();
	void InvalidateDeviceObjects();
	void RestoreDeviceObjects();
	void ManageEvent(SDL_Event *evt);

private:

	void UpdateValue();

	GLuint       colorTex;
	GLuint       bwTex;
	GLAxis       *axis;
	GLLabel      *mouseValue;
	bool         mouseCursor;
	int          gType;

	int          gWidth;
	int          gHeight;
	int          gPosX;
	int          gPosY;
	double       cursorPos;

};

#endif /* _GLGRADIENTH_ */