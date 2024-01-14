#pragma once

#include "objectbase.h"


namespace NLIBNS
{


    class Control;

    enum StockBrushes { sbWhite = 0, sbBlack, sbWindow, sbBtnFace, sbBtnText, sb3DHighlight, sb3DShadow, sbHighlight, sbHighlightText };
    enum StockPens { spWhite = 0, spBlack, spWindow, spBtnFace, spBtnText, sp3DHighlight, sp3DShadow, spHighlight, spHighlightText };
    enum StockFonts { sfCaption = 0, sfSmallCaption, sfMenu };

    enum InterpolationModes {
            imDefault = Gdiplus::InterpolationModeDefault,
            imLQ = Gdiplus::InterpolationModeLowQuality,
            imHQ = Gdiplus::InterpolationModeHighQuality,
            imBilinear = Gdiplus::InterpolationModeBilinear,
            imBicubic = Gdiplus::InterpolationModeBicubic,
            imNearestNeighbor = Gdiplus::InterpolationModeNearestNeighbor,
            imHQBilinear = Gdiplus::InterpolationModeHighQualityBilinear,
            imHQBicubic = Gdiplus::InterpolationModeHighQualityBicubic
    };

    enum PixelOffsetModes {
            pomDefault = Gdiplus::PixelOffsetModeDefault, // Same as none
            pomHighSpeed = Gdiplus::PixelOffsetModeHighSpeed, // Same as none
            pomHighQuality = Gdiplus::PixelOffsetModeHighQuality, // Same as half
            pomNone = Gdiplus::PixelOffsetModeNone, // Pixel center is at int coordinates.
            pomHalf = Gdiplus::PixelOffsetModeHalf // Pixel corner is at int coordinates.
    };

    enum ColorAdjustTypes {
            catDefault = Gdiplus::ColorAdjustTypeDefault,
            catBitmap = Gdiplus::ColorAdjustTypeBitmap,
            catBrush = Gdiplus::ColorAdjustTypeBrush,
            catPen = Gdiplus::ColorAdjustTypePen,
    };

    enum ColorMatrixFlags {
            cmfDefault = Gdiplus::ColorMatrixFlagsDefault,
            cmfSkipGrays = Gdiplus::ColorMatrixFlagsSkipGrays,
            cmfAltGrays = Gdiplus::ColorMatrixFlagsAltGray
    };

    enum LinearGradientModes {
            lgmForwardDiagonal = Gdiplus::LinearGradientModeForwardDiagonal,
            lgmBackwardDiagonal = Gdiplus::LinearGradientModeBackwardDiagonal,
            lgmHorizontal = Gdiplus::LinearGradientModeHorizontal,
            lgmVertical = Gdiplus::LinearGradientModeVertical,
    };

    enum ImageTypes {
            itUndefined, itMemoryBitmap, itBMP, itEMF, itWMF, itJPEG, itPNG, itGIF, itTIFF, itEXIF, itIcon
    };

#if defined(__MINGW32__) || (_MSC_VER >= 1700)
    enum Cursors : int;
#else
    enum Cursors;
#endif

    enum FontCharacterSets {
            fcsDefault = DEFAULT_CHARSET, fcsAnsi = ANSI_CHARSET, fcsBaltic = BALTIC_CHARSET, fcsChineseBig5 = CHINESEBIG5_CHARSET,
            fcsEastEurope = EASTEUROPE_CHARSET, fcsGB2312 = GB2312_CHARSET, fcsGreek = GREEK_CHARSET, fcsHangul = HANGUL_CHARSET,
            fcsMac = MAC_CHARSET, fcsOEM = OEM_CHARSET, fcsRussian = RUSSIAN_CHARSET, fcsShiftJIS = SHIFTJIS_CHARSET,
            fcsSymbol = SYMBOL_CHARSET, fcsTurkish = TURKISH_CHARSET, fcsVietnamese = VIETNAMESE_CHARSET,
            // Korean edition only
            fcsJohab = JOHAB_CHARSET,
            // Middle East edition only
            fcsArabic = ARABIC_CHARSET, fcsHebrew = HEBREW_CHARSET,
            // Thai edition only
            fcsThai = THAI_CHARSET,
#ifdef DESIGNING
            fcsCount = 19
#endif
    };

    enum FontOutputQualities {
            foqAntialiased = ANTIALIASED_QUALITY, foqCleartype = CLEARTYPE_QUALITY, foqDefault = DEFAULT_QUALITY,
            foqDraft = DRAFT_QUALITY, foqNonAntialiased = NONANTIALIASED_QUALITY, foqProof = PROOF_QUALITY,
#ifdef DESIGNING
            foqCount = 6
#endif
    };

enum CanvasTextAlignments {
            ctaLeft = TA_LEFT, ctaTop = TA_TOP, ctaRight = TA_RIGHT, ctaCenter = TA_CENTER,
            ctaBaseline = TA_BASELINE, ctaBottom = TA_BOTTOM, ctaRTLReading = TA_RTLREADING,
#ifdef DESIGNING
            ctaCount = 7
#endif
    };

    typedef uintset<CanvasTextAlignments> CanvasTextAlignmentSet;

    class StockCanvas;
    class Bitmap;

#ifdef DESIGNING
    class DesignProperties;
    class DesignSerializer;
#endif

    class Fonts : public std::vector<std::wstring>
    {
    private:
        typedef std::vector<std::wstring>   base;

        static Fonts *instance;

        Fonts();
        Fonts(const Fonts &orig) {};
        ~Fonts();
    public:
        static Fonts* GetInstance();
        static void FreeInstance();
    };

    extern Fonts *systemfonts;

    // Structure holding all data in fonts, not only to have the current data, but also to be able to save it for later when the font changed.
    struct FontData
    {
        std::wstring family;
        float size;
        int escapement;
        int orientation;
        Color color;
        bool bold;
        bool italic;
        bool underline;
        bool strikeout;
        FontCharacterSets charset;
        FontOutputQualities quality;

        FontData(const std::wstring& family, float size, int escapement, int orientation, Color color, bool bold, bool italic, bool underline, bool strikeout, FontCharacterSets charset, FontOutputQualities quality);
        FontData(const LOGFONT &lf);
        FontData();

        bool operator==(const FontData &other) const;
        bool operator!=(const FontData &other) const;
        operator LOGFONT() const;
    };

    class Font : public Object
    {
    private:
        typedef Object    base;
    private:
        LOGFONT FillLogfont();
        void Create();

