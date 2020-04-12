//  ***************************************************************
//  FreeTypeManager - Creation date: 04/08/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef FreeTypeManager_h__
#define FreeTypeManager_h__

#include <ft2build.h>

#include FT_FREETYPE_H
#include "GUI/RTFont.h"

typedef std::deque<FontState> FontStateStack;

class FreeTypeManager
{
public:
	FreeTypeManager();
	virtual ~FreeTypeManager();

	bool IsFontCode(const WCHAR *pText, FontStateStack *pState);
	bool IsLoaded();
	void MeasureText(rtRectf *pRectOut, const WCHAR *pText, int len, float pixelHeight, bool bUseActualWidthForSpacing);
	
	
	Surface * FreeTypeManager::TextToSurface(CL_Vec2f surfaceSizeToCreate, vector<unsigned short> utf16line, float pixelHeight,
		glColorBytes bgColor, glColorBytes fgColor, bool bUseActualWidthForSpacing, vector<CL_Vec2f> *pOptionalLineStarts, float wordWrapX);
	Surface * FreeTypeManager::TextToSurface(CL_Vec2f surfaceSizeToCreate, string msg, float pixelHeight,
		glColorBytes bgColor, glColorBytes fgColor, bool bUseActualWidthForSpacing, vector<CL_Vec2f> *pOptionalLineStarts, float wordWrapX);

	int GetKerningOffset(FT_UInt c, FT_UInt pc);
	bool Init();

	void MeasureTextAndAddByLinesIntoDeque(const CL_Vec2f &textBounds, const wstring &text, deque<wstring> * pLines, float pixelHeight, CL_Vec2f &vEnclosingSizeOut,
		bool bUseActualWidthForSpacing);
	void SetFontName(string fontName) { m_fontName = fontName; }

protected:

	wstring GetNextLine(const CL_Vec2f &textBounds, WCHAR **pCur, float pixelHeight, CL_Vec2f &vEnclosingSizeOut, bool bUseActualWidthForSpacing);

	void draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y, SoftSurface *pSoftSurf, glColorBytes fgColor);

	float GetLineHeight(float pixelHeight);
	FT_Library  m_library = NULL;
	FT_Face     m_face = NULL;      /* handle to face object */
	vector<FontState> m_fontStates;
	float m_lastLineHeight = 0;
	string m_fontName;

private:
};

#endif // FreeTypeManager_h__#pragma once
