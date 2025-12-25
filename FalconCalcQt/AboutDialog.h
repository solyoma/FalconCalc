#pragma once
#ifndef _ABOUTDIALOG_H
	#define _ABOUTDIALOG_H
#include <QtWidgets/QDialog>
#include "ui_AboutDialog.h"
#include "version.h"

class AboutDialog : public QDialog
{
	Q_OBJECT
public:
	AboutDialog(int version, QWidget* parent = nullptr) : QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
	{
		ui.setupUi(this);									// e.g.	0x00 09 02;      // version 0.9.2
		ui.lblVersion->setText(QString(tr("Version %1").arg(VERSION_STRING)) );
	}
	~AboutDialog() {}

	static int AboutVisible;
private:
	Ui::AboutDialogClass ui;
};

#endif