#include "common.h"
#include "translations.h"
#include "mainForm.h"

struct _LanguageTexts_private
{
   TextIDs id = TextIDs::FCT_ABOUT;
   wchar_t* texts[2];
};

struct _ErrorTexts_private
{
   EngineErrorCodes id = EngineErrorCodes::EEC_NO_ERROR;
   wchar_t* texts[2];
};

struct _Descriptions_private
{
   BuiltinDescId id = BuiltinDescId::DSC_descriptionForE;
   char16_t* texts[2];
};

static wchar_t* __helpTexts[2] = {
    // english
    L"Every Windows has a desktop calculator with many features but FalconCalc offers many unique features not found in them:\n\n"
                    L"FalconCalc continuously evaluates expressions as they are entered. Those may contain built-in and user defined constants\n"
                    L"and functions of any number of arguments with many significant digits (currently 65, but can be any number).\n\n"

                    L"Results are displayed simultaneously as decimal, hexadecimal, octal and binary numbers and as a string of characters.\n\n"
                    L"By pressing the corresponding button the result can be copied to the clipboard in any of these formats.\n\n"
                    L"Arithmetic formulas may contain the following operators:\n"
                    L"  +, -, *, /, ^(power), | (or 'or'), & (or 'and'), xor, << (or 'shl'=shift left), << (or 'shr'=shift right),\n"
                    L"  % (or mod - remainder), '~' bit negation\n\n"
                    L"Logical operators:\n"
                    L"  == (equal), != (not equal), <, >, <=, >= (these results in 1 or 0)\n\n"
                    L"Built-in functions (alternative names separated by a slash['/']):\n\n"
                    L"  abs(x), arcsin/asin(x), arccos/acos(x), arctan/atan(x), cos(x), cosh/ch(x), coth/cth(x), exp(x), fact(n), frac(x),\n"
                    L"  int(x), log/ln(x), log2(x), log10/lg(x), pow(base,power), root(n,x), root3(x), round(n,x), sign(x), sin(x), sinh/sh(x),\n"
                    L"  sqrt(x), tan/tg(x), trunc(x)\n\n"
                    L"Built in mathematical constants:\n"
                    L"  e (base of the natural logarithm), log2e (base 2 logarithm of e), log10e (or lge - base 10 logarithm of e),\n"
                    L"  ln2 (natural logarithm of 2)\n"
                    L"  pi/π, pi2 (=π/2), pi4 (=π/4), ppi (=1/π), tpi (=2/√π), sqpi (=√π), \n"
                    L"  sqrt2 (=√2), psqrt2 (=1/√2)), sqrt3 (=√3), sqrt3P2 (=√3/2)\n\n"
                    L"Built-in physical constants:\n"
                    L"  au  - astronomical unit [m], c - speed of light in vacuum (definition - definition) [m/s], \n"
                    L"  h    - Planck constant  [Js], hbar - reduced Planck constant [Js], \n"
                    L"  qe  - elementary charge [As], me - electron mass [kg], mp - proton mass [kg], \n"
                    L"  k    - Boltzmann constant [J/K],G  - Newtonian constant of gravitation [m^3/kg/s^2],\n"
                    L"  gf  - mean value of the gravitational acceleration on Earth (9.81 m/s²)\n"
                    L"  eps0- electric constant (vacuum permittivity) definition [F /m]\n"
                    L"  mu0 - magnetic constant (vacuum permeability) definition [N/A^2] = 4π·10^(-7), \n"
                    L"  kc  - Coulomb's constant 1/4π eps0 [N m^2/C^2], LA  - Avogadro's number [1/mol],\n"
                    L"  rf  - Earth's radius [m], rg  - molar gas constant (8.31 J/ mol K), rs - Sun's radius\n"
                    L"  u    - atomic mass unit [1]\n\n"
                    L"Any number of user constants and functions may be defined with any valid arithmetic formula including other constants and variables:\n"
                    L"When a variable is modified the value of all dependent variables and functions are automatically changed\n\n"
                    L"Functions may have any number of arguments with any names that is different from the name of any built-in function or constant.\n\n",
                    L"Line format:   name = expression:comment:unit, where name may be a variable name or a function name with parameters\n\n"
                    L"Examples:\n"
                    L"      constR=8.31446261815324:Universal gas constant:J/K/mol\n"
                    L"      solvequad(a, b, c, s) = (-b + s * sqrt(b ^ 2 - 4 * a * c)) / 2 / a:solves the quadratic equation ax² + bx + c = 0\n\n"
    // hungarian
    L"Minden Windowsban van egy saját számológép, ami azonban nem elég sokoldalú. A FaconCalc is egy számológép de sok különleges\ntulajdonsága van "
                    L"ami miatt érdemes használni.\n\nA FalconCalc folyamatosan dolgozza fel a belé írt kifejezéseket, amelyek mind beépített, mint felhasználó\n"
	                L"által definiált állandókat és akárhány változós függvényeket használhatnak, mindezeket tetszőleges (alapból 65 jegy) pontossággal.\n\n"
                    L"Az eredményt 5 féle formában egyidejűleg mutatja: 10-es, 16-os, 8-as és 2-es számrendszerben, illetve mint egy betűsorozatot.\n\n"
                    L"A megfelelő gomb megnyomásával ezeket ezekben a formátumokban a vágólapra másolhatjuk.\n\n"
                    L"Az aritmetikai képletek az alábbi műveleteket tartalmazhatják:\n\n"
                    L"  +, -, *, /, ^(kitevő kifejezés), | (vagy 'or'), & (vagy 'and'), xor, << (vagy 'shl'=shift left), << (vagy 'shr'=shift right),\n"
                    L"  % (vagy mod - maradék), '~' bitenkénti ellentét\n\n"
                    L"Logikai műveletek:\n"
                    L"  == (Egyenlő), != (nem egyenlő), <, >, <=, >= (ezek eredménye 1 vagy 0)\n\n"
                    L"Beépített függvények (a több nevűeket a perjel '/' választja el egymástól) kisbetű-nagybetű nem számít:\n\n"
                    L"  abs(x), arcsin/asin(x), arccos/acos(x), arctan/atan(x), cos(x), cosh/ch(x), coth/cth(x), exp(x), fact(n), frac(x),\n"
                    L"  int(x), log/ln(x), log2(x), log10/lg(x), pow(base,power), root(n,x), root3(x), round(n,x), sign(x), sin(x), sinh/sh(x),\n"
                    L"  sqrt(x), tan/tg(x), trunc(x)\n\n"
                    L"Beépített matematikai állandók ( kisbetű-nagybetú nem számít):\n"
                    L"  e (természetes logaritmus alapja), log2e (e 2 alapú logaritmusa), log10e/lge ( e 10-es alapú logaritmusa),\n"
                    L"  ln2 (2 természetes alapú logaritmusa)\n"
                    L"  pi/π, pi2 (= π/2), pi4 (=π/4), ppi (=1/π), tpi (=2/√π), sqpi (=√π), \n"
                    L"  sqrt2 (=√2), psqrt2 (=1/√2)), sqrt3 (=√3), sqrt3P2 (=√3/2)\n\n"
                    L"Beépített fizikai állandók (egységek []-ben  kisbetű-nagybetú nem számít):\n"
                    L"  au  - csillagászati egység [m], c - fénysebesség vákuumban (definició - pontos érték) [m/s], \n"
                    L"  h    - Planck állandó [Js], hbar - redukált Planck állandó [Js], \n"
                    L"  qe  - elemi töltés [As], me - elektrontömeg [kg], mp - proton tömeg [kg], \n"
                    L"  k    - Boltzmann állandó [J/K],G  - Newtoni gravitációs állandó [m^3/kg/s^2],\n"
                    L"  gf  - átlagos földi gravitációs gyorsulás (9.81 m/s²)\n"
                    L"  eps0- elektromos állandó ('vákum permittivitása' - pontos érték)  [F /m]\n"
                    L"  mu0 - mágneses állandó ('vákum permeabilitása' - pontos érték) [N/A^2] = 4π·10^(-7), \n"
                    L"  kc  - Coulomb állandó (=1/4π eps0) [N m^2/C^2], LA  - Avogadro szám [1/mol],\n"
                    L"  rf  - Föld sugara [m], RG  - moláris gázállandó (8.31 J/ mol K), Rs - a Nap sugara\n"
                    L"  u    - atomi tömegegység [1]\n\n"
                    L"Bárhány saját állandót és függvényt definiálhatunk a beépített és addig definiáltak használatával (ha értelmezhető)\n."
                    L"Ha egy állandó, vagy függvény átdefiniálunk a tőle függgő változók és függvények is módosulnak.\n\n"
                    L"A függvényeknek akárhány paraméterük lehet, de a nevüknek különböznie kell bármely beépített változó, vagy függvény nevétől.\n"

                    L"Változo/függvény definíció formátuma:\n"
                    L"Változó: név = kifejezés:megjegyzés:egység. Függvény:név(x,...) = kifejezés:megjegyzés:egység.\n\n"
                    L"Példák:\n"
                    L"      constR=8.31446261815324:Univerzális gáz állandó:J/K/mol\n"
                    L"      másod(a, b, c, s) = (-b + s * sqrt(b ^ 2 - 4 * a * c)) / 2 / az ax² + bx + c = 0 másodfokú egyenlet valós megoldásai\n\n"
};

