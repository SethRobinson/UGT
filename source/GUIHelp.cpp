#include "PlatformPrecomp.h"
#include "GUIHelp.h"
#include "App.h"
#include "Entity/EntityUtils.h"
#include "Entity/ArcadeInputComponent.h"

void HelpMenuOnSelect(VariantList *pVList) //0=vec2 point of click, 1=entity sent from
{
	Entity *pEntClicked = pVList->m_variant[1].GetEntity();
	LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(), pVList->m_variant[1].Print().c_str());
	Entity *pMenu = GetEntityRoot()->GetEntityByName("HelpMenu");

	//LogMsg("Clicked %s entity at %s", pEntClicked->GetName().c_str(),pVList->m_variant[0].Print().c_str());

	if (pEntClicked->GetName() == "check_autoplay_audio")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetShared()->GetVar("check_autoplay_audio")->Set(uint32(bChecked));
	}

	if (pEntClicked->GetName() == "check_src_audio")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetShared()->GetVar("check_src_audio")->Set(uint32(bChecked));
	}
	
	if (pEntClicked->GetName() == "check_hide_overlay")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetShared()->GetVar("check_hide_overlay")->Set(uint32(bChecked));
	}

	if (pEntClicked->GetName() == "check_disable_sounds")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetShared()->GetVar("check_disable_sounds")->Set(uint32(bChecked));
	}

	if (pEntClicked->GetName() == "check_invisible_mode")
	{
		bool bChecked = IsCheckboxChecked(pEntClicked);
		GetApp()->GetShared()->GetVar("check_invisible_mode")->Set(uint32(bChecked));
	}


	if (pEntClicked->GetName() == "Hide")
	{
		if (GetApp()->IsInputDesktop())
		{
			GetApp()->m_hotKeyHandler.OnHideWindow();

		}
		else
		{
			KillHelpMenu();
		}
		return;
	}

	//GetEntityRoot()->PrintTreeAsText(); //useful for debugging
}


