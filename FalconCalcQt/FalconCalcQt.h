#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FalconCalcQt.h"

namespace FalconCalc {
    class LittleEngine;
}

class FalconCalcQt : public QMainWindow
{
    Q_OBJECT

public:
    FalconCalcQt(QWidget *parent = nullptr);
    ~FalconCalcQt();

private:
    Ui::FalconCalcQtClass ui;
};
