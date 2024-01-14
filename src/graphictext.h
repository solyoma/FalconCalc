#pragma once

#include <string>
#include <vector>

// #include "numconv.h"
#include "controlbase.h"

namespace sa
{
	

// A GraphLabel contains a bitmap representation of its 'text'
// 'text' may contain the following meta characters:
//  { } ^ _ and \
// These characters have special meanings. They may be escaped by '\'.
// (A single '\' is represented in a C++ string as '\\', so a '\\' combination becomes
// "\\\\")
// The characters '{' and '}' delimit a block of characters
// a character outside {} is a block itself
// '^' means the box that follow will be an upper index in smaller sized letters
// '_' denotes a lower index  in smaller sized letters
// if a lower/upper index follows an upper/lower index then they will be printed
// from the same starting position, 
// ???? unless there is a single '|' between them, in ????
// which case '|' is not printed but the next index follows the previous one.
// if two or more upper or lower index follows each other they become indexes of the
// previous block

// 12^{alma}_{korte}^{,szilva}^3

struct GTransformation
{
	// ddeg >0 -> CCW,  <0 -> CW, because y grows downward
	// 'offs' - translate in x direction before rotation by (-offs)
	GTransformation(int ddeg, int offs) : rotation(ddeg), offset(offs) 
	{
		double deg = -ddeg/1800.0*3.141592653589;	// ddeg is in 0.1 degrees
		sine = sin(deg);
		cosi = cos(deg);
	}
	SIZE Transform( int x, int y) 
	{ 
		SIZE s;
		s.cx = (int)((x - offset)* cosi -y * sine),
		s.cy = (int)((x - offset)*sine + y*cosi);
		return s;
	}
	int rotation;
	int offset;				// x offset only!
	double sine, cosi;
};

class GraphicText;

struct TextSection
{
	std::wstring &text;		// from 'GraphicText' object
	std::wstring myText;	// text section copied here
	nlib::Canvas *canvas;	// from 'GraphicText' object
	TextSection *next,		// next section at this text level
		*upper, *lower;		// upper/lower index of this level
	int len;				// in text	len = 0: base section
	int level;				// base: 0, upper > 0, lower < 0
	float nFontSize;		// 0 for same font as selected for the canvas super- and subscript size is 75%
	int x, y;				// of the **top left** of this section in pixels, relative to x=0,y=0
	nlib::Rect box;			// this is the rectangle which contains this text and all of its indices
							// i.e. the leftmost point of the text is 'box.left = x', 
							// highest point (smallest y) of it is 'box.top'
							// rightmost point is 'box.right' and lowest point is 'box.bottom'
							// where all coordinates are relative to x,y and in pixels
	int width, height;		// of this section w.o. indices when displayed on 'canvas' in pixels
	bool _no_processing;	// single section

	TextSection(std:: wstring &text, nlib::Canvas* canvas) : 
		text(text), canvas(canvas), next(0), upper(0), lower(0),
		len(0), nFontSize(12), x(0), y(0), box(), width(0), height(0), _no_processing(false) {}
	~TextSection() { delete upper; delete lower; delete next;}
	bool Process(int &pos, bool isSingleLtter=false);	// read section and process it. returns true at section end and false at end of text
	bool ProcessBrace(int &pos);	// read section and process it. returns true at section end and false at end of text
	void SetPositionAndFontSizes(int size=0, int left=0, int top=0);

	void Draw(int x0, int y0, GTransformation &gt);	// all text is drawn inside the area in 'box' whose left top is x0,y0

};

class GraphicText
{
	bool _visible;
	bool _no_processing;		// when set  a single section for the whole text
	std::wstring _text;
	nlib::Canvas *_canvas;		// The label will be drawn here
	nlib::CanvasTextAlignmentSet _alignment;
	int _rotation;				// in 0.1 degree
	TextSection *_pSections;
	double _dFontHeight;
	void SetupSections(nlib::Canvas* draw_canvas);
public:
	GraphicText();
	virtual ~GraphicText();

	std::wstring Text() const { return _text; }
	void SetCanvas(nlib::Canvas *newCanvas);	// recalculates pSections
	void SetText(std::wstring &text, nlib::Canvas* canvas = 0, bool single = false); 	// canvas==0 then no calculation, 'single' then single section

	void Recalc();	// change the font for the canvas then call this to recalculate  all sections

	nlib::Rect Box();	// area that contains all parts of the text.
				// relative to the left and top of the first section of text
				// To get the sides use Box().Width() and Box().Height()

	int Visible() const {return _visible; }
	bool Empty() const { return _text.empty(); }
	nlib::CanvasTextAlignmentSet Alignment() const { return _alignment; }
	void SetAlignment(nlib::CanvasTextAlignmentSet al);
	int Rotation() const { return _rotation; }
	void SetRotation(int ddeg);	// ddeg: 0.1 degree
	void Draw(int x0, int y0);	// x0, y0 - meaning depends on alignment and rotation:
								// no rotation and left aligned: y0=top, x0=left of first text section
								// rotation = 900  : text rateted CCW 90 degrees around x0,y0 
								// rotation = 2700  : text rateted CW 90 degrees around x0,y0 

	nlib::Canvas *GetCanvas() const { return _canvas; }
};

 // end of namespace sa
 }