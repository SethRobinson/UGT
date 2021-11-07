//  ***************************************************************
//  HotKeyHandler - Creation date: 04/12/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef HotKeyHandler_h__
#define HotKeyHandler_h__
#pragma once


class HotKeySetting
{
public:
	string hotKeyName;
	string hotKeyAction;
	int virtualKey = 0;
	int scanCode = 0;
	bool bShifted = false;
	bool bCtrl = false;
	bool Alt = false;
	unsigned int modifierBits = 0;
	unsigned int m_registrationID = 0;
	string originalString;
	bool m_bFirstTime = true;
};


class HotKeyHandler
{
public:
	
	HotKeyHandler();
	virtual ~HotKeyHandler();

	HotKeySetting GetHotKeyByID(unsigned int ID);
	HotKeySetting GetHotKeyByAction(string action);

	void UnregisterAllHotkeys();
	void ReregisterAllHotkeys();
	vector<HotKeySetting> m_keysActive;
	unsigned int m_registrationCounter = 0;
	void OnHideWindow();
	void OnShowWindow();
	//See https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes
	void RegisterHotkey(HotKeySetting &setting);
protected:
	

private:
};

#endif // HotKeyHandler_h__

