#include <QSettings>
#include <QFile>
#include <QApplication>

#include "FCSettings.h"
#include "schemes.h"

#ifdef _DEBUG
#include <QFile>
#include <QTextStream>
static void __SaveValues(QString valName, std::vector<QString> names, std::vector<std::pair<int, QString>> &values)
{
	QFile of(valName + ".val");
	of.open(QIODevice::WriteOnly);
	QTextStream ofs(&of);
	ofs << "/* -- " << valName << ".val -- */\n\n";
	for(size_t i =0; i < values.size(); ++i)
		ofs << i + 1 << "  "  << values[i].second << " =====> " << names[i] << "\n";
}
static void __SaveString(QString name, QString s)
{
	QFile of(name);
	of.open(QIODevice::WriteOnly);
	QTextStream ofs(&of);
	ofs << "/* -- " << name << " -- */\n\n";
	ofs << s << "\n";
}
#else
#define __SaveValues(a,b,c) 
#define __SaveString(a) 
#endif


static QString fcStyles = {
R"END(
/* If not all values are used then the arg() can skip
   e.g. QString("%1 - %3").arg(1).arg(2).arg(3) will result in the string "1 - 2" and not "1 - 3"
   %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 
   %11 %12 %13 %14 %15 %16 %17 %18 %19 %20
   %21 %22 %23 %24 %25 %26 %27 %28 %29
*/
* {
	background-color:%1;		/* :%2 (1) background */
	color:%3;					/* :%4 (2) color */
}
        
QMainWindow,QDialog {
	background-color:%35;		/* :%36 (17) background */
}
/*---------------------------*/
QTabWidget::tab-bar {
    left: 5px;
}

QTabBar::tab {
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
	min-width: 20ex;
	padding: 2px;
}        
QTabBar::tab:!selected {
    margin-top: 2px; 
}
/*---------------------------*/
QToolTip {
    border-radius: 4px;
}

QPushButton:flat {
	border:1px solid %3;
}

QPushButton {
	padding:2px 4px;
}

QToolButton {
	border:0;
	border-radius:12px;
	height:24px;
	width:24px;
}

QRadioButton, QCheckBox {
	spacing: 5px;
}
QRadioButton::indicator,   
QCheckBox::indicator {
	width: 13px;
	height: 13px;
	position: absolute;
	top:-4px;
}

QTextEdit, 
QLineEdit,
QSpinBox {
    height:20px;
	padding: 0 8px;
}

QTextEdit, 
QSpinBox,
QPushButton,
QTreeView, 
QListView {
    border-radius: 10px;
}
						
QLabel#lblChars {
	background-color: rgba(255,255,255,0);
}
        
/* ------------------ borders ----------------------*/   
QGroupBox::title {
	border-radius: 5px;
	color:%33;					/* :%34 (16) bold title color */
}

QTabWidget:pane,     
QTabBar::tab, 
QToolTip,
QTextEdit, 
QGroupBox,
QSpinBox,
QPushButton {
    border: 2px solid %7;		/* :%8  (3)  border color */
	border-radius:5px;
}

QLineEdit {
    border: 2px solid %7;		/* :%8  (3)  border color */
	border-radius:10px;
}

QListView,
QListWidget {
    border: 2px solid %7;		/* :%8  (3)  border color */
	border-radius:5px;
}

QListView,
QListWidget::item:alternate {
	background-color:%53;		/* :%54  (26) alternate background */
	color:%55;					/* :%56  (27) alternate color */
}

#btnImage {
	border-radius:0px;
}

/* ------------------ colors --------------------*/
QTabBar::tab,
QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
    color:%9;					/* :%10 (4) focused input */
}

QTabBar::tab:selected, 
QTabBar::tab:hover,
QPushButton:hover,
QToolButton:hover {
	background-color:%11;		/* :%12 (5) hover */
}
QTabBar::tab:selected {
	border-color:%13;			/* :%14 (6)  tab border */
    border-bottom-color:%1;		/* :%2  (1)  background */
}

#pnlColorScheme {
	background-color:%1;		/* :%2  (1)  background */
}

QToolButton,
QTextEdit, 
QLineEdit,
QSpinBox {
	background-color:%15;		/* :%16  (7)  input background */
}
QTextEdit, 
QLineEdit,
QSpinBox {
	selection-background-color:%17;	/* :%18 (8)  selection background*/
}

QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
	border-color:%19;			/* :%20 (9)  focused border */
}

QTextEdit:read-only, 
QLineEdit:read-only, 
QPushButton:disabled,
QToolButton:disabled,
QRadioButton:disabled,
QCheckBox:disabled,
QSpinBox:disabled {
	color:%21;					/* :%22 (10) disabled foreground */
	background:%23;				/* :%24 (11) disabled background */
}
/* ----------- QMenu, QMenuBar, QmenuItem ------*/
QMenu {
	border:0;
}
QMenu:disabled {
	color:%21;					/* :%22 (10) disabled foreground */
	background:%23;				/* :%24 (11) disabled background */
}

QMenu::item:selected,
QMenuBar::item:selected {			
	background:%45;				/* :%46 - (22) menu selected background*/
	color:%47;					/* :%48 - (23) menu selected foreground*/
}

QLineEdit.disabled {
	color:%10;
	background:%23;				/* :%24 - (11) disabled background */
}										  

#lblBckImage,
#groupBox_6 {
	background-color:%25;		/* :%26 (12) image background */
}										
QPushButton:pressed,
QToolButton:pressed {
	background-color:%27;		/* :%28 (13) pressed button background */
}

QPusButton:default {
    background-color:%29;		/* :%30 (14)  default background */
}

#lblActualElem {
	color:%31;					/* :%32 (15)  warning color */
}
#btnDeleteColorScheme {
	background-color:%19;		/* :%20 (9) focused border */
}
)END"
};		// styles


static std::vector<QString> __schemeNames = {
	"Background",				// %1
	"TextColor",				// %2
	"BorderColor",				// %3
	"FocusedInput",				// %4
	"HoverColor",				// %5
	"TabBorder",				// %6
	"InputBackground",			// %7
	"SelectedInputBgr",			// %8
	"FocusedBorder",			// %9
	"DisabledFg",				// %10
	"DisabledBg",				// %11
	"ImageBackground",			// %12
	"PressedBg",				// %13
	"DefaultBg",				// %14
	"WarningColor",				// %15
	"BoldTitleColor",			// %16
	"DialogBackground",			// %17
	"MenuBackground",			// %18
	"MenuColor",				// %19
	"MenuHighlightBackground",	// %20
	"MenuHighlightColor",		// %21
	"MenuSelectedBackground",	// %22
	"MenuSelectedColor",		// %23
	"MenuSeparatorColor",		// %24
	"MenuDisabled",				// %25
	"ListAlternateBackground",	// %26
	"ListAlternateColor",		// %27
	"ListSelectionBackground",	// %28
	"ListSelectionColor",		// %29
};

