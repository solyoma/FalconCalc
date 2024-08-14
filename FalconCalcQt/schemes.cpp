#include <QSettings>
#include <QFile>
#include <QApplication>

#include "FCSettings.h"
#include "schemes.h"

FalconCalcScheme FSchemeVector::light("Light:Világos",
	"#E0E0E0",	// sBackground
	"#101010",		// sTextColor	-	foreground
	"#A0A0A0",		// sBorderColor
	"#000000",		// sFocusedInput
	"#888888",		// sHoverColor
	"#9B9B9B",		// sTabBorder
	"#ffffff",		// sInputBackground - editor backgrounds
	"#3b584a",		// sSelectedInputBgr
	"#c0c0c0",		// sFocusedBorder
	"#999999",		// sDisabledFg
	"#555555",		// sDisabledBg
	"#111111",		// sImageBackground
	"#555555",		// sPressedBg	-	button pressed
	"#555555",		// sDefaultBg
	"#feb60e",		// sProgressBarChunk
	"#f0a91f",		// sWarningColor
	"#e28308",		// sBoldTitleColor - GroupBox title
	"#aaaaaa",		// sSpacerColor - spacer for drop operations
	"qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop:0 #E0E0E0, stop:1 #FFFFFF)"			// sDialogBackground
),
FSchemeVector::blue("Blue:Kék",
	"#3b5876",		// sBackground
	"#cccccc",		// sTextColor	-	foreground
	"#747474",		// sBorderColor
	"#f0f0f0",		// sFocusedInput
	"#8faed2",		// sHoverColor
	"#3b589b",		// sTabBorder
	"#1c3a55",		// sInputBackground - editor backgrounds
	"#3b584a",		// sSelectedInputBgr
	"#92b1d5",		// sFocusedBorder
	"#999999",		// sDisabledFg
	"#697a8e",		// sDisabledBg
	"#12273f",		// sImageBackground
	"#555555",		// sPressedBg	-	button pressed
	"#555555",		// sDefaultBg
	"#feb60e",		// sProgressBarChunk
	"#f0a91f",		// sWarningColor
	"#e28308",		// sBoldTitleColor - GroupBox title
	"#aaaaaa",		// sSpacerColor - spacer for drop operations
	"#3b5876"		// sDialogBackground
),
FSchemeVector::dark("Dark:Sötét",
	"#282828",			// sBacground
	"#cccccc",			// sTextColor	-	foreground
	"#4d4d4d",			// sBorderColor
	"#f0f0f0",			// sFocusedInput
	"#383838",			// sHoverColor
	"#9B9B9B",			// sTabBorder
	"#454545",			// sInputBackground - editor backgrounds
	"#666666",			// sSelectedInputBgr
	"#c0c0c0",			// sFocusedBorder
	"#999999",			// sDisabledFg
	"#555555",			// sDisabledBg
	"#111111",			// sImageBackground
	"#555555",			// sPressedBg	-	button pressed
	"#555555",			// sDefaultBg
	"#feb60e",			// sProgressBarChunk
	"#f0a91f",			// sWarningColor
	"#e28308",			// sBoldTitleColor - GroupBox title
	"#aaaaaa",			// sSpacerColor - spacer for drop operations
	"#282828"			// sDialogBackground
),
FSchemeVector::black("Black:Fekete",
	"#191919",			// sBacground
	"#e1e1e1",			// sTextColor	-	foreground
	"#323232",			// sBorderColor
	"#f0f0f0",			// sFocusedInput
	"#282828",			// sHoverColor
	"#9b9b9b",			// sTabBorder
	"#323232",			// sInputBackground - editor backgrounds
	"#4a4a4a",			// sSelectedInputBgr
	"#a8a8a8",			// sFocusedBorder
	"#999999",			// sDisabledFg
	"#191919",			// sDisabledBg
	"#000000",			// sImageBackground
	"#323232",			// sPressedBg	-	button pressed
	"#323232",			// sDefaultBg
	"#feb60e",			// sProgressBarChunk
	"#f0a91f",			// sWarningColor
	"#e28308",			// sBoldTitleColor - GroupBox title
	"#aaaaaa",			// sSpacerColor - spacer for drop operations
	"#191919"			// sDialogBacground
);

