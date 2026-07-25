#include "FCSettings.h"
#include <QMessageBox>

//----------------------------- FCSettings -------------------
/*=============================================================
 * TASK:    centralized settings handler
 *      static members are initialized in the main form
 *------------------------------------------------------------*/
QSettings* FCSettings::_ps = nullptr;
QString FCSettings::homePath;
QString FCSettings::_name = "FalconCalcQt.dat";   // default ini file
bool FCSettings::_initted = false;

void FCSettings::Init()             // MUST be called first, set home path for the user data
{                                   // if the home path didn't exist create it
    homePath = QDir::homePath() +
#if defined (Q_OS_Linux)   || defined (Q_OS_Darwin) || defined(__linux__)
        "/.falconCalc/";
#elif defined(Q_OS_WIN)
        "/Appdata/Local/FalconCalc/";
#endif
    QDir qd(homePath);
    if (!qd.exists())
    {
        if(qd.mkdir(homePath)) // if can't create it isn't initted
            _initted = true;
        else
            QMessageBox::warning(nullptr, QObject::tr("FalconCalc - Warning"),
                                 QObject::tr("Can't create folder\n'%1'\nCalculater state and history will not be preserved").arg(homePath));
    }
}

QString FCSettings::Name()
{
    return homePath + _name;
}

QSettings* FCSettings::Open()
{
    _ps = new QSettings(Name(), QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    _ps->setIniCodec("UTF-8");
#endif
    return _ps;
}

