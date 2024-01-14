#pragma once


namespace NLIBNS
{


    enum ThemeClasses { tcButton, tcClock, tcCombobox, tcCommunications, tcControlPanel, tcDatePicker,
                        tcDragdrop, tcEdit, tcExplorerBar, tcFlyout, tcGlobals, tcHeader, tcListbox,
                        tcListView, tcMenu, tcMenuband, tcNavigation, tcPage, tcProgress, tcRebar,
                        tcScrollbar, tcSearchEditbox, tcSpin, tcStartPanel, tcStatus, tcTab, tcTaskband,
                        tcTaskbar, tcTaskDialog, tcTextStyle, tcToolbar, tcTooltip, tcTrackbar,
                        tcTrayNotify, tcTreeview, tcWindow, tcExplorerListview, tcExplorerTreeview, tcThemeClassMax };

    typedef HANDLE HTHEME; // same as definition in Uxtheme.h

    // States for most themed buttons. Not all parts are valid for all button types.
    enum ThemeButtonStates { tbsNormal, tbsHot, tbsChecked, tbsHotChecked, tbsPressed, tbsDefaulted, tbsDisabled };
    enum ThemeToolbarStates { ttsNormal, ttsHot, ttsOtherSideHot, ttsChecked, ttsHotChecked, ttsNearHot, ttsPressed, ttsDefaulted, ttsDisabled };
    enum ThemeCheckboxStates { cbsCheckedDisabled, cbsCheckedHot, cbsCheckedNormal, cbsCheckedPressed, cbsMixedDisabled, cbsMixedHot, cbsMixedNormal, cbsMixedPressed, cbsUncheckedDisabled, cbsUncheckedHot, cbsUncheckedNormal, cbsUncheckedPressed };
    enum ThemeTreeGlyphStates { ttgsOpen, ttgsClosed, /* Only for tcExplorerTreeview: */ ttgsHotOpen, ttgsHotClosed };
    enum ThemeBorderDrawStates { tbdsNormal, tbdsHot, tbdsFocused, tbdsDisabled };
    enum ThemeHeaderItemStates { thisNormal, thisHot, thisPressed, thisSortedNormal, thisSortedHot, thisSortedPressed, thisIconNormal, thisIconHot, thisIconPressed, thisIconSortedNormal, thisIconSortedHot, thisIconSortedPressed };
    enum ThemeMenubarItemStates { tmbisDisabled, tmbisDisabledHot, tmbisDisabledPushed, tmbisHot, tmbisNormal, tmbisPushed };
    enum ThemeMenuItemStates { tmisDisabled, tmisDisabledHot, tmisHot, tmisNormal };
    enum ThemeMenuCheckBackgroundStates { tmcbgBitmap, tmcbgDisabled, tmcbgNormal };
    enum ThemeMenuCheckStates { tmcBulletDisabled, tmcBulletNormal, tmcCheckmarkDisabled, tmcCheckmarkNormal };
    enum ThemeSubMenuGlyphStates { tsmgNormal, tsmgDisabled };
    enum ThemeWindowCloseButtonStates { twcbDisabled, twcbHot, twcbNormal, twcbPushed  };
    enum ThemeListViewItemStates { tlviDisabled, tlviHot, tlviHotSelected, tlviNormal, tlviSelected, tlviSelectedNotFocused };
    enum ThemeTrackbarThumbStates { tttsNormal, tttsFocused, tttsHot, tttsDisabled, tttsPressed };

    enum ThemeTrackbarThumbs { tttUp, tttDown, tttLeft, tttRight, tttHorizontal, tttVertical };

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum TextDrawOptions : int;
#else
    enum TextDrawOptions;
#endif
    typedef uintset<TextDrawOptions> TextDrawOptionSet;

    struct ThemePopupMenuSizes
    {
        MARGINS baritemmargin; // Margins around an item in the main menu bar.
        Size popupbordersiz; // Border of the whole popup menu.

        int bartextheight; // Height of a generic text in the menu bar.
        int itemtextheight; // Height of a generic text in menu items.

        union
        {
            // Values used when themeing is enabled.
            struct
            {
                SIZE popcheck; // Size of the checkbox glyph, without padding border.
                MARGINS popcheckmargin; // Size of the checkbox padding border.
                SIZE popsep; // (Mostly) Height of the separator lines without padding border.
                MARGINS popitemmargin; // Top/Bottom: margin above and below a line of separator or text. Left/Right: margin left and right of the whole item (but is usually 0). The gutter and background is drawn outside this margin while the selection rectangle is inside.
                int popitembordersiz; // Text margin to the right.
                int popitembgbordersiz; // Text margin to the left.
                MARGINS popcheckbgmargin; // Margin around the checkbox. Top and bottom distances from other menu items, left from left side of the popup menu, right from the text. The gutter is around the checkbox with this margin.

