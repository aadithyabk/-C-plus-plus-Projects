//This file contains the event handlers for events that will be sent across from client to another


#include "ClientGameObjectManagerAddon.h"

#include "PrimeEngine/PrimeEngineIncludes.h"

#include "Characters/SoldierNPC.h"
#include "Characters/Vampire.h"
#include "Characters\Machoke.h"
#include "Characters\Pikachu.h"
#include "Characters\Minion.h"
#include "WayPoint.h"
#include "Tank/ClientTank.h"
#include "CharacterControl/Client/ClientSpaceShip.h"
#include "Triangle.h"
#include "NavMeshManager.h"
#include "ServerGameObjectManagerAddon.h"


#define SQUAREWIDTH 2
using namespace PE::Components;
using namespace PE::Events;
using namespace CharacterControl::Events;
using namespace CharacterControl::Components;

namespace CharacterControl{
namespace Components
{
PE_IMPLEMENT_CLASS1(ClientGameObjectManagerAddon, Component); // creates a static handle and GteInstance*() methods. still need to create construct

void ClientGameObjectManagerAddon::addDefaultComponents()
{
	GameObjectManagerAddon::addDefaultComponents();

	PE_REGISTER_EVENT_HANDLER(Event_CreateSoldierNPC, ClientGameObjectManagerAddon::do_CreateSoldierNPC);
	PE_REGISTER_EVENT_HANDLER(Event_CREATE_WAYPOINT, ClientGameObjectManagerAddon::do_CREATE_WAYPOINT);
	PE_REGISTER_EVENT_HANDLER(Event_CreateVampire, ClientGameObjectManagerAddon::do_CreateVampire);
	// note this component (game obj addon) is added to game object manager after network manager, so network manager will process this event first
	PE_REGISTER_EVENT_HANDLER(PE::Events::Event_SERVER_CLIENT_CONNECTION_ACK, ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK);
	PE_REGISTER_EVENT_HANDLER(Event_CreateCharacter_S_To_C, ClientGameObjectManagerAddon::do_CreateCharacter_S_To_C);
	PE_REGISTER_EVENT_HANDLER(Event_MoveTank_S_to_C, ClientGameObjectManagerAddon::do_MoveTank);
	PE_REGISTER_EVENT_HANDLER(Event_LoadLevel, ClientGameObjectManagerAddon::do_LoadLevel);
	PE_REGISTER_EVENT_HANDLER(Event_ChangeState, ClientGameObjectManagerAddon::do_ChangeState);
	PE_REGISTER_EVENT_HANDLER(Event_UpdateAttribute, ClientGameObjectManagerAddon::do_UpdateAttribute);
	
}


void ClientGameObjectManagerAddon::do_CreateSoldierNPC(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CreateSoldierNPC>());

	Event_CreateSoldierNPC *pTrueEvent = (Event_CreateSoldierNPC*)(pEvt);

	createSoldierNPC(pTrueEvent);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Vector3 pos, int &threadOwnershipMask)
{
	Event_CreateSoldierNPC evt(threadOwnershipMask);
	evt.m_pos = pos;
	evt.m_u = Vector3(1.0f, 0, 0);
	evt.m_v = Vector3(0, 1.0f, 0);
	evt.m_n = Vector3(0, 0, 1.0f);
	
	StringOps::writeToString( "soldier.x_soldiermesh_skin.skina", evt.m_meshFilename, 255);
	StringOps::writeToString( "Default", evt.m_package, 255);
	StringOps::writeToString( "mg34.x_mg34main_mesh.mesha", evt.m_gunMeshName, 64);
	StringOps::writeToString( "CharacterControl", evt.m_gunMeshPackage, 64);
	StringOps::writeToString( "", evt.m_patrolWayPoint, 32);
	createSoldierNPC(&evt);
}

void ClientGameObjectManagerAddon::createSoldierNPC(Event_CreateSoldierNPC *pTrueEvent)
{
	PEINFO("CharacterControl: GameObjectManagerAddon: Creating CreateSoldierNPC\n");

	PE::Handle hSoldierNPC("SoldierNPC", sizeof(SoldierNPC));
	SoldierNPC *pSoldierNPC = new(hSoldierNPC) SoldierNPC(*m_pContext, m_arena, hSoldierNPC, pTrueEvent, m_attackRangeOfSoldier, m_viewRangeOfSoldier,false);
	pSoldierNPC->addDefaultComponents();

	// add the soldier as component to the ObjecManagerComponentAddon
	// all objects of this demo live in the ObjecManagerComponentAddon
	m_pSoldiers.add(pSoldierNPC);
	addComponent(hSoldierNPC);
}

TankController* ClientGameObjectManagerAddon::getPlayer()
{

	PE::Handle *pHC = m_components.getFirstPtr();
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			return (TankController*)pC;
		}
	}
}

