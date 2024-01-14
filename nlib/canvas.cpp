#include "stdafx_zoli.h"
#include "application.h"

#include "canvas.h"
#include "syscontrol.h"
#include "screen.h"
#include "utility.h"
#include "genericfile.h"

#ifdef DESIGNING
#include "property_controlbase.h"
#include "property_canvas.h"
#include "designproperties.h"
#include "serializer.h"
#endif


namespace NLIBNS
{


#ifdef DESIGNING

// Helper function for GeneralFontDesignProperty.
void FontFamilyAllowUIDefault(DesignSerializer* serializer)
{
    auto found = serializer->Find<FontFamilyDesignProperty<Font>>(L"Family");
    if (!found)
        throw L"Property name 'Family' changed in Font class?";
    found->AllowUIDefault();
}

ValuePair<FontCharacterSets> FontCharacterSetStrings[] = {
    VALUEPAIR(fcsAnsi),
    VALUEPAIR(fcsArabic),
    VALUEPAIR(fcsBaltic),
    VALUEPAIR(fcsChineseBig5),
    VALUEPAIR(fcsDefault),
    VALUEPAIR(fcsEastEurope),
    VALUEPAIR(fcsGB2312),
    VALUEPAIR(fcsGreek),
    VALUEPAIR(fcsHangul),
    VALUEPAIR(fcsHebrew),
    VALUEPAIR(fcsJohab),
    VALUEPAIR(fcsMac),
    VALUEPAIR(fcsOEM),
    VALUEPAIR(fcsRussian),
    VALUEPAIR(fcsShiftJIS),
    VALUEPAIR(fcsSymbol),
    VALUEPAIR(fcsThai),
    VALUEPAIR(fcsTurkish),
    VALUEPAIR(fcsVietnamese),
};

ValuePair<FontOutputQualities> FontOutputQualityStrings[] = {
    VALUEPAIR(foqAntialiased),
    VALUEPAIR(foqCleartype),
    VALUEPAIR(foqDefault),
    VALUEPAIR(foqDraft),
    VALUEPAIR(foqNonAntialiased),
    VALUEPAIR(foqProof),
};

#endif
//---------------------------------------------


Fonts* Fonts::instance = NULL;

Fonts* Fonts::GetInstance()
{
    if (!instance)
        instance = new Fonts();
    systemfonts = instance;
    return systemfonts;
}

void Fonts::FreeInstance()
{
    delete instance;
    instance = NULL;
    systemfonts = NULL;
}

bool _base_sortfontsproc__zapp(const std::wstring& a, const std::wstring& b)
{
    return a.compare(b) < 0;
}

int CALLBACK _base_enumfontsproc__zapp( const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam)
{
    auto list = (std::vector<std::wstring>*)lParam;
    list->push_back(lpelfe->lfFaceName);
    return 1;
}

Fonts::Fonts()
{
    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;

    HDC h = GetDC(0);
    EnumFontFamiliesExW(h, &lf, _base_enumfontsproc__zapp, (LPARAM)this, 0);
    ReleaseDC(0, h);

    std::sort(begin(), end(), _base_sortfontsproc__zapp);
    erase(std::unique(begin(), end()), end());
}

Fonts::~Fonts()
{
}


//---------------------------------------------


FontData::FontData(const std::wstring& family, float size, int escapement, int orientation, Color color, bool bold, bool italic, bool underline, bool strikeout, FontCharacterSets charset, FontOutputQualities quality) :
            family(family), size(size), escapement(escapement), orientation(orientation), color(color), bold(bold), italic(italic), underline(underline), strikeout(strikeout), charset(charset), quality(quality)
{
    ;
}

FontData::FontData() :
#ifdef DESIGNING
        family(L"Tahoma"),
#else
        family(L""),
#endif
        size(8.25F), escapement(0), orientation(0), color(clBtnText), bold(false), italic(false), underline(false), strikeout(false), charset(fcsDefault), quality(foqDefault)
{
    ;
}

FontData::FontData(const LOGFONT &lf)
{
    family = lf.lfFaceName;
    size = lf.lfHeight < 0 ? FontSizeFromHeight(lf.lfHeight) : FontSizeFromLogfont(lf);
    escapement = lf.lfEscapement;
    orientation = lf.lfOrientation;
    bold = lf.lfWeight >= FW_BOLD;
    italic = lf.lfItalic == TRUE;
    underline = lf.lfUnderline == TRUE;
    strikeout = lf.lfStrikeOut == TRUE;
    charset = (FontCharacterSets)lf.lfCharSet;
    quality = (FontOutputQualities)lf.lfQuality;
    color = clBtnText;
}

bool FontData::operator==(const FontData &other) const
{
    return bold == other.bold &&
           italic == other.italic &&
           underline == other.underline &&
           strikeout == other.strikeout &&
           size == other.size && family == other.family &&
           escapement == other.escapement &&
           orientation == other.orientation &&
           charset == other.charset &&
           quality == other.quality &&
           color == other.color;
}

bool FontData::operator!=(const FontData &other) const
{
    return !(operator==(other));
}

FontData::operator LOGFONT() const
{
    LOGFONT lf;
#ifdef DESIGNING
    if (family.empty())
        wcscpy(lf.lfFaceName, application->UILogFont().lfFaceName);
    else
        wcscpy(lf.lfFaceName, family.substr(0, 31).c_str());
#else
    wcscpy(lf.lfFaceName, family.c_str());
#endif
    lf.lfHeight = LONG(FontHeightFromSize(size));
    lf.lfWidth = 0;
    lf.lfEscapement = escapement;
    lf.lfOrientation = orientation;
    lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = italic ? TRUE : FALSE;
    lf.lfUnderline = underline ? TRUE : FALSE;
    lf.lfStrikeOut = strikeout ? TRUE : FALSE;
    lf.lfCharSet = charset;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = quality;
    lf.lfPitchAndFamily = DEFAULT_PITCH;

    return lf;
}


//---------------------------------------------


#ifdef DESIGNING
const ValuePair<float> propfontsizes[] = {
    make_ValuePair<float>(6, L"6"),
    make_ValuePair<float>(7, L"7"),
    make_ValuePair<float>(8, L"8"),
    make_ValuePair<float>(9, L"9"),
    make_ValuePair<float>(10, L"10"),
    make_ValuePair<float>(11, L"11"),
    make_ValuePair<float>(12, L"12"),
    make_ValuePair<float>(13, L"13"),
    make_ValuePair<float>(15, L"15"),
    make_ValuePair<float>(16, L"16"),
    make_ValuePair<float>(17, L"17"),
    make_ValuePair<float>(18, L"18"),
    make_ValuePair<float>(20, L"20"),
    make_ValuePair<float>(22, L"22"),
    make_ValuePair<float>(24, L"24"),
    make_ValuePair<float>(26, L"26"),
    make_ValuePair<float>(28, L"28"),
    make_ValuePair<float>(36, L"36"),
    make_ValuePair<float>(48, L"48"),
    make_ValuePair<float>(72, L"72"),
};
const int propfontsizes_cnt = 20;

void Font::EnumerateProperties(DesignSerializer *serializer)
{
    serializer->Add(L"SetFamily", new FontFamilyDesignProperty<Font>(L"Family", std::wstring(), &Font::Family, &Font::SetFamily));
    serializer->Add(L"SetColor", new ColorDesignProperty<Font>(L"Color", std::wstring(), true, false, false, &Font::GetColor, &Font::SetColor))->SetDefault(clBtnText);
    serializer->Add(L"SetSize", new FontSizeDesignProperty<Font>(L"Size", std::wstring(), &Font::Size, &Font::SetSize))->SetDefault(&Font::DefaultSize);
    serializer->Add(L"SetEscapement", new IntDesignProperty<Font>(L"Escapement", std::wstring(), &Font::Escapement, &Font::SetEscapement))->SetDefault(0);
    serializer->Add(L"SetOrientation", new IntDesignProperty<Font>(L"Orientation", std::wstring(), &Font::Orientation, &Font::SetOrientation))->SetDefault(0);
    serializer->Add(L"SetBold", new BoolDesignProperty<Font>(L"Bold", std::wstring(), &Font::Bold, &Font::SetBold))->SetDefault(false);
    serializer->Add(L"SetItalic", new BoolDesignProperty<Font>(L"Italic", std::wstring(), &Font::Italic, &Font::SetItalic))->SetDefault(false);
    serializer->Add(L"SetStrikeout", new BoolDesignProperty<Font>(L"Strikeout", std::wstring(), &Font::Strikeout, &Font::SetStrikeout))->SetDefault(false);
    serializer->Add(L"SetUnderline", new BoolDesignProperty<Font>(L"Underline", std::wstring(), &Font::Underline, &Font::SetUnderline))->SetDefault(false);
    serializer->Add(L"SetCharacterSet", new FontCharacterSetsDesignProperty<Font>(L"CharacterSet", std::wstring(), &Font::CharacterSet, &Font::SetCharacterSet))->SetDefault(fcsDefault);
    serializer->Add(L"SetOutputQuality", new FontOutputQualitiesDesignProperty<Font>(L"OutputQuality", std::wstring(), &Font::OutputQuality, &Font::SetOutputQuality))->SetDefault(foqDefault);
}

float Font::DefaultSize()
{
    return round(application->UIFont().Size());
}
#endif

Font::Font() : handle(NULL)
{
}

Font::Font(const FontData &data) : handle(NULL), data(data)
{
}

Font::Font(const Font &orig) : handle(NULL)
{
    *this = orig;
}

Font::Font(Font &&orig) noexcept : handle(NULL)
{
    *this = std::move(orig);
}

Font::Font(const std::wstring& family, float size, int escapement, int orientation, Color color, bool bold, bool italic, bool underline, bool strikeout, FontCharacterSets charset, FontOutputQualities quality) :
        handle(NULL), data(family, size, escapement, orientation, color, bold, italic, underline, strikeout, charset, quality)
{
}

Font::~Font()
{
    if (handle)
        DeleteObject(handle);
}

Font& Font::operator=(Font &&b) noexcept
{
    Changing();
    std::swap(data, b.data);
    std::swap(handle, b.handle);
    Change(false);

    return *this;
}

Font& Font::operator=(const Font &b)
{
    if (data == b.data)
        return *this;

    bool recreate = handle != 0;
    if (recreate)
        Changing();
    data = b.data;
    Change(recreate);

    return *this;
}

Font& Font::operator= (const FontData &newdata)
{
    if (data == newdata)
        return *this;

    bool recreate = handle != 0;
    if (recreate)
        Changing();
    data = newdata;
    Change(recreate);

    return *this;
}

const FontData& Font::Data() const
{
    return data;
}

bool Font::operator==(const Font &b) const
{
    return data == b.data;
}

bool Font::operator==(const FontData &fdata) const
{
    return data == fdata;
}

bool Font::operator!=(const Font &b) const
{
    return data != b.data;
}

bool Font::operator!=(const FontData &fdata) const
{
    return data != fdata;
}

LOGFONT Font::FillLogfont()
{
    if (data.family.size() > 31)
        throw L"Font family size name can't exceed 32 characters with terminating null.";

    LOGFONT lf(data);
    return lf;
}

void Font::Create()
{
    if (handle)
        throw L"Font not destroyed!";

    LOGFONT lf = FillLogfont();
    handle = CreateFontIndirect(&lf);

    if (handle)
    {
        GetObject(handle, sizeof(LOGFONT), &lf);
#ifdef DESIGNING
        if (!data.family.empty())
            data.family = lf.lfFaceName;
#else
        data.family = lf.lfFaceName;
#endif
        data.size = FontSizeFromHeight(lf.lfHeight);
        data.escapement = lf.lfEscapement;
        data.orientation = lf.lfOrientation;
        data.bold = lf.lfWeight >= FW_BOLD;
        data.italic = lf.lfItalic == TRUE;
        data.underline = lf.lfUnderline == TRUE;
        data.strikeout = lf.lfStrikeOut == TRUE;
        data.charset = (FontCharacterSets)lf.lfCharSet;
        data.quality = (FontOutputQualities)lf.lfQuality;
    }
}

void Font::Change(bool recreate)
{
    if (recreate)
    {
        DeleteObject(handle);
        handle = NULL;
        Create();
    }
    Changed();
}

HFONT Font::Handle()
{
    if (!handle)
        Create();
    return handle;
}

bool Font::HandleCreated()
{
    return handle != 0;
}

const std::wstring& Font::Family() const
{
    return data.family;
}

void Font::SetFamily(const std::wstring& newfamily)
{
    if (newfamily.size() > 31)
        throw L"Font family size name can't exceed 32 characters with terminating null.";

#ifndef DESIGNING
    if (newfamily.empty())
        return;
#endif

    if (data.family == newfamily)
        return;
    Changing();
    data.family = newfamily;
    Change(handle != 0);
}

//float Font::Size()
//{
//    return data.size;
//}

float Font::Size() const
{
    return data.size;
}

void Font::SetSize(float newsize)
{
    if (data.size == newsize)
        return;
    Changing();
    data.size = newsize;
    Change(handle != 0);
}

int Font::Height() const
{
    return FontHeightFromSize(data.size);
}

void Font::SetHeight(int newheight)
{
    float s = FontSizeFromHeight(newheight);
    if (data.size == s)
        return;
    Changing();
    data.size = s;
    Change(handle != 0);
}

int Font::Escapement() const
{
    return data.escapement;
}

void Font::SetEscapement(int newescapement)
{
    if (data.escapement == newescapement)
        return;
    Changing();
    data.escapement = newescapement;
    Change(handle != 0);
}

int Font::Orientation() const
{
    return data.orientation;
}

void Font::SetOrientation(int neworientation)
{
    if (data.orientation == neworientation)
        return;
    Changing();
    data.orientation = neworientation;
    Change(handle != 0);
}

void Font::SetAngle(int newangle)
{
    if (data.escapement == newangle && data.orientation == newangle)
        return;
    Changing();
    data.escapement = newangle;
    data.orientation = newangle;
    Change(handle != 0);
}

Color Font::GetColor() const
{
    return data.color;
}

void Font::SetColor(Color newcolor)
{
    newcolor = newcolor.Solid();
    if (data.color == newcolor)
        return;
    Changing();
    data.color = newcolor;
    Change(handle != 0);
}

bool Font::Bold() const
{
    return data.bold;
}

void Font::SetBold(bool newbold)
{
    if (data.bold == newbold)
        return;
    Changing();
    data.bold = newbold;
    Change(handle != 0);
}

bool Font::Italic() const
{
    return data.italic;
}

void Font::SetItalic(bool newitalic)
{
    if (data.italic == newitalic)
        return;
    Changing();
    data.italic = newitalic;
    Change(handle != 0);
}

bool Font::Underline() const
{
    return data.underline;
}

void Font::SetUnderline(bool newunderline)
{
    if (data.underline == newunderline)
        return;
    Changing();
    data.underline = newunderline;
    Change(handle != 0);
}

bool Font::Strikeout() const
{
    return data.strikeout;
}

void Font::SetStrikeout(bool newstrikeout)
{
    if (data.strikeout == newstrikeout)
        return;
    Changing();
    data.strikeout = newstrikeout;
    Change(handle != 0);
}

FontCharacterSets Font::CharacterSet() const
{
    return data.charset;
}

void Font::SetCharacterSet(FontCharacterSets newcharset)
{
    if (data.charset == newcharset)
        return;
    Changing();
    data.charset = newcharset;
    Change(handle != 0);
}

FontOutputQualities Font::OutputQuality() const
{
    return data.quality;
}

void Font::SetOutputQuality(FontOutputQualities newquality)
{
    if (data.quality == newquality)
        return;
    Changing();
    data.quality = newquality;
    Change(handle != 0);
}


//---------------------------------------------


OwnedFont::OwnedFont() : changing(false)
{
}

OwnedFont::OwnedFont(const FontData &data) : base(data), changing(false), saved(data)
{
}

void OwnedFont::Changing() noexcept
{
    changing = true;
    saved = FontData(Family(), Size(), Escapement(), Orientation(), Color(), Bold(), Italic(), Underline(), Strikeout(), CharacterSet(), OutputQuality());
}

void OwnedFont::Changed() noexcept
{
    base::Changed();

    if (changing)
        DoChanged(saved);
    changing = false;
}


//---------------------------------------------


Brush::Brush() : brush(NULL), data(NULL), storedata(false), type(btColorBrush)
{
    brush = new Gdiplus::SolidBrush(clBlack);
}

Brush::Brush(const Color &color) : brush(NULL), data(NULL), storedata(false), type(btColorBrush)
{
    brush = new Gdiplus::SolidBrush(color);
}

Brush::Brush(const Rect &rect, const Color &c1, const Color &c2, float angle, bool anglescalable, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(false), type(btLinearGradientBrush)
{
    if (wrapmode == bwmNoTile)
        throw L"The linear gradient brush can't be not tiled!";

    Gdiplus::LinearGradientBrush *lgb = new Gdiplus::LinearGradientBrush(rect, c1, c2, angle, anglescalable ? TRUE : FALSE);
    brush = lgb;
    lgb->SetWrapMode((Gdiplus::WrapMode)wrapmode);
}

Brush::Brush(const RectF &rect, const Color &c1, const Color &c2, float angle, bool anglescalable, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(false), type(btLinearGradientBrush)
{
    if (wrapmode == bwmNoTile)
        throw L"The linear gradient brush can't be not tiled!";

    Gdiplus::LinearGradientBrush *lgb = new Gdiplus::LinearGradientBrush(rect, c1, c2, angle, anglescalable ? TRUE : FALSE);
    brush = lgb;
    lgb->SetWrapMode((Gdiplus::WrapMode)wrapmode);
}

Brush::Brush(Bitmap *abmp, bool storebmp, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(storebmp), type(btTextureBrush)
{
    if (!abmp)
        throw L"Bitmap argument is NULL.";

    TextureData *tdata = new TextureData;
    data = tdata;
    if (!storebmp)
        tdata->bmp = abmp;
    else
        tdata->bmp = Bitmap::CreateCopy(*abmp);
    brush = new Gdiplus::TextureBrush(tdata->bmp->GetBitmap(), (Gdiplus::WrapMode)wrapmode);
}

Brush::Brush(Bitmap *abmp, Rect r, bool storebmp, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(storebmp), type(btTextureBrush)
{
    if (!abmp)
        throw L"Bitmap argument is NULL.";

    TextureData *tdata = new TextureData;
    data = tdata;
    if (r.right > abmp->Width())
        r.right = abmp->Width();
    if (r.bottom > abmp->Height())
        r.bottom = abmp->Height();
    if (r.right <= r.left || r.bottom <= r.top)
        throw L"Invalid argument, the rectangle lies outside the passed bitmap!";

    if (!storebmp)
        tdata->bmp = abmp;
    else
        tdata->bmp = new Bitmap(*abmp, r);
    brush = new Gdiplus::TextureBrush(tdata->bmp->GetBitmap(), (Gdiplus::WrapMode)wrapmode);
}

Brush::Brush(const Color &linecolor, const Color &bgcolor, Gdiplus::HatchStyle hatchstyle) : brush(NULL), data(NULL), storedata(false), type(btHatchBrush)
{
    brush = new Gdiplus::HatchBrush(hatchstyle, linecolor, bgcolor);
}

Brush::Brush(Gdiplus::GraphicsPath *apath, const Color &color, bool copypath, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(copypath), type(btPathGradientBrush)
{
    if (!apath)
        throw L"Path argument is NULL.";

    PathData *pdata = new PathData();
    data = pdata;
    if (!copypath)
        pdata->path = apath;
    else
        pdata->path = apath->Clone();
    Gdiplus::PathGradientBrush *pgb = new Gdiplus::PathGradientBrush(pdata->path);
    brush = pgb;
    pgb->SetWrapMode((Gdiplus::WrapMode)wrapmode);
    pgb->SetCenterColor(color);
}

Brush::Brush(Gdiplus::GraphicsPath *apath, const Color &centercolor, Color *colors, int colorcnt, bool copypath, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(copypath), type(btPathGradientBrush)
{
    if (!apath)
        throw L"Path argument is NULL.";

    PathData *pdata = new PathData();
    data = pdata;
    if (!copypath)
        pdata->path = apath;
    else
        pdata->path = apath->Clone();
    Gdiplus::PathGradientBrush *pgb = new Gdiplus::PathGradientBrush(pdata->path);
    brush = pgb;
    pgb->SetWrapMode((Gdiplus::WrapMode)wrapmode);
    pgb->SetCenterColor(centercolor);

    pdata->colorcnt = colorcnt;
    if (colorcnt)
    {
        pdata->colors = new Gdiplus::Color[colorcnt];
        for (int ix = 0; ix < colorcnt; ++ix)
            pdata->colors[ix] = colors[ix];

        pgb->SetSurroundColors(pdata->colors, &colorcnt);
    }
}

Brush::Brush(Gdiplus::GraphicsPath *apath, const Color &centercolor, Gdiplus::Color *colors, int colorcnt, bool copypath, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(copypath), type(btPathGradientBrush)
{
    if (!apath)
        throw L"Path argument is NULL.";

    PathData *pdata = new PathData();
    data = pdata;
    if (!copypath)
        pdata->path = apath;
    else
        pdata->path = apath->Clone();
    Gdiplus::PathGradientBrush *pgb = new Gdiplus::PathGradientBrush(pdata->path);
    brush = pgb;
    pgb->SetWrapMode((Gdiplus::WrapMode)wrapmode);
    pgb->SetCenterColor(centercolor);

    pdata->colorcnt = colorcnt;
    if (colorcnt)
    {
        pdata->colors = new Gdiplus::Color[colorcnt];
        for (int ix = 0; ix < colorcnt; ++ix)
            pdata->colors[ix] = colors[ix];

        pgb->SetSurroundColors(pdata->colors, &colorcnt);
    }
}

Brush::Brush(Point *apoints, int pointcount, const Color &color, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(true), type(btPathGradientBrush)
{
    if (!apoints)
        throw L"Path argument is NULL.";

    PathData *pdata = new PathData();
    data = pdata;
    pdata->points = new Gdiplus::Point[pointcount];
    pdata->pointcnt = pointcount;
    for (int ix = 0; ix < pointcount; ++ix)
        pdata->points[ix] = apoints[ix];
    Gdiplus::PathGradientBrush *pgb = new Gdiplus::PathGradientBrush(pdata->points, pointcount, (Gdiplus::WrapMode)wrapmode);
    brush = pgb;
    pgb->SetCenterColor(color);
}

Brush::Brush(PointF *apoints, int pointcount, const Color &color, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(true), type(btPathGradientBrush)
{
    if (!apoints)
        throw L"Path argument is NULL.";

    PathData *pdata = new PathData();
    data = pdata;
    pdata->pointsF = new Gdiplus::PointF[pointcount];
    pdata->pointcnt = pointcount;
    for (int ix = 0; ix < pointcount; ++ix)
        pdata->pointsF[ix] = apoints[ix];
    Gdiplus::PathGradientBrush *pgb = new Gdiplus::PathGradientBrush(pdata->pointsF, pointcount, (Gdiplus::WrapMode)wrapmode);
    brush = pgb;
    pgb->SetCenterColor(color);
}

Brush::Brush(Point *apoints, int pointcount, const Color &centercolor, Color *colors, int colorcnt, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(true), type(btPathGradientBrush)
{
    if (!apoints)
        throw L"Path argument is NULL.";

    PathData *pdata = new PathData();
    data = pdata;
    pdata->points = new Gdiplus::Point[pointcount];
    pdata->pointcnt = pointcount;
    for (int ix = 0; ix < pointcount; ++ix)
        pdata->points[ix] = apoints[ix];
    Gdiplus::PathGradientBrush *pgb = new Gdiplus::PathGradientBrush(pdata->points, pointcount, (Gdiplus::WrapMode)wrapmode);
    brush = pgb;
    pgb->SetCenterColor(centercolor);

    pdata->colorcnt = colorcnt;
    if (colorcnt)
    {
        pdata->colors = new Gdiplus::Color[colorcnt];
        for (int ix = 0; ix < colorcnt; ++ix)
            pdata->colors[ix] = colors[ix];

        pgb->SetSurroundColors(pdata->colors, &colorcnt);
    }
}

Brush::Brush(PointF *apoints, int pointcount, const Color &centercolor, Gdiplus::Color *colors, int colorcnt, BrushWrapModes wrapmode) : brush(NULL), data(NULL), storedata(true), type(btPathGradientBrush)
{
    if (!apoints)
        throw L"Path argument is NULL.";

    PathData *pdata = new PathData();
    data = pdata;
    pdata->pointsF = new Gdiplus::PointF[pointcount];
    pdata->pointcnt = pointcount;
    for (int ix = 0; ix < pointcount; ++ix)
        pdata->pointsF[ix] = apoints[ix];
    Gdiplus::PathGradientBrush *pgb = new Gdiplus::PathGradientBrush(pdata->pointsF, pointcount, (Gdiplus::WrapMode)wrapmode);
    brush = pgb;
    pgb->SetCenterColor(centercolor);

    pdata->colorcnt = colorcnt;
    if (colorcnt)
    {
        pdata->colors = new Gdiplus::Color[colorcnt];
        for (int ix = 0; ix < colorcnt; ++ix)
            pdata->colors[ix] = colors[ix];

        pgb->SetSurroundColors(pdata->colors, &colorcnt);
    }
}

Brush::Brush(const Brush &orig) : brush(NULL), data(NULL), storedata(false), type(btColorBrush)
{
    *this = orig;
}

Brush::Brush(Brush &&orig) noexcept : brush(NULL), data(NULL), storedata(false), type(btColorBrush)
{
    *this = std::move(orig);
}

Brush::~Brush()
{
    if (storedata)
    {
        if (type == btTextureBrush)
        {
            TextureData *tdata = (TextureData*)data;
            delete tdata->bmp;
            delete tdata;
        }
        else if (type == btPathGradientBrush)
        {
            PathData *pdata = (PathData*)data;
            delete pdata->path;
            delete pdata->points;
            delete pdata->pointsF;
            delete pdata->colors;
            delete pdata;
        }
    }
    delete brush;
}

BrushTypes Brush::Type() const
{
    return type;
}

BrushWrapModes Brush::GetWrapMode() const
{
    if (type == btColorBrush || type == btHatchBrush)
        return bwmTile;
    if (type == btTextureBrush)
    {
        Gdiplus::TextureBrush *tb = (Gdiplus::TextureBrush*)brush;
        return (BrushWrapModes)tb->GetWrapMode();
    }
    if (type == btLinearGradientBrush)
    {
        Gdiplus::LinearGradientBrush *lgb = (Gdiplus::LinearGradientBrush*)brush;
        return (BrushWrapModes)lgb->GetWrapMode();
    }
    if (type == btPathGradientBrush)
    {
        Gdiplus::PathGradientBrush *pgb = (Gdiplus::PathGradientBrush*)brush;
        return (BrushWrapModes)pgb->GetWrapMode();
    }

    throw L"Invalid type";
}

void Brush::SetWrapMode(BrushWrapModes newwrapmode)
{
    if (type == btColorBrush || type == btHatchBrush || (type == btLinearGradientBrush && newwrapmode == bwmNoTile))
        return;
    if (type == btTextureBrush)
    {
        Gdiplus::TextureBrush *tb = (Gdiplus::TextureBrush*)brush;
        tb->SetWrapMode((Gdiplus::WrapMode)newwrapmode);
    }
    if (type == btLinearGradientBrush)
    {
        Gdiplus::LinearGradientBrush *lgb = (Gdiplus::LinearGradientBrush*)brush;
        lgb->SetWrapMode((Gdiplus::WrapMode)newwrapmode);
    }
    if (type == btPathGradientBrush)
    {
        Gdiplus::PathGradientBrush *pgb = (Gdiplus::PathGradientBrush*)brush;
        pgb->SetWrapMode((Gdiplus::WrapMode)newwrapmode);
    }
}

HatchStyle Brush::GetHatchStyle() const
{
    if (type != btHatchBrush)
        return Gdiplus::HatchStyleTotal;
    Gdiplus::HatchBrush *hb = (Gdiplus::HatchBrush*)brush;
    return hb->GetHatchStyle();
}

void Brush::SetHatchStyle(HatchStyle newhatchstyle)
{
    if (type != btHatchBrush)
        return;
    Color c1 = GetColor();
    Color c2 = SecondaryColor();
    delete brush;
    brush = new Gdiplus::HatchBrush(newhatchstyle, c1, c2);
}

Brush& Brush::operator=(Brush &&b) noexcept
{
    std::swap(brush, b.brush);
    std::swap(data, b.data);
    std::swap(storedata, b.storedata);
    std::swap(type, b.type);

    return *this;
}

Brush& Brush::operator=(const Brush &b)
{
    if (&b == this)
        return *this;

    if (storedata)
    {
        if (type == btTextureBrush)
        {
            TextureData *tdata = (TextureData*)data;
            delete tdata->bmp;
            delete tdata;
        }
        else if (type == btPathGradientBrush)
        {
            PathData *pdata = (PathData*)data;
            delete pdata->path;
            delete pdata->points;
            delete pdata->pointsF;
            delete pdata->colors;
            delete pdata;
        }
    }
    delete brush;
    data = NULL;

    type = b.type;
    storedata = b.storedata;

    if ((type != btTextureBrush && type != btPathGradientBrush) || !storedata)
        brush = b.Object().Clone();
    if (type == btTextureBrush)
    {
        TextureData *tdata = new TextureData;
        TextureData *tdatab = (TextureData*)b.data;
        data = tdata;
        if (!storedata)
            *tdata = *(TextureData*)b.data;
        else
        {
            tdata->bmp = Bitmap::CreateCopy(*tdatab->bmp);
            Gdiplus::TextureBrush *tb = new Gdiplus::TextureBrush(tdata->bmp->GetBitmap(), (Gdiplus::WrapMode)b.GetWrapMode());
            brush = tb;

            //Gdiplus::TextureBrush *tbb = (Gdiplus::TextureBrush*)&b.Object();
            //if (tbb->GetTransform(&tdata->mx) == Gdiplus::Ok)
            //    tb->SetTransform(&tdata->mx);
        }
    }
    else if (type == btPathGradientBrush)
    {
        PathData *pdata = new PathData();
        PathData *pdatab = (PathData*)b.data;
        data = pdata;
        if (!storedata)
            *pdata = *(PathData*)b.data;
        else
        {
            Gdiplus::PathGradientBrush *pgb;
            if (pdatab->path)
            {
                pdata->path = pdatab->path->Clone();
                brush = pgb = new Gdiplus::PathGradientBrush(pdata->path);
                pgb->SetWrapMode((Gdiplus::WrapMode)b.GetWrapMode());
            }
            else if (pdatab->points)
            {
                pdata->pointcnt = pdatab->pointcnt;
                pdata->points = new Gdiplus::Point[pdata->pointcnt];
                for (int ix = 0; ix < pdata->pointcnt; ++ix)
                    pdata->points[ix] = pdatab->points[ix];
                brush = pgb = new Gdiplus::PathGradientBrush(pdata->points, pdata->pointcnt, (Gdiplus::WrapMode)b.GetWrapMode());
            }
            else if (pdatab->pointsF)
            {
                pdata->pointcnt = pdatab->pointcnt;
                pdata->pointsF = new Gdiplus::PointF[pdata->pointcnt];
                for (int ix = 0; ix < pdata->pointcnt; ++ix)
                    pdata->pointsF[ix] = pdatab->pointsF[ix];
                brush = pgb = new Gdiplus::PathGradientBrush(pdata->pointsF, pdata->pointcnt, (Gdiplus::WrapMode)b.GetWrapMode());
            }
            pgb->SetCenterColor(b.GetColor());
            if (pdatab->colors)
            {
                pdata->colorcnt = pdatab->colorcnt;
                pdata->colors = new Gdiplus::Color[pdata->colorcnt];
                for (int ix = 0; ix < pdata->colorcnt; ++ix)
                    pdata->colors[ix] = pdatab->colors[ix];

                int colorcnt = pdata->colorcnt;
                pgb->SetSurroundColors(pdata->colors, &colorcnt);
            }

            //Gdiplus::PathGradientBrush *pgbb = (Gdiplus::PathGradientBrush*)&b.Object();
            //if (pgbb->GetTransform(&pdata->mx) == Gdiplus::Ok)
            //    pgb->SetTransform(&pdata->mx);
        }
    }

    return *this;
}

Gdiplus::Brush& Brush::Object() const
{
    return *brush;
}

Color Brush::GetColor() const
{
    Gdiplus::Color res;
    Gdiplus::Color res2[2];
    switch(type)
    {
    case btColorBrush:
        return ((Gdiplus::SolidBrush*)brush)->GetColor(&res) == Gdiplus::Ok ? res : clNone;
    case btHatchBrush:
        return ((Gdiplus::HatchBrush*)brush)->GetForegroundColor(&res) == Gdiplus::Ok ? res : clNone;
    case btLinearGradientBrush:
        return ((Gdiplus::LinearGradientBrush*)brush)->GetLinearColors(res2) == Gdiplus::Ok ? res2[0] : clNone;
    case btPathGradientBrush:
        return ((Gdiplus::PathGradientBrush*)brush)->GetCenterColor(&res) == Gdiplus::Ok ? res : clNone;
    default:
        return clNone;
    }
}

void Brush::SetColor(const Color &newcolor)
{
    Gdiplus::Color cols[2];
    HatchStyle hs;
    switch(type)
    {
    case btColorBrush:
        ((Gdiplus::SolidBrush*)brush)->SetColor(newcolor);
        break;
    case btHatchBrush:
        cols[1] = SecondaryColor();
        hs = GetHatchStyle();
        delete brush;
        brush = new Gdiplus::HatchBrush(hs, newcolor, cols[1]);
        break;
    case btLinearGradientBrush:
        cols[1] = SecondaryColor();
        ((Gdiplus::LinearGradientBrush*)brush)->SetLinearColors(newcolor, cols[1]);
        break;
    case btPathGradientBrush:
        ((Gdiplus::PathGradientBrush*)brush)->SetCenterColor(newcolor);
        break;
    default:
        break;
    }
}

Color Brush::SecondaryColor() const
{
    Gdiplus::Color res;
    Gdiplus::Color res2[2];
    switch(type)
    {
    case btHatchBrush:
        return ((Gdiplus::HatchBrush*)brush)->GetBackgroundColor(&res) == Gdiplus::Ok ? res : clNone;
    case btLinearGradientBrush:
        return ((Gdiplus::LinearGradientBrush*)brush)->GetLinearColors(res2) == Gdiplus::Ok ? res2[1] : clNone;
    default:
        return clNone;
    }
}

void Brush::SetSecondaryColor(const Color &newcolor)
{
    Gdiplus::Color cols[2];
    HatchStyle hs;
    switch(type)
    {
    case btHatchBrush:
        cols[0] = GetColor();
        hs = GetHatchStyle();
        delete brush;
        brush = new Gdiplus::HatchBrush(hs, cols[0], newcolor);
        break;
    case btLinearGradientBrush:
        cols[0] = GetColor();
        ((Gdiplus::LinearGradientBrush*)brush)->SetLinearColors(cols[0], newcolor);
        break;
    default:
        break;
    }
}

int Brush::SurroundColorCount() const
{
    if (type == btPathGradientBrush)
        return ((PathData*)data)->colorcnt;
    else
        return 0;
}

void Brush::SetSurroundColors(Gdiplus::Color *colors, int colorcnt)
{
    if (type != btPathGradientBrush)
        return;

    Gdiplus::PathGradientBrush *pgb = (Gdiplus::PathGradientBrush*)brush;
    PathData *pdata = (PathData*)data;

    delete pdata->colors;
    pdata->colors = NULL;
    pdata->colorcnt = colorcnt;

    if (colorcnt)
    {
        pdata->colors = new Gdiplus::Color[colorcnt];
        for (int ix = 0; ix < colorcnt; ++ix)
            pdata->colors[ix] = colors[ix];

        pgb->SetSurroundColors(pdata->colors, &colorcnt);
    }
}

void Brush::SetSurroundColors(Color *colors, int colorcnt)
{
    if (type != btPathGradientBrush)
        return;

    Gdiplus::PathGradientBrush *pgb = (Gdiplus::PathGradientBrush*)brush;
    PathData *pdata = (PathData*)data;

    delete pdata->colors;
    pdata->colors = NULL;
    pdata->colorcnt = colorcnt;

    if (colorcnt)
    {
        pdata->colors = new Gdiplus::Color[colorcnt];
        for (int ix = 0; ix < colorcnt; ++ix)
            pdata->colors[ix] = colors[ix];

        pgb->SetSurroundColors(pdata->colors, &colorcnt);
    }
}


//---------------------------------------------


Pen::Pen() : pen(NULL), brush(NULL), storedbrush(false), type(ptColorPen), dash(pdsNone), dashlengths(nullptr), dashcount(0), storeddash(false)
{
    pen = new Gdiplus::Pen(clWhite, 0);
}

Pen::Pen(const Pen &orig) : pen(NULL), brush(NULL), storedbrush(false), type(ptColorPen), dash(pdsNone), dashlengths(nullptr), dashcount(0), storeddash(false)
{
    *this = orig;
}

Pen::Pen(const Color &color, float width) : pen(NULL), brush(NULL), storedbrush(false), type(ptColorPen), dash(pdsNone), dashlengths(nullptr), dashcount(0), storeddash(false)
{
    pen = new Gdiplus::Pen(color, width);
}

Pen::Pen(const Color &color, PenDashStyles dashstyle, float width) : pen(NULL), brush(NULL), storedbrush(false), type(ptColorPen), dash(pdsNone), dashlengths(nullptr), dashcount(0), storeddash(false)
{
    dash = dashstyle == pdsCustom ? pdsNone : dashstyle;

    pen = new Gdiplus::Pen(color, width);
    if (dash != pdsNone)
        pen->SetDashStyle(dashstyle == pdsDash ? Gdiplus::DashStyleDash : dashstyle == pdsDot ? Gdiplus::DashStyleDot : dashstyle == pdsDashDot ? Gdiplus::DashStyleDashDot : Gdiplus::DashStyleDashDotDot);
}

Pen::Pen(Brush *abrush, float width, bool storedbrush) : pen(NULL), storedbrush(storedbrush), type(ptBrushPen), dash(pdsNone), dashlengths(nullptr), dashcount(0), storeddash(false)
{
    if (!storedbrush)
        brush = abrush;
    else
        brush = new Brush(*abrush);
    pen = new Gdiplus::Pen(&brush->Object(), width);
}

Pen::Pen(Pen &&other) noexcept : pen(NULL), brush(NULL), storedbrush(false), type(ptColorPen), dash(pdsNone), dashlengths(nullptr), dashcount(0), storeddash(false)
{
    *this = std::move(other);
}

Pen::~Pen()
{
    if (storedbrush)
        delete brush;
    if (storeddash)
        delete[] dashlengths;
    delete pen;
}

Pen& Pen::operator= (const Pen &orig)
{
    if (&orig == this)
        return *this;

    if (storedbrush)
        delete brush;
    if (storeddash)
        delete[] dashlengths;
    delete pen;

    type = orig.type;
    dash = orig.dash;
    dashcount = orig.dashcount;

    if (orig.type == ptColorPen)
    {
        pen = new Gdiplus::Pen(orig.GetColor(), orig.Width());
        storedbrush = false;
        brush = nullptr;

        if (dash == pdsCustom)
        {
            if (orig.storeddash && orig.dashcount)
            {
                storeddash = true;
                dashlengths = new Gdiplus::REAL[orig.dashcount];
                memcpy(dashlengths, orig.dashlengths, sizeof(Gdiplus::REAL) * orig.dashcount);
            }
            else
            {
                storeddash = false;
                if (!orig.dashcount)
                {
                    dash = pdsNone;
                    dashlengths = nullptr;
                }
                else
                    dashlengths = orig.dashlengths;
            }
        }
        else
        {
            storeddash = false;
            dashlengths = nullptr;
        }

        if (dash != pdsNone && dash != pdsCustom)
            pen->SetDashStyle(dash == pdsDash ? Gdiplus::DashStyleDash : dash == pdsDot ? Gdiplus::DashStyleDot : dash == pdsDashDot ? Gdiplus::DashStyleDashDot : Gdiplus::DashStyleDashDotDot);
        else if (dash == pdsCustom)
            pen->SetDashPattern(dashlengths, dashcount);
    }
    else
    {
        dashlengths = nullptr;
        storeddash = false;

        if (!orig.storedbrush)
        {
            pen = new Gdiplus::Pen(&orig.brush->Object(), orig.Width());
            storedbrush = false;
            brush = orig.brush;
        }
        else
        {
            storedbrush = true;
            brush = new Brush(*orig.brush);
            pen = new Gdiplus::Pen(&brush->Object(), orig.Width());
        }
    }

    return *this;
}

Pen& Pen::operator=(Pen &&other) noexcept
{
    std::swap(pen, other.pen);
    std::swap(brush, other.brush);
    std::swap(storedbrush, other.storedbrush);
    std::swap(type, other.type);
    std::swap(dash, other.dash);
    std::swap(dashlengths, other.dashlengths);
    std::swap(dashcount, other.dashcount);
    std::swap(storeddash, other.storeddash);
    return *this;
}

Gdiplus::Pen& Pen::Object() const
{
    return *pen;
}

PenTypes Pen::Type() const
{
    return type;
}

Brush& Pen::GetBrush() const
{
    return *brush;
}

void Pen::SetBrush(Brush *newbrush, bool astoredbrush)
{
    if (!newbrush || (type == ptBrushPen && !storedbrush && !astoredbrush && brush == newbrush))
    {
        if (!newbrush && brush)
        {
            float w = Width();
            Color c = GetColor();
            type = ptColorPen;
            if (storedbrush)
                delete brush;
            storedbrush = false;
            brush = nullptr;
            delete pen;
            pen = new Gdiplus::Pen(c, w);
        }
        return;
    }

    if (dash == pdsCustom)
    {
        if (storeddash)
            delete[] dashlengths;
        dashlengths = nullptr;
        dashcount = 0;
        storeddash = false;
    }
    dash = pdsNone;

    float w = Width();
    type = ptBrushPen;
    delete pen;
    if (storedbrush)
        delete brush;
    storedbrush = astoredbrush;
    if (!astoredbrush)
        brush = newbrush;
    else
        brush = new Brush(*newbrush);
    pen = new Gdiplus::Pen(&brush->Object(), w);
}

PenDashStyles Pen::DashStyle() const
{
    return dash;
}

void Pen::SetDashStyle(PenDashStyles newdash)
{
    Color c = GetColor();
    float w = Width();

    type = ptColorPen;
    if (storedbrush)
        delete brush;
    storedbrush = false;
    brush = nullptr;

    if (storeddash)
        delete[] dashlengths;
    delete pen;
    dashlengths = nullptr;
    dashcount = 0;


    dash = newdash == pdsCustom ? pdsNone : newdash;

    pen = new Gdiplus::Pen(c, w);

    if (dash != pdsNone)
        pen->SetDashStyle(dash == pdsDash ? Gdiplus::DashStyleDash : dash == pdsDot ? Gdiplus::DashStyleDot : dash == pdsDashDot ? Gdiplus::DashStyleDashDot : Gdiplus::DashStyleDashDotDot);
}

void Pen::SetCustomDashPattern(Gdiplus::REAL *lengths, int count, bool storedpattern)
{
    if (count == 0)
    {
        SetDashStyle(pdsNone);
        return;
    }
    if (!storedpattern && !storeddash && lengths == dashlengths && dashcount == count)
        return;

    Color c = GetColor();
    float w = Width();
    type = ptColorPen;
    if (storedbrush)
        delete brush;
    storedbrush = false;
    brush = nullptr;

    if (storeddash)
        delete[] dashlengths;
    delete pen;

    dash = pdsCustom;

    pen = new Gdiplus::Pen(c, w);

    storeddash = storedpattern;
    dashcount = count;
    if (!storeddash)
        dashlengths = lengths;
    else
    {
        dashlengths = new Gdiplus::REAL[count];
        memcpy(dashlengths, lengths, sizeof(Gdiplus::REAL) * count);
    }

    pen = new Gdiplus::Pen(c, w);
    pen->SetDashPattern(dashlengths, dashcount);
}

Color Pen::GetColor() const
{
    if (type == ptColorPen)
    {
        Gdiplus::Color col;
        if (pen->GetColor(&col) == Gdiplus::Ok)
            return col;
        return clNone;
    }
    return brush->GetColor();
}

void Pen::SetColor(const Color &newcolor)
{
    if (type == ptColorPen)
    {
        pen->SetColor(newcolor);
        return;
    }

    float w = Width();
    delete pen;
    if (storedbrush)
        delete brush;

    brush = NULL;
    storedbrush = false;
    type = ptColorPen;
    pen = new Gdiplus::Pen(newcolor, w);
}

float Pen::Width() const
{
    return pen->GetWidth();
}

void Pen::SetWidth(float newwidth)
{
    pen->SetWidth(newwidth);
}


//---------------------------------------------


std::vector<Brush*> Canvas::stockbrushes;
std::vector<Pen*> Canvas::stockpens;
std::vector<Font*> Canvas::stockfonts;

std::map<std::wstring, Brush*> Canvas::brushes;
std::map<std::wstring, Pen*> Canvas::pens;
std::map<std::wstring, Font*> Canvas::fonts;
int Canvas::instances = 0;


//---------------------------------------------


CanvasGraphicsState::CanvasGraphicsState(int id, Canvas *c, Gdiplus::GraphicsState gdistate) :
                instances(new int(1)), skipdelete(new bool(false)), id(id), owner(c),
                gdistate(gdistate), statefont(c->GetFont()), stateadvanced(c->AdvancedTextMode()), 
                statepen(c->GetPen()), statebrush(c->GetBrush()),
                usedefaultmatrix(c->usedefaultmatrix), defaultmatrix(c->defaultmatrix), defaultmatrixflag(c->defaultmatrixflag),
                usebitmapmatrix(c->usebitmapmatrix), bitmapmatrix(c->bitmapmatrix), bitmapmatrixflag(c->bitmapmatrixflag),
                usebrushmatrix(c->usebrushmatrix), brushmatrix(c->brushmatrix), brushmatrixflag(c->brushmatrixflag),
                usepenmatrix(c->usepenmatrix), penmatrix(c->penmatrix), penmatrixflag(c->penmatrixflag),
                usedefaultcolorkey(c->usedefaultcolorkey), defaultlokey(c->defaultlokey), defaulthikey(c->defaulthikey),
                usebitmapcolorkey(c->usebitmapcolorkey), bitmaplokey(c->bitmaplokey), bitmaphikey(c->bitmaphikey),
                usebrushcolorkey(c->usebrushcolorkey), brushlokey(c->brushlokey), brushhikey(c->brushhikey),
                usepencolorkey(c->usepencolorkey), penlokey(c->penlokey), penhikey(c->penhikey),
                textalign(c->textalign), intmode(c->intmode), pommode(c->pommode), antialias(c->antialias),
                usetransf(c->usetransf), transf(c->transf)
{
    if (c->UseImageAttributes())
        stateattrib = c->attrib.Clone();
    else
        stateattrib = NULL;
}

CanvasGraphicsState::CanvasGraphicsState(const CanvasGraphicsState &orig)
{
    *this = orig;
}

CanvasGraphicsState& CanvasGraphicsState::operator=(const CanvasGraphicsState &orig)
{
    gdistate = orig.gdistate;
    statefont = orig.statefont;
    stateadvanced = orig.stateadvanced;
    statepen = orig.statepen;
    statebrush = orig.statebrush;
    stateattrib = orig.stateattrib;
    textalign = orig.textalign;
    antialias = orig.antialias;
    intmode = orig.intmode;
    pommode = orig.pommode;

    defaultmatrix = orig.defaultmatrix;
    defaultmatrixflag = orig.defaultmatrixflag;
    usedefaultmatrix = orig.usedefaultmatrix;
    bitmapmatrix = orig.bitmapmatrix;
    bitmapmatrixflag = orig.bitmapmatrixflag;
    usebitmapmatrix = orig.usebitmapmatrix;
    brushmatrix = orig.brushmatrix;
    brushmatrixflag = orig.brushmatrixflag;
    usebrushmatrix = orig.usebrushmatrix;
    penmatrix = orig.penmatrix;
    penmatrixflag = orig.penmatrixflag;
    usepenmatrix = orig.usepenmatrix;
    usedefaultcolorkey = orig.usedefaultcolorkey;
    usebitmapcolorkey = orig.usebitmapcolorkey;
    usebrushcolorkey = orig.usebrushcolorkey;
    usepencolorkey = orig.usepencolorkey;
    defaultlokey = orig.defaultlokey;
    defaulthikey = orig.defaulthikey;
    bitmaplokey = orig.bitmaplokey;
    bitmaphikey = orig.bitmaphikey;
    brushlokey = orig.brushlokey;
    brushhikey = orig.brushhikey;
    penlokey = orig.penlokey;
    penhikey = orig.penhikey;
    usetransf = orig.usetransf;
    transf = orig.transf;

    id = orig.id;
    owner = orig.owner;;
    instances = orig.instances;
    skipdelete = orig.skipdelete;

    ++*instances;


    return *this;
}

CanvasGraphicsState::~CanvasGraphicsState()
{
    if (!--(*instances))
    {
        if (!*skipdelete)
            delete stateattrib;
        delete instances;
    }
}


//---------------------------------------------


Canvas::Canvas() : graphicsstatepos(0), brush(NULL), outerbrush(true), pen(NULL), outerpen(true), font(NULL), outerfont(true), advanced(true),
        saveddc(NULL), savedrgn(NULL), hasrgn(false), usedefaultmatrix(false), usebitmapmatrix(false), usebrushmatrix(false), usepenmatrix(false),
        usedefaultcolorkey(false), usebitmapcolorkey(false), usebrushcolorkey(false), usepencolorkey(false), usetransf(false),
        textalign(ctaLeft | ctaTop), intmode(imDefault), pommode(pomNone), antialias(false)
{
    if (!instances)
        FillStock();

    instances++;

    savedrgn = CreateRectRgn(0, 0, 0, 0);

    ResetPen();
    ResetBrush();
    ResetFont();
}

bool Canvas::UseImageAttributes()
{
    return usebitmapcolorkey != false || usebitmapmatrix != false || usebrushcolorkey != false || usebrushmatrix != false ||
            usedefaultcolorkey != false || usedefaultmatrix != false || usepencolorkey != false || usepenmatrix != false;
}

void Canvas::SetupImageAttributes()
{
    if (usedefaultcolorkey)
        attrib.SetColorKey(defaultlokey, defaulthikey, (Gdiplus::ColorAdjustType)catDefault);
    if (usebitmapcolorkey)
        attrib.SetColorKey(bitmaplokey, bitmaphikey, (Gdiplus::ColorAdjustType)catBitmap);
    if (usebrushcolorkey)
        attrib.SetColorKey(brushlokey, brushhikey, (Gdiplus::ColorAdjustType)catBrush);
    if (usepencolorkey)
        attrib.SetColorKey(penlokey, penhikey, (Gdiplus::ColorAdjustType)catPen);

    if (usedefaultmatrix)
        attrib.SetColorMatrix(&defaultmatrix, (Gdiplus::ColorMatrixFlags)defaultmatrixflag, (Gdiplus::ColorAdjustType)catDefault);
    if (usebitmapmatrix)
        attrib.SetColorMatrix(&bitmapmatrix, (Gdiplus::ColorMatrixFlags)bitmapmatrixflag, (Gdiplus::ColorAdjustType)catBitmap);
    if (usebrushmatrix)
        attrib.SetColorMatrix(&brushmatrix, (Gdiplus::ColorMatrixFlags)brushmatrixflag, (Gdiplus::ColorAdjustType)catBrush);
    if (usepenmatrix)
        attrib.SetColorMatrix(&penmatrix, (Gdiplus::ColorMatrixFlags)penmatrixflag, (Gdiplus::ColorAdjustType)catPen);
}

void Canvas::SetupTransformations()
{
    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        if (usetransf)
            g->SetTransform(&transf);
        else
            g->ResetTransform();
    }
}

Canvas::~Canvas()
{
    using namespace Gdiplus;
    instances--;

    DeleteObject(savedrgn);
    savedrgn = NULL;
    hasrgn = false;

    if (!outerpen)
        delete pen;
    if (!outerbrush)
        delete brush;
    if (!outerfont)
        delete font;

    if (!instances)
    {
        for (auto it = brushes.begin(); it != brushes.end(); it++)
            delete it->second;
        for (auto it = pens.begin(); it != pens.end(); it++)
            delete it->second;
        for (auto it = fonts.begin(); it != fonts.end(); it++)
            delete it->second;
        brushes.clear();
        pens.clear();
        fonts.clear();

        ClearStock();
    }
}

void Canvas::FillStock()
{
    // Stock Brushes:

    // sbWhite
    stockbrushes.push_back(new Brush(clWhite));

    // sbBlack
    stockbrushes.push_back(new Brush(clBlack));

    // sbWindow
    stockbrushes.push_back(new Brush(clWindow));

    // sbBtnFace
    stockbrushes.push_back(new Brush(clBtnFace));

    // sbBtnText
    stockbrushes.push_back(new Brush(clBtnText));

    // sb3DHighlight
    stockbrushes.push_back(new Brush(cl3DHighlight));

    // sb3DShadow
    stockbrushes.push_back(new Brush(cl3DShadow));

    // sbHighlight
    stockbrushes.push_back(new Brush(clHighlight));

    // sbHighlightText
    stockbrushes.push_back(new Brush(clHighlightText));

    // Stock Pens:

    // spWhite
    stockpens.push_back(new Pen(clWhite, 0));

    // spBlack
    stockpens.push_back(new Pen(clBlack, 0));

    // spWindow
    stockpens.push_back(new Pen(clWindow, 0));

    // spBtnFace
    stockpens.push_back(new Pen(clBtnFace, 0));

    // spBtnText
    stockpens.push_back(new Pen(clBtnText, 0));

    // sp3DHighlight
    stockpens.push_back(new Pen(cl3DHighlight, 0));

    // sp3DShadow
    stockpens.push_back(new Pen(cl3DShadow, 0));

    // spHighlight
    stockpens.push_back(new Pen(clHighlight, 0));

    // spHighlightText
    stockpens.push_back(new Pen(clHighlightText, 0));


    // Stock Fonts:
    NONCLIENTMETRICS met = application->NCMetrics();

    // sfCaption
    stockfonts.push_back(font = new Font(met.lfCaptionFont.lfFaceName, met.lfCaptionFont.lfHeight < 0 ? FontSizeFromHeight(met.lfCaptionFont.lfHeight) : FontSizeFromLogfont(met.lfCaptionFont), 0, 0, clBtnText, met.lfCaptionFont.lfWeight >= FW_SEMIBOLD,
                                         met.lfCaptionFont.lfItalic == TRUE, met.lfCaptionFont.lfUnderline == TRUE, met.lfCaptionFont.lfStrikeOut == TRUE, fcsDefault, foqDefault));

    // sfSmallCaption
    stockfonts.push_back(font = new Font(met.lfSmCaptionFont.lfFaceName, met.lfSmCaptionFont.lfHeight < 0 ? FontSizeFromHeight(met.lfSmCaptionFont.lfHeight) : FontSizeFromLogfont(met.lfSmCaptionFont), 0, 0, clBtnText, met.lfSmCaptionFont.lfWeight >= FW_SEMIBOLD,
                                         met.lfSmCaptionFont.lfItalic == TRUE, met.lfSmCaptionFont.lfUnderline == TRUE, met.lfSmCaptionFont.lfStrikeOut == TRUE, fcsDefault, foqDefault));

    // sfMenu
    stockfonts.push_back(font = new Font(met.lfMenuFont.lfFaceName, met.lfMenuFont.lfHeight < 0 ? FontSizeFromHeight(met.lfMenuFont.lfHeight) : FontSizeFromLogfont(met.lfMenuFont), 0, 0, clBtnText, met.lfMenuFont.lfWeight >= FW_SEMIBOLD,
                                         met.lfMenuFont.lfItalic == TRUE, met.lfMenuFont.lfUnderline == TRUE, met.lfMenuFont.lfStrikeOut == TRUE, fcsDefault, foqDefault));
}

void Canvas::ClearStock()
{
    for (auto it = stockbrushes.begin(); it != stockbrushes.end(); it++)
        delete (*it);
    for (auto it = stockpens.begin(); it != stockpens.end(); it++)
        delete (*it);
    for (auto it = stockfonts.begin(); it != stockfonts.end(); it++)
        delete (*it);
    stockbrushes.clear();
    stockpens.clear();
    stockfonts.clear();
}

Gdiplus::Graphics* Canvas::Get()
{
    using namespace Gdiplus;

    if (!GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        UpdateGraphics(g);
        if (g && g->GetLastStatus() != Ok)
            throw L"Invalid operation on GDI+ graphics object.";
        if (!g)
            throw L"Couldn't acquire GDI+ graphics object.";
        return g;
    }
    else
        return GetGraphics();
}

HDC Canvas::GetDC()
{
    if (HasCanvasState(csDCBusy))
        throw L"The DC is already taken by a GetDC call. Return it with ReturnDC first.";

    HRGN clip;
    if (ClipEmpty() || !GetClip(clip))
        clip = 0;

    saveddc = _GetDC();
    if (!saveddc)
    {
        if (clip != 0)
            DeleteObject(clip);
        throw L"Couldn't acquire DC!";
    }
    int clipstate = GetClipRgn(saveddc, savedrgn);
    hasrgn = clipstate != -1 && clipstate != 0;

    savedadvanced = SetGraphicsMode(saveddc, !advanced ? GM_COMPATIBLE : GM_ADVANCED) == GM_ADVANCED;
    savedfont = (HFONT)SelectObject(saveddc, font->Handle());
    savedbkmode = SetBkMode(saveddc, TRANSPARENT);
    savedtextcolor = SetTextColor(saveddc, font->GetColor()); // Font color.
    savedtextalign = SetTextAlign(saveddc, textalign);

    if (clip != 0)
    {
        SelectClipRgn(saveddc, clip);
        DeleteObject(clip);
    }

    if (saveddc)
        AddCanvasState(csDCBusy);
    return saveddc;
}

void Canvas::ReturnDC()
{
    if (saveddc)
    {
        savedbkmode = SetBkMode(saveddc, savedbkmode);
        savedtextcolor = SetTextColor(saveddc, savedtextcolor);
        savedtextalign = SetTextAlign(saveddc, savedtextalign);
        SelectObject(saveddc, savedfont);
        SetGraphicsMode(saveddc, savedadvanced);
        if (hasrgn)
            SelectClipRgn(saveddc, savedrgn);
        else
            SelectClipRgn(saveddc, NULL);
        hasrgn = false;
        saveddc = 0;
        _ReturnDC();
    }
    else
        throw L"Returning DC when it hasn't been borrowed in the first place.";
}

void Canvas::Release()
{
    ReleaseGraphics();
}

bool Canvas::CompatibleDC()
{
    return true;
}

CanvasStateSet Canvas::CanvasState()
{
    return canvasstate;
}

bool Canvas::AddCanvasState(CanvasStates state)
{
    if (canvasstate.contains(state))
        return true;
    canvasstate << state;
    return false;
}

bool Canvas::RemoveCanvasState(CanvasStates state)
{
    if (!canvasstate.contains(state))
        return false;
    canvasstate -= state;
    return true;
}

bool Canvas::HasCanvasState(CanvasStates state) const
{
    return canvasstate.contains(state);
}

void Canvas::ResetBrush()
{
    if (!outerbrush)
        delete brush;

    brush = stockbrushes[sbWhite];
    outerbrush = true;
}

void Canvas::ResetPen()
{
    if (!outerpen)
        delete pen;

    pen = stockpens[spBlack];
    outerpen = true;
}

void Canvas::ResetFont()
{
    if (!outerfont)
        delete font;

    font = stockfonts[sfCaption];
    outerfont = true;
}

void Canvas::AddBrush(const std::wstring& name, Brush &abrush, bool select)
{
    if (name.size() == 0)
        throw L"Can't add brush with no name.";

    auto it = brushes.find(name);
    if (it != brushes.end() && it->first == name)
        delete it->second;

    Brush *b = new Brush(abrush);
    brushes[name] = b;
    if (select)
    {
        if (!outerbrush)
            delete brush;
        brush = b;
        outerbrush = true;
    }
}

void Canvas::AddBrush(const std::wstring& name, Color color, bool select)
{
    if (name.size() == 0)
        throw L"Can't add brush with no name.";

    auto it = brushes.find(name);
    if (it != brushes.end() && it->first == name)
        delete it->second;

    Brush *b = new Brush(color);
    brushes[name] = b;
    if (select)
    {
        if (!outerbrush)
            delete brush;
        brush = b;
        outerbrush = true;
    }
}

void Canvas::AddPen(const std::wstring& name, Color color, float width, bool select)
{
    if (name.size() == 0)
        throw L"Can't add pen with no name.";

    auto it = pens.find(name);
    if (it != pens.end() && it->first == name)
        delete it->second;

    Pen *p;
    p = new Pen(color, width);
    pens[name] = p;
    if (select)
    {
        if (!outerpen)
            delete pen;
        pen = p;
        outerpen = true;
    }
}

void Canvas::AddFont(const std::wstring& name, Font &font, bool select)
{
    AddFont(name, font.Family(), font.Size(), font.Escapement(), font.Orientation(), font.GetColor(), font.Bold(), font.Italic(), font.Underline(), font.Strikeout(), font.CharacterSet(), font.OutputQuality());
}

void Canvas::AddFont(const std::wstring& name, const std::wstring& family, float size, int escapement, int orientation, Color color, bool bold, bool italic, bool underline, bool strikeout, FontCharacterSets charset, FontOutputQualities quality, bool select)
{
    if (name.size() == 0)
        throw L"Can't add font with no name.";

    auto it = fonts.find(name);
    if (it != fonts.end() && it->first == name)
        delete it->second;

    Font *f;
    f = new Font(family, size, escapement, orientation, color, bold, italic, underline, strikeout, charset, quality);
    fonts[name] = f;
    if (select)
    {
        if (!outerfont)
            delete font;
        font = f;
        outerfont = true;
    }
}

bool Canvas::HasBrush(const std::wstring& name)
{
    auto namebrush = brushes.find(name);
    if (namebrush == brushes.end() || namebrush->first != name)
        return false;
    return true;
}

bool Canvas::HasPen(const std::wstring& name)
{
    auto namepen = pens.find(name);
    if (namepen == pens.end() || namepen->first != name)
        return false;
    return true;
}

bool Canvas::HasFont(const std::wstring& name)
{
    auto namefont = fonts.find(name);
    if (namefont == fonts.end() || namefont->first != name)
        return false;
    return true;
}

void Canvas::RemoveBrush(const std::wstring& name)
{
    std::map<std::wstring, Brush*>::iterator it = brushes.find(name);
    if (it != brushes.end() && it->first == name)
    {
        if (outerbrush && brush == it->second)
            ResetBrush();
        delete it->second;
        brushes.erase(it);
    }
}

void Canvas::RemovePen(const std::wstring& name)
{
    std::map<std::wstring, Pen*>::iterator it = pens.find(name);
    if (it != pens.end() && it->first == name)
    {
        if (outerpen && pen == it->second)
            ResetPen();
        delete it->second;
        pens.erase(it);
    }
}

void Canvas::RemoveFont(const std::wstring& name)
{
    std::map<std::wstring, Font*>::iterator it = fonts.find(name);
    if (it != fonts.end() && it->first == name)
    {
        if (outerfont && font == it->second)
            ResetFont();
        delete it->second;
        fonts.erase(it);
    }
}

void Canvas::SelectBrush(const std::wstring& name)
{
    std::map<std::wstring, Brush*>::iterator it = brushes.find(name);
    if (it == brushes.end() || it->first != name)
        throw L"No font was added with this name";

    if (!outerbrush)
        delete brush;

    brush = it->second;
    outerbrush = true;
}

void Canvas::SelectPen(const std::wstring& name)
{
    std::map<std::wstring, Pen*>::iterator it = pens.find(name);
    if (it == pens.end() || it->first != name)
        throw L"No font was added with this name";

    if (!outerpen)
        delete pen;

    pen = it->second;
    outerpen = true;
}

void Canvas::SelectFont(const std::wstring& name)
{
    std::map<std::wstring, Font*>::iterator it = fonts.find(name);
    if (it == fonts.end() || it->first != name)
        throw L"No font was added with this name";

    if (!outerfont)
        delete font;

    font = it->second;
    outerfont = true;
}

void Canvas::SelectStockBrush(StockBrushes sbrush)
{
    if (!outerbrush)
        delete brush;
    brush = stockbrushes[sbrush];
    outerbrush = true;
}

void Canvas::SelectStockPen(StockPens spen)
{
    if (!outerpen)
        delete pen;
    pen = stockpens[spen];
    outerpen = true;
}

void Canvas::SelectStockFont(StockFonts sfont)
{
    if (!outerfont)
        delete font;
    font = stockfonts[sfont];
    outerfont = true;
}

void Canvas::SetBrush(const Color &color)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(color);
    outerbrush = false;
}

void Canvas::SetBrush(const Rect &rect, const Color &c1, const Color &c2, float angle, bool anglescalable)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(rect, c1, c2, angle, anglescalable);
    outerbrush = false;
}

void Canvas::SetBrush(const RectF &rect, const Color &c1, const Color &c2, float angle, bool anglescalable)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(rect, c1, c2, angle, anglescalable);
    outerbrush = false;
}

void Canvas::SetBrush(Bitmap *bmp, bool storebmp, BrushWrapModes wrapmode)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(bmp, storebmp, wrapmode);
    outerbrush = false;
}

void Canvas::SetBrush(Bitmap *bmp, const Rect &r, bool storebmp, BrushWrapModes wrapmode)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(bmp, r, storebmp, wrapmode);
    outerbrush = false;
}

void Canvas::SetBrush(const Color &linecolor, const Color &bgcolor, Gdiplus::HatchStyle hatchstyle)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(linecolor, bgcolor, hatchstyle);
    outerbrush = false;
}

void Canvas::SetBrush(Gdiplus::GraphicsPath *path, const Color &color, bool storepath, BrushWrapModes wrapmode)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(path, color, storepath, wrapmode);
    outerbrush = false;
}

void Canvas::SetBrush(Point *points, int pointcount, const Color &color, BrushWrapModes wrapmode)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(points, pointcount, color, wrapmode);
    outerbrush = false;
}

void Canvas::SetBrush(PointF *points, int pointcount, const Color &color, BrushWrapModes wrapmode)
{
    if (!outerbrush)
        delete brush;

    brush = new Brush(points, pointcount, color, wrapmode);
    outerbrush = false;
}

void Canvas::SetBrush(Brush &abrush)
{
    if (!outerbrush)
        delete brush;
    brush = new Brush(abrush);
    outerbrush = false;
}

void Canvas::SetPen(Color color, float width)
{
    if (!outerpen)
        delete pen;
    pen = new Pen(color, width);
    outerpen = false;
}

void Canvas::SetPen(Color color, PenDashStyles dashstyle, float width)
{
    if (!outerpen)
        delete pen;
    pen = new Pen(color, dashstyle, width);
    outerpen = false;
}

void Canvas::SetPen(Pen &apen)
{
    if (!outerpen)
        delete pen;
    pen = new Pen(apen);
    outerpen = false;
}

void Canvas::SetFont(const Font &afont)
{
    if (font && afont == *font)
        return;

    if (!outerfont)
        delete font;
    font = new Font(afont);
    outerfont = false;
}

void Canvas::SetFont(const std::wstring& family, float size, int escapement, int orientation, Color color, bool bold, bool italic, bool underline, bool strikeout, FontCharacterSets charset, FontOutputQualities quality)
{
    if (family == font->Family() && size == font->Size() && escapement == font->Escapement() && orientation == font->Orientation() && color == font->GetColor() &&
        bold == font->Bold() && italic == font->Italic() && underline == font->Underline() && strikeout == font->Strikeout() && charset == font->CharacterSet() &&
        quality == font->OutputQuality())
        return;

    if (!outerfont)
        delete font;

    font = new Font(family, size, escapement, orientation, color, bold, italic, underline, strikeout, charset, quality);
    outerfont = false;
}

void Canvas::SetColorMatrix(const ColorMatrix &colormatrix, ColorMatrixFlags flag, ColorAdjustTypes adjusttype)
{
    switch (adjusttype)
    {
    case catDefault:
        usedefaultmatrix = true;
        defaultmatrix = colormatrix;
        defaultmatrixflag = flag;
        break;
    case catBitmap:
        usebitmapmatrix = true;
        bitmapmatrix = colormatrix;
        bitmapmatrixflag = flag;
        break;
    case catBrush:
        usebrushmatrix = true;
        brushmatrix = colormatrix;
        brushmatrixflag = flag;
        break;
    case catPen:
        usepenmatrix = true;
        penmatrix = colormatrix;
        penmatrixflag = flag;
        break;
    }
    attrib.SetColorMatrix(&colormatrix, (Gdiplus::ColorMatrixFlags)flag, (Gdiplus::ColorAdjustType)adjusttype);
}

void Canvas::ResetColorMatrix(ColorAdjustTypes adjusttype)
{
    switch (adjusttype)
    {
    case catDefault:
        usedefaultmatrix = false;
        break;
    case catBitmap:
        usebitmapmatrix = false;
        break;
    case catBrush:
        usebrushmatrix = false;
        break;
    case catPen:
        usepenmatrix = false;
        break;
    }
    attrib.ClearColorMatrix((Gdiplus::ColorAdjustType)adjusttype);
}

void Canvas::SetColorKey(Color low, Color high, ColorAdjustTypes adjusttype)
{
    switch (adjusttype)
    {
    case catDefault:
        usedefaultcolorkey = true;
        break;
    case catBitmap:
        usebitmapcolorkey = true;
        break;
    case catBrush:
        usebrushcolorkey = true;
        break;
    case catPen:
        usepencolorkey = true;
        break;
    }
    attrib.SetColorKey(low, high, (Gdiplus::ColorAdjustType)adjusttype);
}

void Canvas::ResetColorKey(ColorAdjustTypes adjusttype)
{
    switch (adjusttype)
    {
    case catDefault:
        usedefaultcolorkey = false;
        break;
    case catBitmap:
        usebitmapcolorkey = false;
        break;
    case catBrush:
        usebrushcolorkey = false;
        break;
    case catPen:
        usepencolorkey = false;
        break;
    }
    attrib.ClearColorKey((Gdiplus::ColorAdjustType)adjusttype);
}

void Canvas::SetTransform(const Matrix &matrix)
{
    transf = matrix;
    usetransf = true;

    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->SetTransform(&transf);
    }
}

void Canvas::ResetTransform()
{
    transf = Matrix();
    usetransf = false;

    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->ResetTransform();
    }
}

