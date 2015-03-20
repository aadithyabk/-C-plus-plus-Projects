#ifndef _STATES_
#define _STATES_

#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "../Events/Events.h"

namespace CharacterControl{

	namespace Components {

		class Pokemon;
		struct States : public PE::Components::Component
		{
			PE_DECLARE_CLASS(States);

			States(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself);

			Pokemon *m_parentPokemon;
			PE::Handle m_hParentPokemonAnimationHandle;

			virtual void addDefaultComponents();

			virtual void OnEnter();															//Overridden by each state depending on what it wants to do

			virtual void OnExit();															//Overridden by each state depending on what is wants to do

			virtual void Update(PE::Events::Event *pEvt);									//The current state is continuously updated
		};
	}; // namespace Components
};
#endif
