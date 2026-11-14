#pragma once
// interface for font metrix for both the windows and the Qt program

#ifdef QTSA_PROJECT
	#include <QRect>
	#include <QFont>
	#include <QFontMetrics>

	#include "SmartStringQt.h"

	using Rect = QRect;
	using MyFont = QFont;
	using MyFontMetrics = QFontMetrics;
	#define BoundingRect(a)  boundingRect(a);
#else
	#include <windows.h>
	#include <wingdi.h>
	#include "SmartString.h"

class MyFont
{
public:
	MyFont() {};
	MyFont(const MyFont& font): _family(font._family), _weight(font._weight), _pointSize(font._pointSize), _italic(font._italic) {}
	MyFont(const String& family, int pointSize = -1, int weight = -1, bool italic = false) : 
		_family(family), _weight(weight), _pointSize(pointSize), _italic(italic) {}
	MyFont &operator=(MyFont&& other) 
	{ 
		if (this != &other) 
		{
			_family = std::move(other._family);
			_weight = other._weight;
			_pointSize = other._pointSize;
			_italic = other._italic;
		}
		return *this;
	}
	MyFont& operator=(const MyFont& font)
	{
		if (this != &font) 
		{
			_family = font._family;
			_weight = font._weight;
			_pointSize = font._pointSize;
			_italic = font._italic;
		}
				return *this;
	}
	~MyFont() {}

	String family()	const{ return _family;}
	int weight()	const{ return _weight;}
	int pointSize() const{ return _pointSize;}
	bool italic()	const{ return _italic;}
private:
	String _family;
	int _weight = 0;
	int _pointSize =0;
	bool _italic = false;
};

struct Point
{
	int x=0, y;
};
struct Size
{
	int width=0, height=0;
};

struct Rect
{
	Rect() : left(0), top(0), right(0), bottom(0) {}
	Rect(const Rect& r) : left(r.left), top(r.top), right(r.right), bottom(r.bottom) {}
	Rect(int x, int y, int width, int height) : left(x), top(y), right(x + width), bottom(y + height) {}
	Rect(const Point& topLeft, const Size& size) : left(topLeft.x), top(topLeft.y), right(topLeft.x + size.width), bottom(topLeft.y + size.height) {}
	Rect(const Point& topLeft, const Point& bottomRight) : left(topLeft.x), top(topLeft.y), right(bottomRight.x), bottom(bottomRight.y) {}

	int left, top, right, bottom;
};

class MyFontMetrics
{
public:
	MyFontMetrics() {}
	MyFontMetrics(const MyFontMetrics &) {}
	MyFontMetrics(const MyFont &font) {}
	MyFontMetrics& operator=(const MyFontMetrics &);
	MyFontMetrics& operator=(const MyFontMetrics &&);
	~MyFontMetrics() {}

	Rect& BoundingRect(const SCharT &);
	Rect& BoundingRect(const String &);
private:
	TEXTMETRIC _tm;
	HDC _hdc = nullptr;
	MyFont _font;

	void _setup(HDC hdc)
	{
		_hdc = hdc == nullptr ? GetDC(nullptr) : hdc;
		const String arial((const char*)"Arial");
		_font = MyFont(arial, 12, FW_NORMAL, false);
	}
};
#endif