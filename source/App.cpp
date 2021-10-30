/*
 *  App.cpp
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */ 
#include "PlatformPrecomp.h"
#include "App.h"
 
#include "Entity/CustomInputComponent.h" //used for the back button (android)
#include "Entity/FocusInputComponent.h" //needed to let the input component see input messages
#include "Entity/ArcadeInputComponent.h" 
#include "CursorComponent.h"
#include "Entity/EntityUtils.h"
#include "GameLogicComponent.h"
#include "util/TextScanner.h"
#include <io.h>
#include <fcntl.h>
#include "GUIHelp.h"
#include "AutoPlayManager.h"
#include "WinDragRect.h"
#include "ExportToHTML.h"

#ifdef WINAPI
extern HWND g_hWnd;
#endif

#include "Gamepad/GamepadManager.h"

MessageManager g_messageManager;
MessageManager * GetMessageManager() {return &g_messageManager;}

FileManager g_fileManager;
FileManager * GetFileManager() {return &g_fileManager;}

//#include "Audio/AudioManagerSDL.h"
#include "Audio/AudioManagerFMODStudio.h"
#include "Audio/AudioManagerAudiere.h"

AudioManager *g_pAudioManager = NULL; //sound in windows/WebOS/Linux/html5

AudioManager * GetAudioManager(){return g_pAudioManager;}

GamepadManager g_gamepadManager;
GamepadManager * GetGamepadManager() { return &g_gamepadManager; }

#ifdef WINAPI
#include "Gamepad/GamepadProviderDirectX.h"
#include "Gamepad/GamepadProviderXInput.h"
extern bool g_bHasFocus;

#endif
App *g_pApp = NULL;

void OnTranslateButton();
void OnGamepadButton(VariantList *m_pVList);
void OnTakeScreenshot();
void TurnOffRenderDisplay(VariantList* pVList);

string RunLinuxShell(string command)
{

	string temp;
	temp = "\r\nRunning " + command + " ...\r\n";

#ifndef WINAPI
	system(command.c_str());
	return temp;
#else
	return "Doesn't work in windows, can't run " + command;
#endif
}

void DoResync(VariantList *pVList)
{
	LogMsg("Doing actual resync");
	GetApp()->m_pGameLogicComp->m_escapiManager.RequestReInit();
}


void ResyncWithCapture()
{
	ShowQuickMessage("Resyncing with capture...");
	GetMessageManager()->CallStaticFunction(DoResync, 100);
}

void EnableTV(bool bOn)
{

	return;
	if (!bOn)
	{
		//RunLinuxShell("tvservice -o");

		RunLinuxShell("./uhubctl -a off -p 2");
	}
	else
	{
		//RunLinuxShell("tvservice -p; fbset -depth 8; fbset -depth 16");
		RunLinuxShell("./uhubctl -a on -p 2");
	}
}

void SetStdOutToNewConsole();

BaseApp * GetBaseApp() 
{
	if (!g_pApp)
	{
		g_pApp = new App;
	}
	return g_pApp;
}

App * GetApp() 
{
	assert(g_pApp && "GetBaseApp must be called used first");
	return g_pApp;
}

App::App()
{
		m_pExportToHTML = NULL;
		m_pAutoPlayManager = NULL;
		m_usedSubAreaScan = false;
		m_version = "0.72 Beta";
		m_versionNum = 72;
		m_bDidPostInit = false;
		m_gamepad_button_to_scan_active_window = VIRTUAL_KEY_NONE;
		m_cursorShouldBeRestoredToStartPos = false;
		m_cursorPosAtStart.x = m_cursorPosAtStart.y = 0;
		//m_bTestMode = true;
}

App::~App()
{
	//EnableTV(true);
	for (int i = 0; i < m_vecFontInfo.size(); i++)
	{
		SAFE_DELETE(m_vecFontInfo[i]);
	}

	m_vecFontInfo.clear();

	SAFE_DELETE(m_pAutoPlayManager);
	SAFE_DELETE(m_pExportToHTML);
	SAFE_DELETE(m_pWinDragRect);
}

void SetStdOutToNewConsole()
{
	// allocate a console for this app
	AllocConsole();

	// redirect unbuffered STDOUT to the console
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int fileDescriptor = _open_osfhandle((intptr_t)consoleHandle, _O_TEXT);
	FILE *fp = _fdopen(fileDescriptor, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);

	// give the console window a nicer title
	SetConsoleTitleW(L"Debug Output");

// 	
// 	CONSOLE_SCREEN_BUFFER_INFO csbi;
// 	if (GetConsoleScreenBufferInfo(consoleHandle, &csbi))
// 	{
// 		COORD bufferSize;
// 		bufferSize.X = csbi.dwSize.X;
// 		bufferSize.Y = 9999;
// 		SetConsoleScreenBufferSize(consoleHandle, bufferSize);
// 	}
}

bool App::IsInputDesktop()
{
	return !(m_inputMode == "camera");
}

/*
void App::OnGamepadStickUpdate(VariantList* pVList)
{
	CL_Vec2f vStick = pVList->Get(0).GetVector2();
	int32 padID = pVList->Get(1).GetINT32();
	int32 stickIndex = pVList->Get(2).GetINT32(); //0 for left, 1 for right

	LogMsg("Gamepad %d got a reading of %.2f, %.2f on stick %d", padID, vStick.x, vStick.y, stickIndex);

	POINT pt;
	if (GetCursorPos(&pt))
	{
		const float speed = 10.0f;
		pt.x += (speed * vStick.x);
		pt.y += (speed * vStick.y);
		SetCursorPos(pt.x, pt.y);

	}
	else
	{
		LogMsg("Unable to get cursor access. Run as admin?");
	}
}

*/

void App::UpdateCursor()
{

	Gamepad* pPad = GetGamepadManager()->GetDefaultGamepad();
	if (!pPad) return;

	float deadZone = 0.2f;

	POINT pt;
	if (GetCursorPos(&pt))
	{
		float speed = 8.0f;
		GamepadButton* leftStickButton = pPad->GetVirtualButton(VIRTUAL_JOYSTICK_BUTTON_LEFT);

		if (leftStickButton && leftStickButton->m_bDown)
		{
			speed *= 2.0f;
		}

		if (fabs(pPad->GetLeftStick().x) > deadZone)
		{
			pt.x += (long)((speed * pPad->GetLeftStick().x));
		}

		if (fabs(pPad->GetLeftStick().y) > deadZone)
		{
			pt.y += (long)((speed * pPad->GetLeftStick().y));
		}
		SetCursorPos(pt.x, pt.y);
	}
	else
	{
		LogMsg("Unable to get cursor access. Run as admin?");
	}
}

void App::AddFontOverride(string fontName, string language, float widthOverride, float preTranslatedHeightMod = 1.0f)
{
	m_vecFontInfo.resize(m_vecFontInfo.size() + 1);
	m_vecFontInfo[m_vecFontInfo.size() - 1] = new FontLanguageInfo();
	if (fontName.length() > 1)
		m_vecFontInfo[m_vecFontInfo.size() - 1]->SetupFont(fontName);
	m_vecFontInfo[m_vecFontInfo.size() - 1]->m_widthOverride = widthOverride;
	m_vecFontInfo[m_vecFontInfo.size() - 1]->m_preTranslatedHeightMod = preTranslatedHeightMod;
	m_vecFontInfo[m_vecFontInfo.size() - 1]->m_vecFontOverrideName = language;
}

