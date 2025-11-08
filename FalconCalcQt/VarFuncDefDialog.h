#pragma once
#ifndef _VARFUNCDEFDIALOG_H
#define _VARFUNCDEFDIALOG_H
#include <QtWidgets/QDialog>
#include <QtWidgets/QAbstractButton>
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
			ui.lblName->setText(tr("&Function:"));
			ui.lblValue->setText(tr("&Definition:"));
			QString qs = vfd.name + "=" + vfd.body + " - " + vfd.comment;
			if (!vfd.unit.isEmpty())
				qs += vfd.unit;
			ui.lblFDefinition->setText(qs);
		}
		else
			ui.lblFDefinition->setVisible(false);
		EnableDisableOkButton();
	}
	void EnableDisableOkButton()
	{
		QString name = ui.edtName->text();
		bool nameValid = name.indexOf('(') >= 0 && name.indexOf(')') >= 0 || (name.indexOf('(') < name.indexOf(')'));
		ui.buttonBox->buttons()[0]->setEnabled(!name.isEmpty() && nameValid && !ui.edtValue->text().isEmpty());
		if (_vfd.isFunction)
		{
			ui.lblFDefinition->setVisible(!_vfd.name.isEmpty());
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

public slots:
	void on_edtName_textChanged()
	{
		EnableDisableOkButton();
	}
	void on_edtValue_textChanged()
	{
		EnableDisableOkButton();
	}
	//void on_edtUnit_textChanged();
	//void on_edtComment_textChanged();
private:
	Ui::VarFuncDefDialogClass ui;
	VarFuncData &_vfd;
};

#endif						   ;