static _LanguageTexts_private __texts[] = {
 { FCT_OK,              L"O&k", L"&Rendben"},
 { FCT_SAVE ,		    L"&Save", L"&Mentés" },
 { FCT_CANCEL,          L"Cance&l", L"E&lvet"},
 { FCT_CLOSE,           L"C&lose", L"Be&zárás"},
 { FCT_ABOUT,		    L"&About",							L"&Névjegy" },
 { FCT_ANGLEF,			L"Angles in",						L"Szögek formátuma" },
 { FCT_APPEND,			L"Paste &After expression",			L"Beillessztés a &végére" },
 { FCT_BIN,				L"&Binary",							L"&Bináris" },
 { FCT_BYTES,			L"As B&ytes",						L"Bájtként" },
 { FCT_CHARFONT,		L"&Font For 'As String...' Display...",
                        L"&Betű a 'Szöveges megjelenítéshez'..." },
 { FCT_CLEARHIST, 		L"C&lear History",					L"Előzmények &törlése" },
 { FCT_COPYBIN,			L"Copy &Binary",					L"&Bináris érték vágólapra" },
 { FCT_COPYDEC,			L"Copy &Decimal",					L"&Decimális érték vágólapra"},
 { FCT_COPYEXPR,		L"&Copy expression",				L"&Kifejezés vágólapra" },
 { FCT_COPYHEX,			L"Copy He&xadecimal",				L"He&xadecimális érték vágólapra" },
 { FCT_COPYOCT,			L"Copy &Octal",						L"&Oktális érték vágólapra" },
 { FCT_COPYTEXT,		L"Copy &Formula",					L"&Képlet vágólapra" },
 { FCT_DATA,			L"&User data",						L"Sa&ját adat" },
 { FCT_DDIGITS,			L"&Decimal digits:",				L"&Tizedesjegyek:" },
 { FCT_DEC,				L"Deci&mal",						L"Deci&mális" },
 { FCT_DECOPTS,			L"Decimal options",					L"Decimalális opciók" },
 { FCT_DECOPTSOPENER,	L"──────Decimal options────────",
                        L"──────Decimális opciók───────" },
 { FCT_DECSEP,			L"&Use decimal separator",			L"Tizedes elválasztó" },
 { FCT_DEG,					L"De&g",							L"&Fok" },
 { FCT_DISPLAS,			L"Display as",						L"Mutasd mint" },
 { FCT_DOUBLE,			L"As IEE 7&54 double",				L"IEE 7&54 dupla" },
 { FCT_DWORDS,			L"As DWo&rds",						L"&Duplaszóként" },
 { FCT_EDIT,			L"&Edit",							L"&Szerkesztés" },
 { FCT_EDITCOPY,		L"Cop&y ...",						L"&Másolás ..." },
 { FCT_EDITFUNCS, 		L"User &Functions...",  			L"Saját &Függvények..." },
 { FCT_EDITVARS,		L"User &Variables...",  			L"Saját &változók..." },
 { FCT_VARSANDFUNCTIONS,L"Variables and Functions",         L"Változók és függvények"},
 { FCT_ENG,				L"Engeneering f&ormat",				L"Mérnöki f&ormátum" },
 { FCT_EXIT,			L"E&xit",							L"&Kilépés" },
 { FCT_FILE,			L"&File",							L"&Fájl" },
 { FCT_FONT,			L"&Font...",						L"&Betű..." },
 { FCT_GENHELP,			L"&General Help",					L"Á&ltalános súgó" },
 { FCT_GRAD,			L"&Grad",							L"&Grad" },
 { FCT_HELP,			L"&Help",							L"S&úgó" },
 { FCT_HEX,				L"He&xadecimal",					L"He&xadec." },
 { FCT_HEXMIN,			L"Mi&nus sign",						L"E&lőjel" },
 { FCT_HEXOPTS,			L"Hexadecimal Options",				L"Hexadecimális Opciók" },
 { FCT_HEXOPTSOPENER,	L"─────Hexadecimal options──────",
                        L"─────Hexadecimális opciók─────" },
 { FCT_HEXPREFIX,		L"0x p&refix",						L"0&x elöl" },
 { FCT_HISTOPTS,		L"Histor&y Options...",				L"Előzmé&ny beállítások..." },
 // { FCT_HISTORY,		    L"H&istory",						L"E&lőzmények" },
  { FCT_HTML,			L"HT&ML",							L"HT&ML" },
  { FCT_LANGUAGE,		L"&Language",					    L"&Nyelv"},
  { FCT_LITTLEE,			L"&Little endian",					L"&fordított sorrend" },
  { FCT_LOCALE,			L"Set Locale...",					L"Nyelvi környezet..." },
  { FCT_NORMAL,			L"Norm&al",							L"Normál" },
  { FCT_OCT,				L"O&ctal",							L"O&ctális" },
  { FCT_OPTIONS,			L"&Options",						L"&Lehetőségek" },
  { FCT_PASTEEXPR,		L"&Paste expression",				L"Kifejezés &beillesztése" },
  { FCT_RAD,				L"&Rad",							L"&Rad" },
  { FCT_RESULT,			L"Result:",							L"Eredmény:" },
  { FCT_SCI,				L"&Scientific format",				L"&Tudományos formátum" },
  { FCT_SETEN,		    L"&English",					    L"&Angol"},
  { FCT_SETHUN,		     L"&Hungarian",					    L"&Magyar"},
  { FCT_SHOWDECOPTS,		L"Show &Decimal Options",			L"&Decimálos opciók" },
  { FCT_SHOWHEXOPTS,		L"Show Hexadecimal  Options",		L"&Hexadecimális Opciók" },
  { FCT_SHOWHIST,		L"Edit &History",					L"Előzmények &szerk." },
  { FCT_SINGLE,			L"As IEE &754 single",				L"IEE &754" },
  { FCT_STRING,			L"As String:",						L"Szöveg:" },
  { FCT_TEX,				L"&TeX",							L"&TeX" },
  { FCT_THOUSAND,		L"Use thousand se&parator:",		L"Ezres el&választó:" },
  { FCT_TIPBIN  ,        L"Copy binary number to clipboard", L"Bináris szám másolása" },
  { FCT_TIPDE   ,        L"10's exponent displayed as 'Ex'", L"Kitevő 'E' formátumban" },
  { FCT_TIPDEC  ,        L"Copy decimal number to clipboard",
                         L"Decimális szám másolása" },
  { FCT_TIPDEG  ,        L"Degrees",
                         L"Fok (°)" },
  { FCT_TIPDHTML,        L"'a.aaExx' displayed as a.aa×10<sup>xx</sup>",
                         L"'a.aaExx' megjelenítés: a.aa×<sup>xx</sup>" },
  { FCT_TIPDP   ,        L"Double precision floating point format",
                         L"Dupla pontos lebegőpontos számformátum" },
  { FCT_TIPDPOV ,        L"'a.aExx' displayed with 10's power",
                         L"'a.aExx' az xx 10-es kitevőt felső indexben mutatja" },
  { FCT_TIPDTEX ,        L"'a.aExx' displayed as TeX expression a.a×10^{xx}",
                         L"'a.aExx' TeX formátumban: 10^{xx} mutatja" },
  { FCT_TIPGRAD ,        L"360° = 400 grad",
                         L"360° = 400 grad" },
  { FCT_TIPHEX  ,        L"Copy hexadec. number to clipboard",
                         L"Hexadecimális szám másolása" },
  { FCT_TIPINT  ,        L"Left to right order of bytes from least significant to most significant (Intel ordering)",
                         L"A legkisebb helyiértékű számjegyek baloldalon (Intel féle)" },
  { FCT_TIPNEGH ,        L"For HEX, OCT, and BIN: Absolute value of negative numbers are shown with a minus sign",
                         L"Hexadec. oktális és bináris negatív számok abszolút értékének mutatása előjellel" },
  { FCT_TIPOCT  ,        L"Copy octal number to clipboard",
                         L"Oktális szám vágólapra" },
  { FCT_TIPRAD  ,        L"Radians",                         L"Radián" },
  { FCT_TIPSP   ,        L"Single precision floating point format",
                         L"Szimpla pontos lebegőpontos számformátum" },
  { FCT_TIPSPC  ,        L"spc",                             L"betűköz" },
  { FCT_TIPTURN ,        L"1 turn = 360°",                   L"1 Fordulat = 360°" },
  { FCT_TURN,			L"&Turns",							L"&Ford." },
  { FCT_VIEW,			L"&View",							L"&Nézet" },
  { FCT_WORDS,	    	L"As &Words",						L"&Szóként" },

  { FCT_VARIABLES,		L"&Variables", L"&Változók" },
  { FCT_FUNCTIONS,       L"&Functions", L"&Függvények" },
  { FCT_APPENDVF,		L"Append Variable/function",	L"Változó vagy függvény hozzáadása" },
  { FCT_REMOVEVF,		L"Remove actual line", L"Aktuális sor törlése" },
  { FCT_REMOVEALLVF,	    L"Remove all user Variables or Functions", L"Minden változó vagy függvény törlése" },


  { FCT_VNDIALOGTITLE,	L"FalconCalc - ", L"FalconCalc - "},    // append FCT_VNDVAR or FCT_VNDFUNC
  { FCT_VNDVARTITLE,	    L"Edit Variable", L"Változó szerk."},
  { FCT_VNDFUNCTITLE,	L"Edit Function", L"Függvény szerk."},
  { FCT_VNDNAME,		    L"&Name:",          L"&Név:"                         },
  { FCT_VNDVALUE,		L"&Definition:",    L"&Definíció:"             },
  { FCT_VNDUNIT,		    L"&Unit:",          L"&Mértékegység:"                },
  { FCT_VNDCOMMENT,		L"&Comment:",       L"&Megjegyzés:"               },

  { FCT_HELPTITLE,		L"FalconCalc Help", L"FalconCalc Súgó"},
  { FCT_ABOUTFC,			L"About FalconCalc", L"Névjegy" },
  { FCT_ABOUTLINE1,		L"A handy little calculator for everyone\n\n",                         L"Egy ügyes kis számológép, mindenkinek\n\n" },
  { FCT_ABOUTLINE2,      L"(especially useful for physicists and programmers)",                         L"(különösen műszakiaknak és programozóknak)"},
  { FCT_ABOUTLINE3,      L"Based on the 'LongNumber' cross platform",                                   L"Alapja a 'LongNumber', multiplatform"},
  { FCT_ABOUTLINE4,      L"arithmetic library of A. Sólyom",                                            L"aritmetikai könyvtár © Sólyom. A."},
  { FCT_ABOUTLINE5,      L"and", L"és"},
  { FCT_ABOUTLINE6,      L"the NLIB library © Zoltán. Sólyom",                                          L"Zoltán Sólyom NLIB © könyvtára"},
  { FCT_ABOUTLINE7,      L"This program (except NLIB) is open source (GPL3)",                           L"Ez a program az NLIB-et kivéve  Copyright (GPL3)"},
  { FCT_LOCALETITLE,		L"FalconCalc Locale", L"FalconCalc Nyelvi környezet"},
  { FCT_LOCALE1,			L"The locale determines the character used for a decimal point, the ordering of the characters"
                         L" and the correspondance bewtween upper and lowercase letters. Variable and function names may"
                         L" contain accented or special letters in your locale that are not present in other locales and vice-versa.",
                         L"A nyelvi környezet határozza meg a tizedes jel karaktert )pont, vagy vessző), a karakterek sorrendjét és a kis- és nagybetűk"
                         L" megfeleltetését. A változó és függvénynevek tartalmazhatnak ékezetes vagy speciális karaktereket az adott "
                         L"nyelvi környezetben, amelyek más nyelvi környezetben nem találhatók meg."},
  { FCT_LOCALE2,			L"Locale names usually consist of 2, 3 or 5-8 (usually) lowercase characters which may be followed by a hyphen and one or more"
                         L"subtags each separated by a hyphen character (for example: en-US, en-GB, hu-HU). The first part (two lowercase letters) defines"
                         L" the language, the subtags regions and others like zh-yue-HK (same as yue-HK) for Cantonese for Hong Kong SAR. Some implementations"
                         L" may use underscores instead of hyphens (en_US). If you set a locale that is not installed on your"
                         L" system FalconCalc will try to use a similar one (for example: if you set en_GB but only en_US is installed it will use en_US).",
    // WARNING: if I further split this Hungarian string into parts, the compiler gives a warning and the runtime 
    // will show ? characters instead of ő and ű!
    // Maybe it wasnt just me leaving the L out in front of the lines<
                       L"A nyelvi környezet neve 2, 3 vagy 5-8 (általában) kisbetűből all - ez határozza meg a nyelvet-, amit kötőjel és ugyancsak kötőjellel elválasztott betűcsoportok követnek."
                       L"Pl. hu-HU, en-US, de-DE, zh-yue-HK (ez. ugyan az mint a yue-HK). Kötőjel helyett sokszor a nem szabványos aláhúzás szerepel. "
                       L"Speciális név a 'C' környezet, ami az en-US -hez hasonló. A nyelvi környezet és a program nyelve nem kell azonos legyen"},
  { FCT_CURNAMELABEL,    L"Name of current locale:", L"A jelenlegi nyelvi környezet:" },
  { FCT_NEWNAMELABEL,    L"Name of new locale:",     L"Az új nyelvi környezet:" },
// history options
  { FCT_HISTOPTS_TITLE,			L"FalconCalc - History options", L"FalconCalc - Előzmények beállításai" },
  { FCT_MAXHISTDEPTH,			L"Maximum history &depth:",L"Maximum megjegyzettek száma:"  },
  { FCT_MAXHISTDEPTHTIP,		    L"Set the maximum number of expressions kept in the history", L"Ennyi előzmény kifejezést jegyez meg"   },
  { FCT_AUTOSAVE,				L"Sa&ve after this interval:", L"Automatikus mentés ideje:" },
  { FCT_AUTOSAVETIP,			    L"When unchecked, no history will be saved", L"Ha nincs bejelölve az előzményeket nem jegyzi meg"   },
  { FCT_HHMMSS,					L"(HH:MM:SS)",L"(ÓÓ:PP:MP)" },
  { FCT_HISTMINLENGTHLABEL,		L"&Min. length to save:",L"&Megjegyzendő minimális hossz:"   },
  { FCT_HISTMINLENGTHTIP,		L"Expressions shorter than this will not be remembered", L"Csak ennél hosszabb kifejezéseket jegyez meg"   },
  { FCT_HISTMINLENGTHSAVEALL,	L"(0: all)",	L"(0: bármi)"  },
  { FCT_HISTSORTED,				L"Sorted",	L"Sorbarendezés"    },
// Helps
  { FCT_HISTAUTOSAVELABEL,		L"Automatically save history",L"Automatikusan mentse az előzményeket"   },
  { FCT_HISTCLEAR,               L"&Discard history", L"Elő&zmények eldobása" },
};

