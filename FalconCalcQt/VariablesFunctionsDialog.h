#pragma once
#include <QtWidgets/QDialog>
#include <QTableWidget>
#include <QStyledItemDelegate>

#include <QStack>			// for undo
#include <QPair>

#include "SmartString.h"
#include "LongNumber.h"
#include "calculate.h"

#include "ui_VariablesFunctionsDialog.h"

namespace FalconCalc {
	enum WindowSide;
	class LittleEngine;
}


/*=============================================================
 * TASK   :	crete an elided text only for display. 
 *			Keeps the full text of the item intact
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
	ElidingTableWidgetItem(const QString& text) : QTableWidgetItem(text, QTableWidgetItem::UserType) {}

	QVariant data(int role) const override 
	{
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
		else if (role == Qt::ToolTipRole)
		{
			QString fullText = QTableWidgetItem::data(Qt::DisplayRole).toString();
			return QTableWidgetItem::data(Qt::ToolTipRole);
		}
		return QTableWidgetItem::data(role);
	}
	QString Text() const
	{
		return QTableWidgetItem::data(Qt::DisplayRole).toString();
	}
};

class VariablesFunctionsDialog : public QDialog
{
	Q_OBJECT
public:
	VariablesFunctionsDialog(int initialTabIndex, QWidget* parent = nullptr);
	~VariablesFunctionsDialog();

	inline int ActualTab() const { return ui.tabHeader->currentIndex(); }
signals:
	void SignalVarFuncClose();
	void SignalTabChange(int tab);
	void SignalVarFuncMoved();
	void SignalTableDoubleClicked(QString& name);
public slots:
	void SlotSelectTab(int tab);
	void SlotSetColWidths(int which, int cw1, int cw2, int cw3, int cw4); //which:) -> user, 1->builtin
protected:
	void moveEvent(QMoveEvent* e) override;
protected slots:
	void on_tabHeader_currentChanged(int index);
	void on_btnRemoveAll_clicked();
	void on_btnRemoveRow_clicked();
	void on_btnUndo_clicked();
	void on_btnCancel_clicked();
	void on_btnSave_clicked();
	void on_btnAddRow_clicked();
	void on_tblUserVars_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_tblUserVars_cellDoubleClicked(int row, int col);
	void on_tblUserVars_cellChanged(int row, int col);
	void on_tblUserFuncs_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_tblUserFuncs_cellChanged(int row, int col);
	void on_tblUserFuncs_cellDoubleClicked(int row, int col);
	void on_tblBuiltinVars_cellDoubleClicked(int row, int col);
	void on_tblBuiltinFuncs_cellDoubleClicked(int row, int col);
private:
	FalconCalc::LittleEngine* _lengine = nullptr;
	FalconCalc::RowDataMap *_pUserVarsMap, *_pUserFuncsMap,		// actual data
						   *_pUserVarsInMap, *_pUserFuncsInMap; // input data (when VariablesFunctionsDialog is created)
	int _snappedToSide = 0;	  // FalconCalc::WindowSide

	static int _colW[2][4];		// column widths
	bool _changed[2] = { false,false };
	int _cntRowsWithData[2] = { 0,0 }; // count of rows with data (not empty)
	bool _busy = false;
	QString _sTmp;	// for table cell change

	int _snapPixelLimit = 30;	// pixels snap if inside this distance from, main window
	int _snapDist = 0;			// when '_snapped' is true: distance from '_snappedToSide'

	QTableWidget* _pActUserTable;
	struct UndoItem
	{
		int table=-1; // 0: variables, 1: functions
		QVector< QPair<int, QVector<ElidingTableWidgetItem* >>>  data; // first: row, second columns

		bool IsValid() const { return table >= 0 && !data.isEmpty(); }
	};
	QStack<UndoItem> _undoStack;

private:
	QString _GetItemText(QTableWidget *table, int row, int col); // not const: sets _sTmp too
	void _FillBuiltinFuncTable();
	void _FillUserFuncTable();
	void _FillFuncTables();
	void _FillBuiltinVarTable();
	void _FillUserVarTable();
	void _FillVarTables();
	//void _Serialize(VarFuncInfoQt *pvf=nullptr);    // user functions and variables into _vf
	void _AddCellText(QTableWidget* ptw, int row, int col, QString text);
	void _CollectFrom(int index);	// from index-th grid to RowDataMap
	bool _GetActualDataCount(int index);	//0: variables, 1: functions
	void _EnableButtons();
	void _TableDoubleClickedCommon(QTableWidget* ptw, int row, int col);
	void _UserTableEditCommon(QTableWidget* ptw, int row, int col);
private:
	Ui::VariablesFunctionsDialogClass ui;

};
