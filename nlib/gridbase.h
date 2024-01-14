#pragma once

#include "sparse_list.h"
#include "controlbase.h"


namespace NLIBNS
{


class LayeredWindow;

enum GridOptions {  
        goVertLines      = 0x0001, // Draw vertical lines between columns. 
        goHorzLines      = 0x0002, // Draw horizontal lines between rows.
        goLinesFillVert  = 0x0004, // Draw lines up to the bottom of the control
        goLinesFillHorz  = 0x0008, // Draw lines up to the right side of the control
        goAlwaysShowSel  = 0x0010, // Show which cell or row is selected in the control even when it doesn't have the focus.
        goAllowNoSelect  = 0x0020, // Whether it is possible to have no cell or row selected in the list grid.
        goFixedTracking  = 0x0040, // Highlight the fixed cell currently under the mouse.
        goCellTracking   = 0x0080, // Highlight the cell currently under the mouse.
        goMultiSelect    = 0x0100,
#ifdef DESIGNING
        gdoCount = 9
#endif
};
typedef uintset<GridOptions> GridOptionSet;

enum GridSelectionKinds {
        gskNoSelect    = 0,
        gskCellSelect  = 1,
        gskRowSelect   = 2,
        gskColSelect   = 3,
#ifdef DESIGNING
        gskCount = 4
#endif
};

// Used to determine some position inside grids. (i.e. where the mouse is at.)
enum GridPositionTypes {
    gptFixed, // Over a non scrolling fixed cell.
    gptVertEdge, // On the edge between columns. This value is only set when the column is resizable.
    gptHorzEdge, // On the edge between rows. This value is only set when the row is resizable.
    gptCell, // Inside a cell in the grid.
    gptNowhere, // Outside the grid's client area.
    gptAfterCells, // Inside the grid's client area, but not over the header nor inside a cell.
};

struct GridPosition
{
    GridPositionTypes type; // Type of the position.
    int col; // Column at the position. This can be -1 when the type is not a column or its edge. If the type is gptAfterCells, the col can be equal to ColCount(), indicating that the position is after the last column.
    int row; // Row at the position. This can be -1 when the type is not a cell. If the type is gptAfterCells, the row can be equal to RowCount(), indicating that the position is after the last row.
};

enum GridMouseActions { gmaNone, gmaColResize, gmaRowResize, gmaColMove, gmaRowMove }; // List of possible actions the user can do with the mouse.

/* Base class for all the grids in the library. Override the abstract methods in derived classes to specify
 * the properties of columns, rows and the data to be displayed */
class GridBase : public ScrollableControl
{
private:
    typedef ScrollableControl   base;

    bool smoothx; // Scroll columns by units of pixel and not cell.
    bool smoothy; // Scroll rows by units of pixel and not cell.

    GridOptionSet options;
    Color gridcolor;

    int mcoledge; // Column whose edge is currently under the mouse cursor. -1 when the mouse is not over a column edge.
    int mrowedge; // Row whose edge is currently under the mouse cursor. -1 when the mouse is not over a row edge.
    Point mcell; // Current cell under the mouse cursor. -1 when the mouse is not over a cell. Not changed during dragging.
    Point mpos; // Client coordinate where the mouse was pressed before starting an action. Used in detecting cursor movement to start a drag operation. It is also the relative position inside a dragged column or row.
    GridMouseActions maction; // Current action controlled with the mouse cursors. i.e. column resizing.

    // Called when the mouse moves over an edge of the grid to update mcoledge and mrowedge. Changes the mouse cursor if sizing is enabled for the given column.
    void UpdateColEdge(int newedge);
    void UpdateRowEdge(int newedge);
protected:
    //virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void FirstCol(int &col, int *colx = NULL, int *colw = NULL); // Returns the first visible column's index after the fixed columns. When variable addresses are passed, sets colx to the left of the column in client coordinates and colw to the width of the column. Col is set to -1 if there are no columns after the fixed ones.
    virtual void FirstRow(int &row, int *rowy = NULL, int *rowh = NULL); // Returns the first visible row's index after the fixed rows. When variable addresses are passed, sets rowy to the top of the row in client coordinates and rowh to the height of the row. Row is set to -1 if there are no rows after the fixed ones.

