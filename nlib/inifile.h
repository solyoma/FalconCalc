#pragma once

#include "filestream.h"


namespace NLIBNS
{


/// Type for opening and saving values in classical initialization files.
/**
 * An ini file is a file storing lines in the format of key=value, split into sections.
 * IniFile objects hold the keys and values in memory until the data is flushed or the object
 * is destroyed. Open an ini file by passing its path to the constructor or with the Open()
 * method. The file is not kept open, but opened briefly before reading and writing. If the
 * file exists, its sections are read before the file handle is closed. After adding and
 * changing the contents, to save the file to disk, either call Flush() or destroy the ini
 * file object. Call Save() to save the ini file under a new name. Check the value returned
 * by Changed() to determine whether flushing or destroying the object will case the values
 * to be saved. The IniFile object can be emptied with the Clean() method, which removes all
 * sections and keys.
 *
 * An IniFile object can also be used for storing string values accessed by keys without
 * file storage. Call the constructor without arguments to create a memory only IniFile.
 * Saving the values to file is possible with the Save() method, which associates the object
 * with the file name. Call Disconnect() to cancel the association to avoid writing to a file
 * when flushing or destroying the object.
 *
 * For accessing values, both the section and the key must be specified, either by index or
 * key. Section names can't be empty or consisting only of whitespace, and can't contain
 * square bracket characters. Key names can't be empty or consist only of whitespace, and
 * can't contain the equal sign. The key names can't start with the opening square bracket or
 * semicolon. If incorrect section or key names are passed to a function, the function returns
 * without changing the IniFile object. Passing an incorrect index raises the EOutOfRange
 * exception.
 *
 * Internally the values are stored in strings and converted to the wanted type, when the
 * type's getter function is called. Supported types and their getter/setter functions are:
 *    * wstring: GetString(), SetString()
 *    * int: GetInt(), SetInt()
 *    * bool: GetBool(), SetBool()
 *    * float: GetFloat(), SetFloat()
 *    * double: GetDouble, SetDouble()
 *
 * The getter functions have two kinds. One kind returns the wanted value, the other puts the
 * result in the passed variable reference. Both kinds have a type which expects the section
 * and key indexes, and another which expects their names instead. To retrieve the index or
 * name of sections and keys, use SectionIndex(), SectionName(), KeyIndex() and KeyName()
 * respectively.
 *
 * Values from one IniFile object can be copied to another object with the CopyValues() method.
 * Call CopySection() to only copy individual sections between objects or to create a section
 * with the same keys under another section name. RenameSection() changes the name of a section.
 *
 * \remark The section names and keys are looked up in a case insensitive manner, using a general
 * lower case function that doesn't take the current locale into account, to make the ini file
 * compatible on different locales. Numeral values are converted to string using the en-US locale
 * for the same reason. For ASCII ini files, the values are using the current locale when reading
 * strings only.
 */
class IniFile
{
private:
    // Stream used in reading and writing to the ini file.
    typedef std::pair<std::wstring, std::wstring>   KeyValue; // <key, value>
    typedef std::list<KeyValue>         ValueList;
    typedef ValueList::iterator       ValueIterator;
    typedef std::pair<std::wstring, ValueIterator>  SectionPair; // <section name, iterator pointing to the first value in the section in values>
    typedef std::vector<SectionPair>    SectionVector;
    typedef SectionVector::iterator     SectionIterator;
    ValueList values;
    SectionVector sections;

    FileStreamEncoding enc;
    std::wstring fname;
    bool changed;

#ifdef _MSC_VER
    _locale_t locale;
#endif

    int InnerSectionIndex(const std::wstring &sectionname); // Returns the index of the section with the given name. The name must be trimmed and in lower case.
    int InnerKeyIndex(int sectionindex, const std::wstring &keyname); // Returns the index of the value with the given name in a section. The name must be trimmed and in lower case.

    int AddSection(std::wstring sectionname); // Adds a section if it does not exist. Returns the index of the existing or the added section. The name is trimmed before it is added.

