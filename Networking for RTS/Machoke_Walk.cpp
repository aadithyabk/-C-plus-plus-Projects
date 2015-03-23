#include "Machoke_Walk.h"
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PrimeEngine/PrimeEngineIncludes.h"
#include "Pokemon.h"
#include "MachokeAnimationSM.h"
#include "Machoke.h"
#include "../ClientGameObjectManagerAddon.h"
#include "../CharacterControlContext.h"
#include "PrimeEngine/Scene/Floor.h"
#include "../NavMeshManager.h"

using namespace PE::Events;
using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;
namespace CharacterControl
{
	namespace Components
	{
		PE_IMPLEMENT_CLASS1(Machoke_Walk, PE::Components::Component);
		Machoke_Walk::Machoke_Walk(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
			: States(context, arena, hMyself)
		{

		}
		void Machoke_Walk::addDefaultComponents()
		{
			Component::addDefaultComponents();
		}
		void Machoke_Walk::OnEnter()
		{
			PEINFO("Entering Walk");
			PE::Handle h("MachokeAnimSM_Event_WALK", sizeof(MachokeAnimSM_Event_WALK));
			Events::MachokeAnimSM_Event_WALK *pOutEvt = new(h) MachokeAnimSM_Event_WALK();
			m_parentPokemon->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);
			// release memory now that event is processed
			h.release();


		}

		void Machoke_Walk::OnExit()
		{

		}

		bool Machoke_Walk::Approximate(float a, float b)
		{
			if (fabs(a - b) < 0.0001)
			{
				return true;
			}
			return false;
		}

		void Machoke_Walk::Update(PE::Events::Event *pEvt)
		{
			
			////TODO - Get position from mouse click
			if (m_parentPokemon->m_networkView->IsOwner)
			{

				PEINFO("Inside Walk's Update");
				SceneNode *pSN = m_parentPokemon->getFirstComponent<SceneNode>();
				Vector3 currentPos = pSN->m_base.getPos();
				currentPos.m_x += 0.0001;
				pSN->m_base.setPos(currentPos);

			
				PEINFO("Entering Walk");
				Vector3 tankPos;
				CharacterControl::Components::ClientGameObjectManagerAddon *pGameObjectManagerAddon = (CharacterControl::Components::ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
							
			

				pGameObjectManagerAddon->getTank(&tankPos);
				//tankPos = Vector3(10,0,10);
				

				PE::Components::FloorVol * pFloor = m_parentPokemon->m_floorInstancePokemon;;
				int startID, endID;
				//endID = pFloor->CalculateIDs(tankPos);
				Vector3 clickPos = Vector3(5, 0, 10);
				endID = pFloor->CalculateIDs(clickPos);
				startID = pFloor->CalculateIDs(currentPos);
							
							

				pFloor->setIsWalkable();

				pFloor->AStarCopy(startID, endID);
				Matrix4x4 base =  pSN->m_base;
				Vector3 t[8];
				Vector3 t0,t1,t2;
				for(int i = 0 ; i< pFloor->comeFrom.m_size ; i++){
						t0 = pFloor->polygonList[pFloor->comeFrom[i]]->Vertices[0];
						t1 = pFloor->polygonList[pFloor->comeFrom[i]]->Vertices[1];
						t2 = pFloor->polygonList[pFloor->comeFrom[i]]->Vertices[2];

				Vector3 color1 = Vector3(1,1,1);
						t[0]=t0;	t[1]=color1;
						t[2]=t1;	t[3]=color1;
						t[4]=t2;	t[5]=color1;
						t[6]=t0;	t[7]=color1;
						DebugRenderer::Instance()->createLineMesh(true, base,  &t[0].m_x, 4, 1);
				}
	
				m_targetPosition = pFloor->funnelAlgoCopy(currentPos, clickPos);
				//	pSN->m_base.setPos(m_targetPosition);
				//	PEINFO("Target Positiion: %f , %f, %f", m_targetPosition.m_x, m_targetPosition.m_y, m_targetPosition.m_z);
	
				//Vector3 curPos = pSN->m_base.getPos();
		
				float dsqr = (m_targetPosition - currentPos).lengthSqr();
				float tankd = (clickPos - currentPos).lengthSqr();
			

				bool reached = true;
		
				if (!Approximate(dsqr,0.0f) || !(Approximate(tankd,0.0f)))
				{
					// not at the spot yet
				
					static float speed = 1.0f;
					Event_UPDATE *pRealEvt = (Event_UPDATE *)(pEvt);
					float allowedDisp = speed * pRealEvt->m_frameTime;

					Vector3 dir = (m_targetPosition - currentPos);
					if(dir.lengthSqr() != 0)
						dir.normalize();
					float dist;
					if(dsqr<=0){
						dist = 0;
					
					}else
						dist = sqrt(dsqr);
					if (dist > allowedDisp)
					{
						dist = allowedDisp; // can move up to allowedDisp
						reached = false; // not reaching destination yet
					}
			
					//if (dir.lengthSqr() != 0)
					//	pSN->m_base.turnTo(dir);
					// instantaneous turn
					if(dir.lengthSqr() != 0)
						pSN->m_base.turnInDirection(dir, 3.1415f);
					pSN->m_base.setPos(currentPos + dir * dist);
				}
				else
				{
					m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
					DebugRenderer::Instance()->createTextMesh("_", true, false, false, false, 0, 
							Vector3(0.5f, 0.5f, 0), 5.0f, m_pContext->m_gameThreadThreadOwnershipMask);
					m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
				
					Event_ChangeState pEvt(*m_pContext);
					pEvt.changeThisGhost = m_parentPokemon->m_networkView->m_ghostID;
					pEvt.nextState = CharacterControl::m_states::MACHOKE_IDLE;
					ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
					pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&pEvt, m_pContext->getGameObjectManager(), true, false);
			
				}
				CharacterControl::Events::Event_UpdateAttribute pEvt(*m_pContext);
				pEvt.changeThisGhost = m_parentPokemon->m_networkView->m_ghostID;
				pEvt.attributeToBeChanged = POSITION;
				pEvt.position = pSN->m_base.getPos();
				pEvt.direction = pSN->m_base.getN();
				ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
				pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&pEvt, m_pContext->getGameObjectManager(), true, false);

			}
		}
	}
}