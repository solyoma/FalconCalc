#include "VariablesFunctionsDialog.h"

#include <QStack>
#include <QPair>

#include "SmartString.h"
using namespace SmString;
#include "LongNumber.h"
using namespace LongNumber;
#include "calculate.h"



VarFuncInfoQt::VarFuncInfoQt(const FalconCalc::VarFuncInfo& vf)
{
//	vfQt.which = which;				must be set from outside, 
	pOwner = vf.pOwner;
	uBuiltinVarCnt = vf.uBuiltinVarCnt;
	uBuiltinFuncCnt = vf.uBuiltinFuncCnt;
	uUserVarCnt = vf.uUserVarCnt;
	uUserFuncCnt = vf.uUserFuncCnt;
	sBuiltinVars = vf.sBuiltinVars.toQString();
	sBuiltinFuncs = vf.sBuiltinFuncs.toQString();
	sUserVars = vf.sUserVars.toQString();
	sUserFuncs = vf.sUserFuncs.toQString();
}

const void VarFuncInfoQt::ToVarFuncInfo(FalconCalc::VarFuncInfo& vf)
{
	vf.pOwner = vf.pOwner;
	vf.uBuiltinVarCnt	= uBuiltinVarCnt;
	vf.uBuiltinFuncCnt	= uBuiltinFuncCnt;
	vf.uUserVarCnt		= uUserVarCnt;
	vf.uUserFuncCnt		= uUserFuncCnt;
	vf.sBuiltinVars		= sBuiltinVars;
	vf.sBuiltinFuncs	= sBuiltinFuncs;
	vf.sUserVars		= sUserVars;
	vf.sUserFuncs		= sUserFuncs;
}



const QString qsCommentDelimiterString = ":";

VariablesFunctionsDialog::VariablesFunctionsDialog(VarFuncInfoQt &vfi, QWidget* parent) : _vf(vfi), QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);
	pUserVars = ui.tblUserVars;
	pBuiltinVars = ui.tblBuiltinVars;
	pUserFuncs = ui.tblUserFuncs;
	pBuiltinFuncs = ui.tblBuiltinFuncs;

	_FillVarTables();
	_FillFuncTables();
	ui.tabHeader->setCurrentIndex(_vf.which);
	ui.btnRemoveAll->setEnabled(_vf.which ? _vf.uBuiltinFuncCnt : _vf.uBuiltinVarCnt);
	// Enable the button if any rows are selected button->setEnabled(!tableWidget->selectedItems().isEmpty()); })
	QToolButton* pbtn = ui.btnRemoveRow;
	connect(pUserVars->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, pbtn]() {
		pbtn->setEnabled(!pUserVars->selectedItems().isEmpty());
		});
	connect(pUserFuncs->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, pbtn]() {
		pbtn->setEnabled(!pUserFuncs->selectedItems().isEmpty());
		});

	if (_vf.which)
	{
		_pActUserTable = pUserFuncs;
		_pActStack = &_removedFuncRows;
	}
	else
	{
		_pActUserTable = pUserVars;
		_pActStack = &_removedVarRows;
	}
}

VariablesFunctionsDialog::~VariablesFunctionsDialog()
{
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
		_vf.which = 1;
	}
	else
	{
		_pActUserTable = pUserVars;
		_pActStack = &_removedVarRows;
		_vf.which = 0;
	}
	ui.btnRemoveAll->setEnabled(_vf.UserCount());
	ui.btnRemoveRow->setEnabled(!_pActUserTable->selectedItems().isEmpty());
	ui.btnUndo->setEnabled(index ? _removedFuncRows.size() : _removedVarRows.size());
	qDebug("on_tabHeader_currentChanged");
}

void VariablesFunctionsDialog::on_btnRemoveAll_clicked()
{
	ui.btnUndo->setEnabled(_pActUserTable->rowCount());

	for(int r = 0; r < _pActUserTable->rowCount(); ++r)
	{
		QVector<QTableWidgetItem*> rowItems;
		for (int col = 0; col < _pActUserTable->columnCount(); ++col) {
			rowItems.append(_pActUserTable->takeItem(r, col));
		}
		_pActStack->push({ r, rowItems });
		_pActUserTable->removeRow(r);
	}
	_vf.SetUserCount(0);
	qDebug("on_btnRemoveAll_clicked");
}

void VariablesFunctionsDialog::on_btnRemoveRow_clicked()
{
	int r = _pActUserTable->currentRow();	// _pActStack is set in constructor and TAB selection

	// qDebug("on_btnRemoveRow_clicked to remove row #%d", r);
	if (r >= 0)
	{
		QVector<QTableWidgetItem*> rowItems;
		for (int col = 0; col < _pActUserTable->columnCount(); ++col) {
			rowItems.append(_pActUserTable->takeItem(r, col));
		}
		_pActStack->push({ r, rowItems });
		_pActUserTable->removeRow(r);
		ui.btnUndo->setEnabled(true);
		_vf.SetUserCount(_pActUserTable->rowCount());
	}
}

