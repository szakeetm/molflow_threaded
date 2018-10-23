// Copyright (c) 2011 rubicon IT GmbH
#include "GLChart.h"
#include "../GLMenu.h"
#include "../GLToolkit.h"
#include "../../File.h"
//#include "..\GLFileBox.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "../GLMessageBox.h"
#include "../GLButton.h"

//#include <malloc.h>
#include <math.h>
#include <sstream>
#include <cstring> //strcpy, etc.

double NaN;

GLChart::GLChart(int compId):GLComponent(compId) {

	// Default
	GLColor defColor(240,240,240);
	SetBackgroundColor(240,240,240);
	SetChartBackground(defColor);
	SetOpaque(true);
	headerColor.r = 0;
	headerColor.g = 0;
	headerColor.b = 0;
	strcpy(header,"");
	headerVisible = false;
	labelVisible = true;
	labelMode = LABEL_ROW;
	ipanelVisible = false;
	paintAxisFirst = true;
	margin.width = 5;
	margin.height = 5;
	memset(&headerR,0,sizeof(GLCRectangle));
	memset(&viewR,0,sizeof(GLCRectangle));
	memset(&labelR,0,sizeof(GLCRectangle));

	xAxis = new GLAxis(this, HORIZONTAL_DOWN);
	xAxis->SetAnnotation(TIME_ANNO);
	xAxis->SetAutoScale(true);
	y1Axis = new GLAxis(this, VERTICAL_LEFT);
	y2Axis = new GLAxis(this, VERTICAL_RIGHT);
	displayDuration = 3600;

	nbLabel = 0;
	zoomDrag = false;
	zoomDragAllowed = false;

	chartMenu = new GLMenu();
	chartMenu->Add("Chart properties",MENU_CHARTPROP);
	chartMenu->Add(NULL);
	chartMenu->Add("Dataview Y1");
	chartMenu->Add("Dataview Y2");
	chartMenu->Add(NULL);
	chartMenu->Add("Copy to Clipboard",MENU_COPYALL);
	chartMenu->Add("Save file (csv,txt)",MENU_SAVETXT);
	//chartMenu->Add("Load file (TXT)",MENU_LOADTXT);
	dvMenuY1 = chartMenu->GetSubMenu("Dataview Y1");
	dvMenuY2 = chartMenu->GetSubMenu("Dataview Y2");

	zoomButton = new GLButton(0,"Zoom back");
	zoomButton->SetVisible(false);
	Add(zoomButton);

	chartOptions = NULL;
	dvOptions = NULL;

	// Load NaN constant
	size_t iNaN = 0x7ff0000bad0000ffL;
	memcpy(&NaN,&iNaN,8);
	//bool test = isnan(NaN);
}

GLChart::~GLChart() {
	SAFE_DELETE(xAxis);
	SAFE_DELETE(y1Axis);
	SAFE_DELETE(y2Axis);
	SAFE_DELETE(chartOptions);
	SAFE_DELETE(dvOptions);
}

/**
* Return a handle to the x axis
* @return Axis handle
*/
GLAxis *GLChart::GetXAxis() {
	return xAxis;
}

/**
* Return a handle to the left y axis
* @return Axis handle
*/
GLAxis *GLChart::GetY1Axis() {
	return y1Axis;
}

/**
* Return a handle to the right y axis
* @return Axis handle
*/
GLAxis *GLChart::GetY2Axis() {
	return y2Axis;
}

/**
* Sets component margin
* @param d Margin
* @see getMargin
*/
void GLChart::SetMargin(GLCDimension d) {
	margin  = d;
}

/**
* Gets the current margin
* @return Margin
* @see setMargin
*/
GLCDimension GLChart::GetMargin() {
	return margin;
}

void GLChart::SetBackground(GLColor c) {
	SetBackgroundColor(c.r,c.g,c.b);
}

GLColor GLChart::GetBackground() {
	GLColor c;
	GetBackgroundColor(&(c.r),&(c.g),&(c.b));
	return c;
}

/**
* Sets the chart background (curve area)
* @param c Background color
*/
void GLChart::SetChartBackground(GLColor c) {
	chartBackground = c;
}

/**
*
* Gets the chart background (curve area)
* @return Background color
*/
GLColor GLChart::GetChartBackground() {
	return chartBackground;
}

/**
* Paints axis under curve when true
* @param b Painting order
*/
void GLChart::SetPaintAxisFirst(bool b) {
	paintAxisFirst = b;
}

/**
* Return painting order between axis and curve
* @return true if axis are painted under curve
*/
bool GLChart::IsPaintAxisFirst() {
	return paintAxisFirst;
}

/**
* Displays or hides header.
* @param b true if the header is visible, false otherwise
* @see setHeader
*/
void GLChart::SetHeaderVisible(bool b) {
	headerVisible = b;
}

/**
* Sets the header and displays it.
* @param s Graph header
* @see getHeader
*/
void GLChart::SetHeader(const char *s) {
	if(s) strcpy(header,s);
	else  strcpy(header,"");
	SetHeaderVisible(strlen(header)>0);
}

/**
* Gets the current header
* @return Graph header
* @see setHeader
*/
char *GLChart::GetHeader() {
	return header;
}

/**
* Sets the display duration.This will garbage old data in all displayed data views.
* Garbaging occurs when addData is called.
* @param v Displauy duration (millisec). Pass Double.POSITIVE_INFINITY to disable.
* @see addData
*/
void GLChart::SetDisplayDuration(double v) {
	displayDuration = v;
	GetXAxis()->SetAxisDuration(v);
}

/**
* Gets the display duration.
* @return Display duration
* @see setDisplayDuration
*/
double GLChart::GetDisplayDuration() {
	return displayDuration;
}

