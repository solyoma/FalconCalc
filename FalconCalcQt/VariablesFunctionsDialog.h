#pragma once
#include <QtWidgets/QDialog>

#include <QStack>			// for undo
#include <QPair>

#include "ui_VariablesFunctionsDialog.h"

namespace FalconCalc {
	class LittleEngine;
}

struct VarFuncInfoQt
{
	unsigned uBuiltinFuncCnt= 0;
	unsigned uBuiltinVarCnt= 0;
	unsigned uUserFuncCnt= 0;
	unsigned uUserVarCnt= 0;
	QString sBuiltinFuncs;
	QString sBuiltinVars;
	QString sUserFuncs;
	QString sUserVars;
	FalconCalc::LittleEngine* pOwner = nullptr;
};


class VariablesFunctionsDialog : public QDialog
{
	Q_OBJECT
public:
	VariablesFunctionsDialog(int which, VarFuncInfoQt &vfi, QWidget* parent = nullptr);
	~VariablesFunctionsDialog();

	QTableWidget* pUserVars = nullptr;
	QTableWidget* pUserFuncs = nullptr;
	QTableWidget* pBuiltinVars = nullptr;
	QTableWidget* pBuiltinFuncs = nullptr;

	int ActualTab() const { return ui.tabHeader->currentIndex(); }
signals:
	void SignalVarFuncClose();
	void SignalTabChange(int tab);
	void SignalVarFuncMoved();
public slots:
	void SlotSelectTab(int tab);
	void SlotSetColWidths(int which, int cw1, int cw2, int cw3, int cw4); //which:) -> user, 1->builtin
protected:
	void closeEvent(QCloseEvent* e);
	void moveEvent(QMoveEvent* e);
protected slots:
	void on_tabHeader_currentChanged(int index);
	void on_btnRemoveAll_clicked();
	void on_btnRemoveRow_clicked();
	void on_btnUndo_clicked();
	void on_btnAddRow_clicked();
	void on_tblUserVars_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_tblUserFuncs_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
private:
	FalconCalc::LittleEngine* _lengine = nullptr;
	bool _changed = false;
	VarFuncInfoQt _vf;

	QStack<QPair<int, QVector<QTableWidgetItem*>>> _removedVarRows, _removedFuncRows;
	QStack<QPair<int, QVector<QTableWidgetItem*>>>* _pActStack;
private:
	void _ClearUserTables(int whichTab);
	void _FillBuiltinFuncTable();
	void _FillUserFuncTable();
	void _FillFuncTables();
	void _FillBuiltinVarTable();
	void _FillUserVarTable();
	void _FillVarTables();
	void _SerializeInto(int which, QString& dest, size_t& cnt);    // user functions or variables from actual page
	void _AddCellText(QTableWidget* ptw, int row, int col, QString text, bool noElide = false);
private:
	Ui::VariablesFunctionsDialogClass ui;

	QTableWidget* _pActUserTable;
};
