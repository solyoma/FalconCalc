#include <QSettings>
#include <QFile>
#include <QApplication>

#include "FCSettings.h"
#include "schemes.h"

#ifdef NothingImportant
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

static void __SaveStyle(QString styleName, FalconCalcScheme &sch)
{
	QFile of(styleName + ".sty");
	of.open(QIODevice::WriteOnly);
	QTextStream ofs(&of);
	ofs << ((QApplication*)(QApplication::instance()))->styleSheet() << "\n";

	ofs << sch.Table();
}
#else
#define __SaveValues(a,b,c)
#define __SaveString(a)
#define __SaveStyle(a)
#endif
#endif

static QString fcStyles = {
R"END(/* Qt Trick: This MUST be part of the string 'fcStyles' !!!!!!!!!!!!!!!!
	Because if the range of %n has gaps, then the arg() can skip numbers:
   e.g. using 'QString("%1 - %3").arg(1).arg(2).arg(3)' will result in the string "1 - 2" and not "1 - 3"
	therefore to keep the order all arguments (colors) are enumerated here to be replaced by
	the corresponding arguments and stripped of comments after it
   %1 %2 %3 %4 %5 %6 %7 %8 %9 %10
   %11 %12 %13 %14 %15 %16 %17 %18 %19 %20
   %21 %22 %23 %24 %25 %26 %27 %28 %29
	ALL comments will be stripped from the style string before used (See lambda 'eraseComments'
	around lines 671
*/
* {
	background-color:%1;			/* (1) background */
	color:%2;						/* (2) color */
}

QDialog,
QMainWindow {
	background-color:%17;			/* (17) Dialog background */
}

QPushButton {
	padding:2px 4px;
}

QPushButton:flat {
	border:1px solid %3;			/* (3) BorderColor */
}

QTabWidget::tab-bar {
    left: 5px;
}

QTabBar::tab {
    border-top-left-radius: 4px;
    border-top-right-radius: 4px;
	border-color:%6;				/*	(6) tab border */
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

QListView,
QPushButton,
QSpinBox,
QTextEdit,
QTreeView {
    border-radius: 10px;
}

/* ------------------ borders ----------------------*/
QGroupBox,
QPushButton,
QSpinBox,
QTabWidget:pane,
QTabBar::tab,
QTextEdit,
QToolTip {
    border: 2px solid %3;		/* (3)  border color */
	border-radius:5px;
}
QPushButton#btnOpenCloseDecOptions,
QPushButton#btnOpenCloseHexOptions {
	border: none;
}

QGroupBox::title {
	border-radius: 5px;
	color:%16;					/* (16) bold title color */
}

QLineEdit {
    border: 2px solid %3;		/* (3)  border color */
	border-radius:10px;
}

QListView,
QListWidget {
    border: 2px solid %3;		/* (3)  border color */
	border-radius:5px;
}

#btnImage {
	border-radius:0px;
}

/* ------------------ colors --------------------*/
QLineEdit {
	background:%7;				/* (7)	InputBackground */
}

QLabel#lblChars {
	background: %1;				/* (1) background */
}

QTabBar::tab,
QTextEdit:focus,
QLineEdit:focus,
QSpinBox:focus {
    color:%4;					/* :(4) focused input */
}

QTabBar::tab:selected,
QTabBar::tab:hover,
QPushButton:hover,
QToolButton:hover {
	background-color:%5;		/* :(5) hover */
}
QTabBar::tab:selected {
	border-color:%13;			/* :(13)  tab border */
    border-bottom-color:%1;		/* :(1)  background */
}

#pnlColorScheme {
	background-color:%1;		/* :(1)  background */
}

QComboBox,
QLineEdit,
QSpinBox {
	background-color:%7;		/* (7)  input background */
}
QTextEdit,
QLineEdit,
QSpinBox {
	selection-background-color:%8;	/* :(8)  selection background*/
}

QSpinBox::up-arrow,
QSpinBox::down-arrow {			 /* (2)   text*/
	color:%2;
}

QTextEdit:focus,
QLineEdit:focus,
QSpinBox:focus {
	border-color:%9;			/* (9)  focused border */
}

QTextEdit:read-only,
QLineEdit:read-only,
QPushButton:disabled,
QToolButton:disabled,
QRadioButton:disabled,
QCheckBox:disabled,
QSpinBox:disabled {
	color:%10;					/* (10) disabled foreground */
	background:%11;				/* (11) disabled background */
}
/* ----------- QMenu, QMenuBar, QmenuItem ------*/
QMenuBar::item:selected {
	background:%18;				/* (18) menu selected background*/
	color:%19;					/* (19) menu selected foreground*/
}

