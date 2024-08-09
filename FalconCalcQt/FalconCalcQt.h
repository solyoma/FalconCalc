#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FalconCalcQt.h"

#include <QSettings>
#include <QTimer>
/*=============================================================
 * TASK:    centralized settings handler
 *      static members are initialized in FalconBoard.cpp!
 *------------------------------------------------------------*/
struct FCSettings
{
    static QString homePath;       // path for user's home directory
    static void Init();            // set home path for falconBoard
    static inline QString Name();
    static QSettings *Open();
    static void Close() { delete _ps; }

private:
    static QSettings* _ps;
    static QString _name;
};

struct SEMAPHORE {
    int b = 0;
    int operator++() { ++b; return b; }
    int operator--() { if (b) --b; return b; }
    operator bool() { return b; }
};

//---------------------------------------------------
namespace FalconCalc {
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
    void on_actionHex_triggered();
    void on_actionHistOptions_triggered();
    void on_actionPasteAfter_triggered();
    void on_actionSelectFont_triggered();
    void on_actionSetLocale_triggered();

    void on_btnBinary_clicked();
    void on_btnDecimal_clicked();
    void on_btnHexaDecimal_clicked();
    void on_btnOctal_clicked();
    void on_btnOpenCloseDecOptions_clicked();
    void on_btnOpenCloseHexOptions_clicked();
    void on_btnStringFont_clicked();

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

    SEMAPHORE _busy;
    bool _decOpen = true;       // during load state before the window is shown?
    bool _hexOpen = true;       //  -"

    int hHexOptionsHeight = 84, // before display these cannot be determined
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

        // user variables/functions are stored in little engine
    int _actTab = -1;           // 0: variables, 1: functions, actual TAB selected on dialog
    int _varColWidths[2][4];    // 0: user, 1: builtins, followed by column wit=dths

    HistoryDialog* _pHist = nullptr;
    VariablesFunctionsDialog* _pVF = nullptr;
    HistoryOptions* _pHistOpt = nullptr;

    int _titleBarHeight =0,
        _borderWidth = 0;
    bool _histDialogSnapped=true, 
        _varfuncDialogSnapped=true;

private: // functions                            r
    QString _SetColoredLabelText(QString text) const
    { 
        return QString("<font color='%1'>%2</font>").arg(_lblTextColor.name()).arg(text); 
    }

    void _EnableMyTimer(bool enable);

    bool _LoadState(QString name);
    bool _SaveState(QString name);
    void _AddToHistory(QString text);
    void _ShowResults();
    void _ShowMessageOnAllPanels(QString s);
    void _EditVarsCommon(int which);

    void _SetDisplayFlags();

    void _LoadHistory();
    void _SaveHistory();

    enum class Placement {pmTop,pmRight,pmBottom,pmLeft};
    void _PlaceWidget(QWidget &widget, Placement pm);   // relative to the main window

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

private: 
    signals:
    void _StartTimer();
    void _StopTimer();
    void _SignalSelectTab(int tab);
    void _SignalSetColWidths(int which, int cw1, int cw2, int cw3, int cw4); //which: -> user, 1->builtin

    void _SignalHistoryChanged(QStringList& hist);

};
