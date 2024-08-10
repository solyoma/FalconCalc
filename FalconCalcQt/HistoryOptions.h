#pragma once
#include <QtWidgets/QDialog>
#include "ui_HistoryOptions.h"


struct HistOptionData
{
	int watchTimeout  = 0;
	int maxHistDepth  = 0;
	int historySorted = 0;
	int minCharLength = 0;

	bool operator!=(const HistOptionData& o) const
	{
		return !operator==(o);
	}
	bool operator==(const HistOptionData& o) const
	{
		return
			watchTimeout  == o.watchTimeout &&
			maxHistDepth  == o.maxHistDepth &&
			historySorted == o.historySorted &&
			minCharLength == o.minCharLength;
	}
};

class HistoryOptions : public QDialog
{
	Q_OBJECT
public:
	 HistoryOptions(HistOptionData &data, QWidget* parent = nullptr);
	~HistoryOptions();

private:
	Ui::HistoryOptionsClass ui;

	HistOptionData _data;
	HistOptionData &_data0;
private slots:
	void on_btnOk_clicked();
};
