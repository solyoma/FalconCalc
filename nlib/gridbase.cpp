#include "stdafx_zoli.h"
#include "gridbase.h"
#include "themes.h"
#include "syscontrol.h"
#include "application.h"
#include "helperwindows.h"
#include "sparse_list.h"
#ifdef DESIGNING
#include "designerform.h"
#include "property_controlbase.h"
#include "property_gridbase.h"
#include "serializer.h"
#include "designer.h"
#endif


// Half of the width of the drawn indicator for new placement of dragged columns and rows.
#define DRAG_MOVE_W     1


//---------------------------------------------


namespace NLIBNS
{


#ifdef DESIGNING

ValuePair<GridOptions> GridDrawOptionStrings[] = {
            VALUEPAIR(goVertLines),
            VALUEPAIR(goHorzLines),
            VALUEPAIR(goLinesFillVert),
            VALUEPAIR(goLinesFillHorz),
            VALUEPAIR(goAlwaysShowSel),
            VALUEPAIR(goAllowNoSelect),
            VALUEPAIR(goFixedTracking),
            VALUEPAIR(goCellTracking),
            VALUEPAIR(goMultiSelect),
};

ValuePair<GridSelectionKinds> GridSelectionKindStrings[] = {
            VALUEPAIR(gskNoSelect),
            VALUEPAIR(gskCellSelect),
            VALUEPAIR(gskRowSelect),
            VALUEPAIR(gskColSelect),
};

void GridBase::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);

    serializer->HideProperty(L"Text");
    serializer->Find<BoolDesignProperty<Control>>(L"ParentColor")->SetDefault(false);
    serializer->Find<BorderStylesDesignProperty<Control>>(L"BorderStyle")->SetDefault(bsModern);
    serializer->Find(L"ParentBackground")->Hide();
    serializer->Find<WantedKeySetDesignProperty<Control>>(L"WantedKeyTypes")->SetDefault(wkOthers | wkArrows);

    serializer->Add(L"SetOptions", new GridOptionsDesignProperty<GridBase>(L"Options", L"Appearance", &GridBase::Options, &GridBase::SetOptions))->SetDefault(goHorzLines | goVertLines);
    serializer->Add(L"SetGridColor", new ColorDesignProperty<GridBase>(L"GridColor", L"Appearance", true, false, false, &GridBase::GridColor, &GridBase::SetGridColor))->SetDefault(Color(clWindow).Mix(clWindowText, 0.93));
}

Size GridBase::DesignSize()
{
    return Size(320, 152);
}
#endif

GridBase::GridBase(bool smoothx, bool smoothy) :
        smoothx(smoothx), smoothy(smoothy), options(goHorzLines | goVertLines), gridcolor(Color(clWindow).Mix(clWindowText, 0.93)),
        mcoledge(-1), mcell(-1, -1), mpos(-1, -1), maction(gmaNone)
{
    controlstyle << csInTabOrder << csAcceptInput << csNoErase;
    controlstyle -= csParentBackground | csParentColor | csUpdateOnTextChange;
    SetBorderStyle(bsModern);
    SetColor(clWindow);
    SetAutoSizeScroll(true);
    SetWantedKeyTypes(wkOthers | wkArrows);
}

//LRESULT GridBase::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//    return base::WindowProc(uMsg, wParam, lParam);
//}

int GridBase::FixColW()
{
    int cnt = FixColCnt();
    int w = 0;
    for (int ix = 0; ix < cnt; ++ix)
        w += ColW(ix);
    return w;
}

int GridBase::FixRowH()
{
    int cnt = FixRowCnt();
    int h = 0;
    for (int ix = 0; ix < cnt; ++ix)
        h += RowH(ix);
    return h;
}

void GridBase::FirstCol(int &col, int *colx, int *colw)
{
    int fcnt = FixColCnt();
    int cnt = ColCnt();

    if (fcnt == cnt)
    {
        col = -1;
        return;
    }

    if (!smoothx)
    {
        col = HPos() + fcnt;
        if (col >= cnt)
            col = -1;
        else
        {
            if (colx)
                *colx = FixColW();
            if (colw)
                *colw = ColW(col);
        }
        return;
    }

    int siz;
    int left;

    int *pleft = colx ? colx : &left;
    int *psiz = colw ? colw : &siz;

    int fw = FixColW();

    col = fcnt;
    for (*pleft = -HPos() + fw, *psiz = 0; col != cnt; ++col, *pleft += *psiz)
    {
        *psiz = ColW(col);
        if (*pleft + *psiz > fw)
            return;
    }
    col = -1;
}

void GridBase::FirstRow(int &row, int *rowy, int *rowh)
{
    int fcnt = FixRowCnt();
    int cnt = RowCnt();

    if (fcnt == cnt)
    {
        row = -1;
        return;
    }

    if (!smoothy)
    {
        row = VPos() + fcnt;
        if (row >= cnt)
            row = -1;
        else
        {
            if (rowy)
                *rowy = FixRowH();
            if (rowh)
                *rowh = RowH(row);
        }
        return;
    }

    int siz;
    int top;

    int *ptop = rowy ? rowy : &top;
    int *psiz = rowh ? rowh : &siz;

    int fw = FixRowH();

    row = fcnt;
    for (*ptop = -VPos() + fw, *psiz = 0; row != cnt; ++row, *ptop += *psiz)
    {
        *psiz = RowH(row);
        if (*ptop + *psiz > fw)
            return;
    }
    row = -1;
}

void GridBase::Paint(const Rect &updaterect)
{
    Canvas *c = GetCanvas();

    int siz;
    Rect cr = ClientRect();

    int fixw = FixColW();
    int fixh = FixRowH();

    // Find the first fixed corner column to be drawn and draw the corner.
    int fleft = 0;
    int ffirstcol = 0;
    int fccnt = FixColCnt();
    for ( ; ffirstcol < fccnt; ++ffirstcol)
    {
        siz = ColW(ffirstcol);
        if (fleft + siz > updaterect.left)
            break;
        fleft += siz;
    }

    // Draw the corner which never scrolls.
    int ftop = 0;
    int frcnt = FixRowCnt();
    for (int row = 0; row < frcnt && ftop < updaterect.bottom; ++row)
    {
        int h = RowH(row);
        if (ftop + h < updaterect.top)
            continue;
        int x = fleft;
        int col = ffirstcol;
        for ( ; col < fccnt && x < updaterect.right; ++col)
        {
            int w = ColW(col);
            Rect r = Rect(x, ftop, x + w, ftop + h);

            DrawHead(c, col, row, r, r, options.contains(goFixedTracking) && maction == gmaNone && mcell == Point(col, row));

            if (maction == gmaColMove && col == mcoledge)
            {
                Brush b = c->GetBrush();
                c->SetBrush(clHotlight);
                c->FillRect(r.left - DRAG_MOVE_W, r.top, r.left + DRAG_MOVE_W, r.bottom);
                c->SetBrush(b);
            }
            else if (maction == gmaRowMove && row == mrowedge)
            {
                Brush b = c->GetBrush();
                c->SetBrush(clHotlight);
                c->FillRect(r.left, r.top - DRAG_MOVE_W, r.right, r.bottom + DRAG_MOVE_W);
                c->SetBrush(b);
            }

            x += w;
        }
        ftop += h;
    }

    // Find the first column and its left coordinate that will be drawn.
    int left;
    int firstcol;
    int ccnt = ColCnt();
    FirstCol(firstcol, &left);
    if (firstcol < 0)
    {
        firstcol = fccnt;
        left = fixw;
    }
    else
    {
        for ( ; firstcol < ccnt; ++firstcol)
        {
            siz = ColW(firstcol);
            if (left + siz > updaterect.left)
                break;
            left += siz;
        }
    }

    // Find the first row and its top coordinate that will be drawn.
    int top;
    int firstrow;
    int rcnt = RowCnt();
    FirstRow(firstrow, &top);
    if (firstrow < 0)
    {
        firstrow = frcnt;
        top = fixh;
    }
    else
    {
        for ( ; firstrow < rcnt; ++firstrow)
        {
            siz = RowH(firstrow);
            if (top + siz > updaterect.top)
                break;
            top += siz;
        }
    }

    Gdiplus::Region clip;
    bool hasclip = smoothx || smoothy ? c->GetClip(clip) : false;

    // Draw the fixed header columns to the right of the corner.
    if (updaterect.top < fixh)
    {
        ftop = 0;

        if (smoothx)
            c->IntersectClip(Rect(max(updaterect.left, fixw), updaterect.top, updaterect.right, min(updaterect.bottom, fixh)));

        for (int row = 0; row < frcnt && ftop < updaterect.bottom; ++row)
        {
            int h = RowH(row);
            if (ftop + h < updaterect.top)
                continue;
            int x = left;
            int col = firstcol;
            for ( ; col < ccnt && x < updaterect.right; ++col)
            {
                int w = ColW(col);
                Rect r = Rect(x, ftop, x + w, ftop + h);
                Rect clip = r;
                clip.left = max(clip.left, fixw);

                DrawHead(c, col, row, r, clip, options.contains(goFixedTracking) && maction == gmaNone && mcell == Point(col, row));
                if (maction == gmaColMove && col == mcoledge)
                {
                    Brush b = c->GetBrush();
                    c->SetBrush(clHotlight);
                    c->FillRect(clip.left - DRAG_MOVE_W, clip.top, clip.left + DRAG_MOVE_W, clip.bottom);
                    c->SetBrush(b);
                }

                x += w;
            }

            int oldx = x;
            if (x < updaterect.right)
            {

                if (options.contains(goLinesFillHorz))
                {
                    int w = ColW(-1);
                    if (w <= 0)
                        themes->DrawHeaderRightSide(c, Rect(x, ftop, cr.right, ftop + h));
                    else
                    {
                        while (x < updaterect.right)
                        {
                            Rect r = Rect(x, ftop, x + w, ftop + h);
                            DrawHead(c, -1, row, r, r, options.contains(goFixedTracking) && maction == gmaNone && mcell == Point(col, row));
                            x += w;
                        }
                    }
                }
                else
                    c->FillRect(x, ftop, updaterect.right, ftop + h);
            }
            if (updaterect.right > oldx - DRAG_MOVE_W && maction == gmaColMove && mcoledge == ccnt && col == ccnt)
            {
                Brush b = c->GetBrush();
                c->SetBrush(clHotlight);
                c->FillRect(oldx - DRAG_MOVE_W, ftop, oldx + DRAG_MOVE_W, ftop + h);
                c->SetBrush(b);
            }

            ftop += h;
        }

        if (hasclip)
            c->SetClip(clip);
        else
            c->ResetClip();
    }

    // Draw the fixed header rows below the corner.
    if (updaterect.left < fixw)
    {
        fleft = 0;

        if (smoothy)
            c->IntersectClip(Rect(updaterect.left, max(updaterect.top, fixh), min(updaterect.right, fixw), updaterect.bottom));

        for (int col = 0; col < fccnt && fleft < updaterect.right; ++col)
        {
            int w = ColW(col);
            if (fleft + w < updaterect.left)
                continue;
            int y = top;
            int row = firstrow;
            for ( ; row < rcnt && y < updaterect.bottom; ++row)
            {
                int h = RowH(row);
                Rect r = Rect(fleft, y, fleft + w, y + h);
                Rect clip = r;
                clip.top = max(clip.top, fixh);

                DrawHead(c, col, row, r, clip, options.contains(goFixedTracking) && maction == gmaNone && mcell == Point(col, row));
                if (maction == gmaRowMove && row == mrowedge)
                {
                    Brush b = c->GetBrush();
                    c->SetBrush(clHotlight);
                    c->FillRect(clip.left, clip.top - DRAG_MOVE_W, clip.right, clip.bottom + DRAG_MOVE_W);
                    c->SetBrush(b);
                }

                y += h;
            }

            int oldy = y;
            if (y < updaterect.bottom)
            {
                if (options.contains(goLinesFillVert))
                {
                    int h = RowH(-1);
                    if (h <= 0)
                        themes->DrawHeaderItem(c, Rect(fleft, y, fleft + w, cr.bottom), thisNormal);
                    else
                    {
                        while (y < updaterect.bottom)
                        {
                            Rect r = Rect(fleft, y, fleft + w, y + h);
                            DrawHead(c, col, -1, r, r, options.contains(goFixedTracking) && maction == gmaNone && mcell == Point(col, row));
                            y += h;
                        }
                    }
                }
                else
                    c->FillRect(fleft, y, fleft + w, updaterect.bottom);
            }
            if (updaterect.bottom > oldy - DRAG_MOVE_W && maction == gmaRowMove && mrowedge == rcnt && row == rcnt)
            {
                Brush b = c->GetBrush();
                c->SetBrush(clHotlight);
                c->FillRect(fleft, oldy - DRAG_MOVE_W, fleft + w, oldy + DRAG_MOVE_W);
                c->SetBrush(b);
            }

            fleft += w;
        }

        if (hasclip)
            c->SetClip(clip);
        else
            c->ResetClip();
    }


    if (smoothx || smoothy)
        c->IntersectClip(Rect(max(updaterect.left, fixw), max(updaterect.top, fixh), cr.right, cr.bottom));

    // Draw the rows
    int y = top;
    for (int row = firstrow ; row < rcnt && top < updaterect.bottom; ++row)
    {
        int h = RowH(row);
        int x = left;
        for (int col = firstcol; col < ccnt && x < updaterect.right; x += siz, ++col)
        {
            siz = ColW(col);
            Rect r = RectS(x, y, siz, h);
            Rect clip = r;
            clip.left = max(clip.left, fixw);
            clip.top = max(clip.top, fixh);

            // Vertical cell line.
            if (options.contains(goVertLines) || (col == ccnt - 1 && options.contains(goHorzLines) && !options.contains(goLinesFillHorz)))
            {
                c->SetPen(gridcolor);
                --r.right;
                c->Line(r.right, r.top, r.right, r.bottom - 1);
                c->SelectStockPen(spBlack);
            }
            // Horizontal cell line.
            if (options.contains(goHorzLines) || (row == rcnt - 1 && options.contains(goVertLines) && !options.contains(goLinesFillVert)))
            {
                c->SetPen(gridcolor);
                --r.bottom;
                c->Line(r.left, r.bottom, r.right - 1, r.bottom);
                c->SelectStockPen(spBlack);
            }

            bool sel = IsSel(col, row);
            bool hov = SelKind() != gskNoSelect && options.contains(goCellTracking) && ((SelKind() == gskCellSelect && mcell == Point(col, row)) || (SelKind() == gskRowSelect && mcell.x >= fccnt && mcell.y == row) || (SelKind() == gskColSelect && mcell.y >= frcnt && mcell.x == col));

            // Update drawing colors for selected cells.
            if (!themes->VistaListViewThemed())
            {
                if ((options.contains(goAlwaysShowSel) || Focused()) && (sel || hov))
                {
                    if (sel)
                        c->SelectStockBrush(sbHighlight);
                    else
                        c->SetBrush(Color(sbHighlight).Mix(GetColor()));
                    c->GetFont().SetColor(clHighlightText);
                }
            }

            // Draw the cell.
            DrawCell(c, col, row, r, clip, sel, hov);

            // Restore drawing colors after the cell was drawn selected.
            if (!themes->VistaListViewThemed())
            {
                if ((options.contains(goAlwaysShowSel) || Focused()) && (sel || hov))
                {
                    c->SetBrush(GetColor());
                    c->GetFont().SetColor(GetFont().GetColor());
                }
            }
        }

        // Draw anything after the last column in the current row.
        if (x < updaterect.right)
        {
            int w = options.contains(goVertLines) && options.contains(goLinesFillHorz) ? ColW(-1) : 0;
            c->SetPen(gridcolor);

            c->SetBrush(GetColor());

            if (w > 0)
            {
                while (x < updaterect.right)
                {
                    if (w > 1)
                        c->FillRect(Rect(x, y, x + w - 1, y + h - (options.contains(goHorzLines) ? 1 : 0)));
                    c->Line(x + w - 1, y, x + w - 1, y + h - 1 - (options.contains(goHorzLines) ? 1 : 0));
                    if (options.contains(goHorzLines))
                        c->Line(x, y + h - 1, x + w - 1, y + h - 1);
                    x += w;
                }
            }
            else
            {
                if (options.contains(goHorzLines) && options.contains(goLinesFillHorz))
                {
                    c->Line(x, y + h - 1, updaterect.right - 1, y + h - 1);
                    c->FillRect(x, y, updaterect.right, y + h - 1);
                }
                else
                    c->FillRect(x, y, updaterect.right, y + h - (row == rcnt - 1 && options.contains(goVertLines) && options.contains(goLinesFillHorz) && !options.contains(goLinesFillVert) ? 1 : 0));
            }
            c->SelectStockPen(spBlack);
        }

        y += h;
    }

    if (hasclip)
        c->SetClip(clip);
    else
        c->ResetClip();
    if (y >= updaterect.bottom) // Return if the update rectangle do not intersect with the area below the last row.
        return;

    // Paint the area below the last row.
    c->SetBrush(GetColor());
    c->SetPen(gridcolor);

    if ((!options.contains(goVertLines) && !options.contains(goHorzLines)) || !options.contains(goLinesFillVert))
        c->FillRect(updaterect.left, y, updaterect.right, updaterect.bottom);
    else if (options.contains(goHorzLines))
    {
        int right = -1;
        if (firstcol < ccnt && !options.contains(goVertLines) && !options.contains(goLinesFillHorz))
        {
            ColLeft(cr, ccnt - 1, right);
            right += ColW(ccnt - 1);
        }

        int h = RowH(-1);
        //int x = left;
        while (y < updaterect.bottom)
        {
            if (!options.contains(goVertLines))
            {
                if (right >= 0)
                {
                    c->FillRect(left, y, right - 1, y + h - 2);
                    c->FillRect(right, y, updaterect.right, y + h - 2);
                    c->Line(right - 1, y, right - 1, y + h - 1);
                    c->Line(left, y + h - 1, right - 1, y + h - 1);
                }
                else
                {
                    c->FillRect(left, y, updaterect.right, y + h - 2);
                    c->Line(left, y + h - 1, updaterect.right, y + h - 1);
                }
            }
            else
            {
                int x = left;
                for (int col = firstcol; col < ccnt && x < updaterect.right; ++col)
                {
                    int w = ColW(col);
                    c->FillRect(x, y, x + w - 1, y + h - 1);
                    c->Line(x + w - 1, y, x + w - 1, y + h - 2);
                    c->Line(x, y + h - 1, x + w - 1, y + h - 1);
                    x += w;
                }
                if (x < updaterect.right)
                {
                    int w = options.contains(goLinesFillHorz) ? ColW(-1) : -1;
                    if (w <= 0)
                    {
                        c->FillRect(x, y, updaterect.right, y + h - (options.contains(goLinesFillHorz) ? 1 : 0));
                        if (options.contains(goLinesFillHorz))
                            c->Line(x, y + h - 1, updaterect.right, y + h - 1);
                    }
                    else
                    {
                        while (x < updaterect.right)
                        {
                            c->FillRect(x, y, x + w - 1, y + h - 1);
                            c->Line(x + w - 1, y, x + w - 1, y + h - 2);
                            c->Line(x, y + h - 1, x + w - 1, y + h - 1);
                            x += w;
                        }
                    }
                }
            }
            y += h;
        }
    }
    else
    {
        int x = left;
        for (int col = firstcol; col < ccnt && x < updaterect.right; ++col)
        {
            int w = ColW(col);
            c->FillRect(x, y, x + w - 1, updaterect.bottom);
            c->Line(x + w - 1, y, x + w - 1, updaterect.bottom);
            x += w;
        }
        if (x < updaterect.right)
            c->FillRect(x, y, updaterect.right, updaterect.bottom);
    }

    c->SelectStockPen(spBlack);

    if (hasclip)
        c->SetClip(clip);
    else
        c->ResetClip();
}

