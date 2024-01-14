#pragma once


namespace NLIBNS
{


enum RegistryReadAccess { rraNone, rraValue, rraKeys, rraValueAndKeys };
enum RegistryWriteAccess { rwaNone, rwaValue, rwaKeys, rwaValueAndKeys };
class Registry
{
private:
    HKEY openkey; // Handle to a registry key currently open.
public:
    Registry();
    ~Registry();

    bool OpenKey(HKEY key, const std::wstring &path, RegistryReadAccess readaccess, RegistryWriteAccess writeaccess); // Call CloseKey after reading from or writing to the specified registry path. Calls CloseKey before opening a new one if a key was already open. Returns false on failure.
    void CloseKey(); // Closes the last opened key.
    void Flush(); // Causes changes made to the registry to be written to disk, and then closes the key. Values are not written immediately after CloseKey.

    std::wstring ReadString(const std::wstring &name);
};


}
/* End of NLIBNS */

