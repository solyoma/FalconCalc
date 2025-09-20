#pragma once


namespace NLIBNS
{


enum ClassStyles { csByteAlignClient = 0x1000, csByteAlignWindow = 0x2000, csClassDC = 0x40, csDoubleClicks = 0x8,
                   csDropShadow = 0x20000, csGlobalClass = 0x4000, csHRedraw = 0x2, csNoClose = 0x200,
                   csOwnDC = 0x20, csParentDC = 0x80, csSaveBits = 0x800, csVRedraw = 0x1
                 };

enum WindowStyles { wsBorder = 0x00800000L, wsCaption = 0x00C00000L /* == wsDlgFrame && wsBorder */, wsChild = 0x40000000L,
                    wsClipChildren = 0x02000000L, wsClipSiblings = 0x04000000L, wsDisabled = 0x08000000L,
                    wsDlgFrame = 0x00400000L, wsGroup = 0x00020000L /* == wsMinimizeBox. Check wsChild! */  , wsHScroll = 0x00100000L, 
                    wsMaximize = 0x01000000L, wsMaximizeBox = 0x00010000L, wsMinimize = 0x20000000L, wsMinimizeBox = 0x00020000L,
                    wsOverlapped = 0x00000000L, wsPopup = 0x80000000L, wsSysMenu = 0x00080000L,
                    wsTabStop = 0x00010000L /* == wsMaximizeBox. Check wsChild! */ , wsThickFrame = 0x00040000L,
                    wsVisible = 0x10000000L, wsVScroll = 0x00200000L,

                    // Common control styles:

                    ccsAdjustable = 0x00000020L, ccsBottom = 0x00000003L, ccsLeft = 0x00000081L /* ccsTop | ccsVert */, ccsNoDivider = 0x00000040L,
                    ccsNoMoveX = 0x00000082L /* ccsNoMoveY | ccsNoVert */, ccsNoMoveY = 0x00000002L, ccsNoParentAlign = 0x00000008L, ccsNoResize = 0x00000004L,
                    ccsRight = 0x00000083L /* ccsBottom | ccsVert */, ccsTop = 0x00000001L, ccsVert = 0x00000080L,

                    // Removed:
                    /* wsIconic = 0x20000000L because == wsMinimize */
                    /* wsSizeBox = 0x00040000L because == wsThickFrame */
                    /* wsTiled = 0x00000000L because == wsOverlapped and zero */

                    // for buttons:
                    bsPushbutton = 0x00000000L, bsDefPushbutton = 0x00000001L, bsCheckbox = 0x00000002L, bsAutoCheckbox = 0x00000003L, 
                    bsRadiobutton = 0x00000004L, bs3State = 0x00000005L, bsAuto3State = 0x00000006L, bsGroupbox = 0x00000007L, 
                    bsUserButton = 0x00000008L, bsAutoRadiobutton = 0x00000009L, bsPushbox = 0x0000000AL, bsOwnerdraw = 0x0000000BL, 
                    bsTypemask = 0x0000000FL, bsLeftText = 0x00000020L, bsText = 0x00000000L, bsIcon = 0x00000040L, 
                    bsBitmap = 0x00000080L, bsLeft = 0x00000100L, bsRight = 0x00000200L, bsCenter = 0x00000300L, 
                    bsTop = 0x00000400L, bsBottom = 0x00000800L, bsVCenter = 0x00000C00L, bsPushlike = 0x00001000L, 
                    bsMultiline = 0x00002000L, bsNotify = 0x00004000L, bsFlat = 0x00008000L, bsRightButton = bsLeftText, 

                    // for edit boxes:
                    esLeft = 0x0000L, esCenter = 0x0001L, esRight = 0x0002L, esMultiline = 0x0004L,
                    esUppercase = 0x0008L, esLowercase = 0x0010L, esPassword = 0x0020L, esAutoVScroll = 0x0040L,
                    esAutoHScroll = 0x0080L, esNoHideSel = 0x0100L, esOEMConvert = 0x0400L, esReadonly = 0x0800L,
                    esWantReturn = 0x1000L, esNumber = 0x2000L,

                    // for progress bars:
                    /* not supported for themes: */ pbsSmooth = 0x01,
                    pbsVertical = 0x04, pbsMarquee = 0x08,
                    /* Vista and above: */ pbsSmoothReverse = 0x10,

                    // for updown controls:
                    udsAlignLeft = 0x0008, udsAlignRight = 0x0004, udsArrowKeys = 0x0020, udsAutoBuddy = 0x0010,
                    udsHorizontal = 0x0040, udsHotTrack = 0x0100, udsNoThousands = 0x0080, udsSetBuddyint = 0x0002,
                    udsWrap = 0x0001,