#ifdef DESIGNING
bool GridBase::NeedDesignerHittest(int x, int y, LRESULT hittest)
{
    if (maction != gmaNone || hittest == HTHSCROLL || hittest == HTVSCROLL)
        return true;

    Rect cr = ClientRect();
    Point pt = WindowToClient(x, y);
    if (!cr.Contains(pt))
        return false;

    // Find whether mouse is over a column edge.
    if (!FixRowCnt() || pt.y < FixRowH())
        UpdateColEdge(ColumnEdge(pt.x));
    else
        UpdateColEdge(-1);

    // Row edge sizing is secondary to column edge sizing. Only find row edge if there is no column edge under the mouse.
    if (mcoledge < 0)
    {
        // Find whether mouse is over a row edge.
        if (!FixColCnt() || pt.x < FixColW())
            UpdateRowEdge(RowEdge(pt.y));
        else
            UpdateRowEdge(-1);
    }

    mcell = CellAt(pt.x, pt.y);

    return mcoledge >= 0 || mrowedge >= 0;
}
#endif

void GridBase::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
{
    if (maction == gmaNone)
    {
        int fccnt = FixColCnt();
        int frcnt = FixRowCnt();
        if (!vkeys.contains(vksLeft))
        {
            // Find whether mouse is over a column edge.
            if (!FixRowCnt() || y < FixRowH())
                UpdateColEdge(ColumnEdge(x));
            else
                UpdateColEdge(-1);

            // Row edge sizing is secondary to column edge sizing. Only find row edge if there is no column edge under the mouse.
            if (mcoledge < 0)
            {
                // Find whether mouse is over a row edge.
                if (!FixColCnt() || x < FixColW())
                    UpdateRowEdge(RowEdge(y));
                else
                    UpdateRowEdge(-1);
            }

            Point mold = mcell;
            mcell = CellAt(x, y);
            if (mold != mcell && SelKind() != gskNoSelect && (options.contains(goFixedTracking) || options.contains(goCellTracking)))
            {
                if (mold.x >= 0 && mold.y >= 0 && ((options.contains(goCellTracking) && mold.x >= fccnt && mold.y >= frcnt) || (options.contains(goFixedTracking) && (mold.x < fccnt || mold.y < frcnt))))
                {
                    if (mold.x >= fccnt && mold.y >= frcnt && (SelKind() == gskColSelect || SelKind() == gskRowSelect))
                    {
                        if (SelKind() == gskColSelect)
                            InvalidateColCells(mold.x, FixRowCnt());
                        else
                            InvalidateRowCells(mold.y, FixColCnt());
                    }
                    else
                        InvalidateCell(mold);
                }
                if (mcell.x >= 0 && mcell.y >= 0 && ((options.contains(goCellTracking) && mcell.x >= fccnt && mcell.y >= frcnt) || (options.contains(goFixedTracking) && (mcell.x < fccnt || mcell.y < frcnt))))
                {
                    if (mcell.x >= fccnt && mcell.y >= frcnt && (SelKind() == gskColSelect || SelKind() == gskRowSelect))
                    {
                        if (SelKind() == gskColSelect)
                            InvalidateColCells(mcell.x, FixRowCnt());
                        else
                            InvalidateRowCells(mcell.y, FixColCnt());
                    }
                    else
                        InvalidateCell(mcell);
                }
            }
        }
        else if (mpos.x >= 0 && mpos.y >= 0 && (mcell.x < fccnt || mcell.y < frcnt))
        {
#ifdef DESIGNING
            if (Designing())
            {
                base::MouseMove(x, y, vkeys);
                return;
            }
#endif
            Rect cr;
            int xd = abs(GetSystemMetrics(SM_CXDRAG));
            int yd = abs(GetSystemMetrics(SM_CYDRAG));
            if ((((x < mpos.x - xd || x >= mpos.x + xd) && mcell.y < frcnt) || ((y < mpos.y - yd || y >= mpos.y + yd) && mcell.x >= fccnt)) && CanColMove(mcell.x))
                maction = gmaColMove;
            else if ((((y < mpos.y - yd || y >= mpos.y + yd) && mcell.x < fccnt) || ((x < mpos.x - xd || x >= mpos.x + xd) && mcell.y >= frcnt)) && CanRowMove(mcell.y))
                maction = gmaRowMove;
            if (maction != gmaNone)
            {
                cr = ClientRect();
                int l, t;
                ColLeft(cr, mcell.x, l);
                RowTop(cr, mcell.y, t);
                mpos = mpos - Point(l, t);
                if (options.contains(goFixedTracking) && mcell.x >= 0 && mcell.y >= 0)
                    InvalidateCell(mcell);
            }
        }
    }
    else if (maction == gmaColResize)
    {
        int w = x + HPos();
        for (int ix = 0; ix < mcoledge; ++ix)
            w -= ColW(ix);
        int colw = ColW(mcoledge);
        //w = max(0, w);
        if (w != colw)
        {
            UpdateColWidth(mcoledge, w - colw);
            UpdateColEdge(mcoledge);
        }
    }
    else if (maction == gmaRowResize)
    {
        int h = y + VPos();
        for (int ix = 0; ix < mrowedge; ++ix)
            h -= RowH(ix);
        int rowh = RowH(mrowedge);
        //h = max(0, h);
        if (h != rowh)
        {
            UpdateRowHeight(mrowedge, h - rowh);
            UpdateRowEdge(mrowedge);
        }
    }

    if (maction == gmaColMove)
    {
        int newedge = -1;
        Rect cr = ClientRect();
        if (y > -128 && y < FixRowH() + 128)
        {
            int cl, cw;
            int col = ColAt(cr, x, NULL, &cl, &cw);
            if (col == -2)
                newedge = ColCnt();
            else if (col != -1)
            {
                newedge = col;
                if (x - cl > cw / 2)
                    ++newedge;
            }
        }
        if (newedge == mcoledge)
        {
            base::MouseMove(x, y, vkeys);
            return;
        }

        if (mcoledge >= 0)
        {
            int cl;
            ColLeft(cr, mcoledge, cl);
            InvalidateRect(RectS(cl - DRAG_MOVE_W, 0, DRAG_MOVE_W * 2, FixRowH()));
        }
        mcoledge = newedge;
        if (mcoledge >= 0)
        {
            int cl;
            ColLeft(cr, mcoledge, cl);
            InvalidateRect(RectS(cl - DRAG_MOVE_W, 0, DRAG_MOVE_W * 2, FixRowH()));
        }
    }
    else if (maction == gmaRowMove)
    {
        int newedge = -1;
        Rect cr = ClientRect();
        if (x > -128 && x < FixColW() + 128)
        {
            int rt, rh;
            int row = RowAt(cr, y, NULL, &rt, &rh);
            if (row == -2)
                newedge = RowCnt();
            else if (row != -1)
            {
                newedge = row;
                if (y - rt > rh / 2)
                    ++newedge;
            }
        }
        if (newedge == mrowedge)
        {
            base::MouseMove(x, y, vkeys);
            return;
        }

        if (mrowedge >= 0)
        {
            int rt;
            RowTop(cr, mrowedge, rt);
            InvalidateRect(RectS(0, rt - DRAG_MOVE_W, FixColW(), DRAG_MOVE_W * 2));
        }
        mrowedge = newedge;
        if (mrowedge >= 0)
        {
            int rt;
            RowTop(cr, mrowedge, rt);
            InvalidateRect(RectS(0, rt - DRAG_MOVE_W, FixColW(), DRAG_MOVE_W * 2));
        }
    }

    base::MouseMove(x, y, vkeys);
}

void GridBase::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    if (button == mbLeft)
        mpos = Point(x, y);

    if (button == mbLeft && (mcoledge >= 0 || mrowedge >= 0))
    {
        maction = mcoledge >= 0 ? gmaColResize : gmaRowResize;
        if (options.contains(goFixedTracking) && mcell.x >= 0 && mcell.y >= 0 && (mcell.x < FixColCnt() || mcell.y < FixRowCnt()))
            InvalidateCell(mcell);

        //base::MouseDown(x, y, button, vkeys);
        //return;
    }

    //if (mcell != Point(-1, -1) && mcell.x >= FixColCnt() && mcell.y >= FixRowCnt())
    //    SetSelected(mcell.x, mcell.y);

    base::MouseDown(x, y, button, vkeys);
}

void GridBase::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    if (maction == gmaNone)
    {
        base::MouseUp(x, y, button, vkeys);
        return;
    }

    if (options.contains(goFixedTracking) && mcell.x >= 0 && mcell.y >= 0 && (mcell.x < FixColCnt() || mcell.y < FixRowCnt()))
        InvalidateCell(mcell);

    if (maction == gmaColMove || maction == gmaRowMove)
    {
        if (maction == gmaColMove && mcoledge >= 0)
        {
            int cl;
            ColLeft(ClientRect(), mcoledge, cl);
            InvalidateRect(RectS(cl - DRAG_MOVE_W, 0, DRAG_MOVE_W * 2, FixRowH()));
        }
        else if (maction == gmaRowMove && mrowedge >= 0)
        {
            int rt;
            RowTop(ClientRect(), mrowedge, rt);
            InvalidateRect(RectS(0, rt - DRAG_MOVE_W, FixColW(), DRAG_MOVE_W * 2));
        }

        maction = gmaNone;

        MouseMove(x, y, vkeys);
        base::MouseUp(x, y, button, vkeys);
        return;
    }

    if ((maction == gmaColResize && mcoledge >= 0) || (maction == gmaRowResize && mrowedge >= 0))
    {
        bool colsiz = maction == gmaColResize;
        bool rowsiz = maction == gmaRowResize;

        maction = gmaNone;
        if (colsiz)
        {
            int w = x + HPos();
            for (int ix = 0; ix < mcoledge; ++ix)
                w -= ColW(ix);
            int colw = ColW(mcoledge);
            UpdateColWidth(mcoledge, w - colw);
            UpdateColEdge(mcoledge);
        }
        else if (rowsiz)
        {
            int h = y + VPos();
            for (int ix = 0; ix < mrowedge; ++ix)
                h -= RowH(ix);
            int rowh = RowH(mrowedge);
            UpdateRowHeight(mrowedge, h - rowh);
            UpdateRowEdge(mrowedge);
        }

        MouseMove(x, y, vkeys);
        base::MouseUp(x, y, button, vkeys);
        return;
    }
}

void GridBase::MouseLeave()
{
    if (SelKind() != gskNoSelect)
    {
        int fccnt = FixColCnt();
        int frcnt = FixRowCnt();
        if (mcell.x >= 0 && mcell.y >= 0 && ((options.contains(goFixedTracking) && (mcell.x < fccnt || mcell.y < frcnt)) || (options.contains(goCellTracking) && mcell.x >= fccnt && mcell.y >= frcnt)))
        {
            if (mcell.x >= fccnt && mcell.y >= frcnt && (SelKind() == gskRowSelect || SelKind() == gskColSelect))
            {
                if (SelKind() == gskRowSelect)
                    InvalidateRowCells(mcell.y, fccnt);
                else
                    InvalidateColCells(mcell.x, frcnt);
            }
            else
                InvalidateCell(mcell);
        }
    }
    mcell = Point(-1, -1);
    base::MouseLeave();
}

void GridBase::KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys)
{
    switch(keycode)
    {
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
        Navigate(keycode, vkeys);
        break;
    }

    base::KeyPush(keycode, key, vkeys);
}

void GridBase::Navigate(WORD &keycode, VirtualKeyStateSet vkeys)
{
    switch(keycode)
    {
    case VK_LEFT:
        SetHPos(HPos() - HLineStep());
        break;
    case VK_RIGHT:
        SetHPos(HPos() + HLineStep());
        break;
    case VK_UP:
        SetVPos(VPos() - VLineStep());
        break;
    case VK_DOWN:
        SetVPos(VPos() + VLineStep());
        break;
    case VK_HOME:
        if (vkeys.contains(vksCtrl))
            SetVPos(0);
        else
            SetHPos(0);
        break;
    case VK_END:
        if (vkeys.contains(vksCtrl))
            SetVPos(VRange());
        else
            SetHPos(HRange());
        break;
    case VK_PRIOR:
        SetVPos(VPos() - VPageStep()); //max(max(1, VLineStep() / 2), ClientHeight() - FixRowH() - VLineStep()));
        break;
    case VK_NEXT:
        SetVPos(VPos() + VPageStep()); //max(max(1, VLineStep() / 2), ClientHeight() - FixRowH() - VLineStep()));
        break;
    };
}

void GridBase::GainFocus(HWND otherwindow)
{
    InvalidateSelected();
    base::GainFocus(otherwindow);
}

void GridBase::LoseFocus(HWND otherwindow)
{
    InvalidateSelected();
    base::LoseFocus(otherwindow);
}

void GridBase::CaptureChanged()
{
    base::CaptureChanged();

    CancelAction();
}

void GridBase::CancelAction()
{
    if (maction == gmaColMove && mcoledge >= 0)
    {
        int cl;
        ColLeft(ClientRect(), mcoledge, cl);
        InvalidateRect(RectS(cl - DRAG_MOVE_W, 0, DRAG_MOVE_W * 2, FixRowH()));
    }
    else if (maction == gmaRowMove && mrowedge >= 0)
    {
        int rt;
        RowTop(ClientRect(), mrowedge, rt);
        InvalidateRect(RectS(0, rt - DRAG_MOVE_W, FixColW(), DRAG_MOVE_W * 2));
    }

    maction = gmaNone;
    mpos = Point(-1, -1);
}

void GridBase::NeedsDialogCode(WORD key, DialogCodeSet &dialogcode)
{
    if (key == VK_ESCAPE && maction != gmaNone)
    {
        CancelAction();
        dialogcode << dcWantAllKeys;
    }
    base::NeedsDialogCode(key, dialogcode);
}

void GridBase::UpdateColWidth(int col, int widthchange)
{
    InvalidateColumns(col);
    if (widthchange < 0)
        InvalidateSideArea(-widthchange);
    ScrollResize();
}

void GridBase::UpdateRowHeight(int row, int heightchange)
{
    InvalidateRows(row);
    if (heightchange < 0)
        InvalidateBottomArea(-heightchange);
    ScrollResize();
}

auto GridBase::MouseAction() -> GridMouseActions
{
    return maction;
}

int GridBase::MouseColEdge()
{
    return mcoledge;
}

int GridBase::MouseRowEdge()
{
    return mrowedge;
}

Point GridBase::MouseCell()
{
    return mcell;
}

Point GridBase::MousePos()
{
    return mpos;
}

void GridBase::UpdateColEdge(int newedge)
{
    if (newedge >= 0 && !CanColSiz(newedge))
        newedge = -1;

    mcoledge = newedge;
    if (newedge != -1)
        mrowedge = -1;
    if (mcoledge >= 0)
        screencursor->Set(cSizeColumn);
    else
        screencursor->Set(Cursor());
}

void GridBase::UpdateRowEdge(int newedge)
{
    if (newedge >= 0 && !CanRowSiz(newedge))
        newedge = -1;

    mrowedge = newedge;
    if (newedge != -1)
        mcoledge = -1;
    if (mrowedge >= 0)
        screencursor->Set(cSplitV);
    else
        screencursor->Set(Cursor());
}

void GridBase::DrawHead(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool hovered)
{
    themes->DrawHeaderItemClip(c, r, hovered ? thisHot : thisNormal, clip);
}

void GridBase::DrawCell(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool selected, bool hovered)
{
    c->FillRect(r);

    selected = selected && (options.contains(goAlwaysShowSel) || Focused());
    if (SelKind() != gskNoSelect && themes->VistaListViewThemed() && (selected || hovered)) // Draw focus "rectangle".
    {
        Rect r2 = r;
        if (options.contains(goVertLines) && SelKind() == gskRowSelect)
            ++r2.right;
        if (options.contains(goHorzLines) && SelKind() == gskColSelect)
            ++r2.bottom;
        Rect clip2 = r2;
        if (SelKind() == gskRowSelect)
        {
            if (col != FixColCnt())
                r2.left -= 4;
            if (col != ColCnt() - 1)
                r2.right += 4;
            else if (options.contains(goVertLines) || options.contains(goHorzLines))
                --r2.right;
        }
        else if (SelKind() == gskColSelect)
        {
            if (row != FixRowCnt())
                r2.top -= 4;
            if (row != RowCnt() - 1)
                r2.bottom += 4;
            else if (options.contains(goVertLines) || options.contains(goHorzLines))
                --r2.bottom;
        }
        clip2 = clip2.Intersect(clip);
        themes->DrawVistaListViewItemClipped(c, r2, !hovered && !Focused() ? tlviSelectedNotFocused : hovered ? (selected ? tlviHotSelected : tlviHot) : selected ? tlviSelected : tlviNormal, clip2);
    }
}

void GridBase::GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide)
{
    int fw = FixColW();
    int ccnt = ColCnt();
    int fccnt = FixColCnt();
    if (smoothx)
    {
        int wsiz = HorzSiz() - fw;
        if (uw <= fw)
            uw = ccnt > fccnt ? 1 : 0;
        else
            uw = min(uw - fw, wsiz);
        hw = max(0, wsiz - uw);
    }
    else
    {
        int w = uw - fw;
        int col = HPos() + fccnt;
        int ix = col;
        for ( ; ix < ccnt && w >= 0; ++ix)
            w -= ColW(ix);
        uw = max(ix - col - (w < 0 ? 1 : 0), ccnt > fccnt ? 1 : 0);
        hw = ccnt - fccnt - uw;
    }

    int fh = FixRowH();
    int rcnt = RowCnt();
    int frcnt = FixRowCnt();
    if (smoothy)
    {
        int hsiz = VertSiz() - fh;
        if (uh <= fh)
            uh = rcnt > frcnt ? 1 : 0;
        else
            uh = min(uh - fh, hsiz);
        hh = max(0, hsiz - uh);
    }
    else
    {
        int h = uh - fh;
        int row = VPos() + frcnt;
        int ix = row;
        for ( ; ix < rcnt && h >= 0; ++ix)
            h -= RowH(ix);
        for (; row > frcnt && h >= 0; --row)
            h -= RowH(row);
        uh = max(ix - row - (h < 0 ? 1 : 0), rcnt > frcnt ? 1 : 0);
        hh = rcnt - frcnt - uh;
    }
}

bool GridBase::SmoothX()
{
    return smoothx;
}

void GridBase::SetSmoothX(bool newsmoothx)
{
    if (smoothx == newsmoothx)
        return;
    int vpos = VPos();
    // Find the new vertical scroll position.
    if (smoothx)
    {
        int cnt = ColCnt();
        for (int ix = 0; ix < cnt; ++ix)
        {
            vpos -= ColW(ix);
            if (vpos <= 0)
            {
                vpos = ix;
                break;
            }
        }
        if (vpos < 0)
            vpos = 0;
    }
    else
    {
        int cnt = vpos;
        vpos = 0;
        for (int ix = 0; ix < cnt; ++ix)
            vpos += ColW(ix);
    }
    smoothx = newsmoothx;
    ScrollResize();
    SetVPos(vpos);
}

bool GridBase::SmoothY()
{
    return smoothy;
}

void GridBase::SetSmoothY(bool newsmoothy)
{
    if (smoothy == newsmoothy)
        return;
    int hpos = HPos();
    // Find the new vertical scroll position.
    if (smoothy)
    {
        int cnt = RowCnt();
        for (int ix = 0; ix < cnt; ++ix)
        {
            hpos -= RowH(ix);
            if (hpos <= 0)
            {
                hpos = ix;
                break;
            }
        }
        if (hpos < 0)
            hpos = 0;
    }
    else
    {
        int cnt = hpos;
        hpos = 0;
        for (int ix = 0; ix < cnt; ++ix)
            hpos += RowH(ix);
    }
    smoothy = newsmoothy;
    ScrollResize();
    SetHPos(hpos);
}

void GridBase::Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code)
{
    Invalidate();
    base::Scrolled(kind, oldpos, pos, code);
}

