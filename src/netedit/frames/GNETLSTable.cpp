/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2022 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GNETLSTable.cpp
/// @author  Pablo Alvarez Lopez
/// @date    2022-07-21
///
// Table used in GNETLSFrame for editing TLS programs
/****************************************************************************/
#include <config.h>

#include "GNETLSTable.h"

#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>

#define EXTRAMARGING 1
#define DEFAULTWIDTH 190

// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNETLSTable) GNETLSTableMap[] = {
    // text fields
    FXMAPFUNC(SEL_FOCUSIN,  MID_GNE_TLSTABLE_TEXTFIELD,     GNETLSTable::onFocusRow),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TLSTABLE_TEXTFIELD,     GNETLSTable::onCmdEditRow),
    FXMAPFUNC(SEL_KEYPRESS, MID_GNE_TLSTABLE_TEXTFIELD,     GNETLSTable::onCmdKeyPress),
    // add phase button
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TLSTABLE_ADDPHASE,      GNETLSTable::onCmdAddPhase),
    FXMAPFUNC(SEL_KEYPRESS, MID_GNE_TLSTABLE_TEXTFIELD,     GNETLSTable::onCmdKeyPress),
    // remove phase button
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TLSTABLE_REMOVEPHASE,   GNETLSTable::onCmdRemovePhase),
    FXMAPFUNC(SEL_KEYPRESS, MID_GNE_TLSTABLE_TEXTFIELD,     GNETLSTable::onCmdKeyPress),
    // move up phase button
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TLSTABLE_MOVEUPPHASE,   GNETLSTable::onCmdMoveUpPhase),
    FXMAPFUNC(SEL_KEYPRESS, MID_GNE_TLSTABLE_TEXTFIELD,     GNETLSTable::onCmdKeyPress),
    // move down phase button
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_TLSTABLE_MOVEDOWNPHASE, GNETLSTable::onCmdMoveDownPhase),
    FXMAPFUNC(SEL_KEYPRESS, MID_GNE_TLSTABLE_TEXTFIELD,     GNETLSTable::onCmdKeyPress),
};

// Object implementation
FXIMPLEMENT(GNETLSTable, FXHorizontalFrame, GNETLSTableMap, ARRAYNUMBER(GNETLSTableMap))

// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNETLSTable - public methods
// ---------------------------------------------------------------------------

GNETLSTable::GNETLSTable(GNETLSEditorFrame::TLSPhases* TLSPhasesParent) :
    FXHorizontalFrame(TLSPhasesParent->getCollapsableFrame(), GUIDesignAuxiliarTLSTable),
    myProgramFont(new FXFont(getApp(), "Courier New", 10)),
    myIndexFont(new FXFont(getApp(), "Segoe UI", 9)),
    myIndexSelectedFont(new FXFont(getApp(), "Segoe UI", 9, FXFont::Bold)),
    myTLSPhasesParent(TLSPhasesParent) {
    // set default width
    recalcTableWidth();
}


GNETLSTable::~GNETLSTable() {
    // delete fonts
    delete myProgramFont;
    delete myIndexFont;
    delete myIndexSelectedFont;
}


GNETLSEditorFrame::TLSPhases* 
GNETLSTable::getTLSPhasesParent() const {
    return myTLSPhasesParent;
}


