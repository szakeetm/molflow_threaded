// Copyright (c) 2011 rubicon IT GmbH
#include "GLWindow.h"
#include "GLComponent.h"
#include "GLScrollBar.h"
#include "GLList.h"
#include "GLMessageBox.h"
#include "GLMenu.h"
#include "GLToolkit.h"
#include "GLTextField.h"
#include "MathTools.h" //Min max
//#include <malloc.h>
#include  <math.h>
#include <algorithm> //Sort
#include "Geometry_shared.h"
#include "GLWindowManager.h"
#include <Vector.h>
#include <iomanip> //stream setprecision
#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#define ISBOLD(x) ((x)[0]==':' && (x)[1]=='B' && (x)[2]==':')

#define SB_WIDTH         16
#define LABEL_HEIGHT     16

template<class T> int cmp_column(const void *lhs_, const void *rhs_);
size_t clickedCol;
bool sortDescending;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad *mApp;
#endif

GLList::GLList(int compId) :GLComponent(compId) {

	Sortable = false;
	worker = NULL;
	nbCol = 0;
	nbRow = 0;
	selectionMode = SINGLE_ROW;
	cHeight = 15;
	cWidths = NULL;
	cEdits = NULL;
	cAligns = NULL;
	cColors = NULL;
	cNames = NULL;
	rNames = NULL;
	values = NULL;
	uValues = NULL;
	edit = NULL;
	cornerLabel = NULL;
	isEditing = false;
	gridVisible = false;
	SetBorder(BORDER_BEVEL_IN);
	SetBackgroundColor(240, 240, 240);
	SetFontColor(0, 0, 0);
	sbH = new GLScrollBar(compId);
	sbH->SetRange(10, 10, 1);
	sbH->SetOrientation(SB_HORIZONTAL);
	sbV = new GLScrollBar(compId);
	sbV->SetRange(10, 10, 1);
	sbV->SetOrientation(SB_VERTICAL);
	selectedRows.clear();
	selectedCol = -1;
	sbDragged = false;
	motionSelection = false;
	showCLabel = false;
	autoColumnName = false;
	showRLabel = false;
	autoRowName = false;
	rowLabelAlign = ALIGN_CENTER;
	labWidth = 0;
	labHeight = 0;
	labelRowMargin = 10;
	lastColSel = -1;
	lastRowSel = -1;
	selDragged = false;
	colDragged = false;
	SetVScrollVisible(true);
	SetHScrollVisible(true);

	menu = new GLMenu();

}

void GLList::InvalidateDeviceObjects() {
	//Clear(false,false);
}

void GLList::DestroyComponents()
{
	Clear(false, false);
	SAFE_DELETE(sbH);
	SAFE_DELETE(sbV);
	SAFE_DELETE(edit);
	SAFE_FREE(cornerLabel);
	SAFE_DELETE(menu);
	//selectedRows.clear(); selectedRows.shrink_to_fit();
}

GLList::~GLList() {
	Clear(false, false);
	SAFE_DELETE(sbH);
	SAFE_DELETE(sbV);
	SAFE_DELETE(edit);
	SAFE_FREE(cornerLabel);
	SAFE_DELETE(menu);
}

void GLList::Clear(bool keepColumns, bool showProgress) {
	GLProgress *prgList = NULL;
	double all = (double)nbCol*nbRow;
	if (showProgress) {
		prgList = new GLProgress("Clearing facet hits list...", "Please wait");
		prgList->SetProgress(0.0);
		prgList->SetVisible(true);
	}
	if (!keepColumns) {
		for (int i = 0; i < nbCol; i++)
			SAFE_FREE(cNames[i]);
	}
	for (int i = 0; i < nbRow; i++)
		SAFE_FREE(rNames[i]);
	for (int i = 0; i < nbCol*nbRow; i++) {
		if (showProgress) prgList->SetProgress((double)i / all);
		SAFE_FREE(values[i]);
	}

	if (!keepColumns) {
		SAFE_FREE(cNames);
		SAFE_FREE(cWidths);
		SAFE_FREE(cAligns);
		SAFE_FREE(cColors);
		SAFE_FREE(cEdits);
		nbCol = 0;
		selectedCol = -1;
	}
	SAFE_FREE(rNames);

	SAFE_FREE(values);
	SAFE_FREE(uValues);

	nbRow = 0;
	labWidth = 0; //Row labels

	isEditing = false;
	if (showProgress) prgList->SetVisible(false);
	SAFE_DELETE(prgList);
}

void GLList::SetCornerLabel(const char *text) {
	SAFE_FREE(cornerLabel);
	if (text) cornerLabel = strdup(text);
}

void GLList::SetWorker(Worker *w) {
	worker = w;
}

void GLList::SetRowLabelMargin(int margin) {
	labelRowMargin = margin;
}

void GLList::SetSelectionMode(int mode) {
	selectionMode = mode;
	// Editing only alowed in single selection mode
	if (selectionMode == MULTIPLE_ROW)
		for (int i = 0; i < nbCol; i++) cEdits[i] = 0;
	CancelEdit();
}

void GLList::SetColumnEditable(int *editables) {
	if (cEdits && selectionMode == SINGLE_ROW)
		for (int i = 0; i < nbCol; i++) cEdits[i] = editables[i];
}

void GLList::SetGrid(bool visible) {
	gridVisible = visible;
}

void GLList::ResetValues() {

	for (int i = 0; i < nbCol; i++)
		for (int j = 0; j < nbRow; j++)
			SetValueAt(i, j, "", 0);

}

void GLList::SetParent(GLContainer *parent) {
	sbH->SetParent(parent);
	sbV->SetParent(parent);
	GLComponent::SetParent(parent);
}

void GLList::SetColumnLabelVisible(bool visible) {
	showCLabel = visible;
	labHeight = (showCLabel ? LABEL_HEIGHT : 0);
}

void GLList::SetRowLabelAlign(int align) {
	rowLabelAlign = align;
}

void GLList::SetFocus(bool focus) {
	if (!focus) CancelEdit();
	GLComponent::SetFocus(focus);
}

void GLList::SetHScrollVisible(bool visible) {
	hScrollVisible = visible;
	sbHeight = (visible ? SB_WIDTH : 0);
}

void GLList::SetVScrollVisible(bool visible) {
	vScrollVisible = visible;
	sbWidth = (visible ? SB_WIDTH : 0);
}

void GLList::SetAutoRowLabel(bool enable) {
	autoRowName = enable;
}

void GLList::SetRowLabelVisible(bool visible) {
	showRLabel = visible;
}

void GLList::SetAutoColumnLabel(bool enable) {
	autoColumnName = enable;
}

size_t GLList::GetNbRow() {
	return nbRow;
}

size_t  GLList::GetNbColumn() {
	return nbCol;
}

int  GLList::GetSelectedRow(bool searchIndex) {
	if (selectedRows.size() == 0) {
		return -1;
	}
	else {
		return (searchIndex) ? GetValueInt(selectedRows[0], 0) - 1 : (int)selectedRows[0];
	}
}

int  GLList::GetSelectedColumn() {
	return selectedCol;
}

void GLList::SetMotionSelection(bool enable) {
	motionSelection = enable;
}

int GLList::GetUserValueAt(size_t col, size_t row) {
	assert(col < nbCol); assert(row < nbRow);
	if (uValues)
		return uValues[row*nbCol + col];
	else
		return 0;
}

char *GLList::GetValueAt(size_t col, size_t row) {
	assert(!values || col < nbCol); assert(!values || row < nbRow);
	if (values)
		return values[row*nbCol + col];
	else
		return NULL;
}

void GLList::SetBounds(int x, int y, int width, int height) {

	int mx = (sbWidth) ? sbWidth - 1 : 1;
	int my = (sbHeight) ? sbHeight : 2;

	sbV->SetBounds(x + width - (sbWidth - 2), y + 1, sbWidth - 3, height - my);
	sbH->SetBounds(x, y + height - (sbHeight - 2), width - mx, sbHeight - 3);
	GLComponent::SetBounds(x, y, width, height);
	UpdateSBRange();

}

