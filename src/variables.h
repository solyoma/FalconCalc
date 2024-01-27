#pragma once

class TfrmVariables : public nlib::Form
{
public:
	virtual void Destroy();
	TfrmVariables();
    void Setup(const FalconCalc::VARFUNC_INFO &vf);
    void GetVarInfo(FalconCalc::VARFUNC_INFO &dest);
N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::TabControl *tcVars;
	nlib::StringGrid *sgUser;
	nlib::Panel *pBuiltin;
	nlib::StringGrid *sgBuiltin;
	nlib::FlatButton *btnDelVar;
	nlib::FlatButton *btnClear;
	nlib::Button *btnCancel;
	nlib::Button *btnSave;

	void btnSaveClick(void *sender, nlib::EventParameters param);
	void btnCancelClick(void *sender, nlib::EventParameters param);
	void btnClearClick(void *sender, nlib::EventParameters param);
	void btnDelVarClick(void *sender, nlib::EventParameters param);
	void sgBuiltinColumnSizing(void *sender, nlib::ColumnRowSizeParameters param);
	void BuiltinMouseUp(void *sender, nlib::MouseButtonParameters param);
	void BuiltinMouseDown(void *sender, nlib::MouseButtonParameters param);
	void BuiltinMouseMove(void *sender, nlib::MouseMoveParameters param);
	void sgUserKeyPress(void *sender, nlib::KeyPressParameters param);
	void sgUserEditorKeyDown(void *sender, nlib::KeyParameters param);
	void sgUserColumnSizing(void *sender, nlib::ColumnRowSizeParameters param);
	void tcVarsTabChange(void *sender, nlib::TabChangeParameters param);
	void FormClose(void *sender, nlib::FormCloseParameters param);
protected:
	virtual ~TfrmVariables(); /* Don't make public. Call Destroy() to delete the object. */ 
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
    FalconCalc::VARFUNC_INFO _vf;
    bool _changed;
	bool _underResize;		// user/builtin function pane
	int  _gridH;				// size of user var/function grid during resize
    void _CollectInto(SmartString &dest, size_t &cnt);    // from stringrid on actual page
N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmVariables *frmVariables;