Matrix Canvas::Transform()
{
    return transf;
}

bool Canvas::UsingTransform()
{
    return usetransf;
}

void Canvas::MultiplyTransform(const Matrix &matrix, bool prepend)
{
    using namespace Gdiplus;

    transf.Multiply(matrix, prepend ? MatrixOrderPrepend : MatrixOrderAppend);

    usetransf = true;
    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->SetTransform(&transf);
    }
}

void Canvas::RotateTransform(Gdiplus::REAL angle, bool prepend)
{
    using namespace Gdiplus;

    transf.Rotate(angle, prepend ? MatrixOrderPrepend : MatrixOrderAppend);

    usetransf = true;
    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->SetTransform(&transf);
    }
}

void Canvas::RotateAtTransform(Gdiplus::REAL angle, PointF center, bool prepend)
{
    using namespace Gdiplus;

    transf.RotateAt(angle, center, prepend ? MatrixOrderPrepend : MatrixOrderAppend);

    usetransf = true;
    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->SetTransform(&transf);
    }
}

void Canvas::ScaleTransform(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend)
{
    using namespace Gdiplus;

    transf.Scale(x, y, prepend ? MatrixOrderPrepend : MatrixOrderAppend);

    usetransf = true;
    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->SetTransform(&transf);
    }
}

