#include <set>
#include "stdafx_zoli.h"

using namespace nlib;
#undef max
#include "SmartString.h"
using namespace SmString;

#include "LongNumber.h"
using namespace LongNumber;


#include "mainForm.h"

void Initialize()
{
    frmMain = new TfrmMain();
    frmMain->Show();
}

int main(int argc, char **argv)
{
	return Application::GetInstance()->Run();
}

