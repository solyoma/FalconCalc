#include "VariablesFunctionsDialog.h"

VariablesFunctionsDialog::VariablesFunctionsDialog(int which, QWidget* parent) :QDialog(parent)
{
	ui.setupUi(this);
	pUser = ui.tblUser;
	pBuiltin = ui.tblBuiltin;
	//pUser->setColumnCount(4);
	//pBuiltin->setColumnCount(4);
	pUser->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui.tabHeader->setCurrentIndex(which);
}

VariablesFunctionsDialog::~VariablesFunctionsDialog()
{
}

void VariablesFunctionsDialog::SlotSetColWidths(int which, int cw1, int cw2, int cw3, int cw4)
{
	QTableWidget* ptw = which ? ui.tblBuiltin : ui.tblUser;
	ptw->setColumnWidth(0, cw1);
	ptw->setColumnWidth(0, cw2);
	ptw->setColumnWidth(0, cw3);
	ptw->setColumnWidth(0, cw4);
}

void VariablesFunctionsDialog::closeEvent(QCloseEvent* e)
{
	emit SignalClose();
}

void VariablesFunctionsDialog::on_tblUser_columnResized(int, int, int)
{
	;
}

void VariablesFunctionsDialog::on_tblBuiltin_columnResized(int, int, int)
{
	;
}

void VariablesFunctionsDialog::on_tabHeader_currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
	;
}

void VariablesFunctionsDialog::SlotSelectTab(int tab)
{
		ui.tabHeader->setCurrentIndex(tab);
}