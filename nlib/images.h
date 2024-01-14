#pragma once

#include "events.h"


namespace NLIBNS
{


class Bitmap;
class Imagelist;

#ifdef DESIGNING
class ControlImage;
struct ControlBitmap
{
    ControlImage *owner;

    Bitmap *image;
    byte statecount;

    // Saved file properties for serialization.
    std::wstring filename;
    char *filedata;
    unsigned int filesize;
    Color transparent; // Which color should be made transparent in the original file. When this is clNone, no transparent pixels are specified.

    ControlBitmap();
    bool operator==(const ControlBitmap& other) const;
    void Free();
    void CopyTo(ControlBitmap &other) const;
};
#endif

enum ControlImageStates { cisNormal = 0, cisDisabled, cisPressed, cisDown, cisFocused, cisHovered };
enum ControlImageTypes { citBitmap, citImagelist };
class ControlImage : public Object
{
private:
    typedef Object  base;
    
    Control *owner;

    ControlImageTypes imgtype;
    bool transform; // Draw unspecified image states with added effect to available ones.

#ifdef DESIGNING
    // Only used for passing around for the property getter and setter functions
    ControlBitmap ctrlimg;
#endif

    Bitmap *image;
    bool ownbmp; // The bitmap is owned by the ControlImage object.
    byte statecount;

    Imagelist *images;
    int index;
    int disabled;
    int pressed;
    int down;
    int focused;
    int hovered;

protected:
    virtual void DeleteNotify(Object *object);
    virtual void ChangeNotify(Object *object, int changetype);

    virtual ~ControlImage();
public:
    ControlImage(Control *owner);

#ifdef DESIGNING
    static void EnumerateProperties(DesignSerializer *serializer);

    // Only used for passing around for the property getter and setter functions
    ControlBitmap GetControlBitmap();
    void SetControlBitmap(ControlBitmap newbmp);

    virtual ControlBitmap DefaultBitmap();
#endif

    void Draw(Canvas *dest, int x, int y, ControlImageStates state);
    int Width() const;
    int Height() const;
    bool HasImage() const;

    ControlImageTypes ImageType();

    bool Transform();
    void SetTransform(bool newtransform);

    Bitmap *GetBitmap();
    void SetBitmap(Bitmap *newbitmap, bool shared = false);
    int StateCount() const;
    void SetStateCount(int newstatecount);

    Imagelist *Images();
    void SetImages(Imagelist *newimagelist);
    int ImageIndex() const;
    void SetImageIndex(int newimageindex);
    int DisabledIndex() const;
    void SetDisabledIndex(int newdisabledindex);
    int PressedIndex() const;
    void SetPressedIndex(int newpressedindex);
    int DownIndex() const;
    void SetDownIndex(int newdownindex);
    int FocusedIndex() const;
    void SetFocusedIndex(int newfocusedindex);
    int HoveredIndex() const;
    void SetHoveredIndex(int newhoveredindex);

    Form *ParentForm() const; // Returns the parent form of the owner control.
};



}
/* End of NLIBNS */

