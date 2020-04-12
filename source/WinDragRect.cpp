#include "PlatformPrecomp.h"
#include "WinDragRect.h"
#include "App.h"

int DivisibleByFour(int num, int max);

//extern HWND	g_hWnd;
//extern HDC g_hDC;
extern HINSTANCE g_hInstance;


#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif


WinDragRect::WinDragRect()
{
	m_dragMode = DRAGMODE_START;
	m_hWnd = NULL;
	m_font = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, "times");

}

WinDragRect::~WinDragRect()
{
	DeleteObject(m_font);

} 
LRESULT CALLBACK MyNewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	RECT rect;
	//LogMsg("Got win message");
	switch (message)
	{

/*
	case WM_CHAR:
		if (wParam == VK_ESCAPE)
		{
			// handle escape key pressed here
			GetApp()->m_pWinDragRect->End();
			return 0;
		}
	*/

	case WM_PAINT:

	{
		GetApp()->m_pWinDragRect->OnPaint();

		
		return 1;
	}
		break;

	case WM_LBUTTONDOWN:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		GetApp()->m_pWinDragRect->m_cursorStartPT.x = xPos;
		GetApp()->m_pWinDragRect->m_cursorStartPT.y = yPos;
		GetApp()->m_pWinDragRect->m_dragMode = DRAGMODE_STARTED;
		GetApp()->m_pWinDragRect->m_selRect = CL_Rect(xPos, yPos, CL_Size(0, 0));

		LogMsg("Mouse down");
		InvalidateRect(hWnd, 0, TRUE);

	}
		break;
	case WM_LBUTTONUP:
	{
		InvalidateRect(hWnd, 0, TRUE);
		LogMsg("Mouse up");
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (GetApp()->m_pWinDragRect->m_cursorStartPT.x == xPos
			&& GetApp()->m_pWinDragRect->m_cursorStartPT.y == yPos)
		{
			//No change, don't count this
			GetApp()->m_pWinDragRect->m_dragMode = DRAGMODE_START;
		}

		//if we got here...
		GetApp()->m_pWinDragRect->m_dragMode = DRAGMODE_FINISHED;
		//set final rect


	}
		break;


	case WM_MOUSEMOVE:

		InvalidateRect(hWnd, 0, TRUE);

		break;

	case WM_ERASEBKGND:
		
		//LogMsg("Erasing..");
		GetClientRect(hWnd, &rect); 
		HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
		SelectObject((HDC)wParam, brush);
		FillRect((HDC)wParam, &rect, brush);
		
		DeleteObject(brush);
		//FillRect((HDC)wParam, &m_screenRect, CreateSolidBrush(RGB(0, 0, 255)));
		return 1;
		break;
	}

	// Calls the default window procedure for messages we did not handle
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void WinDragRect::OnPaint()
{

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);


	
	POINT pt;
	if (GetCursorPos(&pt))
	{
	}
	else
	{
		LogMsg("Unable to get cursor position");
	}
	

	if (m_dragMode == DRAGMODE_START)
	{


		SelectObject(hdc, GetApp()->m_pWinDragRect->m_font);
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkColor(hdc, RGB(0, 0, 255));
		SetBkMode(hdc, TRANSPARENT);
		string Text = "Select area";
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		rc.left = pt.x + (GetApp()->m_pWinDragRect->m_screenRect.left * -1);
		rc.top = (pt.y - 20)+ GetApp()->m_pWinDragRect->m_screenRect.top * -1;
		rc.right = rc.left;
		rc.bottom = rc.top;

		//LogMsg("PS: %d, %d.  Cursor: %d, %d", ps.rcPaint.left, ps.rcPaint.right, pt.x, pt.y);
		//FillRect(hdc, &rc, CreateSolidBrush(RGB(100, 40, 255)));

		DrawText(hdc, Text.c_str(), Text.size(), &rc, DT_CENTER | DT_NOCLIP | DT_SINGLELINE);

	}

	if (m_dragMode == DRAGMODE_STARTED)
	{
		//draw a box 
		int sizeX = m_cursorStartPT.x - pt.x;
		int sizeY = m_cursorStartPT.y - pt.y;

		RECT rc;
		SetRect(&rc,m_cursorStartPT.x, m_cursorStartPT.y, pt.x+ GetApp()->m_pWinDragRect->m_screenRect.left * -1, pt.y+ GetApp()->m_pWinDragRect->m_screenRect.top * -1);

		//LogMsg("Dragging %d, %d, %d, %d", rc.left,  rc.top, rc.right, rc.bottom);
		HBRUSH brush = CreateSolidBrush(RGB(200, 200, 200));
		SelectObject(hdc, brush);
		FillRect(hdc, &rc, brush);


		//remember this rect size...

		m_selRect = CL_Rect(rc.left, rc.top, rc.right, rc.bottom);
		m_selRect.normalize(); 

		m_selRect.translate(CL_Vec2i(GetApp()->m_pWinDragRect->m_screenRect.left, (GetApp()->m_pWinDragRect->m_screenRect.top)));
	}



	EndPaint(m_hWnd, &ps);
}
void WinDragRect::Start()
{

	m_dragMode = DRAGMODE_START;
	wc = { };
	LogMsg("Starting drag");

	const char CLASS_NAME[] = "RTWindow2";

	wc.lpfnWndProc = MyNewWndProc;
	wc.hInstance = g_hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	DWORD Flags1 = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOPMOST;
	DWORD Flags2 = WS_POPUP| WS_SYSMENU;
	
	m_screenRect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_screenRect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_screenRect.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	m_screenRect.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	m_hWnd = CreateWindowEx(Flags1, CLASS_NAME, "RTTitle", Flags2, m_screenRect.left, m_screenRect.top, m_screenRect.right, m_screenRect.bottom, 0, 0, g_hInstance, 0);

	if (!m_hWnd)
	{
		LogMsg("Error creating transparent window");
	}

	if (m_hWnd == NULL) 
	{
		assert(!"The hell?!");
	}

	COLORREF RRR = RGB(255, 0, 255); 
	SetLayeredWindowAttributes(m_hWnd, RGB(255, 0, 255), (BYTE)130, LWA_ALPHA | LWA_COLORKEY); //LWA_COLORKEY

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	if (GetCursorPos(&m_lastCursorPT))
	{
	}
	else
	{
		LogMsg("Unable to get cursor position");
	}
}