static std::vector<QString> 
	__lightV = 
	{
		"#E0E0E0",		// %1	Background
		"#101010",		// %2	TextColor		
		"#808080",		// %3	BorderColor		
		"#000000",		// %4	FocusedInput		
		"#888888",		// %5	HoverColor		
		"#9B9B9B",		// %6	TabBorder		
		"#ffffff",		// %7	InputBackground
		"#3b584a",		// %8	SelectedInputBgr 
		"#a0a0a0",		// %9	FocusedBorder	
		"#999999",		// %10	DisabledFg		
		"#909090",		// %11	DisabledBg		
		"#111111",		// %12	ImageBackground
		"#555555",		// %13	PressedBg		
		"#e0e0e0",		// %14	DefaultBg		
		"#f0a91f",		// %15	WarningColor		
		"#e28308",		// %16	BoldTitleColor
		"qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop:0 #E0E0E0, stop:1 #FFFFFF)",			// %17 DialogBackground
		"#e0e0e0",		// %18  MenuBackground
		"#101010",		// %19  MenuColor
		"#a0a0a0",		// %20	MenuHighlightBackground
		"#101010",		// %21	MenuHighlightColor
		"#a0a0a0",		// %22	MenuSelectedBackground
		"#101010",		// %23	MenuSelectedColor
		"#101010",		// %24	MenuSeparatorColor
		"#aaaaaa",		// %25	MenuDisabledBackground
		"#a0a0a0",		// %26	ListAlternateBackground
		"#000000",		// %27	ListAlternateColor
		"#aaaaaa",		// %28	ListSelectionBackground
		"#aaaaaa",		// %29	ListSelectionColor
	},						
	__darkV = 				
	{ 						
		"#282828",		// %1	Background
		"#cccccc",		// %2	TextColor		
		"#4d4d4d",		// %3	BorderColor		
		"#f0f0f0",		// %4	FocusedInput		
		"#383838",		// %5	HoverColor		
		"#9B9B9B",		// %6	TabBorder		
		"#454545",		// %7	InputBackground
		"#666666",		// %8	SelectedInputBgr 
		"#c0c0c0",		// %9	FocusedBorder	
		"#999999",		// %10	DisabledFg		
		"#555555",		// %11	DisabledBg		
		"#111111",		// %12	ImageBackground
		"#555555",		// %13	PressedBg		
		"#555555",		// %14	DefaultBg		
		"#e28308",		// %15	WarningColor		
		"#282828",		// %16	BoldTitleColor
		"#000000",		// %17	sDialogBackground
		"#202020",		// %18  MenuBackground
		"#ffffff",		// %19  MenuColor
		"#a0a0aa",		// %20	MenuHighlightBackground
		"#ffffff",		// %21	MenuHighlightColor
		"#aaaaaa",		// %22	MenuSelectedBackground
		"#aaaaaa",		// %23	MenuSelectedColor
		"#aaaaaa",		// %24	MenuSeparatorColor
		"#aaaaaa",		// %25	MenuDisabledBackground
		"#aaaaaa",		// %26	ListAlternateBackground
		"#aaaaaa",		// %27	ListAlternateColor
		"#aaaaaa",		// %28	ListSelectionBackground
		"#aaaaaa",		// %29	ListSelectionColor
},
	__blackV =
	{ 
		"#191919",		// %1	Background
		"#e1e1e1",		// %2	TextColor		
		"#323232",		// %3	BorderColor		
		"#f0f0f0",		// %4	FocusedInput		
		"#282828",		// %5	HoverColor		
		"#9b9b9b",		// %6	TabBorder		
		"#323232",		// %7	InputBackground
		"#4a4a4a",		// %8	SelectedInputBgr 
		"#a8a8a8",		// %9	FocusedBorder	
		"#999999",		// %10	DisabledFg		
		"#191919",		// %11	DisabledBg		
		"#000000",		// %12	ImageBackground
		"#323232",		// %13	PressedBg		
		"#323232",		// %14	DefaultBg		
		"#f0a91f",		// %15	WarningColor		
		"#e28308",		// %16	BoldTitleColor
		"#191919",		// %17	DialogBackground
		"#e0e0e0",		// %18  MenuBackground
		"#000000",		// %19  MenuColor
		"#0000aa",		// %20	MenuHighlightBackground
		"#ffffff",		// %21	MenuHighlightColor
		"#0000aa",		// %22	MenuSelectedBackground
		"#ffffff",		// %23	MenuSelectedColor
		"#aaaaaa",		// %24	MenuSeparatorColor
		"#aaaaaa",		// %25	MenuDisabledBackground
		"#aaaaaa",		// %26	ListAlternateBackground
		"#aaaaaa",		// %27	ListAlternateColor
		"#aaaaaa",		// %28	ListSelectionBackground
		"#aaaaaa",		// %29	ListSelectionColor
},
	__blueV = 
	{
		"#3b5876",		// %1	Background
		"#cccccc",		// %2	TextColor		
		"#747474",		// %3	BorderColor		
		"#f0f0f0",		// %4	FocusedInput		
		"#8faed2",		// %5	HoverColor		
		"#3b589b",		// %6	TabBorder		
		"#1c3a55",		// %7	InputBackground
		"#3b584a",		// %8	SelectedInputBgr 
		"#92b1d5",		// %9	FocusedBorder	
		"#999999",		// %10	DisabledFg		
		"#697a8e",		// %11	DisabledBg		
		"#12273f",		// %12	ImageBackground
		"#555555",		// %13	PressedBg		
		"#555555",		// %14	DefaultBg		
		"#f0a91f",		// %15	WarningColor		
		"#e28308",		// %16	BoldTitleColor
		"#3b5876", 		// %17	DialogBackground
		"#e0e0e0",		// %18  MenuBackground
		"#cce8ff",		// %18  MenuBackground
		"#101010",		// %19  MenuColor
		"#90c8f6",		// %20	MenuHighlightBackground
		"#101010",		// %21	MenuHighlightColor
		"#90c8f6",		// %22	MenuSelectedBackground
		"#101010",		// %23	MenuSelectedColor
		"#aaaaaa",		// %24	MenuSeparatorColor
		"#aaaaaa",		// %25	MenuDisabledBackground
		"#aaaaaa",		// %26	ListAlternateBackground
		"#aaaaaa",		// %27	ListAlternateColor
		"#aaaaaa",		// %28	ListSelectionBackground
		"#aaaaaa",		// %29	ListSelectionColor
	};

