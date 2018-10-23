// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLCHARTOPTIONSH_
#define _GLCHARTOPTIONSH_

//#include "AxisPanel.h"
#include "../GLTabWindow.h"

class GLTitledPanel;
class GLChart;
class GLComponent;
class GLToggle;
class GLCombo;
class AxisPanel;
class GLLabel;
class GLTextField;

class GLChartOptions : public GLTabWindow {

public:

  // Construction
  GLChartOptions(GLChart *chart);
  ~GLChartOptions();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  void error(const char *m);
  void commit();

  GLChart    *chart;
  GLButton   *closeBtn;

  // general panel
  GLTitledPanel *gLegendPanel;

  GLLabel      *generalLegendLabel;
  GLTextField *generalLegendText;

  GLToggle    *generalLabelVisibleCheck;

  GLTitledPanel *gColorFontPanel;

  GLLabel *generalFontHeaderLabel;
  GLLabel *generalFontHeaderSampleLabel;
  GLButton *generalFontHeaderBtn;

  GLLabel *generalFontLabelLabel;
  GLLabel *generalFontLabelSampleLabel;
  GLButton *generalFontLabelBtn;

  GLLabel *generalBackColorLabel;
  GLLabel *generalBackColorView;
  GLButton *generalBackColorBtn;

  GLTitledPanel *gGridPanel;

  GLCombo *generalGridCombo;

  GLCombo *generalLabelPCombo;
  GLLabel *generalLabelPLabel;

  GLCombo *generalGridStyleCombo;
  GLLabel *generalGridStyleLabel;
  GLTitledPanel *gMiscPanel;

  GLLabel *generalDurationLabel;
  GLTextField *generalDurationText;

  // Axis panel
  AxisPanel *y1Panel;
  AxisPanel *y2Panel;
  AxisPanel *xPanel;

};

#endif /* _GLCHARTOPTIONSH_ */
