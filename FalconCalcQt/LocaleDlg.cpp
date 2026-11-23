#include <QLocale>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include "LocaleDlg.h"

LocaleDlg::LocaleDlg(QWidget* parent) : ModernDialog(ModernDialogParams(false, 0), parent)
{
	Ui::LocaleDlgClass::setupUi(ContentWidget());
	QString currentLocaleName;
	
	QLocale locale = edtLocale->locale();
	QString localeName = locale.name(); // e.g. "en_US"

	lblCurrentLocale->setText(localeName);
	edtLocale->setText(localeName);
	connect(buttonBox, &QDialogButtonBox::accepted,
		this, &LocaleDlg::on_buttonBox_accepted);
	connect(buttonBox, &QDialogButtonBox::rejected,
		this, &LocaleDlg::on_buttonBox_rejected);
}

QLocale LocaleDlg::GetNewLocale() const
{
	return QLocale(edtLocale->text().trimmed());
}

//void LocaleDlg::on_edtLocale_textChanged(QString newText)
//{
//	bool state = (newText.length() < 5 && newText.toUpper() != "C") || newText[2] != QChar('_');
//	QPushButton* pb = ui.buttonBox->button(QDialogButtonBox::Save);
//	if (pb)
//		pb->setEnabled(!state);
//}

void LocaleDlg::on_buttonBox_accepted()
{
	accept();
}

void LocaleDlg::on_buttonBox_rejected()
{
	reject();
}