FalconCalcScheme FSchemeVector::light("Light:Világos",  __schemeNames, __lightV);
FalconCalcScheme FSchemeVector::dark("Dark:Sötét",		__schemeNames, __darkV);
FalconCalcScheme FSchemeVector::black("Black:Fekete",	__schemeNames, __blackV);
FalconCalcScheme FSchemeVector::blue("Blue:Kék",		__schemeNames, __blueV);

FSchemeVector schemes;		// default styles: default, system, blue, dark, black

// ================================ FalconCalcSchemes ============================

FalconCalcScheme::FalconCalcScheme(const QString sMenuTitle, std::vector<QString> names)
{
	_values.resize(_names.size());
	for (int i =0; i < (int)_names.size(); ++i )
		_values[i] = std::make_pair(i+1, "");
}

FalconCalcScheme::FalconCalcScheme(const QString sMenuTitle,  std::vector<QString> names,  std::vector<QString> values) : _sMenuTitle(sMenuTitle), _names(names)
{
	_values.resize(_names.size());
	for (int i =0; i < (int)_names.size(); ++i )
		_values[i] = std::make_pair(i+1, values[i]);
	// DEBUG
	__SaveValues(sMenuTitle.left(4), _names, _values);
	__SaveString("styeStr.val", fcStyles);
}

FalconCalcScheme::FalconCalcScheme()
{
}

QString FalconCalcScheme::MenuTitleForLanguage(int lang)
{
	if (_sMenuTitle.isEmpty())	// system colors
		_sMenuTitle = QString("Default:Alapértelmezett");		// modify when new user interface language added!
	QStringList sl = _sMenuTitle.split(':');
	if (lang < 0 || lang >= sl.size())
		lang = 0;
	return sl[lang];
}
QString& FalconCalcScheme::operator[](int index)
{
	return _values[index].second;
}

// ================================ FSchemeVector ============================
static const QString __def = "Default:Alapértelmezett";
static const QString __light = "Light:Világos";
static const QString __blue = "Blue:Kék";
static const QString __dark = "Dark:Sötét";
static const QString __blck = "Black:Fekete";

FSchemeVector::FSchemeVector()
{
//	reserve(6);		// for default, system, light, dark, black, blue 
	FalconCalcScheme schSystem;
	schSystem._sMenuTitle = QMainWindow::tr("System");
	push_back(schSystem);		
	push_back(light);
	push_back(dark);
	push_back(black);
	push_back(blue);

}

FalconCalcScheme FSchemeVector::SchemeFor(Scheme sch)
{
	return QVector<FalconCalcScheme>::operator[]((int)sch);
}

void FSchemeVector::push_back(const FalconCalcScheme& scheme)
{
	wasSaved = false;
	QVector<FalconCalcScheme>::push_back(scheme);
}


void FSchemeVector::ReadAndSetupSchemes()
{
	const QString schemeFile = FCSettings::homePath + "FalconCalc.fsty";
	bool li = false, da = false, bk = false, bl = false;	// light, dark, black, blue set
	if (QFile::exists(schemeFile ))
	{
		QSettings s(schemeFile, QSettings::IniFormat);	// in program directory;
		s.setIniCodec("UTF-8");
		QStringList keys = s.childGroups();	// get all top level keys with subkeys
		FalconCalcScheme fgst;
		for (auto &k : keys)
		{
			s.beginGroup(k);
			fgst._sMenuTitle = s.value("Title", "").toString();			// this will appear in the menu bar
			if( (fgst._sMenuTitle == __light ) ||(fgst._sMenuTitle == __blue)||(fgst._sMenuTitle == __dark)||(fgst._sMenuTitle == __blck))
				continue;	// already added
			for (auto& d : fgst._values)
				d.second = s.value(fgst._names[d.first], "").toString();
			s.endGroup();

			push_back(fgst);
		}
	}
	wasSaved = true;
}

