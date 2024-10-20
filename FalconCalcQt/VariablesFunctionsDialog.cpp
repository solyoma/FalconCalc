#include <QStack>
#include <QPair>

#include "SmartString.h"
using namespace SmString;
#include "LongNumber.h"
using namespace LongNumber;
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
	pUserVars = ui.tblUserVars;
	pBuiltinVars = ui.tblBuiltinVars;
	pUserFuncs = ui.tblUserFuncs;
	pBuiltinFuncs = ui.tblBuiltinFuncs;

	_rvUserFuncs = new RowDataMap();
	_rvUserFuncsIn = new RowDataMap();
	_rvUserVars = new RowDataMap();
	_rvUserVarsIn = new RowDataMap();

	_FillVarTables();
	_FillFuncTables();

	// Enable the button if any rows are selected button->setEnabled(!tableWidget->selectedItems().isEmpty()); })
	QToolButton* pbtn = ui.btnRemoveRow;
	connect(pUserVars->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, pbtn]() {
		pbtn->setEnabled(!pUserVars->selectedItems().isEmpty());
		});
	connect(pUserFuncs->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, pbtn]() {
		pbtn->setEnabled(!pUserFuncs->selectedItems().isEmpty());
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

	connect(pUserVars->horizontalHeader(),		&QHeaderView::sectionResized, [this](int index, int, int newSize)
		{
			for (int row = 0; row < pUserVars->rowCount(); ++row) {
				auto item = pUserVars->item(row, index);
				if (item) {
					pUserVars->update();  // Trigger a repaint to update the elided text
				}
			}
		}
	);
//	connect(pUserFuncs->horizontalHeader(),		&QHeaderView::sectionResized, resFunc);
	connect(pBuiltinVars->horizontalHeader(), &QHeaderView::sectionResized, [this](int index, int, int newSize)
		{
			for (int row = 0; row < pBuiltinVars->rowCount(); ++row) {
				auto item = pBuiltinVars->item(row, index);
				if (item) {
					pBuiltinVars->update();  // Trigger a repaint to update the elided text
				}
			}
		}
	);
//	connect(pBuiltinFuncs->horizontalHeader(),	&QHeaderView::sectionResized, resFunc);
	/*----------------------------------------------------------*/

	if (initialTabIndex)
	{
		_pActUserTable = pUserFuncs;
		_pActStack = &_removedFuncRows;
	}
	else
	{
		_pActUserTable = pUserVars;
		_pActStack = &_removedVarRows;
	}
	_busy = false;
}

VariablesFunctionsDialog::~VariablesFunctionsDialog()
{
	delete _rvUserFuncs		;
	delete _rvUserFuncsIn	;
	delete _rvUserVars		;
	delete _rvUserVarsIn	;
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
		_pActUserTable = pUserFuncs;
		_pActStack = &_removedFuncRows;
	}
	else
	{
		_pActUserTable = pUserVars;
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
		for (int i = 0; i < _rvUserVars->size(); ++i)
		{
			s = (*_rvUserVars)[i].Serialize();
			sv.push_back(s);
		}
		lengine->AddUserVariables(sv);
	}
	if (_changed[FUNCTIONS])
	{
		lengine->functions.clear();
		for (int i = 0; i < _rvUserFuncs->size(); ++i)
		{
			s = (*_rvUserFuncs)[i].Serialize();
			sv.push_back(s);
		}
		lengine->AddUserFunctions(sv);
	}

	ui.btnSave->setEnabled(false); //  _changed[] = false);

	lengine->clean = _changed[0] = _changed[1] = false;			
	emit SignalVarFuncSaved();
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
	QStringList sl;
	ElidingTableWidgetItem* ptw;
	size_t pos = 0, pos1;
	QString qs;
	int cntFunctions = LittleEngine::builtinFunctions.size();
	ui.tblBuiltinFuncs->setRowCount(cntFunctions);

	for (size_t row = 0; row < cntFunctions; ++row)
	{
		BuiltinFunc& bif = LittleEngine::builtinFunctions[row];

		ptw = new ElidingTableWidgetItem(bif.name.toQString());	
		ui.tblBuiltinFuncs->setItem(row, 0, ptw);
		ptw = new ElidingTableWidgetItem("-");
		ui.tblBuiltinFuncs->setItem(row, 1, ptw);
		ptw = new ElidingTableWidgetItem("-");
		ui.tblBuiltinFuncs->setItem(row, 2, ptw);
		ptw = new ElidingTableWidgetItem(sl[1]);	
		ui.tblBuiltinFuncs->setItem(row, 3, bif.desc.toQString());
	}
}