                    // for static controls:
                    ssLeft = 0x00000000L, ssCenter = 0x00000001L, ssRight = 0x00000002L, ssIcon = 0x00000003L,
                    ssBlackRect = 0x00000004L, ssGrayRect = 0x00000005L, ssWhiteRect = 0x00000006L, ssBlackFrame = 0x00000007L,
                    ssGrayFrame = 0x00000008L, ssWhiteFrame = 0x00000009L, ssUserItem = 0x0000000AL, ssSimple = 0x0000000BL,
                    ssLeftNoWordwrap = 0x0000000CL, ssOwnerDraw = 0x0000000DL, ssBitmap = 0x0000000EL, ssEnhMetafile = 0x0000000FL,
                    ssEtchedHorz = 0x00000010L, ssEtchedVert = 0x00000011L, ssEtchedFrame = 0x00000012L, ssTypemask = 0x0000001FL,
                    ssRealsizeControl = 0x00000040L, ssNoPrefix = 0x00000080L, ssNotify = 0x00000100L, ssCenterImage = 0x00000200L,
                    ssRightJust = 0x00000400L, ssRealsizeImage = 0x00000800L, ssSunken = 0x00001000L, ssEditControl = 0x00002000L,
                    ssEndEllipsis = 0x00004000L, ssPathEllipsis = 0x00008000L, ssWordEllipsis = 0x0000C000L, ssEllipsisMask = 0x0000C000L,

                    // for listboxes:
                    lbsNotify = 0x0001L, lbsSort = 0x0002L, lbsNoRedraw = 0x0004L, lbsMultipleSel = 0x0008L, lbsOwnerdrawFixed = 0x0010L,
                    lbsOwnerdrawVariable = 0x0020L, lbsHasStrings = 0x0040L, lbsUseTabstops = 0x0080L, lbsNoIntegralHeight = 0x0100L,
                    lbsMulticolumn = 0x0200L, lbsWantKeyboardInput = 0x0400L, lbsExtendedSel = 0x0800L, lbsDisableNoScroll = 0x1000L,
                    lbsNodata = 0x2000L, lbsNosel = 0x4000L, lbsCombobox = 0x8000L,

                    // for comboboxes:
                    cbsSimple = 0x0001L, cbsDropdown = 0x0002L, cbsDropdownList = 0x0003L, cbsOwnerdrawFixed = 0x0010L,
                    cbsOwnerdrawVariable = 0x0020L, cbsAutoHScroll = 0x0040L, cbsOEMConvert = 0x0080L, cbsSort = 0x0100L,
                    cbsHasStrings = 0x0200L, cbsNoIntegralHeight = 0x0400L, cbsDisableNoScroll = 0x0800L, cbsUppercase = 0x2000L,
                    cbsLowercase = 0x4000L,

                    // for tabcontrols:
                    tcsBottom = 0x0002 /* Not supported when themed */, tcsButtons = 0x0100, tcsFixedWidth = 0x0400, tcsToolButtons = 0x0008, tcsFocusNever = 0x8000,
                    tcsFocusOnClick = 0x1000, tcsIconLeft = 0x0010, tcsLabelLeft = 0x0020 /* implies  tcsIconLeft */, tcsHotTrack = 0x0040,
                    tcsMultiline = 0x0200, tcsMultiselect = 0x0004, tcsOwnerDrawFixed = 0x2000, tcsRaggedRight = 0x0800 /* don't stratch tabs to take up whole control width. default */,
                    tcsRight = tcsBottom /* Not supported when themed */, tcsRightJustify = 0x0000 /* Only for multiline controls */, tcsScrollOpposite = 0x0001,
                    tcsSingleline = 0x0000, tcsTabs = 0x0000, tcsTooltips = 0x4000, tcsVertical = 0x080 /* Only for multiline, not supported when themed. */,

                    // for listviews:
                    lvsAlignLeft = 0x0800, lvsAlignMask = 0x0c00, lvsAlignTop = 0x0000, lvsAutoArrange = 0x0100,
                    lvsEditLabels = 0x0200, lvsIcon = 0x0000, lvsList = 0x0003, lvsNoColumnHeader = 0x4000,
                    lvsNoLabelWrap = 0x0080, lvsNoScroll = 0x2000, lvsNoSortHeader = 0x8000, lvsOwnerData = 0x1000,
                    lvsOwnerdrawFixed = 0x0400, lvsDetails = 0x0001, lvsShareImagelists = 0x0040, lvsAlwaysShowSelect = 0x0008,
                    lvsSingleSelect = 0x0004, lvsSmallIcon = 0x0002, lvsSortAscending = 0x0010, lvsSortDescending = 0x0020,
                    
