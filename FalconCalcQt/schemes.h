#pragma once
#ifndef _SCHEMES_H
#define _SCHEMES_H

#include <tuple>

#include <QMap>

#include <QObject>
#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QStringList>

enum class Scheme { schSystem, schLight, schDark, schBlack, schBlue };

struct FalconCalcScheme
{
	friend class FSchemeVector;
	QString MenuTitleForLanguage(int lang);
	QString& operator[](int index);

	FalconCalcScheme();

	FalconCalcScheme(const QString sMenuTitle, std::vector<QString> names);
	FalconCalcScheme(const QString sMenuTitle, std::vector<QString> names, std::vector<QString> values);

	int Size() const { return _values.size(); }
	QString Table() const
	{
		QString s = "------------color array -------\n";
		for (size_t i = 0; i < _names.size(); ++i)
			s += QString("%1 ").arg(i+1, 2) + _names[i] + " = " + _values[i].second + "\n";
		return s;
	}
private:
	QString _sMenuTitle;
	std::vector<std::pair<int, QString> > _values;
	std::vector<QString> _names;
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
