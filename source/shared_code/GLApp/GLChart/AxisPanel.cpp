// Copyright (c) 2011 rubicon IT GmbH
#include "../GLChart/GLChart.h"
#include "AxisPanel.h"
#include "../GLMessageBox.h"
#include "../GLColorBox.h"
//#include "../GLToolkit.h"
#include "../GLToggle.h"
#include "../GLCombo.h"
#include "../GLButton.h"
#include "../GLWindowManager.h"
#include "../GLTitledPanel.h"
#include "../GLLabel.h"
#include "../GLTextField.h"
#include "../GLTypes.h" //MSG_TEXT

static int textCR = 0;
static int textCG = 0;
static int textCB = 0;

/**
 * Construct an axis setting panel.
 * @param a Axis
 * @param axisType Type of axe
 * @param parentChart parent chart (can be null)
 */
AxisPanel::AxisPanel(GLAxis *a,int axisType,GLChart *parentChart)  {

    char tmp[256];

    pAxis  = a;
    pChart = parentChart;
    type   = axisType;

    scalePanel = new GLTitledPanel("Scale");
    scalePanel->SetBold(true);
    settingPanel = new GLTitledPanel("Axis settings");
    settingPanel->SetBold(true);

    MinLabel = new GLLabel("Min");
    MinText = new GLTextField(0,"");
    MinLabel->SetTextColor(textCR,textCG,textCB);
    MinLabel->SetEnabled(!a->IsAutoScale());
    sprintf(tmp,"%g",a->GetMinimum());
    MinText->SetText(tmp);
    MinText->SetEditable(true);
    MinText->SetEnabled(!a->IsAutoScale());

    MaxLabel = new GLLabel("Max");
    MaxText = new GLTextField(0,"");
    MaxLabel->SetTextColor(textCR,textCG,textCB);
    MaxLabel->SetEnabled(!a->IsAutoScale());
    sprintf(tmp,"%g",a->GetMaximum());
    MaxText->SetText(tmp);
    MaxText->SetEditable(true);
    MaxText->SetEnabled(!a->IsAutoScale());

    AutoScaleCheck = new GLToggle(0,"Auto scale");
    AutoScaleCheck->SetTextColor(textCR,textCG,textCB);
    AutoScaleCheck->SetState(a->IsAutoScale());

    ScaleLabel = new GLLabel("Mode");
    ScaleLabel->SetTextColor(textCR,textCG,textCB);
    ScaleCombo = new GLCombo(0);
    ScaleCombo->SetSize(2);
    ScaleCombo->SetValueAt(0,"Linear");
    ScaleCombo->SetValueAt(1,"Logarithmic");
    ScaleCombo->SetSelectedIndex(a->GetScale());

    SubGridCheck = new GLToggle(0,"Show sub grid");
    SubGridCheck->SetTextColor(textCR,textCG,textCB);
    SubGridCheck->SetState(a->IsSubGridVisible());

    VisibleCheck = new GLToggle(0,"Visible");
    VisibleCheck->SetTextColor(textCR,textCG,textCB);
    VisibleCheck->SetState(a->IsVisible());

    OppositeCheck = new GLToggle(0,"Draw opposite");
    OppositeCheck->SetTextColor(textCR,textCG,textCB);
    OppositeCheck->SetState(a->IsDrawOpposite());

    FormatCombo = new GLCombo(0);
    FormatCombo->SetSize(8);
    FormatCombo->SetValueAt(0,"Automatic");
    FormatCombo->SetValueAt(1,"Scientific");
    FormatCombo->SetValueAt(2,"Time (hh:mm:ss)");
    FormatCombo->SetValueAt(3,"Decimal int");
    FormatCombo->SetValueAt(4,"Hexadecimal int");
    FormatCombo->SetValueAt(5,"Binary int");
    FormatCombo->SetValueAt(6,"Scientific int");
    FormatCombo->SetValueAt(7,"Date");
    FormatCombo->SetSelectedIndex(a->GetLabelFormat());

    FormatLabel = new GLLabel("Label format");
    FormatLabel->SetTextColor(textCR,textCG,textCB);

    TitleLabel = new GLLabel("Title");
    TitleLabel->SetTextColor(textCR,textCG,textCB);
    TitleText = new GLTextField(0,"");
    TitleText->SetEditable(true);
    TitleText->SetText(a->GetName());

    ColorLabel = new GLLabel("Color");
    ColorLabel->SetTextColor(textCR,textCG,textCB);
    ColorView = new GLLabel("");
    ColorView->SetOpaque(true);
    ColorView->SetBorder(BORDER_ETCHED);
    GLColor aColor = a->GetAxisColor();
    ColorView->SetBackgroundColor(aColor.r,aColor.g,aColor.b);
    ColorBtn = new GLButton(0,"...");

    PositionLabel = new GLLabel("Position");
    PositionLabel->SetTextColor(textCR,textCG,textCB);
    PositionCombo = new GLCombo(0);
    switch(type) {

      case X_TYPE:
        PositionCombo->SetSize(4);
        PositionCombo->SetValueAt(0,"Down");
        PositionCombo->SetValueAt(1,"Up");
        PositionCombo->SetValueAt(2,"Y1 Origin");
        PositionCombo->SetValueAt(3,"Y2 Origin");
        PositionCombo->SetSelectedIndex(a->GetPosition()-1);
        break;

      case Y1_TYPE:
        PositionCombo->SetSize(2);
        PositionCombo->SetValueAt(0,"Left");
        PositionCombo->SetValueAt(1,"X Origin");
        PositionCombo->SetSelectedIndex((a->GetPosition() == VERTICAL_ORG) ? 1 : 0);
        break;

      case Y2_TYPE:
        PositionCombo->SetSize(2);
        PositionCombo->SetValueAt(0,"Right");
        PositionCombo->SetValueAt(1,"X Origin");
        PositionCombo->SetSelectedIndex((a->GetPosition() == VERTICAL_ORG) ? 1 : 0);
        break;

    }
}

