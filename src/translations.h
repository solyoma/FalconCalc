#pragma once
// multi language support for calculator
// currently only english and hungarian

#include "EngineErrors.h"
#include "defines.h"

enum TextIDs {
	FCT_OK,              // L"O&k", L"&Rendben"
	FCT_SAVE,
	FCT_CANCEL,
	FCT_CLOSE,			// L"Close", L"Bezárás"
	FCT_ABOUT,
	FCT_ANGLEF,
	FCT_APPEND,
	FCT_BIN,
	FCT_BYTES,
	FCT_CHARFONT,
	FCT_CLEARHIST,
	FCT_COPYBIN,
	FCT_COPYDEC,
	FCT_COPYEXPR,
	FCT_COPYHEX,
	FCT_COPYOCT,
	FCT_COPYTEXT,
	FCT_DATA,
	FCT_DDIGITS,
	FCT_DEC,
	FCT_DECOPTS,
	FCT_DECOPTSOPENER,
	FCT_DECSEP,
	FCT_DEG,
	FCT_DISPLAS,
	FCT_DOUBLE,
	FCT_DWORDS,
	FCT_EDIT,
	FCT_EDITCOPY,
	FCT_EDITFUNCS,
	FCT_EDITVARS,
	FCT_VARSANDFUNCTIONS,
	FCT_ENG,
	FCT_EXIT,
	FCT_FILE,
	FCT_FONT,
	FCT_GENHELP,
	FCT_GRAD,
	FCT_HELP,
	FCT_HEX,
	FCT_HEXMIN,
	FCT_HEXOPTS,
	FCT_HEXOPTSOPENER,
	FCT_HEXPREFIX,
	FCT_HISTOPTS,
	//FCT_HISTORY			,
	FCT_HTML,
	FCT_LANGUAGE,
	FCT_LITTLEE,
	FCT_LOCALE,
	FCT_NORMAL,
	FCT_OCT,
	FCT_OPTIONS,
	FCT_PASTEEXPR,
	FCT_RAD,
	FCT_RESULT,
	FCT_SCI,
	FCT_SETEN,
	FCT_SETHUN,
	FCT_SHOWDECOPTS,
	FCT_SHOWHEXOPTS,
	FCT_SHOWHIST,
	FCT_SINGLE,
	FCT_STRING,
	FCT_TEX,
	FCT_THOUSAND,
	FCT_TIPBIN,
	FCT_TIPDE,
	FCT_TIPDEC,
	FCT_TIPDEG,
	FCT_TIPDHTML,
	FCT_TIPDP,
	FCT_TIPDPOV,
	FCT_TIPDTEX,
	FCT_TIPGRAD,
	FCT_TIPHEX,
	FCT_TIPINT,
	FCT_TIPNEGH,
	FCT_TIPOCT,
	FCT_TIPRAD,
	FCT_TIPSP,
	FCT_TIPSPC,
	FCT_TIPTURN,
	FCT_TURN,
	FCT_VIEW,
	FCT_WORDS,

	FCT_VARIABLES,		// L"&Variables", L"&Változók"
	FCT_FUNCTIONS,		// L"&Functions", L"&Függvények"
	FCT_APPENDVF,		// L"Append Variable/function",	L"Változó vagy függvény hozzáadása"
	FCT_REMOVEVF,		// L"Remove actual line", L"Aktuális sor törlése"
	FCT_REMOVEALLVF,	// L"Remove all user Variables or Functions", L"Minden változó v. függvény törlése"

	FCT_VNDIALOGTITLE,	// L"FalconCalc - ", L"FalconCalc - ",
	FCT_VNDVARTITLE,	// L"Edit Variable", L"Változó szerk."
	FCT_VNDFUNCTITLE,	// L"Edit Function", L"Függvény szerk."
	FCT_VNDNAME,		// L"&Name:", L"&Név:"
	FCT_VNDVALUE,		// L"&Definition:", L"&Definíció:"
	FCT_VNDUNIT,		// L"&Unit:", L"&Mértékegység:"
	FCT_VNDCOMMENT,		// L"&Comment:", L"&Megjegyzés:"

	FCT_HELPTITLE,		// L"FalconCalc Help"

	FCT_ABOUTFC,			// L"About FalconCalc"
	FCT_ABOUTLINE1,
	FCT_ABOUTLINE2,
	FCT_ABOUTLINE3,
	FCT_ABOUTLINE4,
	FCT_ABOUTLINE5,
	FCT_ABOUTLINE6,
	FCT_ABOUTLINE7,

	// locale
	FCT_LOCALETITLE,	// L"FalconCalc Locale"
	FCT_LOCALE1,		// L"The locale determines the character used for a decimal point, the ordering of the characters"
	// " and the correspondance bewtween upper and lowercase letters. Variable and function names may"
	// " contain accented or special letters in your locale that are not present in other locales and vice-versa."
	FCT_LOCALE2,		// L"Locale names usually consist of two lowercase and two uppercase letters separated..."
	FCT_CURNAMELABEL,	// L"Name of current locale:"
	FCT_NEWNAMELABEL,	// L"Name of new locale:"
	// history options
	FCT_HISTOPTS_TITLE,			// L"FalconCalc - History options", L"FalconCalc - Előzmények beállításai"
	FCT_MAXHISTDEPTH,			// L"Maximum history &depth:",L"Maximum megjegyzettek száma:"
	FCT_MAXHISTDEPTHTIP,		// L"Set the maximum number of expressions kept in the history", L"Ennyi előzmény kifejezést jegyez meg"
	FCT_AUTOSAVE,				// L"Sa&ve after this interval:", L"Automatikus mentés ideje:"
	FCT_AUTOSAVETIP,			// L"When unchecked, no history will be saved", L"Ha nincs bejelölve az előzményeket nem jegyzi meg"
	FCT_HHMMSS,					// L"(HH:MM:SS)",L"(ÓÓ:PP:MP)"
	FCT_HISTMINLENGTHLABEL,		// L"&Min. length to save:",L"&Min. kif. hossz:"
	FCT_HISTMINLENGTHTIP,		// 	L"Expressions shorter than this will not be remembered", L"Csak ennél hosszabb kifejezéseket jegyez meg";
	FCT_HISTMINLENGTHSAVEALL,	// L"(0: all)",	L"(0: mindet)"
	FCT_HISTSORTED,				// L"Sorted",	L"Sorbarendezés"
	FCT_HISTAUTOSAVELABEL,		// L"Automatically save history",L"Automatikusan mentse az előzményeket"
	FCT_HISTCLEAR,				// L"&Discard history", L"Elő&zmények eldobása"

	FCT_LAST
};

class LanguageTexts
{
public:
	LanguageTexts();
	constexpr AppLanguage GetLanguage() const { return _lang; }
	bool SetLanguage(AppLanguage lang);		// true: must set menus and help text in the selected language
											// false: already OK
	wchar_t* GetHelpText() const;		// returns the help text in the current language
	   // these return the text in the current language
	wchar_t* GetTranslationFor(TextIDs id) const; 
	wchar_t* GetTranslationFor(EngineErrorCodes id) const;
	char16_t* GetTranslationFor(BuiltinDescId id) const;
private:
	AppLanguage _lang= AppLanguage::lanNotSet;
};

extern LanguageTexts lt;






