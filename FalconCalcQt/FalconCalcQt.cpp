#include "FalconCalcQt.h"

#include <QDir>
#include <QFile>
#include <QtextStream>
#include <QString>
#include <QSettings>
#include <QTimer>
#include <QClipboard>
#include <QtGui>
#include <QMessageBox>
#include <QFontDialog>

#include <vector>

#include "SmartString.h"
using namespace SmString;

#include "LongNumber.h"
using namespace LongNumber;

#include "calculate.h"
using namespace FalconCalc;

#include "AboutDialog.h"
#include "VariablesFunctionsDialog.h"
#include "HistoryDialog.h"
#include "HistoryOptions.h"
#include "HelpDialog.h"
#include "FCSettings.h"
#include "schemes.h"

//#ifdef NothingImportant
#ifdef _DEBUG
#include <QFile>
#include <QTextStream>
static void __SaveStyle(QString styleName)
{
	QFile of(styleName + ".sty");
	of.open(QIODevice::WriteOnly);
	QTextStream ofs(&of);
	ofs << ((QApplication*)(QApplication::instance()))->styleSheet() << "\n";
}
#else
	#define __SaveStyle(a) 
#endif
//#endif

FalconCalc::LittleEngine* lengine = nullptr;

QString STATE_VER_STRING = QStringLiteral("FalconCalc State File V1.0");

//------------------------ FalconCalcQt --------------

const QString FalconCalcQt_HIST_FILE= "FalconCalc.hist";
const QString FalconCalcQt_DAT_FILE = "FalconCalc.dat";
const QString FalconCalcQt_CFG_FILE = "FalconCalc.cfg";

FalconCalcQt::FalconCalcQt(QWidget *parent)  : QMainWindow(parent)
{
    ui.setupUi(this);
	FCSettings::Init();

// make mode selection actions exclusive
	QActionGroup* actionGroup = new QActionGroup(this);
	actionGroup->setExclusive(true);
	actionGroup->addAction(ui.actionSystemMode);
	actionGroup->addAction(ui.actionLightMode);
	actionGroup->addAction(ui.actionDarkMode);
	actionGroup->addAction(ui.actionBlackMode);
	actionGroup->addAction(ui.actionBlueMode);
		// same for variable
	actionGroup = new QActionGroup(this);
	actionGroup->setExclusive(true);
	actionGroup->addAction(ui.actionEditVars);
	actionGroup->addAction(ui.actionEditFunc);
	lengine = new LittleEngine;
	lengine->displayFormat.useNumberPrefix = ui.chkHexPrefix->isChecked();
	lengine->displayFormat.strThousandSeparator = " "_ss;
	lengine->displayFormat.displWidth = 59;

	_schemeVector = new FSchemeVector();	// with color schemes light, dark, black and blue
	_LoadState(FCSettings::homePath + FalconCalcQt_CFG_FILE);

	_LoadHistory();


	lengine->ssNameOfDatFile = SmartString(FCSettings::homePath +  FalconCalcQt_DAT_FILE);
	try
	{
		lengine->LoadUserData();	// may throw because of many errors
	}
	catch (...)
	{
		;
	}

	_watchdogTimer.setInterval(_watchTimeout*1000);

	for (int i = 0; i < 4; ++i)
	{
		_varColWidths[0][i] =
			_varColWidths[1][i] = 100;
	}

	connect(this, &FalconCalcQt::_StopTimer, &_watchdogTimer, &QTimer::stop);
	connect(this, SIGNAL(_StartTimer()), &_watchdogTimer, SLOT(start()) );
	connect(&_watchdogTimer, &QTimer::timeout, this, &FalconCalcQt::_watchdogTimerSlot);
	emit _StartTimer();
}

FalconCalcQt::~FalconCalcQt()     
{
	lengine->SaveUserData();
	_SaveState(FCSettings::homePath + FalconCalcQt_CFG_FILE);
	_SaveHistory();
	delete lengine;
	delete _schemeVector;
}

void FalconCalcQt::showEvent(QShowEvent * event)
{
    if (!_bAlreadyShown)
    {
        hDecOptionsHeight = ui.frmDecimal->height();
        hHexOptionsHeight = ui.gbHexOptions->height();
		if (!_decOpen)
			on_btnOpenCloseDecOptions_clicked();
		if (!_hexOpen)
			on_btnOpenCloseHexOptions_clicked();

		_borderWidth = geometry().left() - pos().x(),
		_titleBarHeight = frameGeometry().height() - geometry().height() - 2 * _borderWidth;
	}
	if (isVisible())
	{
		_bAlreadyShown = true;
		ui.edtInfix->setFocus();
	}
}

/*=============================================================
 * TASK   :	when the main window moves if there are open 
 *			history or variable window coupled with it it will move
 *			those too
 * PARAMS :	e:
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS:
 *------------------------------------------------------------*/
void FalconCalcQt::moveEvent(QMoveEvent* e)
{
	if (_busy)
		return;

	if (_pHist && _histDialogSnapped)
	{
		++_busy;

		int x0 = e->oldPos().x(), y0 = e->oldPos().y(), 
			x = e->pos().x(), y = e->pos().y(), 
			xh0 = _pHist->frameGeometry().left(), yh0 = _pHist->frameGeometry().top(),
			dx = x - x0, dy = y - y0;

		//qDebug("main: (%d, %d)->(%d, %d), delta (%d,%d)", x0, y0, x, y, dx, dy);
		//qDebug(" hist: (%d, %d)->(%d, %d), delta (%d,%d)", xh0, yh0, xh0 + dx, yh0 + dy, dx, dy);

		_pHist->move(xh0 + dx, yh0 + dy);

		//int xh = _pHist->frameGeometry().left(), yh = _pHist->frameGeometry().top();
		//qDebug("    moved hist: (%d, %d)", xh, yh);
		//_pHist->move(_pHist->geometry().left() + e->pos().x() - e->oldPos().x(), _pHist->geometry().top() + e->pos().y() - e->oldPos().y());
		--_busy;
	}
	if (_pVF && _varfuncDialogSnapped)
	{
		++_busy;

		int x0 = e->oldPos().x(), y0 = e->oldPos().y(), 
			x = e->pos().x(), y = e->pos().y(), 
			xh0 = _pVF->frameGeometry().left(), yh0 = _pVF->frameGeometry().top(),
			dx = x - x0, dy = y - y0;

		_pVF->move(xh0 + dx, yh0 + dy);
		--_busy;
	}
}

