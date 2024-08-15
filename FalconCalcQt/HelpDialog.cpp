#include "HelpDialog.h"
#include <QScreen>
#include <QDesktopWidget>

static const QString sHelpText = QObject::tr(u8"Every Windows has a desktop calculator with many features but FalconCalc offers many unique features not found in them/"
    "FalconCalc continuously evaluates expressions as they are entered.Those may contain <i>built - in</i> and <i>user defined</i> constants<br>"
    "and functions of any number of arguments with many important digits <br>"
    "(currently 65, but can be set any number, if you compile your own exe).<br>"
    "<br>"
    "Results are displayed simultaneously as <i>decimal</i>, <i>hexadecimal</i>, <i>octal</i> and <i>binary</i> numbers and as a string of characters.<br>"
    "both in normal notation or one which can be put in an <i>HTML</i>, or a T<span style=\"vertical-align: -5px;\">E</span>X file<br>"
    "By pressing the corresponding button the result can be copied to the clipboard in any of theses formats.<br>"
    "<b>Arithmetic formulas</b> may contain the following operators:<br>"
    "<i>+, -, *, /, ^</i> (power), <i>| </i>(or <i>'or'</i>), & (or <i>'and'</i>), <i>xor</i>, <i> << </i>(or <i>'shl'</i> =shift left), <i> << </i>(or <i>'shr'</i> =shift right),<br>"
    " % </i>(or <i>mod</i> - remainder), <i>'~'</i> bit negation<br>"
    "<br>"
    "Logical operators:<br>"
    "<i> ==, != </i>(not equal), <i><, >, <=, >= </i> (these results in 1 or 0)<br>"
    "<br>"
    "<b>Built-in functions</b> (alternative names in parenthesis):<br>"
    "<br>"
    "<i>abs, arcsin(asin), arccos(acos), arctan(atan), cos, cosh(ch), coth(cth), exp, fact, frac, int,<br>"
    "log(ln), log2, log10(lg), round, sign, sin, sinh(sh), sqrt, tan(tg), trunc</i><br>"
    "<br>"
    "<b>Built in mathematical constants</b>:<br>"
    "<br>"
    "<i>e</i> (base of the natural logarithm), <i>log2e</i> (base 2 logarithm of e), <i>log10e</i> (or </i>lge</i> - base<br>"
    "10 logarithm of e), <i>ln2</i> (natural logarithm of 2)<br>"
    "<i>pi</i>(or &#960;), <i>pi2</i> (&#960;/2), <i>pi4</i> (&#960;/4), <i>ppi</i> (1/&#960;), <i>tpi</i> (2/&#8730;&#960;), <i>sqpi</i> (&#8730;&#960;), <br>"
    "<i>sqrt2</i> (&#8730;2), <i>psqrt2</i> (1/&#8730;2)), </i>sqrt3</i> (&#8730;3), <i>sqrt3P2</i> (&#8730;3/2)<br>"
    "<br>"
    "<b>Built-in physical constants</b>:<br>"
    "<br>"
    "au  - astronomical unit [m],<br>"
    "c - speed of light in vacuum (definition - exact value) [m/s], <br>"
    "h   - Planck constant  [Js],<br>"
    "hbar - reduced Planck constant [Js],<br>"
    "qe  - elementary charge [As],<br>"
    "me - electron mass [kg],<br>"
    "mp - proton mass [kg], <br>"
    "u   - atomic mass unit [1],<br>"
    "k - Boltzmann constant [J/K],<br>"
    "G  - Newtonian constant of gravitation [m^3/kg/s^2],<br>"
    "gf  - mean value of the gravitational acceleration on Earth (9.81 m/s²)<br>"
    "eps0- electric constant (vacuum permittivity) exact value [F /m]<br>"
    "mu0 - magnetic constant (vacuum permeability) exact value [N/A²] = 4&#960;·10<sup>-7</sup>, <br>"
    "kc  - Coulomb's constant 1/4&#960; eps0 [N m^2/C²],<br>"
    "LA  - Avogadro's number [1/mol],<br>"
    "rf  - Earth's radius [m],<br>"
    "rg  - molar gas constant (8.31 J/ mol K),<br>"
    "rs  - Sun's radius [m]<br>"
    "u   - atomic mass unit<br>"
    "<br>"
    "Any number of user constants and functions may be defined with any valid arithmetic formula including other constants and variables:<br>"
    "Line format:   name = expression<b>:</b>comment<b>:</b>unit, where name may be a variable name or a function name like '<i>a(x,y)</i>'<br>"
    "<br>"
    "<b>Examples</b>:<br>"
    "      constR = 8.31446261815324:Universal gas constant:J/K/mol<br>"
    "      solvequad(a, b, c, s) = (-b + s * sqrt(b ^ 2 - 4 * a * c)) / 2 / a:solves the quadratic equation ax² + bx + c = 0<br>"
    "<br>"
    "When a variable is modified the value of all dependent variables and functions are automatically changed<br>"
    "Functions may have any number of arguments with any names that is different from the name of any built-in function or constant.)<br>");

int HelpDialog::helpVisible = false;

HelpDialog::HelpDialog(QWidget* parent) :QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);
    ui.lblHelptext->setText(sHelpText);
    QScreen* screen = QGuiApplication::primaryScreen();
    move(screen->geometry().center() - geometry().center());
    ++helpVisible;
}
HelpDialog::~HelpDialog()
{
    if(helpVisible)
        --helpVisible;
}
