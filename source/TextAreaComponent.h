//  ***************************************************************
//  TextAreaComponent - Creation date: 04/08/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef TextAreaComponent_h__
#define TextAreaComponent_h__
#pragma once

#include "Entity/Component.h"
#include "Network/NetHTTP.h"
#include "GameLogicComponent.h"


class TextAreaComponent : public EntityComponent
{
public:

	TextAreaComponent();
	virtual ~TextAreaComponent();

	void Init(TextArea textArea);
	virtual void OnAdd(Entity *pEnt);

	void OnTouchStart(VariantList *pVList);
	bool ReadTranslationFromJSON(char *pData);
	void OnUpdate(VariantList *pVList);
	void DrawWordRectsForLine(LineInfo line);
	void DrawHighlightRectIfAudioIsPlaying();
	void OnRender(VariantList *pVList);
	TextArea m_textArea;
	void RequestAudio(bool bUseSrcLanguage, bool bShowMessage);
	bool IsStillPlayingOrPlanningToPlay();
	bool FinishedWithTranslation();

protected:

	void StopSoundIfItWasPlaying();
	bool ReadAudioFromJSON(char* pData);
	void RequestTranslation();
	glColorBytes GetTextColor(bool bIsDialog);
	bool IsDialog(bool bIsTranslating);
	void OnSelected(VariantList* pVList);
	void OnTargetLanguageChanged();

	void OnKillAllText();
	bool TranslatingToAsianLanguage();
	bool TranslatingFromAsianLanguage();
	void FitText(float *pHeightInOut, float widthMod,  float trueCharCount);
	void TweakForSending(const string &text, CL_Rectf &rect, float &height, bool isTranslated);
	vector<CL_Vec2f> ComputeLocalLineOffsets();
	void RenderLineByLine();
	void FitAndWordWrapToRect(const CL_Rectf &tempRect, wstring &wtext, deque<wstring> &wlinesOut, CL_Vec2f &wrappedSizeOut,
		bool bUseActualWidthForSpacing, float &pixelHeightOut);
	void RenderAsDialog();

	Entity *m_pTextBox = NULL;
	NetHTTP m_netHTTP;
	NetHTTP m_netAudioHTTP; //so we can request translations and audio at the same time
	Surface *m_pSourceLanguageSurf = NULL;
	Surface *m_pDestLanguageSurf = NULL;
	string m_translatedString;
	CL_Vec2f *m_pPos2d;
	CL_Vec2f *m_pSize2d;
	CL_Rectf m_textAreaRect;
	bool m_bWaitingForTranslation = true;
	Entity* m_pSpeakerIconSrc;
	Entity* m_pSpeakerIconDest;
	AudioHandle m_audioHandle = AUDIO_HANDLE_BLANK;
	string m_fileNameToRemove;
	
};


#endif // TextAreaComponent_h__