bool App::InitFonts()
{

	TextScanner ts;
	if (ts.LoadFile("fonts.txt"))
	{

		//another scan
		for (int i = 0; i < ts.GetLineCount(); i++)
		{
			vector<string> words = ts.TokenizeLine(i);

			if (words.size() > 1)
			{
				if (words[0] == "add_font")
				{
					//should probably do some basic validation to make sure we're getting the right stuff but nah
					LogMsg( (string("Adding ") + words[1] + " font file").c_str());
					AddFontOverride(words[1], words[2], StringToFloat(words[3]), StringToFloat(words[4]));
				}
			}
		}
	}
	else
	{
		return false;
	}

	return true;

}

bool App::Init()
{
	
	if (m_bInitted)
	{
		return true;
	}
	LogMsg("Initting PlayTrans: Universal Game Translator %s by Seth A. Robinson (www.rtsoft.com)", m_version.c_str());

	if (!BaseApp::Init()) return false;

	m_pWinDragRect = new WinDragRect();
	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS || GetEmulatedPlatformID() == PLATFORM_ID_WEBOS)
	{
		//SetLockedLandscape( true); //if we don't allow portrait mode for this game
		//SetManualRotationMode(true); //don't use manual, it may be faster (33% on a 3GS) but we want iOS's smooth rotations
	}


	if (!InitFonts())
	{
		LogMsg("Error loading fonts.txt file");
		return false;
	}

	
	//if we wanted Japanese to be smaller we could do this
	//AddFontOverride("", "ja", 0.0f, 0.88f); //use default font, but apply this size override

	bool bExisted = false;

	m_varDB.Load("config.dat", &bExisted);
	
	if (!bExisted)
	{
		//Set Defaults
		m_varDB.GetVar("check_autoplay_audio")->Set(uint32(0));
		m_varDB.GetVar("check_src_audio")->Set(uint32(1));

	}
	LogMsg("The Save path is %s", GetSavePath().c_str());
	LogMsg("Region string is %s", GetRegionString().c_str());

#ifdef _DEBUG
	LogMsg("Built in debug mode");
#endif
#ifndef C_NO_ZLIB
	//fonts need zlib to decompress.  When porting a new platform I define C_NO_ZLIB and add zlib support later sometimes
	if (!GetFont(FONT_SMALL)->Load("interface/font_trajan.rtfont")) return false;

	//GetFont(FONT_SMALL)->SetSmoothing(false);

	if (!GetFont(FONT_LARGE)->Load("interface/font_trajan_big.rtfont"))
	{
		LogMsg("Can't load font 2");
		return false;
	}
#endif

	if (m_bTestMode)
	{
		GetBaseApp()->SetFPSVisible(true);
	}

#ifdef PLATFORM_WINDOWS
	//If you don't have directx, just comment out this and remove the dx lib dependency, directx is only used for the
	//gamepad input on windows
	GamepadProviderXInput *pTemp = new GamepadProviderXInput();
	pTemp->PreallocateControllersEvenIfMissing(true);
	GetGamepadManager()->AddProvider(pTemp); //use XInput joysticks

	//do another scan for directx devices
	GamepadProviderDirectX *pTempDirectX = new GamepadProviderDirectX;
	pTempDirectX->SetIgnoreXInputCapableDevices(true);
	GetGamepadManager()->AddProvider(pTempDirectX); //use directx joysticks
	
	SetFPSLimit(100);
#endif

	//arcade input component is a way to tie keys/etc to send signals through GetBaseApp()->m_sig_arcade_input

	ArcadeInputComponent *pComp = (ArcadeInputComponent*)GetBaseApp()->GetEntityRoot()->AddComponent(new ArcadeInputComponent);

	for (int i=0; i < GetGamepadManager()->GetGamepadCount(); i++)
	{
		Gamepad *pPad = GetGamepadManager()->GetGamepad( (eGamepadID) i);
		pPad->m_sig_gamepad_buttons.connect(OnGamepadButton);
		pPad->ConnectToArcadeComponent(pComp, true, false);

		//if we cared about the analog stick exact input, we'd do this.  For fighting games, this would be good, so you don't fail
		//your dragon punch because FPS was slow and polling missed something.

		//pPad->m_sig_left_stick.connect(1, boost::bind(&OnGamepadStickUpdate, this, _1));	
//		pPad->m_sig_right_stick.connect(1, boost::bind(&App::OnGamepadStickUpdate, this, _1));	
	}

	m_pAutoPlayManager = new AutoPlayManager();
	m_pExportToHTML = new ExportToHTML();

	//check for updates?
	m_updateChecker.CheckForUpdate();
	return true;
}

void App::Kill()
{
	SAFE_DELETE(m_pAutoPlayManager);
	BaseApp::Kill();
	SAFE_DELETE(g_pAudioManager);
}

void App::OnExitApp(VariantList *pVarList)
{
	m_varDB.Save("config.dat");

	LogMsg("Exiting the app");
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	GetBaseApp()->AddOSMessage(o);
}

void App::StartHidingOverlays()
{
	m_bHidingOverlays = true;
	if (!GetCursorPos(&m_hidingOverlayMousePosStart))
	{
		m_bHidingOverlays = false;
		LogMsg("Unable to get control of mouse to track it");
	}
}

void App::HidingOverlayUpdate()
{
	if (!m_bHidingOverlays) return; //don't need to worry about it now
	POINT pt;
	int sensitivity = 5;

	if (!GetCursorPos(&pt))
	{
		m_bHidingOverlays = false;
		LogMsg("Unable to get control of mouse to track it");
	}
	else
	{
		if (abs(m_hidingOverlayMousePosStart.x - pt.x) > sensitivity
			||
			abs(m_hidingOverlayMousePosStart.y - pt.y) > sensitivity
			)
		{
			m_bHidingOverlays = false;
		}

	}
}

#define kFilteringFactor 0.1f
#define C_DELAY_BETWEEN_SHAKES_MS 500

//testing accelerometer readings. To enable the test, search below for "ACCELTEST"
//Note: You'll need to look at the  debug log to see the output. (For android, run PhoneLog.bat from UGT/android)
void App::OnAccel(VariantList *pVList)
{
	
	if ( int(pVList->m_variant[0].GetFloat()) != MESSAGE_TYPE_GUI_ACCELEROMETER) return;

	CL_Vec3f v = pVList->m_variant[1].GetVector3();

	LogMsg("Accel: %s", PrintVector3(v).c_str());

	v.x = v.x * kFilteringFactor + v.x * (1.0f - kFilteringFactor);
	v.y = v.y * kFilteringFactor + v.y * (1.0f - kFilteringFactor);
	v.z = v.z * kFilteringFactor + v.z * (1.0f - kFilteringFactor);

	// Compute values for the three axes of the accelerometer
	float x = v.x - v.x;
	float y = v.y - v.x;
	float z = v.z - v.x;

	//Compute the intensity of the current acceleration 
	if (sqrt(x * x + y * y + z * z) > 2.0f)
	{
		Entity *pEnt = GetEntityRoot()->GetEntityByName("jumble");
		if (pEnt)
		{
			//GetAudioManager()->Play("audio/click.wav");
            VariantList vList(CL_Vec2f(), pEnt);
			pEnt->GetFunction("OnButtonSelected")->sig_function(&vList);
		}
		LogMsg("Shake!");
	}
}