Entity * CreateHelpMenu(Entity *pRoot)
{

	float width = 1024;
	float height = 768;

	CL_Rectf r = CL_Rectf(0,0, width, height);
	Entity* pBG = CreateOverlayRectEntity(pRoot, r, MAKE_RGBA(40, 40, 40, 200));
	pBG->SetName("HelpMenu");

	AddFocusIfNeeded(pBG);


	Entity *pLogo = CreateOverlayButtonEntity(pBG, "Logo", "interface/logo.rttex", width / 2, 20);
	SetAlignmentEntity(pLogo, ALIGNMENT_UPPER_CENTER);

	Entity *pButtonEntity;

	string msg;

	HotKeySetting h;

	if (GetApp()->IsInputDesktop())
	{
		h = GetApp()->m_hotKeyHandler.GetHotKeyByAction("hotkey_to_scan_active_window");
		msg += "Translate active window - `$" + h.originalString + "`` or `$"+ ProtonVirtualKeyToString(GetApp()->m_gamepad_button_to_scan_active_window)+"`` on Gamepad\n";
		h = GetApp()->m_hotKeyHandler.GetHotKeyByAction("hotkey_to_scan_whole_desktop");
		msg += "Translate entire desktop - `$" + h.originalString + "``\n";
	
		h = GetApp()->m_hotKeyHandler.GetHotKeyByAction("hotkey_to_scan_draggable_area");
		msg += "Translate draggable rectangle - `$" + h.originalString + "``\n";

		
		msg += "Continue after a translation - `$<space>``\n";
		msg += "Show options that work after a translation is done - `$?`` or `$H`` key\n";
	
	}
	else
	{
		msg += "In camera mode for 360 controller & Luma-key.";
		msg += "  `$B`` to translate/close translation.  `$A ``to select option. `$Left Joy`` to move cursor. `$X`` for screenshot.";
		msg += "  `$Y`` to re-sync with video signal.  `$Dpad-Left`` to force line-by-line translation. `$Dpad-Right`` to force dialog-mode translation.";
		msg += "  `$Dpad-Up`` to show original picture.  `$Dpad-Down`` to show pre-translation OCR results.";
		msg += "  Hold `$LJoy Button`` to move fast. `$Select + A`` for alt translation.\n";
	}
	msg += "Source language hint: `$" + GetApp()->m_source_language_hint+"``\n";

	msg += "\nVersion "+GetApp()->GetAppVersion()+" by Seth A. Robinson (c) 2019-2021\n";

	Entity *pText = CreateTextBoxEntity(pBG, "Text", CL_Vec2f(100, 180), CL_Vec2f(800, 800), msg);

	float y = 420;
	Entity* pEnt;
	float startX = 100;
	float spacerY = 10;

	bool bAutoPlay= GetApp()->GetVar("check_autoplay_audio")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_autoplay_audio", "Automatically speak dialog", startX, y, bAutoPlay, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&HelpMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;

	bool bPlaySrc = GetApp()->GetVar("check_src_audio")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_src_audio", "Speak pre-translated text", startX, y, bPlaySrc, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&HelpMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;

	bool bPlayHide = GetApp()->GetVar("check_hide_overlay")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_hide_overlay", "Hide overlay text until cursor is moved", startX, y, bPlayHide, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&HelpMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;

	//second column of options
	startX = 520;
	y = 420;

	bool bDisableSounds = GetApp()->GetVar("check_disable_sounds")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_disable_sounds", "Disable capture sound", startX, y, bDisableSounds, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&HelpMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;

	bool bDisableVisuals = GetApp()->GetVar("check_invisible_mode")->GetUINT32() != 0;
	pEnt = CreateCheckbox(pBG, "check_invisible_mode", "Invisible mode (no overlay)", startX, y, bDisableVisuals, FONT_SMALL, 1.0f);
	pEnt->GetFunction("OnButtonSelected")->sig_function.connect(&HelpMenuOnSelect);
	y += GetSize2DEntity(pEnt).y;
	y += spacerY;


	if (GetApp()->GetGoogleKey() == "yourkeygoeshere")
	{
		Entity *pCrap = CreateTextLabelEntity(pBG, "Crap", 50, 560, "`4WARNING``: You need to edit config.txt and change\n'`$yourkeygoeshere``' to your Google API key!");
		SetupTextEntity(pCrap, FONT_LARGE, 0.7f);
	}
	
	pButtonEntity = CreateTextButtonEntity(pBG, "Hide", width/2, 600+60, "Ok (minimize and listen for hotkeys)", false);
	pButtonEntity->GetFunction("OnButtonSelected")->sig_function.connect(&HelpMenuOnSelect);
	SetupTextEntity(pButtonEntity, FONT_LARGE);
	SetAlignmentEntity(pButtonEntity, ALIGNMENT_CENTER);


	pButtonEntity = CreateTextButtonEntity(pBG, "Exit", width / 2, 660+60, "Quit (shutdown)", false);
	//pButtonEntity->GetFunction("OnButtonSelected")->sig_function.connect(&HelpMenuOnSelect);

	pButtonEntity->GetFunction("OnButtonSelected")->sig_function.connect(1, boost::bind(&App::OnExitApp, GetApp(), _1));
	SetupTextEntity(pButtonEntity, FONT_LARGE);
	SetAlignmentEntity(pButtonEntity, ALIGNMENT_CENTER);

	//AddHotKeyToButton(pButtonEntity, VIRTUAL_KEY_BACK);

	return pBG;
}

void KillHelpMenu()
{
	Entity *pEnt = GetHelpMenu();
	if (pEnt)
	{
		//pEnt->SetTaggedForDeletion();
		KillEntity(pEnt, 100);
	}
	GetApp()->GetShared()->Save("config.dat");

}

Entity * GetHelpMenu()
{
	return GetEntityRoot()->GetEntityByName("HelpMenu");
}