void ClientGameObjectManagerAddon::reset()
{
	for (int i = 0; i < m_pSoldiers.m_size; i++)
	{
		m_pSoldiers[i]->Despawn();
		m_pSoldiers[i]->Respawn();
	}
	TankController* pTC = getPlayer();
	pTC->Reset();
}

int ClientGameObjectManagerAddon::getTank(Vector3 * pos)
{
	PE::Handle *pHC = m_components.getFirstPtr();
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
				TankController *pTK = (TankController *)(pC);
				PE::Handle hFisrtSN = pTK->getFirstComponentHandle<SceneNode>();
				SceneNode *pFirstSN = hFisrtSN.getObject<SceneNode>();
				pos->m_x = pFirstSN->m_base.getPos().getX();
				pos->m_y = pFirstSN->m_base.getPos().getY();
				pos->m_z = pFirstSN->m_base.getPos().getZ();
				return 1;
		}
	}
	return 0;
}
void ClientGameObjectManagerAddon::do_CreateVampire(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CreateVampire>());

	Event_CreateVampire *pTrueEvent = (Event_CreateVampire*)(pEvt);

	createVampire(pTrueEvent);
}

void ClientGameObjectManagerAddon::createVampire(Vector3 pos, int &threadOwnershipMask)
{
	Event_CreateVampire evt(threadOwnershipMask);
	evt.m_pos = pos;
	evt.m_u = Vector3(1.0f, 0, 0);
	evt.m_v = Vector3(0, 1.0f, 0);
	evt.m_n = Vector3(0, 0, 1.0f);

	StringOps::writeToString("SoldierTransform.mesha", evt.m_meshFilename, 255);
	StringOps::writeToString("Soldier", evt.m_package, 255);
	StringOps::writeToString("mg34.x_mg34main_mesh.mesha", evt.m_gunMeshName, 64);
	StringOps::writeToString("CharacterControl", evt.m_gunMeshPackage, 64);
	StringOps::writeToString("", evt.m_patrolWayPoint, 32);
	createVampire(&evt);
}

void ClientGameObjectManagerAddon::createVampire(Event_CreateVampire *pTrueEvent)
{
	PEINFO("CharacterControl: GameObjectManagerAddon: Creating CreateSoldierNPC\n");

	PE::Handle hVampire("Vampire", sizeof(Vampire));
	Vampire *pVampire = new(hVampire)Vampire(*m_pContext, m_arena, hVampire, pTrueEvent);
	pVampire->addDefaultComponents();

	// add the soldier as component to the ObjecManagerComponentAddon
	// all objects of this demo live in the ObjecManagerComponentAddon
	addComponent(hVampire);
}

void ClientGameObjectManagerAddon::createNavMesh(){


			//NavMeshManager::Instance()->createNavMesh();
		/*	PE::Handle hNavMeshManager("NavMeshManager", sizeof(NavMeshManager));
			NavMeshManager *pNavMeshManager = new(hNavMeshManager) NavMeshManager(*m_pContext, m_arena, hNavMeshManager);
			pNavMeshManager->addDefaultComponents();
			pNavMeshManager->createNavMesh();*/

}


void ClientGameObjectManagerAddon::do_CREATE_WAYPOINT(PE::Events::Event *pEvt)
{
	PEINFO("GameObjectManagerAddon::do_CREATE_WAYPOINT()\n");

	assert(pEvt->isInstanceOf<Event_CREATE_WAYPOINT>());

	Event_CREATE_WAYPOINT *pTrueEvent = (Event_CREATE_WAYPOINT*)(pEvt);

	PE::Handle hWayPoint("WayPoint", sizeof(WayPoint));
	WayPoint *pWayPoint = new(hWayPoint) WayPoint(*m_pContext, m_arena, hWayPoint, pTrueEvent);
	pWayPoint->addDefaultComponents();

	addComponent(hWayPoint);
}

