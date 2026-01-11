#pragma once

#ifndef _SMARTSTRING_QT_H
	#define _SMARTSTRING_QT_H

#include <algorithm>
#include <vector>
#include <locale>

#include <QChar>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QLocale>
#include <QFile>
#include <QTextStream>

#include "defines.h"

using CaseSens = Qt::CaseSensitivity;

namespace SmString {

	#define csCaseInsensitive Qt::CaseInsensitive 
	#define csCaseSensitive Qt::CaseSensitive	

	typedef QString String;
	typedef std::string UTF8String;

	//-------------- class SCharT - wrapper for QChar with some additional functionality ---
	class SCharT : public QChar
	{
		friend class SmartString;
	public:
		SCharT() {}
		template<typename T> SCharT(T t) : QChar(t) {}

		operator QChar() { return *this; }
		operator char16_t() const { return unicode(); }
		operator wchar_t() const { return (wchar_t)unicode(); }
		operator int() const { return (int)unicode(); }

		SCharT& operator+=(const SCharT n) { *this = *this + n; return *this; }
		SCharT& operator-=(const SCharT n) { *this = *this - n; return *this; }
		SCharT  operator+ (const SCharT n) const { QChar sch; sch = (unicode() + n.unicode()); return sch; }
		SCharT  operator- (const SCharT n) const { QChar sch; sch = (unicode() - n.unicode()); return sch; }

		SCharT operator++() { *this = unicode() + 1; return *this; }
		SCharT operator++(int) { SCharT p(*this); (*this)++; return p; }
		SCharT operator--() { *this = unicode() - 1; return *this; }
		SCharT operator--(int) { SCharT p(*this); (*this)--; return p; }

		bool operator<(const SCharT& sch)  const { return QChar(*this) < QChar(sch); }
		bool operator>(const SCharT& sch)  const { return QChar(*this) > QChar(sch); }
		bool operator<=(const SCharT& sch) const { return QChar(*this) <= QChar(sch); }
		bool operator>=(const SCharT& sch) const { return QChar(*this) >= QChar(sch); }
		bool operator!=(const QChar& qc) const { return QChar(*this) != qc; }
		bool operator==(const QChar& qc) const { return QChar(*this) == qc; }

		bool IsDigit() const { return (QChar*)(this)->isDigit(); }
		bool IsAlnum() const { return (QChar*)(this)->isLetterOrNumber(); }
		bool IsAlpha() const { return (QChar*)(this)->isLetter(); }

		SCharT ToUpper() { return this->toUpper(); }
		SCharT ToLower() { return this->toLower(); }
		SCharT ToUpper(std::locale loc) { *this = std::toupper((wchar_t)unicode(), loc); return *this; }
		SCharT ToLower(std::locale loc) { *this = std::tolower((wchar_t)unicode(), loc); return *this; }

	};

	//-------------- class SmartString - wrapper for QString with some additional functionality ---

	class SmartStringVector;	// forward declaration

	class SmartString : public QString
	{
	public:
		static int npos; // defined in main.cpp (no SmartStringQt.cpp - yet)
	public:
		SmartString() : QString() {}
		template<typename T> SmartString(T t) : QString(t) {}
		SmartString(const UTF8String& utf8s) : QString( QString::fromUtf8(utf8s.c_str())) {}
		SmartString(const std::wstring& ws) : QString(QString::fromStdWString(ws)) {}
		SmartString(const std::u16string& ws) : QString(QString::fromStdU16String(ws)) {}
		SmartString(const char16_t *ws) : QString(QString::fromStdU16String(std::u16string(ws))) {}

		SmartString(int len, SCharT ch) : QString(len, ch) {}
		SmartString(SmartString::iterator from, SmartString::iterator to)
		{
			SmartString s = mid(from - begin(), to - from);
		}
		~SmartString() {}

