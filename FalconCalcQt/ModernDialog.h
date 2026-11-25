#pragma once

#ifndef MODERNDIALOG_H
#define MODERNDIALOG_H

#include <QString>
#include <QWidget>
#include <QLabel>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDialog>
#include <QFontDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QWindow>
#include <QMouseEvent>

enum ModernDialogButtons{ NoButtons, MinimizeButton, CloseButton};

struct ModernDialogTitleData
{
	ModernDialogTitleData(int buttons = 0) : buttons(buttons){}
	//ModernDialogTitleData& operator=(const ModernDialogTitleData& o)
	//{
	//	buttons = o.buttons ;
	//	text = o.text ;
	//	color = o.color ;
	//	backgroundColor = o.backgroundColor ;
	//	font = o.font ;
	//	alignment == o.alignment;
	//	return *this;
	//}

	bool operator==(const ModernDialogTitleData& o) const
	{
		return buttons == o.buttons &&
			text == o.text &&
			color == o.color &&
			backgroundColor == o.backgroundColor &&
			font == o.font &&
			alignment == o.alignment;
	}

	QString text, 
			color, 
			backgroundColor;
	QFont font = QFont("Arial", 12, 700, false);
	Qt::Alignment alignment = Qt::AlignHCenter;
	int buttons = 0;
};

struct ModernDialogResizeData
{
	ModernDialogResizeData(bool isResizable = false) : isResizable(isResizable) 
	{
		;
	}
	bool operator==(const ModernDialogResizeData& o) const
	{
		return
		isResizable			== o.isResizable		 &&
		resizeBorderWidth	== o.resizeBorderWidth &&
		minSizeX			== o.minSizeX			 &&
		minSizeY			== o.minSizeY			 &&
		maxSizeX			== o.maxSizeX			 &&
		maxSizeY			== o.maxSizeY			 ;

	}
	bool isResizable = false;
	int resizeBorderWidth = 6;
	int minSizeX = 100, minSizeY = 100;
	int maxSizeX = 16777215, maxSizeY = 16777215;
};

struct ModernDialogParams : public ModernDialogTitleData, public ModernDialogResizeData
{
	ModernDialogParams(bool hasTitleBar = true, int buttons = NoButtons, 
						ModernDialogResizeData resizeData = ModernDialogResizeData())
					:	ModernDialogTitleData(buttons), hasTitleBar(hasTitleBar) 
	{
		ModernDialogResizeData::operator==(resizeData);
	}
	ModernDialogParams(const ModernDialogParams& o) : ModernDialogTitleData(o), ModernDialogResizeData(o)
	{
		hasTitleBar = o.hasTitleBar;
	}
	ModernDialogParams operator=(const ModernDialogParams& o)
	{
		hasTitleBar = o.hasTitleBar;
		ModernDialogTitleData::operator=(o);
		ModernDialogResizeData::operator=(o);
		return *this;
	}
	bool operator==(const ModernDialogParams& o) const
	{
		return hasTitleBar == o.hasTitleBar &&
			ModernDialogTitleData::operator==(o) &&
			ModernDialogResizeData::operator==(o);
	}
	bool hasTitleBar;

};

class ModernTitleBar : public QWidget
{
	Q_OBJECT
public:
	explicit ModernTitleBar(const ModernDialogTitleData &title, QWidget* parent = nullptr);
	ModernTitleBar(const ModernTitleBar& o)  { *this = o; };
	ModernTitleBar(const ModernTitleBar&& o) noexcept
	{
		_title = o._title;
		_pTitleLabel	 = o._pTitleLabel;	   delete _pTitleLabel	   	;	 _pTitleLabel	  = nullptr;
		_pCloseButton	 = o._pCloseButton;	   delete _pCloseButton		;	 _pCloseButton	  = nullptr;
		_pMinimizeButton = o._pMinimizeButton; delete _pMinimizeButton	;	 _pMinimizeButton = nullptr;
	}
	~ModernTitleBar() { delete _pTitleLabel; delete _pCloseButton; delete _pMinimizeButton; }
	ModernTitleBar& operator=(const ModernTitleBar& o)
	{
		_title = o._title;
		_pTitleLabel = new QLabel(o._pTitleLabel);
		_pCloseButton = new QPushButton(o._pCloseButton);
		_pMinimizeButton = new QPushButton(o._pMinimizeButton);
		return *this;
	}

	void SetTitleText(const QString& title);
	void SetFont(const QFont &font);
	void SetFont(const QString family, int size=-1, int weight = -1, bool italic = false);
	void SetColor(const QColor& color);
	void SetBackgroundColor(const QColor& color);
	QFont Font() const { return _pTitleLabel->font(); }
signals:
	void SignalClose();
	void SignalMinimize();
protected:
	void mousePressEvent(QMouseEvent* event) override;
	// void mouseMoveEvent(QMouseEvent* event) override; we'll be using startSytemMove()
private:
	ModernDialogTitleData _title;
	QLabel* _pTitleLabel = nullptr;			  // if these three are all nullptr, then the title bar is not visible
	QPushButton* _pCloseButton = nullptr,
			   * _pMinimizeButton = nullptr;

	// QPoint _dragPosition;			  we'll be using startSytemMove()
	// bool _dragging = false;
};

class ModernFontDialog;
class ModernColorDialog;

class ModernDialog : public QDialog
{
	Q_OBJECT

public:
	friend class ModernFontDialog;
	friend class ModernColorDialog;

public:
	explicit ModernDialog(const ModernDialogParams& params, QWidget* parent = nullptr);
	~ModernDialog() {}	// no need to delete anything, Qt does it automatically
	QWidget* ContentWidget() const { return _pContentWidget; }
	QSize FullSize() const 
	{ 
		return _pContentWidget->size().grownBy(_contentMargins);
	}
	void SetResizable(bool resizable);
	bool IsResizable() const { return _params.isResizable; }
	// SetDialogTitle only sets the title if the dialog was created with a title bar, but
	// can be called even when it wasn't. If any color is empty the one from the palette is used.
	void SetDialogTitle(const ModernDialogParams &title);
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
//	void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;
private:
	Qt::Edges _ResizeEdgesAt(const QPoint& pos) const;
	void _InstallResizeTracking(QWidget* widget);
	void _SetResizeCursor(QWidget* widget, Qt::Edges edges);

	ModernDialogParams _params;	// all data from title, resize and resize flag
	QWidget* _pContentWidget = nullptr,
		   * _pFrameWidget = nullptr;	
	QMargins _contentMargins = QMargins(2, 6, 6, 6);
	ModernTitleBar* _pTitleBar = nullptr;
	QVBoxLayout *_pMainLayout;	// allows us to change the outer margins when enabling or disabling resizing.
};

class ModernFontDialog : public ModernDialog
{
	Q_OBJECT

public:
	explicit ModernFontDialog(const QFont& initialFont,
		ModernDialogParams params,
		QWidget* parent = nullptr);

	QFont selectedFont() const;

private slots:
	void updatePreview();
	void accept() override;

private:
	QFontComboBox* _pFamily;
	QSpinBox* _pSize;
	QCheckBox* _pBold;
	QCheckBox* _pItalic;
	QCheckBox* _pUnderline;
	QLabel* _pPreview;

	QFont     _selectedFont;
};
#endif