WayPoint *ClientGameObjectManagerAddon::getWayPoint(const char *name)
{
	PE::Handle *pHC = m_components.getFirstPtr();

	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<WayPoint>())
		{
			WayPoint *pWP = (WayPoint *)(pC);
			if (StringOps::strcmp(pWP->m_name, name) == 0)
			{
				// equal strings, found our waypoint
				return pWP;
			}
		}
	}
	return NULL;
}

SoldierNPC *ClientGameObjectManagerAddon::getSoldier()
{
	PE::Handle *pHC = m_components.getFirstPtr();

	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<SoldierNPC>())
		{
			SoldierNPC *pWP = (SoldierNPC *)(pC);
				// equal strings, found our waypoint
			return pWP;
			
		}
	}
	return NULL;
}

TankController *ClientGameObjectManagerAddon::getTank()

	{

		PE::Handle *pHC = m_components.getFirstPtr();

		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<TankController>())
			{


				TankController *pWP = (TankController *)(pC);
				return pWP;

			}
		}
	
	return NULL;
}



Vampire *ClientGameObjectManagerAddon::getVampire(int id)
{

	if (id != 0)
	{

		PE::Handle *pHC = m_components.getFirstPtr();
		int count = 0;
		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<Vampire>())
			{
				/*count++;
				if(count == id)
				{
				Vampire *pWP = (Vampire *)(pC);
				return pWP;
				}*/
				Vampire *pWP = (Vampire *)(pC);
				if (pWP->vampireNumber == id)
				{
					if (pWP->m_VampireBehaviorSM)
						return pWP;
				}

			}
		}
	}
	else
	{

		PE::Handle *pHC = m_components.getFirstPtr();

		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<Vampire>())
			{


				Vampire *pWP = (Vampire *)(pC);
				return pWP;

			}
		}
	}
	return NULL;
}


void ClientGameObjectManagerAddon::createTank(int index, int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    Tank

	//game object manager
	//  TankController
	//    scene node
	
	
	//pMeshInstance->initFromFile("imrod.x_imrodmesh_mesh.mesha", "Default", threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("createTank SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	//Vector3 spawnPos(-36.0f + 6.0f * index, 0 , 21.0f);
	Vector3 spawnPos(-11.0f + 6.0f * index, 0.1f , 11.0f);
	pSN->m_base.setPos(spawnPos);
	
	//pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects

	PE::Handle hTankController("TankController", sizeof(TankController));
	TankController *pTankController = new(hTankController) TankController(*m_pContext, m_arena, hTankController, 0.05f, spawnPos,  0.05f);
	pTankController->addDefaultComponents();

	addComponent(hTankController);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of square controller just for referencing purposes
	pTankController->addComponent(hSN, &alllowedEventsToPropagate[0]);
}

void ClientGameObjectManagerAddon::createSpaceShip(int &threadOwnershipMask)
{

	//create hierarchy:
	//scene root
	//  scene node // tracks position/orientation
	//    SpaceShip

	//game object manager
	//  SpaceShipController
	//    scene node

	PE::Handle hMeshInstance("MeshInstance", sizeof(MeshInstance));
	MeshInstance *pMeshInstance = new(hMeshInstance) MeshInstance(*m_pContext, m_arena, hMeshInstance);

	pMeshInstance->addDefaultComponents();
	pMeshInstance->initFromFile("space_frigate_6.mesha", "FregateTest", threadOwnershipMask);

	// need to create a scene node for this mesh
	PE::Handle hSN("SCENE_NODE", sizeof(SceneNode));
	SceneNode *pSN = new(hSN) SceneNode(*m_pContext, m_arena, hSN);
	pSN->addDefaultComponents();

	Vector3 spawnPos(0, 0, 0.0f);
	pSN->m_base.setPos(spawnPos);

	pSN->addComponent(hMeshInstance);

	RootSceneNode::Instance()->addComponent(hSN);

	// now add game objects

	PE::Handle hSpaceShip("ClientSpaceShip", sizeof(ClientSpaceShip));
	ClientSpaceShip *pSpaceShip = new(hSpaceShip) ClientSpaceShip(*m_pContext, m_arena, hSpaceShip, 0.05f, spawnPos,  0.05f);
	pSpaceShip->addDefaultComponents();

	addComponent(hSpaceShip);

	// add the same scene node to tank controller
	static int alllowedEventsToPropagate[] = {0}; // we will pass empty array as allowed events to propagate so that when we add
	// scene node to the square controller, the square controller doesnt try to handle scene node's events
	// because scene node handles events through scene graph, and is child of space ship just for referencing purposes
	pSpaceShip->addComponent(hSN, &alllowedEventsToPropagate[0]);

	pSpaceShip->activate();
}


