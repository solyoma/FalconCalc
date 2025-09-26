#include <QStack>
#include <QPair>

#include "calculate.h"

using namespace FalconCalc;

#include "VariablesFunctionsDialog.h"

constexpr int	VARIABLES = 0,
				FUNCTIONS = 1;

const QString qsEgString = "=";
const QString qsCommentDelimiterString = ":";

int VariablesFunctionsDialog::_colW[2][4] = { {80,100,60,300},{50,200,60,300} };

/* ============================ VariablesFunctionsDialog =================*/

VariablesFunctionsDialog::VariablesFunctionsDialog(int initialTabIndex, QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);
	_busy = true;

	_pUserFuncsMap = new RowDataMap();
	_pUserFuncsInMap = new RowDataMap();
	_pUserVarsMap = new RowDataMap();
	_pUserVarsInMap = new RowDataMap();

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
	{
		_pActUserTable = ui.tblUserFuncs;
		_pActStack = &_removedFuncRows;
	}
	else
	{
		_pActUserTable = ui.tblUserVars;
		_pActStack = &_removedVarRows;
	}
	_busy = false;
}

VariablesFunctionsDialog::~VariablesFunctionsDialog()
{
	delete _pUserFuncsMap		;
	delete _pUserFuncsInMap	;
	delete _pUserVarsMap		;
	delete _pUserVarsInMap	;
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

void VariablesFunctionsDialog::closeEvent(QCloseEvent* e)
{
	emit SignalVarFuncClose();
}

void VariablesFunctionsDialog::moveEvent(QMoveEvent* e)
{
	emit SignalVarFuncMoved();
}

void VariablesFunctionsDialog::on_tabHeader_currentChanged(int index)
{
	if (index)
	{
		_pActUserTable = ui.tblUserFuncs;
		_pActStack = &_removedFuncRows;
	}
	else
	{
		_pActUserTable = ui.tblUserVars;
		_pActStack = &_removedVarRows;
	}
	ui.btnRemoveAll->setEnabled(_pActUserTable->rowCount());
	ui.btnRemoveRow->setEnabled(!_pActUserTable->selectedItems().isEmpty());
	ui.btnUndo->setEnabled(index ? _removedFuncRows.size() : _removedVarRows.size());
	qDebug("on_tabHeader_currentChanged");
}

void VariablesFunctionsDialog::on_btnRemoveAll_clicked()
{
	ui.btnUndo->setEnabled(_pActUserTable->rowCount());

	for(int r = 0; r < _pActUserTable->rowCount(); ++r)
	{
		QVector<ElidingTableWidgetItem*> rowItems;
		for (int col = 0; col < _pActUserTable->columnCount(); ++col) {
			rowItems.append(reinterpret_cast<ElidingTableWidgetItem*>(_pActUserTable->takeItem(r, col)) );
		}
		_pActStack->push({ r, rowItems });
		_pActUserTable->removeRow(r);
	}
	qDebug("on_btnRemoveAll_clicked");
}

void VariablesFunctionsDialog::on_btnRemoveRow_clicked()
{
	int r = _pActUserTable->currentRow();	// _pActStack is set in constructor and TAB selection

	// qDebug("on_btnRemoveRow_clicked to remove row #%d", r);
	if (r >= 0)
	{
		QVector<ElidingTableWidgetItem*> rowItems;
		for (int col = 0; col < _pActUserTable->columnCount(); ++col) {
			rowItems.append(reinterpret_cast<ElidingTableWidgetItem*>(_pActUserTable->takeItem(r, col)) );
		}
		_pActStack->push({ r, rowItems });
		_pActUserTable->removeRow(r);
		ui.btnUndo->setEnabled(true);
	}
}

void VariablesFunctionsDialog::on_btnUndo_clicked()
{
	auto lastRemoved = _pActStack->pop();
	int rowIndex = lastRemoved.first;
	QVector<ElidingTableWidgetItem*> rowItems = lastRemoved.second;

	// Insert the row back at its original position
	_pActUserTable->insertRow(rowIndex);
	for (int col = 0; col < rowItems.size(); ++col) {
		_pActUserTable->setItem(rowIndex, col, rowItems[col]);
	}
	ui.btnUndo->setEnabled(_pActStack->size());
	ui.btnRemoveAll->setEnabled(_pActUserTable->rowCount());
}

void VariablesFunctionsDialog::on_btnSave_clicked()
{
	if (!_changed[0] && !_changed[1])		// should never happen (Save button not enabled)
		return;

	if (ActualTab() == VARIABLES)
		_CollectFrom(VARIABLES);	// changed FUNCTIONs already collected in tcVarsTabChanged()
	else if (ActualTab() == FUNCTIONS)
		_CollectFrom(FUNCTIONS);	// changed VARIABLEs -"-

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
	int row = _pActUserTable->rowCount();
	_pActUserTable->setRowCount(row + 1);
	for (int col = 0; col < _pActUserTable->columnCount(); ++col)
	{
		ElidingTableWidgetItem* pwi = new ElidingTableWidgetItem("");
		_pActUserTable->setItem(row, col, pwi);
	}
	qDebug("on_btnAddRow_clicked - row #%d added", row);
}

void VariablesFunctionsDialog::on_tblUserVars_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	// DEBUG
	int rowp = previous ? previous->row() : -1,
		colp = previous ? previous->column() : -1,
		rowc = current ? current->row() : -1,
		colc = current ? current->column() : -1;
	qDebug("on_tblUserVars_currentItemChanged from (%d,%d) to (%d,%d)", rowp, colp, rowc, colc);
	// /DEBUG
	_sTmp = ui.tblUserVars->item(rowc, colc)->text();
}

