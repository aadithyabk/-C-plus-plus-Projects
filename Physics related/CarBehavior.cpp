#include "CarBehavior.h"
#include "PrimeEngine/PrimeEngineIncludes.h"
#include "ClientGameObjectManagerAddon.h"
#include "CharacterControlContext.h"
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;

static float finalRotation  = 0;
static	float forwardSpeed1;
static		float reverseSpeed1;
static float acceleration1 = 0.05f;
static float maxSpeed1 = 3.0f;
static float accleratedDuration1;
static float maxDuration1 = 0.12f;
float curSpeed1= 0.0f;
float previousTime1= 0.0f;
static float Debug_Fly_Speed = 6.0f;
float new_time1= 0.0f;
int inAir = 0;
int turnCount = 0;
namespace PE
{

	namespace Components
	{

		PE_IMPLEMENT_CLASS1(CarBehavior, Component);

		CarBehavior::CarBehavior(PE::GameContext &context, PE::MemoryArena arena, PE::Handle hMyself)
			: Component(context,arena,hMyself)
		{}


		void CarBehavior::addDefaultComponents()
		{
			Component::addDefaultComponents();
			PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, CarBehavior::do_UPDATE);
		}

		void CarBehavior::do_calculateResultant(int carNum)
		{
			SceneNode *pSN_body;
			SceneNode *pSN_FLT;
			SceneNode *pSN_FRT;
			SceneNode *pSN_RRT;
			SceneNode *pSN_RLT;
			SceneNode *m_SceneNodes[10];
			Handle h[5];
			Vector3 forceResultant = Vector3(0,0,0);
			if(carNum == 1) //FirstCar
			{
				pSN_body = hSN_body.getObject<SceneNode>(); //Scene node of the body
				pSN_FLT = hSN_FLT.getObject<SceneNode>();   //Scene node of the Front Left tyre
				pSN_FRT = hSN_FRT.getObject<SceneNode>();   //Scene node of the Front Right tyre
				pSN_RLT = hSN_RLT.getObject<SceneNode>();   //Scene node of the Rear left tyre
				pSN_RRT = hSN_RRT.getObject<SceneNode>();   //Scene node of the Rear right tyre
				h[0] = hSN_body;
				h[1] = hSN_FLT;
				h[2] = hSN_FRT;
				h[3] = hSN_RLT;
				h[4] = hSN_RRT;
			}
			else //Second Car
			{
				pSN_body = hSN_body_1.getObject<SceneNode>();
				pSN_FLT = hSN_FLT_1.getObject<SceneNode>();
				pSN_FRT = hSN_FRT_1.getObject<SceneNode>();
				pSN_RLT = hSN_RLT_1.getObject<SceneNode>();
				pSN_RRT = hSN_RRT_1.getObject<SceneNode>();

				h[0] = hSN_body_1;
				h[1] = hSN_FLT_1;
				h[2] = hSN_FRT_1;
				h[3] = hSN_RLT_1;
				h[4] = hSN_RRT_1;
			}


			m_SceneNodes[0] = pSN_body;
			m_SceneNodes[1] = pSN_FLT;
			m_SceneNodes[2] = pSN_RLT;
			m_SceneNodes[3] = pSN_FRT;
			m_SceneNodes[4] = pSN_RRT;

			for(int i = 0; i < 5; i++)
			{
				//Calculate the new position of each tyre depending on the forces acting on it
				m_SceneNodes[i]->m_Physics->AddForces(h[i]); 
			}
			
			Vector3 _AngleAlongRamp = pSN_FLT->m_Physics->NewPosition - pSN_RLT->m_Physics->NewPosition;

			if(pSN_FLT->m_Physics->m_state == Physics::ONRAMP)
			{			
				Vector3 forward  = pSN_body->m_worldTransform.getN();
				forward.m_x = 0;
				Vector3 direction = _AngleAlongRamp;/
				direction.m_x = 0;

				forward.m_x = 0;
				forward.normalize();

				float angle = acos((forward.dotProduct(direction))/(forward.length() * direction.length()));
				float _Angle = angle * (3.14 /180);
				
				////-ve turn down
				////+VE turn up 
				
				pSN_body->m_base.turnUp(_Angle);
			
				
			}
		}
	}
}
