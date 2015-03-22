#ifndef _MACHOKE_
#define _MACHOKE_
#include "Pokemon.h"
#include "PrimeEngine/Events/Component.h"
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/Events/Component.h"
#include "../Events/Events.h"
#include "PrimeEngine/Physics/PhysicsComponent.h"


namespace CharacterControl{

	namespace Components {

		class States;
		class Idle;
		class Walk;
		class Machoke_Dead;
		class Machoke_Walk;
		class Machoke_Swipe;
		class Machoke_Follow;
		class MachokeAnimationSM;
		struct Machoke : public Pokemon
		{
			PE_DECLARE_CLASS(Machoke);

			Machoke(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself,Vector3 pos);

			virtual void addDefaultComponents();
			
			Idle *idleState;
			Machoke_Walk *walkState;
			Machoke_Dead *deadState;
			Machoke_Swipe *swipeState;
			Machoke_Follow *followState;

			MachokeAnimationSM *pMachokeAnimSM;
			PE::Components::PhysicsComponent* pPC;
			
		};
	}; // namespace Components
};
#endif
