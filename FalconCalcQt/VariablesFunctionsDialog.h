#pragma once
#include <QtWidgets/QDialog>
#include "ui_VariablesFunctionsDialog.h"

class VariablesFunctionsDialog : public QDialog
{
	Q_OBJECT
public:
	VariablesFunctionsDialog(int which, QWidget* parent = nullptr);
	~VariablesFunctionsDialog();


	QTableWidget* pUser = nullptr;
	QTableWidget* pBuiltin = nullptr;

	int ActualTab() const { return ui.tabHeader->currentIndex(); }
signals:
	void SignalClose();
	void SignalTabChange(int tab);
public slots:
	void SlotSelectTab(int tab);
	void SlotSetColWidths(int which, int cw1, int cw2, int cw3, int cw4); //which:) -> user, 1->builtin
protected:
	void closeEvent(QCloseEvent* e);
private:
	Ui::VariablesFunctionsDialogClass ui;

protected:
	void on_tblUser_columnResized(int, int, int);	// column, old width, new width
	void on_tblBuiltin_columnResized(int, int, int);	// column, old width, new width
	void on_tabHeader_currentChanged(const QModelIndex& current, const QModelIndex& previous);
};