    virtual bool IsSel(int col, int row) = 0; // Returns whether the passed cell is selected. Should work even when whole row or column selection is enabled, in which case only the relevant coordinate should be considered.

    int FixColW(); // Width of all fixed columns.
    int FixRowH(); // Height of all fixed rows.

    virtual void GetOverflow(int &uw, int &uh, int &hw, int &hh, bool &hnohide, bool &vnohide);
    virtual void Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code);

    virtual void Paint(const Rect &updaterect);

    bool ColLeft(const Rect &clientrect, int col, int &colleft, bool *first = NULL); // Returns the left coordinate of the passed column in colleft if column is visible. Returns true if column is visible. Fixed columns count as visible even if they don't fit the current window size.
    int ColAt(const Rect &clientrect, int x, bool *first = NULL, int *colleft = NULL, int *colwidth = NULL); // Returns the column under the given x coordinate and sets first to true if the returned column is the first visible non fixed. The result is -1 when the coordinate is outside the client area or -2 if the coordinate is to the right of the last column. When colleft is set, it receives the left x coordinate of the returned column, or the left of the area to the right of the last column when the result is -2. When colwidth is set, it receives the width of the column or -1 when there is no column at the given coordinate.
    bool RowTop(const Rect &clientrect, int row, int &rowtop, bool *first = NULL); // Sets rowtop to the top coordinate of the passed row if row is visible. Returns true if row is visible. Fixed rows count as visible even if they don't fit the current window size.
    int RowAt(const Rect &clientrect, int y, bool *first = NULL, int *rowtop = NULL, int *rowheight = NULL); // Returns the row under the given y coordinate and sets first to true if the returned row is the first visible non fixed. The result is -1 when the coordinate is outside the client area or -2 if the coordinate is below the last row. When rowtop is set, it receives the top y coordinate of the returned row, or the top of the area below of the last row when the result is -2. When rowheight is set, it receives the height of the row or -1 when there is no row at the given coordinate.

    virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
    virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
    virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
    virtual void MouseLeave();
    virtual void KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);

    // Keyboard navigation function called when pressing the navigation keys. The default behavior is to simply scroll the window.
    virtual void Navigate(WORD &keycode, VirtualKeyStateSet vkeys);

    virtual void GainFocus(HWND otherwindow); // Invalidates the selected cell or cells.
    virtual void LoseFocus(HWND otherwindow); // Invalidates the selected cell or cells.

    virtual void NeedsDialogCode(WORD key, DialogCodeSet &dialogcode);
    virtual void CaptureChanged();

    void InvalidateColumns(int first, int last = -1); // Invalidate columns between first and last.
    void InvalidateRows(int first, int last = -1); // Invalidate rows between first and last.
    void InvalidateRowCells(int row, int first, int last = -1); // Invalidate cells in a row between first and last.
    void InvalidateColCells(int col, int first, int last = -1); // Invalidate cells in a column between first and last.
    void InvalidateSideArea(int width); // Invalidates the empty area of the window to the right of the last column in the passed width.
    void InvalidateBottomArea(int height); // Invalidates the area below the last row in the passed height. 
    void InvalidateSide(); // Invalidates the empty area of the window to the right of the last column.
    void InvalidateBottom(); // Invalidates the area below the last row.

    virtual int HorzSiz() = 0; // The size of all grid cells horizontally, including the fixed columns. If smooth horizontal scrolling is not active, the function is not called.
    virtual int VertSiz() = 0; // The size of all grid cells vertically, including the fixed rows. If smooth vertical scrolling is not active, the function is not called.

    virtual int FixColCnt() = 0; // Number of fixed/header columns.
    virtual int FixRowCnt() = 0; // Number of fixed/header rows.
    virtual int ColCnt() = 0; // Number of columns shown in the grid.
    virtual int RowCnt() = 0; // Number of rows shown in the grid.

    virtual int ColW(int col) = 0; // Width of the specified column. Passed index of the column can be -1, in which case the result must be a general column width used in drawing lines after the last column. In that case if the result is 0 or less, there won't be general column lines drawn.
    virtual int RowH(int row) = 0; // Height of the specified row. Passed index of the row can be -1, in which case the result must be a general row height used in drawing lines after the last row. In that case if the result is 0 or less, there won't be general row lines drawn.

    virtual bool CanColSiz(int col) = 0; // Return whether the given column can be resized.
    virtual bool CanRowSiz(int row) = 0; // Return whether the given row can be resized.

    virtual bool CanColMove(int col) = 0; // Return whether the given column can be moved by dragging.
    virtual bool CanRowMove(int row) = 0; // Return whether the given row can be moved by dragging.

    virtual GridSelectionKinds SelKind() = 0;

    virtual void UpdateColWidth(int col, int widthchange); // Called when resizing a column with the mouse. If MouseAction is gmaNone, the function was called when the user released the mouse button.
    virtual void UpdateRowHeight(int row, int heightchange); // Called when resizing a row with the mouse. If MouseAction is gmaNone, the function was called when the user released the mouse button.

    int CEdge(int x, int col, int colwidth, bool first); // Helper for ColumnEdge() and other functions, returning the column whose edge is near the passed x coordinate, when the passed coordinate is relative to the left of the column which is colwidth wide. Set first to true for the first column after the fixed columns. The value of col and colwidth can be -1 in which case the x coordinate is positioned after the last column. -1 is returned when no edge was found.
    int ColumnEdge(int x); // Returns the index of the column whose edge is near the given coordinate. If no columns are near that coordinate, the function returns -1.
    int REdge(int y, int row, int rowheight, bool first); // Helper for RowEdge() and other functions, returning the row whose edge is near the passed y coordinate, when the passed coordinate is relative to the top of the row which is rowheight tall. Set first to true for the first row after the fixed rows. The value of row and rowheight can be -1 in which case the y coordinate is positioned after the last row. -1 is returned when no edge was found.
    int RowEdge(int y); // Returns the index of the row whose edge is near the given coordinate. If no rows are near that coordinate, the function returns -1.

    virtual void DrawHead(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool hovered); // Draw header cell with the given index in the specified rectangle on the canvas.
    virtual void DrawCell(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool selected, bool hovered); // Draw normal cell with the given index in the specified rectangle on the canvas.

    virtual void CancelAction(); // Called when the ESC key is pressed or when the capture focus changes from the grid to another window.

    GridPosition PositionAt(int x, int y); // Returns a detailed description of the position under the specified x,y coordinates.

    GridOptionSet Options();
    void SetOptions(GridOptionSet newoptions);
    Color GridColor();
    void SetGridColor(Color newgridcolor);

    Rect CellRect(int col, int row); // Returns the rectangle of a single cell with the current scroll position. The result is an empty rectangle if the cell is currently not visible.
    Rect ColRect(int col); // Returns the rectangle of a column visible in the client area with the current scroll position. The result is an empty rectangle if the column is not currently visible.
    Rect RowRect(int row); // Returns the rectangle of a row visible in the client area with the current scroll position. The result is an empty rectangle if the row is not currently visible.
    void InvalidateCell(int col, int row);
    void InvalidateCell(const Point &p);
    void InvalidateColumn(int col);
    void InvalidateRow(int row);
    void InvalidateSelected();

    Point CellAt(int x, int y); // Returns the cell under the passed client coordinate or -1 if no cell is found, or if the coordinate is outside the client area.
    
    bool ScrollToCell(int col = -1, int row = -1); // If the passed cell, column or row is not fully visible, the control sets the scroll position to make it show entirely in the window if possible. If the column is wider than what fits the control, its left side is positioned to the left of the control. If only the given row or given column should be fully visible, pass -1 as the other coordinate. Returns whether scrolling was necessary. The function does nothing if the window handle is not created.

    int MouseColEdge();
    int MouseRowEdge();
    Point MouseCell();
    Point MousePos();
    GridMouseActions MouseAction(); // The current action done with the mouse.

    // Change smooth scrolling setting for the x and y axis.
    bool SmoothX();
    void SetSmoothX(bool newsmoothx);
    bool SmoothY();
    void SetSmoothY(bool newsmoothy);

    virtual void OptionsChanged() {}

    virtual ~GridBase() {}
    GridBase(bool smoothx, bool smoothy);
public:
#ifdef DESIGNING
    static void EnumerateProperties(DesignSerializer *serializer);

    virtual bool NeedDesignerHittest(int x, int y, LRESULT hittest);
    virtual Size DesignSize();