void Canvas::TranslateTransform(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend)
{
    using namespace Gdiplus;

    transf.Translate(x, y, prepend ? MatrixOrderPrepend : MatrixOrderAppend);

    usetransf = true;
    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->SetTransform(&transf);
    }
}

void Canvas::ShearTransform(Gdiplus::REAL x, Gdiplus::REAL y, bool prepend)
{
    using namespace Gdiplus;

    transf.Shear(x, y, prepend ? MatrixOrderPrepend : MatrixOrderAppend);

    usetransf = true;
    if (GraphicsCreated())
    {
        Gdiplus::Graphics *g = GetGraphics();
        g->SetTransform(&transf);
    }
}

Brush& Canvas::GetBrush()
{
    if (outerbrush)
    {
        brush = new Brush(*brush);
        outerbrush = false;
    }

    return *brush;
}

Pen& Canvas::GetPen()
{
    if (outerpen)
    {
        pen = new Pen(*pen);
        outerpen = false;
    }

    return *pen;
}

Font& Canvas::GetFont()
{
    if (outerfont)
    {
        font = new Font(*font);
        outerfont = false;
    }

    return *font;
}

void Canvas::Clear(Color newcolor)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    graphics->Clear(newcolor);
    if (graphics && graphics->GetLastStatus() != Ok)
        throw L"Invalid operation on GDI+ graphics object.";
}

