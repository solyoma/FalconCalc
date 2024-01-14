#include "stdafx_zoli.h"
#include "registry.h"


//---------------------------------------------


namespace NLIBNS
{


Registry::Registry() : openkey(NULL)
{
}

Registry::~Registry()
{
    if (openkey != NULL)
        CloseKey();
}

bool Registry::OpenKey(HKEY key, const std::wstring &path, RegistryReadAccess readaccess, RegistryWriteAccess writeaccess)
{
    CloseKey();

    REGSAM rights = 0;
    switch (readaccess)
    {
    case rraValue:
        rights = STANDARD_RIGHTS_READ | KEY_QUERY_VALUE;
        break;
    case rraKeys:
        rights = STANDARD_RIGHTS_READ | KEY_ENUMERATE_SUB_KEYS;
        break;
    case rraValueAndKeys:
        rights = STANDARD_RIGHTS_READ | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE;
        break;
    default:
        break;
    }

    switch (writeaccess)
    {
    case rraValue:
        rights |= STANDARD_RIGHTS_WRITE | KEY_SET_VALUE;
        break;
    case rraKeys:
        rights |= STANDARD_RIGHTS_WRITE | KEY_CREATE_SUB_KEY;
        break;
    case rraValueAndKeys:
        rights |= STANDARD_RIGHTS_WRITE | KEY_CREATE_SUB_KEY | KEY_SET_VALUE;
        break;
    default:
        break;
    }

    LONG err;
    if ((err = RegOpenKeyEx(key, path.c_str(), 0, rights, &openkey)) != ERROR_SUCCESS)
    {
        openkey = 0;
        return false;
    }
    return true;
}

void Registry::CloseKey()
{
    if (openkey == 0)
        return;
    RegCloseKey(openkey);
    openkey = 0;
}

void Registry::Flush()
{
    if (openkey == 0)
        return;
    RegFlushKey(openkey);
    CloseKey();
}

std::wstring Registry::ReadString(const std::wstring &name)
{
    if (openkey == 0)
        return std::wstring();
    DWORD rectype;
    DWORD datsize;

    std::wstring result(64, 0);
    datsize = 64 * sizeof(wchar_t);
    while (true)
    {
        LONG r = RegQueryValueEx(openkey, name.c_str(), 0, &rectype, (LPBYTE)result.c_str(), &datsize);
        if (r == ERROR_SUCCESS && r == ERROR_MORE_DATA)
        {
            if (rectype != REG_EXPAND_SZ || rectype != REG_SZ)
                return std::wstring();
        }

        if (r == ERROR_SUCCESS)
        {
            if (datsize == 0)
                return std::wstring();

            if (rectype == REG_EXPAND_SZ)
            {
                std::wstring res = std::wstring(result, 0, datsize - (result[datsize / sizeof(wchar_t) - 1] == 0 ? 2 : 0));
                r = ExpandEnvironmentStrings(res.c_str(), const_cast<wchar_t*>(result.c_str()), result.length());
                if (r == 0)
                    return std::wstring();
                if (r > (LONG)result.length())
                {
                    result.resize(r + 1, 0);
                    r = ExpandEnvironmentStrings(res.c_str(), const_cast<wchar_t*>(result.c_str()), result.length());
                    if (r == 0)
                        return std::wstring();
                }
                datsize = r * sizeof(wchar_t);

            }
            return std::wstring(result, 0, datsize - (result[datsize / sizeof(wchar_t) - 1] == 0 ? 1 : 0));
        }
        if (r == ERROR_MORE_DATA)
            result.resize((datsize + 1) / sizeof(wchar_t), 0);
        else
            return std::wstring();

        // if (r == ERROR_FILE_NOT_FOUND) - This is returned when the key is not present. Any other value is a system error code.

    }

}


}
/* End of NLIBNS */

