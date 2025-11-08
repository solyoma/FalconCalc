#include <QStack>
#include <QPair>

#include "calculate.h"

using namespace FalconCalc;

#include "VarFuncdefDialog.h"
#include "VariablesFunctionsDialog.h"

constexpr const int	VARIABLES = 0,
					FUNCTIONS = 1,
					MIN_ROW_COUNT = 10;

const QString qsEgString = "=";
const QString qsCommentDelimiterString = ":";

int VariablesFunctionsDialog::_colW[2][4] = { {80,100,60,300},{50,200,60,300} };

/* ============================ VariablesFunctionsDialog =================*/

VariablesFunctionsDialog::VariablesFunctionsDialog(int initialTabIndex, QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);
	_busy = true;
		// map									contains 
	_pUserFuncsMap = new RowDataMap();		// actual functions
	_pUserFuncsInMap = new RowDataMap();	// input functions
	_pUserVarsMap = new RowDataMap();		// actual variables
	_pUserVarsInMap = new RowDataMap();		// input  variables

	_FillVarTables();
	_FillFuncTables();

	// Enable the button if any rows are selected button->setEnabled(!tableWidget->selectedItems().isEmpty()); })
	QToolButton* pbtn = ui.btnRemoveRow;
	connect(ui.tblUserVars->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, pbtn]() {
		pbtn->setEnabled(!ui.tblUserVars->selectedItems().isEmpty());
		});
	connect(ui.tblUserFuncs->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, pbtn]() {
		pbtn->setEnabled(!ui.tblUserFuncs->selectedItems().isEmpty());
		});
	/*----------------------------------------------------------*/
	// connect sectionResized signal to update the item when the column is resized
	//auto resFunc = [this](int index, int, int newSize)
	//	{
	//		for (int row = 0; row < tableWidget().rowCount(); ++row) {
	//			auto item = table.item(row, index);
	//			if (item) {
	//				table.update(item->row(), item->column());  // Trigger a repaint to update the elided text
	//			}
	//		}
	//	};

	connect(ui.tblUserVars->horizontalHeader(),		&QHeaderView::sectionResized, [this](int index, int, int newSize)
		{
			for (int row = 0; row < ui.tblUserVars->rowCount(); ++row) {
				auto item = ui.tblUserVars->item(row, index);
				if (item) {
					ui.tblUserVars->update();  // Trigger a repaint to update the elided text
				}
			}
		}
	);
//	connect(ui.tblUserFuncs->horizontalHeader(),		&QHeaderView::sectionResized, resFunc);
	connect(ui.tblBuiltinVars->horizontalHeader(), &QHeaderView::sectionResized, [this](int index, int, int newSize)
		{
			for (int row = 0; row < ui.tblBuiltinVars->rowCount(); ++row) {
				auto item = ui.tblBuiltinVars->item(row, index);
				if (item) {
					ui.tblBuiltinVars->update();  // Trigger a repaint to update the elided text
				}
			}
		}
	);
//	connect(ui.tblBuiltinFuncs->horizontalHeader(),	&QHeaderView::sectionResized, resFunc);
	/*----------------------------------------------------------*/

	if (initialTabIndex)
		_pActUserTable = ui.tblUserFuncs;
	else
		_pActUserTable = ui.tblUserVars;

	ui.tabHeader->setCurrentIndex(initialTabIndex);
	_busy = false;
}

VariablesFunctionsDialog::~VariablesFunctionsDialog()
{
	delete _pUserFuncsMap	; _pUserFuncsMap = nullptr;
	delete _pUserFuncsInMap	; _pUserFuncsInMap = nullptr;
	delete _pUserVarsMap	; _pUserVarsMap = nullptr;
	delete _pUserVarsInMap	; _pUserVarsInMap = nullptr;
	emit SignalVarFuncClose();
}

void VariablesFunctionsDialog::SlotSetColWidths(int which, int cw1, int cw2, int cw3, int cw4)
{
	QTableWidget* ptw = which ? ui.tblBuiltinVars : ui.tblUserVars;

	auto setw = [&]()
		{
			ptw->setColumnWidth(0, cw1);
			ptw->setColumnWidth(0, cw2);
			ptw->setColumnWidth(0, cw3);
			ptw->setColumnWidth(0, cw4);
		};

	setw();

	ptw = which ? ui.tblBuiltinFuncs : ui.tblUserFuncs;
	setw();
}