void FSchemeVector::Save()
{
	const QString schemeFile = FCSettings::homePath + "FalconCalc.fsty";

	if (QFile::exists(schemeFile))
		QFile::remove(schemeFile);

	QSettings s(schemeFile, QSettings::IniFormat);	// in program directory;
	for (int i = 4; i < size(); ++i)	// do not save builtins
	{
		char st[] = "0"; st[0] += i - 1;
		FalconCalcScheme& fgst = operator[](i);
		s.beginGroup(st);
		s.setValue("Title", fgst._sMenuTitle);			// this will appear in the menu bar
		for (int i = 0; i < (int)fgst._names.size(); ++i)
			s.setValue(fgst._names[i], fgst._values[i].second);
		s.endGroup();
	}
	wasSaved = true;
}

int FSchemeVector::IndexOf(FalconCalcScheme& fgst)
{
	return IndexOf(fgst._sMenuTitle);
}

int FSchemeVector::IndexOf(const QString& title) // title: maybe full name including  all languages, like "Blue:Kék"
{
	QStringList sl, slf; // for schemes on many languages
	QString sfull;
	for (int i = 2; i < size(); ++i)
	{
		sfull = operator[](i)._sMenuTitle;
		if (sfull == title)	// full title matched
			return i;
		sl = title.split(':');	// check for name collision
		slf = sfull.split(':');
		for (int k = 0; k < sl.size() && k < slf.size(); ++k)
			if (sl[k] == slf[k])
				return i;
	}
	return -1;
}

