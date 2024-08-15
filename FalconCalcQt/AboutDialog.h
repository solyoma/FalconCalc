#pragma once
#ifndef _ABOUTDIALOG_H
	#define _ABOUTDIALOG_H
#include <QtWidgets/QDialog>
#include "ui_AboutDialog.h"

class AboutDialog : public QDialog
{
	Q_OBJECT
public:
	AboutDialog(QWidget* parent = nullptr) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
	{
		ui.setupUi(this);
	}
	~AboutDialog() {}

	static int AboutVisible;
private:
	Ui::AboutDialogClass ui;
};

#endif