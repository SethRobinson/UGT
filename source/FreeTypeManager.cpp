#include "PlatformPrecomp.h"
#include "FreeTypeManager.h"
#include "Renderer/SoftSurface.h"
#include "util/utf8.h"
#include "util/MathUtils.h"
#include "util/MiscUtils.h"

FreeTypeManager::FreeTypeManager()
{
}

FreeTypeManager::~FreeTypeManager()
{
}


bool FreeTypeManager::IsFontCode(const WCHAR *pText, FontStateStack *pState)
{
	if (pText[0] == '`')
	{

		if (pText[1] == 0)
		{
			return true; //malformed font code, remove this line if you want to be able to print ` codes
		}
		//it's a formatting command that is coming
		if (pText[1] == '`')
		{
			if (pState->size() > 1) pState->pop_front();
			return true;
		}

		for (unsigned int i = 0; i < m_fontStates.size(); i++)
		{
			if (pText[1] == m_fontStates[i].m_triggerChar)
			{
				pState->push_front(m_fontStates[i]);
				return true;
			}
		}
	}

	return false;
}

bool FreeTypeManager::IsLoaded()
{
	return m_face != NULL;
}

void FreeTypeManager::MeasureText(rtRectf *pRectOut, const WCHAR *pText, int len, float pixelHeight, bool bUseActualWidthForSpacing)
{

	rtRectf dst(0, 0, 0, 0);
	FontStateStack state;
	FT_Bool       use_kerning;
	
	if (!IsLoaded())
	{
		*pRectOut = dst;
		LogMsg("Error: Font not loaded!");
		return;
	}

	FT_Error error = FT_Set_Pixel_Sizes(
		m_face,   /* handle to face object */
		0,      /* pixel_width           */
		pixelHeight);   /* pixel_height          */

	if (error)
	{
		LogMsg("FT_Set_Pixel_Sizes error");
		return;
	}
	use_kerning = FT_HAS_KERNING(m_face);
	FT_GlyphSlot  slot = m_face->glyph;  /* a small shortcut */
	int           pen_x, pen_y;
	pen_x = 0;
	pen_y = (m_face->size->metrics.ascender + m_face->size->metrics.descender) / 64;
	float baseY = pen_y;

	int lines = 1;
	
	for (int i = 0; i < len; i++)
	{
		m_lastLineHeight = slot->metrics.vertAdvance >> 6;
		//OPTIMIZE: We don't really need IsFontCode and to calculate states to simply measure things.. unless later we handle font
		//changes..
		if (IsFontCode(&pText[i], &state))
		{
			if (pText[i + 1] != 0) i++; //also advance past the color control code
			continue;
		}

		if (pText[i] == '\n')
		{
			lines++;
			dst.right = rt_max(dst.right, pen_x);
			pen_x = 0;
			pen_y += m_lastLineHeight;
			continue;
		}

		//get dimensions of thing
		error = FT_Load_Char(m_face, pText[i], FT_LOAD_RENDER);
		if (error)
		{
			LogMsg("Error loading font char");
			continue;  /* ignore errors */
		}
		if (bUseActualWidthForSpacing)
		{
			pen_x += slot->bitmap.width + 2;
		}
		else
		{
			pen_x += slot->advance.x / 64;
		}
		float letterHeight = slot->metrics.vertAdvance >> 6;
		float offsetY = slot->metrics.vertAdvance >> 6;
	
		dst.bottom = rt_max(dst.bottom, lines* m_lastLineHeight);
	}

	dst.right = rt_max(dst.right, pen_x);
	*pRectOut = dst;
}