void FalconCalcQt::on_actionPasteAfter_triggered()
{
	QString qs = ui.edtInfix->text()+QGuiApplication::clipboard()->text();
	ui.edtInfix->setText(qs);
	ui.edtInfix->setFocus();
}

void FalconCalcQt::_SlotHistClosing()
{
	ui.actionEditHist->setChecked(false);
	if (_pHist)
	{
		disconnect(_pHist, &HistoryDialog::SignalSelection, this, &FalconCalcQt::_SlotDataFromHistory);
		disconnect(_pHist, &HistoryDialog::SignalRemoveHistLine, this, &FalconCalcQt::_SlotRemoveHistLine);
		disconnect(_pHist, &HistoryDialog::SignalClearHistory, this, &FalconCalcQt::_SlotClearHistory);
		disconnect(_pHist, &HistoryDialog::SignalHistClose, this, &FalconCalcQt::_SlotHistClosing);
		disconnect(_pHist, &HistoryDialog::SignalHistOptions, this, &FalconCalcQt::_SlotHistOptions);
		disconnect(_pHist, &HistoryDialog::SignalHistMoved, this, &FalconCalcQt::_SlotHistMoved);
		delete _pHist;
		_pHist = nullptr;
	}
}

void FalconCalcQt::on_actionEditHist_triggered()
{
	if (_pHist)		// then hide
	{
		_SlotHistClosing();
	}
	else			// create and show history dialog
	{
		_pHist = new HistoryDialog(_slHistory, this);
		ui.actionEditHist->setChecked(true);
		connect(_pHist, &HistoryDialog::SignalSelection		, this, &FalconCalcQt::_SlotDataFromHistory);
		connect(_pHist, &HistoryDialog::SignalRemoveHistLine, this, &FalconCalcQt::_SlotRemoveHistLine );
		connect(_pHist, &HistoryDialog::SignalClearHistory	, this, &FalconCalcQt::_SlotClearHistory);
		connect(_pHist, &HistoryDialog::SignalHistClose			, this, &FalconCalcQt::_SlotHistClosing);
		connect(_pHist, &HistoryDialog::SignalHistOptions	, this, &FalconCalcQt::_SlotHistOptions);
		connect(_pHist, &HistoryDialog::SignalHistMoved			, this, &FalconCalcQt::_SlotHistMoved);
		_PlaceWidget(*_pHist, Placement::pmBottom);
		_pHist->show();
		_histDialogSnapped = true;
	}
	this->activateWindow();		// get the focus back
	this->raise();
	ui.edtInfix->setFocus();
}

void FalconCalcQt::_SlotVarsFuncsClosing()
{
	if (_pVF)
	{
		disconnect(_pVF, &VariablesFunctionsDialog::SignalVarFuncClose, this, &FalconCalcQt::_SlotVarsFuncsClosing);
		disconnect(_pVF, &VariablesFunctionsDialog::SignalTabChange, this, &FalconCalcQt::_SlotVarTabChanged);
		disconnect(this, &FalconCalcQt::_SignalSelectTab, _pVF, &VariablesFunctionsDialog::SlotSelectTab);
		disconnect(this, &FalconCalcQt::_SignalSetColWidths, _pVF, &VariablesFunctionsDialog::SlotSetColWidths);
		disconnect(_pVF, &VariablesFunctionsDialog::SignalVarFuncMoved, this, &FalconCalcQt::_SlotVarFuncMoved);
		disconnect(_pVF, &VariablesFunctionsDialog::SignalVarFuncSaved, this, &FalconCalcQt::_SlotVarFuncSaved);
		delete _pVF;
		_pVF = nullptr;
	}
	_actTab = -1;
	ui.actionEditFunc->setChecked(false);
	ui.actionEditVars->setChecked(false);
}

void FalconCalcQt::_SlotVarFuncMoved()
{
	if (_busy)
		return;

	_varfuncDialogSnapped = false;
}

void FalconCalcQt::_EditVarsCommon(int which)
{
	if (_pVF)	
	{
		if (which != _actTab)	// either tab change
		{
			_actTab = which;
			emit _SignalSelectTab(which);
		}
		else					// or close
		{
			_SlotVarsFuncsClosing();
		}
	}
	else
	{
		VarFuncInfo vf;
		lengine->GetVarFuncInfo(vf);
		VarFuncInfoQt vfQt(vf);

		_pVF = new VariablesFunctionsDialog(vfQt, this);
		connect(_pVF, &VariablesFunctionsDialog::SignalVarFuncClose, this, &FalconCalcQt::_SlotVarsFuncsClosing);
		connect(_pVF, &VariablesFunctionsDialog::SignalTabChange, this, &FalconCalcQt::_SlotVarTabChanged);
		connect(this, &FalconCalcQt::_SignalSelectTab, _pVF, &VariablesFunctionsDialog::SlotSelectTab);
		connect(this, &FalconCalcQt::_SignalSetColWidths, _pVF, &VariablesFunctionsDialog::SlotSetColWidths);
		connect(_pVF, &VariablesFunctionsDialog::SignalVarFuncMoved, this, &FalconCalcQt::_SlotVarFuncMoved);
		connect(_pVF, &VariablesFunctionsDialog::SignalVarFuncSaved, this, &FalconCalcQt::_SlotVarFuncSaved);
		_actTab = which;
		ui.actionEditFunc->setChecked(which);
		ui.actionEditVars->setChecked(!which);
		++_busy;
		_PlaceWidget(*_pVF, Placement::pmRight);
		_varfuncDialogSnapped = true;

		_pVF->show();
		--_busy;
	}
	this->activateWindow();		// get the focus back
	this->raise();
	ui.edtInfix->setFocus();
}

