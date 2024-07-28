#include "stdafx_zoli.h"
#include "inifile.h"
#ifdef __MINGW32__
#include <float.h>
#endif


//---------------------------------------------


namespace NLIBNS
{


/**
 * An empty object is created, which can be filled with sections containing key/value pairs. Call
 * Save() to associate the object with a file and file name. If you want to read the values of
 * an existing ini file, either call Open() or construct the IniFile object with the constructor
 * expecting a file name.
 * \sa Disconnect(), Open(), Save(), Clear(), Flush()
 */
IniFile::IniFile() : changed(false)
{
}

/**
 * If the file exists and it contains valid ini sections and key/value pairs, they are read into memory.
 * The IniFile object will be associated with the file name, and before destroyed, Flush() will be
 * called to save any changes. The read values can be cleared by calling Clear(), but an empty IniFile
 * can be created with the constructor with no arguments and saved under the name of an existing file.
 * \param filename Path to the ini file which will be associated with this object.
 * \param encoding Specify the encoding to be used when sparsing the ini file when reading or writing.
 * Incorrect encoding when reading ini files can result in corrupted data.
 * \sa Disconnect(), Open(), Save(), Clear(), Flush()
 */
IniFile::IniFile(const std::wstring& filename, FileStreamEncoding encoding) : changed(false)
{
#ifdef _MSC_VER
    locale = _wcreate_locale(LC_ALL, L"en-US");
#endif
    Open(filename, encoding);
}

/**
 * Flush() causes the values in the IniFile object to be saved, if the object is associated with a file
 * name, so there is no need to call Save() before object destruction. Call Disconnect() while the object
 * is valid to prevent saving its values to a file.
 * \sa Disconnect(), Save(), Flush()
 */
IniFile::~IniFile()
{
    Flush();
#ifdef _MSC_VER
    _free_locale(locale);
#endif
}

bool IniFile::GetLine(FileStream &file, std::wstring &line)
{
    if (!file.is_open() || file.fail())
        return false;

    struct clearstruct
    {
        FileStream &file;
        clearstruct(FileStream &file) : file(file) {}
        ~clearstruct() { file.clear(); }
    } clearstruct(file);

    int pos = 0;
    line.resize(0);
    do
    {
        file.clear();
        line.resize(pos + (pos < 128 ? 32 : pos < 256 ? 64 : pos / 4));
        file.getline(const_cast<wchar_t*>(line.c_str() + pos), line.length() - pos);
        int glen = file.gcount() - (file.fail() ? 0 : 1);
        pos += glen;
    } while(file.fail() && !file.eof());

    if ((file.eof() && file.fail()) || file.bad())
        return false;

    line.resize(pos);
    return true;
}

/**
 * Disconnected IniFile objects don't get saved on destruction, and the values stored in memory are not
 * deleted. Call Save() or Open() if you want to associate the object with an ini file again. Call Clear()
 * to remove all values from the object.
 * \sa Open(), Save(), Clear()
 */
void IniFile::Disconnect()
{
    fname = std::wstring();
}

/**
 * Clearing the object doesn't cancel the association between it and the associated file name. To do that
 * call Disconnect(). If the object is destroyed or flushed after calling Clear(), an empty file will be
 * produced.
 * \sa Disconnect(), Flush()
 */
void IniFile::Clear()
{
    if (sections.empty())
        return;
    sections.resize(0);
    values.resize(0);
    changed = true;
}

/**
 * The copied sections merge into the sections with the same name in the destination object or
 * completely replace them, depending on the value of \a fullreplace. If the sections are merged, values
 * under existing keys are overwritten.
 * \param dest An IniFile object to hold the copied values.
 * \param fullreplace Set to \a true to delete all sections and values in the other object before the copy.
 * \sa CopySection()
 */
void IniFile::CopyValues(IniFile &dest, bool fullreplace)
{
    if (&dest == this)
        return;

    if (fullreplace)
    {
        dest.values.assign(values.begin(), values.end());
        dest.sections.assign(sections.begin(), sections.end());

        auto sit = dest.sections.begin();
        for (auto it = values.begin(), oit = dest.values.begin(); it != values.end() && sit != dest.sections.end(); ++it, ++oit)
        {
            if (it == sit->second)
            {
                sit->second = oit;
                ++sit;
            }
        }
        dest.changed = true;
    }
    else
    {
        for (auto sit = sections.begin(); sit != sections.end(); ++sit)
            CopySection(dest, sit, sit->first, false);
    }

}

/**
 * Setting or deleting values puts the object to a changed state. Call Open(), Save() or Flush() to read or
 * write the values and remove the changed state from the IniFile object. IniFile objects that are not changed
 * don't write to the file of their associated file name even after a call to Flush(). Save() forces writing.
 * \sa Open(), Save(), Flush()
 */
bool IniFile::Changed()
{
    return changed;
}

/**
 * If no name is associated with the object, an empty string is returned.
 * \sa Encoding(), Disconnect(), Open(), Save()
 */
const std::wstring& IniFile::FileName()
{
    return fname;
}

/**
 * The encoding determines how characters are stored in an ini file. The encoding can be set with the constructor
 * which accepts a file name and an encoding, or by passing the appropriate value to Open() or Save(). Calling
 * Flush() or destroying the IniFile object writes data with the specified encoding to the file whose file name
 * is associated with the object.
 * \remark If no file was opened or saved before calling this function, the result is undefined.
 */
FileStreamEncoding IniFile::Encoding()
{
    return enc;
}

void IniFile::CheckSection(int index)
{
    if (index < 0 || index >= (int)sections.size())
        throw EOutOfRange(L"Section index out of range.", index);
}

void IniFile::CheckSectionKey(int sectionindex, int keyindex)
{
    CheckSection(sectionindex);
    if (keyindex < 0 || keyindex >= KeyCount(sectionindex))
        throw EOutOfRange(L"Key index out of range.", keyindex);
}

bool IniFile::CheckSectionName(const std::wstring& sectionname)
{
    return !sectionname.empty() && sectionname.find_first_of(L"[]") == std::wstring::npos;
}

bool IniFile::CheckKeyName(const std::wstring& keyname)
{
    return !keyname.empty() && keyname.find(L"=") == std::wstring::npos && keyname[0] != L'[' && keyname[0] != L';';
}

int IniFile::AddSection(std::wstring sectionname)
{
    sectionname = trim(sectionname);
    int ix = InnerSectionIndex(GenToLower(sectionname));
    if (ix < 0) // Section does not exist yet.
    {
        ix = sections.size();
        sections.push_back(SectionPair(sectionname, values.end()));
        changed = true;
    }
    return ix;
}

void IniFile::SetValue(int sectionix, int keyix, const std::wstring &value)
{
    CheckSectionKey(sectionix, keyix);

    auto it = sections[sectionix].second;
    std::advance(it, keyix);
    if (it->second != value)
    {
        it->second = value;
        changed = true;
    }
}

void IniFile::AddValue(int six, const std::wstring &key, const std::wstring &value) // Key must be trimmed and valid.
{
    CheckSection(six);

    int kix = InnerKeyIndex(six, GenToLower(key));
    if (kix >= 0)
    {
        auto it = sections[six].second;
        std::advance(it, kix);
        if (it->second != value)
        {
            it->second = value;
            changed = true;
        }
        return;
    }

    changed = true;
    auto it = six == (int)sections.size() - 1 ? values.end() : sections[six + 1].second;
    it = values.insert(it, KeyValue(key, value));
    if (sections[six].second == values.end())
        sections[six].second = it;
}

void IniFile::AddValue(std::wstring section, std::wstring key, const std::wstring &value)
{
    key = trim(key);
    section = trim(section);
    if (!CheckSectionName(section) || !CheckKeyName(key))
        return;

    int six = InnerSectionIndex(GenToLower(section));
    if (six < 0)
        six = AddSection(section);

    AddValue(six, key, value);
}

void IniFile::CopySection(IniFile &dest, SectionIterator sectit, const std::wstring &destsection, bool fullreplace)
{
    if (!CheckSectionName(destsection) || (this == &dest && destsection == sectit->first))
        return;

    if (fullreplace)
        dest.DeleteSection(destsection);

    int ix = dest.AddSection(destsection);
    auto endit = (sectit + 1 == sections.end() ? values.end() : (sectit + 1)->second);
    for (auto it = sectit->second; it != endit; ++it)
        dest.AddValue(ix, it->first, it->second);
}

/**
 * The destination section is created if it does not exist.
 * \param dest Destination IniFile, which can be the same object as the source.
 * \param sectionindex Index of the section holding the keys and values to be copied to \a other.
 * If this value is out of range, an EOutOfRange exception is raised.
 * \param destsection Name of the destination section which will hold the values after the copy.
 * \param fullreplace Set to \a true to remove all keys and values from the destination section if it
 * already exists.
 * \remark Whitespace only strings or strings containing square bracket characters are invalid. Passing
 * an invalid \a destsection is not an error, but in that case no section is copied.
 * \sa CopyValues(), RenameSection()
 */
void IniFile::CopySection(IniFile &dest, int sectionindex, const std::wstring &destsection, bool fullreplace)
{
    CheckSection(sectionindex);
    CopySection(*this, sections.begin() + sectionindex, destsection, fullreplace);
}

/**
 * The destination section is created if it does not exist.
 * \param dest Destination IniFile, which can be the same object as the source.
 * \param sectionname Name of the section holding the keys and values to be copied to \a other. If the section
 * is not found, nothing is copied.
 * \param destsection Name of the destination section which will hold the values after the copy.
 * \param fullreplace Set to \a true to remove all keys and values from the destination section if it
 * already exists.
 * \remark Whitespace only strings or strings containing square bracket characters are invalid. Passing
 * an invalid \a sectionname or \a destsection is not an error, but in that case no section is copied.
 * \sa CopyValues(), RenameSection()
 */
void IniFile::CopySection(IniFile &dest, const std::wstring &sectionname, const std::wstring &destsection, bool fullreplace)
{
    int sectionindex = InnerSectionIndex(GenToLower(trim(sectionname)));
    if (sectionindex < 0)
        return;
    CopySection(*this, sections.begin() + sectionindex, destsection, fullreplace);
}

/**
 * The operation fails if a section named \a newsectionname already exists.
 * \param sectionindex Index of the section to rename. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \param newsectionname New name for the section.
 * \remark Whitespace only strings or strings containing square bracket characters are invalid. Passing
 * an invalid \a newsectionname is not an error, but in that case the section is not renamed.
 * \sa CopySection()
 */
void IniFile::RenameSection(int sectionindex, const std::wstring &newsectionname)
{
    CheckSection(sectionindex);

    std::wstring sname = trim(newsectionname);

    int nix = InnerSectionIndex(GenToLower(sname));
    if (nix >= 0 || sections[sectionindex].first == sname)
        return;
    sections[sectionindex].first = sname;
    changed = true;
}

/**
 * The operation fails if \a sectionname is not found or a section named \a newsectionname
 * already exists.
 * \param sectionname Current name of the section to rename.
 * \param newsectionname New name for the section.
 * \remark Whitespace only strings or strings containing square bracket characters are invalid.
 * Passing an invalid \a newsectionname is not an error, but in that case the section is not renamed.
 * \sa CopySection()
 */
void IniFile::RenameSection(const std::wstring &sectionname, const std::wstring &newsectionname)
{
    if (sectionname == newsectionname)
        return;

    int ix = InnerSectionIndex(GenToLower(trim(sectionname)));
    RenameSection(ix, newsectionname);
}


/**
 * \remark The number of sections in memory can change without affecting an ini file on the disk.
 * \sa SectionIndex(), KeyCount()
 */
int IniFile::SectionCount()
{
    return sections.size();
}

int IniFile::InnerSectionIndex(const std::wstring &sectionname)
{
    for (auto it = sections.begin(); it != sections.end(); ++it)
        if (GenToLower(it->first) == sectionname)
            return it - sections.begin();
    return -1;
}

/**
 * The index of the section can be passed to value getter and setter functions, or to functions
 * managing sections.
 * \param sectionname Name of the section.
 * \return Index of the section if \a sectionname was found or -1.
 * \sa SectionName(), KeyIndex()
 */
int IniFile::SectionIndex(const std::wstring &sectionname)
{
    std::wstring sname = trim(sectionname);
    if (!CheckSectionName(sname))
        return -1;
    return InnerSectionIndex(GenToLower(sname));
}

/**
 * The name of the section can be passed to value getter and setter functions, or to functions
 * managing sections. Use SectionCount() to get the upper limit of section indexes.
 * \param sectionindex Index of the section. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \return Name of the section if \a sectionindex is valid.
 * \sa SectionCount(), SectionIndex(), KeyName()
 */
const std::wstring& IniFile::SectionName(int sectionindex)
{
    CheckSection(sectionindex);
    return sections[sectionindex].first;
}

/**
 * Use DeleteKey() to delete single keys from a section instead.
 * \param sectionindex Index of the section to delete. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \sa SectionCount(), DeleteKey()
 */
void IniFile::DeleteSection(int sectionindex)
{
    CheckSection(sectionindex);

    auto sit = sections.begin() + sectionindex;
    if (sit->second != values.end())
    {
        auto it2 = sectionindex == (int)sections.size() - 1 ? values.end() : sections[sectionindex + 1].second;
        values.erase(sit->second, it2);
    }
    sections.erase(sit);
    changed = true;
}

/**
 * If the section with \a sectionname is not found, no sections are deleted. Use DeleteKey() to delete
 * single keys from a section instead.
 * \param sectionname Name of the section to delete.
 * \sa SectionCount(), DeleteKey()
 */
void IniFile::DeleteSection(const std::wstring& sectionname)
{
    int ix = SectionIndex(sectionname);
    if (ix >= 0)
        DeleteSection(ix);
}

/**
 * \param sectionindex Index of the section whose keys are counted. If this value is out of
 * range, an EOutOfRange exception is raised.
 * \remark The number of keys of a section in memory can change without affecting an ini file on the disk.
 * \sa KeyIndex(), SectionCount()
 */
int IniFile::KeyCount(int sectionindex)
{
    CheckSection(sectionindex);

    auto it = sections[sectionindex].second;
    if (it == values.end())
        return 0;
    auto it2 = sectionindex == (int)sections.size() - 1 ? values.end() : sections[sectionindex + 1].second;
    return std::distance(it, it2);
}

/**
 * \param sectionname Name of the section whose keys are counted.
 * \remark The number of keys of a section in memory can change without affecting an ini file on the disk.
 * \sa KeyIndex(), SectionCount()
 */
int IniFile::KeyCount(const std::wstring& sectionname)
{
    int ix = SectionIndex(sectionname);
    if (ix >= 0)
        return KeyCount(ix);
    return 0;
}

int IniFile::InnerKeyIndex(int sectionindex, const std::wstring &keyname)
{
    auto it = sections[sectionindex].second;
    if (it == values.end())
        return - 1;

    auto it2 = sectionindex == (int)sections.size() - 1 ? values.end() : sections[sectionindex + 1].second;

    int ix = 0;
    for ( ; it != it2; ++it, ++ix)
    {
        if (GenToLower((*it).first) == keyname)
            return ix;
    }
    return -1;
}

/**
 * The index of the key can be passed to value getter and setter functions, or to functions
 * managing keys.
 * \param sectionindex Index of the section containing the key. If this value is out of
 * range, an EOutOfRange exception is raised.
 * \param keyname Name of the key.
 * \return Index of the key if \a keyname was found or -1.
 * \sa KeyName(), SectionIndex()
 */
int IniFile::KeyIndex(int sectionindex, const std::wstring &keyname)
{
    CheckSection(sectionindex);

    std::wstring key = trim(keyname);
    if (!CheckKeyName(key))
        return -1;

    return InnerKeyIndex(sectionindex, GenToLower(key));
}

/**
 * The index of the key can be passed to value getter and setter functions, or to functions
 * managing keys.
 * \param sectionname Name of the section containing the key.
 * \param keyname Name of the key.
 * \return Index of the key if \a keyname was found or -1.
 * \sa KeyName(), SectionIndex()
 */
int IniFile::KeyIndex(const std::wstring& sectionname, const std::wstring& keyname)
{
    int ix = SectionIndex(sectionname);

    if (ix >= 0)
        return KeyIndex(ix, keyname);
    return -1;
}

/**
 * The name of the key can be passed to value getter and setter functions, or to functions
 * managing keys. Use SectionCount() to get the upper limit of section indexes and KeyCount()
 * to get the upper limit of key indexes.
 * \param sectionindex Index of the section containing the key. If this value is out of
 * range, an EOutOfRange exception is raised.
 * \param keyindex Index of the key. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \return Name of the key if \a sectionindex and \a keyindex are valid.
 * \sa KeyCount(), KeyIndex(), SectionCount(), SectionName()
 */
const std::wstring& IniFile::KeyName(int sectionindex, int keyindex)
{
    CheckSectionKey(sectionindex, keyindex);

    auto it = sections[sectionindex].second;
    std::advance(it, keyindex);

    return it->first;
}

/**
 * The name of the key can be passed to value getter and setter functions, or to functions
 * managing keys. Use KeyCount() to get the upper limit of key indexes.
 * \param sectionname Name of the section containing the key.
 * \param keyindex Index of the key. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \return Name of the key if \a sectionname and \a keyindex are valid. An empty string
 * is returned if the section was not found.
 * \sa KeyCount(), KeyIndex(), SectionName()
 */
const std::wstring IniFile::KeyName(const std::wstring &sectionname, int keyindex)
{
    int six = SectionIndex(sectionname);
    if (six < 0)
        return std::wstring();
    CheckSectionKey(six, keyindex);

    auto it = sections[six].second;
    std::advance(it, keyindex);

    return it->first;
}

/**
 * Deleting the last key in a section deletes the section as well. Use DeleteSection() to delete all keys
 * from a section. For setting an empty value to a key, use the appropriate setter function and pass it an
 * empty string instead. Use SectionCount() to get the upper limit of section indexes and KeyCount() to get
 * the upper limit of key indexes.
 * \param sectionindex Index of the section containing the key. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \param keyindex Index of the key to delete. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \sa SectionCount(), KeyCount(), DeleteSection()
 */
void IniFile::DeleteKey(int sectionindex, int keyindex)
{
    CheckSectionKey(sectionindex, keyindex);

    auto it = sections[sectionindex].second;
    bool firstval = keyindex == 0;
    std::advance(it, keyindex);

    it = values.erase(it);
    if (firstval)
    {
        if (it == (sectionindex == (int)sections.size() - 1 ? values.end() : sections[sectionindex + 1].second))
            sections.erase(sections.begin() + sectionindex);
        else
            sections[sectionindex].second = it;
    }
    changed = true;
}

/**
 * If \a sectionname or \a keyname is invalid or not found, no key is deleted. Deleting the last key in a
 * section deletes the section as well. Use DeleteSection() to delete all keys from a section. For setting
 * an empty value to a key, use the appropriate setter function and pass it an empty string instead.
 * \param sectionname Name of the section containing the key.
 * \param keyname Name of the key to delete.
 * \sa DeleteSection()
 */
void IniFile::DeleteKey(const std::wstring& sectionname, const std::wstring& keyname)
{
    int ix = SectionIndex(sectionname);
    if (ix >= 0)
    {
        int vix = KeyIndex(ix, keyname);
        if (vix >= 0)
            DeleteKey(ix, vix);
    }
}

std::wstring IniFile::FromInt(int val)
{
    //bool neg = val < 0;
    //int vallen = val < 0 ? 2 : 1;
    //while ((neg && val < -9) || (!neg && val > 9))
    //{
    //    val /= 10;
    //    ++vallen;
    //}
    //std::wstring newval(vallen, L'x');
    //_itow(val, const_cast<wchar_t*>(newval.c_str()), 10);
    //return newval;

    wchar_t dest[12];
    _itow(val, dest, 10);
    return dest;
}

int IniFile::ToInt(const std::wstring &value)
{
#ifdef _MSC_VER
    return _wtoi_l(value.c_str(), locale);
#else
    return atoi(WideToANSI(value).c_str());
#endif
}

std::wstring IniFile::FromBool(bool val)
{
    return val ? L"1" : L"0";
}

bool IniFile::ToBool(std::wstring value)
{
    value = trim(value);
    return value != L"0" && GenToLower(value) != L"false";
}

std::wstring IniFile::FromFloat(float val)
{
#ifdef __MINGW32__
    char *c = setlocale(LC_NUMERIC, nullptr);
    if (strcmp(c, "C") != 0)
        c = _strdup(setlocale(LC_NUMERIC, "C"));
    else
        c = nullptr;

    // No wide char support.
    //wchar_t fstr[3 + FLT_MANT_DIG - FLT_MIN_EXP + 1 + /* safety first */ 64];
    //swprintf_l(fstr, L"%f", val);
    char cfstr[3 + FLT_MANT_DIG - FLT_MIN_EXP + 1 + /* safety first */ 64];
    sprintf(cfstr, "%f", val);
    int len = strlen(cfstr);
    wchar_t *w = ANSIToWide(cfstr, len);
    std::wstring fstr(w, len);
    delete[] w;

    if (c != nullptr)
    {
        setlocale(LC_NUMERIC, c);
        free(c);
    }
#else
    wchar_t fstr[3 + FLT_MANT_DIG - FLT_MIN_EXP + 1];
    _swprintf_s_l(fstr, 3 + FLT_MANT_DIG - FLT_MIN_EXP, L"%f", locale, val);
#endif

    return fstr;
}

float IniFile::ToFloat(const std::wstring &value)
{
#ifdef _MSC_VER
    return _wtof_l(value.c_str(), locale);
#else
    return atof(WideToANSI(value).c_str());
#endif
}

std::wstring IniFile::FromDouble(double val)
{
#ifdef __MINGW32__
    //wchar_t fstr[3 + DBL_MANT_DIG - DBL_MIN_EXP + 1 + /* safety first */ 128];
    //swprintf_l(fstr, L"%Lf", val);
    char cfstr[3 + DBL_MANT_DIG - DBL_MIN_EXP + 1 + /* safety first */ 128];
    sprintf(cfstr, "%lf", val);
    int len = strlen(cfstr);
    wchar_t *w = ANSIToWide(cfstr, len);
    std::wstring fstr(w, len);
    delete[] w;

#else
    wchar_t fstr[3 + DBL_MANT_DIG - DBL_MIN_EXP + 1];
    _swprintf_s_l(fstr, 3 + DBL_MANT_DIG - DBL_MIN_EXP, L"%Lf", locale, val);
#endif
    return fstr;
}

double IniFile::ToDouble(const std::wstring &value)
{
#ifdef _MSC_VER
    return _wtof_l(value.c_str(), locale);
#else
    return atof(WideToANSI(value).c_str());
#endif
}

Rect IniFile::ToRect(const std::wstring& value)
{
    std::vector<std::wstring> coords;
    splitstring(value, L',', coords);
    if (coords.size() != 4)
        return Rect();

    Rect r;
    for (int ix = 0; ix < 4; ++ix)
    {
        int len = coords[ix].size();
        int pos = 0;
        while (len != 0 && coords[ix][len - 1] == L' ')
            --len;
        while (len != 0 && coords[ix][pos] == L' ')
            ++pos, --len;

        if (len == 0 || !StrToInt(coords[ix], ix == 0 ? r.left : ix == 1 ? r.top : ix == 2 ? r.right : r.bottom, pos, len))
            return Rect();
    }
    return r;
}

std::wstring IniFile::FromRect(const Rect& val)
{
    return IntToStr(val.left) + L", " + IntToStr(val.top) + L", " + IntToStr(val.right) + L", " + IntToStr(val.bottom);
}


/**
 * Values are stored in the IniFile object as strings. No conversion occurs when GetString() is
 * called.
 * \param sectionindex Index of the section containing the value. If this value is out of
 * range, an EOutOfRange exception is raised.
 * \param keyindex Index of the key holding the value. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \return String in the specified section and key.
 * \sa SetString(), GetInt(), GetBool(), GetFloat(), GetDouble()
 */
const std::wstring& IniFile::GetString(int sectionindex, int keyindex)
{
    CheckSectionKey(sectionindex, keyindex);
    auto &it = sections[sectionindex].second;
    std::advance(it, keyindex);
    return it->second;
}

/**
 * Values are stored in the IniFile object as strings. No conversion occurs when GetString() is
 * called.
 * \param sectionindex Index of the section containing the value. If this value is out of
 * range, an EOutOfRange exception is raised.
 * \param keyindex Index of the key holding the value. If this value is out of range, an EOutOfRange
 * exception is raised.
 * \param [out] result Variable receiving the string in the specified section and key.
 * \sa SetString(), GetInt(), GetBool(), GetFloat(), GetDouble()
 */
void IniFile::GetString(int sectionindex, int keyindex, std::wstring &result)
{
    result = GetString(sectionindex, keyindex);
}

/**
 * Values are stored in the IniFile object as strings. No conversion occurs when GetString() is
 * called.
 * \param sectionname Name of the section containing the value.
 * \param keyname Name of the key holding the value.
 * \param defaultvalue Value to return if the section or key was not found.
 * \return String in the specified section and key, or the value passed in \a defaultvalue if
 * either the section or the key was not found.
 * \sa SetString(), GetInt(), GetBool(), GetFloat(), GetDouble()
 */
const std::wstring& IniFile::GetString(const std::wstring& sectionname, const std::wstring& keyname, const std::wstring& defaultvalue)
{
    int ix = SectionIndex(sectionname);
    if (ix >= 0)
    {
        int vix = KeyIndex(ix, keyname);
        if (vix >= 0)
            return GetString(ix, vix);
    }
    return defaultvalue;
}

/**
 * Values are stored in the IniFile object as strings. No conversion occurs when GetString() is
 * called.
 * \param sectionname Name of the section containing the value.
 * \param keyname Name of the key holding the value.
 * \defaultvalue Value to return if the section or key was not found.
 * \param [out] result Variable receiving the string in the specified section and key, or the value
 * passed in \a defaultvalue if either the section or the key was not found.
 * \sa SetString(), GetInt(), GetBool(), GetFloat(), GetDouble()
 */
void IniFile::GetString(const std::wstring& sectionname, const std::wstring& keyname, std::wstring &result, const std::wstring& defaultvalue)
{
    result = GetString(sectionname, keyname, defaultvalue);
}

void IniFile::SetString(int sectionindex, int keyindex, const std::wstring& value)
{
    SetValue(sectionindex, keyindex, value);
}

void IniFile::SetString(const std::wstring &sectionname, const std::wstring &keyname, const std::wstring& value)
{
    AddValue(sectionname, keyname, value);
}

int IniFile::GetInt(int sectionindex, int keyindex)
{
    return ToInt(GetString(sectionindex, keyindex));
}

int IniFile::GetInt(const std::wstring& sectionname, const std::wstring& keyname, int defaultvalue)
{
    int ix = SectionIndex(sectionname);
    if (ix >= 0)
    {
        int vix = KeyIndex(ix, keyname);
        if (vix >= 0)
            return GetInt(ix, vix);
    }
    return defaultvalue;
}

void IniFile::GetInt(int sectionindex, int keyindex, int &result)
{
    result = GetInt(sectionindex, keyindex);
}

void IniFile::GetInt(const std::wstring& sectionname, const std::wstring& keyname, int &result, int defaultvalue)
{
    result = GetInt(sectionname, keyname, defaultvalue);
}

void IniFile::SetInt(int sectionindex, int keyindex, int value)
{
    SetValue(sectionindex, keyindex, FromInt(value));
}

void IniFile::SetInt(const std::wstring &sectionname, const std::wstring &keyname, int value)
{
    AddValue(sectionname, keyname, FromInt(value));
}

bool IniFile::GetBool(int sectionindex, int keyindex)
{
    return ToBool(GetString(sectionindex, keyindex));
}

void IniFile::GetBool(int sectionindex, int keyindex, bool &result)
{
    result = GetBool(sectionindex, keyindex);
}

bool IniFile::GetBool(const std::wstring& sectionname, const std::wstring& keyname, bool defaultvalue)
{
    int ix = SectionIndex(sectionname);
    if (ix >= 0)
    {
        int vix = KeyIndex(ix, keyname);
        if (vix >= 0)
            return GetBool(ix, vix);
    }
    return defaultvalue;
}

void IniFile::GetBool(const std::wstring& sectionname, const std::wstring& keyname, bool &result, bool defaultvalue)
{
    result = GetBool(sectionname, keyname, defaultvalue);
}

void IniFile::SetBool(int sectionindex, int keyindex, bool value)
{
    SetValue(sectionindex, keyindex, FromBool(value));
}

void IniFile::SetBool(const std::wstring &sectionname, const std::wstring &keyname, bool value)
{
    AddValue(sectionname, keyname, FromBool(value));
}

float IniFile::GetFloat(int sectionindex, int keyindex)
{
    return ToFloat(GetString(sectionindex, keyindex));
}

void IniFile::GetFloat(int sectionindex, int keyindex, float &result)
{
    result = GetFloat(sectionindex, keyindex);
}

float IniFile::GetFloat(const std::wstring& sectionname, const std::wstring& keyname, float defaultvalue)
{
    int ix = SectionIndex(sectionname);

    if (ix >= 0)
    {
        int vix = KeyIndex(ix, keyname);
        if (vix >= 0)
            return GetFloat(ix, vix);
    }
    return defaultvalue;
}

void IniFile::GetFloat(const std::wstring& sectionname, const std::wstring& keyname, float &result, float defaultvalue)
{
    result = GetFloat(sectionname, keyname, defaultvalue);
}

void IniFile::SetFloat(int sectionindex, int keyindex, float value)
{
    SetValue(sectionindex, keyindex, FromDouble(value));
}

void IniFile::SetFloat(const std::wstring &sectionname, const std::wstring &keyname, float value)
{
    AddValue(sectionname, keyname, FromFloat(value));
}

double IniFile::GetDouble(int sectionindex, int keyindex)
{
    return ToDouble(GetString(sectionindex, keyindex));
}

void IniFile::GetDouble(int sectionindex, int keyindex, double &result)
{
    result = GetDouble(sectionindex, keyindex);
}

double IniFile::GetDouble(const std::wstring& sectionname, const std::wstring& keyname, double defaultvalue)
{
    int ix = SectionIndex(sectionname);

    if (ix >= 0)
    {
        int vix = KeyIndex(ix, keyname);
        if (vix >= 0)
            return GetDouble(ix, vix);
    }
    return defaultvalue;
}

void IniFile::GetDouble(const std::wstring& sectionname, const std::wstring& keyname, double &result, double defaultvalue)
{
    result = GetDouble(sectionname, keyname, defaultvalue);
}

void IniFile::SetDouble(int sectionindex, int keyindex, double value)
{
    SetValue(sectionindex, keyindex, FromDouble(value));
}

void IniFile::SetDouble(const std::wstring &sectionname, const std::wstring &keyname, double value)
{
    AddValue(sectionname, keyname, FromDouble(value));
}

Rect IniFile::GetRect(int sectionindex, int keyindex)
{
    return ToRect(GetString(sectionindex, keyindex));
}

void IniFile::GetRect(int sectionindex, int keyindex, Rect &result)
{
    result = ToRect(GetString(sectionindex, keyindex));
}

Rect IniFile::GetRect(const std::wstring& sectionname, const std::wstring& keyname, const Rect& defaultvalue)
{
    int ix = SectionIndex(sectionname);

    if (ix >= 0)
    {
        int vix = KeyIndex(ix, keyname);
        if (vix >= 0)
            return GetRect(ix, vix);
    }
    return defaultvalue;
}

void IniFile::GetRect(const std::wstring& sectionname, const std::wstring& keyname, Rect& result, const Rect& defaultvalue)
{
    result = GetRect(sectionname, keyname, defaultvalue);
}

void IniFile::SetRect(int sectionindex, int keyindex, const Rect& value)
{
    SetValue(sectionindex, keyindex, FromRect(value));
}

void IniFile::SetRect(const std::wstring &sectionname, const std::wstring &keyname, const Rect& value)
{
    AddValue(sectionname, keyname, FromRect(value));
}

/**
 * Flush() does nothing if the values stored in the object have not been changed since construction or after
 * the last Open(), Save() or Flush() call. If you want to force writing to the file, call Save() with the
 * associated file name.
 * \return Whether writing the ini file was successful. Returns \a true if there was nothing to be done.
 * The result is \a false if no file name is associated with the IniFile object.
 * \sa Open(), Save(), Clear()
 */
bool IniFile::Flush()
{
    if (fname.empty())
        return false;

    if (!changed)
        return true;

    //if (sections.empty() && !FileExists(fname))
    //{
    //    changed = false;
    //    return true;
    //}

    FileStream file(fname, std::ios_base::out | std::ios_base::trunc, enc);
    if (!file.is_open())
        return false;

    file.seekp(0);
    if (file.encoding() == feUTF8)
        file.writebom();

    for (auto it = sections.begin(); it != sections.end(); ++it)
    {
        file << L"[" << it->first << L"]" << std::endl;

        auto it3 = (it + 1 == sections.end() ? values.end() : (it + 1)->second);
        for (auto it2 = it->second; it2 != it3; ++it2)
            file << it2->first << L"=" << it2->second << std::endl;

        file << std::endl;
    }

    if (!file.fail())
        changed = false;

    return !changed;
}

/**
 * The IniFile object is associated with the passed file name if opening and reading from it was successful.
 * On error the values are deleted and the previous file name association is canceled.
 * You can use the constructor expecting a file name argument to create an IniFile which reads from the
 * file upon construction. Call Disconnect() after opening the file if changes should not be written
 * back to the file.
 * \param filename The path to the ini file which will be associated with this object.
 * \param encoding Specify the encoding to be used when sparsing the ini file when reading or writing.
 * Incorrect encoding when reading ini files can result in corrupted data.
 * \return \a true if opening and reading the file was successful.
 * \remark Successful reading does not mean that data was correctly retrieved from the file, only that
 * reading the contents of the file stream did not fail. If the encoding is incorrect, this could result
 * in corrupted sections and values.
 * \sa IniFile(const std::wstring&,FileStreamEncoding), Save(), Disconnect()
 */
bool IniFile::Open(const std::wstring& filename, FileStreamEncoding encoding)
{
    changed = false;
    fname = std::wstring();
    sections.resize(0);
    values.resize(0);

    if (!ValidFileName(filename, true))
        return false;

    fname = filename;
    enc = encoding;

    FileStream file(filename, std::ios_base::in, encoding);

    if (file.fail())
        return false;

    if (encoding == feUTF8)
        file.skipbom();

    int sectionindex = -1;
    std::wstring line;
    while (GetLine(file, line))
    {
        if (line.empty() || line[0] == L';') // Skip empty lines or comments.
            continue;

        // Find the first non-space character in the line.
        size_t linefirst = line.find_first_not_of(_trim_characters);
        if (linefirst == std::wstring::npos)
            continue;

        if (line[linefirst] == L'[') // Supposedly a section line. If the section name is incorrect, ignore the values after it till a correct section is found.
        {
            size_t sectionend = line.find(L']', linefirst + 1);
            if (sectionend == std::wstring::npos || sectionend == linefirst + 1 || line.find_first_not_of(_trim_characters, sectionend + 1) != std::wstring::npos) // No closing bracket, empty section name, or garbage after closing bracket.
            {
                sectionindex = -1;
                continue;
            }

            line = trim(line.substr(linefirst + 1, sectionend - linefirst - 1));
            if (line.empty()) // Nothing written between the [] brackets.
            {
                sectionindex = -1;
                continue;
            }

            sectionindex = InnerSectionIndex(GenToLower(line));
            if (sectionindex < 0)
            {
                sections.push_back(SectionPair(line, values.end()));
                sectionindex = sections.size() - 1;
            }
            continue;
        }

        if (sectionindex < 0)
            continue;

        size_t eq = line.find(L'=', linefirst);
        if (eq == linefirst || eq == std::wstring::npos)
            continue;

        std::wstring keyname = line.substr(linefirst, line.find_last_not_of(_trim_characters, eq - 1) - linefirst + 1);
        AddValue(sectionindex, keyname, line.substr(eq + 1));
    }

    changed = false;

    if (!file.is_open() || file.fail())
    {
        fname = std::wstring();
        sections.resize(0);
        values.resize(0);
        file.close();
        return false;
    }
    return true;
}

/**
 * Saving causes the object to be associated with the passed file name. Call Disconnect() after Save() if you don't
 * want to write any further changes to the file. Calling Flush() has the same effect if the same file name has already
 * been associated with the object by a previous call to Save(), Open() or by the constructor, and the object contains
 * changed values. Unlike Flush() which does nothing in unchanged objects, Save() forces writing to the file.
 * \param filename The path to the ini file which will be associated with this object.
 * \param encoding Specify the encoding to be used when the values are written to the ini file.
 * \return Whether the operation has been successful.
 * \sa Flush(), Open(), Disconnect()
 */
bool IniFile::Save(const std::wstring& filename, FileStreamEncoding encoding)
{
    if (!ValidFileName(filename, true))
        return false;
    if (!changed && enc == encoding && filename == fname)
        return true;

    fname = filename;
    enc = encoding;
    changed = true;
    return Flush();
}


}
/* End of NLIBNS */