void GLList::SetSize(size_t nbColumn, size_t nbR, bool keepData, bool showProgress) {

	if (nbColumn == nbCol && nbR == this->nbRow)
		// Already the good size
		return;

	if (nbColumn == 0) return;

	if (!keepData) {
		//Clear & reallocate
		Clear(false, showProgress);
		nbCol = nbColumn;
		this->nbRow = nbR;
		cEdits = (int *)malloc(nbCol * sizeof(int));
		memset(cEdits, 0, nbCol * sizeof(int));
		cWidths = (int *)malloc(nbCol * sizeof(int));
		for (size_t i = 0; i < nbCol; i++) cWidths[i] = 50;
		cAligns = (int *)malloc(nbCol * sizeof(int));
		memset(cAligns, 0, nbCol * sizeof(int));
		cColors = (int *)malloc(nbCol * sizeof(int));
		memset(cColors, 0, nbCol * sizeof(int));
		cNames = (char **)malloc(nbCol * sizeof(char *));
		memset(cNames, 0, nbCol * sizeof(char *));
	}
	else { //reallocate, keeping data
		if (nbCol != nbColumn) {
			cEdits = (int *)realloc(cEdits, nbColumn * sizeof(int));
			if (nbColumn > nbCol) memset(cEdits + nbCol, 0, (nbColumn - nbCol) * sizeof(int)); //Init new area
			cWidths = (int *)realloc(cWidths, nbColumn * sizeof(int));
			for (size_t i = nbCol; i < nbColumn; i++) cWidths[i] = 50; //Init new area
			cAligns = (int *)realloc(cAligns, nbCol * sizeof(int));
			if (nbColumn > nbCol) memset(cAligns + nbCol, 0, (nbColumn - nbCol) * sizeof(int));
			cColors = (int *)realloc(cColors, nbCol * sizeof(int));
			if (nbColumn > nbCol) memset(cColors + nbCol, 0, (nbColumn - nbCol) * sizeof(int));
			cNames = (char **)realloc(cNames, nbCol * sizeof(char*));
			if (nbColumn > nbCol) memset(cNames + nbCol, 0, (nbColumn - nbCol) * sizeof(char*));
			nbCol = nbColumn;
		}
	}
	if (nbR) {
		if (!keepData) {
			this->nbRow = nbR;
			rNames = (char **)malloc(nbR * sizeof(char *));
			memset(rNames, 0, nbR * sizeof(char *));
			values = (char **)malloc(nbCol*nbR * sizeof(char *));
			uValues = (int *)malloc(nbCol*nbR * sizeof(int));
			memset(values, 0, nbCol*nbR * sizeof(char *));
			memset(uValues, 0, nbCol*nbR * sizeof(int));
		}
		else {
			rNames = (char **)realloc(rNames, nbR * sizeof(char *));
			if (nbR > this->nbRow) memset(rNames + this->nbRow, 0, (nbR - this->nbRow) * sizeof(char *));
			values = (char **)realloc(values, nbCol*nbR * sizeof(char *));
			if (nbR > this->nbRow) memset(values + nbCol * this->nbRow, 0, nbCol*(nbR - this->nbRow) * sizeof(char *));
			uValues = (int *)realloc(uValues, nbCol*nbR * sizeof(int));
			if (nbR > this->nbRow) memset(uValues + nbCol * this->nbRow, 0, nbCol*(nbR - this->nbRow) * sizeof(int));
			this->nbRow = nbR;
		}
	}
	else {
		values = NULL;
		uValues = NULL;
		rNames = NULL;
	}
	CreateAutoLabel();
	UpdateSBRange();

}

void GLList::CreateAutoLabel() {

	if (nbCol > 0 && autoColumnName) {

		for (int i = 0; i < nbCol && cNames; i++)
			SAFE_FREE(cNames[i]);

		char tmp[256];
		for (int i = 0; i < nbCol; i++) {
			sprintf(tmp, "%d", i);
			cNames[i] = strdup(tmp);
		}

	}

	if (nbRow > 0 && autoRowName) {

		for (int i = 0; i < nbRow && rNames; i++)
			SAFE_FREE(rNames[i]);

		char tmp[256];
		labWidth = 0;
		for (int i = 0; i < nbRow; i++) {
			sprintf(tmp, "%d", i);
			rNames[i] = strdup(tmp);
			int w = GLToolkit::GetDialogFont()->GetTextWidth(rNames[i]);
			if (w > labWidth) labWidth = w;
		}

	}

}

void GLList::UpdateSBRange() {

	if (nbRow && nbCol) {

		int mx = (sbWidth) ? sbWidth - 1 : 1;
		int my = (sbHeight) ? sbHeight + labHeight : 2 + labHeight;
		int lw = GetColumnStart(nbCol);

		sbV->SetRange((int)nbRow*cHeight, height - my, cHeight);
		int w = (showRLabel) ? labWidth + labelRowMargin : 0;
		sbH->SetRange(lw, width - mx - w, 10);

	}
	else {
		sbV->SetRange(10, 10, 1);
		sbH->SetRange(10, 10, 1);
	}

}

void GLList::SetColumnLabels(const char ** names) {
	if (cNames) {
		for (int i = 0; i < nbCol; i++) {
			SAFE_FREE(cNames[i]);
			if (names[i]) cNames[i] = strdup(names[i]);
		}
	}
}

void GLList::SetRowLabels(const char **names) {

	if (rNames) {
		labWidth = 0;
		for (int i = 0; i < nbRow; i++) {
			SAFE_FREE(rNames[i]);
			if (names[i]) {
				rNames[i] = strdup(names[i]);
				int w = GLToolkit::GetDialogFont()->GetTextWidth(rNames[i]);
				if (w > labWidth) labWidth = w;
			}
		}
	}

}

void GLList::SetColumnWidths(int *widths) {
	if (cWidths)
		for (size_t i = 0; i < nbCol; i++) cWidths[i] = widths[i];
	UpdateSBRange();
}

void GLList::SetColumnWidthForAll(int width) {
	if (cWidths)
		for (size_t i = 0; i < nbCol; i++) cWidths[i] = width;
	UpdateSBRange();
}

void GLList::AutoSizeColumn() {
	int w = 0;
	if (cWidths) {
		GLFont2D *fnt = GLToolkit::GetDialogFont();
		for (int i = 0; i < nbCol; i++) {
			int maxWidth = 10;
			for (int j = 0; j < nbRow; j++) {
				char *v = GetValueAt(i, j);
				if (v) w = fnt->GetTextWidth(v);
				if (w > maxWidth) maxWidth = w;
			}
			if (cNames && cNames[i] && (w = fnt->GetTextWidth(cNames[i])) > maxWidth)
				maxWidth = w;
			cWidths[i] = maxWidth + 10;
		}
		UpdateSBRange();
	}

}

void GLList::SetColumnAligns(int *aligns) {
	if (cAligns)
		for (int i = 0; i < nbCol; i++) cAligns[i] = aligns[i];
}

void GLList::SetAllColumnAlign(int align) {
	if (cAligns)
		for (int i = 0; i < nbCol; i++) cAligns[i] = align;
}

void GLList::SetColumnColors(int *colors) {
	if (cColors)
		for (int i = 0; i < nbCol; i++) cColors[i] = colors[i];
}

void GLList::SetAllColumnColors(int color) {
	if (cColors)
		for (int i = 0; i < nbCol; i++) cColors[i] = color;
}

void GLList::SetColumnLabel(size_t colId, const char *name) {
	if (cNames && colId < nbCol) {
		SAFE_FREE(cNames[colId]);
		if (name) cNames[colId] = strdup(name);
	}
}

void GLList::SetRowLabel(size_t rowId, const char *name) {
	if (rNames && rowId < nbRow) {
		SAFE_FREE(rNames[rowId]);
		if (name) {
			rNames[rowId] = strdup(name);
			int w = GLToolkit::GetDialogFont()->GetTextWidth(rNames[rowId]);
			if (w > labWidth) labWidth = w;
		}
	}
}

void GLList::SetColumnWidth(size_t colId, int width) {
	if (cWidths && colId < nbCol) {
		cWidths[colId] = width;
	}
	UpdateSBRange();
}

void GLList::SetColumnAlign(size_t colId, int align) {
	if (cAligns && colId < nbCol) {
		cAligns[colId] = align;
	}
}

void GLList::SetColumnColor(size_t colId, int color) {
	if (cColors && colId < nbCol) {
		cColors[colId] = color;
	}
}

void GLList::SetRow(size_t row, const char **vals) {
	for (int i = 0; i < nbCol; i++)
		SetValueAt(i, row, vals[i]);
}

void GLList::SetValueAt(size_t col, size_t row, const char *value, int userData, bool searchIndex) {
	assert(col < nbCol); assert(row < nbRow);
	if (this->values) {
		if (col >= 0 && col < this->nbCol && row >= 0 && row < this->nbRow) {
			size_t rowIndex = row;
			if (searchIndex) rowIndex = FindIndex(row, 0);

			char *cell = this->values[(rowIndex*this->nbCol) + col];
			if (value == NULL) {
				SAFE_FREE(cell);
			}
			else {
				if (cell) {
					if (strcmp(value, cell) != 0) {
						SAFE_FREE(cell);
						this->values[(rowIndex*this->nbCol) + col] = strdup(value);
					}
				}
				else {
					this->values[(rowIndex*this->nbCol) + col] = strdup(value);
				}
			}
			this->uValues[(rowIndex*this->nbCol) + col] = userData;
		}
	}
}

