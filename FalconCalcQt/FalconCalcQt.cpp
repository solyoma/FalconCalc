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

#include <vector>

#include "SmartString.h"
using namespace SmString;

#include "LongNumber.h"
using namespace LongNumber;

#include "calculate.h"
using namespace FalconCalc;

#include "VariablesFunctionsDialog.h"
#include "HistoryDialog.h"
#include "HistoryOptions.h"
#include "HelpDialog.h"


FalconCalc::LittleEngine* lengine = nullptr;

QString STATE_VER_STRING = QStringLiteral("FalconCalc State File V1.0");


//----------------------------- FCSettings -------------------
/*=============================================================
 * TASK:    centralized settings handler
 *      static members are initialized in FalconBoard.cpp!
 *------------------------------------------------------------*/
QSettings* FCSettings::_ps = nullptr;
QString FCSettings::homePath;
QString FCSettings::_name = "FalconCalcQt.dat";   // default ini file

void FCSettings::Init()             // set home path for falconBoard
{
    homePath = QDir::homePath() +
#if defined (Q_OS_Linux)   || defined (Q_OS_Darwin) || defined(__linux__)
        "/.falconCalc/";
#elif defined(Q_OS_WIN)
        "/Appdata/Local/FalconCalc/";
#endif
}

QString FCSettings::Name()
{
    return homePath + _name;
}

QSettings* FCSettings::Open()
{
    _ps = new QSettings(Name(), QSettings::IniFormat);
    _ps->setIniCodec("UTF-8");
    return _ps;
}

//------------------------ FalconCalcQt --------------

const QString FalconCalcQt_HIST_FILE= "FalconCalc.hist";
const QString FalconCalcQt_DAT_FILE = "FalconCalc.dat";
const QString FalconCalcQt_CFG_FILE = "FalconCalc.cfg";

FalconCalcQt::FalconCalcQt(QWidget *parent)  : QMainWindow(parent)
{
    ui.setupUi(this);

	FCSettings::Init();

	lengine = new LittleEngine;
	lengine->displayFormat.useNumberPrefix = ui.chkHexPrefix->isChecked();
	lengine->displayFormat.strThousandSeparator = " "_ss;
	lengine->displayFormat.displWidth = 59;

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
	_StartTimer();
}

FalconCalcQt::~FalconCalcQt()     
{
	lengine->SaveUserData();
	_SaveState(FCSettings::homePath + FalconCalcQt_CFG_FILE);
	_SaveHistory();
	delete lengine;
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
			on_btnOpenCloseDecOptions_clicked();
    }
	if (isVisible())
	{
		_bAlreadyShown = true;
		ui.edtInfix->setFocus();
	}
}

void FalconCalcQt::resizeEvent(QResizeEvent* event)
{
    if (!isVisible())
        return;

    static int cnt = 0;

    if (ui.frmDecimal->isVisible())
        hDecOptionsHeight = ui.frmDecimal->height();
    if (ui.gbHexOptions->isVisible())
        hHexOptionsHeight = ui.gbHexOptions->height();

    // DEBUG
    // QRect geom = geometry();
	// ui.lblResults->setText(QString("#%5 (%1,%2 %3 x %4) - dec H:%6, hex:%7").arg(geom.left()).arg(geom.top()).arg(geom.width()).arg(geom.height()).arg(++cnt).arg(hDecOptionsHeight).arg(hHexOptionsHeight));
	// /DEBUG
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
	if (_pHist && _histDialogSnapped)
	{
		int x0 = e->oldPos().x(), y0 = e->oldPos().y(), 
			x = e->pos().x(), y = e->pos().y(), 
			l = geometry().left(), t=geometry().top(),
			xh = _pHist->geometry().left(), yh = _pHist->geometry().top();
		qDebug("hist: (%d, %d)->(%d, %d), e-new (%d,%d), e-old (%d, %d), lt:(%d, %d)", xh, yh, xh + x - x0, yh + y - y0, x, y, x0, y0,l,t);
		_pHist->move(xh + x - x0, yh + y - y0 - QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight)- QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth));
		//_pHist->move(_pHist->geometry().left() + e->pos().x() - e->oldPos().x(), _pHist->geometry().top() + e->pos().y() - e->oldPos().y());
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
	disconnect(_pHist, &HistoryDialog::SignalSelection	, this, &FalconCalcQt::_SlotDataFromHistory);
	disconnect(_pHist, &HistoryDialog::SignalRemoveHistLine, this, &FalconCalcQt::_SlotRemoveHistLine );
	disconnect(_pHist, &HistoryDialog::SignalClearHistory	, this, &FalconCalcQt::_SlotClearHistory);
	disconnect(_pHist, &HistoryDialog::SignalClose		, this, &FalconCalcQt::_SlotHistClosing);
	disconnect(_pHist, &HistoryDialog::SignalHistOptions, this, &FalconCalcQt::_SlotHistOptions);
	delete _pHist;
	_pHist = nullptr;		// will be deleted by 
}

