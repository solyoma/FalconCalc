#include "FCSettings.h"

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