void FalconCalcQt::on_actionEditVars_triggered()
{
	_EditVarsCommon(0);
}

void FalconCalcQt::on_actionEditFunc_triggered()
{
	_EditVarsCommon(1);
}

void FalconCalcQt::on_actionClearHistory_triggered()
{
	if (QMessageBox::question(this, tr("FalconCalc - Question"), tr("This will remove the whole of history.\nAre you sure?")) == QMessageBox::Yes)
	{
		_slHistory.clear();
		if (_pHist)
			_pHist->Clear();
	}
}
void FalconCalcQt::on_actionHex_triggered()
{
    on_btnOpenCloseHexOptions_clicked();
}
void FalconCalcQt::on_actionHistOptions_triggered()
{
	HistOptionData hoData;

	hoData.watchTimeout  = _watchTimeout;
	hoData.maxHistDepth  = _maxHistDepth;
	hoData.historySorted = _historySorted;
	hoData.minCharLength = _minCharLength;

	HistoryOptions *pHistOpt = new HistoryOptions(hoData, this);
	pHistOpt->setModal(true);
	if (pHistOpt->exec())	// then hoData may be changed
	{
		emit _StopTimer();

		_watchTimeout  = hoData.watchTimeout;
		_maxHistDepth  = hoData.maxHistDepth;
		_historySorted = hoData.historySorted;
		_minCharLength = hoData.minCharLength;

		if ((int)_maxHistDepth < _slHistory.size())
		{
			do
				_slHistory.pop_back();
			while ((int)_maxHistDepth < _slHistory.size());
		}
		if (_pHist)
			_pHist->SetList(_slHistory);
		_watchdogTimer.setInterval(hoData.watchTimeout*1000);
		emit _StartTimer();
	}
	delete pHistOpt;
}

