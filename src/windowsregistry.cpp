#include "logicalaccess/windowsregistry.hpp"

#ifdef _WIN32

#include "windows.h"
#include "Winreg.h"
#include "logicalaccess/logs.hpp"

using namespace logicalaccess;

WindowsRegistry::WindowsRegistry()
{
}


WindowsRegistry::~WindowsRegistry()
{
}

bool WindowsRegistry::writeBinary(const std::string &keypath,
                                  const std::string &keyname,
                                  const ByteVector &data)
{
    HKEY hKey;
    LPCTSTR sk = TEXT(keypath.c_str());

    LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sk, 0, KEY_ALL_ACCESS, &hKey);
    if (ret == ERROR_SUCCESS)
    {
        LOG(INFOS) << "Success opening registry key.";
    }
    else
    {
        LOG(ERRORS) << "Error opening registry key.";
        return false;
    }

    LPCTSTR value = TEXT(keyname.c_str());
    ret = RegSetValueEx(hKey, value, 0, REG_BINARY, (CONST BYTE *)&data[0],
                        static_cast<DWORD>(data.size()));
    if (ret == ERROR_SUCCESS)
    {
        LOG(INFOS) << "Success writing the registry key.";
    }
    else
    {
        LOG(ERRORS) << "Error writing the registry key. Error code: " << ret;
        return false;
    }

    ret = RegCloseKey(hKey);
    if (ret != ERROR_SUCCESS)
    {
        LOG(ERRORS) << "Error closing the registry key. Error code: " << ret;
        return false;
    }
    return true;
}


bool WindowsRegistry::readBinary(const std::string &keypath,
                                 const std::string &keyname,
                                 ByteVector &out)
{
    HKEY hKey;
    LPCTSTR sk = TEXT(keypath.c_str());

    LONG ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sk, 0, KEY_READ, &hKey);
    if (ret == ERROR_SUCCESS)
    {
        LOG(INFOS) << "Success opening registry key.";
    }
    else
    {
        LOG(ERRORS) << "Error opening registry key. Error code = " << ret;
        return false;
    }
    LPCTSTR value = TEXT(keyname.c_str());

    ByteVector tmpbuf(100);
    DWORD size = static_cast<DWORD>(tmpbuf.size());
    //DWORD flags = RRF_RT_ANY;
    ret = RegQueryValueEx(hKey, value, nullptr, nullptr, &tmpbuf[0], &size);
    tmpbuf.resize(size);
    if (ret == ERROR_SUCCESS)
    {
        out = tmpbuf;
        LOG(INFOS) << "Success reading the registry key.";
    }
    else if (ret == ERROR_FILE_NOT_FOUND)
    {
        // Not found actually means that all techno are enabled.
        out = {1, 2, 3, 4, 5, 6};
        return true;
    }
    else
    {
        LOG(ERRORS) << "Error reading the registry key. Error code: " << ret;
        return false;
    }

    ret = RegCloseKey(hKey);
    if (ret != ERROR_SUCCESS)
    {
        LOG(ERRORS) << "Error closing the registry key. Error code: " << ret;
    }
    return true;
}

#endif
