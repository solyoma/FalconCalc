#pragma once

#include <fstream>
#include <string>
class TStringList;

namespace FalconCalc {
	class LittleEngine;
}

class TfrmMain : public nlib::Form
{
public:
	TfrmMain();

	void ShowDecOptions(bool show);
	void ShowHexOptions(bool show);

	virtual void Destroy();

	TStringList *slHistory;
	FalconCalc::LittleEngine *_pEngine();

N_PUBLIC: /* Designer generated list of public members. Do not edit by hand. */
	nlib::FontDialog *FontDialog1;
	nlib::PopupMenu *pmCopy;
	nlib::MenuItem *miCopyDec;
	nlib::MenuItem *miCopyHex;
	nlib::MenuItem *miCopyOct;
	nlib::MenuItem *miCopyBin;
	nlib::MenuItem *miCopySep;
	nlib::MenuItem *miCopyText;
	nlib::Menubar *mnuMain;
	nlib::MenuItem *miFile;
	nlib::MenuItem *miExit;
	nlib::MenuItem *miEdit;
	nlib::MenuItem *miCopy;
	nlib::MenuItem *miPaste;
	nlib::MenuItem *miAppend;
	nlib::MenuItem *MenuItem1;
	nlib::MenuItem *miCopyDecimal;
	nlib::MenuItem *miCopyHexadecimal;
	nlib::MenuItem *miCopyOctal;
	nlib::MenuItem *miCopyBinary;
	nlib::MenuItem *MenuItem2;
	nlib::MenuItem *miEditVars;
	nlib::MenuItem *miEditFuncs;
	nlib::MenuItem *MenuItem3;
	nlib::MenuItem *miShowHist;
	nlib::MenuItem *miClearHist;
	nlib::MenuItem *miOptions;
	nlib::MenuItem *miShowDecOpts;
	nlib::MenuItem *miShowHexOpts;
	nlib::MenuItem *miCharFont;
	nlib::MenuItem *miHistOpts;
	nlib::MenuItem *miHelp;
	nlib::MenuItem *miAbout;
	nlib::MenuItem *miGenHelp;
	nlib::FlatButton *btnOpenHexOptions;
	nlib::FlatButton *btnOpenDecOptions;
	nlib::Panel *pToolbar;
	nlib::FlatButton *tbExit;
	nlib::Bevel *Bevel1;
	nlib::FlatButton *tbHistory;
	nlib::FlatButton *tbCopy;
	nlib::FlatButton *tbPaste;
	nlib::Edit *edtInfix;
	nlib::Groupbox *gbResults;
	nlib::Button *btnDecimal;
	nlib::Button *btnHexadecimal;
	nlib::Button *btnOctal;
	nlib::Button *btnBinary;
	nlib::Panel *pnlDec;
	nlib::Panel *pnlHex;
	nlib::Panel *pnlOct;
	nlib::Panel *pnlBin;
	nlib::Panel *pnlDecOpt;
	nlib::Groupbox *Groupbox1;
	nlib::Checkbox *chkSep;
	nlib::Combobox *cbThousandSep;
	nlib::Checkbox *chkSci;
	nlib::Checkbox *chkEng;
	nlib::Checkbox *chkDecDigits;
	nlib::Edit *spnDecDigits;
	nlib::UpDown *UpDown1;
	nlib::Groupbox *rgAngleUnit;
	nlib::Radiobox *rdDeg;
	nlib::Radiobox *rdGrad;
	nlib::Radiobox *rdRad;
	nlib::Groupbox *Groupbox3;
	nlib::Radiobox *rdNormal;
	nlib::Radiobox *rdHtml;
	nlib::Radiobox *rdTex;
	nlib::Radiobox *rdNone;
	nlib::Panel *pnlHexOpt;
	nlib::Groupbox *Groupbox2;
	nlib::Checkbox *chkMinus;
	nlib::Checkbox *chkLittleEndian;
	nlib::Checkbox *chkBytes;
	nlib::Checkbox *chkWords;
	nlib::Checkbox *chkDWords;
	nlib::Checkbox *chkIEEESingle;
	nlib::Checkbox *chkIEEEDouble;
	nlib::Combobox *cbInfix;
	nlib::Label *Label1;
	nlib::Edit *edtChars;
	nlib::FlatButton *btnFont;
	nlib::FlatButton *btnCloseDecOptions;
	nlib::FlatButton *btnCloseHexOptions;

