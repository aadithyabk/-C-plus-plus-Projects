#include "States.h"
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"
#include "PrimeEngine/PrimeEngineIncludes.h"
using namespace PE;
using namespace PE::Components;
using namespace CharacterControl::Events;
using namespace PE::Events;
namespace CharacterControl
{
	namespace Components
	{
		PE_IMPLEMENT_CLASS1(States, PE::Components::Component);
		States::States(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
			: Component(context, arena, hMyself)
		{

		}
		void States::addDefaultComponents()
		{
			Component::addDefaultComponents();
			
		}
		void States::OnEnter()
		{

		}

		void States::OnExit()
		{

		}
		void States::Update(PE::Events::Event *pEvt)
		{

		}
	}
}