bool GridBase::ColLeft(const Rect &clientrect, int col, int &colleft, bool *first)
{
    if (first)
        *first = false;

    int fcnt = FixColCnt();
    if (col < fcnt)
    {
        int left = 0;
        colleft = 0;
        while (left < col)
            colleft += ColW(left++);
        return true;
    }

    if (!smoothx && col - fcnt < HPos())
        return false;

    int firstcol;
    int siz;
    FirstCol(firstcol, &colleft, &siz);

    if (firstcol < 0 || col < firstcol || colleft >= clientrect.right)
        return false;

    if (first)
        *first = col == firstcol;

    while (col != firstcol && colleft < clientrect.right)
    {
        colleft += siz;
        if (++firstcol != col)
            siz = ColW(firstcol);
    }
    return colleft < clientrect.right;
}

bool GridBase::RowTop(const Rect &clientrect, int row, int &rowtop, bool *first)
{
    if (first)
        *first = false;

    int fcnt = FixRowCnt();
    if (row < fcnt)
    {
        int top = 0;
        rowtop = 0;
        while (top < row)
            rowtop += RowH(top++);
        return true;
    }

    if (!smoothy && row - fcnt < VPos())
        return false;

    int firstrow;
    int siz;
    FirstRow(firstrow, &rowtop, &siz);

    if (firstrow < 0 || row < firstrow || rowtop >= clientrect.bottom)
        return false;

    if (first)
        *first = row == firstrow;

    while (row != firstrow && rowtop < clientrect.bottom)
    {
        rowtop += siz;
        if (++firstrow != row)
            siz = RowH(firstrow);
    }
    return rowtop < clientrect.bottom;
}

Rect GridBase::ColRect(int col)
{
    if (col < 0 || col >= ColCnt())
        return Rect();

    Rect cr(ClientRect());
    int x;
    if (!ColLeft(cr, col, x))
        return Rect();

    int w = ColW(col);
    int fcnt = FixColCnt();
    if (col >= fcnt)
    {
        int fw = FixColW();
        if (x < fw)
        {
            w -= fw - x;
            x = fw;
        }
    }

    Rect r(x, 0, min(x + w, cr.right), cr.bottom);
    if (options.contains(goLinesFillVert))
        return r;

    int row;
    int rowy;
    int siz;
    int cnt = RowCnt();
    FirstRow(row, &rowy, &siz);
    if (row < 0)
        r.bottom = FixRowH();
    else
    {
        for (rowy += siz, ++row; row < cnt && rowy < cr.bottom; ++row, rowy += siz)
            siz = RowH(row);
        r.bottom = min(cr.bottom, rowy);
    }

    //if (r.Empty())
    //    return Rect();

    return r;
}

Rect GridBase::RowRect(int row)
{
    if (row < 0 || row >= RowCnt())
        return Rect();

    Rect cr(ClientRect());
    int y;
    if (!RowTop(cr, row, y))
        return Rect();

    int h = RowH(row);
    int fcnt = FixRowCnt();
    if (row >= fcnt)
    {
        int fw = FixRowH();
        if (y < fw)
        {
            h -= fw - y;
            y = fw;
        }
    }

    Rect r(0, y, cr.right, min(y + h, cr.bottom));
    if (options.contains(goLinesFillHorz))
        return r;

    int col;
    int colx;
    int siz;
    int cnt = ColCnt();
    FirstCol(col, &colx, &siz);
    if (col < 0)
        r.right = FixColW();
    else
    {
        for (colx += siz, ++col; col < cnt && colx < cr.right; ++col, colx += siz)
            siz = ColW(col);
        r.right = min(cr.right, colx);
    }

    //if (r.Empty())
    //    return Rect();

    return r;
}

Rect GridBase::CellRect(int col, int row)
{
    if (row < 0 || row >= RowCnt() || col < 0 || col >= ColCnt())
        return Rect();

    Rect cr = ClientRect();

    int left;
    int top;
    if (!ColLeft(cr, col, left))
        return Rect();
    if (!RowTop(cr, row, top))
        return Rect();

    int w = ColW(col);
    int h = RowH(row);

    return Rect(left, top, left + w, top + h);
}

int GridBase::CEdge(int x, int col, int colwidth, bool first)
{
    int drag = 9 * Scaling;
    int fcnt = FixColCnt();
    if (x < drag)
    {
        if (first)
            return fcnt - 1;
        return (col < 0 ? ColCnt() : col) - 1;
    }
    if (x > colwidth || col < 0 || col >= ColCnt())
        return -1;
    if (colwidth - x <= drag)
        return col;
    return -1;
}

int GridBase::REdge(int y, int row, int rowheight, bool first)
{
    int drag = 3 * Scaling;
    int fcnt = FixRowCnt();
    if (y < drag)
    {
        if (first)
            return fcnt - 1;
        return (row < 0 ? RowCnt() : row) - 1;
    }
    if (y > rowheight || row < 0 || row >= RowCnt())
        return -1;
    if (rowheight - y <= drag)
        return row;
    return -1;
}

int GridBase::ColumnEdge(int x)
{
    int cnt = ColCnt();
    if (!cnt || x < 0)
        return -1;

    Rect cr = ClientRect();

    // Find edge near x in fixed cols first, if x is inside the fixed part.
    int fcnt = FixColCnt();
    int left = 0;
    int siz = 0;
    int col = 0;
    for ( ; col != fcnt; ++col, left += siz)
    {
        siz = ColW(col);
        if (left + siz > x) // X was inside col at (x - left) position.
            return CEdge(x - left, col, siz, false);
    }

    int fw = siz;
    FirstCol(col, &left, &siz);
    if (col < 0)
        return CEdge(x - fw, -1, -1, true);

    if (fw > left)
    {
        siz -= fw - left;
        left = fw;
    }

    if (left + siz > x)
        return CEdge(x - left, col, min(siz, cr.right - left), fcnt > 0);

    for (++col, left += siz; col < cnt; ++col, left += siz)
    {
        siz = ColW(col);
        if (left + siz > x)
            return CEdge(x - left, col, min(siz, cr.right - left), false);
    }

    return CEdge(x - left, -1, -1, false);
}

int GridBase::RowEdge(int y)
{
    int cnt = RowCnt();
    if (!cnt || y < 0)
        return -1;

    //Rect cr = ClientRect();

    // Find edge near y in fixed rows first, if y is inside the fixed part.
    int fcnt = FixRowCnt();
    int top = 0;
    int siz = 0;
    int row = 0;
    for ( ; row != fcnt; ++row, top += siz)
    {
        siz = RowH(row);
        if (top + siz > y) // X was inside col at (x - left) position.
            return REdge(y - top, row, siz, false);
    }

    int fh = siz;
    FirstRow(row, &top, &siz);
    if (row < 0)
        return REdge(y - fh, -1, -1, true);

    if (fh > top)
    {
        siz -= fh - top;
        top = fh;
    }

    if (top + siz > y)
        return REdge(y - top, row, siz, fcnt > 0);

    for (++row, top += siz; row < cnt; ++row, top += siz)
    {
        siz = RowH(row);
        if (top + siz > y)
            return REdge(y - top, row, siz, false);
    }

    return REdge(y - top, -1, -1, false);
}

int GridBase::ColAt(const Rect &clientrect, int x, bool *first, int *colleft, int *colwidth)
{
    if (first)
        *first = false;
    if (x < 0 || x >= clientrect.right)
    {
        if (colleft)
            *colleft = -1;
        if (colwidth)
            *colwidth = -1;
        return -1;
    }

    int cnt = ColCnt();
    int fcnt = FixColCnt();

    int col = 0;
    int left = 0;
    int siz = 0;
    for (; col != fcnt; ++col, left += siz)
    {
        siz = ColW(col);
        if (left + siz > x)
        {
            if (colleft)
                *colleft = left;
            if (colwidth)
                *colwidth = siz;
            return col;
        }
    }

    int cleft;
    siz = 0;
    FirstCol(col, &cleft, &siz);
    if (x < cleft + siz)
    {
        if (first)
            *first = true;
        if (colleft)
            *colleft = cleft; //max(left, cleft);
        if (colwidth)
            *colwidth = siz; //- (left > cleft ? left - cleft : 0);
        return col;
    }
    for (++col, cleft += siz ; col < cnt; ++col, cleft += siz)
    {
        siz = ColW(col);
        if (x < cleft + siz)
        {
            if (colleft)
                *colleft = cleft;
            if (colwidth)
                *colwidth = siz;
            if (first)
                *first = false;
            return col;
        }
    }

    if (colleft)
        *colleft = max(left, cleft);
    if (colwidth)
        *colwidth = -1;
    return -2;
}

int GridBase::RowAt(const Rect &clientrect, int y, bool *first, int *rowtop, int *rowheight)
{
    if (first)
        *first = false;
    if (y < 0 || y >= clientrect.bottom)
    {
        if (rowtop)
            *rowtop = -1;
        if (rowheight)
            *rowheight = -1;
        return -1;
    }

    int cnt = RowCnt();
    int fcnt = FixRowCnt();

    int row = 0;
    int top = 0;
    int siz = 0;
    for (; row != fcnt; ++row, top += siz)
    {
        siz = RowH(row);
        if (top + siz > y)
        {
            if (rowtop)
                *rowtop = top;
            if (rowheight)
                *rowheight = siz;
            return row;
        }
    }

    int rtop;
    siz = 0;
    FirstRow(row, &rtop, &siz);
    if (y < rtop + siz)
    {
        if (first)
            *first = true;
        if (rowtop)
            *rowtop = max(top, rtop);
        if (rowheight)
            *rowheight = siz - (top > rtop ? top - rtop : 0);
        return row;
    }
    for (++row, rtop += siz ; row < cnt; ++row, rtop += siz)
    {
        siz = RowH(row);
        if (y < rtop + siz)
        {
            if (rowtop)
                *rowtop = rtop;
            if (rowheight)
                *rowheight = siz;
            if (first)
                *first = false;
            return row;
        }
    }

    if (rowtop)
        *rowtop = max(top, rtop);
    if (rowheight)
        *rowheight = -1;
    return -2;

}

Point GridBase::CellAt(int x, int y)
{
    Rect cr = ClientRect();

    int col = ColAt(cr, x);
    if (col < 0)
        return Point(-1, -1);
    int row = RowAt(cr, y);
    if (row < 0)
        return Point(-1, -1);

    return Point(col, row);
}

GridPosition GridBase::PositionAt(int x, int y)
{
    GridPosition pos;
    pos.type = gptNowhere;
    Rect cr = ClientRect();
    if (x < 0 || y < 0 || x >= cr.right || y >= cr.bottom)
    {
        pos.col = -1;
        pos.row = -1;
        return pos;
    }

    int colx, colw;
    bool cfirst;
    pos.col = ColAt(cr, x, &cfirst, &colx, &colw);

    if (pos.col == -1)
    {
        pos.row = -1;
        return pos;
    }

    int rowy, rowh;
    bool rfirst;
    pos.row = RowAt(cr, y, &rfirst, &rowy, &rowh);

    if (pos.row == -1)
    {
        pos.col = -1;
        return pos;
    }

    // Column edge.
    int fccnt = FixColCnt();
    int frcnt = FixRowCnt();
    int edge = pos.col != -1 ? CEdge(x - colx, pos.col, min(colw, cr.right - colx), cfirst) : -1;
    if (edge >= 0 && (frcnt == 0 || (pos.row >= 0 && pos.row < frcnt)) && CanColSiz(edge))
    {
        pos.col = edge;
        pos.type = gptVertEdge;
    }
    else if (pos.col >= 0)
        pos.type = pos.col < fccnt ? gptFixed : gptCell;
    else // pos.col == -2, to the right
    {
        pos.col = -1;
        pos.type = gptAfterCells;
    }

    // Row edge.

    edge = (pos.type != gptVertEdge && pos.row != -1) ? REdge(y - rowy, pos.row, min(rowh, cr.bottom - rowy), rfirst) : -1;
    if (edge >= 0 && (fccnt == 0 || (pos.col >= 0 && pos.col < fccnt)) && CanRowSiz(edge))
    {
        pos.row = edge;
        pos.type = gptHorzEdge;
    }
    else if (pos.row >= 0)
    {
        if (pos.type != gptVertEdge)
            pos.type = pos.row < frcnt && (pos.type != gptAfterCells || options.contains(goLinesFillHorz)) ? gptFixed : pos.type != gptAfterCells ? gptCell : gptAfterCells;
    }
    else // pos.row == -2, below
    {
        pos.row = -1;
        if (pos.type != gptVertEdge && (pos.type != gptFixed || !options.contains(goLinesFillVert)))
            pos.type = gptAfterCells;
    }

    return pos;
}

void GridBase::InvalidateColumns(int first, int last)
{
    if (!HandleCreated())
        return;

    int cnt = ColCnt();
    if (!cnt || first >= cnt)
        return;

    int fcnt = FixColCnt();
    if (last < 0)
        last = cnt - 1;
    first = max(first, 0);
    last = max(first, min(last, cnt - 1));
    Rect r;
    if (first < fcnt)
    {
        r = ColRect(first++);
        while (first < fcnt && first < last)
            r.right += ColW(first++);
        InvalidateRect(r);
    }

    int col;
    FirstCol(col);
    if (last < col)
        return;
    first = max(first, col);
    r = ColRect(first);
    Rect cr = ClientRect();
    while (first < last && r.right < cr.right)
        r.right += ColW(++first);
    r.right = min(r.right, cr.right);

    InvalidateRect(r);
}

void GridBase::InvalidateRows(int first, int last)
{
    if (!HandleCreated())
        return;

    int cnt = RowCnt();
    if (!cnt || first >= cnt)
        return;

    int fcnt = FixRowCnt();
    if (last < 0)
        last = cnt - 1;
    first = max(first, 0);
    last = max(first, last);
    Rect r;
    if (first < fcnt)
    {
        r = RowRect(first++);
        while (first < fcnt && first < last)
            r.bottom += RowH(first++);
        InvalidateRect(r);
    }

    int row;
    FirstRow(row);
    if (last < row)
        return;
    first = max(first, row);
    r = RowRect(first);
    Rect cr = ClientRect();
    while (first < last && r.bottom < cr.bottom)
        r.bottom += RowH(++first);
    r.bottom = min(r.bottom, cr.bottom);

    InvalidateRect(r);
}

void GridBase::InvalidateRowCells(int row, int first, int last)
{
    if (!HandleCreated() || row < 0)
        return;

    int ccnt = ColCnt();
    if (first >= ccnt)
        return;
    int rcnt = RowCnt();
    if (row >= rcnt)
        return;

    int fccnt = FixColCnt();

    if (last < 0)
        last = ccnt - 1;
    first = max(first, 0);
    last = max(first, last);

    int frcnt = FixRowCnt();
    Rect cr = ClientRect();
    int top;

    RowTop(cr, row, top);
    int h = RowH(row);
    if (row >= frcnt && smoothy)
    {
        int fh = FixRowH();
        if (fh > top)
        {
            h -= fh - top;
            top = fh;
        }
    }

    Rect r(0, top, 0, top + h);

    int siz;
    if (first < fccnt)
    {
        for ( ; first != fccnt && first < last && r.right < cr.right; ++first, r.right += siz)
            siz = ColW(first);
        InvalidateRect(r);
        if (last < fccnt)
            return;
        r.left = r.right;
    }
    else
        r.left = FixColW();

    int col;
    int left;
    FirstCol(col, &left, &siz);
    if (col < 0)
        return;
    first = col;
    r.right = left + siz;
    for (++col; col <= last && r.right < cr.right; ++col, r.right += siz)
        siz = ColW(col);

    InvalidateRect(r);
}

void GridBase::InvalidateColCells(int col, int first, int last)
{
    if (!HandleCreated() || col < 0)
        return;

    int rcnt = RowCnt();
    if (first >= rcnt)
        return;
    int ccnt = ColCnt();
    if (col >= ccnt)
        return;

    int frcnt = FixRowCnt();

    if (last < 0)
        last = rcnt - 1;
    first = max(first, 0);
    last = max(first, last);

    int fccnt = FixColCnt();
    Rect cr = ClientRect();
    int left;

    ColLeft(cr, col, left);
    int w = ColW(col);
    if (col >= fccnt && smoothx)
    {
        int fw = FixColW();
        if (fw > left)
        {
            w -= fw - left;
            left = fw;
        }
    }

    Rect r(left, 0, left + w, 0);

    int siz;
    if (first < frcnt)
    {
        for ( ; first != frcnt && first < last && r.bottom < cr.bottom; ++first, r.bottom += siz)
            siz = RowH(first);
        InvalidateRect(r);
        if (last < frcnt)
            return;
        r.top = r.bottom;
    }
    else
        r.top = FixRowH();

    int row;
    int top;
    FirstRow(row, &top, &siz);
    if (row < 0)
        return;
    first = row;
    r.bottom = top + siz;
    for (++row; row <= last && r.bottom < cr.bottom; ++row, r.bottom += siz)
        siz = RowH(row);

    InvalidateRect(r);
}

void GridBase::InvalidateCell(int col, int row)
{
    if (!HandleCreated())
        return;

    Rect r = CellRect(col, row);
    if (r.Empty())
        return;
    InvalidateRect(r);
}

void GridBase::InvalidateCell(const Point &p)
{
    InvalidateCell(p.x, p.y);
}

void GridBase::InvalidateColumn(int col)
{
    if (!HandleCreated())
        return;
    Rect r = ColRect(col);
    if (r.Empty())
        return;
    InvalidateRect(r);
}

void GridBase::InvalidateRow(int row)
{
    if (!HandleCreated())
        return;
    Rect r = RowRect(row);
    if (r.Empty())
        return;
    InvalidateRect(r);
}

void GridBase::InvalidateSelected()
{
    if (SelKind() == gskNoSelect)
        return;

    Rect cr = ClientRect();
    if (SelKind() == gskRowSelect)
    {
        int rcnt = RowCnt();
        int row, rowy, rowh;
        FirstRow(row, &rowy, &rowh);
        int l = FixColW();

        while (row >= 0 && row < rcnt && rowy < cr.bottom)
        {
            if (IsSel(-1, row))
                InvalidateRect(Rect(l, rowy, cr.right, rowy + rowh));
            ++row;
            rowy += rowh;
            rowh = RowH(row);
        }
    }
    else if (SelKind() == gskColSelect)
    {
        int ccnt = ColCnt();
        int col, colx, colw;
        FirstCol(col, &colx, &colw);
        int t = FixRowH();

        while (col >= 0 && col < ccnt && colx < cr.right)
        {
            if (IsSel(col, -1))
                InvalidateRect(Rect(colx, t, colx + colw, cr.bottom));
            ++col;
            colx += colw;
            colw = ColW(col);
        }
    }
    else
    {
        int rcnt = RowCnt();
        int row, rowy, rowh;
        FirstRow(row, &rowy, &rowh);

        int ccnt = ColCnt();
        int col, colx, colw;
        FirstCol(col, &colx, &colw);

        // Fill a vector with all column widths.
        std::vector<int> wvec;
        int c = col;
        int cx = colx;
        while (c >= 0 && c < ccnt && cx < cr.right)
        {
            wvec.push_back(colw);
            ++c;
            cx += colw;
            colw = ColW(c);
        }

        while (row >= 0 && row < rcnt && rowy < cr.bottom)
        {
            c = col;
            cx = colx;
            for (auto it = wvec.begin(); it != wvec.end(); ++it)
            {
                if (IsSel(c, row))
                    InvalidateRect(RectS(cx, rowy, *it, rowh));
                ++c;
                cx += *it;
            }
            ++row;
            rowy += rowh;
            rowh = RowH(row);
        }
    }
}

void GridBase::InvalidateSideArea(int width)
{
    if (!HandleCreated() || width <= 0)
        return;

    Rect cr = ClientRect();
    int left;
    int cnt = ColCnt();
    ColLeft(cr, cnt - 1, left);
    if (left < 0)
        return;
    left += ColW(cnt - 1);

    Rect r;
    r = Rect(max(left, cr.left), 0, min(left + width, cr.right), cr.bottom);
    InvalidateRect(r);
}

void GridBase::InvalidateBottomArea(int height)
{
    if (!HandleCreated() || height <= 0)
        return;

    Rect cr = ClientRect();
    int cnt = RowCnt();
    int top;
    RowTop(cr, cnt - 1, top);
    if (top < 0)
        return;
    top += RowH(cnt - 1);

    Rect r(0, max(top, cr.top), cr.right, min(top + height, cr.bottom));
    InvalidateRect(r);
}