void Canvas::Line(int x1, int y1, int x2, int y2)
{
    LineF(x1, y1, x2, y2);
    //if (antialias)
    //{
    //    float fx1 = x1;
    //    float fy1 = y1;
    //    float fx2 = x2;
    //    float fy2 = y2;

    //    if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
    //    {
    //        fx1 -= 0.5f;
    //        fx2 -= 0.5f;
    //        fy1 -= 0.5f;
    //        fy2 -= 0.5f;
    //    }

    //    LineF(fx1, fy1, fx2, fy2);
    //    return;
    //}

    //using namespace Gdiplus;
    //Gdiplus::Graphics *graphics = Get();
    //if (graphics->DrawLine(&pen->Object(), x1, y1, x2, y2) != Ok)
    //    throw L"Invalid operation on GDI+ graphics object. Canvas::Line";
}

void Canvas::Line(Point start, Point end)
{
    Line(start.x, start.y, end.x, end.y);
}

void Canvas::Line(PointF start, PointF end)
{
    Line(start.x, start.y, end.x, end.y);
}

void Canvas::LineF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    enum linedirenum { ldeLT, ldeT, ldeRT, ldeR, ldeRB, ldeB, ldeLB, ldeL };
    linedirenum dir;
    if (abs(y1 - y2) < 0.001)
        dir = (x1 < x2) ? ldeR : ldeL;
    else if (abs(x1 - x2) < 0.001)
        dir = (y1 < y2) ? ldeB : ldeT;
    else if (x1 < x2)
        dir = y1 < y2 ? ldeRB : ldeRT;
    else
        dir = y1 < y2 ? ldeLB : ldeLT;

    switch (dir)
    {
    case ldeR:
    case ldeL:
        if (antialias)
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
            }
            else
            {
                x1 += 0.4f;
                x2 += 0.6f;
                y1 += 0.5f;
                y2 += 0.5f;
            }
        }
        else
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
            }
            else
            {
                x1 += 0.5f;
                x2 += 0.5f;
                y1 += 0.5f;
                y2 += 0.5f;
            }
        }
        break;
    case ldeB:
        if (antialias)
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                y1 -= 0.4f;
                y2 += 0.2f;
            }
            else
            {
                x1 += 0.5f;
                x2 += 0.5f;
                y2 += 0.6f;
            }
        }
        else
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
            }
            else
            {
                x1 += 0.5f;
                x2 += 0.5f;
                y1 += 0.5f;
                y2 += 0.5f;
            }
        }
        break;
    case ldeT:
        if (antialias)
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                y2 -= 0.3f;
                y1 += 0.2f;
            }
            else
            {
                x1 += 0.5f;
                x2 += 0.5f;
                y1 += 0.6f;
                y2 += 0.2f;
            }
        }
        else
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
            }
            else
            {
                x1 += 0.5f;
                x2 += 0.5f;
                y2 += 0.5f;
                y1 += 0.5f;
            }
        }
        break;
    case ldeRB:
        if (antialias)
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                x1 -= 0.4f;
                x2 += 0.2f;
                y1 -= 0.4f;
            }
            else
            {
                x1 += 0.2f;
                x2 += 0.8f;
                y1 += 0.1f;
                y2 += 0.5f;
            }
        }
        else
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                y1 += 0.1f;
                y2 -= 0.1f;
            }
            else
            {
                x1 += 0.5f;
                x2 += 0.5f;
                y1 += 0.6f;
                y2 += 0.1f;
            }
        }
        break;
    case ldeLB:
        if (antialias)
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                x2 -= 0.4f;
                x1 += 0.2f;
                y1 -= 0.4f;
            }
            else
            {
                x2 += 0.2f;
                x1 += 0.8f;
                y1 += 0.1f;
                y2 += 0.5f;
            }
        }
        else
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                y1 += 0.1f;
                y2 -= 0.1f;
            }
            else
            {
                x2 += 0.5f;
                x1 += 0.5f;
                y1 += 0.6f;
                y2 += 0.1f;
            }
        }
        break;
    case ldeRT:
        if (antialias)
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                x1 -= 0.4f;
                x2 += 0.2f;
                y2 -= 0.4f;
            }
            else
            {
                x1 += 0.2f;
                x2 += 0.8f;
                y2 += 0.1f;
                y1 += 0.5f;
            }
        }
        else
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                y2 += 0.1f;
                y1 -= 0.1f;
            }
            else
            {
                x1 += 0.5f;
                x2 += 0.5f;
                y2 += 0.6f;
                y1 += 0.1f;
            }
        }
        break;
    case ldeLT:
        if (antialias)
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                x2 -= 0.4f;
                x1 += 0.2f;
                y2 -= 0.4f;
            }
            else
            {
                x2 += 0.2f;
                x1 += 0.8f;
                y2 += 0.1f;
                y1 += 0.5f;
            }
        }
        else
        {
            if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
            {
                y2 += 0.1f;
                y1 -= 0.1f;
            }
            else
            {
                x2 += 0.5f;
                x1 += 0.5f;
                y2 += 0.6f;
                y1 += 0.1f;
            }
        }
        break;
    }

    if (graphics->DrawLine(&pen->Object(), x1, y1, x2, y2) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::LineF";
}

void Canvas::FillRect(int x1, int y1, int x2, int y2)
{
    FillRectF(x1, y1, x2, y2);
}

void Canvas::FillRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            x1 -= 0.5f;
            x2 -= 0.5f;
            y1 -= 0.5f;
            y2 -= 0.5f;
        }
    }

    if (graphics->FillRectangle(&brush->Object(), x1, y1, x2 - x1, y2 - y1) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::FillRectF";
}

void Canvas::FillRect(const Rect &rect)
{
    FillRect(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::FillRectF(const RectF &rect)
{
    FillRectF(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::FillEllipse(int x1, int y1, int x2, int y2)
{
    FillEllipseF(x1, y1, x2, y2);
}

void Canvas::FillEllipseF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            x1 -= 0.5f;
            y1 -= 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
        else
        {
            x1 -= 0.15f;
            y1 -= 0.15f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            x1 -= 0.5f;
            y1 -= 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
        else
        {
        }
    }

    if (graphics->FillEllipse(&brush->Object(), x1, y1, x2 - x1, y2 - y1) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::FillEllipseF";
}

void Canvas::FillEllipse(const Rect &rect)
{
    FillEllipse(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::FillEllipseF(const RectF &rect)
{
    FillEllipseF(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::FillPolygon(Point *points, int pointcount)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::Point *pt = new Gdiplus::Point[pointcount];
    for (int ix = 0; ix < pointcount; ++ix)
        pt[ix] = points[ix];
    if (graphics->FillPolygon(&brush->Object(), pt, pointcount) != Ok)
    {
        delete[] pt;
        throw L"Invalid operation on GDI+ graphics object. Canvas::FillPolygon";
    }
    delete[] pt;
}

void Canvas::FillPolygonF(PointF *points, int pointcount)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::PointF *pt = new Gdiplus::PointF[pointcount];
    for (int ix = 0; ix < pointcount; ++ix)
        pt[ix] = points[ix];
    if (graphics->FillPolygon(&brush->Object(), pt, pointcount) != Ok)
    {
        delete[] pt;
        throw L"Invalid operation on GDI+ graphics object. Canvas::FillPolygonF";
    }
    delete[] pt;
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPath(int x1, int y1, int x2, int y2, int roundwidth, int roundheight)
{
    return CreateRoundPathF(x1, y1, x2, y2, roundwidth, roundheight);
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPath(const Rect &rect, int roundwidth, int roundheight)
{
    return CreateRoundPath(rect.left, rect.top, rect.right, rect.bottom, roundwidth, roundheight);
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPathF(const RectF &rect, float roundwidth, float roundheight)
{
    return CreateRoundPathF(rect.left, rect.top, rect.right, rect.bottom, roundwidth, roundheight);
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPathF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundwidth, Gdiplus::REAL roundheight)
{
    Gdiplus::GraphicsPath *gp = new Gdiplus::GraphicsPath();

    roundwidth = min(roundwidth, (x2 - x1 + 1) / 2) * 2;
    roundheight = min(roundheight, (y2 - y1 + 1) / 2) * 2;

    float fx1 = x1, fx2 = x2, fy1 = y1, fy2 = y2;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fy1 = y1 - 0.1f;
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
        }
    }

    gp->AddArc(fx1, fy1, roundwidth, roundheight, 180.0f, 90.0f);

    fx1 = x1;
    fy1 = y1;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fy1 = y1 - 0.1f;
            fx2 = x2 - 1.0f;
        }
        else
        {
            fx2 = x2 - 0.5f;
            fy1 = y1 + 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fx2 = x2 - 0.5f;
        }
        else
        {
            fx2 = x2 - 0.1f;
            fy1 = y1 + 0.5f;
        }
    }

    gp->AddArc(fx2 - roundwidth, fy1, roundwidth, roundheight, -90.0f, 90.0f);

    fy1 = y1;
    fx2 = x2;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fx2 = x2 - 1.0f;
            fy2 = y2 - 1.0f;
        }
        else
        {
            fx2 = x2 - 0.5f;
            fy2 = y2 - 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fx2 = x2 - 0.5f;
        }
        else
        {
            fx2 = x2 - 0.1f;
            fy2 = y2 - 0.1f;
        }
    }

    gp->AddArc(fx2 - roundwidth, fy2 - roundheight, roundwidth, roundheight, 0.0f, 90.0f);

    fx2 = x2;
    fy2 = y2;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fy2 = y2 - 1.0f;
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
            fy2 = y2 - 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
            fy2 = y2 - 0.1f;
        }
    }

    gp->AddArc(fx1, fy2 - roundheight, roundwidth, roundheight, 90.0f, 90.0f);
    gp->CloseFigure();
    return gp;
}

void Canvas::FillRoundRect(int x1, int y1, int x2, int y2, int roundwidth, int roundheight)
{
    FillRoundRectF(x1, y1, x2, y2, roundwidth, roundheight);
}

void Canvas::FillRoundRect(const Rect &rect, int roundwidth, int roundheight)
{
    FillRoundRect(rect.left, rect.top, rect.right, rect.bottom, roundwidth, roundheight);
}

void Canvas::FillRoundRectF(const RectF &rect, float roundwidth, float roundheight)
{
    FillRoundRectF(rect.left, rect.top, rect.right, rect.bottom, roundwidth, roundheight);
}

void Canvas::FillRoundRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundwidth, Gdiplus::REAL roundheight)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    Gdiplus::GraphicsPath *gp = CreateRoundPathF(x1, y1, x2, y2, roundwidth, roundheight);
    if (graphics->FillPath(&brush->Object(), gp) != Ok)
    {
        delete gp;
        throw L"Invalid operation on GDI+ graphics object. Canvas::FillRoundRectF";
    }
    delete gp;
}

void Canvas::FillPath(Gdiplus::GraphicsPath *path)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    Gdiplus::Matrix m;
    m.Translate(-0.5, -0.5);
    path->Transform(&m);
    if (graphics->FillPath(&brush->Object(), path) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::FillPath";
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPath(int x1, int y1, int x2, int y2, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft)
{
    return CreateRoundPathF(x1, y1, x2, y2, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPath(const Rect &rect, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft)
{
    return CreateRoundPath(rect.left, rect.top, rect.right, rect.bottom, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPathF(const RectF &rect, float roundtopleft, float roundtopright, float roundbottomright, float roundbottomleft)
{
    return CreateRoundPathF(rect.left, rect.top, rect.right, rect.bottom, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

Gdiplus::GraphicsPath* Canvas::CreateRoundPathF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft)
{
    Gdiplus::GraphicsPath *gp = new Gdiplus::GraphicsPath();

    roundtopleft = max(roundtopleft, 0.1) * 2;
    roundtopright = max(roundtopright, 0.1) * 2;
    roundbottomright = max(roundbottomright, 0.1) * 2;
    roundbottomleft = max(roundbottomleft, 0.1) * 2;

    float fx1 = x1, fx2 = x2, fy1 = y1, fy2 = y2;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fy1 = y1 - 0.1f;
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
        }
    }

    gp->AddArc(fx1, fy1, roundtopleft, roundtopleft, 180.0f, 90.0f);

    fx1 = x1;
    fy1 = y1;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fy1 = y1 - 0.1f;
            fx2 = x2 - 1.0f;
        }
        else
        {
            fx2 = x2 - 0.5f;
            fy1 = y1 + 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fx2 = x2 - 0.5f;
        }
        else
        {
            fx2 = x2 - 0.1f;
            fy1 = y1 + 0.5f;
        }
    }

    gp->AddArc(fx2 - roundtopright, fy1, roundtopright, roundtopright, -90.0f, 90.0f);

    fy1 = y1;
    fx2 = x2;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fx2 = x2 - 1.0f;
            fy2 = y2 - 1.0f;
        }
        else
        {
            fx2 = x2 - 0.5f;
            fy2 = y2 - 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fx2 = x2 - 0.5f;
        }
        else
        {
            fx2 = x2 - 0.1f;
            fy2 = y2 - 0.1f;
        }
    }
    gp->AddArc(fx2 - roundbottomright, fy2 - roundbottomright, roundbottomright, roundbottomright, 0.0f, 90.0f);

    fx2 = x2;
    fy2 = y2;

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            fy2 = y2 - 1.0f;
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
            fy2 = y2 - 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
        }
        else
        {
            fx1 = x1 + 0.5f;
            fy1 = y1 + 0.5f;
            fy2 = y2 - 0.1f;
        }
    }

    gp->AddArc(fx1, fy2 - roundbottomleft, roundbottomleft, roundbottomleft, 90.0f, 90.0f);
    gp->CloseFigure();
    return gp;
}

