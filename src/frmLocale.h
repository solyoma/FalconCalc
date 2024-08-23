#pragma once

class TfrmLocale : public nlib::Form
{
public:
	TfrmLocale();
	virtual void Destroy();
public:
	std::wstring LocaleName() const;

N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::Button *btnSave;
	nlib::Button *btnCancel;
	nlib::Label *lblText;
	nlib::Label *lblInput;
	nlib::Edit *edtLocale;
	nlib::Label *lblOldLocale;
	nlib::Panel* Panel1;
	nlib::Label *Label1;
	nlib::Label *Label2;

	void btnSaveClick(void *sender, nlib::EventParameters param);
	void btnCancelClick(void *sender, nlib::EventParameters param);
	void edtLocaleKeypress(void* sender, nlib::KeyPressParameters param);
protected:
	virtual ~TfrmLocale(); /* Don't make public. Call Destroy() to delete the object. */
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
	std::wstring _wsLocName;
private:
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmLocale *frmLocale;