void GridBase::InvalidateSide()
{
    if (!HandleCreated())
        return;

    Rect cr = ClientRect();
    int left;
    int cnt = ColCnt();
    ColLeft(cr, cnt - 1, left);
    if (left < 0)
        return;
    left += ColW(cnt - 1);

    Rect r;
    r = Rect(max(left, cr.left), 0, cr.right, cr.bottom);
    if (!r.Empty())
        InvalidateRect(r);
}

void GridBase::InvalidateBottom()
{
    if (!HandleCreated())
        return;

    Rect cr = ClientRect();
    int cnt = RowCnt();
    int top;
    RowTop(cr, cnt - 1, top);
    if (top < 0)
        return;
    top += RowH(cnt - 1);

    Rect r(0, max(top, cr.top), cr.right, cr.bottom);
    if (!r.Empty())
        InvalidateRect(r);
}

bool GridBase::ScrollToCell(int col, int row)
{
    if (!HandleCreated())
        return false;

    int ccnt = ColCnt();
    int rcnt = RowCnt();
    if (col >= ccnt || row >= rcnt)
        throw L"Specified cell out of range.";

    int fccnt = FixColCnt();
    int frcnt = FixRowCnt();
    if (col < fccnt && row < frcnt)
        return false;

    bool changed = false;
    Rect cr = ClientRect();
    if (col >= fccnt && (smoothx || col != HPos()))
    {
        int fw = FixColW();

        int firstcol;
        int left;
        int siz;
        FirstCol(firstcol, &left, &siz);
        if (firstcol == col)
        {
            if (left < fw)
            {
                SetHPos(HPos() - (fw - left));
                changed = true;
            }
        }
        else if (firstcol > col)
        {
            changed = true;

            if (!smoothx)
                SetHPos(col - fccnt);
            else
            {
                int w = fw - left;
                while (firstcol != col)
                    w += ColW(--firstcol);
                SetHPos(HPos() - w);
            }
        }
        else // firstcol < col
        {
            if (!smoothx)
            {
                int w = 0;
                int s = ColW(col);
                while (col > firstcol && col != fccnt && w + s < cr.right)
                {
                    w += s;
                    s = ColW(--col);
                }

                if (w + s > cr.right)
                {
                    changed = true;
                    SetHPos(col + 1);
                }
            }
            else
            {
                int w = left + siz;
                int colw = 0;
                for (++firstcol; firstcol <= col; ++firstcol)
                {
                    w += colw;
                    colw = ColW(firstcol);
                }
                if (w + colw > cr.right)
                {
                    changed = true;
                    SetHPos(HPos() + min(w - fw, w + colw - cr.right));
                }
            }
        }
    }

    if (row >= frcnt && (smoothy || row != VPos()))
    {
        int fh = FixRowH();

        int firstrow;
        int top;
        int siz;
        FirstRow(firstrow, &top, &siz);
        if (firstrow == row)
        {
            if (top < fh)
            {
                SetVPos(VPos() - (fh - top));
                changed = true;
            }
        }
        else if (firstrow > row)
        {
            changed = true;

            if (!smoothy)
                SetVPos(row - frcnt);
            else
            {
                int h = fh - top;
                while (firstrow != row)
                    h += RowH(--firstrow);
                SetVPos(VPos() - h);
            }
        }
        else // firstrow < row
        {
            if (!smoothy)
            {
                int h = 0;
                int s = RowH(row);
                while (row > firstrow && row != frcnt && h + s < cr.bottom)
                {
                    h += s;
                    s = RowH(--row);
                }

                if (h + s > cr.bottom)
                {
                    changed = true;
                    SetVPos(row + 1);
                }
            }
            else
            {
                int h = top + siz;
                int rowh = 0;
                for (++firstrow; firstrow <= row; ++firstrow)
                {
                    h += rowh;
                    rowh = RowH(firstrow);
                }
                if (h + rowh > cr.bottom)
                {
                    changed = true;
                    SetVPos(VPos() + min(h - fh, h + rowh - cr.bottom));
                }
            }
        }
    }

    return changed;
}

GridOptionSet GridBase::Options()
{
    return options;
}

void GridBase::SetOptions(GridOptionSet newoptions)
{
    if (options == newoptions)
        return;
    options = newoptions;
    OptionsChanged();
    if (!HandleCreated())
        return;

    Invalidate();
}

Color GridBase::GridColor()
{
    return gridcolor;
}

void GridBase::SetGridColor(Color newgridcolor)
{
    if (gridcolor == newgridcolor)
        return;
    gridcolor = newgridcolor;
    if (!HandleCreated() || (!options.contains(goVertLines) && !options.contains(goHorzLines)))
        return;
    Invalidate();
}


//---------------------------------------------


#ifdef DESIGNING
void CustomGridElem::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);

    serializer->HideProperty(L"AccessLevel");
    serializer->HideProperty(L"Name");
    serializer->HideProperty(L"Tag");

    serializer->Add(std::wstring(), new BoolDesignProperty<CustomGridElem>(L"Fixed", L"Appearance", &CustomGridElem::GetFixed, NULL));
    serializer->Add(std::wstring(), new BoolDesignProperty<CustomGridElem>(L"Resizable", L"Behavior", &CustomGridElem::GetResizable, &CustomGridElem::SetResizable))->SetDefault(&CustomGridElem::DefaultResizable);
    serializer->Add(std::wstring(), new BoolDesignProperty<CustomGridElem>(L"Visible", L"Appearance", &CustomGridElem::GetVisible, &CustomGridElem::SetVisible))->SetDefault(true);
}

CustomGridElem::CustomGridElem(CustomGrid *owner, int index) : base(), owner(owner), index(index)
{
}

bool CustomGridElem::GetFixed()
{
    return IsFixed();
}

int CustomGridElem::Index()
{
    return index;
}

CustomGrid* CustomGridElem::Owner()
{
    return owner;
}

bool CustomGridElem::GetResizable()
{
    return IsResizable();
}

void CustomGridElem::SetResizable(bool newres)
{
    ChangeResizable(newres);
}

bool CustomGridElem::DefaultResizable()
{
    return IsDefaultResizable();
}

bool CustomGridElem::GetVisible()
{
    return IsVisible();
}

void CustomGridElem::SetVisible(bool newvis)
{
    ChangeVisible(newvis);
}


//---------------------------------------------


void GridColumn::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->Add(std::wstring(), new IntDesignProperty<GridColumn>(L"Width", L"Appearance", &GridColumn::GetWidth, &GridColumn::SetWidth))->SetDefault(&GridColumn::DefaultWidth);
    serializer->Add(std::wstring(), new IntDesignProperty<GridColumn>(L"DefaultWidth", L"Behavior", &GridColumn::DefaultWidth, &GridColumn::SetDefaultWidth))->SetDefault(&GridColumn::GridDefaultWidth);
}

GridColumn::GridColumn(CustomGrid *owner, int index) : base(owner, index)
{
}

bool GridColumn::IsFixed()
{
    return Index() < Owner()->FixedColCount();
}

bool GridColumn::IsResizable()
{
    return Owner()->ColResize(Index());
}

void GridColumn::ChangeResizable(bool newres)
{
    Owner()->SetColResize(Index(), newres);
}

bool GridColumn::IsDefaultResizable()
{
    return Owner()->ColumnsResizable();
}

bool GridColumn::IsVisible()
{
    return Owner()->ColVisible(Index());
}

void GridColumn::ChangeVisible(bool newvis)
{
    return Owner()->SetColVisible(Index(), newvis);
}

int GridColumn::GetWidth()
{
    return Owner()->ColWidth(Index());
}

void GridColumn::SetWidth(int newwidth)
{
    Owner()->SetColWidth(Index(), newwidth);
}

int GridColumn::DefaultWidth()
{
    return Owner()->DefColWidth(Index());
}

int GridColumn::GridDefaultWidth()
{
    return Owner()->DefaultColWidth();
}

void GridColumn::SetDefaultWidth(int newdefwidth)
{
    Owner()->SetDefColWidth(Index(), newdefwidth);
}

void GridColumn::ResetWidth()
{
    Owner()->ResetColWidth(Index());
}

//---------------------------------------------


void GridRow::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->Add(std::wstring(), new IntDesignProperty<GridRow>(L"Height", L"Appearance", &GridRow::GetHeight, &GridRow::SetHeight))->SetDefault(&GridRow::DefaultHeight);
    serializer->Add(std::wstring(), new IntDesignProperty<GridRow>(L"DefaultHeight", L"Behavior", &GridRow::DefaultHeight, &GridRow::SetDefaultHeight))->SetDefault(&GridRow::GridDefaultHeight);
}

GridRow::GridRow(CustomGrid *owner, int index) : base(owner, index)
{
}

bool GridRow::IsFixed()
{
    return Index() < Owner()->FixedRowCount();
}

bool GridRow::IsResizable()
{
    return Owner()->RowResize(Index());
}

void GridRow::ChangeResizable(bool newres)
{
    Owner()->SetRowResize(Index(), newres);
}

bool GridRow::IsDefaultResizable()
{
    return Owner()->RowsResizable();
}

bool GridRow::IsVisible()
{
    return Owner()->RowVisible(Index());
}

void GridRow::ChangeVisible(bool newvis)
{
    return Owner()->SetRowVisible(Index(), newvis);
}

int GridRow::GetHeight()
{
    return Owner()->RowHeight(Index());
}

void GridRow::SetHeight(int newheight)
{
    Owner()->SetRowHeight(Index(), newheight);
}

int GridRow::DefaultHeight()
{
    return Owner()->DefRowHeight(Index());
}

int GridRow::GridDefaultHeight()
{
    return Owner()->DefaultRowHeight();
}

void GridRow::SetDefaultHeight(int newdefwidth)
{
    Owner()->SetDefRowHeight(Index(), newdefwidth);
}

void GridRow::ResetHeight()
{
    Owner()->ResetRowHeight(Index());
}


//---------------------------------------------


#endif
CustomGrid::OrderList::OrderList() : first(NULL), last(NULL), cnt(0)
{
}

CustomGrid::OrderList::~OrderList()
{
    FreeAll();
}

CustomGrid::OrderList::OrderList(OrderList &&other) noexcept : first(NULL), last(NULL), cnt(0)
{
    *this = std::move(other);
}

void CustomGrid::OrderList::FreeAll()
{
    while (first)
    {
        OrderData *tmp = first;
        first = first->next;
        delete tmp;
    }
    last = nullptr;
    cnt = 0;
}

CustomGrid::OrderList& CustomGrid::OrderList::operator=(OrderList &&other) noexcept
{
    std::swap(cnt, other.cnt);
    std::swap(first, other.first);
    std::swap(last, other.last);
    return *this;
}

CustomGrid::OrderList& CustomGrid::OrderList::operator=(const OrderList &other)
{
    FreeAll();

    OrderData *otmp = other.first;
    while (otmp)
    {
        if (!last)
            last = new OrderData();
        else
        {
            last->next = new OrderData();
            last = last->next;
        }

        *last = *otmp;
        otmp = otmp->next;
    }
    cnt = other.cnt;
    return *this;
}

int CustomGrid::OrderList::HiddenBetween(int pos1, int pos2, const sparse_list<bool> &vis)
{
    if (pos2 < pos1)
        return 0;
    auto it = vis.spos(pos1);
    auto it2 = vis.spos(pos2);
    if (it != vis.send())
        return (it2 - it) + (it.base() - vis.begin() == pos1 ? 1 : 0);
    else if (it2 == vis.send())
        return 0;
    else
        return (it2 - vis.sbegin()) + 1;

}

void CustomGrid::OrderList::resize(int newsize, const sparse_list<bool> &vis)
{
    if (newsize < 0 || newsize == cnt)
        return;
    if (!first)
    {
        first = new OrderData();
        first->cnt = newsize;
        last = first;
    }
    else if (newsize > cnt)
    {
        if (last->pos + last->cnt != cnt)
        {
            last->next = new OrderData();
            last = last->next;
            last->pos = cnt;
        }
        last->cnt = newsize - last->pos;
    }
    else if (cnt == 0)
        FreeAll();
    else
    {
        OrderData *dat = NULL;
        while (Next(dat))
        {
            OrderData *cur = Next(dat);
            if (cur->pos + cur->cnt > newsize)
            {
                if (cur->pos >= newsize)
                {
                    if (last == cur)
                        last = dat;

                    if (dat)
                        dat->next = cur->next;
                    else
                        first = cur->next;
                    delete cur;
                }
                else
                {
                    cur->hidden -= HiddenBetween(newsize, cur->pos + cur->cnt - 1, vis);
                    cur->cnt = newsize - cur->pos;
                }
            }
            dat = !dat ? first : dat->next;
        }
    }
    cnt = newsize;
}

int CustomGrid::OrderList::pos_of(int index, bool findhidden, const sparse_list<bool> &vis)
{
    if (index < 0 || index >= cnt)
        return -1;
    if (vis.is_set(index) && !findhidden)
        return -1;

    OrderData *dat = first;
    int c = 0;
    while (dat)
    {
        if (dat->pos <= index && dat->pos + dat->cnt > index)
            return c + (index - dat->pos) - HiddenBetween(dat->pos, index - 1, vis);
        c += dat->cnt - dat->hidden;
        dat = dat->next;
    }

    throw L"Position of index not found.";
}

int CustomGrid::OrderList::which_at(int index, const sparse_list<bool> &vis)
{
    if (index < 0 || index >= cnt - (int)vis.data_size())
        return -1;
    OrderData *dat = first;
    while (dat)
    {
        if (index < dat->cnt - dat->hidden)
        {
            auto it = vis.spos(dat->pos + index);
            int lastpos = dat->pos;

            if (it != vis.send() && it.base() - vis.begin() >= lastpos)
            {
                while (it != vis.send() && it.base() - vis.begin() - lastpos < index)
                {
                    index -= it.base() - vis.begin() - lastpos;
                    lastpos = it.base() - vis.begin() + 1;
                    ++it;
                }
            }
            return lastpos + index;
        }
        index -= dat->cnt - dat->hidden;
        dat = dat->next;
    }
    throw L"Position at index not found";
}

CustomGrid::OrderList::OrderData* CustomGrid::OrderList::Next(OrderData *prev)
{
    if (!prev)
        return first;
    return prev->next;
}

bool CustomGrid::OrderList::Contains(OrderData *dat, int index)
{
    return (dat->pos <= index && dat->pos + dat->cnt > index);
}

void CustomGrid::OrderList::move_from_to(int index, int to, const sparse_list<bool> &vis)
{
    if (index < 0 || index >= cnt || to < 0 || to > cnt)
        return;

    OrderData *iprev = NULL; // Data of the block previous to the one holding index.
    OrderData *tprev = to == cnt ? last : NULL; // Data of the block previous to the one holding to.
    bool ifound = false;
    bool tfound = to == cnt;
    while (!ifound || !tfound)
    {
        if (!ifound && Contains(Next(iprev), index))
            ifound = true;
        if (!tfound && Contains(Next(tprev), to))
            tfound = true;
        if (!ifound)
            iprev = Next(iprev);
        if (!tfound)
            tprev = Next(tprev);
    }

    OrderData *icur = Next(iprev);
    OrderData *tcur = Next(tprev);

    if (index == icur->pos + icur->cnt - 1 && ((to == cnt && Next(icur) == NULL) || (to != cnt && Next(icur) && to == Next(icur)->pos)))
        return;

    bool indexhidden = vis.is_set(index);
    if (icur->cnt == 1)
    {
        if (icur == first)
            first = icur->next;
        if (icur == last)
            last = iprev;
        if (iprev)
            iprev->next = icur->next;

        delete icur;

        if (iprev && iprev->next && iprev->pos + iprev->cnt == iprev->next->pos)
        {
            icur = iprev->next;
            iprev->hidden += icur->hidden;
            iprev->cnt += icur->cnt;
            iprev->next = icur->next;
            if (icur == last)
                last = iprev;
            delete icur;
        }

        icur = nullptr;
    }
    else if (index == icur->pos || index == icur->pos + icur->cnt - 1)
    {
        if (index == icur->pos)
            ++icur->pos;
        --icur->cnt;
        if (indexhidden)
            --icur->hidden;
    }
    else
    {
        OrderData *dat = new OrderData();
        dat->next = icur->next;
        icur->next = dat;
        dat->pos = index + 1;
        dat->cnt = icur->pos + icur->cnt - index - 1;
        icur->cnt = index - icur->pos;
        if (icur == last)
            last = dat;
        if (indexhidden)
            --icur->hidden;
        dat->hidden = HiddenBetween(dat->pos, dat->pos + dat->cnt - 1, vis);
        icur->hidden -= dat->hidden;
    }

    if (tcur && tprev && index == tprev->pos + tprev->cnt && index + 1 == tcur->pos)
    {
        tprev->hidden += tcur->hidden + (indexhidden ? 1 : 0);
        tprev->cnt += tcur->cnt + 1;
        tprev->next = tcur->next;
        if (last == tcur)
            last = tprev;
        delete tcur;
        return;
    }

    if (tprev && index == tprev->pos + tprev->cnt)
    {
        ++tprev->cnt;
        if (indexhidden)
            ++tprev->hidden;
        return;
    }

    if (tcur && index == tcur->pos - 1)
    {
        --tcur->pos;
        ++tcur->cnt;
        if (indexhidden)
            ++tcur->hidden;
        return;
    }

    if (tcur && to > tcur->pos)
    {
        tprev = tcur;
        tcur = new OrderData();
        tcur->next = tprev->next;
        tprev->next = tcur;
        tcur->pos = to;
        tcur->cnt = tprev->pos + tprev->cnt - to;
        tprev->cnt = to - tprev->pos;
        if (tprev == last)
            last = tcur;
        tcur->hidden = HiddenBetween(tcur->pos, tcur->pos + tcur->cnt - 1, vis);
        tprev->hidden -= tcur->hidden;
    }

    tcur = new OrderData();
    tcur->pos = index;
    tcur->cnt = 1;
    tcur->hidden = indexhidden ? 1 : 0;
    if (tprev)
    {
        tcur->next = tprev->next;
        tprev->next = tcur;
    }
    else
    {
        tcur->next = first;
        first = tcur;
    }
    if (last == tprev)
        last = tcur;
}

void CustomGrid::OrderList::remove(int index, const sparse_list<bool> &vis)
{
    if (index < 0 || index >= cnt)
        return;

    OrderData *iprev = NULL; // Data of the block previous to the one holding index.
    bool ifound = false;
    while (!ifound)
    {
        if (!ifound && Contains(Next(iprev), index))
            ifound = true;
        if (!ifound)
            iprev = Next(iprev);
    }

    OrderData *icur = Next(iprev);

    bool indexhidden = vis.is_set(index);
    if (icur->cnt == 1)
    {
        if (icur == first)
            first = icur->next;
        if (icur == last)
            last = iprev;
        if (iprev)
            iprev->next = icur->next;

        delete icur;

        if (iprev && iprev->next && iprev->pos + iprev->cnt == iprev->next->pos)
        {
            icur = iprev->next;
            iprev->hidden += icur->hidden;
            iprev->cnt += icur->cnt;
            iprev->next = icur->next;
            if (icur == last)
                last = iprev;
            delete icur;
        }
    }
    else
    {
        if (index == icur->pos)
            ++icur->pos;
        --icur->cnt;
        if (indexhidden)
            --icur->hidden;
    }

    icur = first;
    while (icur)
    {
        if (icur->pos > index)
            --icur->pos;
        icur = icur->next;
    }

    --cnt;
}

void CustomGrid::OrderList::insert(int index)
{
    if (index < 0 || index > cnt)
        return;
    if (index == cnt)
    {
        if (!first)
        {
            first = new OrderData();
            first->cnt = 1;
            last = first;
        }
        else if (last->pos + last->cnt != cnt)
        {
            last->next = new OrderData();
            last = last->next;
            last->pos = cnt;
            last->cnt = 1;
        }
        else
            ++last->cnt;
    }
    else
    {
        OrderData *dat = first;
        bool found = false;
        while (dat)
        {
            if (!found && Contains(dat, index))
            {
                found = true;
                ++dat->cnt;
            }
            else if (dat->pos > index)
                ++dat->pos;
            dat = dat->next;
        }
    }
    ++cnt;
}