void GLList::ScrollToVisible(size_t row, size_t col, bool searchIndex) {

	// Vertical scroll ------------------
	int rowIndex;
	if (searchIndex) rowIndex = FindIndex(row, 0);
	else rowIndex = (int)row;
	if (rowIndex >= 0 && rowIndex < nbRow) {
		int sy = rowIndex * cHeight;
		int pos = sbV->GetPosition();

		int _height = sbHeight ? (height - sbHeight - labHeight) : height - labHeight - 2; //full height minus label, minus Hscrollbar
		if (_height < cHeight) _height = cHeight; //avoid negative height values, even if facet list is out of screen

		// Scroll forward
		if ((sy + cHeight) > (_height + pos)) {
			sbV->SetPosition(sy - _height + cHeight);
		}

		// Scroll backward
		if (sy < pos)
			sbV->SetPosition(sy);

	}

	// Horizontal scroll ----------------

	int labW = (showRLabel) ? labWidth + labelRowMargin : 0;
	int _width = sbWidth ? (width - sbWidth - labW) : width - 2 - labW;
	int pos = sbH->GetPosition();

	if (selectionMode == SINGLE_CELL && col >= 0) {

		// Scroll left
		int colStart = GetColumnStart(col) - pos;
		if (colStart < 0) sbH->SetPosition(colStart + pos);

		// Scroll right
		int sup = colStart + cWidths[col] - _width;
		if (sup > 0) sbH->SetPosition(pos + sup);

	}

	if (selectionMode == BOX_CELL && col >= 0 && lastColSel == col) {

		// Scroll left
		int colStart = GetColumnStart(col) - pos;
		if (colStart < 0) sbH->SetPosition(colStart + pos);

		// Scroll right
		int sup = colStart + cWidths[col] - _width;
		if (sup > 0) sbH->SetPosition(pos + sup);

	}

}

void GLList::ScrollToVisible() {
	if (!selectedRows.size()) return; //was !=
	ScrollToVisible(lastRowSel, selectedCol); //was selected[0]
}

void GLList::ScrollUp() {

	// Scroll backward
	int start, end;
	GetVisibleRows(&start, &end);
	if (start > 0) {
		int sy = ((int)start - 1)*cHeight;
		sbV->SetPosition(sy);
	}

}

void GLList::ScrollDown() {

	// Scroll forward
	int start, end;
	GetVisibleRows(&start, &end);
	if (end < nbRow - 1) {
		int sy = ((int)start + 1)*cHeight;
		sbV->SetPosition(sy);
	}
	else {
		sbV->SetPositionMax();
	}

}

void GLList::ClearSelection() {
	SetSelectedRows(std::vector<size_t>());
	selectedCol = -1;
}

void GLList::SetSelectedRow(int row, bool searchIndex) {

	if (row < 0) { //-1 flag
		ClearSelection();
	}
	else if (row < nbRow) {
		size_t rowIndex;
		if (searchIndex) rowIndex = FindIndex(row, 0);
		else rowIndex = row;
		SetSelectedRows({ rowIndex });
		lastRowSel = (int)rowIndex;
	}

}

void GLList::SetSelectedCell(size_t column, size_t row) {

	if (row < 0) {
		ClearSelection();
	}
	else if (row < nbRow) {
		SetSelectedRows({ row });
		lastRowSel = (int)row;
		if (column < nbCol) {
			selectedCol = (int)column;
			lastColSel = (int)column;
		}
		ScrollToVisible();

	}
}

std::vector<size_t> GLList::GetSelectedRows(bool searchIndex) {
	if (!searchIndex) return selectedRows;
	else {
		std::vector<size_t> ret;
		for (const auto& sel : selectedRows)
			ret.push_back(GetValueInt(sel, 0) - 1);
		return ret;
	}
}

void GLList::AddSelectedRow(int row, bool searchIndex) {

	if (row<0 || row>nbRow - 1 || selectionMode != MULTIPLE_ROW)
		return;

	// Check if already added
	size_t rowIndex = searchIndex ? FindIndex(row, 0) : row;
	bool found = Contains(selectedRows, rowIndex);

	if (found && !GetWindow()->IsShiftDown()) {

		if (selectedRows.size() < 2 && rowIndex != lastRowSel) {
			ClearSelection();
			return;
		}

		// Remove i from selection
		selectedRows.erase(std::find(selectedRows.begin(), selectedRows.end(), rowIndex));
		if (lastRowSel == row) lastRowSel = -1;

	}

	if (!found) {
		selectedRows.push_back(rowIndex);
	}
	lastRowSel = (int)rowIndex;
}

void GLList::SetSelectedRows(std::vector<size_t> selection, bool searchIndex) {
	if (searchIndex) {
		selectedRows.clear();
		for (const auto& sel : selection)
			selectedRows.push_back((size_t)FindIndex(sel, 0));
	}
	else
		selectedRows = selection;
	//selectedRows.shrink_to_fit();
}

void GLList::SelectAllRows() {
	selectedRows.clear();
	for (int i = 0; i < nbRow; i++)
		selectedRows.push_back(i);
	ScrollToVisible();
}

