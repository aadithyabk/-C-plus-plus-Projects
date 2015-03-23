//This is the physics for a PvP based game that I did. I drew spheres as bounding volumes around different skeletal joints. 
//The players could punch or block each other. So I checked for the collision between the spheres when punch animation is being played.
//If the other player is not blocking which I get to know by the joint number with twhich the collision takes place, I reduce the health.

void PhysicsManager::do_UPDATE(Events::Event *pEvt)
{

	ClientGameObjectManagerAddon *pGameObjectManagerAddon = (ClientGameObjectManagerAddon *)(m_pContext->get<CharacterControlContext>()->getGameObjectManagerAddon());
	for(int i = 0; i < m_JointSceneNodes.m_size; i++)
	{
		JointSceneNode *jSN1 = m_JointSceneNodes[i];
		
		for(int j = i+1; j < m_JointSceneNodes.m_size; j++)  //Loop thorugh all joints
		{
			JointSceneNode *jSN2 = m_JointSceneNodes[j];
			Physics *pC1 = jSN1->m_Physics;   //Physics component of that node
			Physics *pC2 = jSN2->m_Physics;
			const char* sample = jSN1->name.c_str();
			
			Vector3 centerTocenter = (jSN1->m_worldTransform.getPos()) - (jSN2->m_worldTransform.getPos());  
			float length = centerTocenter.length(); //Distance between the centres of sphere
			
				//To make sure that collision is not checked with two physics components of the same player
				if((pC1->m_playerNum == 1 && pC2->m_playerNum == 2 ) || (pC1->m_playerNum == 2 && pC2->m_playerNum == 1))
				{
					//To avoid collision between joints that are not required. For example: head and spine.
					if(!(((jSN1->m_jointNum ==3 && jSN2->m_jointNum == 4) || (jSN1->m_jointNum ==4 && jSN2->m_jointNum == 3))))
					{
						if(!(((jSN1->m_jointNum ==5 && jSN2->m_jointNum == 1) || (jSN1->m_jointNum ==1 && jSN2->m_jointNum == 5))))
						{
							
							//Sphere-Sphere collision
							if(centerTocenter.length() < (pC1->radius + pC2->radius)) 
							{
								
								Vampire* _vampire1 = pGameObjectManagerAddon->getVampire(1);
								Vampire* _vampire2 = pGameObjectManagerAddon->getVampire(2);
								
								if(_vampire1->m_MovementSM->m_state == 4) //If he is in punch state
								{	
									vampire2->m_vampireHealth -= 0.5;  
									
									if(_vampire2->m_vampireHealth<=0)
										_vampire2->onDeath(_vampire2);  //Vampire is  Dead
										
									_vampire2->CollisionDetected(_vampire1, jSN1, _vampire2, jSN2);
								}
								else 
								{
									_vampire1->m_vampireHealth -= 0.5;
									if(_vampire1->m_vampireHealth==0)
										_vampire1->onDeath(_vampire1);
									_vampire1->CollisionDetected(_vampire1, jSN1, _vampire2, jSN2);
								}
							}
						}
					}
				}
			}
		}
	}
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

	//Line in paranetric form
	float x = _Point1.m_x + t*(_Point2.m_x - _Point1.m_x);  
	float y = _Point1.m_y + t*(_Point2.m_y - _Point1.m_y);
	float z = _Point1.m_z + t*(_Point2.m_z - _Point1.m_z);

	float _Value = m_A*x + m_B*y + m_C*z;
	
	_PointOfIntersection->m_x = x;
	_PointOfIntersection->m_y = y;
	_PointOfIntersection->m_z = z;

	if(_Part2 == 0)
		return false;
	else
		return true;
}
}; // namespace Components
}; // namespace PE