void SendFakeMouseClick(bool bOnDown)
{
	//Fake a mouse click
	POINT pt;
	if (GetCursorPos(&pt))
	{
		ScreenToClient(g_hWnd, &pt);

		float xPos = (float)pt.x;
		float yPos = (float)pt.y;

		ConvertCoordinatesIfRequired(xPos, yPos);

		if (bOnDown)
		{
			GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_CLICK_START, (float)xPos, (float)yPos, 0, 0);
		}
		else
		{
			GetMessageManager()->SendGUIEx2(MESSAGE_TYPE_GUI_CLICK_END, (float)xPos, (float)yPos, 0, 0);
		}
	}

}

void OnGamepadButton(VariantList *m_pVList)
{
	eVirtualKeys vKey = (eVirtualKeys)m_pVList->Get(0).GetUINT32();
	eVirtualKeyInfo vKeyInfo = (eVirtualKeyInfo)m_pVList->Get(1).GetUINT32();
	int gamepadID = m_pVList->Get(2).GetUINT32();

	string buttonName;
	string action;

	if (vKeyInfo == VIRTUAL_KEY_PRESS)
	{

		if (!g_bHasFocus && GetApp()->m_captureMode == CAPTURE_MODE_WAITING)
		{
			//give us focus?
			if (vKey == GetApp()->m_gamepad_button_to_scan_active_window)
			{
				LogMsg("SCANNING FROM gamepad button");
				SaveCursorPos();
				GetApp()->m_cursorShouldBeRestoredToStartPos = true;

				//relocate cursor to middle of window
				SetCursorPos(GetApp()->m_window_pos_x + GetApp()->m_capture_width/2,
					GetApp()->m_window_pos_y + GetApp()->m_capture_height/2);
			

				if (GetApp()->m_usedSubAreaScan)
				{
					//they've previously scanned a sub area by hand, so let's use that
					GetApp()->ScanSubArea();
				}
				else
				{
					GetApp()->ScanActiveWindow();

				}

			}
			return;
		}
		else
		{
			if ( /*g_bHasFocus &&*/ GetApp()->m_captureMode == CAPTURE_MODE_SHOWING)
			{
				if (vKey == GetApp()->m_gamepad_button_to_scan_active_window)
				{
					LogMsg("Closing capture window");
					GetMessageManager()->CallStaticFunction(TurnOffRenderDisplay, 200, NULL);

				}
			}

		}

		action = "Pressed";
		if (vKey == VIRTUAL_DPAD_BUTTON_RIGHT)
		{
			LogMsg("Toggling from gamepad button");

			if (GetApp()->IsInputDesktop() && GetHelpMenu() != NULL)
			{
				return;//don't allow us to translate the help menu, it screws stuff up
			}
			OnTranslateButton();
			return;
		}

		if (vKey == VIRTUAL_DPAD_BUTTON_DOWN)
		{
			SendFakeMouseClick(true);
		}

		if (vKey == VIRTUAL_DPAD_BUTTON_UP)
		{
			ResyncWithCapture();
		}

		if (vKey == VIRTUAL_DPAD_BUTTON_LEFT)
		{
			OnTakeScreenshot();
		}
		
		if (vKey == VIRTUAL_DPAD_LBUTTON)
		{
			GetApp()->ModLanguageByIndex(-1);
		}

		if (vKey == VIRTUAL_DPAD_RBUTTON)
		{
			GetApp()->ModLanguageByIndex(1);
		}

	}
	else
	{
		action = "Released";

		if (vKey == VIRTUAL_DPAD_BUTTON_DOWN)
		{
			SendFakeMouseClick(false);
		}
	}

#ifdef _DEBUG
	LogMsg("`6Gamepad `w%d``: `#%s`` is `$%s````", gamepadID, ProtonVirtualKeyToString(vKey).c_str(), action.c_str());
#endif

}

Variant* App::GetVar(const string& keyName)
{
	return GetShared()->GetVar(keyName);
}


//test for arcade keys.  To enable this test, search for TRACKBALL/ARCADETEST: below and uncomment the stuff under it.
//Note: You'll need to look at the debug log to see the output.  (For android, run PhoneLog.bat from UGT/android)

void App::OnArcadeInput(VariantList *pVList)
{

	int vKey = pVList->Get(0).GetUINT32();
	eVirtualKeyInfo keyInfo = (eVirtualKeyInfo) pVList->Get(1).GetUINT32();
	
	string pressed;

	switch (keyInfo)
	{
		case VIRTUAL_KEY_PRESS:
			pressed = "pressed";
			break;

		case VIRTUAL_KEY_RELEASE:
			pressed = "released";
			break;

		default:
			LogMsg("OnArcadeInput> Bad value of %d", keyInfo);
	}
	
	string keyName = "unknown";

	eViewMode mode = VIEW_MODE_DEFAULT;

	switch (vKey)
	{

		case VIRTUAL_KEY_DIR_LEFT:
			keyName = "Left";
			if (keyInfo == VIRTUAL_KEY_PRESS)
			{
				SetLineByLineMode();
			}

			break;

		case VIRTUAL_KEY_DIR_RIGHT:
			keyName = "Right";

			if (keyInfo == VIRTUAL_KEY_PRESS)
			{
				SetDialogMode();
			}
			break;

		case VIRTUAL_KEY_DIR_UP:
		{

			keyName = "Up";
			if (keyInfo == VIRTUAL_KEY_PRESS)
				mode = VIEW_MODE_HIDE_ALL;
		}
			break;

		case VIRTUAL_KEY_DIR_DOWN:
		{
			keyName = "Down";
			if (keyInfo == VIRTUAL_KEY_PRESS)
				mode = VIEW_MODE_SHOW_SOURCE;
		}
			break;
	}
	
	SetViewMode(mode);
	//LogMsg("Arcade input: Hit %d (%s) (%s)", vKey, keyName.c_str(), pressed.c_str());
}

FontLanguageInfo * App::GetFreeTypeManager(string language)
{
	int languageID = 0;

	for (int i = 0; i < m_vecFontInfo.size(); i++)
	{
		if (m_vecFontInfo[i]->m_vecFontOverrideName == language)
		{
			languageID = i;
		}
	}

	return m_vecFontInfo[languageID];
}

bool App::DoesFontHaveOverride(string language)
{
	for (int i = 0; i < m_vecFontInfo.size(); i++)
	{
		if (m_vecFontInfo[i]->m_vecFontOverrideName == language)
		{
			return true;
		}
	}

	return false;
}

void App::SetViewMode(eViewMode viewMode)
{
	m_viewMode = viewMode;
}

void ShowQuickMessage(string msg)
{
	ShowTextMessage(msg, 1000, 0);
}

string App::GetActiveTranslationEngineName()
{
	if (m_translationEngine == TRANSLATION_ENGINE_DEEPL)
	{
		return "Deepl";
	}

	return "Google";
}