                    // for tooltips:
                    ttsAlwaysTip = 0x0001, ttsBalloon = 0x0040, ttsCloseButton = 0x0080, ttsNoAnimate = 0x0010,
                    ttsNoFade = 0x0020, ttsNoPrefix = 0x0002, ttsUseVisualStyle = 0x0100,

                    // for status bars:
                    stbsSizeGrip = 0x0100, stbsToolTips = 0x0800,

                    // for toolbars:
                    tbsAltDrag = 0x0400 /* Drag with alt instead of the shift key */, tbsCustomErase = 0x2000 /* Sends a custom drawing notification in WM_ERASEBKGND */,
                    tbsFlat = 0x0800, tbsList = 0x1000, tbsRegisterDrop = 0x4000 /* receives TBN_GETOBJECT notification to request drop target objects*/,
                    tbsTooltips = 0x0100, tbsTransparent = 0x8000 /* Transparency for the background. Does not make buttons transparent. Use tbsFlat or tbsList for that. */, 
                    tbsWrap = 0x0200 /* Wraps buttons or separator blocks if the width is too narrow. */,

                    // for toolbar buttons:
                    tbbsAutoWidth = 0x0010 /* Measure the button width from the image and text. */, tbbsCheck = 0x0002, tbbsGroup = 0x0004,
                    tbbsCheckGroup = (tbbsCheck | tbbsGroup), tbbsDropDown = 0x0008 /* The button contains a drop down arrow. If tbbsWholeDropDown is not used, the arrow will be in a separate part. */,
                    tbbsNoPrefix = 0x0020, tbbsSeparator = 0x0001, tbbsShowText = 0x0040 /* Combine wiht tbsList. Show text for the button. Not setting this will show the text as tooltip if tbsExMixed is set too. */,
                    tbbsWholeDropDown = 0x0080 /* The button contains the drop down arrow in the button part. */, 
                  };


enum ExtendedWindowStyles { wsExAcceptFiles = 0x00000010L, wsExAppWindow = 0x00040000L, wsExClientEdge = 0x00000200L,
                            wsExComposited = 0x02000000L, wsExContextHelp = 0x00000400L, wsExControlParent = 0x00010000L,
                            wsExDlgModalFrame = 0x00000001L, wsExLayered = 0x00080000, wsExLayoutRTL = 0x00400000L,
                            wsExLeft = 0x00000000L, wsExLeftScrollbar = 0x00004000L, wsExLTRReading = 0x00000000L,
                            wsExMDIChild = 0x00000040L, wsExNoActivate = 0x08000000L, wsExNoInheritLayout = 0x00100000L,
                            wsExNoParentNotify = 0x00000004L, wsExRight = 0x00001000L, wsExRightScrollbar = 0x00000000L,
                            wsExRTLReading = 0x00002000L, wsExStaticEdge = 0x00020000L, wsExToolWindow = 0x00000080L,
                            wsExTopmost = 0x00000008L, wsExTransparent = 0x00000020L, wsExWindowEdge = 0x00000100L,

                            // for listviews:
                            /*     Vista and above: */ lvsExAutoAutoArrange = 0x01000000, lvsExAutoCheckSelect = 0x08000000,
                            lvsExAutoSizeColumns = 0x10000000, lvsExColumnSnappoints = 0x40000000,
                            lvsExColumnOverflow = 0x80000000 /* Only when lvsExHeaderInAllViews is set. */,
                            lvsExHeaderInAllViews = 0x02000000, lvsExJustifyColumns = 0x00200000, 
                            lvsExTransparentBackground = 0x00400000, lvsExTransparentShadowText = 0x00800000 /* Only with lvsExTransparentBackground. */,
                            /* */

                            lvsExBorderSelect = 0x00008000, lvsExCheckboxes = 0x00000004,
                            lvsExDoubleBuffer = 0x00010000, lvsExFlatScrollbars = 0x00000100, lvsExFullRowSelect = 0x00000020  /* Only in details view. */,
                            lvsExGridLines = 0x00000001  /* Only in details view. */, lvsExHeaderDragDrop = 0x00000010 /* Only in details view. */,
                            lvsExHideLabels = 0x00020000, lvsExInfotip = 0x00000400, lvsExLabeltip = 0x00004000, lvsExMultiWorkareas = 0x00002000,
                            lvsExOneClickActivate = 0x00000040, lvsExSimpleSelect = 0x00100000 /* Only in Icon view style. */,
                            lvsExSnapToGrid = 0x00080000 /* Onlyl in Icon view style */, lvsExSubitemImages = 0x00000002 /* Only in details view. */,
                            lvsExTrackSelect = 0x00000008, lvsExTwoClickActivate = 0x00000080, lvsExUnderlineCold = 0x00001000 /* Only with lvsExTwoClickActivate. */,
                            lvsExUnderlineHot = 0x00000800 /* Only with lvsExOneClickActivate or lvsExTwoClickActivate. */,

