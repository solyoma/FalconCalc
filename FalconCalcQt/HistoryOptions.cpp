#include "HistoryOptions.h"

HistoryOptions::HistoryOptions(HistOptionData& data, QWidget* parent) :
	ModernDialog(data.params, parent),_data0(data), _data(data)
{
	Ui::HistoryOptionsClass::setupUi(ContentWidget());

	if (data.historySorted)
		chksorted->setChecked(true);
	if (data.maxHistDepth > 0)
		chkMaxDepth->setChecked(true);
	int n = std::abs(data.maxHistDepth);
	if (n > 200)
		n = 200;
	spnHistDepth->setValue(n);

	n = data.minCharLength;
	if (n < 0)
		n = 0;
	else if (n > 10)
		n = 10;
	spnMinSaveLength->setValue(n);
	n = data.watchTimeout;	// in seconds, max value:600 sec : 10 min
	if (n > 0)
		chkInterval->setChecked(true);
	n = std::abs(n);
	if (n > 600)
		n = 600;
	spnMinutes->setValue(n / 60);
	spnSeconds->setValue(n % 60);

	connect(btnOk, &QPushButton::clicked, this, &HistoryOptions::on_btnOk_clicked);
	connect(btnCancel, &QPushButton::clicked, this, [this]() { reject(); });
}

HistoryOptions::~HistoryOptions()
{

}

void HistoryOptions::on_btnOk_clicked()
{
	_data.historySorted = chksorted->isChecked();

	int n = spnHistDepth->value();
	if (!chkMaxDepth->isChecked())
		n = -n;
	_data.maxHistDepth = n;

	n = spnMinSaveLength->value();
	if (!chkMaxDepth->isChecked())
		n = -n;
	_data.minCharLength = n;

	n = spnMinutes->value() * 60 + spnSeconds->value();
	if (!chkInterval->isChecked())
		n = -n;
	_data.watchTimeout = n;

	if (_data != _data0)
	{
		_data0 = _data;
	}
}