/**
* Sets the header color
* @param c Header color
*/
void GLChart::SetHeaderColor(GLColor c) {
	headerColor = c;
	SetHeaderVisible(true);
}

/**
* Displays or hide labels.
* @param b true if labels are visible, false otherwise
* @see isLabelVisible
*/
void GLChart::SetLabelVisible(bool b) {
	labelVisible = b;
}

/**
* Determines wether labels are visivle or not.
* @return true if labels are visible, false otherwise
*/
bool GLChart::IsLabelVisible() {
	return labelVisible;
}

/**
* Set the label placement.
* @param p Placement
* @see LABEL_UP
* @see LABEL_DOWN
* @see LABEL_ROW
* @see LABEL_LEFT
* @see LABEL_RIGHT
*/
void GLChart::SetLabelPlacement(int p) {
	labelMode = p;
}

/**
* Returns the current label placement.
* @return Label placement
* @see setLabelPlacement
*/
int GLChart::GetLabelPlacement() {
	return labelMode;
}

/**
* Display the global graph option dialog.
*/
void GLChart::ShowOptionDialog() {

	if( chartOptions==NULL ) chartOptions = new GLChartOptions(this);
	chartOptions->DoModal();

}

/**
* Display the data view option dialog.
*/
void GLChart::ShowDataOptionDialog(GLDataView *v) {

	if( dvOptions==NULL ) dvOptions = new GLDataViewOptions(this);
	dvOptions->SetDataView(v);
	dvOptions->DoModal();

}

/**
* Determines wether the graph is zoomed.
* @return true if the , false otherwise
*/
bool GLChart::IsZoomed() {
	return xAxis->IsZoomed() || y1Axis->IsZoomed() || y2Axis->IsZoomed();
}

/**
* Enter zoom mode. This happens when you hold the left mouse button down
* and drag the mouse.
*/
void GLChart::EnterZoom() {
	if (!zoomDragAllowed) {
		zoomDragAllowed = true;
	}
}

/**
* Exit zoom mode.
*/
void GLChart::ExitZoom() {
	xAxis->Unzoom();
	y1Axis->Unzoom();
	y2Axis->Unzoom();
	zoomButton->SetVisible(false);
	zoomDragAllowed = false;
}

/**
* Remove all dataview from the graph.
*/
void GLChart::UnselectAll() {
	GetY1Axis()->ClearDataView();
	GetY2Axis()->ClearDataView();
	GetXAxis()->ClearDataView();
}

// Message management

void GLChart::ProcessMessage(GLComponent *src,int message) {

	if( src==zoomButton ) {
		ExitZoom();
	}

}

void GLChart::InvalidateDeviceObjects() {
	IVALIDATE_DLG(chartOptions);
	IVALIDATE_DLG(dvOptions);
	GLAxis::Invalidate();
	GLComponent::InvalidateDeviceObjects();
}

void GLChart::RestoreDeviceObjects() {
	RVALIDATE_DLG(chartOptions);
	RVALIDATE_DLG(dvOptions);
	GLAxis::Revalidate();
	GLComponent::RestoreDeviceObjects();
}

void GLChart::paintLabel(GLDataView *v,GLAxis *axis,int x,int y,int w) {

	int xm   = x + (w - labelWidth) / 2 + 2;
	int ym   = y;
	GLAxis::DrawSampleLine(posX+xm,posY+ym + labelHeight/2 + 1, v);
	GLColor c = v->GetLabelColor();
	GLToolkit::GetDialogFont()->SetTextColor((float)c.r/255.0f,(float)c.g/255.0f,(float)c.b/255.0f);
	GLToolkit::GetDialogFont()->DrawText(posX+xm + 44,posY+ym + 2,v->GetExtendedName(),false);
	labelRect[nbLabel].rect.x = xm;
	labelRect[nbLabel].rect.y = ym;
	labelRect[nbLabel].rect.width  = labelWidth;
	labelRect[nbLabel].rect.height = labelHeight;
	labelRect[nbLabel].view = v;
	nbLabel++;

}

// paint Label and header
void GLChart::paintLabelAndHeader() {

	int nbv1 = y1Axis->GetViewNumber();
	int nbv2 = y2Axis->GetViewNumber();

	// Draw header
	if (headerR.width>0) {
		int xpos = ((headerR.width - headerWidth) / 2);
		GLColor c = headerColor;
		GLToolkit::GetDialogFontBold()->SetTextColor((float)c.r/255.0f,(float)c.g/255.0f,(float)c.b/255.0f);
		GLToolkit::GetDialogFontBold()->DrawText(posX+xpos,posY+headerR.y + 1,header,false);
	}

	// Draw labels
	nbLabel = 0;
	if (labelR.width>0) {

		GLDataView *v;
		int i,k = 0;
		int totLabel = 0;
		for (i = 0; i < nbv1; i++) {
			v = y1Axis->GetDataView(i);
			if (v->IsLabelVisible()) totLabel++;
		}
		for (i = 0; i < nbv2; i++) {
			v = y2Axis->GetDataView(i);
			if (v->IsLabelVisible()) totLabel++;
		}

		if( labelMode==LABEL_ROW && labelPerLine>1 ) {

			int rowWidth = labelR.width / labelPerLine;

			// Draw labels (in row/column)
			for (i = 0; i < nbv1; i++) {
				v = y1Axis->GetDataView(i);
				if (v->IsLabelVisible()) {
					int x = (k%labelPerLine) * rowWidth + labelR.x;
					int y = (k/labelPerLine) * labelHeight + labelR.y;
					paintLabel(v,y1Axis,x,y,rowWidth);
					k++;
				}
			}

			for (i = 0; i < nbv2; i++) {
				v = y2Axis->GetDataView(i);
				if (v->IsLabelVisible()) {
					int x = (k%labelPerLine) * rowWidth + labelR.x;
					int y = (k/labelPerLine) * labelHeight + labelR.y;
					paintLabel(v,y2Axis,x,y,rowWidth);
					k++;
				}
			}

		} else {

			// Draw labels (in column)
			for (i = 0; i < nbv1; i++) {
				v = y1Axis->GetDataView(i);
				if (v->IsLabelVisible()) {
					int y = labelR.y + (labelR.height-totLabel*labelHeight)/2  + labelHeight * k;
					paintLabel(v,y1Axis,labelR.x,y,labelR.width);
					k++;
				}
			}

			for (i = 0; i < nbv2; i++) {
				v = y2Axis->GetDataView(i);
				if (v->IsLabelVisible()) {
					int y = labelR.y + (labelR.height-totLabel*labelHeight)/2 + labelHeight * k;
					paintLabel(v,y2Axis,labelR.x,y,labelR.width);
					k++;
				}
			}

		}

	}

}

