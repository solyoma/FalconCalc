#include "HelpDialog.h"
#include <QScreen>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    #include <QDesktopWidget>
#endif
int HelpDialog::helpVisible = false;

HelpDialog::HelpDialog(QWidget* parent) :QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);
//    ui.lblHelptext->setText(sHelpText);
    QScreen* screen = QGuiApplication::primaryScreen();
    move(screen->geometry().center() - geometry().center());
    ++helpVisible;
}
HelpDialog::~HelpDialog()
{
    if(helpVisible)
        --helpVisible;
}
