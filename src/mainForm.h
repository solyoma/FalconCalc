#pragma once

#include "wcommon.h"
#include <fstream>
#include <string>
#include <locale>
class TStringList;

class TfrmMain : public nlib::Form
{
public:
	TfrmMain();

	void ShowDecOptions(bool show);
	void ShowHexOptions(bool show);
	void OpenVarsOrFunctions(void* sender, int which, nlib::EventParameters param);

	virtual void Destroy();

	TStringList *pslHistory;
	FalconCalc::LittleEngine *pEngine();

	bool InMoving() { return (bool)_inMoving; }

N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::FontDialog* FontDialog1;
	nlib::PopupMenu* pmCopy;
	nlib::MenuItem* miCopyDec;
	nlib::MenuItem* miCopyHex;
	nlib::MenuItem* miCopyOct;
	nlib::MenuItem* miCopyBin;
	nlib::MenuItem* miCopySep;
	nlib::MenuItem* miCopyText;
	nlib::Menubar* mnuMain;
	nlib::MenuItem* miFile;
	nlib::MenuItem* miExit;
	nlib::MenuItem* miEdit;
	nlib::MenuItem* miCopy;
	nlib::MenuItem* miPaste;
	nlib::MenuItem* miAppend;
	nlib::MenuItem* separator1;
	nlib::MenuItem* mnuEditCopy;
	nlib::MenuItem* miCopyDecimal;
	nlib::MenuItem* miCopyHexadecimal;
	nlib::MenuItem* miCopyOctal;
	nlib::MenuItem* miCopyBinary;
	nlib::MenuItem* miData;
	nlib::MenuItem* miEditVars;
	nlib::MenuItem* miEditFuncs;
	nlib::MenuItem* separator2;
	nlib::MenuItem* miClearHist;
	nlib::MenuItem* miView;
	nlib::MenuItem* miShowDecOpts;
	nlib::MenuItem* miShowHexOpts;
	nlib::MenuItem* miCharFont;
	nlib::MenuItem* miHistory;
	nlib::MenuItem* miShowHist;
	nlib::MenuItem* separator3;
	nlib::MenuItem* miHistOpts;
	nlib::MenuItem* miHelp;
	nlib::MenuItem* miAbout;
	nlib::MenuItem* miGenHelp;
	nlib::FlatButton* btnOpenHexOptions;
	nlib::FlatButton* btnOpenDecOptions;
	nlib::Panel* pToolbar;
	nlib::FlatButton* tbExit;
	nlib::Bevel* Bevel1;
	nlib::FlatButton* tbHistory;
	nlib::FlatButton* tbCopy;
	nlib::FlatButton* tbPaste;
	nlib::Edit* edtInfix;
	nlib::Groupbox* gbResults;
	nlib::Button* btnDecimal;
	nlib::Button* btnHexadecimal;
	nlib::Button* btnOctal;
	nlib::Button* btnBinary;
	nlib::Panel* pnlDec;
	nlib::Panel* pnlHex;
	nlib::Panel* pnlOct;
	nlib::Panel* pnlBin;
	nlib::Panel* pnlDecOpt;
	nlib::Groupbox* gbDecOptions;
	nlib::Checkbox* chkSep;
	nlib::Combobox* cbThousandSep;
	nlib::Checkbox* chkSci;
	nlib::Checkbox* chkEng;
	nlib::Checkbox* chkDecDigits;
	nlib::Edit* spnDecDigits;
	nlib::UpDown* UpDownDecDigits;
	nlib::Checkbox* chkDecDelim;
	nlib::Groupbox* gbAngleUnit;
	nlib::Radiobox* rdDeg;
	nlib::Radiobox* rdGrad;
	nlib::Radiobox* rdRad;
	nlib::Radiobox* rdTurn;
	nlib::Groupbox* gbDisplayFormat;
	nlib::Radiobox* rdNormal;
	nlib::Radiobox* rdHtml;
	nlib::Radiobox* rdTex;
	nlib::Radiobox* rdNone;
	nlib::Panel* pnlHexOpt;
	nlib::Groupbox* gbHexOptions;
	nlib::Checkbox* chkMinus;
	nlib::Checkbox* chkLittleEndian;
	nlib::Checkbox* chkBytes;
	nlib::Checkbox* chkWords;
	nlib::Checkbox* chkDWords;
	nlib::Checkbox* chkIEEESingle;
	nlib::Checkbox* chkIEEEDouble;
	nlib::Checkbox* chkHexPrefix;
	nlib::Label* Label1;
	nlib::Edit* edtChars;
	nlib::FlatButton* btnFont;
	nlib::FlatButton* btnCloseDecOptions;
	nlib::FlatButton* btnCloseHexOptions;
	nlib::FlatButton* btnClearInfix;