void FalconCalcQt::on_actionDec_triggered()
{
    on_btnOpenCloseDecOptions_clicked();
}
void FalconCalcQt::on_actionSetLocale_triggered()
{

}
void FalconCalcQt::on_actionSystemMode_triggered()
{
	_actScheme = Scheme::schSystem;
	_schemeVector->PrepStyle(_actScheme);
	ui.actionSystemMode->setChecked(true);
}
void FalconCalcQt::on_actionLightMode_triggered()
{
	_actScheme = Scheme::schLight;
	_schemeVector->PrepStyle(_actScheme);
	ui.actionLightMode->setChecked(true);
	//__SaveStyle("light");
}
void FalconCalcQt::on_actionDarkMode_triggered()
{
	_actScheme = Scheme::schDark;
	_schemeVector->PrepStyle(_actScheme);
	ui.actionDarkMode->setChecked(true);
	//__SaveStyle("dark");
}
void FalconCalcQt::on_actionBlackMode_triggered()
{
	_actScheme = Scheme::schBlack;
	_schemeVector->PrepStyle(_actScheme);
	ui.actionBlackMode->setChecked(true);
	//__SaveStyle("black");
}
void FalconCalcQt::on_actionBlueMode_triggered()
{
	_actScheme = Scheme::schBlue;
	_schemeVector->PrepStyle(_actScheme);
	ui.actionBlueMode->setChecked(true);
	//__SaveStyle("blue");
}
void FalconCalcQt::on_actionSelectFont_triggered()
{
    on_btnStringFont_clicked();
}
void FalconCalcQt::on_actionHelp_triggered()
{
	if (!HelpDialog::helpVisible)
	{
		HelpDialog* ph = new HelpDialog(this);
		ph->setAttribute(Qt::WA_DeleteOnClose);
		ph->show();
	}
}
void FalconCalcQt::on_actionAbout_triggered()
{
	AboutDialog* pAbD = new AboutDialog(this);
	pAbD->setAttribute(Qt::WA_DeleteOnClose);
	pAbD->show();
}
void FalconCalcQt::on_btnBinary_clicked()
{
	_clipBoard->setText(ui.lblBin->text());
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_btnDecimal_clicked()	   // decimal display chaged
{
	QString text = ui.lblDec->text();
	int pos = text.indexOf(QChar(183));
	if (pos > 0)							   // sci or eng with display mode "normal"
	{
		text.replace(pos, 8, "E"); // replace "·10<sup>" with E
		pos = text.indexOf('<', pos);
		text.remove(pos, 6); // "</sup>"
	}
	else if ((pos = text.indexOf('x')) > 0)			// sci or eng with display mode Html
	{
		text.replace(pos, 14, "E"); // replace "·10<sup>" with E
		pos = text.indexOf('&', pos);
		text.remove(pos, 12); // "</sup>"
	}
	_clipBoard->setText(text);
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_btnHexaDecimal_clicked()
{
	_clipBoard->setText(ui.lblHex->text());
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_btnOctal_clicked()
{
	_clipBoard->setText(ui.lblOct->text());
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_btnOpenCloseDecOptions_clicked()
{
    QRect r = geometry();
    int h = height();
    QString text = ui.btnOpenCloseDecOptions->text();

    if (ui.frmDecimal->isVisible())	  // then hide it
    {
        text[0] = QChar(0x25ba);
        ui.btnOpenCloseDecOptions->setText(text);
        ui.frmDecimal->hide();      
        int h = r.height() - hDecOptionsHeight;
        r.setHeight(h);
        setMinimumSize(QSize(r.width(), h));
        setMaximumSize(QSize(r.width(), h));
		_decOpen = false;
    }
    else							// not visible:show it
    {
        text[0] = QChar(0x25bc);
        ui.btnOpenCloseDecOptions->setText(text);
        int h = r.height() + hDecOptionsHeight;
        r.setHeight(h);
        setMinimumSize(QSize(r.width(), h));
        setMaximumSize(QSize(r.width(), h));
        ui.frmDecimal->show();
		_decOpen = true;
    }
	if (_pHist && _histDialogSnapped)
		_PlaceWidget(*_pHist, Placement::pmBottom);
	if (_pVF && _varfuncDialogSnapped)
		_PlaceWidget(*_pVF, Placement::pmRight);
}

void FalconCalcQt::on_btnOpenCloseHexOptions_clicked()
{
    QRect r = geometry();
    QString text = ui.btnOpenCloseHexOptions->text();

    if (ui.gbHexOptions->isVisible())	// then close it
    {
        text[0] = QChar(0x25ba);
        int h = r.height() - hHexOptionsHeight;
        ui.btnOpenCloseHexOptions->setText(text);
        ui.gbHexOptions->hide();      
        r.setHeight(h);
        setMinimumSize(QSize(r.width(), h));
        setGeometry(r);
		_hexOpen = false;
    }
    else
    {
        text[0] = QChar(0x25bc);
        ui.btnOpenCloseHexOptions->setText(text);
        int h = r.height() + hHexOptionsHeight;
        r.setHeight(h);
        setMinimumSize(QSize(r.width(), h));
        setGeometry(r);
        ui.gbHexOptions->show();
		_hexOpen = true;
    }
	if (_pHist && _histDialogSnapped)
		_PlaceWidget(*_pHist, Placement::pmBottom);
	if (_pVF && _varfuncDialogSnapped)
		_PlaceWidget(*_pVF, Placement::pmRight);
}

void FalconCalcQt::on_btnStringFont_clicked()
{
	ui.lblChars->setFont(QFontDialog::getFont(0, ui.lblChars->font(), this));
}

void FalconCalcQt::on_cbThousandSep_currentIndexChanged(int newIndex)
{
	if (ui.chkSep->isChecked())
	{
		lengine->displayFormat.strThousandSeparator = SmartString(newIndex > 0 ? ui.cbThousandSep->currentText()[0].unicode() : ' ');
		if (lengine->displayFormat.strThousandSeparator == SmartString('s'))	// 'space'
			lengine->displayFormat.strThousandSeparator = " ";
		_ShowResults();
	}
}

void FalconCalcQt::on_chkDecDelim_toggled(bool b)
{
	lengine->displayFormat.useFractionSeparator = ui.chkDecDelim->isChecked();
	_ShowResults();
}
void FalconCalcQt::on_chkDecDigits_toggled(bool b)
{
	lengine->displayFormat.decDigits = b ? ui.spnDecDigits->value() : -1;
	_ShowResults();
}
void FalconCalcQt::on_chkEng_toggled(bool b)
{
	if (!b && ui.chkSci->isChecked())
		return;
	lengine->displayFormat.mainFormat = b ? NumberFormat::rnfEng : NumberFormat::rnfGeneral;
	_ShowResults();
}
void FalconCalcQt::on_chkSci_toggled(bool b)
{
	if (!b && ui.chkEng->isChecked())
		return;
	lengine->displayFormat.mainFormat = b ? NumberFormat::rnfSci : NumberFormat::rnfGeneral;
	_ShowResults();

}
void FalconCalcQt::on_chkSep_toggled(bool b)
{
	if (b)
		on_cbThousandSep_currentIndexChanged(ui.cbThousandSep->currentIndex());
	else
	{
		lengine->displayFormat.strThousandSeparator.clear();
		_ShowResults();
	}
}

union __HexFlags {
	struct {
		bool hasMinus : 1,
			hasPrefix : 1,
			asBytes : 1,
			asWords : 1,
			asDWords : 1,
			asIEEES : 1,
			asIEEED : 1,
			littleEndian : 1;
	};
	int val;
} __hexFlags;

void FalconCalcQt::_LoadHistory()
{
	QFile f(FCSettings::homePath + FalconCalcQt_HIST_FILE);
	if (f.open(QIODevice::ReadOnly))
	{
		QTextStream ifs(&f);
		ifs.setCodec("UTF-8");
		QString s;
		while (!ifs.atEnd())
		{
			try 
			{
				s = ifs.readLine();
			}
			catch (...)
			{
				;
			}
			_slHistory.push_back(s);
		}
	}
}

void FalconCalcQt::_SaveHistory()
{
	QFile f(FCSettings::homePath + FalconCalcQt_HIST_FILE);
	if (f.open(QIODevice::WriteOnly))
	{
		QTextStream ofs(&f);
		ofs.setCodec("UTF-8");
		for (auto& s : _slHistory)
			ofs << s << "\n";
	}
}

void FalconCalcQt::_PlaceWidget(QWidget& w, Placement pm)
{
	int xw,		// place window top left here
		yw;		// includes border and title bar for the dialog widget
	QScreen *actScreen = screen();

	Placement pm0 = pm;
	QRect geom = frameGeometry();
	do
	{
		switch (pm)
		{
			case Placement::pmTop:
				xw = geom.x(); yw = geom.y() - w.frameGeometry().height();
				break;
			case Placement::pmRight:
				xw = geom.x() + geom.width(); yw = geom.y();
				break;
			case Placement::pmBottom:
				xw = geom.x(); yw = geom.y() + geom.height();
				break;
			default:
			case Placement::pmLeft:
				xw = geom.x() - w.frameGeometry().width(); yw = geom.y();
				break;
		}
		// if(xw +w.width())
	} while (0);	// Should see if it fits screen not yet
	w.move(xw, yw);
}

bool FalconCalcQt::_SaveUserData(QString fileName, VarFuncInfoQt& vf)		// from 
{
	if (fileName.isEmpty())
		fileName = lengine->ssNameOfDatFile.toQString();

	QFile of(fileName);
	if (!of.open(QIODevice::WriteOnly))
	{
		QMessageBox::critical(this, tr("FalconCalcQt - error"), tr("Can't save user data\n%1\n").arg(fileName));
		return false;
	}
	QTextStream ofs(&of);

	ofs << VERSION_STRING << "\n[Locale]\nloc=" << std::cout.getloc().name().c_str()
		<< L"\n\n[Variables]\n";
	std::wstring sDelim = ssCommentDelimiterString.ToWideString();


}

void FalconCalcQt::_SetHexDisplFomatForFlags()
{
	// defaults:
	lengine->displayFormat.hexFormat = HexFormat::rnHexNormal;
	lengine->displayFormat.trippleE = IEEEFormat::rntHexNotIEEE;
	lengine->displayFormat.littleEndian = false;
	lengine->displayFormat.bSignedBinOrHex = false;
	lengine->displayFormat.useNumberPrefix = false;

	if (__hexFlags.val)
	{
		if(__hexFlags.asBytes)
			lengine->displayFormat.hexFormat = HexFormat::rnHexByte;
		else if(__hexFlags.asWords)
			lengine->displayFormat.hexFormat = HexFormat::rnHexWord;
		if(__hexFlags.asDWords)
			lengine->displayFormat.hexFormat = HexFormat::rnHexDWord;
		if (__hexFlags.littleEndian)
			lengine->displayFormat.littleEndian = true;
		lengine->displayFormat.trippleE = __hexFlags.asIEEES ? IEEEFormat::rntHexIEEE754Single : (__hexFlags.asIEEED ? IEEEFormat::rntHexIEEE754Double:IEEEFormat::rntHexNotIEEE);
		lengine->displayFormat.bSignedBinOrHex = __hexFlags.hasMinus;
		lengine->displayFormat.useNumberPrefix = __hexFlags.hasPrefix;
	}
	++_busy;
	ui.chkBytes->setChecked(__hexFlags.asBytes);
	ui.chkWords->setChecked(__hexFlags.asWords);
	ui.chkDWords->setChecked(__hexFlags.asDWords);
	ui.chkIEEESingle->setChecked(__hexFlags.asIEEES);
	ui.chkIEEEDouble->setChecked(__hexFlags.asIEEED);
	--_busy;
	_ShowResults();
}

void FalconCalcQt::on_chkHexPrefix_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.hasPrefix= b;
	_SetHexDisplFomatForFlags();
}

void FalconCalcQt::on_chkMinus_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.hasMinus = b;
	_SetHexDisplFomatForFlags();
}
void FalconCalcQt::on_chkBytes_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.asBytes = b;
	if(b)
		__hexFlags.asWords = __hexFlags.asDWords = false;
	_SetHexDisplFomatForFlags();
}
void FalconCalcQt::on_chkWords_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.asWords = b;
	if (b)
		__hexFlags.asBytes = __hexFlags.asDWords = false;
	_SetHexDisplFomatForFlags();
}
void FalconCalcQt::on_chkDWords_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.asDWords = b;
	if (b)
		__hexFlags.asBytes = __hexFlags.asWords = false;
	_SetHexDisplFomatForFlags();
}
void FalconCalcQt::on_chkIEEESingle_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.asIEEES = b;
	if(b)
		__hexFlags.asIEEED = false;

	_SetHexDisplFomatForFlags();
}
void FalconCalcQt::on_chkIEEEDouble_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.asIEEED = b;
	if(b)
		__hexFlags.asIEEES = false;
	_SetHexDisplFomatForFlags();

}
void FalconCalcQt::on_chkLittleEndian_toggled(bool b)
{
	if (_busy)
		return;
	__hexFlags.littleEndian = b;
	_SetHexDisplFomatForFlags();
}