#endif
};

/* Sub classes for the CustomGrid designer. */
// Base class of designed columns and rows.
#ifdef DESIGNING
class CustomGrid;
class CustomGridElem : public Object
{
private:
    typedef Object  base;
    
    CustomGrid *owner;
    int index;
    bool GetFixed();

    bool GetResizable();
    void SetResizable(bool newres);
    bool DefaultResizable();
    bool GetVisible();
    void SetVisible(bool newvis);
    bool DefaultVisible();
protected:
    CustomGrid* Owner();

    virtual bool IsFixed() = 0;
    virtual bool IsResizable() = 0;
    virtual void ChangeResizable(bool newres) = 0;
    virtual bool IsDefaultResizable() = 0;
    virtual bool IsVisible() = 0;
    virtual void ChangeVisible(bool newvis) = 0;

    virtual ~CustomGridElem() {}
public:
    static void EnumerateProperties(DesignSerializer *serializer);
    CustomGridElem(CustomGrid *owner, int index);

    int Index();

};

class GridColumn : public CustomGridElem
{
private:
    typedef CustomGridElem  base;
protected:
    virtual bool IsFixed();
    virtual bool IsResizable();
    virtual void ChangeResizable(bool newres);
    virtual bool IsDefaultResizable();
    virtual bool IsVisible();
    virtual void ChangeVisible(bool newvis);

    int GetWidth();
    void SetWidth(int newwidth);
    int DefaultWidth();
    int GridDefaultWidth();
    void SetDefaultWidth(int newdefwidth);
    void ResetWidth();

    virtual ~GridColumn() {}
public:
    static void EnumerateProperties(DesignSerializer *serializer);
    GridColumn(CustomGrid *owner, int index);
};

class GridRow : public CustomGridElem
{
private:
    typedef CustomGridElem  base;
protected:
    virtual bool IsFixed();
    virtual bool IsResizable();
    virtual void ChangeResizable(bool newres);
    virtual bool IsDefaultResizable();
    virtual bool IsVisible();
    virtual void ChangeVisible(bool newvis);

    int GetHeight();
    void SetHeight(int newheight);
    int DefaultHeight();
    int GridDefaultHeight();
    void SetDefaultHeight(int newdefheight);
    void ResetHeight();

    virtual ~GridRow() {}
public:
    static void EnumerateProperties(DesignSerializer *serializer);
    GridRow(CustomGrid *owner, int index);
};
#endif

/* Base class for most grid controls. Allows sizing, changing the order of and showing/hiding rows and columns. */ 
class CustomGrid : public GridBase
{
private:
    typedef GridBase    base;

    int fixcols;
    int fixrows;
    int cols;
    int rows;

    int colw;
    int rowh;
    int fillw;
    int fillh;
    sparse_list<int> colwlist;
    sparse_list<int> rowhlist;
    sparse_list<int> defcolwlist;
    sparse_list<int> defrowhlist;

    bool colsiz; // The columns are resizable by default.
    bool rowsiz; // The rows are resizable by default.
    sparse_list<bool> colsizlist;
    sparse_list<bool> rowsizlist;

    sparse_list<bool> colvislist;
    sparse_list<bool> rowvislist;

    bool coldrag; // The columns can be drag-moved.
    bool rowdrag; // The rows can be drag-moved.

    class OrderList
    {
        struct OrderData
        {
            int pos;
            int cnt;
            int hidden;
            OrderData *next;
        };

        OrderData *first;
        OrderData *last;
        int cnt;

        void FreeAll();
        int HiddenBetween(int pos1, int pos2, const sparse_list<bool> &vis);
        OrderData* Next(OrderData *prev); // Returns 'first' if 'prev' is NULL, otherwise returns prev->next.
        bool Contains(OrderData *dat, int index); // Returns true if the data block contains index. That is, dat->pos <= index and dat->pos + dat->cnt > index.
    public:
        OrderList();
        ~OrderList();
        OrderList(OrderList &&other) noexcept;

        OrderList& operator=(OrderList &&other) noexcept;
        OrderList& operator=(const OrderList &other);

        void resize(int newsize, const sparse_list<bool> &vis);