//---------------------------------------------


#ifdef DESIGNING
void CustomGrid::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);

    serializer->Add(L"SetColCount", new IntDesignProperty<CustomGrid>(L"ColCount", L"Contents", &CustomGrid::ColCount, &CustomGrid::SetColCount))->SetDefault(4);
    serializer->Add(L"SetRowCount", new IntDesignProperty<CustomGrid>(L"RowCount", L"Contents", &CustomGrid::RowCount, &CustomGrid::SetRowCount))->SetDefault(4);
    serializer->Add(L"SetFixedColCount", new IntDesignProperty<CustomGrid>(L"FixedColCount", L"Contents", &CustomGrid::FixedColCount, &CustomGrid::SetFixedColCount))->SetDefault(1);
    serializer->Add(L"SetFixedRowCount", new IntDesignProperty<CustomGrid>(L"FixedRowCount", L"Contents", &CustomGrid::FixedRowCount, &CustomGrid::SetFixedRowCount))->SetDefault(1);
    serializer->Add(std::wstring(), new GridElemDesignProperty<CustomGrid, GridColumn>(L"Columns", L"Contents", &CustomGrid::ColCount, NULL, NULL));
    serializer->Add(std::wstring(), new GridElemDesignProperty<CustomGrid, GridRow>(L"Rows", L"Contents", &CustomGrid::RowCount, NULL, NULL));

    serializer->Add(L"SetSelectionKind", new GridSelectionKindsDesignProperty<CustomGrid>(L"SelectionKind", L"Appearance", &CustomGrid::SelectionKind, &CustomGrid::SetSelectionKind))->SetDefault(gskCellSelect);

    serializer->Add(L"SetDefaultColWidth", new IntDesignProperty<CustomGrid>(L"DefaultColWidth", L"Appearance", &CustomGrid::DefaultColWidth, &CustomGrid::SetDefaultColWidth))->SetDefault(64);
    serializer->Add(L"SetDefaultRowHeight", new IntDesignProperty<CustomGrid>(L"DefaultRowHeight", L"Appearance", &CustomGrid::DefaultRowHeight, &CustomGrid::SetDefaultRowHeight))->SetDefault(24);
    serializer->Add(L"SetFillColWidth", new IntDesignProperty<CustomGrid>(L"FillColWidth", L"Appearance", &CustomGrid::FillColWidth, &CustomGrid::SetFillColWidth))->SetDefault(-1);
    serializer->Add(L"SetFillRowHeight", new IntDesignProperty<CustomGrid>(L"FillRowHeight", L"Appearance", &CustomGrid::FillRowHeight, &CustomGrid::SetFillRowHeight))->SetDefault(24);
    serializer->Add(L"SetColumnsResizable", new BoolDesignProperty<CustomGrid>(L"ColumnsResizable", L"Appearance", &CustomGrid::ColumnsResizable, &CustomGrid::SetColumnsResizable))->SetDefault(false);
    serializer->Add(L"SetRowsResizable", new BoolDesignProperty<CustomGrid>(L"RowsResizable", L"Appearance", &CustomGrid::RowsResizable, &CustomGrid::SetRowsResizable))->SetDefault(false);
    serializer->Add(L"SetColWidth", new IntArrayDesignProperty<CustomGrid>(L"ColWidth", &CustomGrid::ColWidthIndexes, &CustomGrid::ColWidth, &CustomGrid::SetColWidth));
    serializer->Add(L"SetRowHeight", new IntArrayDesignProperty<CustomGrid>(L"RowHeight", &CustomGrid::RowHeightIndexes, &CustomGrid::RowHeight, &CustomGrid::SetRowHeight));
    serializer->Add(L"SetDefColWidth", new IntArrayDesignProperty<CustomGrid>(L"DefColWidth", &CustomGrid::DefColWidthIndexes, &CustomGrid::DefColWidth, &CustomGrid::SetDefColWidth));
    serializer->Add(L"SetDefRowHeight", new IntArrayDesignProperty<CustomGrid>(L"DefRowHeight", &CustomGrid::DefRowHeightIndexes, &CustomGrid::DefRowHeight, &CustomGrid::SetDefRowHeight));
    serializer->Add(L"SetColVisible", new BoolArrayDesignProperty<CustomGrid>(L"ColVisible", &CustomGrid::ColVisibleIndexes, &CustomGrid::ColVisible, &CustomGrid::SetColVisible));
    serializer->Add(L"SetRowVisible", new BoolArrayDesignProperty<CustomGrid>(L"RowVisible", &CustomGrid::RowVisibleIndexes, &CustomGrid::RowVisible, &CustomGrid::SetRowVisible));
    serializer->Add(L"SetColResize", new BoolArrayDesignProperty<CustomGrid>(L"ColResize", &CustomGrid::ColResizeIndexes, &CustomGrid::ColResize, &CustomGrid::SetColResize));
    serializer->Add(L"SetRowResize", new BoolArrayDesignProperty<CustomGrid>(L"RowResize", &CustomGrid::RowResizeIndexes, &CustomGrid::RowResize, &CustomGrid::SetRowResize));
    serializer->Add(L"SetColumnDrag", new BoolDesignProperty<CustomGrid>(L"ColumnDrag", L"Appearance", &CustomGrid::ColumnDrag, &CustomGrid::SetColumnDrag))->SetDefault(false);
    serializer->Add(L"SetRowDrag", new BoolDesignProperty<CustomGrid>(L"RowDrag", L"Appearance", &CustomGrid::RowDrag, &CustomGrid::SetRowDrag))->SetDefault(false);

    serializer->Add(L"SetSmoothHorzScroll", new BoolDesignProperty<CustomGrid>(L"SmoothHorzScroll", L"Behavior", &CustomGrid::SmoothHorzScroll, &CustomGrid::SetSmoothHorzScroll))->SetDefault(false);
    serializer->Add(L"SetSmoothVertScroll", new BoolDesignProperty<CustomGrid>(L"SmoothVertScroll", L"Behavior", &CustomGrid::SmoothVertScroll, &CustomGrid::SetSmoothVertScroll))->SetDefault(false);

    serializer->AddEvent<CustomGrid, AllowColumnRowEvent>(L"OnBeginColumnResize", L"Control");
    serializer->AddEvent<CustomGrid, ColumnRowSizeEvent>(L"OnColumnSizing", L"Control");
    serializer->AddEvent<CustomGrid, ColumnRowSizeEvent>(L"OnColumnSized", L"Control");
    serializer->AddEvent<CustomGrid, AllowColumnRowEvent>(L"OnBeginColumnDrag", L"Control");
    serializer->AddEvent<CustomGrid, AllowColumnRowEvent>(L"OnBeginRowResize", L"Control");
    serializer->AddEvent<CustomGrid, ColumnRowSizeEvent>(L"OnRowSizing", L"Control");
    serializer->AddEvent<CustomGrid, ColumnRowSizeEvent>(L"OnRowSized", L"Control");
    serializer->AddEvent<CustomGrid, AllowColumnRowEvent>(L"OnBeginRowDrag", L"Control");
}

void CustomGrid::SaveColumns()
{
    savedata = new SaveData;
    savedata->fixcnt = fixcols;
    savedata->cnt = cols;
    savedata->whlist = colwlist;
    savedata->defwhlist = defcolwlist;
    savedata->sizlist = colsizlist;
    savedata->vislist = colvislist;
    savedata->order = colorder;
}

void CustomGrid::SaveRows()
{
    savedata = new SaveData;
    savedata->fixcnt = fixrows;
    savedata->cnt = rows;
    savedata->whlist = rowhlist;
    savedata->defwhlist = defrowhlist;
    savedata->sizlist = rowsizlist;
    savedata->vislist = rowvislist;
    savedata->order = roworder;
}

void CustomGrid::RestoreColumns()
{
    fixcols = savedata->fixcnt;
    cols = savedata->cnt;
    colwlist = std::move(savedata->whlist);
    defcolwlist = std::move(savedata->defwhlist);
    colsizlist = std::move(savedata->sizlist);
    colvislist = std::move(savedata->vislist);
    colorder = std::move(savedata->order);
    Invalidate();
    ScrollResize();
}

void CustomGrid::RestoreRows()
{
    fixrows = savedata->fixcnt;
    rows = savedata->cnt;
    rowhlist = std::move(savedata->whlist);
    defrowhlist = std::move(savedata->defwhlist);
    colsizlist = std::move(savedata->sizlist);
    colvislist = std::move(savedata->vislist);
    roworder = std::move(savedata->order);
    Invalidate();
    ScrollResize();
}

void CustomGrid::FreeSaved()
{
    delete savedata;
    savedata = NULL;
}

void CustomGrid::ColWidthIndexes(std::vector<int> &indexes)
{
    indexes.reserve(colwlist.data_size());
    for (auto it = colwlist.sbegin(); it != colwlist.send(); ++it)
        indexes.push_back(it.base() - colwlist.begin());
}

void CustomGrid::RowHeightIndexes(std::vector<int> &indexes)
{
    indexes.reserve(rowhlist.data_size());
    for (auto it = rowhlist.sbegin(); it != rowhlist.send(); ++it)
        indexes.push_back(it.base() - rowhlist.begin());
}

void CustomGrid::DefColWidthIndexes(std::vector<int> &indexes)
{
    indexes.reserve(defcolwlist.data_size());
    for (auto it = defcolwlist.sbegin(); it != defcolwlist.send(); ++it)
        indexes.push_back(it.base() - defcolwlist.begin());
}

void CustomGrid::DefRowHeightIndexes(std::vector<int> &indexes)
{
    indexes.reserve(defrowhlist.data_size());
    for (auto it = defrowhlist.sbegin(); it != defrowhlist.send(); ++it)
        indexes.push_back(it.base() - defrowhlist.begin());
}

void CustomGrid::ColVisibleIndexes(std::vector<int> &indexes)
{
    indexes.reserve(colvislist.data_size());
    for (auto it = colvislist.sbegin(); it != colvislist.send(); ++it)
        indexes.push_back(it.base() - colvislist.begin());
}

void CustomGrid::RowVisibleIndexes(std::vector<int> &indexes)
{
    indexes.reserve(rowvislist.data_size());
    for (auto it = rowvislist.sbegin(); it != rowvislist.send(); ++it)
        indexes.push_back(it.base() - rowvislist.begin());
}

void CustomGrid::ColResizeIndexes(std::vector<int> &indexes)
{
    indexes.reserve(colsizlist.data_size());
    for (auto it = colsizlist.sbegin(); it != colsizlist.send(); ++it)
        indexes.push_back(it.base() - colsizlist.begin());
}

void CustomGrid::RowResizeIndexes(std::vector<int> &indexes)
{
    indexes.reserve(rowsizlist.data_size());
    for (auto it = rowsizlist.sbegin(); it != rowsizlist.send(); ++it)
        indexes.push_back(it.base() - rowsizlist.begin());
}
#endif

CustomGrid::CustomGrid() : base(false, false),
    fixcols(1), fixrows(1), cols(4), rows(4), colw(64), rowh(24), fillw(-1), fillh(24), colsiz(false), rowsiz(false),
    coldrag(false), rowdrag(false), dragwin(NULL), selkind(gskCellSelect), pos(1, 1)
{
    colwlist.resize(4);
    rowhlist.resize(4);
    defcolwlist.resize(4);
    defrowhlist.resize(4);
    colsizlist.resize(4);
    rowsizlist.resize(4);
    colvislist.resize(4);
    rowvislist.resize(4);
    colorder.resize(4, colvislist);
    roworder.resize(4, rowvislist);
}

void CustomGrid::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys) /* virtual */
{
    if (button == mbLeft)
    {
        if (base::MouseColEdge() >= 0)
        {
            if (vkeys.contains(vksDouble))
            {
                int coledge = base::MouseColEdge();
                int col = WhichCol(coledge);
                if (colwlist.is_set(col))
                {
                    int w = colwlist[col];
                    colwlist.unset(col);
                    base::InvalidateColumns(coledge);
                    if (w > DefColWidth(col))
                        InvalidateSideArea(w - DefColWidth(col));
                    ScrollResize();
                }

                Control::MouseDown(x, y, button, vkeys);
                return;
            }
        }
        else if (base::MouseRowEdge() >= 0)
        {
            if (vkeys.contains(vksDouble))
            {
                int rowedge = base::MouseRowEdge();
                int row = WhichRow(rowedge);
                if (rowhlist.is_set(row))
                {
                    int h = rowhlist[row];
                    rowhlist.unset(row);
                    base::InvalidateRows(rowedge);
                    if (h > DefRowHeight(row))
                        InvalidateBottomArea(h - DefRowHeight(row));
                    ScrollResize();
                }

                Control::MouseDown(x, y, button, vkeys);
                return;
            }
        }
        else
        {
            Point mcell = MouseCell();
            if (mcell != Point(-1, -1) && mcell.x >= FixColCnt() && mcell.y >= FixRowCnt())
                SetSelected(mcell.x, mcell.y);
        }
    }

    base::MouseDown(x, y, button, vkeys);
}

void CustomGrid::MouseMove(short x, short y, VirtualKeyStateSet vkeys)
{
    base::MouseMove(x, y, vkeys);

    if (MouseAction() == gmaColMove || MouseAction() == gmaRowMove)
    {
        if (!dragwin)
            CreateDragWindow();
        dragwin->SetTopLeft(ClientToScreen(Point(x - dragpos.x, y - dragpos.y)));
        if (!dragwin->Visible())
            dragwin->Show();
    }
}

void CustomGrid::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    if (dragwin)
    {
        if (MouseAction() == gmaColMove && base::MouseColEdge() >= 0 && base::MouseColEdge() != base::MouseCell().x && base::MouseColEdge() != base::MouseCell().x + 1)
            MoveCol(MouseCell().x, MouseColEdge());
        else if (MouseAction() == gmaRowMove && base::MouseRowEdge() >= 0 && base::MouseRowEdge() != base::MouseCell().y && base::MouseRowEdge() != base::MouseCell().y + 1)
            MoveRow(MouseCell().y, MouseRowEdge());
        dragwin->Destroy();
        dragwin = NULL;
    }
    base::MouseUp(x, y, button, vkeys);
}

void CustomGrid::Navigate(WORD &keycode, VirtualKeyStateSet vkeys)
{
    if (selkind == gskNoSelect || ((keycode == VK_LEFT || keycode == VK_RIGHT) && selkind == gskRowSelect) ||
                                  ((keycode == VK_UP || keycode == VK_DOWN) && selkind == gskColSelect) ||
                                  ((selkind == gskCellSelect || selkind == gskRowSelect) && vkeys.contains(vksCtrl) && keycode != VK_HOME && keycode != VK_END) ||
                                  (selkind == gskColSelect && vkeys.contains(vksCtrl) && keycode != VK_PRIOR && keycode != VK_NEXT) ||
                                  (selkind == gskRowSelect && !vkeys.contains(vksCtrl) && (keycode == VK_HOME || keycode == VK_END)) ||
                                  (selkind == gskColSelect && !vkeys.contains(vksCtrl) && (keycode == VK_PRIOR || keycode == VK_NEXT)))
    {
        base::Navigate(keycode, vkeys);
        return;
    }

    Point p = Selected();

    if (Options().contains(goAllowNoSelect) && p.x < 0 && p.y < 0)
        return;

    int step;
    int cnt, client, first, pos, siz;
    switch(keycode)
    {
    case VK_LEFT:
        SetSelected(p.x - 1, p.y);
        break;
    case VK_RIGHT:
        SetSelected(p.x + 1, p.y);
        break;
    case VK_UP:
        SetSelected(p.x, p.y - 1);
        break;
    case VK_DOWN:
        SetSelected(p.x, p.y + 1);
        break;
    case VK_HOME:
        if (vkeys.contains(vksCtrl))
            SetSelected(0, 0);
        else
        {
            if (selkind != gskColSelect)
                SetSelected(0, p.y);
            else
                SetSelected(p.x, 0);
        }
        break;
    case VK_END:
        if (vkeys.contains(vksCtrl))
            SetSelected(cols, rows);
        else
        {
            if (selkind != gskColSelect)
                SetSelected(cols, p.y);
            else
                SetSelected(p.x, rows);
        }
        break;
    case VK_PRIOR:
        if (vkeys.contains(vksCtrl))
            SetSelected(0, 0);
        else
        {
            if (selkind == gskColSelect)
            {
                cnt = FixColCnt();
                pos = 0;
                first = p.x;
                siz = ColW(first);
                client = ClientWidth() - FixColW();
                step = 0;
                while (first > cnt && pos + siz < client)
                {
                    --first;
                    pos += siz;
                    siz = ColW(first);
                    ++step;
                }

                SetSelected(p.x - step, p.y);
            }
            else
            {
                cnt = FixRowCnt();
                pos = 0;
                first = p.y;
                siz = RowH(first);
                client = ClientHeight() - FixRowH();
                step = 0;
                while (first > cnt && pos + siz < client)
                {
                    --first;
                    pos += siz;
                    siz = RowH(first);
                    ++step;
                }

                SetSelected(p.x, p.y - step);
            }
        }
        break;
    case VK_NEXT:
        if (vkeys.contains(vksCtrl))
            SetSelected(cols, rows);
        else
        {
            if (selkind == gskColSelect)
            {
                cnt = ColCnt();
                pos = 0;
                first = p.x;
                siz = ColW(first);
                client = ClientWidth() - FixColW();
                step = 0;
                while (first < cnt - 1 && pos + siz < client)
                {
                    ++first;
                    pos += siz;
                    siz = ColW(first);
                    ++step;
                }

                SetSelected(p.x + step, p.y);
            }
            else
            {
                cnt = RowCnt();
                pos = 0;
                first = p.y;
                siz = RowH(first);
                client = ClientHeight() - FixRowH();
                step = 0;
                while (first < cnt - 1 && pos + siz < client)
                {
                    ++first;
                    pos += siz;
                    siz = RowH(first);
                    ++step;
                }

                SetSelected(p.x, p.y + max(1, step));
            }
        }
        break;
    };
}

void CustomGrid::CancelAction()
{
    if (dragwin)
    {
        dragwin->Destroy();
        dragwin = NULL;
    }

    base::CancelAction();
}

int CustomGrid::MouseColEdge()
{
    int edge = base::MouseColEdge();
    return WhichCol(edge);
}

int CustomGrid::MouseRowEdge()
{
    int edge = base::MouseRowEdge();
    return WhichRow(edge);
}

Point CustomGrid::MouseCell()
{
    Point pt = base::MouseCell();
    if (pt.x >= 0)
        pt.x = WhichCol(pt.x);
    if (pt.y >= 0)
        pt.y = WhichRow(pt.y);
    return pt;
}

