#include "HelpDialog.h"
#include <QScreen>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    #include <QDesktopWidget>
#endif
int HelpDialog::helpVisible = false;

HelpDialog::HelpDialog(QWidget* parent) : ModernDialog(ModernDialogParams(), parent)
{
	Ui::HelpDialogClass::setupUi(ContentWidget());
    resize(FullSize());

    QScreen* screen = QGuiApplication::primaryScreen();
    ++helpVisible;
    connect(okButton, &QPushButton::clicked, this, [this]() { accept(); });
}

HelpDialog::~HelpDialog()
{
    if(helpVisible)
        --helpVisible;
}

void HelpDialog::showEvent(QShowEvent *e)
{
    int w = screen()->geometry().width() ;
    int h = screen()->geometry().height();
    w -= geometry().width();
    h -= geometry().height();
    move(w/2, h/2);
}
