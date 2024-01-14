#include "stdafx_zoli.h"
#include "stdafx_lc.h"

using namespace nlib;
#ifdef _OLD_CODE
    #include "numconsts.h"
    #include "numconv.h"
#else
    #include "LongNumber.h"
    using namespace LongNumber;
#endif

#include "main.h"

void Initialize()
{
    frmMain = new TfrmMain();
    frmMain->Show();
}

int main(int argc, char **argv)
{
	return Application::GetInstance()->Run();
}