void GLList::Paint() {

	if (!parent) return;
	GLComponent::Paint();


	if (nbRow == 0 || nbCol == 0) return;

	// Viewport dimension,origin
	int sX = sbH->GetPosition();
	int sY = sbV->GetPosition();
	int labW = (showRLabel) ? labWidth + labelRowMargin : 0;
	int _height = sbHeight ? (height - sbHeight) : height;
	int _width = sbWidth ? (width - sbWidth - labW) : width - 2 - labW;
	int wT = GetColumnStart(nbCol);
	int maxX = Min(wT - sX, _width);
	int sx = -sX, sy = -sY + labHeight - 2;

	// Vertical scroll
	if (sbWidth) {
		sbV->Paint();
		glColor3f(0.5f, 0.5f, 0.5f);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glBegin(GL_LINES);
		_glVertex2i(posX + width - sbWidth + 2, posY);
		_glVertex2i(posX + width - sbWidth + 2, posY + height);
		glEnd();
	}

	// horizontal scrool
	if (sbHeight) {
		sbH->Paint();
		glColor3f(0.5f, 0.5f, 0.5f);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glBegin(GL_LINES);
		_glVertex2i(posX, posY + height - sbHeight + 2);
		_glVertex2i(posX + width - 1, posY + height - sbHeight + 2);
		glEnd();
	}

	// Bottom rigth corner between scrollbars
	if (sbWidth && sbHeight)
		GLToolkit::DrawBox(posX + width - sbWidth + 2, posY + height - sbHeight + 3, sbWidth - 3, sbHeight - 3, 212, 208, 200);

	GLToolkit::GetDialogFontBold()->SetTextColor(0.0f, 0.0f, 0.0f);
	GLToolkit::GetDialogFont()->SetTextColor(0.0f, 0.0f, 0.0f);

	// Upper left corner (when both column and row labels)
	if (showCLabel && showRLabel && cornerLabel) {
		int wT = GLToolkit::GetDialogFont()->GetTextWidth(cornerLabel);
		int px = (labW - wT) / 2;
		GLToolkit::GetDialogFont()->DrawText(posX + px, posY + 2, cornerLabel, false);
	}

	// Column labels

	int mb = (sbHeight ? sbHeight - 1 : 1);

	if (showCLabel && height > 16 + sbHeight) {

		// Label border
		GetWindow()->ClipRect(this, 1 + labW, 0, _width, labHeight);
		sx = -sX;
		for (int i = 0; i < nbCol && sx < _width - 1; i++) {
			if (sx + cWidths[i] > 0) {
				GLToolkit::DrawBox(sx, 1, cWidths[i] - 1, 14, 200, 200, 200, true);
			}
			sx += cWidths[i];
		}

		// Label text
		sx = -sX;
		for (int i = 0; i < nbCol && sx < _width; i++) {
			if (sx + cWidths[i] > 0 && cNames[i]) {
				int dx = 0;
				if (sx < 0) {
					GetWindow()->ClipRect(this, 1 + labW, 0, cWidths[i] + sx, labHeight);
					dx = sx;
				}
				else {
					if (sx + cWidths[i] <= _width)
						GetWindow()->ClipRect(this, sx + labW, 0, cWidths[i], labHeight);
					else
						GetWindow()->ClipRect(this, sx + labW, 0, _width - sx, labHeight);
				}
				int wT = GLToolkit::GetDialogFont()->GetTextWidth(cNames[i]);
				int xT = (cWidths[i] - wT) / 2;
				GLToolkit::GetDialogFont()->DrawText(xT + dx, 2, cNames[i], false);
			}
			sx += cWidths[i];
		}

	}


	// Paint selected cells
	switch (selectionMode) {

	case SINGLE_ROW:
	case MULTIPLE_ROW:
		for (auto& sel : selectedRows) {
			if ((sel + 1)*cHeight >= sY && sel*cHeight <= sY + _height) {
				int mu = (showCLabel ? labHeight : 1);
				GetWindow()->Clip(this, 1 + labW, mu, (sbWidth ? sbWidth : 1), Min(mb, this->GetHeight() - mu - 1)); //maintain minimum height to avoid SetViewPort error
				GLToolkit::DrawBox(0, (int)sel*cHeight - sY, maxX - 1, cHeight, 160, 160, 255);
			}
		}
		if (selectedRows.size() && (lastRowSel + 1)*cHeight >= sY && lastRowSel*cHeight <= sY + _height) {
			GLToolkit::DrawBox(0, lastRowSel*cHeight - sY, maxX - 1, cHeight, 100, 100, 255);
		}
		break;

	case BOX_CELL:
		size_t selR, selC, lgthR, lgthC;
		if (GetSelectionBox(&selR, &selC, &lgthR, &lgthC)) {
			for (size_t i = selR; i < selR + lgthR; i++) {
				if ((i + 1)*cHeight >= sY && i*cHeight <= sY + _height) {
					sx = GetColumnStart(selC) - sbH->GetPosition() - 1;
					int wS = GetColsWidth(selC, lgthC);
					if (sx + wS > 0 && sx < _width) {
						GetWindow()->Clip(this, 1 + labW, (showCLabel ? labHeight : 1), (sbWidth ? sbWidth : 1), mb);
						GLToolkit::DrawBox(sx, (int)i*cHeight - sY, wS, cHeight, 160, 160, 255);
					}
				}
			}
		}
		break;

	case SINGLE_CELL:
		if (selectedRows.size() == 1 && selectedCol >= 0) {
			if ((selectedRows[0] + 1)*cHeight >= sY && selectedRows[0] * cHeight <= sY + _height) {
				sx = GetColumnStart(selectedCol) - sbH->GetPosition() - 1;
				if (sx + cWidths[selectedCol] > 0 && sx < _width) {
					GetWindow()->Clip(this, 1 + labW, (showCLabel ? labHeight : 1), (sbWidth ? sbWidth : 1), mb);
					GLToolkit::DrawBox(sx, (int)(selectedRows[0])*cHeight - sY, cWidths[selectedCol], cHeight, 160, 160, 255);
				}
			}
		}
		break;
	}


	// Paint rows
	int hc = _height - labHeight + (sbHeight ? 1 : 0);
	sx = 1 - sX;
	for (int i = 0; i < nbCol && sx < _width; i++) {

		if (sx + cWidths[i] > 0) {

			// Clip column
			int dx = 0;
			if (sx < 0) {
				GetWindow()->ClipRect(this, 1 + labW, labHeight, cWidths[i] + sx, hc);
				dx = sx;
			}
			else {
				if (sx + cWidths[i] <= _width)
					GetWindow()->ClipRect(this, sx + labW, labHeight, cWidths[i] - 2, hc);
				else
					GetWindow()->ClipRect(this, sx + labW, labHeight, _width - sx, hc);
			}

			// Loop row
			for (int j = 0; j < nbRow; j++) {
				if ((j + 1)*cHeight >= sY && (j)*cHeight <= sY + _height) {
					char *value = GetValueAt(i, j);
					if (value) {
						int px = 1; int wT;
						int offset;
						GLFont2D* font;
						if (ISBOLD(value)) {

							font = GLToolkit::GetDialogFontBold();

							offset = 3;
						}
						else {
							font = GLToolkit::GetDialogFont();
							offset = 0;
						}
						switch (cAligns[i]) {
						case ALIGN_CENTER:
							wT = font->GetTextWidth(value);
							px = (cWidths[i] - wT) / 2;
							break;
						case ALIGN_RIGHT:
							wT = font->GetTextWidth(value);
							px = cWidths[i] - wT - 2;
							break;

						}
						switch (cColors[i]) {
						case COLOR_RED:
							font->SetTextColor(1.0f, 0.0f, 0.0f);
							break;
						case COLOR_GREEN:
							font->SetTextColor(0.0f, 1.0f, 0.0f);
							break;
						case COLOR_BLUE:
							font->SetTextColor(0.0f, 0.0f, 1.0f);
							break;
						default:
							font->SetTextColor(0.0f, 0.0f, 0.0f);
							break;
						}
						font->DrawText(px + dx, j*cHeight - sY + (showCLabel ? 1 : 2), value + offset, false);
					}
				} // End if row visible
			} // End row loop
		} // End if colum visible

		sx += cWidths[i];
	}// End row loop


	// Paint the grid
	if (gridVisible) {

		GetWindow()->ClipToWindow();
		sx = -sX;
		sy = -sY + labHeight - 2;
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glColor3f(0.75f, 0.75f, 0.75f);
		glBegin(GL_LINES);

		for (int j = 0; j < nbRow; j++) {
			sy += cHeight;
			if (sy > (showCLabel ? labHeight : 0) && sy <= _height) {
				_glVertex2i(posX + 1, posY + sy + 1);
				_glVertex2i(posX + maxX + labW, posY + sy + 1);
			}
		}
		int py = Min(_height, sy + 1);
		for (int i = 0; i < nbCol; i++) {
			if (sx + cWidths[i] > 0 && sx + cWidths[i] <= _width + 1) {
				_glVertex2i(posX + sx + cWidths[i] + labW, posY + 1);
				_glVertex2i(posX + sx + cWidths[i] + labW, posY + py);
			}
			sx += cWidths[i];
		}
		glEnd();

	}

	// Paint row label
	GetWindow()->ClipRect(this, 1, labHeight, labW, hc);
	for (int j = 0; j < nbRow && showRLabel && rNames; j++) {
		if ((j + 1)*cHeight >= sY && (j)*cHeight <= sY + _height) {
			GLToolkit::DrawBox(1, j*cHeight - sY + (showCLabel ? 0 : 1), labW - 2, 14, 200, 200, 200, true);
			char *value = rNames[j];
			if (value) {
				int px = 1; int wT;
				switch (rowLabelAlign) {
				case ALIGN_CENTER:
					wT = GLToolkit::GetDialogFontBold()->GetTextWidth(value);
					px = (labW - wT) / 2;
					break;
				case ALIGN_RIGHT:
					wT = GLToolkit::GetDialogFontBold()->GetTextWidth(value);
					px = labW - wT - 2;
					break;
				}
				GLToolkit::GetDialogFont()->DrawText(px, j*cHeight - sY + (showCLabel ? 1 : 2), value, false);
			}
		}
	}

	// Restore clip
	GetWindow()->ClipToWindow();

	if (isEditing) {
		int x, y, w, h;
		edit->GetBounds(&x, &y, &w, &h);
		edit->Paint();
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glColor3f(0.4f, 0.4f, 0.4f);
		glBegin(GL_LINE_STRIP);
		_glVertex2i(x, y - 1);
		_glVertex2i(x + w, y - 1);
		_glVertex2i(x + w, y + h - 1);
		_glVertex2i(x, y + h - 1);
		_glVertex2i(x, y);
		glEnd();
	}
	GLToolkit::CheckGLErrors("GLList::Paint()");
}

int GLList::GetValueInt(size_t row, size_t column) {
	try {
		int cmp;
		if (!(this->values[row*nbCol + column])) return -1;
		sscanf((this->values[row*nbCol + column]), "%d", &cmp);
		return cmp;
	}
	catch (Error &e) {
		char errMsg[512];
		sprintf(errMsg, "%s\nWhile finding:%zd", e.GetMsg(), row);
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
	}
}

double GLList::GetValueDouble(size_t row, size_t column) {
	try {
		float cmp;
		if (!(this->values[row*nbCol + column])) return -1;
		sscanf((this->values[row*nbCol + column]), "%g", &cmp);
		return (double)cmp;
	}
	catch (Error &e) {
		char errMsg[512];
		sprintf(errMsg, "%s\nWhile finding:%zd", e.GetMsg(), row);
		GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
	}
}

int  GLList::FindIndex(size_t index, size_t inColumn) {
	for (size_t row = 0; row < nbRow; row++) {
		if (GetValueInt(row, inColumn) == ((int)index + 1)) return (int)row;
	}
	std::stringstream errmsg;
	errmsg << "Facetlist FindIndex(facet " << index + 1 << " in column " << inColumn + 1 << ")\nFacet not found in list!";
	GLMessageBox::Display(errmsg.str().c_str(), "Molflow Bug", GLDLG_OK, GLDLG_ICONERROR);
	return -1; //invalid flag
}