void App::ToggleTranslationEngine()
{
	eTranslationEngine oldEngine = m_translationEngine;
	m_translationEngine = (eTranslationEngine)mod( ((int)m_translationEngine + 1), (int)TRANSLATION_ENGINE_COUNT);

	

	ShowQuickMessage("Translation engine is " + GetActiveTranslationEngineName());
	GetAudioManager()->Play("audio/alert.wav");

	m_sig_target_language_changed();
}
void App::SetTargetLanguage(string languageCode, string languageName, bool bShowMessage)
{
	if (bShowMessage)
	{
		ShowQuickMessage("Target language is " + languageName);
		GetAudioManager()->Play("audio/alert.wav");
	}

	if (m_target_language != languageCode)
	{
		m_target_language = languageCode;
		m_sig_target_language_changed();
	}
}

void AppInputRawKeyboard(VariantList *pVList)
{
	char key = (char) pVList->Get(0).GetUINT32();
	bool bDown = pVList->Get(1).GetUINT32() != 0;
	//LogMsg("Raw key %c (%d)",key, (int)bDown);
}

void OnTakeScreenshot()
{

	GetApp()->m_pGameLogicComp->OnTakeScreenshot();
}

void TurnOffRenderDisplay(VariantList* pVList)
{
	GetApp()->m_pGameLogicComp->UpdateStatusMessage("");

	if (!GetApp()->IsInputDesktop() && GetHelpMenu() != NULL)
	{
		KillHelpMenu();
		return;
	}

	GetApp()->m_sig_kill_all_text();
	GetApp()->SetCaptureMode(CAPTURE_MODE_WAITING);
	GetApp()->m_pGameLogicComp->m_escapiManager.SetPauseCapture(false);
	//GetAudioManager()->Play("audio/blip2.wav");

	if (GetApp()->IsInputDesktop())
	{
		LogMsg("Hiding window");
		GetApp()->m_hotKeyHandler.OnHideWindow();
	}

	if (GetApp()->m_cursorShouldBeRestoredToStartPos)
	{
		GetApp()->m_cursorShouldBeRestoredToStartPos = false;

		if (GetApp()->m_oldHWND != 0)
		{
			LogMsg("Restoring hwnd");
			SetForegroundWindow(GetApp()->m_oldHWND);
		}

		RestoreCursorPos();
	}

	GetApp()->m_oldHWND = 0;
}

void OnTranslateButton()
{ 

	if (GetApp()->GetCaptureMode() == CAPTURE_MODE_DRAGRECT)
	{
		LogMsg("Ignoring translate button, currently dragging rect");
	}

	if (GetApp()->GetCaptureMode() == CAPTURE_MODE_WAITING)
	{
	
		GetApp()->m_sig_kill_all_text();
		GetApp()->m_pGameLogicComp->m_escapiManager.SetPauseCapture(true);

		GetApp()->m_oldHWND = GetForegroundWindow();
		MoveWindow(g_hWnd, GetApp()->m_window_pos_x, GetApp()->m_window_pos_y, GetApp()->m_capture_width, GetApp()->m_capture_height, false);
		GetApp()->m_pGameLogicComp->StartProcessingFrameForText();
		GetApp()->SetCaptureMode(CAPTURE_MODE_SHOWING);
	
		if (GetApp()->GetShared()->GetVar("check_disable_sounds")->GetUINT32() == 0)
		{
			AudioHandle handle = GetAudioManager()->Play("audio/wall.mp3");
			GetAudioManager()->SetVol(handle, 0.34f);
		}
		GetApp()->m_hotKeyHandler.OnShowWindow();
	}
	else
	{
		GetMessageManager()->CallStaticFunction(TurnOffRenderDisplay, 200, NULL);
	}
}

void App::ModLanguageByIndex(int change, bool bShowMessage)
{
	if (m_currentLanguageIndex == -1)
	{
		//special case
		if (change >= 0)
		{
			m_currentLanguageIndex = 0;
		}
		else
		{
			m_currentLanguageIndex = (int)m_languages.size() - 1;
		}
	}
	else
	{
		m_currentLanguageIndex = mod(m_currentLanguageIndex + change, (int)m_languages.size());
	}

	SetTargetLanguage(m_languages[m_currentLanguageIndex].m_languageCode, m_languages[m_currentLanguageIndex].m_name, bShowMessage);
}

void App::ShowHelp()
{
	string msg;
	msg += "Dismiss translation screen - SPACE\n";
	msg += "Read text out loud - Left click\n";
	msg += "Read text out load as alternate language Shift-Left click\n";
	msg += "Next/previous language (languages set in config.txt) - [ and ]\n";
	msg += "Quick set target language - 1 through 9\n";
	msg += "Toggle force line by line translation mode - L\n";
	msg += "Toggle force dialog translation mode - D\n";
	msg += "Show original image - Hold UP ARROW\n";
	msg += "Show pre translated OCR results - Hold DOWN ARROW\n";
	msg += "To look up kanji from original - Shift-Right click kanji on original image\n";
	msg += "To copy text - Right click on it\n";
	msg += "Take screenshot - S, E to export to html\n";
	string title = "PlayTrans: UGT "+m_version+" - Hotkeys";

	MessageBox(g_hWnd, _T(msg.c_str()), title.c_str(), NULL);
}


void App::SetGlobalTextHintingToAuto()
{
	SetGlobalTextHinting(HINTING_AUTO);
	ShowQuickMessage("Toggling dialog translation mode to Auto");
	GetApp()->m_sig_target_language_changed();
}

void App::SetDialogMode()
{
	if (GetGlobalTextHinting() == HINTING_DIALOG)
	{
		SetGlobalTextHintingToAuto();
		return;
	}
	
	GetApp()->SetGlobalTextHinting(HINTING_DIALOG);
	ShowQuickMessage("Toggling translation mode to force dialog");
	GetApp()->m_sig_target_language_changed();
}

void App::SetLineByLineMode()
{
	if (GetGlobalTextHinting() == HINTING_LINE_BY_LINE)
	{
		SetGlobalTextHintingToAuto();
		return;
	}
	GetApp()->SetGlobalTextHinting(HINTING_LINE_BY_LINE);
	ShowQuickMessage("Toggling translation mode to force line by line");
	GetApp()->m_sig_target_language_changed();
}