Scheme FSchemeVector::PrepStyle(Scheme m)
{
	if (m == Scheme::schSystem)
		((QApplication*)(QApplication::instance()))->setStyleSheet("");
	else
	{
		FalconCalcScheme sch( SchemeFor(m) );

		__SaveString("fcStyles.str", fcStyles);
		QString sz = "%1-%3-%2";
		sz = sz.arg(1).arg(2).arg(3);

		QString ss =											//		variable				index in 
			QString(fcStyles)									//   index    name				style sheet
			.arg(sch._values[ 0].second).arg(sch._values[ 0].first)	// %1	sBackground				    1
			.arg(sch._values[ 1].second).arg(sch._values[ 1].first)	// %2	sTextColor				    3
			.arg(sch._values[ 2].second).arg(sch._values[ 2].first)	// %3	sBorderColor			    5
			.arg(sch._values[ 3].second).arg(sch._values[ 3].first)	// %4	sFocusedInput			    7
			.arg(sch._values[ 4].second).arg(sch._values[ 4].first)	// %5	sHoverColor				    9
			.arg(sch._values[ 5].second).arg(sch._values[ 5].first)	// %6	sTabBorder				   11
			.arg(sch._values[ 6].second).arg(sch._values[ 6].first)	// %7	sInputBackground		   13
			.arg(sch._values[ 7].second).arg(sch._values[ 7].first)	// %8	sSelectedInputBgr 		   15
			.arg(sch._values[ 8].second).arg(sch._values[ 8].first)	// %9	sFocusedBorder			   17
			.arg(sch._values[ 9].second).arg(sch._values[ 9].first)	// %10	sDisabledFg				   19
			.arg(sch._values[10].second).arg(sch._values[10].first)	// %11	sDisabledBg				   21
			.arg(sch._values[11].second).arg(sch._values[11].first)	// %12	sImageBackground		   23
			.arg(sch._values[12].second).arg(sch._values[12].first)	// %13	sPressedBg				   25
			.arg(sch._values[13].second).arg(sch._values[13].first)	// %14	sDefaultBg				   27
			.arg(sch._values[14].second).arg(sch._values[14].first)	// %15	sWarningColor			   29
			.arg(sch._values[15].second).arg(sch._values[15].first)	// %16	sBoldTitleColor			   31
			.arg(sch._values[16].second).arg(sch._values[16].first)	// %17	sDialogBackground		   33
			.arg(sch._values[17].second).arg(sch._values[17].first)	// %18  MenuBackground			   35
			.arg(sch._values[18].second).arg(sch._values[18].first)	// %19  MenuColor				   37
			.arg(sch._values[19].second).arg(sch._values[19].first)	// %20	MenuHighlightBackground	   39
			.arg(sch._values[20].second).arg(sch._values[20].first)	// %21	MenuHighlightColor		   41
			.arg(sch._values[21].second).arg(sch._values[21].first)	// %22	MenuSelectedBackground	   43
			.arg(sch._values[22].second).arg(sch._values[22].first)	// %23	MenuSelectedColor		   45
			.arg(sch._values[23].second).arg(sch._values[23].first)	// %24	MenuSeparatorColor		   47
			.arg(sch._values[24].second).arg(sch._values[24].first)	// %25	MenuDisabledBackground	   49
			.arg(sch._values[25].second).arg(sch._values[25].first)	// %26	ListAlternateBackground	   51
			.arg(sch._values[26].second).arg(sch._values[26].first)	// %27	ListAlternateColor		   53
			.arg(sch._values[27].second).arg(sch._values[27].first)	// %28	ListSelectionBackground	   55
			.arg(sch._values[28].second).arg(sch._values[28].first);// %29	ListSelectionColor		   57

		// DEBUG
		QString qsDbg;
		for (size_t i = 0; i < sch._values.size(); i++)
			qsDbg += QString("\x25%1 -> \x25%2\n").arg(2*i+1).arg(2*i + 2);

		QString qsRes = qsDbg
			.arg(sch._values[ 0].first).arg(sch._values[ 0].second)		//  1		  // %1
			.arg(sch._values[ 1].first).arg(sch._values[ 1].second)		//  3		  // %2
			.arg(sch._values[ 2].first).arg(sch._values[ 2].second)		//  5		  // %3
			.arg(sch._values[ 3].first).arg(sch._values[ 3].second)		//  7		  // %4
			.arg(sch._values[ 4].first).arg(sch._values[ 4].second)		//  9		  // %5
			.arg(sch._values[ 5].first).arg(sch._values[ 5].second)		// 11		  // %6
			.arg(sch._values[ 6].first).arg(sch._values[ 6].second)		// 13		  // %7
			.arg(sch._values[ 7].first).arg(sch._values[ 7].second)		// 15		  // %8
			.arg(sch._values[ 8].first).arg(sch._values[ 8].second)		// 17		  // %9
			.arg(sch._values[ 9].first).arg(sch._values[ 9].second)		// 19		  // %10
			.arg(sch._values[10].first).arg(sch._values[10].second)		// 21		  // %11
			.arg(sch._values[11].first).arg(sch._values[11].second)		// 23		  // %12
			.arg(sch._values[12].first).arg(sch._values[12].second)		// 25		  // %13
			.arg(sch._values[13].first).arg(sch._values[13].second)		// 27		  // %14
			.arg(sch._values[14].first).arg(sch._values[14].second)		// 29		  // %15
			.arg(sch._values[15].first).arg(sch._values[15].second)		// 31		  // %16
			.arg(sch._values[16].first).arg(sch._values[16].second)		// 33		  // %17
			.arg(sch._values[17].first).arg(sch._values[17].second)		// 35		  // %18
			.arg(sch._values[18].first).arg(sch._values[18].second)		// 37		  // %19
			.arg(sch._values[19].first).arg(sch._values[19].second)		// 39		  // %20
			.arg(sch._values[20].first).arg(sch._values[20].second)		// 41		  // %21
			.arg(sch._values[21].first).arg(sch._values[21].second)		// 43		  // %22
			.arg(sch._values[22].first).arg(sch._values[22].second)		// 45		  // %23
			.arg(sch._values[23].first).arg(sch._values[23].second)		// 47		  // %24
			.arg(sch._values[24].first).arg(sch._values[24].second)		// 49		  // %25
			.arg(sch._values[25].first).arg(sch._values[25].second)		// 51		  // %26
			.arg(sch._values[26].first).arg(sch._values[26].second)		// 53		  // %27
			.arg(sch._values[27].first).arg(sch._values[27].second)		// 55		  // %28
			.arg(sch._values[28].first).arg(sch._values[28].second);	// 57		  // %29
			__SaveString("piecemal.val", qsRes);

		__SaveString("oriStyeStr.val", ss);
		// /DEBUG

		((QApplication*)(QApplication::instance()))->setStyleSheet(ss);
	}
	currentScheme = m;
	return m;
}
