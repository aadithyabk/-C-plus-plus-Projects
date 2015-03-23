#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "Physics.h"
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

// Sibling/Children includes

#include "../../PrimeEngine/Scene/MeshInstance.h"
#include "../../PrimeEngine/Scene/SceneNode.h"
#include "../../PrimeEngine/Scene/DrawList.h"
#include "../../PrimeEngine/Scene/SH_DRAW.h"
#include "../../PrimeEngine/Lua/LuaEnvironment.h"
#include "../../PrimeEngine/Scene/DebugRenderer.h"

#include <list>

namespace PE {
namespace Components {

//PE_IMPLEMENT_CLASS1(Plane, Component);

Plane::Plane()
{
	m_NormalVector = Vector3(0,0,0);
	m_Position = Vector3(0,0,0);
}
void Plane::CalculateCorners(Vector3 _TopLeft, Vector3 _TopRight, Vector3 _BottomRight, Vector3 _BottomLeft)
{
	m_TopLeft = _TopLeft;
	m_TopRight = _TopRight;
	m_BottomLeft = _BottomLeft;
	m_BottomRight = _BottomRight;

}
void Plane::CalculateEquation(Handle _SceneNodeHandle)
{
	SceneNode* _SceneNode = _SceneNodeHandle.getObject<SceneNode>();

	Vector3 _BottomRightWorld = _SceneNode->m_worldTransform * m_BottomRight;
	Vector3 _BottomLeftWorld = _SceneNode->m_worldTransform * m_BottomLeft;
	Vector3 _m_TopLeftWorld = _SceneNode->m_worldTransform * m_TopLeft;

	Vector3 _Line1 = _BottomLeftWorld - _BottomRightWorld;
	Vector3 _Line2 = _m_TopLeftWorld - _BottomRightWorld;

	m_NormalVector = _Line1.crossProduct(_Line2);
	m_NormalVector.normalize();

	m_A = m_NormalVector.m_x;
	m_B = m_NormalVector.m_y;
	m_C = m_NormalVector.m_z;
	m_D = -(m_A*_BottomLeftWorld.m_x + m_B*_BottomLeftWorld.m_y + m_C*_BottomLeftWorld.m_z); 

}
void Plane::DrawPlane(Handle _CameraSceneNodeHandle, int i)
{
	SceneNode* _CameraSceneNode = _CameraSceneNodeHandle.getObject<SceneNode>();
	Vector3 color;
	if(i==0)
		color = Vector3(1.0f, 1.0f, 1.0f);
	else
		color = Vector3(1.0f, 0.0f, 0.0f);

	Vector3 _CameraPosition = _CameraSceneNode->m_base.getPos();
		
	Vector3 _Pos1 = m_TopLeft;
	Vector3 _Pos2 = m_TopRight;
	Vector3 _Pos3 = m_BottomRight; 
	Vector3 _Pos4 = m_BottomLeft; 

	_Pos1 = _CameraSceneNode->m_worldTransform * _Pos1;
	_Pos2 = _CameraSceneNode->m_worldTransform * _Pos2;
	_Pos3 = _CameraSceneNode->m_worldTransform * _Pos3;
	_Pos4 = _CameraSceneNode->m_worldTransform * _Pos4;

	Vector3 linepts1[] = {_Pos1, color, _Pos2, color};			
	Vector3 linepts2[] = {_Pos2, color, _Pos3, color};			
	Vector3 linepts3[] = {_Pos3, color, _Pos4, color};			
	Vector3 linepts4[] = {_Pos4, color, _Pos1, color};			

	DebugRenderer::Instance()->createLineMesh(true, _CameraSceneNode->m_worldTransform, &linepts1[0].m_x, 2, 4);
	DebugRenderer::Instance()->createLineMesh(true, _CameraSceneNode->m_worldTransform, &linepts2[0].m_x, 2, 4);
	DebugRenderer::Instance()->createLineMesh(true, _CameraSceneNode->m_worldTransform, &linepts3[0].m_x, 2, 4);
	DebugRenderer::Instance()->createLineMesh(true, _CameraSceneNode->m_worldTransform, &linepts4[0].m_x, 2, 4);
}
void Plane::CalculateAndDrawNormal(Handle _CameraSceneNodeHandle, bool _ShouldDraw)
{
	SceneNode* _CameraSceneNode = _CameraSceneNodeHandle.getObject<SceneNode>();

	Vector3 _Line1 = _CameraSceneNode->m_worldTransform * m_BottomRight - _CameraSceneNode->m_worldTransform * m_BottomLeft;
	Vector3 _Line2 = _CameraSceneNode->m_worldTransform * m_TopLeft - _CameraSceneNode->m_worldTransform * m_BottomLeft;

	m_NormalVector = _Line2.crossProduct(_Line1);
	m_NormalVector.normalize();

	Vector3 _Point1 = _CameraSceneNode->m_worldTransform * m_Center;
	Vector3 _Point2 = _Point1 + m_NormalVector * 3;

	//Vector3 _Point1Test = _CameraSceneNode->m_worldTransform *( m_Center +  Vector3(20,0,0));
	//Vector3 _Point2Test = _Point1 + m_NormalVector * 3;
	Vector3 color = Vector3(1.0f, 1.0f, 1.0f);
	//Vector3 colorTest = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 linepts1[] = {_Point1, color, _Point2, color};	
	//Vector3 lineptsTest[] = {_Point1Test, color, _Point2Test, colorTest};
	if(_ShouldDraw)	
	{
		//DebugRenderer::Instance()->createLineMesh(false, _CameraSceneNode->m_worldTransform, &lineptsTest[0].m_x, 2, 4);
		//DebugRenderer::Instance()->createLineMesh(false, _CameraSceneNode->m_worldTransform, &linepts1[0].m_x, 2, 4);
	}
}
bool Plane::CheckifPointInsidePlane(Vector3 _Point, Handle _CameraSceneNodeHandle)
{
	SceneNode* _CameraSceneNode = _CameraSceneNodeHandle.getObject<SceneNode>();

	bool _ReturnValue = false;
	Vector3 _PointOnPlane = m_BottomLeft;
	_PointOnPlane = _CameraSceneNode->m_worldTransform * _PointOnPlane;

	float DotProduct = m_NormalVector.dotProduct((_Point - _PointOnPlane));

	Vector3 color = Vector3(1.0f, 1.0f, 1.0f);
	Vector3 linepts1[] = {_Point, color, _PointOnPlane, color};	
	//DebugRenderer::Instance()->createLineMesh(false, _CameraSceneNode->m_worldTransform, &linepts1[0].m_x, 2, 4);

	if(DotProduct < 0)
		_ReturnValue = true;

	return _ReturnValue;
}
}; // namespace Components
}; // namespace PE