void AxisPanel::AddToPanel(GLTabWindow *parent,int pIdx) {

    parent->Add(pIdx,scalePanel);
    parent->Add(pIdx,MinLabel);
    parent->Add(pIdx,MinText);
    parent->Add(pIdx,MaxLabel);
    parent->Add(pIdx,MaxText);
    parent->Add(pIdx,AutoScaleCheck);
    parent->Add(pIdx,ScaleLabel);
    parent->Add(pIdx,ScaleCombo);

    parent->Add(pIdx,settingPanel);
    parent->Add(pIdx,SubGridCheck);
    parent->Add(pIdx,OppositeCheck);
    parent->Add(pIdx,VisibleCheck);
    parent->Add(pIdx,FormatCombo);
    parent->Add(pIdx,FormatLabel);
    parent->Add(pIdx,TitleLabel);
    parent->Add(pIdx,TitleText);
    parent->Add(pIdx,ColorLabel);
    parent->Add(pIdx,ColorView);
    parent->Add(pIdx,ColorBtn);
    parent->Add(pIdx,PositionLabel);
    parent->Add(pIdx,PositionCombo);

    scalePanel->SetBounds(5,10,290,100);
	scalePanel->SetCompBounds(MinLabel,10, 20, 35, 19);
	scalePanel->SetCompBounds(MinText,50, 20, 90, 19);
	scalePanel->SetCompBounds(MaxLabel,145, 20, 40, 19);
	scalePanel->SetCompBounds(MaxText,190, 20, 90, 19);
	scalePanel->SetCompBounds(ScaleLabel,10, 45, 100, 19);
	scalePanel->SetCompBounds(ScaleCombo,115, 45, 165, 19);
	scalePanel->SetCompBounds(AutoScaleCheck,7, 70, 275, 19);

    settingPanel->SetBounds(5,120,290,170);
	settingPanel->SetCompBounds(FormatLabel,10, 20, 100, 19);
	settingPanel->SetCompBounds(FormatCombo,115, 20, 165, 19);
	settingPanel->SetCompBounds(TitleLabel,10, 45, 100, 19);
	settingPanel->SetCompBounds(TitleText,115, 45, 165, 19);
	settingPanel->SetCompBounds(ColorLabel,10, 70, 100, 19);
	settingPanel->SetCompBounds(ColorView,115, 70, 140, 19);
	settingPanel->SetCompBounds(ColorBtn,260, 70, 20, 19);
	settingPanel->SetCompBounds(PositionLabel,10, 95, 100, 19);
	settingPanel->SetCompBounds(PositionCombo,115, 95, 165, 19);
	settingPanel->SetCompBounds(SubGridCheck,7, 120, 130, 19);
	settingPanel->SetCompBounds(OppositeCheck,120, 120, 130, 19);
	settingPanel->SetCompBounds(VisibleCheck,7, 145, 280, 19);

}