void ClientGameObjectManagerAddon::do_SERVER_CLIENT_CONNECTION_ACK(PE::Events::Event *pEvt)
{
	Event_SERVER_CLIENT_CONNECTION_ACK *pRealEvt = (Event_SERVER_CLIENT_CONNECTION_ACK *)(pEvt);
	PE::Handle *pHC = m_components.getFirstPtr();

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pRealEvt->m_clientId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->activate();
				break;
			}
			++itc;
		}
	}
}

void ClientGameObjectManagerAddon::spawnCharacter(CharacterControl::CharacterCodes character, CharacterControl::TeamCodes team)
{

	static int i = 0;
	i++;
	if(character == MACHOKE)
	{
		PE::Handle hMachoke("Machoke", sizeof(Machoke));
		Vector3 pos; 
		if(i==0)
			pos = Vector3(0, 0, 10);
		else
			pos = Vector3(10, 0, 10);
		Machoke *pMachoke = new(hMachoke)Machoke(*m_pContext, m_arena, hMachoke, pos);
		pMachoke->addDefaultComponents();
		addComponent(hMachoke);
		pMachoke->pokemonType = MACHOKE;

		PE::Handle hNetworkView("NetworkView_Machoke_NotOwned", sizeof(NetworkView));
		NetworkView *pNetworkView = new(hNetworkView)NetworkView(*m_pContext, m_arena, hNetworkView);
		pNetworkView->IsOwner = true;
		pNetworkView->hPokemon = hMachoke;
		pMachoke->m_networkView = pNetworkView;
		
		ServerGameObjectManagerAddon *pServerGame = (CharacterControl::Components::ServerGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
		pNetworkView->m_ghostID = pServerGame->ghostID;
		selectedCharacter = MACHOKE;
		
		MyNetworkManager *myNetworkManager = MyNetworkManager::Instance();
		myNetworkManager->m_networkViews.add(pNetworkView);

		//Send event to other clients.
		Event_CreateCharacter_C_To_S evt(*m_pContext);
		evt.CharacterCode = MACHOKE;
		evt.pos = pos;
		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true, false);
	}
	else if(character == PIKACHU)
	{
		PE::Handle hPikachu("Pikachu", sizeof(Pikachu));
		Vector3 pos = Vector3(5, 0, 10);
		Pikachu *pPikachu = new(hPikachu)Pikachu(*m_pContext, m_arena, hPikachu, pos);
		pPikachu->addDefaultComponents();
		addComponent(hPikachu);
		pPikachu->pokemonType = PIKACHU;

		PE::Handle hNetworkView("NetworkView_Machoke_NotOwned", sizeof(NetworkView));
		NetworkView *pNetworkView = new(hNetworkView)NetworkView(*m_pContext, m_arena, hNetworkView);
		pNetworkView->IsOwner = true;
		pNetworkView->hPokemon = hPikachu;
		pPikachu->m_networkView = pNetworkView;
		ServerGameObjectManagerAddon *pServerGame = (CharacterControl::Components::ServerGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
		pNetworkView->m_ghostID = pServerGame->ghostID;
		
		selectedCharacter = PIKACHU;
		MyNetworkManager *myNetworkManager = MyNetworkManager::Instance();
		myNetworkManager->m_networkViews.add(pNetworkView);

		//Send event to other clients.
		Event_CreateCharacter_C_To_S evt(*m_pContext);
		evt.CharacterCode = PIKACHU;
		evt.pos = pos;
		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true, false);
	}
	else if(character == MINION)
	{
		PE::Handle hMinion("Minion", sizeof(Minion));
		Vector3 pos = Vector3(5, 0, 10);
		//Minions will spawn depending on team
		Minion *pMinion = new(hMinion)Minion(*m_pContext, m_arena, hMinion, pos, team);
		pMinion->addDefaultComponents();
		addComponent(hMinion);
		pMinion->pokemonType = MINION;

		PE::Handle hNetworkView("NetworkView_Machoke_NotOwned", sizeof(NetworkView));
		NetworkView *pNetworkView = new(hNetworkView)NetworkView(*m_pContext, m_arena, hNetworkView);
		pNetworkView->IsOwner = true;
		pNetworkView->hPokemon = hMinion;
		pMinion->m_networkView = pNetworkView;
		ServerGameObjectManagerAddon *pServerGame = (CharacterControl::Components::ServerGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
		pNetworkView->m_ghostID = pServerGame->ghostID;
		
		selectedCharacter = MINION;
		MyNetworkManager *myNetworkManager = MyNetworkManager::Instance();
		myNetworkManager->m_networkViews.add(pNetworkView);

		//Send event to other clients.
		Event_CreateCharacter_C_To_S evt(*m_pContext);
		evt.CharacterCode = MINION;
		evt.pos = pos;
		ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
		pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&evt, m_pContext->getGameObjectManager(), true, false);
	}

	

	


}