                int popitemtabwidth; // Size of the tab between the menu text and its shortcut.

                SIZE popupsubmnusiz; // Sub menu triangle glyph size.
                MARGINS popupsubmnucontent; // Sub menu triangle glyph padding borders.
            };

            // Values for non-themed menus.
            struct
            {
                int rowheight; // Size of a single row in the popup menu minus the margin.
                int rowmargin; // Top and bottom margin of a single row.
                int sepheight; // Height of a separator item.
                int iconsize; // Size for the icon, this place occupies the checkbox and the bullet as well
                int icontextdist; // Distance between the text and the icon.
                SIZE checksize; // Size of the check/bullet bitmap.
                int textmargin; // Space to the right side of the longest text.
                int itemmargin; // Space at the right side of the menu or shortcut text to border of the menu.
                int shortcutmargin; // Space between the left side of the longest shortcut text and the text, but only when shortcut text is present.
                int subsize; // Height and width of the sub menu triangle glyph.
                int submargin; // Distance between the right side of the sub triangle and the popup menu border.
            };
        };
    };

    class Canvas;
    enum ThemePartSizes { tpsMin = 0, tpsTrue = 1, tpsDraw = 2 };
    class Themes
    {
        static Themes *instance;

        ThemePopupMenuSizes sizes;

        HTHEME handles[tcThemeClassMax];

        Themes();
        Themes(const Themes &copy) { }
        ~Themes();

        bool isthemed; // Reflects the user set value of SetThemed but not the real state of the system or the application. True by default. Own drawing functions use this setting only.

        void ThemesChanged();
        void CloseThemes();

        void DrawGenTreeviewGlyph(Canvas *canvas, const Rect &r, ThemeTreeGlyphStates glyphstate, bool usevista);
        Size MeasureGenTreeviewGlyph(ThemeTreeGlyphStates glyphstate, bool usevista);

        friend class Application;
    public:
        static Themes* GetInstance();
        static void FreeInstance();

        std::wstring ThemeClassname(ThemeClasses themeclass);
        HTHEME ThemeHandle(ThemeClasses themeclass);

        bool ThemesEnabled(); // Returns true if themes are enabled in the system. The return value is true even if theming is turned off for this particular application. If you would like to know whether the application is themed as well, call AppThemed().
        bool AppThemed(); // Returns true if themes are enabled in the system and the app is themed. Setting themed to false will cause this to return false as well.
        void SetThemed(bool newthemed); // Sets App themed to true or false if themes are enabled in the system.

        void SetWindowTheme(HWND controlhandle, const wchar_t *pszSubAppName, const wchar_t *pszSubIdList);

        bool IsPopupMenuItemTransparent(ThemeMenuItemStates tate);
        bool IsWindowCloseButtonTransparent(ThemeWindowCloseButtonStates state);

        // Erasing function
        void DrawParentBackground(HWND controlhandle, Canvas *canvas, const Rect &r);
        //void DrawParentBackground(HWND controlhandle, HDC dc, const Rect &r);

        // Drawing functions. Works even if theming is turned off in the system. Uses other methods in that case.
        void DrawButton(Canvas *canvas, const Rect &r, ThemeButtonStates state);
        void DrawCheckbox(Canvas *canvas, const Rect &r, ThemeCheckboxStates state);
        void DrawToolbarButton(Canvas *canvas, const Rect &r, ThemeToolbarStates state);
        void DrawToolbarDropdownButton(Canvas *canvas, const Rect &r, ThemeToolbarStates state);
        void DrawToolbarDropdownButtonGlyph(Canvas *canvas, const Rect &r, ThemeToolbarStates state);
        void DrawToolbarSplitbutton(Canvas *canvas, const Rect &r, ThemeToolbarStates state);
        void DrawToolbarSplitbuttonDropdown(Canvas *canvas, const Rect &r, ThemeToolbarStates state);
        void DrawToolbarSeparator(Canvas *canvas, const Rect &r, ThemeToolbarStates state);
        void DrawToolbarSeparatorVert(Canvas *canvas, const Rect &r, ThemeToolbarStates state);
        void DrawControlBorder(Canvas *canvas, const Rect &r);
        void DrawEditBorder(/*HWND controlhandle,*/ Canvas *canvas, const Rect &r, ThemeBorderDrawStates borderstate);
        void DrawTreeviewGlyph(Canvas *canvas, const Rect &r, ThemeTreeGlyphStates glyphstate);
        void DrawVistaTreeviewGlyph(Canvas *canvas, const Rect &r, ThemeTreeGlyphStates glyphstate); // This is safe for XP as well, but uses Vista styled drawing on the appropriate system.
        void DrawHeaderItem(Canvas *canvas, const Rect &r, ThemeHeaderItemStates state);
        void DrawHeaderItemClip(Canvas *canvas, const Rect &r, ThemeHeaderItemStates state, const Rect &clip);
        void DrawHeaderRightSide(Canvas *canvas, const Rect &r);
        void DrawMenubarBackground(Canvas *canvas, const Rect &r, bool active);
        void DrawMenubarItemHighlight(Canvas *canvas, const Rect &r, ThemeMenubarItemStates itemstate);
        void DrawPopupMenuBackground(Canvas *canvas, const Rect &r);
        void DrawPopupMenuItemBackground(Canvas *canvas, const Rect &r);
        void DrawPopupMenuItemHighlight(Canvas *canvas, const Rect &r, ThemeMenuItemStates itemstate);
        void DrawPopupMenuItemGutter(Canvas *canvas, const Rect &r);
        void DrawPopupMenuCheckBackground(Canvas *canvas, const Rect &r, ThemeMenuCheckBackgroundStates state);
        void DrawPopupMenuCheck(Canvas *canvas, const Rect &r, ThemeMenuItemStates mistate, ThemeMenuCheckStates state);
        void DrawSubMenuGlyph(Canvas *canvas, const Rect &r, ThemeMenuItemStates mistate, ThemeSubMenuGlyphStates state);
        void DrawPopupMenuSeparator(Canvas *canvas, const Rect &r);
        void DrawTabPane(Canvas *canvas, const Rect &r);

