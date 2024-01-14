#include "stdafx_zoli.h"
#include "application.h"

#include "themes.h"
#include "screen.h"
#include "canvas.h"
#include "imagelist.h"


//---------------------------------------------


namespace NLIBNS
{

    namespace
    {
        // Manages a bitmap to be used for classic interface drawing when a background/whiteish checker pattern is needed.
        // The brush must be deleted just like one created with CreateSolidBrush().
        HBRUSH  GetDisablePatternBrush()
        {
            HBRUSH disablepattern = nullptr;

            char *bmidata = new char[sizeof(BITMAPINFO) + sizeof(RGBQUAD) + 64];
            BITMAPINFO *bmi = (BITMAPINFO*)bmidata;

            memset(bmidata, 0, sizeof(BITMAPINFO) + sizeof(RGBQUAD) + 64);

            bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi->bmiHeader.biWidth = 8;
            bmi->bmiHeader.biHeight = -8;
            bmi->bmiHeader.biPlanes = 1;
            bmi->bmiHeader.biBitCount = 1;
            bmi->bmiHeader.biCompression = BI_RGB;
            bmi->bmiHeader.biClrUsed = 2;
            bmi->bmiColors[0] = Color(clBtnFace);
            bmi->bmiColors[1] = Color(clBtnFace).Mix(clWhite);

            byte *bits = (byte*)bmidata + sizeof(BITMAPINFO) + sizeof(RGBQUAD);

            for (int ix = 0; ix < 64; ++ix)
                bits[ix] = ((ix / 4) % 2) == 0 ? 0xAA : 0x55;

            disablepattern = CreateDIBPatternBrushPt(bmidata, DIB_RGB_COLORS);

            return disablepattern;
        }
    }

    Themes* themes = NULL;
    Themes* Themes::instance = NULL;

#define VSCLASS_GLOBALS             L"GLOBALS"
#define VSCLASS_SEARCHEDITBOX       L"SEARCHEDITBOX"
#define VSCLASS_EXPLORERLISTVIEW    L"EXPLORER::LISTVIEW"
#define VSCLASS_EXPLORERTREEVIEW    L"EXPLORER::TREEVIEW"

    const wchar_t *ThemeNames[tcThemeClassMax] = { VSCLASS_BUTTON, VSCLASS_CLOCK, VSCLASS_COMBOBOX, VSCLASS_COMMUNICATIONS, VSCLASS_CONTROLPANEL,
                                                   VSCLASS_DATEPICKER, VSCLASS_DRAGDROP, VSCLASS_EDIT, VSCLASS_EXPLORERBAR, VSCLASS_FLYOUT,
                                                   VSCLASS_GLOBALS, VSCLASS_HEADER, VSCLASS_LISTBOX, VSCLASS_LISTVIEW, VSCLASS_MENU, VSCLASS_MENUBAND,
                                                   VSCLASS_NAVIGATION, VSCLASS_PAGE, VSCLASS_PROGRESS, VSCLASS_REBAR, VSCLASS_SCROLLBAR,
                                                   VSCLASS_SEARCHEDITBOX, VSCLASS_SPIN, VSCLASS_STARTPANEL, VSCLASS_STATUS, VSCLASS_TAB, VSCLASS_TASKBAND,
                                                   VSCLASS_TASKBAR, VSCLASS_TASKDIALOG, VSCLASS_TEXTSTYLE, VSCLASS_TOOLBAR, VSCLASS_TOOLTIP, VSCLASS_TRACKBAR,
                                                   VSCLASS_TRAYNOTIFY, VSCLASS_TREEVIEW, VSCLASS_WINDOW, VSCLASS_EXPLORERLISTVIEW, VSCLASS_EXPLORERTREEVIEW,
                                                 };

    Themes* Themes::GetInstance()
    {
        if (!instance)
            instance = new Themes();
        themes = instance;
        return instance;
    }

    void Themes::FreeInstance()
    {
        delete instance;
        instance = NULL;
    }

    Themes::Themes() : isthemed(true)
    {
        memset(handles,0,sizeof(HTHEME)*(int)tcThemeClassMax);
        sizes.itemtextheight = -1;
    }

    Themes::~Themes()
    {
        CloseThemes();
    }

    std::wstring ThemeClassname(ThemeClasses themeclass)
    {
        if ((int)themeclass < 0 || (int) themeclass >= tcThemeClassMax)
            throw L"Argument not a theme class.";

        return ThemeNames[(int)themeclass];
    }

    HTHEME Themes::ThemeHandle(ThemeClasses themeclass)
    {
        if ((int)themeclass < 0 || (int) themeclass >= tcThemeClassMax)
            throw L"Argument not a theme class.";

        if (ThemesEnabled() && handles[(int)themeclass] == NULL)
            handles[(int)themeclass] = OpenThemeData(application->Handle(), ThemeNames[(int)themeclass]);
        return handles[(int)themeclass];
    }

    bool Themes::ThemesEnabled()
    {
        if (Win32MajorVersion <= 5 && Win32MinorVersion < 1)
            return false;
        return IsThemeActive() == TRUE;
    }

    bool Themes::AppThemed()
    {
        return isthemed && ThemesEnabled() && IsAppThemed() == TRUE;
    }

    void Themes::SetThemed(bool newthemed)
    {
        if (isthemed == newthemed)
            return;
        isthemed = newthemed;
    }

    void Themes::CloseThemes()
    {
        for (int ix = 0; ix < (int)tcThemeClassMax; ++ix)
            if (handles[ix] != NULL)
                CloseThemeData(handles[ix]);
        memset(handles, 0, sizeof(HTHEME) * (int)tcThemeClassMax);
        sizes.itemtextheight = -1;
    }

    void Themes::ThemesChanged()
    {
        CloseThemes();
    }

    void Themes::SetWindowTheme(HWND controlhandle, const wchar_t *pszSubAppName, const wchar_t *pszSubIdList)
    {
        ::SetWindowTheme(controlhandle, pszSubAppName, pszSubIdList);
    }

    void Themes::DrawParentBackground(HWND controlhandle, Canvas *canvas, const Rect &r)
    {
        if (!AppThemed())
            throw L"Can't draw parent background for non-themed app.";

        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";
        DrawThemeParentBackground(controlhandle, dc, !r.Empty() ? const_cast<Rect*>(&r) : NULL);
        canvas->ReturnDC();
    }

    bool Themes::IsPopupMenuItemTransparent(ThemeMenuItemStates state)
    {
        return AppThemed() && IsThemeBackgroundPartiallyTransparent(ThemeHandle(tcMenu), MENU_POPUPITEM, state) != FALSE;
    }

    bool Themes::IsWindowCloseButtonTransparent(ThemeWindowCloseButtonStates state)
    {
        return AppThemed() && IsThemeBackgroundPartiallyTransparent(ThemeHandle(tcWindow), WP_CLOSEBUTTON, state) != FALSE;
    }

    void Themes::DrawButton(Canvas *canvas, const Rect &r, ThemeButtonStates state)
    {
        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";

        if (AppThemed())
            DrawThemeBackground(ThemeHandle(tcButton), dc, BP_PUSHBUTTON, state == tbsHot ? PBS_HOT : state == tbsPressed ? PBS_PRESSED : state == tbsDisabled ? PBS_DISABLED : state == tbsDefaulted ? PBS_DEFAULTED : PBS_NORMAL, &r, NULL);
        else
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH | (state == tbsHot ? DFCS_HOT : state == tbsPressed ? DFCS_PUSHED : state == tbsDisabled ? DFCS_INACTIVE : /*state == tbsDefaulted ? PBS_DEFAULTED : PBS_NORMAL*/ 0) );

