#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FalconCalcQt.h"

#include <QSettings>
#include <QTimer>

#include "common.h"
#include "schemes.h"

#include "EngineErrors.h"

#include "HelpDialog.h"

struct SEMAPHORE {
    int b = 0;
    int operator++() { ++b; return b; }
    int operator--() { if (b) --b; return b; }
    operator bool() { return b; }
};

//---------------------------------------------------
namespace FalconCalc {
    enum WindowSide  { wsNone, wsTop, wsRight, wsBottom, wsLeft };
    class LittleEngine;
}

class VariablesFunctionsDialog;
class HistoryDialog;
class HistoryOptions;

class FalconCalcQt : public QMainWindow
{
    Q_OBJECT

public:
    FalconCalcQt(QWidget *parent = nullptr);
    ~FalconCalcQt();

    QStringList pslHistory;

protected:
    void moveEvent(QMoveEvent* event);
    void showEvent(QShowEvent* event);

private slots:
    void on_actionAbout_triggered();
    void on_actionClearHistory_triggered();
    void on_actionDec_triggered();
    void on_actionHelp_triggered();
    void on_actionEditHist_triggered();
    void on_actionEditVars_triggered();
    void on_actionEditFunc_triggered();
    void on_actionEnglish_triggered();
    void on_actionHex_triggered();
    void on_actionHistOptions_triggered();
    void on_actionHungarian_triggered();
    void on_actionPasteAfter_triggered();
    void on_actionSelectFont_triggered();
    void on_actionSetLocale_triggered();
    void on_actionSystemMode_triggered();
    void on_actionLightMode_triggered();
    void on_actionDarkMode_triggered();
    void on_actionBlackMode_triggered();
    void on_actionBlueMode_triggered();

	void on_cbInfix_currentTextChanged(const QString& newText);

    void on_btnBinary_clicked();
    void on_btnDecimal_clicked();
    void on_btnHexaDecimal_clicked();
    void on_btnOctal_clicked();
    void on_btnOpenCloseDecOptions_clicked();
    void on_btnOpenCloseHexOptions_clicked();
    void on_btnStringFont_clicked();
    void on_btnClearInfix_clicked();

    void on_cbThousandSep_currentIndexChanged(int newIndex);

    void on_chkDecDelim_toggled(bool b);
    void on_chkDecDigits_toggled(bool b);
    void on_chkEng_toggled(bool b);
    void on_chkSep_toggled(bool b);
    void on_chkSci_toggled(bool b);
    void on_chkMinus_toggled(bool b);
    void on_chkHexPrefix_toggled(bool b);
    void on_chkBytes_toggled(bool b);
    void on_chkWords_toggled(bool b);
    void on_chkDWords_toggled(bool b);
    void on_chkIEEESingle_toggled(bool b);
    void on_chkIEEEDouble_toggled(bool b);
    void on_chkLittleEndian_toggled(bool b);

    void on_edtInfix_textChanged(const QString& newText);

    void on_rbDeg_toggled(bool b);
    void on_rbRad_toggled(bool b);
    void on_rbGrad_toggled(bool b);
    void on_rbTurns_toggled(bool b);
    void on_rbNormal_toggled(bool b);
    void on_rbHtml_toggled(bool b);
    void on_rbTex_toggled(bool b);
    void on_rbNone_toggled(bool b);

    void on_spnDecDigits_valueChanged(int val);
private:
    Ui::FalconCalcQtClass ui;

	int _version=0x000900;      // version 0.9.0
	AppLanguage _appLanguage = AppLanguage::lanNotSet;

    SEMAPHORE _busy;
    bool _decOpen = true;       // during load state before the window is shown?
    bool _hexOpen = true;       //  -"
    FSchemeVector *_schemeVector = nullptr;
    Scheme _actScheme = Scheme::schSystem;

    int hHexOptionsHeight = 78, // before display these cannot be determined
        hDecOptionsHeight = 124;
    bool _bAlreadyShown = false;
    QColor _lblTextColor;
        // history
    bool _added=false;		    // already added to history by timer, reset by keypress
    bool _bAutoSave=false;	    // autosave history activated?
    size_t _maxHistDepth=100;   // == 0: unlimited
    int _minCharLength=3;	// 0: all, else # of characters below which there's no save
    bool _historySorted = false;//  if true sort history after adding item

    int _watchTimeout=10;	    // default: 10 sec: 0 - switch off autosave
    QTimer _watchdogTimer;
    QStringList _slHistory;     // history list

    QClipboard *_clipBoard = QGuiApplication::clipboard();

        // user variables/functions are stored in little engine
    int _actTab = -1;           // 0: variables, 1: functions, actual TAB selected on dialog
    int _varColWidths[2][4];    // 0: user, 1: builtins, followed by column wit=dths

    HistoryDialog* _pHist = nullptr;
    VariablesFunctionsDialog* _pVF = nullptr;

    int _titleBarHeight =0,
        _borderWidth = 0;
    bool _histDialogSnapped=true, 
        _varfuncDialogSnapped=true;

private: // functions
    QString _SetColoredLabelText(QString text) const
    { 
        return QString("<font color='%1'>%2</font>").arg(_lblTextColor.name()).arg(text); 
    }

    void _EnableMyTimer(bool enable);

	int _GetVersion(QStringRef s) const; // from "x.y.z" to 0xXXYYZZ (0xXX = x, 0xYY = y, 0xZZ = z)
    bool _LoadState();
    bool _SaveState();
    void _AddToHistory(QString text);
    void _ShowResults();
    void _ShowMessageOnAllPanels(QString s);
    void _EditVarsCommon(int which);

    void _SetHexDisplFomatForFlags();

    void _LoadHistory();
    void _SaveHistory();

    enum class Placement {pmTop,pmRight,pmBottom,pmLeft};
    void _PlaceWidget(QWidget &widget, Placement pm);   // relative to the main window
    void _SetResultLabelText(EngineErrorCodes eec);

private slots:
    void _watchdogTimerSlot();
    void _SlotDataFromHistory(QString qs);
    void _SlotRemoveHistLine(int row);
    void _SlotClearHistory();
    void _SlotHistClosing();
    void _SlotHistOptions();
    void _SlotHistMoved();
    
    void _SlotVarsFuncsClosing();
    void _SlotVarFuncMoved();
    void _SlotVarTabChanged(int newTab);
    void _SlotVarFuncTableDoubleClicked(QString& name);

    //void _SlotVarFuncSaved(FalconCalc::RowDataMap& vf);

private: 
    signals:
    void _StartTimer();
    void _StopTimer();
    void _SignalSelectTab(int tab);
    void _SignalSetColWidths(int which, int cw1, int cw2, int cw3, int cw4); //which: -> user, 1->builtin

    void _SignalHistoryChanged(QStringList& hist);

};