void AppInput(VariantList *pVList)
{

	//0 = message type, 1 = parent coordinate offset, 2 is fingerID
	eMessageType msgType = eMessageType( int(pVList->Get(0).GetFloat()));
	CL_Vec2f pt = pVList->Get(1).GetVector2();
	//pt += GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

	uint32 fingerID = 0;
	if ( msgType != MESSAGE_TYPE_GUI_CHAR && pVList->Get(2).GetType() == Variant::TYPE_UINT32)
	{
		fingerID = pVList->Get(2).GetUINT32();
	}

	CL_Vec2f vLastTouchPt = GetBaseApp()->GetTouch(fingerID)->GetLastPos();

	switch (msgType)
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		//LogMsg("Touch start: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;
	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		//LogMsg("Touch move: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;

	case MESSAGE_TYPE_GUI_CLICK_MOVE_RAW:
		//LogMsg("Touch raw move: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;
	case MESSAGE_TYPE_GUI_CLICK_END:
		//LogMsg("Touch end: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;

	case MESSAGE_TYPE_GUI_CHAR:

		int key = pVList->Get(2).GetUINT32();
		//LogMsg("Hit key %c (%d)", key, (int)key);
		
			if (key == '1') GetApp()->SetTargetLanguage("en", "English");
			if (key == '2')  GetApp()->SetTargetLanguage("ja", "Japanese");
			if (key == '3')  GetApp()->SetTargetLanguage("zh-CN", "Chinese Simplified");
			if (key == '4')  GetApp()->SetTargetLanguage("fr", "French");
			if (key == '5')  GetApp()->SetTargetLanguage("de", "German");
			if (key == '6')  GetApp()->SetTargetLanguage("fi", "Finish");
			if (key == '7')  GetApp()->SetTargetLanguage("ko", "Korean");
			if (key == '8')  GetApp()->SetTargetLanguage("es", "Spanish");
			if (key == '9')  GetApp()->SetTargetLanguage("ru", "Russian");
			if (key == '0')  GetApp()->SetTargetLanguage("hi", "Hindi");
			if (key == 't')  GetApp()->ToggleTranslationEngine();

			if (key == '[')  GetApp()->ModLanguageByIndex(-1, true);
			if (key == ']')  GetApp()->ModLanguageByIndex(1, true);

			
			if (key == '?' || key == 'h')
			{
				GetApp()->ShowHelp();
			}

			if (key == 'd')
			{
				GetApp()->SetDialogMode();
			}
			
			if (key == 'l')
			{
				GetApp()->SetLineByLineMode();
			}

			if (key == VIRTUAL_KEY_BACK) //escape key
			{
				
					if (GetApp()->m_captureMode == CAPTURE_MODE_SHOWING)
					{
						OnTranslateButton();
					}
					else
					{
						GetApp()->m_hotKeyHandler.OnHideWindow();
					}
				
			}
			
			if (key == ' ')
			{
				OnTranslateButton();
			}
			
			if (key == '=')
			{
				GetApp()->ModLanguageByIndex(1);
			}


			if (key == '-')
			{
				GetApp()->ModLanguageByIndex(-1);
			}

			if (key == 'e')
			{
				GetApp()->GetExportToHTML()->Export();
			}


			if (key == 'r')
			{
				ResyncWithCapture();
			}

			if (key == 's')
			{
				OnTakeScreenshot();
			}
		break;

	}	
}

unsigned int GetModifiersForHotKey(HotKeySetting setting)
{
	if (setting.hotKeyName.empty())
	{
		string msg = "Option "+setting.hotKeyAction+" set wrong!  Check the config.txt file";
		ShowQuickMessage(msg);
		LogMsg(msg.c_str());
		return 0;
	}
	unsigned int modifier = 0;

	if (setting.Alt)
	{
		modifier = modifier | MOD_ALT;
	}

	if (setting.bShifted)
	{
		modifier = modifier | MOD_SHIFT;
	}
	if (setting.bCtrl)
	{
		modifier = modifier | MOD_CONTROL;
	}
	return modifier;
}

void InitCURLIfNeeded();

void App::Update()
{
	//game can think here.  The baseApp::Update() will run Update() on all entities, if any are added.  The only one
	//we use in this example is one that is watching for the Back (android) or Escape key to quit that we setup earlier.

	BaseApp::Update();

	if (!m_bDidPostInit)
	{
		//stuff I want loaded during the first "Update"
		m_bDidPostInit = true;
		
		//for android, so the back key (or escape on windows) will quit out of the game
		Entity *pEnt = GetEntityRoot()->AddEntity(new Entity);
		EntityComponent *pComp = pEnt->AddComponent(new CustomInputComponent);
		//tell the component which key has to be hit for it to be activated
		pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));
		//attach our function so it is called when the back key is hit
		//pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, this, _1));

		//nothing will happen unless we give it input focus
		pEnt->AddComponent(new FocusInputComponent);

		//ACCELTEST:  To test the accelerometer uncomment below: (will print values to the debug output)
		//SetAccelerometerUpdateHz(25); //default is 0, disabled
		//GetBaseApp()->m_sig_accel.connect(1, boost::bind(&App::OnAccel, this, _1));

		//TRACKBALL/ARCADETEST: Uncomment below to see log messages on trackball/key movement input
		pComp = pEnt->AddComponent(new ArcadeInputComponent);
		GetBaseApp()->m_sig_arcade_input.connect(1, boost::bind(&App::OnArcadeInput, this, _1));
	
		//these arrow keys will be triggered by the keyboard, if applicable
		AddKeyBinding(pComp, "Left", VIRTUAL_KEY_DIR_LEFT, VIRTUAL_KEY_DIR_LEFT);
		AddKeyBinding(pComp, "Right", VIRTUAL_KEY_DIR_RIGHT, VIRTUAL_KEY_DIR_RIGHT);
		AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
		AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
		AddKeyBinding(pComp, "Fire", VIRTUAL_KEY_CONTROL, VIRTUAL_KEY_GAME_FIRE);

		//INPUT TEST - wire up input to some functions to manually handle.  AppInput will use LogMsg to
		//send them to the log.  (Each device has a way to view a debug log in real-time)
		GetBaseApp()->m_sig_input.connect(&AppInput);

		//this one gives raw up and down of keyboard events, where the one above only gives
		//MESSAGE_TYPE_GUI_CHAR which is just the down and includes keyboard repeats from
		//holding the key
		//GetBaseApp()->m_sig_raw_keyboard.connect(&AppInputRawKeyboard);
		
		/*
		//file handling test, if TextScanner.h is included at the top..

		TextScanner t;
		t.m_lines.push_back("Testing 123");
		t.m_lines.push_back("Heck yeah!");
		t.m_lines.push_back("Whoopsopsop!");

		LogMsg("Saving file...");
		t.SaveFile("temp.txt");


		TextScanner b;
		b.LoadFile("temp.txt");
		b.DumpToLog();
		*/
		InitCURLIfNeeded();
		AddFocusIfNeeded(GetEntityRoot());
		Entity *pScreenShot = CreateOverlayEntity(GetBaseApp()->GetEntityRoot(), "Screenshot", "", 0, 0, true);

		Entity *pGameLogicEnt = pScreenShot->AddEntity(new Entity("GameLogic"));
		m_pGameLogicComp = (GameLogicComponent*)pGameLogicEnt->AddComponent(new GameLogicComponent());

		if (!IsInputDesktop())
		{
#ifdef WINAPI
			SetWindowPos(g_hWnd, HWND_TOP, m_window_pos_x, m_window_pos_y, m_capture_width, m_capture_height, 0);
#endif
		} 
		else 
		{
			m_hotKeyHandler.RegisterHotkey(m_hotkey_for_whole_desktop);
			m_hotKeyHandler.RegisterHotkey(m_hotkey_for_active_window);
			m_hotKeyHandler.RegisterHotkey(m_hotkey_for_draggable_area);
			//GetApp()->m_hotKeyHandler.OnHideWindow();
		}

		//init hotkeys
		GetBaseApp()->m_sig_loadSurfaces.connect(1, boost::bind(&App::OnLoadSurfaces, this));
		GetBaseApp()->m_sig_unloadSurfaces.connect(1, boost::bind(&App::OnUnloadSurfaces, this));

		if (IsInputDesktop())
		{
			if (GetHelpMenu() == NULL)
			{
				CreateHelpMenu(m_pGameLogicComp->GetParent());
			}
		}
	}

	//game is thinking. 
	g_gamepadManager.Update();

	m_pAutoPlayManager->Update();
	HidingOverlayUpdate();


	if (GetCaptureMode() == CAPTURE_MODE_DRAGRECT)
	{
		m_pWinDragRect->Update();
	}

	m_updateChecker.Update();

		if (g_bHasFocus)
		{
			UpdateCursor();
		}


		if (IsShowingHelp())
		{

		}
		else
		{
			if (IsInputDesktop())
			{
				KillHelpMenu();
			}
			else
			{
				if (GetApp()->m_captureMode == CAPTURE_MODE_WAITING)
				{
					KillHelpMenu();
				}
			}
		}
}