void VariablesFunctionsDialog::on_btnUndo_clicked()
{
	auto lastRemoved = _pActStack->pop();
	int rowIndex = lastRemoved.first;
	QVector<QTableWidgetItem*> rowItems = lastRemoved.second;

	// Insert the row back at its original position
	_pActUserTable->insertRow(rowIndex);
	for (int col = 0; col < rowItems.size(); ++col) {
		_pActUserTable->setItem(rowIndex, col, rowItems[col]);
	}
	ui.btnUndo->setEnabled(_pActStack->size());
	ui.btnRemoveAll->setEnabled(_vf.UserCount());
	_vf.SetUserCount(_pActUserTable->rowCount());
}

void VariablesFunctionsDialog::on_btnSave_clicked()
{
	_Serialize();
}

void VariablesFunctionsDialog::on_btnAddRow_clicked()
{
	qDebug("on_btnAddRow_clicked");
}

void VariablesFunctionsDialog::on_tblUserVars_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	qDebug("on_tblUserVars_itemClicked");
}

void VariablesFunctionsDialog::on_tblUserFuncs_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
{
	qDebug("on_tblUserFuncs_itemClicked");
}

void VariablesFunctionsDialog::_ClearUserTables(int whichTab)
{
	QTableWidget* ptU, * ptB;
	int rowc;
	if (whichTab)
	{
		ptU = ui.tblUserFuncs;
		ptB = ui.tblBuiltinFuncs;
		rowc = _vf.uUserFuncCnt;
	}
	else
	{
		ptU = ui.tblUserVars;
		ptB = ui.tblBuiltinVars;
		rowc = _vf.uUserVarCnt;
	}

	ptU->setRowCount(rowc);
	for (int i = 0; i < rowc; ++i)
	{
		QTableWidgetItem* pwi = ptU->item(i, 0);
		if (pwi)
		{
			pwi->text().clear();
			ptU->item(i, 1)->text().clear();
			ptU->item(i, 2)->text().clear();
			ptU->item(i, 3)->text().clear();
		}
	}
}

void VariablesFunctionsDialog::_FillBuiltinFuncTable()
{
	QStringList sl;
	QTableWidgetItem* ptw;
	size_t pos = 0, pos1;
	QString qs;
	ui.tblBuiltinFuncs->setRowCount(_vf.uBuiltinFuncCnt);

	for (size_t row = 0; row < _vf.uBuiltinFuncCnt; ++row)
	{
		pos1 = _vf.sBuiltinFuncs.indexOf(QChar('\n'),pos);
		sl = _vf.sBuiltinFuncs.mid(pos, pos1 - pos).split(qsCommentDelimiterString,Qt::SkipEmptyParts);
		pos = pos1+1;

		ptw = new QTableWidgetItem(sl[0]);	
		ui.tblBuiltinFuncs->setItem(row, 0, ptw);
		ptw = new QTableWidgetItem("-");
		ui.tblBuiltinFuncs->setItem(row, 1, ptw);
		ptw = new QTableWidgetItem("-");
		ui.tblBuiltinFuncs->setItem(row, 2, ptw);
		ptw = new QTableWidgetItem(sl[1]);	
		ui.tblBuiltinFuncs->setItem(row, 3, ptw);
	}
}

void VariablesFunctionsDialog::_FillUserFuncTable()
{
	QStringList sl;
	size_t pos = 0, pos1, n;
	QString qs;
	if(ui.tblUserFuncs->rowCount() < (int)_vf.uUserFuncCnt)
		ui.tblUserFuncs->setRowCount(_vf.uUserFuncCnt);
	for (size_t row = 0; row < _vf.uUserFuncCnt; ++row)
	{
		pos1 = _vf.sUserFuncs.indexOf(QChar('\n'),pos);
		sl = _vf.sUserFuncs.mid(pos, pos1 - pos).split(qsCommentDelimiterString, Qt::SkipEmptyParts);
		pos = pos1+1;

		_AddCellText(ui.tblUserFuncs, row, 0, sl[0]);
		_AddCellText(ui.tblUserFuncs, row, 1, sl[1]);
		if(sl.size() == 3)
			qs = "-", n = 2;
		else
			qs = sl[2], n = 3;
		_AddCellText(ui.tblUserFuncs, row, 2, qs);
		_AddCellText(ui.tblUserFuncs, row, 3, sl[n], true);
	}
	assert(_vf.uUserFuncCnt == pUserFuncs->rowCount());	 // _vf should contain valid data
}

void VariablesFunctionsDialog::_FillFuncTables()   
{
	_ClearUserTables(1);
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
		_AddCellText(ui.tblBuiltinVars, row, 3, sl[3],true);
	}
}

