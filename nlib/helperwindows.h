#pragma once

#include "generalcontrol.h"
#include "canvas.h"


namespace NLIBNS
{


/* Window class with the only purpose to display a floating image with transparency, eg. a dragged image.
 * The window cannot be placed on another control. If controls are placed on it, their drawing must be
 * done by the window.
 */
class LayeredWindow : public Form
{
private:
    typedef Form base;

    Bitmap image;
protected:
    LayeredWindow(const LayeredWindow &other);
    ~LayeredWindow();

    virtual void CreateClassParams(ClassParams &params);
    virtual void CreateWindowParams(WindowParams &params);

    void UpdateImage();

    using base::SetBounds;
public:
    LayeredWindow();
    LayeredWindow(const Bitmap &image);
    LayeredWindow(Bitmap &&image);
    virtual void Destroy();

    virtual void Show();

    const Bitmap& Image();
    void SetImage(const Bitmap &newimage);
    void SetImage(Bitmap &&newimage);

    void SetTopLeft(const Point &tl);
};


}
/* End of NLIBNS */

