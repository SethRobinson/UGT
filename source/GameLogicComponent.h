//  ***************************************************************
//  GameLogicComponent - Creation date: 02/05/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GameLogicComponent_h__
#define GameLogicComponent_h__

#include "Entity/Component.h"
#include "Network/NetHTTP.h"
#include "util/cJSON.h"
#include "EscapiManager.h"
#include "WinDesktopCapture.h"

class TextAreaComponent;


class WordInfo
{
public:
	CL_Rectf m_rect;
	string m_word;
};

class LineInfo
{
public:
	CL_Rectf m_lineRect;
	string m_text;
	vector<WordInfo> m_words;
};

class TextArea
{
public:

	bool m_bIsDialog = false; //if yes, we'll send to google without CRs so it can translate it differently
	string rawText;
	string text;
	vector<unsigned short> wideText; //after utf8 is modified to shorts

	string language;
	CL_Rectf m_rect = CL_Rectf(0,0,0,0);
	vector<CL_Vec2f> m_lineStarts; //top left position of each line start
	vector<LineInfo> m_lines;
	float m_ySpacingToNextLineAverage = 0;
	float m_averageTextHeight = 0;

};


class GameLogicComponent : public EntityComponent
{
public:

	GameLogicComponent();
	virtual ~GameLogicComponent();

	void OnSelected(VariantList* pVList);

	void CreateExamineOverlay();

	void KillExamineOverlay();

	virtual void OnAdd(Entity *pEnt);

	void UpdateStatusMessage(string msg);

	void OnUpdate(VariantList *pVList);
	void OnRender(VariantList *pVList);
	void OnTakeScreenshot();

	void OnFinishedTranslations();

	void OnTargetLanguageChanged();

	Entity *m_pScreenShot;

	NetHTTP m_netHTTP;
	std::vector<TextArea> m_textareas;
	void StartProcessingFrameForText();
	EscapiManager m_escapiManager;
	WinDesktopCapture m_desktopCapture;
	string m_status;

	std::vector<TextAreaComponent*> m_textComps;
	void AddTextBox(TextAreaComponent *p);
	void RemoveTextBox(TextAreaComponent *p);

private:

	bool ProcessParagraphGoogleWay(const cJSON* paragraph, TextArea& textArea);
	bool ReadFromParagraph(const cJSON *paragraph, TextArea &textArea);
	void ConstructEntityFromTextArea(TextArea &textArea);
	void ConstructEntitiesFromTextAreas();
	void MergeWithPreviousTextIfNeeded(TextArea& textArea);
	bool BuildDatabase(char *pJson);
	Entity* m_pSettingsIcon = NULL;
	bool m_bCalledOnFinishedTranslations = false;

};

void RestoreCursorPos();
void SaveCursorPos();
bool IsAsianLanguage(string languageCode);
#endif // GameLogicComponent_h__