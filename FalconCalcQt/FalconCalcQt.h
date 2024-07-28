#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FalconCalcQt.h"

class FalconCalcQt : public QMainWindow
{
    Q_OBJECT

public:
    FalconCalcQt(QWidget *parent = nullptr);
    ~FalconCalcQt();

private:
    Ui::FalconCalcQtClass ui;
};
