#include "Machoke.h"
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Scene/SkeletonInstance.h"
#include "PrimeEngine/Scene/MeshInstance.h"
#include "PrimeEngine/Scene/RootSceneNode.h"
#include "MachokeAnimationSM.h"
#include "PrimeEngine/PrimeEngineIncludes.h"
#include "../ServerGameObjectManagerAddon.h"
#include "../CharacterControlContext.h"
#include "../States/Idle.h"
#include "../States/Walk.h"
#include "Machoke_Walk.h";
#include "Machoke_Idle.h";
#include "Machoke_Dead.h"
#include "Machoke_Swipe.h"
#include "Machoke_follow.h"

using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;
namespace CharacterControl
{
	namespace Components
	{
		PE_IMPLEMENT_CLASS1(Machoke, PE::Components::Component);
		Machoke::Machoke(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself,Vector3 pos)
			: Pokemon(context,  arena,  hMyself)
		{
			m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

			PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
			SceneNode *pMainSN = new(hSN)SceneNode(*m_pContext, m_arena, hSN);
			pMainSN->addDefaultComponents();

			pMainSN->m_base.setPos(pos);

			RootSceneNode::Instance()->addComponent(hSN);

			// add the scene node as component of Machoke without any handlers. this is just data driven way to locate scnenode for Machoke's components
			{
				static int allowedEvts[] = { 0 };
				addComponent(hSN, &allowedEvts[0]);
			}

			BoxComponent box(Vector3(-1.5f, 0.f, -1.25f), Vector3(1.5f, 3.f, 1.25f), "Soldier"); /
			Handle h("PhysicsComponent", sizeof(PhysicsComponent));
			pPC = new (h)PhysicsComponent(*m_pContext, m_arena, h);
			pPC->m_Comp = this;
			pPC->Initialize(pMainSN, box, true);

			int numskins = 1;
			for (int iSkin = 0; iSkin < numskins; ++iSkin)
			{
				float z = (iSkin / 4) * 1.5f;
				float x = (iSkin % 4) * 1.5f;
				PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
				SceneNode *pSN = new(hSN)SceneNode(*m_pContext, m_arena, hSN);
				pSN->addDefaultComponents();

				pSN->m_base.setPos(Vector3(x, 0, z));
				pSN->m_base.turnAboutAxis(3.142f, Vector3(0, 1, 0));															// Rotation scene node to rotate Machoke properly, since Machoke from Maya is facing wrong direction
				
				PE::Handle hRotateSN("SCENE_NODE", sizeof(SceneNode));
				SceneNode *pRotateSN = new(hRotateSN)SceneNode(*m_pContext, m_arena, hRotateSN);
				pRotateSN->addDefaultComponents();

				pSN->addComponent(hRotateSN);

				pRotateSN->m_base.turnLeft(3.1415);

				PE::Handle hMachokeAnimSM("MachokeAnimationSM", sizeof(MachokeAnimationSM));
				pMachokeAnimSM = new(hMachokeAnimSM)MachokeAnimationSM(*m_pContext, m_arena, hMachokeAnimSM);
				pMachokeAnimSM->addDefaultComponents();

				pMachokeAnimSM->m_debugAnimIdOffset = 0;

				PE::Handle hSkeletonInstance("SkeletonInstance", sizeof(SkeletonInstance));
				SkeletonInstance *pSkelInst = new(hSkeletonInstance)SkeletonInstance(*m_pContext, m_arena, hSkeletonInstance,
					hMachokeAnimSM);
				pSkelInst->addDefaultComponents();

				
				pSkelInst->initFromFiles("Mutant_Mutant__Hips.skela", "Mutant", m_pContext->m_gameThreadThreadOwnershipMask);   //Load the skeleton
	
				pSkelInst->setAnimSet("Mutant_Idle_Mutant__Hips.animseta", "Mutant");											//Load the animation set
	
				PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
				MeshInstance *pMeshInstance = new(hMeshInstance)MeshInstance(*m_pContext, m_arena, hMeshInstance);
				pMeshInstance->addDefaultComponents();

				pMeshInstance->initFromFile("MutantMesh.mesha", "Mutant", m_pContext->m_gameThreadThreadOwnershipMask);			//Load the mesh

				pSkelInst->addComponent(hMeshInstance);

				// add skin to scene node
				pRotateSN->addComponent(hSkeletonInstance);

				pMainSN->addComponent(hSN);																						//Add this component to the scene

				currentEnumState = MACHOKE_IDLE;																				//Set the initial state for this particular pokemon
		
				SetStateFirstTime(currentEnumState);																			//Set the reference for the current state
				
			}

			m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
		}
		void Machoke::addDefaultComponents()
		{
			//Component::addDefaultComponents();
			Pokemon::addDefaultComponents();
		}
		
	}

	
}