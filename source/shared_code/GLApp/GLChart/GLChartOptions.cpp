// Copyright (c) 2011 rubicon IT GmbH
#include "GLChart.h"
#include "../GLMessageBox.h"
#include "../GLColorBox.h"
#include "../GLToolkit.h"
#include "../GLWindowManager.h"
#include "../GLTitledPanel.h"
#include "../GLToggle.h"
#include "../GLButton.h"
#include "../GLCombo.h"
#include "../GLLabel.h"
#include "../GLTextField.h"
#include "../MathTools.h"
#include "AxisPanel.h"

static int textCR = 0;
static int textCG = 0;
static int textCB = 0;

/**
 * GLChartOption constructor.
 * @param chart Chart to be edited.
 */

GLChartOptions::GLChartOptions(GLChart *chart) : GLTabWindow() {

  char tmp[256];

    SetTitle("Chart properties");
    this->chart = chart;

    // **********************************************
    // General panel construction
    // **********************************************

    gLegendPanel = new GLTitledPanel("Legends");
    gLegendPanel->SetBold(true);
    gColorFontPanel= new GLTitledPanel("Colors & Fonts");
    gColorFontPanel->SetBold(true);
    gGridPanel= new GLTitledPanel("Axis grid");
    gGridPanel->SetBold(true);
    gMiscPanel= new GLTitledPanel("Misc");
    gMiscPanel->SetBold(true);

    generalLegendLabel = new GLLabel("Chart title");
    generalLegendLabel->SetTextColor(textCR,textCG,textCB);
    generalLegendText = new GLTextField(0,"");
    generalLegendText->SetEditable(true);
    generalLegendText->SetText(chart->GetHeader());

    generalLabelVisibleCheck = new GLToggle(0,"Visible");
    generalLabelVisibleCheck->SetTextColor(textCR,textCG,textCB);
    generalLabelVisibleCheck->SetState(chart->IsLabelVisible());

    generalBackColorLabel = new GLLabel("Chart background");
    generalBackColorLabel->SetTextColor(textCR,textCG,textCB);
    generalBackColorView = new GLLabel("");
    generalBackColorView->SetOpaque(true);
    generalBackColorView->SetBorder(BORDER_ETCHED);
    GLColor bColor = chart->GetChartBackground();
    generalBackColorView->SetBackgroundColor(bColor.r,bColor.g,bColor.b);
    generalBackColorBtn = new GLButton(0,"...");

    generalLabelPLabel = new GLLabel("Placement");
    generalLabelPLabel->SetTextColor(textCR,textCG,textCB);

    generalLabelPCombo = new GLCombo(0);
    generalLabelPCombo->SetSize(5);
    generalLabelPCombo->SetValueAt(0,"Bottom");
    generalLabelPCombo->SetValueAt(1,"Top");
    generalLabelPCombo->SetValueAt(2,"Right");
    generalLabelPCombo->SetValueAt(3,"Left");
    generalLabelPCombo->SetValueAt(4,"Row");
    generalLabelPCombo->SetSelectedIndex(chart->GetLabelPlacement());

    generalGridCombo = new GLCombo(0);
    generalGridCombo->SetSize(6);
    generalGridCombo->SetValueAt(0,"None");
    generalGridCombo->SetValueAt(1,"On X");
    generalGridCombo->SetValueAt(2,"On Y1");
    generalGridCombo->SetValueAt(3,"On Y2");
    generalGridCombo->SetValueAt(4,"On X and Y1");
    generalGridCombo->SetValueAt(5,"On X and Y2");

    bool vx = chart->GetXAxis()->IsGridVisible();
    bool vy1 = chart->GetY1Axis()->IsGridVisible();
    bool vy2 = chart->GetY2Axis()->IsGridVisible();

    int sel = 0;
    if (vx && !vy1 && !vy2) sel = 1;
    if (!vx && vy1 && !vy2) sel = 2;
    if (!vx && !vy1 && vy2) sel = 3;
    if (vx && vy1 && !vy2) sel = 4;
    if (vx && !vy1 && vy2) sel = 5;

    generalGridCombo->SetSelectedIndex(sel);

    generalGridStyleLabel = new GLLabel("Style");
    generalGridStyleLabel->SetTextColor(textCR,textCG,textCB);

    generalGridStyleCombo = new GLCombo(0);
    generalGridStyleCombo->SetSize(5);
    generalGridStyleCombo->SetValueAt(0,"Solid");
    generalGridStyleCombo->SetValueAt(1,"Dot");
    generalGridStyleCombo->SetValueAt(2,"Short dash");
    generalGridStyleCombo->SetValueAt(3,"Long dash");
    generalGridStyleCombo->SetValueAt(4,"Dot dash");
    generalGridStyleCombo->SetSelectedIndex(chart->GetY1Axis()->GetGridStyle());

    generalDurationLabel = new GLLabel("Display duration (s)");
    generalDurationLabel->SetTextColor(textCR,textCG,textCB);
    generalDurationText = new GLTextField(0,"");
    generalDurationText->SetEditable(true);

    double d = chart->GetDisplayDuration();
    if(d==MAX_VALUE) sprintf(tmp,"Infinity");
    else             sprintf(tmp,"%g",d);
    generalDurationText->SetText(tmp);

    generalFontHeaderLabel = new GLLabel("Header font");
    generalFontHeaderLabel->SetTextColor(textCR,textCG,textCB);
    generalFontHeaderSampleLabel = new GLLabel("Sample text");
    generalFontHeaderSampleLabel->SetTextColor(textCR,textCG,textCB);
    generalFontHeaderSampleLabel->SetOpaque(false);
    generalFontHeaderBtn = new GLButton(0,"...");

    generalFontLabelLabel = new GLLabel("Label font");
    generalFontLabelLabel->SetTextColor(textCR,textCG,textCB);
    generalFontLabelSampleLabel = new GLLabel("Sample 0123456789");
    generalFontLabelSampleLabel->SetTextColor(textCR,textCG,textCB);
    generalFontLabelSampleLabel->SetOpaque(false);
    generalFontLabelBtn = new GLButton(0,"...");

    // Global frame construction

    SetPanelNumber(4);
    SetPanelName(0,"General");
    SetPanelName(1,"X axis");
    SetPanelName(2,"Y1 axis");
    SetPanelName(3,"Y2 axis");

    Add(0,gLegendPanel);
    Add(0,generalLabelVisibleCheck);
    Add(0,generalLabelPLabel);
    Add(0,generalLabelPCombo);

    Add(0,gGridPanel);
    Add(0,generalGridCombo);
    Add(0,generalGridStyleLabel);
    Add(0,generalGridStyleCombo);

    Add(0,gColorFontPanel);
    Add(0,generalBackColorLabel);
    Add(0,generalBackColorView);
    Add(0,generalBackColorBtn);
    Add(0,generalFontHeaderLabel);
    Add(0,generalFontHeaderSampleLabel);
    Add(0,generalFontHeaderBtn);
    Add(0,generalFontLabelLabel);
    Add(0,generalFontLabelSampleLabel);
    Add(0,generalFontLabelBtn);

    Add(0,gMiscPanel);
    Add(0,generalLegendLabel);
    Add(0,generalLegendText);
    Add(0,generalDurationLabel);
    Add(0,generalDurationText);

    gLegendPanel->SetBounds(5,10,290,50);
	SetCompBoundsRelativeTo(gLegendPanel,generalLabelVisibleCheck,5, 20, 80, 19);
	SetCompBoundsRelativeTo(gLegendPanel,generalLabelPLabel,120, 20, 65, 19);
	SetCompBoundsRelativeTo(gLegendPanel,generalLabelPCombo,190, 20, 95, 19);

    gColorFontPanel->SetBounds(5,70,290,100);
	SetCompBoundsRelativeTo(gColorFontPanel,generalBackColorLabel,10, 20, 140, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalBackColorView,155, 20, 105, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalBackColorBtn,265, 20, 20, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalFontHeaderLabel,10, 45, 90, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalFontHeaderSampleLabel,105, 45, 145, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalFontHeaderBtn,265, 45, 20, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalFontLabelLabel,10, 70, 90, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalFontLabelSampleLabel,105, 70, 145, 19);
	SetCompBoundsRelativeTo(gColorFontPanel,generalFontLabelBtn,265, 70, 20, 19);

    gGridPanel->SetBounds(5,178,290,50);
	SetCompBoundsRelativeTo(gGridPanel,generalGridCombo,10, 20, 120, 19);
	SetCompBoundsRelativeTo(gGridPanel,generalGridStyleLabel,135, 20, 45, 19);
	SetCompBoundsRelativeTo(gGridPanel,generalGridStyleCombo,185, 20, 100, 19);

    gMiscPanel->SetBounds(5,238,290,75);
	SetCompBoundsRelativeTo(gMiscPanel,generalLegendLabel,10, 20, 70, 19);
	SetCompBoundsRelativeTo(gMiscPanel,generalLegendText,85, 20, 200, 19);
	SetCompBoundsRelativeTo(gMiscPanel,generalDurationLabel,10, 45, 120, 19);
	SetCompBoundsRelativeTo(gMiscPanel,generalDurationText,135, 45, 150, 19);

    // **********************************************
    // Axis panel construction
    // **********************************************
    xPanel  = new AxisPanel(chart->GetXAxis() ,X_TYPE ,chart);
    xPanel->AddToPanel(this,1);
    y1Panel = new AxisPanel(chart->GetY1Axis(),Y1_TYPE,chart);
    y1Panel->AddToPanel(this,2);
    y2Panel = new AxisPanel(chart->GetY2Axis(),Y2_TYPE,chart);
    y2Panel->AddToPanel(this,3);

    closeBtn = new GLButton(0,"Close");
    closeBtn->SetBounds(215, 320, 80, 19);
    GLWindow::Add(closeBtn);

    UpdateBar();
    SetTextColor(120,120,120);

    // Center dialog
    int wD = 300;
    int hD = 385;
    int wS,hS;
    GLToolkit::GetScreenSize(&wS,&hS);
    int xD = (wS-wD)/2;
    int yD = (hS-hD)/2;
    SetBounds(xD,yD,wD,hD);

    RestoreDeviceObjects();

  }

  GLChartOptions::~GLChartOptions() {
    delete y1Panel;
    delete y2Panel;
    delete xPanel;
  }

  void GLChartOptions::commit() {
    GLWindowManager::FullRepaint();
  }

  // Mouse Listener
  void GLChartOptions::ProcessMessage(GLComponent *src,int message) {

    char tmp[128];

    
    if (src == closeBtn) {
      SetVisible(false);
    } else if (src == generalBackColorBtn) {

      GLColor c = chart->GetChartBackground();
      if( GLColorBox::Display("Choose background",&c.r,&c.g,&c.b) ) {
        chart->SetChartBackground(c);
        generalBackColorView->SetBackgroundColor(c.r,c.g,c.b);
        commit();
      }

    } else if (src == generalFontHeaderBtn) {
      // TODO
      /*
      Font f = ATKFontChooser->GetNewFont(this,"Choose Header Font", chart->GetHeaderFont());
      if (f != null) {
        chart->SetHeaderFont(f);
        generalFontHeaderSampleLabel->SetFont(f);
        commit();
      }
      */
    } else if (src == generalFontLabelBtn) {
      // TODO
      /*
      Font f = ATKFontChooser->GetNewFont(this,"Choose label Font", chart->GetXAxis()->GetFont());
      if (f != null) {
        chart->GetXAxis()->SetFont(f);
        chart->GetY1Axis()->SetFont(f);
        chart->GetY2Axis()->SetFont(f);
        chart->SetLabelFont(f);
        generalFontLabelSampleLabel->SetFont(f);
        Commit();
      }
      */
    } else if (src == generalLabelVisibleCheck) {

      chart->SetLabelVisible(generalLabelVisibleCheck->GetState());
      commit();

      
    } else if (src == generalGridCombo) {

      int sel = generalGridCombo->GetSelectedIndex();

      switch (sel) {
        case 1: // On X
          chart->GetXAxis()->SetGridVisible(true);
          chart->GetY1Axis()->SetGridVisible(false);
          chart->GetY2Axis()->SetGridVisible(false);
          break;
        case 2: // On Y1
          chart->GetXAxis()->SetGridVisible(false);
          chart->GetY1Axis()->SetGridVisible(true);
          chart->GetY2Axis()->SetGridVisible(false);
          break;
        case 3: // On Y2
          chart->GetXAxis()->SetGridVisible(false);
          chart->GetY1Axis()->SetGridVisible(false);
          chart->GetY2Axis()->SetGridVisible(true);
          break;
        case 4: // On X,Y1
          chart->GetXAxis()->SetGridVisible(true);
          chart->GetY1Axis()->SetGridVisible(true);
          chart->GetY2Axis()->SetGridVisible(false);
          break;
        case 5: // On X,Y2
          chart->GetXAxis()->SetGridVisible(true);
          chart->GetY1Axis()->SetGridVisible(false);
          chart->GetY2Axis()->SetGridVisible(true);
          break;
        default: // None
          chart->GetXAxis()->SetGridVisible(false);
          chart->GetY1Axis()->SetGridVisible(false);
          chart->GetY2Axis()->SetGridVisible(false);
          break;
      }
      commit();

      
    } else if (src == generalGridStyleCombo) {

      int s = generalGridStyleCombo->GetSelectedIndex();
      chart->GetXAxis()->SetGridStyle(s);
      chart->GetY1Axis()->SetGridStyle(s);
      chart->GetY2Axis()->SetGridStyle(s);
      commit();

      
    } else if (src == generalLabelPCombo) {

      int s = generalLabelPCombo->GetSelectedIndex();
      chart->SetLabelPlacement(s);
      commit();

    // General ------------------------------------------------------------
    } else if (src == generalLegendText) {

      chart->SetHeader(generalLegendText->GetText().c_str());
      commit();

    } else if (src == generalDurationText) {

      if (iequals(generalDurationText->GetText().c_str(),"infinty")) {
        chart->SetDisplayDuration(MAX_VALUE);
        return;
      }
      double d;
      if( !generalDurationText->GetNumber(&d)) {
        error("Display duration: malformed number.");
        d = chart->GetDisplayDuration();
        if(d==MAX_VALUE) sprintf(tmp,"Infinity");
        else             sprintf(tmp,"%g",d);
        generalDurationText->SetText(tmp);
        return;
      }
      chart->SetDisplayDuration(d);
      commit();

    } else {

      xPanel->ProcessMessage(src,message);
      y1Panel->ProcessMessage(src,message);
      y2Panel->ProcessMessage(src,message);

    }

    GLTabWindow::ProcessMessage(src,message);
  }

  // Error message
  void GLChartOptions::error(const char *m) {
    GLMessageBox::Display(m,"Chart options",GLDLG_OK,GLDLG_ICONERROR);
  }
