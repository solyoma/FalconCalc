#pragma once
#ifndef _HELPDIALOG_H
	
#define _HELPDIALOG_H
#include <QtWidgets/QDialog>
#include "ui_HelpDialog.h"
#include "ModernDialog.h"

class HelpDialog : public ModernDialog, private Ui::HelpDialogClass
{
	Q_OBJECT
public:
	HelpDialog(QWidget* parent = nullptr);
	~HelpDialog();

	static int helpVisible;

protected:
	void showEvent(QShowEvent* event) override;
};

#endif