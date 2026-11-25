#include <QFormLayout> 

#include "ModernDialog.h"
#ifdef DEBUG
#include "qdebug.h"
#endif

// ---------------------------------------------------------
// ModernTitleBar
// ---------------------------------------------------------

ModernTitleBar::ModernTitleBar(const ModernDialogTitleData& title, QWidget* parent)
        : _title(title), QWidget(parent)
{
    setObjectName("titleBar");
    setFixedHeight(42);

    _pTitleLabel = new QLabel(_title.text, this);
    _pTitleLabel->setFont(title.font);
    //_pTitleLabel->SetColor(title.color);
    //_pTitleLabel->SetBackgroundColor(title.backgroundColor);
    _pTitleLabel->setObjectName("dialogTitle");

    // Let mouse events reach the title bar for dragging.
    _pTitleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);


    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 6, 4);
    layout->setSpacing(0);
    layout->addWidget(_pTitleLabel);

    if (_title.buttons)
    {
        layout->addStretch();
        // add buttons
        if (_title.buttons & ModernDialogButtons::MinimizeButton)
        {
            _pMinimizeButton = new QPushButton(QString::fromUtf8("_"), this);
            _pMinimizeButton->setObjectName("minimizeButton");
            _pMinimizeButton->setFixedSize(32, 32);
            _pMinimizeButton->setCursor(Qt::PointingHandCursor);
            _pMinimizeButton->setFocusPolicy(Qt::NoFocus);

            connect(_pMinimizeButton, &QPushButton::clicked,
                this, &ModernTitleBar::SignalMinimize);
            layout->addWidget(_pMinimizeButton);
        }
        if (_title.buttons & ModernDialogButtons::CloseButton)
        {
            _pCloseButton = new QPushButton(QString::fromUtf8("×"), this);
            _pCloseButton->setObjectName("closeButton");
            _pCloseButton->setFixedSize(32, 32);
            _pCloseButton->setCursor(Qt::PointingHandCursor);
            _pCloseButton->setFocusPolicy(Qt::NoFocus);

            connect(_pCloseButton, &QPushButton::clicked,
                this, &ModernTitleBar::SignalClose);
            layout->addWidget(_pCloseButton);

        }
    }


}

void ModernTitleBar::SetTitleText(const QString& title)
{
    _title.text = title;
    if(_pTitleLabel)
        _pTitleLabel->setText(title);
}
void ModernTitleBar::SetFont(const QFont& font)
{
    if (_pTitleLabel)
    {
        _pTitleLabel->setFont(_title.font);
    }
}

void ModernTitleBar::SetFont(const QString family, int pointSize, int weight, bool italic)
{ 
    _title.font = QFont(family, pointSize,  weight, italic);
    SetFont(_title.font); 
}
void ModernTitleBar::SetColor(const QColor& color) 
{ 
    _title.color = color.name();
    if (_pTitleLabel)
    {
        QPalette pal = _pTitleLabel->palette();
        pal.setColor(QPalette::WindowText, color);
        _pTitleLabel->setPalette(pal);
    }
}
void ModernTitleBar::SetBackgroundColor(const QColor& color) 
{ 
    _title.backgroundColor = color.name();
    if (_pTitleLabel)
    {
        QPalette pal = palette();
        pal.setColor(QPalette::Window, color);
        setPalette(pal);
    }
}

void ModernTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QWindow* handle = window()->windowHandle();

        if (handle && handle->startSystemMove())
        {
            event->accept();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}
//void ModernTitleBar::mousePressEvent(QMouseEvent* event)
//{
    //if (event->button() == Qt::LeftButton) 
    //{
    //    _dragging = true;
    //    _dragPosition =
    //        event->globalPos() -
    //        window()->frameGeometry().topLeft();

    //    event->accept();
    //}
//}

//void ModernTitleBar::mouseMoveEvent(QMouseEvent* event)
//{
//    if (_dragging &&
//        (event->buttons() & Qt::LeftButton)) {
//
//        window()->move(event->globalPos() - _dragPosition);
//        event->accept();
//    }
//}

// ---------------------------------------------------------
// ModernDialog
// ---------------------------------------------------------