wstring FreeTypeManager::GetNextLine(const CL_Vec2f &textBounds, WCHAR **pCur, float pixelHeight, CL_Vec2f &vEnclosingSizeOut, bool bUseActualWidthForSpacing)
{
	//special case to cage a cr at the start
	if ((*pCur)[0] == '\n')
	{
		(*pCur) += 1;
		return L"";
	}

	rtRectf r(0, 0, 0, 0);
	wstring text;
	int lastWrapPoint = 0;

	while (1)
	{
		if ((*pCur)[text.length()] == 0)
		{
			//end of text, return what we have
			(*pCur) += text.length();

			return text;
		}

#ifdef _DEBUG 
		if ((*pCur)[text.length()] == '\r')
		{
			//assert(!"Don't have backslash r's (hex 0d in your strings!");
		}
#endif

		if ((*pCur)[text.length()] == '\n')
		{
			//hardcoded cr, force word wrap here
			(*pCur) += text.length() + 1; //ignore the \n part
			return text;
		}

		text += (*pCur)[text.length()];

		if ((*pCur)[text.length()] == '`')
		{
			//special color code, skip to the next part
			text += (*pCur)[text.length()];
			continue;
		}

		MeasureText(&r, (*pCur), text.length(), pixelHeight, bUseActualWidthForSpacing);

		if (r.GetWidth() > textBounds.x)
		{
			if (lastWrapPoint == 0)
			{
				//roughly break here, maybe the word was too long to wrap
				//text.erase(text.length() - 1, 1);
			}
			else
			{
				//break at the last space we had marked
				text.erase(lastWrapPoint, text.length() - lastWrapPoint);
				(*pCur) += 1; //also get rid of the floating space
			}

			(*pCur) += text.length();
			return text;
		}
		else
		{
			if (vEnclosingSizeOut.x < r.GetWidth()) vEnclosingSizeOut.x = r.GetWidth();

			if ((*pCur)[text.length()] == ' ')
			{
				lastWrapPoint = (int)text.length();

			}
		}

	}

	assert(!"Error");
	return L"";

}

void FreeTypeManager::MeasureTextAndAddByLinesIntoDeque(const CL_Vec2f &textBounds, const wstring &text, deque<wstring> * pLines, float pixelHeight, 
	CL_Vec2f &vEnclosingSizeOut, bool bUseActualWidthForSpacing)
{
	m_lastLineHeight = 0;

	vEnclosingSizeOut = CL_Vec2f(0, 0);

	if (textBounds.x == 0)
	{
		LogError("MeasureTextAndAddByLinesIntoDeque: Can't word wrap with boundsX being 0!");
		return;
	}
	WCHAR *pCur = (WCHAR*)&text[0];
	int lineCount = 0;
	while (pCur[0])
	{
		if (pLines)
		{
			pLines->push_back(GetNextLine(textBounds, &pCur, pixelHeight, vEnclosingSizeOut, bUseActualWidthForSpacing));
		}
		else
		{
			GetNextLine(textBounds, &pCur, pixelHeight, vEnclosingSizeOut, bUseActualWidthForSpacing);
		}
		lineCount++;
	}

	vEnclosingSizeOut.y = float(lineCount)*GetLineHeight(pixelHeight);

}


void FreeTypeManager::draw_bitmap(FT_Bitmap*  bitmap,
	FT_Int      x,
	FT_Int      y, SoftSurface *pSoftSurf, glColorBytes fgColor)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;


	/* for simplicity, we assume that `bitmap->pixel_mode' */
	/* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */
	byte bwvalue = 0;

	for (i = x, p = 0; i < x_max; i++, p++)
	{
		for (j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 ||
				i >= pSoftSurf->GetWidth() || j >= pSoftSurf->GetHeight())
				continue;
			
			byte bwvalue = bitmap->buffer[q * bitmap->width + p];
			float alpha = (float)bwvalue / 255.0f;

			if (bwvalue > 0)
			{
				pSoftSurf->SetPixel(i, j, glColorBytes(fgColor.r*alpha, fgColor.g*alpha, fgColor.b*alpha, 255));
			}

			//image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
}

template <typename T>
T nextPowerOfTwo(T n)
{
	std::size_t k = 1;
	--n;
	do {
		n |= n >> k;
		k <<= 1;
	} while (n & (n + 1));
	return n + 1;
}

Surface * FreeTypeManager::TextToSurface(CL_Vec2f surfaceSizeToCreate, string msg, float pixelHeight,
	glColorBytes bgColor, glColorBytes fgColor, bool bUseActualWidthForSpacing, vector<CL_Vec2f> *pOptionalLineStarts,
	float wordWrapX)
{
	// Convert it to utf-16 to freetype can understand it
	vector<unsigned short> utf16line;
	utf8::utf8to16(msg.begin(), msg.end(), back_inserter(utf16line));
	
	return TextToSurface(surfaceSizeToCreate, utf16line, pixelHeight,
		bgColor, fgColor, bUseActualWidthForSpacing, pOptionalLineStarts, wordWrapX);
}

float FreeTypeManager::GetLineHeight(float pixelHeight)
{
	assert(m_lastLineHeight != 0 && "This needs to be set before using, too expensive to do here each time");
	return m_lastLineHeight;
}

