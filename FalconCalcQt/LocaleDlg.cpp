#include <QLocale>
#include <QtWidgets/QDialog>
#include "LocaleDlg.h"

LocaleDlg::LocaleDlg(QWidget* parent) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);
	QString currentLocaleName;
	
	QLocale locale = ui.edtLocale->locale();
	QString localeName = locale.name(); // e.g. "en_US"

	ui.lblCurrentLocale->setText(localeName);
	ui.edtLocale->setText(localeName);
}

QLocale LocaleDlg::GetNewLocale() const
{
	return QLocale(ui.edtLocale->text().trimmed());
}
