
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
			//on entering this state, begin walking aniatmion
			PE::Handle h("MachokeAnimSM_Event_WALK", sizeof(MachokeAnimSM_Event_WALK));
			Events::MachokeAnimSM_Event_WALK *pOutEvt = new(h) MachokeAnimSM_Event_WALK();
			m_parentPokemon->getFirstComponent<PE::Components::SceneNode>()->handleEvent(pOutEvt);
			// release memory now that event is processed
			h.release();


		}

		void Machoke_Walk::OnExit()
		{
			//nothing to be done on OnExit
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
			//when the character is in walk state, it will be called every frame from the do_Update() of Pokemon class
			//As long as it is in walk state, walk using AI to that particular destination.
			if (m_parentPokemon->m_networkView->IsOwner)
			{

				SceneNode *pSN = m_parentPokemon->getFirstComponent<SceneNode>();
				Vector3 currentPos = pSN->m_base.getPos();
				currentPos.m_x += 0.0001;
				pSN->m_base.setPos(currentPos);

				CharacterControl::Components::ClientGameObjectManagerAddon *pGameObjectManagerAddon = (CharacterControl::Components::ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
				PE::Components::FloorVol * pFloor = m_parentPokemon->m_floorInstancePokemon;;
				
				int startID, endID;
				
				Vector3 clickPos = Vector3(5, 0, 10);
				endID = pFloor->CalculateIDs(clickPos);
				startID = pFloor->CalculateIDs(currentPos);
					
				pFloor->setIsWalkable();
				pFloor->AStarCopy(startID, endID);
			
				m_targetPosition = pFloor->funnelAlgoCopy(currentPos, clickPos);
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
					if(dsqr<=0)
					{
						dist = 0;
					}
					else
						dist = sqrt(dsqr);
			
					if (dist > allowedDisp)
					{
						dist = allowedDisp; // can move up to allowedDisp
						reached = false; // not reaching destination yet
					}
			
					if(dir.lengthSqr() != 0)
						pSN->m_base.turnInDirection(dir, 3.1415f);
						
					pSN->m_base.setPos(currentPos + dir * dist);
				}
				else
				{
					//On reaching the destination, change state to idle.
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
				//Continuously update this character's positon on all other clients.
				//Event_UpdateAttributes tells the other clients to update the position of this ghost 
				//in their game instance
				
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