	void miExitClick(void* sender, nlib::EventParameters param);
	void miCopyClick(void* sender, nlib::EventParameters param);
	void miPasteClick(void* sender, nlib::EventParameters param);
	void miAppendClick(void* sender, nlib::EventParameters param);
	void miCopyDecClick(void* sender, nlib::EventParameters param);
	void miCopyHexClick(void* sender, nlib::EventParameters param);
	void miCopyOctClick(void* sender, nlib::EventParameters param);
	void miCopyBinClick(void* sender, nlib::EventParameters param);
	void miEditVarsClick(void* sender, nlib::EventParameters param);
	void miEditFuncsClick(void* sender, nlib::EventParameters param);
	void miShowHistClick(void* sender, nlib::EventParameters param);
	void miClearHistClick(void* sender, nlib::EventParameters param);
	void miShowDecOptsClick(void* sender, nlib::EventParameters param);
	void miShowHexOptsClick(void* sender, nlib::EventParameters param);
	void miCharFontClick(void* sender, nlib::EventParameters param);
	void miHistOptsClick(void* sender, nlib::EventParameters param);
	void miAboutClick(void* sender, nlib::EventParameters param);
	void miGenHelpClick(void* sender, nlib::EventParameters param);
	void btnOpenHexOptionsClick(void* sender, nlib::EventParameters param);
	void btnOpenDecOptionsClick(void* sender, nlib::EventParameters param);
	void btnCloseHexOptionsClick(void* sender, nlib::EventParameters param);
	void btnCloseDecOptionsClick(void* sender, nlib::EventParameters param);
	void btnFontClick(void* sender, nlib::EventParameters param);
	void chkDoubleClick(void* sender, nlib::EventParameters param);
	void chkSingleClick(void* sender, nlib::EventParameters param);
	void chkAsDWordsClick(void* sender, nlib::EventParameters param);
	void chkAsWordsClick(void* sender, nlib::EventParameters param);
	void chkAsBytesClick(void* sender, nlib::EventParameters param);
	void chkLittleEndianClick(void* sender, nlib::EventParameters param);
	void chkMinusClick(void* sender, nlib::EventParameters param);
	void chkHexPrefixClick(void* sender, nlib::EventParameters param);
	void rdDegClick(void* sender, nlib::EventParameters param);
	void spnDecDigitsTextChanged(void* sender, nlib::EventParameters param);
	void chkDecDigitsClick(void* sender, nlib::EventParameters param);
	void chkDecDelimClick(void* sender, nlib::EventParameters param);
	void chkSciClick(void* sender, nlib::EventParameters param);
	void cbThousandSepChanged(void* sender, nlib::EventParameters param);
	void chkSepClick(void* sender, nlib::EventParameters param);
	void btnCopyFormatClick(void* sender, nlib::EventParameters param);
	void edtInfixTextChanged(void* sender, nlib::EventParameters param);
	void edtInfixKeyDown(void* sender, nlib::KeyParameters param);
	void btnClearInfixClick(void* sender, nlib::EventParameters param);
	void tbPasteClick(void* sender, nlib::EventParameters param);
	void tbCopyClick(void* sender, nlib::EventParameters param);
	void tbHistoryClick(void* sender, nlib::EventParameters param);
	void tbExitClick(void* sender, nlib::EventParameters param);
	void FormClose(void* sender, nlib::FormCloseParameters param);
	void FormMove(void* sender, nlib::EventParameters param);
	void StartMove(void* sender, nlib::EventParameters param);
	void FormSizeMoveEnded(void* sender, nlib::SizePositionChangedParameters param);
	void pnlDecPaint(void* sender, nlib::PaintParameters param);
	void rdNormalClick(void* sender, nlib::EventParameters param);
	//void cbInfixTextChanged(void* sender, nlib::EventParameters param);
	//void cbInfixKeyPress(void* sender, nlib::KeyPressParameters param);
	void chkIEEEDoubleClick(void* sender, nlib::EventParameters param);
	void chkIEEESingleClick(void* sender, nlib::EventParameters param);
protected:
	virtual ~TfrmMain(); /* Don't make public. Call Destroy() to delete the object. */
	virtual LRESULT WindowProc(UINT wMessage, WPARAM w,LPARAM l) override;
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
    int _nDecOptTop,  // when open
        _nHexBtnTop;  // when dec options panel open
	ConditionFlag _busy,
				_inMoving;
    // history options
    bool _added;		 // already added to history by timer, reset by keypress
	bool _bAutoSave;	 // autosave activated?
	int _watchdog;		 // add to history if _watchdog is > given number, reset when formula changes
    int _watchLimit;	 // default: 10: 0 - switch off autosave
    size_t _maxHistDepth;// == 0: unlimited
	size_t _minCharLength = 0;	// 0: all, else # of characters below which there's no save
	std::wstring _wsUserDir;	// store user directory here

    nlib::Checkbox *	_chkArr[7];

	void _EnableMyTimer(bool enable);

	std::wstring _GetUserDir();
	void _GetVirtualDisplaySize();
	bool _LoadState(std::wstring name);
	bool _SaveState(std::wstring name);
	void _AddToHistory(std::wstring text);
	void _ShowResults();
	void _ShowMessageOnAllPanels(std::wstring s);

N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmMain *frmMain;
class WindowInfo;
extern WindowInfo wiMain;