void AxisPanel::ProcessMessage(GLComponent *src,int message) {

    if (src == AutoScaleCheck) {

      bool b = AutoScaleCheck->GetState();

      pAxis->SetAutoScale(b);

      if (!b) {

        double min,max;
        MinText->GetNumber(&min);
        MaxText->GetNumber(&max);
        if (max > min) {
           pAxis->SetMinimum(min);
           pAxis->SetMaximum(max);
        }

      }

      MinLabel->SetEnabled(!b);
      MinText->SetEnabled(!b);
      MaxLabel->SetEnabled(!b);
      MaxText->SetEnabled(!b);

      commit();

      
    } else if (src == FormatCombo) {

      int s = FormatCombo->GetSelectedIndex();
      pAxis->SetLabelFormat(s);
      commit();

      
    } else if (src == PositionCombo) {
      int s = PositionCombo->GetSelectedIndex();
      switch(type) {

        case X_TYPE:
          pAxis->SetPosition(s+1);
          break;

        case Y1_TYPE:
          switch(s) {
            case 0:
              pAxis->SetPosition(VERTICAL_LEFT);
              break;
            case 1:
              pAxis->SetPosition(VERTICAL_ORG);
              break;
          }
          break;

        case Y2_TYPE:
          switch (s) {
            case 0:
              pAxis->SetPosition(VERTICAL_RIGHT);
              break;
            case 1:
              pAxis->SetPosition(VERTICAL_ORG);
              break;
          }
          break;

      }
      commit();

    } else if (src == ScaleCombo) {

      int s = ScaleCombo->GetSelectedIndex();
      pAxis->SetScale(s);
      commit();

      
    } else if (src == SubGridCheck) {

      pAxis->SetSubGridVisible(SubGridCheck->GetState());
      commit();

    } else if (src == OppositeCheck) {

      pAxis->SetDrawOpposite(OppositeCheck->GetState());
      commit();

    } else if (src == VisibleCheck) {

      pAxis->SetVisible(VisibleCheck->GetState());
      commit();

    } else if (src == ColorBtn) {

      GLColor c = pAxis->GetAxisColor();
      if( GLColorBox::Display("Choose axis color",&c.r,&c.g,&c.b) ) {
        pAxis->SetAxisColor(c);
        ColorView->SetBackgroundColor(c.r,c.g,c.b);
        commit();
      }

    } else if ((src == MinText || src == MaxText) && !pAxis->IsAutoScale() && message==MSG_TEXT ) {

      double min,max;
      if( !MinText->GetNumber(&min) ) {
        error("Min: malformed number.");
        return;
      }
      if(!MaxText->GetNumber(&max)) {
        error("Max: malformed number.");
        return;
      }

      if (max <= min) {
        error("Min must be strictly lower than max.");
        return;
      }

      if (pAxis->GetScale() == LOG_SCALE) {
        if (min <= 0 || max <= 0) {
          error("Min and max must be strictly positive with logarithmic scale.");
          return;
        }
      }

      pAxis->SetMinimum(min);
      pAxis->SetMaximum(max);
      commit();

    } else if (src == TitleText) {

      pAxis->SetName(TitleText->GetText().c_str());
      commit();

    }

}

void AxisPanel::commit() {
  GLWindowManager::FullRepaint();
}

void AxisPanel::error(const char *m) {
  GLMessageBox::Display(m,"Chart options",GLDLG_OK,GLDLG_ICONERROR);
}