static _Descriptions_private __descriptions[] = {
    // descriptions of constants
    // mathematical constants
 { DSC_descriptionForE,	         u"Euler's number"								,   u"Euler szám"},
 { DSC_descriptionForPi,	     u"π - half the circumference of a unit circle" ,   u"π - az egységkör kerületének fele"},
 { DSC_descriptionForRpi,	     u"1/π"                                         ,   u"1/π"},
 { DSC_descriptionForTwoPi,	     u"2π"											,   u"2π"},
 { DSC_descriptionForPiP2,	     u"π/2"											,   u"π/2"},
 { DSC_descriptionForPiP4,	     u"π/4"                                         ,   u"π/4"},
 { DSC_descriptionForRpi2,	     u"2/π"                                         ,   u"2/π"},
 { DSC_descriptionForSqpi,	     u"√π"                                          ,   u"√π"},
 { DSC_descriptionForSqrt2,	     u"√2"											,   u"√2"},
 { DSC_descriptionForRsqrt2,	 u"1/√2"									    ,   u"1/√2"},
 { DSC_descriptionForSqrt3,	     u"√3"											,   u"√3"},
 { DSC_descriptionForSqrt3P2,	 u"√3/2"                                        ,   u"√3/2"},
 { DSC_descriptionForLn10,	     u"natural logarithm of 10"                     ,   u"ln(10)"},
 { DSC_descriptionForLn2,	     u"natural logarithm of 2 "                     ,   u"ln(2)"},
 { DSC_descriptionForRln10,	     u"1/ln(10)"									,   u"1/ln(10)"},	
 { DSC_descriptionForRln2,	     u"1/ln(2)"										,   u"1/ln(2)"},	
 { DSC_descriptionForLog2e,	     u"base 2 logarithm of e"                       ,   u"e 2-es alapú logaritmusa"},
 { DSC_descriptionForLg10e,	     u"base 10 logarithm of e"                      ,   u"lg(e)"},
 { DSC_descriptionForLge,	     u"base 10 logarithm of e"                      ,   u"lg(e)"},
 // physics 					
 { DSC_descriptionForA0,	     u"Bohr radius"									,   u"Bohr sugár"},	
 { DSC_descriptionForFsc,	     u"fine-structure constant (about 1/137)"		,   u"finomszerkezeti állandó (kb 1/137)"},	
 { DSC_descriptionForAu,	     u"astronomical unit (definition)"				,   u"csillagászati egység (definíció)"},	
 { DSC_descriptionForC,	         u"speed of light in vacuum (definition)"		,   u"fénysebesség vákuumban (definíció)"},		
 { DSC_descriptionForEps0,	     u"ε₀, vacuum electric permittivity (definition)",  u"ε₀ - vákuum permittivitás  (definíció)"},
 { DSC_descriptionForG,	         u"Newtonian constant of gravitation"			,   u"Newton-féle gravitációs állandó"},	
 { DSC_descriptionForGf,	     u"average g on Earth"							,   u"g átlagos értéke a Földön"},		
 { DSC_descriptionForH,	         u"Planck constant"								,   u"Planck állandó"},	
 { DSC_descriptionForHbar,	     u"reduced Planck constant (h/2π)"				,   u"redukált Plank állandó (h/2π)"},		
 { DSC_descriptionForKb,	     u"Boltzmann constant"							,   u"Boltzmann állandó"},		
 { DSC_descriptionForKc,	     u"= 1/4πε₀ Coulomb constant"					,   u"= 1/4πε₀ Coulomb konstans"},	
 { DSC_descriptionForLa,	     u"Avogadro constant (definition)"				,   u"Avogadro-szám (definíció)"},	
 { DSC_descriptionForMe,	     u"electron mass"								,   u"elektrontömeg"},	
 { DSC_descriptionForMf,	     u"mass of the Earth"							,   u"a Föld tömege"},	
 { DSC_descriptionForMp,	     u"proton mass"									,   u"protontömeg"},	
 { DSC_descriptionForMs,	     u"mass of the Sun"								,   u"a Nap tömege"},	
 { DSC_descriptionForMu0,	     u"4π*10ˉ⁷ vacuum magnetic permeability"		,   u"4π*10ˉ⁷ a \"vákuum permeabilitása\""},	
 { DSC_descriptionForQe,	     u"elementary charge"							,   u"elemi töltés"},	
 { DSC_descriptionForRfsc,	     u"reciprocal of the fine structure constant (approx 137)",
		                         u"a finomszerkezeti állandó reciproka (kb. 137)"},  
 { DSC_descriptionForRf,	     u"radius of the Earth"                         ,   u"a Föld sugara"},
 { DSC_descriptionForRg,	     u"molar gas constant R"                        ,   u"egyetemes gázállandó"},
 { DSC_descriptionForRs,	     u"radius of the Sun"                           ,   u"a Nap sugara"},
 { DSC_descriptionForSb,	     u"Stefan–Boltzmann constant"                   ,   u"Stefan-Boltzmann állandó"},
 { DSC_descriptionForU,	         u"atomic mass unit (=(mass of C12)/12)"        ,   u"atomi tömegegység (a C12 tömegének 12-ed része)"}
};