void GLChart::setRect(GLCRectangle *r,int x,int y,int w,int h) {
	r->x = x;
	r->y = y;
	r->width = w;
	r->height = h;
}

// Compute size of graph items (Axe,label,header,....
void GLChart::measureGraphItems(int w,int h,GLDataView **views,int nbView) {

	int i;
	int MX = margin.width;
	int MY = margin.height;
	int labelTHeight = 0; // Total label height

	// Reset sizes ------------------------------------------------------
	memset(&headerR,0,sizeof(GLCRectangle));
	headerR.height = 10;
	memset(&viewR,0,sizeof(GLCRectangle));
	memset(&labelR,0,sizeof(GLCRectangle));
	labelWidth = 0;
	headerWidth = 0;
	axisWidth = 0;
	axisHeight = 0;
	y1AxisThickness = 0;
	y2AxisThickness = 0;

	// Measure header ------------------------------------------------------
	if ( headerVisible && (strlen(header)>0) ) {
		headerWidth = GLToolkit::GetDialogFontBold()->GetTextWidth(header);
		int h = GLToolkit::GetDialogFontBold()->GetTextHeight();
		setRect(&headerR,MX , MY , w-2*MX , h + 10);
	}

	// Compute label number ------------------------------------------------------
	nbLabel=0;
	for (i = 0; i < y1Axis->GetViewNumber(); i++)
		if( y1Axis->GetDataView(i)->IsLabelVisible() )
			nbLabel++;
	for (i = 0; i < y2Axis->GetViewNumber(); i++)
		if( y2Axis->GetDataView(i)->IsLabelVisible() )
			nbLabel++;

	// Measure labels ------------------------------------------------------
	if (labelVisible && (nbLabel>0)) {

		GLDataView *v;
		i = 0;

		double maxLength = 0;
		for (i = 0; i < y1Axis->GetViewNumber(); i++) {
			v = y1Axis->GetDataView(i);
			if (v->IsLabelVisible()) {
				int w = GLToolkit::GetDialogFont()->GetTextWidth(v->GetExtendedName());
				if (w > maxLength) maxLength = w;
			}
		}
		for (i = 0; i < y2Axis->GetViewNumber(); i++) {
			v = y2Axis->GetDataView(i);
			if (v->IsLabelVisible()) {
				int w = GLToolkit::GetDialogFont()->GetTextWidth(v->GetExtendedName());
				if (w > maxLength) maxLength = w;
			}
		}

		labelHeight = GLToolkit::GetDialogFont()->GetTextHeight() + 2;
		labelTHeight = (labelHeight * nbLabel) + 10;
		labelWidth = (int) (maxLength + 55); // sample line width & margin

		switch( labelMode ) {
		case LABEL_UP:
			setRect(&labelR,MX ,MY + headerR.height ,w-2*MX ,labelTHeight);
			break;
		case LABEL_DOWN:
			setRect(&labelR,MX ,h-MY-labelTHeight, w-2*MX, labelTHeight);
			break;
		case LABEL_RIGHT:
			setRect(&labelR,w-MX-labelWidth+2, MY + headerR.height, labelWidth, h-2*MY-headerR.height);
			break;
		case LABEL_LEFT:
			setRect(&labelR,MX, MY + headerR.height, labelWidth, h - 2 * MY - headerR.height);
			break;
		case LABEL_ROW:
			labelPerLine = w/labelWidth;
			if(labelPerLine>nbLabel) labelPerLine = nbLabel;
			if(labelPerLine<=1) {
				// Revert to classic LABEL_DOWN
				setRect(&labelR,MX ,h-MY-labelTHeight, w-2*MX, labelTHeight);
			} else {
				labelTHeight = labelHeight*(nbLabel/labelPerLine);
				if(nbLabel%labelPerLine!=0) labelTHeight += labelHeight;
				setRect(&labelR,MX ,h-MY-labelTHeight, w-2*MX, labelTHeight);
			}
			break;
		}

	}

	// Measure view Rectangle --------------------------------------------
	switch (labelMode) {
	case LABEL_UP:
		setRect(&viewR,MX, MY + headerR.height + labelR.height , w - 2 * MX, h - 2*MY - headerR.height - labelR.height);
		break;
	case LABEL_DOWN:
	case LABEL_ROW:
		setRect(&viewR,MX, MY + headerR.height , w - 2 * MX, h - 2 * MY - headerR.height - labelR.height);
		break;
	case LABEL_RIGHT:
		setRect(&viewR,MX, MY + headerR.height , w - 2 * MX - labelR.width , h - 2 * MY - headerR.height);
		break;
	case LABEL_LEFT:
		setRect(&viewR,MX + labelR.width, MY + headerR.height, w - 2 * MX - labelR.width, h - 2 * MY - headerR.height);
		break;
	}

	// Measure Axis ------------------------------------------------------
	xAxisThickness =  xAxis->GetLabelFontDimension();
	if(xAxis->GetOrientation()==HORIZONTAL_UP) {
		xAxisUpMargin = xAxisThickness/2;
	} else {
		xAxisUpMargin = 0;
	}

	axisHeight = viewR.height - xAxisThickness;

	xAxis->ComputeXScale(views,nbView);
	y1Axis->MeasureAxis(0, axisHeight);
	y2Axis->MeasureAxis(0, axisHeight);
	y1AxisThickness = y1Axis->GetThickness();
	if(y1AxisThickness==0) y1AxisThickness = 5;
	y2AxisThickness = y2Axis->GetThickness();
	if(y2AxisThickness==0) y2AxisThickness = 5;

	axisWidth = viewR.width - (y1AxisThickness+y2AxisThickness);

	xAxis->MeasureAxis(axisWidth, 0);

}

