#pragma once
#ifndef _FCSETTINGS_H
#define _FCSETTINGS_H
#include <QString>
#include <QSettings>
#include <QDir>
/*=============================================================
* TASK:    centralized settings handler
*      static members are initialized in FalconBoard.cpp!
*------------------------------------------------------------*/
struct FCSettings
{
    static QString homePath;       // path for user's home directory
    static void Init();            // set home path for falconBoard
    static inline QString Name();
    static QSettings* Open();
    static void Close() { delete _ps; }

private:
    static QSettings* _ps;
    static QString _name;
};

#endif // _FCSETTINGS_H