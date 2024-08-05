#pragma once
#include <QtWidgets/QDialog>
#include "ui_HistoryDialog.h"
#include <QtWidgets/QListWidget>

class HistoryDialog : public QDialog
{
	Q_OBJECT
public:
	HistoryDialog(const QStringList& lstHist, QWidget* parent = nullptr);
	~HistoryDialog();

	QListWidget* pListWidget() const { return ui.lstHistory; }
	void Clear();
protected:
	void closeEvent(QCloseEvent* e);
public:
signals:
	void SignalSelection(QString s);
	void SignalClose();
	void SignalHistOptions();
	void SignalRemoveHistLine(int row);
	void SignalClearHistory();
private slots:
	void on_lstHistory_doubleClicked(const QModelIndex&);
	void on_btnCancel_clicked();
	void on_btnHistOptions_clicked();
	void on_btnRemoveAll_clicked();
	void on_btnRemove_clicked();
private:
	Ui::HistoryDialogClass ui;
};

