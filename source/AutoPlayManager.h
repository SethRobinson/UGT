#pragma once

class TextAreaComponent;

class AutoPlayManager
{
	public:
		
		AutoPlayManager();

		void OnKillAllText();

		void Reset();
		void OnAddDialog(TextAreaComponent* pEnt);
		void OnLoadingFinished(TextAreaComponent* pEnt);
		void Update();
		bool m_bHaveStartedPlaying = false;

		std::list<TextAreaComponent*> m_areas;
		std::list<TextAreaComponent*> m_loadedAreas;
};

