#pragma once
#include <QScreen>
#include <QWidget>

#include "ModernDialog.h"
#include "ui_HistoryOptions.h"


struct HistOptionData
{
	ModernDialogParams params;	// see ModernDialog.h
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

class HistoryOptions : public ModernDialog, private Ui::HistoryOptionsClass
{
	Q_OBJECT
public:
	 HistoryOptions(HistOptionData &data, QWidget* parent = nullptr);
	~HistoryOptions();

protected:
	void showEvent(QShowEvent* event) override
	{
		int w = screen()->geometry().width();
		int h = screen()->geometry().height();
		w -= geometry().width();
		h -= geometry().height();
		move(w / 2, h / 2);

	}

private:
	Ui::HistoryOptionsClass ui;

	HistOptionData _data;
	HistOptionData &_data0;
private slots:
	void on_btnOk_clicked();
};