void CustomGrid::CreateDragWindow()
{
    Bitmap bmp;
    bmp.SetFont(GetFont());
    Point cell = MouseCell();
    if (MouseAction() == gmaColMove)
    {
        int h = FixRowH();
        int w = ColWidth(cell.x);
        cell.x = WhichCol(cell.x);
        bmp.SetSize(min(w, 320), min(h, 320));

        Rect r(0, 0, w, 0);
        int cnt = FixRowCnt();
        for (int ix = 0; ix < cnt && r.top < bmp.Height(); ++ix)
        {
            int row = WhichRow(ix);
            r.bottom += RowH(ix);
            DrawHead(&bmp, cell.x, row, r, r, false);

            r.top = r.bottom;
        }
    }
    else
    {
        int w = FixColW();
        int h = RowHeight(cell.y);
        cell.y = WhichRow(cell.y);
        bmp.SetSize(min(w, 320), min(h, 320));

        Rect r(0, 0, 0, h);
        int cnt = FixColCnt();
        for (int ix = 0; ix < cnt && r.left < bmp.Width(); ++ix)
        {
            int col = WhichRow(ix);
            r.right += ColW(ix);
            DrawHead(&bmp, col, cell.y, r, r, false);

            r.left = r.right;
        }
    }

    dragpos = MousePos();
    dragpos.x = min(min(160, bmp.Width()), max(0, dragpos.x));
    dragpos.y = min(min(160, bmp.Height()), max(0, dragpos.y));

    Color cl = clBlack;
    Gdiplus::GraphicsPath *gp = bmp.CreateEllipsePath(0, 0, 640, 640);
    Gdiplus::Matrix mx;
    mx.Translate(-340.0 + dragpos.x, -320.0 + dragpos.y);
    gp->Transform(&mx);
    Brush br(gp, Color(200, 200, 200), &cl, 1);
    delete gp;

    Bitmap tmp(bmp.Width(), bmp.Height());
    tmp.SetBrush(br);
    tmp.FillRect(0, 0, bmp.Width(), bmp.Height());

    int w = bmp.Width();
    int h = bmp.Height();
    auto tmpbits = tmp.LockBits(glmReadOnly, PixelFormat32bppARGB);
    auto bmpbits = bmp.LockBits(glmReadWrite, PixelFormat32bppARGB);
    byte *tmpline = (byte*)tmpbits->Scan0;
    byte *bmpline = (byte*)bmpbits->Scan0;

    Color mix = Color(clHotlight).ToRGB();
    for (int iy = 0; iy < h; ++iy)
    {
        for (int ix = 0; ix < w; ++ix)
        {
            bmpline[ix * 4 + 3] = tmpline[ix * 4];
            float alpha = ((float)tmpline[ix * 4] / 255.0) * 0.95;
            bmpline[ix * 4 + 0] = (byte)(alpha * min(255, 0.95 * bmpline[ix * 4 + 0] + 0.05 * mix.B()));
            bmpline[ix * 4 + 1] = (byte)(alpha * min(255, 0.95 * bmpline[ix * 4 + 1] + 0.05 * mix.G()));
            bmpline[ix * 4 + 2] = (byte)(alpha * min(255, 0.95 * bmpline[ix * 4 + 2] + 0.05 * mix.R()));
        }
        tmpline += tmpbits->Stride;
        bmpline += bmpbits->Stride;
    }
    tmp.UpdateBits();
    bmp.UpdateBits();

    //bmp.Draw(&tmp, 0, 0);

    dragwin = new LayeredWindow(std::move(bmp));
    dragwin->SetTopmost(true);
    dragwin->Show();
}

int CustomGrid::ColPos(int col, bool findhidden)
{
    if (col < 0 || col >= cols)
        return -1;
#ifdef DESIGNING
    if (Designing())
        return col;
#endif

    return colorder.pos_of(col, findhidden, colvislist);
}

int CustomGrid::RowPos(int row, bool findhidden)
{
    if (row < 0 || row >= rows)
        return -1;
#ifdef DESIGNING
    if (Designing())
        return row;
#endif

    return roworder.pos_of(row, findhidden, rowvislist);
}

int CustomGrid::WhichCol(int pos)
{
    if (pos < 0 || pos > cols)
        return -1;

    if (pos == ColCnt())
        return ColCount();

#ifdef DESIGNING
    if (Designing())
        return pos;
#endif

    return colorder.which_at(pos, colvislist);
}

int CustomGrid::WhichRow(int pos)
{
    if (pos < 0 || pos > rows)
        return -1;

    if (pos == RowCnt())
        return RowCount();

#ifdef DESIGNING
    if (Designing())
        return pos;
#endif

    return roworder.which_at(pos, rowvislist);
}

void CustomGrid::MoveCol(int col, int before)
{
    if (col < 0 || before < 0 || col >= cols || before > cols)
        return;

    bool colvis = !colvislist.is_set(col);
    int cp = cols;
    if (colvis)
        cp = ColPos(col);
    colorder.move_from_to(col, before, colvislist);
    if (colvis)
        base::InvalidateColumns(min(cp, ColPos(col)));
}

void CustomGrid::MoveRow(int row, int before)
{
    if (row < 0 || before < 0 || row >= rows || before > rows)
        return;

    bool rowvis = !rowvislist.is_set(row);
    int rp = rows;
    if (rowvis)
        rp = RowPos(row);
    roworder.move_from_to(row, before, rowvislist);
    if (rowvis)
        base::InvalidateRows(min(rp, RowPos(row)));
}

void CustomGrid::PushUpdatePoint(Point *pt)
{
    updatept.push_back(pt);
}

void CustomGrid::PopUpdatePoint()
{
    updatept.pop_back();
}

int CustomGrid::HorzSiz() /* virtual */
{
    int res = 0;
    int def = 0;
#ifdef DESIGNING
    if (Designing())
    {
        for (auto it = colwlist.sbegin(); it != colwlist.send(); ++it)
            res += *it;
        for (auto it = defcolwlist.sbegin(); it != defcolwlist.send(); ++it)
        {
            if (!colwlist.is_set(it.base() - defcolwlist.begin()))
                res += *it;
            else
                ++def;
        }

        return res + colw * (cols - colwlist.data_size() - (defcolwlist.data_size() - def));
    }
#endif
    int hid = 0;
    for (auto it = colwlist.sbegin(); it != colwlist.send(); ++it)
    {
        if (!colvislist.is_set(it.base() - colwlist.begin()))
            res += *it;
        else
            ++hid;
    }
    for (auto it = defcolwlist.sbegin(); it != defcolwlist.send(); ++it)
    {
        if (!colvislist.is_set(it.base() - defcolwlist.begin()) && !colwlist.is_set(it.base() - defcolwlist.begin()))
            res += *it;
        else
            ++def;
    }

    return res + colw * (cols - colwlist.data_size() - colvislist.data_size() + hid - defcolwlist.data_size() + def);
}

int CustomGrid::VertSiz() /* virtual */
{
    int res = 0;
    int def = 0;
#ifdef DESIGNING
    if (Designing())
    {
        for (auto it = rowhlist.sbegin(); it != rowhlist.send(); ++it)
            res += *it;
        for (auto it = defrowhlist.sbegin(); it != defrowhlist.send(); ++it)
        {
            if (!rowhlist.is_set(it.base() - defrowhlist.begin()))
                res += *it;
            else
                ++def;
        }
        return res + rowh * (rows - rowhlist.data_size() - (defrowhlist.data_size() - def));
    }
#endif
    int hid = 0;
    for (auto it = rowhlist.sbegin(); it != rowhlist.send(); ++it)
    {
        if (!rowvislist.is_set(it.base() - rowhlist.begin()))
            res += *it;
        else
            ++hid;
    }
    for (auto it = defrowhlist.sbegin(); it != defrowhlist.send(); ++it)
    {
        if (!rowvislist.is_set(it.base() - defrowhlist.begin()) && !rowhlist.is_set(it.base() - defrowhlist.begin()))
            res += *it;
        else
            ++def;
    }

    return res + rowh * (rows - rowhlist.data_size() - rowvislist.data_size() + hid - defrowhlist.data_size() + def);
}

int CustomGrid::FixColCnt() /* virtual */
{
#ifdef DESIGNING
    if (Designing())
        return fixcols;
#endif
    int res = fixcols;
    for (int ix = 0; ix < fixcols; ++ix)
        if (colvislist.is_set(ix))
            --res;
    return res;
}

int CustomGrid::FixRowCnt() /* virtual */
{
#ifdef DESIGNING
    if (Designing())
        return fixrows;
#endif
    int res = fixrows;
    for (int ix = 0; ix < fixrows; ++ix)
        if (rowvislist.is_set(ix))
            --res;
    return res;
}

int CustomGrid::ColCnt() /* virtual */
{
#ifdef DESIGNING
    if (Designing())
        return cols;
#endif
    return cols - colvislist.data_size();
}

int CustomGrid::RowCnt() /* virtual */
{
#ifdef DESIGNING
    if (Designing())
        return rows;
#endif
    return rows - rowvislist.data_size();
}

int CustomGrid::ColW(int col) /* virtual */
{
    if (col < 0)
        return fillw;

    col = WhichCol(col);

    if (colwlist.is_set(col))
        return colwlist[col];
    if (defcolwlist.is_set(col))
        return defcolwlist[col];
    return colw;
}

int CustomGrid::RowH(int row) /* virtual */
{
    if (row < 0)
        return fillh;

    row = WhichRow(row);

    if (rowhlist.is_set(row))
        return rowhlist[row];
    if (defrowhlist.is_set(row))
        return defrowhlist[row];
    return rowh;
}

bool CustomGrid::CanColSiz(int col) /* virtual */
{
#ifdef DESIGNING
    if (Designing())
        return true;
#endif

    if (ColResize(WhichCol(col)))
    {
        if (OnBeginColumnResize)
        {
            bool allow = true;
            OnBeginColumnResize(this, AllowColumnRowParameters(col, allow));
            if (!allow)
                return false;
        }
        return true;
    }
    return false;
}

bool CustomGrid::CanRowSiz(int row) /* virtual */
{
#ifdef DESIGNING
    if (Designing())
        return true;
#endif

    if (RowResize(WhichRow(row)))
    {
        if (OnBeginRowResize)
        {
            bool allow = true;
            OnBeginRowResize(this, AllowColumnRowParameters(row, allow));
            if (!allow)
                return false;
        }
        return true;
    }
    return false;
}

void CustomGrid::UpdateColWidth(int col, int widthchange) /* virtual */
{
    int wcol = WhichCol(col);
    int colw = (colwlist.is_set(wcol) ? colwlist[wcol] : DefColWidth(wcol));
    int w = colw + widthchange;
    if ((MouseAction() == gmaColResize && OnColumnSizing) || (MouseAction() == gmaNone && OnColumnSized))
    {
        if (MouseAction() == gmaColResize)
            OnColumnSizing(this, ColumnRowSizeParameters(wcol, w));
        else
            OnColumnSized(this, ColumnRowSizeParameters(wcol, w));
    }
    w = max(0, w);
    if (w == colw)
        return;

    colwlist[wcol] = w;

    base::UpdateColWidth(col, widthchange);
}

void CustomGrid::UpdateRowHeight(int row, int heightchange) /* virtual */
{
    int wrow = WhichRow(row);
    int rowh = (rowhlist.is_set(wrow) ? rowhlist[wrow] : DefRowHeight(wrow));
    int h = rowh + heightchange;
    if ((MouseAction() == gmaRowResize && OnRowSizing) || (MouseAction() == gmaNone && OnRowSized))
    {
        if (MouseAction() == gmaRowResize)
            OnRowSizing(this, ColumnRowSizeParameters(wrow, h));
        else
            OnRowSized(this, ColumnRowSizeParameters(wrow, h));
    }
    h = max(0, h);
    if (h == rowh)
        return;

    rowhlist[wrow] = h;

    base::UpdateRowHeight(row, heightchange);
}

bool CustomGrid::CanColMove(int col)
{
#ifdef DESIGNING
    if (Designing())
        return false;
#endif

    if (coldrag == false)
        return false;

    col = WhichCol(col);
    if (OnBeginColumnDrag)
    {
        bool allow = true;
        OnBeginColumnDrag(this, AllowColumnRowParameters(col, allow));
        return allow;
    }
    return true;
}

bool CustomGrid::CanRowMove(int row)
{
#ifdef DESIGNING
    if (Designing())
        return false;
#endif

    if (rowdrag == false)
        return false;

    row = WhichRow(row);
    if (OnBeginRowDrag)
    {
        bool allow = true;
        OnBeginRowDrag(this, AllowColumnRowParameters(row, allow));
        return allow;
    }
    return true;
}

GridSelectionKinds CustomGrid::SelKind()
{
    return selkind;
}

bool CustomGrid::IsSel(int col, int row)
{
    if (selkind == gskNoSelect)
        return false;

    return (selkind == gskRowSelect && pos.y == row) || (selkind == gskColSelect && pos.x == col) || (selkind == gskCellSelect && pos == Point(col, row));
}

GridSelectionKinds CustomGrid::SelectionKind()
{
    return selkind;
}

void CustomGrid::SetSelectionKind(GridSelectionKinds newselkind)
{
    if (selkind == newselkind)
        return;

    if (selkind == gskRowSelect && pos.y >= 0)
        InvalidateRow(pos.y);
    else if (selkind == gskColSelect && pos.x >= 0)
        InvalidateColumn(pos.x);
    else if (selkind == gskCellSelect && pos != Point(-1, -1))
        InvalidateCell(pos);

    selkind = newselkind;

    if (!HandleCreated() || ((selkind == gskNoSelect || Options().contains(goAllowNoSelect)) && pos == Point(-1, -1)))
        return;

    if (selkind == gskNoSelect)
        pos = Point(-1, -1);
    else if (selkind == gskRowSelect)
        SetSelected(-1, pos.y < 0 ? FixRowCnt() : pos.y);
    else if (selkind == gskColSelect)
        SetSelected(pos.x < 0 ? FixColCnt() : pos.x, -1);
    else
        SetSelected(pos.x < 0 ? FixColCnt() : pos.x, pos.y < 0 ? FixRowCnt() : pos.y);
}

void CustomGrid::DeleteColumn(int col)
{
    if (cols <= 1 || col >= cols)
        return;

    int pcol = ColPos(col);
    CountChanging(pcol, -1, true);

    if (!updatept.empty())
    {
        std::for_each(updatept.begin(), updatept.end(), [col](Point *pt) {
            if (col == pt->x)
                *pt = Point(-1, -1);
            else if (pt->x > col)
                --pt->x;
        });
    }

    int w = colvislist.is_set(col) ? 0 : ColWidth(col);
    if (fixcols > col || fixcols == cols - 1)
    {
        --fixcols;
#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"FixColCount");
#endif
    }

    colorder.remove(col, colvislist);

    --cols;
    colwlist.erase(colwlist.begin() + col);
    defcolwlist.erase(defcolwlist.begin() + col);
    colsizlist.erase(colsizlist.begin() + col);
    colvislist.erase(colvislist.begin() + col);

    CountChanged(pcol, -1, true);

    if (w)
        base::UpdateColWidth(pcol, -w);
}

void CustomGrid::DeleteRow(int row)
{
    if (rows <= 1 || row >= rows)
        return;

    int prow = RowPos(row);
    CountChanging(-1, prow, true);

    if (!updatept.empty())
    {
        std::for_each(updatept.begin(), updatept.end(), [row](Point *pt) {
            if (row == pt->y)
                *pt = Point(-1, -1);
            else if (pt->y > row)
                --pt->y;
        });
    }

    int h = rowvislist.is_set(row) ? 0 : RowHeight(row);
    if (fixrows > row || fixrows == rows - 1)
    {
        --fixrows;
#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"FixRowCount");
#endif
    }

    roworder.remove(row, rowvislist);

    --rows;
    rowhlist.erase(rowhlist.begin() + row);
    defrowhlist.erase(defrowhlist.begin() + row);
    rowsizlist.erase(rowsizlist.begin() + row);
    rowvislist.erase(rowvislist.begin() + row);

    CountChanged(-1, prow, true);

    if (h)
        base::UpdateRowHeight(prow, -h);
}

int CustomGrid::ColCount() const
{
    return cols;
}

void CustomGrid::SetColCount(int newcolcnt)
{
    newcolcnt = max(1, newcolcnt);
    if (cols == newcolcnt)
        return;

    CountChanging(-1, -1, false);

    BeginUpdate();

    cols = newcolcnt;
    if (fixcols > cols - 1)
    {
        fixcols = cols - 1;
#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"FixColCount");
#endif
    }

    colorder.resize(newcolcnt, colvislist);

    colwlist.resize(newcolcnt);
    defcolwlist.resize(newcolcnt);
    colsizlist.resize(newcolcnt);
    colvislist.resize(newcolcnt);

    if (!updatept.empty())
    {
        std::for_each(updatept.begin(), updatept.end(), [newcolcnt](Point *pt) {
            if (pt->x >= newcolcnt)
                *pt = Point(-1, -1);
        });
    }

    EndUpdate();

    CountChanged(-1, -1, false);
    Invalidate();

    //Point p = Selected();
    //if (p.x >= cols)
    //    SetSelected(0, p.y);
}

int CustomGrid::RowCount() const
{
    return rows;
}

void CustomGrid::SetRowCount(int newrowcnt)
{
    newrowcnt = max(1, newrowcnt);
    if (rows == newrowcnt)
        return;

    CountChanging(-1, -1, false);

    BeginUpdate();

    rows = newrowcnt;
    if (fixrows > rows - 1)
    {
        fixrows = rows - 1;
#ifdef DESIGNING
        if (Designing() && designer && designer->MainPropertyOwner(this))
            designer->InvalidateRow(this, L"FixRowCount");
#endif
    }

    roworder.resize(newrowcnt, rowvislist);

    rowhlist.resize(newrowcnt);
    defrowhlist.resize(newrowcnt);
    rowsizlist.resize(newrowcnt);
    rowvislist.resize(newrowcnt);

    if (!updatept.empty())
    {
        std::for_each(updatept.begin(), updatept.end(), [newrowcnt](Point *pt) {
            if (pt->y >= newrowcnt)
                *pt = Point(-1, -1);
        });
    }

    EndUpdate();

    CountChanged(-1, -1, false);
    Invalidate();

    //Point p = Selected();
    //if (p.y >= rows)
    //    SetSelected(p.x, 0);
}

void CustomGrid::InsertColumn(int before)
{
    if (before < 0 || before > cols)
        return;

    CountChanging(ColPos(before), -1, false);

    BeginUpdate();

    colorder.insert(before);
    colwlist.insert_space(colwlist.begin() + before);
    defcolwlist.insert_space(defcolwlist.begin() + before);
    colsizlist.insert_space(colsizlist.begin() + before);
    colvislist.insert_space(colvislist.begin() + before);

    if (!updatept.empty())
    {
        std::for_each(updatept.begin(), updatept.end(), [before](Point *pt) {
            if (pt->x >= before)
                ++pt->x;
        });
    }

    EndUpdate();

    before = ColPos(before);
    CountChanged(before, -1, false);
    base::InvalidateColumns(before);
}

void CustomGrid::InsertRow(int before)
{
    if (before < 0 || before > rows)
        return;

    CountChanging(-1, RowPos(before), false);

    BeginUpdate();

    roworder.insert(before);
    rowhlist.insert_space(rowhlist.begin() + before);
    defrowhlist.insert_space(defrowhlist.begin() + before);
    rowsizlist.insert_space(rowsizlist.begin() + before);
    rowvislist.insert_space(rowvislist.begin() + before);

    if (!updatept.empty())
    {
        std::for_each(updatept.begin(), updatept.end(), [before](Point *pt) {
            if (pt->y >= before)
                ++pt->y;
        });
    }

    EndUpdate();

    before = RowPos(before);
    CountChanged(-1, before, false);
    base::InvalidateRows(before);
}

int CustomGrid::FixedColCount() const
{
    return fixcols;
}

void CustomGrid::SetFixedColCount(int newfixcolcnt)
{
    newfixcolcnt = max(0, min(cols - 1, newfixcolcnt));
    if (fixcols == newfixcolcnt)
        return;

    CountChanging(-1, -1, false);

    fixcols = newfixcolcnt;

    CountChanged(-1, -1, false);
    Invalidate();
}

int CustomGrid::FixedRowCount() const
{
    return fixrows;
}

void CustomGrid::SetFixedRowCount(int newfixrowcnt)
{
    newfixrowcnt = max(0, min(rows - 1, newfixrowcnt));
    if (fixrows == newfixrowcnt)
        return;

    CountChanging(-1, -1, false);

    fixrows = newfixrowcnt;

    CountChanged(-1, -1, false);
    Invalidate();
}

bool CustomGrid::ColResize(int col)
{
    if (colsizlist.is_set(col))
        return !colsiz;
    return colsiz;
}

void CustomGrid::SetColResize(int col, bool newres)
{
    if (newres == colsiz)
        colsizlist.unset(col);
    else
        colsizlist[col] = newres;
}

bool CustomGrid::ColumnsResizable()
{
    return colsiz;
}