		// **** replacements for std::string functions 
		void erase(int pos, int len=-1)
		{
			if(len < 0)
				len = length() - pos;
			*this = left(pos) + mid(pos + len);
		}
		void erase(SmartString::iterator from, SmartString::iterator to)
		{
			int p1 = from - begin();
			int p2 = to - begin();
			*this = left(p1) + mid(p2);
		}
		inline bool empty() const { return isEmpty(); }
		int find_first_not_of(const QChar& qch, int pos = 0) const
		{
			if (pos < 0)
				return npos;
			for (int i = pos; i < length(); i++)
			{
				if (at(i) != qch)
					return i;
			}
			return npos;
		}
		int find_first_of(const QChar& qch, int pos = 0) const
		{
			return indexOf(qch, pos);
		}
		int find_last_of(const QChar& qch, int pos = -1) const
		{
			int i = lastIndexOf(qch, pos);
			if(i <0)
				return npos;
			return i;
		}
		int find_last_not_of(const QChar& qch, int pos = -1) const
		{
			if (pos >= 0)
				return npos;
			pos = length() + pos;
			if(pos < 0)
				return npos;
			for (int i = length()-1; i >= pos ;i--)
			{
				if (at(i) != qch)
					return i;
			}
			return npos;
		}
		SmartString &insert( int pos, int count, const QChar qch)
		{
			QString::insert(pos, &qch, 1);
			return *this;
		}

		inline void pop_back() { return chop(1); }
		// **** end of replacement for std::string functions

		// overriden 
		SCharT at(int pos, QChar defch = QChar(0)) const
		{
			return pos >= length() ? defch : QString::at(pos);
		}

		void FromWideString(const std::wstring ws)
		{
			*this = QString::fromStdWString(ws);
		}

		UTF8String toUtf8String() const
		{
			return this->toUtf8().constData();
		}

		template<typename T> SmartString& operator=(const T t) { QString::operator=(t); return *this; }
		SmartString& operator=(const std::string &ws) { *this = fromStdString(ws); return *this; }
		SmartString& operator=(const std::wstring &ws) 
		{ 
			*this = fromStdWString(ws); 
			return *this; 
		}
		SmartString& operator=(const std::u16string &u16s) { *this = fromStdU16String(u16s); return *this; }
		SmartString& operator=(const char16_t *p16s) { *this = fromStdU16String(p16s); return *this; }
		SmartString& operator=(const char ch) { *this = SmartString(ch); return *this; }
		template<typename T> SmartString& operator+=(const T t) { QString::operator+=(t); return *this; }
		bool operator==(const SmartString& ss) const { return static_cast<const QString&>(*this) == static_cast<const QString&>(ss); }
		bool operator!=(const SmartString& ss) const { return static_cast<const QString&>(*this) != static_cast<const QString&>(ss); }

		int CompareWith(const SmartString ss, CaseSens caseSensitivity) const {
			return QString::compare(QString(*this), QString(ss), caseSensitivity);
		}
		SmartString left(int n, QChar fillChar = QChar(-1)) const // may extend the string to the right
		{
			if (n == npos || n == length())	// full string or outside of string
				return *this;

			if (n < length() || fillChar == QChar(-1))
				return QString::left(n);
			return *this + SmartString(n - length(), fillChar);
		}
		SmartString right(int n, QChar fillChar = QChar(-1)) const	// may extend the string to the left
		{
			if (n == npos || n == length())	// full string or outside of string
				return *this;
			if (n < length() || fillChar == QChar(-1))
				return QString::right(n);
			return SmartString(n - length(), fillChar) + *this;
		}
		// similar to QString::mid() but may extend the string with a given character too
		SmartString mid(int pos, int n = -1, SCharT fillChar = SCharT(-1)) const
		{
			if (pos < 0 && n == -1) return *this; // full string
			if (n == -1) n = length() - pos;
			if (pos + n <= length() || fillChar == SCharT(-1))
				return QString::mid(pos, n);
			SmartString res = QString::mid(pos, n);
			return res + SmartString(pos + n - length(), fillChar);
		}

		template<typename T> int indexOf(T t, int pos = 0) const	// start from 'pos'
		{
			return QString::indexOf(t, pos);
		}

		int indexOfRegex(const SmartString regexpString, int pos = 0) const		// only first occurance
		{
			QRegExp rx(regexpString);
			return rx.indexIn(*this, (int)pos);
		}
		SmartString asUpperCase() const { QLocale loc; return loc.toUpper(QString(*this)); }
		SmartString asLowerCase() const { QLocale loc; return loc.toLower(QString(*this));}

