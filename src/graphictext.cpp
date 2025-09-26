#include "stdafx_zoli.h"

// DEBUG
#include <fstream>

#include "graphictext.h"
#include "translations.h"

namespace sa
{

	// valid formats in text:  "A" => a single letter following an '^' or '_' is equivalent to "{A}
	//					"AAAA" "{BBBB}" : single section, 
	//					"AA{BB}CC"		: 3 sections on same level
	//					"AAAA^BCCC"     : a section "AAAA" with an upper section "B" followed by a non index section "CCCC", 
	//					"AAAA^B_C",		: a section "AAAA" with an upper section "B" and a lower section "C"
	//					"A_{B^C}"		: a section "A" with a lower index section "B" which has an upper index section "C"
	// invalid:   "A^B^C", "A_B_C", "A^B_C^D"

#define INC_POS		{\
						++pos; if((unsigned)pos > text.length()) goto ERR;\
							  ch = text[pos];\
					}
#define INC_POS2	{\
						pos += 2; if((unsigned)pos > text.length()) goto ERR;\
						ch = text[pos];\
					}
/*========================================================
 * TASK: to process an expression between '{' and '}' braces	
 *			into text section before graphing the text
 * EXPECTS: 'pos' - reference to a variable holding the first position of
 *           this section in text *after* the opening brace. 
 * RETURNS: true: more text to process, false: EOT encountered
 *          pos is set after the '}'
 * REMARKS: throws a const char* expression if an ending brace is not found
 *-------------------------------------------------------*/
bool TextSection::ProcessBrace(int &pos)
{
	bool Result = Process(pos, false);		// any number of letters
	if (text[pos] != '}')
		throw EEC_MISSING_BRACE; // "syntax error: missing '}'";
	return Result && ((unsigned)++pos < text.length());	// skip closing brace
}

/*========================================================
 * TASK: to process an expression into text section before graphing the text
 *          
 * EXPECTS: 'pos' - reference to a variable holding the first position of
 *				this section in text. If the preceeding (!) character exists and
 *		        it is '^' (upper index) or '_' (lower index) and the current 
 *				character is not an opening curly brace a single character is 
 *				processed (it may have its own upper or lower index).
 *          'isSingleLtter' - single index letter outside of braces. it may have its own
 *				indices though
 *			_no_processing : if true: all text is one section
 * RETURNS: false: EOT encountered
 * REMARKS: throws a const char* expression if a syntax error is found
 *-------------------------------------------------------*/
bool TextSection::Process(int &pos, bool isSingleLetter)
{
	if(text.empty() || (unsigned)pos >= text.length())
		return false;

	if(_no_processing)
	{
		myText = text;
		pos += text.length();
		return true;
	}

	bool Result = true;
	int spos = pos;		// start position for this text

	if(text[pos] == '}')
			goto ERR;		// must not begin with a closing brace
	while( (unsigned)pos < text.length())
	{
		wchar_t ch = text[pos];		// actual character

		if(ch == '{' )		// section started with a brace?
			Result = ProcessBrace(++pos);
		spos = pos;		// section starts here

		while( Result && (unsigned)pos < text.length() &&  ch != '}' && ch != '{')
		{
			if(ch == L'^' || ch == L'_')	// text is followed by an upper/lower index
			{
				if(!len)	// otherwise this is an uther index for the same text
				{
					len = pos - spos;		// end of text section is found: set length
					myText = text.substr(spos,len);
				}
				TextSection *&pts = (ch == L'^') ? upper : lower;	
				// if the text ends with this character or the index already exists
				if(pts || ++pos == text.length() )											
					goto ERR;
				ch = text[pos];
									// create upper/lower index
				pts = new TextSection(text, canvas); 
				ch = text[pos];
				if(ch == L'{')				// if this is a block of text section(s)
					Result = pts->ProcessBrace(++pos);		
				else								// else a single letter upper/lower index
					Result = pts->Process(pos, true);
				ch = text[pos];
				spos = pos;
			}
			else if(len)				// already set: following text belongs to an other block
			{
				next = new TextSection(text, canvas);
				Result = next->Process(pos, false);
				ch = text[pos];
			}
			else 
			{
				if(ch == L'\\')		// store both '\' and next letter
					++pos;
				ch = text[++pos];	// next letter or '^','_','{','}'
				// non special letter: add to section
				if(isSingleLetter)
				{
					len = pos-spos;
					myText = text.substr(spos,len);
					return ch ? true:false;
				}

			}
		}
		if(ch == '}')
		{
			if(!len)
			{
				len = pos - spos;
				myText = text.substr(spos,len);
			}
			return Result;
		}
	}
	if(!len)
	{
		len = pos - spos;
		myText = text.substr(spos,len);
	}

	return Result && ((unsigned)pos < text.length());
ERR:
	delete upper;
	delete lower;
	throw EEC_SYNTAX_ERROR;
}

