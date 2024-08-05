#include "HistoryDialog.h"

HistoryDialog::HistoryDialog(const QStringList& lstHist, QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);
	ui.lstHistory->addItems(lstHist);
}

HistoryDialog::~HistoryDialog()
{

}

void HistoryDialog::Clear()
{
	ui.lstHistory->clear();
}

void HistoryDialog::closeEvent(QCloseEvent* e)
{
	emit SignalClose();
}

void HistoryDialog::on_btnCancel_clicked()
{
	emit SignalClose();
}

void HistoryDialog::on_btnHistOptions_clicked()
{
	emit SignalHistOptions();
}

void HistoryDialog::on_btnRemoveAll_clicked()
{
	emit SignalClearHistory();
}

void HistoryDialog::on_btnRemove_clicked()
{
	int i = ui.lstHistory->currentIndex().row();
	emit SignalRemoveHistLine(i);
}

void HistoryDialog::on_lstHistory_doubleClicked(const QModelIndex &index)
{
	QString s = ui.lstHistory->item(index.row())->text();
	emit SignalSelection(s);
}
