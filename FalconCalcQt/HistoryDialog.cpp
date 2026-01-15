#include "HistoryDialog.h"
#include <QMoveEvent>

HistoryDialog::HistoryDialog(const QStringList& lstHist, QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);
	ui.lstHistory->addItems(lstHist);
}

HistoryDialog::~HistoryDialog()
{
	emit SignalHistClose();
}

void HistoryDialog::SetList(const QStringList& lstHist)
{
	ui.lstHistory->clear();
	ui.lstHistory->addItems(lstHist);
}

void HistoryDialog::Clear()
{
	ui.lstHistory->clear();
}

void HistoryDialog::closeEvent(QCloseEvent* e)
{
	emit SignalHistClose();
}

void HistoryDialog::moveEvent(QMoveEvent* e)
{
	// DEBUG
	//int x0 = e->oldPos().x(), y0 = e->oldPos().y(),
	//	x = e->pos().x(), y = e->pos().y(),
	//	xh0 = geometry().left(), yh0 = geometry().top();
	// /DEBUG

	// qDebug(" - hist dialog moved: (%d, %d)->(%d, %d), delta (%d,%d), ", xh0, yh0, xh0 + x - x0, yh0 + y - y0, x - x0, y - y0);

	emit SignalHistMoved();
}

void HistoryDialog::on_btnCancel_clicked()
{
	emit SignalHistClose();
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