QMenu {
	border:0;
}
QMenu:disabled {
	color:%10;					/* (10) disabled foreground */
	background:%11;				/* (11) disabled background */
}

QMenu::item {
  background-color:%1;			/* will be the the same as (1) color */
}

QMenu::item:selected {
	background:%18;				/* (22) MenuSelectedBackground*/
	color:%23;					/* (23) MenuSelectedColor     */
}

WMenu::item::highlighted {
	background:%20;				/* (20)	MenuHighlightBackground */
	color:%21;					/* (21)	MenuHighlightColor */
}

QLineEdit.disabled {
	color:%10;					/* (10) disabled foreground */
	background:%11;				/* (11) disabled background */
}

#lblBckImage,
#groupBox_6 {
	background-color:%12;		/* (12) image background */
}
QPushButton:pressed,
QToolButton:pressed {
	background-color:%13;		/* (13) pressed button background */
}

QPusButton:default {
    background-color:%14;		/* (14)  default background */
}


QTableWidget {
	background:%24;				/* (24) TableWidgetBackground*/
	color:%25;					/* (25) color */
	alternate-background-color:%26;	/* (26) alternate background */
}

QHeaderView::section {
	background:%24;				/* (24) TableWidgetBackground */
	color:%25;					/* (25) color */
}

#lblActualElem {
	color:%15;					/* (15)  warning color */
}
#btnDeleteColorScheme {
	background-color:%9;		/* (9) focused border */
}
)END"
};		// styles


static std::vector<QString> __SchemeItemNames = {
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
	"MenuBarBackground",		// %18
	"MenuBarColor",				// %19
	"MenuHighlightBackground",	// %20
	"MenuHighlightColor",		// %21
	"MenuSelectedBackground",	// %22
	"MenuSelectedColor",		// %23
	"TableWidgetBackground",	// %24
	"TableWidgetColor",			// %25
	"TableWidgetAlternateBackground",// %26
	"MenuSeparatorColor",		// %27
	"MenuDisabled",				// %28
	"TableWidgetAlternateColor",// %29
};

static std::vector<QString>
	__lightV =
	{
		"#EEEEEE",		// %1	Background
		"#101010",		// %2	TextColor
		"#A0A0A0",		// %3	BorderColor
		"#000000",		// %4	FocusedInput
		"#DDDDDD",		// %5	HoverColor
		"#9B9B9B",		// %6	TabBorder
		"#FFFFFF",		// %7	InputBackground
		"#EEEEEE",		// %8	SelectedInputBgr
		"#a0a0a0",		// %9	FocusedBorder
		"#CCCCCC",		// %10	DisabledFg
		"#909090",		// %11	DisabledBg
		"#111111",		// %12	ImageBackground
		"#555555",		// %13	PressedBg
		"#e0e0e0",		// %14	DefaultBg
		"#f0a91f",		// %15	WarningColor
		"#e28308",		// %16	BoldTitleColor
		"#E0E0E0",		// %17  DialogBackground
		"#cce8ff",		// %18  MenuBarBackground
		"#101010",		// %19  MenuBarColor
		"#90c8f6",		// %20	MenuHighlightBackground
		"#101010",		// %21	MenuHighlightColor
		"#EEEEEE",		// %22	MenuSelectedBackground
		"#101010",		// %23	MenuSelectedColor
		"#e0e0e0",		// %24	TableWidgetBackground
		"#000000",		// %25	TableWidgetColor
		"#d0d0d0",		// %26	TableWidgetAlternateBackground
		"#101010",		// %27	MenuSeparatorColor
		"#aaaaaa",		// %28	MenuDisabledBackground
		"#aaaaaa",		// %29	TableWidgetAlternateColor
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
		"#e28308",		// %16	BoldTitleColor
		"#000000",		// %17	sDialogBackground
		"#e0e0e0",		// %18  MenuBackground
		"#000000",		// %19  MenuColor
		"#0000aa",		// %20	MenuHighlightBackground
		"#ffffff",		// %21	MenuHighlightColor
		"#a0a0a0",		// %22	MenuSelectedBackground
		"#000000",		// %23	MenuSelectedColor
		"#303030",		// %24	TableWidgetBackground
		"#aaaaaa",		// %25	TableWidgetColor
		"#505050",		// %26	TableWidgetAlternateBackground
		"#aaaaaa",		// %27	MenuSeparatorColor
		"#aaaaaa",		// %28	MenuDisabledBackground
		"#aaaaaa",		// %29	TableWidgetAlternateColor
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
		"#a0a0a0",		// %22	MenuSelectedBackground
		"#000000",		// %23	MenuSelectedColor
		"#191919",		// %24	TableWidgetBackground
		"#aaaaaa",		// %25	TableWidgetColor
		"#303030",		// %26	TableWidgetAlternateBackground
		"#aaaaaa",		// %27	MenuSeparatorColor
		"#aaaaaa",		// %28	MenuDisabledBackground
		"#aaaaaa",		// %29	TableWidgetAlternateColor
},
	__blueV =
	{
		"#3b5876",		// %1	Background
		"#cccccc",		// %2	TextColor
		"#747474",		// %3	BorderColor
		"#f0f0f0",		// %4	FocusedInput
		"#8faed2",		// %5	HoverColor
		"#3b589b",		// %6	TabBorder
		"#4b6a8b",		// %7	InputBackground
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
		"#cce8ff",		// %18  MenuBackground
		"#101010",		// %19  MenuColor
		"#90c8f6",		// %20	MenuHighlightBackground
		"#101010",		// %21	MenuHighlightColor
		"#90c8f6",		// %22	MenuSelectedBackground
		"#101010",		// %23	MenuSelectedColor
		"#3b5876",		// %24	TableWidgetBackground
		"#cccccc",		// %25	TableWidgetColor
		"#476a8d",		// %26	TableWidgetAlternateBackground
		"#aaaaaa",		// %27	MenuSeparatorColor
		"#aaaaaa",		// %28	MenuDisabledBackground
		"#aaaaaa",		// %29	TableWidgetAlternateColor
	};