	void miExitClick(void *sender, nlib::EventParameters param);
	void miCopyClick(void *sender, nlib::EventParameters param);
	void miPasteClick(void *sender, nlib::EventParameters param);
	void miAppendClick(void *sender, nlib::EventParameters param);
	void miCopyDecClick(void *sender, nlib::EventParameters param);
	void miCopyHexClick(void *sender, nlib::EventParameters param);
	void miCopyOctClick(void *sender, nlib::EventParameters param);
	void miCopyBinClick(void *sender, nlib::EventParameters param);
	void miEditVarsClick(void *sender, nlib::EventParameters param);
	void miEditFuncsClick(void *sender, nlib::EventParameters param);
	void miShowHistClick(void *sender, nlib::EventParameters param);
	void miClearHistClick(void *sender, nlib::EventParameters param);
	void miShowDecOptsClick(void *sender, nlib::EventParameters param);
	void miShowHexOptsClick(void *sender, nlib::EventParameters param);
	void miCharFontClick(void *sender, nlib::EventParameters param);
	void miHistOptsClick(void *sender, nlib::EventParameters param);
	void miAboutClick(void *sender, nlib::EventParameters param);
	void miGenHelpClick(void *sender, nlib::EventParameters param);
	void btnOpenHexOptionsClick(void *sender, nlib::EventParameters param);
	void btnOpenDecOptionsClick(void *sender, nlib::EventParameters param);
	void btnCloseHexOptionsClick(void *sender, nlib::EventParameters param);
	void btnCloseDecOptionsClick(void *sender, nlib::EventParameters param);
	void btnFontClick(void *sender, nlib::EventParameters param);
	void chkIEEEDoubleClick(void *sender, nlib::EventParameters param);
	void chkIEEESingleClick(void *sender, nlib::EventParameters param);
	void chkAsDWordsClick(void *sender, nlib::EventParameters param);
	void chkAsWordsClick(void *sender, nlib::EventParameters param);
	void chkAsBytesClick(void *sender, nlib::EventParameters param);
	void chkLittleEndianClick(void *sender, nlib::EventParameters param);
	void chkMinusClick(void *sender, nlib::EventParameters param);
	void rdDegClick(void *sender, nlib::EventParameters param);
	void spnDecDigitsTextChanged(void *sender, nlib::EventParameters param);
	void chkDecDigitsClick(void *sender, nlib::EventParameters param);
	void chkSciClick(void *sender, nlib::EventParameters param);
	void cbThousandSepChanged(void *sender, nlib::EventParameters param);
	void chkSepClick(void *sender, nlib::EventParameters param);
	void btnCopyFormatClick(void *sender, nlib::EventParameters param);
	void edtInfixTextChanged(void *sender, nlib::EventParameters param);
	void edtInfixKeyDown(void *sender, nlib::KeyParameters param);
	void tbPasteClick(void *sender, nlib::EventParameters param);
	void tbCopyClick(void *sender, nlib::EventParameters param);
	void tbHistoryClick(void *sender, nlib::EventParameters param);
	void tbExitClick(void *sender, nlib::EventParameters param);
	void FormClose(void *sender, nlib::FormCloseParameters param);
	void FormMove(void *sender, nlib::EventParameters param);
	void StartMove(void *sender, nlib::EventParameters param);
	void pnlDecPaint(void *sender, nlib::PaintParameters param);
	void rdNormalClick(void *sender, nlib::EventParameters param);
	void cbInfixTextChanged(void *sender, nlib::EventParameters param);
	void cbInfixKeyPress(void *sender, nlib::KeyPressParameters param);
protected:
	virtual ~TfrmMain(); /* Don't make public. Call Destroy() to delete the object. */ 
	virtual LRESULT WindowProc(UINT wMessage, WPARAM w,LPARAM l) override;
N_PROTECTED: /* Designer generated list of protected members. Do not edit by hand. */
private:
    int nDecOptTop,  // when open
        nHexBtnTop;  // when dec options panel open
	bool busy;
    // history options
    bool added;			// already added to history by timer, reset by keypress
	bool bAutoSave;		// autosave activated?
	int coMoveDX,		// not 0 then  move history window together with main 
		coMoveDY;		//  x and y coord difference between left tops of the windows
	int watchdog;     // add to history if watchdog is > given number, reset when formula changes
    int watchLimit;   // default: 10: 0 - switch off autosave
    size_t maxHistDepth; // == 0: unlimited

    Checkbox *chkArr[7];

	void EnableMyTimer(bool enable);
	bool LoadState(std::wstring name);
	bool SaveState(std::wstring name);
	void AddToHistory(std::wstring text);
	void ShowResults();
	void ShowMessageOnAllPanels(std::wstring s);

N_PRIVATE: /* Designer generated list of private members. Do not edit by hand. */
	void InitializeFormAndControls(); /* Control initializations. Do not remove. */
};

extern TfrmMain *frmMain;