        int pos_of(int index, bool findhidden, const sparse_list<bool> &vis);
        int which_at(int index, const sparse_list<bool> &vis);
        void move_from_to(int index, int to, const sparse_list<bool> &vis);
        void remove(int index, const sparse_list<bool> &vis);
        void insert(int index);
    };
    
    OrderList colorder;
    OrderList roworder;

    std::list<Point*> updatept; // A col/row coordinate that will be updated when deleting, inserting etc. columns and rows to point to the same place after the change, or set to (-1,-1) if the same cell was removed.

    Point dragpos; // Position of the mouse within a fixed cell being dragged.
    LayeredWindow *dragwin; // Window that displays a drag image when moving columns or rows.

    GridSelectionKinds selkind; // Whether a cell, a whole row or nothing is shown as selected.
    Point pos; // Current selection. When a whole row is selected, pos.x should be ignored.

    void CreateDragWindow();
protected:
    virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
    virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
    virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

    virtual void Navigate(WORD &keycode, VirtualKeyStateSet vkeys);

    virtual int HorzSiz(); // The size of all grid cells horizontally, including the fixed columns. If smooth horizontal scrolling is not active, the function is not called.
    virtual int VertSiz(); // The size of all grid cells vertically, including the fixed rows. If smooth vertical scrolling is not active, the function is not called.

    virtual int FixColCnt(); // Number of fixed/header columns.
    virtual int FixRowCnt(); // Number of fixed/header rows.
    virtual int ColCnt(); // Number of columns shown in the grid.
    virtual int RowCnt(); // Number of rows shown in the grid.

    virtual int ColW(int col); // Width of the specified column. Passed index of the column can be -1, in which case the result must be a general column width used in drawing lines after the last column. In that case if the result is 0 or less, there won't be general column lines drawn.
    virtual int RowH(int row); // Height of the specified row. Passed index of the row can be -1, in which case the result must be a general row height used in drawing lines after the last row. In that case if the result is 0 or less, there won't be general row lines drawn.

    virtual bool CanColSiz(int col); // Return whether the given column can be resized.
    virtual bool CanRowSiz(int row); // Return whether the given row can be resized.

    virtual bool CanColMove(int col); // Return whether the given column can be moved by dragging.
    virtual bool CanRowMove(int row); // Return whether the given row can be moved by dragging.

    virtual GridSelectionKinds SelKind();
    bool IsSel(int col, int row);

    virtual void UpdateColWidth(int col, int widthchange); // Called when resizing a column with the mouse. If MouseAction is gmaNone, the function was called when the user released the mouse button.
    virtual void UpdateRowHeight(int row, int heightchange); // Called when resizing a row with the mouse. If MouseAction is gmaNone, the function was called when the user released the mouse button.

    virtual void CancelAction(); // Called when the ESC key is pressed or when the capture focus changes from the grid to another window.

    virtual void OptionsChanged();

    bool ColLeft(const Rect &clientrect, int col, int &colleft, bool *first = NULL); // Returns the left coordinate of the passed column in colleft if column is visible. Returns true if column is visible. Fixed columns count as visible even if they don't fit the current window size.
    int ColAt(const Rect &clientrect, int x, bool *first = NULL, int *colleft = NULL, int *colwidth = NULL); // Returns the column under the given x coordinate and sets first to true if the returned column is the first visible non fixed. The result is -1 when the coordinate is outside the client area or -2 if the coordinate is to the right of the last column. When colleft is set, it receives the left x coordinate of the returned column, or the left of the area to the right of the last column when the result is -2. When colwidth is set, it receives the width of the column or -1 when there is no column at the given coordinate.
    bool RowTop(const Rect &clientrect, int row, int &rowtop, bool *first = NULL); // Sets rowtop to the top coordinate of the passed row if row is visible. Returns true if row is visible. Fixed rows count as visible even if they don't fit the current window size.
    int RowAt(const Rect &clientrect, int y, bool *first = NULL, int *rowtop = NULL, int *rowheight = NULL); // Returns the row under the given y coordinate and sets first to true if the returned row is the first visible non fixed. The result is -1 when the coordinate is outside the client area or -2 if the coordinate is below the last row. When rowtop is set, it receives the top y coordinate of the returned row, or the top of the area below of the last row when the result is -2. When rowheight is set, it receives the height of the row or -1 when there is no row at the given coordinate.