void ClientGameObjectManagerAddon::do_MoveTank(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_MoveTank_S_to_C>());

	Event_MoveTank_S_to_C *pTrueEvent = (Event_MoveTank_S_to_C*)(pEvt);

	PE::Handle *pHC = m_components.getFirstPtr();

	int itc = 0;
	for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
	{
		Component *pC = (*pHC).getObject<Component>();

		if (pC->isInstanceOf<TankController>())
		{
			if (itc == pTrueEvent->m_clientTankId) //activate tank controller for local client based on local clients id
			{
				TankController *pTK = (TankController *)(pC);
				pTK->overrideTransform(pTrueEvent->m_transform);
				break;
			}
			++itc;
		}
	}
}

void ClientGameObjectManagerAddon::do_LoadLevel(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_LoadLevel>());
	Event_LoadLevel *pTrueEvent = (Event_LoadLevel*)(pEvt);
	//char* objectToLoad1 = pTrueEvent->objectToLoad;
	char* objectToLoad2 = objectToLoad;
	//m_pContext->getLuaEnvironment()->runString("LevelLoader.loadLevel('ccontrollvl0.x_PEObject45_nazi_.lua', 'CharacterControl')");
	//m_pContext->getLuaEnvironment()->runString("LevelLoader.CreateGameObject(\"PEObject33_Tstreetlight\",1.0, 0.0, -0.0, 0.0, 0.0, 1.0, -0.0, 0.0, -0.0, -0.0, 1.0, 0.0, 105.408753, 0.0, 471.322242, 1.0, 'ccontrollvl0.x_PEObject33_Tstre.lua', 'CharacterControl',0x42883a0f, 0x1c6a11e0, 0x97b5001b, 0x213b583a)");
	m_pContext->getLuaEnvironment()->runString(objectToLoad);
	m_pContext->getLuaEnvironment()->runString("LevelLoader.CreateGameObject(\"PEObject17_pCar\", -0.886791, 0.0, -0.46217, 0.0, -0.0, 1.0, -0.0, 0.0, 0.46217, 0.0, -0.886791, 0.0, 200.346931, 0.0, 425.207712, 1.0, 'ccontrollvl0.x_PEObject17_pCar.lua', 'CharacterControl', 0xb2cfd370, 0x1c5411e0, 0x9f4e001b, 0x213b583a)");
	m_pContext->getLuaEnvironment()->runString("LevelLoader.loadLevel('char_highlight.x_level.levela', 'Basic')");
	m_pContext->getLuaEnvironment()->runString("LevelLoader.CreateGameObject(\"PEObject47_MutantMesh\", 1.0, 0.0, -0.0, 0.0, 0.0, 1.0, -0.0, 0.0, -0.0, -0.0, 1.0, 0.0, 0.0, 0.0, -0.0, 1.0, 'Mutant.x_PEObject47_Mutan.lua', 'CharacterControl', 0, 0, 0, 0)"); 
	//LevelLoader.CreateGameObject("PEObject33_Tstreetlight",1.0, 0.0, -0.0, 0.0, 0.0, 1.0, -0.0, 0.0, -0.0, -0.0, 1.0, 0.0, 105.408753, 0.0, 471.322242, 1.0, 'ccontrollvl0.x_PEObject33_Tstre.lua', 'CharacterControl',0x42883a0f, 0x1c6a11e0, 0x97b5001b, 0x213b583a)
}