void App::OnUnloadSurfaces()
{
	LogMsg("Loading surfaces...");
	m_hotKeyHandler.UnregisterAllHotkeys();
	m_pGameLogicComp->m_desktopCapture.GetSurface()->HardKill();
}

int GetDesiredX()
{
	return GetApp()->m_window_pos_x;
}
int GetDesiredY()
{
	return GetApp()->m_window_pos_y;
}

void OnGotInitialWindowPosition(RECT r)
{

	if (GetApp()->IsInputDesktop())
	{
		LogMsg("Initial window pos is %d, %d", r.left, r.top);
		GetApp()->m_window_pos_x = r.left;
		GetApp()->m_window_pos_y = r.top;
	}

}

void App::OnLoadSurfaces()
{
	LogMsg("Loading surfaces...");
	m_hotKeyHandler.UnregisterAllHotkeys();
	m_hotKeyHandler.ReregisterAllHotkeys();
	
	if (m_captureMode == CAPTURE_MODE_SHOWING)
	{
		MoveWindow(g_hWnd, m_window_pos_x, m_window_pos_y, m_capture_width, m_capture_height, false);
		GetApp()->m_hotKeyHandler.OnShowWindow();
	}
}

bool IsAppShowingHelp()
{
	return GetApp()->IsShowingHelp();
}

bool App::IsShowingHelp()
{
	if (!GetApp()->IsInputDesktop()) return false;
	return GetApp()->m_viewMode == VIEW_MODE_DEFAULT && GetApp()->m_captureMode == CAPTURE_MODE_WAITING;
		

}

void App::ScanSubArea()
{
	//first, change video mode

	GetBaseApp()->SetVideoMode(m_capture_width, m_capture_height, false, 0);
	SetPrimaryScreenSize(m_capture_width, m_capture_height);
	SetupScreenInfo(m_capture_width, m_capture_height, ORIENTATION_DONT_CARE);
	//g_bHasFocus = true;
	OnTranslateButton();
	//GetApp()->m_hotKeyHandler.OnShowWindow();

}

int DivisibleByFour(int num, int max)
{
	while (num % 4 != 0) num++;
	return num;
}

void App::ScanActiveWindow()
{
	RECT pos;
	GetWindowRect(GetForegroundWindow(), &pos);
	m_window_pos_x = pos.left;
	m_window_pos_y = pos.top;
	m_capture_width = pos.right - pos.left;
	m_capture_height = pos.bottom - pos.top;
	m_capture_width = DivisibleByFour(m_capture_width, 0);
	ScanSubArea();
}

void App::HandleHotKeyPushed(HotKeySetting setting)
{

	if (GetApp()->GetCaptureMode() == CAPTURE_MODE_DRAGRECT)
	{
		//not now
		return;
	}

	if (GetApp()->GetCaptureMode() == CAPTURE_MODE_SHOWING)
	{
		OnTranslateButton(); //toggle it off I guess
		return;
	}

	if (setting.hotKeyAction == "hotkey_to_scan_whole_desktop")
	{
	
		HWND        hDesktopWnd = GetDesktopWindow();
		HDC         hDesktopDC = GetDC(hDesktopWnd);
		int windowWidth = GetDeviceCaps(hDesktopDC, HORZRES);
		int windowHeight = GetDeviceCaps(hDesktopDC, VERTRES);
		ReleaseDC(hDesktopWnd, hDesktopDC);

		m_capture_width = windowWidth;
		m_capture_height = windowHeight;
		m_window_pos_x = 0;
		m_window_pos_y = 0;
		LogMsg("Scanning full desktop");

		ScanSubArea();
	}

	if (setting.hotKeyAction == "hotkey_to_scan_active_window")
	{
		if (GetApp()->m_usedSubAreaScan)
		{
			//they've previously scanned a sub area by hand, so let's use that
			GetApp()->ScanSubArea();
		}
		else
		{
			GetApp()->ScanActiveWindow();

		}
		return;
	}

	if (setting.hotKeyAction == "hotkey_to_scan_draggable_area")
	{
		
		if (GetApp()->GetCaptureMode() == CAPTURE_MODE_DRAGRECT)
		{
			LogMsg("Ending drag mode");
			GetApp()->m_pWinDragRect->End();
			GetApp()->SetCaptureMode(CAPTURE_MODE_WAITING);
			return;
		}

		if (GetApp()->GetCaptureMode() == CAPTURE_MODE_WAITING && !g_bHasFocus)
		{
			LogMsg("Scanning draggable area");
			GetApp()->SetCaptureMode(CAPTURE_MODE_DRAGRECT);
			GetApp()->m_pWinDragRect->Start();
		}
		else
		{
			
			LogMsg("Scanning draggable area");
			GetApp()->SetCaptureMode(CAPTURE_MODE_DRAGRECT);
			GetApp()->m_pWinDragRect->Start();
 
		}
	}
}

void App::Draw()
{
	//Use this to prepare for raw GL calls
	PrepareForGL();
#ifdef _DEBUG
	//LogMsg("Doing draw");
#endif
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CLEAR_GL_ERRORS() //honestly I don't know why I get a 0x0502 GL error when doing the FIRST gl action that requires a context with emscripten only

	/*
	//draw our game stuff
	DrawFilledRect(10.0f,10.0f,GetScreenSizeXf()/3,GetScreenSizeYf()/3, MAKE_RGBA(255,255,0,255));
	DrawFilledRect(0,0,64,64, MAKE_RGBA(0,255,0,100));

	//after our 2d rect call above, we need to prepare for raw GL again. (it keeps it in ortho mode if we don't for speed)
	PrepareForGL();
	RenderSpinningTriangle();
	//RenderGLTriangle();
	//let's blit a bmp, but first load it if needed

	if (!m_surf.IsLoaded())
	{
		m_surf.LoadFile("interface/test.bmp");
	}

	m_surf.Bind();

	//RenderTexturedGLTriangle();
	//RenderTexturedGLTriangleWithDrawElements();

	//blit the logo with the Y mirrored
	//rtRect texRect = rtRect(0, m_surf.GetHeight(), m_surf.GetWidth(), 0);
	//rtRect destRect = rtRect(0,0, m_surf.GetWidth(), m_surf.GetHeight());
	//m_surf.BlitEx(destRect, texRect);

	//make the logo spin like a wheel, whee!
	//m_surf.BlitEx(destRect, texRect, MAKE_RGBA(255,255,255,255) , 180*SinGamePulseByMS(3000), CL_Vec2f(m_surf.GetWidth()/2,m_surf.GetHeight()/2));

	//blit it normally
	m_surf.Blit(0, 0);
	//m_surf.Blit(100, 100);

	m_surf.BlitScaled(100, 200, CL_Vec2f(1,1), ALIGNMENT_CENTER, MAKE_RGBA(255,255,255,255), SinGamePulseByMS(3000)*360);

	m_surf.BlitRotated(400, 200, CL_Vec2f(0.2f,0.2f), ALIGNMENT_CENTER, MAKE_RGBA(255,255,255,255), SinGamePulseByMS(4000)*360,
		CL_Vec2f(20,-20), NULL);
		*/

	//GetFont(FONT_SMALL)->Draw(0,0, "test");
	//the base handles actually drawing the GUI stuff over everything else, if applicable

		BaseApp::Draw();
}