                            // for toolbars: (These must be set with TB_SETEXTENDEDSTYLE to work.)
                            tbsExSeparateArrows = 0x00000001, tbsExHideClippedButtons = 0x00000010, tbsExDoubleBuffer = 0x00000080,
                            tbsExMixed = 0x00000008 /* Only display text for buttons which have the tbbsShowText style. Must be used together with the tbbsList style. */,


                          };

typedef uintset<ClassStyles> ClassStyleSet;
typedef uintset<WindowStyles> WindowStyleSet;
typedef uintset<ExtendedWindowStyles> ExtendedWindowStyleSet;

extern const WindowStyleSet wsOverlappedWindow;
extern const WindowStyleSet wsPopupWindow;
extern const WindowStyleSet wsTiledWindow;
extern const WindowStyleSet lbsStandard;

extern const ExtendedWindowStyleSet wsExOverlappedWindow;
extern const ExtendedWindowStyleSet wsExPaletteWindow;

struct ClassParams
{
    std::wstring classname;
    ClassStyleSet style;
    HICON icon;
    HICON iconsm;
    HBRUSH brush;
    HCURSOR cursor;
    int wndextra;
    WNDPROC wndproc;

    ClassParams() : icon(NULL), iconsm(NULL), brush(NULL), cursor(NULL), wndextra(0), wndproc(NULL) {}
};

struct WindowParams
{
    std::wstring windowtext;
    WindowStyleSet style;
    ExtendedWindowStyleSet extstyle;
    int x;
    int y;
    int width;
    int height;
    HWND parent;
    //HICON icon;
    //HICON iconsm;
    //HCURSOR cursor;
    HMENU menu;

    WindowParams() : x(0), y(0), width(0), height(0), parent(NULL)/*, icon(NULL), iconsm(NULL), cursor(NULL)*/, menu(NULL) { }
};

ATOM RegisterWindowClass(const ClassParams &params);
ATOM RegisterWindowClass(const std::wstring &classname, UINT style, int wndextra, HICON hIcon, HICON hIconSm, HBRUSH hBrush, HCURSOR hCursor);
HWND CreateWindowHandle(const std::wstring &classname, const WindowParams &params, LPVOID lpParam);
HWND CreateWindowHandle(const std::wstring &classname, const std::wstring &windowtext, DWORD exstyle, DWORD style, int x, int y, int width, int height, HWND parent, HMENU menu, LPVOID param);
WNDPROC ReplaceWndProc(HWND handle, WNDPROC newproc);

void InitCommonControl(DWORD control);

void ConstraintSizing(byte fwSide, Rect &r, int minwidth, int minheight, int maxwidth, int maxheight);
void ConstraintSizing(byte fwSide, Rect &r, int minwidth, int minheight);
void ConstraintSizing(byte fwSide, Rect &r, int minwidth, int minheight, int maxwidth, int maxheight, int widthdiff, int modx, int heightdiff, int mody);

class Form;
Form* GetNextForm(Form *current, bool disabled = false, bool hidden = false); // Returns the first form below the passed form if exists, or NULL.
Form* GetPrevForm(Form *current, bool disabled = false, bool hidden = false); // Returns the first form above the passed form if exists, or NULL.
Form* GetTopForm(bool disabled = false, bool hidden = false); // Returns the first form if one exists, or NULL.
Form* GetBottomForm(bool disabled = false, bool hidden = false); // Returns the last form if one exists, or NULL.

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
enum VirtualKeyStates : int;
#else
enum VirtualKeyStates;
#endif
typedef uintset<VirtualKeyStates> VirtualKeyStateSet;

UINT VirtualKeysFromWParam(WPARAM wParam);
WPARAM WParamFromVirtualKeys(VirtualKeyStateSet vkeys);

bool ContainsAccelerator(const std::wstring &str, WCHAR key);

// Enumerate all child windows in z-order, starting with the window at the bottom (or top if we believe ms documentation, but it's under other windows...)
template<typename LISTTYPE>
void EnumChildWindowsInZOrder(HWND parent, LISTTYPE &result)
{
    HWND hwnd = GetTopWindow(parent);
    if (!hwnd)
        return;
    result.push_back(hwnd);
    EnumChildWindowsInZOrder(hwnd, result);
    while ((hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) != NULL)
        result.push_back(hwnd);
}

std::wstring GetClassName(HWND handle); // Calls the system's GetClassName to fetch the class name of a window. Returns an empty string if unsuccessful.


}
/* End of NLIBNS */