    void PushUpdatePoint(Point *pt);
    void PopUpdatePoint();

    virtual Point SelectedCell();

    virtual void CountChanged(int col, int row, bool deleted); // Called when the visible count of columns or rows have changed, including change in the fixed cells. Updates scrollbars and the currently selected cell position.
    virtual void CountChanging(int col, int row, bool deleting); // Called before the count is changed in the grid. The arguments are in GridBase coordinates.

    CustomGrid();
public:
#ifdef DESIGNING
    static void EnumerateProperties(DesignSerializer *serializer);

    struct SaveData
    {
        int fixcnt;
        int cnt;
        sparse_list<int> whlist;
        sparse_list<int> defwhlist;
        sparse_list<bool> sizlist;
        sparse_list<bool> vislist;
        OrderList order;
    } *savedata;

    virtual void SaveColumns();
    virtual void SaveRows();
    virtual void RestoreColumns();
    virtual void RestoreRows();
    virtual void FreeSaved();

    void ColWidthIndexes(std::vector<int> &indexes);
    void RowHeightIndexes(std::vector<int> &indexes);
    void DefColWidthIndexes(std::vector<int> &indexes);
    void DefRowHeightIndexes(std::vector<int> &indexes);
    void ColVisibleIndexes(std::vector<int> &indexes);
    void RowVisibleIndexes(std::vector<int> &indexes);
    void ColResizeIndexes(std::vector<int> &indexes);
    void RowResizeIndexes(std::vector<int> &indexes);
#endif
    GridSelectionKinds SelectionKind();
    void SetSelectionKind(GridSelectionKinds newselkind);

    virtual void DeleteColumn(int col);
    virtual void InsertColumn(int before);
    virtual void DeleteRow(int row);
    virtual void InsertRow(int before);

    bool SmoothHorzScroll();
    void SetSmoothHorzScroll(bool newsmoothscroll);
    bool SmoothVertScroll();
    void SetSmoothVertScroll(bool newsmoothscroll);

    using base::MouseAction;
    using base::InvalidateSelected;

    Rect CellRect(int col, int row); // Returns the rectangle of a single cell with the current scroll position. The result is an empty rectangle if the cell is currently not visible.
    Rect ColRect(int col); // Returns the rectangle of a column visible in the client area with the current scroll position. The result is an empty rectangle if the column is not currently visible.
    Rect RowRect(int row); // Returns the rectangle of a row visible in the client area with the current scroll position. The result is an empty rectangle if the row is not currently visible.
    Point CellAt(int x, int y); // Returns the cell under the passed client coordinate or -1 if no cell is found, or if the coordinate is outside the client area.

    bool ScrollToCell(int col = -1, int row = -1); // If the passed cell, column or row is not fully visible, the control sets the scroll position to make it show entirely in the window if possible. If the column is wider than what fits the control, its left side is positioned to the left of the control. If only the given row or given column should be fully visible, pass -1 as the other coordinate. Returns whether scrolling was necessary.

    void InvalidateCell(int col, int row);
    void InvalidateCell(const Point &p);
    void InvalidateColumn(int col);
    void InvalidateRow(int row);

    int ColCount() const;
    virtual void SetColCount(int newcolcnt);
    int RowCount() const;
    virtual void SetRowCount(int newrowcnt);
    int FixedColCount() const;
    virtual void SetFixedColCount(int newfixcolcnt);
    int FixedRowCount() const;
    virtual void SetFixedRowCount(int newfixrowcnt);

    bool ColResize(int col);
    void SetColResize(int col, bool newres);
    bool ColumnsResizable();
    void SetColumnsResizable(bool newres);
    bool RowResize(int row);
    void SetRowResize(int row, bool newres);
    bool RowsResizable();
    void SetRowsResizable(bool newres);

    bool ColVisible(int col);
    virtual void SetColVisible(int col, bool newvis);
    bool RowVisible(int row);
    virtual void SetRowVisible(int row, bool newvis);

