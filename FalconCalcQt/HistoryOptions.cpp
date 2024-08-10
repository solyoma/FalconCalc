#include "HistoryOptions.h"

HistoryOptions::HistoryOptions(HistOptionData& data, QWidget* parent) : _data0(data), _data(data), QDialog(parent)
{
	ui.setupUi(this);

	if (data.historySorted)
		ui.chksorted->setChecked(true);
	if (data.maxHistDepth > 0)
		ui.chkMaxDepth->setChecked(true);
	int n = std::abs(data.maxHistDepth);
	if (n > 200)
		n = 200;
	ui.spnHistDepth->setValue(n);

	n = data.minCharLength;
	if (n < 0)
		n = 0;
	else if (n > 10)
		n = 10;
	ui.spnMinSaveLength->setValue(n);
	n = data.watchTimeout;	// in seconds, max value:600 sec : 10 min
	if (n > 0)
		ui.chkInterval->setChecked(true);
	n = std::abs(n);
	if (n > 600)
		n = 600;
	ui.spnMinutes->setValue(n / 60);
	ui.spnSeconds->setValue(n % 60);
}

HistoryOptions::~HistoryOptions()
{

}

void HistoryOptions::on_btnOk_clicked()
{
	_data.historySorted = ui.chksorted->isChecked();

	int n = ui.spnHistDepth->value();
	if (!ui.chkMaxDepth->isChecked())
		n = -n;
	_data.maxHistDepth = n;

	n = ui.spnMinSaveLength->value();
	if (!ui.chkMaxDepth->isChecked())
		n = -n;
	_data.minCharLength = n;

	n = ui.spnMinutes->value() * 60 + ui.spnSeconds->value();
	if (!ui.chkInterval->isChecked())
		n = -n;
	_data.watchTimeout = n;

	if (_data != _data0)
	{
		_data0 = _data;
	}
}
