#include "PlatformPrecomp.h"
#include "AutoPlayManager.h"
#include "TextAreaComponent.h"
#include "App.h"

AutoPlayManager::AutoPlayManager()
{
	LogMsg("AutoPlay manager initted");
	GetApp()->m_sig_kill_all_text.connect(1, boost::bind(&AutoPlayManager::OnKillAllText, this));

}

void AutoPlayManager::OnKillAllText()
{
	Reset();
}

void AutoPlayManager::Reset()
{
	m_bHaveStartedPlaying = false;
	m_areas.clear();
	m_loadedAreas.clear();
}

void AutoPlayManager::OnAddDialog(TextAreaComponent* pEnt)
{
	LogMsg("Adding %s", pEnt->m_textArea.text.c_str());
	m_areas.push_back(pEnt);

	
}

void AutoPlayManager::OnLoadingFinished(TextAreaComponent* pEnt)
{
	m_loadedAreas.push_back(pEnt);

}

void AutoPlayManager::Update()
{
	if (m_areas.empty()) return;
	
		if (!m_bHaveStartedPlaying)
		{
		
			if (GetApp()->GetShared()->GetVar("check_src_audio")->GetUINT32() == 0)
			{
				//only continue if the translation has been done already
				if (!m_areas.front()->FinishedWithTranslation()) return; //wait
			}
			m_bHaveStartedPlaying = true;

			m_areas.front()->RequestAudio(true, false);
		}
		else
		{
			if (!m_areas.front()->IsStillPlayingOrPlanningToPlay())
			{
				//kill it and move on to the next
				m_areas.pop_front();
				if (!m_areas.empty())
				{
					if (GetApp()->GetShared()->GetVar("check_src_audio")->GetUINT32() == 0)
					{
						//only continue if the translation has been done already
						if (!m_areas.front()->FinishedWithTranslation()) return; //wait
					}

					m_areas.front()->RequestAudio(true, false);
				}
			}
		}

	

}