    void SetValue(int sectionix, int keyix, const std::wstring& value);
    void AddValue(int sectionix, const std::wstring& key, const std::wstring& value);
    void AddValue(std::wstring section, std::wstring key, const std::wstring& value);
    std::wstring FromInt(int val);
    int ToInt(const std::wstring& value);
    std::wstring FromBool(bool val);
    bool ToBool(std::wstring value);
    std::wstring FromFloat(float val);
    float ToFloat(const std::wstring& value);
    std::wstring FromDouble(double val);
    double ToDouble(const std::wstring &value);
    Rect ToRect(const std::wstring& value);
    std::wstring FromRect(const Rect& val);

    // Checker functions that throw an EOutOfRange exception when the passed value is invalid.
    void CheckSection(int index);
    void CheckSectionKey(int sectionindex, int keyindex);
    // Checker functions that return whether the passed value is valid for indexes or names.
    bool CheckSectionName(const std::wstring& sectionname);
    bool CheckKeyName(const std::wstring& keyname);

    void CopySection(IniFile &dest, SectionIterator it, const std::wstring &destsection, bool fullreplace);

    bool GetLine(FileStream &file, std::wstring &line); // Retreives a line from file and returns true if either anything was read or eof hasn't been found yet.
public:
    /* constructor */ IniFile(); ///< Constructor creating an IniFile not associated to a file name.
    /* constructor */ IniFile(const std::wstring& filename, FileStreamEncoding encoding = feUTF8); ///< Constructor creating an IniFile associated to a file name.
    /* destructor */ virtual ~IniFile(); ///< Destructor that calls Flush() before the object is destroyed.

    void Disconnect(); ///< Cancels the association between the IniFile object and its file storage.
    void Clear(); ///< Deletes all sections from memory, resulting in an empty IniFile object.
    bool Flush(); ///< Writes the values to the ini file associated with this object if the object has changed values.
    bool Open(const std::wstring& filename, FileStreamEncoding encoding = feUTF8); ///< Clears the object and reads the values from the file with the passed file name.
    bool Save(const std::wstring& filename, FileStreamEncoding encoding = feUTF8); ///< Writes the values of the IniFile object to a file with the passed file name.

    bool Changed(); ///< Determines whether there are changed values in the object.
    const std::wstring& FileName(); ///< Returns the file name associated with the IniFile object.
    FileStreamEncoding Encoding(); ///< Returns the encoding last used for opening or saving an ini file.

    void CopyValues(IniFile &dest, bool fullreplace = true); ///< Copies the stored values with all sections and keys to another IniFile object.
    void CopySection(IniFile &dest, int sectionindex, const std::wstring &destsection, bool fullreplace = true); ///< Copies the values of one section to another section in this or another IniFile object.
    void CopySection(IniFile &dest, const std::wstring &sectionname, const std::wstring &destsection, bool fullreplace = true); ///< Copies the values of one section to another section in this or another IniFile object.
    void RenameSection(int sectionindex, const std::wstring &newsectionname); ///< Changes the name of a section.
    void RenameSection(const std::wstring &sectionname, const std::wstring &newsectionname); ///< Changes the name of a section.

    int SectionCount(); ///< Returns the number of sections currently in memory.
    int SectionIndex(const std::wstring &sectionname); ///< Returns the index of a section by name.
    const std::wstring& SectionName(int sectionindex); ///< Returns the name of a section at a given index.
    void DeleteSection(int sectionindex); ///< Deletes all values in a section at a given index.
    void DeleteSection(const std::wstring& sectionname); ///< Deletes all values in a section by name.
    int KeyCount(int sectionindex); ///< Returns the number of keys under a section currently in memory.
    int KeyCount(const std::wstring& sectionname); ///< Returns the number of keys under a section currently in memory.
    int KeyIndex(int sectionindex, const std::wstring &keyname); ///< Returns the index of a key by name.
    int KeyIndex(const std::wstring& sectionname, const std::wstring& keyname); ///< Returns the index of a key by name.
    const std::wstring& KeyName(int sectionindex, int keyindex); ///< Returns the name of a key at a given index.
    const std::wstring KeyName(const std::wstring &sectionname, int keyindex); ///< Returns the name of a key at a given index.
    void DeleteKey(int sectionindex, int keyindex); ///< Deletes a key at a given index.
    void DeleteKey(const std::wstring& sectionname, const std::wstring& keyname); ///< Deletes a key by name.

