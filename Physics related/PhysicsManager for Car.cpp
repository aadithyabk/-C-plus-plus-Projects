#define NOMINMAX

#include "PhysicsManager.h"
#include "Physics.h"
#include <time.h>

#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "PrimeEngine/Render/ShaderActions/SetPerFrameConstantsShaderAction.h"
#include "PrimeEngine/Scene/DebugRenderer.h"
#include "../CharacterControl/CharacterControlIncludes.h"
#include "PrimeEngine/Events/Event.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "CarBehavior.h"

namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(PhysicsManager, Component);

Handle PhysicsManager::s_hInstance;
float new_time = 0.0f;
float previousTime = 0.0f;
float	gravity = 9.8f;
float	Cdrag = 0.4297f;
float	Crr = 12.8f;
float	mass = 500.0f;
int		radius = 1.0;
float factor = 1.001;
PhysicsManager::PhysicsManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	, m_SceneNodes(context, arena, 16)
{
	
}

void PhysicsManager::Construct(PE::GameContext &context, PE::MemoryArena arena)
{
	
	Handle h("PHYSICSMANAGER", sizeof(PhysicsManager));
	PhysicsManager *pPhysicsManager = new(h) PhysicsManager(context, arena, h);
	pPhysicsManager->addDefaultComponents();
	s_hInstance = h;
}

void PhysicsManager::addDefaultComponents()
{
	Component::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Events::Event_PHYSICS_START, PhysicsManager::do_PRE_UPDATE);
	PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, PhysicsManager::do_UPDATE);
	PE_REGISTER_EVENT_HANDLER(Events::Event_PHYSICS_END, PhysicsManager::do_POST_UPDATE);
} 

void PhysicsManager::do_PRE_UPDATE(Events::Event *pEvt)
{
	
	//ADDING GRAVITY
	for(int i=0 ; i<m_SceneNodes.m_size ; i++)
	{

		if(m_SceneNodes[i].getObject<SceneNode>()->m_Physics->m_IsKinematic == false)
		{
			Handle h = m_SceneNodes[i];
			if(strcmp(h.getDbgName(),"Car_SCENE_NODE")==0)
				continue;
			Vector3 _CurrentPos = m_SceneNodes[i].getObject<SceneNode>()->m_base.getPos();
			//_CurrentPos.m_y -= 0.3f;
			SceneNode *_SN = m_SceneNodes[i].getObject<SceneNode>();
			
			if(strcmp(m_SceneNodes[i].getDbgName(),"SCENE_NODE_FrontLeft_Tyre") == 0 || strcmp(m_SceneNodes[i].getDbgName(),"Car_SCENE_NODE_FRT") == 0)
				m_SceneNodes[i].getObject<SceneNode>()->m_Physics->AddForce(Vector3(0,-0.3 * factor,0), m_SceneNodes[i]);
			else
				m_SceneNodes[i].getObject<SceneNode>()->m_Physics->AddForce(Vector3(0,-0.3,0), m_SceneNodes[i]);//m_ForceList.add(Vector3(0,-0.3,0));
		
			
		}
	}

}