void WinDragRect::End()
{
	LogMsg("Finishing drag");
	DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	UnregisterClass(wc.lpszClassName, g_hInstance);
}

void WinDragRect::Render()
{

}

void WinDragRect::Update()
{


	if (m_dragMode == DRAGMODE_FINISHED)
	{

		//LogMsg("Selecting: %s", PrintRect(m_selRect).c_str());
		End();

		GetApp()->SetCaptureMode(CAPTURE_MODE_WAITING);
		
		GetApp()->m_window_pos_x = m_selRect.left;
		GetApp()->m_window_pos_y = m_selRect.top;
		
		GetApp()->m_capture_width = m_selRect.get_width();
		GetApp()->m_capture_height = m_selRect.get_height();


		if (GetApp()->m_capture_width < 4 || GetApp()->m_capture_height < 4)
		{
			LogMsg("Too small!");
			End();
			GetApp()->SetCaptureMode(CAPTURE_MODE_WAITING);
			return;
		}

		GetApp()->m_capture_width = DivisibleByFour(GetApp()->m_capture_width, 0);

		GetApp()->ScanSubArea();

		return;
	}
	bool bRequestRepaint = false;

	POINT pt;
	if (GetCursorPos(&pt))
	{
	}
	else
	{
		LogMsg("Unable to get cursor position");
	}

	if (pt.x != m_lastCursorPT.x || pt.y != m_lastCursorPT.y)
	{
		m_lastCursorPT = pt;
		bRequestRepaint = true;
	}

	HDC hDC_Desktop = GetDC(m_hWnd);
	RECT rect = { pt.x, pt.y, pt.x+6, pt.y+6 };
	
	
	//HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
	//FillRect(hDC_Desktop, &rect, blueBrush);
	

	ReleaseDC(m_hWnd, hDC_Desktop);
	if (bRequestRepaint)
	{ 
	//	InvalidateRect(m_hWnd, 0, TRUE);
	}

	if (GetAsyncKeyState(VK_ESCAPE))
	{
		this->End();
		GetApp()->SetCaptureMode(CAPTURE_MODE_WAITING);
		return;
	}
}