void VariablesFunctionsDialog::on_tblUserVars_cellDoubleClicked(int row, int col)
{
	_sTmp = ui.tblUserVars->item(row, col)->text();
}

void VariablesFunctionsDialog::on_tblUserFuncs_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	// DEBUG
	int rowp = previous ? previous->row() : -1,
		colp = previous ? previous->column() : -1,
		rowc = current ? current->row() : -1,
		colc = current ? current->column() : -1;
	qDebug("on_tblUserFuncs_currentItemChanged from (%d,%d) to (%d,%d)", rowp, colp, rowc, colc);
	// /DEBUG
}

void VariablesFunctionsDialog::on_tblUserVars_cellChanged(int row, int col)
{
	if (_busy)
		return;
	QString qsCellData = ui.tblUserVars->item(row, col)->text();
	_changed[VARIABLES] = _changed[VARIABLES] || _sTmp != qsCellData;
	qDebug("on_tblUserVars_cellChanged at (%d,%d) _changed: %s", row, col,_changed ? "yes" :"no");
	ui.btnSave->setEnabled(_changed);
}

void VariablesFunctionsDialog::on_tblUserFuncs_cellChanged(int row, int col)
{
	if (_busy)
		return;
	QString qsCellData = ui.tblUserVars->item(row, col)->text();
	_changed[FUNCTIONS] = _changed[FUNCTIONS] || _sTmp != qsCellData;
	ui.btnSave->setEnabled(_changed);
}

void VariablesFunctionsDialog::on_tblUserFuncs_cellDoubleClicked(int row, int col)
{
}


void VariablesFunctionsDialog::_FillBuiltinFuncTable()
{
	ElidingTableWidgetItem* ptw;
	size_t pos = 0, pos1 = 0;
	QString qs;
	int cntFunctions = LittleEngine::builtinFunctions.size();
	ui.tblBuiltinFuncs->setRowCount(cntFunctions);

	for (int row = 0; row < cntFunctions; ++row)
	{
		BuiltinFunc& bif = LittleEngine::builtinFunctions[row];

		ptw = new ElidingTableWidgetItem(bif.name.toQString());	
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
		_AddCellText(ui.tblUserFuncs, row, 0, f.name.toQString());
		_AddCellText(ui.tblUserFuncs, row, 1, f.body.toQString());
		_AddCellText(ui.tblUserFuncs, row, 2, f.unit.toQString());
		_AddCellText(ui.tblUserFuncs, row, 3, f.desc.toQString());
	}
	_pUserFuncsInMap = _pUserFuncsMap;
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
	int row = 0;
	DisplayFormat df;
	df.nFormatSwitchFracLength = 3;
	df.nFormatSwitchIntLength = 15;
	df.strThousandSeparator = " ";
	for (auto &bi: builtIns)
	{
		_AddCellText(ui.tblBuiltinVars, row, 0, bi.second->name.toQString());
		_AddCellText(ui.tblBuiltinVars, row, 1, bi.second->value.ToDecimalString(df).toQString());
		_AddCellText(ui.tblBuiltinVars, row, 2, bi.second->unit.toQString());
		_AddCellText(ui.tblBuiltinVars, row, 3, bi.second->desc.toQString());
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
	_pUserVarsInMap = _pUserVarsMap;

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
	RowDataMap  * pDmIn = table ? _pUserFuncsInMap : _pUserVarsInMap,
				* pDm = table ? _pUserFuncsMap : _pUserVarsMap;
	QTableWidget* ptw = table ? ui.tblUserVars : ui.tblUserFuncs;

	RowData rd;
	// collect data from table into the non-input variables
	pDm->clear();
	for (int row = 1; row < ptw->rowCount(); ++row)
	{
		if (!ptw->item(row,0)->text().isEmpty() && !ptw->item(row, 1)->text().isEmpty())
		{
			rd.cols[0] = SmartString(ptw->item(row, 1)->text());		// name
			rd.cols[1] = SmartString(ptw->item(row, 1)->text());		// body/definition
			rd.cols[2] = SmartString(ptw->item(row, 1)->text());		// unit
			rd.cols[3] = SmartString(ptw->item(row, 1)->text());		// description
		}
		(*pDm)[rd.cols[0]] = rd;			// actual is always set
	}

	// next check for changes
	if (pDm->size() != pDmIn->size())	// data deleted or added
		_changed[table] = true;
	else for (int i = 1; i < pDm->size(); ++i)
		if ((*pDm)[i] != (*pDmIn)[i])
		{
			_changed[table] = true;
			break;
		}
	ui.btnSave->setEnabled(_changed[0] || _changed[1]);
}

void VariablesFunctionsDialog::SlotSelectTab(int tab)
{
		ui.tabHeader->setCurrentIndex(tab);
}