void VariablesFunctionsDialog::moveEvent(QMoveEvent* e)
{
	emit SignalVarFuncMoved();
}

void VariablesFunctionsDialog::on_tabHeader_currentChanged(int index)
{
	if (index)
		_pActUserTable = ui.tblUserFuncs;
	else
		_pActUserTable = ui.tblUserVars;
	_EnableButtons();
//	qDebug("on_tabHeader_currentChanged()");
}

void VariablesFunctionsDialog::on_btnRemoveAll_clicked()	// only removes rows with data
{
	int cnt = _pActUserTable->rowCount();
	UndoItem undoItem;
	undoItem.table = ActualTab();

	for (int r = 0; r < cnt; ++r)
	{
		QVector<ElidingTableWidgetItem* > columnData;
		for (int col = 0; col < _pActUserTable->columnCount(); ++col) 
			if (!_GetItemText(_pActUserTable, r, col).isEmpty())  // only delete valid data, not empty space
				columnData.push_back(reinterpret_cast<ElidingTableWidgetItem*>(_pActUserTable->item(r, col)));

		if (columnData.size())
			undoItem.data.push_back({ r,columnData });
	
	}
	if (undoItem.IsValid())
		_undoStack.push(undoItem);

	_pActUserTable->setRowCount(0);	// clear table
	_pActUserTable->setRowCount(MIN_ROW_COUNT);// and set empty rows
	_EnableButtons();
	qDebug("on_btnRemoveAll_clicked");
}

void VariablesFunctionsDialog::on_btnRemoveRow_clicked() // selected rows
{
	UndoItem undoItem;
	undoItem.table = ActualTab();

	auto selection = _pActUserTable->selectedRanges();	// only single range is allowed

	for (int r = selection.at(0).topRow(); r <= selection.at(0).bottomRow(); ++r)
	{
		if (!_GetItemText(_pActUserTable, r, 0).isEmpty())
		{
			QVector<ElidingTableWidgetItem*> columnData; // each item points to an existing ElidingTableWidgetItem
			for (int col = 0; col < _pActUserTable->columnCount(); ++col)
			{
				qDebug("  item (%d, %d) = %s", r, col, _pActUserTable->item(r, col)->text().toStdString().c_str());
				columnData.append(reinterpret_cast<ElidingTableWidgetItem*>(_pActUserTable->takeItem(r, col)));
			}
			undoItem.data.push_back({ r, columnData });
		}
	}
	if (undoItem.IsValid())
	{
		_undoStack.push(undoItem);
		for (int r = selection.at(0).bottomRow(); r >= selection.at(0).topRow(); --r)
			_pActUserTable->removeRow(r);

		if (_pActUserTable->rowCount() < MIN_ROW_COUNT)
			_pActUserTable->setRowCount(MIN_ROW_COUNT); // keep some empty rows
	}
	_EnableButtons();
}

void VariablesFunctionsDialog::on_btnUndo_clicked()
{
	if (_undoStack.isEmpty())	// should never occur (button disabled)
		return;

	UndoItem lastRemoved = _undoStack.pop();
	if(lastRemoved.table != ActualTab())
		ui.tabHeader->setCurrentIndex(lastRemoved.table);
	int cnt = lastRemoved.data.size();
	for(int i = 0; i < cnt; ++i)
	{
		int rowIndex = lastRemoved.data.at(i).first;
		QVector<ElidingTableWidgetItem*> rowItems = lastRemoved.data.at(i).second;

		// Insert the row back at its original position
		_pActUserTable->insertRow(rowIndex);
		for (int col = 0; col < rowItems.size(); ++col) {
			_pActUserTable->setItem(rowIndex, col, rowItems[col]);
		}
	}
	_EnableButtons();
	if (_cntRowsWithData[ActualTab()] < _pActUserTable->rowCount())	// empty rows at end
	{
		_pActUserTable->setRowCount(_cntRowsWithData[ActualTab()]);
		if (_pActUserTable->rowCount() < MIN_ROW_COUNT)
			_pActUserTable->setRowCount(MIN_ROW_COUNT); // keep some empty rows
	}

}