namespace PE {
namespace Components{


PE_IMPLEMENT_CLASS1(Physics, Component);

Physics::Physics(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	, m_instances(context, arena, 16)
	, m_ForceList(context, arena, 16)
{
	m_MaxRestPeriod = 30;
	m_RestPeriod = 30;
}
void Physics::addComponent(Handle hComponent, int *pAllowedEvents /* = NULL */)
{
	if (hComponent.getObject<Component>()->isInstanceOf<MeshInstance>())
	{
		m_instances.add(hComponent);
	}

	Component::addComponent(hComponent, pAllowedEvents);
}

void Physics::removeComponent(int index)
{
	Handle hComponent = m_components[index];
	if (hComponent.getObject<Component>()->isInstanceOf<MeshInstance>())
	{
		m_instances.remove(m_instances.indexOf(hComponent));
	}
	
	Component::removeComponent(index);
}

void Physics::addDefaultComponents()
{
	Component::addDefaultComponents();
}
void Physics::CreateBB(PositionBufferCPU *_PosBufCPU, int tyreNum)
{
	m_shape = 1;
	float minX = 100000.0f;
	float maxX = -1000000.0f;
	float minY = 100000.0f;
	float maxY = -1000000.0f;
	float minZ = 100000.0f;
	float maxZ = -1000000.0f;
	for(int i=0; i < _PosBufCPU->m_values.m_size ; i+=3)
	{
		float x = _PosBufCPU->m_values[i];
		float y = _PosBufCPU->m_values[i+1];
		float z = _PosBufCPU->m_values[i+2];

		if(x < minX)
			minX = x;
		if(x > maxX)
			maxX = x;

		if(y < minY)
			minY = y;
		if(y > maxY)
			maxY = y;

		if(z < minZ)
			minZ = z;
		if(z > maxZ)
			maxZ = z;
	}

	if(minY == maxY)
	{
		minY -= 0.25f;
		maxY += 0.25f;
		m_IsAntiGravity = true;
	}
		

	
	if(tyreNum == 2)
	{
		
		m_BoundingBox.m_MaxX = maxX/12 - 0.8f;
		m_BoundingBox.m_MinX = minX;
		m_BoundingBox.m_MaxY = maxY /2;
		m_BoundingBox.m_MinY = minY;
		m_BoundingBox.m_MaxZ = maxZ /8 - 2.0f;
		m_BoundingBox.m_MinZ = minZ-0.1f;
	}
	else if(tyreNum == 1)
	{
		m_BoundingBox.m_MaxX = maxX;
		m_BoundingBox.m_MinX = minX + 1.5f;
		m_BoundingBox.m_MaxY = maxY /2;
		m_BoundingBox.m_MinY = minY;
		m_BoundingBox.m_MaxZ = maxZ /8 - 2.0f;
		m_BoundingBox.m_MinZ = minZ-0.1f;

	
	}
	else if(tyreNum == 4)
	{
		m_BoundingBox.m_MaxX = maxX;
		m_BoundingBox.m_MinX = minX + 1.5f;
		m_BoundingBox.m_MaxY = maxY /2;
		m_BoundingBox.m_MinY = minY;
		m_BoundingBox.m_MaxZ = maxZ ;
		m_BoundingBox.m_MinZ = minZ+3.8f;
	}
	else if(tyreNum == 3)
	{
		m_BoundingBox.m_MaxX = maxX/12 - 0.8f;
		m_BoundingBox.m_MinX = minX;
		m_BoundingBox.m_MaxY = maxY /2;
		m_BoundingBox.m_MinY = minY;
		m_BoundingBox.m_MaxZ = maxZ ;
		m_BoundingBox.m_MinZ = minZ+3.8f;
	}
	else if(tyreNum == 10)
	{
		m_BoundingBox.m_MaxX = maxX ;
		m_BoundingBox.m_MinX = minX;
		m_BoundingBox.m_MaxY = maxY;
		m_BoundingBox.m_MinY = minY;
		m_BoundingBox.m_MaxZ = maxZ + 0.6f;
		m_BoundingBox.m_MinZ = minZ;
		slope = (maxY - minY)/(maxX -minX);
	}
	else
	{
		m_BoundingBox.m_MaxX = maxX;
		m_BoundingBox.m_MinX = minX;
		m_BoundingBox.m_MaxY = maxY;
		m_BoundingBox.m_MinY = minY;
		m_BoundingBox.m_MaxZ = maxZ ;
		m_BoundingBox.m_MinZ = minZ;
	}



	//PEINFO("The points for %s are %f, %f :: %f, %f :: %f, %f",assetName, maxX, minX, maxY, minY, maxZ, minZ);
	Vector3 _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MinX , m_BoundingBox.m_MinY, m_BoundingBox.m_MinZ);
	m_BoundingBox.m_BoundingBoxPoints[0] = _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MaxX, m_BoundingBox.m_MinY, m_BoundingBox.m_MinZ);
	m_BoundingBox.m_BoundingBoxPoints[1] = _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MaxX, m_BoundingBox.m_MinY, m_BoundingBox.m_MaxZ);
	m_BoundingBox.m_BoundingBoxPoints[2] = _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MinX, m_BoundingBox.m_MinY, m_BoundingBox.m_MaxZ);
	m_BoundingBox.m_BoundingBoxPoints[3] = _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MinX, m_BoundingBox.m_MaxY, m_BoundingBox.m_MinZ);
	m_BoundingBox.m_BoundingBoxPoints[4] = _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MaxX, m_BoundingBox.m_MaxY, m_BoundingBox.m_MinZ);
	m_BoundingBox.m_BoundingBoxPoints[5] = _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MaxX, m_BoundingBox.m_MaxY, m_BoundingBox.m_MaxZ);
	m_BoundingBox.m_BoundingBoxPoints[6] = _Pos1;
	_Pos1 = Vector3(m_BoundingBox.m_MinX, m_BoundingBox.m_MaxY, m_BoundingBox.m_MaxZ);
	m_BoundingBox.m_BoundingBoxPoints[7] = _Pos1;

	
	m_BoundingBox.CreatePlaneEquations();
	//m_Shape = SHAPE.BOX;
}
void Physics::DrawBox(Handle _SceneNodeHandle)
{
	SceneNode *myScene=_SceneNodeHandle.getObject<SceneNode>();
		Vector3 addendum = myScene->m_base.getPos();
		//PEINFO("addendum in Physics = <%f, %f, %f>",addendum.m_x, addendum.m_y, addendum.m_z);
		Matrix4x4 base = myScene->m_worldTransform;
		int i=0;
		Vector3 linepts[100];
	

		Vector3 pos0= base*m_BoundingBox.m_BoundingBoxPoints[0];
		Vector3 pos1= base*m_BoundingBox.m_BoundingBoxPoints[1];
		Vector3 pos2= base*m_BoundingBox.m_BoundingBoxPoints[2];
		Vector3 pos3= base*m_BoundingBox.m_BoundingBoxPoints[3];
		Vector3 pos4= base*m_BoundingBox.m_BoundingBoxPoints[4];
		Vector3 pos5= base*m_BoundingBox.m_BoundingBoxPoints[5];
		Vector3 pos6= base*m_BoundingBox.m_BoundingBoxPoints[6];
		Vector3 pos7= base*m_BoundingBox.m_BoundingBoxPoints[7];


		
		Vector3 color(1.0f, 1.0f, 1.0f);
	


		 linepts[i++] = pos4; 
		 linepts[i++] = color;
		 linepts[i++] = pos5; 
		 linepts[i++] = color;
		 
		 linepts[i++] = pos5; 
		 linepts[i++] = color;
		 linepts[i++] = pos6; 
		 linepts[i++] = color;
		 
		 
		 linepts[i++] = pos6; 
		 linepts[i++] = color;
		 linepts[i++] = pos7; 
		 linepts[i++] = color;

		 linepts[i++] = pos7; 
		 linepts[i++] = color;
		 linepts[i++] = pos4; 
		 linepts[i++] = color;

		 linepts[i++] = pos4; 
		 linepts[i++] = color;
		 linepts[i++] = pos0; 
		 linepts[i++] = color;

		 linepts[i++] = pos0; 
		 linepts[i++] = color;
		 linepts[i++] = pos1; 
		 linepts[i++] = color;

		 linepts[i++] = pos1; 
		 linepts[i++] = color;
		 linepts[i++] = pos5; 
		 linepts[i++] = color;

		 linepts[i++] = pos6; 
		 linepts[i++] = color;
		 linepts[i++] = pos2; 
		 linepts[i++] = color;

		 linepts[i++] = pos2; 
		 linepts[i++] = color;
		 linepts[i++] = pos1; 
		 linepts[i++] = color;

		 linepts[i++] = pos7; 
		 linepts[i++] = color;
		 linepts[i++] = pos3; 
		 linepts[i++] = color;

		 linepts[i++] = pos3; 
		 linepts[i++] = color;
		 linepts[i++] = pos0; 
		 linepts[i++] = color;


		 linepts[i++] = pos2; 
		 linepts[i++] = color;
		 linepts[i++] = pos3; 
		 linepts[i++] = color;
	
	
	
		//DebugRenderer::Instance()->createLineMesh(false, base,  &linepts[0].m_x, 24, 1);
		
		m_BoundingBox.DrawPlanes(_SceneNodeHandle);
}