Surface * FreeTypeManager::TextToSurface(CL_Vec2f surfaceSizeToCreate, vector<unsigned short> utf16line, float pixelHeight,
	glColorBytes bgColor, glColorBytes fgColor, bool bUseActualWidthForSpacing, vector<CL_Vec2f> *pOptionalLineStarts,
	float wordWrapX)
{
	int minSize = 10;

	FT_Bool       use_kerning;
	if (surfaceSizeToCreate.y < minSize) surfaceSizeToCreate.y = minSize;
	if (pixelHeight < minSize) pixelHeight = minSize;

	//if (pixelHeight < 16) pixelHeight = 20;
	
	FT_Error error = FT_Set_Pixel_Sizes(
		m_face,   /* handle to face object */
		0,      /* pixel_width           */
		pixelHeight);   /* pixel_height          */

	if (error)
	{
		LogMsg("FT_Set_Pixel_Sizes error");
		return NULL;
	}

	use_kerning = FT_HAS_KERNING(m_face);

	SoftSurface softSurf;
	softSurf.Init(surfaceSizeToCreate.x, surfaceSizeToCreate.y, SoftSurface::SURFACE_RGBA);
	softSurf.FillColor(bgColor);

	FT_GlyphSlot  slot = m_face->glyph;  /* a small shortcut */
	int           pen_x, pen_y;

	pen_x = 0;
	pen_y = (m_face->size->metrics.ascender+ m_face->size->metrics.descender) / 64;
	float baseY = pen_y;
	
	FT_UInt lastChar = 0;
	float kerning = 0;
	int lineCount = 0;

	if (wordWrapX == 0 && pOptionalLineStarts && pOptionalLineStarts->size() > lineCount)
	{
		pen_x = pOptionalLineStarts->at(lineCount).x;
		pen_y = baseY + pOptionalLineStarts->at(lineCount).y;
	}

	for (int n = 0; n < utf16line.size(); n++)
	{
		/* load glyph image into the slot (erase previous one) */
	
		bool bForceCR = (wordWrapX > 0 && pen_x != 0 && pen_x > (wordWrapX - (slot->advance.x / 64)));

		if (utf16line[n] == '\n' || bForceCR)
		{

			if (bForceCR && utf16line[n] != '\n')
			{
				n--; //do this letter again
			}
			pen_x = 0;
			pen_y += slot->metrics.vertAdvance >> 6;
			lastChar = 0;
			lineCount++;
			if (wordWrapX == 0 && pOptionalLineStarts && pOptionalLineStarts->size() > lineCount)
			{
				pen_x = pOptionalLineStarts->at(lineCount).x;
				pen_y = baseY+pOptionalLineStarts->at(lineCount).y;
			}
			continue;
		}

		error = FT_Load_Char(m_face, utf16line[n], FT_LOAD_RENDER);
		if (error)
			continue;  /* ignore errors */

		draw_bitmap(&slot->bitmap,
			pen_x + slot->bitmap_left,
			pen_y-slot->bitmap_top, &softSurf, fgColor);
			
		/* increment pen position */
		if (bUseActualWidthForSpacing)
		{
			pen_x += slot->bitmap.width + 2;
		}
		else
		{
			pen_x += slot->advance.x / 64;
		}
		lastChar = utf16line[n];
	}

	Surface *pSurf = new Surface();
	softSurf.FlipY();
	pSurf->InitFromSoftSurface(&softSurf, true, 0);

	return pSurf;
}

int FreeTypeManager::GetKerningOffset(FT_UInt leftGlyph, FT_UInt rightGlyph)
{
	FT_Vector kerning;
	int error;

	error = FT_Get_Kerning(m_face, leftGlyph, rightGlyph, FT_KERNING_DEFAULT, &kerning);

	if (error) {
		// TODO error handling.
	}

	return kerning.x / 64;
}


bool FreeTypeManager::Init()
{
	string fontName = "SourceHanSerif-Medium.ttc";

	FT_Error error = FT_Init_FreeType(&m_library);
	if (error)
	{
		LogMsg("Freetype error");
	}

	error = FT_New_Face(m_library,
		fontName.c_str(),
		0,
		&m_face);
	
	if (error == FT_Err_Unknown_File_Format)
	{
		LogMsg("Freetype: the font file could be opened and read, but it appears that its font format is unsupported");
		return false;
	}
	else if (error)
	{
		LogMsg("Freetype: another error code means that the font file could not be opened or read, or that it is broken");
		return false;
	}

	return true;
}