ModernDialog::ModernDialog(const ModernDialogParams& params, QWidget* parent)
    : QDialog(parent),_params(params)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setObjectName("ModernDialog");

    _pContentWidget = new QWidget(this);
    _pContentWidget->setObjectName("DialogContent");

    _pFrameWidget = new QWidget(this);
    _pFrameWidget->setObjectName("DialogFrame");
    QVBoxLayout* frameLayout = new QVBoxLayout(_pFrameWidget);
    frameLayout->setContentsMargins(_contentMargins);
    frameLayout->setSpacing(0);

    if (params.hasTitleBar)
    {
        _pTitleBar = new ModernTitleBar((const ModernDialogTitleData&)params, this);
        frameLayout->addWidget(_pTitleBar);
		if (params.buttons & ModernDialogButtons::MinimizeButton)
            connect(_pTitleBar, &ModernTitleBar::SignalMinimize,
		        this, &ModernDialog::showMinimized);
		if (params.buttons & ModernDialogButtons::CloseButton)
            connect(_pTitleBar, &ModernTitleBar::SignalClose,
                this, &ModernDialog::reject);
    }

    frameLayout->addWidget(_pContentWidget, 1);

    _pMainLayout = new QVBoxLayout(this);
    _pMainLayout->setSpacing(0);
    _pMainLayout->addWidget(_pFrameWidget);
    SetResizable(_params.isResizable);

    setMouseTracking(true);
}

void ModernDialog::SetDialogTitle(const ModernDialogParams& newParams)
{
    if (newParams == _params)
        return;

    bool bTitleBar = _pTitleBar && _pTitleBar->isVisible();

    if( newParams.buttons == ModernDialogButtons::NoButtons)
    {
        if (_params.buttons != ModernDialogButtons::NoButtons)
        {
            _params.buttons = ModernDialogButtons::NoButtons;
            if (!_params.hasTitleBar && bTitleBar)
                _pTitleBar->hide();
        }
    }

    if (_pTitleBar)
    {
        _pTitleBar->SetTitleText(newParams.text);
        if (newParams.color.isEmpty())
        {
            QPalette pal = _pTitleBar->palette();
            pal.setColor(QPalette::WindowText, palette().color(QPalette::WindowText));
            _pTitleBar->setPalette(pal);
        }
        else
			_pTitleBar->SetColor(QColor(newParams.color));
        if (newParams.backgroundColor.isEmpty())
        {
            QPalette pal = _pTitleBar->palette();
            pal.setColor(QPalette::Window, palette().color(QPalette::Window));
            _pTitleBar->setPalette(pal);
        }
		else
            _pTitleBar->SetBackgroundColor(QColor(newParams.backgroundColor));
    }
      // must be the last
    if (newParams.hasTitleBar != _params.hasTitleBar)
    {
       if(_params.hasTitleBar)
       {
            _params.hasTitleBar = false;
            if (bTitleBar)
                _pTitleBar->hide();
       }    
    }
}

void ModernDialog::SetResizable(bool resizable)
{
    _params.isResizable = resizable;

    int m = resizable ? _params.resizeBorderWidth : 0;

    _pMainLayout->setContentsMargins(m, m, m, m);

    setMouseTracking(resizable);
    _InstallResizeTracking(_pFrameWidget);
    unsetCursor();

    if (resizable)
    {
        setMinimumSize(_params.minSizeX, _params.minSizeY);
        setMaximumSize(_params.maxSizeX, _params.maxSizeY);
    }
}

Qt::Edges ModernDialog::_ResizeEdgesAt(const QPoint& pos) const
{
    Qt::Edges edges;
    const QRect r = _pFrameWidget->geometry();
    const int b = _params.resizeBorderWidth;

    if (pos.x() <= r.left() + b)
        edges |= Qt::LeftEdge;
    else if (pos.x() >= r.right() - b)
        edges |= Qt::RightEdge;

    if (pos.y() <= r.top() + b)
        edges |= Qt::TopEdge;
    else if (pos.y() >= r.bottom() - b)
        edges |= Qt::BottomEdge;

    return edges;
}

void ModernDialog::_SetResizeCursor(QWidget* widget, Qt::Edges edges)
{
    if ((edges == (Qt::LeftEdge | Qt::TopEdge)) ||
        (edges == (Qt::RightEdge | Qt::BottomEdge)))
        widget->setCursor(Qt::SizeFDiagCursor);

    else if ((edges == (Qt::RightEdge | Qt::TopEdge)) ||
        (edges == (Qt::LeftEdge | Qt::BottomEdge)))
        widget->setCursor(Qt::SizeBDiagCursor);

    else if (edges & (Qt::LeftEdge | Qt::RightEdge))
        widget->setCursor(Qt::SizeHorCursor);

    else if (edges & (Qt::TopEdge | Qt::BottomEdge))
        widget->setCursor(Qt::SizeVerCursor);

    else
        widget->unsetCursor();
}

void ModernDialog::_InstallResizeTracking(QWidget* widget)
{
    widget->setMouseTracking(_params.isResizable);
    widget->installEventFilter(this);

    const auto children = widget->findChildren<QWidget*>();

    for (QWidget* child : children)
    {
        child->setMouseTracking(_params.isResizable);
        child->installEventFilter(this);
    }
}