Gdiplus::GraphicsPath* Canvas::CreateEllipsePath(int x1, int y1, int x2, int y2)
{
    return CreateEllipsePathF(x1, y1, x2, y2);
}

Gdiplus::GraphicsPath* Canvas::CreateEllipsePathF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2)
{
    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            x2 -= 1.0f;
            y2 -= 1.0f;
        }
        else
        {
            x1 += 0.5f;
            y1 += 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            x2 -= 1.0f;
            y2 -= 1.0f;
        }
        else
        {
            x1 += 0.5f;
            y1 += 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
    }

    Gdiplus::GraphicsPath *gp = new Gdiplus::GraphicsPath();
    gp->AddEllipse(x1, y1, x2 - x1, y2 - y1);

    return gp;
}

Gdiplus::GraphicsPath* Canvas::CreateEllipsePath(const Rect &rect)
{
    return CreateEllipsePath(rect.left, rect.top, rect.right, rect.bottom);
}

Gdiplus::GraphicsPath* Canvas::CreateEllipsePathF(const RectF &rect)
{
    return CreateEllipsePathF(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::FillRoundRect(int x1, int y1, int x2, int y2, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft)
{
    FillRoundRectF(x1, y1, x2, y2, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

void Canvas::FillRoundRect(const Rect &rect, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft)
{
    FillRoundRect(rect.left, rect.top, rect.right - 1, rect.bottom - 1, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

void Canvas::FillRoundRectF(const RectF &rect, float roundtopleft, float roundtopright, float roundbottomright, float roundbottomleft)
{
    FillRoundRectF(rect.left, rect.top, rect.right - 1, rect.bottom - 1, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

void Canvas::FillRoundRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    Gdiplus::GraphicsPath *gp = CreateRoundPathF(x1, y1, x2, y2, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
    if (graphics->FillPath(&brush->Object(), gp) != Ok)
    {
        delete gp;
        throw L"Invalid operation on GDI+ graphics object. Canvas::FillRoundRectF";
    }
    delete gp;
}

void Canvas::FrameRect(int x1, int y1, int x2, int y2)
{
    //if (x1 > x2)
    //    std::swap(x1, x2);
    //if (y1 > y2)
    //    std::swap(y1, y2);

    //Line(x1, y1, x2 - 1, y1);
    //Line(x2 - 1, y1 + (y1 < y2 - 1 ? 1 : 0), x2 - 1, y2 - 1);
    //Line(x2 - (x1 < x2 - 1 ? 2 : 1), y2 - 1, x1, y2 - 1);
    //Line(x1, y2 - (y1 < y2 - 1 ? 2 : 1), x1, y1 + (y1 < y2 - 1 ? 1 : 0));

    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();


    if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
    {
    }
    else
    {
        FrameRectF(x1, y1, x2, y2);
        return;
    }

    if (graphics->DrawRectangle(&pen->Object(), x1, y1, x2 - x1 - 1, y2 - y1 - 1) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::FrameRectF";
}

void Canvas::FrameRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (antialias)
    {
        if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
        {
        }
        else
        {
            x1 += 0.5f;
            y1 += 0.5f;
            x2 += 0.5f;
            y2 += 0.5f;
        }
    }
    else
    {
        {
            x1 += 0.5f;
            y1 += 0.5f;
            x2 += 0.5f;
            y2 += 0.5f;
        }
    }

    if (graphics->DrawRectangle(&pen->Object(), x1, y1, x2 - x1 - 1, y2 - y1 - 1) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::FrameRectF";
}

void Canvas::FrameRect(const Rect &rect)
{
    FrameRect(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::FrameRectF(const RectF &rect)
{
    FrameRectF(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::DrawEllipse(int x1, int y1, int x2, int y2)
{
    //if (antialias)
    //{
    //    float fx1 = x1;
    //    float fy1 = y1;
    //    float fx2 = x2;
    //    float fy2 = y2;

    //    DrawEllipseF(fx1, fy1, fx2, fy2);
    //    return;
    //}

    //using namespace Gdiplus;
    //Gdiplus::Graphics *graphics = Get();
    //if (graphics->DrawEllipse(&pen->Object(), x1, y1, x2 - x1, y2 - y1) != Ok)
    //    throw L"Invalid operation on GDI+ graphics object. Canvas::DrawEllipse";

    DrawEllipseF(x1, y1, x2, y2);
}

void Canvas::DrawEllipse(const Rect &rect)
{
    DrawEllipse(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::DrawEllipseF(const RectF &rect)
{
    DrawEllipseF(rect.left, rect.top, rect.right, rect.bottom);
}

void Canvas::DrawEllipseF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (antialias)
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            x2 -= 1.0f;
            y2 -= 1.0f;
        }
        else
        {
            x1 += 0.5f;
            y1 += 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
    }
    else
    {
        if (pommode == pomDefault || pommode == pomHighSpeed || pommode == pomNone)
        {
            x2 -= 1.0f;
            y2 -= 1.0f;
        }
        else
        {
            x1 += 0.5f;
            y1 += 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
    }

    if (graphics->DrawEllipse(&pen->Object(), x1, y1, x2 - x1, y2 - y1) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::DrawEllipseF";
}

void Canvas::RoundFrameRect(int x1, int y1, int x2, int y2, int roundwidth, int roundheight)
{
    RoundFrameRectF(x1, y1, x2, y2, roundwidth, roundheight);
}

void Canvas::RoundFrameRect(const Rect &rect, int roundwidth, int roundheight)
{
    RoundFrameRect(rect.left, rect.top, rect.right, rect.bottom, roundwidth, roundheight);
}

void Canvas::RoundFrameRectF(const RectF &rect, float roundwidth, float roundheight)
{
    RoundFrameRectF(rect.left, rect.top, rect.right, rect.bottom, roundwidth, roundheight);
}

void Canvas::RoundFrameRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundwidth, Gdiplus::REAL roundheight)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::GraphicsPath *gp = CreateRoundPathF(x1, y1, x2, y2, roundwidth, roundheight);
    if (graphics->DrawPath(&pen->Object(), gp) != Ok)
    {
        delete gp;
        throw L"Invalid operation on GDI+ graphics object. Canvas::RoundFrameRectF";
    }
    delete gp;
}

void Canvas::RoundFrameRect(int x1, int y1, int x2, int y2, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft)
{
    RoundFrameRectF(x1, y1, x2, y2, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

void Canvas::RoundFrameRect(const Rect &rect, int roundtopleft, int roundtopright, int roundbottomright, int roundbottomleft)
{
    RoundFrameRectF(rect.left, rect.top, rect.right, rect.bottom, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

void Canvas::RoundFrameRectF(const RectF &rect, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft)
{
    RoundFrameRectF(rect.left, rect.top, rect.right, rect.bottom, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
}

void Canvas::RoundFrameRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Gdiplus::REAL roundtopleft, Gdiplus::REAL roundtopright, Gdiplus::REAL roundbottomright, Gdiplus::REAL roundbottomleft)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::GraphicsPath *gp = CreateRoundPathF(x1, y1, x2, y2, roundtopleft, roundtopright, roundbottomright, roundbottomleft);
    if (graphics->DrawPath(&pen->Object(), gp) != Ok)
    {
        delete gp;
        throw L"Invalid operation on GDI+ graphics object. Canvas::RoundFrameRectF";
    }
    delete gp;
}

void Canvas::DrawFrame(int x1, int y1, int x2, int y2, bool raised)
{
    DrawFrameF(x1, y1, x2, y2, raised);
}

void Canvas::DrawFrameF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, bool raised)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    auto savestate = graphics->Save();
    if (x2 < x1)
        std::swap(x2, x1);
    if (y2 < y1)
        std::swap(y2, y1);
    --x2;
    --y2;

    SelectStockPen(raised ? sp3DHighlight : sp3DShadow);
    LineF(x1, y1, x1, y2);
    LineF(x1, y1, x2, y1);
    SelectStockPen(!raised ? sp3DHighlight : sp3DShadow);
    LineF(x2, y1, x2, y2);
    LineF(x1, y2, x2, y2);

    if (graphics->Restore(savestate) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::DrawFrameF";
}

void Canvas::DrawFrame(const Rect &rect, bool raised)
{
    DrawFrame(rect.left, rect.top, rect.right, rect.bottom, raised);
}

void Canvas::DrawFrameF(const RectF &rect, bool raised)
{
    DrawFrameF(rect.left, rect.top, rect.right, rect.bottom, raised);
}

void Canvas::GradientRect(int x1, int y1, int x2, int y2, Color color1, Color color2, LinearGradientModes mode)
{
    GradientRectF(x1, y1, x2, y2, color1, color2, mode);
}

void Canvas::GradientRect(const Rect &rect, Color color1, Color color2, LinearGradientModes mode)
{
    GradientRect(rect.left, rect.top, rect.right, rect.bottom, color1, color2, mode);
}

void Canvas::GradientRectF(const RectF &rect, Color color1, Color color2, LinearGradientModes mode)
{
    GradientRectF(rect.left, rect.top, rect.right, rect.bottom, color1, color2, mode);
}

void Canvas::GradientRectF(Gdiplus::REAL x1, Gdiplus::REAL y1, Gdiplus::REAL x2, Gdiplus::REAL y2, Color color1, Color color2, LinearGradientModes mode)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    float gx1 = x1;
    float gx2 = x2;
    float gy1 = y1;
    float gy2 = y2;
    if (antialias)
    {
        if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
        {
            gx1 -= 0.5f;
            gy1 -= 0.5f;
            //gx2 -= 0.5f;
            //gy2 -= 0.5f;
        }
        else
        {
            gx1 += 0.1f;
            gy1 += 0.1f;
            //gx2 -= 0.5f;
            //gy2 -= 0.5f;
        }
    }
    else
    {
        if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
        {
            gx1 -= 0.5f;
            gy1 -= 0.5f;
            //gx2 -= 0.5f;
            //gy2 -= 0.5f;
        }
        else
        {
            gx1 += 0.1f;
            gy1 += 0.1f;
            //gx2 -= 0.5f;
            //gy2 -= 0.5f;
        }
    }
    Gdiplus::RectF rb = Gdiplus::RectF(gx1, gy1, x2 - x1 - 0.5, y2 - y1 - 0.5);

    LinearGradientBrush lgbrush(rb, color1, color2, (Gdiplus::LinearGradientMode)mode);

    if (antialias)
    {
        if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
        {
            x1 -= 0.5f;
            y1 -= 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
        else
        {
        }
    }
    else
    {
        if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
        {
            x1 -= 0.5f;
            y1 -= 0.5f;
            x2 -= 0.5f;
            y2 -= 0.5f;
        }
        else
        {
        }
    }

    Gdiplus::RectF r = Gdiplus::RectF(x1, y1, x2 - x1, y2 - y1);
    if (graphics->FillRectangle(&lgbrush, r) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::GradientRectF";
}

void Canvas::Draw(Bitmap *bitmap, int x, int y)
{
    Draw(bitmap, x, y, bitmap->Width(), bitmap->Height(), 0, 0, bitmap->Width(), bitmap->Height());
}

void Canvas::Draw(Bitmap *bitmap, int x, int y, int width, int height)
{
    Draw(bitmap, x, y, width, height, 0, 0, width, height);
}

void Canvas::Draw(Bitmap *bitmap, int destx, int desty, int srcx, int srcy, int width, int height)
{
    Draw(bitmap, destx, desty, width, height, srcx, srcy, width, height);
}

void Canvas::Draw(Bitmap *bitmap, int destx, int desty, int destwidth, int destheight, int srcx, int srcy, int srcwidth, int srcheight)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (graphics->DrawImage(bitmap->GetBitmap(), Gdiplus::Rect(destx, desty, destwidth, destheight), srcx, srcy, srcwidth, srcheight, UnitPixel, UseImageAttributes() ? &attrib : NULL) != Ok)
        throw (std::wstring(L"Invalid operation on GDI+ graphics object. Canvas::Draw") + (bitmap == 0 ? std::wstring(L" No bmp") : std::wstring(L" bmp(") + IntToStr(bitmap->Width()) + L", " + IntToStr(bitmap->Height()) + L")" ) + IntToStr(destx) + L", " + IntToStr(desty) + L", " + IntToStr(destwidth) + L", " + IntToStr(destheight) + L", " + IntToStr(srcx) + L", " + IntToStr(srcy) + L", " + IntToStr(srcwidth) + L", " + IntToStr(srcheight)).c_str();

    //float fdestx = destx;
    //float fdesty = desty;
    //if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
    //{
    //    fdestx += 0.5;
    //    fdesty += 0.5;
    //}

    //DrawF(bitmap, fdestx, fdesty, destwidth, destheight, srcx, srcy, srcwidth, srcheight);
}

void Canvas::DrawF(Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y)
{
    DrawF(bitmap, x, y, (Gdiplus::REAL)bitmap->Width(), (Gdiplus::REAL)bitmap->Height(), 0., 0., (Gdiplus::REAL)bitmap->Width(), (Gdiplus::REAL)bitmap->Height());
}

void Canvas::DrawF(Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::REAL width, Gdiplus::REAL height)
{
    DrawF(bitmap, x, y, width, height, 0., 0., width, height);
}

void Canvas::DrawF(Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL width, Gdiplus::REAL height)
{
    DrawF(bitmap, destx, desty, width, height, srcx, srcy, width, height);
}

void Canvas::DrawF(Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL destwidth, Gdiplus::REAL destheight, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL srcwidth, Gdiplus::REAL srcheight)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (graphics->DrawImage(bitmap->GetBitmap(), Gdiplus::RectF(destx, desty, destwidth, destheight), srcx, srcy, srcwidth, srcheight, UnitPixel, UseImageAttributes() ? &attrib : NULL) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::DrawF";
}

void Canvas::Draw(Gdiplus::Bitmap *bitmap, int x, int y)
{
    Draw(bitmap, x, y, bitmap->GetWidth(), bitmap->GetHeight(), 0, 0, bitmap->GetWidth(), bitmap->GetHeight());
}

void Canvas::DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y)
{
    DrawF(bitmap, x, y, (Gdiplus::REAL)bitmap->GetWidth(), (Gdiplus::REAL)bitmap->GetHeight(), 0., 0., (Gdiplus::REAL)bitmap->GetWidth(), (Gdiplus::REAL)bitmap->GetHeight());
}

void Canvas::Draw(Gdiplus::Bitmap *bitmap, int x, int y, int width, int height)
{
    Draw(bitmap, x, y, width, height, 0, 0, width, height);
}

void Canvas::DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL x, Gdiplus::REAL y, Gdiplus::REAL width, Gdiplus::REAL height)
{
    DrawF(bitmap, x, y, width, height, 0., 0., width, height);
}

void Canvas::Draw(Gdiplus::Bitmap *bitmap, int destx, int desty, int srcx, int srcy, int width, int height)
{
    Draw(bitmap, destx, desty, width, height, srcx, srcy, width, height);
}

void Canvas::DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL width, Gdiplus::REAL height)
{
    DrawF(bitmap, destx, desty, width, height, srcx, srcy, width, height);
}

void Canvas::Draw(Gdiplus::Bitmap *bitmap, int destx, int desty, int destwidth, int destheight, int srcx, int srcy, int srcwidth, int srcheight)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (graphics->DrawImage(bitmap, Gdiplus::Rect(destx, desty, destwidth, destheight), srcx, srcy, srcwidth, srcheight, UnitPixel, UseImageAttributes() ? &attrib : NULL) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::Draw2";

    //float fdestx = destx;
    //float fdesty = desty;
    //if (pommode == pomNone || pommode == pomHighSpeed || pommode == pomDefault)
    //{
    //    fdestx += 0.5;
    //    fdesty += 0.5;
    //}

    //DrawF(bitmap, fdestx, fdesty, destwidth, destheight, srcx, srcy, srcwidth, srcheight);
}

void Canvas::DrawF(Gdiplus::Bitmap *bitmap, Gdiplus::REAL destx, Gdiplus::REAL desty, Gdiplus::REAL destwidth, Gdiplus::REAL destheight, Gdiplus::REAL srcx, Gdiplus::REAL srcy, Gdiplus::REAL srcwidth, Gdiplus::REAL srcheight)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();

    if (graphics->DrawImage(bitmap, Gdiplus::Rect(destx, desty, destwidth, destheight), srcx, srcy, srcwidth, srcheight, UnitPixel, UseImageAttributes() ? &attrib : NULL) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::DrawF2";
}

void Canvas::DrawCursor(Cursors cursor, int x, int y, int frame)
{
    using namespace Gdiplus;
    HCURSOR hc = screencursor->GetCursor(cursor);
    Get();
    HDC dc = GetDC();
    DrawIconEx(dc, x, y, hc, 0, 0, frame, NULL, DI_DEFAULTSIZE | DI_NORMAL);
    ReturnDC();
}

void Canvas::DrawIcon(Icon &icon, int x, int y)
{
    icon.Draw(this, x, y);
}

void Canvas::DrawIcon(Icon &icon, int x, int y, int width, int height)
{
    icon.Draw(this, x, y, width, height);
}

InterpolationModes Canvas::InterpolationMode()
{
    return intmode;
}

void Canvas::SetInterpolationMode(InterpolationModes newmode)
{
    if (intmode == newmode)
        return;
    using namespace Gdiplus;
    intmode = newmode;
    if (GraphicsCreated())
        Get()->SetInterpolationMode((Gdiplus::InterpolationMode)intmode);
}

PixelOffsetModes Canvas::PixelOffsetMode()
{
    return pommode;
}

void Canvas::SetPixelOffsetMode(PixelOffsetModes newmode)
{
    if (pommode == newmode)
        return;
    using namespace Gdiplus;
    pommode = newmode;
    if (GraphicsCreated())
        Get()->SetPixelOffsetMode((Gdiplus::PixelOffsetMode)pommode);
}

bool Canvas::ClipEmpty()
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    return graphics->IsClipEmpty() == TRUE ? true : false;
}

void Canvas::ResetClip()
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->ResetClip() != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::ResetClip";
}

void Canvas::ClipRect(Rect &r)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::Rect gr;
    if (graphics->GetClipBounds(&gr) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::ClipRect";
    r = gr;
}

void Canvas::TranslateClip(int x, int y)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->TranslateClip(x, y) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::TranslateClip";
}

void Canvas::TranslateClip(Gdiplus::REAL x, Gdiplus::REAL y)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->TranslateClip(x, y) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::TranslateClip";
}

void Canvas::ClipRectF(RectF &r)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::RectF gr;
    if (graphics->GetClipBounds(&gr) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::ClipRectF";
    r = gr;
}

//void Canvas::ClipRegion(Gdiplus::Region &rgn)
//{
//    using namespace Gdiplus;
//    Gdiplus::Graphics *graphics = Get();
//    if (graphics->GetClip(&rgn) != Ok)
//        throw L"Invalid operation on GDI+ graphics object.";
//}

bool Canvas::GetClip(Gdiplus::Region &rgn)
{
    Gdiplus::Graphics *graphics = Get();
    return graphics->GetClip(&rgn) == Gdiplus::Ok;
}

bool Canvas::GetClip(HRGN &rgn)
{
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::Region grgn;
    if (graphics->GetClip(&grgn) != Gdiplus::Ok)
        return false;
    rgn = grgn.GetHRGN(graphics);
    return rgn != 0;
}

bool Canvas::GetClipBounds(Rect &r)
{
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::Rect rect;
    if (graphics->GetClipBounds(&rect) == Gdiplus::Ok)
    {
        r = rect;
        return true;
    }
    return false;
}

bool Canvas::GetClipBoundsF(RectF &r)
{
    Gdiplus::Graphics *graphics = Get();
    Gdiplus::RectF rect;
    if (graphics->GetClipBounds(&rect) == Gdiplus::Ok)
    {
        r = rect;
        return true;
    }
    return false;
}

void Canvas::SetClip(const Rect &rect)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->SetClip(Gdiplus::Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top)) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::SetClip";
}

void Canvas::SetClip(HRGN rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Region r2(rgn);
    if (graphics->SetClip(&r2) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::SetClip2";
}

void Canvas::SetClip(const Gdiplus::Region &rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->SetClip(&rgn) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::SetClip3";
}

void Canvas::ExcludeClip(const Rect &rect)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->ExcludeClip(Gdiplus::Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top)) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::ExcludeClip";
}

void Canvas::ExcludeClip(HRGN rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Region r(rgn);
    if (graphics->ExcludeClip(&r) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::ExcludeClip2";
}

void Canvas::ExcludeClip(const Gdiplus::Region &rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->ExcludeClip(&rgn) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::ExcludeClip3";
}

void Canvas::IncludeClip(const Rect &rect)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->SetClip(Gdiplus::Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top), CombineModeUnion) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::IncludeClip";
}

void Canvas::IncludeClip(HRGN rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Region r(rgn);
    if (graphics->SetClip(&r, CombineModeUnion) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::IncludeClip2";
}

void Canvas::IncludeClip(const Gdiplus::Region &rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->SetClip(&rgn, CombineModeUnion) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::IncludeClip3";
}

void Canvas::IntersectClip(const Rect &rect)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->IntersectClip(Gdiplus::Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top)) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::IntersectClip";
}

void Canvas::IntersectClip(HRGN rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    Region r(rgn);
    if (graphics->IntersectClip(&r) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::IntersectClip2";
}

void Canvas::IntersectClip(const Gdiplus::Region &rgn)
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    if (graphics->IntersectClip(&rgn) != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::IntersectClip3";
}

CanvasGraphicsState Canvas::SaveState()
{
    using namespace Gdiplus;
    Gdiplus::Graphics *graphics = Get();
    CanvasGraphicsState state(graphicsstatepos++, this, graphics->Save());
    savedstates.push_back(state);
    return state;
}

void Canvas::RestoreState(CanvasGraphicsState state)
{
    using namespace Gdiplus;
    if (state.owner != this)
        throw L"You are trying to restore a state you didn't get from this canvas";
    for (auto it = savedstates.rbegin(); it != savedstates.rend(); ++it)
    {
        if (state.id == (*it).id)
        {
            Gdiplus::Graphics *graphics = Get();
            if (graphics->Restore(state.gdistate) != Ok)
                throw L"Invalid operation on GDI+ graphics object. Canvas::RestoreState";
            SetFont(state.statefont);
            SetAdvancedTextMode(state.stateadvanced);
            SetPen(state.statepen);
            SetBrush(state.statebrush);

            antialias = state.antialias;
            pommode = state.pommode;
            intmode = state.intmode;
            textalign = state.textalign;

            defaultmatrix = state.defaultmatrix;
            defaultmatrixflag = state.defaultmatrixflag;
            usedefaultmatrix = state.usedefaultmatrix;
            bitmapmatrix = state.bitmapmatrix;
            bitmapmatrixflag = state.bitmapmatrixflag;
            usebitmapmatrix = state.usebitmapmatrix;
            brushmatrix = state.brushmatrix;
            brushmatrixflag = state.brushmatrixflag;
            usebrushmatrix = state.usebrushmatrix;
            penmatrix = state.penmatrix;
            penmatrixflag = state.penmatrixflag;
            usepenmatrix = state.usepenmatrix;
            usedefaultcolorkey = state.usedefaultcolorkey;
            usebitmapcolorkey = state.usebitmapcolorkey;
            usebrushcolorkey = state.usebrushcolorkey;
            usepencolorkey = state.usepencolorkey;
            defaultlokey = state.defaultlokey;
            defaulthikey = state.defaulthikey;
            bitmaplokey = state.bitmaplokey;
            bitmaphikey = state.bitmaphikey;
            brushlokey = state.brushlokey;
            brushhikey = state.brushhikey;
            penlokey = state.penlokey;
            penhikey = state.penhikey;
            usetransf = state.usetransf;
            transf = state.transf;

            SetupImageAttributes();
            SetupTransformations();

            *state.skipdelete = true;

            savedstates.erase((it + 1).base(), savedstates.end());
            return;
        }
    }

    throw L"State not found!";
}

Size Canvas::MeasureText(const std::wstring& text)
{
    Get();
    HDC dc = GetDC();
    //HFONT of = (HFONT)SelectObject(dc, font->Handle());
    Size s;
    GetTextExtentPoint32(dc, text.c_str(), text.size(), &s);
    //SelectObject(dc, of);
    ReturnDC();
    return s;
}

void Canvas::TextDraw(const Rect &clip, int x, int y, const std::wstring& text)
{
    //using namespace Gdiplus;
    Get();
    HDC dc = GetDC();
    //HFONT of = (HFONT)SelectObject(dc, font->Handle());
    ExtTextOut(dc, x, y, ETO_CLIPPED, &clip, text.c_str(), text.size(), 0);
    //SelectObject(dc, of);
    ReturnDC();
}

void Canvas::DrawGrayText(const Rect &clip, int x, int y, const std::wstring& text)
{
    Get();
    HDC dc = GetDC();
    //HFONT of = (HFONT)SelectObject(dc, font->Handle());
    COLORREF oldcol = SetTextColor(dc, Color(clGrayText));
    ExtTextOut(dc, x, y, ETO_CLIPPED, &clip, text.c_str(), text.size(), 0);
    SetTextColor(dc, oldcol);
    //SelectObject(dc, of);
    ReturnDC();
}

Size Canvas::MeasureFormattedText(const std::wstring& text, TextDrawOptionSet options, int wantedwidth)
{
    Get();
    HDC dc = GetDC();
    Rect r = Rect(0, 0, wantedwidth, 0);
    //HFONT of = (HFONT)SelectObject(dc, font->Handle());
    int h = DrawText(dc, text.c_str(), text.length(), &r, (UINT)options | DT_CALCRECT);
    //SelectObject(dc, of);
    ReturnDC();

    return Size(r.right, h);
}

void Canvas::FormatText(const Rect &r, const std::wstring& text, TextDrawOptionSet options)
{
    Get();
    HDC dc = GetDC();
    //HFONT of = (HFONT)SelectObject(dc, font->Handle());
    DrawText(dc, text.c_str(), text.length(), const_cast<Rect*>(&r), options);
    //SelectObject(dc, of);
    ReturnDC();
}

void Canvas::FormatGrayText(const Rect &r, const std::wstring& text, TextDrawOptionSet options)
{
    Get();
    HDC dc = GetDC();
    //HFONT of = (HFONT)SelectObject(dc, font->Handle());
    COLORREF oldcol = SetTextColor(dc, Color(clGrayText));
    DrawText(dc, text.c_str(), text.length(), const_cast<Rect*>(&r), options);
    SetTextColor(dc, oldcol);
    //SelectObject(dc, of);
    ReturnDC();
}

bool Canvas::Antialias()
{
    return antialias;
}

void Canvas::SetAntialias(bool newantialias)
{
    if (newantialias == antialias)
        return;
    antialias = newantialias;
    if (GraphicsCreated())
    {
        Get()->SetSmoothingMode(antialias ? Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
    }
}

void Canvas::UpdateGraphics(Gdiplus::Graphics *graphics)
{
    using namespace Gdiplus;

    graphics->SetSmoothingMode(antialias ? SmoothingModeAntiAlias : SmoothingModeNone);
    graphics->SetInterpolationMode((Gdiplus::InterpolationMode)intmode);
    graphics->SetPixelOffsetMode((Gdiplus::PixelOffsetMode)pommode);
    if (usetransf)
        graphics->SetTransform(&transf);
    if (graphics && graphics->GetLastStatus() != Ok)
        throw L"Invalid operation on GDI+ graphics object. Canvas::UpdateGraphics";
}

void Canvas::Flush()
{
    Gdiplus::Graphics *graphics = Get();
    graphics->Flush();
}

CanvasTextAlignmentSet Canvas::TextAlignment()
{
    return textalign;
}

void Canvas::SetTextAlignment(CanvasTextAlignmentSet newalign)
{
    textalign = newalign;
}

TEXTMETRIC Canvas::TextMetrics()
{
    Get();
    HDC dc = GetDC();
    TEXTMETRIC tm;
    GetTextMetrics(dc, &tm);
    ReturnDC();

    return tm;
}

bool Canvas::AdvancedTextMode()
{
    return advanced;
}

void Canvas::SetAdvancedTextMode(bool newadvanced)
{
    if (advanced == newadvanced)
        return;
    advanced = newadvanced;
}


//---------------------------------------------

StockCanvas* StockCanvas::GetInstance()
{
    if (!instance)
        instance = new StockCanvas();
    return instance;
}

void StockCanvas::FreeInstance()
{
    delete instance;
    instance = NULL;
}

void StockCanvas::SettingsChanged()
{
    ClearStock();
    FillStock();
}


//---------------------------------------------


ControlCanvas::ControlCanvas(Control *owner) : owner(owner), dc(NULL), device(NULL), gotdc(NULL), graphics(NULL)
{
}

ControlCanvas::ControlCanvas(HDC dc) : owner(NULL), dc(dc), device(NULL), gotdc(NULL), graphics(NULL)
{
}

ControlCanvas::~ControlCanvas()
{
    if (HasCanvasState(csDCBusy))
        ReturnDC();
    ReleaseGraphics();
}

void ControlCanvas::Update(HDC dc, HANDLE device)
{
    if (dc == this->dc && device == this->device)
        return;
    Release();
    this->dc = dc;
    this->device = device;
}

#ifdef DEBUG
int gotgraphicscnt = 0;
#endif

Gdiplus::Graphics* ControlCanvas::GetGraphics()
{
    using namespace Gdiplus;
    if (HasCanvasState(csDCBusy))
        throw L"The DC is busy. Can't start any operation on it until ReturnDC is called.";

    if (graphics)
        return graphics;

    if (device && dc)
        graphics = new Gdiplus::Graphics(dc, device);
    else if (dc)
        graphics = new Gdiplus::Graphics(dc);
    else
        graphics = new Gdiplus::Graphics(owner->Handle());

#ifdef DEBUG
    ++gotgraphicscnt;
#endif

    if (graphics && graphics->GetLastStatus() != Ok)
        throw L"Invalid graphics object from GDI+.";

    return graphics;
}

void ControlCanvas::ReleaseGraphics()
{
    using namespace Gdiplus;
    if (HasCanvasState(csDCBusy))
        throw L"Cannot release the graphics object while the dc is taken! Did you forget to call ReturnDC?";

#ifdef DEBUG
    if (graphics)
        --gotgraphicscnt;
#endif

    delete graphics;
    graphics = NULL;

    dc = NULL;
    device = NULL;
}

#ifdef DEBUG
int gotdccnt = 0;
#endif
HDC ControlCanvas::_GetDC()
{
    using namespace Gdiplus;
//    if (!graphics && dc)
//    {
//#ifdef DEBUG
//        ++gotdccnt;
//#endif
//        Get();
//        gotdc = graphics->GetHDC();
//        AddCanvasState(csDCBusy);
//        return dc;
//    }
//    else
//    {
        if (!graphics)
            Get();
        gotdc = graphics->GetHDC();
        if (gotdc)
        {
#ifdef DEBUG
            ++gotdccnt;
#endif
        }
        //else
        //    throw L"Couldn't acquire a dc for drawing.";
        return gotdc;
//    }
}

void ControlCanvas::_ReturnDC()
{
    using namespace Gdiplus;

    if (!HasCanvasState(csDCBusy))
        throw L"No DC to return.";

    RemoveCanvasState(csDCBusy);
    if (graphics)
    {
#ifdef DEBUG
        --gotdccnt;
#endif
        graphics->ReleaseHDC(gotdc);
        if (graphics->GetLastStatus() != Ok)
            throw L"DC not returned correctly.";
    }
    else
        throw L"Graphics object not created.";
}

bool ControlCanvas::GraphicsCreated()
{
    return graphics != NULL;
}


//---------------------------------------------


Bitmap::Bitmap(char) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL) // This constructor is private.
{
}

Bitmap::Bitmap(Gdiplus::PixelFormat pixelformat) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    bmp = new Gdiplus::Bitmap(1, 1, pixelformat);
}

Bitmap::Bitmap(int width, int height, Gdiplus::PixelFormat pixelformat) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    bmp = new Gdiplus::Bitmap(width, height, pixelformat);
}

Bitmap::Bitmap(HMODULE module, const wchar_t *resource) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    HRSRC res = FindResource(module, resource, RT_RCDATA);
    if (res == NULL)
        throw L"Can't find resource with this name";

    int datasize = SizeofResource(module, res);
    void *data = LockResource(LoadResource(module, res));
    if (!data)
        throw L"Couldn't load the resource.";

    MemoryIStream *stream = new MemoryIStream(data, datasize);
    BitmapFromStream(stream);
    stream->Release();
    if (!bmp)
        throw L"Couldn't create bitmap from stream on resource.";
}

Bitmap::Bitmap(void *filedata, int datasize) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    MemoryIStream *stream = new MemoryIStream(filedata, datasize);
    BitmapFromStream(stream);
    stream->Release();
    if (!bmp)
        throw L"Couldn't create bitmap from stream.";
}