void Physics::CreateSphere(Handle hSceneNode, float radius)
{ 
		m_shape=0;
		int lats=5, longs=5;
		Vector3 linepts[100];
		int i, j;
		int k = 0;
 
	    Vector3 color = Vector3(0.5, 0.5, 0.0);
	    SceneNode *jSN = hSceneNode.getObject<SceneNode>();
        for (i = 0; i <= lats; i++)
        {
            double lat0 = 3.142 * (-0.5 + (double)(i - 1) / lats);
            double z0 = sin(lat0);
            double zr0 = cos(lat0);

            double lat1 = 3.142 * (-0.5 + (double)i / lats);
            double z1 = sin(lat1);
            double zr1 = cos(lat1);

            for (j = 0; j <= longs; j++)
            {
				k =0;
                double lng = 2 * 3.142 * (double)(j - 1) / longs;
                double x = cos(lng);
                double y = sin(lng);

				Vector3 pos1 = Vector3(radius * x * zr0,radius *  y * zr0,radius *  z0);
				Vector3 pos2 = Vector3(radius * x * zr1,radius *  y * zr1,radius *  z1);
				linepts[k++] =  jSN->m_worldTransform * pos1;; linepts[k++] = color;
				linepts[k++] =  jSN->m_worldTransform * pos2; linepts[k++] = color;
				DebugRenderer::Instance()->createLineMesh(false,jSN->m_worldTransform ,  &linepts[0].m_x, 24, 1);
            }
		}
}


void Physics::AddForce(Vector3 p, Handle h)
{
	SceneNode *pSN = h.getObject<SceneNode>();
	pSN->m_Physics->m_ForceList.add(p);
}
	


void Physics::AddForces(Handle h)
{
	SceneNode *pSN = h.getObject<SceneNode>();
	Vector3 newPos = pSN->m_worldTransform.getPos();
	Vector3 InitialPos = pSN->m_worldTransform.getPos();
	for(int i =0;i < pSN->m_Physics->m_ForceList.m_size; i++)
	{
		//No need to calculate for x and z. Only for the sake of completeness
		newPos.m_x += pSN->m_Physics->m_ForceList[i].m_x;
		newPos.m_y += pSN->m_Physics->m_ForceList[i].m_y;
		newPos.m_z += pSN->m_Physics->m_ForceList[i].m_z;
	}
	pSN->m_base.setPos(newPos);
	pSN->m_Physics->m_ForceList.clear(); //Clear the force list for that frame
}

Vector3* Physics::GetAllPoints()
{
	return m_BoundingBox.m_BoundingBoxPoints;
}



}; // namespace Components
}; // namespace PE
