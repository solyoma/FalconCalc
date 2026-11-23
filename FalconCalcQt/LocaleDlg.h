#pragma once
#ifndef _LOCALEDIALOG_H
#define _LOCALEDIALOG_H
#include <QtWidgets/QDialog>
#include <QLocale>
#include "ModernDialog.h"
#include "ui_LocaleDlg.h"

class LocaleDlg : public ModernDialog, private Ui::LocaleDlgClass
{
	Q_OBJECT
public:
	explicit LocaleDlg(QWidget* parent = nullptr);
	~LocaleDlg() {}

	QLocale GetNewLocale() const;

	static int LocaleVisible;
private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
};

#endif