#include "PlatformPrecomp.h"
#include "ExportToHTML.h"
#include "WinUtils.h"
#include "App.h"
#include "GameLogicComponent.h"
#include "TextAreaComponent.h"
#include "util/TextScanner.h"

#ifdef WINAPI
extern HWND				g_hWnd;
#endif
bool RTCreateDirectory(const std::string& dir_name); // I don't know why this isn't defined somewhere but whatever

ExportToHTML::ExportToHTML()
{
}

ExportToHTML::~ExportToHTML()
{
}

string ExportToHTML::ExportToString(string mode, bool bAddEndingCRs)
{
	string final;
	
	for (int i = 0; i < GetApp()->GetGameLogicComponent()->m_textComps.size(); i++)
	{
		TextAreaComponent* comp = GetApp()->GetGameLogicComponent()->m_textComps[i];

		string preTranslatedText;

		//add pre translated text?
			//all together as one block
			if (comp->IsDialog(true))
			{
				preTranslatedText += comp->m_textArea.rawText;
			}
			else
			{
				for (int j = 0; j < comp->m_textArea.m_lines.size(); j++)
				{
					preTranslatedText += comp->m_textArea.m_lines[j].m_text+"\r\n";
				}
			}

			if (bAddEndingCRs)
			{
				preTranslatedText += "\r\n";
			}

			if (mode == "full" || mode == "pre")
			{
				final += preTranslatedText;
			}
	
		//add post translated text?
		if (mode == "full" || mode == "post")
		{
			string translatedText = comp->GetTranslatedText();

			final += translatedText;

			if (translatedText.empty())
			{
				//well, since we don't have a translation, let's just put the original there I guess
				final += preTranslatedText;
			}
		}

		if (bAddEndingCRs)
		{
			final += "\r\n\r\n";
		}

	}
	return final;
}

void ExportToHTML::AddOverlays(string* pHTML, const string itemTemplate)
{

	for (int i = 0; i < GetApp()->GetGameLogicComponent()->m_textComps.size(); i++)
	{
		TextAreaComponent* comp = GetApp()->GetGameLogicComponent()->m_textComps[i];
		string work = itemTemplate;

		StringReplace("[TRANSLATED_TEXT]", toString(comp->GetTranslatedText()), work);
		StringReplace("[END_X]", toString(comp->m_textArea.m_rect.right), work);
		StringReplace("[END_Y]", toString(comp->m_textArea.m_rect.bottom), work);
		StringReplace("[WIDTH]", toString(comp->m_textArea.m_rect.get_width()), work);
		StringReplace("[HEIGHT]", toString(comp->m_textArea.m_rect.get_height()), work);
		StringReplace("[X]", toString(comp->m_textArea.m_rect.left), work);
		StringReplace("[Y]", toString(comp->m_textArea.m_rect.top), work);
		StringReplace("[FONT_SIZE]", toString(comp->m_textArea.m_averageTextHeight * 0.83f), work);

		string finalText;

		//if (comp->IsDialog(true)) //uh, I guess we don't actually treat dialog/line by line text differently now here
		   
			//we'll need to add our own line feeds
			for (int j = 0; j < comp->m_textArea.m_lines.size(); j++)
			{
				if (j > 0)
				{
					finalText += "\n";
				}
				finalText += comp->m_textArea.m_lines[j].m_text;
			}
		
		StringReplace("\n", "<br>", finalText);
		StringReplace("[TEXT]", toString(finalText), work);

		*pHTML += work;
	}

	//LogMsg("Found text comp %s", comp->m_textArea.rawText.c_str());
	ShowQuickMessage("Exporting to htmlexport/index.html");

}

bool ExportToHTML::Export()
{
	string msg = "Nothing to export";
	string title = "Export to HTML";
	
	if (!FileExists("temp.jpg"))
	{
		msg = "Nothing to export, temp.jpg doesn't even exist.";
		MessageBox(g_hWnd, _T(msg.c_str()), title.c_str(), NULL);
		return false;
	}
	string htmlDir = "htmlexport/";
	RTCreateDirectory(htmlDir);
	CopyFile("temp.jpg", (htmlDir+"background.jpg").c_str(), false);

	string htmlFile = "index.html";
	RemoveFile(htmlDir + htmlFile);

	//inject header
	TextScanner t;
	t.LoadFile(htmlDir + "header_insert.txt");

	string html;
	TextScanner itemTemp;
	itemTemp.LoadFile(htmlDir + "text_overlay_template.txt");
	AddOverlays(&html, itemTemp.GetAllRaw());
	t.AppendFromString(html);
	//setup a div so any text after this goes below the image, I couldn't figure out a better way to go below the
	//background image properly
	t.AppendFromString(string("<div style=\"top: ")+toString(GetApp()->m_capture_height)+"; position:relative; \">");
	t.AppendToFile(htmlDir + htmlFile);
	
	//add the footer too
	t.LoadFile(htmlDir + "footer_insert.txt");
	t.AppendToFile(htmlDir + htmlFile);
	LaunchURL(GetBaseAppPath() + htmlDir + htmlFile);
	//string browserExe = "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
	//string parms = "--window-position=1920,0 --new-window www.google.com";
	return true; //success
}