void
GNETLSTable::recalcTableWidth() {
    // get minimum width of all elements
    int minimumTableWidth = 0;
    // get pointer to name column
    Column* nameColumn = nullptr;
    // iterate over all columns
    for (const auto& column : myColumns) {
        // check if this is the name column
        if (column->getType() == 'm') {
            // save column
            nameColumn = column;
        } else {
            // get minimum column width
            const auto  minimunColWidth = column->getColumnMinimumWidth();
            // set columnwidth
            column->setColumnWidth(minimunColWidth);
            // update minimum table width
            minimumTableWidth += minimunColWidth;
        }
    }
    // adjust name column
    if (nameColumn) {
        // get column name width
        const int minimumColNameWidth = nameColumn->getColumnMinimumWidth();
        // get scrollBar width 
        const int scrollBarWidth = myTLSPhasesParent->getTLSEditorParent()->getScrollBarWidth();
        // get frame area width - padding (30, constant, 15 left, 15 right)
        const auto frameAreaWidth = myTLSPhasesParent->getTLSEditorParent()->getViewNet()->getViewParent()->getFrameAreaWidth() - 30;
        // continue depending of minimum table width
        if ((frameAreaWidth - (minimumTableWidth + minimumColNameWidth + scrollBarWidth)) > 0) {
            nameColumn->setColumnWidth(frameAreaWidth - minimumTableWidth - scrollBarWidth);
            setWidth(frameAreaWidth);
        } else {
            nameColumn->setColumnWidth(minimumColNameWidth);
            setWidth(minimumTableWidth + minimumColNameWidth);
        }
    } else if (minimumTableWidth > 0) {
        setWidth(minimumTableWidth);
    } else {
        setWidth(DEFAULTWIDTH);
    }
}


void
GNETLSTable::clearTable() {
    // clear rows (always before columns)
    for (const auto& row : myRows) {
        delete row;
    }
    // clear columns
    for (const auto& column : myColumns) {
        delete column;
    }
    // drop rows and columns
    myRows.clear();
    myColumns.clear();
}


void
GNETLSTable::setTableSize(const std::string &columnsType, const int numberRow) {
    // first clear table
    clearTable();
    // create columns
    for (int i = 0; i < (FXint)columnsType.size(); i++) {
        myColumns.push_back(new Column(this, i, columnsType.at(i)));
    }
    // create rows
    for (int i = 0; i < numberRow; i++) {
        myRows.push_back(new Row(this));
    }
    // mark first and last rows
    if (myRows.size() > 0) {
        myRows.front()->markAsFirstRow();
        myRows.back()->markAsLastRow();
    }
    // if we have only a row, disable remove button
    if (myRows.size() == 1) {
        myRows.front()->disableRemoveRow();
    }
}


void
GNETLSTable::setItemText(FXint row, FXint column, const std::string& text) {
    if ((row >= 0) && (row < (FXint)myRows.size()) && 
        (column >= 0) && (column < (FXint)myColumns.size())) {
        myRows.at(row)->setText(column, text);
    } else {
        throw ProcessError("Invalid row or column");
    }
}


std::string
GNETLSTable::getItemText(const int row, const int column) const {
    if ((row >= 0) && (row < (FXint)myRows.size()) && 
        (column >= 0) && (column < (FXint)myColumns.size())) {
        return myRows.at(row)->getText(column);
    }
    throw ProcessError("Invalid row or column");
}


int
GNETLSTable::getNumRows() const {
    return (int)myRows.size();
}


int
GNETLSTable::getCurrentSelectedRow() const {
    return myCurrentSelectedRow;
}


void
GNETLSTable::selectRow(const int row) {
    if ((row >= 0) && (row < (FXint)myRows.size())) {
        // update current selected row
        myCurrentSelectedRow = row;
        // update index label
        updateIndexLabel();
    } else {
        throw ProcessError("Invalid row");
    }
}


void
GNETLSTable::setColumnLabelTop(const int column, const std::string& text, const std::string& tooltip) {
    if ((column >= 0) && (column < (int)myColumns.size())) {
        myColumns.at(column)->setColumnLabelTop(text, tooltip);
    } else {
        throw ProcessError("Invalid column");
    }
}


void
GNETLSTable::setColumnLabelBot(const int column, const std::string& text) {
    if ((column >= 0) && (column < (int)myColumns.size())) {
        myColumns.at(column)->setColumnLabelBot(text);
    } else {
        throw ProcessError("Invalid column");
    }
}