void VariablesFunctionsDialog::on_btnCancel_clicked()
{
	emit SignalVarFuncClose();
}

void VariablesFunctionsDialog::on_btnSave_clicked()
{
	if (ActualTab() == VARIABLES)
		_CollectFrom(VARIABLES);	// changed FUNCTIONs already collected in tcVarsTabChanged()
	else if (ActualTab() == FUNCTIONS)
		_CollectFrom(FUNCTIONS);	// changed VARIABLEs  already collected in tcVarsTabChanged()

	SmartString s;
	SmartStringVector sv;
	if (_changed[VARIABLES])
	{
		lengine->variables.clear();
		for (int i = 0; i < _pUserVarsMap->size(); ++i)
		{
			s = (*_pUserVarsMap)[i].Serialize();
			sv.push_back(s);
		}
		lengine->AddUserVariables(sv);
	}
	if (_changed[FUNCTIONS])
	{
		lengine->functions.clear();
		for (int i = 0; i < _pUserFuncsMap->size(); ++i)
		{
			s = (*_pUserFuncsMap)[i].Serialize();
			sv.push_back(s);
		}
		lengine->AddUserFunctions(sv);
	}

	ui.btnSave->setEnabled(false); //  _changed[] = false);

	lengine->clean = _changed[0] = _changed[1] = false;			
	if(lengine->SaveUserData())
		lengine->LoadUserData();
}

void VariablesFunctionsDialog::on_btnAddRow_clicked()
{
	_GetActualDataCount(ActualTab());
	int rowCnt = _pActUserTable->rowCount();
	if (_cntRowsWithData[ActualTab()] == rowCnt) // then add data row
	{
		_pActUserTable->setRowCount(rowCnt + 1);
		for (int col = 0; col < _pActUserTable->columnCount(); ++col)
		{
			ElidingTableWidgetItem* pwi = new ElidingTableWidgetItem("");
			_pActUserTable->setItem(rowCnt, col, pwi);
		}
		qDebug("on_btnAddRow_clicked - row #%d added", rowCnt);
	}
	int row = _cntRowsWithData[ActualTab()];
	if (ActualTab() ==  FUNCTIONS)
	{
		ui.tblUserFuncs->selectRow(row);
		ui.tblUserFuncs->setFocus();
		on_tblUserFuncs_cellDoubleClicked(row, 1);
	}
	else
	{
		ui.tblUserVars->setFocus();
		ui.tblUserVars->selectRow(row);
		on_tblUserVars_cellDoubleClicked(_cntRowsWithData[VARIABLES], 1);
	}
}

void VariablesFunctionsDialog::on_tblUserVars_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	// DEBUG
	int rowp = previous ? previous->row() : -1,
		colp = previous ? previous->column() : -1,
		rowc = current ? current->row() : -1,
		colc = current ? current->column() : -1;
	//qDebug("on_tblUserVars_currentItemChanged from (%d,%d) to (%d,%d)", rowp, colp, rowc, colc);
	// /DEBUG
	_GetItemText(ui.tblUserVars,rowc, colc);
}

void VariablesFunctionsDialog::_UserTableEditCommon(QTableWidget* ptw, int row, int col)
{
	VarFuncData vfd;  
	vfd.name = _GetItemText(ptw, row, 0);
	vfd.body = _GetItemText(ptw, row, 1);
	vfd.unit = _GetItemText(ptw, row, 2);
	vfd.comment = _GetItemText(ptw, row, 3);
	vfd.isFunction = (ptw == ui.tblUserFuncs);
	VarFuncDefDialog vfdDialog(vfd, this);
	if(vfdDialog.exec() == QDialog::Accepted)
	{
		_changed[vfd.isFunction] |= (vfd.name != _GetItemText(ptw, row, 0)) ||
									(vfd.body != _GetItemText(ptw, row, 1)) ||
									(vfd.unit != _GetItemText(ptw, row, 2)) ||
									(vfd.comment != _GetItemText(ptw, row, 3));
		_AddCellText(ptw, row, 0, vfd.name);
		_AddCellText(ptw, row, 1, vfd.body);
		_AddCellText(ptw, row, 2, vfd.unit);
		_AddCellText(ptw, row, 3, vfd.comment);

		ui.btnSave->setEnabled(_changed[0] || _changed[1]);
	}
}

