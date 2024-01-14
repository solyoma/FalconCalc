#include "stdafx_zoli.h"
#include "helperwindows.h"


//---------------------------------------------


namespace NLIBNS
{


LayeredWindow::LayeredWindow()
{
    controlstyle -= csChild | csMouseCapture | csParentColor | csUpdateOnTextChange | csEraseOnTextChange | csParentTooltip | csShowTooltip | csEraseToColor | csParentFont;
    controlstyle << csTransparent << csNoErase << csShowDontActivate;
    controlstate -= csVisible;
}

LayeredWindow::LayeredWindow(const LayeredWindow &other) : base(other), image(Bitmap::MoveCopy(other.image))
{
    controlstyle -= csChild | csMouseCapture | csParentColor | csUpdateOnTextChange | csEraseOnTextChange | csParentTooltip | csShowTooltip | csEraseToColor | csParentFont;
    controlstyle << csTransparent << csNoErase << csShowDontActivate;
    controlstate -= csVisible;

    UpdateImage();
}

LayeredWindow::LayeredWindow(const Bitmap &otherimage) : image(Bitmap::MoveCopy(otherimage))
{

    controlstyle -= csChild | csMouseCapture | csParentColor | csUpdateOnTextChange | csEraseOnTextChange | csParentTooltip | csShowTooltip | csEraseToColor | csParentFont;
    controlstyle << csTransparent << csNoErase << csShowDontActivate;
    controlstate -= csVisible;

    UpdateImage();
}

LayeredWindow::LayeredWindow(Bitmap &&otherimage) : image(std::move(otherimage))
{
    controlstyle -= csChild | csMouseCapture | csParentColor | csUpdateOnTextChange | csEraseOnTextChange | csParentTooltip | csShowTooltip | csEraseToColor | csParentFont;
    controlstyle << csTransparent << csNoErase << csShowDontActivate;
    controlstate -= csVisible;

    UpdateImage();
}

LayeredWindow::~LayeredWindow()
{
}

void LayeredWindow::Destroy()
{

    base::Destroy();
}

void LayeredWindow::CreateClassParams(ClassParams &params)
{
    base::CreateClassParams(params);
}

void LayeredWindow::CreateWindowParams(WindowParams &params)
{
    base::CreateWindowParams(params);

    params.extstyle = wsExLayered | wsExTransparent | wsExNoActivate;
    params.style = wsPopup;
}

void LayeredWindow::Show()
{
    if (HandleCreated() && Visible())
        return;
    UpdateImage();
    base::Show();
}

void LayeredWindow::UpdateImage()
{
    HDC dc = GetDC(NULL);
    if (!dc)
        return;

    Rect r = WindowRect();
    Point pos = r.TopLeft();
    Size siz = Size(image.Width(), image.Height());
    Point pt;

    HDC cdc = CreateCompatibleDC(dc);

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    //HBITMAP bmp = CreateCompatibleBitmap(cdc, image.Width(), image.Height()); //image.HandleCopy(clNone);
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = image.Width();
    bmi.bmiHeader.biHeight = -image.Height();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    char *bits;
    HBITMAP bmp = CreateDIBSection(cdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

    auto i = image.LockBits(glmReadOnly, PixelFormat32bppARGB);
    char *line = (char*)i->Scan0;
    for (int ix = 0; ix < -bmi.bmiHeader.biHeight; ++ix)
    {
        memcpy(bits, line, 4 * bmi.bmiHeader.biWidth);
        bits += i->Stride;
        line += i->Stride;
    }
    image.UpdateBits();

    GdiFlush();

    HBITMAP obmp = (HBITMAP)SelectObject(cdc, bmp);

    UpdateLayeredWindow(Handle(), dc, &pos, &siz, cdc, &pt, clWhite, &bf, ULW_ALPHA);

    SelectObject(cdc, obmp);
    DeleteObject(bmp);
    DeleteDC(cdc);
    ReleaseDC(NULL, dc);
}

const Bitmap& LayeredWindow::Image()
{
    return image;
}

void LayeredWindow::SetImage(const Bitmap &newimage)
{
    image.Copy(newimage);
    if (HandleCreated() && Visible())
        UpdateImage();
}

void LayeredWindow::SetImage(Bitmap &&newimage)
{
    image = std::move(newimage);
    if (HandleCreated() && Visible())
        UpdateImage();
}

void LayeredWindow::SetTopLeft(const Point &tl)
{
    base::SetBounds(RectS(tl.x, tl.y, image.Width(), image.Height()));
}


//---------------------------------------------


}
/* End of NLIBNS */