void ClientGameObjectManagerAddon::do_CreateCharacter_S_To_C(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_CreateCharacter_S_To_C>());
	MyNetworkManager *pNetworkManager = MyNetworkManager::Instance();
	Event_CreateCharacter_S_To_C *pTrueEvent = (Event_CreateCharacter_S_To_C*)(pEvt);
	if (pTrueEvent->CharacterCode == MACHOKE)
	{
		//Create Machoke
		PEINFO("CharacterControl: GameObjectManagerAddon: Creating Machoke\n");
		enemyCharacter = MACHOKE;
		PE::Handle hMachoke("Machoke", sizeof(Machoke));
		Vector3 pos = pTrueEvent->pos;
		Machoke *pMachoke = new(hMachoke)Machoke(*m_pContext, m_arena, hMachoke, pos);
		pMachoke->addDefaultComponents();
		pMachoke->pokemonType = MACHOKE;
		// add the soldier as component to the ObjecManagerComponentAddon
		// all objects of this demo live in the ObjecManagerComponentAddon
		addComponent(hMachoke);
		PE::Handle hNetworkView("NetworkView_Machoke_NotOwned", sizeof(NetworkView));
		NetworkView *pNetworkView = new(hNetworkView)NetworkView(*m_pContext, m_arena, hNetworkView);
		pNetworkView->IsOwner = false;
		pNetworkView->m_ghostID = pTrueEvent->m_ghostID;

		ServerGameObjectManagerAddon *pServerGame = (CharacterControl::Components::ServerGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
		pServerGame->ghostID++;

		pNetworkView->hPokemon = hMachoke;

		pMachoke->m_networkView = pNetworkView;

		
		pNetworkManager->m_networkViews.add(pNetworkView);


	}
	if (pTrueEvent->CharacterCode == PIKACHU)
	{
		//Create Pikachu
		PEINFO("CharacterControl: GameObjectManagerAddon: Creating Pikachu\n");

		PE::Handle hPikachu("Pikachu", sizeof(Pikachu));
		Vector3 pos = pTrueEvent->pos;
		Pikachu *pPikachu = new(hPikachu)Pikachu(*m_pContext, m_arena, hPikachu, pos);
		pPikachu->addDefaultComponents();
		enemyCharacter = PIKACHU;
		pPikachu->pokemonType = PIKACHU;
		// add the soldier as component to the ObjecManagerComponentAddon
		// all objects of this demo live in the ObjecManagerComponentAddon
		addComponent(hPikachu);
		PE::Handle hNetworkView("NetworkView_Machoke_NotOwned", sizeof(NetworkView));
		NetworkView *pNetworkView = new(hNetworkView)NetworkView(*m_pContext, m_arena, hNetworkView);
		pNetworkView->IsOwner = false;
		pNetworkView->m_ghostID = pTrueEvent->m_ghostID;
		pNetworkView->hPokemon = hPikachu;

		ServerGameObjectManagerAddon *pServerGame = (CharacterControl::Components::ServerGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
		pServerGame->ghostID++;

		pPikachu->m_networkView = pNetworkView;

		pNetworkManager->m_networkViews.add(pNetworkView);
	}


	if (pTrueEvent->CharacterCode == MINION)
	{
		//Create Minion
		PEINFO("CharacterControl: GameObjectManagerAddon: Creating Minion\n");

		PE::Handle hMinion("Minion", sizeof(Minion));
		Vector3 pos = pTrueEvent->pos;
		Minion *pMinion = new(hMinion)Minion(*m_pContext, m_arena, hMinion, pos, TEAM_A);
		pMinion->addDefaultComponents();
		enemyCharacter = MINION;
		pMinion->pokemonType = MINION;
		// add the soldier as component to the ObjecManagerComponentAddon
		// all objects of this demo live in the ObjecManagerComponentAddon
		addComponent(hMinion);
		PE::Handle hNetworkView("NetworkView_Machoke_NotOwned", sizeof(NetworkView));
		NetworkView *pNetworkView = new(hNetworkView)NetworkView(*m_pContext, m_arena, hNetworkView);
		pNetworkView->IsOwner = false;
		pNetworkView->m_ghostID = pTrueEvent->m_ghostID;
		pNetworkView->hPokemon = hMinion;

		ServerGameObjectManagerAddon *pServerGame = (CharacterControl::Components::ServerGameObjectManagerAddon *)(m_pContext->get<CharacterControl::CharacterControlContext>()->getGameObjectManagerAddon());
		pServerGame->ghostID++;

		pMinion->m_networkView = pNetworkView;

		pNetworkManager->m_networkViews.add(pNetworkView);
	}
}

void ClientGameObjectManagerAddon::changeState()
{
	Event_ChangeState pEvt(*m_pContext);
	pEvt.changeThisGhost = 1;
	pEvt.nextState = CharacterControl::m_states::MACHOKE_WALK;
	ClientNetworkManager *pNetworkManager = (ClientNetworkManager *)(m_pContext->getNetworkManager());
	pNetworkManager->getNetworkContext().getEventManager()->scheduleEvent(&pEvt, m_pContext->getGameObjectManager(), true, false);
}	

void ClientGameObjectManagerAddon::do_ChangeState(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_ChangeState>());
	Event_ChangeState *pTrueEvent = (Event_ChangeState*)(pEvt);
	int ghostToBeChanged = pTrueEvent->changeThisGhost;

	Pokemon *pokemon;
	NetworkView *currentView;
	MyNetworkManager *myNetworkManager = MyNetworkManager::Instance();
	for (int i = 0; i < myNetworkManager->m_networkViews.m_size; i++)
	{
		currentView = myNetworkManager->m_networkViews[i];
		if (currentView->m_ghostID == ghostToBeChanged)
		{
			//Found the Ghost to be changed
			PE::Handle hPokemon = currentView->hPokemon;
			pokemon = hPokemon.getObject<Pokemon>();
			nextState = pTrueEvent->nextState;
			pokemon->ChangeState(nextState);
		}
	}
}