void FalconCalcQt::on_actionEditHist_triggered()
{
	if (_pHist)		// then hide
	{
		_SlotHistClosing();
	}
	else			// show
	{
		_pHist = new HistoryDialog(_slHistory, this);
		ui.actionEditHist->setChecked(true);
		connect(_pHist, &HistoryDialog::SignalSelection		, this, &FalconCalcQt::_SlotDataFromHistory);
		connect(_pHist, &HistoryDialog::SignalRemoveHistLine, this, &FalconCalcQt::_SlotRemoveHistLine );
		connect(_pHist, &HistoryDialog::SignalClearHistory	, this, &FalconCalcQt::_SlotClearHistory);
		connect(_pHist, &HistoryDialog::SignalClose			, this, &FalconCalcQt::_SlotHistClosing);
		connect(_pHist, &HistoryDialog::SignalHistOptions	, this, &FalconCalcQt::_SlotHistOptions);
		_PlaceWidget(*_pHist, Placement::pmBottom);
		_pHist->show();
	}
	this->activateWindow();		// get the focus back
	this->raise();
	ui.edtInfix->setFocus();
}

void FalconCalcQt::_SlotVarsFuncsClosing()
{
	disconnect(_pVF, &VariablesFunctionsDialog::SignalClose, this, &FalconCalcQt::_SlotVarsFuncsClosing);
	disconnect(_pVF, &VariablesFunctionsDialog::SignalTabChange, this, &FalconCalcQt::_SlotVarTabChanged);
	disconnect(this, &FalconCalcQt::_SignalSelectTab, _pVF, &VariablesFunctionsDialog::SlotSelectTab);
	disconnect(this, &FalconCalcQt::_SignalSetColWidths, _pVF, &VariablesFunctionsDialog::SlotSetColWidths);
	delete _pVF;
	_pVF = nullptr;
	_actTab = -1;
	ui.actionEditFunc->setChecked(false);
	ui.actionEditVars->setChecked(false);
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
		_pVF = new VariablesFunctionsDialog(which, this);
		connect(_pVF, &VariablesFunctionsDialog::SignalClose, this, &FalconCalcQt::_SlotVarsFuncsClosing);
		connect(_pVF, &VariablesFunctionsDialog::SignalTabChange, this, &FalconCalcQt::_SlotVarTabChanged);
		connect(this, &FalconCalcQt::_SignalSelectTab, _pVF, &VariablesFunctionsDialog::SlotSelectTab);
		connect(this, &FalconCalcQt::_SignalSetColWidths, _pVF, &VariablesFunctionsDialog::SlotSetColWidths);
		_actTab = which;
		ui.actionEditFunc->setChecked(which);
		ui.actionEditVars->setChecked(!which);
		_PlaceWidget(*_pVF, Placement::pmRight);
		_pVF->show();
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
	if (QMessageBox::question(this, tr("FalconCalc Question"), tr("This will remove the whole history.\nAre you sure?")) == QMessageBox::Yes)
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
	if (_pHistOpt)
	{
		delete _pHistOpt;
		_pHistOpt = nullptr;
	}
	else
	{
	}
}
void FalconCalcQt::on_actionDec_triggered()
{
    on_btnOpenCloseDecOptions_clicked();
}
void FalconCalcQt::on_actionSetLocale_triggered()
{

}
void FalconCalcQt::on_actionSelectFont_triggered()
{
    on_btnStringFont_clicked();
}
void FalconCalcQt::on_actionHelp_triggered()
{

}
void FalconCalcQt::on_actionAbout_triggered()
{

}
void FalconCalcQt::on_btnBinary_clicked()
{

}
void FalconCalcQt::on_btnDecimal_clicked()
{

}
void FalconCalcQt::on_btnHexaDecimal_clicked()
{

}
void FalconCalcQt::on_btnOctal_clicked()
{

}
void FalconCalcQt::on_btnOpenCloseDecOptions_clicked()
{
    QRect r = geometry();
    int h = height();
    QString text = ui.btnOpenCloseDecOptions->text();

    if (ui.frmDecimal->isVisible())
    {
        text[0] = QChar(0x25ba);
        ui.btnOpenCloseDecOptions->setText(text);
        ui.frmDecimal->hide();      
        int h = r.height() - hDecOptionsHeight;
        r.setHeight(h);
        setMinimumSize(QSize(r.width(), h));
        setMaximumSize(QSize(r.width(), h));
//        setGeometry(r);
    }
    else
    {
        text[0] = QChar(0x25bc);
        ui.btnOpenCloseDecOptions->setText(text);
        int h = r.height() + hDecOptionsHeight;
        r.setHeight(h);
        setMinimumSize(QSize(r.width(), h));
        setMaximumSize(QSize(r.width(), h));
//        setGeometry(r);
        ui.frmDecimal->show();
    }
}