void VariablesFunctionsDialog::on_tblUserVars_cellDoubleClicked(int row, int col)
{
	if (col == 0) // only name column
		_TableDoubleClickedCommon(ui.tblUserVars,row,col);
	else
		_UserTableEditCommon(ui.tblUserVars,row,col);
}

void VariablesFunctionsDialog::on_tblUserFuncs_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	// DEBUG
	int rowp = previous ? previous->row() : -1,
		colp = previous ? previous->column() : -1,
		rowc = current ? current->row() : -1,
		colc = current ? current->column() : -1;
	//qDebug("on_tblUserFuncs_currentItemChanged from (%d,%d) to (%d,%d)", rowp, colp, rowc, colc);
	// /DEBUG
}

void VariablesFunctionsDialog::on_tblUserVars_cellChanged(int row, int col)
{
	if (_busy)
		return;
	QString qsCellData = _GetItemText(ui.tblUserVars, row, col);
	_changed[VARIABLES] = _changed[VARIABLES] || _sTmp != qsCellData;
	_EnableButtons();
	//qDebug("on_tblUserVars_cellChanged at (%d,%d) _changed: %s", row, col,_changed ? "yes" :"no");
}

void VariablesFunctionsDialog::on_tblUserFuncs_cellChanged(int row, int col)
{
	if (_busy)
		return;
	
	QString qsCellData = _GetItemText(ui.tblUserVars, row, col);
	_changed[FUNCTIONS] = _changed[FUNCTIONS] || _sTmp != qsCellData;
	_EnableButtons();
}

void VariablesFunctionsDialog::on_tblUserFuncs_cellDoubleClicked(int row, int col)
{
	if (col == 0) // only name column
		_TableDoubleClickedCommon(ui.tblUserFuncs, row, col);
	else
		_UserTableEditCommon(ui.tblUserFuncs,row,col);
}

void VariablesFunctionsDialog::_TableDoubleClickedCommon(QTableWidget* ptw, int row, int col)
{
	   QString name = _GetItemText(ptw, row, 0);
	   if ((ptw == ui.tblBuiltinFuncs) || (ptw == ui.tblUserFuncs))
		   name = name.mid(0, name.indexOf('(')+1); // remove arguments)

	   emit SignalTableDoubleClicked(name);
}

void VariablesFunctionsDialog::on_tblBuiltinVars_cellDoubleClicked(int row, int col)
{
	_TableDoubleClickedCommon(ui.tblBuiltinVars, row, col);
}

void VariablesFunctionsDialog::on_tblBuiltinFuncs_cellDoubleClicked(int row, int col)
{
	_TableDoubleClickedCommon(ui.tblBuiltinFuncs, row, col);
}


QString VariablesFunctionsDialog::_GetItemText(QTableWidget *table, int row, int col)
{
	QTableWidgetItem *pItem = table->item(row, col);
	return (_sTmp = pItem ? reinterpret_cast<ElidingTableWidgetItem*>(pItem)->Text() : "");
}

void VariablesFunctionsDialog::_FillBuiltinFuncTable()
{
	ElidingTableWidgetItem* ptw;
	size_t pos = 0, pos1 = 0;
	QString qs;
	int cntFunctions = LittleEngine::builtinFunctions.size();
	ui.tblBuiltinFuncs->setRowCount(cntFunctions);

	auto argStr = [](BuiltinFunc& f)
		{
			if (f.funct1) return QString("(x)");
			return QString("(x,y)");
		};
	for (int row = 0; row < cntFunctions; ++row)
	{
		BuiltinFunc& bif = LittleEngine::builtinFunctions[row];

		ptw = new ElidingTableWidgetItem(bif.name.toQString() + argStr(bif));	
		ui.tblBuiltinFuncs->setItem(row, 0, ptw);
		ptw = new ElidingTableWidgetItem("-");
		ui.tblBuiltinFuncs->setItem(row, 1, ptw);
		ptw = new ElidingTableWidgetItem("-");
		ui.tblBuiltinFuncs->setItem(row, 2, ptw);
		ptw = new ElidingTableWidgetItem(bif.desc.toQString());	
		ui.tblBuiltinFuncs->setItem(row, 3, ptw);
	}
}

