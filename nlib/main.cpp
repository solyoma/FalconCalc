#include "stdafx_zoli.h"
#include "application.h"

#ifdef _MSC_VER
#pragma comment(lib,"ComCtl32.lib")
#pragma comment(lib,"GdiPlus.lib")
#pragma comment(lib,"Uxtheme.lib")
#pragma comment(lib,"urlmon.lib")
#ifdef DESIGNING
#pragma comment(lib,"zlibwapi.lib")
#endif
#endif

#ifdef __MINGW32__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmain"
int main(int argc, wchar_t *argv[]);
#pragma GCC diagnostic pop

extern "C"
{
    ImageList_ReadEx_T ImageList_ReadEx_REDEFINED = NULL;
    ImageList_WriteEx_T ImageList_WriteEx_REDEFINED = NULL;
    CopyStgMedium_T CopyStgMedium_REDEFINED = NULL;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else

int main(int argc, wchar_t *argv[]);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#endif
{
    using namespace NLIBNS;


    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef __MINGW32__
    HMODULE Comctl32libHandle = LoadLibrary(L"comctl32.dll");
    HMODULE UrlmonlibHandle = LoadLibrary(L"urlmon.dll");
    ImageList_ReadEx_REDEFINED = (ImageList_ReadEx_T)GetProcAddress(Comctl32libHandle, "ImageList_ReadEx");
    ImageList_WriteEx_REDEFINED = (ImageList_WriteEx_T)GetProcAddress(Comctl32libHandle, "ImageList_WriteEx");
    CopyStgMedium_REDEFINED = (CopyStgMedium_T)GetProcAddress(UrlmonlibHandle, "CopyStgMedium");
#endif

    Application::SetInstance(hInstance);
    Application::SetCmdShow(nCmdShow);
    Application::InitValues();
    int argcnt;
    LPWSTR *argcmd = CommandLineToArgvW(GetCommandLine(), &argcnt);
    Application::SetArgsCnt(argcnt);
    for (int ix = 0; ix < argcnt; ++ix)
        Application::AddArgsVar(argcmd[ix]);
    GetLastError(); // Clear this.

    _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);

    int r = main(argcnt, argcmd);
    LocalFree(argcmd);
    return r;
}