FSchemeVector schemes;		// default styles: default, system, blue, dark, black

static 	QString
		MenuTitle,			// this will appear in the menu bar	it may contain a series of titles for all languages separated by commas
		sDialogBackground,
		sBackground,
		sTextColor,
		sBorderColor,
		sFocusedInput,
		sHoverColor,
		sTabBorder,				 
		sInputBackground,
		sSelectedInputBgr,
		sFocusedBorder,
		sDisabledFg,
		sDisabledBg,
		sImageBackground,
		sPressedBg,
		sDefaultBg,
		sProgressBarChunk,
		sWarningColor,
		sBoldTitleColor,
		sSpacerColor;		// for drag & drop in tnvImages

static QString fcStyles = {
R"END(
* {
	background-color:%1;       /* %1 background */
	color:%2;                  /* %2 color */
}
        
QDialog {
	background-color:%18;       /* %1 background */
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

QProgressBar,
QPushButton:flat {
	border:0;
}

QProgressBar::chunk{
	width: 10px;
	background-color:%15;	   /* %15 progressbar chunk */
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
QLineEdit,
QSpinBox,
QPushButton,
QTreeView, 
QListView {
    border-radius: 10px;
}
						/* these 2 do not work */
QTreeView::branch:open:has-children {
	color %2;

}
QTreeView::branch:closed:has-children {
	color:%2;
}
        
/* ------------------ borders ----------------------*/   
QTabWidget:pane,     
QTabBar::tab, 
QToolTip,
QTextEdit, 
QLineEdit,
QGroupBox,
QSpinBox,
QPushButton,
QTreeView, 
QListView {
    border: 2px solid %3;	  /* %3   border color */
}

#btnImage {
	border-radius:0px;
}

/* ------------------ colors --------------------*/
QGroupBox::title {
	border-radius: 5px;
	color:%17				/* %17 bold title color */
}

QTabBar::tab,
QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
    color:%4;				/* %4 focused input */
}

QMenu::item {
	background-color:%1;
	color:%2;
}
QTabBar::tab:selected, 
QTabBar::tab:hover,
QPushButton:hover,
QToolButton:hover {
	background-color:%5;	/* %5 - hover */
}
QTabBar::tab:selected {
	border-color:%6;		/* %6 - tab border */
    border-bottom-color:%1; /* %1 - background */
}

#pnlColorScheme {
	background-color:%1; /* %1 - background */
}

QToolButton,
QTextEdit, 
QLineEdit,
QSpinBox {
	background-color:%7;	/* %7 - input background */
}
QTextEdit, 
QLineEdit,
QSpinBox {
	selection-background-color:%8;	/* %8 */
}

QTextEdit:focus, 
QLineEdit:focus,
QSpinBox:focus {
	border-color:%9;		/* %9 - focused border */
}

QTextEdit:read-only, 
QLineEdit:read-only, 
QPushButton:disabled,
QToolButton:disabled,
QRadioVutton:disabled,
QCheckBox:disabled,
QSpinBox:disabled,
QMenu:disabled {
	color:%10;			/* %10 - disabled foreground */
	background:%11;		/* %11 - disabled background */
}

QLineEdit.disabled {
	color:%10;
	background:%11;	   /* %11 - disabled background */
}

#lblBckImage,
#groupBox_6 {
	background-color:%12; /* %12 -image background */
}
QPushButton:pressed,
QToolButton:pressed {
	background-color:%13; /* %13 - pressed button background */
}

QPusButton:default {
    background-color:%14; /* %14 - default background */
}

#lblActualElem {
	color:%16;			/* %16 - warning color */
}
#btnDeleteColorScheme {
	background-color:%9;	/* %9 focused border */
}
)END"
};		// styles