bool ModernDialog::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget *>(watched);

    if (_params.isResizable && widget)
    {
        if (event->type() == QEvent::MouseMove)
        {
            auto *mouse = static_cast<QMouseEvent *>(event);

            QPoint pos = widget->mapTo(this, mouse->pos());

            _SetResizeCursor(widget, _ResizeEdgesAt(pos));
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
            auto *mouse = static_cast<QMouseEvent *>(event);

            if (mouse->button() == Qt::LeftButton)
            {
                QPoint pos = widget->mapTo(this, mouse->pos());
                Qt::Edges edges = _ResizeEdgesAt(pos);

                if (edges)
                {
                    if (windowHandle())
                        windowHandle()->startSystemResize(edges);

                    return true;
                }
            }
        }
        else if (event->type() == QEvent::Leave)
        {
            widget->unsetCursor();
        }
    }

    return QDialog::eventFilter(watched, event);
}

void ModernDialog::mousePressEvent(QMouseEvent* event)
{
    if (_params.isResizable && event->button() == Qt::LeftButton)
    {
        Qt::Edges edges = _ResizeEdgesAt(event->pos());

        if (edges)
        {
            if (windowHandle())
                windowHandle()->startSystemResize(edges);

            event->accept();
            return;
        }
    }

    QDialog::mousePressEvent(event);
}

//void ModernDialog::mouseMoveEvent(QMouseEvent* event)
//{
//    if (_params.isResizable)
//        _UpdateResizeCursor(event->pos());
//
//    QDialog::mouseMoveEvent(event);
//}

void ModernDialog::leaveEvent(QEvent* event)
{
    unsetCursor();
    QDialog::leaveEvent(event);
}
// ****************** ModernFontDialog ***************

ModernFontDialog::ModernFontDialog(const QFont& initialFont,
    ModernDialogParams params,
    QWidget* parent)
    : ModernDialog(params, parent)
{
    _pFamily = new QFontComboBox();
    _pSize = new QSpinBox();
    _pBold = new QCheckBox("Bold");
    _pItalic = new QCheckBox("Italic");
    _pUnderline = new QCheckBox("Underline");
    _pPreview = new QLabel("The quick brown fox jumps over the lazy dog");

    _pSize->setRange(6, 96);

    _pFamily->setCurrentFont(initialFont);
    _pSize->setValue(initialFont.pointSize() > 0 ?
        initialFont.pointSize() : 10);

    _pBold->setChecked(initialFont.bold());
    _pItalic->setChecked(initialFont.italic());
    _pUnderline->setChecked(initialFont.underline());

    auto* layout = new QVBoxLayout(_pContentWidget);

    auto* form = new QFormLayout();

    form->addRow("Font:", _pFamily);
    form->addRow("Size:", _pSize);

    layout->addLayout(form);
    layout->addWidget(_pBold);
    layout->addWidget(_pItalic);
    layout->addWidget(_pUnderline);

    layout->addSpacing(10);

    _pPreview->setMinimumHeight(80);
    _pPreview->setAlignment(Qt::AlignCenter);

    layout->addWidget(_pPreview);

    auto* buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel);

    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
        this, &ModernFontDialog::accept);

    connect(buttons, &QDialogButtonBox::rejected,
        this, &ModernFontDialog::reject);

    connect(_pFamily, &QFontComboBox::currentFontChanged,
        this, &ModernFontDialog::updatePreview);

    connect(_pSize, qOverload<int>(&QSpinBox::valueChanged),
        this, &ModernFontDialog::updatePreview);

    connect(_pBold, &QCheckBox::toggled,
        this, &ModernFontDialog::updatePreview);

    connect(_pItalic, &QCheckBox::toggled,
        this, &ModernFontDialog::updatePreview);

    connect(_pUnderline, &QCheckBox::toggled,
        this, &ModernFontDialog::updatePreview);

    updatePreview();
}

void ModernFontDialog::updatePreview()
{
    QFont font = _pFamily->currentFont();

    font.setPointSize(_pSize->value());
    font.setBold(_pBold->isChecked());
    font.setItalic(_pItalic->isChecked());
    font.setUnderline(_pUnderline->isChecked());

    _pPreview->setFont(font);
}
void ModernFontDialog::accept()
{
    _selectedFont = _pFamily->currentFont();

    _selectedFont.setPointSize(_pSize->value());
    _selectedFont.setBold(_pBold->isChecked());
    _selectedFont.setItalic(_pItalic->isChecked());
    _selectedFont.setUnderline(_pUnderline->isChecked());

    ModernDialog::accept();
}
QFont ModernFontDialog::selectedFont() const
{
    return _selectedFont;
}