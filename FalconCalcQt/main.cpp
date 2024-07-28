#include "FalconCalcQt.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FalconCalcQt w;
    w.show();
    return a.exec();
}