static int _GetIndexFontSize(int size) // relative to size
{
	static int _nFontSizes[] = {8,9,10,11,12,14,16,18,20,22,24,26,28,36,48,72};
	int index;
	for(index = 0; index < sizeof(_nFontSizes)/sizeof(_nFontSizes[0]); ++ index)
		if(_nFontSizes[index] == size)
			break;
		else if(_nFontSizes[index] > size)
		{
			--index;
			break;
		}
	if(index < 0)
		index = 0;
	else if(index >= sizeof(_nFontSizes)/sizeof(_nFontSizes[0]) )
		index = -1;	// leave larger font intact

	return (index > 0 ? _nFontSizes[index-1] : (index == 0 ? size : size*2/3));
}

/*=========================================================
 * TASK: to calculate the 'x' and 'y' coordinates and
 *       sizes for a text section including and all of its indices and followers
 * EXPECTS: 'size': size of font for this level
 *			'left': x coordinate of current text, relative to left of first section in pixels
 *			'top' : y coordinate of current text, relative to top of first section in pixels
 *---------------------------------------------------------*/
void TextSection::SetPositionAndFontSizes(int size, int left, int top)
{
	x = left;
	y = top;
	nlib::Font &font = canvas->GetFont();
	float savedSize = font.Size();
	nFontSize = size ? size : savedSize;
	if(size)
		font.SetSize((float)size);

	SIZE cxcy = canvas->MeasureText(myText);

	width  = cxcy.cx;	// width w.o. upper and lower indices
	height = cxcy.cy;	// height w.o. upper and lower indices
					// default box contains 'myText'
	box.left = x;
	box.top = y;
	box.right = x + width;
	box.bottom = y + height;

	if(upper || lower)
	{
		// get font size of index 
		// fonts larger than 72 point have their indices 2/3rd as large

		int indexSize = (int)(_GetIndexFontSize(size ? size : (int)nFontSize));

	    cxcy = canvas->MeasureText(L"W");
		int offset = 2*cxcy.cy/3;
		int wu=0,hu=0,wl=0,hl=0;
		font.SetSize((float)indexSize);
		if(upper)
		{
		   upper->SetPositionAndFontSizes(indexSize, x + width, y - cxcy.cy + (height - offset));
		   box.top = upper->box.top;
		   wu = upper->box.right - upper->box.left+1;
		   hu = upper->box.bottom - upper->box.top + 1;
		}
		if(lower)
		{
		   cxcy = canvas->MeasureText(L"W");
		   lower->SetPositionAndFontSizes(indexSize, x + width, y + offset);
		   box.bottom = lower->box.bottom;
		   wl = lower->box.right - lower->box.left+1;
		   hl = lower->box.bottom - lower->box.top + 1;
		}
		// modify box if needed
		if(lower && lower->box.top < box.top)	// y coordinate from top to bottom
			box.top = lower->box.top;
		if(upper && upper->box.bottom > box.bottom)
			box.bottom = upper->box.bottom;
		box.right += (wu > wl ? wu:wl);		
	}
	if(next)
	{
		 next->SetPositionAndFontSizes(size, box.right, y);
		 box.right = next->box.right;
		 if(box.top > next->box.top)
			 box.top = next->box.top;
		 if(box.bottom < next->box.bottom)
			 box.bottom = next->box.bottom;
	}
	if(size)
		font.SetSize(savedSize);
}

void TextSection::Draw(int x0, int y0, GTransformation &gt) // expects: 'box' and coordinates are set in GraphicText
{								   
	nlib::Font &font = canvas->GetFont();
	int savedSize = (int)font.Size(),		 // save current data
		escapement = font.Escapement(),
		orientation = font.Orientation();
	font.SetSize(nFontSize);
	font.SetColor(nlib::clBlack);
	font.SetEscapement(gt.rotation);
	font.SetOrientation(gt.rotation);

	// draw relative to a very large clipping rectangle
	nlib::Rect clip = nlib::Rect(0, 0, 4000, 4000); // x0+box.left, y0+box.top, x0+box.right+1,y0+box.bottom);
	SIZE s = gt.Transform(x, y);
	canvas->TextDraw(clip, x0 + s.cx, y0 + s.cy, myText);

	font.SetSize((float)savedSize);		// restore font data
	font.SetEscapement(escapement);
	font.SetOrientation(orientation);

	if(upper)
		upper->Draw(x0, y0, gt);
	if(lower)
		lower->Draw(x0, y0, gt);
	if(next)
		next->Draw(x0, y0, gt);
}	

