#include <QtWidgets/QApplication>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QStyleFactory>

#include "defines.h"
#include "common.h"
#include "SmartStringQt.h"
using namespace SmString;
#include "LongNumber.h"
using namespace LongNumber;
#include "calculate.h"
#include "FalconCalcQt.h"

int SmString::SmartString::npos = -1;

// program language and locale used are two different things.
// both are set in the .cfg file, one as an index, the other as a name
static QString TranslateApp(QApplication &app, QTranslator &translator)
{
	QDir dir(":/FalconCalcQt/translations");
	QStringList fileNames = dir.entryList(QStringList("*.qm"), QDir::Files, QDir::Name);
	fileNames.sort();

	QString homePath = QDir::homePath() +
#if defined (Q_OS_Linux) || defined (Q_OS_Darwin) || defined(__linux__)
		"/.FalconCalc/";
#elif defined(Q_OS_WIN)
		"/Appdata/Local/FalconCalc/";
#endif

	QString qs;

	int ixLang = -1; // -1=system, else index in fileNames
	QFile f(homePath + QString(FalconCalc_State_File));
	if(f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		while (!f.atEnd() && !(qs = f.readLine().trimmed()).startsWith("language"))
			;
		if(!f.atEnd())
		{
			int n = qs.indexOf('=');
			qs = qs.mid(n + 1).trimmed();	// missing: not set, hu': English, 'en': Hungarian

			ixLang = fileNames.indexOf("FalconCalcQt_" + qs + ".qm");
			if (ixLang < 0)
			{
				for (int i = 0; i < fileNames.size(); i++) // find first which starts with "??_"
				{
					if (fileNames[i].startsWith("FalconCalcQt_" + qs.left(2)))
					{
						ixLang = i;
						break;
					}
				}
			}
		}
		f.close();
	}

	if (ixLang >= 0)
	{
		qs = ":/FalconCalcQt/translations/" + fileNames[ixLang];


		bool loaded = translator.load(qs);
		if (loaded)
		{
			qs = translator.language();
			bool res = true;
			if (qs != "C" && qs != "en_US")	 // only set when not American English
			{
				res = app.installTranslator(&translator);
				if (!res)
					QMessageBox::warning(nullptr, QObject::tr("FalconCalcQt - Error"), QObject::tr("Cannot install language file %1").arg(fileNames[ixLang]));
			}
		}
	}
	return qs;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setStyle("fusion");

	QTranslator translator;
	TranslateApp(a, translator);
    FalconCalcQt w;

	w.show();
    return a.exec();
}
