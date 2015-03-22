#define NOMINMAX

#include "RootSceneNode.h"

#include "Light.h"
#include "DrawList.h"

#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Render/ShaderActions/SetPerFrameConstantsShaderAction.h"

#include "CameraManager.h"
#include "CameraSceneNode.h"
namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(RootSceneNode, SceneNode);

Handle RootSceneNode::s_hTitleInstance;
Handle RootSceneNode::s_hInstance;
Handle RootSceneNode::s_hCurInstance;

void RootSceneNode::Construct(PE::GameContext &context, PE::MemoryArena arena)
{
	Handle h("ROOT_SCENE_NODE", sizeof(RootSceneNode));
	RootSceneNode *pRootSceneNode = new(h) RootSceneNode(context, arena, h);
	pRootSceneNode->addDefaultComponents();
	pRootSceneNode->m_toggle=0;
	pRootSceneNode->m_intensity=1;
	SetInstance(h);
	
	s_hTitleInstance = Handle("ROOT_SCENE_NODE", sizeof(RootSceneNode));
	RootSceneNode *pTitleRootSceneNode = new(s_hTitleInstance) RootSceneNode(context, arena, h);
	pTitleRootSceneNode->addDefaultComponents();
	SetTitleAsCurrent();
}

void RootSceneNode::addDefaultComponents()
{
	SceneNode::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Events::Event_GATHER_DRAWCALLS, RootSceneNode::do_GATHER_DRAWCALLS);
	PE_REGISTER_EVENT_HANDLER(Events::Event_GATHER_DRAWCALLS_Z_ONLY, RootSceneNode::do_GATHER_DRAWCALLS);
}