    const std::wstring& GetString(int sectionindex, int keyindex); ///< Get a value as string.
    const std::wstring& GetString(const std::wstring& sectionname, const std::wstring& keyname, const std::wstring& defaultvalue); ///< Get a value as string.
    int GetInt(int sectionindex, int keyindex); ///< Get a value as int.
    int GetInt(const std::wstring& sectionname, const std::wstring& keyname, int defaultvalue); ///< Get a value as int.
    bool GetBool(int sectionindex, int keyindex); ///< Get a value as bool.
    bool GetBool(const std::wstring& sectionname, const std::wstring& keyname, bool defaultvalue); ///< Get a value as bool.
    float GetFloat(int sectionindex, int keyindex); ///< Get a value as float.
    float GetFloat(const std::wstring& sectionname, const std::wstring& keyname, float defaultvalue); ///< Get a value as float.
    double GetDouble(int sectionindex, int keyindex); ///< Get a value as double.
    double GetDouble(const std::wstring& sectionname, const std::wstring& keyname, double defaultvalue); ///< Get a value as double.
    Rect GetRect(int sectionindex, int keyindex); ///< Get a value as a rectangle.
    Rect GetRect(const std::wstring& sectionname, const std::wstring& keyname, const Rect& defaultvalue); ///< Get a value as a rectangle.

    void GetString(int sectionindex, int keyindex, std::wstring &result);
    void GetString(const std::wstring& sectionname, const std::wstring& keyname, std::wstring &result, const std::wstring& defaultvalue);
    void SetString(int sectionindex, int keyindex, const std::wstring& value);
    void SetString(const std::wstring &sectionname, const std::wstring &keyname, const std::wstring& value);
    void GetInt(int sectionindex, int keyindex, int &result);
    void GetInt(const std::wstring& sectionname, const std::wstring& keyname, int &result, int defaultvalue);
    void SetInt(int sectionindex, int keyindex, int value);
    void SetInt(const std::wstring &sectionname, const std::wstring &keyname, int value);
    void GetBool(int sectionindex, int keyindex, bool &result);
    void GetBool(const std::wstring& sectionname, const std::wstring& keyname, bool &result, bool defaultvalue);
    void SetBool(int sectionindex, int keyindex, bool value);
    void SetBool(const std::wstring &sectionname, const std::wstring &keyname, bool value);
    void GetFloat(int sectionindex, int keyindex, float &result);
    void GetFloat(const std::wstring& sectionname, const std::wstring& keyname, float &result, float defaultvalue);
    void SetFloat(int sectionindex, int keyindex, float value);
    void SetFloat(const std::wstring &sectionname, const std::wstring &keyname, float value);
    void GetDouble(int sectionindex, int keyindex, double &result);
    void GetDouble(const std::wstring& sectionname, const std::wstring& keyname, double &result, double defaultvalue);
    void SetDouble(int sectionindex, int keyindex, double value);
    void SetDouble(const std::wstring &sectionname, const std::wstring &keyname, double value);
    void GetRect(int sectionindex, int keyindex, Rect &result);
    void GetRect(const std::wstring& sectionname, const std::wstring& keyname, Rect& result, const Rect& defaultvalue);
    void SetRect(int sectionindex, int keyindex, const Rect& value);
    void SetRect(const std::wstring &sectionname, const std::wstring &keyname, const Rect& value);
};


}
/* End of NLIBNS */