static _ErrorTexts_private __errorTexts[] = {
 // error messages
 { EEC_NO_ERROR,                                L"Result:"                      , L"Eredmény:" },
 { EEC_VARIABLE_DEFINITION_MISSING,             L"Variable definition missing"  , L"A változó definíciója hiányzik" },
 { EEC_UNKNOWN_FUNCTION_IN_EXPRESSION,	        L"Unknown function in expression",L"Ismeretlen függvény a képletben"},
 { EEC_SYNTAX_ERROR,					        L"Syntax error"                 , L"Hibás szintaxis"},
 { EEC_STACK_ERROR,					            L"Stack error"                  , L"Stack hiba"},
 { EEC_OVERFLOW_ERROR,					        L"Overflow error"               , L"Túlcsordulási hiba"},
 { EEC_DOMAIN_ERROR,					        L"Domain error"                 , L"Ért. tartomány hiba"},
 { EEC_MISSING_BRACE,					        L"Missing closing '}'"          , L"Záró '}' hiányzik"},
 { EEC_LARGE_FOR64,					            L"Number can't fit in a 64 bit integer", L"A szám hosszabb, mint 64 bit"},
 { EEC_RECURSIVE_FUNCTIONS_ARE_NOT_ALLOWED,		L"Recursive functions are not allowed",	L"Rekurzív függvény. Nem megengedett."},
 { EEC_NO_FUNCTION_ARGUMENT,					L"No function argument"			, L"Nincsenek függvényparaméterek"},
 { EEC_MISSING_BINARY_NUMBER,					L"Missing binary number"        , L"Hiányzó bináris szám"},
 { EEC_MISMATCHED_PARENTHESIS,					L"Mismatched parenthesis"       , L"Zárójelezési hiba"},
 { EEC_INVALID_FUNCTION_DEFINITION,				L"Invalid function definition"  ,
                                                L"Hibás függvény definíció"},
 { EEC_INVALID_CHARACTER_IN_FUNCTION_DEFINITION,L"Invalid character in function definition",
                                                L"Hibás karakter a függvény definícióban"},
 { EEC_ILLEGAL_OPERATOR_AT_LINE_END,			L"Illegal operator at end of expression",
                                                L"M'veleti jel a kifejezés végén"},
 { EEC_ILLEGAL_OCTAL_NUMBER,					L"Illegal octal number",			L"Nem oktális szám"},
 { EEC_ILLEGAL_NUMBER_No2,					    L"Illegal number #2",				L"Nem szám #2"},
 { EEC_ILLEGAL_NUMBER_No1,					    L"Illegal number #1",				L"Nem szám #1"},
 { EEC_ILLEGAL_HEXADECIMAL_NUMBER,				L"Illegal hexadecimal number",		L"Nem hexadecimális szám"},
 { EEC_ILLEGAL_CHARACTER_NUMBER,				L"Illegal character number",		L"Hibás betűk"},
 { EEC_ILLEGAL_BINARY_NUMBER,					L"Illegal binary number",           L"Nem bináris szám"},
 { EEC_ILLEGAL_AT_LINE_END,					    L"Illegal at line end",				L"Sor végén nem megengedett"},
 { EEC_FUNCTION_MISSING_OPENING_BRACE,			L"Function missing opening brace",	L"A függvény definíció nyitó zárójele hiányzik"},
 { EEC_FUNCTION_DEFINITION_MISSING_RIGHT_BRACE,	L"Function definition missing right brace", L"A függvény definíció záró zárójele hiányzik"},
 { EEC_EXPRESSION_ERROR,					    L"Expression error",				L"Hiba a kifejezésben"},
 { EEC_SEPARATOR_DECPOINT_BRACE_MISMATCH,		L"Separator or decimal character or parenthesis mismatch",				                                                
                                                L"Rossz tizedesjel, elválasztó karakter, vagy zárójel"},
 { EEC_DIVISON_BY_0,					        L"Divison by 0",                    L"0-val osztás"},
 { EEC_CLOSING_QUOTE_NOT_FOUND,					L"Closing quote not found",         L"Záró idézőjel hiányzik"},
 { EEC_BUILTIN_VARIABLES_CANNOT_BE_REDEFINED,	L"Builtin variables cannot be redefined", L"Beépített változókat nem lehet átdefiniálni"},
 { EEC_BUILTIN_FUNCTIONS_CANNOT_BE_REDEFINED,	L"Builtin functions cannot be redefined", L"Beépített függvényeket nem lehet átdefiniálni"},
};