void VariablesFunctionsDialog::_FillUserFuncTable()
{
	if(ui.tblUserFuncs->rowCount() < LittleEngine::functions.size())
		ui.tblUserFuncs->setRowCount(LittleEngine::functions.size());
	RowData rd;
	for (int row = 0; row < LittleEngine::functions.size(); ++row)
	{
		Func& f = LittleEngine::functions[row];
		rd = { f.name, f.body, f.unit, f.desc };
		(*_pUserFuncsMap)[f.name] = rd;
		QString s = f.name.toQString()+QString("(");
		size_t i;
		for (i = 0; i < f.args.size() - 1; ++i)
			s += f.args.at(i).toQString() + ",";
		s += f.args.at(i).toQString() + ")";

		_AddCellText(ui.tblUserFuncs, row, 0, s);
		_AddCellText(ui.tblUserFuncs, row, 1, f.body.toQString());
		_AddCellText(ui.tblUserFuncs, row, 2, f.unit.toQString());
		_AddCellText(ui.tblUserFuncs, row, 3, f.desc.toQString());
	}
	*_pUserFuncsInMap = *_pUserFuncsMap;
}

void VariablesFunctionsDialog::_FillFuncTables()   
{
	_FillUserFuncTable();
	_FillBuiltinFuncTable();
}

void VariablesFunctionsDialog::_FillBuiltinVarTable()
{
	QString qs;
	LongNumber::ConstantsMap builtIns;
	ui.tblBuiltinVars->setRowCount(builtIns.size());
	DisplayFormat df;
	df.nFormatSwitchFracLength = 3;
	df.nFormatSwitchIntLength = 15;
	df.strThousandSeparator = " ";
	int row = 0;
	for (auto &bi: builtIns)
	{
		_AddCellText(ui.tblBuiltinVars, row, 0, bi.second->name.toQString());
		_AddCellText(ui.tblBuiltinVars, row, 1, bi.second->value.ToDecimalString(df).toQString());
		_AddCellText(ui.tblBuiltinVars, row, 2, bi.second->unit.toQString());
		_AddCellText(ui.tblBuiltinVars, row, 3, bi.second->desc.toQString());
		++row;
	}
}

void VariablesFunctionsDialog::_FillUserVarTable()
{
	if(ui.tblUserVars->rowCount() < LittleEngine::variables.size())
		ui.tblUserVars->setRowCount(LittleEngine::variables.size());

	RowData rd;
	for (int row = 0; row < LittleEngine::variables.size(); ++row)
	{
		Variable& v = LittleEngine::variables[row];
		rd = { v.name, v.body, v.unit, v.desc };
		(*_pUserVarsMap)[v.name] = rd;
		_AddCellText(ui.tblUserVars, row, 0, v.name.toQString());
		_AddCellText(ui.tblUserVars, row, 1, v.body.toQString());
		_AddCellText(ui.tblUserVars, row, 2, v.unit.toQString());
		_AddCellText(ui.tblUserVars, row, 3, v.desc.toQString());
	}
	*_pUserVarsInMap = *_pUserVarsMap;

}

void VariablesFunctionsDialog::_FillVarTables()
{
	_busy = true;
	_FillUserVarTable();
	_FillBuiltinVarTable();
	_busy = false;
}

//void VariablesFunctionsDialog::_Serialize(VarFuncInfoQt* pvf)
//{
//	if (_busy || (!pvf && !_changed) )    // 'changed' must be modified in caller when necessary
//		return;
//	if (!pvf)
//		pvf = &_vf;
//	auto _serialize = [this](int which, VarFuncInfoQt * pvf)
//		{
//			QTableWidget* pw = which ? ui.tblUserFuncs : ui.tblUserVars;
//			QString& qs = which ? _vf.sUserFuncs : _vf.sUserVars;
//			unsigned& cnt = (which ? pvf->uUserFuncCnt : pvf->uUserVarCnt);
//			int n = 0; // count of user variables/functions
//			qs.clear();
//			for (int i = 0; i < pw->rowCount(); ++i)
//			{
//				if (!pw->item(i, 0)->text().isEmpty() && !pw->item(i, 1)->text().isEmpty())
//				{
//					qs += pw->item(i, 0)->text() + qsCommentDelimiterString + pw->item(i, 1)->text();
//					// unit (even when empty)
//					qs += qsCommentDelimiterString + pw->item(i, 3)->text();
//					// description / body
//					qs += qsCommentDelimiterString + pw->item(i, 2)->text() + "\n";
//					++n;
//				}
//			}
//			cnt = n;
//		};
//	_serialize(0, pvf);
//	_serialize(1, pvf);
//}