void RootSceneNode::do_GATHER_DRAWCALLS(Events::Event *pEvt)
{
	DrawList *pDrawList = NULL;
	bool zOnly = pEvt->isInstanceOf<Events::Event_GATHER_DRAWCALLS_Z_ONLY>();
	
	pDrawList = zOnly ? DrawList::ZOnlyInstance() : DrawList::Instance();
	
	SceneNode *pRoot = RootSceneNode::Instance();
	Events::Event_GATHER_DRAWCALLS *pDrawEvent = zOnly ? NULL : (Events::Event_GATHER_DRAWCALLS *)(pEvt);
	Events::Event_GATHER_DRAWCALLS_Z_ONLY *pZOnlyDrawEvent = zOnly ? (Events::Event_GATHER_DRAWCALLS_Z_ONLY *)(pEvt) : NULL;

	// set some effect constants here that will be constant per frame

	bool setGlobalValues = true;
	if (!zOnly && pDrawEvent->m_drawOrder != EffectDrawOrder::First)
		setGlobalValues = false;

	if (setGlobalValues)
	{
		
		// fill in the data object that will be submitted to pipeline
		Handle &h = pDrawList->nextGlobalShaderValue();
		h = Handle("RAW_DATA", sizeof(SetPerFrameConstantsShaderAction));
		SetPerFrameConstantsShaderAction *p = new(h) SetPerFrameConstantsShaderAction(*m_pContext, m_arena);
		p->m_data.gGameTimes[0] = pDrawEvent ? pDrawEvent->m_gameTime : 0;
		p->m_data.gGameTimes[1] = pDrawEvent ? pDrawEvent->m_frameTime : 0;
		

		
		#if APIABSTRACTION_D3D9

		/*Key Events for toggle of TextChoord and WindIntensity*/
		if(GetAsyncKeyState('T') & 0x8000)
		{
			if(m_toggle==0)
				m_toggle = 1;
			else
				m_toggle = 0;
		}
		

		if(GetAsyncKeyState('H') & 0x8000)
		{
			if(m_intensity <=1)
			m_intensity+=0.05;
		}
		
			if(GetAsyncKeyState('L') & 0x8000)
		{
			if(m_intensity >=0)
			m_intensity-=0.05;
		}




		//SETTING WIND VALUES & SOURCES
		Vector3 _WindEffect = Vector3(0.0f, 0.0f, 0.0f);
		float _Intensity = 100;

		//CAMERA
		Handle hCam = CameraManager::Instance()->getActiveCameraHandle();
		CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();

		Vector3 _CameraPosition = pcam->m_base.getPos();

		static float x = 0;
		x += 0.05f;

		if(m_SoldierCount > 0)
		{
			for(int i=0 ; i<m_SoldierCount ; i++)
			{
				SceneNode* _Soldier = m_SoldierSceneNodes[i];
				
				//Source of wind is the position of soldier
				if(i == 0)
				{
					
					p->m_data.windSource0[0] = _Soldier->m_base.getPos().m_x;
					p->m_data.windSource0[1] = _Soldier->m_base.getPos().m_y;
					p->m_data.windSource0[2] = _Soldier->m_base.getPos().m_z;
					p->m_data.windSource0[3] = 1.0f;
				}
				else
				{
					p->m_data.windSource1[0] = _Soldier->m_base.getPos().m_x;
					p->m_data.windSource1[1] = _Soldier->m_base.getPos().m_y;
					p->m_data.windSource1[2] = _Soldier->m_base.getPos().m_z;
					p->m_data.windSource1[3] = 1.0f;
				}
			}
		}
		else
		{
			p->m_data.windSource0[0] = 0.0f;
			p->m_data.windSource0[1] = 0.0f;
			p->m_data.windSource0[2] = 0.0f;
			p->m_data.windSource0[3] = 0.0f;
			
			
			p->m_data.windSource1[0] = 0.0f;
			p->m_data.windSource1[1] = 0.0f;
			p->m_data.windSource1[2] = 0.0f;
			p->m_data.windSource1[3] = 0.0f;
		}

		p->m_data.windSource2[0] = _CameraPosition.m_x;
		p->m_data.windSource2[1] = _CameraPosition.m_y;
		p->m_data.windSource2[2] = _CameraPosition.m_z;
		p->m_data.windSource2[3] = 1.0f;
		p->m_data.keyToggle[0] = m_toggle;
		p->m_data.keyToggle[1] = m_intensity;
		#endif
	}

	// set some effect constants here that will be constant per object group
	// NOTE at this point we have only one object group so we set it on top level per frame

	if (setGlobalValues)
	{
		Handle &hsvPerObjectGroup = pDrawList->nextGlobalShaderValue();
		hsvPerObjectGroup = Handle("RAW_DATA", sizeof(SetPerObjectGroupConstantsShaderAction));
		SetPerObjectGroupConstantsShaderAction *psvPerObjectGroup = new(hsvPerObjectGroup) SetPerObjectGroupConstantsShaderAction(*m_pContext, m_arena);
	
		psvPerObjectGroup->m_data.gViewProj = pDrawEvent ? pDrawEvent->m_projectionViewTransform : pZOnlyDrawEvent->m_projectionViewTransform;

		psvPerObjectGroup->m_data.gViewInv = pDrawEvent ? pDrawEvent->m_viewInvTransform : Matrix4x4();
		// TODO: fill these in for motion blur
		psvPerObjectGroup->m_data.gPreviousViewProjMatrix = Matrix4x4();
		psvPerObjectGroup->m_data.gViewProjInverseMatrix = Matrix4x4();

		psvPerObjectGroup->m_data.gDoMotionBlur = 0;
		psvPerObjectGroup->m_data.gEyePosW = pDrawEvent ? pDrawEvent->m_eyePos : pZOnlyDrawEvent->m_eyePos;


		#if	APIABSTRACTION_OGL 
		/*Key Events for toggle of TextChoord and WindIntensity*/
		if(GetAsyncKeyState('T') & 0x8000)
		{
			if(m_toggle==0)
				m_toggle = 1;
			else
				m_toggle = 0;
		}
		

		if(GetAsyncKeyState('H') & 0x8000)
		{
			if(m_intensity <=1)
			m_intensity+=0.05;
		}
		
			if(GetAsyncKeyState('L') & 0x8000)
		{
			if(m_intensity >=0)
			m_intensity-=0.05;
		}




		//SETTING WIND VALUES & SOURCES

			Vector3 _WindEffect = Vector3(0.0f, 0.0f, 0.0f);
		float _Intensity = 100;

		//CAMERA
		Handle hCam = CameraManager::Instance()->getActiveCameraHandle();
		CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();

		Vector3 _CameraPosition = pcam->m_base.getPos();

		static float x = 0;
		x += 0.05f;


		//SineWave
		/*float y1 = 10 + sin(x) * 10;
		float y2 = 10 + sin(x) * 10;*/
		
		if(m_SoldierCount > 0)
		{
			for(int i=0 ; i<m_SoldierCount ; i++)
			{
				SceneNode* _Soldier = m_SoldierSceneNodes[i];
				if(i == 0)
				{

					psvPerObjectGroup->m_data.windSource0[0] = _Soldier->m_base.getPos().m_x;
					psvPerObjectGroup->m_data.windSource0[1] = _Soldier->m_base.getPos().m_y;
					psvPerObjectGroup->m_data.windSource0[2] = _Soldier->m_base.getPos().m_z;
					psvPerObjectGroup->m_data.windSource0[3] = 1.0f;
				}
				else
				{
					psvPerObjectGroup->m_data.windSource1[0] = _Soldier->m_base.getPos().m_x;
					psvPerObjectGroup->m_data.windSource1[1] = _Soldier->m_base.getPos().m_y;
					psvPerObjectGroup->m_data.windSource1[2] = _Soldier->m_base.getPos().m_z;
					psvPerObjectGroup->m_data.windSource1[3] = 1.0f;
				}
			}
		}
		else
		{
			psvPerObjectGroup->m_data.windSource0[0] = 0.0f;
			psvPerObjectGroup->m_data.windSource0[1] = 0.0f;
			psvPerObjectGroup->m_data.windSource0[2] = 0.0f;
			psvPerObjectGroup->m_data.windSource0[3] = 0.0f;
			
			
			psvPerObjectGroup->m_data.windSource1[0] = 0.0f;
			psvPerObjectGroup->m_data.windSource1[1] = 0.0f;
			psvPerObjectGroup->m_data.windSource1[2] = 0.0f;
			psvPerObjectGroup->m_data.windSource1[3] = 0.0f;
		}

		psvPerObjectGroup->m_data.windSource2[0] = _CameraPosition.m_x;
		psvPerObjectGroup->m_data.windSource2[1] = _CameraPosition.m_y;
		psvPerObjectGroup->m_data.windSource2[2] = _CameraPosition.m_z;
		psvPerObjectGroup->m_data.windSource2[3] = 1.0f;

		psvPerObjectGroup->m_data.keyToggle[0] = m_toggle;
		psvPerObjectGroup->m_data.keyToggle[1] = m_intensity;

		#endif

		// the light that drops shadows is defined by a boolean isShadowCaster in maya light objects
		PrimitiveTypes::UInt32 iDestLight = 0;
		if (pRoot->m_lights.m_size)
		{
			for(PrimitiveTypes::UInt32 i=0; i<(pRoot->m_lights.m_size); i++){
				Light *pLight = pRoot->m_lights[i].getObject<Light>();
				if(pLight->castsShadow()){
					Matrix4x4 worldToView = pLight->m_worldToViewTransform;
					Matrix4x4 lightProjectionViewWorldMatrix = (pLight->m_viewToProjectedTransform * worldToView);
					psvPerObjectGroup->m_data.gLightWVP = lightProjectionViewWorldMatrix;
					
					psvPerObjectGroup->m_data.gLights[iDestLight] = pLight->m_cbuffer;
					iDestLight++;

					break;
				}
			}
		}
		for (PrimitiveTypes::UInt32 iLight = 0;iLight < pRoot->m_lights.m_size; iLight++)
		{
			Light *pLight = pRoot->m_lights[iLight].getObject<Light>();
			if(pLight->castsShadow())
				continue;
			psvPerObjectGroup->m_data.gLights[iDestLight] = pLight->m_cbuffer;
			iDestLight++;
		}
	}
}
}; // namespace Components
}; // namespace PE