void FalconCalcQt::on_edtInfix_textChanged(const QString& newText)
{
	if (_busy)
		return;
	++_busy;
	if (!newText.isEmpty())
	{
		emit _StopTimer();		
		try
		{
			lengine->infix.FromQString(newText);
			RealNumber res = lengine->Calculate();
			ui.lblResults->setText("Results");
			_ShowResults();
		}
		catch (std::wstring ws)
		{
			ui.lblResults->setText(SmartString(ws).toQString());
			_ShowMessageOnAllPanels("???");
		}
		catch (Trigger_Type tt)
		{
			ui.lblResults->setText(triggerMap[tt].toQString());
			_ShowMessageOnAllPanels("???");
		}
		catch (...)
		{
			_ShowMessageOnAllPanels("???");
		}

		_added = false;
		emit _StartTimer();		// restart timer
	}
//	_ShowMessageOnAllPanels("");
	--_busy;
}

void FalconCalcQt::on_rbDeg_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.angUnit = LongNumber::AngularUnit::auDeg;
	_ShowResults();
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_rbRad_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.angUnit = LongNumber::AngularUnit::auRad;
	_ShowResults();
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_rbGrad_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.angUnit = LongNumber::AngularUnit::auGrad;
	_ShowResults();
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_rbTurns_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.angUnit = LongNumber::AngularUnit::auTurn;
	_ShowResults();
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_rbNormal_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.expFormat = ExpFormat::rnsfGraph;
	_ShowResults();
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_rbHtml_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.expFormat = ExpFormat::rnsfSciHTML;
	_ShowResults();
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_rbTex_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.expFormat = ExpFormat::rnsfSciTeX;
	_ShowResults();
	ui.edtInfix->setFocus();
}
void FalconCalcQt::on_rbNone_toggled(bool b)
{
	if (!b)
		return;
	lengine->displayFormat.expFormat = ExpFormat::rnsfE;
	_ShowResults();
	ui.edtInfix->setFocus();
}