Bitmap::Bitmap(IStream* stream) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    BitmapFromStream(stream);
    if (!bmp)
        throw L"Couldn't create bitmap from IStream.";
}

Bitmap::Bitmap(const std::wstring &filename) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    bmp = Gdiplus::Bitmap::FromFile(filename.c_str());
    if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
    {
        delete bmp;
        bmp = NULL;
    }

    // Make the bitmap independent from the file.
    if (bmp)
    {
        if (!LockBits()) // Bug in some GDI+ implementations prevent locking bits if the image was opened from a stream or file.
        {
            CloneConstruct();
            // Try again
            //if (bmp)
            //{
            //    if (!LockBits())
            //    {
            //        delete bmp;
            //        bmp = 0;
            //    }
            //}
        }
        else
            UpdateBits();

        if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
        {
            delete bmp;
            bmp = NULL;
        }
    }

    if (!bmp)
        throw L"Couldn't load the bitmap.";
}

//Bitmap::Bitmap(const Bitmap &orig) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
//{
//    if (orig.HasCanvasState(csDCBusy))
//        throw L"Return the DC of the original bitmap before creating a copy.";
//    if (orig.bitmapstate.contains(bsLocked))
//        throw L"Call UpdateBits of the original bitmap before creating a copy.";
//
//    bmp = orig.bmp->Clone(0, 0, orig.Width(), orig.Height(), PixelFormatDontCare);
//}

Bitmap::Bitmap(Bitmap &&orig) noexcept : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    *this = std::move(orig);
}

Bitmap::Bitmap(HBITMAP hbmp) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    // Copy the bits in hbmp to the gdi+ bitmap to preserve the alpha channel and avoid handle sharing problems.
    HDC dc = 0;
    byte *buff = NULL;
    Gdiplus::BitmapData lockedbits;
    bool locked = false;
    try
    {
        dc = ::GetDC(0);

        // Get the image information from the handle.
        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        int result = GetDIBits(dc, hbmp, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
        if (result == FALSE || result == ERROR_INVALID_PARAMETER)
            throw L"Couldn't get the image information.";

        int w = bmi.bmiHeader.biWidth;
        int h = abs(bmi.bmiHeader.biHeight);
        bmp = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!bmp)
            throw L"Couldn't allocate memory for bitmap.";

        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;
        bmi.bmiHeader.biClrUsed = 0;
        bmi.bmiHeader.biClrImportant = 0;
        bmi.bmiHeader.biHeight = -abs(bmi.bmiHeader.biHeight);

        // Get the bits for the color bitmap.
        int scanwidth = w * 4;
        buff = new byte[scanwidth * h];
        result = GetDIBits(dc, hbmp, 0, h, buff, &bmi, DIB_RGB_COLORS);
        if (result == FALSE || result == ERROR_INVALID_PARAMETER)
            throw L"Couldn't get the image information.";

        Gdiplus::Rect r = Gdiplus::Rect(0, 0, w, h);
        locked = bmp->LockBits(&r, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &lockedbits) == Gdiplus::Ok;
        if (!locked)
            throw L"Couldn't lock the bits of the destination bitmap.";
        byte *line = (byte*)lockedbits.Scan0;
        byte *buffpos = buff;
        for (int y = 0; y < h; ++y)
        {
            memcpy(line, buffpos, w * 4);
            buffpos += w * 4;
            line += lockedbits.Stride;
        }
        bmp->UnlockBits(&lockedbits);
        locked = false;
    }
    catch(...)
    {
        if (locked)
            bmp->UnlockBits(&lockedbits);
        delete bmp;
        delete[] buff;
        if (dc)
            ReleaseDC(0, dc);
        throw;
    }
    delete[] buff;
    ReleaseDC(0, dc);

    if (!bmp)
        throw L"Couldn't load the bitmap.";
}

void Bitmap::BitmapFromStream(IStream *stream)
{
    bmp = Gdiplus::Bitmap::FromStream(stream);
    if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
    {
        delete bmp;
        bmp = NULL;
    }
    if (!bmp)
        throw L"Couldn't create a new bitmap from stream.";

    if (!LockBits()) // Bug in some GDI+ implementations prevent locking bits if the image was opened from a stream or file.
    {
        CloneConstruct();
        // Try again
        //if (bmp)
        //{
        //    if (!LockBits())
        //    {
        //        delete bmp;
        //        bmp = 0;
        //    }
        //    if (!bmp)
        //        throw L"Couldn't lock bitmap from stream.";
        //}
    }
    else
        UpdateBits();

    if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
    {
        delete bmp;
        bmp = NULL;
    }

    if (!bmp)
        throw L"Couldn't copy bitmap from stream.";
}

void Bitmap::CloneConstruct()
{
    Gdiplus::Bitmap *tmp = bmp;

    auto pf = tmp->GetPixelFormat();
    int w = tmp->GetWidth();
    int h = tmp->GetHeight();

    bmp = new Gdiplus::Bitmap(w, h, pf);

    if (!bmp)
    {
        delete tmp;
        return;
    }
    if (pf == PixelFormat1bppIndexed || pf == PixelFormat4bppIndexed || pf == PixelFormat8bppIndexed)
    {
        int palsize = tmp->GetPaletteSize();
        char *pal = new char[palsize];
        Gdiplus::ColorPalette *colors = (Gdiplus::ColorPalette*)pal;
        if (tmp->GetPalette(colors, palsize) != Gdiplus::Ok)
        {
            delete[] pal;
            delete tmp;
            delete bmp;
            bmp = 0;
            return;
        }
        if (bmp->SetPalette(colors) != Gdiplus::Ok)
        {
            delete[] pal;
            delete tmp;
            delete bmp;
            bmp = 0;
            return;
        }
        delete[] pal;

        Gdiplus::BitmapData tmpdata;
        Gdiplus::BitmapData bmpdata;
        Gdiplus::Rect r(0, 0, w, h);
        if (tmp->LockBits(&r, Gdiplus::ImageLockModeRead, pf, &tmpdata) != Gdiplus::Ok)
        {
            delete tmp;
            delete bmp;
            bmp = 0;
            return;
        }
        if (bmp->LockBits(&r, Gdiplus::ImageLockModeWrite, pf, &bmpdata) != Gdiplus::Ok)
        {
            delete tmp;
            delete bmp;
            bmp = 0;
            return;
        }


        if (tmpdata.Stride >= 0 && bmpdata.Stride >= 0)
            memcpy(bmpdata.Scan0, tmpdata.Scan0, tmpdata.Height * tmpdata.Stride);
        else
        {
            byte *tmpbuf = (byte*)tmpdata.Scan0;
            byte *bmpbuf = (byte*)bmpdata.Scan0;
            int len = abs(tmpdata.Stride);
            for (unsigned int ix = 0; ix < tmpdata.Height; ++ix)
            {
                memcpy(bmpbuf, tmpbuf, len);
                tmpbuf += tmpdata.Stride;
                bmpbuf += bmpdata.Stride;
            }
        }

        tmp->UnlockBits(&tmpdata);
        bmp->UnlockBits(&bmpdata);
    }
    else
    {
        auto gr = Gdiplus::Graphics::FromImage(bmp);
        if (!gr || gr->DrawImage(tmp, 0, 0, w, h) != Gdiplus::Ok)
        {
            delete gr;
            delete tmp;
            delete bmp;
            bmp = 0;
            return;
        }
    }
    delete tmp;
}

