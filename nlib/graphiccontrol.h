#pragma once

#include "controlbase.h"


namespace NLIBNS
{


    enum LabelShowAccelerators {
                lsaShow,
                lsaHide,
                lsaAuto,
#ifdef DESIGNING
                lsaCount = 3
#endif
    };
    class Label : public Control
    {
    private:
        typedef Control base;

        Control *accesscontrol;

        bool autosize;
        bool wordwrap;
        LabelShowAccelerators accel;
        bool accelvisible;
        //bool enabled;

        TextAlignments textalign;
        VerticalTextAlignments valign;

        Size Measure(const Rect &rect); // Measures the new size of the label with auto size, given an initial window rectangle.
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);
        virtual void InitHandle();
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual bool HandleSysKey(WCHAR key);

        virtual void WindowBoundsChanged(const Rect &oldrect, const Rect &newrect);
        virtual void Paint(const Rect &updaterect);
        virtual void EraseBackground();
        virtual void Showing();

        virtual void DeleteNotify(Object *object);

        virtual ~Label();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual Size DesignSize();
#endif

        Label();

        bool Transparent();
        void SetTransparent(bool newtransparent);

        bool AutoSize();
        void SetAutoSize(bool newautosize);

        bool WordWrap();
        void SetWordWrap(bool newwordwrap);

        LabelShowAccelerators ShowAccelerator();
        void SetShowAccelerator(LabelShowAccelerators newshowaccelerator);
        bool AcceleratorVisible();

        //bool Enabled();
        //void SetEnabled(bool newenabled);

        Control *AccessControl();
        void SetAccessControl(Control *newaccesscontrol);

        TextAlignments TextAlignment();
        void SetTextAlignment(TextAlignments newtextalign);

        virtual void SetText(const std::wstring &newtext);

        VerticalTextAlignments VerticalTextAlignment();
        void SetVerticalTextAlignment(VerticalTextAlignments newverticalalign);

        void Resize(); // Sets control size based on current text and settings.
    };

    enum BevelLineTypes {
                bltSunken, bltRaised, bltDoubleSunken, bltDoubleRaised, bltSunkenRaised, bltRaisedSunken,
#ifdef DESIGNING
                bltCount = 6
#endif
                        };
    enum BevelShapeTypes {
                bstLeftLine, bstTopLine, bstRightLine, bstBottomLine, bstMiddleHorzLine, bstMiddleVertLine, bstBox, bstSpacer,
#ifdef DESIGNING
                bstCount = 8
#endif
                         };
    class Bevel : public Control
    {
    private:
        typedef Control base;

        BevelLineTypes linetype;
        BevelShapeTypes shape;

        Rect client;
        Rect LineRect(Rect r); // Returns the rectangle of the bevel's shape if its window has the passed client rectangle.
    protected:
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void Paint(const Rect &updaterect);

        virtual ~Bevel();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);
        virtual Size DesignSize();
#endif

        Bevel();

        BevelLineTypes LineType();
        void SetLineType(BevelLineTypes newlinetype);

        BevelShapeTypes Shape();
        void SetShape(BevelShapeTypes newshape);

    };


}
/* End of NLIBNS */