        HFONT handle;

        FontData data;

        void Change(bool recreate);
    protected:
        virtual void Changing() noexcept {};
        virtual void Changed() noexcept {};

        virtual void Destroy() { base::Destroy(); }
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        float DefaultSize();
#endif
        virtual ~Font();

        Font();
        Font(const FontData &data);
        Font(const Font &orig);
        Font(Font &&orig) noexcept;
        Font(const std::wstring& family, float size, int escapement, int orientation, Color color, bool bold, bool italic, bool underline, bool strikeout, FontCharacterSets charset, FontOutputQualities quality);

        bool operator==(const Font &b) const;
        bool operator==(const FontData &fdata) const;
        bool operator!=(const Font &b) const;
        bool operator!=(const FontData &fdata) const;
        Font& operator=(Font &&b) noexcept;
        Font& operator=(const Font &b);
        Font& operator=(const FontData &newdata);

        const FontData& Data() const;
        HFONT Handle();
        bool HandleCreated();

        const std::wstring& Family() const;
        void SetFamily(const std::wstring& newfamily);
        float Size() const;
        void SetSize(float newsize);
        int Height() const;
        void SetHeight(int newheight);
        int Escapement() const;
        void SetEscapement(int newescapement);
        int Orientation() const;
        void SetOrientation(int neworientation);
        void SetAngle(int newangle); // Sets both the escapement and the orientation to the same value.
        Color GetColor() const;
        void SetColor(Color newcolor);
        bool Bold() const;
        void SetBold(bool newbold);
        bool Italic() const;
        void SetItalic(bool newitalic);
        bool Underline() const;
        void SetUnderline(bool newunderline);
        bool Strikeout() const;
        void SetStrikeout(bool newstrikeout);
        FontCharacterSets CharacterSet() const;
        void SetCharacterSet(FontCharacterSets newcharset);
        FontOutputQualities OutputQuality() const;
        void SetOutputQuality(FontOutputQualities newquality);
    };

    // Font which belongs to a control or canvas, so it can notify its parent.
    class OwnedFont : public Font
    {
    private:
        typedef Font base;

        bool changing;
        FontData saved;

        OwnedFont(const OwnedFont &copy) : base() { throw L"Cannot copy control fonts"; }

        virtual void Changing() noexcept;
        virtual void Changed() noexcept;
    protected:
        virtual void DoChanged(const FontData &saveddata) = 0;

        OwnedFont();
        OwnedFont(const FontData &data);
    public:
    };

    enum BrushTypes { btColorBrush, btHatchBrush, btTextureBrush, btLinearGradientBrush, btPathGradientBrush };
    enum BrushWrapModes { bwmTile = Gdiplus::WrapModeTile , bwmTileFlipX = Gdiplus::WrapModeTileFlipX, bwmTileFlipY = Gdiplus::WrapModeTileFlipY, bwmTileFilpXY = Gdiplus::WrapModeTileFlipXY, bwmNoTile = Gdiplus::WrapModeClamp };
    using Gdiplus::HatchStyle;
    class Brush
    {
    private:
        Gdiplus::Brush *brush;
        struct TextureData
        {
            Bitmap *bmp;
            //Gdiplus::Matrix mx;
        };
        struct PathData
        {
            Gdiplus::GraphicsPath *path;
            Gdiplus::Point *points;
            Gdiplus::PointF *pointsF;
            int pointcnt;

            Gdiplus::Color *colors;
            int colorcnt;
        };
        void *data;
        bool storedata;