void PhysicsManager::do_UPDATE(Events::Event *pEvt)
{
	static bool collided = false;
	int static _FirstTime = 0;

	Vector3 finalPos;

	 
	 for(int i=0 ; i<m_SceneNodes.m_size ;i++)
	 {
		  Handle hTemp = m_SceneNodes[i];
		  SceneNode *_SN1 = m_SceneNodes[i].getObject<SceneNode>();
		  const char* name = hTemp.getDbgName();
		  
		  
		  if(strcmp(name,"Car_SCENE_NODE")==0)
					continue;

		  //Box component
		  if(_SN1->m_Physics->m_shape == 0)
		  {
			  
			   for(int j=0;j<m_SceneNodes.m_size;j++)
			   {
				   if (i == j) continue;
					SceneNode *_SN2 = m_SceneNodes[j].getObject<SceneNode>();

					//Sphere Component
					if(_SN2->m_Physics->m_shape == 1)
					{
						    Handle h = m_SceneNodes[j];
						    const char* name1 = h.getDbgName();
						  
						    Vector3 pos = _SN2->m_base.getPos();
						    AxisAlignedBB _BoundingBox = _SN2->m_Physics->m_BoundingBox;
						    _BoundingBox.ConvertToWorldCoordinates(&(_SN1->m_worldTransform));

						 
							Matrix4x4 local = _SN2->m_worldTransform.inverse();
							Vector3 localPos = local * _SN1->m_worldTransform.getPos();
							 
							Vector3 point = SphereInPlane (localPos, _BoundingBox ,radius);

							Vector3 CenterToPoint = point - localPos;
							float length =  CenterToPoint.length();
							Vector3 direction = _SN2->m_worldTransform * CenterToPoint - _SN1->m_base.getPos();
							Vector3 _NewPos = _SN1->m_base.getPos();
							  
							
							if(length < radius && _SN2->IsWall == true)
							{
								if(_SN1->m_Physics->carNum == 1)
									{
										_SN1->m_Physics->AddForce(Vector3(0,0,_SN1->pCar->m_velocity*100), m_SceneNodes[i]);
										
									}

							}

							   
							if(length < radius && _SN2->IsRamp == false)
							{
								//Negate gravity
								//Gravity acting on front tyres is more than the rear tyres

								if(strcmp(hTemp.getDbgName(),"SCENE_NODE_FrontLeft_Tyre") == 0 || strcmp(hTemp.getDbgName(),"Car_SCENE_NODE_FRT") == 0)
									_SN1->m_Physics->AddForce(Vector3(0,0.3 * factor,0), m_SceneNodes[i]);
								else
									_SN1->m_Physics->AddForce(Vector3(0,0.3,0),hTemp);//m_ForceList.add(Vector3(0,0.3,0));
								
						
							}
							
							if(strcmp(hTemp.getDbgName(),"SCENE_NODE_FrontLeft_Tyre")==0)
								if(strcmp(h.getDbgName(),"RAMP_SCENE_NODE")==0)
									if(_SN1->m_Physics->carNum == 1)
									{
										
										Vector3 sphereCenter = _SN2->m_worldTransform.inverse() * _SN1->m_worldTransform.getPos();
										_SN1->pCar->DistanceFromRamp = abs(_SN2->m_base.getPos().m_y - sphereCenter.m_y);
										_SN1->pCar->m_maxDist = _SN2->m_Physics->m_BoundingBox.m_MaxY;
										
									}
							
							if (length < radius && _SN2->IsRamp == true)
							{
								//Resolve collision with ramp


								Vector3 collidedPoint = _SN2->m_worldTransform * point;
								Vector3 NewPoint = Vector3(collidedPoint.m_x, collidedPoint.m_y, collidedPoint.m_z);
								Vector3 Normal = NewPoint - collidedPoint;

								_SN1->requiredPoint = _SN2->requiredPoint;



								Normal.normalize();
								Normal = Vector3(0, 1, 1);
								Vector3 destination = Normal * 0.65;


								_SN1->m_Physics->AddForce(Vector3(destination.m_x, destination.m_y, destination.m_z), hTemp);//m_ForceList.add(Vector3(destination.m_x,destination.m_y ,destination.m_z));
								//_SN1->m_Physics->AddForce(Vector3(0,1.0,0),hTemp);//m_ForceList.add(Vector3(0,1.0,0));
							}
					}
							
				
					else
					{
						//Collision between cars

						if(_SN2->m_Physics->m_shape == 0 && _SN1->m_Physics->carNum != _SN2->m_Physics->carNum)
						{
							Handle h1 = m_SceneNodes[i];
							Handle h2 = m_SceneNodes[j];
							Vector3 centerTocenter = (_SN1->m_worldTransform.getPos())- (_SN2->m_worldTransform.getPos());
							float centerDistance = centerTocenter.length();
							if(centerTocenter.length() < (radius + radius))
							{
								if(!collided)
								{
									float Velocity1 = _SN1->m_Physics->m_Velocity;
									float Velocity2 = _SN2->m_Physics->m_Velocity;
									if(Velocity1 > Velocity2)
									{
										Vector3 direction = -centerTocenter;
										_SN2->m_Physics->m_ForceList.add(direction * 0.5);	
									}
									else
									{
										Vector3 direction = -centerTocenter;
										_SN2->m_Physics->m_ForceList.add(direction * 10);	
									}
								}
							}
						}
					}
				}
		  }
	 }
	
	 for(int k =0; k < m_SceneNodes.m_size; k++)
	  {
		   SceneNode *_SN = m_SceneNodes[k].getObject<SceneNode>();
		  if(_SN->IsCar == 1)
			   if(_SN->m_Physics->carNum == 1)
					_SN->pCar->do_calculateResultant(1);
			   else
					_SN->pCar->do_calculateResultant(2);
	  }

}




void PhysicsManager::do_POST_UPDATE(Events::Event *pEvt)
{
	 //Draw bounding volumes	
	 for(int i=0 ; i<m_SceneNodes.m_size ; i++)
	 {
		 Handle _SceneNode = m_SceneNodes[i];

		 SceneNode *_SN = m_SceneNodes[i].getObject<SceneNode>();

		 if (_SceneNode.getObject<SceneNode>()->m_Physics->m_shape == 0)
			 _SceneNode.getObject<SceneNode>()->m_Physics->CreateSphere(_SceneNode, 1);

		 else

			 _SceneNode.getObject<SceneNode>()->m_Physics->DrawBox(_SceneNode);
	 }
}