// Paint the zoom mode label
void GLChart::paintZoomButton(int x,int y) {

	if( IsZoomed() ) {
		zoomButton->SetBounds(posX+x+7,posY+y+5,80,19);
		zoomButton->SetVisible(80<(axisWidth-7) && 19<(axisHeight-5));
	} else {
		zoomButton->SetVisible(false);
	}

	if( zoomButton->IsVisible() )
		PaintComponents();

}

// Paint the zoom rectangle
void GLChart::paintZoomSelection(int x,int y) {

	if (zoomDrag) {
		GLCRectangle r = buildRect(zoomX, zoomY, lastX, lastY);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glLineWidth(1.0f);
		glColor3f(0.0f,0.0f,0.0f);
		glBegin(GL_LINE_STRIP);
		_glVertex2i(r.x+x        ,r.y+y);
		_glVertex2i(r.x+r.width+x,r.y+y);
		_glVertex2i(r.x+r.width+x,r.y+r.height+y);
		_glVertex2i(r.x+x        ,r.y+r.height+y);
		_glVertex2i(r.x+x        ,r.y+y);
		glEnd();
	}

}

void GLChart::paintChartBackground(int xOrg,int yOrg) {

	int rb;
	int gb;
	int bb;
	GetBackgroundColor(&rb,&gb,&bb);
	if( ((chartBackground.r != rb) || (chartBackground.g != gb) || (chartBackground.b != bb))
		&& axisWidth > 0 && axisHeight > 0 ) {
			GLToolkit::DrawBox(xOrg, yOrg - axisHeight, axisWidth, axisHeight,chartBackground.r,chartBackground.g,chartBackground.b);
	}

}

/**
* Paint the components. Use the repaint method to repaint the graph.
* @param g Graphics object.
*/
void GLChart::Paint() {

	if(!parent) return;
	GLComponent::Paint();

	// Create a vector containing all views
	int nbv1 = y1Axis->GetViewNumber();
	int nbv2 = y2Axis->GetViewNumber();
	int nbView = nbv1 + nbv2;

	GLDataView *views[MAX_VIEWS];
	for(int i=0;i<nbv1;i++) views[i]      = y1Axis->GetDataView(i);
	for(int i=0;i<nbv2;i++) views[nbv1+i] = y2Axis->GetDataView(i);

	// Compute bounds of label and graph
	measureGraphItems(width,height,views,nbView);

	// Draw label and header
	paintLabelAndHeader();

	// Paint chart background
	int xOrg = viewR.x + y1AxisThickness;
	int yOrg = viewR.y + axisHeight + xAxisUpMargin;

	int xOrgY1 = viewR.x;
	int xOrgY2 = viewR.x + y1AxisThickness + axisWidth;
	int yOrgY  = viewR.y + xAxisUpMargin;

	paintChartBackground(posX+xOrg,posY+yOrg);

	// Paint zoom stuff
	paintZoomSelection(posX,posY);

	if (paintAxisFirst) {

		//Draw axes
		y1Axis->PaintAxis(posX+xOrgY1, posY+yOrgY, xAxis, posX+xOrg, posY+yOrg, GetBackground(),!y2Axis->IsVisible() || nbv2==0);
		y2Axis->PaintAxis(posX+xOrgY2, posY+yOrgY, xAxis, posX+xOrg, posY+yOrg, GetBackground(),!y1Axis->IsVisible() || nbv1==0);
		if( xAxis->GetPosition()==HORIZONTAL_ORG2)
			xAxis->PaintAxis(posX+xOrg, posY+yOrg, y2Axis, 0, 0, GetBackground(),true);
		else
			xAxis->PaintAxis(posX+xOrg, posY+yOrg, y1Axis, 0, 0, GetBackground(),true);

		//Draw data
		y1Axis->PaintDataViews(xAxis, xOrg, yOrg);
		y2Axis->PaintDataViews(xAxis, xOrg, yOrg);
		GetWindow()->ClipToWindow();

	} else {

		//Draw data
		y1Axis->PaintDataViews(xAxis, xOrg, yOrg);
		y2Axis->PaintDataViews(xAxis, xOrg, yOrg);
		GetWindow()->ClipToWindow();

		//Draw axes
		y1Axis->PaintAxis(posX+xOrgY1, posY+yOrgY, xAxis, posX+xOrg, posY+yOrg, GetBackground(),!y2Axis->IsVisible() || nbv2==0);
		y2Axis->PaintAxis(posX+xOrgY2, posY+yOrgY, xAxis, posX+xOrg, posY+yOrg, GetBackground(),!y1Axis->IsVisible() || nbv1==0);
		if (xAxis->GetPosition() == HORIZONTAL_ORG2)
			xAxis->PaintAxis(posX+xOrg, posY+yOrg, y2Axis, 0, 0, GetBackground(),true);
		else
			xAxis->PaintAxis(posX+xOrg, posY+yOrg, y1Axis, 0, 0, GetBackground(),true);

	}

	redrawPanel();
	paintZoomButton(xOrg,yOrgY);

}