long
GNETLSTable::onFocusRow(FXObject* sender, FXSelector, void*) {
    int selectedRow = -1;
    // search selected text field
    for (int rowIndex = 0; rowIndex < (int)myRows.size(); rowIndex++) {
        // iterate over every cell
        for (const auto &cellTextField : myRows.at(rowIndex)->getCells()) {
            if (cellTextField->getTextField() == sender) {
                selectedRow = rowIndex;
            }
        }
    }
    // update index label
    updateIndexLabel();
    // switch phase
    if (myCurrentSelectedRow != selectedRow) {
        myCurrentSelectedRow = selectedRow;
        myTLSPhasesParent->switchPhase();
    }
    return 0;
}


long 
GNETLSTable::onCmdEditRow(FXObject* sender, FXSelector, void*) {
    // search selected text field
    for (int columnIndex = 0; columnIndex < (int)myColumns.size(); columnIndex++) {
        for (int rowIndex = 0; rowIndex < (int)myRows.size(); rowIndex++) {
            // get text field
            const auto textField = myRows.at(rowIndex)->getCells().at(columnIndex)->getTextField();
            if (textField == sender) {
                // edit value and change value depending of result
                if (myTLSPhasesParent->changePhaseValue(columnIndex, rowIndex, textField->getText().text())) {
                    textField->setTextColor(FXRGB(0, 0, 0));
                } else {
                    textField->setTextColor(FXRGB(255, 0, 0));
                }
                return 1;
            }
        }
    }
    // nothing to edit
    return 0;
}


long 
GNETLSTable::onCmdKeyPress(FXObject* sender, FXSelector sel, void* ptr) {
    // get FXEvent
    FXEvent* eventInfo = (FXEvent*)ptr;
    // check code
    if (eventInfo->code == 65362) {
        // move up
        if (myCurrentSelectedRow > 0) {
            myCurrentSelectedRow -= 1;
        } else {
            // we're in the first, then select last
            myCurrentSelectedRow = ((int)myRows.size() - 1);
        }
        // update index label
        updateIndexLabel();
        return 1;
    } else if (eventInfo->code == 65364) {
        // move down
        if (myCurrentSelectedRow < ((int)myRows.size() - 1)) {
            myCurrentSelectedRow += 1;
        } else {
            // we're in the last, then select first
            myCurrentSelectedRow = 0;
        }
        // update index label
        updateIndexLabel();
        return 1;
    } else {
        // continue handling key pres
        return sender->handle(sender, sel, ptr);
    }
}


long 
GNETLSTable::onCmdAddPhase(FXObject* sender, FXSelector, void*) {
    // search selected text field
    for (int indexRow = 0; indexRow < (int)myRows.size(); indexRow++) {
        // iterate over every cell
        for (const auto &cellTextField : myRows.at(indexRow)->getCells()) {
            if (cellTextField->getAddPhaseButton() == sender) {
                // add row
                myTLSPhasesParent->addPhase(indexRow);
                // stop
                return 0;
            }
        }
    }
    return 0;
}


long
GNETLSTable::onCmdRemovePhase(FXObject* sender, FXSelector, void*) {
    // search selected text field
    for (int indexRow = 0; indexRow < (int)myRows.size(); indexRow++) {
        // iterate over every cell
        for (const auto &cellTextField : myRows.at(indexRow)->getCells()) {
            if (cellTextField->getButton() == sender) {
                // remove row
                myTLSPhasesParent->removePhase(indexRow);
                // stop
                return 0;
            }
        }
    }
    return 0;
}


long 
GNETLSTable::onCmdMoveUpPhase(FXObject* sender, FXSelector, void*) {
    // search selected text field
    for (int indexRow = 0; indexRow < (int)myRows.size(); indexRow++) {
        // iterate over every cell
        for (const auto &cellTextField : myRows.at(indexRow)->getCells()) {
            if (cellTextField->getButton() == sender) {
                // move phase up
                myTLSPhasesParent->movePhaseUp(indexRow);
                // stop
                return 0;
            }
        }
    }
    return 0;
}


