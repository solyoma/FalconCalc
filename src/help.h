#pragma once

class TfrmHelp : public nlib::Form
{
public:
	virtual void Destroy();
	TfrmHelp();
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::Label *Label1;
	nlib::Label *Label2;
	nlib::Label *Label3;

	void Label3Click(void *sender, nlib::EventParameters param);
	void frmHelpClick(void *sender, nlib::EventParameters param);
	void frmHelpKeyDown(void *sender, nlib::KeyParameters param);
protected:
	virtual ~TfrmHelp(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmHelp *frmHelp;