void CustomGrid::SetColumnsResizable(bool newres)
{
    if (colsiz == newres)
        return;
    colsiz = newres;
    colsizlist.clear();
    colsizlist.resize(cols);
}

bool CustomGrid::RowResize(int row)
{
    if (rowsizlist.is_set(row))
        return !rowsiz;
    return rowsiz;
}

void CustomGrid::SetRowResize(int row, bool newres)
{
    if (newres == rowsiz)
        rowsizlist.unset(row);
    else
        rowsizlist[row] = newres;
}

bool CustomGrid::RowsResizable()
{
    return rowsiz;
}

void CustomGrid::SetRowsResizable(bool newres)
{
    if (rowsiz == newres)
        return;
    rowsizlist.clear();
    rowsizlist.resize(rows);
    rowsiz = newres;
}

bool CustomGrid::ColumnDrag()
{
    return coldrag;
}

void CustomGrid::SetColumnDrag(bool newdrag)
{
    coldrag = newdrag;
}

bool CustomGrid::RowDrag()
{
    return rowdrag;
}

void CustomGrid::SetRowDrag(bool newdrag)
{
    rowdrag = newdrag;
}

bool CustomGrid::SmoothHorzScroll()
{
    return SmoothX();
}

void CustomGrid::SetSmoothHorzScroll(bool newsmoothscroll)
{
    SetSmoothX(newsmoothscroll);
}

bool CustomGrid::SmoothVertScroll()
{
    return SmoothY();
}

void CustomGrid::SetSmoothVertScroll(bool newsmoothscroll)
{
    SetSmoothY(newsmoothscroll);
}

bool CustomGrid::ColVisible(int col)
{
    return !colvislist.is_set(col);
}

void CustomGrid::SetColVisible(int col, bool newvis)
{
    if (newvis != colvislist.is_set(col))
        return;

    CountChanging(ColPos(col, true), -1, !newvis);

    int w = 0;
    if (newvis)
        colvislist.unset(col);
    else
    {
        if (col >= fixcols && ColCnt() - FixColCnt() == 1)
            return;
        colvislist[col] = false;
        w = ColWidth(col);
    }
#ifdef DESIGNING
    if (Designing())
        return;
#endif

    col = ColPos(col, true);
    CountChanged(col, -1, !newvis);
    if (w)
        base::UpdateColWidth(col, -w);
    else
        base::InvalidateColumns(col);
}

bool CustomGrid::RowVisible(int row)
{
    return !rowvislist.is_set(row);
}

void CustomGrid::SetRowVisible(int row, bool newvis)
{
    if (newvis != rowvislist.is_set(row))
        return;

    CountChanging(-1, RowPos(row, true), !newvis);

    int h = 0;
    if (newvis)
        rowvislist.unset(row);
    else
    {
        if (row >= fixrows && RowCnt() - FixRowCnt() == 1)
            return;
        rowvislist[row] = false;
        h = RowHeight(0);
    }
#ifdef DESIGNING
    if (Designing())
        return;
#endif

    row = RowPos(row, true);
    CountChanged(-1, row, !newvis);
    if (h)
        base::UpdateRowHeight(row, -h);
    else
        base::InvalidateRows(row);
}

int CustomGrid::ColWidth(int col)
{
    if (colwlist.is_set(col))
        return colwlist[col];
    return DefColWidth(col);
}

void CustomGrid::SetColWidth(int col, int newwidth)
{
    newwidth = max(0, newwidth);
    if (colwlist.is_set(col) && colwlist[col] == newwidth)
        return;

    int w = ColWidth(col);
    colwlist[col] = newwidth;

#ifdef DESIGNING
    if (!Designing() && colvislist.is_set(col))
        return;
#endif

    base::UpdateColWidth(ColPos(col), newwidth - w);
}

int CustomGrid::DefColWidth(int col)
{
    if (defcolwlist.is_set(col))
        return defcolwlist[col];
    return colw;
}

int CustomGrid::FillColWidth()
{
    return fillw;
}

void CustomGrid::SetFillColWidth(int newwidth)
{
    if (fillw == newwidth)
        return;
    fillw = newwidth;
    if (HandleCreated() && Options().contains(goLinesFillHorz))
        InvalidateSide();
}

void CustomGrid::SetDefColWidth(int col, int newwidth)
{
    newwidth = max(0, newwidth);
    int w = DefColWidth(col);
    if (w == newwidth)
        return;

    if (newwidth == colw)
        defcolwlist.unset(col);
    else
        defcolwlist[col] = newwidth;

#ifdef DESIGNING
    if (!colwlist.is_set(col) && (Designing() || !colvislist.is_set(col)))
        base::UpdateColWidth(ColPos(col), newwidth - w);
#else
    if (!colwlist.is_set(col) && !colvislist.is_set(col))
        base::UpdateColWidth(ColPos(col), newwidth - w);
#endif
}

int CustomGrid::DefaultColWidth()
{
    return colw;
}

void CustomGrid::SetDefaultColWidth(int newwidth)
{
    newwidth = max(0, newwidth);
    if (colw == newwidth)
        return;
    colw = newwidth;
    ScrollResize();
    Invalidate();
}

void CustomGrid::ResetColWidth(int col)
{
    if (!colwlist.is_set(col))
        return;
    int w = colwlist[col];
    colwlist.unset(col);

#ifdef DESIGNING
    if (!Designing() && colvislist.is_set(col))
        return;
#endif
    base::UpdateColWidth(ColPos(col), DefColWidth(col) - w);
}

int CustomGrid::RowHeight(int row)
{
    if (rowhlist.is_set(row))
        return rowhlist[row];
    return DefRowHeight(row);
}

void CustomGrid::SetRowHeight(int row, int newheight)
{
    newheight = max(0, newheight);
    if (rowhlist.is_set(row) && rowhlist[row] == newheight)
        return;

    int h = RowHeight(row);
    rowhlist[row] = newheight;

#ifdef DESIGNING
    if (!Designing() && rowvislist.is_set(row))
        return;
#endif

    base::UpdateRowHeight(RowPos(row), newheight - h);
}

int CustomGrid::DefRowHeight(int row)
{
    if (defrowhlist.is_set(row))
        return defrowhlist[row];
    return rowh;
}

int CustomGrid::FillRowHeight()
{
    return fillh;
}

void CustomGrid::SetFillRowHeight(int newheight)
{
    if (fillh == newheight)
        return;
    fillh = newheight;
    if (HandleCreated() && Options().contains(goLinesFillVert))
        InvalidateBottom();
}

void CustomGrid::SetDefRowHeight(int row, int newheight)
{
    newheight = max(0, newheight);
    int w = DefRowHeight(row);
    if (w == newheight)
        return;

    if (newheight == rowh)
        defrowhlist.unset(row);
    else
        defrowhlist[row] = newheight;

#ifdef DESIGNING
    if (!rowhlist.is_set(row) && (Designing() || !rowvislist.is_set(row)))
        base::UpdateRowHeight(RowPos(row), newheight - w);
#else
    if (!rowhlist.is_set(row) && !rowvislist.is_set(row))
        base::UpdateRowHeight(RowPos(row), newheight - w);
#endif
}

int CustomGrid::DefaultRowHeight()
{
    return rowh;
}

void CustomGrid::SetDefaultRowHeight(int newheight)
{
    newheight = max(0, newheight);
    if (rowh == newheight)
        return;
    rowh = newheight;
    ScrollResize();
    Invalidate();
}

void CustomGrid::ResetRowHeight(int row)
{
    if (!rowhlist.is_set(row))
        return;
    int w = rowhlist[row];
    rowhlist.unset(row);

#ifdef DESIGNING
    if (!Designing() && rowvislist.is_set(row))
        return;
#endif
    base::UpdateRowHeight(RowPos(row), DefRowHeight(row) - w);
}

Rect CustomGrid::CellRect(int col, int row)
{
    col = ColPos(col);
    row = RowPos(row);
    if (col < 0 || row < 0)
        return Rect();
    return base::CellRect(col, row);
}

Rect CustomGrid::ColRect(int col)
{
    col = ColPos(col);
    if (col < 0)
        return Rect();
    return base::ColRect(col);
}

Rect CustomGrid::RowRect(int row)
{
    row = RowPos(row);
    if (row < 0)
        return Rect();
    return base::RowRect(row);
}

Point CustomGrid::CellAt(int x, int y)
{
    Point pt = base::CellAt(x, y);
    return Point(WhichCol(pt.x), WhichRow(pt.y));
}

bool CustomGrid::ScrollToCell(int col, int row)
{
    if (col >= 0)
        col = ColPos(col);
    if (row >= 0)
        row = RowPos(row);
    return base::ScrollToCell(col, row);
}

Point CustomGrid::SelectedCell()
{
    Point pt = pos;
    if (pt.x >= 0)
        pt.x = WhichCol(pt.x);
    if (pt.y >= 0)
        pt.y = WhichRow(pt.y);
    return pt;
}

Point CustomGrid::Selected()
{
    return SelectedCell();
}

void CustomGrid::SetSelected(int col, int row)
{
    if (selkind == gskNoSelect || pos == Point(col, row) || (pos == Point(-1, -1) && (col < 0 || row < 0)))
        return;

    int ccnt = ColCnt();
    int rcnt = RowCnt();

    int fccnt = FixColCnt();
    int frcnt = FixRowCnt();

    if (((col < 0 && selkind != gskRowSelect) || (row < 0 && selkind != gskColSelect)) && Options().contains(goAllowNoSelect))
    {
        col = -1;
        row = -1;
    }
    else
    {
        if (selkind != gskRowSelect)
            col = min(max(col, fccnt), ccnt - 1);
        if (selkind != gskColSelect)
            row = min(max(row, frcnt), rcnt - 1);
    }
    if (selkind == gskRowSelect)
        col = -1;
    if (selkind == gskColSelect)
        row = -1;

    if (pos == Point(col, row))
        return;

    Point oldpos = pos;
    pos = Point(col, row);

    if (selkind == gskRowSelect)
    {
        if (oldpos.y != -1)
            base::InvalidateRow(oldpos.y);
        if (pos.y != -1)
            base::InvalidateRow(pos.y);
    }
    else if (selkind == gskColSelect)
    {
        if (oldpos.x != -1)
            base::InvalidateColumn(oldpos.x);
        if (pos.x != -1)
            base::InvalidateColumn(pos.x);
    }
    else
    {
        if (oldpos.x != -1)
            base::InvalidateCell(oldpos.x, oldpos.y);
        if (pos.x != -1)
            base::InvalidateCell(pos.x, pos.y);
    }

    if (pos.x >= 0 || pos.y >= 0)
        base::ScrollToCell(pos.x, pos.y);
}

void CustomGrid::InvalidateCell(int col, int row)
{
    col = ColPos(col);
    if (col >= 0)
        row = RowPos(row);
    if (col < 0 || row < 0)
        return;
    base::InvalidateCell(col, row);
}

void CustomGrid::InvalidateCell(const Point &p)
{
    InvalidateCell(p.x, p.y);
}

void CustomGrid::InvalidateColumn(int col)
{
    col = ColPos(col);
    if (col < 0)
        return;
    base::InvalidateColumn(col);
}

void CustomGrid::InvalidateRow(int row)
{
    row = RowPos(row);
    if (row < 0)
        return;
    base::InvalidateRow(row);
}

GridPosition CustomGrid::PositionAt(int x, int y)
{
    GridPosition pos = base::PositionAt(x, y);
    if (pos.col >= 0)
    {
        if (pos.col == ColCnt())
            pos.col = ColCount();
        else
            pos.col = WhichCol(pos.col);

    }
    if (pos.row >= 0)
    {
        if (pos.row == RowCnt())
            pos.row = RowCount();
        else
            pos.row = WhichRow(pos.row);
    }
    return pos;
}

bool CustomGrid::ColLeft(const Rect &clientrect, int col, int &colleft, bool *first)
{
    col = ColPos(col);
    if (col < 0)
    {
        if (first)
            *first = false;
        colleft = -1;
        return false;
    }
    return base::ColLeft(clientrect, col, colleft, first);
}

int CustomGrid::ColAt(const Rect &clientrect, int x, bool *first, int *colleft, int *colwidth)
{
    int col = base::ColAt(clientrect, x, first, colleft, colwidth);
    if (col >= 0)
        return WhichCol(col);
    return col;
}

bool CustomGrid::RowTop(const Rect &clientrect, int row, int &rowtop, bool *first)
{
    row = RowPos(row);
    if (row < 0)
    {
        if (first)
            *first = false;
        rowtop = -1;
        return false;
    }
    return base::RowTop(clientrect, row, rowtop, first);
}

int CustomGrid::RowAt(const Rect &clientrect, int y, bool *first, int *rowtop, int *rowheight)
{
    int row = base::RowAt(clientrect, y, first, rowtop, rowheight);
    if (row >= 0)
        return WhichRow(row);
    return row;
}

void CustomGrid::CountChanged(int col, int row, bool deleted)
{
    ScrollResize();
    if (selkind == gskNoSelect)
        return;

    int ccnt = ColCnt();
    int rcnt = RowCnt();
    int fccnt = FixColCnt();
    int frcnt = FixRowCnt();

    int poscol = col == -1 || col > pos.x ? pos.x : col < pos.x ? pos.x + (deleted ? -1 : 1) : deleted ? pos.x : pos.x + 1;
    int posrow = row == -1 || row > pos.y ? pos.y : row < pos.y ? pos.y + (deleted ? -1 : 1) : deleted ? pos.y : pos.y + 1;
    if (poscol >= 0)
    {
        if (poscol >= ccnt)
            poscol = ccnt - 1;
        else if (poscol < fccnt)
            poscol = fccnt;
    }
    if (posrow >= 0)
    {
        if (posrow >= rcnt)
            posrow = rcnt - 1;
        else if (posrow < frcnt)
            posrow = frcnt;
    }
    if (pos != Point(poscol, posrow))
        SetSelected(poscol, posrow);
}

void CustomGrid::CountChanging(int col, int row, bool deleting)
{
    ;
}

void CustomGrid::OptionsChanged()
{
    if (!Options().contains(goAllowNoSelect) && selkind != gskNoSelect && pos == Point(-1, -1))
        SetSelected(0, 0);
}


//---------------------------------------------


#ifdef DESIGNING
void StringGrid::EnumerateProperties(DesignSerializer *serializer)
{
    base::EnumerateProperties(serializer);
    serializer->AddEvent<StringGrid, GridCellAlignmentEvent>(L"OnCellTextAlign", L"Drawing");
    serializer->Add(L"SetAutoEdit", new BoolDesignProperty<StringGrid>(L"AutoEdit", L"Behavior", &StringGrid::AutoEdit, &StringGrid::SetAutoEdit))->SetDefault(false);
    serializer->Add(L"SetTabEdited", new BoolDesignProperty<StringGrid>(L"TabEdited", L"Behavior", &StringGrid::TabEdited, &StringGrid::SetTabEdited))->SetDefault(false);

    serializer->AddEvent<StringGrid, BeginCellEditEvent>(L"OnBeginCellEdit", L"Editing");
    serializer->AddEvent<StringGrid, CellEditedEvent>(L"OnCellEdited", L"Editing");
    serializer->AddEvent<StringGrid, EndCellEditEvent>(L"OnEndCellEdit", L"Editing");

    serializer->AddEvent<StringGrid, KeyEvent>(L"OnEditorKeyDown", L"Editing");
    serializer->AddEvent<StringGrid, KeyEvent>(L"OnEditorKeyUp", L"Editing");
    serializer->AddEvent<StringGrid, KeyPushEvent>(L"OnEditorKeyPush", L"Editing");
    serializer->AddEvent<StringGrid, KeyPressEvent>(L"OnEditorKeyPress", L"Editing");
    serializer->AddEvent<StringGrid, DialogCodeEvent>(L"OnEditorDialogCode", L"Editing");

}
#endif

StringGrid::StringGrid() : editor(NULL), editcell(-1, -1), autoedit(false), tabedit(false)
{
    strings.resize(RowCount());
}

LRESULT StringGrid::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int mousex, mousey;
    GridPosition pos;
    Rect r;

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        if (!autoedit || MouseAction() != gmaNone)
            break;
        mousex = GET_X_LPARAM(lParam);
        mousey = GET_Y_LPARAM(lParam);
        pos = PositionAt(mousex, mousey);
        if (pos.type != gptCell)
            break;

        EditCell(pos.col, pos.row);
        if (!Editing())
            break;
        r = CellRect(pos.col, pos.row);
        SendMessage(editor->Handle(), WM_LBUTTONDOWN, 0, MAKELPARAM(mousex - r.left, mousey - r.top));
        return 0;
    }
    return base::WindowProc(uMsg, wParam, lParam);
}

void StringGrid::MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    if (Editing())
    {
        GridPosition pos = PositionAt(x, y);
        if (pos.type != gptCell || pos.col != editcell.x || pos.row != editcell.y)
            EndEditCell(false);
    }

    base::MouseDown(x, y, button, vkeys);
}

void StringGrid::MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys)
{
    base::MouseUp(x, y, button, vkeys);

    if (Editing() && !editor->Focused())
    {
        editor->Focus();
        editor->Invalidate();
    }
}

void StringGrid::Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code)
{
    base::Scrolled(kind, oldpos, pos, code);
    if (!editor || !editor->IsVisible())
        return;

    if (kind == skHorizontal)
    {
        int left = 0;
        bool first = false;
        if ((!SmoothX() && pos + FixColCnt() <= editcell.x) || ColLeft(ClientRect(), editcell.x, left, &first))
        {
            if (SmoothX() && first)
            {
                int fw = FixColW();
                editor->SetInnerLeftMargin(left - fw);
                editor->SetLeft(fw);
            }
            else
            {
                editor->SetInnerLeftMargin(0);
                editor->SetLeft(left);
            }
        }
        else
            editor->SetLeft(-editor->Width());
    }
    else
    {
        int top = 0;
        bool first = false;
        if ((!SmoothY() && pos + FixRowCnt() <= editcell.y) || RowTop(ClientRect(), editcell.y, top, &first))
        {
            int h = !SmoothY() ? 0 : RowH(editcell.y) - (Options().contains(goHorzLines) ? 1 : 0);

            if (SmoothY() && first)
            {
                int fh = FixRowH();
                int eh = editor->Height();
                editor->SetHeight(top + h - fh);
                editor->SetTopMargin(editor->TopMargin() + ((top + h - fh) - eh));
                editor->SetTop(fh);
            }
            else
            {
                if (SmoothY())
                {
                    int eh = editor->Height();
                    editor->SetHeight(h);
                    editor->SetTopMargin(editor->TopMargin() + (h - eh));
                }
                editor->SetTop(top);
            }
        }
        else
            editor->SetTop(-editor->Height());
    }
}

void StringGrid::EditCell(int col, int row)
{
    if (col < FixedColCount() || col >= ColCount() || !ColVisible(col) || row < FixedRowCount() || row >= RowCount() || !RowVisible(row))
        return;

    if (editcell == Point(col, row) && editor && editor->IsVisible())
        return;

    Point p = Point(col, row);
    PushUpdatePoint(&p);

    if (editor && editor->IsVisible())
        EndEditCell(false);

    PopUpdatePoint();

    CreateEditor(); // Make sure the editor is created.
    if (editor)
        editor->Hide(); // Hide the editor to make sure it is not updated while it is on screen.

    editcell = Point(p.x, p.y);
    PushUpdatePoint(&editcell);

    if (OnBeginCellEdit)
    {
        bool allow = true;
        OnBeginCellEdit(this, BeginCellEditParameters(p.x, p.y, allow));
        if (!allow)
            return;
    }

    PopUpdatePoint();

    if (editcell.x < FixedColCount() || editcell.x >= ColCount() || editcell.y < FixedRowCount() || editcell.y >= RowCount())
        return;

    SetSelected(editcell.x, editcell.y);
    ScrollToCell(editcell.x, editcell.y);

    Rect r = CellRect(editcell.x, editcell.y);

    if (Options().contains(goVertLines))
        --r.right;
    if (Options().contains(goHorzLines))
        --r.bottom;

    TextAlignments align = taLeft;
    VerticalTextAlignments valign = vtaTop;
    if (OnCellTextAlign)
        OnCellTextAlign(this, GridCellAlignmentParameters(align, valign));

    std::wstring str = String(editcell.x, editcell.y);
    // Update the editor's dimensions and text.
    Size s = GetCanvas()->MeasureText(str.empty() ? L"My" : str);

    editor->SetTopMargin(valign == vtaTop ? 2 * Scaling : valign == vtaMiddle ? (r.Height() - s.cy) / 2 : r.Height() - s.cy - 2 * Scaling);
    editor->SetBounds(r);
    //editor->SetTextAlign(align);
    editor->SetText(str);
    editor->SetVisible(true);
    editor->Focus();
}