// ================================ FSchemeVector ============================
QString FalconCalcScheme::MenuTitleForLanguage(int lang)
{
	if (MenuTitle.isEmpty())	// system colors
		MenuTitle = QString("Default:Alapértelmezett");		// modify when new user interface language added!
	QStringList sl = MenuTitle.split(':');
	if (lang < 0 || lang >= sl.size())
		lang = 0;
	return sl[lang];
}
QString& FalconCalcScheme::operator[](int index)
{
	switch (index)
	{				
		case 0:		return sBackground;
		case 1:		return sTextColor;
		case 2:		return sBorderColor;
		case 3:		return sFocusedInput;
		case 4:		return sHoverColor;
		case 5:		return sTabBorder;
		case 6:		return sInputBackground;
		case 7:		return sSelectedInputBgr;
		case 8:		return sFocusedBorder;
		case 9:		return sDisabledFg;
		case 10:	return sDisabledBg;
		case 11:	return sImageBackground;
		case 12:	return sPressedBg;
		case 13:	return sDefaultBg;
		case 14:	return sProgressBarChunk;
		case 15:	return sWarningColor;
		case 16:	return sBoldTitleColor;
		case 17:	return sSpacerColor;
		default:	
		case 18:	return sDialogBackground;
	}
}

FalconCalcScheme::FalconCalcScheme()
{
}

