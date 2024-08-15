#pragma once
#ifndef _HELPDIALOG
	#define _HELPDIALOG
	#include <QtWidgets/QDialog>
	#include "ui_HelpDialog.h"

class HelpDialog : public QDialog
{
	Q_OBJECT
public:
	HelpDialog(QWidget* parent = nullptr);
	~HelpDialog();

	static int helpVisible;
private:
	Ui::HelpDialogClass ui;
};

#endif