Bitmap&& Bitmap::MoveCopy(const Bitmap &orig)
{
    if (orig.HasCanvasState(csDCBusy))
        throw L"Return the DC of the original bitmap before creating a copy.";
    if (orig.bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits of the original bitmap before creating a copy.";

    Bitmap b('\0');
    b.bmp = nullptr;
    b.gotdc = 0;
    b.bitmapstate = 0;
    b.lockedbits = nullptr;

    b.bmp = orig.bmp->Clone(0, 0, orig.Width(), orig.Height(), PixelFormatDontCare);

    return std::move(b);
}

Bitmap* Bitmap::CreateCopy(const Bitmap &orig)
{
    if (orig.HasCanvasState(csDCBusy))
        throw L"Return the DC of the original bitmap before creating a copy.";
    if (orig.bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits of the original bitmap before creating a copy.";

    Bitmap *b = new Bitmap('\0');
    b->bmp = nullptr;
    b->gotdc = 0;
    b->bitmapstate = 0;
    b->lockedbits = nullptr;

    b->bmp = orig.bmp->Clone(0, 0, orig.Width(), orig.Height(), PixelFormatDontCare);

    return b;
}

void Bitmap::Copy(const Bitmap &src)
{
    if (HasCanvasState(csDCBusy))
        throw L"Return the DC before changing the bitmap.";
    if (bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits before changing the bitmap.";
    if (src.HasCanvasState(csDCBusy))
        throw L"Return the DC of the original bitmap before creating a copy.";
    if (src.bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits of the original bitmap before creating a copy.";

    ReleaseGraphics();
    delete bmp;
    bmp = nullptr;
    gotdc = 0;
    bitmapstate = 0;
    lockedbits = nullptr;

    bmp = src.bmp->Clone(0, 0, src.Width(), src.Height(), PixelFormatDontCare);
}

//Bitmap& Bitmap::operator=(const Bitmap &orig)
//{
//    if (HasCanvasState(csDCBusy))
//        throw L"Return the DC before changing the bitmap.";
//    if (bitmapstate.contains(bsLocked))
//        throw L"Call UpdateBits before changing the bitmap.";
//    if (orig.HasCanvasState(csDCBusy))
//        throw L"Return the DC of the original bitmap before creating a copy.";
//    if (orig.bitmapstate.contains(bsLocked))
//        throw L"Call UpdateBits of the original bitmap before creating a copy.";
//
//    ReleaseGraphics();
//    delete bmp;
//    gotdc = NULL;
//    bitmapstate = 0;
//    lockedbits = NULL;
//
//    bmp = orig.bmp->Clone(0, 0, orig.Width(), orig.Height(), PixelFormatDontCare);
//    return *this;
//}

Bitmap& Bitmap::operator=(Bitmap &&orig) noexcept
{
    std::swap(bmp, orig.bmp);
    std::swap(graphics, orig.graphics);
    std::swap(lockedbits, orig.lockedbits);
    std::swap(gotdc, orig.gotdc);
    std::swap(bitmapstate, orig.bitmapstate);

    return *this;
}

Bitmap::Bitmap(const Bitmap &orig, const Rect &srcrect) : bmp(NULL), graphics(NULL), lockedbits(NULL), gotdc(NULL)
{
    if (orig.HasCanvasState(csDCBusy))
        throw L"Return the DC of the original bitmap before creating a copy.";
    if (orig.bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits of the original bitmap before creating a copy.";

    bmp = orig.bmp->Clone((int)srcrect.left, (int)srcrect.top, (int)srcrect.Width(), (int)srcrect.Height(), PixelFormatDontCare);
}

Bitmap::~Bitmap()
{
    if (HasCanvasState(csDCBusy))
        ReturnDC();
    if (bitmapstate.contains(bsLocked))
        UpdateBits();

    ReleaseGraphics();
    delete bmp;
}

// Helper function for saving the bitmap.
int GetEncoderClsid(GdiplusImageEncoders format, CLSID* pClsid)
{
    using namespace Gdiplus;
    const wchar_t *encoderstrings[] = { L"image/bmp", L"image/jpeg", L"image/gif", L"image/tiff", L"image/png" };

    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if(size == 0)
        return -1;  // Failure

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j)
    {
        if( wcscmp(pImageCodecInfo[j].MimeType, encoderstrings[(int)format]) == 0 )
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

bool Bitmap::Save(const std::wstring &filename, GdiplusImageEncoders encoder)
{
    if (HasCanvasState(csDCBusy))
        throw L"Return the DC before saving the bitmap.";
    if (bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits before saving the bitmap.";

    if (encoder == gieAuto)
    {
        std::wstring s = GenToLower(GetFileExtension(filename));
        //  gieBMP = 0, gieJPEG, gieGIF, gieTIFF, giePNG
        if (s == L"bmp")
            encoder = gieBMP;
        else if (s == L"jpg" || s == L"jpeg")
            encoder = gieJPEG;
        else if (s == L"gif")
            encoder = gieGIF;
        else if (s == L"tiff")
            encoder = gieTIFF;
        else if (s == L"png")
            encoder = giePNG;
        else
            return false;
    }

    CLSID clsid;
    if (GetEncoderClsid(encoder, &clsid) < 0)
        return false;
    return bmp->Save(filename.c_str(), &clsid, NULL) == Gdiplus::Ok;
}

bool Bitmap::Save(IStream *stream, GdiplusImageEncoders encoder)
{
    if (HasCanvasState(csDCBusy))
        throw L"Return the DC before saving the bitmap.";
    if (bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits before saving the bitmap.";
    if (encoder == gieAuto)
        throw L"Cannot save to stream with gieAuto";

    CLSID clsid;
    if (GetEncoderClsid(encoder, &clsid) < 0)
        return false;
    return bmp->Save(stream, &clsid, NULL) == Gdiplus::Ok;
}

bool Bitmap::Load(IStream* stream)
{
    if (HasCanvasState(csDCBusy))
        throw L"Return the DC before loading a new bitmap.";
    if (bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits before loading a new bitmap.";

    ReleaseGraphics();
    delete bmp;

    bmp = NULL;
    graphics = NULL;
    lockedbits = NULL;
    gotdc = NULL;

    BitmapFromStream(stream);

    //bmp = Gdiplus::Bitmap::FromStream(stream);
    //if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
    //{
    //    delete bmp;
    //    bmp = NULL;
    //    return false;
    //}

    // // Make the bitmap indipendent from the stream's HGLOBAL.
    //if (bmp)
    //{
    //    LockBits();
    //    UpdateBits();
    //}


    if (!bmp)
        return false;

    return true;
}

bool Bitmap::Load(const std::wstring &filename)
{
    if (HasCanvasState(csDCBusy))
        throw L"Return the DC before loading a new bitmap.";
    if (bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits before loading a new bitmap.";

    ReleaseGraphics();
    delete bmp;

    bmp = NULL;
    graphics = NULL;
    lockedbits = NULL;
    gotdc = NULL;

    //bmp = Gdiplus::Bitmap::FromFile(filename.c_str());
    //if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
    //{
    //    delete bmp;
    //    bmp = NULL;
    //    return false;
    //}

    // // Make the bitmap indipendent from the file.
    //if (bmp)
    //{
    //    LockBits();
    //    UpdateBits();
    //}

    bmp = Gdiplus::Bitmap::FromFile(filename.c_str());
    if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
    {
        delete bmp;
        bmp = NULL;
    }

    // Make the bitmap indipendent from the file.
    if (bmp)
    {
        if (!LockBits()) // Bug in some GDI+ implementations prevent locking bits if the image was opened from a stream or file.
        {
            CloneConstruct();
            // Try again
            //if (bmp)
            //{
            //    if (!LockBits())
            //    {
            //        delete bmp;
            //        bmp = 0;
            //    }
            //}
        }
        else
            UpdateBits();
        if (bmp && bmp->GetLastStatus() != Gdiplus::Ok)
        {
            delete bmp;
            bmp = NULL;
        }
    }

    if (!bmp)
        return false;

    return true;
}

Gdiplus::Graphics* Bitmap::GetGraphics()
{
    using namespace Gdiplus;
    if (HasCanvasState(csDCBusy))
        throw L"The DC is busy. Can't start any operation on it until ReturnDC is called.";
    if (bitmapstate.contains(bsLocked))
        throw L"The bits of the bitmap are locked. Call UpdateBits before getting the graphics.";

    if (graphics)
        return graphics;
    graphics = Gdiplus::Graphics::FromImage(bmp);
    UpdateGraphics(graphics);

    return graphics;
}

void Bitmap::ReleaseGraphics()
{
    using namespace Gdiplus;
    if (!graphics)
        return;

    if (HasCanvasState(csDCBusy))
        throw L"Cannot release graphics with DC taken. Call ReturnDC first.";

    delete graphics;
    graphics = NULL;
}

void Bitmap::SetSize(int newwidth, int newheight)
{
    if (HasCanvasState(csDCBusy))
        throw L"Return the DC before changing the bitmap's size.";
    if (bitmapstate.contains(bsLocked))
        throw L"Call UpdateBits before changing the bitmap's size.";

    Gdiplus::Graphics *gorig = graphics;
    Gdiplus::Bitmap *tmp = bmp;
    graphics = NULL;

    bmp = new Gdiplus::Bitmap(newwidth, newheight, bmp->GetPixelFormat());
    Get();
    if (graphics->DrawImage(tmp,0,0) != Gdiplus::Ok)
    {
        delete graphics;
        delete bmp;
        bmp = tmp;
        graphics = gorig;
        throw L"Gdiplus error occured while resizing the bitmap.";
    }

    delete gorig;
    delete tmp;
}

HDC Bitmap::_GetDC()
{
    using namespace Gdiplus;

    if (bitmapstate.contains(bsLocked))
        return 0; //throw L"Cannot get the DC of a locked bitmap. Call UpdateBits first.";

    if (!graphics)
        graphics = Gdiplus::Graphics::FromImage(bmp);

    gotdc = graphics->GetHDC();
    return gotdc;
}

void Bitmap::_ReturnDC()
{
    using namespace Gdiplus;

    if (!HasCanvasState(csDCBusy))
        throw L"No DC to return.";
    RemoveCanvasState(csDCBusy);
    graphics->ReleaseHDC(gotdc);
}

bool Bitmap::CompatibleDC()
{
    return false;
}

Gdiplus::BitmapData* Bitmap::LockBits(GdiplusLockMode lockmode, Gdiplus::PixelFormat format)
{
    using namespace Gdiplus;
    if (bitmapstate.contains(bsLocked))
        throw L"Bits of this bitmap are already locked. Call UpdateBits first.";
    if (HasCanvasState(csDCBusy))
        throw L"The bitmap's DC is taken by a GetDC call. Return it with ReturnDC first.";

    bitmapstate << bsLocked;
    lockedbits = new Gdiplus::BitmapData();

    if (format == PixelFormatDontCare)
        format = bmp->GetPixelFormat();

    Gdiplus::Rect rtmp = Gdiplus::Rect(0,0,bmp->GetWidth(),bmp->GetHeight());
    if (bmp->LockBits(&rtmp, lockmode == glmReadOnly ? Gdiplus::ImageLockModeRead : lockmode == glmWriteOnly ? Gdiplus::ImageLockModeWrite : (Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeRead), format, lockedbits) != Gdiplus::Ok)
    {
        delete lockedbits;
        lockedbits = 0;
        bitmapstate -= bsLocked;
    }
    return lockedbits;
}

Gdiplus::BitmapData* Bitmap::LockBits(const Rect &area, GdiplusLockMode lockmode, Gdiplus::PixelFormat format)
{
    using namespace Gdiplus;

    if (bitmapstate.contains(bsLocked))
        throw L"Bits of this bitmap are already locked. Call UpdateBits first.";
    lockedbits = new Gdiplus::BitmapData();

    bitmapstate << bsLocked;

    if (format == PixelFormatDontCare)
        format = bmp->GetPixelFormat();

    Gdiplus::Rect rtmp = area.Empty() ? Gdiplus::Rect(0, 0, Width(), Height()) : Gdiplus::Rect(area.left, area.top, area.right - area.left, area.bottom - area.top);
    if (bmp->LockBits(&rtmp, lockmode == glmReadOnly ? Gdiplus::ImageLockModeRead : lockmode == glmWriteOnly ? Gdiplus::ImageLockModeWrite : (Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeRead), format, lockedbits) != Gdiplus::Ok)
    {
        delete lockedbits;
        lockedbits = 0;
        bitmapstate -= bsLocked;
    }
    return lockedbits;
}

void Bitmap::UpdateBits()
{
    if (!bitmapstate.contains(bsLocked))
        throw L"No bits were locked for update.";
    bmp->UnlockBits(lockedbits);
    bitmapstate -= bsLocked;
    delete lockedbits;
    lockedbits = NULL;
}

Gdiplus::Bitmap* Bitmap::GetBitmap()
{
    if (bitmapstate.contains(bsLocked))
        throw L"The bitmap has locked bits. Call UpdateBits first..";
    if (HasCanvasState(csDCBusy))
        throw L"The bitmap's DC is taken by a GetDC call. Return it with ReturnDC first.";

    return bmp;
}

int Bitmap::Width() const
{
    return bmp->GetWidth();
}

int Bitmap::Height() const
{
    return bmp->GetHeight();
}

bool Bitmap::GraphicsCreated()
{
    return graphics != NULL;
}

Gdiplus::PixelFormat Bitmap::PixelFormat()
{
    return bmp->GetPixelFormat();
}

HBITMAP Bitmap::HandleCopy(Color background)
{
    if (bitmapstate.contains(bsLocked))
        throw L"The bitmap has locked bits. Call UpdateBits first..";
    if (HasCanvasState(csDCBusy))
        throw L"The bitmap's DC is taken by a GetDC call. Return it with ReturnDC first.";

    //HBITMAP hbm;
    //if (bmp->GetHBITMAP(background, &hbm) != Gdiplus::Ok)
    //    return NULL;
    //return hbm;

    int w = Width();
    int h = Height();

    Gdiplus::PixelFormat pf = PixelFormat();
    if (pf == PixelFormat16bppGrayScale || pf == PixelFormat16bppRGB565 || pf == PixelFormat16bppARGB1555)
        pf = PixelFormat16bppRGB555;
    else if (((int)pf >> 8 & 0xff) > 32)
        pf = ((int)pf & PixelFormatAlpha) == 0 || pf == PixelFormat48bppRGB || pf == PixelFormat32bppCMYK ? PixelFormat32bppRGB : PixelFormat32bppARGB;
    else if (pf == PixelFormat32bppRGB)
        pf = PixelFormat32bppARGB;

    int palsiz = 0;
    Gdiplus::ColorPalette *pal = nullptr;
    if ((pf & PixelFormatIndexed) != 0)
    {
        palsiz = bmp->GetPaletteSize();
        pal = (Gdiplus::ColorPalette*)new char[palsiz];
        bmp->GetPalette(pal, palsiz);
    }

    char *infptr = new char[sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (pal == nullptr ? 0 : pal->Count - 1)];
    BITMAPINFO *inf = (BITMAPINFO*)infptr;
    BITMAPINFOHEADER *bh = &inf->bmiHeader;
    bh->biSize = sizeof(BITMAPINFOHEADER);
    bh->biWidth = w;
    bh->biHeight = -h;
    bh->biPlanes = 1;
    int bitcnt = (pf >> 8) & 0xff;
    bh->biBitCount = bitcnt;
    bh->biCompression = BI_RGB;
    bh->biSizeImage = 0;
    bh->biXPelsPerMeter = 0; 
    bh->biYPelsPerMeter = 0; 
    bh->biClrUsed = pal == nullptr ? 0 : pal->Count;
    bh->biClrImportant = 0;

    if (pal)
    {
        memcpy(&inf->bmiColors, pal, sizeof(RGBQUAD) * pal->Count);
        delete[] pal;
        pal = nullptr;
    }

    HDC dc = ::GetDC(0);
    byte *bits;
    HBITMAP result = CreateDIBSection(dc, inf, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);

    int realw = (bitcnt * w + 7) / 8;
    int linew = int((bitcnt * w + 31) / 32) * 4;

    auto p = LockBits(Rect(0, 0, w, h), nlib::glmReadOnly, pf);
    byte *line = (byte*)p->Scan0;
    byte *bline = bits;

    bool usealpha = false;
    if (pf == PixelFormat32bppARGB) // Only PixelFormat32bppARGB is supported with alpha.
    {
        background = background.ToRGB();
        usealpha = background.A() != 0;
    }

    for (int iy = 0; iy < h; ++iy, line += p->Stride, bline += linew)
    {
        memcpy(bline, line, realw);
        
        if (!usealpha)
            continue;
        for (int ix = 0; ix < w; ++ix)
        {
            if (bline[ix * 4 + 3] == 255)
                continue;
            double d = (double)bline[ix * 4 + 3] / 255.;
            bline[ix * 4 + 0] = int(max(0, min(255, (d * bline[ix * 4 + 0]) + (1.0 - d) * (background.B()))));
            bline[ix * 4 + 1] = int(max(0, min(255, (d * bline[ix * 4 + 1]) + (1.0 - d) * (background.G()))));
            bline[ix * 4 + 2] = int(max(0, min(255, (d * bline[ix * 4 + 2]) + (1.0 - d) * (background.R()))));
            bline[ix * 4 + 3] = 255;
        }
    }

    ReleaseDC(0, dc);
    delete[] infptr;

    return result;
}

Color Bitmap::Pixel(int x, int y)
{
    if (bitmapstate.contains(bsLocked))
        throw L"The bitmap has locked bits. Call UpdateBits first..";
    if (HasCanvasState(csDCBusy))
        throw L"The bitmap's DC is taken by a GetDC call. Return it with ReturnDC first.";
    Gdiplus::Color col;
    if (bmp->GetPixel(x, y, &col) != Gdiplus::Ok)
        throw L"Couldn't get the pixel at the specified position!";

    return col;
}

void Bitmap::SetPixel(int x, int y, Color color)
{
    if (bitmapstate.contains(bsLocked))
        throw L"The bitmap has locked bits. Call UpdateBits first..";
    if (HasCanvasState(csDCBusy))
        throw L"The bitmap's DC is taken by a GetDC call. Return it with ReturnDC first.";

    if (bmp->SetPixel(x, y, color) != Gdiplus::Ok)
        throw L"Couldn't set the pixel at the specified position!";
}

ImageTypes Bitmap::GetImageType()
{
    GUID guid;
    bmp->GetRawFormat(&guid);
    if (guid == Gdiplus::ImageFormatMemoryBMP)
        return itMemoryBitmap;
    if (guid == Gdiplus::ImageFormatBMP)
        return itBMP;
    if (guid == Gdiplus::ImageFormatEMF)
        return itEMF;
    if (guid == Gdiplus::ImageFormatWMF)
        return itWMF;
    if (guid == Gdiplus::ImageFormatJPEG)
        return itJPEG;
    if (guid == Gdiplus::ImageFormatPNG)
        return itPNG;
    if (guid == Gdiplus::ImageFormatGIF)
        return itGIF;
    if (guid == Gdiplus::ImageFormatTIFF)
        return itTIFF;
#ifdef EXIF_GUID_SUPPORTED
    if (guid == Gdiplus::ImageFormatEXIF)
        return itEXIF;
#endif
    if (guid == Gdiplus::ImageFormatIcon)
        return itIcon;
    return itUndefined;
}

void Bitmap::TextDraw(const Rect &clip, int x, int y, const std::wstring& text)
{
    int w = Width();
    int h = Height();
    Rect r = Rect(max(0, clip.left), max(0, clip.top), min(clip.right, w), min(clip.bottom, h));
    if (r.Empty())
        return;

    Get();
    HDC dc = GetDC();

    HDC cdc = CreateCompatibleDC(dc);
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = r.Width();
    bmi.bmiHeader.biHeight = -r.Height();
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    byte *bits;
    HBITMAP bmp = CreateDIBSection(cdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

    auto lock = LockBits(r, glmReadWrite, PixelFormat32bppARGB);
    byte *line = (byte*)lock->Scan0;
    byte *bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(bline, line, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    GdiFlush();

    HBITMAP ob = (HBITMAP)SelectObject(cdc, bmp);
    HFONT of = (HFONT)SelectObject(cdc, GetFont().Handle());
    Rect r2 = RectS(0, 0, r.Width(), r.Height());

    SetBkMode(cdc, TRANSPARENT);
    ExtTextOut(cdc, x - r.left, y - r.top, ETO_CLIPPED, &r2, text.c_str(), text.size(), 0);

    SelectObject(cdc, of);
    SelectObject(cdc, ob);
    DeleteDC(cdc);

    line = (byte*)lock->Scan0;
    bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(line, bline, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    UpdateBits();

    DeleteObject(bmp);

    ReturnDC();
}

void Bitmap::DrawGrayText(const Rect &clip, int x, int y, const std::wstring& text)
{
    int w = Width();
    int h = Height();
    Rect r = Rect(max(0, clip.left), max(0, clip.top), min(clip.right, w), min(clip.bottom, h));
    if (r.Empty())
        return;

    Get();
    HDC dc = GetDC();

    HDC cdc = CreateCompatibleDC(dc);
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = r.Width();
    bmi.bmiHeader.biHeight = -r.Height();
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    byte *bits;
    HBITMAP bmp = CreateDIBSection(cdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

    auto lock = LockBits(r, glmReadWrite, PixelFormat32bppARGB);
    byte *line = (byte*)lock->Scan0;
    byte *bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(bline, line, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    GdiFlush();

    HBITMAP ob = (HBITMAP)SelectObject(cdc, bmp);
    HFONT of = (HFONT)SelectObject(cdc, GetFont().Handle());
    Rect r2 = RectS(0, 0, r.Width(), r.Height());

    SetBkMode(cdc, TRANSPARENT);
    SetTextColor(cdc, Color(clGrayText));
    ExtTextOut(cdc, x - r.left, y - r.top, ETO_CLIPPED, &r2, text.c_str(), text.size(), 0);

    SelectObject(cdc, of);
    SelectObject(cdc, ob);
    DeleteDC(cdc);

    line = (byte*)lock->Scan0;
    bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(line, bline, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    UpdateBits();

    DeleteObject(bmp);

    ReturnDC();
}

void Bitmap::FormatText(const Rect &clip, const std::wstring& text, TextDrawOptionSet options)
{
    int w = Width();
    int h = Height();
    Rect r = Rect(max(0, clip.left), max(0, clip.top), min(clip.right, w), min(clip.bottom, h));
    if (r.Empty())
        return;

    Get();
    HDC dc = GetDC();

    HDC cdc = CreateCompatibleDC(dc);
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = r.Width();
    bmi.bmiHeader.biHeight = -r.Height();
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    byte *bits;
    HBITMAP bmp = CreateDIBSection(cdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

    auto lock = LockBits(r, glmReadWrite, PixelFormat32bppARGB);
    byte *line = (byte*)lock->Scan0;
    byte *bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(bline, line, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    GdiFlush();

    HBITMAP ob = (HBITMAP)SelectObject(cdc, bmp);
    HFONT of = (HFONT)SelectObject(cdc, GetFont().Handle());
    Rect r2 = RectS(0, 0, clip.Width(), clip.Height());

    SetBkMode(cdc, TRANSPARENT);
    DrawText(cdc, text.c_str(), text.length(), &r2, options);

    SelectObject(cdc, of);
    SelectObject(cdc, ob);
    DeleteDC(cdc);

    line = (byte*)lock->Scan0;
    bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(line, bline, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    UpdateBits();

    DeleteObject(bmp);

    ReturnDC();
}

void Bitmap::FormatGrayText(const Rect &clip, const std::wstring& text, TextDrawOptionSet options)
{
    int w = Width();
    int h = Height();
    Rect r = Rect(max(0, clip.left), max(0, clip.top), min(clip.right, w), min(clip.bottom, h));
    if (r.Empty())
        return;

    Get();
    HDC dc = GetDC();

    HDC cdc = CreateCompatibleDC(dc);
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = r.Width();
    bmi.bmiHeader.biHeight = -r.Height();
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    byte *bits;
    HBITMAP bmp = CreateDIBSection(cdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

    auto lock = LockBits(r, glmReadWrite, PixelFormat32bppARGB);
    byte *line = (byte*)lock->Scan0;
    byte *bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(bline, line, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    GdiFlush();

    HBITMAP ob = (HBITMAP)SelectObject(cdc, bmp);
    HFONT of = (HFONT)SelectObject(cdc, GetFont().Handle());
    Rect r2 = RectS(0, 0, clip.Width(), clip.Height());

    SetBkMode(cdc, TRANSPARENT);
    SetTextColor(cdc, Color(clGrayText));
    DrawText(cdc, text.c_str(), text.length(), &r2, options);

    SelectObject(cdc, of);
    SelectObject(cdc, ob);
    DeleteDC(cdc);

    line = (byte*)lock->Scan0;
    bline = bits;
    for (int iy = 0; iy < -bmi.bmiHeader.biHeight; ++iy)
    {
        memcpy(line, bline, bmi.bmiHeader.biWidth * 4);
        bline += 4 * bmi.bmiHeader.biWidth;
        line += lock->Stride;
    }

    UpdateBits();

    DeleteObject(bmp);

    ReturnDC();
}


//---------------------------------------------


template<typename Entry, typename C>
static void _FillIconEntryVector(const char *data, unsigned int datalen, C &result)
{
    const char *pos = data;
    if (datalen < sizeof(IconDir))
        return;
    IconDir dir = *(IconDir*)data;
    if (dir.entrycnt == 0 || dir.icon == 0 || dir.icon > 2)
        return;
    pos += sizeof(IconDir);
    int entryhsize = sizeof(Entry);
    if (datalen < sizeof(IconDir) + dir.entrycnt * entryhsize)
        return;

    for (int ix = 0; ix < dir.entrycnt; ++ix, pos += entryhsize)
    {
        Entry entry;
        entry = *(Entry*)pos;
        if (entry.bitcnt == 0)
        {
            int palcnt = entry.colorcnt;
            while (!entry.bitcnt || palcnt > 1)
            {
                palcnt /= 2;
                entry.bitcnt++;
            }
            entry.bitcnt = max(1, entry.bitcnt / 3);
        }
        result.push_back(*(IconEntry*)&entry);
    }
}

template<typename C>
static void _FillIconEntryVector(const char *data, unsigned int datalen, C &result, bool icongroup)
{
    if (!icongroup)
        _FillIconEntryVector<IconDirEntry, C>(data, datalen, result);
    else
        _FillIconEntryVector<GroupIconDirEntry, C>(data, datalen, result);
}

void FillIconEntryVector(const char *data, unsigned int datalen, std::vector<IconEntry> &result, bool icongroup)
{
    _FillIconEntryVector(data, datalen, result, icongroup);
}

void FillIconEntryVector(const char *data, unsigned int datalen, std::list<IconEntry> &result, bool icongroup)
{
    _FillIconEntryVector(data, datalen, result, icongroup);
}

template<typename C>
static void _IconFromFile(FileReader &f, C &result)
{
    IconDir dir;
    f.Read(&dir, sizeof(IconDir));
    if (dir.entrycnt == 0 || dir.icon == 0 || dir.icon > 2)
        return;

    //size_t basepos = f.Tell();

    int entrycnt = 0;
    std::list<IconDirEntry> entries;
    entries.resize(dir.entrycnt);

    for (IconDirEntry &entry : entries)
    {
        if (f.Read(&entry, sizeof(IconDirEntry)) != sizeof(IconDirEntry))
            return;
        if (entry.bitcnt == 0)
        {
            int palcnt = entry.colorcnt;
            while (!entry.bitcnt || palcnt > 1)
            {
                palcnt /= 2;
                entry.bitcnt++;
            }
            entry.bitcnt = max(1, entry.bitcnt / 3);
        }
        ++entrycnt;
    }

    int added = 0;

    for (IconDirEntry &entry : entries)
    {
        char *dat = new char[entry.datasize];
        if (!dat)
            break;

        f.Seek(entry.dataoffset, frpBegin);
        if (f.Read(dat, entry.datasize) != entry.datasize)
        {
            delete[] dat;
            break;
        }

        HICON h = CreateIconFromResourceEx((byte*)dat, entry.datasize, TRUE, 0x00030000, entry.width, entry.height, 0);

        delete[] dat;

        if (h == NULL)
            break;

        result.push_back(h);
        ++added;
    }

    if (added != entrycnt)
    {
        while(added--)
            result.pop_back();
    }
}

void IconFromFile(FileReader &&f, std::vector<Icon> &result)
{
    _IconFromFile(f, result);
}

void IconFromFile(FileReader &&f, std::list<Icon> &result)
{
    _IconFromFile(f, result);
}

void IconFromFile(const std::wstring &fname, std::vector<Icon> &result)
{
    FILESTD ifstream fstr(fname, std::ios_base::in | std::ios_base::binary);
    IconFromFile(fstr, result);
}

void IconFromFile(const std::wstring &fname, std::list<Icon> &result)
{
    FILESTD ifstream fstr(fname, std::ios_base::in | std::ios_base::binary);
    IconFromFile(fstr, result);
}

void IconFromFile(FILE *f, std::vector<Icon> &result)
{
    IconFromFile(FileReader(f), result);
}

void IconFromFile(FILE *f, std::list<Icon> &result)
{
    IconFromFile(FileReader(f), result);
}

void IconFromFile(std::istream &stream, std::vector<Icon> &result)
{
    IconFromFile(FileReader(stream), result);
}

void IconFromFile(std::istream &stream, std::list<Icon> &result)
{
    IconFromFile(FileReader(stream), result);
}


template<typename Entry>
static int FindNearestIconEntry(std::vector<IconEntry> &result, int maxwidth, int maxheight, int maxcoldepth)
{
    int cnt = result.size();
    if (!cnt)
        return -1;

    int sel = 0;
    Entry *selected = (Entry*)&result[0];
    for (int ix = 1; ix < cnt; ++ix)
    {
        Entry &entry = *(Entry*)&result[ix];
        int selw = selected->width == 0 ? 256 : selected->width;
        int selh = selected->height == 0 ? 256 : selected->height;
        int w = entry.width == 0 ? 256 : entry.width;
        int h = entry.height == 0 ? 256 : entry.height;

        int bettersize = ((selw > maxwidth || selh > maxheight) && (w*h <= selw*selh)) || (selw <= maxwidth && selh <= maxheight && w <= maxwidth && h <= maxheight && w*h >= selw*selh) ? (w*h == selw*selh ? 0 : 1) : -1;
        bool betterbpp = (selected->bitcnt > maxcoldepth && entry.bitcnt < maxcoldepth) || (selected->bitcnt <= maxcoldepth && entry.bitcnt <= maxcoldepth && entry.bitcnt > selected->bitcnt);

        if (bettersize == 1 || (bettersize == 0 && betterbpp))
        {
            sel = ix;
            selected = (Entry*)&result[ix];
        }
    }
    return sel;
}

int FindNearestIconEntry(std::vector<IconEntry> &result, int maxwidth, int maxheight, int maxcoldepth, bool icongroup)
{
    if (!icongroup)
        return FindNearestIconEntry<IconDirEntry>(result, maxwidth, maxheight, maxcoldepth);
    else
        return FindNearestIconEntry<GroupIconDirEntry>(result, maxwidth, maxheight, maxcoldepth);
}


//---------------------------------------------


Icon::Icon() : handle(NULL), width(0), height(0)
{
}

Icon::Icon(HICON handle) : handle(handle), width(0), height(0)
{
    if (!handle)
        return;

    ICONINFO inf = {0};
    GetIconInfo(handle, &inf);
    HDC dc = 0;

    try
    {
        dc = GetDC(0);
        BITMAP bmpinfo;

        if (inf.hbmColor) // Color bitmap.
        {
            GetObject(inf.hbmColor, sizeof(BITMAP), &bmpinfo);
            width = bmpinfo.bmWidth;
            height = bmpinfo.bmHeight;
        }
        else // Black and white bitmap.
        {
            GetObject(inf.hbmMask, sizeof(BITMAP), &bmpinfo);
            width = bmpinfo.bmWidth;
            height = bmpinfo.bmHeight / 2;
        }
    }
    catch(...)
    {
        ;
    }

    if (dc)
        ReleaseDC(0, dc);
    if (inf.hbmMask)
        DeleteObject(inf.hbmMask);
    if (inf.hbmColor)
        DeleteObject(inf.hbmColor);
}

Icon::Icon(Icon &&ico) noexcept : handle(ico.handle), width(ico.width), height(ico.height)
{
    ico.handle = 0;
    ico.width = 0;
    ico.height = 0;
}

Icon& Icon::operator=(Icon &&ico) noexcept
{
    std::swap(handle, ico.handle);
    std::swap(width, ico.width);
    std::swap(height, ico.height);
    return *this;
}

Icon::~Icon()
{
    Clear();
}

Icon&& Icon::MoveCopy(const Icon &orig)
{
    Icon ico;
    ico.Copy(orig);

    return std::move(ico);
}

Icon* Icon::CreateCopy(const Icon &orig)
{
    Icon *ico = new Icon;
    ico->Copy(orig);

    return ico;
}

HICON Icon::Handle()
{
    return handle;
}

bool Icon::Empty() const
{
    return handle == 0;
}

void Icon::Clear()
{
    if (handle == 0)
        return;
    DestroyIcon(handle);
    handle = 0;
    width = 0;
    height = 0;
}

int Icon::Width() const
{
    if (width == 0)
        return 256;
    return width;
}

int Icon::Height() const
{
    if (height == 0)
        return 256;
    return height;
}

Size Icon::IconSize() const
{
    return Size(Width(), Height());
}

void Icon::Copy(const Icon &src)
{
    if (&src == this)
        return;
    Clear();
    handle = DuplicateIcon(NULL, src.handle);
    if (handle == 0) // Failed DuplicateIcon.
        return;
    width = src.width;
    height = src.height;
}

bool Icon::ToBitmap(Bitmap &bmp) const
{
    if (!handle)
        return false;

    ICONINFO inf = {0};
    GetIconInfo(handle, &inf);
    HDC dc = 0;
    byte *colbuff = NULL;
    byte *maskbuff = NULL;
    byte *bmibuf = NULL;
    byte *buff = NULL;
    byte *mbuff = NULL;

    int w, h;

    try
    {
        dc = GetDC(0);

        if (inf.hbmColor) // Color bitmap.
        {
            // Get the image information from the handle.
            BITMAPINFO bmi;
            memset(&bmi, 0, sizeof(BITMAPINFO));
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

            int result = GetDIBits(dc, inf.hbmColor, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
            if (result == FALSE || result == ERROR_INVALID_PARAMETER)
                throw L"Couldn't get the image information.";

            w = bmi.bmiHeader.biWidth;
            h = abs(bmi.bmiHeader.biHeight);

            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            bmi.bmiHeader.biSizeImage = 0;
            bmi.bmiHeader.biClrUsed = 0;
            bmi.bmiHeader.biClrImportant = 0;
            bmi.bmiHeader.biHeight = -abs(bmi.bmiHeader.biHeight);

            // Get the bits for the color bitmap.
            int scanwidth = w * 4;
            buff = new byte[scanwidth * h];
            result = GetDIBits(dc, inf.hbmColor, 0, h, buff, &bmi, DIB_RGB_COLORS);
            if (result == FALSE || result == ERROR_INVALID_PARAMETER)
                throw L"Couldn't get the image information.";

            int mscanwidth;

            bool hasalpha = false;
            // Scan the pixels to see whether there is alpha channel in the bitmap data.
            for (int ix = 0; ix < w * h && !hasalpha; ++ix)
                hasalpha = buff[ix * 4 + 3] != 0;

            if (!hasalpha)
            {
                // Get the bits for the monochrome bitmap.
                bmibuf = new byte[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)];
                memset(bmibuf, 0, sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
                BITMAPINFO &mbmi = *(BITMAPINFO*)bmibuf;
                mbmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                mbmi.bmiHeader.biWidth = w;
                mbmi.bmiHeader.biHeight = -h /* * 2*/;
                mbmi.bmiHeader.biPlanes = 1;
                mbmi.bmiHeader.biBitCount = 1;
                mbmi.bmiHeader.biCompression = BI_RGB;
                mbmi.bmiHeader.biSizeImage = 0;
                mbmi.bmiHeader.biClrUsed = 0;
                mbmi.bmiHeader.biClrImportant = 0;

                mscanwidth = (w + 7) / 8;
                mscanwidth = (mscanwidth + sizeof(DWORD) - 1) / sizeof(DWORD) * sizeof(DWORD); // Fit the line length to DWORD boundary.

                mbuff = new byte[mscanwidth * h /* * 2*/];
                result = GetDIBits(dc, inf.hbmMask, 0, h /** 2*/, mbuff, &mbmi, DIB_RGB_COLORS);
                if (result == FALSE || result == ERROR_INVALID_PARAMETER)
                    throw L"Couldn't get the image information.";
            }

            bmp = Bitmap(w, h, bmp.PixelFormat());

            auto bits = bmp.LockBits(glmWriteOnly, PixelFormat32bppARGB);
            byte *line = (byte*)bits->Scan0;
            byte *buffpos = buff; //bmi.bmiHeader.biHeight < 0 ? buff + scanwidth * (h - 1)
            byte *mbuffpos = NULL;
            if (!hasalpha)
                mbuffpos = mbuff; //((BITMAPINFO*)bmibuf)->bmiHeader.biHeight < 0 ? mbuff + mscanwidth * (h - 1)
            for (int y = 0; y < h; ++y)
            {
                memcpy(line, buffpos, scanwidth);

                if (!hasalpha)
                {
                    for (int x = 0; x < w; ++x)
                    {
                        //line[x * 4 + 0] = buffpos[x * 4 + 0];
                        //line[x * 4 + 1] = buffpos[x * 4 + 1];
                        //line[x * 4 + 2] = buffpos[x * 4 + 2];
                        line[x * 4 + 3] = (mbuffpos[x / 8] & (1 << (7 - (x % 8)))) == 0 ? 255 : 0;
                    }

                    mbuffpos += mscanwidth /* * (((BITMAPINFO*)bmibuf)->bmiHeader.biHeight < 0 ? -1 : 1)*/;
                }

                line += bits->Stride;
                buffpos += scanwidth /* * (bmi.bmiHeader.biHeight < 0 ? -1 : 1)*/;
            }
            bmp.UpdateBits();
        }
        else // Black and white bitmap.
        {
            // Get the image information from the handle.
            bmibuf = new byte[sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD)];
            memset(bmibuf, 0, sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
            BITMAPINFO &mbmi = *(BITMAPINFO*)bmibuf;
            mbmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

            int result = GetDIBits(dc, inf.hbmMask, 0, 0, NULL, &mbmi, DIB_RGB_COLORS);
            if (result == FALSE || result == ERROR_INVALID_PARAMETER)
                throw L"Couldn't get the image information.";

            w = mbmi.bmiHeader.biWidth;
            h = abs(mbmi.bmiHeader.biHeight) / 2;
            mbmi.bmiHeader.biHeight = -(mbmi.bmiHeader.biHeight);
            mbmi.bmiHeader.biPlanes = 1;
            mbmi.bmiHeader.biBitCount = 1;
            mbmi.bmiHeader.biCompression = BI_RGB;
            mbmi.bmiHeader.biSizeImage = 0;
            mbmi.bmiHeader.biClrUsed = 0;
            mbmi.bmiHeader.biClrImportant = 0;

            int mscanwidth = (w + 7) / 8;
            mscanwidth = (mscanwidth + sizeof(DWORD) - 1) / sizeof(DWORD) * sizeof(DWORD); // Fit the line length to DWORD boundary.

            mbuff = new byte[mscanwidth * h * 2];
            result = GetDIBits(dc, inf.hbmMask, 0, h * 2, mbuff, &mbmi, DIB_RGB_COLORS);
            if (result == FALSE || result == ERROR_INVALID_PARAMETER)
                throw L"Couldn't get the image information.";

            bmp = Bitmap(w, h, bmp.PixelFormat());
            auto bits = bmp.LockBits(glmWriteOnly, PixelFormat32bppARGB);
            byte *line = (byte*)bits->Scan0;

            byte *mbuffpos = mbuff;
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    line[x * 4 + 0] = (mbuffpos[h * mscanwidth + x / 8] & (1 << (7 - (x % 8)))) == 0 ? 0 : 255;
                    line[x * 4 + 1] = (mbuffpos[h * mscanwidth + x / 8] & (1 << (7 - (x % 8)))) == 0 ? 0 : 255;
                    line[x * 4 + 2] = (mbuffpos[h * mscanwidth + x / 8] & (1 << (7 - (x % 8)))) == 0 ? 0 : 255;
                    line[x * 4 + 3] = (mbuffpos[x / 8] & (1 << (7 - (x % 8)))) == 0 ? 255 : 0;
                }

                mbuffpos += mscanwidth /* * (((BITMAPINFO*)bmibuf)->bmiHeader.biHeight < 0 ? -1 : 1)*/;
                line += bits->Stride;
            }
            bmp.UpdateBits();
        }
    }
    catch(...)
    {
        ;
    }

    if (dc)
        ReleaseDC(0, dc);
    if (inf.hbmMask)
        DeleteObject(inf.hbmMask);
    if (inf.hbmColor)
        DeleteObject(inf.hbmColor);
    delete[] colbuff;
    delete[] maskbuff;
    delete[] bmibuf;
    delete[] buff;
    delete[] mbuff;

    return true;
}


void Icon::Draw(Canvas *canvas, int x, int y) const
{
    if (Empty())
        throw L"Cannot draw uninitialized icon.";

    HDC dc = canvas->GetDC();
    if (!dc)
        throw L"Couldn't acquire dc from canvas!";

    ::DrawIconEx(dc, x, y, handle, 0, 0, 0, 0, DI_NORMAL);

    canvas->ReturnDC();
}

void Icon::Draw(Canvas *canvas, int x, int y, int width, int height) const
{
    if (Empty())
        throw L"Cannot draw uninitialized icon.";

    HDC dc = canvas->GetDC();
    if (!dc)
        throw L"Couldn't acquire dc from canvas!";

    ::DrawIconEx(dc, x, y, handle, width, height, 0, 0, DI_NORMAL);

    canvas->ReturnDC();
}


// --------------------------------------------------------------------


   //                              Point Size * LOGPIXELSY
   // height = Internal Leading + -------------------------
   //                                        72

   //          -(Point Size * LOGPIXELSY)
   // height = --------------------------
   //                       72
   //
   // plf->lfHeight = -MulDiv (nPtSize, GetDeviceCaps (hdc, LOGPIXELSY), 72);

   //              (Height - Internal Leading) * 72
   // Point Size = --------------------------------
   //                         LOGPIXELSY

float FontSizeFromLogfont(const LOGFONT &lf, HDC measuredc)
{
    bool error = false;
    HDC tmpdc = measuredc;
    HFONT tmpfont = NULL;
    HFONT ofont = NULL;
    TEXTMETRIC tm;
    float fontsize = 0;
    try {
        if (tmpdc == NULL)
            tmpdc = GetDC(0);

        if (lf.lfHeight < 0)
            fontsize = float((float(-lf.lfHeight) * 72.0) / (float)LogPixelsY);
        else
        {
            tmpfont = CreateFontIndirect(&lf);
            ofont = (HFONT)SelectObject(tmpdc, tmpfont);
            GetTextMetrics(tmpdc, &tm);
            fontsize = float(float((lf.lfHeight - tm.tmInternalLeading) * 72.0) / (float)LogPixelsY);
        }
    }
    catch(...)
    {
        error = true;
    }
    if (ofont != NULL)
        SelectObject(tmpdc,ofont);
    if (tmpfont)
        DeleteObject(tmpfont);
    if (tmpdc && measuredc == NULL)
        ReleaseDC(0, tmpdc);

    if (error)
        throw L"Error getting font size.";

    return fontsize;
}

// Compute font size from given height. This function only works with negative values where the internal leading is not included
float FontSizeFromHeight(int height)
{
    if (height >= 0)
        throw L"Cannot compute font size from a positive height value!";

    return float(-height * 72.0) / (float)LogPixelsY ;
}

int FontHeightFromSize(float size)
{
    return int(((-size * (float)LogPixelsY) / 72.0) + (size > 0.0 ? -0.5 : 0.5));
}


//---------------------------------------------


}
/* End of NLIBNS */

