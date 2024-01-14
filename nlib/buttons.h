#pragma once

#include "syscontrol.h"
#include "images.h"


namespace NLIBNS
{


    /*  ButtonImagePositions: Position of the image relative to the text when the image is shown in a button.
     *      bipLeft: Image to the left of the text.
     *      bipAbove: Image above text.
     *      bipRight: Image to the right of the text.
     *      bipBelow: Image below text.
     *      bipCenter: Image and text at the center, image behind the text.
     */
    enum ButtonImagePositions {
            bipLeft, bipAbove, bipRight, bipBelow, bipCenter,
#ifdef DESIGNING
            bipCount = 5
#endif
    };

    /*  ButtonContentPositions: Which side the margin should be relative to when positioning image and text in a button.
     *      If the contents of the button are positioned relative to one side and the margin is -1, it will be 6 * Scaling.
     *      bcpLeft: Image and text positioned relative to the left side of the button.
     *      bcpTop: Image and text positioned relative to the top of the button.
     *      bcpRight: Image and text positioned relative to the right side of the button.
     *      bcpBottom: Image and text positioned relative to the bottom of the button.
     *      bcpCenter: Image and text positioned at the center of the button.
     */
    enum ButtonContentPositions {
            bcpLeft, bcpTop, bcpRight, bcpBottom, bcpCenter,
#ifdef DESIGNING
            bcpCount = 5
#endif
    };

    class Button : public SystemControl
    {
    private:
        typedef SystemControl base;

        bool cancel; // True if the escape key should activate the button.
    protected:
        virtual void CreateClassParams(ClassParams &params);
        virtual void CreateWindowParams(WindowParams &params);

        virtual void EraseBackground();
        virtual Rect OpaqueRect();

        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual bool HandleCommand(Control *parent, WPARAM wParam);
        //virtual bool HandleCtlColor(Control *parent, HDC dc, HBRUSH &bgbrush);

        virtual void Paint(const Rect &updaterect);

        virtual ~Button();
    public:
#ifdef DESIGNING
        static void EnumerateProperties(DesignSerializer *serializer);

        virtual Size DesignSize();
#endif

        Button();

        bool Cancel();
        void SetCancel(bool newcancel);

        void Click();

        NotifyEvent OnClick;
    };


    /*
     * FlatButtonTypes: Describes possible button behavior (but not appearance)
     *  fbtPushbutton - normal button behavior.
     *  fbtCheckbutton - checkbox behavior.
     *  fbtCheckRadiobutton - radiobuttons but they can be all unpressed.
     *  fbtRadiobutton - radiobox behavior.
     *  fbtDropdownButton - Button with additional space and a small triangle at the top right corner.
     *  fbtSplitbutton - Button with a separate area with a small triangle that is only shown hot if the cursor is over it not the main button body.
     */
    enum FlatButtonTypes {
            fbtPushbutton, fbtCheckbutton, fbtCheckRadiobutton, fbtRadiobutton, fbtDropdownButton, fbtSplitbutton,
#ifdef DESIGNING
            fbtCount = 6
#endif
    };

    /*
     * ButtonOwnerDrawOperations: Used for owner drawn flat buttons in the drawing event. Specifies the current operation to be done.
     *  bodoBackground - button background drawing, excluding the split button area. For non-flat buttons, this operation should draw the background of the whole button, including the split button area.
     *  bodoSecondaryBackground - button's split button background drawing. For non-flat buttons, this operation is not used.
     *  bodoContents - image and text drawing in the main button area.
     *  bodoSecondaryContents - drawing of the down arrow or other image in the split button area.
     */
    enum ButtonOwnerDrawOperations : int {
        bodoBackground, bodoSecondaryBackground, bodoContents, bodoSecondaryContents
    };

    /*
     * ButtonOwnerDrawState: The current state of the button for in the drawing event. Enabled or focused states can be checked from the control, so those are not included.
     *  bodsNormal - not pressed, not focused and not hovered.
     *  bodsHovered - mouse pointer over the main button area.
     *  bodsSecondaryHovered - mouse pointer over the split button area.
     *  bodsPressed - mouse pointer pressed while hovering the main button area.
     *  bodsSecondaryPressed - mouse pointer pressed while hovering the split button area.
     *  bodsDown - the button was pressed and stays down.
     */
    enum ButtonOwnerDrawState : int {
        bodsNormal, bodsHovered, bodsSecondaryHovered, bodsPressed, bodsSecondaryPressed, bodsDown
    };

    class FlatButton : public Control
    {
    private:
        typedef Control base;

        FlatButtonTypes type;
        bool flat;
        bool down; // Call the OnDownChanged event after changing this value.
        int groupindex;
        bool hovered;
        bool splithovered; // The split button side's down, not for the whole button.
        bool pressed; // Mouse button was pressed while cursor was over the control.
        bool showtext;
        ButtonImagePositions imagepos;
        ButtonContentPositions contentpos;
        int margin; // Distance of the contained (image / text) from the aligned side of the button. -1 means auto.
        int spacing; // Space between the image and the text. -1 means auto.