// ================= class GraphicText ===============

// DEBUG
#if 0
void LogSections(std::wofstream &ofs, TextSection *&ps, int level)
{
	std::wstring ws;
	ws.resize(level, L'\t');
	ofs << ws << "Text: " << ps->myText << "\n"
		<< ws <<    "\tlen:" << ps->len << "\n"
		<< ws <<    "\tx:" << ps->x << "\n"
		<< ws <<    "\ty:" << ps->y << "\n"
		<< ws <<    "\twidth:" << ps->width << "\n"
		<< ws <<    "\theight:" << ps->height << "\n"
		<< ws <<    "\tbox.left:" << ps->box.left << "\n"
		<< ws <<    "\tbox.top:" << ps->box.top << "\n"
		<< ws <<    "\tbox.right:" << ps->box.right << "\n"
		<< ws <<    "\tbox.bottom:" << ps->box.bottom << "\n";
	if(ps->upper)
	{
		ofs << ws << "Upper Index:\n";
		LogSections(ofs, ps->upper, level+1);
	}
	if(ps->lower)
	{
		ofs << ws << "Lower Index:\n";
		LogSections(ofs, ps->lower, level+1);
	}
	if(ps->next)
	{
		ofs << "Next section:\n";
		LogSections(ofs, ps->next, level);
	}
}
#endif

GraphicText::GraphicText() : _pSections(0), _canvas(0), _rotation(0), _dFontHeight(0), _no_processing(false)
{
}

GraphicText::~GraphicText()
{
	delete _pSections;
}

void GraphicText::SetCanvas(nlib::Canvas *newCanvas)
{
	if(_canvas == newCanvas)
		return;

	_canvas = newCanvas;			// needed to 'delete' the canvas
	SetupSections(newCanvas);		// this alse sets canvas, but only when not NULL
}

void GraphicText::SetupSections(nlib::Canvas* draw_canvas)
{
	delete _pSections;
	_pSections = 0;
	if(draw_canvas)
		_canvas = draw_canvas;

	if(_text.empty() || !_canvas)
		return;

	_dFontHeight = _canvas->GetFont().Height();

	_pSections = new TextSection(_text, _canvas);
	_pSections->_no_processing = _no_processing;
	int pos = 0;
	_pSections->Process(pos);	  // create lists of sections
	_pSections->SetPositionAndFontSizes();
}

void GraphicText::SetText(std:: wstring& text, nlib::Canvas* draw_canvas, bool single)
{
	_no_processing = single;

	_text = text; 

	SetupSections(draw_canvas);

// DEBUG
	//std::wofstream ofs("graphictext.log");
	//ofs << "Text: "<< text << "\n____" << std::endl;
	//LogSections(ofs, _pSections,0);
}

void GraphicText::Recalc()
{
	if(!_pSections)
		return;
	_dFontHeight = _canvas->GetFont().Height();
	_pSections->SetPositionAndFontSizes();
}

nlib::Rect GraphicText::Box()
{
	if(_pSections)
		return _pSections->box;
	else 
		return nlib::Rect(0,0,0,0);
}

void GraphicText::SetAlignment(nlib::CanvasTextAlignmentSet al)
{
	_alignment = al;
}

void GraphicText::SetRotation(int ddeg)			// ddeg: 0.1 degree
{
	_rotation = ddeg;
}
/*=================================================
 * TASK: draw text on _canvas
 * EXPECTS: x0 = left, y0 = top of text relative
 *			         to top left of 'box'
 *------------------------------------------------*/
void GraphicText::Draw(int x0, int y0)
{
	if(!_pSections)
		SetupSections(_canvas);

	int offs = 0;

	if(_alignment.contains(nlib::ctaCenter) ) 
		offs = Box().Width()/2;
	else if(_alignment.contains(nlib::ctaRight) )
		offs = Box().Width(); 

	GTransformation gt(_rotation, offs); 
	_pSections->Draw(x0,y0, gt);
}


// end of namespace sa
}