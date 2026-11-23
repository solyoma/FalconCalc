#pragma once
#include <QtWidgets/QDialog>
#include "ModernDialog.h"
#include "ui_HistoryDialog.h"
#include <QtWidgets/QListWidget>

class HistoryDialog : public ModernDialog, private Ui::HistoryDialogClass
{
	Q_OBJECT
public:
	HistoryDialog(const QStringList& lstHist, ModernDialogParams &params, QWidget* parent = nullptr);
	~HistoryDialog();

	QListWidget* pListWidget() const { return lstHistory; }
	void SetList(const QStringList& lstHist);
	void Clear();
protected:
	void closeEvent(QCloseEvent* e);
	void moveEvent(QMoveEvent* e);
public:
signals:
	void SignalSelection(QString s);
	void SignalHistClose();
	void SignalHistOptions();
	void SignalRemoveHistLine(int row);
	void SignalClearHistory();
	void SignalHistMoved();
private slots:
	void on_lstHistory_doubleClicked(const QModelIndex&);
	void on_btnCancel_clicked();
	void on_btnHistOptions_clicked();
	void on_btnRemoveAll_clicked();
	void on_btnRemove_clicked();
private:
	Ui::HistoryDialogClass ui;
};