        bool showfocus; // Draw the focus rectangle on the button.
        bool accelvisible; // Draw the prefix line for the accelerator character that comes after the &.

        bool cancel;

        ControlImage *image;

        void PopButtons(bool thistoo); // Sets all buttons within the same group to down = false. Except this one if thistoo is false.
        void UpdateButtonTypes(FlatButtonTypes newtype); // Sets all buttons within the same group to a new type. Usually to switch between fbtCheckRadiobutton and fbtRadiobutton.
        bool DownInGroup(); // Returns true when there is another button with the same groupindex that is down.
        int UnusedIndex(); // Returns a groupindex that is 1 higher than the current highest.
        FlatButtonTypes GroupType(); // The type of other FlatButtons with the same groupindex as this one.
    protected:
        virtual void CreateClassParams(ClassParams &params);

        //virtual void CreateWindowParams(WindowParams &params);
        virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        virtual void Paint(const Rect &updaterect);
        virtual void EraseBackground();

        virtual void Resizing();

        virtual void GainFocus(HWND otherwindow);
        virtual void LoseFocus(HWND otherwindow);

        virtual void CaptureChanged();
        virtual void MouseEnter();
        virtual void MouseLeave();
        virtual void MouseMove(short x, short y, VirtualKeyStateSet vkeys);
        virtual void MouseDown(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);
        virtual void MouseUp(short x, short y, MouseButtons button, VirtualKeyStateSet vkeys);

        virtual void KeyPush(WORD &keycode, WCHAR &key, VirtualKeyStateSet vkeys);

        virtual void ChangeNotify(Object *object, int changetype);

        virtual ~FlatButton();
    public:
#ifdef DESIGNING
        virtual Size DesignSize();
        static void EnumerateProperties(DesignSerializer *serializer);
#endif 

        FlatButton();

        Rect ButtonRect(); // Client rectangle excluding the right part of a split button.
        Rect SplitRect(); // Rectangle fro the right part of a split button in client coordinates.
        int SplitWidth(); // Width of the right part of a split button.

        ControlImage* Image();

        bool Down();
        void SetDown(bool newdown);
        FlatButtonTypes Type();
        void SetType(FlatButtonTypes newtype);

        int GroupIndex();
        void SetGroupIndex(int newindex);
        bool Flat();
        void SetFlat(bool newflat);
        bool ShowText();
        void SetShowText(bool newshowtext);

        ButtonImagePositions ImagePosition();
        void SetImagePosition(ButtonImagePositions newpos);

        ButtonContentPositions ContentPosition();
        void SetContentPosition(ButtonContentPositions newpos);

        int Margin();
        void SetMargin(int newmargin);
        int Spacing();
        void SetSpacing(int newspacing);
        void SetImageLayout(ButtonImagePositions newpos, int newmargin, int newspacing);

        bool Cancel();
        void SetCancel(bool newcancel);

        void Click();
        void SplitClick();

        NotifyEvent OnClick; // Called when the button was clicked with the mouse.
        NotifyEvent OnDownChanged; // Called when the down value of the button has changed, even if it was changed programatically. Only called for the button being pushed for grouped buttons, if another button was down in the same group.
        NotifyEvent OnSplitClick; // Called when the button is a splitbutton and the right side was clicked.

        ButtonMeasureSplitSizeEvent OnMeasureSplitSize; // Called when drawing the button. The width of the split button area can be changed as needed.
        ButtonOwnerDrawEvent OnPaint; // Called before each drawing operation in the button. It is possible to replace the drawing for only one part of the button.
    };

    struct ButtonMeasurements
    {
        Rect btnarea; // (In) Area of the button. For split buttons, this area shouldn't include the splitter and the right split part. Same as the passed value.
        Size imgsize; // (In) Size of button image. Same as the passed value.

        Size ts; // (Out) Size of button text. Measured using the passed canvas.

        int margin; // (Out) Measured margin if the passed value was -1. Otherwise the passed value scaled to the current dpi.
        int spacing; // (Out) Measured spacing if the passed value was -1. Otherwise the passed value scaled to the current dpi.

        Rect content; // (Out) Position and area of contents.
        Point textpos; // (Out) Position of text relative to the content rectangle.
        Point imgpos; // (Out) Position of image relative to the content rectangle.
    };
    void MeasureButtonContents(Canvas *canvas, const Rect &btnarea, const std::wstring &text, const Size &imgsize, ButtonImagePositions imagepos, ButtonContentPositions contentpos, int margin, int spacing, ButtonMeasurements &mes);

}
/* End of NLIBNS */

