#include "PlatformPrecomp.h"
#include "CursorComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

CursorComponent::CursorComponent()
{
#ifdef WINAPI
	ShowCursor(false);
#endif
	SetName("Cursor");
}

CursorComponent::~CursorComponent()
{
#ifdef WINAPI
	ShowCursor(true);
#endif
}

void CursorComponent::OnAdd(Entity* pEnt)
{
	EntityComponent::OnAdd(pEnt);
	m_pArrowEnt = NULL;
	m_bDisable = false;
	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();

	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&CursorComponent::OnRender, this, _1));
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&CursorComponent::OnUpdate, this, _1));
	AddInputMovementFocusIfNeeded(GetParent());
	GetParent()->GetParent()->GetFunction("OnKillingControls")->sig_function.connect(1, boost::bind(&CursorComponent::OnKillingControls, this, _1));

}

void CursorComponent::OnRemove()
{
	EntityComponent::OnRemove();
}
void CursorComponent::OnKillingControls(VariantList* pVList)
{
	RemoveFocusIfNeeded(this->GetParent());
	m_bDisable = true;
}

void CursorComponent::OnUpdatePos(CL_Vec2f vPos)
{
	LogMsg("Got %s", PrintVector2(vPos).c_str());
	//DinkSetCursorPosition(NativeToDinkCoords(vPos));
}

void CursorComponent::OnRender(VariantList* pVList)
{
	LogMsg("Rending");
	//CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
}

void CursorComponent::OnUpdate(VariantList* pVList)
{


}

