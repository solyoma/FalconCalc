#pragma once

#include "designproperties.h"
#include "screen.h"
#include "canvas.h"

//---------------------------------------------


namespace NLIBNS
{


// Properties

// Cursors list
// Global for the largest rectangle a system cursor can fit in.
extern Rect cdp__cursorrect__g_zapp; // Largest rectangle which can hold the image of any cursor in the system.

extern ValuePair<Cursors> CursorStrings[];
template<typename PropertyHolder>
class GeneralCursorDesignProperty : public GeneralListDesignProperty<PropertyHolder, Cursors>
{
private:
    typedef GeneralListDesignProperty<PropertyHolder, Cursors> base;

    void ComputeCursorrect()
    {
        if (cdp__cursorrect__g_zapp.Width() > 0)
            return;

        Rect r;
        int fc;
        Cursors cr[] = { cDefault, cNormal, cIBeam, cWait, cCross, cUp, cSizeNWSE, cSizeNESW, cSizeWE, cSizeNS, cSizeAll, cNo, cHand, cAppStarting, (Cursors)-1 };
        for (int ix = 0; (int)cr[ix] != -1; ix++)
        {
            if (screencursor->CursorSize(cr[ix], r, fc))
            {
                cdp__cursorrect__g_zapp.right = max(cdp__cursorrect__g_zapp.right, r.Width());
                cdp__cursorrect__g_zapp.bottom = max(cdp__cursorrect__g_zapp.right, r.Height());
            }
        }
    }

    Rect CursorRect()
    {
        ComputeCursorrect();
        return cdp__cursorrect__g_zapp;
    }

public:
    GeneralCursorDesignProperty(const std::wstring &name, const std::wstring &category, typename base::readertype *reader) : base(name, category, reader, CursorStrings, (int)cCount)
    {
        this->propertystyle << psThumbImage;
        this->propertystyle << psDrawItem;
    }

    virtual void DrawThumbImage(Object *propholder, Canvas *c, const Rect &r, int index)
    {
        ComputeCursorrect();

        Cursors val = this->CallGetter(propholder);

        bool drawdefault = false;
        if (val == cDefault)
        {
            val = cNormal;
            drawdefault = true;
        }

        Rect cr;
        int fc;
        if (!screencursor->CursorSize(val, cr, fc))
            return;

        Bitmap bmp(cr.Width(), cr.Height());
        bmp.SetBrush(clWhite);
        bmp.FillRect(Rect(0, 0, cr.Width(), cr.Height()));
        bmp.DrawCursor(val, -cr.left, -cr.top, 0); 

        float div(min(1, min(float(r.Width()) / cr.Width(), float(r.Height()) / cr.Height())) );
        RectF rf = RectFS(0, 0, cr.Width() * div, cr.Height() *div);

        if (drawdefault)
        {
            ColorMatrix cm;
            cm.TransformAlpha(0.3F);
            c->SetColorMatrix(cm);
        }

        c->DrawF(&bmp, r.left + ((float)r.Width() - rf.Width()) / 2, r.top + ((float)r.Height() - rf.Height()) / 2, min(r.Width(), rf.Width()), min(r.Height(), rf.Height()), 0, 0, bmp.Width(), bmp.Height());

        if (drawdefault)
            c->ResetColorMatrix();
    }

    virtual void MeasureListItem(Object *propholder, MeasureItemParameters param)
    {
        ComputeCursorrect();
        param.height = CursorRect().Height() + int(2 * Scaling) * 2; 
    }

    virtual void DrawListItem(Object *propholder, DrawItemParameters param)
    {
        ComputeCursorrect(); 

        Canvas *c = param.canvas;

        Rect cr;
        int fc;
        if (screencursor->CursorSize(this->ListItemValue(propholder, param.index), cr, fc))
            c->DrawCursor(this->ListItemValue(propholder, param.index), param.rect.left - cr.left + int(2 * Scaling) + (CursorRect().Width() - cr.Width()) / 2, param.rect.top - cr.top + (param.rect.Height() - cr.Height()) / 2);

        param.rect.left += int(2 * Scaling) + CursorRect().Width() + int(2 * Scaling);
        std::wstring str = this->ListItem(propholder, param.index);
        Size s = c->MeasureText(str);

        c->TextDraw(param.rect, param.rect.left, param.rect.top + (param.rect.Height() - s.cy) / 2, str);
    }
};

template<typename PropertyHolder>
class CursorDesignProperty : public GeneralCursorDesignProperty<PropertyHolder>
{
private:
    typedef GeneralCursorDesignProperty<PropertyHolder> base;
public:
    template<typename GetterProc, typename SetterProc>
    CursorDesignProperty(const std::wstring &name, const std::wstring &category, GetterProc getter, SetterProc setter) : base(name, category, CreatePropertyReader<PropertyHolder, Cursors>(getter, setter))
    { }
};


}
/* End of NLIBNS */

