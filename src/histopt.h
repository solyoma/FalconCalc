#pragma once

class TfrmHistOptions : public nlib::Form
{
public:
	virtual void Destroy();
    TfrmHistOptions();
	void Setup(size_t depth, size_t timeout, bool sorted);
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::Checkbox *chkDepth;
	nlib::Checkbox *chkAutoSave;
	nlib::Checkbox *chkSort;
	nlib::Edit *spinDepth;
	nlib::UpDown *spinDepthBtn;
	nlib::Edit *edtInterval;
	nlib::Button *btnOk;
	nlib::Button *btnCancel;

	void btnCancelClick(void *sender, nlib::EventParameters param);
	void btnOkClick(void *sender, nlib::EventParameters param);
	void edtIntervalKeyDown(void *sender, nlib::KeyParameters param);
	void chkSortClick(void *sender, nlib::EventParameters param);
	void chkAutoSaveClick(void *sender, nlib::EventParameters param);
	void chkDepthClick(void *sender, nlib::EventParameters param);
protected:
    virtual ~TfrmHistOptions(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmHistOptions *frmHistOptions;
wchar_t TimeSeparator();	// yet returns ':'
