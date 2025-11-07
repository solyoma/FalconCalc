#pragma once
#ifndef _VARFUNCDEFDIALOG_H
#define _VARFUNCDEFDIALOG_H
#include <QtWidgets/QDialog>
#include "ui_VarFuncDefDialog.h"

struct VarFuncData
{
	bool isFunction = false; // false = variable, true = function
	QString name;		 // for functions it contains symbolic variables too
	QString body;		 // for variables a number or a formula
	QString unit;		 // only used for variables
	QString comment;	 // for both variables and functions
};

class VarFuncDefDialog : public QDialog
{
	Q_OBJECT
public:						 // vfd changed on dialog acceptance
	VarFuncDefDialog(VarFuncData &vfd, QWidget* parent = nullptr) : _vfd(vfd), QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
	{
		ui.setupUi(this);
		ui.edtName->setText(vfd.name);
		ui.edtValue->setText(vfd.body);
		ui.edtUnit->setText(vfd.unit);
		ui.edtComment->setText(vfd.comment);
		if (vfd.isFunction)
		{
			ui.lblName->setText (tr("&Function:"));
			ui.lblValue->setText(tr("&Definition:"));
			QString qs = vfd.name + "=" + vfd.body + " - " + vfd.comment;
			if(!vfd.unit.isEmpty() )
				qs += vfd.unit;
			ui.lblFDefinition->setText(qs);
		}
	
	}
	void accept() override
	{
		_vfd.name    = ui.edtName->text();
		_vfd.body    = ui.edtValue->text();
		_vfd.unit    = ui.edtUnit->text();
		_vfd.comment = ui.edtComment->text();
		QDialog::accept();
	}
	~VarFuncDefDialog() {}

private:
	Ui::VarFuncDefDialogClass ui;
	VarFuncData &_vfd;
};

#endif