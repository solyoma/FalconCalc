#ifndef _clipboardH
	#define _clipboardH

class Clipboard
{
public:
	Clipboard(): hWnd(0), hwndNextViewer(0) 
	{
	}
	~Clipboard() 
	{ 
		if(hwndNextViewer) 
			ChangeClipboardChain(hWnd, hwndNextViewer); 
	}

	void Activate(HWND hwnd)
	{
		hWnd = hwnd;
		hwndNextViewer = SetClipboardViewer(hwnd);
	}

	bool SetText(wstring text)
	{
		if(!OpenClipboard(hWnd) )
			return false;

		EmptyClipboard();
		int len = text.length()+1;
		// first add unicode data
		HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, len*sizeof(wchar_t));
		wchar_t *pw = (wchar_t *)GlobalLock(handle);
		wcscpy(pw, text.c_str());
		GlobalUnlock(handle);
		SetClipboardData(CF_UNICODETEXT, handle);
		// then as UTF8 data
		char *buf = new char[len*2];
		wcstombs(buf, text.c_str(), 2*len);
		handle = GlobalAlloc(GMEM_MOVEABLE, len*2);
		char *pc = (char*)GlobalLock(handle);
		memcpy(pc, buf, len*2);
		GlobalUnlock(handle);
		SetClipboardData(CF_TEXT, handle);
		CloseClipboard();
		delete [] buf;
		return true;
	}

	wstring GetText()
	{
		wstring res;
		BOOL bUnicode = IsClipboardFormatAvailable(CF_UNICODETEXT) ,  
			 bText = IsClipboardFormatAvailable(CF_TEXT) ;
		if( (!bUnicode && !bText) || !OpenClipboard(hWnd) )
			return res;

		HGLOBAL handle;
		if(bUnicode) 
		{
			if( handle = GetClipboardData(CF_UNICODETEXT) )
			{
				wchar_t *pw = (wchar_t *)GlobalLock(handle);
				res = pw;
				GlobalUnlock(handle);
			}
		}
		else // bText - UTF8
		{
			if( handle = GetClipboardData(CF_TEXT) )
			{
				char *pw = (char *)GlobalLock(handle);
				size_t len = strlen(pw)+1;
				wchar_t *buf = new wchar_t[len];
				mbstowcs(buf, pw, len);
				GlobalUnlock(handle);
				res = buf;
				delete [] buf;
			}
		}
		CloseClipboard();
		return res;
	}
			// call this at event WM_CHANGECBCHAIN
	void ChangeNextViewer(HWND w, HWND l) // returns false if not our viewer has been changed
	{
		if ( w == hwndNextViewer) 
			hwndNextViewer = l; 
		else if (hwndNextViewer != NULL)
			SendMessage(hwndNextViewer, WM_CHANGECBCHAIN, (WPARAM) w, (LPARAM)l); 
	}

			// call this at event WM_DRAWCLIPBOARD
	void Forward(UINT msg, WPARAM w,LPARAM l)
	{
		if(hwndNextViewer)
			SendMessage(hwndNextViewer, msg, w, l);
	}

private:
	HWND hWnd,
		 hwndNextViewer;

};

#endif