#pragma once

struct VarFuncData
{
	bool isFunction = false; // false = variable, true = function
	std::wstring name;		 // for functions it contains symbolic variables too
	std::wstring body;		 // for variables a number or a formula
	std::wstring unit;		 // only used for variables
	std::wstring comment;	 // for both variables and functions
};

class TVarFuncDefDialog : public nlib::Form
{
public:
	TVarFuncDefDialog(VarFuncData &vfd);
	virtual void Destroy();
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::Label *lblDefinition;
	nlib::Label *lblName;
	nlib::Label *lblBodyValue;
	nlib::Label *lblUnit;
	nlib::Edit *edtName;
	nlib::Edit *edtBody;
	nlib::Edit *edtUnit;
	nlib::Button *btnOk;
	nlib::Button *btnCancel;
	nlib::Label *lblComment;
	nlib::Edit *edtComment;

	void edtName_textChanged(void *sender, nlib::EventParameters param);
	void edtBody_textChanged(void *sender, nlib::EventParameters param);
	void btnOk_click(void* sender, nlib::EventParameters param);
	void btnCancel_click(void* sender, nlib::EventParameters param);
protected:
	virtual ~TVarFuncDefDialog(); /* Don't make public. Call Destroy() to delete the object. */
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
	VarFuncData &_vfd;
	void _EnableDisableOkButton();
	void _Collect(); // collect data from controls to _vfd
	void _ShowDefinition();
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TVarFuncDefDialog *varFuncDefDialog;

