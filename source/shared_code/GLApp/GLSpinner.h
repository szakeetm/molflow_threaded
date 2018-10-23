// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLSPINNERH_
#define _GLSPINNERH_

#include "GLComponent.h"

class GLSpinner : public GLComponent {

public:

  // Construction
  GLSpinner(int compId);

  // Components method
  void SetValue(double value);
  double GetValue();
  void SetMinMax(double min,double max);
  void SetIncrement(double inc);
  void SetFormat(const char *format);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);

private:

  double value;
  double increment;
  double min;
  double max;
  char format[64];
  int  state;

};

#endif /* _GLSPINNERH_ */
