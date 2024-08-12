#pragma once
#ifndef _SCHEMES_H
#define _SCHEMES_H

#include <QObject>
#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QStringList>

enum class Scheme { schSystem, schLight, schDark, schBlack, schBlue };

struct FalconCalcScheme
{
	QString
		MenuTitle,			// this will appear in the menu bar	if at least one user scheme is set it may contain a series of titles for all languages separated by colons
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

	QString MenuTitleForLanguage(int lang);
	QString& operator[](int index);

	FalconCalcScheme();

	FalconCalcScheme(const char* t,	 // menu title
		const char* c0,	 // sBacground
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
		const char* c17	 // sSpacerColor - spacer for drop operations
	);
};


/*========================================================
 * Style sheets (skins) for FalconCalc
 * REMARKS: - default styles (default, system, blue, dark, black)
 *				are always present
 *			- styles are read from FalconCalc.sty in program
 *				directory
 *			- if a style there has the same Title as any
 *				of the last 3 of the default they will be
 *				overwritten
 *			- styles default and system are always the first
 *				2 styles used, and if not redefined then
 *				blue, dark and black are the last ones
 *			- if a style requested which is no longer
 *				present, the 'default' style is used instead
 *-------------------------------------------------------*/
class FSchemeVector : public  QVector<FalconCalcScheme>
{
	static FalconCalcScheme light, dark, black, blue;
public:
	Scheme currentScheme = Scheme::schSystem;
	bool wasSaved = false; // set from outside

	FSchemeVector();

	FalconCalcScheme SchemeFor(Scheme sch);

	void ReadAndSetupSchemes();	// into menu items
	void Save();			// into "FalconCalc.fsty"
	void push_back(const FalconCalcScheme& scheme);
	int IndexOf(FalconCalcScheme& fgst);
	int IndexOf(const QString& title);
	Scheme PrepStyle(Scheme m);
};

extern FSchemeVector schemes;		// default styles: default, system, blue, dark, black

/* QGroupBox {
     background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                       stop: 0 #E0E0E0, stop: 1 #FFFFFF);
     border: 2px solid gray;
     border-radius: 5px;
     margin-top: 1ex; /* leave space at the top for the title * /
 }

 QGroupBox::title{
	 subcontrol - origin: margin;
	 subcontrol - position: top;
	 padding: 0 3px;
	*background - color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1,
									   stop : 0 #FF0ECE, stop: 1 #FFFFFF);
 }

	 QFrame{
		 border: 1px solid lightgray;
		 border - radius:5px;
 }

	 QLineEdit{
		 border: 1px solid gray;
		 border - radius:5px;
 }

	 QLabelo::framel{
		 border - radius:5px;
 }
	 */


#endif