        BrushTypes type;
    public:
        Brush(); // Default brush constructor that creates a black color brush.
        Brush(const Color &color); // Creates a color brush with the passed color.
        Brush(const Rect &rect, const Color &c1, const Color &c2, float angle, bool anglescalable = false, BrushWrapModes wrapmode = bwmTile); // Creates a linear gradient brush with the given starting and ending colors and angle. 0 angle is the direction right, 90 for down.
        Brush(const RectF &rect, const Color &c1, const Color &c2, float angle, bool anglescalable = false, BrushWrapModes wrapmode = bwmTile); // Creates a linear gradient brush with the given starting and ending colors and angle. 0 angle is the direction right, 90 for down.
        Brush(Bitmap *bmp, bool storebmp = true, BrushWrapModes wrapmode = bwmTile); // Creates a texture brush from the given bitmap. If storebmp is true only a copy of bmp is used in the brush. Otherwise the bitmap must be valid till the brush is destroyed.
        Brush(Bitmap *bmp, Rect r, bool storebmp = true, BrushWrapModes wrapmode = bwmTile); // Creates a texture brush from a portion of the given bitmap. If the passed rectangle would produce an image outside the bitmap's range the size of it will be reduced to the maximum available space. If the brush created this way has a width or height of 0, an exception is raised. If storebmp is true only a copy of bmp is used in the brush. Otherwise the bitmap must be valid till the brush is destroyed.
        Brush(const Color &linecolor, const Color &bgcolor, Gdiplus::HatchStyle hatchstyle); // Creates a hatch brush with the given line color, background color and style.
        Brush(Gdiplus::GraphicsPath *path, const Color &color, bool copypath = true, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors can be set by directly accessing the brush through the Object() function. If storepath is true only a copy of the path is used in the brush. Otherwise the path must be valid till the brush is destroyed.
        Brush(Gdiplus::GraphicsPath *path, const Color &centercolor, Color *colors, int colorcnt, bool copypath = true, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors can be set by directly accessing the brush through the Object() function. If storepath is true only a copy of the path is used in the brush. Otherwise the path must be valid till the brush is destroyed. The colors array is always copied.
        Brush(Gdiplus::GraphicsPath *path, const Color &centercolor, Gdiplus::Color *colors, int colorcnt, bool copypath = true, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors can be set by directly accessing the brush through the Object() function. If storepath is true only a copy of the path is used in the brush. Otherwise the path must be valid till the brush is destroyed. The colors array is always copied.
        Brush(Point *points, int pointcount, const Color &color, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors must be set by calling Object() and casting it to Gdiplus::PathGradientBrush. Because the passed points must be converted to Gdiplus::Point structures, it is not necessary to keep the array of points valid after this call.
        Brush(PointF *points, int pointcount, const Color &color, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors must be set by calling Object() and casting it to Gdiplus::PathGradientBrush. Because the passed points must be converted to Gdiplus::Point structures, it is not necessary to keep the array of points valid after this call.
        Brush(Point *points, int pointcount, const Color &centercolor, Color *colors, int colorcnt, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors must be set by calling Object() and casting it to Gdiplus::PathGradientBrush. Because the passed points must be converted to Gdiplus::Point structures, it is not necessary to keep the array of points valid after this call.
        Brush(PointF *points, int pointcount, const Color &centercolor, Gdiplus::Color *colors, int colorcnt, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors must be set by calling Object() and casting it to Gdiplus::PathGradientBrush. Because the passed points must be converted to Gdiplus::Point structures, it is not necessary to keep the array of points valid after this call.
        Brush(const Brush &orig); // Creates a copy of the passed brush.
        Brush(Brush &&orig) noexcept; // Move constructor.
        ~Brush();

        BrushTypes Type() const; // Returns the type of the gdiplus brush object stored by this brush.
        BrushWrapModes GetWrapMode() const; // Returns the wrap mode of the brush if it is a texture brush. Otherwise returns bwmNoTile.
        void SetWrapMode(BrushWrapModes newwrapmode); // Changes the current wrap mode for brushes that support it.
        HatchStyle GetHatchStyle() const; // Returns the hatch style of a hatch brush or HatchStyleTotal if the brush is not a hatch brush.
        void SetHatchStyle(HatchStyle newhatchstyle); // Changes the current hatch style for hatch brushes. Does nothing otherwise.

        Brush& operator=(const Brush &b);
        Brush& operator=(Brush &&b) noexcept;

        Gdiplus::Brush& Object() const;

        Color GetColor() const; // Returns the color of the current color brush, the first color of the current linear gradient brush or line color of the current hatch brush, or the center color of path gradient brushes. Returns clNone for texture brushes.
        void SetColor(const Color &newcolor); // Changes the color of the current color brush, the first color of the current linear graident brush or the background color of a hatch brush. Does nothing otherwise.
        Color SecondaryColor() const; // Returns the second color of a linear gradient brush, the background color of the current hatch brush, or clNone color otherwise.
        void SetSecondaryColor(const Color &newcolor); // Changes the second color of a linear gradient brush or background color for a hatch brush. Does nothing otherwise.
        int SurroundColorCount() const; // Returns the number of surround colors for path gradient brushes. Returns 0 for any other type.
        void SetSurroundColors(Gdiplus::Color *colors, int colorcnt); // Sets the surround color for path gradient brushes. Does nothing for other types.
        void SetSurroundColors(Color *colors, int colorcnt); // Sets the surround color for path gradient brushes. Does nothing  for other types.
    };

    enum PenTypes { ptColorPen, ptBrushPen };
    enum PenDashStyles { pdsNone, pdsDash, pdsDot, pdsDashDot, pdsDashDotDot, pdsCustom };
    class Pen
    {
    private:
        Gdiplus::Pen *pen;
        Brush *brush;
        bool storedbrush;
        PenTypes type;

        PenDashStyles dash;
        Gdiplus::REAL *dashlengths;
        int dashcount;
        bool storeddash;
    public:
        Pen(); // Default pen constructor that creates a white pen with a width of 1.
        Pen(const Pen &orig); // Creates a copy of the passed pen.
        Pen(const Color &color, float width = 0);
        Pen(const Color &color, PenDashStyles dashstyle, float width = 0);
        Pen(Brush *brush, float width = 0, bool storedbrush = true); // Creates a pen from a brush with the given width. If storedbrush is true, the brush object is copied to the pen. Otherwise the brush must be valid till the pen is destroyed.
        Pen(Pen &&other) noexcept;
        ~Pen();

        Pen& operator=(const Pen &b);
        Pen& operator=(Pen &&other) noexcept;

        Gdiplus::Pen& Object() const;

        PenTypes Type() const;

        Brush& GetBrush() const; // Returns the brush used by the pen if it was constructed with one.
        void SetBrush(Brush *newbrush, bool storedbrush = true); // Recreates the pen with the same width and the given brush. If storedbrush is true, the brush object is owned by the pen and it will destroy it. Otherwise only a copy is used. Setting a brush removes any dash style previously set.
        PenDashStyles DashStyle() const; // Returns the current dash pattern used to draw lines with the pen.
        void SetDashStyle(PenDashStyles newdash); // Sets a style used for drawing dashed lines with the pen. pdsCustom cannot be set. If you need a custom dash pattern, call SetCustomDashPattern(). Setting a dash style removes any brush previously set.
        void SetCustomDashPattern(Gdiplus::REAL *lengths, int count, bool storedpattern = true); // Sets a custom dash pattern to be used when drawing lines with the pen.  If storedpattern is set to false, a copy is made of the lengths array. Otherwise it is your responsibility that the array stays valid while the pen is used, and that it is destroyed after use. Setting a dash pattern removes a brush if set.
        Color GetColor() const; // Returns the color set for this pen if it is a color pen. Brush pens return the GetColor() result of the stored brush. Because not all brushes can use a color, the result might be different from the real color of the brush.
        void SetColor(const Color &newcolor);
        float Width() const;
        void SetWidth(float newwidth);
    };

    class Canvas;
    class CanvasGraphicsState
    {
        int *instances;
        bool *skipdelete;
        int id;
        Canvas *owner;

        CanvasGraphicsState(int id, Canvas *c, Gdiplus::GraphicsState gdistate);
        Gdiplus::GraphicsState gdistate;
        Font statefont;
        bool stateadvanced;
        Pen statepen;
        Brush statebrush;
        Gdiplus::ImageAttributes *stateattrib;

        bool usedefaultmatrix;
        ColorMatrix defaultmatrix;
        ColorMatrixFlags defaultmatrixflag;

        bool usebitmapmatrix;
        ColorMatrix bitmapmatrix;
        ColorMatrixFlags bitmapmatrixflag;

        bool usebrushmatrix;
        ColorMatrix brushmatrix;
        ColorMatrixFlags brushmatrixflag;

        bool usepenmatrix;
        ColorMatrix penmatrix;
        ColorMatrixFlags penmatrixflag;

        bool usedefaultcolorkey;
        Color defaultlokey;
        Color defaulthikey;

        bool usebitmapcolorkey;
        Color bitmaplokey;
        Color bitmaphikey;

        bool usebrushcolorkey;
        Color brushlokey;
        Color brushhikey;

        bool usepencolorkey;
        Color penlokey;
        Color penhikey;

        CanvasTextAlignmentSet textalign;
        InterpolationModes intmode;
        PixelOffsetModes pommode;
        bool antialias;

        bool usetransf;
        Matrix transf;

        friend class Canvas;
    public:
        CanvasGraphicsState(const CanvasGraphicsState &orig);
        CanvasGraphicsState& operator=(const CanvasGraphicsState &orig);
        ~CanvasGraphicsState();
    };


    enum TextDrawOptions : int {
            tdoBottom = DT_BOTTOM,
            tdoCenter = DT_CENTER,
            tdoEditControl = DT_EDITCONTROL,
            tdoEndEllipsis = DT_END_ELLIPSIS,
            tdoExpandTabs = DT_EXPANDTABS,
            tdoHidePrefix = DT_HIDEPREFIX,
            tdoNoClip = DT_NOCLIP,
            tdoNoFullWidthCharBreak = DT_NOFULLWIDTHCHARBREAK,
            tdoNoPrefix = DT_NOPREFIX,
            tdoPathEllipsis = DT_PATH_ELLIPSIS,
            tdoPrefixOnly = DT_PREFIXONLY,
            tdoRight = DT_RIGHT,
            tdoRTLReading = DT_RTLREADING,
            tdoSingleLine = DT_SINGLELINE,
            tdoTop = DT_TOP,
            tdoVCenter = DT_VCENTER,
            tdoWordBreak = DT_WORDBREAK,
            tdoWordEllipsis = DT_WORD_ELLIPSIS,
    };
    typedef uintset<TextDrawOptions> TextDrawOptionSet;

    class Icon;

    enum CanvasStates { csDCBusy = 1 };
    typedef uintset<CanvasStates> CanvasStateSet;
    class Canvas
    {
    private:
        static int instances;

        static std::vector<Brush*> stockbrushes;
        static std::vector<Pen*> stockpens;
        static std::vector<Font*> stockfonts;

        // The font doesn't use the Ggdiplus so it must be saved as well in states. CanvasStateSet contains the font plus the gdiplus state.
        CanvasStateSet canvasstate;
        int graphicsstatepos; // The id number of the previously returned state, which is used to identify them when the user returns one state.
        std::vector<CanvasGraphicsState> savedstates; // A vector containing all out states that were handed in the order they were created. Once a state is returned, all states that were created after that are also removed.

        void FillStock();
        void ClearStock();

        static std::map<std::wstring, Brush*> brushes;
        static std::map<std::wstring, Pen*> pens;
        static std::map<std::wstring, Font*> fonts;

        Brush *brush;
        bool outerbrush; // The brush is not owned by the canvas, so it shouldn't delete it.

        Pen *pen;
        bool outerpen; // The pen is not owned by the canvas, so it shouldn't delete it.

        Font *font;
        bool outerfont; // The font is not owned by the canvas, so it shouldn't delete it.

        bool advanced; // Graphics mode is advanced or not. Internally SetGraphicsMode is called to set this, but it only works with functions that use the GDI's DC and not with GDI+.

        // These values are saved on GetDC and restored on ReturnDC.
        HDC saveddc; // DC got when GetDC is called.
        HRGN savedrgn; // Clipping region for DC saved in GetDC and restored in ReturnDC.
        bool hasrgn; // True if savedrgn contains a valid clipping region after GetDC.
        HFONT savedfont; // Font saved when selecting a font to the DC of the canvas.
        bool savedadvanced; // The advanced mode saved when getting a dc.
        int savedbkmode; // The text background mode.
        Color savedtextcolor; // Font color.
        CanvasTextAlignmentSet savedtextalign; // Text alignment got with the GetTextAlign windows function.

        Gdiplus::ImageAttributes attrib;

        ColorMatrix defaultmatrix;
        ColorMatrixFlags defaultmatrixflag;
        bool usedefaultmatrix;
        ColorMatrix bitmapmatrix;
        ColorMatrixFlags bitmapmatrixflag;
        bool usebitmapmatrix;
        ColorMatrix brushmatrix;
        ColorMatrixFlags brushmatrixflag;
        bool usebrushmatrix;
        ColorMatrix penmatrix;
        ColorMatrixFlags penmatrixflag;
        bool usepenmatrix;
        bool usedefaultcolorkey;
        Color defaultlokey;
        Color defaulthikey;
        bool usebitmapcolorkey;
        Color bitmaplokey;
        Color bitmaphikey;
        bool usebrushcolorkey;
        Color brushlokey;
        Color brushhikey;
        bool usepencolorkey;
        Color penlokey;
        Color penhikey;

        bool usetransf;
        Matrix transf;

        bool UseImageAttributes();
        void SetupImageAttributes();
        void SetupTransformations();

        CanvasTextAlignmentSet textalign;
        InterpolationModes intmode;
        PixelOffsetModes pommode;
        bool antialias;

        // Set everything to default values
        void ResetBrush();
        void ResetPen();
        void ResetFont();

        // Prevent copying.
#ifdef _MSC_VER
        Canvas& operator=(const Canvas&) {}
        Canvas(const Canvas&) {}
#else
        Canvas& operator=(const Canvas&) = delete;
        Canvas(const Canvas&) = delete;
#endif
    protected:
        virtual Gdiplus::Graphics* GetGraphics() = 0;
        virtual void ReleaseGraphics() = 0;
        virtual bool GraphicsCreated() = 0;

        virtual HDC _GetDC() = 0;
        virtual void _ReturnDC() = 0;

        Gdiplus::Graphics* Get();

        void UpdateGraphics(Gdiplus::Graphics *graphics); // Called when the graphics object is recreated.

        CanvasStateSet CanvasState();
        bool AddCanvasState(CanvasStates state);
        bool RemoveCanvasState(CanvasStates state);
        bool HasCanvasState(CanvasStates state) const;

        friend class StockCanvas;
        friend class CanvasGraphicsState;
    public:
        Canvas();
        virtual ~Canvas();

        void Release();

        HDC GetDC();
        void ReturnDC();
        virtual bool CompatibleDC();

        void Clear(Color newcolor);

        void AddBrush(const std::wstring& name, Color color, bool select = false);
        void AddPen(const std::wstring& name, Color color, float width, bool select = false);
        void AddFont(const std::wstring& name, const std::wstring& family, float size = 8.25f, int escapement = 0, int orientation = 0, Color color = clBtnText, bool bold = false, bool italic = false, bool underline = false, bool strikeout = false, FontCharacterSets charset = fcsDefault, FontOutputQualities quality = foqDefault, bool select = false);

        void AddBrush(const std::wstring& name, Brush &brush, bool select = false);
        void AddPen(const std::wstring& name, Pen &pen, bool select = false);
        void AddFont(const std::wstring& name, Font &font, bool select = false);

        bool HasBrush(const std::wstring& name);
        bool HasPen(const std::wstring& name);
        bool HasFont(const std::wstring& name);

        void RemoveBrush(const std::wstring& name);
        void RemovePen(const std::wstring& name);
        void RemoveFont(const std::wstring& name);
        void SelectBrush(const std::wstring& name);
        void SelectPen(const std::wstring& name);
        void SelectFont(const std::wstring& name);
        void SelectStockBrush(StockBrushes sbrush);
        void SelectStockPen(StockPens spen);
        void SelectStockFont(StockFonts sfont);

        void SetBrush(const Color &color); // Creates a color brush with the passed color.
        void SetBrush(const Rect &rect, const Color &c1, const Color &c2, float angle, bool anglescalable = false); // Creates a linear gradient brush with the given starting and ending colors and angle. 0 angle is the direction right, 90 for down.
        void SetBrush(const RectF &rect, const Color &c1, const Color &c2, float angle, bool anglescalable = false); // Creates a linear gradient brush with the given starting and ending colors and angle. 0 angle is the direction right, 90 for down.
        void SetBrush(Bitmap *bmp, bool storebmp = true, BrushWrapModes wrapmode = bwmTile); // Creates a texture brush from the given bitmap.
        void SetBrush(Bitmap *bmp, const Rect &r, bool storebmp = true, BrushWrapModes wrapmode = bwmTile); // Creates a texture brush from a portion of the given bitmap. If the passed rectangle would produce an image outside the bitmap's range the size of it will be reduced to the maximum available space. If the brush created this way has a width or height of 0, an exception is raised.
        void SetBrush(const Color &linecolor, const Color &bgcolor, Gdiplus::HatchStyle hatchstyle); // Creates a hatch brush with the given line color, background color and style.
        void SetBrush(Gdiplus::GraphicsPath *path, const Color &color, bool storepath = true, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors must be set by calling Object() and casting it to Gdiplus::PathGradientBrush.
        void SetBrush(Point *points, int pointcount, const Color &color, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors must be set by calling Object() and casting it to Gdiplus::PathGradientBrush.
        void SetBrush(PointF *points, int pointcount, const Color &color, BrushWrapModes wrapmode = bwmNoTile); // Creates a path gradient brush with the specified center color and wrap mode. The bounding colors must be set by calling Object() and casting it to Gdiplus::PathGradientBrush.


        void SetPen(Color color, float width = 0);
        void SetPen(Color color, PenDashStyles dashstyle, float width = 0);
        void SetFont(const std::wstring& family, float size, int escapement, int orientation, Color color, bool bold, bool italic, bool underline, bool strikeout, FontCharacterSets charset, FontOutputQualities quality);
        //void SetFont(::Font *font);

        void SetBrush(Brush &brush);
        void SetPen(Pen &pen);
        void SetFont(const Font &font);

        Brush& GetBrush();
        Pen& GetPen();
        Font& GetFont();

        void Line(int x1, int y1, int x2, int y2);
        void Line(Point start, Point end);
        void Line(PointF start, PointF end);
        void LineF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2);

        void FillRect(int x1, int y1, int x2, int y2);
        void FillRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2);
        void FillRect(const Rect &rect);
        void FillRectF(const RectF &rect);

        void FillPolygon(Point *points, int pointcount);
        void FillPolygonF(PointF *points, int pointcount);

        void FillEllipse(int x1, int y1, int x2, int y2);
        void FillEllipseF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2);
        void FillEllipse(const Rect &rect);
        void FillEllipseF(const RectF &rect);

        Gdiplus::GraphicsPath* CreateRoundPath(int x1, int y1, int x2, int y2, int roundwidth, int roundheight);
        Gdiplus::GraphicsPath* CreateRoundPathF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundwidth, Gdiplus::REAL roundheight);
        Gdiplus::GraphicsPath* CreateRoundPath(const Rect &rect, int roundwidth, int roundheight);
        Gdiplus::GraphicsPath* CreateRoundPathF(const RectF &rect, float roundwidth, float roundheight);

        Gdiplus::GraphicsPath* CreateRoundPath(int x1, int y1, int x2, int y2, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft);
        Gdiplus::GraphicsPath* CreateRoundPathF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft);
        Gdiplus::GraphicsPath* CreateRoundPath(const Rect &rect, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft);
        Gdiplus::GraphicsPath* CreateRoundPathF(const RectF &rect, float roundtopleft, float roundtopright, float roundbottomright, float roundbottomleft);

        Gdiplus::GraphicsPath* CreateEllipsePath(int x1, int y1, int x2, int y2);
        Gdiplus::GraphicsPath* CreateEllipsePathF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2);
        Gdiplus::GraphicsPath* CreateEllipsePath(const Rect &rect);
        Gdiplus::GraphicsPath* CreateEllipsePathF(const RectF &rect);

        void FillPath(Gdiplus::GraphicsPath *path);

        void FillRoundRect(int x1, int y1, int x2, int y2, int roundwidth, int roundheight);
        void FillRoundRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundwidth, Gdiplus::REAL roundheight);
        void FillRoundRect(const Rect &rect, int roundwidth, int roundheight);
        void FillRoundRectF(const RectF &rect, float roundwidth, float roundheight);

        void FillRoundRect(int x1, int y1, int x2, int y2, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft);
        void FillRoundRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft);
        void FillRoundRect(const Rect &rect, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft);
        void FillRoundRectF(const RectF &rect, float roundtopleft, float roundtopright, float roundbottomright, float roundbottomleft);

        void FrameRect(int x1, int y1, int x2, int y2);
        void FrameRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2);
        void FrameRect(const Rect &rect);
        void FrameRectF(const RectF &rect);

