#include "PlatformPrecomp.h"
#include "BlockerManager.h"

BlockerManager::BlockerManager()
{
}

BlockerManager::~BlockerManager()
{
}

void BlockerManager::AddBlocker()
{
	Blocker b;

	b.m_pSurf = GetResourceManager()->GetSurfaceAnim("interface/blocker1.rttex");
	b.m_vPos = CL_Vec2f(RandomRangeFloat(0, GetScreenSizeXf()), RandomRangeFloat(0, GetScreenSizeYf()));
	b.m_vTargetSize = CL_Vec2f(RandomRangeFloat(0.3f, 1.0f), RandomRangeFloat(0.3f, 1.0f));
	b.m_vTargetSize.y = b.m_vTargetSize.x; //make it round

	m_blockerList.push_back(b);

	GetAudioManager()->Play("audio/blip3.wav");
}

void BlockerManager::RemoveBlocker()
{
	if (GetBlockerCount() > 0)
	{
		m_removeList.push_front(m_blockerList.back());
		m_blockerList.pop_back();
	}
}

int BlockerManager::GetBlockerCount()
{
	return m_blockerList.size();
}

void BlockerManager::RenderBlocker(Blocker *pBlocker)
{
	pBlocker->m_vSize = LerpVector(pBlocker->m_vSize, pBlocker->m_vTargetSize, 0.05f);

	pBlocker->m_pSurf->BlitRotatedAnim(pBlocker->m_vPos.x, pBlocker->m_vPos.y, 0, 0, pBlocker->m_vSize,
		ALIGNMENT_CENTER, MAKE_RGBA(255, 255, 255, 255), 0);
}


void BlockerManager::RenderBlockerRemove(Blocker *pBlocker)
{
	pBlocker->m_vSize = LerpVector(pBlocker->m_vSize, CL_Vec2f(0,0), 0.05f);

	if (pBlocker->m_vSize == CL_Vec2f(0, 0))
	{
		pBlocker->m_remove = true;
		return;
	}
	pBlocker->m_pSurf->BlitRotatedAnim(pBlocker->m_vPos.x, pBlocker->m_vPos.y, 0, 0, pBlocker->m_vSize,
		ALIGNMENT_CENTER, MAKE_RGBA(255, 255, 255, 255), 0);
}

void BlockerManager::SetBlockerCount(int count)
{
	//first add if needed
	while (GetBlockerCount() < count)
	{
		AddBlocker();
	}

	while (GetBlockerCount() > count)
	{
		RemoveBlocker();
	}
}

void BlockerManager::Render()
{
	list<Blocker>::iterator itor = m_blockerList.begin();

	while (itor != m_blockerList.end())
	{
		RenderBlocker(&(*itor));
		itor++;
	}

	itor = m_removeList.begin();

	while (itor != m_removeList.end())
	{
		RenderBlockerRemove(&(*itor));
		list<Blocker>::iterator itorOld = itor;
		itor++;

		if (itorOld->m_remove)
			m_removeList.erase(itorOld);

	}
}
