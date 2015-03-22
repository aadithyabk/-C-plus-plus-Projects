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
namespace PE{

	namespace Components{

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


			//Calculate force resultant on all four tyres and apply to the center of the body
			for( int i =0; i < pSN_FLT->m_Physics->m_ForceList.m_size; i++)
			{
				forceResultant.m_x += pSN_FLT->m_Physics->m_ForceList[i].m_x;
				forceResultant.m_y += pSN_FLT->m_Physics->m_ForceList[i].m_y;
				forceResultant.m_z += pSN_FLT->m_Physics->m_ForceList[i].m_z;

			}
			
			for( int i =0; i < pSN_FRT->m_Physics->m_ForceList.m_size; i++)
			{
			forceResultant.m_x += pSN_FRT->m_Physics->m_ForceList[i].m_x;
			forceResultant.m_y += pSN_FRT->m_Physics->m_ForceList[i].m_y;
			forceResultant.m_z += pSN_FRT->m_Physics->m_ForceList[i].m_z;

			}

			for( int i =0; i < pSN_RLT->m_Physics->m_ForceList.m_size; i++)
			{
			forceResultant.m_x += pSN_RLT->m_Physics->m_ForceList[i].m_x;
			forceResultant.m_y += pSN_RLT->m_Physics->m_ForceList[i].m_y;
			forceResultant.m_z += pSN_RLT->m_Physics->m_ForceList[i].m_z;

			}
			for( int i =0; i < pSN_RRT->m_Physics->m_ForceList.m_size; i++)
			{
			forceResultant.m_x += pSN_RRT->m_Physics->m_ForceList[i].m_x;
			forceResultant.m_y += pSN_RRT->m_Physics->m_ForceList[i].m_y;
			forceResultant.m_z += pSN_RRT->m_Physics->m_ForceList[i].m_z;

			}

			for(int i = 0; i < 5; i++)
			{
				m_SceneNodes[i]->m_Physics->AddForces(h[i]); //Add forces to the body
			}

			

			float _Diff = pSN_FLT->m_Physics->NewPosition.m_y - pSN_RLT->m_Physics->NewPosition.m_y;
			

			//float finalY = pSN_RLT->m_Physics->NewPosition.m_y + (_Diff);
			Vector3 _Initial = pSN_body->m_base.getPos();
			float finalY = pSN_RLT->m_Physics->NewPosition.m_y + (_Diff)/2;

			forceResultant += _Initial;


			Vector3 _AngleAlongRamp = pSN_FLT->m_Physics->NewPosition - pSN_RLT->m_Physics->NewPosition;


			
					else 
							inAir=0;
					new_time1 = (clock()- previousTime1)/10000;
					//pSN_FLT->m_base.setPos(pSN_FLT->m_base.getPos() + forceResultant);
					//Vector3 finalPos = pSN_body->m_base.getPos() + new_time1 * (forceResultant - pSN_body->m_base.getPos());
					static float yPrev = 0;
					static float yNew = 0;
					yPrev = yNew;
					//PEINFO("Force Resultant is %f %f %f",forceResultant.m_x, forceResultant.m_y, forceResultant.m_z);
					yNew = forceResultant.m_y;
						if(pSN_body->m_base.getPos().m_y<-7)
						{
							pSN_body->m_base.setPos(Vector3 (-10 , 0 , 70));
							m_velocity = 0.0f;
						}
						else
					pSN_body->m_base.setPos(forceResultant);

					Vector3 N = pSN_body->m_base.getN();
					//pSN_body->m_base.turnUp(-0.01);


					if(pSN_FLT->m_Physics->m_state == Physics::ONRAMP)
					{

						if(turnCount <10)
						{
							pSN_body->m_base.turnUp(3.145/40);

							turnCount ++;

						}			
						Vector3 forward  = pSN_FLT->m_worldTransform.getN();
						forward.m_x = 0;
						Vector3 direction = _AngleAlongRamp;//pSN_FLT->m_worldTransform.getPos() - pSN_RLT->m_worldTransform.getPos();
						direction.m_x = 0;

						forward.m_x = 0;
						forward.normalize();

						float angle = acos((forward.dotProduct(direction))/(forward.length() * direction.length()));
						float _Angle = angle * (3.14 /180);

						if(_Angle != 0)
							_Angle = -_Angle;

						Vector3 angleSign = forward.crossProduct(direction);
						int _Sign = 1;
						if(angleSign.m_x > 0)
							_Sign = -1;
						_Angle *= (float)_Sign;

						_Angle *= (angleSign.m_x/fabs(angleSign.m_x));
						
						////-ve turn down
						////+VE turn up 
						if(_Angle < 0)
						{
							pSN_body->m_base.turnUp(_Angle);
							finalRotation= angle;
							
						}
						else if(_Angle < 0)
						{
							pSN_body->m_base.turnUp(_Angle);
						}
					}
					else
						turnCount = 0;
			
					pSN_FLT->m_Physics->m_ForceList.clear();
					pSN_FRT->m_Physics->m_ForceList.clear();
					pSN_RLT->m_Physics->m_ForceList.clear();
					pSN_RRT->m_Physics->m_ForceList.clear();
				
			
				}
		}
	}