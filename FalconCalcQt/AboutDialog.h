#pragma once
#ifndef _ABOUTDIALOG_H
	#define _ABOUTDIALOG_H
#include "ModernDialog.h"
#include "ui_AboutDialog.h"
#include "version.h"

class AboutDialog : public ModernDialog, private Ui::AboutDialogClass
{
	Q_OBJECT
public:
	AboutDialog(ModernDialogParams params,int version, QWidget* parent = nullptr) : ModernDialog(params, parent)
	{
		Ui::AboutDialogClass::setupUi(ContentWidget());
		resize(FullSize());

		// version: e.g.	0x00 09 02;      // version 0.9.2
		lblVersion->setText(QString(tr("Version %1").arg(VERSION_STRING)) );
		connect(okButton, &QPushButton::clicked, this, [this](){ accept(); });
	}
	~AboutDialog() {}

	static int AboutVisible;
};

#endif