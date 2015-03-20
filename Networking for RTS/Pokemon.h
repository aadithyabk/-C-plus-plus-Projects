#ifndef _POKEMON_
#define _POKEMON_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/Events/Component.h"
#include "../Events/Events.h"
#include "PrimeEngine/Scene/Floor.h"


namespace CharacterControl{

	namespace Components {
		//Forward Declaration to avoid circular includes
		class Idle;
		class Walk;
		class States;
		class Machoke_Walk;
		class Machoke_Idle;
		class Machoke_Dead;
		class Machoke_Swipe;
		class Machoke_Follow;

		class Pikachu_Walk;
		class Pikachu_Idle;

		class Minion_Walk;
		class Minion_Idle;
		class NetworkView;
		struct Pokemon : public PE::Components::Component
		{
			PE_DECLARE_CLASS(Pokemon);
			
			Pokemon(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);

			virtual void addDefaultComponents();

			CharacterCodes pokemonType;

			NetworkView *m_networkView; 								//Each pokemon has a view of the network

			m_states currentEnumState;									//Indicate the current state of each pokemon
			States *m_currentState;										//Reference to the current state of each pokemon
			Idle *idleState;
			Machoke_Walk *machokeWalkState;
			Machoke_Idle *machokeIdleState;
			Machoke_Dead *machokeDeadState;
			Machoke_Swipe *machokeSwipeState;
			Machoke_Follow *machokeFollowState;

			Pikachu_Walk *pikachuWalkState;
			Pikachu_Idle *pikachuIdleState;

			Minion_Walk *minionWalkState;
			Minion_Idle *minionIdleState;

			int health;
		    
			void ChangeState(CharacterControl::m_states nextState);
			
			void UpdateAttribute(int health);						    //Update Health			
		    void UpdateAttribute(Vector3 position, Vector3 direction);	//Update Position and direction

			void SetStateFirstTime(m_states startState);

			PE_DECLARE_IMPLEMENT_EVENT_HANDLER_WRAPPER(do_UPDATE)
			virtual void do_UPDATE(PE::Events::Event *pEvt);

			PE::Components::FloorVol * m_floorInstancePokemon;
		};
	}; // namespace Components
};
#endif
