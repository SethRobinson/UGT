//  ***************************************************************
//  CursorComponent - Creation date: 9/13/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef CursorComponent_h__
#define CursorComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"


class CursorComponent : public EntityComponent
{
public:
	CursorComponent();
	virtual ~CursorComponent();

	virtual void OnAdd(Entity* pEnt);
	virtual void OnRemove();

private:

	void OnRender(VariantList* pVList);
	void OnUpdate(VariantList* pVList);

	void OnInput(VariantList* pVList);
	void OnUpdatePos(CL_Vec2f vPos);
	void OnKillingControls(VariantList* pVList);
	CL_Vec2f* m_pPos2d;

	/*
	CL_Vec2f *m_pSize2d;
	float *m_pScale;
	uint32 *m_pColor;
	uint32 *m_pColorMod;
	float *m_pAlpha;
	uint32 *m_pAlignment;
	float *m_pRotation; //in degrees
	*/
	Entity* m_pArrowEnt;
	bool m_bDisable;


};

#endif // CursorComponent_h__