Point StringGrid::CellEdited()
{
    return editcell;
}

void StringGrid::CreateEditor()
{
    if (editor)
        return;
    editor = new ControlEdit();
    editor->SetVisible(false);
    editor->SetParent(this);
    editor->OnKeyDown = CreateEvent(this, &StringGrid::editorkeydown);
    editor->OnKeyPress = CreateEvent(this, &StringGrid::editorkeypress);
    editor->OnKeyPush = CreateEvent(this, &StringGrid::editorkeypush);
    editor->OnKeyUp = CreateEvent(this, &StringGrid::editorkeyup);
    editor->OnGainFocus = CreateEvent(this, &StringGrid::editorgainfocus);
    editor->OnLoseFocus = CreateEvent(this, &StringGrid::editorlosefocus);
    editor->OnEnter = CreateEvent(this, &StringGrid::editorenter);
    editor->OnLeave = CreateEvent(this, &StringGrid::editorleave);
    editor->OnDialogCode = CreateEvent(this, &StringGrid::editordlgcode);
    editor->SetMargins(2 * Scaling, 2 * Scaling);
}

void StringGrid::editorkeydown(void *sender, KeyParameters param)
{
    if (OnEditorKeyDown)
    {
        OnEditorKeyDown(sender, param);
        if (param.keycode == 0)
            return;
    }

    Point p;
    int len;
    switch (param.keycode)
    {
    case VK_TAB:
        if (!tabedit)
            break;
        p = CellEdited();
        if (p.x < 0 || p.y < 0)
            break;
        if (param.vkeys.contains(vksShift))
        {
            if (p.x != WhichCol(FixColCnt()))
                EditCell(WhichCol(ColPos(p.x) - 1), p.y);
            else if (p.y != WhichCol(FixRowCnt()))
                EditCell(WhichCol(ColCnt() - 1), WhichRow(RowPos(p.y) - 1));
            len = EditorText().length();
            SetEditorSelStartAndLength(0, len);
            param.keycode = 0;
        }
        else
        {
            if (p.x != WhichCol(ColCnt() - 1))
                EditCell(WhichCol(ColPos(p.x) + 1), p.y);
            else if (p.y != WhichCol(RowCnt() - 1))
                EditCell(WhichCol(FixColCnt()), WhichRow(RowPos(p.y) + 1));
            len = EditorText().length();
            SetEditorSelStartAndLength(0, len);
            param.keycode = 0;
        }

        break;
    }
}

void StringGrid::editorkeyup(void *sender, KeyParameters param)
{
    if (OnEditorKeyUp)
    {
        OnEditorKeyUp(sender, param);
        if (param.keycode == 0)
            return;
    }
}

void StringGrid::editorkeypush(void *sender, KeyPushParameters param)
{
    if (OnEditorKeyPush)
    {
        OnEditorKeyPush(sender, param);
        if (param.key == 0)
            return;
    }
}

void StringGrid::editorkeypress(void *sender, KeyPressParameters param)
{
    if (OnEditorKeyPress)
    {
        OnEditorKeyPress(sender, param);
        if (param.key == 0)
            return;
    }

    switch (param.key)
    {
    case VK_ESCAPE:
        EndEditCell(true);
        param.key = 0;
        break;
    case VK_RETURN:
        EndEditCell(false);
        param.key = 0;
        break;
    case VK_TAB:
        if (!tabedit)
            break;
        param.key = 0;
        break;
    }
}

void StringGrid::EndEditCell(bool cancel)
{
    if (!editor || !editor->IsVisible())
        return;

    if (!cancel)
    {
        bool b;
        std::wstring str;
        b = true;
        str = editor->Text();
        editor->Hide();

        Point p = editcell;
        PushUpdatePoint(&p);

        if (OnCellEdited)
            OnCellEdited(this, CellEditedParameters(p.x, p.y, b, str));
        if (OnEndCellEdit)
            OnEndCellEdit(this, EndCellEditParameters(p.x, p.y, !b));

        PopUpdatePoint();
        if (b && p.x != -1 && p.y != -1)
            SetString(p.x, p.y, str);
        editcell = Point(-1, -1);
    }
    else
    {
        editor->Hide();
        if (OnEndCellEdit)
            OnEndCellEdit(this, EndCellEditParameters(editcell.x, editcell.y, true));
        editcell = Point(-1, -1);
    }
    Focus();
}

std::wstring StringGrid::EditorText()
{
    if (!editor || !editor->IsVisible())
        return std::wstring();
    return editor->Text();
}

void StringGrid::SetEditorText(const std::wstring &text)
{
    if (!editor || !editor->IsVisible())
        return;
    editor->SetText(text);
}

void StringGrid::EditorSelStartAndLength(int &selstart, int &sellen)
{
    if (!editor || !editor->IsVisible())
        return;
    editor->SelStartAndLength(selstart, sellen);
}

void StringGrid::SetEditorSelStartAndLength(int selstart, int sellen)
{
    if (!editor || !editor->IsVisible())
        return;
    editor->SetSelStartAndLength(selstart, sellen);
}

bool StringGrid::Editing()
{
    return editor && editor->IsVisible();
}

void StringGrid::GainFocus(HWND otherwindow)
{
    if (editor && editor->IsVisible() && editor->EditHandle() == otherwindow)
        return;
    base::GainFocus(otherwindow);
}

void StringGrid::LoseFocus(HWND otherwindow)
{
    if (editor && editor->IsVisible() && editor->EditHandle() == otherwindow)
        return;
    base::LoseFocus(otherwindow);
}

void StringGrid::ActiveEnter(Control *other)
{
    if (editor && editor->IsVisible() && editor->Editor() == other)
        return;
    base::ActiveEnter(other);
}

void StringGrid::ActiveLeave(Control *other)
{
    if (editor && editor->IsVisible() && editor->Editor() == other)
        return;
    EndEditCell(false);
    base::ActiveLeave(other);
}

void StringGrid::editorenter(void *sender, ActiveChangedParameters param)
{
    if (param.other == this)
        return;
    ActiveEnter(param.other);
}

void StringGrid::editorleave(void *sender, ActiveChangedParameters param)
{
    if (param.other == this)
        return;
    ActiveLeave(param.other);
}

void StringGrid::editorgainfocus(void *sender, FocusChangedParameters param)
{
    if (param.otherwindow == Handle())
        return;
    GainFocus(param.otherwindow);
}

void StringGrid::editorlosefocus(void *sender, FocusChangedParameters param)
{
    if (param.otherwindow == Handle())
        return;
    LoseFocus(param.otherwindow);
}

void StringGrid::editordlgcode(void *sender, DialogCodeParameters param)
{
    if (tabedit)
        param.result << dcWantTab;
    if (OnEditorDialogCode)
        OnEditorDialogCode(this, param);
}

bool StringGrid::Focused()
{
    if (editor && editor->IsVisible())
        return true;
    return base::Focused();
}

void StringGrid::Focus()
{
    if (Focused())
    {
        if (editor && editor->IsVisible())
            editor->Focus();
        return;
    }
    base::Focus();
}

bool StringGrid::AutoEdit()
{
    return autoedit;
}

void StringGrid::SetAutoEdit(bool newautoedit)
{
    autoedit = newautoedit;
}

bool StringGrid::TabEdited()
{
    return tabedit;
}

void StringGrid::SetTabEdited(bool newtabedit)
{
    tabedit = newtabedit;
}

void StringGrid::DrawHead(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool hovered)
{
    base::DrawHead(c, col, row, r, clip, hovered);
    col = WhichCol(col);
    row = WhichRow(row);
    if (!StringSet(col, row))
        return;

    TextAlignments align = taLeft;
    VerticalTextAlignments valign = vtaTop;
    if (OnCellTextAlign)
        OnCellTextAlign(this, GridCellAlignmentParameters(align, valign));

    std::wstring str = String(col, row);
    int x = r.left;
    int y = r.top;

    Size s;
    if (align != taLeft && valign != vtaTop)
        s = c->MeasureText(str);
    x += align == taLeft ? 2 * Scaling : taCenter ? (r.Width() - s.cx) / 2 : r.Width() - s.cx - 2 * Scaling;
    y += valign == vtaTop ? 2 * Scaling : vtaMiddle ? (r.Height() - s.cy) / 2 : r.Height() - s.cy - 2 * Scaling;

    c->TextDraw(r, x, y, str);
}

void StringGrid::DrawCell(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool selected, bool hovered)
{
    base::DrawCell(c, col, row, r, clip, selected, hovered);
    col = WhichCol(col);
    row = WhichRow(row);

    if (!StringSet(col, row) || (Editing() && editcell.x == col && editcell.y == row))
        return;

    TextAlignments align = taLeft;
    VerticalTextAlignments valign = vtaTop;
    if (OnCellTextAlign)
        OnCellTextAlign(this, GridCellAlignmentParameters(align, valign));

    std::wstring str = String(col, row);
    int x = r.left;
    int y = r.top;

    Size s;
    if (align != taLeft && valign != vtaTop)
        s = c->MeasureText(str);
    x += align == taLeft ? 2 * Scaling : taCenter ? (r.Width() - s.cx) / 2 : r.Width() - s.cx - 2 * Scaling;
    y += valign == vtaTop ? 2 * Scaling : vtaMiddle ? (r.Height() - s.cy) / 2 : r.Height() - s.cy - 2 * Scaling;

    c->TextDraw(r, x, y, str);
}

bool StringGrid::StringSet(int col, int row)
{
    return strings.is_set(row) && strings[row].is_set(col) && !strings[row][col].first.empty();
}

std::wstring StringGrid::String(int col, int row)
{
    if (!strings.is_set(row) || !strings[row].is_set(col))
        return std::wstring();
    return strings[row][col].first;
}

std::pair<std::wstring, void*> StringGrid::StringAndData(int col, int row)
{
    if (!strings.is_set(row) || ! strings[row].is_set(col))
        return std::make_pair(std::wstring(), nullptr);
    return strings[row][col];
}

void StringGrid::SetString(int col, int row, const std::wstring &str)
{
    if (str.empty() && (!strings.is_set(row) || !strings[row].is_set(col)))
    {
        if (Editing() && editcell == Point(col, row))
            editor->SetText(std::wstring());
        return;
    }
    if (!strings.is_set(row) || !strings[row].is_set(col))
    {
        if (!strings.is_set(row))
            strings[row].resize(ColCount());
        strings[row][col] = std::make_pair(str, nullptr);
    }
    else
        strings[row][col].first = str;
    InvalidateCell(col, row);
    if (Editing() && editcell == Point(col, row))
        editor->SetText(String(col, row));
}

void StringGrid::SetString(int col, int row, std::wstring &&str)
{
    if (str.empty() && (!strings.is_set(row) || !strings[row].is_set(col)))
    {
        if (Editing() && editcell == Point(col, row))
            editor->SetText(std::wstring());
        return;
    }
    if (!strings.is_set(row) || !strings[row].is_set(col))
    {
        if (!strings.is_set(row))
            strings[row].resize(ColCount());
        strings[row][col] = std::make_pair(std::move(str), nullptr);
    }
    else
        strings[row][col].first = std::move(str);
    InvalidateCell(col, row);
    if (Editing() && editcell == Point(col, row))
        editor->SetText(String(col, row));
}

void StringGrid::SetStringAndData(int col, int row, const std::wstring &str, void *data)
{
    if (str.empty() && data == nullptr && (!strings.is_set(row) || !strings[row].is_set(col)))
    {
        if (Editing() && editcell == Point(col, row))
            editor->SetText(std::wstring());
        return;
    }
    if (!strings.is_set(row))
        strings[row].resize(ColCount());
    strings[row][col] = std::make_pair(str, data);
    InvalidateCell(col, row);
    if (Editing() && editcell == Point(col, row))
        editor->SetText(String(col, row));
}

void StringGrid::SetStringAndData(int col, int row, std::wstring &&str, void *data)
{
    if (str.empty() && data == nullptr && (!strings.is_set(row) || !strings[row].is_set(col)))
    {
        if (Editing() && editcell == Point(col, row))
            editor->SetText(std::wstring());
        return;
    }
    if (!strings.is_set(row))
        strings[row].resize(ColCount());
    strings[row][col] = std::make_pair(std::move(str), data);
    InvalidateCell(col, row);
    if (Editing() && editcell == Point(col, row))
        editor->SetText(String(col, row));
}

void* StringGrid::Data(int col, int row)
{
    if (!strings.is_set(row) || ! strings[row].is_set(col))
        return nullptr;
    return strings[row][col].second;
}

void StringGrid::SetData(int col, int row, void *data)
{
    if (data == nullptr && (!strings.is_set(row) || !strings[row].is_set(col)))
        return;
    if (!strings.is_set(row) || !strings[row].is_set(col))
    {
        if (!strings.is_set(row))
            strings[row].resize(ColCount());
        strings[row][col] = std::make_pair(std::wstring(), data);
    }
    else
        strings[row][col].second = data;
}

void StringGrid::DeleteColumn(int col)
{
    if (Editing())
    {
        if (editcell.x == col)
            EndEditCell(true);
        else if (ColVisible(col) && ColPos(col) < editcell.x)
            editor->SetLeft(editor->Left() - ColWidth(col));
    }

    auto it = strings.sbegin();
    while(it != strings.send())
    {
        it->erase(it->begin() + col);
        if (it->data_size() == 0)
            it = strings.erase(it);
        else
            ++it;
    }

    base::DeleteColumn(col);
}

void StringGrid::InsertColumn(int before)
{
    for (auto it = strings.sbegin(); it != strings.send(); ++it)
        it->insert_space(it->begin() + before);

    base::InsertColumn(before);

    if (Editing() && ColPos(before) < editcell.x)
        editor->SetLeft(editor->Left() + ColWidth(before));
}

void StringGrid::SetColCount(int newcolcnt)
{
    if (Editing() && editcell.x >= newcolcnt)
        EndEditCell(true);

    auto it = strings.sbegin();
    while(it != strings.send())
    {
        it->resize(newcolcnt);
        if (it->data_size() == 0)
            it = strings.erase(it);
        else
            ++it;
    }
    base::SetColCount(newcolcnt);
}

void StringGrid::DeleteRow(int row)
{
    if (Editing())
    {
        if (editcell.y == row)
            EndEditCell(true);
        else if (RowVisible(row) && RowPos(row) < editcell.y)
            editor->SetTop(editor->Top() - RowHeight(row));
    }

    strings.erase(strings.begin() + row);
    base::DeleteRow(row);
}

void StringGrid::InsertRow(int before)
{
    strings.insert_space(strings.begin() + before);

    base::InsertRow(before);

    if (Editing() && RowPos(before) < editcell.y)
        editor->SetTop(editor->Top() + RowHeight(before));
}

void StringGrid::SetRowCount(int newrowcnt)
{
    if (Editing() && editcell.y >= newrowcnt)
        EndEditCell(true);

    strings.resize(newrowcnt);
    base::SetRowCount(newrowcnt);
}

void StringGrid::SetColVisible(int col, bool newvis)
{
    if (Editing())
    {
        if (editcell.x == col)
        {
            if (!newvis)
                EndEditCell(true);
        }
        else if ((ColVisible(col) != newvis) && ColPos(col) < editcell.x)
            editor->SetLeft(editor->Left() + ColWidth(col) * (!newvis ? -1 : 1));
    }
    base::SetColVisible(col, newvis);
}

void StringGrid::SetRowVisible(int row, bool newvis)
{
    if (Editing())
    {
        if (editcell.y == row)
        {
            if (!newvis)
                EndEditCell(true);
        }
        else if ((RowVisible(row) != newvis) && RowPos(row) < editcell.y)
            editor->SetTop(editor->Top() + RowHeight(row) * (!newvis ? -1 : 1));
    }
    base::SetRowVisible(row, newvis);
}

void StringGrid::SwapColData(int a, int b)
{
    if (a == b || a < 0 || b < 0 || a >= ColCount() || b >= ColCount())
        return;

    Point ed = !Editing() || (editcell.x != a && editcell.x != b) ? Point(-1, -1) : editcell;
    if (ed.x >= 0)
        EndEditCell(false);

    for (auto it = strings.sbegin(); it != strings.send(); ++it)
    {
        if (!it->is_set(a) && !it->is_set(b))
            continue;
        std::pair<std::wstring, void*> apair;
        std::pair<std::wstring, void*> bpair;
        if (it->is_set(a))
            apair = std::move((*it)[a]);
        if (it->is_set(b))
            apair = std::move((*it)[b]);
        if (!bpair.first.empty() && bpair.second != nullptr)
            (*it)[a] = std::move(bpair);
        else
            it->unset(a);
        if (!apair.first.empty() && apair.second != nullptr)
            (*it)[b] = std::move(apair);
        else
            it->unset(b);
    }

    if (ed.x >= 0)
        EditCell(ed.x == a ? b : a, ed.y);
    InvalidateColumns(min(a, b));
}

void StringGrid::SwapRowData(int a, int b)
{
    if (a == b || a < 0 || b < 0 || a >= RowCount() || b >= RowCount())
        return;

    Point ed = !Editing() || (editcell.y != a && editcell.y != b) ? Point(-1, -1) : editcell;
    if (ed.x >= 0)
        EndEditCell(false);

    if (strings.is_set(a) || strings.is_set(b))
    {
        sparse_list<std::pair<std::wstring, void*>> astr;
        sparse_list<std::pair<std::wstring, void*>> bstr;
        if (strings.is_set(a))
            astr = std::move(strings[a]);
        if (strings.is_set(b))
            bstr = std::move(strings[b]);
        if (bstr.data_size() > 0)
            strings[a] = std::move(bstr);
        else
            strings.unset(a);
        if (astr.data_size() > 0)
            strings[b] = std::move(astr);
        else
            strings.unset(b);
    }

    if (ed.x >= 0)
        EditCell(ed.x, ed.y == a ? b : a);
    InvalidateRows(min(a, b));
}

Point StringGrid::SelectedCell()
{
    if (Editing())
        return editcell;
    return base::SelectedCell();
}

void StringGrid::CountChanging(int col, int row, bool deleting)
{
    base::CountChanging(col, row, deleting);
    if (!Editing() || (col == -1 && row == -1))
        return;

    if (deleting)
    {
        col = WhichCol(col);
        row = WhichRow(row);
        if (editcell == Point(col, row))
            EndEditCell(true);
    }

}

void StringGrid::CountChanged(int col, int row, bool deleted)
{
    base::CountChanged(col, row, deleted);
    if (!Editing())
        return;

    if (editcell.x < FixedColCount() || editcell.y < FixedRowCount() || editcell.x >= ColCount() || editcell.y >= RowCount() || !ColVisible(editcell.x) || !RowVisible(editcell.y))
    {
        EndEditCell(true);
        return;
    }

    if (col == -1 && row == -1)
        return;

    Point ecell = Point(ColPos(editcell.x), RowPos(editcell.y));
    if (ecell.x < col && ecell.y < row)
        return;

    if (col >= 0 && ecell.x >= col)
        ecell.x += deleted ? -1 : 1;
    if (row >= 0 && ecell.y >= row)
        ecell.y += deleted ? -1 : 1;
    ecell = Point(WhichCol(ecell.x), WhichRow(ecell.y));
    if (ecell == editcell)
        return;

    Rect r = CellRect(ecell.x, ecell.y);
    --r.right;
    --r.bottom;
    editor->SetBounds(r);
}


//---------------------------------------------


}
/* End of NLIBNS */