void ClientGameObjectManagerAddon::do_UpdateAttribute(PE::Events::Event *pEvt)
{
	assert(pEvt->isInstanceOf<Event_UpdateAttribute>());
	Event_UpdateAttribute *pTrueEvent = (Event_UpdateAttribute*)(pEvt);
	int ghostToBeChanged = pTrueEvent->changeThisGhost;

	MyNetworkManager *myNetworkManager = MyNetworkManager::Instance();
	for (int i = 0; i < myNetworkManager->m_networkViews.m_size; i++)
	{
		NetworkView *currentView = myNetworkManager->m_networkViews[i];
		if (currentView->m_ghostID == ghostToBeChanged)
		{
			PE::Handle hPokemon = currentView->hPokemon;
			Pokemon *pokemon = hPokemon.getObject<Pokemon>();
			if (pTrueEvent->attributeToBeChanged == POSITION)
			{
				pokemon->UpdateAttribute(pTrueEvent->position,pTrueEvent->direction);
			}
			else if (pTrueEvent->attributeToBeChanged == HEALTH)
			{
				pokemon->UpdateAttribute(pTrueEvent->health);
			}
		}
	}
}

Pokemon* ClientGameObjectManagerAddon::getEnemyCharacter()
{
	PE::Handle *pHC = m_components.getFirstPtr();
	if (enemyCharacter == MACHOKE)
	{
		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<Machoke>())
			{
				Machoke *pMachoke = (Machoke *)(pC);
				return pMachoke;

			}
		}
	}
	else if (enemyCharacter == PIKACHU)
	{
		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<Pikachu>())
			{

				Pikachu *pPikachu = (Pikachu *)(pC);
				return pPikachu;

			}
		}
	}
	return NULL;
}

Pokemon* ClientGameObjectManagerAddon::getPlayerCharacter()
{
	PE::Handle *pHC = m_components.getFirstPtr();
	if (selectedCharacter == MACHOKE)
	{
		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<Machoke>())
			{
				Machoke *pMachoke = (Machoke *)(pC);
				return pMachoke;

			}
		}
	}
	else if (selectedCharacter == PIKACHU)
	{
		for (PrimitiveTypes::UInt32 i = 0; i < m_components.m_size; i++, pHC++) // fast array traversal (increasing ptr)
		{
			Component *pC = (*pHC).getObject<Component>();

			if (pC->isInstanceOf<Pikachu>())
			{

				Pikachu *pPikachu = (Pikachu *)(pC);
				return pPikachu;

			}
		}
	}
	return NULL;
}
}
}