// Build a valid rectangle with the given coordinates
GLCRectangle GLChart::buildRect(int x1, int y1, int x2, int y2) {

	GLCRectangle r;

	if (x1 < x2) {
		if (y1 < y2) {
			setRect(&r,x1, y1, x2 - x1, y2 - y1);
		} else {
			setRect(&r,x1, y2, x2 - x1, y1 - y2);
		}
	} else {
		if (y1 < y2) {
			setRect(&r,x2, y1, x1 - x2, y2 - y1);
		} else {
			setRect(&r,x2, y2, x1 - x2, y1 - y2);
		}
	}

	return r;
}

// ************************************************************************
// Event management
void GLChart::ManageEvent(SDL_Event *evt) {

	GLContainer::ManageEvent(evt);
	GLContainer::RelayEvent(evt);

	if( !evtProcessed ) {
		switch(evt->type) {
		case SDL_MOUSEBUTTONUP:
			mouseReleased(evt);
			break;
		case SDL_MOUSEBUTTONDOWN:
			mousePressed(evt);
			break;
		case SDL_MOUSEBUTTONDBLCLICK:
			break;
		case SDL_MOUSEMOTION:
			mouseDragged(evt);
			break;
		}
	}

}

bool GLChart::insideRect(LabelRect *r,int x,int y) {
	return (x >= r->rect.x) && (x <= (r->rect.width + r->rect.x)) &&
		(y >= r->rect.y) && (y <= (r->rect.height + r->rect.y));
}

void GLChart::mouseDragged(SDL_Event *evt) {
	int ex = GetWindow()->GetX(this,evt);
	int ey = GetWindow()->GetY(this,evt);
	if (zoomDrag) {
		lastX = ex;
		lastY = ey;
	}
}

void GLChart::mouseReleased(SDL_Event *evt) {
	int ex = GetWindow()->GetX(this,evt);
	int ey = GetWindow()->GetY(this,evt);
	if (zoomDrag) {
		GLCRectangle r = buildRect(zoomX, zoomY, ex, ey);
		zoomDrag = false;
		xAxis->Zoom(r.x+posX, r.x+r.width+posX);
		y1Axis->Zoom(r.y+posY, r.y + r.height+posY);
		y2Axis->Zoom(r.y+posY, r.y + r.height+posY);
	}
	ipanelVisible = false;
}

void GLChart::showChartMenu(int x,int y) {

	int menuId = chartMenu->Track(GetWindow(), x, y);
	if( menuId>=0 ) {
		int nbV1 = y1Axis->GetViewNumber();
		int nbV2 = y2Axis->GetViewNumber();
		if( menuId == MENU_CHARTPROP ) {
			ShowOptionDialog();
		} else if( menuId == MENU_COPYALL ) {
			CopyAllToClipboard();
		} else if( menuId == MENU_SAVETXT ) {
			SaveFile();
		}/* else if( menuId == MENU_LOADTXT ) {
			LoadFile();
		}*/ else if( menuId>=MENU_DVPROPY1 && menuId<MENU_DVPROPY1+nbV1) { 
			ShowDataOptionDialog(y1Axis->GetDataView(menuId-MENU_DVPROPY1));
		} else if( menuId>=MENU_DVPROPY2 && menuId<MENU_DVPROPY2+nbV2) {
			ShowDataOptionDialog(y2Axis->GetDataView(menuId-MENU_DVPROPY2));
		}
	}

}

void GLChart::mousePressed(SDL_Event *evt) {

	int ex = GetWindow()->GetX(this,evt);
	int ey = GetWindow()->GetY(this,evt);

	// Left button click
	if (evt->button.button == SDL_BUTTON_LEFT) {

		// Zoom management
		if (GetWindow()->IsCtrlDown() || zoomDragAllowed) {
			zoomDrag = true;
			zoomX = ex;
			zoomY = ey;
			lastX = ex;
			lastY = ey;
			return;
		}

		// Look for the nearest value on each dataView
		SearchInfo *si;
		SearchInfo *msi = NULL;
		msi = y1Axis->SearchNearest(ex+posX, ey+posY, xAxis);
		si = y2Axis->SearchNearest(ex+posX, ey+posY, xAxis);
		if (si->found && si->dist < msi->dist) {
			delete msi;
			msi = si;
		}
		if (msi->found) showPanel(msi);
		delete msi;

		// Click on label
		int i = 0;
		bool found = false;
		while (i < nbLabel && !found) {
			LabelRect *r = labelRect + i;
			found = insideRect(r,ex,ey);
			if (found) {
				//Display the Dataview options
				ShowDataOptionDialog(r->view);
			}
			i++;
		}

	}

	// Right button click
	if (evt->button.button == SDL_BUTTON_RIGHT) {

		char tmp[64];

		dvMenuY1->Clear();
		for(int i=0;i<y1Axis->GetViewNumber();i++) {
			char *name = y1Axis->GetDataView(i)->GetName();
			if(strlen(name)>0) {
				dvMenuY1->Add(name,MENU_DVPROPY1+i);
			} else {
				sprintf(tmp,"Dataview #%d",i);
				dvMenuY1->Add(tmp,MENU_DVPROPY1+i);
			}
		}
		dvMenuY2->Clear();
		for(int i=0;i<y2Axis->GetViewNumber();i++) {
			char *name = y2Axis->GetDataView(i)->GetName();
			if(strlen(name)>0) {
				dvMenuY2->Add(name,MENU_DVPROPY2+i);
			} else {
				sprintf(tmp,"Dataview #%d",i);
				dvMenuY2->Add(tmp,MENU_DVPROPY2+i);
			}
		}

		showChartMenu(posX+ex, posY+ey);

	}

}

