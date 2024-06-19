#pragma once

class ConditionFlag;

class TfrmHistory : public nlib::Form
{
public:
	virtual void Destroy();
	TfrmHistory();
								// 0:unconditional to visible 
	FalconCalc::WindowSide GetSnapSide();
	void Snap();				// snap this window to the side of the main window using '_snappedSide' and '_snapDist'
	constexpr bool Snapped() const { return _snappedToSide != FalconCalc::wsNone; }
	constexpr FalconCalc::WindowSide SnappedSide() const { return _snappedToSide; }

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
	void FormSizeMoveEnded(void* sender, nlib::SizePositionChangedParameters param);
protected:
	virtual ~TfrmHistory(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
	ConditionFlag _busy;
	FalconCalc::WindowSide _snappedToSide = FalconCalc::wsNone;
	int _snapDist = 0;				// from '_side'
	int _snapPixelLimit = 10;		// snap if inside this distance from, main window

N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmHistory *frmHistory;