void GLList::GetVisibleRows(int *start, int *end) {

	int pos = sbV->GetPosition();
	int sidx = pos / cHeight;
	int eidx = sidx + height / cHeight;
	if (eidx >= (int)nbRow) eidx = (int)nbRow - 1;
	*start = sidx;
	*end = eidx;

}

int GLList::GetRowForLocation(int x, int y) {

	int pos = (y + sbV->GetPosition() - labHeight);
	int idx = pos / cHeight;

	if (idx >= nbRow || pos < 0) {
		return -1;
	}
	else {
		return idx;
	}
}

size_t GLList::GetRowForLocationSat(int x, int y) {

	int pos = (y + sbV->GetPosition() - labHeight);
	int idx = pos / cHeight;

	if (pos < 0) return 0;
	if (idx >= nbRow) return (nbRow - 1);
	return idx;

}

int GLList::GetColForLocation(int x, int y) {

	int labW = (showRLabel) ? labWidth + labelRowMargin : 0;
	int sx = -sbH->GetPosition() + labW;
	int i = 0;
	bool found = false;

	while (!found && sx < width - sbWidth && i < nbCol) {
		found = (x >= sx) && (x <= sx + cWidths[i]);
		if (!found) {
			sx += cWidths[i];
			i++;
		}
	}

	if (!found) {
		return -1;
	}
	else {
		return i;
	}

}

size_t GLList::GetColForLocationSat(int x, int y) {

	int labW = (showRLabel) ? labWidth + labelRowMargin : 0;
	int sx = -sbH->GetPosition() + labW;
	int i = 0;
	bool found = false;

	if (x < sx) return 0;

	while (!found && i < nbCol) {
		found = (x >= sx) && (x <= sx + cWidths[i]);
		if (!found) {
			sx += cWidths[i];
			i++;
		}
	}

	if (!found) {
		return nbCol - 1;
	}
	else {
		return i;
	}

}

int GLList::GetColsWidth(size_t c, size_t lgth) {

	int sum = 0;
	size_t i = c;
	while (i < nbCol && (i - c) < lgth) {
		sum += cWidths[i];
		i++;
	}
	return sum;

}

int GLList::GetColumnEdge(int x, int y) {

	int sx = -sbH->GetPosition();
	int i = 0;
	bool found = false;

	while (!found && sx < width - 15 && i < nbCol) {
		found = (x >= sx - 3) && (x <= sx + 3);
		if (!found) {
			sx += cWidths[i];
			i++;
		}
	}
	if (!found) found = (x >= sx - 3) && (x <= sx + 3);

	if (i <= 0 || !found) {
		return -1;
	}
	else {
		return i - 1;
	}

}

void GLList::MoveColumn(int x, int y) {

	int dx = x - lastColX;
	int dy = y - lastColY;

	if ((cWidths[draggedColIdx] + dx) > 5) {
		cWidths[draggedColIdx] += dx;
	}

	lastColX = x;
	lastColY = y;
	UpdateSBRange();

}

int GLList::GetDraggedCol() {
	return draggedColIdx;
}

int GLList::GetColWidth(size_t col) {
	if (col >= 0 && col < nbCol)
		return cWidths[col];
	else
		return 0;
}

int GLList::GetColumnStart(size_t colIdx) {

	int sum = 0;
	size_t i = 0;
	while (i < nbCol && i < colIdx) { sum += cWidths[i]; i++; }
	return sum;

}

void GLList::MapEditText() {

	if (edit && selectedRows.size() > 0) {

		ScrollToVisible(selectedRows[0], selectedCol, false);
		//int sx = GetColumnStart(selectedCol) - sbH->GetPosition() + labWidth+ labelRowMargin;
		int sx = GetColumnStart(selectedCol) - sbH->GetPosition() + labWidth + showRLabel * labelRowMargin;
		int sy = (int)(selectedRows[0])*cHeight - sbV->GetPosition() + labHeight;

		// Clip text on column
		int _width = sbWidth ? (width - sbWidth) : width - 2;
		int wText;

		if (sx < 0) {
			wText = cWidths[selectedCol] + sx - 1;
			sx = 1;
		}
		else {
			if (sx + cWidths[selectedCol] <= _width)
				wText = cWidths[selectedCol];
			else
				wText = _width - sx + 1;
		}

		if (wText > 5) {
			edit->SetBounds(posX + sx, posY + sy, wText, cHeight + 2);
			edit->SetText(GetValueAt(selectedCol, selectedRows[0]));
			edit->SelectAll();
			edit->SetFocus(true);
		}
		else {
			// Abort
			CancelEdit();
		}

	}

}

void GLList::UpdateCell() {

	if (!isEditing) return;

	char tmp[128];

	if (cEdits[selectedCol] == EDIT_NUMBER) {
		double val;
		if (!edit->GetNumber(&val)) {
			if (cNames) {
				sprintf(tmp, "Wrong number format at line %zd (%s)", selectedRows[0] + 1, cNames[selectedCol]);
			}
			else {
				sprintf(tmp, "Wrong number format at line %zd (%d)", selectedRows[0] + 1, selectedCol + 1);
			}
			GLMessageBox::Display(tmp, "Error", GLDLG_OK, GLDLG_ICONWARNING);
		}
		else {
			sprintf(tmp, "%.10g", val);
			SetValueAt(selectedCol, selectedRows[0], tmp);
		}
	}
	else {
		SetValueAt(selectedCol, selectedRows[0], edit->GetText().c_str());
	}

}

void GLList::CancelEdit() {

	if (edit) {
		UpdateCell();
		edit->SetFocus(false);
		isEditing = false;
		//SetSelectedRow(-1);
		//selectedCol = -1;
	}

}

#define EDIT_CANCEL  0
#define EDIT_RELAY   1
#define EDIT_IGNORE  2

int GLList::RelayToEditText(SDL_Event *evt) {

	switch (evt->type) {
		 
	case SDL_MOUSEWHEEL:
		return EDIT_IGNORE; //Don't use mousewheel while editing text. Causes problems on Apple devices
	{
#ifdef __APPLE__
		int appleInversionFactor = -1; //Invert mousewheel on Apple devices
#else
		int appleInversionFactor = 1;
#endif
		if (evt->wheel.y != 0) { //Vertical scroll
			int newPos = (int)selectedRows[0] - appleInversionFactor * evt->wheel.y;
			if ((0<=newPos)&&(newPos<=(nbRow-1))) {
				UpdateCell();
				selectedRows[0]=newPos;
				MapEditText();
			}
			return EDIT_RELAY;
		}
	}
	
	case SDL_MOUSEBUTTONUP:
			if (GetWindow()->IsInComp(edit, evt->button.x, evt->button.y) || edit->IsCaptured())
				return EDIT_RELAY;
		
		break;
	case SDL_MOUSEBUTTONDOWN:

			if (GetWindow()->IsInComp(edit, evt->button.x, evt->button.y))
				return EDIT_RELAY;
		
		break;
	case SDL_MOUSEBUTTONDBLCLICK:
		if (GetWindow()->IsInComp(edit, evt->button.x, evt->button.y))
			return EDIT_RELAY;
		break;
	case SDL_MOUSEMOTION:
		return EDIT_RELAY;
		break;
	case SDL_KEYDOWN: {
		switch (evt->key.keysym.sym) {
		case SDLK_DOWN:
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if (selectedRows[0] < nbRow - 1) {
				UpdateCell();
				selectedRows[0]++; lastRowSel = (int)selectedRows[0];
				parent->ProcessMessage(this, MSG_LIST);
				MapEditText();
				//ScrollToVisible(selectedRows[0],0,false);
				return EDIT_IGNORE;
			}
			break;
		case SDLK_UP:
			if (selectedRows[0] > 0) {
				UpdateCell();
				selectedRows[0]--; lastRowSel = (int)selectedRows[0]; //Can't go negative
				parent->ProcessMessage(this, MSG_LIST);
				MapEditText();
				//ScrollToVisible(selectedRows[0],0,false);
				return EDIT_IGNORE;
			}
			break;
		case SDLK_LEFT:
			if (edit->GetCursorPos() == 0 && selectedCol > 0 && cEdits[selectedCol - 1]) {
				UpdateCell();
				selectedCol--; lastColSel = selectedCol; //Can't go negative
				parent->ProcessMessage(this, MSG_LIST);
				MapEditText();
				return EDIT_IGNORE;
			}
			break;
		case SDLK_RIGHT:
			int lgth = edit->GetTextLength();
			if (edit->GetCursorPos() == lgth && selectedCol < nbCol - 1 && cEdits[selectedCol + 1]) {
				UpdateCell();
				selectedCol++; lastColSel = selectedCol;
				parent->ProcessMessage(this, MSG_LIST);
				MapEditText();
				return EDIT_IGNORE;
			}
			break;
		}
		return EDIT_RELAY;
	}
	break;
	case SDL_KEYUP:
		return EDIT_RELAY;
		break;
	case SDL_TEXTINPUT:
		return EDIT_RELAY;
		break;
	}
	

	return EDIT_CANCEL;

}

