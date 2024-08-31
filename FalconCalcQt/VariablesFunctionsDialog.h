#pragma once
#include <QtWidgets/QDialog>
#include <QTableWidget.h>
#include <QStyledItemDelegate>

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

	FalconCalc::VarFuncInfo ToVarFuncInfo() const;

	constexpr unsigned UserCount() const { return which ? uUserFuncCnt : uUserVarCnt; }
	void SetUserCount(unsigned count) 
	{ 
		if (which) 
			uUserFuncCnt = count; 
		else 
			uUserVarCnt = count; 
	}
};

/*=============================================================
 * TASK   :	crete an elided text for display 
 * PARAMS :
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: Maybe this class is not needed, because
 *			the delegate that displays the data modifies number strings
 *			before display and loose all but the first digit
 *------------------------------------------------------------*/
class ElidingTableWidgetItem : public QTableWidgetItem	// default elision is wrong
{
public:
	ElidingTableWidgetItem(const QString& text) : QTableWidgetItem(text) {}

	//void setData(int role, const QVariant& value) override {
	//	QTableWidgetItem::setData(role, value);
	//}
	//QVariant data(int role) const override {
	//	return QTableWidgetItem::data(role);
	//}

	//void setData(int role, const QVariant& value) override {
	//	if (role == Qt::DisplayRole || role == Qt::EditRole) {
	//		QTableWidgetItem::setData(Qt::UserRole, value); // Store full text in UserRole
	//	}
	//	QTableWidgetItem::setData(role, value);
	//}

	QVariant data(int role) const override {
		if (role == Qt::DisplayRole) 
		{
			QFontMetrics metrics(QTableWidgetItem::tableWidget()->font());
			int col = column();
			int columnWidth = QTableWidgetItem::tableWidget()->columnWidth(col) - 10;
			QString fullText = QTableWidgetItem::data(Qt::DisplayRole).toString();
			QString elidedText = metrics.elidedText(fullText, col == 1 ? Qt::ElideMiddle : Qt::ElideRight, columnWidth);
			// DEBUG
			//if(fullText != elidedText)
			//	qDebug("Eliding  row: %d, col:%d, full:%s, elided:%s", row(), column(), fullText.toStdString().c_str(),elidedText.toStdString().c_str());
			// /DEBUG
			return elidedText;
		}
		return QTableWidgetItem::data(role);
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
	FalconCalc::VarFuncInfo<QString, QStringList> _vf;

	QStack<QPair<int, QVector<ElidingTableWidgetItem*>>> _removedVarRows, _removedFuncRows;
	QStack<QPair<int, QVector<ElidingTableWidgetItem*>>>* _pActStack;
private:
	void _ClearUserTables(int whichTab);
	void _FillBuiltinFuncTable();
	void _FillUserFuncTable();
	void _FillFuncTables();
	void _FillBuiltinVarTable();
	void _FillUserVarTable();
	void _FillVarTables();
	void _Serialize(VarFuncInfoQt *pvf=nullptr);    // user functions and variables into _vf
	void _AddCellText(QTableWidget* ptw, int row, int col, QString text);
	void convert(const FalconCalc::VarFuncInfo<QString,QStringList>& vfQt, FalconCalc::VarFuncInfo<SmartString, StringVector>&vf)
private:
	Ui::VariablesFunctionsDialogClass ui;

	QTableWidget* _pActUserTable;
};