    int ColPos(int col, bool findhidden = false); // Returns the current place of the column of index 'col'. If the column is hidden and findhidden is false, returns -1, otherwise returns the position of the next visible column. (Real -> Current order)
    int RowPos(int row, bool findhidden = false); // Returns the current place of the row of index 'row'. If the row is hidden and findhidden is false, returns -1, otherwise returns the position of the next visible row. (Real -> Current order)
    int WhichCol(int pos); // Returns which column is at the given position. (Current order -> Real)
    int WhichRow(int pos); // Returns which row is at the given position. (Current order -> Real)

    GridPosition PositionAt(int x, int y); // Returns a detailed description of the position under the specified (x, y) coordinates.

    void MoveCol(int col, int before); // Changes the position of a column to be placed in front of another. 'col' is the real position of the moved column, 'before' is the current position of the destination column.
    void MoveRow(int row, int before); // Changes the position of a row to be placed in front of another. 'row' is the real position of the moved row, 'before' is the current position of the destination row.

    int ColWidth(int col);
    void SetColWidth(int col, int newwidth);
    int DefColWidth(int col);
    int FillColWidth();
    void SetFillColWidth(int newwidth);
    void SetDefColWidth(int col, int newwidth);
    int DefaultColWidth();
    void SetDefaultColWidth(int newwidth);
    void ResetColWidth(int col);

    int RowHeight(int row);
    void SetRowHeight(int row, int newheight);
    int DefRowHeight(int row);
    int FillRowHeight();
    void SetFillRowHeight(int newheight);
    void SetDefRowHeight(int row, int newheight);
    int DefaultRowHeight();
    void SetDefaultRowHeight(int newheight);
    void ResetRowHeight(int row);

    bool ColumnDrag();
    void SetColumnDrag(bool newdrag);
    bool RowDrag();
    void SetRowDrag(bool newdrag);

    int MouseColEdge(); // Updated to indicate the real column edge, even for reordered columns.
    int MouseRowEdge(); // Updated to indicate the real row edge, even for reordered rows.
    Point MouseCell(); // The current cell under the mouse. Not updated while the mouse button is pressed.

    Point Selected(); // Returns the currently selected cell.
    void SetSelected(int col, int row); // Selects the specified cell if the selection kind is not gksNoSelect. If the selection kind is gskColSelect or gskRowSelect, the meaningless coordinate is ignored.

    AllowColumnRowEvent OnBeginColumnResize; // Event called when the mouse moves over a column edge and the column is resizable. When the allow parameter is set false, the cursor won't change to resize.
    ColumnRowSizeEvent OnColumnSizing; // Called every time the size of the column is about to be changed due to sizing it with the mouse. The size can be changed to a different value before.
    ColumnRowSizeEvent OnColumnSized; // Called when sizing the column has finished by releasing the mouse button or switching to another window in the system. The OnColumnSizing event is called before OnColumnSized, so if the size is updated during sizing, it is not necessary to change the size in OnColumnSized.
    AllowColumnRowEvent OnBeginColumnDrag; // Called when the mouse was clicked on a column and the cursor was moved enough to start a drag operation. Set the allow parameter to false to prevent the column to be moved.
    AllowColumnRowEvent OnBeginRowResize; // Event called when the mouse moves over a row edge and the row is resizable. When the allow parameter is set false, the cursor won't change to resize.
    ColumnRowSizeEvent OnRowSizing; // Called every time the size of the row is about to be changed due to sizing it with the mouse. The size can be changed to a different value before.
    ColumnRowSizeEvent OnRowSized; // Called when sizing the row has finished by releasing the mouse button or switching to another window in the system. The OnRowSizing event is called before OnRowSized, so if the size is updated during sizing, it is not necessary to change the size in OnRowSized.
    AllowColumnRowEvent OnBeginRowDrag; // Called when the mouse was clicked on a row and the cursor was moved enough to start a drag operation. Set the allow parameter to false to prevent the row to be moved.
};

class ControlEdit;
class StringGrid : public CustomGrid
{
private:
    typedef CustomGrid  base;

    sparse_list<sparse_list<std::pair<std::wstring, void*>>> strings;

    ControlEdit *editor; // The editor in the control to allow the user to update the text of cells.
    Point editcell; // The cell being edited when the editor is visible.
    bool autoedit; // Whether the editor should be automatically shown when the user clicks inside a cell.
    bool tabedit; // When the editor is open, pressing the TAB key goes to the next cell.