bool GLList::GetSelectionBox(size_t *row, size_t *col, size_t *rowLength, size_t *colLength) {

	//if( selectionMode==BOX_CELL && selectedRows.size()==1 && selectedCol!=-1 ) {
	if (selectedRows.size() == 1 && selectedCol != -1 && lastRowSel != -1) {
		*row = Min(selectedRows[0], (size_t)lastRowSel);
		*col = Min(selectedCol, lastColSel);
		*rowLength = Max((size_t)lastRowSel, selectedRows[0]) - Min((size_t)lastRowSel, selectedRows[0]) + 1;
		*colLength = Max(lastColSel, selectedCol) - Min(lastColSel, selectedCol) + 1;
		return (*rowLength > 0) && (*colLength > 0);
	}
	return false;
}

void GLList::HandleWheel(SDL_Event *evt) {

	int mx = GetWindow()->GetX(this, evt);
	int my = GetWindow()->GetY(this, evt);

#ifdef __APPLE__
	int appleInversionFactor = -1; //Invert mousewheel on Apple devices
#else
	int appleInversionFactor = 1;
#endif

	if (evt->wheel.y != 0 && !selectedRows.empty()) { //Vertical scroll
		int newPos = (int)selectedRows[0] - appleInversionFactor * evt->wheel.y;
		if (selectedRows.size() == 1 && selectedRows[0] >= 0 && newPos>=0 && newPos<=(nbRow-1)) {
			selectedRows[0]=newPos; //Can't go negative
			lastRowSel = (int)selectedRows[0];
			parent->ProcessMessage(this, MSG_LIST);
			ScrollToVisible();
		}
		else {
			if ((evt->wheel.y * appleInversionFactor)>0) ScrollUp();
			else ScrollDown();
		}
	}

}

void GLList::CopyToClipboard(size_t row, size_t col, size_t rowLght, size_t colLgth) {
	if (this == mApp->facetList) mApp->UpdateFacetHits(true); //Normally only visible rows are updated
	std::ostringstream clipBoard;
	for (size_t i = row; i < row + rowLght; i++) {
		for (size_t j = col; j < col + colLgth; j++) {
			char *v = GetValueAt(j, i);
			if (v) {
				clipBoard << v;
			}
			if (j < col + colLgth - 1) {
				clipBoard << '\t';
			}
		}
		clipBoard << '\r';
		clipBoard << '\n';
	}
	GLToolkit::CopyTextToClipboard(clipBoard.str());

	/*
	// Compute data length
	size_t totalLength = 0;
	for (size_t i = row; i < row + rowLght; i++) {
		for (size_t j = col; j < col + colLgth; j++) {
			char *v = GetValueAt(j, i);
			if (v) totalLength += strlen(v);
			if (j < col + colLgth - 1) totalLength++;
		}
		totalLength += 2;
	}
	if (!totalLength) return;

#ifdef _WIN32

	if (!OpenClipboard(NULL))
		return;

	EmptyClipboard();

	HGLOBAL hText = NULL;
	char   *lpszText;

	if (!(hText = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, totalLength + 1))) {
		CloseClipboard();
		return;
	}
	if (!(lpszText = (char *)GlobalLock(hText))) {
		CloseClipboard();
		GlobalFree(hText);
		return;
	}

	for (size_t i = row; i < row + rowLght; i++) {
		for (size_t j = col; j < col + colLgth; j++) {
			char *v = GetValueAt(j, i);
			if (v) {
				strcpy(lpszText, v);
				lpszText += strlen(v);
			}
			if (j < col + colLgth - 1) {
				*lpszText++ = '\t';
			}
		}
		*lpszText++ = '\r';
		*lpszText++ = '\n';
	}
	*lpszText++ = 0;

	SetClipboardData(CF_TEXT, hText);
	GlobalUnlock(hText);
	CloseClipboard();
	GlobalFree(hText);

#endif
*/
}

void GLList::CopyAllToClipboard() {
	CopyToClipboard(0, 0, nbRow, nbCol);
}

void GLList::CopySelectionToClipboard() {
	std::stringstream clipboardData; clipboardData << std::setprecision(8);
	if (selectionMode == MULTIPLE_ROW) { //Most probably facet hit list
		if (this == mApp->facetList) mApp->UpdateFacetHits(true);
		// Compute data length

		for (int s = 0; s < selectedRows.size(); s++) {
			for (int j = 0; j < nbCol; j++) {
				clipboardData << GetValueAt(j, selectedRows[s]);

				if (j < nbCol - 1) clipboardData << '\t';
			}
			clipboardData << "\r\n";
		}

	}

	else if (selectionMode == BOX_CELL) {  //Texture plotter, for example
		size_t rowStart, colStart, rowNb, colNb;
		GetSelectionBox(&rowStart, &colStart, &rowNb, &colNb);
		for (size_t r = rowStart; r < (rowStart + rowNb); r++) {
			for (size_t c = colStart; c < (colStart + colNb); c++) {
				clipboardData << GetValueAt(c, r);
				if (c < (colStart + colNb - 1)) clipboardData << '\t';
			}
			clipboardData << "\r\n";
		}
		clipboardData << '\0';
	}

	if (clipboardData.str().empty()) return;

	GLToolkit::CopyTextToClipboard(clipboardData.str());

}

