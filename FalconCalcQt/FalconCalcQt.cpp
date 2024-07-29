#include "FalconCalcQt.h"

FalconCalc::LittleEngine* lengine = nullptr;


FalconCalcQt::FalconCalcQt(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

FalconCalcQt::~FalconCalcQt()
{}