//****************************************
// redraw the panel
void GLChart::redrawPanel() {

	if (!ipanelVisible) return;

	// Udpate serachInfo
	GLCPoint p;
	GLDataView *vy = lastSearch.dataView;
	GLDataView *vx = lastSearch.xdataView;
	DataList *dy = lastSearch.value;
	DataList *dx = lastSearch.xvalue;
	GLAxis *yaxis = lastSearch.axis;

	if (xAxis->IsXY()) {
		p = yaxis->transform(vx->GetTransformedValue(dx->y),
			vy->GetTransformedValue(dy->y),
			xAxis);
	} else {
		p = yaxis->transform(dy->x,
			vy->GetTransformedValue(dy->y),
			xAxis);
	}

	lastSearch.x = p.x;
	lastSearch.y = p.y;

	showPanel(&lastSearch);
}

char **GLChart::buildPanelString(SearchInfo *si) {

	static char *str[4];
	char tmp[256];

	char *xValue;
	if (xAxis->GetAnnotation() == TIME_ANNO) {
		sprintf(tmp,"Time= %s",xAxis->FormatTimeValue(si->value->x));
		xValue = strdup(tmp);
	} else {
		sprintf(tmp,"X= %g",si->value->x);
		xValue = strdup(tmp);
	}
	if (xAxis->IsXY()) {
		sprintf(tmp,"%s (Y%d)",si->dataView->GetExtendedName(),(si->axis==y1Axis?1:2));
		str[0] = strdup(tmp);
		str[1] = xValue;
		sprintf(tmp,"X= %s",si->xdataView->FormatValue(si->xdataView->GetTransformedValue(si->xvalue->y)).c_str());
		str[2] = strdup(tmp);
		sprintf(tmp,"Y= %s %s",si->dataView->FormatValue(si->dataView->GetTransformedValue(si->value->y)).c_str(), si->dataView->GetUnit());
		str[3] = strdup(tmp);
	} else {
		sprintf(tmp,"%s (Y%d)",si->dataView->GetExtendedName(),(si->axis==y1Axis?1:2));
		str[0] = strdup(tmp);
		str[1] = xValue;
		sprintf(tmp,"Y= %s %s",si->dataView->FormatValue(si->dataView->GetTransformedValue(si->value->y)).c_str(), si->dataView->GetUnit());
		str[2] = strdup(tmp);
		str[3] = NULL;
	}

	return str;

}

// Save a tab separated field data file (TXT)

void GLChart::SaveFile() {

	char tmp[128];

	int nbv1 = y1Axis->GetViewNumber();
	int nbv2 = y2Axis->GetViewNumber();
	int nbView = nbv1 + nbv2;
	if(!nbView) return;

	std::string fn = NFD_SaveFile_Cpp("csv,txt","");
	if (!fn.empty()) {
		if (FileUtils::GetExtension(fn) == "") fn += ".csv";
		std::string separator;
		if (FileUtils::GetExtension(fn) == "csv") separator = ",";
		else separator = "\t";

		FILE *f = fopen(fn.c_str(),"w");
		if(f) {


			// Get DataList handle and write dataview name
			DataList *ptr[MAX_VIEWS];
			int j=0;
			fprintf(f,"X axis\t");
			for(int i=0;i<nbv1;i++) {
				ptr[j++] = y1Axis->GetDataView(i)->GetData();
				sprintf(tmp,"%s",y1Axis->GetDataView(i)->GetName());
				int l = (int)strlen(tmp)-1;
				if( l>=0 && tmp[l]>=128 ) tmp[l]=0;
				fprintf(f,"%s",tmp);
				if(j<nbView) fprintf(f, "%s",separator.c_str());
			}
			for(int i=0;i<nbv2;i++) {
				ptr[j++] = y2Axis->GetDataView(i)->GetData();
				sprintf(tmp,"%s",y2Axis->GetDataView(i)->GetName());
				int l = (int)strlen(tmp)-1;
				if( l>=0 && tmp[l]>=128 ) tmp[l]=0;
				fprintf(f, "%s",tmp);
				if(j<nbView) fprintf(f, "%s",separator.c_str());
			}
			fprintf(f,"\n");

			bool eof = false;
			while(!eof) {
				eof = true;
				for(int i=0;i<nbView;i++) if( ptr[i] ) eof = false;
				if( !eof ) {
					for(int i=0;i<nbView;i++) {
						if( ptr[i] ) {
							if (i==0) fprintf(f,"%g%s",ptr[i]->x, separator.c_str());
							fprintf(f,"%g",ptr[i]->y);
							ptr[i]=ptr[i]->next;
						}
						if(i<nbView-1) fprintf(f,"%s", separator.c_str());
					}
					fprintf(f,"\n");
				}
			}
			fclose(f);

		} else {
			GLMessageBox::Display("Cannot open file for writing","Error",GLDLG_OK,GLDLG_ICONERROR);
		}

	}

}