void GLList::ManageEvent(SDL_Event *evt) {

	if (!parent) return;

	int mx = GetWindow()->GetX(this, evt);
	int my = GetWindow()->GetY(this, evt);
	int labW = (showRLabel) ? labWidth + labelRowMargin : 0;

	SetCursor(CURSOR_DEFAULT);


	// Edition
	if (isEditing) {
		if (GetWindow()->IsInComp(edit, evt->button.x, evt->button.y)) SetCursor(CURSOR_TEXT);
		int relay = RelayToEditText(evt);
		switch (relay) {
		case EDIT_CANCEL:
			CancelEdit();
			break;
		case EDIT_IGNORE:
			return;
		case EDIT_RELAY:
			edit->ManageEvent(evt);
			return;
		}
	}


	// Dragging

	if (sbDragged == SB_VERTICAL && evt->type == SDL_MOUSEMOTION) {
		sbV->ManageEvent(evt);
		return;
	}
	if (sbDragged == SB_HORIZONTAL && evt->type == SDL_MOUSEMOTION) {
		sbH->ManageEvent(evt);
		return;
	}
	if (colDragged && evt->type == SDL_MOUSEMOTION) {
		MoveColumn(mx, my);
		SetCursor(CURSOR_SIZEHS);
		parent->ProcessMessage(this, MSG_LIST_COL);
		return;
	}
	if (selDragged && evt->type == SDL_MOUSEMOTION) {
		lastColSel = (int)GetColForLocationSat(mx, my);
		lastRowSel = (int)GetRowForLocationSat(mx, my);
		ScrollToVisible(lastRowSel, lastColSel);
		return;
	}

	if (evt->type == SDL_MOUSEBUTTONUP) {
		// Cancel drag
		switch (sbDragged) {
		case SB_VERTICAL:  sbV->ManageEvent(evt); break;
		case SB_HORIZONTAL:sbH->ManageEvent(evt); break;
		}
		sbDragged = 0;
		colDragged = false;
		if (selDragged) {
			lastColSel = (int)GetColForLocationSat(mx, my);
			lastRowSel = (int)GetRowForLocationSat(mx, my);
			parent->ProcessMessage(this, MSG_LIST);
		}
		selDragged = false;
		return;
	}


	// Scrollbar

	if (sbWidth && mx > width - sbWidth
		&& (evt->type != SDL_MOUSEWHEEL)) {
		sbV->ManageEvent(evt);
		if (evt->type == SDL_MOUSEBUTTONDOWN)
			sbDragged = SB_VERTICAL;
		return;
	}

	if (sbHeight && my > height - sbHeight) {
		sbH->ManageEvent(evt);
		if (evt->type == SDL_MOUSEBUTTONDOWN)
			sbDragged = SB_HORIZONTAL;
		return;
	}


	// Column label
	if (showCLabel) {

		if (mx >= labW && mx <= width - sbWidth && my >= 0 && my < labHeight) {

			if (Sortable && evt->type == SDL_MOUSEBUTTONDOWN && evt->button.button == SDL_BUTTON_LEFT && nbRow > 1 && !(GetColumnEdge(mx - labW, my) >= 0)) {
				GLToolkit::SetCursor(CURSOR_BUSY);
				menu->Clear();
				int sCol = GetColForLocation(mx, my);
				if (sCol >= 0) {
					if (this->Sortable) {
						size_t clickedColTmp = sCol;
						if (clickedColTmp == clickedCol)
							sortDescending = !sortDescending;
						clickedCol = clickedColTmp;
						if (this == mApp->facetList) mApp->UpdateFacetHits(true);
						//define sort variable type
#ifdef MOLFLOW
						typedef int SORTVAR;
#endif

#ifdef SYNRAD
						typedef double SORTVAR;
#endif


						// Step 1) Allocate the rows
						SORTVAR **table = new SORTVAR*[nbRow];

						// Step 2) Allocate the columns
						for (int i = 0; i < nbRow; i++)
							table[i] = new SORTVAR[nbCol];

						// Step 3) Use the table
						for (int i = 0; i < nbRow; i++) {
							for (int j = 0; j < nbCol; j++) {
#ifdef MOLFLOW
								table[i][j] = GetValueInt(i, j);
#endif

#ifdef SYNRAD
								table[i][j] = GetValueDouble(i, j);
#endif
							}
						}

						std::vector<size_t> selFacetIds;
						for (auto& sel : selectedRows)
							selFacetIds.push_back(GetValueInt(sel, 0) - 1);

						std::qsort(table, nbRow, sizeof(SORTVAR*), cmp_column<SORTVAR>);

						lastRowSel = -1;

						char tmp[256];
						for (int i = 0; i < nbRow; i++) {
							//for (int j = 0; j < nbCol; j++) { //enough to set facet index
							sprintf(tmp, "%d", (int)table[i][0]);
							SetValueAt(0, i, tmp);
							//}
						}

						//if (selectedRows.size()<1000) SetSelectedRows(selFacets,selectedRows.size(),true); //TOFIX
						if (selectedRows.size() > 1000) {
							ReOrder();
							SetSelectedRows(selectedRows, false);
						}
						else {
							SetSelectedRows(selFacetIds, true);
						}
						// Step 4) Release the memory
						for (int i = 0; i < nbRow; i++)
							delete[] table[i];

						delete[] table;

					}
				}
				return;
			}

			if (evt->type == SDL_MOUSEBUTTONDOWN && evt->button.button == SDL_BUTTON_LEFT) {
				draggedColIdx = GetColumnEdge(mx - labW, my);
				if (draggedColIdx >= 0) {
					SetCursor(CURSOR_SIZEHS);
					colDragged = true;
					lastColX = mx;
					lastColY = my;
				}
				else {
					int sCol = GetColForLocation(mx, my);
					ClearSelection();
					if (selectionMode == BOX_CELL && sCol >= 0) {
						// Select column
						selectedCol = (int)sCol;
						lastColSel = (int)sCol;
						SetSelectedRow(0);
						lastRowSel = (int)nbRow - 1;
					}
					parent->ProcessMessage(this, MSG_LIST);
				}
				return;
			}

			if (evt->type == SDL_MOUSEBUTTONDOWN && evt->button.button == SDL_BUTTON_RIGHT) {
				char tmp[256];
				menu->Clear();
				int sCol = GetColForLocation(mx, my);
				if (sCol >= 0) {
					if (cNames) sprintf(tmp, "Copy column %s", cNames[sCol]);
					else       sprintf(tmp, "Copy column #%d", sCol);
					menu->Add(tmp, 0);
					int menuId = menu->Track(GetWindow(), posX + mx, posY + my);
					if (menuId == 0) {
						CopyToClipboard(0, sCol, nbRow, 1);
					}
				}
				return;
			}

		}

	}

	// Row label
	if (showRLabel) {

		if (mx >= 0 && mx < labW && my >= labHeight && my < height) {
			if (evt->type == SDL_MOUSEBUTTONDOWN && evt->button.button == SDL_BUTTON_LEFT) {
				switch (selectionMode) {

				case SINGLE_CELL:
					ClearSelection();
					break;
				case SINGLE_ROW:
					SetSelectedRow(GetRowForLocation(mx, my));
					break;
				case MULTIPLE_ROW:
					if (GetWindow()->IsCtrlDown())
						AddSelectedRow(GetRowForLocation(mx, my));
					else
						SetSelectedRow(GetRowForLocation(mx, my));
					break;
				case BOX_CELL:
					selectedCol = 0;
					lastColSel = (int)nbCol - 1;
					SetSelectedRow(GetRowForLocation(mx, my));
					if (selectedRows.size() > 0) lastRowSel = (int)(selectedRows[0]);
					break;

				}
				parent->ProcessMessage(this, MSG_LIST);
				return;
			}

			if (evt->type == SDL_MOUSEBUTTONDOWN && evt->button.button == SDL_BUTTON_RIGHT) {
				char tmp[256];
				menu->Clear();
				int sRow = GetRowForLocation(mx, my);
				if (sRow >= 0) {
					if (rNames) sprintf(tmp, "Copy row %s", rNames[sRow]);
					else       sprintf(tmp, "Copy row #%d", sRow);
					menu->Add(tmp, 0);
					int menuId = menu->Track(GetWindow(), posX + mx, posY + my);
					if (menuId == 0) {
						CopyToClipboard(sRow, 0, 1, nbCol);
					}
				}
				return;
			}

		}

	}


	// List

	if (evt->type == SDL_MOUSEMOTION && motionSelection) {
		switch (selectionMode) {
		case SINGLE_ROW:
			if (my < height - 3) {
				SetSelectedRow(GetRowForLocation(mx, my));
			}
			break;
		}
	}

	if (evt->type == SDL_MOUSEMOTION) {
		if (showCLabel && mx >= labW && mx <= width - sbWidth && my >= 0 && my < labHeight) {
			int edge = GetColumnEdge(mx - labW, my);
			if (edge >= 0) SetCursor(CURSOR_SIZEHS);
		}
	}

	if (evt->type == SDL_MOUSEWHEEL) {
		HandleWheel(evt);
	}

	if (evt->type == SDL_MOUSEBUTTONUP)
		if (evt->button.button == SDL_BUTTON_LEFT && motionSelection) {
			switch (selectionMode) {
			case SINGLE_ROW:
				SetSelectedRow(GetRowForLocation(mx, my));
				if (selectedRows.size() > 0) parent->ProcessMessage(this, MSG_LIST);
				break;
			}
		}



	if (evt->type == SDL_MOUSEBUTTONDOWN) {

		switch (selectionMode) {

		case BOX_CELL:
			if (evt->button.button == SDL_BUTTON_LEFT) {
				selectedCol = GetColForLocation(mx, my);
				SetSelectedRow(GetRowForLocation(mx, my));
				if (selectedCol != -1 && selectedRows.size() > 0) {
					selDragged = true;
					lastColSel = selectedCol;
					lastRowSel = (int)(selectedRows[0]);
				}
			}
			if (evt->button.button == SDL_BUTTON_RIGHT) {
				menu->Clear();
				menu->Add("Copy all", 0);
				size_t selR, selC, lgthR, lgthC;
				if (GetSelectionBox(&selR, &selC, &lgthR, &lgthC))
					menu->Add("Copy selection", 1);
				int menuId = menu->Track(GetWindow(), mx + posX, my + posY);
				if (menuId == 0) {
					CopyAllToClipboard();
				} if (menuId == 1) {
					CopySelectionToClipboard();
				}
			}

			break;

		case SINGLE_ROW:
		case SINGLE_CELL:

			if (evt->button.button == SDL_BUTTON_LEFT) {
				selectedCol = lastColSel = GetColForLocation(mx, my);
				SetSelectedRow(GetRowForLocation(mx, my));
				parent->ProcessMessage(this, MSG_LIST);
				ScrollToVisible();
				if (selectedCol >= 0 && cEdits[selectedCol]) {
					isEditing = (selectedRows.size() == 1);
					if (isEditing) {
						if (!edit) {
							edit = new GLTextField(0, "");
							edit->SetBorder(BORDER_NONE);
							edit->SetParent(GetParent());
						}
						MapEditText();
						return;
					}
				}
			}
			if (evt->button.button == SDL_BUTTON_RIGHT) {
				menu->Clear();
				menu->Add("Copy all", 0);
				if (selectedRows.size() == 1 && selectedCol > 0)
					menu->Add("Copy selection", 1);
				int menuId = menu->Track(GetWindow(), mx + posX, my + posY);
				if (menuId == 0) {
					CopyAllToClipboard();
				}
				else if (menuId == 1) {

					if (selectionMode == SINGLE_CELL)
						CopyToClipboard(selectedRows[0], selectedCol, 1, 1);
					else
						CopyToClipboard(selectedRows[0], 0, 1, nbCol);
				}
			}

			break;

		case MULTIPLE_ROW:

			if (evt->button.button == SDL_BUTTON_LEFT) {
				selectedCol = GetColForLocation(mx, my);
				if (GetWindow()->IsCtrlDown())
					AddSelectedRow(GetRowForLocation(mx, my));
				else {
					SetSelectedRow(GetRowForLocation(mx, my));
					//ScrollToVisible();
				}
				parent->ProcessMessage(this, MSG_LIST);

			}

			if (evt->button.button == SDL_BUTTON_RIGHT) {
				menu->Clear();
				menu->Add("Copy all", 0);
				// TODO: Improve me
				if (selectedRows.size() > 0) menu->Add("Copy selection", 1);
				int menuId = menu->Track(GetWindow(), mx + posX, my + posY);
				if (menuId == 0) {
					CopyAllToClipboard();
				}
				else if (menuId == 1) {
					CopySelectionToClipboard();
				}
			}


			break;

		}
	}

	if (evt->type == SDL_MOUSEBUTTONDBLCLICK)
		if (evt->button.button == SDL_BUTTON_LEFT) {
			switch (selectionMode) {
			case SINGLE_ROW:
				SetSelectedRow(GetRowForLocation(mx, my));
				if (selectedRows.size() > 0) parent->ProcessMessage(this, MSG_LIST_DBL);
				break;
			case MULTIPLE_ROW:
				if (GetWindow()->IsCtrlDown())
					AddSelectedRow(GetRowForLocation(mx, my));
				else
					SetSelectedRow(GetRowForLocation(mx, my));
				if (selectedRows.size() > 0) parent->ProcessMessage(this, MSG_LIST_DBL);
				break;
			}
		}


	// Key press

	if (evt->type == SDL_KEYDOWN) {

		switch (evt->key.keysym.sym) {

		case SDLK_LEFT:
			switch (selectionMode) {
			case SINGLE_CELL:
			case BOX_CELL:
				if (selectedCol > 0 && selectedRows.size() == 1) {
					selectedCol--; //Can't go negative
					lastColSel = selectedCol;
					parent->ProcessMessage(this, MSG_LIST);
					ScrollToVisible();
				}
				break;
				break;
			}
			break;

		case SDLK_RIGHT:
			switch (selectionMode) {
			case SINGLE_CELL:
			case BOX_CELL:
				if (selectedCol >= 0 && selectedCol < nbCol - 1 && selectedRows.size() == 1) {
					selectedCol++;
					lastColSel = selectedCol;
					parent->ProcessMessage(this, MSG_LIST);
					ScrollToVisible();
				}
				break;
				break;
			}
			break;

		case SDLK_DOWN:
			switch (selectionMode) {
			case SINGLE_CELL:
			case BOX_CELL:
				if (selectedRows.size() == 1 && selectedRows[0] < nbRow - 1 && selectedCol >= 0) {
					selectedRows[0]++;
					lastRowSel = (int)(selectedRows[0]);
					parent->ProcessMessage(this, MSG_LIST);
					ScrollToVisible();
				}
				break;
			case SINGLE_ROW:
			case MULTIPLE_ROW:
				//if(selectedRows.size()==1 && selectedRows[0]<nbRow-1) {
				if (lastRowSel < nbRow - 1) {
					if (GetWindow()->IsShiftDown()) {
						AddSelectedRow(lastRowSel + 1);
						//lastRowSel=selectedRows[selectedRows.size()-1];
					}
					else
					{
						selectedRows = { (size_t)lastRowSel + 1 };
						lastRowSel = (int)(selectedRows[0]);
					}
					parent->ProcessMessage(this, MSG_LIST);
					ScrollToVisible();
				}
				break;
			}
			break;

		case SDLK_UP:
			switch (selectionMode) {
			case SINGLE_CELL:
			case BOX_CELL:
				if (selectedRows.size() == 1 && selectedRows[0] > 0 && selectedCol >= 0) {
					selectedRows[0]--;  //Can't go negative
					lastRowSel = (int)(selectedRows[0]);
					parent->ProcessMessage(this, MSG_LIST);
					ScrollToVisible();
				}
				break;
			case SINGLE_ROW:
			case MULTIPLE_ROW:
				if (lastRowSel > 0) {
					if (GetWindow()->IsShiftDown()) {
						AddSelectedRow(lastRowSel - 1);
						//lastRowSel=selectedRows[selectedRows.size()-1];
					}
					else
					{
						selectedRows = { (size_t)lastRowSel - 1 };
						lastRowSel = (int)(selectedRows[0]);
					}
					parent->ProcessMessage(this, MSG_LIST);
					ScrollToVisible();
				}
				break;
			}
			break;
		}

	}

}