    void CreateEditor();

    void editorkeydown(void *sender, KeyParameters param);
    void editorkeypress(void *sender, KeyPressParameters param);
    void editorkeypush(void *sender, KeyPushParameters param);
    void editorkeyup(void *sender, KeyParameters param);
    void editorgainfocus(void *sender, FocusChangedParameters param);
    void editorlosefocus(void *sender, FocusChangedParameters param);
    void editorenter(void *sender, ActiveChangedParameters param);
    void editorleave(void *sender, ActiveChangedParameters param);
    void editordlgcode(void *sender, DialogCodeParameters param);
protected:
    virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
    virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
    virtual void GainFocus(HWND otherwindow);
    virtual void LoseFocus(HWND otherwindow);
    virtual void ActiveEnter(Control *other);
    virtual void ActiveLeave(Control *other);

    virtual void Scrolled(ScrollbarKind kind, int oldpos, int pos, ScrollCode code);

    bool StringSet(int col, int row);

    virtual void DrawHead(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool hovered);
    virtual void DrawCell(Canvas *c, int col, int row, const Rect &r, const Rect &clip, bool selected, bool hovered);

    virtual Point SelectedCell();

    virtual void CountChanging(int col, int row, bool deleting);
    virtual void CountChanged(int col, int row, bool deleted); // Called when the visible count of columns or rows have changed, including change in the fixed cells. The arguments are in GridBase coordinates.
public:
#ifdef DESIGNING
    static void EnumerateProperties(DesignSerializer *serializer);
#endif
    StringGrid();

    using base::Options;
    using base::SetOptions;
    //using base::SelectionKind;
    //using base::SetSelectionKind;

    std::wstring String(int col, int row);
    std::pair<std::wstring, void*> StringAndData(int col, int row);
    void SetString(int col, int row, const std::wstring &str);
    void SetString(int col, int row, std::wstring &&str);
    void SetStringAndData(int col, int row, const std::wstring &str, void *data);
    void SetStringAndData(int col, int row, std::wstring &&str, void *data);
    void* Data(int col, int row);
    void SetData(int col, int row, void *data);

    virtual void DeleteColumn(int col);
    virtual void InsertColumn(int before);
    virtual void DeleteRow(int row);
    virtual void InsertRow(int before);

    virtual void SetColCount(int newcolcnt);
    virtual void SetRowCount(int newrowcnt);

    virtual void SetColVisible(int col, bool newvis);
    virtual void SetRowVisible(int row, bool newvis);

    virtual void SwapRowData(int a, int b);
    virtual void SwapColData(int a, int b);

    bool Editing(); // Returns true if the editor is visible in the grid.
    void EditCell(int col, int row); // Selects the given row or cell in the grid and opens the editor for it.
    Point CellEdited(); // The cell currently edited, or Point(-1, -1) if the editor is not visible.
    void EndEditCell(bool cancel); // Call to hide the editor and call the edit events. When cancel is false, OnCellEdited will be called like the user pressed enter in the editor.
    std::wstring EditorText(); // The text currently in the editor when a cell is being edited, or an empty string.
    void SetEditorText(const std::wstring &text); // Update the text in the editor when a cell is being edited.
    void EditorSelStartAndLength(int &selstart, int &sellen); // Sets the passed values to the current cursor position and selection in the editor, when a cell is being edited. Otherwise the values are not changed.
    void SetEditorSelStartAndLength(int selstart, int sellen); // Update the current cursor position and selection in the editor, when a cell is being edited.

    bool AutoEdit();
    void SetAutoEdit(bool newautoedit);

    bool TabEdited();
    void SetTabEdited(bool newtabedit);

    virtual bool Focused();
    virtual void Focus();

    BeginCellEditEvent OnBeginCellEdit;
    CellEditedEvent OnCellEdited;
    EndCellEditEvent OnEndCellEdit;

    KeyEvent OnEditorKeyDown;
    KeyEvent OnEditorKeyUp;
    KeyPushEvent OnEditorKeyPush;
    KeyPressEvent OnEditorKeyPress;
    DialogCodeEvent OnEditorDialogCode;

    GridCellAlignmentEvent OnCellTextAlign;
};


}
/* End of NLIBNS */

