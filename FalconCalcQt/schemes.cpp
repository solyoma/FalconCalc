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
	for(size_t i =0; i < values.size(); ++i)
		ofs << i + 1 << "  "  << values[i].second << " =====> " << names[i] << "\n";
}
static void __SaveString(QString name, QString s)
{
	QFile of(name);
	of.open(QIODevice::WriteOnly);
	QTextStream ofs(&of);
	ofs << s << "\n";
}
#else
#define __SaveValues(a,b,c) 
#define __SaveString(a) 
#endif


static QString fcStyles = {
R"END(
* {
	background-color:%1;       /* :1 background */
	color:%2;                  /* :2 color */
}
        
QMainWindow,QDialog {
	background-color:%17;       /* :17 background */
}

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

QToolTip {
    border-radius: 4px;
}

QPushButton:flat {
	border:1px solid %3;
}

QToolButton {
	border:0;
	border-radius:12px;
	height:24px;
	width:24px;
}
#btnSelectUplinkIcon {
	width:32em;
}

QTextEdit, 
QLineEdit,
QSpinBox {
    height:20px;
	padding: 0 8px;
}

QPushButton {
	padding:2px 4px;
}
QRadioButton::indicator,   
QCheckBox::indicator {
	width: 13px;
	height: 13px;
	position: absolute;
	top:-4px;
}

QRadioButton, QCheckBox {
	spacing: 5px;
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
QTabWidget:pane,     
QTabBar::tab, 
QToolTip,
QTextEdit, 
QGroupBox,
QSpinBox,
QPushButton {
    border: 2px solid %3;	  /* :3   border color */
	border-radius:5px;
}

QLineEdit {
    border: 2px solid %3;	  /* :3   border color */
	border-radius:10px;
}

QListView,
QListWidget {
    border: 2px solid %3;	  /* :3   border color */
	border-radius:5px;
}

QListView,
QListWidget::item:alternate {
	background-color:%26;
	color:%27;
}

#btnImage {
	border-radius:0px;
}

/* ------------------ colors --------------------*/
QGroupBox::title {
	border-radius: 5px;
	color:%16				/* :16 bold title color */
}

QTabBar::tab,
QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
    color:%4;				/* :4 focused input */
}

QTabBar::tab:selected, 
QTabBar::tab:hover,
QPushButton:hover,
QToolButton:hover {
	background-color:%5;	/* :5 - hover */
}
QTabBar::tab:selected {
	border-color:%6;		/* :6 - tab border */
    border-bottom-color:%1; /* :1 - background */
}

#pnlColorScheme {
	background-color:%1; /* :1 - background */
}

QToolButton,
QTextEdit, 
QLineEdit,
QSpinBox {
	background-color:%7;	/* :7 - input background */
}
QTextEdit, 
QLineEdit,
QSpinBox {
	selection-background-color:%8;	/* :8 */
}

QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
	border-color:%9;		/* :9 - focused border */
}

QTextEdit:read-only, 
QLineEdit:read-only, 
QPushButton:disabled,
QToolButton:disabled,
QRadioVutton:disabled,
QCheckBox:disabled,
QSpinBox:disabled,
QMenu:disabled {
	color:%10;			/* :10 - disabled foreground */
	background:%11;		/* :11 - disabled background */
}

QMenu {
	border:0;
}

QMenu::item {			
	background:%18;		/* :18 - menu item background*/
	color:%19;		    /* :19 - menu item foreground*/
}

QMenu::item:hover {
	background:%18;		/* :18 - menu item hover background*/
	color:%19;			/* :19 - menu item hover foreground*/
}

QMenu::item:selected {
	background:%22;		/* :22 - menu item selected background*/
	color:%23			/* :23 - menu item selected foreground*/
}

QLineEdit.disabled {
	color:%10;
	background:%11;	   /* :11 - disabled background */
}

#lblBckImage,
#groupBox_6 {
	background-color:%12; /* :12 -image background */
}
QPushButton:pressed,
QToolButton:pressed {
	background-color:%13; /* :13 - pressed button background */
}

QPusButton:default {
    background-color:%14; /* :14 - default background */
}

#lblActualElem {
	color:%15;			/* :15 - warning color */
}
#btnDeleteColorScheme {
	background-color:%9;	/* :9 focused border */
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
		"#555555",		// %11	DisabledBg		
		"#111111",		// %12	ImageBackground
		"#555555",		// %13	PressedBg		
		"#555555",		// %14	DefaultBg		
		"#f0a91f",		// %15	WarningColor		
		"#e28308",		// %16	BoldTitleColor
		"qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop:0 #E0E0E0, stop:1 #FFFFFF)",			// %17 DialogBackground
		"#e0e0e0",		// %18  MenuBackground
		"#191010",		// %19  MenuColor
		"#90c8f6",		// %20	MenuHighlightBackground
		"#ffffff",		// %21	MenuHighlightColor
		"#90c8f6",		// %22	MenuSelectedBackground
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
		_values[i] = std::make_pair(i, "");
}

FalconCalcScheme::FalconCalcScheme(const QString sMenuTitle,  std::vector<QString> names,  std::vector<QString> values) : _sMenuTitle(sMenuTitle), _names(names)
{
	_values.resize(_names.size());
	for (int i =0; i < (int)_names.size(); ++i )
		_values[i] = std::make_pair(i, values[i]);
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
		QString ss =
			QString(fcStyles)
			.arg(sch._values[ 0].second)	// %1	sBackground
			.arg(sch._values[ 1].second)	// %2	sTextColor		
			.arg(sch._values[ 2].second)	// %3	sBorderColor		
			.arg(sch._values[ 3].second)	// %4	sFocusedInput		
			.arg(sch._values[ 4].second)	// %5	sHoverColor		
			.arg(sch._values[ 5].second)	// %6	sTabBorder		
			.arg(sch._values[ 6].second)	// %7	sInputBackground
			.arg(sch._values[ 7].second)	// %8	sSelectedInputBgr 
			.arg(sch._values[ 8].second)	// %9	sFocusedBorder	
			.arg(sch._values[ 9].second)	// %10	sDisabledFg		
			.arg(sch._values[10].second)	// %11	sDisabledBg		
			.arg(sch._values[11].second)	// %12	sImageBackground
			.arg(sch._values[12].second)	// %13	sPressedBg		
			.arg(sch._values[13].second)	// %14	sDefaultBg		
			.arg(sch._values[14].second)	// %16	sWarningColor		
			.arg(sch._values[15].second)	// %17	sBoldTitleColor
			.arg(sch._values[16].second)	// %19	sDialogBackground
			.arg(sch._values[17].second)	// %20  MenuBackground
			.arg(sch._values[18].second)	// %21  MenuColor
			.arg(sch._values[19].second)	// %22	MenuHighlightBackground
			.arg(sch._values[20].second)	// %23	MenuHighlightColor
			.arg(sch._values[21].second)	// %24	MenuSelectedBackground
			.arg(sch._values[22].second)	// %25	MenuSelectedColor
			.arg(sch._values[23].second)	// %26	MenuSeparatorColor
			.arg(sch._values[24].second)	// %27	MenuDisabledBackground
			.arg(sch._values[25].second)	// %28	ListAlternateBackground
			.arg(sch._values[26].second)	// %29	ListAlternateColor
			.arg(sch._values[27].second)	// %30	ListSelectionBackground
			.arg(sch._values[28].second);	// %31	ListSelectionColor

		// DEBUG 1 line
			__SaveString("rawStyeStr.val", ss);

		((QApplication*)(QApplication::instance()))->setStyleSheet(ss);
	}
	currentScheme = m;
	return m;
}