void VariablesFunctionsDialog::_FillUserFuncTable()
{
	if(ui.tblUserFuncs->rowCount() < LittleEngine::functions.size())
		ui.tblUserFuncs->setRowCount(LittleEngine::functions.size());
	RowData rd;
	for (size_t row = 0; row < LittleEngine::functions.size(); ++row)
	{
		Func& f = LittleEngine::functions[row];
		rd = { f.name, f.body, f.unit, f.desc };
		_rvUserFuncs[f->name] = rd;
		_AddCellText(ui.tblUserFuncs, row, 0, f.name.toQString());
		_AddCellText(ui.tblUserFuncs, row, 1, f.body.toQString());
		_AddCellText(ui.tblUserFuncs, row, 2, f.unit.toQString());
		_AddCellText(ui.tblUserFuncs, row, 3, f.desc.toQString());
	}
	_rvUserFuncsIn = _rvUserFuncs;
}

void VariablesFunctionsDialog::_FillFuncTables()   
{
	_FillUserFuncTable();
	_FillBuiltinFuncTable();
}

void VariablesFunctionsDialog::_FillBuiltinVarTable()
{
	QStringList sl;
	size_t pos = 0, pos1;
	QString qs;
	ui.tblBuiltinVars->setRowCount(_vf.uBuiltinVarCnt);

	for (size_t row = 0; row < _vf.uBuiltinVarCnt; ++row)
	{
		pos1 = _vf.sBuiltinVars.indexOf(QChar('\n'), pos);
		sl = _vf.sBuiltinVars.mid(pos, pos1 - pos).split(qsCommentDelimiterString,Qt::SkipEmptyParts);
		pos = pos1 + 1;

		_AddCellText(ui.tblBuiltinVars, row, 0, sl[0]);
		_AddCellText(ui.tblBuiltinVars, row, 1, sl[1]);
		_AddCellText(ui.tblBuiltinVars, row, 2, sl[2]);
		_AddCellText(ui.tblBuiltinVars, row, 3, sl[3]);
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
		_rvUserVars[v->name] = rd;
		_AddCellText(ui.tblUserVars, row, 0, v.name.toQString());
		_AddCellText(ui.tblUserVars, row, 1, v.body.toQString());
		_AddCellText(ui.tblUserVars, row, 2, v.unit.toQString());
		_AddCellText(ui.tblUserVars, row, 3, v.desc.toQString());
	}
	_rvUserVarsIn = _rvUserVars;

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
	RowDataMap& rvIn = table ? _rvUserFuncsIn : _rvUserVarsIn,
		& rv = table ? _rvUserFuncs : _rvUserVars;
	QTableWidget* ptw = table ? pUserVars : pUserFuncs;

	RowData rd;
	// collect data from table into the non-input variables
	rv.clear();
	for (int row = 1; row < ptw->rowCount(); ++row)
	{
		if (!ptw->item(row,0)->text().isEmpty() && !ptw->item(row, 1)->text().isEmpty())
		{
			rd.cols[0] = SmartString(ptw->item(row, 1)->text());		// name
			rd.cols[1] = SmartString(ptw->item(row, 1)->text());		// body/definition
			rd.cols[2] = SmartString(ptw->item(row, 1)->text());		// unit
			rd.cols[3] = SmartString(ptw->item(row, 1)->text());		// description
		}
		rv[rd.cols[0]] = rd;			// actual is always set
	}

	// next check for changes
	if (rv.size() != rvIn.size())	// data deleted or added
		_changed[table] = true;
	else for (int i = 1; i < rv.size(); ++i)
		if (rv[i] != rvIn[i])
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
