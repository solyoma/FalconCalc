#pragma once
#ifndef _VARFUNCDEFDIALOG_H
#define _VARFUNCDEFDIALOG_H
#include <QtWidgets/QDialog>
#include <QtWidgets/QAbstractButton>
#include "ModernDialog.h"
#include "ui_VarFuncDefDialog.h"

struct VarFuncData
{
	bool isFunction = false; // false = variable, true = function
	QString name;		 // for functions it contains symbolic variables too
	QString body;		 // for variables a number or a formula
	QString unit;		 // only used for variables
	QString comment;	 // for both variables and functions
};

class VarFuncDefDialog : public ModernDialog, private Ui::VarFuncDefDialogClass
{
	Q_OBJECT
public:						 // vfd changed on dialog acceptance
	VarFuncDefDialog(VarFuncData &vfd, ModernDialogParams& params, QWidget* parent = nullptr) : _vfd(vfd), ModernDialog(params, parent)
	{
		Ui::VarFuncDefDialogClass::setupUi(ContentWidget());
		resize(FullSize());
		QMetaObject::connectSlotsByName(this);
		connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

		edtName->setText(vfd.name);
		edtValue->setText(vfd.body);
		edtUnit->setText(vfd.unit);
		edtComment->setText(vfd.comment);
		if (vfd.isFunction)
		{
			lblName->setText(tr("&Function:"));
			lblValue->setText(tr("&Definition:"));
			QString qs = vfd.name + "=" + vfd.body + " - " + vfd.comment;
			if (!vfd.unit.isEmpty())
				qs += vfd.unit;
			lblFDefinition->setText(qs);
		}
		else
			lblFDefinition->setVisible(false);
		EnableDisableOkButton();
	}
	void EnableDisableOkButton()
	{
		QString name = edtName->text();
		bool nameValid = (name.indexOf('(') >= 0) && (name.indexOf(')') >= 0) && (name.indexOf('(') < name.indexOf(')'));
		buttonBox->buttons()[0]->setEnabled(!name.isEmpty() && nameValid && !edtValue->text().isEmpty());
		if (_vfd.isFunction)
		{
			lblFDefinition->setVisible(!_vfd.name.isEmpty());
		}
	}

	void accept() override
	{
		_vfd.name    = edtName->text();
		_vfd.body    = edtValue->text();
		_vfd.unit    = edtUnit->text();
		_vfd.comment = edtComment->text();
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

#endif