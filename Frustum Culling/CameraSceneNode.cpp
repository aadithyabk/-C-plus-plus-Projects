#include "CameraSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include "../Scene/DebugRenderer.h"
#include <math.h>

#define Z_ONLY_CAM_BIAS 0.0f
namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(CameraSceneNode, SceneNode);
Vector3 m_farLowerLeft;
CameraSceneNode::CameraSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) : SceneNode(context, arena, hMyself)
{
	m_near = 0.05f;
	m_far = 2000.0f;
}
void CameraSceneNode::addDefaultComponents()
{
	Component::addDefaultComponents();
	PE_REGISTER_EVENT_HANDLER(Events::Event_CALCULATE_TRANSFORMATIONS, CameraSceneNode::do_CALCULATE_TRANSFORMATIONS);
}

void CameraSceneNode::do_CALCULATE_TRANSFORMATIONS(Events::Event *pEvt)
{
	Vector3 linepts[100];
	int ipt = 0;
	float planeEquations[6][4];
	Matrix4x4 m;
	m.loadIdentity();
	Vector3 color(0.5f, 0.5f, 0.5f);
	
	Handle hParentSN = getFirstParentByType<SceneNode>();
	if (hParentSN.isValid())
	{
		Matrix4x4 parentTransform = hParentSN.getObject<PE::Components::SceneNode>()->m_worldTransform;
		m_worldTransform = parentTransform * m_base;
	}
	
	Matrix4x4 &mref_worldTransform = m_worldTransform;

	Vector3 pos = Vector3(mref_worldTransform.m[0][3], mref_worldTransform.m[1][3], mref_worldTransform.m[2][3]); // has the position of the camera
	Vector3 n = Vector3(mref_worldTransform.m[0][2], mref_worldTransform.m[1][2], mref_worldTransform.m[2][2]); //has normals
	Vector3 target = pos + n;
	Vector3 up = Vector3(mref_worldTransform.m[0][1], mref_worldTransform.m[1][1], mref_worldTransform.m[2][1]);

	m_worldToViewTransform = CameraOps::CreateViewMatrix(pos, target, up);

	m_worldTransform2 = mref_worldTransform;

	m_worldTransform2.moveForward(Z_ONLY_CAM_BIAS);

	pos = Vector3(m_worldTransform2.m[0][3], m_worldTransform2.m[1][3], m_worldTransform2.m[2][3]);
	n = Vector3(m_worldTransform2.m[0][2], m_worldTransform2.m[1][2], m_worldTransform2.m[2][2]);
	target = pos2 + n2;
	up = Vector3(m_worldTransform2.m[0][1], m_worldTransform2.m[1][1], m_worldTransform2.m[2][1]);

	m_worldToViewTransform2 = CameraOps::CreateViewMatrix(pos2, target2, up2);
    
        PrimitiveTypes::Float32 aspect = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth()) / (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
    
        PrimitiveTypes::Float32 verticalFov = 0.33f * PrimitiveTypes::Constants::c_Pi_F32;
        if (aspect < 1.0f)
        {
             //ios portrait view
             static PrimitiveTypes::Float32 factor = 0.5f;
             verticalFov *= factor;
        }

	m_viewToProjectedTransform = CameraOps::CreateProjectionMatrix(verticalFov, 
									       aspect,
										m_near, m_far);
	
	
	
	float hFar,hNear,wFar,wNear;
	float _FOV = 45.0f * 3.142/180;
	
	//Caluclate Height of far plane
	hFar = 2 * tan(verticalFov/2) * m_far;		
	//Calculate Height of near plane
	hNear = 2 * tan(verticalFov/2) * m_near;	
	//Width of near plane
	wNear = hNear * (aspect);
	//Width of far plane
	wFar  = hFar * (aspect);											
	color = Vector3(0.5f, 0.5f, 0);
	 
	 //Calculate 4 points of the far plane
	 
	Vector3 farTopRight = Vector3(0,0,0);
	farTopRight = farTopRight + Vector3(0,0,m_far);
	farTopRight = farTopRight + Vector3(0, Hfar/2, 0) + Vector3((Wfar/2) ,0, 0);
	farTopRight = m_worldTransform * farTopRight;
	
	Vector3 farTopLeft = Vector3(0,0,0);
	farTopLeft = farTopLeft + Vector3(0,0,m_far); farTopLeft =  farTopLeft + Vector3(0, Hfar/2, 0) + Vector3(-(Wfar/2), 0 , 0);
	farTopLeft = m_worldTransform * farTopLeft;

	Vector3 farLowerLeft = Vector3(0,0,0);
	farLowerLeft = farLowerLeft + Vector3(0,0,m_far); farLowerLeft =  farLowerLeft + Vector3(0, -(Hfar/2), 0) + Vector3(-(Wfar/2), 0 , 0);
	farLowerLeft = m_worldTransform * farLowerLeft;

	Vector3 farLowerRight = Vector3(0,0,0);
	farLowerRight = farLowerRight + Vector3(0,0,m_far); farLowerRight =  farLowerRight + Vector3(0, -(Hfar/2), 0) + Vector3((Wfar/2), 0 , 0);
	farLowerRight = m_worldTransform * farLowerRight;

	//Draw the far plane
	ipt = 0;
	linepts[ipt++] = farTopLeft; linepts[ipt++] = color;
	linepts[ipt++] = farTopRight; linepts[ipt++] = color;
	linepts[ipt++] = farLowerLeft; linepts[ipt++] = color;
	linepts[ipt++] = farLowerRight; linepts[ipt++] = color;
	linepts[ipt++] = farTopLeft; linepts[ipt++] = color;
	linepts[ipt++] = farLowerLeft; linepts[ipt++] = color;
	linepts[ipt++] = farTopRight; linepts[ipt++] = color;
	linepts[ipt++] = farLowerRight; linepts[ipt++] = color;

	DebugRenderer::Instance()->createLineMesh(false, m, &linepts[0].m_x, 16, 0);

	//Calculate four points of near plane
	
	Vector3 nearTopRight = Vector3(0,0,0);
	nearTopRight = nearTopRight + Vector3(0,0,m_near); 
	nearTopRight = nearTopRight + Vector3(0, Hnear/2, 0) + Vector3((Wnear/2) ,0, 0);
	nearTopRight = m_worldTransform * nearTopRight;
	
	Vector3 nearTopLeft = Vector3(0,0,0);
	nearTopLeft = nearTopLeft + Vector3(0,0,m_near); 
	nearTopLeft =  nearTopLeft + Vector3(-(Wnear/2), Hnear/2, 0); 
	nearTopLeft = m_worldTransform * nearTopLeft;

	
	Vector3 nearLowerRight = Vector3(0,0,0);
	nearLowerRight = nearLowerRight + Vector3(0,0,m_near); nearLowerRight = nearLowerRight + Vector3((Wnear/2), 0,  0 ) + Vector3(0 , -(Hnear/2), 0);
	nearLowerRight = m_worldTransform * nearLowerRight;
	
	Vector3 nearLowerLeft = Vector3(0,0,0);
	nearLowerLeft = nearLowerLeft + Vector3(0,0,m_near); nearLowerLeft =  nearLowerLeft + Vector3(-(Wnear/2), 0, 0) + Vector3(0, -(Hnear/2), 0); 
	nearLowerLeft = m_worldTransform * nearLowerLeft;

	//Draw near plane
	ipt = 0;
	linepts[ipt++] = nearTopLeft; linepts[ipt++] = color;
	linepts[ipt++] = nearTopRight; linepts[ipt++] = color;
	linepts[ipt++] = nearLowerLeft; linepts[ipt++] = color;
	linepts[ipt++] = nearLowerRight; linepts[ipt++] = color;
	linepts[ipt++] = nearTopLeft; linepts[ipt++] = color;
	linepts[ipt++] = nearLowerLeft; linepts[ipt++] = color;
	linepts[ipt++] = nearTopRight; linepts[ipt++] = color;
	linepts[ipt++] = nearLowerRight; linepts[ipt++] = color;
	DebugRenderer::Instance()->createLineMesh(false, m, &linepts[0].m_x, 16, 4);

	//Connect the two planes (Draw the side planes)

	ipt = 0;
	linepts[ipt++] = nearLowerLeft; linepts[ipt++] = color;
	linepts[ipt++] = farLowerleft; linepts[ipt++] = color;
	linepts[ipt++] = nearTopLeft; linepts[ipt++] = color;
	linepts[ipt++] = farTopLeft; linepts[ipt++] = color;
	linepts[ipt++] = nearTopRight; linepts[ipt++] = color;
	linepts[ipt++] = farTopRight; linepts[ipt++] = color;
	linepts[ipt++] = nearLowerRight; linepts[ipt++] = color;
	linepts[ipt++] = farLowerRight; linepts[ipt++] = color;
	
	DebugRenderer::Instance()->createLineMesh(false, m, &linepts[0].m_x, 16, 4);

	
	m_nearTopLeft = nearTopLeft;
	m_farTopLeft = farTopLeft;
	m_nearTopRight = nearTopRight;
	m_farTopRight = farTopRight;
	m_nearLowerLeft = nearLowerLeft;
	m_nearLowerRight  = nearLowerRight;
	m_farLowerLeft = farLowerLeft;
	m_farLowerRight = farLowerRight;

	DebugRenderer::Instance()->createLineMesh(false, m, &linepts[0].m_x, 20, 4);
	
	SceneNode::do_CALCULATE_TRANSFORMATIONS(pEvt);
}

}; // namespace Components
}; // namespace PE
