#pragma once
using namespace std;

#include "SmartString.h"

using SmString::SmartString;

class TStringList 
{
public:
	TStringList() {}
	TStringList(TStringList& sl) :_caseSens(sl._caseSens), _allowDuplicates(sl._allowDuplicates), 
										_sorted(sl._sorted), _special(sl._special), 
													_cntMax(sl._cntMax), _strings(sl._strings) {}
	virtual ~TStringList() {}

	SmartString operator[](size_t index) const { if(index >= (int)_strings.size() ) SmartString(); else return _strings[index].ws; }
	void *objects(size_t index) const { if (index >= _strings.size()) return nullptr; else return _strings[index].po; }
	TStringList &operator=(TStringList& sl) {
		_caseSens = sl._caseSens; _allowDuplicates = sl._allowDuplicates;
			_sorted = sl._sorted; _special = sl._special;
			_cntMax = sl._cntMax; _strings = sl._strings;
	}
	SmartString &operator=(vector<SmartString> w) { _strings = w;}

	bool CaseSensitive() const {return _caseSens;}
	bool Duplicates()    const {return _allowDuplicates;}
	bool Sorted()        const { return _sorted; }
	bool Special()		 const { return _special; }
	size_t Capacity()		 const { if(_cntMax) return _cntMax; else return (size_t)-1; }
	const vector<SmartString> &Lines() {return _strings.AsStringVector(); }

	size_t Count() const { return _strings.size(); }
	int IndexOf(SmartString s);

	int Add(const SmartString& s1);	// add a single line of text
	int Add(const SmartString& s1, bool multiple);		 // can add multiple lines of text separated with LF or CR or CR/LF pairs
													 // in one call if 'multiple' is true. It splits the text to lines 
	                                                 // and trims each line of leading or trailing \n \r \f \t \v 
	size_t Insert(int where, SmartString what);
	void Clear();

	void SetCaseSensitive(bool par) {_caseSens =par;}
	void SetDuplicates(bool par);
	void SetCapacity(const size_t max=0);
	void SetSorted(bool par, bool special=false);
	void Sort();
	bool LoadFromFile(SmartString name);
	bool SaveToFile(SmartString name);

	bool Delete(size_t index);
private:
	bool _caseSens=false;
	bool _allowDuplicates = true;
	bool _sorted=false;				// true: lines are alphabetically sorted
	bool _special=false;				// if set the first line is excluded from sorting
	size_t _cntMax=0;				// max number of lines this list can hold (0:unlimited)
								// if not 0 drops lines from end of list when the count
								// reaches cntMax. For sorted lists new lines are added
								// first and the excess lines are deleted afterward
	// internal storage for lines
	class _Strings
	{
	public:
		struct __S;

		_Strings() {}
		_Strings(const _Strings&s) : _data(s._data) {}
		_Strings(const _Strings&&s) : _data(s._data) {};
		_Strings(const SmartString str) { _data.push_back(__S(str)); }

		vector<SmartString> wtmp;

		friend struct __S;
		typedef vector<__S> Svec;
		typedef vector<__S>::iterator iterator;
		typedef vector<__S>::const_iterator const_iterator;

		// Object to store a string together with an
		// arbitrary object
		struct __S
		{
			__S() : po(nullptr) {}
			__S(const SmartString str, void *p=nullptr) : ws(str), po(p) {}
			__S(const __S &s): ws(s.ws),po(s.po) {}
			__S &operator=(const __S &s) 
			{ 
				ws = s.ws; 
				po = s.po; 
				return *this;
			}
			bool operator==(const __S &s) { return ws == s.ws; }
			SmartString ws;
			void *  po;
		};

		_Strings &operator=(const _Strings &w)
		{
			_data.clear();
			// ?? miert rossz?			for (auto it = w.cbegin(); it != w.cend(); ++it) 
			//	_data.push_back(*it);
			_data.resize(w.size());
			for (size_t i = 0; i < w.size(); ++i)
				_data[i] = w._data[i];
			return *this;
		}

		_Strings &operator=(_Strings &&w) 
		{ 
			_data.clear();
			for(auto it = w.begin(); it != w.end(); ++it)
				_data.push_back(*it);
			return *this;
		}

		_Strings &operator=(vector<__S> w) 
		{ 
			_data.clear();
			for(auto it = w.begin(); it != w.end(); ++it)
				_data.push_back(*it);
			return *this;
		}

		_Strings &operator=(vector<SmartString> w) 
		{ 
			_data.clear();
			for(auto it = w.begin(); it != w.end(); ++it)
				_data.push_back(__S(*it,0) );
			return *this;
		}


		__S &operator[](size_t index) { return _data[index]; }
		const __S &operator[](size_t index) const { return _data[index]; }

		vector<SmartString> &AsStringVector() 
		{ 
			wtmp.clear();
			for(auto it = _data.begin(); it != _data.end(); ++it)
				wtmp.push_back(it->ws);
			return wtmp;
		}

		iterator begin() { return _data.begin(); }
		iterator end()   { return _data.end();   }
		const_iterator cbegin() { return _data.cbegin(); }
		const_iterator cend() { return _data.cend(); }

		iterator insert(iterator pos, const __S &s)
		{
			return _data.insert(pos, const_cast<__S &>(s));
		}
		void push_back(__S &s) { _data.push_back(s); }
		void push_back(const __S &s) { _data.push_back(s); }
		size_t size() const { return _data.size(); }
		size_t capacity() const { return _data.capacity(); }
		void reserve(size_t n) { _data.reserve(n); }
		void resize(size_t n) { _data.resize(n); }
		void resize(size_t n, __S &val) { _data.resize(n, val); }
		void clear() {_data.clear();} 
		iterator erase(iterator it) { return _data.erase(it); }
	private:
		Svec _data;
	} _strings;


	bool _compare(bool cases, const SmartString &s1, const SmartString &s2);
//	bool _compare(bool cases, const _Strings::__S &s1, const _Strings::__S &s2);

	void _DeleteDuplicates();
};