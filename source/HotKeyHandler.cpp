#include "PlatformPrecomp.h"
#include "HotKeyHandler.h"
#include <iostream>
#include "windows.h"
#include "App.h"

void OnTranslateButton();
extern HWND	g_hWnd;
extern AudioHandle g_lastAudioHandle;

void HandleWMHotkey(UINT message, WPARAM wParam, LPARAM lParam)
{
	int ID = (int)wParam;
	//LogMsg("Got hotkey %d, %d, ID: %d", message, lParam, ID);
	
	GetApp()->HandleHotKeyPushed(GetApp()->m_hotKeyHandler.GetHotKeyByID(ID));
}

HotKeySetting HotKeyHandler::GetHotKeyByID(unsigned int ID)
{
	for (int i = 0; i < m_keysActive.size(); i++)
	{
		if (m_keysActive[i].m_registrationID == ID)
		{
			return m_keysActive[i];
		}
	}
	
	HotKeySetting crap;
	return crap;
}

HotKeySetting HotKeyHandler::GetHotKeyByAction(string action)
{
	for (int i = 0; i < m_keysActive.size(); i++)
	{
		if (m_keysActive[i].hotKeyAction== action)
		{
			return m_keysActive[i];
		}
	}

	HotKeySetting crap;
	crap.hotKeyAction = "Error";
	return crap;
}

HotKeyHandler::HotKeyHandler()
{
}

HotKeyHandler::~HotKeyHandler()
{
	UnregisterAllHotkeys();
}

void HotKeyHandler::UnregisterAllHotkeys()
{
	for (int i = 0; i < m_keysActive.size(); i++)
	{
		if (!UnregisterHotKey(g_hWnd, m_keysActive[i].m_registrationID))
		{
			LogMsg("Error unregistering hotkey %s", m_keysActive[i].originalString.c_str());
		}
		else
		{
			m_keysActive[i].m_registrationID = 0; //invalid now
		}
	}

}


void HotKeyHandler::ReregisterAllHotkeys()
{
	LogMsg("Reregistering hotkeys");
	UnregisterAllHotkeys();

	for (int i = 0; i < m_keysActive.size(); i++)
	{
		RegisterHotkey(m_keysActive[i]);
	}

}

extern string g_fileName;

void HotKeyHandler::OnHideWindow()
{

	if (!g_fileName.empty()) return;

	WINDOWPLACEMENT win;
	memset(&win, 0, sizeof(WINDOWPLACEMENT));
	win.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(g_hWnd, &win);
	
	//I don't think we need to screw with window position like below was doing
	/*
	win.rcNormalPosition.left = GetApp()->m_window_pos_x;
	win.rcNormalPosition.top = GetApp()->m_window_pos_y;
	win.rcNormalPosition.right = GetApp()->m_capture_width + GetApp()->m_window_pos_x;
	win.rcNormalPosition.bottom = GetApp()->m_capture_height + GetApp()->m_window_pos_y;
	*/


	win.showCmd = SW_MINIMIZE;

	SetWindowPlacement(g_hWnd, &win);
	
	//LogMsg("ForceHWND is %d", GetApp()->m_forceHWND);

	if (GetApp()->m_forceHWND != 0)
	{
		//Let's look at this one
		
		//ShowWindow(GetApp()->m_forceHWND, SW_SHOW);
		GetApp()->m_forceHWND = 0;
	}
	else
	{
		//LogMsg("It's zero");
	}

	LogMsg("Hiding window...%d, %d", GetApp()->m_capture_width, GetApp()->m_capture_height);
	//ShowWindow(g_hWnd, SW_MINIMIZE); 
	
	if (GetApp()->m_audio_stop_when_window_is_closed)
	{

		GetAudioManager()->Stop(g_lastAudioHandle);
	}
} 

extern bool g_bHasFocus;
void HotKeyHandler::OnShowWindow()
{
	if (!g_fileName.empty()) return;

#ifdef _DEBUG
	LogMsg("Showing window...%d, %d.  Focus is %d", GetApp()->m_capture_width, GetApp()->m_capture_height, (int)g_bHasFocus);
#endif
	//ShowWindow(g_hWnd, SW_SHOW);
	
	WINDOWPLACEMENT win;
	memset(&win, 0, sizeof(WINDOWPLACEMENT));
	win.length = sizeof(WINDOWPLACEMENT);
	win.rcNormalPosition.left = GetApp()->m_window_pos_x;
	win.rcNormalPosition.top = GetApp()->m_window_pos_y;
	win.rcNormalPosition.right = GetApp()->m_capture_width + GetApp()->m_window_pos_x;
	win.rcNormalPosition.bottom = GetApp()->m_capture_height + GetApp()->m_window_pos_y;

	win.showCmd = SW_RESTORE;
	SetWindowPlacement(g_hWnd, &win);
	
	win.showCmd = SW_SHOW;
	SetWindowPlacement(g_hWnd, &win);
	SetForegroundWindow(g_hWnd);
}

void HotKeyHandler::RegisterHotkey(HotKeySetting &setting)
{

	if (setting.virtualKey == 0)
	{
		return;
	}

	m_registrationCounter++;

	if (!RegisterHotKey(g_hWnd, m_registrationCounter, setting.modifierBits, setting.virtualKey))
	{
		if (setting.m_bFirstTime)
		{
			string msg = string("Error registering hotkey ") + setting.originalString + " for "+setting.hotKeyAction+", it's already being used by something else!\n\nYou'll need to change the keys to use in our config.txt, or change it in the program already using it. (Check NVidia/UPlay/Origin/Steam/etc overlay hotkeys for example)";
			LogMsg(msg.c_str());
			
			MessageBox(0, _T(msg.c_str()), _T("Error registering hotkey"), MB_OK | MB_ICONEXCLAMATION);

		}
	} 
	else
	{
		setting.m_registrationID = m_registrationCounter;
		
		if (setting.m_bFirstTime)
		{
			setting.m_bFirstTime = false;
			m_keysActive.push_back(setting);
		}
	}

}

