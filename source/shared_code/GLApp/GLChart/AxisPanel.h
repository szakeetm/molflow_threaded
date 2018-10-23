// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLCHARTAXISPANELH_
#define _GLCHARTAXISPANELH_

class GLAxis;
class GLTitledPanel;
class GLButton;
class GLComponent;
class GLCombo;
class GLToggle;
class GLTextField;
class GLTabWindow;
class GLLabel;
class GLChart;

class AxisPanel {

public:

   AxisPanel(GLAxis *a,int axisType,GLChart *parentChart);
   void AddToPanel(GLTabWindow *parent,int pIdx);
   void ProcessMessage(GLComponent *src,int message);

private:

  void commit();
  void error(const char *m);

  GLAxis  *pAxis;
  GLChart *pChart;
  int     type;

  GLTitledPanel  *scalePanel;
  GLTitledPanel  *settingPanel;

  GLLabel     *MinLabel;
  GLTextField *MinText;
  GLLabel     *MaxLabel;
  GLTextField *MaxText;
  GLToggle    *AutoScaleCheck;

  GLLabel    *ScaleLabel;
  GLCombo    *ScaleCombo;
  GLToggle   *SubGridCheck;
  GLToggle   *VisibleCheck;
  GLToggle   *OppositeCheck;

  GLCombo    *FormatCombo;
  GLLabel    *FormatLabel;

  GLLabel     *TitleLabel;
  GLTextField *TitleText;

  GLLabel     *ColorLabel;
  GLLabel     *ColorView;
  GLButton    *ColorBtn;

  GLLabel     *PositionLabel;
  GLCombo     *PositionCombo;

};

#endif /* _GLCHARTAXISPANELH_ */