void App::OnScreenSizeChange()
{
	BaseApp::OnScreenSizeChange();
}

void App::OnEnterBackground()
{
	//save your game stuff here, as on some devices (Android <cough>) we never get another notification of quitting.
	LogMsg("Entered background");

	
	BaseApp::OnEnterBackground();
}

void App::OnEnterForeground()
{
	LogMsg("Entered foreground");
	BaseApp::OnEnterForeground();
}

const char * GetAppName() {return "PlayTrans: UGT";}

//the stuff below is for android/webos builds.  Your app needs to be named like this.

//note: these are put into vars like this to be compatible with my command-line parsing stuff that grabs the vars

const char * GetBundlePrefix()
{
	const char * bundlePrefix = "com.rtsoft.";
	return bundlePrefix;
}  

const char * GetBundleName()
{
	const char * bundleName = "playtrans";
	return bundleName;
}

HotKeySetting App::GetHotKeyDataFromConfig(string data, string action)
{
	vector<string> words = StringTokenize(data, ",");
	HotKeySetting key;

	for (int i = 0; i < words.size(); i++)
	{
		words[i] = ToLowerCaseString(words[i]);
		words[i] = StripWhiteSpace(words[i]);

		if (words[i] == "shift")
		{
			key.bShifted = true;
		}
		else if (words[i] == "control" || words[i] == "ctrl")
		{
			key.bCtrl = true;
		}
		else if (words[i] == "alt")
		{
			key.Alt = true;
		}
		else
		{
			//must be the key name
			for (int j = 0; j < m_keyData.size(); j++)
			{
				if (words[i] == m_keyData[j].keyName)
				{
					key.hotKeyName = m_keyData[j].keyName;
					key.scanCode = m_keyData[j].scanCode;
					key.virtualKey = m_keyData[j].virtualKey;
				}
			}
		}
	}

	key.hotKeyAction = action;
	key.modifierBits = GetModifiersForHotKey(key);
	key.originalString = data;

	if (key.hotKeyName.empty())
	{ 
		ShowQuickMessage("Error with hotkey " + action + ", unknown key");
	}

	LogMsg(string ("Registered hotkey " + action + " to " + data).c_str());
	return key;
}

bool App::LoadConfigFile()
{
	const int buffSize = 256;
	char buff[buffSize];
	
	LogMsgNoCR("Valid key names: ");
	int lineCounter = 0;

	for (int i = 2; i <= 88; i++)
	{
		GetKeyNameTextA(i << 16, buff, buffSize);
		KeyData k;
		k.keyName = ToLowerCaseString(buff);
		k.virtualKey = MapVirtualKey(i, MAPVK_VSC_TO_VK_EX);
		k.scanCode = i;

		m_keyData.push_back(k);
		
	/*
	//to display all possible keys
		LogMsgNoCR("%s", buff);
		if (i > 2)
		LogMsgNoCR(", ");
		
		lineCounter += strlen(buff) + 2;
		if (lineCounter > 80)
		{
			lineCounter = 0;
			LogMsg("");
		}
		*/
	}
	
	LogMsg("");
	LogMsg("Reading config.txt");

	string audio = "sdl";
	string audioDevice;

	TextScanner ts;
	if (ts.LoadFile("config.txt"))
	{
		m_capture_width = StringToInt(ts.GetParmString("capture_width", 1));
		m_capture_height = StringToInt(ts.GetParmString("capture_height", 1));
		m_window_pos_x = StringToInt(ts.GetParmString("window_pos_x", 1));
		m_window_pos_y = StringToInt(ts.GetParmString("window_pos_y", 1));
		m_show_live_video = StringToInt(ts.GetParmString("show_live_video", 1));
		m_google_api_key = ts.GetParmString("google_api_key", 1);
		m_deepl_api_key = ts.GetParmString("deepl_api_key", 1);
		if (ts.GetParmString("deepl_api_url", 1) != "")
		{
			m_deepl_api_url = ts.GetParmString("deepl_api_url", 1);
		}

		m_jpg_quality_for_scan = StringToInt(ts.GetParmString("jpg_quality_for_scan", 1));
		m_inputMode = ts.GetParmString("input", 1);
		 
		m_log_capture_text_to_file = ts.GetParmString("log_capture_text_to_file", 1);
		m_place_capture_text_on_clipboard = ts.GetParmString("place_capture_text_on_clipboard", 1);


		audioDevice = ts.GetParmString("audio_device", 1);
		if (ts.GetParmString("input_camera_device_id", 1) != "")
		{
			m_input_camera_device_id = StringToInt(ts.GetParmString("input_camera_device_id", 1));
		}

		if (ts.GetParmString("audio", 1) != "")
		{
			audio = ts.GetParmString("audio", 1);
		}
		
		if (ts.GetParmString("minimum_brightness_for_lumakey", 1) != "")
		{
			m_minimum_brightness_for_lumakey = StringToInt(ts.GetParmString("minimum_brightness_for_lumakey", 1));
		}

		if (ts.GetParmString("audio_stop_when_window_is_closed", 1) != "")
		{
			m_audio_stop_when_window_is_closed = StringToBool(ts.GetParmString("audio_stop_when_window_is_closed", 1));
		}
		if (ts.GetParmString("audio_default_language", 1) != "")
		{
			m_audio_default_language = ts.GetParmString("audio_default_language", 1);
		}
		if (ts.GetParmString("check_for_update_on_startup", 1) != "")
		{
			m_check_for_update_on_startup = ts.GetParmString("check_for_update_on_startup", 1);
		}
		
		if (ts.GetParmString("auto_glue_vertical_tolerance", 1) != "")
		{
			m_auto_glue_vertical_tolerance = StringToFloat(ts.GetParmString("auto_glue_vertical_tolerance", 1));
		}

		if (ts.GetParmString("auto_glue_horizontal_tolerance", 1) != "")
		{
			m_auto_glue_horizontal_tolerance = StringToFloat(ts.GetParmString("auto_glue_horizontal_tolerance", 1));
		}

		string translationEngine = ToLowerCaseString(ts.GetParmString("translation_engine", 1));
		if (translationEngine == "deepl")
		{
			m_translationEngine = TRANSLATION_ENGINE_DEEPL;
			LogMsg("Using Deepl for translation, I hope you set its API key.");
		}
		
		if (ts.GetParmString("source_language_hint", 1) != "")
		{
			m_source_language_hint = ts.GetParmString("source_language_hint", 1);
		}
		if (ts.GetParmString("google_text_detection_command", 1) != "")
		{
			m_google_text_detection_command = ts.GetParmString("google_text_detection_command", 1);
		}

		m_gamepad_button_to_scan_active_window = StringToProtonVirtualKey(ToLowerCaseString(ts.GetParmString("gamepad_button_to_scan_active_window", 1)));

		m_hotkey_for_whole_desktop = GetHotKeyDataFromConfig(ts.GetParmString("hotkey_to_scan_whole_desktop", 1), "hotkey_to_scan_whole_desktop");
		m_hotkey_for_active_window = GetHotKeyDataFromConfig(ts.GetParmString("hotkey_to_scan_active_window", 1), "hotkey_to_scan_active_window");
		m_hotkey_for_draggable_area = GetHotKeyDataFromConfig(ts.GetParmString("hotkey_to_scan_draggable_area", 1),"hotkey_to_scan_draggable_area");
		if (ts.GetParmString("kanji_lookup_website", 1) != "")
			m_kanji_lookup_website = ts.GetParmString("kanji_lookup_website", 1);
	}
	else
	{
		LogMsg("Couldn't find config.txt");
		return false;
	}

	//another scan
	for (int i = 0; i < ts.GetLineCount(); i++)
	{
		vector<string> words = ts.TokenizeLine(i);

		if (words.size() > 1)
		{
			if (words[0] == "add_switchable_language")
			{
				LanguageSetting lang;
				lang.m_languageCode = words[1];
				lang.m_name = words[2];
				m_languages.push_back(lang);
			}
		}
	}

	this->ModLanguageByIndex(1, false); //go to first language

	if (audio == "sdl" || audio == "fmod")
	{
#ifdef RT_ENABLE_FMOD
		LogMsg("Enable FMOD audio");
		g_pAudioManager = new AudioManagerFMOD();
#else
		LogMsg("Not compiled with FMOD, so using Audiere for audio");
		g_pAudioManager = new AudioManagerAudiere();
#endif
}
	else if (audio == "audiere")
	{
		LogMsg("Using Audiere for audio");
		g_pAudioManager = new AudioManagerAudiere();
	}
	else
	{
		LogMsg("Audio set to none in config.txt, so disabling it.");
		g_pAudioManager = new AudioManager(); //dummy base, won't play any audio
	}

	LogMsg("Auto-glue set to %.2f and %.2f.", m_auto_glue_vertical_tolerance, m_auto_glue_horizontal_tolerance);

	g_pAudioManager->SetPreferOGG(false);
	GetAudioManager()->SetRequestedDriverByName(audioDevice);
	return true;
}

