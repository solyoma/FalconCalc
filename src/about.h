#pragma once

class TfrmAbout : public nlib::Form
{
public:
	virtual void Destroy();
	TfrmAbout();
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::Label *Label1;
	nlib::Label *Label2;
	nlib::Label *Label3;
	nlib::Label *Label4;
	nlib::Label *Label5;
	nlib::Label *Label6;
	nlib::Label *Label7;
	nlib::Label *Label8;

	void frmAboutClick(void *sender, nlib::EventParameters param);
	void frmAboutKeyDown(void *sender, nlib::KeyParameters param);
protected:
	virtual ~TfrmAbout(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmAbout *frmAbout;