void FalconCalcQt::on_btnOpenCloseHexOptions_clicked()
{
    QRect r = geometry();
    QString text = ui.btnOpenCloseHexOptions->text();

    if (ui.gbHexOptions->isVisible())
    {
        text[0] = QChar(0x25ba);
        ui.btnOpenCloseHexOptions->setText(text);
        ui.gbHexOptions->hide();      
        int h = r.height() - hHexOptionsHeight;
        r.setHeight(h);
        setMinimumSize(QSize(r.width(), h));
        setGeometry(r);
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
    }
}

void FalconCalcQt::on_btnStringFont_clicked()
{

}

void FalconCalcQt::on_cbThousandSep_currentIndexChanged(int newIndex)
{

}

void FalconCalcQt::on_chkDecDelim_toggled(bool b)
{

}
void FalconCalcQt::on_chkDecDigits_toggled(bool b)
{

}
void FalconCalcQt::on_chkEng_toggled(bool b)
{

}
void FalconCalcQt::on_chkSep_toggled(bool b)
{

}
void FalconCalcQt::on_chkSci_toggled(bool b)
{

}
void FalconCalcQt::on_chkHexPrefix_toggled(bool b)
{

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

void FalconCalcQt::_SetDisplayFlags()
{
	if (!__hexFlags.val)
	{
		lengine->displayFormat.hexFormat = HexFormat::rnHexNormal;
		lengine->displayFormat.trippleE = IEEEFormat::rntHexNotIEEE;
		lengine->displayFormat.littleEndian = false;
		lengine->displayFormat.bSignedBinOrHex = false;
		lengine->displayFormat.useNumberPrefix = false;
	}
	else
	{
		if(__hexFlags.asBytes)
			lengine->displayFormat.hexFormat = HexFormat::rnHexByte;
		if(__hexFlags.asWords)
			lengine->displayFormat.hexFormat = HexFormat::rnHexWord;
		if(__hexFlags.asDWords)
			lengine->displayFormat.hexFormat = HexFormat::rnHexDWord;
		lengine->displayFormat.trippleE = __hexFlags.asIEEES ? IEEEFormat::rntHexIEEE754Single : IEEEFormat::rntHexIEEE754Double;
		lengine->displayFormat.bSignedBinOrHex = __hexFlags.hasMinus;
		lengine->displayFormat.useNumberPrefix = __hexFlags.hasPrefix;
	}
}

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
	int xw,
		yw;
	QScreen *actScreen = screen();

	Placement pm0 = pm;
	do
	{
		switch (pm)
		{
			case Placement::pmTop:
				xw = x(); yw = y() - w.height();
				break;
			case Placement::pmRight:
				xw = x() + width(); yw = y();
				break;
			case Placement::pmBottom:
				xw = x(); yw = y() + height() + QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
				break;
			case Placement::pmLeft:
				xw = x() - w.width(); yw = y();
				break;
		}
		// if(xw +w.width())
	} while (0);	// Should see if it fits screen not yet
	w.move(xw, yw);
}

void FalconCalcQt::on_chkMinus_toggled(bool b)
{
	__hexFlags.hasMinus = b;
	_SetDisplayFlags();
}
void FalconCalcQt::on_chkBytes_toggled(bool b)
{
	__hexFlags.asBytes = b;
	_SetDisplayFlags();
}
void FalconCalcQt::on_chkWords_toggled(bool b)
{
	__hexFlags.asWords = b;
	_SetDisplayFlags();
}
void FalconCalcQt::on_chkDWords_toggled(bool b)
{
	__hexFlags.asDWords = b;
	_SetDisplayFlags();
}
void FalconCalcQt::on_chkIEEESingle_toggled(bool b)
{
	__hexFlags.asIEEES = b;
	_SetDisplayFlags();
}
void FalconCalcQt::on_chkIEEEDouble_toggled(bool b)
{
	__hexFlags.asIEEED = b;
	_SetDisplayFlags();

}
void FalconCalcQt::on_chkLittleEndian_toggled(bool b)
{
	__hexFlags.littleEndian = b;
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
			if (n == 3 && data[0] == OPTIONS)
			{
				if(data[1].toInt() == 0)
					_decOpen = false;		 // default: true - it was open on creation
				if(data[2].toInt() == 0)
					_hexOpen = false;
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
	ofs << OPTIONS << "=" << ui.frmDecimal->isVisible() << "|" << ui.gbHexOptions->isVisible() << "\n";
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
	SmartString ssinfix(infix);

	if (_minCharLength >= infix.length())	// do not add too short strings
		return;

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
		_slHistory.insert(0, infix);		
	}
	_slHistory.push_front(infix);			 // new line to the top
		
	if (_historySorted)
		_slHistory.sort();

	_added = true;
	if (_pHist)
	{
		_pHist->Clear();
		_pHist->pListWidget()->addItems(_slHistory);
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
		ui.lblDec->setText(lengine->ResultAsDecString().toQString());
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

}
void FalconCalcQt::_SlotClearHistory()
{

}
void FalconCalcQt::_SlotHistOptions()
{
	on_actionHistOptions_triggered();
}

void FalconCalcQt::_SlotVarTabChanged(int newTab)
{
	_actTab = newTab;
}