extern bool g_isBaseAppInitted;
extern string g_fileName;

void OnAppLostFocus()
{

	if (!g_fileName.empty()) return;
	//GetApp()->SetSizeForGUIIfNeeded();
	if (!g_isBaseAppInitted) return;
	
#ifdef _DEBUG
	LogMsg("OnAppLostFocus> View mode is %d, capturemode is %d", GetApp()->m_viewMode, GetApp()->m_captureMode);
#endif

	GetBaseApp()->GetTouch(0)->SetIsDown(false);

	Gamepad* pPad = GetGamepadManager()->GetDefaultGamepad();
	if (pPad)
	{
		pPad->ClearState();
	}
	if (GetApp()->m_captureMode == CAPTURE_MODE_SHOWING)
	{
		//OnTranslateButton(); //get rid of the screen
	}
	else
	{
		//if (GetApp()->m_)
		//GetApp()->m_hotKeyHandler.OnHideWindow();
	}
}

void OnAppGotFocus()
{
	if (!g_isBaseAppInitted) return;
#ifdef _DEBUG
	LogMsg("OnAppGotFocus> View mode is %d, capturemode is %d", GetApp()->m_viewMode, GetApp()->m_captureMode);
#endif
	if (GetApp()->IsShowingHelp())
	{
		LogMsg("Setting size for GUI");
		GetApp()->SetSizeForGUIIfNeeded();
		
		if (GetApp()->m_pGameLogicComp && GetHelpMenu() == NULL)
		{
				CreateHelpMenu(GetApp()->m_pGameLogicComp->GetParent());
		}

	}
}

void OnSettingForGuiIfNeeded()
{
	GetApp()->SetSizeForGUIIfNeeded();
}

void CenterWindow(HWND hWnd);

void App::SetSizeForGUIIfNeeded()
{

	int guiWidth = 1024;
	int guiHeight = 768;

	if (guiWidth == GetScreenSizeX() && guiHeight == GetScreenSizeY())
	{
		LogMsg("Window is already %d, %d", guiWidth, guiHeight);
		return;
	}

	LogMsg("Changing screensize for GUI");

	m_capture_height = guiHeight;
	m_capture_width = guiWidth;

	SetPrimaryScreenSize(guiWidth, guiHeight);
	SetupScreenInfo(guiWidth, guiHeight, ORIENTATION_DONT_CARE);
	GetBaseApp()->SetVideoMode(guiWidth, guiHeight, false, 0);
}

bool App::OnPreInitVideo()
{
	//only called for desktop systems
	//override in App.* if you want to do something here.  You'd have to
	//extern these vars from main.cpp to change them...

	//SetEmulatedPlatformID(PLATFORM_ID_WINDOWS);
	LoadConfigFile();

	int windowWidth = m_capture_width;
	int windowHeight = m_capture_height;
	
	if (IsInputDesktop())
	{
		HWND        hDesktopWnd = GetDesktopWindow();
		HDC         hDesktopDC = GetDC(hDesktopWnd);
		windowWidth = GetDeviceCaps(hDesktopDC, HORZRES);
		windowHeight = GetDeviceCaps(hDesktopDC, VERTRES);
		ReleaseDC(hDesktopWnd, hDesktopDC);
	
		m_window_pos_x = 0;
		m_window_pos_y = 0;

		windowWidth = 1024;
		windowHeight = 768;
	}

	m_capture_width = windowWidth;
	m_capture_height = windowHeight;
	
	SetPrimaryScreenSize(windowWidth, windowHeight);
	SetupScreenInfo(windowWidth, windowHeight, ORIENTATION_DONT_CARE);
	return true; //no error
}

void FontLanguageInfo::SetupFont(string fontName)
{
	m_pVecFreeTypeManager = new FreeTypeManager();
	m_pVecFreeTypeManager->SetFontName(fontName);
}

FreeTypeManager* FontLanguageInfo::GetFont()
{
	if (m_pVecFreeTypeManager)
	{

		if (!m_pVecFreeTypeManager->IsLoaded())
		{
			m_pVecFreeTypeManager->Init();
		}

		return m_pVecFreeTypeManager;
	}
	
	return GetApp()->GetFreeTypeManager("")->GetFont(); //default font as none is set
}