long
GNETLSTable::onCmdMoveDownPhase(FXObject* sender, FXSelector, void*) {
    // search selected text field
    for (int indexRow = 0; indexRow < (int)myRows.size(); indexRow++) {
        // iterate over every cell
        for (const auto &cellTextField : myRows.at(indexRow)->getCells()) {
            if (cellTextField->getButton() == sender) {
                // move phase down
                myTLSPhasesParent->movePhaseDown(indexRow);
                // stop
                return 0;
            }
        }
    }
    return 0;
}


void 
GNETLSTable::updateIndexLabel() {
    // update radio buttons checks
    for (int rowIndex = 0; rowIndex < (int)myRows.size(); rowIndex++) {
        // iterate over every cell
        for (const auto &cellIndexLabel : myRows.at(rowIndex)->getCells()) {
            if (cellIndexLabel->getIndexLabel()) {
                if (myCurrentSelectedRow == rowIndex) {
                    cellIndexLabel->showIndexLabelBold();
                } else {
                    cellIndexLabel->showIndexLabelNormal();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GNETLSTable::Cell - methods
// ---------------------------------------------------------------------------

GNETLSTable::Cell::Cell(GNETLSTable* TLSTable, FXTextField* textField, int col, int row) :
    myTLSTable(TLSTable),
    myTextField(textField),
    myCol(col),
    myRow(row) {
    // create
    textField->create();
}


GNETLSTable::Cell::Cell(GNETLSTable* TLSTable, FXLabel* indexLabel, FXLabel* indexLabelBold, int col, int row) :
    myTLSTable(TLSTable),
    myIndexLabel(indexLabel),
    myIndexLabelBold(indexLabelBold),
    myCol(col),
    myRow(row) {
    // create both
    indexLabel->create();
    indexLabelBold->create();
    // hide bold and set background
    indexLabelBold->hide();
    indexLabelBold->setBackColor(FXRGBA(210, 233, 255, 255));
}


GNETLSTable::Cell::Cell(GNETLSTable* TLSTable, FXButton* button, int col, int row) :
    myTLSTable(TLSTable),
    myButton(button),
    myCol(col),
    myRow(row) {
    // create
    button->create();
}


GNETLSTable::Cell::Cell(GNETLSTable* TLSTable, int col, int row) :
    myTLSTable(TLSTable),
    myCol(col),
    myRow(row) {
    // build locator popup
    auto menuButtonPopup = new FXPopup(TLSTable->myColumns.at(col)->getVerticalCellFrame(), POPUP_VERTICAL);
    // build menu button
    auto menuButton = new MFXMenuButtonTooltip(TLSTable->myColumns.at(col)->getVerticalCellFrame(), "\tAdd phase\tAdd phase.",
        GUIIconSubSys::getIcon(GUIIcon::ADD), menuButtonPopup, GUIDesignTLSTableCheckableButtonIcon);
    // default phase
    myAddPhaseButton = new MFXButtonTooltip(menuButtonPopup,
        "\tDefault phase\tAdd default phase.",
        GUIIconSubSys::getIcon(GUIIcon::LOCATEJUNCTION), TLSTable, MID_GNE_TLSTABLE_ADDPHASE, GUIDesignButtonIcon);
    // copy phase
    myCopyPhaseButton = new MFXButtonTooltip(menuButtonPopup,
        "\tCopy phase\tCopy phase.",
        GUIIconSubSys::getIcon(GUIIcon::LOCATEEDGE), TLSTable, MID_GNE_TLSTABLE_COPYPHASE, GUIDesignButtonIcon);
    // create elements
    menuButtonPopup->create();
    menuButton->create();
    myAddPhaseButton->create();
    myCopyPhaseButton->create();
}


FXTextField* 
GNETLSTable::Cell::getTextField() {
    return myTextField;
}


FXLabel* 
GNETLSTable::Cell::getIndexLabel() {
    return myIndexLabel;
}


FXButton* 
GNETLSTable::Cell::getButton() {
    return myButton;
}



FXButton*
GNETLSTable::Cell::getAddPhaseButton() {
    return myAddPhaseButton;
}


FXButton* 
GNETLSTable::Cell::getCopyPhaseButton() {
    return myCopyPhaseButton;
}


void
GNETLSTable::Cell::showIndexLabelNormal() {
    myIndexLabel->show();
    myIndexLabelBold->hide();
    // recalc both
    myIndexLabel->recalc();
    myIndexLabelBold->recalc();
}


void 
GNETLSTable::Cell::showIndexLabelBold() {
    myIndexLabel->hide();
    myIndexLabelBold->show();
    // recalc both
    myIndexLabel->recalc();
    myIndexLabelBold->recalc();
}


int 
GNETLSTable::Cell::getCol() const {
    return myCol;
}


int
GNETLSTable::Cell::getRow() const {
    return myRow;
}


char 
GNETLSTable::Cell::getType() const {
    return myTLSTable->myColumns.at(myCol)->getType();
}


GNETLSTable::Cell::Cell() :
    myCol(-1),
    myRow(-1) {
}

// ---------------------------------------------------------------------------
// GNETLSTable::Column - methods
// ---------------------------------------------------------------------------

GNETLSTable::Column::Column(GNETLSTable* table, const int index, const char type) :
    myTable(table),
    myIndex(index),
    myType(type) {
    // create vertical frame
    myVerticalFrame = new FXVerticalFrame(table, GUIDesignAuxiliarTLSTable);
    // create top label
    switch (myType) {
        case 's':
        case 'i':
        case 'd':
        case 't':
        case 'b':
            // empty label
            myTopLabel = new MFXLabelTooltip(myVerticalFrame, "", nullptr, GUIDesignLabelTLSTableEmpty);
            break;
        default:
            // ticked label
            myTopLabel = new MFXLabelTooltip(myVerticalFrame, "", nullptr, GUIDesignLabelTLSTable);
            break;
    }
    // create vertical frame for cells
    myVerticalCellFrame = new FXVerticalFrame(myVerticalFrame, GUIDesignAuxiliarTLSTable);
    // create bot label
    switch (myType) {
        case 's':
            // label with icon
            myBotLabel = new FXLabel(myVerticalFrame, "", GUIIconSubSys::getIcon(GUIIcon::SUM), GUIDesignLabelTLSTable);
            break;
        case 'u':
        case 'p':
            // ticked label
            myBotLabel = new FXLabel(myVerticalFrame, "", nullptr, GUIDesignLabelTLSTable);
            break;
        default:
            // empty label
            myBotLabel = new FXLabel(myVerticalFrame, "", nullptr, GUIDesignLabelTLSTableEmpty);
            break;
    }
    // create elements
    myVerticalFrame->create();
    myTopLabel->create();
    myVerticalCellFrame->create();
    myBotLabel->create();
}


GNETLSTable::Column::~Column() {
    // delete vertical frame (this also delete all childrens)
    delete myVerticalFrame;
}


FXVerticalFrame*
GNETLSTable::Column::getVerticalCellFrame() const {
    return myVerticalCellFrame;
}


char
GNETLSTable::Column::getType() const {
    return myType;
}


void
GNETLSTable::Column::setColumnLabelTop(const std::string& text, const std::string& tooltip) {
    myTopLabel->setText(text.c_str());
    myTopLabel->setTipText(tooltip.c_str());
}


void
GNETLSTable::Column::setColumnLabelBot(const std::string& text) {
    myBotLabel->setText(text.c_str());
}


int
GNETLSTable::Column::getColumnMinimumWidth() {
    // declare columnWidth
    int columnWidth = 0;
    // check column type
    if (myType == 's') {
        // set index column width
        columnWidth = 30;
    } else if (isTextFieldColumn()) {
        // calculate top label width
        columnWidth = myTopLabel->getFont()->getTextWidth(myTopLabel->getText().text(), myTopLabel->getText().length() + EXTRAMARGING);
        // iterate over all textFields and check widths
        for (const auto& row : myTable->myRows) {
            // get text field
            const auto textField = row->getCells().at(myIndex)->getTextField();
            // get textField width
            const auto textFieldWidth = textField->getFont()->getTextWidth(textField->getText().text(), textField->getText().length() + EXTRAMARGING);
            // compare widths
            if (textFieldWidth > columnWidth) {
                columnWidth = textFieldWidth;
            }
        }
        // calculate bot label width
        const auto botLabelWidth = myBotLabel->getFont()->getTextWidth(myBotLabel->getText().text(), myBotLabel->getText().length() + EXTRAMARGING);
        if (botLabelWidth > columnWidth) {
            columnWidth = botLabelWidth;
        }
    } else {
        // is an index column, then return icon size
        columnWidth = GUIDesignHeight;
    }
    return columnWidth;
}


void
GNETLSTable::Column::setColumnWidth(const int colWidth) {
    // only adjust for textField columns
    if (isTextFieldColumn()) {
        for (const auto& row : myTable->myRows) {
            row->getCells().at(myIndex)->getTextField()->setWidth(colWidth);
        }
    }
    // adjust labels and vertical frames
    myVerticalFrame->setWidth(colWidth);
    myTopLabel->setWidth(colWidth);
    myVerticalCellFrame->setWidth(colWidth);
    myBotLabel->setWidth(colWidth);
}


bool
GNETLSTable::Column::isTextFieldColumn() const {
    return ((myType == 'u') || (myType == 'f') || (myType == 'p') || (myType == 'm') || (myType == '-'));
}


GNETLSTable::Column::Column() :
    myIndex(0),
    myType('-') {}

// ---------------------------------------------------------------------------
// GNETLSTable::Row - methods
// ---------------------------------------------------------------------------

GNETLSTable::Row::Row(GNETLSTable* table) :
    myTable(table) {
    // build textFields
    for (int columnIndex = 0; columnIndex < (FXint)table->myColumns.size(); columnIndex++) {
        // get number of cells
        const int numCells = (int)myCells.size();
        // continue depending of type
        switch (table->myColumns.at(columnIndex)->getType()) {
            case ('s'): {
                // create labels for index
                auto indexLabel = new FXLabel(table->myColumns.at(columnIndex)->getVerticalCellFrame(), toString(myTable->myRows.size()).c_str(), nullptr, GUIDesignLabelTLSTableIndex);
                auto indexLabelBold = new FXLabel(table->myColumns.at(columnIndex)->getVerticalCellFrame(), toString(myTable->myRows.size()).c_str(), nullptr, GUIDesignLabelTLSTableIndex);
                // set fonts
                indexLabel->setFont(myTable->myIndexFont);
                indexLabelBold->setFont(myTable->myIndexSelectedFont);
                myCells.push_back(new Cell(table, indexLabel, indexLabelBold, columnIndex, numCells));
                break;
            }
            case ('u'): 
            case ('f'): {
                // create text field for duration or float values
                auto textField = new FXTextField(table->myColumns.at(columnIndex)->getVerticalCellFrame(), GUIDesignTextFieldNCol, table, MID_GNE_TLSTABLE_TEXTFIELD, GUIDesignTextFieldTLSTableReal);
                myCells.push_back(new Cell(table, textField, columnIndex, numCells));
                break;
            }
            case ('p'): {
                // create text field for program (state)
                auto textField = new FXTextField(table->myColumns.at(columnIndex)->getVerticalCellFrame(), GUIDesignTextFieldNCol, table, MID_GNE_TLSTABLE_TEXTFIELD, GUIDesignTextFieldTLSTable);
                // set special font
                textField->setFont(myTable->myProgramFont);
                myCells.push_back(new Cell(table, textField, columnIndex, numCells));
                break;
            }
            case ('m'):
            case ('-'): {
                // create normal text field
                auto textField = new FXTextField(table->myColumns.at(columnIndex)->getVerticalCellFrame(), GUIDesignTextFieldNCol, table, MID_GNE_TLSTABLE_TEXTFIELD, GUIDesignTextFieldTLSTable);
                myCells.push_back(new Cell(table, textField, columnIndex, numCells));
                break;
            }
            case ('i'): {
                // create popup for adding new phases
                myCells.push_back(new Cell(table, columnIndex, numCells));
                break;
            }
            case ('d'): {
                // create button for delete phase
                auto button = new FXButton(table->myColumns.at(columnIndex)->getVerticalCellFrame(), "\t\tDelete this phase.", GUIIconSubSys::getIcon(GUIIcon::REMOVE), table, MID_GNE_TLSTABLE_REMOVEPHASE, GUIDesignButtonIcon);
                myCells.push_back(new Cell(table, button, columnIndex, numCells));
                break;
            }
            case ('t'): {
                // create button for move up phase
                auto button = new FXButton(table->myColumns.at(columnIndex)->getVerticalCellFrame(), "\t\tMove this phase up.", GUIIconSubSys::getIcon(GUIIcon::ARROW_UP), table, MID_GNE_TLSTABLE_MOVEUPPHASE, GUIDesignButtonIcon);
                myCells.push_back(new Cell(table, button, columnIndex, numCells));
                break;
            }
            case ('b'): {
                // create button for move down phase
                auto button = new FXButton(table->myColumns.at(columnIndex)->getVerticalCellFrame(), "\t\tMove this phase down.", GUIIconSubSys::getIcon(GUIIcon::ARROW_DOWN), table, MID_GNE_TLSTABLE_MOVEDOWNPHASE, GUIDesignButtonIcon);
                myCells.push_back(new Cell(table, button, columnIndex, numCells));
                break;
            }
            default:
                throw ProcessError("Invalid Cell type");
        }
    }
}


GNETLSTable::Row::~Row() {
    // destroy all textFields
    for (const auto& cell : myCells) {
        if (cell->getTextField()) {
            cell->getTextField()->destroy();
        } else if (cell->getIndexLabel()) {
            cell->getIndexLabel()->destroy();
        }
        delete cell;
    }
}


std::string
GNETLSTable::Row::getText(int index) const {
    if (myCells.at(index)->getTextField()) {
        return myCells.at(index)->getTextField()->getText().text();
    } else {
        throw ProcessError("Cell doesn't have a textField");
    }
}


void
GNETLSTable::Row::setText(int index, const std::string& text) const {
    // set text
    myCells.at(index)->getTextField()->setText(text.c_str());
}


const std::vector<GNETLSTable::Cell*> &
GNETLSTable::Row::getCells() const {
    return myCells;
}


void 
GNETLSTable::Row::markAsFirstRow() {
    // search move up button and disable it
    for (const auto &cell : myCells) {
        if (cell->getType() == 't') {
            cell->getButton()->disable();
        }
    }
}


void
GNETLSTable::Row::markAsLastRow() {
    // search move up button and disable it
    for (const auto &cell : myCells) {
        if (cell->getType() == 'b') {
            cell->getButton()->disable();
        }
    }
}


void
GNETLSTable::Row::disableRemoveRow() {
    // search move up button and disable it
    for (const auto &cell : myCells) {
        if (cell->getType() == 'd') {
            cell->getButton()->disable();
        }
    }
}


GNETLSTable::Row::Row() {}

/****************************************************************************/