void PhysicsManager::CreatePhysicsComponent(Handle _SceneNode, Mesh *_MeshData,bool _IsKinematic)
{
	PE::Handle hPhysicsInstance("Physics", sizeof(Physics));
	Physics *pPhysicsInstance = new(hPhysicsInstance) Physics(*m_pContext, m_arena, hPhysicsInstance);

	_SceneNode.getObject<SceneNode>()->m_Physics = pPhysicsInstance;
	_SceneNode.getObject<SceneNode>()->m_HasPhysics = true;
	int tyreNum = 0;
	pPhysicsInstance->CreateBB(_MeshData->m_hPositionBufferCPU.getObject<PositionBufferCPU>(),tyreNum);
	pPhysicsInstance->m_IsKinematic = _IsKinematic;

	m_SceneNodes.add(_SceneNode);

	//pPhysicsInstance->DrawBox(_SceneNode);
}

void PhysicsManager::CreatePhysicsComponent(Handle _SceneNode, int a)
{
	static int j = 0;
	j++;
	
	PE::Handle hPhysicsInstance("Physics", sizeof(Physics));
	Physics *pPhysicsInstance = new(hPhysicsInstance) Physics(*m_pContext, m_arena, hPhysicsInstance);
	pPhysicsInstance->m_IsKinematic = false;
	_SceneNode.getObject<SceneNode>()->m_Physics = pPhysicsInstance;
	_SceneNode.getObject<SceneNode>()->m_Physics->carNum = j;
	_SceneNode.getObject<SceneNode>()->m_HasPhysics = true;
	pPhysicsInstance->CreateSphere(_SceneNode, a);

	m_SceneNodes.add(_SceneNode);

}

bool PhysicsManager::Raycast(Vector3 _Point1, Vector3 _Point2, AxisAlignedBB _Box, Handle _SceneNodeHandle, Vector3* _PointOfIntersection, int _Plane)
{
	SceneNode* _SceneNode = _SceneNodeHandle.getObject<SceneNode>();

	Plane *_EachPlane = _Box.m_Planes[_Plane];
	_EachPlane->CalculateEquation(_SceneNode);
	
	float m_A = _EachPlane->m_A;
	float m_B = _EachPlane->m_B;
	float m_C = _EachPlane->m_C;
	float m_D = _EachPlane->m_D;

	float _Part1 = m_D - (m_A*_Point1.m_x) - (m_B*_Point1.m_y) - (m_C*_Point1.m_z);
	float _Part2 = (m_A*(_Point2.m_x - _Point1.m_x) - m_B*(_Point2.m_y-_Point1.m_y) - m_C*(_Point2.m_z-_Point1.m_z));
	float t = _Part1/_Part2;

	float x = _Point1.m_x + t*(_Point2.m_x - _Point1.m_x);
	float y = _Point1.m_y + t*(_Point2.m_y - _Point1.m_y);
	float z = _Point1.m_z + t*(_Point2.m_z - _Point1.m_z);

	float _Value = m_A*x + m_B*y + m_C*z;
	
	_PointOfIntersection->m_x = x;
	_PointOfIntersection->m_y = y;
	_PointOfIntersection->m_z = z;

	//_EachPlane->m_NormalVector.dotProduct(_Point1);


	if(_Part2 == 0)
		return false;
	else
		return true;
}


Vector3 PhysicsManager::SphereInPlane(Vector3 center, AxisAlignedBB _Box, int radius)
{
	//Closest point approach
	
	Vector3 p = Vector3(0,0,0);
	Vector3 q = Vector3(0,0,0);

	p.m_x = center.m_x;
	p.m_y = center.m_y;
	p.m_z = center.m_z;
	if(p.m_x < _Box.m_MinX) { p.m_x = _Box.m_MinX;}

	if(p.m_x > _Box.m_MaxX) { p.m_x = _Box.m_MaxX;}

	q.m_x = p.m_x;

	if(p.m_y < _Box.m_MinY) { p.m_y = _Box.m_MinY;}

	if(p.m_y > _Box.m_MaxY) { p.m_y = _Box.m_MaxY;}

	q.m_y = p.m_y;
	
	if(p.m_z < _Box.m_MinZ) { p.m_z = _Box.m_MinZ;}

	if(p.m_z > _Box.m_MaxZ) { p.m_z = _Box.m_MaxZ;}

	q.m_z = p.m_z;

	return q;

}

}; // namespace Components
}; // namespace PE
