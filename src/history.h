#pragma once

class TfrmHistory : public nlib::Form
{
public:
	virtual void Destroy();
	TfrmHistory();
								// 0:unconditional to visible 
	int InsideSnapAreaFromMain(int dist);	// returns area index: 0: none 1 top, 2 right, 3 bottom, 4 left
	void Snap(int dist);		// snap to main window if neare than dist pixels, 
	bool snapped;
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::Listbox *lstHistory;
	nlib::FlatButton *btnDelete;
	nlib::FlatButton *btnClear;
	nlib::Checkbox *chkSorted;
	nlib::Button *btnCancel;

	void btnCancelClick(void *sender, nlib::EventParameters param);
	void chkSortClick(void *sender, nlib::EventParameters param);
	void btnClearClick(void *sender, nlib::EventParameters param);
	void btnDeleteClick(void *sender, nlib::EventParameters param);
	void lstHistoryDblClick(void *sender, nlib::MouseButtonParameters param);
	void FormClose(void *sender, nlib::FormCloseParameters param);
	void FormMove(void *sender, nlib::EventParameters param);
	void FormKeyPress(void *sender, nlib::KeyPressParameters param);
protected:
	virtual ~TfrmHistory(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmHistory *frmHistory;
