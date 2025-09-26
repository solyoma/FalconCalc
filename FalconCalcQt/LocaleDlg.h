#pragma once
#ifndef _LOCALEDIALOG_H
#define _LOCALEDIALOG_H
#include <QtWidgets/QDialog>
#include <QLocale>
#include "ui_LocaleDlg.h"

class LocaleDlg : public QDialog
{
	Q_OBJECT
public:
	LocaleDlg(QWidget* parent = nullptr);
	~LocaleDlg() {}

	QLocale GetNewLocale() const;

	static int LocaleVisible;
private:
	Ui::LocaleDlgClass ui;
};

#endif