        void RoundFrameRect(int x1, int y1, int x2, int y2, int roundwidth, int roundheight);
        void RoundFrameRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundwidth, Gdiplus::REAL roundheight);
        void RoundFrameRect(const Rect &rect, int roundwidth, int roundheight);
        void RoundFrameRectF(const RectF &rect, float roundwidth, float roundheight);

        void RoundFrameRect(int x1, int y1, int x2, int y2, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft);
        void RoundFrameRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft);
        void RoundFrameRect(const Rect &rect, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft);
        void RoundFrameRectF(const RectF &rect, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft);

        // Draws a frame with classic 3D effect.
        void DrawFrame(int x1, int y1, int x2, int y2, bool raised);
        void DrawFrameF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, bool raised);
        void DrawFrame(const Rect &rect, bool raised);
        void DrawFrameF(const RectF &rect, bool raised);

        void DrawEllipse(int x1, int y1, int x2, int y2);
        void DrawEllipseF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2);
        void DrawEllipse(const Rect &rect);
        void DrawEllipseF(const RectF &rect);

        void GradientRect(int x1, int y1, int x2, int y2, Color color1, Color color2, LinearGradientModes mode);
        void GradientRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Color color1, Color color2, LinearGradientModes mode);
        void GradientRect(const Rect &rect, Color color1, Color color2, LinearGradientModes mode);
        void GradientRectF(const RectF &rect, Color color1, Color color2, LinearGradientModes mode);

        void Draw(Bitmap *bitmap, int x, int y);
        void DrawF(Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y);

        void Draw(Bitmap *bitmap, int x, int y, int width, int height);
        void DrawF(Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::REAL width, Gdiplus::REAL height);

        void Draw(Bitmap *bitmap, int destx, int desty, int srcx, int srcy, int width, int height);
        void DrawF(Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL width, Gdiplus::REAL height);

        void Draw(Bitmap *bitmap, int destx, int desty, int destwidth, int destheight, int srcx, int srcy, int srcwidth, int srcheight);
        void DrawF(Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL destwidth, Gdiplus::REAL destheight, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL srcwidth, Gdiplus::REAL srcheight);

        void Draw(Gdiplus::Bitmap *bitmap, int x, int y);
        void DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y);

        void Draw(Gdiplus::Bitmap *bitmap, int x, int y, int width, int height);
        void DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::REAL width, Gdiplus::REAL height);

        void Draw(Gdiplus::Bitmap *bitmap, int destx, int desty, int srcx, int srcy, int width, int height);
        void DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL width, Gdiplus::REAL height);

        void Draw(Gdiplus::Bitmap *bitmap, int destx, int desty, int destwidth, int destheight, int srcx, int srcy, int srcwidth, int srcheight);
        void DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL destwidth, Gdiplus::REAL destheight, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL srcwidth, Gdiplus::REAL srcheight);

        InterpolationModes InterpolationMode();
        void SetInterpolationMode(InterpolationModes newmode); // Select how Gdiplus scales images.

        PixelOffsetModes PixelOffsetMode();
        void SetPixelOffsetMode(PixelOffsetModes newmode); // Select how Gdiplus scales images.

        void SetColorMatrix(const ColorMatrix &colormatrix, ColorMatrixFlags flag = cmfDefault, ColorAdjustTypes adjusttype = catDefault);
        void ResetColorMatrix(ColorAdjustTypes adjusttype = catDefault);

        void SetColorKey(Color low, Color high, ColorAdjustTypes adjusttype = catDefault);
        void ResetColorKey(ColorAdjustTypes adjusttype = catDefault);

        void SetTransform(const Matrix &matrix);
        void ResetTransform();
        Matrix Transform();
        bool UsingTransform();
        void MultiplyTransform(const Matrix &matrix, bool prepend);
        void RotateTransform(Gdiplus::REAL angle, bool prepend);
        void RotateAtTransform(Gdiplus::REAL angle, PointF center, bool prepend);
        void ScaleTransform(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend);
        void TranslateTransform(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend);
        void ShearTransform(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend);

        void DrawCursor(Cursors cursor, int x, int y, int frame = 0);

        void DrawIcon(Icon &icon, int x, int y);
        void DrawIcon(Icon &icon, int x, int y, int width, int height);

        bool ClipEmpty();
        void ResetClip();
        void TranslateClip(int x, int y);
        void TranslateClip(Gdiplus::REAL x, Gdiplus::REAL y);
        void ClipRect(Rect &r);
        void ClipRectF(RectF &r);
        //void ClipRegion(Gdiplus::Region &rgn);
        bool GetClip(Gdiplus::Region &rgn); // Returns false if the region was not retrieved.
        bool GetClip(HRGN &rgn); // Returns false if the region was not retrieved. The region must be deleted with DeleteObject.
        bool GetClipBounds(Rect &r);
        bool GetClipBoundsF(RectF &r);
        void SetClip(const Rect &rect);
        void SetClip(HRGN rgn);
        void SetClip(const Gdiplus::Region &rgn);
        void ExcludeClip(const Rect &rect);
        void ExcludeClip(HRGN rgn);
        void ExcludeClip(const Gdiplus::Region &rgn);
        void IncludeClip(const Rect &rect);
        void IncludeClip(HRGN rgn);
        void IncludeClip(const Gdiplus::Region &rgn);
        void IntersectClip(const Rect &rect);
        void IntersectClip(HRGN rgn);
        void IntersectClip(const Gdiplus::Region &rgn);

        Size MeasureText(const std::wstring& text);
        virtual void TextDraw(const Rect &clip, int x, int y, const std::wstring& text);
        virtual void DrawGrayText(const Rect &clip, int x, int y, const std::wstring& text);
        Size MeasureFormattedText(const std::wstring& text, TextDrawOptionSet options, int wantedwidth = 0);
        virtual void FormatText(const Rect &r, const std::wstring& text, TextDrawOptionSet options);
        virtual void FormatGrayText(const Rect &r, const std::wstring& text, TextDrawOptionSet options);
        CanvasTextAlignmentSet TextAlignment();
        void SetTextAlignment(CanvasTextAlignmentSet newalign);
        TEXTMETRIC TextMetrics(); // Returns the metrics of the current font.
        bool AdvancedTextMode(); // Whether the dc is set to advanced graphics mode, used with text output.
        void SetAdvancedTextMode(bool newadvanced);  // Sets the graphics mode for text output to advanced or "compatible".

        CanvasGraphicsState SaveState();
        void RestoreState(CanvasGraphicsState state);

        bool Antialias();
        void SetAntialias(bool newantialias);

        void Flush();
    };

    class Application;
    class StockCanvas : public Canvas
    {
    private:
        typedef Canvas base;

        StockCanvas() { };
        StockCanvas(const StockCanvas &copy) { }

        static StockCanvas *instance;
        static void FreeInstance();
        static StockCanvas* GetInstance();

        virtual Gdiplus::Graphics* GetGraphics() { throw L"This canvas cannot do graphic operations."; }
        virtual void ReleaseGraphics() { };
        virtual HDC _GetDC() { return NULL; }
        virtual void _ReturnDC() { }
        virtual bool GraphicsCreated() { return false; }

        void SettingsChanged();
        friend Application;

        StockCanvas& operator=(const StockCanvas &other) { throw ""; } // Prevent copying.
    };

    class ControlCanvas : public Canvas
    {
    private:
        typedef Canvas base;

        Control *owner;

        HDC dc;
        HANDLE device; // For printing.

        HDC gotdc; // DC got by GetDC call, in case it is different from the original.
        Gdiplus::Graphics *graphics;

        //HPAINTBUFFER hdblbuff; // Vista double buffer context.
        //int dblcnt; // Number of times StartDoubleBuffering has been called. Only the first call is effective, the others just increase this value. It is -1 if DisableDoubleBuffering was called first, and ignores all start or end calls.

        void Update(HDC dc, HANDLE device);

        // Prevent copying.
#ifdef _MSC_VER
        ControlCanvas(const ControlCanvas&) {}
        ControlCanvas& operator=(const ControlCanvas&) {}
#else
        ControlCanvas(const ControlCanvas&) = delete;
        ControlCanvas& operator=(const ControlCanvas&) = delete;
#endif
    protected:
        virtual Gdiplus::Graphics* GetGraphics();
        virtual void ReleaseGraphics();
        virtual bool GraphicsCreated();
        virtual HDC _GetDC();
        virtual void _ReturnDC();

        friend class Control;
    public:
        ControlCanvas(Control *owner);
        ControlCanvas(HDC dc);
        virtual ~ControlCanvas();
    };

    enum BitmapStates { bsLocked = 1 };
    enum GdiplusImageEncoders { gieAuto = -1, gieBMP = 0, gieJPEG, gieGIF, gieTIFF, giePNG };
    enum GdiplusLockMode { glmReadWrite, glmReadOnly, glmWriteOnly };

    typedef uintset<BitmapStates> BitmapStateSet;
    class Bitmap : public Canvas
    {
    private:
        typedef Canvas base;

        Gdiplus::Bitmap *bmp;
        Gdiplus::Graphics *graphics;

        Gdiplus::BitmapData *lockedbits;
        HDC gotdc; // dc got by GetDC call

        BitmapStateSet bitmapstate;

        void BitmapFromStream(IStream *stream);
        void CloneConstruct();

        // Prevent copying.
#ifdef _MSC_VER
        Bitmap(const Bitmap&) {}
        Bitmap& operator=(const Bitmap&) {}
#else
        Bitmap(const Bitmap &orig) = delete;
        Bitmap& operator=(const Bitmap &orig) = delete;
#endif

        explicit Bitmap(char); // Constructor used in CreateCopy to be able to create a Bitmap object without constructing its bmp member.
    protected:
        virtual Gdiplus::Graphics* GetGraphics();
        virtual void ReleaseGraphics();
        virtual bool GraphicsCreated();
        virtual bool CompatibleDC();
        virtual HDC _GetDC();
        virtual void _ReturnDC();

        friend Canvas;
    public:
        static Bitmap&& MoveCopy(const Bitmap &orig); // Use instead of the copy constructor, to clone data of another bitmap.  The original is not modified in any way. (The copy constructor could be mistakenly used in stl containers etc.)
        static Bitmap* CreateCopy(const Bitmap &orig); // Use instead of the copy constructor, to clone data of another bitmap. (The copy constructor could be mistakenly used in stl containers etc.)

        Bitmap(Gdiplus::PixelFormat pixelformat = PixelFormat32bppARGB);
        Bitmap(int width, int height, Gdiplus::PixelFormat pixelformat = PixelFormat32bppARGB);
        Bitmap(HMODULE module, const wchar_t *resource); // The type of resource must be RCDATA in the .rc file.
        Bitmap(const std::wstring &filename); // Loads an image from file.
        Bitmap(Bitmap &&orig) noexcept;
        Bitmap(const Bitmap &orig, const Rect &srcrect); // Construct this bitmap from a part of another.
        Bitmap(void *filedata, int datasize); // Create a bitmap from a file fully loaded to memory.
        Bitmap(IStream* stream); // Create a bitmap from a windows stream.
        Bitmap(HBITMAP hbmp); // Create bitmap from a bitmap handle by copying the bits in hbmp to the bitmap.
        virtual ~Bitmap();

        void Copy(const Bitmap &src); // Copies the contents of the source bitmap.
        Bitmap& operator=(Bitmap &&orig) noexcept;

        Gdiplus::PixelFormat PixelFormat();

        //bool Save(wchar_t *filename, GdiplusImageEncoders encoder = gieAuto); // Saves image to a file. The encoder specifies the format. If it is gieAuto, the format is taken from the file name. Returns true on success.
        bool Save(const std::wstring &filename, GdiplusImageEncoders encoder = gieAuto); // Saves image to a file. The encoder specifies the format. If it is gieAuto, the format is taken from the file name. Returns true on success.
        bool Save(IStream *stream, GdiplusImageEncoders encoder); // Saves image to a windows stream. The encoder specifies the format. The general gieAuto doesn't work as there is no file extension to guess the output format.
        bool Load(const std::wstring &filename); // Loads image from file.
        bool Load(IStream *stream); // Loads image from a windows stream.

        virtual void TextDraw(const Rect &clip, int x, int y, const std::wstring& text);
        virtual void DrawGrayText(const Rect &clip, int x, int y, const std::wstring& text);
        virtual void FormatText(const Rect &r, const std::wstring& text, TextDrawOptionSet options);
        virtual void FormatGrayText(const Rect &r, const std::wstring& text, TextDrawOptionSet options);

        Gdiplus::Bitmap* GetBitmap();

        void SetSize(int newwidth, int newheight);

        Gdiplus::BitmapData* LockBits(GdiplusLockMode lockmode = glmReadWrite, Gdiplus::PixelFormat format = PixelFormatDontCare); // Locks the whole area.
        Gdiplus::BitmapData* LockBits(const Rect &area, GdiplusLockMode lockmode = glmReadWrite, Gdiplus::PixelFormat format = PixelFormatDontCare); // Passing an empty area is equal to using the whole area of the bitmap.
        void UpdateBits(); // Must be called after LockBits before any other operation can be made with the bitmap.

        Color Pixel(int x, int y);
        void SetPixel(int x, int y, Color color);

        int Width() const;
        int Height() const;

        ImageTypes GetImageType();

        HBITMAP HandleCopy(Color background); // Creates a GDI copy of the GDI+ bitmap. If the image has transparent pixels those will be painted on the specified color. (Only bitmaps with format PixelFormat32bppARGB is supported for transparency.) Specify clNone or a color with 0 alpha to return the image as is. If the background color's alpha is not 0, its opaque color will be used. It is your responsibility to delete the handle.
    };


    // Helper structures and functions for handling windows icons.