void GLChart::CopyAllToClipboard() {
	/*

	// Compute data length
	//size_t totalLength = 0;

	char tmp[128];

	int nbv1 = y1Axis->GetViewNumber();
	int nbv2 = y2Axis->GetViewNumber();
	int nbView = nbv1 + nbv2;
	if(!nbView) return;

	// Get DataList handle and write dataview name
	DataList *ptr[MAX_VIEWS];
	int j=0;
	//totalLength+=strlen("X axis\t"); //X axis
	for(int i=0;i<nbv1;i++) {
		ptr[j++] = y1Axis->GetDataView(i)->GetData();
		//sprintf(tmp,"%s",y1Axis->GetDataView(i)->GetName());
		//int l = (int)strlen(tmp)-1;
		//if( l>=0 && tmp[l]>=128 ) tmp[l]=0;
		//totalLength+=strlen(tmp);
		//if(j<nbView) totalLength+=strlen("\t");
	}
	for(int i=0;i<nbv2;i++) {
		ptr[j++] = y2Axis->GetDataView(i)->GetData();
		//sprintf(tmp,"%s",y2Axis->GetDataView(i)->GetName());
		//int l = (int)strlen(tmp)-1;
		//if( l>=0 && tmp[l]>=128 ) tmp[l]=0;
		//totalLength+=strlen(tmp);
		//if(j<nbView) totalLength+=strlen("\t");
	}
	//totalLength+=strlen("\n");
	
	bool eof = false;
	while(!eof) {
		eof = true;
		for(int i=0;i<nbView;i++) if( ptr[i] ) eof = false;
		if( !eof ) {
			for(int i=0;i<nbView;i++) {
				if( ptr[i] ) {
					if (i==0) {
						sprintf(tmp,"%g\t",ptr[i]->x);totalLength+=strlen(tmp);
					}
					sprintf(tmp,"%g",ptr[i]->y);totalLength+=strlen(tmp);
					ptr[i]=ptr[i]->next;
				}
				if(i<nbView-1) totalLength+=strlen("\t");
			}
			totalLength+=strlen("\n");
		}
	}

	if( !totalLength ) return;
	*/

	/*
#ifdef _WIN32

	if( !OpenClipboard(NULL) )
		return;

	EmptyClipboard();

	HGLOBAL hText = NULL;
	char   *lpszText;

	//totalLength=150000;

	if(!(hText = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, totalLength+1 ))) {
		CloseClipboard();
		return; 
	}
	if(!(lpszText = (char *)GlobalLock(hText))) {
		CloseClipboard();
		GlobalFree(hText);
		return;
	}

	j=0;
	//X axis
	strcpy(lpszText,"X axis\t");lpszText += strlen("X axis\t");

	for(int i=0;i<nbv1;i++) {
		ptr[j++] = y1Axis->GetDataView(i)->GetData();
		sprintf(tmp,"%s",y1Axis->GetDataView(i)->GetName());
		int l = (int)strlen(tmp)-1;
		if( l>=0 && tmp[l]>=128 ) tmp[l]=0;
		strcpy(lpszText,tmp);
		lpszText += strlen(tmp);
		if(j<nbView) *lpszText++ = '\t';
	}
	for(int i=0;i<nbv2;i++) {
		ptr[j++] = y2Axis->GetDataView(i)->GetData();
		sprintf(tmp,"%s",y2Axis->GetDataView(i)->GetName());
		int l = (int)strlen(tmp)-1;
		if( l>=0 && tmp[l]>=128 ) tmp[l]=0;
		strcpy(lpszText,tmp);
		lpszText += strlen(tmp);
		if(j<nbView) *lpszText++ = '\t';
	}
	*lpszText++ = '\n';

	eof = false;
	while(!eof) {
		eof = true;
		for(int i=0;i<nbView;i++) if( ptr[i] ) eof = false;
		if( !eof ) {
			for(int i=0;i<nbView;i++) {
				if( ptr[i] ) {
					if (i==0) {
						sprintf(tmp,"%g\t",ptr[i]->x);
						strcpy(lpszText,tmp);
						lpszText += strlen(tmp);
					}
					sprintf(tmp,"%g",ptr[i]->y);
					strcpy(lpszText,tmp);
					lpszText += strlen(tmp);
					ptr[i]=ptr[i]->next;
				}
				if(i<nbView-1) *lpszText++ = '\t';
			}
			*lpszText++ = '\n';
		}
	}

	*lpszText++ = 0;

	SetClipboardData(CF_TEXT,hText);
	GlobalUnlock (hText);
	CloseClipboard();
	GlobalFree(hText);

#endif
	*/

DataList *ptr[MAX_VIEWS];
int nbv1 = y1Axis->GetViewNumber();
int nbv2 = y2Axis->GetViewNumber();
int nbView = nbv1 + nbv2;
if (!nbView) return;
std::ostringstream clipboardStream;
char tmp[128];

	int j = 0;
	//X axis
	clipboardStream << "X axis\t";

	for (int i = 0; i < nbv1; i++) {
		ptr[j++] = y1Axis->GetDataView(i)->GetData();
		sprintf(tmp, "%s", y1Axis->GetDataView(i)->GetName());
		int l = (int)strlen(tmp) - 1;
		if (l >= 0 && tmp[l] >= 128) tmp[l] = 0;
		clipboardStream << tmp;
		if (j < nbView) clipboardStream << '\t';
	}
	for (int i = 0; i < nbv2; i++) {
		ptr[j++] = y2Axis->GetDataView(i)->GetData();
		sprintf(tmp, "%s", y2Axis->GetDataView(i)->GetName());
		int l = (int)strlen(tmp) - 1;
		if (l >= 0 && tmp[l] >= 128) tmp[l] = 0;
		clipboardStream << tmp;
		if (j < nbView) clipboardStream << '\t';
	}
	clipboardStream <<  '\n';

	bool eof = false;
	while (!eof) {
		eof = true;
		for (int i = 0; i < nbView; i++) if (ptr[i]) eof = false;
		if (!eof) {
			for (int i = 0; i < nbView; i++) {
				if (ptr[i]) {
					if (i == 0) {
						sprintf(tmp, "%g\t", ptr[i]->x);
						clipboardStream << tmp;
					}
					sprintf(tmp, "%g", ptr[i]->y);
					clipboardStream << tmp;
					ptr[i] = ptr[i]->next;
				}
				if (i < nbView - 1) clipboardStream << '\t';
			}
			clipboardStream << '\n';
		}
	}

	GLToolkit::CopyTextToClipboard(clipboardStream.str());

}

