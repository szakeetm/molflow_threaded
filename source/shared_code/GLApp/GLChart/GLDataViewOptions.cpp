// Copyright (c) 2011 rubicon IT GmbH
#include "GLChart.h"
#include "../GLMessageBox.h"
#include "../GLColorBox.h"
#include "../GLToolkit.h"
#include "../GLWindowManager.h"
#include "../GLCombo.h"
#include "../GLButton.h"
#include "../GLSpinner.h"
#include "../GLToggle.h"
#include "../GLLabel.h"
#include "../GLTitledPanel.h"
#include "../GLTextField.h"

static int textCR = 0;
static int textCG = 0;
static int textCB = 0;

/**
  * Dialog constructor.
  * @param chart Chart used to commit change (can be null)
  * @param v DataView to edit
  */
GLDataViewOptions::GLDataViewOptions(GLChart *chart) : GLTabWindow() {

    this->dataView = NULL;
    this->chart = chart;

    SetTitle("Data view options");

    SetPanelNumber(7);
    SetPanelName(0,"Curve");
    SetPanelName(1,"Bar");
    SetPanelName(2,"Marker");
    SetPanelName(3,"Transform");
    SetPanelName(4,"Interpolation");
    SetPanelName(5,"Smoothing");
    SetPanelName(6,"Math");

    // Line panel construction

    viewTypeLabel = new GLLabel("View type");
    viewTypeLabel->SetTextColor(textCR,textCG,textCB);

    viewTypeCombo = new GLCombo(0);
    viewTypeCombo->SetSize(2);
    viewTypeCombo->SetValueAt(0,"Line");
    viewTypeCombo->SetValueAt(1,"Bar graph");

    lineColorView = new GLLabel("");
    lineColorView->SetOpaque(true);
    lineColorView->SetBorder(BORDER_ETCHED);
    lineColorBtn = new GLButton(0,"...");
    lineColorLabel = new GLLabel("Line Color");
    lineColorLabel->SetTextColor(textCR,textCG,textCB);

    fillColorView = new GLLabel("");
    fillColorView->SetOpaque(true);
    fillColorView->SetBorder(BORDER_ETCHED);
    fillColorBtn = new GLButton(0,"...");
    fillColorLabel = new GLLabel("Fill Color");
    fillColorLabel->SetTextColor(textCR,textCG,textCB);

    lineWidthLabel = new GLLabel("Line Width");
    lineWidthLabel->SetTextColor(textCR,textCG,textCB);
    lineWidthSpinner = new GLSpinner(0);
    lineWidthSpinner->SetIncrement(1.0);
    lineWidthSpinner->SetMinMax(0.0,10.0);

    lineDashLabel = new GLLabel("Line style");
    lineDashLabel->SetTextColor(textCR,textCG,textCB);
    lineDashCombo = new GLCombo(0);
    lineDashCombo->SetSize(5);
    lineDashCombo->SetValueAt(0,"Solid");
    lineDashCombo->SetValueAt(1,"Point dash");
    lineDashCombo->SetValueAt(2,"Short dash");
    lineDashCombo->SetValueAt(3,"Long dash");
    lineDashCombo->SetValueAt(4,"Dot dash");

    fillStyleLabel = new GLLabel("Fill style");
    fillStyleLabel->SetTextColor(textCR,textCG,textCB);
    fillStyleCombo = new GLCombo(0);
    fillStyleCombo->SetSize(11);
    fillStyleCombo->SetValueAt(0,"No fill");
    fillStyleCombo->SetValueAt(1,"Solid");
    fillStyleCombo->SetValueAt(2,"Large leff hatch");
    fillStyleCombo->SetValueAt(3,"Large right hatch");
    fillStyleCombo->SetValueAt(4,"Large cross hatch");
    fillStyleCombo->SetValueAt(5,"Small leff hatch");
    fillStyleCombo->SetValueAt(6,"Small right hatch");
    fillStyleCombo->SetValueAt(7,"Small cross hatch");
    fillStyleCombo->SetValueAt(8,"Dot pattern 1");
    fillStyleCombo->SetValueAt(9,"Dot pattern 2");
    fillStyleCombo->SetValueAt(10,"Dot pattern 3");

    lineNameLabel = new GLLabel("Name");
    lineNameLabel->SetTextColor(textCR,textCG,textCB);
    lineNameText = new GLTextField(0,"");
    lineNameText->SetEditable(true);

    Add(0,viewTypeLabel);
    Add(0,viewTypeCombo);
    Add(0,lineColorLabel);
    Add(0,lineColorView);
    Add(0,lineColorBtn);
    Add(0,fillColorLabel);
    Add(0,fillColorView);
    Add(0,fillColorBtn);
    Add(0,lineWidthLabel);
    Add(0,lineWidthSpinner);
    Add(0,lineDashLabel);
    Add(0,lineDashCombo);
    Add(0,fillStyleLabel);
    Add(0,fillStyleCombo);
    Add(0,lineNameLabel);
    Add(0,lineNameText);

    viewTypeLabel->SetBounds(10, 10, 100, 19);
    viewTypeCombo->SetBounds(115, 10, 125, 19);

    lineColorLabel->SetBounds(10, 35, 100, 19);
    lineColorView->SetBounds(115, 35, 100, 19);
    lineColorBtn->SetBounds(220, 35, 20, 19);

    fillColorLabel->SetBounds(10, 60, 100, 19);
    fillColorView->SetBounds(115, 60, 100, 19);
    fillColorBtn->SetBounds(220, 60, 20, 19);

    fillStyleLabel->SetBounds(10, 85, 100, 19);
    fillStyleCombo->SetBounds(115, 85, 125, 19);

    lineWidthLabel->SetBounds(10, 110, 100, 19);
    lineWidthSpinner->SetBounds(115, 110, 125, 19);

    lineDashLabel->SetBounds(10, 135, 100, 19);
    lineDashCombo->SetBounds(115, 135, 125, 19);

    lineNameLabel->SetBounds(10, 160, 100, 19);
    lineNameText->SetBounds(115, 160, 125, 19);

    // Bar panel construction
    barWidthLabel = new GLLabel("Bar Width");
    barWidthLabel->SetTextColor(textCR,textCG,textCB);
    barWidthSpinner = new GLSpinner(0);
    barWidthSpinner->SetIncrement(1.0);
    barWidthSpinner->SetMinMax(0.0,100.0);

    fillMethodLabel = new GLLabel("Filling method");
    fillMethodLabel->SetTextColor(textCR,textCG,textCB);
    fillMethodCombo = new GLCombo(0);
    fillMethodCombo->SetSize(3);
    fillMethodCombo->SetValueAt(0,"From Up");
    fillMethodCombo->SetValueAt(1,"From Zero");
    fillMethodCombo->SetValueAt(2,"From Bottom");

    Add(1,barWidthLabel);
    Add(1,barWidthSpinner);
    Add(1,fillMethodLabel);
    Add(1,fillMethodCombo);

    barWidthLabel->SetBounds(10, 10, 100, 19);
    barWidthSpinner->SetBounds(115, 10, 125, 19);

    fillMethodLabel->SetBounds(10, 35, 100, 19);
    fillMethodCombo->SetBounds(115, 35, 125, 19);

    // Marker panel construction
    markerColorView = new GLLabel("");
    markerColorView->SetOpaque(true);
    markerColorView->SetBorder(BORDER_ETCHED);

    markerColorBtn = new GLButton(0,"...");

    markerColorLabel = new GLLabel("Color");
    markerColorLabel->SetTextColor(textCR,textCG,textCB);

    markerSizeLabel = new GLLabel("Size");
    markerSizeLabel->SetTextColor(textCR,textCG,textCB);

    markerSizeSpinner = new GLSpinner(0);
    markerSizeSpinner->SetIncrement(1.0);
    markerSizeSpinner->SetMinMax(0.0,20.0);

    markerStyleLabel = new GLLabel("Marker style");
    markerStyleLabel->SetTextColor(textCR,textCG,textCB);

    markerStyleCombo = new GLCombo(0);
    markerStyleCombo->SetSize(11);
    markerStyleCombo->SetValueAt(0,"None");
    markerStyleCombo->SetValueAt(1,"Dot");
    markerStyleCombo->SetValueAt(2,"Box");
    markerStyleCombo->SetValueAt(3,"triangle");
    markerStyleCombo->SetValueAt(4,"Diamond");
    markerStyleCombo->SetValueAt(5,"Star");
    markerStyleCombo->SetValueAt(6,"Vert. line");
    markerStyleCombo->SetValueAt(7,"Horz. line");
    markerStyleCombo->SetValueAt(8,"Cross");
    markerStyleCombo->SetValueAt(9,"Circle");
    markerStyleCombo->SetValueAt(10,"Sqaure");

    labelVisibleCheck = new GLToggle(0,"Legend visible");
    labelVisibleCheck->SetTextColor(textCR,textCG,textCB);

    Add(2,markerColorLabel);
    Add(2,markerColorView);
    Add(2,markerColorBtn);
    Add(2,markerSizeLabel);
    Add(2,markerSizeSpinner);
    Add(2,markerStyleLabel);
    Add(2,markerStyleCombo);
    Add(2,labelVisibleCheck);

    markerColorLabel->SetBounds(10, 10, 100, 19);
    markerColorView->SetBounds(115, 10, 100, 19);
    markerColorBtn->SetBounds(220, 10, 20, 19);

    markerSizeLabel->SetBounds(10, 35, 100, 19);
    markerSizeSpinner->SetBounds(115, 35, 125, 19);

    markerStyleLabel->SetBounds(10, 60, 100, 19);
    markerStyleCombo->SetBounds(115, 60, 125, 19);

    labelVisibleCheck->SetBounds(8, 85, 225, 19);

    // Transform panel construction
    transformHelpLabel = new GLLabel("This apply a polynomial transform\nto the data view:\n y' = A0 + A1*y + A2*y^2");
    transformHelpLabel->SetTextColor(textCR,textCG,textCB);

    transformA0Label = new GLLabel("A0");
    transformA0Label->SetTextColor(textCR,textCG,textCB);
    transformA0Text = new GLTextField(0,"");
    transformA0Text->SetEditable(true);

    transformA1Label = new GLLabel("A1");
    transformA1Label->SetTextColor(textCR,textCG,textCB);
    transformA1Text = new GLTextField(0,"");
    transformA1Text->SetEditable(true);

    transformA2Label = new GLLabel("A2");
    transformA2Label->SetTextColor(textCR,textCG,textCB);
    transformA2Text = new GLTextField(0,"");
    transformA2Text->SetEditable(true);

    Add(3,transformHelpLabel);
    Add(3,transformA0Label);
    Add(3,transformA0Text);
    Add(3,transformA1Label);
    Add(3,transformA1Text);
    Add(3,transformA2Label);
    Add(3,transformA2Text);

    transformHelpLabel->SetBounds(10, 100, 240, 100);

    transformA0Label->SetBounds(60, 10, 30, 19);
    transformA0Text->SetBounds(95, 10, 100, 19);
    transformA1Label->SetBounds(60, 40, 30, 19);
    transformA1Text->SetBounds(95, 40, 100, 19);
    transformA2Label->SetBounds(60, 70, 30, 19);
    transformA2Text->SetBounds(95, 70, 100, 19);

    //Interpolation panel

    noInterpBtn = new GLToggle(0,"None");
    noInterpBtn->SetTextColor(textCR,textCG,textCB);
    linearBtn = new GLToggle(0,"Linear");
    linearBtn->SetTextColor(textCR,textCG,textCB);
    cosineBtn = new GLToggle(0,"Cosine");
    cosineBtn->SetTextColor(textCR,textCG,textCB);
    cubicBtn = new GLToggle(0,"Cubic");
    cubicBtn->SetTextColor(textCR,textCG,textCB);
    hermiteBtn = new GLToggle(0,"Hermite");
    hermiteBtn->SetTextColor(textCR,textCG,textCB);

    GLLabel *stepLabel = new GLLabel("Step");
    stepLabel->SetTextColor(textCR,textCG,textCB);

    stepSpinner = new GLSpinner(0);
    stepSpinner->SetIncrement(1.0);
    stepSpinner->SetMinMax(2.0,100.0);

    GLLabel *tensionLabel = new GLLabel("Tension");
    tensionLabel->SetTextColor(textCR,textCG,textCB);
    tensionText = new GLTextField(0,"");
    tensionText->SetEditable(true);
    tensionText->SetEnabled(false);

    GLLabel *biasLabel = new GLLabel("Bias");
    biasLabel->SetTextColor(textCR,textCG,textCB);
    biasText = new GLTextField(0,"");
    biasText->SetEditable(true);
    biasText->SetEnabled(false);

    noInterpBtn->SetBounds(8,10,90,19);
    linearBtn->SetBounds(8,35,90,19);
    cosineBtn->SetBounds(8,60,90,19);
    cubicBtn->SetBounds(8,85,90,19);
    hermiteBtn->SetBounds(8,110,90,19);
    stepLabel->SetBounds(140,10,50,25);
    stepSpinner->SetBounds(190,10,50,19);
    tensionLabel->SetBounds(140,35,50,19);
    tensionText->SetBounds(190,35,50,19);
    biasLabel->SetBounds(140,60,50,19);
    biasText->SetBounds(190,60,50,19);

    Add(4,noInterpBtn);
    Add(4,linearBtn);
    Add(4,cosineBtn);
    Add(4,cubicBtn);
    Add(4,hermiteBtn);
    Add(4,stepLabel);
    Add(4,stepSpinner);
    Add(4,tensionLabel);
    Add(4,tensionText);
    Add(4,biasLabel);
    Add(4,biasText);

    // Smoothing panel
    noSmoothBtn = new GLToggle(0,"None");
    noSmoothBtn->SetTextColor(textCR,textCG,textCB);

    flatSmoothBtn = new GLToggle(0,"Flat");
    flatSmoothBtn->SetTextColor(textCR,textCG,textCB);

    triangularSmoothBtn = new GLToggle(0,"Linear");
    triangularSmoothBtn->SetTextColor(textCR,textCG,textCB);

    gaussianSmoothBtn = new GLToggle(0,"Gaussian");
    gaussianSmoothBtn->SetTextColor(textCR,textCG,textCB);

    GLLabel *neighborLabel = new GLLabel("Neighbors");
    neighborLabel->SetTextColor(textCR,textCG,textCB);

    neighborSpinner = new GLSpinner(0);
    neighborSpinner->SetIncrement(2.0);
    neighborSpinner->SetMinMax(2.0,99.0);

    GLLabel *sigmaLabel = new GLLabel("Sigma");
    sigmaLabel->SetTextColor(textCR,textCG,textCB);
    sigmaText = new GLTextField(0,"");
    sigmaText->SetEditable(true);
    sigmaText->SetEnabled(false);

    GLTitledPanel *bPanel = new GLTitledPanel("Boundary extrapolation");
    bPanel->SetBold(true);

    noExtBtn = new GLToggle(0,"None");
    noExtBtn->SetTextColor(textCR,textCG,textCB);

    flatExtBtn = new GLToggle(0,"Flat");
    flatExtBtn->SetTextColor(textCR,textCG,textCB);

    linearExtBtn = new GLToggle(0,"Linear");
    linearExtBtn->SetTextColor(textCR,textCG,textCB);

    noSmoothBtn->SetBounds(8,10,90,19);
    flatSmoothBtn->SetBounds(8,35,90,19);
    triangularSmoothBtn->SetBounds(8,60,90,19);
    gaussianSmoothBtn->SetBounds(8,85,90,19);
    neighborLabel->SetBounds(140,10,50,19);
    neighborSpinner->SetBounds(190,10,50,19);
    sigmaLabel->SetBounds(140,35,50,19);
    sigmaText->SetBounds(190,35,50,19);
    bPanel->SetBounds(5,110,235,70);
    bPanel->SetCompBounds(noExtBtn,5,20,90,19);
    bPanel->SetCompBounds(flatExtBtn,5,45,90,19);
    bPanel->SetCompBounds(linearExtBtn,120,20,90,19);

    Add(5,noSmoothBtn);
    Add(5,flatSmoothBtn);
    Add(5,triangularSmoothBtn);
    Add(5,gaussianSmoothBtn);
    Add(5,neighborLabel);
    Add(5,neighborSpinner);
    Add(5,sigmaLabel);
    Add(5,sigmaText);
    Add(5,bPanel);
    Add(5,noExtBtn);
    Add(5,flatExtBtn);
    Add(5,linearExtBtn);

    //Math panel
    noMathBtn = new GLToggle(0,"No operation");
    noMathBtn->SetTextColor(textCR,textCG,textCB);
    derivativeBtn = new GLToggle(0,"Derivative");
    derivativeBtn->SetTextColor(textCR,textCG,textCB);
    integralBtn = new GLToggle(0,"Integral");
    integralBtn->SetTextColor(textCR,textCG,textCB);
    fftModBtn = new GLToggle(0,"FFT (modulus)");
    fftModBtn->SetTextColor(textCR,textCG,textCB);
    fftPhaseBtn = new GLToggle(0,"FFT (phase radians)");
    fftPhaseBtn->SetTextColor(textCR,textCG,textCB);

    noMathBtn->SetBounds(8,10,160,19);
    derivativeBtn->SetBounds(8,35,160,19);
    integralBtn->SetBounds(8,60,160,19);
    fftModBtn->SetBounds(8,85,160,19);
    fftPhaseBtn->SetBounds(8,110,160,19);

    Add(6,noMathBtn);
    Add(6,derivativeBtn);
    Add(6,integralBtn);
    Add(6,fftModBtn);
    Add(6,fftPhaseBtn);

    // Global frame construction
    nameLabel = new GLLabel("");
    nameLabel->SetBounds(10, 188, 170, 19);
    GLWindow::Add(nameLabel);

    closeBtn = new GLButton(0,"Close");
    closeBtn->SetBounds(160, 188, 80, 19);
    GLWindow::Add(closeBtn);

    Update();
    SetTextColor(120,120,120);

    // Center dialog
    int wD = 245;
    int hD = 270;
    int wS,hS;
    GLToolkit::GetScreenSize(&wS,&hS);
    int xD = (wS-wD)/2;
    int yD = (hS-hD)/2;
    SetBounds(xD,yD,wD,hD);

    RestoreDeviceObjects();

  }
  
  GLDataViewOptions::~GLDataViewOptions() {
  }

  void GLDataViewOptions::SetDataView(GLDataView *v) {
    dataView = v;
    updateControls();
  }

  void GLDataViewOptions::updateControls() {

    char tmp[256];

    if( !dataView ) return;

    nameLabel->SetText(dataView->GetName());

    viewTypeCombo->SetSelectedIndex(dataView->GetViewType());

    GLColor dColor = dataView->GetColor();
    lineColorView->SetBackgroundColor(dColor.r,dColor.g,dColor.b);
    GLColor fColor = dataView->GetFillColor();
    fillColorView->SetBackgroundColor(fColor.r,fColor.g,fColor.b);
    GLColor mColor = dataView->GetMarkerColor();
    markerColorView->SetBackgroundColor(mColor.r,mColor.g,mColor.b);

    lineWidthSpinner->SetValue(dataView->GetLineWidth());

    lineDashCombo->SetSelectedIndex(dataView->GetStyle());
    fillStyleCombo->SetSelectedIndex(dataView->GetFillStyle());
    lineNameText->SetText(dataView->GetName());
    barWidthSpinner->SetValue(dataView->GetBarWidth());
    fillMethodCombo->SetSelectedIndex(dataView->GetFillMethod());
    markerSizeSpinner->SetValue(dataView->GetMarkerSize());
    markerStyleCombo->SetSelectedIndex(dataView->GetMarker());
    labelVisibleCheck->SetState(dataView->IsLabelVisible());

    sprintf(tmp,"%g",dataView->GetA0());
    transformA0Text->SetText(tmp);
    sprintf(tmp,"%g",dataView->GetA1());
    transformA1Text->SetText(tmp);
    sprintf(tmp,"%g",dataView->GetA2());
    transformA2Text->SetText(tmp);
    stepSpinner->SetValue(dataView->GetInterpolationStep());

    noInterpBtn->SetState(false);
    linearBtn->SetState(false);
    cubicBtn->SetState(false);
    cosineBtn->SetState(false);
    hermiteBtn->SetState(false);

    switch(dataView->GetInterpolationMethod()) {
      case INTERPOLATE_NONE:
        noInterpBtn->SetState(true);
        break;
      case INTERPOLATE_LINEAR:
        linearBtn->SetState(true);
        break;
      case INTERPOLATE_CUBIC:
        cubicBtn->SetState(true);
        break;
      case INTERPOLATE_COSINE:
        cosineBtn->SetState(true);
        break;
      case INTERPOLATE_HERMITE:
        hermiteBtn->SetState(true);
        break;
    }

    noExtBtn->SetState(false);
    flatExtBtn->SetState(false);
    linearExtBtn->SetState(false);

    switch(dataView->GetSmoothingExtrapolation()) {
      case SMOOTH_EXT_NONE:
        noExtBtn->SetState(true);
        break;
      case SMOOTH_EXT_FLAT:
        flatExtBtn->SetState(true);
        break;
      case SMOOTH_EXT_LINEAR:
        linearExtBtn->SetState(true);
        break;
    }

    noSmoothBtn->SetState(false);
    flatSmoothBtn->SetState(false);
    triangularSmoothBtn->SetState(false);
    gaussianSmoothBtn->SetState(false);

    switch(dataView->GetSmoothingMethod()) {
      case SMOOTH_NONE:
        noSmoothBtn->SetState(true);
        break;
      case SMOOTH_FLAT:
        flatSmoothBtn->SetState(true);
        break;
      case SMOOTH_TRIANGULAR:
        triangularSmoothBtn->SetState(true);
        break;
      case SMOOTH_GAUSSIAN:
        gaussianSmoothBtn->SetState(true);
        break;
    }

    noMathBtn->SetState(false);
    derivativeBtn->SetState(false);
    integralBtn->SetState(false);
    fftModBtn->SetState(false);
    fftPhaseBtn->SetState(false);

    switch(dataView->GetMathFunction()) {
      case MATH_NONE:
        noMathBtn->SetState(true);
        break;
      case MATH_DERIVATIVE:
        derivativeBtn->SetState(true);
        break;
      case MATH_INTEGRAL:
        integralBtn->SetState(true);
        break;
      case MATH_FFT_MODULUS:
        fftModBtn->SetState(true);
        break;
      case MATH_FFT_PHASE:
        fftPhaseBtn->SetState(true);
        break;
    }

    sprintf(tmp,"%g",dataView->GetHermiteTension());
    tensionText->SetText(tmp);
    tensionText->SetEnabled(false);

    sprintf(tmp,"%g",dataView->GetHermiteBias());
    biasText->SetText(tmp);
    biasText->SetEnabled(false);

    neighborSpinner->SetValue(dataView->GetSmoothingNeighbors());

    sprintf(tmp,"%g",dataView->GetSmoothingGaussSigma());
    sigmaText->SetText(tmp);
    sigmaText->SetEnabled(false);

    switch(dataView->GetInterpolationMethod()) {
      case INTERPOLATE_HERMITE:
        biasText->SetEnabled(true);
        tensionText->SetEnabled(true);
        break;
    }

    switch(dataView->GetSmoothingMethod()) {
      case SMOOTH_GAUSSIAN:
        sigmaText->SetEnabled(true);
        break;
    }

  }

  /**
   * Commit change. Repaint the graph.
   */
  void GLDataViewOptions::commit() {
    updateControls();
    GLWindowManager::FullRepaint();
  }

  void GLDataViewOptions::ProcessMessage(GLComponent *src,int message) {
    
    int v;
    double d;

    if (src == closeBtn) {
      SetVisible(false);
    } else if (src == lineColorBtn) {

      GLColor c = dataView->GetColor();
      if( GLColorBox::Display("Choose line color",&c.r,&c.g,&c.b) ) {
        dataView->SetColor(c);
        lineColorView->SetBackgroundColor(c.r,c.g,c.b);
        commit();
      }

    } else if (src == fillColorBtn) {

      GLColor c = dataView->GetFillColor();
      if( GLColorBox::Display("Choose fill color",&c.r,&c.g,&c.b) ) {
        dataView->SetFillColor(c);
        fillColorView->SetBackgroundColor(c.r,c.g,c.b);
        commit();
      }

    } else if (src == markerColorBtn) {

      GLColor c = dataView->GetMarkerColor();
      if( GLColorBox::Display("Choose marker color",&c.r,&c.g,&c.b) ) {
        dataView->SetMarkerColor(c);
        markerColorView->SetBackgroundColor(c.r,c.g,c.b);
        commit();
      }

    } else if (src == lineDashCombo) {
      dataView->SetStyle(lineDashCombo->GetSelectedIndex());
      commit();
    } else if (src == fillStyleCombo) {
      dataView->SetFillStyle(fillStyleCombo->GetSelectedIndex());
      commit();
    } else if (src == fillMethodCombo) {
      dataView->SetFillMethod(fillMethodCombo->GetSelectedIndex());
      commit();
    } else if (src == viewTypeCombo) {
      dataView->SetViewType(viewTypeCombo->GetSelectedIndex());
      commit();
    } else if (src == markerStyleCombo) {
      dataView->SetMarker(markerStyleCombo->GetSelectedIndex());
      commit();
    } else if (src == labelVisibleCheck) {
      dataView->SetLabelVisible(labelVisibleCheck->GetState());
      commit();
    } else if (src == lineWidthSpinner) {
      v = (int) lineWidthSpinner->GetValue();
      dataView->SetLineWidth(v);
      commit();
    } else if (src == barWidthSpinner) {
      v = (int) barWidthSpinner->GetValue();
      dataView->SetBarWidth(v);
      commit();
    } else if (src == markerSizeSpinner) {
      v = (int) markerSizeSpinner->GetValue();
      dataView->SetMarkerSize(v);
      commit();
    } else if (src == stepSpinner) {
      v = (int) stepSpinner->GetValue();
      dataView->SetInterpolationStep(v);
      commit();
    } else if (src == neighborSpinner) {
      v = (int) neighborSpinner->GetValue();
      dataView->SetSmoothingNeighbors(v);
      commit();
    } else if (src == noInterpBtn) {
      if(noInterpBtn->GetState())
        dataView->SetInterpolationMethod(INTERPOLATE_NONE);
      commit();
    } else if (src == linearBtn) {
      if(linearBtn->GetState())
        dataView->SetInterpolationMethod(INTERPOLATE_LINEAR);
      commit();
    } else if (src == cosineBtn) {
      if(cosineBtn->GetState())
        dataView->SetInterpolationMethod(INTERPOLATE_COSINE);
      commit();
    } else if (src == cubicBtn) {
      if(cubicBtn->GetState())
        dataView->SetInterpolationMethod(INTERPOLATE_CUBIC);
      commit();
    } else if (src == hermiteBtn) {
      if(hermiteBtn->GetState())
        dataView->SetInterpolationMethod(INTERPOLATE_HERMITE);
      commit();
    } else if (src == noSmoothBtn) {
      if(noSmoothBtn->GetState())
        dataView->SetSmoothingMethod(SMOOTH_NONE);
      commit();
    } else if (src == flatSmoothBtn) {
      if(flatSmoothBtn->GetState())
        dataView->SetSmoothingMethod(SMOOTH_FLAT);
      commit();
    } else if (src == triangularSmoothBtn) {
      if(triangularSmoothBtn->GetState())
        dataView->SetSmoothingMethod(SMOOTH_TRIANGULAR);
      commit();
    } else if (src == gaussianSmoothBtn) {
      if(gaussianSmoothBtn->GetState())
        dataView->SetSmoothingMethod(SMOOTH_GAUSSIAN);
      commit();
    } else if (src == noExtBtn) {
      if(noExtBtn->GetState())
        dataView->SetSmoothingExtrapolation(SMOOTH_EXT_NONE);
      commit();
    } else if (src == flatExtBtn) {
      if(flatExtBtn->GetState())
        dataView->SetSmoothingExtrapolation(SMOOTH_EXT_FLAT);
      commit();
    } else if (src == linearExtBtn) {
      if(linearExtBtn->GetState())
        dataView->SetSmoothingExtrapolation(SMOOTH_EXT_LINEAR);
      commit();
    } else if (src == noMathBtn) {
      if(noMathBtn->GetState())
        dataView->SetMathFunction(MATH_NONE);
      commit();
    } else if (src == derivativeBtn) {
      if(derivativeBtn->GetState())
        dataView->SetMathFunction(MATH_DERIVATIVE);
      commit();
    } else if (src == integralBtn) {
      if(integralBtn->GetState())
        dataView->SetMathFunction(MATH_INTEGRAL);
      commit();
    } else if (src == fftModBtn) {
      if(fftModBtn->GetState())
        dataView->SetMathFunction(MATH_FFT_MODULUS);
      commit();
    } else if (src == fftPhaseBtn) {
      if(fftPhaseBtn->GetState())
        dataView->SetMathFunction(MATH_FFT_PHASE);
      commit();
    } else if (src == transformA0Text && message==MSG_TEXT) {

      if(!transformA0Text->GetNumber(&d)) {
        error("Malformed A0 value");
        return;
      }
      dataView->SetA0(d);
      commit();

    } else if (src == transformA1Text && message==MSG_TEXT) {

      if(!transformA1Text->GetNumber(&d)) {
        error("Malformed A1 value");
        return;
      }
      dataView->SetA1(d);
      commit();

    } else if (src == transformA2Text && message==MSG_TEXT) {

      if(!transformA2Text->GetNumber(&d)) {
        error("Malformed A2 value");
        return;
      }
      dataView->SetA2(d);
      commit();

    } else if (src == lineNameText) {

      dataView->SetName(lineNameText->GetText().c_str());
      commit();

    } else if (src == tensionText && message==MSG_TEXT) {

      if(!tensionText->GetNumber(&d)) {
        error("Malformed tension value");
        return;
      }
      dataView->SetHermiteTension(d);
      commit();

    } else if (src == biasText && message==MSG_TEXT) {

      if(!biasText->GetNumber(&d)) {
        error("Malformed bias value");
        return;
      }
      dataView->SetHermiteBias(d);
      commit();

    } else if (src == sigmaText && message==MSG_TEXT) {

      if(!sigmaText->GetNumber(&d)) {
        error("Malformed sigma value");
        return;
      }
      dataView->SetSmoothingGaussSigma(d);
      commit();

    }

    GLTabWindow::ProcessMessage(src,message);

}

// Error message
void GLDataViewOptions::error(const char *m) {
  GLMessageBox::Display(m,"Chart options",GLDLG_OK,GLDLG_ICONERROR);
}