void FalconCalcQt::on_spnDecDigits_valueChanged(int val)
{
	lengine->displayFormat.decDigits = ui.chkDecDigits->isChecked() ? val : -1;
	_ShowResults();
}


static const QString
		MAINFORMAT("mainFormat"),
		DECFORMAT("decFormat"),
		HEXFORMAT("hexFormat"),
		FONTNAME("fontName"),
		FONTDATA("fontData"),
		OPTIONS("options"),
		HISTOPTIONS("histOptions"),
		VARCOLS("varCols"),
		LAST("last");

/*=============================================================
 * TASK   : reads line from file splits it up first at a '=' sign, 
 *			then the remaining at '|' delimiters
 * PARAMS : ifs: open text stream
 *			data: split up line, field #0 is the key
 * EXPECTS:
 * GLOBALS:
 * RETURNS: >0  => (# of strings in line + 1) and data filled
 *			<=0  => error, equal line not found
 * REMARKS: keeps empty fields as empty wstrings
 *------------------------------------------------------------*/
static int __ReadAndSplitLine(QTextStream& ifs, QStringList& data)
{
	if (ifs.atEnd())	// no more output?
		return 0;

	static QString qs;
	do
	{
		qs = ifs.readLine();
		qs = qs.trimmed();
	} while (!ifs.atEnd() && (qs.isEmpty() || qs[0] == '#') );

	int n = qs.indexOf('='); // format: key=val1|val2|val3...
	if (n < 0)
		return -1;

	data.clear();
	data.push_back(qs.left(n));
	data += qs.mid(n+1).split('|');
	return data.size();
}

void FalconCalcQt::_EnableMyTimer(bool enable)
{	  
	if (enable)
		emit _StartTimer();
	else
		emit _StopTimer();
}