#pragma pack(push, 2)
    // Icon directory header at the top of a group icon resource or icon file data.
    struct IconDir
    {
        WORD reserved; // Must be 0.
        WORD icon; // Must be 1. 0 would mean cursor data.
        WORD entrycnt;  // Number of group icon entries in the data.
    };

    // Structure for the common part of a single entry in an icon or icon group data. When groupentry is true, the data in the structure is for a GroupIconDirEntry.
    struct IconDirEntry
    {
        BYTE width; // Width of icon in pixels.
        BYTE height; // Height of icon in pixels.
        BYTE colorcnt; // Number of colors in the bitmap data when it is less than 8bpp.
        BYTE reserved; // Must be 0.
        WORD planes; // Always 0 or 1 but never used for anything.
        WORD bitcnt; // Bits per pixel.
        DWORD datasize; // Size of the data for the icon entry not counting this header.
        DWORD dataoffset; // Offset of the data from the beginning of the icon file.
    };

    struct GroupIconDirEntry
    {
        BYTE width; // Width of icon in pixels.
        BYTE height; // Height of icon in pixels.
        BYTE colorcnt; // Number of colors in the bitmap data when it is less than 8bpp.
        BYTE reserved; // Must be 0.
        WORD planes; // Always 0 or 1 but never used for anything.
        WORD bitcnt; // Bits per pixel.
        DWORD datasize; // Size of the data for the icon entry not counting this header.
        WORD resid; // ID of the data in the resource of the executable or DLL. Loaded with LoadResource and RT_ICON.
    };