template<class T> int cmp_column(const void *lhs_, const void *rhs_) {
	// optimize this to taste
	const T **lhs = (const T**)(lhs_);
	const T **rhs = (const T**)(rhs_);
	//int *lhs = (int*)(lhs_);
	//int *rhs = (int*)(rhs_);
	if ((*lhs)[clickedCol] < (*rhs)[clickedCol]) return (sortDescending) ? 1 : -1;
	if ((*lhs)[clickedCol] > (*rhs)[clickedCol]) return (sortDescending) ? -1 : 1;
	return 0;
}

void GLList::ReOrder() {
	char tmp[64];

	for (int i = 0; i < nbRow; i++) {
		sprintf(tmp, "%d", i + 1);
		SetValueAt(0, i, tmp);
	}
}

void GLList::PasteClipboardText(bool allowExpandRows, bool allowExpandColumns, int extraRowsAtEnd) {
	if (!SDL_HasClipboardText()) return;
	char* content = SDL_GetClipboardText();

//#ifdef _WIN32

//	if (OpenClipboard(NULL)) {
//		HGLOBAL hMem;
//		if (hMem = GetClipboardData(CF_TEXT)) {
//			LPVOID ds = GlobalLock(hMem);
//			if (ds) {
//				char *content = (char *)ds;

				//Get selection and initialize variables
				size_t row = 0;
				size_t col = 0;
				size_t colBegin = 0;
				size_t u, v, wu, wv;
				if (GetSelectionBox(&v, &u, &wv, &wu)) {
					row = v;
					col = colBegin = u;
				}

				//Count clipboard table size
				int clipboardRows = 0;
				int clipboardCols = 1;

				if (allowExpandRows || allowExpandColumns) {
					for (size_t m = 0; content[m] != 0 && content[m] != '\r' && content[m] != '\n'; m++) {
						if (content[m] == '\t') {
							clipboardCols++;
						}
					}

					for (size_t m = 0; content[m] != 0; m++) {
						if (content[m] == '\r') {
							clipboardRows++;
						}
					}
					bool needsMoreRows = allowExpandRows && (row + clipboardRows + extraRowsAtEnd > nbRow);
					bool needsMoreCols = allowExpandColumns && (col + clipboardCols > nbCol);
					bool ok = true;
					if (Max(needsMoreRows * (row + clipboardRows - nbRow), needsMoreCols*(col + clipboardCols - nbCol)) >= 20)
						ok = GLMessageBox::Display("Increase list size by more than 20 rows/columns?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK;
					if (ok && (needsMoreRows || needsMoreCols)) this->SetSize(allowExpandColumns ? Max(nbCol, col + clipboardCols) : nbCol, allowExpandRows ? Max(nbRow, row + clipboardRows + extraRowsAtEnd) : nbRow, true, false);
				}

				size_t cursor = 0;

				size_t length = strlen(content);

				char tmp[MAX_TEXT_SIZE];
				tmp[0] = '\0';
				while (cursor < length) {
					char c = content[cursor];
					if (c == '\t') {
						if (col < nbCol && row < nbRow) SetValueAt(col, row, tmp);
						col++;
						tmp[0] = '\0';
					}
					else if (c == '\r') {
						if (((cursor + 1) < length) && content[cursor + 1] == '\n') cursor++; //Anticipating \n after \r
						if (col < nbCol && row < nbRow) SetValueAt(col, row, tmp);
						row++;
						tmp[0] = '\0';
						col = colBegin;
					}
					else if (strlen(tmp) < (MAX_TEXT_SIZE - 1)) {
						size_t len = strlen(tmp);
						tmp[len] = c;
						tmp[len + 1] = '\0';
					}
					cursor++;
				}

//				GlobalUnlock(hMem);
//			}
//		}
//		CloseClipboard();
//	}
// #endif

}

void GLList::SetFontColor(int r, int g, int b) {
	FontColorR = r;
	FontColorG = g;
	FontColorB = b;
}