void VariablesFunctionsDialog::_AddCellText(QTableWidget* ptw, int row, int col, QString text)
{
	ElidingTableWidgetItem* ptwi;
	QString qsHint = text;
	ptwi = new ElidingTableWidgetItem(text);
	ptwi->setToolTip(qsHint);
	_busy = true;
	ptw->setItem(row, col, ptwi);
	_busy = false;
}

void VariablesFunctionsDialog::_CollectFrom(int table)
{
	RowDataMap  * pDmIn = table ?	_pUserFuncsInMap	: _pUserVarsInMap,
				* pDm = table ?		_pUserFuncsMap		: _pUserVarsMap;
	QTableWidget* ptw = table ?		ui.tblUserFuncs		: ui.tblUserVars;

	RowData rd;
	// collect data from table into the actual (non-input) variables
	pDm->clear();
	auto cellText = [&](int row, int col) -> QString
		{
			return reinterpret_cast<ElidingTableWidgetItem*>(ptw->item(row, col))->Text();
		};
	for (int row = 0; row < ptw->rowCount(); ++row)
	{
		if (!_GetItemText(ptw,row,0).isEmpty() && !_GetItemText(ptw, row, 1).isEmpty())
		{								// text() may be the elided text, toolTip is the whole text
			rd.cols[0] = SmartString(cellText(row, 0));		// name
			rd.cols[1] = SmartString(cellText(row, 1));		// body/definition
			rd.cols[2] = SmartString(cellText(row, 2));		// unit
			rd.cols[3] = SmartString(cellText(row, 3));		// description
			(*pDm)[rd.cols[0]] = rd;			// actual is always set
		}
	}

	// next check for changes
	if (pDm->size() != pDmIn->size())	// data deleted or added
		_changed[table] = true;
	else for (int i = 0; i < pDm->size(); ++i)
		if ((*pDm)[i] != (*pDmIn)[i])
		{
			_changed[table] = true;
			break;
		}
	ui.btnSave->setEnabled(_changed[0] || _changed[1]);
}

void VariablesFunctionsDialog::_EnableButtons()
{
	ui.btnSave->setEnabled(_changed[VARIABLES] || _changed[FUNCTIONS] || _undoStack.size());
		
	(void)_GetActualDataCount(-1);
	ui.btnRemoveAll->setEnabled(_cntRowsWithData[ActualTab()]);

	ui.btnRemoveRow->setEnabled(!_pActUserTable->selectedItems().isEmpty() && 
		                        !_pActUserTable->selectedItems()[0]->text().isEmpty());
	ui.btnUndo->setEnabled(_undoStack.size());

}

bool VariablesFunctionsDialog::_GetActualDataCount(int index)
{
	auto GetCnt = [](QTableWidget* ptw)
		{
			int cnt = 0;
			for (int r = 0; r < ptw->rowCount(); ++r)
				if (ptw->item(r, 0) != nullptr && !ptw->item(r, 0)->text().isEmpty())
					++cnt;
			return cnt;
		};

	if (index == VARIABLES || index < 0)
			_cntRowsWithData[VARIABLES] = GetCnt(ui.tblUserVars);
	if (index == FUNCTIONS || index < 0)
			_cntRowsWithData[FUNCTIONS] = GetCnt(ui.tblUserFuncs);
	return index == VARIABLES ?
			_cntRowsWithData[VARIABLES] != 0:
					index == FUNCTIONS ? _cntRowsWithData[FUNCTIONS] != 0 :
						(_cntRowsWithData[VARIABLES] + _cntRowsWithData[FUNCTIONS]) != 0;
}

void VariablesFunctionsDialog::SlotSelectTab(int tab)
{
		ui.tabHeader->setCurrentIndex(tab);
}