bool FalconCalcQt::_LoadState(QString name)
{
    QFile fcfg(name);
	if (!fcfg.open(QIODevice::ReadOnly))
		return false;
	
    QTextStream ifcfg(&fcfg);
    ifcfg.setCodec("UTF-8");

    QString s;
    s = ifcfg.readLine();
    if (s != STATE_VER_STRING)
        return false;

	QStringList data;
	int n, val = 0;

	auto mainFormat = [&]()	-> bool // returns true if not processed, false if processed
		{
			if (n == 2 && data[0] == MAINFORMAT)	// only one field
			{
				val = data[1].toInt();
				lengine->displayFormat.mainFormat = static_cast<NumberFormat>(val);
				++_busy;
				switch (lengine->displayFormat.mainFormat)
				{
					case NumberFormat::rnfSci:
						ui.chkSci->setChecked(true);
						break;
					case NumberFormat::rnfEng:
						ui.chkEng->setChecked(true);
						break;
				}
				--_busy;
				return true;
			}
			return false;
		};
	auto decFormat = [&]()
		{
			if (n == 6 && data[0] == DECFORMAT)		// 6 fields
			{
				++_busy;
				// 1: decimal digits
				val = data[1].toInt();	// # of decimal digits n > 0 => used digits, n < 0 => used = abs(n+1)
				lengine->displayFormat.decDigits = val;
				if (val >= 0)
					ui.chkDecDigits->setChecked(true);
				else
					val = std::abs(val + 1);
				ui.spnDecDigits->setValue(val);
				// 2: exponent display format
				val = data[2].toInt();	// (0)E: 1E5, (1)HTML: 1<sp>12</sup>, (2)TeX: 1^{12}, (3)normal: 1²³
				lengine->displayFormat.expFormat = static_cast<ExpFormat>(val);
				if (lengine->displayFormat.expFormat == ExpFormat::rnsfE)
					ui.rbNone->setChecked(true);
				else if (lengine->displayFormat.expFormat == ExpFormat::rnsfSciHTML)
					ui.rbHtml->setChecked(true);
				else if (lengine->displayFormat.expFormat == ExpFormat::rnsfSciTeX)
					ui.rbTex->setChecked(true);
				else if (lengine->displayFormat.expFormat == ExpFormat::rnsfGraph)
				{
					ui.rbNormal->setChecked(true);
				}
				// 3: thousand separator string
				if (!data[3].isEmpty())	// can only be '.', ',' and space
				{
					if (data[3][0] == L'1')
						ui.chkSep->setChecked(true);
					else if (data[3][1] == L'0')
						ui.cbThousandSep->setCurrentIndex(0);
					else if (data[3][1] == L'1')
						ui.cbThousandSep->setCurrentIndex(1);
					else if (data[3][1] == L'2')
						ui.cbThousandSep->setCurrentIndex(2);
					if (ui.chkSep->isChecked())
						lengine->displayFormat.strThousandSeparator = SmartString(ui.cbThousandSep->currentIndex() > 0 ? ui.cbThousandSep->currentText()[0].unicode() : ' ');
				}
				// 4: fraction separator
				if (!data[4].isEmpty() && data[4] == L"1")
					ui.chkDecDelim->setChecked(true);
				// 5: angular unit 0:
				if (!data[5].isEmpty())
					lengine->displayFormat.angUnit = static_cast<AngularUnit>(data[5].toInt());

				--_busy;

				return true;
			}
			return false;

		};
	auto hexFormat = [&]()
		{
			if (n == 6 && data[0] == HEXFORMAT)	// 5 fields
			{
				++_busy;
				// 1. main Hex format
				int val = data[1].toInt();
				lengine->displayFormat.hexFormat = static_cast<HexFormat>(val);
				switch (lengine->displayFormat.hexFormat)
				{
					case HexFormat::rnHexNormal:
						break;
					case HexFormat::rnHexByte:
						ui.chkBytes->setChecked(true);
						break;
					case HexFormat::rnHexWord:
						ui.chkWords->setChecked(true);
						break;
					case HexFormat::rnHexDWord:
						ui.chkDWords->setChecked(true);
						break;
				}
				// 2. endianness
				val = data[2].toInt();
				if (val)
					ui.chkLittleEndian->setChecked(true);
				// 3. signed bin or hex?
				val = data[3].toInt();
				if (val)
					ui.chkMinus->setChecked(true);
				// 4. IEEE format
				val = data[4].toInt();
				// val = 0: no check
				if (val == 1)
					ui.chkIEEESingle->setChecked(true);
				else if (val == 2)
					ui.chkIEEEDouble->setChecked(true);
				--_busy;
				// 5. number prefix is used on hex. numbers
				val = data[5].toInt();
				lengine->displayFormat.useNumberPrefix = val;
				ui.chkHexPrefix->setChecked(val);
				return true;
			}
			return false;

		};
	auto fontName = [&]()
		{
			if (n == 2 && data[0] == FONTNAME)  // 2 fields
			{
				QFont f = ui.lblChars->font();
				f.setFamily(data[1]);
				ui.lblChars->setFont(f);
				return true;
			}
			return false;

		};
	auto fontData = [&]()
		{
			if (n == 3 && data[0] == FONTDATA)	// 3 fields
			{
				QFont f = ui.lblChars->font();
				f.setPointSizeF(data[1].toFloat());
				f.setFamily(data[2]);
				//QPen pen = ui.lblChars->color();//  
				//pen.setColor(QColor(data[3]));
				ui.lblChars->setFont(f);
				return true;
			}
			return false;

		};
	auto options = [&]()					  // 3 fields
		{
			if ( (n == 3 || n == 4) && data[0] == OPTIONS)
			{
				if(data[1].toInt() == 0)
					_decOpen = false;		 // default: true - it was open on creation
				if(data[2].toInt() == 0)
					_hexOpen = false;
				if (n == 4)
					_actScheme = (Scheme)data[3].toInt();
				return true;
			}
			return false;

		};
	auto histOptions = [&]()
		{
			if (n == 5 && data[0] == HISTOPTIONS)	// 5 fields
			{
				_watchTimeout = data[1].toInt();
				_maxHistDepth = data[2].toInt();
				_historySorted = data[3].toInt() != 0;
				_minCharLength = data[4].toInt();

				if (_watchTimeout > 0)
				{
					_watchdogTimer.setInterval(_watchTimeout * 1000);
					emit _StartTimer();
				}
				return true;
			}
			return false;

		};
	auto varCols = [&]()
		{
			if (n == 9 && data[0] == VARCOLS)
			{
				for (int col = 0; col < 4; ++col)
				{
					_varColWidths[0][col] = data[col + 1].toInt();
					_varColWidths[1][col] = data[col + 4 + 1].toInt();
				}
				return true;
			}
			return false;
		};
	auto last = [&](QString& lastinfix)
		{
			if (n == 2 && data[0] == LAST)
			{
				lastinfix = data[1];
			}
		};

	QString wsLlastInfix;

	while ((n = __ReadAndSplitLine(ifcfg, data)) > 0)
	{
		if (!mainFormat())
			if (!decFormat())
				if (!hexFormat())
					if (!fontName())
						if (!fontData())
							if (!options())
								if (!histOptions())
									if (!varCols())
										last(wsLlastInfix);
	}

	switch (_actScheme)
	{
		case Scheme::schSystem:
			on_actionSystemMode_triggered();
			break;
		case Scheme::schLight:
			on_actionLightMode_triggered();
			break;
		case Scheme::schDark:
			on_actionDarkMode_triggered();
			break;
		case Scheme::schBlack:
			on_actionBlackMode_triggered();
			break;
		case Scheme::schBlue:
			on_actionBlueMode_triggered();
			break;
	}

	_EnableMyTimer(_watchTimeout > 0);
	--_busy;
	return true;
}

bool FalconCalcQt::_SaveState(QString name)
{
	QFile f(name);
	if (!f.open(QIODevice::WriteOnly))
		return false;

	QTextStream ofs(&f);
	ofs.setCodec("UTF-8");

	ofs << STATE_VER_STRING << "\n";
	ofs << MAINFORMAT << "=" << (int)lengine->displayFormat.mainFormat << "\n";

	//int u = UpDown1->Position() + (chkDecDigits->Checked() ? 0x100 : 0); // 0x100: checked state. must use Position as num_digits may be -1
	QString wsep = (ui.chkSep->isChecked() ? "1" : "0") + QString().setNum(ui.cbThousandSep->currentIndex());
	ofs << DECFORMAT << "=" << lengine->displayFormat.decDigits << "|" << (int)lengine->displayFormat.expFormat
		<< "|" << wsep << "|" << (int)lengine->displayFormat.useFractionSeparator
		<< "|" << (int)lengine->AngleUnit() << "\n";

	ofs << HEXFORMAT << "=" << (int)lengine->displayFormat.hexFormat << "|" << (int)lengine->displayFormat.littleEndian << "|" <<
		(int)lengine->displayFormat.bSignedBinOrHex << "|" << (int)lengine->displayFormat.trippleE << "|" << (int)lengine->displayFormat.useNumberPrefix << "\n";

	ofs << FONTNAME << "=" << ui.lblChars->font().family() << "\n";
	ofs << FONTDATA << "=" << (int)ui.lblChars->font().pointSize() << "|" << (int)ui.lblChars->font().style() << "|" << _lblTextColor.name() << "\n";
	ofs << OPTIONS << "=" << _decOpen << "|" << _hexOpen << "|" << (int)_actScheme << "\n";
	ofs << HISTOPTIONS << "=" << _watchTimeout << "|" << _maxHistDepth << "|" << _historySorted << "|" << _minCharLength << "\n";
	ofs << VARCOLS << "=";
		for (int col = 0; col < 4; ++col)
		{
			ofs << _varColWidths[0][col];
			ofs << "|";
		}
		for (int col = 0; col < 4; ++col)
		{
			ofs << _varColWidths[1][col];
			if (col != 3)
				ofs << "|";
		}
	if (!ui.edtInfix->text().isEmpty())
		ofs << LAST << ui.edtInfix->text() << "\n";
	return true;
}