        canvas->ReturnDC();
    }

    void Themes::DrawCheckbox(Canvas *canvas, const Rect &r, ThemeCheckboxStates state)
    {
        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";

        if (AppThemed())
            DrawThemeBackground(ThemeHandle(tcButton), dc, BP_CHECKBOX, state == cbsCheckedDisabled ? CBS_CHECKEDDISABLED : state == cbsCheckedHot ? CBS_CHECKEDHOT : state == cbsCheckedNormal ? CBS_CHECKEDNORMAL :
                                    state == cbsCheckedPressed ? CBS_CHECKEDPRESSED : state == cbsMixedDisabled ? CBS_MIXEDDISABLED : state == cbsMixedHot ? CBS_MIXEDHOT : state == cbsMixedNormal ? CBS_MIXEDNORMAL :
                                    state == cbsMixedPressed ? CBS_MIXEDPRESSED : state == cbsUncheckedDisabled ? CBS_UNCHECKEDDISABLED : state == cbsUncheckedHot ? CBS_UNCHECKEDHOT :
                                    state == cbsUncheckedNormal ? CBS_UNCHECKEDNORMAL : CBS_UNCHECKEDPRESSED,
                                    &r, NULL);
        else
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONCHECK |
                           (state == cbsUncheckedHot || state == cbsMixedHot || state == cbsCheckedHot ? DFCS_HOT :
                            state == cbsCheckedPressed || state == cbsMixedPressed || state == cbsUncheckedPressed ? DFCS_PUSHED :
                            state == cbsCheckedHot || state == cbsCheckedNormal || state == cbsCheckedPressed ? DFCS_CHECKED :
                            state == cbsUncheckedDisabled || state == cbsMixedDisabled || state == cbsCheckedDisabled ? DFCS_INACTIVE :
                            /*state == tbsDefaulted ? PBS_DEFAULTED :
                            PBS_NORMAL*/
                            0) );

        canvas->ReturnDC();
    }

    namespace
    {
        inline ThemeButtonStates ThemeToolbarStateToButtonState(ThemeToolbarStates state)
        {
            switch (state)
            {
            case ttsNormal:
            case ttsNearHot:
            case ttsOtherSideHot:
                return tbsNormal;
            case ttsHot:
                return tbsHot;
            case ttsChecked:
                return tbsChecked;
            case ttsHotChecked:
                return tbsHotChecked;
            case ttsPressed:
                return tbsPressed;
            case ttsDefaulted:
                return tbsDefaulted;
            case ttsDisabled:
            default:
                return tbsDisabled;
            }
        }
    };

    void Themes::DrawToolbarButton(Canvas *canvas, const Rect &r, ThemeToolbarStates state)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";

            DrawThemeBackground(ThemeHandle(tcToolbar), dc, TP_BUTTON, state == ttsHot ? TS_HOT : state == ttsOtherSideHot ? TS_OTHERSIDEHOT : state == ttsChecked ? state == ttsNearHot ? TS_NEARHOT : TS_CHECKED : state == ttsHotChecked ? TS_HOTCHECKED : state == ttsPressed ? TS_PRESSED : state == ttsDisabled ? TS_DISABLED : state == ttsDefaulted ? TS_HOT : TS_NORMAL, &r, NULL);

            canvas->ReturnDC();
        }
        else
        {
            //ttsNormal, ttsHot, ttsOtherSideHot, ttsChecked, ttsHotChecked, ttsNearHot, ttsPressed, ttsDefaulted, ttsDisabled
            if (state != ttsNormal && state != ttsNearHot && state != ttsDisabled)
                canvas->DrawFrame(r, state == ttsHot || state == ttsOtherSideHot || state == ttsDefaulted);
        }
    }

    void Themes::DrawToolbarDropdownButton(Canvas *canvas, const Rect &r, ThemeToolbarStates state)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";

            DrawThemeBackground(ThemeHandle(tcToolbar), dc, TP_DROPDOWNBUTTON, state == ttsHot ? TS_HOT : state == ttsOtherSideHot ? TS_OTHERSIDEHOT : state == ttsChecked ? state == ttsNearHot ? TS_NEARHOT : TS_CHECKED : state == ttsHotChecked ? TS_HOTCHECKED : state == ttsPressed ? TS_PRESSED : state == ttsDisabled ? TS_DISABLED : state == ttsDefaulted ? TS_HOT : TS_NORMAL, &r, NULL);

            canvas->ReturnDC();
        }
        else
        {
            //ttsNormal, ttsHot, ttsOtherSideHot, ttsChecked, ttsHotChecked, ttsNearHot, ttsPressed, ttsDefaulted, ttsDisabled
            if (state != ttsNormal && state != ttsNearHot && state != ttsDisabled)
                canvas->DrawFrame(r, state == ttsHot || state == ttsOtherSideHot || state == ttsDefaulted);
        }
    }

    void Themes::DrawToolbarDropdownButtonGlyph(Canvas *canvas, const Rect &r, ThemeToolbarStates state)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";

            DrawThemeBackground(ThemeHandle(tcToolbar), dc, TP_DROPDOWNBUTTONGLYPH, state == ttsHot ? TS_HOT : state == ttsOtherSideHot ? TS_OTHERSIDEHOT : state == ttsChecked ? state == ttsNearHot ? TS_NEARHOT : TS_CHECKED : state == ttsHotChecked ? TS_HOTCHECKED : state == ttsPressed ? TS_PRESSED : state == ttsDisabled ? TS_DISABLED : state == ttsDefaulted ? TS_HOT : TS_NORMAL, &r, NULL);

            canvas->ReturnDC();
        }
        else
        {
            Bitmap bmp(5, 5);
            auto data = bmp.LockBits(Rect( 0, 0, 5, 5), glmWriteOnly, PixelFormat32bppARGB);
            byte *sc = (byte*)data->Scan0;
            int y = 1;
            int w = 5;
            int x = 0;
            for ( ; y < 4; ++y, ++x, w -= 2)
            {
                for (int ix = x; ix < x + w; ++ix)
                {
                    sc[ix * 4 + 0] = 0;
                    sc[ix * 4 + 1] = 0;
                    sc[ix * 4 + 2] = 0;
                    sc[ix * 4 + 3] = 255;
                }
                sc += data->Stride;
            }

            bmp.UpdateBits();

            auto cgs = canvas->SaveState();

            canvas->SetColorKey(clWhite, clWhite);
            ColorMatrix mx;
            Color changed = ((Color)(state == ttsDisabled ? clGrayText : clMenuText)).ToRGB();
            mx.m[4][0] = changed.R();
            mx.m[4][1] = changed.G();
            mx.m[4][2] = changed.B();
            canvas->SetColorMatrix(mx);
            canvas->Draw(&bmp, r.left, r.top, min(r.Width(), 5), min(r.Height(), 5), 0, 0, min(r.Width(), 5), min(r.Height(), 5));

            canvas->RestoreState(cgs);
        }
    }

    void Themes::DrawToolbarSplitbutton(Canvas *canvas, const Rect &r, ThemeToolbarStates state)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";

            DrawThemeBackground(ThemeHandle(tcToolbar), dc, TP_SPLITBUTTON, state == ttsHot ? TS_HOT : state == ttsOtherSideHot ? TS_OTHERSIDEHOT : state == ttsChecked ? state == ttsNearHot ? TS_NEARHOT : TS_CHECKED : state == ttsHotChecked ? TS_HOTCHECKED : state == ttsPressed ? TS_PRESSED : state == ttsDisabled ? TS_DISABLED : state == ttsDefaulted ? TS_HOT : TS_NORMAL, &r, NULL);

            canvas->ReturnDC();
        }
        else
        {
            //ttsNormal, ttsHot, ttsOtherSideHot, ttsChecked, ttsHotChecked, ttsNearHot, ttsPressed, ttsDefaulted, ttsDisabled
            if (state != ttsNormal && state != ttsNearHot && state != ttsDisabled)
                canvas->DrawFrame(r, state == ttsHot || state == ttsOtherSideHot || state == ttsDefaulted);
        }
    }

    void Themes::DrawToolbarSplitbuttonDropdown(Canvas *canvas, const Rect &r, ThemeToolbarStates state)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";

            DrawThemeBackground(ThemeHandle(tcToolbar), dc, TP_SPLITBUTTONDROPDOWN, state == ttsHot ? TS_HOT : state == ttsOtherSideHot ? TS_OTHERSIDEHOT : state == ttsChecked ? state == ttsNearHot ? TS_NEARHOT : TS_CHECKED : state == ttsHotChecked ? TS_HOTCHECKED : state == ttsPressed ? TS_PRESSED : state == ttsDisabled ? TS_DISABLED : state == ttsDefaulted ? TS_HOT : TS_NORMAL, &r, NULL);

            canvas->ReturnDC();
        }
        else
        {
            //ttsNormal, ttsHot, ttsOtherSideHot, ttsChecked, ttsHotChecked, ttsNearHot, ttsPressed, ttsDefaulted, ttsDisabled
            if (state != ttsNormal && state != ttsNearHot && state != ttsDisabled && state != ttsDefaulted)
                canvas->DrawFrame(r, state == ttsHot || state == ttsOtherSideHot);

            int left = r.left + (r.Width() - 5) / 2;
            int top = r.top + (r.Height() - 5) / 2;
            DrawToolbarDropdownButtonGlyph(canvas, Rect(left, top, left + 5, top + 5), state);
        }
    }

    void Themes::DrawToolbarSeparator(Canvas *canvas, const Rect &r, ThemeToolbarStates state)
    {
        canvas->SelectStockPen(sp3DShadow);
        canvas->Line(r.left, r.top, r.left, r.bottom);
        canvas->SelectStockPen(sp3DHighlight);
        canvas->Line(r.left + 1, r.top, r.left + 1, r.bottom);
    }

    void Themes::DrawToolbarSeparatorVert(Canvas *canvas, const Rect &r, ThemeToolbarStates state)
    {
        canvas->SelectStockPen(sp3DShadow);
        canvas->Line(r.left, r.top, r.right, r.top);
        canvas->SelectStockPen(sp3DHighlight);
        canvas->Line(r.left, r.top + 1, r.right, r.top + 1);
    }

    void Themes::DrawControlBorder(Canvas *canvas, const Rect &r)
    {
        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";

        Size s = MeasureEditBorderWidth();

        HRGN rgn = 0;
        int hasrgn = -1;

        if (r.left + s.cx <= r.right - s.cx && r.top + s.cy <= r.bottom - s.cy)
        {
            rgn = CreateRectRgn(0,0,0,0);
            if (rgn)
                hasrgn = GetClipRgn(dc, rgn);
            ExcludeClipRect(dc, r.left + s.cx, r.top + s.cy, r.right - s.cx, r.bottom - s.cy);
        }

        if (AppThemed())
        {
            int part = 0;
            int state = 0;
            DrawThemeBackground(ThemeHandle(tcEdit), dc, part, state, &r, NULL);
        }
        else
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);

        if (rgn)
        {
            if (hasrgn > 0)
                SelectClipRgn(dc, rgn);
            else if (hasrgn == 0)
                SelectClipRgn(dc, NULL);
            DeleteObject(rgn);
        }

        canvas->ReturnDC();
    }

    void Themes::DrawEditBorder(/*HWND controlhandle,*/ Canvas *canvas, const Rect &r, ThemeBorderDrawStates borderstate)
    {
        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";

        Size s = MeasureEditBorderWidth();

        HRGN rgn = 0;
        int hasrgn = -1;

        if (r.left + s.cx <= r.right - s.cx && r.top + s.cy <= r.bottom - s.cy)
        {
            rgn = CreateRectRgn(0,0,0,0);
            if (rgn)
                hasrgn = GetClipRgn(dc, rgn);
            ExcludeClipRect(dc, r.left + s.cx, r.top + s.cy, r.right - s.cx, r.bottom - s.cy);
        }

        if (AppThemed())
        {
            //int part = EP_EDITTEXT;
//            int state = borderstate == tbdsNormal ? ETS_NORMAL : borderstate == tbdsHot ? ETS_HOT : borderstate == tbdsFocused ? ETS_FOCUSED : ETS_DISABLED;
            int part = EP_EDITBORDER_NOSCROLL;
            int state = borderstate == tbdsNormal ? EPSN_NORMAL : borderstate == tbdsHot ? EPSN_HOT : borderstate == tbdsFocused ? EPSN_FOCUSED : EPSN_DISABLED;
            //if (IsThemeBackgroundPartiallyTransparent(ThemeHandle(tcEdit), part, state))
            //    DrawThemeParentBackground(controlhandle, dc, const_cast<Rect*>(&r));

            DrawThemeBackground(ThemeHandle(tcEdit), dc, part, state, &r, NULL);
        }
        else
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);

        if (rgn)
        {
            if (hasrgn > 0)
                SelectClipRgn(dc, rgn);
            else if (hasrgn == 0)
                SelectClipRgn(dc, NULL);
            DeleteObject(rgn);
        }

        canvas->ReturnDC();
    }

    void Themes::DrawTreeviewGlyph(Canvas *canvas, const Rect &r, ThemeTreeGlyphStates glyphstate)
    {
        DrawGenTreeviewGlyph(canvas, r, glyphstate, false);
    }

    void Themes::DrawVistaTreeviewGlyph(Canvas *canvas, const Rect &r, ThemeTreeGlyphStates glyphstate)
    {
        DrawGenTreeviewGlyph(canvas, r, glyphstate, true);
    }

    void Themes::DrawGenTreeviewGlyph(Canvas *canvas, const Rect &r, ThemeTreeGlyphStates glyphstate, bool usevista)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            if (usevista && Win32MajorVersion >= 6)
                DrawThemeBackground(ThemeHandle(tcExplorerTreeview), dc, glyphstate == ttgsOpen || glyphstate == ttgsClosed ? TVP_GLYPH : TVP_HOTGLYPH, glyphstate == ttgsOpen || glyphstate == ttgsHotOpen ? GLPS_OPENED : GLPS_CLOSED, &r, NULL);
            else
                DrawThemeBackground(ThemeHandle(tcTreeview), dc, glyphstate == ttgsOpen || glyphstate == ttgsClosed ? TVP_GLYPH : TVP_HOTGLYPH, glyphstate == ttgsOpen || glyphstate == ttgsHotOpen ? GLPS_OPENED : GLPS_CLOSED, &r, NULL);

            canvas->ReturnDC();
        }
        else
        {
            Rect r2 = r;
            canvas->SelectStockPen(spBlack);
            canvas->FrameRect(r);
            canvas->SelectStockBrush(sbWhite);
            InflateRect(&r2, -1, -1);
            canvas->FillRect(r2);

            if (glyphstate == ttgsOpen || glyphstate == ttgsHotOpen)
                canvas->Line(r2.left + r2.Width() / 2, r2.top + 1, r2.left + r2.Width() / 2, r2.bottom - 1);
            canvas->Line(r2.left + 1, r2.top + r2.Height() / 2, r2.right - 1, r2.top + r2.Height() / 2);
        }
    }

    void Themes::DrawTabPane(Canvas *canvas, const Rect &r)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcTab), dc, TABP_PANE, 0, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            canvas->SelectStockBrush(sbBtnFace);
            canvas->FillRect(r);
        }
    }

    void Themes::DrawTrackbarThumb(Canvas *canvas, const Rect &r, ThemeTrackbarThumbs thumb, ThemeTrackbarThumbStates state)
    {
        if (false/*AppThemed()*/)
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcTrackbar), dc, thumb == tttHorizontal ? TKP_THUMB : thumb == tttVertical ? TKP_THUMBVERT : thumb == tttUp ? TKP_THUMBTOP : thumb == tttDown ? TKP_THUMBBOTTOM : thumb == tttLeft ? TKP_THUMBLEFT : TKP_THUMBRIGHT,
                    state == tttsDisabled ? TUS_DISABLED : state == tttsFocused ? TUS_FOCUSED : state == tttsHot ? TUS_HOT : state == tttsNormal ? TUS_NORMAL : TUS_PRESSED, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            int vsize = 0; // Size of the arrow V shape at the end of the thumb button.
            int width = r.right - r.left;
            int height = r.bottom - r.top;

            if (r.Width() < 3 || r.Height() < 3)
                return;

            if (thumb == tttUp || thumb == tttDown || thumb == tttLeft || thumb == tttRight)
            {
                Size s = MeasureTrackburThumb(r, thumb);
                width = s.cx;
                height = s.cy;
            }

            if (thumb == tttUp || thumb == tttDown)
            {
                if ((width % 2) == 0)
                    --width;
                vsize = width / 2;

                if (vsize > height - 3)
                {
                    vsize = max(1, height - 3);
                    width = vsize * 2 + 1;
                }


            }
            else if (thumb == tttLeft || thumb == tttRight)
            {
                if ((height % 2) == 0)
                    --height;
                vsize = height / 2;

                if (vsize > width - 3)
                {
                    vsize = max(1, width - 3);
                    width = vsize * 2 + 1;
                }
            }

            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            if (thumb == tttHorizontal || thumb == tttVertical)
                DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH | (state == tttsHot ? DFCS_HOT : state == tttsDisabled ? DFCS_INACTIVE : 0));
            else
            {
                Rect r2 = r;
                if (width != r2.Width())
                {
                    r2.left = r2.left + (r2.Width() - width) / 2;
                    r2.right = r2.left + width;
                }
                if (height != r2.Height())
                {
                    r2.top = r2.top + (r2.Height() - height) / 2;
                    r2.bottom = r2.top + height;
                }
                
                HPEN pens[4] = { 
                    CreatePen(PS_SOLID, 0, Color(cl3DHighlight)), CreatePen(PS_SOLID, 0, Color(cl3DDkShadow)),
                    CreatePen(PS_SOLID, 0, Color(cl3DLight)), CreatePen(PS_SOLID, 0, Color(cl3DShadow))
                };

                HPEN open = 0;
                HBRUSH brush = 0;
                HBRUSH obrush = 0;
                try
                {
                    int vl = thumb == tttLeft ? vsize : 0;
                    int vt = thumb == tttUp ? vsize : 0;
                    int vr = thumb == tttRight ? vsize : 0;
                    int vb = thumb == tttDown ? vsize : 0;
                    const int endpos = thumb == tttUp ? 1 : thumb == tttRight ? 2 : thumb == tttDown ? 3 : /*thumb == tttLeft ?*/ 4;
                    const Point corners[4] = { Point(r2.left + vl, r2.top + vt), Point(r2.right - 1 - vr , r2.top + vt),
                                               Point(r2.right - 1 - vr, r2.bottom - 1 - vb), Point(r2.left + vl, r2.bottom - 1 - vb) };

                    Point bpoints[5];
                    for (int ix = 0; ix < 5; ++ix)
                        bpoints[ix] = ix != endpos ? corners[ix - (endpos < ix ? 1 : 0)] : Point(thumb == tttUp || thumb == tttDown ? r2.left + width / 2 : thumb == tttLeft ? r2.left : r2.right - 1, thumb == tttLeft || thumb == tttRight ? r2.top + height / 2 : thumb == tttUp ? r2.top : r2.bottom - 1);

                    for (int penbase = 0, rounds = 0; rounds < 2; ++rounds, penbase += 2)
                    {
                        if (rounds == 1)
                        {
                            if (thumb != tttLeft)
                                ++bpoints[0].x;
                            if (thumb != tttUp)
                                ++bpoints[0].y;
                        }

                        MoveToEx(dc, bpoints[0].x, bpoints[0].y, nullptr);
                        for (int ix = 0; ix < 5; ++ix)
                        {
                            if (rounds == 1)
                            {
                                int nextix = ix + 1;
                                if (nextix != 5)
                                {
                                    if (nextix == 4)
                                        ++bpoints[nextix].x;
                                    else if ((thumb == tttUp && nextix == 3) || (thumb != tttUp && thumb != tttRight && nextix == 1) || nextix == 2)
                                        --bpoints[nextix].x;

                                    if (nextix == 1)
                                        ++bpoints[nextix].y;
                                    else if ((thumb == tttLeft && nextix == 2) || (thumb != tttLeft && thumb != tttDown && nextix == 4) || nextix == 3)
                                        --bpoints[nextix].y;
                                }
                            }

                            if (open == 0)
                                open = (HPEN)GetCurrentObject(dc, OBJ_PEN);
                            switch (thumb)
                            {
                            case tttUp:
                            case tttLeft:
                                SelectObject(dc, pens[penbase + (ix == 1 || ix == 2 || ix == 3 ? 1 : 0)]);
                                break;
                            case tttRight:
                                SelectObject(dc, pens[penbase + (ix == 2 || ix == 3 ? 1 : 0)]);
                                break;
                            case tttDown:
                                SelectObject(dc, pens[penbase + (ix == 1 || ix == 2 ? 1 : 0)]);
                                break;
                            }

                            LineTo(dc, bpoints[ix == 4 ? 0 : ix + 1].x, bpoints[ix == 4 ? 0 : ix + 1].y);
                            if (thumb != tttDown && thumb != tttLeft && ((thumb == tttLeft && ix == 2) || (thumb != tttLeft && ix == 3)))
                                LineTo(dc, bpoints[ix + 1].x, bpoints[ix + 1].y - 1);
                            if (thumb == tttDown && ix == 2)
                                LineTo(dc, bpoints[ix + 1].x - 1, bpoints[ix + 1].y - 1);
                            if (thumb == tttLeft && ix == 3)
                                LineTo(dc, bpoints[ix + 1].x + 1, bpoints[ix + 1].y - 1);
                        }
                    }

                    if (thumb != tttLeft)
                        ++bpoints[0].x;
                    if (thumb != tttUp)
                        ++bpoints[0].y;

                    if (thumb != tttUp && thumb != tttRight)
                        --bpoints[1].x;

                    if (thumb == tttUp)
                        ++bpoints[2].y;

                    if (thumb == tttLeft)
                        ++bpoints[3].x;

                    ++bpoints[4].x;

                    COLORREF prevtx = CLR_INVALID;
                    COLORREF prevbk = CLR_INVALID;
                    int prevbkmode = 0;
                    if (state == tttsDisabled)
                    {
                        prevbk = SetBkColor(dc, Color(clBtnFace));
                        prevtx = SetTextColor(dc, Color(clBtnFace).Mix(clWhite));
                        prevbkmode = SetBkMode (dc, OPAQUE);
                    }

                    SelectObject(dc, GetStockObject(NULL_PEN));
                    if (state != tttsDisabled)
                        brush = CreateSolidBrush(Color(clBtnFace));
                    else
                        brush = GetDisablePatternBrush();
                    obrush = (HBRUSH)SelectObject(dc, brush);

                    Polygon(dc, bpoints, 5);
                    if (state == tttsDisabled)
                    {
                        if (prevbk != CLR_INVALID)
                            SetBkColor(dc, prevbk);
                        if (prevtx != CLR_INVALID)
                            SetTextColor(dc, prevtx);
                        if (prevbkmode != 0)
                            SetBkMode(dc, prevbkmode);
                    }
                }
                catch(...)
                {
                }

                if (open)
                    SelectObject(dc, open);
                if (obrush)
                    SelectObject(dc, obrush);

                for (int ix = 0; ix < 4; ++ix)
                    if (pens[ix])
                        DeleteObject(pens[ix]);
                if (brush)
                    DeleteObject(brush);
            }
            canvas->ReturnDC();
        }
    }

    void Themes::DrawTrackbarTrack(Canvas *canvas, const Rect &r, bool vertical)
    {
        if (AppThemed())
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcTrackbar), dc, !vertical ? TKP_TRACK : TKP_TRACKVERT, TRS_NORMAL, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            HPEN pens[4] = { 
                CreatePen(PS_SOLID, 0, Color(cl3DDkShadow)), CreatePen(PS_SOLID, 0, Color(cl3DHighlight)),
                CreatePen(PS_SOLID, 0, Color(cl3DShadow)), CreatePen(PS_SOLID, 0, Color(cl3DLight))
            };

            HPEN open = 0;

            try
            {
                Rect r2 = r;

                open = (HPEN)SelectObject(dc, pens[0]);
                MoveToEx(dc, r2.left, r2.bottom - 1, nullptr);
                LineTo(dc, r2.left, r2.top);
                LineTo(dc, r2.right - 1, r2.top);
                LineTo(dc, r2.right - 1, r2.top + 1);

                SelectObject(dc, pens[1]);
                LineTo(dc, r2.right - 1, r2.bottom - 1);
                LineTo(dc, r2.left, r2.bottom - 1);

                InflateRect(&r2, -1, -1);
                clamp(r2.left, r.left, r.right);
                clamp(r2.right, r.left, r.right);
                clamp(r2.top, r.top, r.bottom);
                clamp(r2.bottom, r.top, r.bottom);

                SelectObject(dc, pens[2]);
                MoveToEx(dc, r2.left, r2.bottom - 1, nullptr);
                LineTo(dc, r2.left, r2.top);
                LineTo(dc, r2.right - 1, r2.top);
                LineTo(dc, r2.right - 1, r2.top + 1);

                SelectObject(dc, pens[2]);
                LineTo(dc, r2.right - 1, r2.bottom - 1);
                LineTo(dc, r2.left, r2.bottom - 1);

                InflateRect(&r2, -1, -1);
                clamp(r2.left, r.left, r.right);
                clamp(r2.right, r.left, r.right);
                clamp(r2.top, r.top, r.bottom);
                clamp(r2.bottom, r.top, r.bottom);

                FillRect(dc, &r2, (HBRUSH)(COLOR_BTNFACE + 1));
            }
            catch(...)
            {
                if (open)
                    SelectObject(dc, open);

                for (int ix = 0; ix < 4; ++ix)
                    if (pens[ix])
                        DeleteObject(pens[ix]);
                canvas->ReturnDC();

                throw;
            }

            if (open)
                SelectObject(dc, open);

            for (int ix = 0; ix < 4; ++ix)
                if (pens[ix])
                    DeleteObject(pens[ix]);
            canvas->ReturnDC();
        }
    }

    Size Themes::MeasureTrackburThumb(const Rect &r, ThemeTrackbarThumbs thumb)
    {
        if (AppThemed())
        {
            HDC dc = GetDC(0);

            Size s;
            GetThemePartSize(ThemeHandle(tcTrackbar), dc, thumb == tttHorizontal ? TKP_THUMB : thumb == tttVertical ? TKP_THUMBVERT : thumb == tttUp ? TKP_THUMBTOP : thumb == tttDown ? TKP_THUMBBOTTOM : thumb == tttLeft ? TKP_THUMBLEFT : TKP_THUMBRIGHT, TUS_NORMAL, const_cast<Rect*>(&r), TS_DRAW, &s);
            ReleaseDC(0, dc);
            return s;
        }
        else
        {
            Size s;
            if (thumb == tttHorizontal || thumb == tttVertical)
                s = Size(r.Width(), r.Height());
            else if (thumb == tttUp || thumb == tttDown)
            {
                s = Size(11 * Scaling, 21 * Scaling);
                if (r.Height() < s.cy)
                {
                    s.cx = int(float(r.Height()) / s.cy * s.cx);
                    s.cy = r.Height();
                }
                if (r.Width() < s.cx)
                {
                    s.cy = int(float(r.Width()) / s.cx * s.cy);
                    s.cx = r.Width();
                }
            }
            else
            {
                s = Size(21 * Scaling, 11 * Scaling);
                if (r.Width() < s.cx)
                {
                    s.cy = int(float(r.Width()) / s.cx * s.cy);
                    s.cx = r.Width();
                }
                if (r.Height() < s.cy)
                {
                    s.cx = int(float(r.Height()) / s.cy * s.cx);
                    s.cy = r.Height();
                }
            }
            return s;
        }
    }

    Size Themes::MeasureTrackburThumbDefaultSize(ThemeTrackbarThumbs thumb)
    {
        if (AppThemed())
        {
            HDC dc = GetDC(0);

            Size s;
            Rect measurerect = Rect(0, 0, 1000, 1000);
            GetThemePartSize(ThemeHandle(tcTrackbar), dc, thumb == tttHorizontal ? TKP_THUMB : thumb == tttVertical ? TKP_THUMBVERT : thumb == tttUp ? TKP_THUMBTOP : thumb == tttDown ? TKP_THUMBBOTTOM : thumb == tttLeft ? TKP_THUMBLEFT : TKP_THUMBRIGHT, TUS_NORMAL, const_cast<Rect*>(&measurerect), TS_TRUE, &s);
            ReleaseDC(0, dc);

            return s;
        }
        else
        {
            if (thumb == tttHorizontal || thumb == tttUp || thumb == tttDown)
                return Size(11 * Scaling, 21 * Scaling);
            return Size(21 * Scaling, 11 * Scaling);
        }
    }

    void Themes::DrawHeaderItem(Canvas *canvas, const Rect &r, ThemeHeaderItemStates state)
    {
        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";

        if (AppThemed())
            DrawThemeBackground(ThemeHandle(tcHeader), dc, HP_HEADERITEM, state == thisNormal ? HIS_NORMAL : state == thisHot ? HIS_HOT : state == thisPressed ? HIS_PRESSED : state == thisSortedNormal ? HIS_SORTEDNORMAL : state == thisSortedHot ? HIS_SORTEDHOT : state == thisSortedPressed ? HIS_SORTEDPRESSED : state == thisIconNormal ? HIS_ICONNORMAL : state == thisIconHot ? HIS_ICONHOT : state == thisIconPressed ? HIS_ICONPRESSED : state == thisIconSortedNormal ? HIS_ICONSORTEDNORMAL : state == thisIconSortedHot ? HIS_ICONSORTEDHOT : state == thisIconSortedPressed ? HIS_ICONSORTEDPRESSED : 0, &r, NULL);
        else
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH | (state == thisHot || state == thisIconHot || state == thisSortedHot || state == thisIconSortedHot ? DFCS_HOT : state == thisPressed || state == thisIconPressed || state == thisSortedPressed || state == thisIconSortedPressed ? DFCS_PUSHED : 0) );

        canvas->ReturnDC();
    }

    void Themes::DrawHeaderItemClip(Canvas *canvas, const Rect &r, ThemeHeaderItemStates state, const Rect &clip)
    {
        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";

        if (AppThemed())
            DrawThemeBackground(ThemeHandle(tcHeader), dc, HP_HEADERITEM, state == thisNormal ? HIS_NORMAL : state == thisHot ? HIS_HOT : state == thisPressed ? HIS_PRESSED : state == thisSortedNormal ? HIS_SORTEDNORMAL : state == thisSortedHot ? HIS_SORTEDHOT : state == thisSortedPressed ? HIS_SORTEDPRESSED : state == thisIconNormal ? HIS_ICONNORMAL : state == thisIconHot ? HIS_ICONHOT : state == thisIconPressed ? HIS_ICONPRESSED : state == thisIconSortedNormal ? HIS_ICONSORTEDNORMAL : state == thisIconSortedHot ? HIS_ICONSORTEDHOT : state == thisIconSortedPressed ? HIS_ICONSORTEDPRESSED : 0, &r, &clip);
        else
        {
            IntersectClipRect(dc, clip.left, clip.top, clip.right, clip.bottom);
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH | (state == thisHot || state == thisIconHot || state == thisSortedHot || state == thisIconSortedHot ? DFCS_HOT : state == thisPressed || state == thisIconPressed || state == thisSortedPressed || state == thisIconSortedPressed ? DFCS_PUSHED : 0));
        }

        canvas->ReturnDC();
    }

    void Themes::DrawHeaderRightSide(Canvas *canvas, const Rect &r)
    {
        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";

        if (AppThemed())
            DrawThemeBackground(ThemeHandle(tcHeader), dc, HP_HEADERITEMRIGHT, HIRS_NORMAL, &r, NULL);
        else
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH);

        canvas->ReturnDC();
    }

    void Themes::DrawMenubarBackground(Canvas *canvas, const Rect &r, bool active)
    {
        BOOL flatmenu = false;
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_BARBACKGROUND, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_BARBACKGROUND, active ? MB_ACTIVE : MB_INACTIVE, &r, NULL);
            canvas->ReturnDC();
        }
        else if (AppThemed() || (SystemParametersInfo(SPI_GETFLATMENU, 0, &flatmenu, 0) != 0 && flatmenu)) // XP draws its menu without themeing.
        {
            Brush oldbrush = canvas->GetBrush();

            canvas->SetBrush(clMenubar);
            canvas->FillRect(r);

            canvas->SetBrush(oldbrush);
        }
        else
        {
            Brush oldbrush = canvas->GetBrush();

            canvas->SetBrush(clBtnFace);
            canvas->FillRect(r);

            canvas->SetBrush(oldbrush);
        }
    }

    void Themes::DrawPopupMenuBackground(Canvas *canvas, const Rect &r)
    {
        BOOL flatmenu = false;
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPBORDERS, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPBORDERS, 0, &r, NULL);
            canvas->ReturnDC();

            //int checktypes[] = {
            //    0, MENU_POPUPBORDERS, MENU_POPUPBACKGROUND, MENU_POPUPITEM, MENU_POPUPGUTTER, 	MENU_BARBACKGROUND,MENU_BARITEM,	MENU_CHEVRON_TMSCHEMA,	MENU_MENUBARDROPDOWN_TMSCHEMA	,MENU_MENUBARITEM_TMSCHEMA,	MENU_MENUDROPDOWN_TMSCHEMA,  MENU_MENUITEM_TMSCHEMA,
            //    MENU_POPUPBACKGROUND, MENU_POPUPBORDERS,MENU_POPUPCHECK,MENU_POPUPCHECKBACKGROUND,MENU_POPUPGUTTER,MENU_POPUPITEM,MENU_POPUPSEPARATOR,MENU_POPUPSUBMENU,MENU_SEPARATOR_TMSCHEMA,MENU_SYSTEMCLOSE,MENU_SYSTEMMAXIMIZE,	MENU_SYSTEMMINIMIZE,MENU_SYSTEMRESTORE
            //};
            //int coltypes[] = {
            //    TMT_ACCENTCOLORHINT, TMT_ACTIVEBORDER, TMT_ACTIVECAPTION, TMT_APPWORKSPACE, TMT_BACKGROUND, TMT_BLENDCOLOR,
            //    TMT_BODYTEXTCOLOR, TMT_BORDERCOLOR, TMT_BORDERCOLORHINT, TMT_BTNFACE, TMT_BTNHIGHLIGHT, TMT_BTNSHADOW,
            //    TMT_BTNTEXT, TMT_BUTTONALTERNATEFACE, TMT_CAPTIONTEXT, TMT_DKSHADOW3D, TMT_EDGEDKSHADOWCOLOR,
            //    TMT_EDGEFILLCOLOR, TMT_EDGEHIGHLIGHTCOLOR, TMT_EDGELIGHTCOLOR, TMT_EDGESHADOWCOLOR, TMT_FILLCOLOR,
            //    TMT_FILLCOLORHINT, TMT_FROMCOLOR1, TMT_FROMCOLOR2, TMT_FROMCOLOR3, TMT_FROMCOLOR4, TMT_FROMCOLOR5,
            //    TMT_GLOWCOLOR, TMT_GLYPHTEXTCOLOR, TMT_GLYPHTRANSPARENTCOLOR, TMT_GRADIENTACTIVECAPTION,
            //    TMT_GRADIENTCOLOR1, TMT_GRADIENTCOLOR2, TMT_GRADIENTCOLOR3, TMT_GRADIENTCOLOR4, TMT_GRADIENTCOLOR5,
            //    TMT_GRADIENTINACTIVECAPTION, TMT_GRAYTEXT, TMT_HEADING1TEXTCOLOR, TMT_HEADING2TEXTCOLOR,
            //    TMT_HIGHLIGHT, TMT_HIGHLIGHTTEXT, TMT_HOTTRACKING, TMT_INACTIVEBORDER, TMT_INACTIVECAPTION,
            //    TMT_INACTIVECAPTIONTEXT, TMT_INFOBK, TMT_INFOTEXT, TMT_LIGHT3D, TMT_MENU, TMT_MENUBAR,
            //    TMT_MENUHILIGHT, TMT_MENUTEXT, TMT_SCROLLBAR, TMT_SHADOWCOLOR, TMT_TEXTBORDERCOLOR,
            //    TMT_TEXTCOLOR, TMT_TEXTCOLORHINT, TMT_TEXTSHADOWCOLOR, TMT_TRANSPARENTCOLOR, TMT_WINDOW,
            //    TMT_WINDOWFRAME, TMT_WINDOWTEXT
            //};
            // //DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPBACKGROUND, 0, &r, NULL);
            //COLORREF col;
            //for (int iy = 0; iy < sizeof(checktypes) / sizeof(int); ++iy)
            //{
            //    for (int ix = 0; ix < sizeof(coltypes) / sizeof(int); ++ix)
            //    {
            //        col = 0;
            //        GetThemeColor(ThemeHandle(tcMenu), checktypes[iy], 0, coltypes[ix], &col);
            //        if (col == 0)
            //        {
            //            col = 0xffffff;
            //            GetThemeColor(ThemeHandle(tcMenu), checktypes[iy], 0, coltypes[ix], &col);
            //        }
            //        if (col == 0xf5f5f5)
            //            throw L"Found it!";
            //    }
            //}
            //GetThemeColor(ThemeHandle(tcMenu), MENU_POPUPBORDERS, 0, TMT_EDGEFILLCOLOR, &col);
            //col = 0;
            //GetThemeColor(ThemeHandle(tcMenu), MENU_POPUPBACKGROUND, 0, TMT_FILLCOLORHINT, &col);
            //col = 0;
            //GetThemeColor(ThemeHandle(tcMenu), 0, 0, TMT_EDGEFILLCOLOR, &col);
            //col = 0;
        }
        else if (AppThemed() || (SystemParametersInfo(SPI_GETFLATMENU, 0, &flatmenu, 0) != 0 && flatmenu))
        {
            Brush oldbrush = canvas->GetBrush();
            Pen oldpen = canvas->GetPen();

            canvas->SetPen(cl3DShadow);
            canvas->FrameRect(r);
            canvas->SetBrush(clMenu);
            canvas->FillRect(r.Inflate(-1, -1));

            canvas->SetPen(oldpen);
            canvas->SetBrush(oldbrush);
        }
        else
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_BUTTON, DFCS_BUTTONPUSH);
            canvas->ReturnDC();
        }
    }

    void Themes::DrawMenubarItemHighlight(Canvas *canvas, const Rect &r, ThemeMenubarItemStates itemstate)
    {
        BOOL flatmenu = false;
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_BARITEM, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_BARITEM, itemstate == tmbisDisabled ? MBI_DISABLED : itemstate == tmbisDisabledHot ? MBI_DISABLEDHOT : itemstate == tmbisDisabledPushed ? MBI_DISABLEDPUSHED : itemstate == tmbisHot ? MBI_HOT : itemstate == tmbisPushed ? MBI_PUSHED : MBI_NORMAL, &r, NULL);
            canvas->ReturnDC();
        }
        else if (AppThemed() || (SystemParametersInfo(SPI_GETFLATMENU, 0, &flatmenu, 0) != 0 && flatmenu))
        {
            if (itemstate == tmbisDisabled || /*itemstate == tmbisDisabledHot || itemstate == tmbisDisabledPushed ||*/ itemstate == tmbisNormal)
                return; // Do nothing;
            Brush oldbrush = canvas->GetBrush();
            Pen oldpen = canvas->GetPen();

            canvas->SetBrush(clMenuHilight);
            canvas->FillRect(r.Inflate(-1, -1));
            canvas->SetPen(clHighlight);
            canvas->FrameRect(r);

            canvas->SetPen(oldpen);
            canvas->SetBrush(oldbrush);
        }
        else
        {
            if (itemstate == tmbisDisabled || /*itemstate == tmbisDisabledHot || itemstate == tmbisDisabledPushed ||*/ itemstate == tmbisNormal)
                return; // Do nothing;
            Brush oldbrush = canvas->GetBrush();

            canvas->SetBrush(clHighlight);
            canvas->FillRect(r);

            canvas->SetBrush(oldbrush);
        }
    }

    void Themes::DrawPopupMenuItemBackground(Canvas *canvas, const Rect &r)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPBACKGROUND, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPBACKGROUND, 0, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            ;
        }
    }

    void Themes::DrawPopupMenuItemHighlight(Canvas *canvas, const Rect &r, ThemeMenuItemStates itemstate)
    {
        BOOL flatmenu = false;
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPITEM, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPITEM, itemstate == tmisDisabled ? MPI_DISABLED : itemstate == tmisDisabledHot ? MPI_DISABLEDHOT : itemstate == tmisHot ? MPI_HOT : MPI_NORMAL, &r, NULL);
            canvas->ReturnDC();
        }
        else if (AppThemed() || (SystemParametersInfo(SPI_GETFLATMENU, 0, &flatmenu, 0) != 0 && flatmenu))
        {
            if (itemstate == tmisDisabled /*|| itemstate == tmisDisabledHot*/ || itemstate == tmisNormal)
            {
                canvas->FillRect(r);
                return;
            }
            Brush oldbrush = canvas->GetBrush();
            Pen oldpen = canvas->GetPen();

            canvas->SetBrush(clMenuHilight);
            canvas->FillRect(r.Inflate(-1, -1));
            canvas->SetPen(clHighlight);
            canvas->FrameRect(r);

            canvas->SetPen(oldpen);
            canvas->SetBrush(oldbrush);
        }
        else
        {
            if (itemstate == tmisDisabled /*|| itemstate == tmisDisabledHot*/ || itemstate == tmisNormal)
            {
                canvas->FillRect(r);
                return;
            }
            Brush oldbrush = canvas->GetBrush();

            canvas->SetBrush(clHighlight);
            canvas->FillRect(r);

            canvas->SetBrush(oldbrush);
        }
    }

    void Themes::DrawPopupMenuSeparator(Canvas *canvas, const Rect &r)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPSEPARATOR, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPSEPARATOR, 0, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            Pen oldpen = canvas->GetPen();

            int dtop = (r.Height() - 2) / 2;
            canvas->SetPen(cl3DShadow);
            canvas->Line(r.left, r.top + dtop, r.right - 1, r.top + dtop);
            canvas->SetPen(cl3DHighlight);
            canvas->Line(r.left, r.top + dtop + 1, r.right - 1, r.top + dtop + 1);

            canvas->SetPen(oldpen);
        }
    }

    void Themes::DrawPopupMenuItemGutter(Canvas *canvas, const Rect &r)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPGUTTER, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPGUTTER, 0, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            ;
        }
    }

    void Themes::DrawPopupMenuCheckBackground(Canvas *canvas, const Rect &r, ThemeMenuCheckBackgroundStates state)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPCHECKBACKGROUND, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPCHECKBACKGROUND, state == tmcbgBitmap ? MCB_BITMAP : state == tmcbgDisabled ? MCB_DISABLED : MCB_NORMAL, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            ;
        }
    }

    void Themes::DrawPopupMenuCheck(Canvas *canvas, const Rect &r, ThemeMenuItemStates mistate, ThemeMenuCheckStates state)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPCHECK, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPCHECK, state == tmcBulletDisabled ? MC_BULLETDISABLED : state == tmcBulletNormal ? MC_BULLETNORMAL : state == tmcCheckmarkDisabled ? MC_CHECKMARKDISABLED : MC_CHECKMARKNORMAL, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            //HDC dc = canvas->GetDC();
            //if (!dc)
            //    throw L"Couldn't acquire dc from canvas!";
            //DrawFrameControl(dc, const_cast<Rect*>(&r), DFC_MENU, (state == tmcBulletDisabled || state == tmcBulletNormal ? DFCS_MENUBULLET : DFCS_MENUCHECK) | (state == tmcBulletDisabled || state == tmcCheckmarkDisabled ? DFCS_INACTIVE : 0));
            //canvas->ReturnDC();
            Bitmap check(r.Width(), r.Height());
            HDC dc = check.GetDC();
            if (!dc)
                throw L"Couldn't acquire a dc from a temporary bitmap.";
            Rect r2 = Rect(0, 0, r.Width(), r.Height());
            DrawFrameControl(dc, &r2, DFC_MENU, (state == tmcBulletDisabled || state == tmcBulletNormal ? DFCS_MENUBULLET : DFCS_MENUCHECK) | (state == tmcBulletDisabled || state == tmcCheckmarkDisabled ? DFCS_INACTIVE : 0));
            check.ReturnDC();

            auto cgs = canvas->SaveState();

            canvas->SetColorKey(clWhite, clWhite);
            ColorMatrix mx;
            Color changed = ((Color)((mistate == tmisDisabled || mistate == tmisDisabledHot) ? clGrayText : mistate == tmisHot ? clHighlightText : clMenuText)).ToRGB();
            mx.m[4][0] = changed.R();
            mx.m[4][1] = changed.G();
            mx.m[4][2] = changed.B();
            canvas->SetColorMatrix(mx);
            canvas->Draw(&check, r.left, r.top, r.Width(), r.Height(), 0, 0, check.Width(), check.Height());

            canvas->RestoreState(cgs);
        }
    }

    void Themes::DrawSubMenuGlyph(Canvas *canvas, const Rect &r, ThemeMenuItemStates mistate, ThemeSubMenuGlyphStates state)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPSUBMENU, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeBackground(ThemeHandle(tcMenu), dc, MENU_POPUPSUBMENU, state == tsmgDisabled ? MSM_DISABLED : MSM_NORMAL, &r, NULL);
            canvas->ReturnDC();
        }
        else
        {
            Bitmap check(r.Width(), r.Height());
            HDC dc = check.GetDC();
            if (!dc)
                throw L"Couldn't acquire a dc from a temporary bitmap.";
            Rect r2 = Rect(0, 0, r.Width(), r.Height());
            DrawFrameControl(dc, &r2, DFC_MENU, DFCS_MENUARROW | (state == tsmgDisabled ? DFCS_INACTIVE : 0));
            check.ReturnDC();

            auto cgs = canvas->SaveState();

            canvas->SetColorKey(clWhite, clWhite);
            ColorMatrix mx;
            Color changed = ((Color)((mistate == tmisDisabled || mistate == tmisDisabledHot) ? clGrayText : mistate == tmisHot ? clHighlightText : clMenuText)).ToRGB();
            mx.m[4][0] = changed.R();
            mx.m[4][1] = changed.G();
            mx.m[4][2] = changed.B();
            canvas->SetColorMatrix(mx);
            canvas->Draw(&check, r.left, r.top, r.Width(), r.Height(), 0, 0, check.Width(), check.Height());

            canvas->RestoreState(cgs);
        }
    }

    Size Themes::MeasureMenubarTextExtent(Canvas *canvas, const std::wstring& str, TextDrawOptionSet options)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_BARITEM, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            Rect r;
            GetThemeTextExtent(ThemeHandle(tcMenu), dc, MENU_BARITEM, 0/*MBI_NORMAL*/, str.data(), str.length(), options, NULL, &r);
            canvas->ReturnDC();
            return Size(r.Width(), r.Height());
        }
        else
        {
            Font oldfont = canvas->GetFont();

            canvas->SetFont(application->MenuFont());
            Size s = canvas->MeasureFormattedText(str, options);

            canvas->SetFont(oldfont);

            return s;
        }
    }

    Size Themes::MeasurePopupMenuItemTextExtent(Canvas *canvas, const std::wstring& str, TextDrawOptionSet options)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPITEM, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            Rect r;
            GetThemeTextExtent(ThemeHandle(tcMenu), dc, MENU_POPUPITEM, 0/*MBI_NORMAL*/, str.data(), str.length(), options, NULL, &r);
            canvas->ReturnDC();
            return Size(r.Width(), r.Height());
        }
        else
        {
            Font oldfont = canvas->GetFont();

            canvas->SetFont(application->MenuFont());
            Size s = canvas->MeasureFormattedText(str, options);

            canvas->SetFont(oldfont);

            return s;
        }
    }

    void Themes::DrawMenubarText(Canvas *canvas, const Rect &r, const std::wstring& str, TextDrawOptionSet options, ThemeMenubarItemStates state)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_BARITEM, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeText(ThemeHandle(tcMenu), dc, MENU_BARITEM, state == tmbisDisabled ? MBI_DISABLED : state == tmbisDisabledHot ? MBI_DISABLEDHOT : state == tmbisDisabledPushed ? MBI_DISABLEDPUSHED : state == tmbisHot ? MBI_HOT : state == tmbisPushed ? MBI_PUSHED : MBI_NORMAL, str.data(), str.length(), options, 0, &r);
            canvas->ReturnDC();
        }
        else
        {
            Font oldfont = canvas->GetFont();

            canvas->SetFont(application->MenuFont());
            canvas->GetFont().SetColor((state == tmbisDisabled || state == tmbisDisabledHot || state == tmbisDisabledPushed) ? clGrayText : state == tmbisHot || state == tmbisPushed ? clHighlightText : clMenuText);
            canvas->FormatText(r, str, options);

            canvas->SetFont(oldfont);
        }
    }

    void Themes::DrawPopupMenuItemText(Canvas *canvas, const Rect &r, const std::wstring& str, TextDrawOptionSet options, ThemeMenuItemStates state)
    {
        if (AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPITEM, 0))
        {
            HDC dc = canvas->GetDC();
            if (!dc)
                throw L"Couldn't acquire dc from canvas!";
            DrawThemeText(ThemeHandle(tcMenu), dc, MENU_POPUPITEM, state == tmisDisabled ? MPI_DISABLED : state == tmisDisabledHot ? MPI_DISABLEDHOT : state == tmisHot ? MPI_HOT : MPI_NORMAL, str.data(), str.length(), options, 0, &r);
            canvas->ReturnDC();
        }
        else
        {
            Font oldfont = canvas->GetFont();

            canvas->SetFont(application->MenuFont());
            canvas->GetFont().SetColor((state == tmisDisabled || state == tmisDisabledHot) ? clGrayText : state == tmisHot ? clHighlightText : clMenuText);
            canvas->FormatText(r, str, options);

            canvas->SetFont(oldfont);
        }
    }

    void Themes::DrawMenubarItem(Canvas *c, const Rect &r, const std::wstring& text, ThemeMenubarItemStates state, int imagewidth)
    {
        DrawMenubarItemHighlight(c, r, state);

        auto sizes = MenuSizes();
        Rect r2 = r;
        r2.left += sizes.baritemmargin.cxLeftWidth;
        //if (imagelist && imageindex >= 0 && imageindex < imagelist->Count())
        //{
        //    r2.left += imagelist->Width();
        //    imagelist->Draw(c, imageindex, r2.left, r2.top + (r2.Height() - imagelist->Height()) / 2.0);
        //    r2.left += 4;
        //}
        if (imagewidth > 0)
            r2.left += imagewidth + 4;
        r2.top += (r.Height() - sizes.bartextheight) / 2.0;
        DrawMenubarText(c, r2, text, tdoSingleLine, state);
    }

    Point Themes::MenubarImagePosition(const Rect &r, int imageheight)
    {
        auto sizes = MenuSizes();
        return Point(r.left + sizes.baritemmargin.cxLeftWidth, r.top + (r.Height() - imageheight) / 2);
    }

    void Themes::DrawPopupMenuSeparatorItem(Canvas *c, const Rect &r, ThemeMenuItemStates state, int imagewidth)
    {
        auto sizes = MenuSizes();
        if (MenuThemed())
        {
            int checkwidth = imagewidth > 0 ? max(imagewidth, sizes.popcheck.cx) : sizes.popcheck.cx;

            if (IsPopupMenuItemTransparent(state))
                DrawPopupMenuItemBackground(c, r);
            DrawPopupMenuItemGutter(c, Rect(r.left, r.top, r.left + sizes.popitemmargin.cxLeftWidth + checkwidth + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckmargin.cxRightWidth + sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckbgmargin.cxRightWidth, r.bottom));

            // Everything is inside the item margin.
            Rect r2 = r;
            r2.left += sizes.popitemmargin.cxLeftWidth;
            r2.right -= sizes.popitemmargin.cxRightWidth;

            themes->DrawPopupMenuItemHighlight(c, r2, state);

            r2.left += checkwidth + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckmargin.cxRightWidth + sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckbgmargin.cxRightWidth;
            DrawPopupMenuSeparator(c, Rect(r2.left, r2.top + sizes.popitemmargin.cyTopHeight, r2.right, r2.top + sizes.popitemmargin.cyTopHeight + sizes.popsep.cy));
        }
        else
        {
            DrawPopupMenuItemHighlight(c, r, state);
            themes->DrawPopupMenuSeparator(c, Rect(r.left + 1, r.top, r.right - 1, r.top + sizes.sepheight));
        }
    }

    Point Themes::PopupMenuItemImagePosition(const Rect &r, int imagewidth, int imageheight)
    {
        auto sizes = MenuSizes();
        int checkwidth = max(imagewidth, sizes.popcheck.cx);
        int checkheight = max(imageheight, sizes.popcheck.cy);
        return Point(r.left + sizes.popitemmargin.cxLeftWidth + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckbgmargin.cxLeftWidth + (checkwidth - imagewidth) / 2, r.top + sizes.popcheckbgmargin.cyTopHeight + sizes.popcheckmargin.cyTopHeight + (checkheight - imageheight) / 2);
    }

    void Themes::DrawPopupMenuItem(Canvas *c, const Rect &r, const std::wstring& text, const std::wstring& shortcut, ThemeMenuItemStates state, bool checked, bool grouped, bool hassubitem, int imagewidth, int imageheight, int longestshortcutw)
    {
        auto sizes = MenuSizes();
        bool enabled = state == tmisNormal || state == tmisHot;
        bool useimage = imagewidth >= 0 && imageheight >= 0;
        int checkwidth = useimage ? max(imagewidth, sizes.popcheck.cx) : sizes.popcheck.cx;
        int checkheight = useimage ? max(imageheight, sizes.popcheck.cy) : sizes.popcheck.cy;
        if (MenuThemed())
        {
            if (IsPopupMenuItemTransparent(state))
                DrawPopupMenuItemBackground(c, r);
            DrawPopupMenuItemGutter(c, Rect(r.left, r.top, r.left + sizes.popitemmargin.cxLeftWidth + checkwidth + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckmargin.cxRightWidth + sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckbgmargin.cxRightWidth, r.bottom));

            // Everything is inside the item margin.
            Rect r2 = r;
            r2.left += sizes.popitemmargin.cxLeftWidth;
            r2.right -= sizes.popitemmargin.cxRightWidth;

            //if (IsSelected(item))
            themes->DrawPopupMenuItemHighlight(c, r2, state);

            // Draw checkbox, bullet:
            //if (useimage)
            //    imagelist->Draw(c, imageindex, sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckmargin.cxLeftWidth + (checkwidth - imagelist->Width()) / 2, r2.top + sizes.popcheckbgmargin.cyTopHeight + sizes.popcheckmargin.cyTopHeight + (checkheight - imagelist->Height()) / 2);
            if (checked)
            {
                DrawPopupMenuCheckBackground(c, RectS(r2.left + sizes.popcheckbgmargin.cxLeftWidth, r2.top + sizes.popcheckbgmargin.cyTopHeight, checkwidth + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckmargin.cxRightWidth, checkheight + sizes.popcheckmargin.cyTopHeight + sizes.popcheckmargin.cyBottomHeight), enabled ? tmcbgNormal : tmcbgDisabled);
                DrawPopupMenuCheck(c, RectS(r2.left + sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckmargin.cxLeftWidth + (checkwidth - sizes.popcheck.cx) / 2, r2.top + sizes.popcheckbgmargin.cyTopHeight + sizes.popcheckmargin.cyTopHeight + (checkheight - sizes.popcheck.cy) / 2, sizes.popcheck.cx, sizes.popcheck.cy), state, enabled ? (grouped ? tmcBulletNormal : tmcCheckmarkNormal) : (grouped ? tmcBulletDisabled : tmcCheckmarkDisabled) );
            }
            r2.left += checkwidth + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckmargin.cxRightWidth + sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckbgmargin.cxRightWidth;

            r2.left += sizes.popitembgbordersiz;
            Size s = themes->MeasurePopupMenuItemTextExtent(c, text, tdoSingleLine);

            Rect r3 = r2;
            r3.top += (r3.Height() - sizes.itemtextheight) / 2;

            // Text:
            themes->DrawPopupMenuItemText(c, r3, text, tdoSingleLine, state);

            // Shortcut:
            if (!shortcut.empty())
            {
                r3 = r2;
                r3.top += (r3.Height() - sizes.itemtextheight) / 2;
                s = themes->MeasurePopupMenuItemTextExtent(c, shortcut, tdoSingleLine);
                r3.left = r3.right - sizes.popupsubmnusiz.cx - sizes.popupsubmnucontent.cxLeftWidth - sizes.popupsubmnucontent.cxRightWidth; // Exclude triangle size
                r3.left -= s.cx;
                themes->DrawPopupMenuItemText(c, r3, shortcut, tdoSingleLine | tdoNoPrefix, state);
            }

            // Small triangle:
            if (hassubitem)
            {
                //if (addhovered)
                //    themes->DrawMenuItem(c, Rect(r2.right - sizes.popupsubmnusiz.cx - sizes.popupsubmnucontent.cxLeftWidth - sizes.popupsubmnucontent.cxRightWidth, r2.top, r2.right, r2.bottom), tmisHot);

                themes->DrawSubMenuGlyph(c, RectS(r2.right - sizes.popupsubmnucontent.cxRightWidth - sizes.popupsubmnusiz.cx, r2.top + sizes.popupsubmnucontent.cyTopHeight + sizes.popcheckmargin.cyTopHeight, sizes.popupsubmnusiz.cx, sizes.popupsubmnusiz.cy), state, enabled ? tsmgNormal : tsmgDisabled);
            }
        }
        else
        {
            DrawPopupMenuItemHighlight(c, r, state);

            // Draw image, checkbox, bullet:
            if (checked)
                DrawPopupMenuCheck(c, RectS(r.left + 1 + (checkwidth - sizes.checksize.cx) / 2, r.top + (r.Height() - sizes.checksize.cy) / 2, sizes.checksize.cx, sizes.checksize.cy), state, enabled ? (grouped ? tmcBulletNormal : tmcCheckmarkNormal) : (grouped ? tmcBulletDisabled : tmcCheckmarkDisabled));

            // Text
            Rect r2 = r;
            r2.left += max(checkwidth, sizes.iconsize) + sizes.icontextdist;
            r2.right -= sizes.textmargin + sizes.itemmargin;
            r2.top += (r.Height() - sizes.itemtextheight) / 2;
            DrawPopupMenuItemText(c, r2, text, tdoSingleLine, state);

            // Shortcut:
            if (!shortcut.empty())
            {
                if (!longestshortcutw)
                    r2.left += MeasurePopupMenuItemTextExtent(c, text, tdoSingleLine).cx + sizes.textmargin + sizes.shortcutmargin;
                r2.right = r.right - sizes.itemmargin;
                if (longestshortcutw)
                    r2.left = r2.right - longestshortcutw;
                DrawPopupMenuItemText(c, r2, shortcut, tdoSingleLine | tdoNoPrefix, state);
            }
            //r2.top = r.top;

            // Small triangle:
            if (hassubitem)
            {
                //if (addhovered)
                //    themes->DrawMenuItem(c, Rect(r.right - sizes.itemmargin, r.top, r.right, r.bottom), tmisHot);
                DrawSubMenuGlyph(c, RectS(r.right - sizes.submargin - sizes.subsize, r.top + (r.Height() - sizes.subsize) / 2, sizes.subsize, sizes.subsize), state, enabled ? tsmgNormal : tsmgDisabled);
            }
        }
    }

    bool Themes::MenuThemed()
    {
        return AppThemed() && IsThemePartDefined(ThemeHandle(tcMenu), MENU_POPUPITEM, 0);
    }

    bool Themes::VistaListViewThemed()
    {
        return AppThemed() && IsThemePartDefined(ThemeHandle(tcExplorerListview), LVP_LISTITEM, 0);
    }

    void Themes::DrawVistaListViewItem(Canvas *canvas, const Rect &r, ThemeListViewItemStates state)
    {
        if (!AppThemed() || !IsThemePartDefined(ThemeHandle(tcExplorerListview), LVP_LISTITEM, 0))
            return;

        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";
        DrawThemeBackground(ThemeHandle(tcExplorerListview), dc, LVP_LISTITEM, state == tlviDisabled ? LISS_DISABLED : state == tlviHot ? LISS_HOT : state == tlviHotSelected ? LISS_HOTSELECTED : state == tlviSelected ? LISS_SELECTED : state == tlviSelectedNotFocused ? LISS_SELECTEDNOTFOCUS : LISS_NORMAL, &r, NULL);
        canvas->ReturnDC();
    }

    void Themes::DrawVistaListViewItemClipped(Canvas *canvas, const Rect &r, ThemeListViewItemStates state, const Rect &clip)
    {
        if (!AppThemed() || !IsThemePartDefined(ThemeHandle(tcExplorerListview), LVP_LISTITEM, 0))
            return;

        HDC dc = canvas->GetDC();
        if (!dc)
            throw L"Couldn't acquire dc from canvas!";
        DrawThemeBackground(ThemeHandle(tcExplorerListview), dc, LVP_LISTITEM, state == tlviDisabled ? LISS_DISABLED : state == tlviHot ? LISS_HOT : state == tlviHotSelected ? LISS_HOTSELECTED : state == tlviSelected ? LISS_SELECTED : state == tlviSelectedNotFocused ? LISS_SELECTEDNOTFOCUS : LISS_NORMAL, &r, &clip);
        canvas->ReturnDC();
    }

    //void Themes::DrawWindowCloseButton(Canvas *canvas, const Rect &r, ThemeWindowCloseButtonStates state)
    //{
    //    if (AppThemed())
    //    {
    //        HDC dc = canvas->GetDC();
    //        if (!dc)
    //            throw L"Couldn't acquire dc from canvas!";
    //        DrawThemeBackground(ThemeHandle(tcWindow), dc, WP_CLOSEBUTTON, state == wcbDisabled ? CBS_DISABLED : state == wcbHot ? CBS_HOT : state == wcbNormal ? CBS_NORMAL : CBS_PUSHED, &r, NULL);
    //        canvas->ReturnDC();
    //    }
    //    else
    //    {
    //#error No themeing not done yet!
    //    }
    //}

    Size Themes::MeasureMenubarItem(int imagewidth)
    {
        auto sizes = themes->MenuSizes();
        return Size(sizes.baritemmargin.cxLeftWidth + sizes.baritemmargin.cxRightWidth + (imagewidth <= 0 ? 0 : imagewidth + 4), GetSystemMetrics(SM_CYMENU) - 1);
    }

    Size Themes::MeasurePopupMenuItem(int imagewidth, int imageheight)
    {
        auto sizes = MenuSizes();
        if (MenuThemed())
        {
            return Size(max(imagewidth, sizes.popcheck.cx) + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckmargin.cxRightWidth +
                        sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckbgmargin.cxRightWidth +
                        sizes.popitemmargin.cxLeftWidth + sizes.popitemmargin.cxRightWidth +
                        sizes.popitembordersiz + sizes.popitembgbordersiz + sizes.popupsubmnusiz.cx +
                        sizes.popupsubmnucontent.cxLeftWidth + sizes.popupsubmnucontent.cxRightWidth //+ sizes.popitemtabwidth
                        , // Height
                        max(imageheight, sizes.popcheck.cy) + sizes.popcheckmargin.cyTopHeight + sizes.popcheckmargin.cyBottomHeight +
                        sizes.popcheckbgmargin.cyTopHeight + sizes.popcheckbgmargin.cyBottomHeight);
        }
        else
            return Size(max(imagewidth, sizes.iconsize) + sizes.icontextdist + sizes.itemmargin + sizes.textmargin + sizes.shortcutmargin, max(imageheight, sizes.rowheight) + sizes.rowmargin * 2);
    }

    Size Themes::MeasurePopupMenuSeparator(int imagewidth)
    {
        auto sizes = MenuSizes();
        if (MenuThemed())
        {
            return Size(max(imagewidth, sizes.popcheck.cx) + sizes.popcheckmargin.cxLeftWidth + sizes.popcheckmargin.cxRightWidth +
                        sizes.popcheckbgmargin.cxLeftWidth + sizes.popcheckbgmargin.cxRightWidth +
                        sizes.popitemmargin.cxLeftWidth + sizes.popitemmargin.cxRightWidth +
                        sizes.popitembordersiz + sizes.popitembgbordersiz + sizes.popupsubmnusiz.cx +
                        sizes.popupsubmnucontent.cxLeftWidth + sizes.popupsubmnucontent.cxRightWidth //+ sizes.popitemtabwidth
                        , // Height
                        sizes.popitemmargin.cyTopHeight + sizes.popitemmargin.cyBottomHeight + sizes.popsep.cy);
        }
        else
            return Size(max(imagewidth, sizes.iconsize) + sizes.icontextdist + sizes.itemmargin + sizes.textmargin + sizes.shortcutmargin, sizes.sepheight);
    }

    Size Themes::MeasureTreeviewGlyph(ThemeTreeGlyphStates glyphstate)
    {
        return MeasureGenTreeviewGlyph(glyphstate, false);
    }

    Size Themes::MeasureVistaTreeviewGlyph(ThemeTreeGlyphStates glyphstate)
    {
        return MeasureGenTreeviewGlyph(glyphstate, true);
    }

    Size Themes::MeasureGenTreeviewGlyph(ThemeTreeGlyphStates glyphstate, bool usevista)
    {
        Size s;
        if (AppThemed())
        {
            HDC dc = GetDC(0);
            if (usevista && Win32MajorVersion >= 6)
                GetThemePartSize(ThemeHandle(tcExplorerTreeview), dc, glyphstate == ttgsOpen || glyphstate == ttgsClosed ? TVP_GLYPH : TVP_HOTGLYPH, glyphstate == ttgsOpen || glyphstate == ttgsHotOpen ? GLPS_OPENED : GLPS_CLOSED, NULL, TS_TRUE, &s);
            else
                GetThemePartSize(ThemeHandle(tcTreeview), dc, glyphstate == ttgsOpen || glyphstate == ttgsClosed ? TVP_GLYPH : TVP_HOTGLYPH, glyphstate == ttgsOpen || glyphstate == ttgsHotOpen ? GLPS_OPENED : GLPS_CLOSED, NULL, TS_DRAW, &s);
            ReleaseDC(0, dc);
        }
        else
        {
            s = Size(LONG(9 * Scaling + 0.5),LONG(9 * Scaling));
            if ((s.cx % 2) == 0)
                s.cx++;
            if ((s.cy % 2) == 0)
                s.cy++;
        }
        return s;
    }

    Rect Themes::MeasureButtonBackgroundArea(const Rect &clientrect)
    {
        if (AppThemed())
        {
            Rect r;
            HDC dc = GetDC(0);
            GetThemeBackgroundContentRect(ThemeHandle(tcButton), dc, BP_PUSHBUTTON, PBS_NORMAL, &clientrect, &r); // Measures the area covered by the elem bg if its window bg is same as r2.
            ReleaseDC(0, dc);
            return r;
        }
        else
            return clientrect;
    }

    Rect Themes::MeasureButtonFocusRectangle(const Rect &clientrect)
    {
        if (AppThemed())
        {
            Rect r;
            HDC dc = GetDC(0);
            GetThemeBackgroundContentRect(ThemeHandle(tcButton), dc, BP_PUSHBUTTON, PBS_NORMAL, &clientrect, &r); // Measures the area covered by the elem bg if its window bg is same as r2.
            ReleaseDC(0, dc);
            return r;
        }
        else
            return clientrect.Inflate(-4, -4);
    }

    Size Themes::MeasureEditBorderWidth()
    {
        if (AppThemed())
        {
            HDC dc = GetDC(0);
            Size s;

            Rect r = Rect(0, 0, 100, 100);
            GetThemePartSize(ThemeHandle(tcEdit), dc, EP_EDITTEXT, EPSN_NORMAL, &r, TS_MIN, &s);
            ReleaseDC(0, dc);
            return s;
        }
        else
        {
            Size s = Size(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE));
            return s;
        }
    }

    Size Themes::MeasureHeaderItemPartSize(ThemePartSizes partsize, ThemeHeaderItemStates state)
    {
        Size s;
        if (AppThemed())
        {
            HDC dc = GetDC(0);
            Rect measurerect(0, 0, 100, 100);
            GetThemePartSize(ThemeHandle(tcHeader), dc, HP_HEADERITEM, state == thisNormal ? HIS_NORMAL : state == thisHot ? HIS_HOT : state == thisPressed ? HIS_PRESSED : state == thisSortedNormal ? HIS_SORTEDNORMAL : state == thisSortedHot ? HIS_SORTEDHOT : state == thisSortedPressed ? HIS_SORTEDPRESSED : state == thisIconNormal ? HIS_ICONNORMAL : state == thisIconHot ? HIS_ICONHOT : state == thisIconPressed ? HIS_ICONPRESSED : state == thisIconSortedNormal ? HIS_ICONSORTEDNORMAL : state == thisIconSortedHot ? HIS_ICONSORTEDHOT : state == thisIconSortedPressed ? HIS_ICONSORTEDPRESSED : 0, &measurerect, TS_TRUE, &s);
            ReleaseDC(0, dc);
        }
        else
            s = Size(6 * Scaling, 24 * Scaling);

        return s;
    }

    Size Themes::MeasureToolbarDropButtonGlyph()
    {
        Size s;
        if (AppThemed())
        {
            HDC dc = GetDC(0);
            Rect measurerect(0, 0, 100, 100);
            GetThemePartSize(ThemeHandle(tcToolbar), dc, TP_DROPDOWNBUTTONGLYPH, TS_HOT, &measurerect, TS_TRUE, &s);
            ReleaseDC(0, dc);
        }
        else
            s = Size(5, 5);

        return s;
    }

    const ThemePopupMenuSizes& Themes::MenuSizes()
    {
        if (sizes.itemtextheight == -1)
        {
            const wchar_t *measuretext = L"aAwWmMyYjJlL";

            if (MenuThemed())
            {
                HTHEME hTheme = ThemeHandle(tcMenu);

                GetThemePartSize(hTheme, NULL, MENU_POPUPCHECK, 0, NULL, TS_TRUE, &sizes.popcheck);
                GetThemePartSize(hTheme, NULL, MENU_POPUPSEPARATOR, 0, NULL, TS_TRUE, &sizes.popsep);

                GetThemeInt(hTheme, MENU_POPUPITEM, 0, TMT_BORDERSIZE, &sizes.popitembordersiz);
                GetThemeInt(hTheme, MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &sizes.popitembgbordersiz);

                GetThemeMargins(hTheme, NULL, MENU_POPUPCHECK, 0, TMT_CONTENTMARGINS, NULL, &sizes.popcheckmargin);
                GetThemeMargins(hTheme, NULL, MENU_POPUPCHECKBACKGROUND, 0, TMT_CONTENTMARGINS, NULL, &sizes.popcheckbgmargin);
                GetThemeMargins(hTheme, NULL, MENU_POPUPITEM, 0, TMT_CONTENTMARGINS, NULL, &sizes.popitemmargin);

                GetThemeMargins(hTheme, NULL, MENU_BARITEM, 0, TMT_CONTENTMARGINS, NULL, &sizes.baritemmargin);
                GetThemePartSize(hTheme, NULL, MENU_POPUPBORDERS, 0, NULL, TS_TRUE, &sizes.popupbordersiz);
                GetThemePartSize(hTheme, NULL, MENU_POPUPSUBMENU, 0, NULL, TS_TRUE, &sizes.popupsubmnusiz);
                GetThemeMargins(hTheme, NULL, MENU_POPUPSUBMENU, 0, TMT_CONTENTMARGINS, NULL, &sizes.popupsubmnucontent);


                HDC dc = GetDC(0);
                if (!dc)
                    throw L"Couldn't acquire a dc.";
                //HFONT oldfont = 0;
                try
                {
                    //oldfont = (HFONT)SelectObject(dc, application->MenuFont().Handle());

                    Rect tmp;
                    GetThemeTextExtent(ThemeHandle(tcMenu), dc, MENU_BARITEM, 0, measuretext, 12, DT_SINGLELINE, NULL, &tmp);
                    sizes.bartextheight = tmp.Height();
                    GetThemeTextExtent(ThemeHandle(tcMenu), dc, MENU_POPUPITEM, 0, measuretext, 12, DT_SINGLELINE, NULL, &tmp);
                    sizes.itemtextheight = tmp.Height();
                    GetThemeTextExtent(ThemeHandle(tcMenu), dc, MENU_POPUPITEM, 0, L"    ", 4, tdoSingleLine | tdoExpandTabs, NULL, &tmp);
                    sizes.popitemtabwidth = tmp.Width();

                    //SelectObject(dc, oldfont);
                    //oldfont = 0;
                }
                catch(...)
                {
                    //if (oldfont)
                    //    SelectObject(dc, oldfont);
                    ;
                }
                ReleaseDC(0, dc);
            }
            else // XP and older draws its menu without themeing with the same sizes as the classic non-themed menu.
            {
                HDC dc = GetDC(0);
                if (!dc)
                    throw L"Couldn't acquire a dc.";
                HFONT oldfont = 0;
                try
                {
                    sizes.baritemmargin.cxLeftWidth = sizes.baritemmargin.cxRightWidth = 8;
                    sizes.baritemmargin.cyTopHeight = sizes.baritemmargin.cyBottomHeight = 3;
                    sizes.popupbordersiz = Size(3, 3);

                    float fontscaling = Win32MajorVersion < 6 ? application->MenuFont().Size() / 8.0 : 1.0;

                    oldfont = (HFONT)SelectObject(dc, application->MenuFont().Handle());
                    Rect tmp;
                    DrawText(dc, measuretext, 12, &tmp, DT_SINGLELINE | DT_CALCRECT);
                    sizes.bartextheight = sizes.itemtextheight = tmp.Height();

                    sizes.iconsize = 16 * Scaling;
                    sizes.checksize = BitmapSize((HBITMAP)LoadImage(NULL, MAKEINTRESOURCE(32760 /* OBM_CHECK */), IMAGE_BITMAP, 0, 0, LR_SHARED)); //13.6 * Scaling + max(0, 12 * (fontscaling - 1));
                    sizes.rowmargin = 2;
                    sizes.rowheight = max(sizes.iconsize, sizes.itemtextheight);
                    sizes.sepheight = 9.6 * Scaling;
                    sizes.icontextdist = 1 + max(0, 14 * (fontscaling - 1));
                    sizes.textmargin = 3 * Scaling * fontscaling;
                    sizes.itemmargin = 19 * Scaling * fontscaling;
                    sizes.shortcutmargin = 4 * Scaling * fontscaling;
                    sizes.subsize = 12 * fontscaling * Scaling;
                    sizes.submargin = 1 * Scaling;

                    SelectObject(dc, oldfont);
                    oldfont = 0;
                }
                catch(...)
                {
                    if (oldfont)
                        SelectObject(dc, oldfont);
                }
                ReleaseDC(0, dc);
            }
        }
        return sizes;
    }

    void Themes::TestMeasure(ThemeClasses tclass, int partid, int stateid, const Rect &measurerect)
    {
        HDC dc = GetDC(0);

        Point pt1, pt2, pt3, pt4, pt5, pt6, pt7;
        Size s1, s2, s3;
        Rect r1, r2, r3, r4;

        GetThemePartSize(ThemeHandle(tclass), dc, partid, stateid, const_cast<Rect*>(&measurerect), TS_MIN, &s1);
        GetThemePartSize(ThemeHandle(tclass), dc, partid, stateid, const_cast<Rect*>(&measurerect), TS_TRUE, &s2);
        GetThemePartSize(ThemeHandle(tclass), dc, partid, stateid, const_cast<Rect*>(&measurerect), TS_DRAW, &s3);

        GetThemeRect(ThemeHandle(tclass), partid, stateid, TMT_ANIMATIONBUTTONRECT, &r1);
        GetThemeRect(ThemeHandle(tclass), partid, stateid, TMT_ATLASRECT, &r2);
        GetThemeRect(ThemeHandle(tclass), partid, stateid, TMT_CUSTOMSPLITRECT, &r3);
        GetThemeRect(ThemeHandle(tclass), partid, stateid, TMT_DEFAULTPANESIZE, &r4);

        int i1, i2, i3, i4, i5, i6;
        GetThemeInt(ThemeHandle(tclass), partid, stateid, TMT_BORDERSIZE, &i1);
        GetThemeInt(ThemeHandle(tclass), partid, stateid, TMT_HEIGHT, &i2);
        GetThemeInt(ThemeHandle(tclass), partid, stateid, TMT_PROGRESSCHUNKSIZE, &i3);
        GetThemeInt(ThemeHandle(tclass), partid, stateid, TMT_PROGRESSSPACESIZE, &i4);
        GetThemeInt(ThemeHandle(tclass), partid, stateid, TMT_TEXTBORDERSIZE, &i5);
        GetThemeInt(ThemeHandle(tclass), partid, stateid, TMT_WIDTH, &i6);

        int im1, im2, im3, im4, im5, im6;
        GetThemeMetric(ThemeHandle(tclass), dc, partid, stateid, TMT_BORDERSIZE, &im1);
        GetThemeMetric(ThemeHandle(tclass), dc, partid, stateid, TMT_HEIGHT, &im2);
        GetThemeMetric(ThemeHandle(tclass), dc, partid, stateid, TMT_PROGRESSCHUNKSIZE, &im3);
        GetThemeMetric(ThemeHandle(tclass), dc, partid, stateid, TMT_PROGRESSSPACESIZE, &im4);
        GetThemeMetric(ThemeHandle(tclass), dc, partid, stateid, TMT_TEXTBORDERSIZE, &im5);
        GetThemeMetric(ThemeHandle(tclass), dc, partid, stateid, TMT_WIDTH, &im6);
        if (i1 != im1 || i2 != im2 || i3 != im3 || i4 != im4 || i5 != im5 || i6 != im6)
            throw L"Difference in metric and int value.";

        MARGINS margins1, margins2, margins3;
        GetThemeMargins(ThemeHandle(tclass), dc, partid, stateid, TMT_CAPTIONMARGINS, NULL, &margins1);
        GetThemeMargins(ThemeHandle(tclass), dc, partid, stateid, TMT_CONTENTMARGINS, NULL, &margins2);
        GetThemeMargins(ThemeHandle(tclass), dc, partid, stateid, TMT_SIZINGMARGINS, NULL, &margins3);

        GetThemePosition(ThemeHandle(tclass), partid, stateid, TMT_MINSIZE, &pt1);
        GetThemePosition(ThemeHandle(tclass), partid, stateid, TMT_MINSIZE1, &pt2);
        GetThemePosition(ThemeHandle(tclass), partid, stateid, TMT_MINSIZE2, &pt3);
        GetThemePosition(ThemeHandle(tclass), partid, stateid, TMT_MINSIZE5, &pt4);
        GetThemePosition(ThemeHandle(tclass), partid, stateid, TMT_NORMALSIZE, &pt5);
        GetThemePosition(ThemeHandle(tclass), partid, stateid, TMT_OFFSET, &pt6);
        GetThemePosition(ThemeHandle(tclass), partid, stateid, TMT_TEXTSHADOWOFFSET, &pt7);

        int i7 = GetThemeSysSize(ThemeHandle(tclass), SM_CXBORDER);
        int i8 = GetThemeSysSize(ThemeHandle(tclass), SM_CXVSCROLL);
        int i9 = GetThemeSysSize(ThemeHandle(tclass), SM_CXHSCROLL);
        int i10 = GetThemeSysSize(ThemeHandle(tclass), SM_CXSIZE);
        int i11 = GetThemeSysSize(ThemeHandle(tclass), SM_CYSIZE);
        int i12 = GetThemeSysSize(ThemeHandle(tclass), SM_CXSMSIZE);
        int i13 = GetThemeSysSize(ThemeHandle(tclass), SM_CYSMSIZE);
        int i14 = GetThemeSysSize(ThemeHandle(tclass), SM_CXMENUSIZE);
        int i15 = GetThemeSysSize(ThemeHandle(tclass), SM_CYMENUSIZE);
        int i16 = GetThemeSysSize(ThemeHandle(tclass), SM_CXPADDEDBORDER);


        Rect rm = measurerect;
        Rect rc, re;
        GetThemeBackgroundContentRect(ThemeHandle(tclass), dc, partid, stateid, &rm, &rc); // Measures the area covered by the elem bg if its window bg is same as r2.
        GetThemeBackgroundExtent(ThemeHandle(tclass), dc, partid, stateid, &rc, &re); // Measure area required for the drawn elem if its bg is same as r. (I.e. if it has a margin which isn't drawn onto, r2 is expanded.)

        ReleaseDC(0, dc);
    }



}
/* End of NLIBNS */

