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
struct ModernDialogTitle
{
	ModernDialogTitle(int buttons = 0) : buttons(buttons){}

	QString text, 
			color, 
			backgroundColor;
	QFont font;
	int buttons;
};

struct ModernDialogResizeData
{
	ModernDialogResizeData(bool isResizable = false) : isResizable(isResizable) 
	{
		;
	}
	bool isResizable = false;
	int resizeBorderWidth = 6;
	int minSizeX = 100, minSizeY = 100;
	int maxSizeX = 16777215, maxSizeY = 16777215;
};

struct ModernDialogParams : public ModernDialogTitle, public ModernDialogResizeData
{
	ModernDialogParams(bool haveTitleBar = true, int buttons = NoButtons, ModernDialogResizeData resizeData = ModernDialogResizeData()):
		ModernDialogTitle(buttons), hasTitleBar(haveTitleBar) 
	{
		ModernDialogResizeData(this) = resizeData;
	}
	bool hasTitleBar;

};

class ModernTitleBar : public QWidget
{
	Q_OBJECT
public:
	explicit ModernTitleBar(const ModernDialogTitle &title, QWidget* parent = nullptr);
	void SetTitleText(const QString& title);
	void SetFont(const QFont& font) { _pTitle->setFont(font); }
	void SetColor(const QColor& color) { QPalette pal = _pTitle->palette(); pal.setColor(QPalette::WindowText, color); _pTitle->setPalette(pal); }
	void SetBackgroundColor(const QColor& color) { QPalette pal = palette(); pal.setColor(QPalette::Window, color); setPalette(pal); }
	QFont Font() const { return _pTitle->font(); }
signals:
	void SignalClose();
	void SignalMinimize();
protected:
	void mousePressEvent(QMouseEvent* event) override;
	// void mouseMoveEvent(QMouseEvent* event) override; we'll be using startSytemMove()
private:
	ModernDialogTitle _title;
	QLabel* _pTitle = nullptr;			  // if these three are all nullptr, then the title bar is not visible
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

	ModernDialogParams _params;
	QWidget* _pContentWidget = nullptr,
		   * _pFrameWidget = nullptr;	
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