#pragma pack(pop)

    union IconEntry
    {
        IconDirEntry icondirentry;
        GroupIconDirEntry groupicondirentry;
    };

    void FillIconEntryVector(const char *data, unsigned int datalen, std::vector<IconEntry> &result, bool icongroup); // Adds IconEntry structures to a vector from the passed icon file data or group icon resource. The value of icongroup specifies whether the data is from a group icon resource. The data must be large enough for at least one entry or nothing is added. The entries are not checked for validity.
    void FillIconEntryVector(const char *data, unsigned int datalen, std::list<IconEntry> &result, bool icongroup); // Adds IconEntry structures to a list from the passed icon file data or group icon resource. The value of icongroup specifies whether the data is from a group icon resource. The data must be large enough for at least one entry or nothing is added. The entries are not checked for validity.
    int FindNearestIconEntry(std::vector<IconEntry> &result, int maxwidth, int maxheight, int maxcoldepth, bool icongroup); // Returns the index of an icon entry which best matches the given pixel dimensions and color depth. The dimensions below or nearest the max sizes take priority, then the color depth which is below or if not possible, nearest to the max color depth.
    int FindNearestIconEntry(std::list<IconEntry> &result, int maxwidth, int maxheight, int maxcoldepth, bool icongroup); // Returns the index of an icon entry which best matches the given pixel dimensions and color depth. The dimensions below or nearest the max sizes take priority, then the color depth which is below or if not possible, nearest to the max color depth.

    class Icon
    {
    private:
        HICON handle; // Handle of the icon. It is destroyed with the Icon object, so no shared handles are allowed.
        BYTE width; // Width of icon in pixels. 0 means 256 pixels.
        BYTE height; // Height of icon in pixels. 0 means 256 pixels.

        // Prevent copying.
#ifdef _MSC_VER
        Icon& operator=(const Icon&) {}
        Icon(const Icon&) {}
#else
        Icon& operator=(const Icon&) = delete;
        Icon(const Icon&) = delete;
#endif
    protected:
    public:
        static Icon&& MoveCopy(const Icon &orig); // Use instead of a copy constructor to clone data of another icon. The original is not modified in any way. (The copy constructor could be mistakenly used in stl containers etc.)
        static Icon* CreateCopy(const Icon &orig); // Use instead of a copy constructor to clone data of another icon. (The copy constructor could be mistakenly used in stl containers etc.)

        Icon();
        Icon(HICON handle); // The object will destroy the handle when it is destroyed.
        Icon(Icon&&) noexcept;
        Icon& operator=(Icon&&) noexcept;
        ~Icon();

        HICON Handle(); // Use this with caution. The object owns this handle which is only valid during its lifetime.

        bool Empty() const; // Returns true if handle is 0.
        void Clear(); // Deletes the contained icon.

        void Copy(const Icon &src); // Copies the source Icon, duplicating its contents. If the copy is unsuccessful, this icon will be empty as a result.

        int Width() const;
        int Height() const;
        Size IconSize() const;

        bool ToBitmap(Bitmap &bmp) const;
        void Draw(Canvas *canvas, int x, int y) const;
        void Draw(Canvas *canvas, int x, int y, int width, int height) const;
    };

    void IconFromFile(const std::wstring &fname, std::vector<Icon> &result); // Loads icons from a file and adds them to the passed vector. If an error occurs or the file is not valid, no icons are added.
    void IconFromFile(const std::wstring &fname, std::list<Icon> &result); // Loads icons from a file and adds them to the passed list. If an error occurs or the file is not valid, no icons are added.
    void IconFromFile(FILE *f, std::vector<Icon> &result); // Loads icons from a file and adds them to the passed vector. If an error occurs or the file is not valid, no icons are added.
    void IconFromFile(FILE *f, std::list<Icon> &result); // Loads icons from a file and adds them to the passed list. If an error occurs or the file is not valid, no icons are added.
    void IconFromFile(std::istream &stream, std::vector<Icon> &result); // Loads icons from a file and adds them to the passed vector. If an error occurs or the file is not valid, no icons are added.
    void IconFromFile(std::istream &stream, std::list<Icon> &result); // Loads icons from a file and adds them to the passed list. If an error occurs or the file is not valid, no icons are added.


    float FontSizeFromLogfont(const LOGFONT &lf, HDC measuredc = NULL); // defined in screen.cpp
    float FontSizeFromHeight(int height); // defined in screen.cpp
    int FontHeightFromSize(float size); // defined in screen.cpp


}
/* End of NLIBNS */