// Load data From a file

#define TRUNC(x) {int l = (int)strlen(x)-1; if(l>=0 && x[l]<32) x[l]=0; }

void GLChart::LoadFile() {

	char tmp[128];

	int nbv1 = y1Axis->GetViewNumber();
	int nbv2 = y2Axis->GetViewNumber();
	int nbView = nbv1 + nbv2;
	if(!nbView) return;

	std::string fn = NFD_OpenFile_Cpp("csv", "");
	if (!fn.empty()) {

		FILE *f = fopen(fn.c_str(),"r");
		if(f) {

			Clear();

			fgets(tmp,128,f);
			TRUNC(tmp);
			// Create a new DataView
			GLDataView *v = new GLDataView();
			v->SetName(tmp);
			double x = 0.0,y;
			while(!feof(f)) {
				fgets(tmp,128,f);
				TRUNC(tmp);
				sscanf(tmp,"%lf",&y);
				v->Add(x,y);
				x = x + 1.0;
			}
			GetY1Axis()->AddDataView(v);
			fclose(f);

		} else {
			GLMessageBox::Display("Cannot open file for writing","Error",GLDLG_OK,GLDLG_ICONERROR);
		}

	}

}

/**
* Display the value tooltip.
* @param g Graphics object
* @param si SearchInfo structure.
* @see JLAxis#searchNearest
*/
void GLChart::showPanel(SearchInfo *si) {

	int maxh = 0;
	int h = 0;
	int w;
	int maxw = 0;
	int x0 = 0,y0 = 0;
	char **str;

	str = buildPanelString(si);

	// Compute panel size

	w = GLToolkit::GetDialogFontBold()->GetTextWidth(str[0]);
	maxw = w;
	h = maxh = GLToolkit::GetDialogFont()->GetTextHeight();

	for (int i = 1; i < 4; i++) {
		if( str[i] ) {
			w = GLToolkit::GetDialogFont()->GetTextWidth(str[i]);
			if (w > maxw) maxw = w;
			maxh += h;
		}
	}

	maxw += 12;
	maxh += 10;

	glColor3f(0.0f,0.0f,0.0f);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glLineWidth(1.0f);

	switch (si->placement) {
	case BOTTOMRIGHT:
		x0 = si->x + 10;
		y0 = si->y + 10;
		glBegin(GL_LINES);
		_glVertex2i(si->x, si->y);
		_glVertex2i(si->x + 10, si->y + 10);
		glEnd();
		break;
	case BOTTOMLEFT:
		x0 = si->x - 10 - maxw;
		y0 = si->y + 10;
		glBegin(GL_LINES);
		_glVertex2i(si->x, si->y);
		_glVertex2i(si->x - 10, si->y + 10);
		glEnd();
		break;
	case TOPRIGHT:
		x0 = si->x + 10;
		y0 = si->y - 10 - maxh;
		glBegin(GL_LINES);
		_glVertex2i(si->x, si->y);
		_glVertex2i(si->x + 10, si->y - 10);
		glEnd();
		break;
	case TOPLEFT:
		x0 = si->x - 10 - maxw;
		y0 = si->y - 10 - maxh;
		glBegin(GL_LINES);
		_glVertex2i(si->x, si->y);
		_glVertex2i(si->x - 10, si->y - 10);
		glEnd();
		break;
	}

	// Draw panel
	GLToolkit::DrawBox(x0, y0, maxw, maxh,255,255,255,false,false,true);

	//Draw info
	GLToolkit::GetDialogFontBold()->SetTextColor(0.0f,0.0f,0.0f);
	GLToolkit::GetDialogFont()->SetTextColor(0.0f,0.0f,0.0f);
	GLToolkit::GetDialogFontBold()->DrawText(x0 + 4, y0 + 3 ,str[0],false);
	y0 += 3;
	for (int i = 1; i < 4; i++) {
		if( str[i] )
			GLToolkit::GetDialogFont()->DrawText(x0 + 5, y0 + 3 + i*h,str[i],false);
	}

	lastSearch = *si;
	ipanelVisible = true;
	for (int i = 0; i < 4; i++) if(str[i]) free(str[i]);

}

//**************************************************
//

/**
* Remove points that exceed displayDuration.
* @param v DataView containing points
* @return Number of deleted points
*/
int GLChart::garbageData(GLDataView *v) {

	int nb = 0;

	if (displayDuration != MAX_VALUE) {
		nb = v->GarbagePointTime(displayDuration);
	}

	return nb;
}

/**
* Add data to dataview , perform fast update when possible and garbage old data
* (if a display duration is specified).
* @param v The dataview
* @param x x coordinates (real space)
* @param y y coordinates (real space)
* @see setDisplayDuration
*/
void GLChart::AddData(GLDataView *v, double x, double y) {

	//Add data
	v->Add(x, y);
	garbageData(v);

}

void GLChart::RemoveDataView(GLDataView *view) {
	if (view != NULL) {
		GLAxis *axis = view->GetAxis();
		if (axis != NULL) {
			axis->RemoveDataView(view);
		}
	}
}