void FalconCalcQt::_AddToHistory(QString infix)
{
	infix = infix.trimmed();
	if (infix.isEmpty() || _minCharLength >= infix.length() || lengine->resultType == FalconCalc::LittleEngine::ResultType::rtInvalid)	// do not add too short strings
		return;

	SmartString ssinfix(infix);

	if (LittleEngine::variables.count(ssinfix) || LongNumber::constantsMap.count(ssinfix))		// single, already defined variable?
	{
		_added = true;	// so won't try it to add again
		return;
	}
	// no it's an expression, may be a definition, we don't care
	int n;
	if ((n = _slHistory.indexOf(infix)) >= 0)
	{
		if (n == 0)							// already at top
			return;							// nothing to do
		_slHistory.removeAt(n);				// not at top delete expression from inside
	}
	_slHistory.push_front(infix);			 // new line to the top
	while ((int)_maxHistDepth >= _slHistory.size())
		_slHistory.pop_back();
		
	if (_historySorted)
		_slHistory.sort();

	_added = true;
	if (_pHist)
	{
		_pHist->Clear();
		_pHist->SetList(_slHistory);
	}
}

void FalconCalcQt::_ShowResults()
{
	if (ui.edtInfix->text().isEmpty() || lengine->resultType == LittleEngine::ResultType::rtInvalid)
		_ShowMessageOnAllPanels("???");
	else if(lengine->resultType == LittleEngine::ResultType::rtDefinition)
		_ShowMessageOnAllPanels("Definition");
	else
	{
		ExpFormat ef = lengine->displayFormat.expFormat;
		if (ef == ExpFormat::rnsfGraph) // in QT the @normal mode = HTML mode for non QT version
			lengine->displayFormat.expFormat = ExpFormat::rnsfSciHTML;
		QString qs = lengine->ResultAsDecString().toQString();	
		if (ef == ExpFormat::rnsfGraph) // in QT the @normal mode = HTML mode for non QT version
			qs.replace('x',183 );

		if (ef == ExpFormat::rnsfSciHTML)
		{
			qs.replace("<", "&lt;");
			qs.replace(">", "&gt;");
		}
		lengine->displayFormat.expFormat = ef;

		ui.lblDec->setText(qs);
		ui.lblHex->setText(lengine->ResultAsHexString().toQString());
		ui.lblOct->setText(lengine->ResultAsOctString().toQString());
		ui.lblBin->setText(lengine->ResultAsBinString().toQString());
		ui.lblChars->setText(lengine->ResultAsCharString().toQString());
	}
	ui.edtInfix->setFocus();
}

void FalconCalcQt::_ShowMessageOnAllPanels(QString s)
{
	ui.lblDec->setText(s);
	ui.lblHex->setText(s);
	ui.lblOct->setText(s);
	ui.lblBin->setText(s);
}

void FalconCalcQt::_SlotDataFromHistory(QString qs)
{
	if (qs.isEmpty())
		return;
	ui.edtInfix->setText(qs);
	ui.edtInfix->setFocus();
}

void FalconCalcQt::_watchdogTimerSlot()
{
	if (_added)
		return;
	_AddToHistory(ui.edtInfix->text());
}


void FalconCalcQt::_SlotRemoveHistLine(int row)
{
	if (row >= 0)
	{
		_slHistory.removeAt(row);
		_pHist->SetList(_slHistory);
	}
}
void FalconCalcQt::_SlotClearHistory()
{
	on_actionClearHistory_triggered();
}

void FalconCalcQt::_SlotHistOptions()
{
	on_actionHistOptions_triggered();
}

void FalconCalcQt::_SlotHistMoved()
{
	if (_busy)
		return;

	_histDialogSnapped = false;
}

void FalconCalcQt::_SlotVarTabChanged(int newTab)
{
	_actTab = newTab;
}

void FalconCalcQt::_SlotVarFuncSaved(VarFuncInfoQt& vfQt)
{
	VarFuncInfo vf;
	vf.pOwner			= vfQt.pOwner;
	vf.uBuiltinVarCnt	= vfQt.uBuiltinVarCnt;
	vf.uBuiltinFuncCnt	= vfQt.uBuiltinFuncCnt;
	vf.uUserVarCnt		= vfQt.uUserVarCnt;
	vf.uUserFuncCnt		= vfQt.uUserFuncCnt;
	vf.sBuiltinVars		= vfQt.sBuiltinVars;
	vf.sBuiltinFuncs	= vfQt.sBuiltinFuncs;
	vf.sUserVars		= vfQt.sUserVars;
	vf.sUserFuncs		= vfQt.sUserFuncs;

	lengine->LoadUserData();
}

