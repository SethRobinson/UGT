//  ***************************************************************
//  WinDragRect - Creation date: 04/08/2020
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2020 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef WinDragRect_h__
#define WinDragRect_h__

enum eDragMode
{
	DRAGMODE_START,
	DRAGMODE_STARTED,
	DRAGMODE_FINISHED

};

class WinDragRect
{
public:
	WinDragRect();
	virtual ~WinDragRect();

	void OnPaint();
	void Start();
	void End();
	void Render();
	void Update();

	HFONT m_font;
	RECT m_screenRect;
	POINT m_lastCursorPT;
	POINT m_cursorStartPT;
	CL_Rect m_selRect;
	eDragMode m_dragMode;

protected:
	HWND m_hWnd;
	WNDCLASS wc;
	
private:
};

#endif // WinDragRect_h__