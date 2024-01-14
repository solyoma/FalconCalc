#pragma once

#include "objectbase.h"


namespace NLIBNS
{


class Bitmap;
class Canvas;

enum ImagelistColorDepths {
        icdDeviceDependent = ILC_COLORDDB, icdDefault = ILC_COLOR, icd4bit = ILC_COLOR4, icd8bit = ILC_COLOR8, icd16bit = ILC_COLOR16, icd24bit = ILC_COLOR24, icd32bit = ILC_COLOR32,
#ifdef DESIGNING
        icdCount = 7
#endif
        };

class Imagelist : public NonVisualControl
{
private:
    typedef NonVisualControl    base;

    HIMAGELIST handle;
    
    int imgwidth;
    int imgheight;
    bool masked;
    ImagelistColorDepths colordepth;
    int count;

    Color paintcolor;
    Color maskcolor;

    void CreateHandle();
    void DestroyHandle();

    HBITMAP HandleFromBitmap(Bitmap *bmp); // Copies the specified bitmap into a handle of the format of the imagelist and returns the handle. It must be deleted later.
protected:
    void Changed();

    virtual ~Imagelist();
public:
#ifdef DESIGNING
    static void EnumerateProperties(DesignSerializer *serializer);
#endif
    Imagelist();
    Imagelist(int imgwidth, int imgheight, bool masked, ImagelistColorDepths colordepth);
    Imagelist(const Imagelist &orig);
    Imagelist(Imagelist &&orig);

    Imagelist& operator=(Imagelist &&orig);
    Imagelist& operator=(const Imagelist &orig);

    bool HandleCreated() const; // Indicates whether the imagelist handle is already created. Check this first if you don't want a handle to be created if it does not exist yet, instead of getting the handle with the Handle method.
    HIMAGELIST Handle(); // Returns the handle to the imagelist, or creates a new handle if it was not created yet.

    int Count() const;

    int Width() const; // Width of one image.
    int Height() const; // Height of one image.
    //int Width(); // Width of one image.
    //int Height(); // Height of one image.
    void SetSizes(int newwidth, int newheight); // Clears the imagelist and updates the size of images that can be added.
    void SetWidth(int newwidth); // Clears the imagelist and updates the size of images that can be added.
    void SetHeight(int newheight); // Clears the imagelist and updates the size of images that can be added.

    bool Masked(); // Returns whether the imagelist contains two separate bitmaps, one for the images and another for the mask for the images.
    void SetMasked(bool newmasked); // Clears the imagelist and sets whether the imagelist contains two separate bitmaps, one for the images and another for the mask for the images. 

    ImagelistColorDepths ColorDepth(); // Returns the current color depth of images in the imagelist.
    void SetColorDepth(ImagelistColorDepths newcolordepth); // Clears the images and sets a new color depth for images to be added.

    Color PaintColor(); // The color used to draw the background (instead of skipping the transparent pixels) when the images are masked.
    void SetPaintColor(Color newpaintcolor); // Change the color used to draw the background (instead of skipping the transparent pixels) when the images are masked.
    Color MaskColor(); // The color defaulted as transparent when adding images to the imagelist. Transparent gif and png images are first drawn to a bitmap filled with this color, then the color is made to be the mask color. Only used when mask is turned on, or when the color depth is not 32 bits.
    void SetMaskColor(Color newmaskcolor); // Spedifies the color defaulted as transparent when adding images to the imagelist. Transparent gif and png images are first drawn to a bitmap filled with this color, then the color is made to be the mask color. Only used when mask is turned on, or when the color depth is not 32 bits.

    // The imagelist doesn't keep the handles, those must be disposed of with DeleteObject manually.
    int Add(HBITMAP img, HBITMAP mask = NULL); // Mask is ignored when the imagelist is not masked. If the imagelist is masked and mask is null, the MaskColor is used as the color of pixels that will be turned transparent by a mask.
    int AddIcon(HICON icon); // Adds an icon to the imagelist.
    int Add(Bitmap *bmp); // Adds a bitmap converted to the imagelist's image format. If the imagelist is not masked, its color depth is 32 bits and the bitmap contains an alpha channel, its transparency is preserved. Otherwise it is first painted on MaskColor, and if the imagelist is masked, it is added as masked by that color.

    void Replace(int index, HBITMAP img, HBITMAP mask = NULL); // Same as Add, but replaces the image at the given index.
    void ReplaceIcon(int index, HICON icon);
    void Replace(int index, Bitmap *bmp);

    void Remove(int index);
    void Clear();

    bool WriteToStream(IStream *stream); // Writes the image data to the passed stream interface.
    bool ReadFromStream(IStream *stream); // Reads the image data from the passed stream interface.
    void ReadFromResource(HMODULE module, const wchar_t *resname); // Reads the image data from a resource file.

    void Move(int src, int dest); // Moves the image at the source index to the dest index.

    HICON CreateIcon(int index); // Returns a handle of a freshly created icon from the image at the specified index. DestroyIcon must be used to destroy the returned icon handle.

    void Draw(Canvas *canvas, int index, int x, int y) const;
};


}
/* End of NLIBNS */

