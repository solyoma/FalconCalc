#include "stdafx_zoli.h"
#include "stdafx_lc.h"

using namespace SmString;		
#include "SmartString.h"
#include "TStringList.h"

using namespace std;

void TStringList::SetDuplicates(bool par) 
{
	if(par == _allowDuplicates) 
		return; 
	else 
		_allowDuplicates =par; 
	if(!_allowDuplicates) _DeleteDuplicates(); 
}
void TStringList::SetSorted(bool par, bool spec) 
{ 
	if(par == _sorted) 
		return; 
	_sorted=par; 
	_special = spec;
	if(_sorted) 
		Sort();
}
void TStringList::SetCapacity(const size_t maxh) 
{ 
	_cntMax = maxh; // do not check for cntMax == maxh !
	if(_cntMax && _strings.size() > _cntMax)
	{
		_strings.reserve(_cntMax);
		_strings.resize (_cntMax);
	}
}


void TStringList::Sort() 
{ 
	if(!_allowDuplicates) 
		_DeleteDuplicates(); 
	auto it0 = _strings.begin();
	if(_special)
			++it0;
	sort(it0, _strings.end(), [this](const _Strings::__S &s1, const _Strings::__S &s2) { return _compare(_caseSens, s1.ws, s2.ws); }); 
}

//=====================================================================================
// TASK: add multiple lines to list
// RETURNS: size of list!
int TStringList::Add(const SmartString & s, bool multiple)
{
	if(!multiple)
		return Add(s);
    int en; // ,res=0;
    SmartString s0 = s, s1;
    do
    {
		en = s0.indexOf("\n"_ss);
        if( !en )
            en = s.length();
        s1 = s0.substr(1, en-1);
        s1.Trim(); // maybe ended with cr/lf, started with spaces, etc
//        if(IndexOf(s1) < 0 )
//            res +=Add(s1);
        s0 = s0.substr(en+1, s0.length());
    }
    while(s0.length());
	SetCapacity(_cntMax);

	return _strings.size(); // res;
}
//=====================================================================================
// TASK: Add single string to list
// RETURNS: index of added or -1 if already present or cut because of count constraint
int TStringList::Add(const SmartString& s1) 
{
	int ind;

	if (_sorted)
	{
		auto it = find_if(_strings.begin(), _strings.end(), [&s1, this](const _Strings::__S &s) {return _compare(_caseSens, s1, s.ws); });
		if (!_allowDuplicates && it != _strings.begin() && (_caseSens && *(it-1) == s1) || (!_caseSens && ((it-1)->ws.asLowerCase()) == s1.asLowerCase()))
			return -1;
		_strings.insert(it, s1);
		ind = (int)(it - _strings.begin());
	}
	else
	{
		if (!_allowDuplicates && find_if(_strings.begin(), _strings.end(), [&s1, this](const _Strings::__S &s) -> bool { if (!_caseSens) return s.ws.asLowerCase() == s1.asLowerCase(); return s.ws == s1; } ) != _strings.end())
			return -1;
		_strings.insert(_strings.begin(), s1);
		ind = 0;
	}
	SetCapacity(_cntMax);
	return (ind >= (int)_cntMax) ? -1 : ind;
}

bool TStringList::LoadFromFile(wstring name)
{
	_strings.clear();
	std::wifstream fs(name,ios_base::in);
	if(fs.fail() )
		return false;
	//fs.skipbom();
	//if(fs.fail() )
	//	return false;
	wstring s;
	while(!getline(fs,s).fail() )
		Add(SmartString(s) );
	SetCapacity(_cntMax);		// drop excess items
	return !fs.bad();
}

bool TStringList::SaveToFile(wstring name)
{
	//nlib::FileStream fs(name,ios_base::out);
	std::ofstream fs(name,ios_base::out);
	if(fs.fail() )
		return false;
	for(auto it=_strings.begin(); it != _strings.end(); ++it)
	{
		fs << it->ws << "\n";
		if(fs.fail() )
			return false;
	}
	return true;
}

bool TStringList::_compare(bool caseSens, const SmartString &s1, const SmartString &s2) 
{
	if(_caseSens) 
		return s1 < s2; 
	else 
		return s1.asLowerCase() < s2.asLowerCase();
//	SmartString ss1 = GenToLower(s1), ss2 = GenToLower(s2);
//	bool b = ss1 < ss2;
//	return b;
}

static bool _equal(bool caseSens, const SmartString &s1, const SmartString &s2) 
{
	if(caseSens) 
		return s1 == s2; 
	else 
		return s1.asLowerCase() == s2.asLowerCase();
}

void TStringList::_DeleteDuplicates() 
{
	vector<_Strings::__S> v;
	_Strings::iterator its;
	for(its = _strings.begin(); its != _strings.end(); ++its)
		if(find_if(its+1, _strings.end(), [this,&its](const _Strings::__S &s) {return _compare(_caseSens, s.ws,its->ws); }) == _strings.end() )
			v.push_back(*its);
	_strings = move(v);
}
bool TStringList::Delete(size_t n)
{
	auto it = _strings.erase(_strings.begin()+n);
	return it != _strings.end();
}

int TStringList::IndexOf(SmartString s)
{
	for(size_t n= 0; n < _strings.size(); ++n)
		if(_equal(_caseSens, s, _strings[n].ws))
			return (int)n;
	return -1;
}

size_t TStringList::Insert(int where, SmartString what)
{
	_strings.insert(_strings.begin()+where, what);
	SetCapacity(_cntMax);		// drop excess items
	return _strings.size();
}
void TStringList::Clear()
{
	_strings.clear();
}