FalconCalcScheme FSchemeVector::light("Light:Világos",  __SchemeItemNames, __lightV);
FalconCalcScheme FSchemeVector::dark("Dark:Sötét",		__SchemeItemNames, __darkV);
FalconCalcScheme FSchemeVector::black("Black:Fekete",	__SchemeItemNames, __blackV);
FalconCalcScheme FSchemeVector::blue("Blue:Kék",		__SchemeItemNames, __blueV);

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
	//__SaveValues(sMenuTitle.left(4), _names, _values);
	//__SaveString("styeStr.val", fcStyles);
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

// DEBUG
#include <QFile>
#include <QTextStream>
// /DEBUG

Scheme FSchemeVector::PrepStyle(Scheme m)
{
	// to minimize the # of copies required we start from the last comment
	auto eraseComments = [](QString &s)
		{
			int from = 0;
			while ((from = s.indexOf("/*", from)) >= 0)
			{
				int i = s.indexOf("*/", from + 2);	// skip '/*'
				if (i > 0)
					i -= from-2;
				while(s[from+i]=='\n' && s[from+i+1] == '\n')
					++i;
				s.remove(from, i);
			}
		};

	QString ss;
	FalconCalcScheme sch( SchemeFor(m) );

	if (m == Scheme::schSystem)
	{
		//ss = "QGroupBox#gbResults::title,\n"
		//	"QGroupBox#gbDecOptions::title,\n"
		//	"QGroupBox#gbHexOptions::title {\n"
		//	"  height: 0px;\n"
		//	"}\n";
		((QApplication*)(QApplication::instance()))->setStyleSheet(ss);
	}
	else
	{

		//__SaveString("fcStyles.str", fcStyles);
		ss =				//		variable
			QString(fcStyles)		//   index    name
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
			.arg(sch._values[14].second)	// %15	sWarningColor
			.arg(sch._values[15].second)	// %16	sBoldTitleColor
			.arg(sch._values[16].second)	// %17	sDialogBackground
			.arg(sch._values[17].second)	// %18  MenuBackground
			.arg(sch._values[18].second)	// %19  MenuColor
			.arg(sch._values[19].second)	// %20	MenuHighlightBackground
			.arg(sch._values[20].second)	// %21	MenuHighlightColor
			.arg(sch._values[21].second)	// %22	MenuSelectedBackground
			.arg(sch._values[22].second)	// %23	MenuSelectedColor
			.arg(sch._values[23].second)	// %24	MenuSeparatorColor
			.arg(sch._values[24].second)	// %25	MenuDisabledBackground
			.arg(sch._values[25].second)	// %26	ListAlternateBackground
			.arg(sch._values[26].second)	// %27	ListAlternateColor
			.arg(sch._values[27].second)	// %28	ListSelectionBackground
			.arg(sch._values[28].second);	// %29	ListSelectionColor
		eraseComments(ss);
		((QApplication*)(QApplication::instance()))->setStyleSheet(ss);
	}
#ifdef NothingImportant
		QString s;
		switch (m)
		{
			case Scheme::schSystem:s = "system"; break;
			case Scheme::schLight: s = "light"; break;
			case Scheme::schDark:  s = "dark"; break;
			case Scheme::schBlack: s = "black"; break;
			case Scheme::schBlue:  s = "blue"; break;
		}
		__SaveStyle(QString("FalconCalc_%1.sty").arg(s), sch);
#endif
	currentScheme = m;
	return m;
}

