#pragma once

class TfrmHistOptions : public nlib::Form
{
public:
	virtual void Destroy();
    TfrmHistOptions();
	void Setup(size_t depth, size_t timeout, bool sorted, int minLength);
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::Panel* Panel1;
	nlib::Checkbox *chkDepth;
	nlib::Checkbox *chkAutoSave;
	nlib::Checkbox *chkSort;
	nlib::Edit *spinDepth;
	nlib::UpDown *spinDepthBtn;
	nlib::Edit *edtInterval;
	nlib::Checkbox* cbClearHist;
	nlib::Button *btnOk;
	nlib::Button *btnCancel;
	nlib::Label *Label1;
	nlib::Edit *edtMinLength;
	nlib::UpDown *sbMinLength;
	nlib::Label *Label2;
	nlib::Label* lblNone;


	void btnCancelClick(void *sender, nlib::EventParameters param);
	void btnOkClick(void *sender, nlib::EventParameters param);
	void cbClearHistClick(void* sender, nlib::EventParameters param);
	void edtIntervalKeyPressed(void* sender, nlib::KeyPressParameters param);
	void edtSpinDepthKeyPressed(void* sender, nlib::KeyPressParameters param);
	void edtMinLengthKeyPressed(void* sender, nlib::KeyPressParameters param);
	void chkSortClick(void *sender, nlib::EventParameters param);
	void chkAutoSaveClick(void *sender, nlib::EventParameters param);
	void chkDepthClick(void *sender, nlib::EventParameters param);

	constexpr bool ClearHistory() const { return _clearhistory; }
protected:
    virtual ~TfrmHistOptions(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:

	bool _clearhistory = false;

	void _DropInvalidKeys(nlib::KeyParameters& param);
	void _DropInvalidKeys(nlib::KeyPressParameters& param);
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmHistOptions *frmHistOptions;
wchar_t TimeSeparator();	// yet returns ':'
