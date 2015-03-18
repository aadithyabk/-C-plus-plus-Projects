#include "Pokemon.h"
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PrimeEngine/PrimeEngineIncludes.h"
#include "../States/Idle.h"
#include "../States/Walk.h"
#include "Machoke_Walk.h"
#include "Machoke_Idle.h"
#include "Pikachu_Walk.h"
#include "Pikachu_Idle.h"
#include "Machoke_Dead.h"
#include "Machoke_Swipe.h"
#include "Machoke_follow.h"

#include "Minion_Walk.h"
#include "Minion_Idle.h"
#include "../ServerGameObjectManagerAddon.h"
#include "../CharacterControlContext.h"
using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;
namespace CharacterControl
{
	namespace Components
	{
		
		PE_IMPLEMENT_CLASS1(Pokemon , PE::Components::Component);
		Pokemon::Pokemon(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
			: Component(context, arena, hMyself)
		{
			PE::Handle hIdle("Idle", sizeof(Idle));
			idleState = new(hIdle)Idle(*m_pContext, m_arena, hIdle);
			idleState->m_parentPokemon = this;


			PE::Handle hMachokewalk("Machoke_Walk", sizeof(Machoke_Walk));
			machokeWalkState = new(hMachokewalk)Machoke_Walk(*m_pContext, m_arena, hMachokewalk);
			machokeWalkState->m_parentPokemon = this;
			

			PE::Handle hMachokeIdle("Machoke_Idle", sizeof(Machoke_Idle));
			machokeIdleState = new(hMachokeIdle)Machoke_Idle(*m_pContext, m_arena, hMachokeIdle);
			machokeIdleState->m_parentPokemon = this;

			PE::Handle hMachokeDead("Machoke_Dead", sizeof(Machoke_Dead));
			machokeDeadState = new(hMachokeDead)Machoke_Dead(*m_pContext, m_arena, hMachokeDead);
			machokeDeadState->m_parentPokemon = this;

			PE::Handle hMachokeSwipe("Machoke_Swipe", sizeof(Machoke_Swipe));
			machokeSwipeState = new(hMachokeSwipe)Machoke_Swipe(*m_pContext, m_arena, hMachokeSwipe);
			machokeSwipeState->m_parentPokemon = this;

			PE::Handle hMachokeFollow("Machoke_Follow", sizeof(Machoke_Follow));
			machokeFollowState = new(hMachokeFollow)Machoke_Follow(*m_pContext, m_arena, hMachokeFollow);
			machokeFollowState->m_parentPokemon = this;

			PE::Handle hPikachuIdle("Pikachu_Idle", sizeof(Pikachu_Idle));
			pikachuIdleState = new(hPikachuIdle)Pikachu_Idle(*m_pContext, m_arena, hPikachuIdle);
			pikachuIdleState->m_parentPokemon = this;

			PE::Handle hPikachuWalk("Pikachu_Walk", sizeof(Pikachu_Walk));
			pikachuWalkState = new(hPikachuWalk)Pikachu_Walk(*m_pContext, m_arena, hPikachuWalk);
			pikachuWalkState->m_parentPokemon = this;

			PE::Handle hMinionIdle("Minion_Idle", sizeof(Minion_Idle));
			minionIdleState = new(hMinionIdle)Minion_Idle(*m_pContext, m_arena, hMinionIdle);
			minionIdleState->m_parentPokemon = this;

			PE::Handle hMinionWalk("Minion_Walk", sizeof(Minion_Walk));
			minionWalkState = new(hMinionWalk)Minion_Walk(*m_pContext, m_arena, hMinionWalk);
			minionWalkState->m_parentPokemon = this;

			PE::Handle hFloor("FloorVol", sizeof(PE::Components::FloorVol));
			PE::Components::FloorVol *pFloor = new(hFloor) PE::Components::FloorVol(*m_pContext, m_arena, hFloor);
			pFloor->addDefaultComponents();
			m_floorInstancePokemon= pFloor;

			health = 100;
		}
		void Pokemon::addDefaultComponents()
		{
			Component::addDefaultComponents();
			PE_REGISTER_EVENT_HANDLER(PE::Events::Event_UPDATE, do_UPDATE);
		}
		void Pokemon::ChangeState(m_states nextState)
		{
			m_currentState->OnExit();

			currentEnumState = nextState;
			if (currentEnumState == WALK)
				m_currentState = machokeWalkState;
			
			if (currentEnumState == MACHOKE_WALK)
				m_currentState = machokeWalkState;

			if (currentEnumState == MACHOKE_IDLE)
				m_currentState = machokeIdleState;

			if (currentEnumState == MACHOKE_DEAD)
				m_currentState = machokeDeadState;

			if (currentEnumState == MACHOKE_SWIPE)
				m_currentState = machokeSwipeState;

			if (currentEnumState == MACHOKE_FOLLOW)
				m_currentState = machokeFollowState;

			if (currentEnumState == PIKACHU_WALK)
				m_currentState = pikachuWalkState;

			if (currentEnumState == PIKACHU_IDLE)
				m_currentState = pikachuIdleState;

			if (currentEnumState == MINION_WALK)
				m_currentState = minionWalkState;

			if (currentEnumState == MINION_IDLE)
				m_currentState = minionIdleState;

			m_currentState->OnEnter();
		}

		void Pokemon::do_UPDATE(PE::Events::Event *pEvt)
		{
			m_currentState->Update(pEvt);
		}

		void Pokemon::UpdateAttribute(int health)
		{
			this->health = health;
		}

		void Pokemon::SetStateFirstTime(m_states startState)
		{
			currentEnumState = startState;
			if (currentEnumState == WALK)
				m_currentState = machokeWalkState;

			if (currentEnumState == MACHOKE_WALK)
				m_currentState = machokeWalkState;

			if (currentEnumState == MACHOKE_IDLE)
				m_currentState = machokeIdleState;

			if (currentEnumState == MACHOKE_DEAD)
				m_currentState = machokeDeadState;

			if (currentEnumState == MACHOKE_SWIPE)
				m_currentState = machokeSwipeState;

			if (currentEnumState == MACHOKE_FOLLOW)
				m_currentState = machokeFollowState;

			if (currentEnumState == PIKACHU_WALK)
				m_currentState = pikachuWalkState;

			if (currentEnumState == PIKACHU_IDLE)
				m_currentState = pikachuIdleState;

			if (currentEnumState == MINION_WALK)
				m_currentState = minionWalkState;

			if (currentEnumState == MINION_IDLE)
				m_currentState = minionIdleState;

			m_currentState->OnEnter();
		}

		void Pokemon::UpdateAttribute(Vector3 position, Vector3 direction)
		{
			SceneNode* pSN = this->getFirstComponent<SceneNode>();
			pSN->m_base.turnInDirection(direction, 3.1415f);
			pSN->m_base.setPos(position);
		}
	}
}