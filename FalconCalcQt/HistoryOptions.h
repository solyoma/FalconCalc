#pragma once
#include <QtWidgets/QDialog>
#include "ui_HistoryOptions.h"

class HistoryOptions : public QDialog
{
	Q_OBJECT
public:
	 HistoryOptions(QWidget* parent = nullptr);
	~HistoryOptions();

private:
	Ui::HistoryOptionsClass ui;
};