        void DrawMenubarItem(Canvas *c, const Rect &r, const std::wstring& text, ThemeMenubarItemStates state, int imagewidth);
        void DrawPopupMenuItem(Canvas *c, const Rect &r, const std::wstring& text, const std::wstring& shortcut, ThemeMenuItemStates state, bool checked, bool grouped, bool hassubitem, int imagewidth, int imageheight, int longestshortcutw);
        void DrawPopupMenuSeparatorItem(Canvas *c, const Rect &r, ThemeMenuItemStates state, int imagewidth);

        void DrawVistaListViewItemClipped(Canvas *c, const Rect &r, ThemeListViewItemStates state, const Rect &clip);
        void DrawVistaListViewItem(Canvas *c, const Rect &r, ThemeListViewItemStates state);

        // Returns the position where an image would be drawn for a menu item, if it had the given rectangle and the image has the given sizes.
        Point MenubarImagePosition(const Rect &r, int imageheight);
        Point PopupMenuItemImagePosition(const Rect &r, int imagewidth, int imageheight);

        //void DrawWindowCloseButton(Canvas *canvas, const Rect &r, ThemeWindowCloseButtonStates state);
        bool MenuThemed();
        bool VistaListViewThemed();

        // Measurements of different themed elements.
        Size MeasureTreeviewGlyph(ThemeTreeGlyphStates glyphstate);
        Size MeasureVistaTreeviewGlyph(ThemeTreeGlyphStates glyphstate); // This is safe for XP as well, but uses Vista styled measuring on the appropriate system.

        Size MeasureEditBorderWidth();

        Rect MeasureButtonBackgroundArea(const Rect &clientrect); // Returns the area of the button entirely covered when it is drawn to the clientrect rectangle.
        Rect MeasureButtonFocusRectangle(const Rect &clientrect); // Returns the rectangle of the focus rectangle for focused buttons.

        Size MeasureHeaderItemPartSize(ThemePartSizes partsize, ThemeHeaderItemStates state);

        Size MeasureToolbarDropButtonGlyph();

        void DrawTrackbarThumb(Canvas *canvas, const Rect &r, ThemeTrackbarThumbs thumb, ThemeTrackbarThumbStates state);
        void DrawTrackbarTrack(Canvas *canvas, const Rect &r, bool vertical);
        Size MeasureTrackburThumb(const Rect &r, ThemeTrackbarThumbs thumb);
        Size MeasureTrackburThumbDefaultSize(ThemeTrackbarThumbs thumb);

        const ThemePopupMenuSizes& MenuSizes();

        Size MeasureMenubarTextExtent(Canvas *canvas, const std::wstring& str, TextDrawOptionSet options);
        Size MeasurePopupMenuItemTextExtent(Canvas *canvas, const std::wstring& str, TextDrawOptionSet options);
        void DrawMenubarText(Canvas *canvas, const Rect &r, const std::wstring& str, TextDrawOptionSet options, ThemeMenubarItemStates state);
        void DrawPopupMenuItemText(Canvas *canvas, const Rect &r, const std::wstring& str, TextDrawOptionSet options, ThemeMenuItemStates state);

        Size MeasureMenubarItem(int imagewidth);
        Size MeasurePopupMenuItem(int imageheight, int imagewidth);
        Size MeasurePopupMenuSeparator(int imagewidth);

        void TestMeasure(ThemeClasses tclass, int partid, int stateid, const Rect &r = Rect(0, 0, 100, 100));
    };

    extern Themes *themes;


}
/* End of NLIBNS */