FalconCalcScheme::FalconCalcScheme(const char* t,	 // menu title
	const char* c0,	 // sBackground
	const char* c1,	 // sTextColor	-	foreground
	const char* c2,	 // sBorderColor
	const char* c3,	 // sFocusedInput
	const char* c4,	 // sHoverColor
	const char* c5,	 // sTabBorder
	const char* c6,	 // sInputBackground - editor backgrounds
	const char* c7,	 // sSelectedInputBgr
	const char* c8,	 // sFocusedBorder
	const char* c9,	 // sDisabledFg
	const char* c10, // sDisabledBg
	const char* c11, // sImageBackground
	const char* c12, // sPressedBg	-	button pressed
	const char* c13, // sDefaultBg
	const char* c14, // sProgressBarChunk
	const char* c15, // sWarningColor
	const char* c16, // sBoldTitleColor - GroupBox title
	const char* c17, // sSpacerColor - spacer for drop operations
	const char* c18	 // sDialogBackground
)					 
{
	MenuTitle = t;			// this will appear in the menu bar
	sBackground = c0;
	sTextColor = c1;
	sBorderColor = c2;
	sFocusedInput = c3;
	sHoverColor = c4;
	sTabBorder = c5;
	sInputBackground = c6;
	sSelectedInputBgr = c7;
	sFocusedBorder = c8;
	sDisabledFg = c9;
	sDisabledBg = c10;
	sImageBackground = c11;
	sPressedBg = c12;
	sDefaultBg = c13;
	sProgressBarChunk = c14;
	sWarningColor = c15;
	sBoldTitleColor = c16;
	sSpacerColor = c17;
	sDialogBackground = c18;
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
	schSystem.MenuTitle = QMainWindow::tr("System");
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
		for (auto k : keys)
		{
			s.beginGroup(k);
			fgst.MenuTitle = s.value("Title", "").toString();			// this will appear in the menu bar
			fgst.sBackground = s.value("Background", "").toString();
			fgst.sTextColor = s.value("TextColor", "").toString();
			fgst.sBorderColor = s.value("BorderColor", "").toString();
			fgst.sFocusedInput = s.value("FocusedInput", "").toString();
			fgst.sHoverColor = s.value("HoverColor", "").toString();
			fgst.sTabBorder = s.value("TabBorder", "").toString();
			fgst.sInputBackground = s.value("InputBackground", "").toString();
			fgst.sSelectedInputBgr = s.value("SelectedInputBgr", "").toString();
			fgst.sFocusedBorder = s.value("FocusedBorder", "").toString();
			fgst.sDisabledFg = s.value("DisabledFg", "").toString();
			fgst.sDisabledBg = s.value("DisabledBg", "").toString();
			fgst.sImageBackground = s.value("ImageBackground", "").toString();
			fgst.sPressedBg = s.value("PressedBg", "").toString();
			fgst.sDefaultBg = s.value("DefaultBg", "").toString();
			fgst.sProgressBarChunk = s.value("ProgressBarChunk", "").toString();
			fgst.sWarningColor = s.value("WarningColor", "").toString();
			fgst.sBoldTitleColor = s.value("BoldTitleColor", "").toString();
			fgst.sSpacerColor = s.value("SpacerColor", "").toString();
			s.endGroup();
			if( (fgst.MenuTitle == __light ) ||(fgst.MenuTitle == __blue)||(fgst.MenuTitle == __dark)||(fgst.MenuTitle == __blck))
				continue;	// already added

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
	for (int i = 2; i < size(); ++i)	// do not save default and system
	{
		char st[] = "0"; st[0] += i - 1;
		FalconCalcScheme& fgst = operator[](i);
		s.beginGroup(st);
		s.setValue("Title", fgst.MenuTitle);			// this will appear in the menu bar
		s.setValue("Background", fgst.sBackground);
		s.setValue("TextColor", fgst.sTextColor);
		s.setValue("BorderColor", fgst.sBorderColor);
		s.setValue("FocusedInput", fgst.sFocusedInput);
		s.setValue("HoverColor", fgst.sHoverColor);
		s.setValue("TabBorder", fgst.sTabBorder);
		s.setValue("InputBackground", fgst.sInputBackground);
		s.setValue("SelectedInputBgr", fgst.sSelectedInputBgr);
		s.setValue("FocusedBorder", fgst.sFocusedBorder);
		s.setValue("DisabledFg", fgst.sDisabledFg);
		s.setValue("DisabledBg", fgst.sDisabledBg);
		s.setValue("ImageBackground", fgst.sImageBackground);
		s.setValue("PressedBg", fgst.sPressedBg);
		s.setValue("DefaultBg", fgst.sDefaultBg);
		s.setValue("ProgressBarChunk", fgst.sProgressBarChunk);
		s.setValue("WarningColor", fgst.sWarningColor);
		s.setValue("BoldTitleColor", fgst.sBoldTitleColor);
		s.setValue("SpacerColor", fgst.sSpacerColor);
		s.endGroup();
	}
	wasSaved = true;
}

int FSchemeVector::IndexOf(FalconCalcScheme& fgst)
{
	return IndexOf(fgst.MenuTitle);
}

int FSchemeVector::IndexOf(const QString& title) // title: maybe full name including  all languages, like "Blue:Kék"
{
	QStringList sl, slf; // for schemes on many languages
	QString sfull;
	for (int i = 2; i < size(); ++i)
	{
		sfull = operator[](i).MenuTitle;
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
			.arg(sch.sBackground)		// %1 
			.arg(sch.sTextColor)			// %2 
			.arg(sch.sBorderColor)		// %3 
			.arg(sch.sFocusedInput)		// %4 
			.arg(sch.sHoverColor)		// %5 
			.arg(sch.sTabBorder)			// %6 
			.arg(sch.sInputBackground)	// %7
			.arg(sch.sSelectedInputBgr)	// %8 
			.arg(sch.sFocusedBorder)		// %9 
			.arg(sch.sDisabledFg)		// %10
			.arg(sch.sDisabledBg)		// %11
			.arg(sch.sImageBackground)	// %12
			.arg(sch.sPressedBg)			// %13
			.arg(sch.sDefaultBg)			// %14
			.arg(sch.sProgressBarChunk)	// %15
			.arg(sch.sWarningColor)		// %16
			.arg(sch.sBoldTitleColor)	// %17
			//			.arg(schemes[which].sSpacerColor)		// %18
			;
/*
		if (m == Scheme::schBlue)		// blue
			ss += QString(R"(QCheckBox::indicator:checked {
		image: url(":/icons/Resources/blue-checked.png");
	}

QCheckBox::indicator:unchecked {
	image: url(":/icons/Resources/blue-unchecked.png");
}
)");
*/
		((QApplication*)(QApplication::instance()))->setStyleSheet(ss);
	}
	currentScheme = m;
	return m;
}