		std::wstring ToWideString() const
		{
			return this->toStdWString();
		}
		QString toQString() const { return *this; }

		void LTrim()
		{
			int i = 0;
			while (i < length() && at(i).isSpace()) i++;
			if (i > 0) *this = mid(i);
		}
		void RTrim()
		{
			int i = length() - 1;
			while (i >= 0 && at(i).isSpace()) i--;
			if (i < length() - 1) *this = left(i + 1);
		}
		void Trim()
		{
			LTrim();
			RTrim();
		}
		SmartString LTrimmed() const { SmartString s = *this; s.LTrim(); return s; }
		SmartString RTrimmed() const { SmartString s = *this; s.RTrim(); return s; }
		SmartString Trimmed() const { return trimmed(); }
		void RemoveWhiteSpace()	// even from inside
		{
			*this = simplified();
		}

		SmartStringVector Split(const SCharT ch, bool keepEmpty) const;
		SmartStringVector SplitRegex(const SmartString regex, bool keepEmpty) const;
		void Reverse() // order of characters
		{
			std::reverse(begin(), end());
		}
	};

	// ************** type definition *********
	inline const SmartString operator""_ss(const char* ps, size_t len)
	{
		return SmartString(ps);
	}


	// ************** SmartStringVector *********
	class SmartStringVector : public QStringList
	{
	public:
		SmartStringVector() : QStringList() {}
		SmartStringVector(const QStringList& sl) :QStringList(sl) {}
		SmartStringVector(const SmartString s, const SCharT ch, bool keepEmpty, bool trim)
		{
			*this = s.Split(ch, keepEmpty);
			if (trim)
				for (int i = 0; i < size(); ++i)
					(*this)[i] = (*this)[i].trimmed();
		}
		SmartStringVector(const SmartString s, SmartString regex, bool keepEmpty, bool trim)
		{
			*this = s.SplitRegex(regex, keepEmpty);
			if (trim)
				for (int i = 0; i < size(); ++i)
					(*this)[i] = (*this)[i].trimmed();
		}
		SmartStringVector(const SmartStringVector& sv) :QStringList(sv) {}
		SmartStringVector& operator=(const SmartStringVector& sv) 
		{ 
			QStringList::operator=(sv); return *this; 
		}

		bool IsSorted() const { return _isSorted; }
		bool isCaseSensitive() const { return _caseSens == csCaseSensitive; }
		void SetSorted(bool setSorted)		// when setSorted is true sorts the vector except for the first (topmost) item
		{
			bool wasSorted = _isSorted;
			_isSorted = setSorted;
			if (!wasSorted)
				_Sort();
		}
		void Add(SmartString s) { *this << QString(s); }
		bool LoadFromFile(SmartString name)
		{
			QFile f(name);
			if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
				return false;
			QTextStream ifs(&f);

			SmartString line;
			bool sorted = _isSorted;
			sorted = false;
			while (!ifs.atEnd())
			{
				ifs >> line;
				push_back(line);
			}
			SetSorted(sorted);	// sort if needed
			return true;
		}
		bool SaveToFile(SmartString name)
		{ 
			QFile f(name);
			if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
				return false;
			QTextStream ofs(&f);

			for (const auto& s : *this)
				ofs << s << "\n";
			return true;
		}
		int  IndexOf(const SmartString s, int from = 0) { return indexOf(s, from); }	// uses _caseSens and linear search
		void Delete(int n) { erase(begin() + n); }
	private:
		bool _changed = false;
		bool _isSorted = false;
		CaseSens _caseSens = csCaseInsensitive;
		void _Sort();	// the 0th indexed item remains that
	};

	inline SmartStringVector SmartString::Split(const SCharT ch, bool keepEmpty) const
	{
		return QString::split(QChar(ch), keepEmpty ? Qt::KeepEmptyParts : Qt::SkipEmptyParts);
	}
	inline SmartStringVector SmartString::SplitRegex(const SmartString regex, bool keepEmpty) const
	{
		return QString::split(QRegExp(QString(regex)), keepEmpty ? Qt::KeepEmptyParts : Qt::SkipEmptyParts);
	}
}    // namespace SmString
#endif
