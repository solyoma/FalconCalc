#include "SmartString.h"
using namespace SmString;
#include "LongNumber.h"
using namespace LongNumber;
#include "calculate.h"
#include "FalconCalcQt.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QDir>
#include <QTranslator>

QString TranslateApp(QApplication &app)
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
	QFile f(homePath + FalconCalcQt_DAT_FILE);
	if(f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qs = f.readLine().trimmed();
		if (qs.startsWith(DAT_VER_STRING))
		{
			while (!(qs = f.readLine().trimmed()).isEmpty())
			{
				if (qs == "[Locale]")	 // windows .ini format
				{
					qs = f.readLine().trimmed();
					int n = qs.indexOf('=');
					if (qs.left(n) == "loc")
					{
						qs = qs.mid(n + 1).trimmed();	// e.g. "C" or "de" or "de_DE"
						ixLang = fileNames.indexOf("FalconCalc_" + qs + ".qm");
						if (ixLang < 0 && qs.length() > 2)
						{
							for (int i = 0; i < fileNames.size(); i++) // find first which starts with "??_"
							{
								if (fileNames[i].startsWith("FalconCalc_" + qs.left(2)))
								{
									ixLang = i;
									break;
								}
							}
							break;
						}
						break;
					}
				}
			}
		}
		f.close();
	}

	if (ixLang >= 0)
	{
		qs = ":/FalconBoard/translations/" + fileNames[ixLang];

		QTranslator translator;

		bool loaded = translator.load(qs);
		if (loaded)
			qs = translator.language();
		if (loaded && qs != "C" && qs != "en_US")	 // only set when not American English
			app.installTranslator(&translator);
	}
	return qs;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	//QString qs, qsn;
	//qs = QLocale::system().name();

	TranslateApp(a);
    FalconCalcQt w;

	w.show();
    return a.exec();
}
