#pragma once
#include <QtWidgets/QDialog>

#include <QStack>			// for undo
#include <QPair>

#include "ui_VariablesFunctionsDialog.h"

namespace FalconCalc {
	class LittleEngine;
	struct VarFuncInfo;
}

struct VarFuncInfoQt
{
	int which = 0;	// varFunc
	unsigned uBuiltinFuncCnt= 0;
	unsigned uBuiltinVarCnt= 0;
	unsigned uUserFuncCnt= 0;
	unsigned uUserVarCnt= 0;
	QString sBuiltinFuncs;
	QString sBuiltinVars;
	QString sUserFuncs;
	QString sUserVars;
	FalconCalc::LittleEngine* pOwner = nullptr;

	VarFuncInfoQt() {}
	VarFuncInfoQt(const FalconCalc::VarFuncInfo&);

	bool operator==(const VarFuncInfoQt& vf) const;
	bool operator!=(const VarFuncInfoQt& vf) const { return !operator==(vf); }

	FalconCalc::VarFuncInfo ToVarFuncInfo();

	constexpr unsigned UserCount() const { return which ? uUserFuncCnt : uUserVarCnt; }
	void SetUserCount(unsigned count) 
	{ 
		if (which) 
			uUserFuncCnt = count; 
		else 
			uUserVarCnt = count; 
	}
};


class VariablesFunctionsDialog : public QDialog
{
	Q_OBJECT
public:
	VariablesFunctionsDialog(VarFuncInfoQt &vfi, QWidget* parent = nullptr);
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
	void SignalVarFuncSaved(VarFuncInfoQt&vf);
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
	void on_btnSave_clicked();
	void on_btnAddRow_clicked();
	void on_tblUserVars_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_tblUserVars_cellDoubleClicked(int row, int col);
	void on_tblUserVars_cellChanged(int row, int col);
	void on_tblUserFuncs_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_tblUserFuncs_cellChanged(int row, int col);
	void on_tblUserFuncs_cellDoubleClicked(int row, int col);
private:
	FalconCalc::LittleEngine* _lengine = nullptr;
	bool _changed = false;
	bool _busy = false;
	QString _sTmp;	// for table cell change
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
	void _Serialize(VarFuncInfoQt *pvf=nullptr);    // user functions and variables into _vf
	void _AddCellText(QTableWidget* ptw, int row, int col, QString text, bool noElide = false);
private:
	Ui::VariablesFunctionsDialogClass ui;

	QTableWidget* _pActUserTable;
};