wchar_t* LanguageTexts::GetHelpText() const
{
    return __helpTexts[ (int)_lang - 1];
}

wchar_t* LanguageTexts::GetTranslationFor(TextIDs id) const
{
    int nLang = (int)_lang;
    if (nLang == 0)
        nLang = 0;
    else
        --nLang;

    if(!((int)id >= 0 && (int)id < sizeof(__texts)/sizeof(__texts[0])))
		return L"";
	if (__texts[(int)id].id == id)
        return __texts[(int)id].texts[nLang];

    // search for the id
    for (size_t i = 0; i < sizeof(__texts) / sizeof(__texts[0]); i++)
    {
        if (__texts[i].id == id)
            return __texts[i].texts[nLang];
    }
    return L"";
}

wchar_t* LanguageTexts::GetTranslationFor(EngineErrorCodes id) const
{
    int nLang = (int)_lang;
    if (nLang == 0)
        nLang = 0;
    else 
        --nLang;
    if (!((int)id >= 0 && (int)id < sizeof(__errorTexts) / sizeof(__errorTexts[0])))
        return L"";
    if (__errorTexts[(int)id].id == id)
        return __errorTexts[(int)id].texts[nLang];

    // search for the id
    for (size_t i = 0; i < sizeof(__errorTexts) / sizeof(__errorTexts[0]); i++)
    {
        if (__errorTexts[i].id == id)
            return __errorTexts[i].texts[nLang];
    }
    return L"";
}

char16_t* LanguageTexts::GetTranslationFor(BuiltinDescId id) const
{
    int nLang = (int)_lang;
    if (nLang == 0)
        nLang = 0;
    else
        --nLang;
    if (!((int)id >= 0 && (int)id < sizeof(__descriptions) / sizeof(__descriptions[0])))
        return u"";
    if (__descriptions[(int)id].id == id)
        return __descriptions[(int)id].texts[nLang];
    // search for the id
    for (size_t i = 0; i < sizeof(__descriptions) / sizeof(__descriptions[0]); i++)
    {
        if (__descriptions[i].id == id)
            return __descriptions[i].texts[nLang];
    }
    return u"";
}

LanguageTexts::LanguageTexts()
{
}

bool LanguageTexts::SetLanguage(AppLanguage lang)
{
    if (_lang == lang || (lang != AppLanguage::lanEng && lang != AppLanguage::lanHun) )
        return false;

    _lang = lang;
    return true;    // must set all texts
}

LanguageTexts lt;