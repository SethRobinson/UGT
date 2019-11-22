//  ***************************************************************
//  BlockerManager - Creation date: 02/06/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef BlockerManager_h__
#define BlockerManager_h__

class Blocker
{
public:
	SurfaceAnim *m_pSurf = NULL;
	CL_Vec2f m_vPos = CL_Vec2f(0, 0);
	CL_Vec2f m_vSize = CL_Vec2f(0, 0);
	CL_Vec2f m_vTargetSize = CL_Vec2f(1, 1);

	bool m_remove = false;
};

class BlockerManager
{
public:
	BlockerManager();
	virtual ~BlockerManager();

	void AddBlocker();
	void RemoveBlocker();
	int GetBlockerCount();

	void RenderBlocker(Blocker *pBlocker);
	void RenderBlockerRemove(Blocker *pBlocker);
	void SetBlockerCount(int count);
	void Render();

protected:
	
	list<Blocker> m_blockerList;
	list<Blocker> m_removeList;

private:
};

#endif // BlockerManager_h__