void VariablesFunctionsDialog::_FillUserVarTable()
{
	QStringList sl;
	size_t pos = 0, pos1;
	QString qs;
	if(ui.tblUserVars->rowCount() < (int)_vf.uUserVarCnt)
		ui.tblUserVars->setRowCount(_vf.uUserVarCnt);
	for (size_t row = 0; row < _vf.uUserVarCnt; ++row)
	{
		pos1 = _vf.sUserVars.indexOf(QChar('\n'), pos);
		sl = _vf.sUserVars.mid(pos, pos1 - pos).split(qsCommentDelimiterString,Qt::KeepEmptyParts);
		pos = pos1 + 1;

		_AddCellText(ui.tblUserVars, row, 0, sl[0]);   // name
		_AddCellText(ui.tblUserVars, row, 1, sl[1]);   // definition

		qs = "-";		// may be overwritten
		switch(sl.size())
		{
			case 2:
				_AddCellText(ui.tblUserVars, row, 2, qs);   // definition
				break;
			case 3:
				_AddCellText(ui.tblUserVars, row, 2, sl[2]);   // definition
				break;
			case 4:
				_AddCellText(ui.tblUserVars, row, 2, sl[3]);   // unit
				if (!sl[2].isEmpty())
					qs = sl[2];
				_AddCellText(ui.tblUserVars, row, 3, sl[2],true);   // description
				break;
		}
	}
	assert(_vf.uUserVarCnt == pUserVarss->rowCount());	 // _vf should contain valid data
}

void VariablesFunctionsDialog::_FillVarTables()
{
	_ClearUserTables(0);
	_FillUserVarTable();
	_FillBuiltinVarTable();

	//QTableWidgetItem* ptw;
	//int rowU = 0, rowB = 0;
	//for (auto& f : FalconCalc::LittleEngine::variables)
	//{
	//	ptw = new QTableWidgetItem(f.second.data.name.toQString());
	//	if (f.second.body.empty())
	//	{
	//		ui.tblBuiltinVars->setItem(rowB, 0, ptw);
	//		ptw = new QTableWidgetItem("-");
	//		ui.tblBuiltinVars->setItem(rowB, 1, ptw);
	//		ptw = new QTableWidgetItem(f.second.data.unit.toQString());
	//		ui.tblBuiltinVars->setItem(rowB, 2, ptw);
	//		ptw = new QTableWidgetItem(f.second.data.desc.toQString());
	//		ui.tblBuiltinVars->setItem(rowB, 3, ptw);
	//		++rowB;
	//	}
	//	else		// user
	//	{
	//		ui.tblUserVars->setItem(rowU, 0, ptw);
	//		ptw = new QTableWidgetItem(f.second.body.toQString());
	//		ui.tblBuiltinVars->setItem(rowU, 1, ptw);
	//		ptw = new QTableWidgetItem(f.second.data.unit.toQString());
	//		ui.tblBuiltinVars->setItem(rowU, 2, ptw);
	//		ptw = new QTableWidgetItem(f.second.data.desc.toQString());
	//		ui.tblBuiltinVars->setItem(rowU, 3, ptw);
	//		++rowU;
	//	}
	//}
}

void VariablesFunctionsDialog::_Serialize()
{
	if (!_changed)    // 'changed' must be modified in caller when necessary
		return;
	auto _serialize = [this](int which)
		{
			QTableWidget* pw = which ? ui.tblUserFuncs : ui.tblUserVars;
			QString& qs = which ? _vf.sUserFuncs : _vf.sUserVars;
			unsigned& cnt = (which ? _vf.uUserFuncCnt : _vf.uUserVarCnt);
			int n = 0; // count of user variables/functions
			qs.clear();
			for (int i = 1; i < pw->rowCount(); ++i)
			{
				if (!pw->item(0, i)->text().isEmpty() && !pw->item(1, i)->text().isEmpty())
				{
					qs += pw->item(0, i)->text() + qsCommentDelimiterString + pw->item(1, i)->text();
					// unit (even when empty)
					qs += qsCommentDelimiterString + pw->item(3, i)->text();
					// description / body
					qs += qsCommentDelimiterString + pw->item(2, i)->text() + "\n";
					++n;
				}
			}
			cnt = n;
		};
	_serialize(0);
	_serialize(1);
}

void VariablesFunctionsDialog::_AddCellText(QTableWidget* ptw, int row, int col, QString text, bool noElide)
{
	QTableWidgetItem* ptwi;
	QString qsHint = text;
	if (!noElide && text.length() > 16)
		text = text.left(14) + QChar(0x2026);
	ptwi = new QTableWidgetItem(text);
	ptwi->setToolTip(qsHint);
	ptw->setItem(row, col, ptwi);
}

void VariablesFunctionsDialog::SlotSelectTab(int tab)
{
		ui.tabHeader->setCurrentIndex(tab);
}
