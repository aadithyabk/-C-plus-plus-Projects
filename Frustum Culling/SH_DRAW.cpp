//THis is the drawCalls function  to render static meshes.

void SingleHandler_DRAW::do_GATHER_DRAWCALLS(Events::Event *pEvt)
{
	Vector3 nearTopLeft, farTopLeft,nearTopRight, farTopRight, nearLowerLeft, nearLowerRight, farLowerLeft, farLowerRight;
	
	// the mesh is who we are drawing for is previous distributor
	// and this object is current distributor
	Component *pCaller = pEvt->m_prevDistributor.getObject<Component>();
	Mesh *pMeshCaller = (Mesh *)pCaller;

	
	if (pMeshCaller->m_instances.m_size == 0)
		return; // no instances of this mesh
	Events::Event_GATHER_DRAWCALLS *pDrawEvent = NULL;
	Events::Event_GATHER_DRAWCALLS_Z_ONLY *pZOnlyDrawEvent = NULL;

	if (pEvt->isInstanceOf<Events::Event_GATHER_DRAWCALLS>())
		pDrawEvent = (Events::Event_GATHER_DRAWCALLS *)(pEvt);
	else
		pZOnlyDrawEvent = (Events::Event_GATHER_DRAWCALLS_Z_ONLY *)(pEvt);
    
	pMeshCaller->m_numVisibleInstances = pMeshCaller->m_instances.m_size;
	int numInstances = pMeshCaller->m_instances.m_size;// assume all instances are visible
    
    	// check for bounding volumes here and mark each instance as visible or not visible and set m_numVisibleInstances to number of visible instances
    
	Vector3 color(0.5f, 0.0f, 0.0f);
			
	float xmax= 0.0,ymax= 0.0,xmin= 0.0,ymin = 0.0,zmin = 0.0,zmax = 0.0;
	int maxX = 5; // maybe need more to get framerate lower
	Vector3 linepts[100];
	Matrix4x4 m,bbox;
	m.loadIdentity();
				
	m.importScale(5.0f, 5.0f, 5.0f);
	Vector3 pos0,pos1,pos2,pos3,pos4,pos5,pos6,pos7;
	Vector3 points[7];
	//Check if culling is to be done
	 if ( pMeshCaller->m_performBoundingVolumeCulling)
	 {
		 pMeshCaller->m_numVisibleInstances = 0;
        
		//Loop through all the meshes
        	for (int iInst = 0; iInst < pMeshCaller->m_instances.m_size; ++iInst)
        	{
	                MeshInstance *pInst = pMeshCaller->m_instances[iInst].getObject<MeshInstance>();
			
			xmax= pMeshCaller->m_hPositionBufferCPU.getObject<PositionBufferCPU>()->xmax;
			xmin= pMeshCaller->m_hPositionBufferCPU.getObject<PositionBufferCPU>()->xmin;
			ymax= pMeshCaller->m_hPositionBufferCPU.getObject<PositionBufferCPU>()->ymax;
			ymin= pMeshCaller->m_hPositionBufferCPU.getObject<PositionBufferCPU>()->ymin;
			zmax= pMeshCaller->m_hPositionBufferCPU.getObject<PositionBufferCPU>()->zmax;
			zmin= pMeshCaller->m_hPositionBufferCPU.getObject<PositionBufferCPU>()->zmin;
			
			Handle hParent1SN = pInst->getFirstParentByType<SceneNode>();
			SceneNode *myScene = hParent1SN.getObject<SceneNode>();	
	
			//Get the 8 points for the bounding box
			pos0 = Vector3(xmin,ymin,zmin);
			pos1 = Vector3(xmax,ymin,zmin);
			pos3 = Vector3(xmax,ymax,zmin);
			pos2 = Vector3(xmin,ymax,zmin);
			pos4 = Vector3(xmin,ymin,zmax);
			pos5 = Vector3(xmax,ymin,zmax);
			pos6 = Vector3(xmax,ymax,zmax);
			pos7 = Vector3(xmin,ymax,zmax);
	
			Matrix4x4 base = myScene->m_worldTransform;
			Vector3 resultantPosition0 =  base * pos0;
			Vector3 resultantPosition1 =  base * pos1;
			Vector3 resultantPosition2 =  base * pos2;
			Vector3 resultantPosition3 =  base * pos3;
			Vector3 resultantPosition4 =  base * pos4;
			Vector3 resultantPosition5 =  base * pos5;
			Vector3 resultantPosition6 =  base * pos6;
			Vector3 resultantPosition7 =  base * pos7;
	
			CameraSceneNode *pCam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
			nearTopLeft = pCam->m_nearTopLeft;
			nearTopRight = pCam->m_nearTopRight;
			nearLowerLeft = pCam->m_nearLowerLeft;
			nearLowerRight = pCam->m_nearLowerRight;
			farTopLeft = pCam->m_farTopLeft;
			farTopRight = pCam->m_farTopRight;
			farLowerLeft = pCam->m_farLowerLeft;
			farLowerRight = pcam->m_farLowerRight;
						
			//Obtain Planes
				
			//Left plane
			plane leftPlane = plane();
			leftPlane.CalculateEquationOfPlane(nearTopLeft, nearLowerLeft, farTopLeft);
		
			//Near plane
			plane nearPlane = plane();
			nearPlane.CalculateEquationOfPlane( nearLowerRight , nearLowerLeft, nearTopRight);
	
			//Right plane
			plane rightPlane = plane();
			rightPlane.CalculateEquationOfPlane(farLowerRight, nearLowerRight, farTopRight);
		
			//Bottom plane
			plane bottomPlane = plane();
			bottomPlane.CalculateEquationOfPlane(nearLowerLeft, nearLowerRight, farLowerLeft);
	
			//Far plane
			plane farPlane = plane();
			farPlane.CalculateEquationOfPlane(farTopLeft, farLowerLeft,  farTopRight);
	
			//Top plane
			plane topPlane = plane();
			topPlane.CalculateEquationOfPlane(nearTopRight, nearTopLeft, farTopRight);
				
				
			//Check if any point of the bounding box is outside the frustum
				
			bool result1 =  point.DetermineInsideOrOutside(pos_res0, pos_res1, pos_res2, pos_res3, pos_res4, pos_res5, pos_res7, pos_res7, nearLowerLeft, leftPlane );
			bool result2 =  point.DetermineInsideOrOutside(pos_res0, pos_res1, pos_res2, pos_res3, pos_res4, pos_res5, pos_res7, pos_res7, nearLowerRight, rightPlane );	
			bool result3 =  point.DetermineInsideOrOutside(pos_res0, pos_res1, pos_res2, pos_res3, pos_res4, pos_res5, pos_res7, pos_res7, nearTopLeft, topPlane );
			bool result4 =  point.DetermineInsideOrOutside(pos_res0, pos_res1, pos_res2, pos_res3, pos_res4, pos_res5, pos_res7, pos_res7, nearLowerRight, bottomPlane );
			bool result5 =  point.DetermineInsideOrOutside(pos_res0, pos_res1, pos_res2, pos_res3, pos_res4, pos_res5, pos_res7, pos_res7, nearTopLeft, nearPlane );
			bool result6 =  point.DetermineInsideOrOutside(pos_res0, pos_res1, pos_res2, pos_res3, pos_res4, pos_res5, pos_res7, pos_res7, farTopLeft, farPlane );
				
	
			if(!result1 && !result2 && !result3 && !result4 && !result5 && !result6)
	            	{
				//Present Inside the frustum
						
				pInst->m_culledOut = false;
				++pMeshCaller->m_numVisibleInstances;
				drawLine(pMeshCaller, myScene,pos_res0, pos_res1, pos_res2, pos_res3, pos_res4, pos_res5, pos_res6, pos_res7);		//Draw bounding box for this mesh
	        	}
	           	else
	        	{
				//Needs to be culled
	        		  pInst->m_culledOut = true;
	        	}
        	}	
        }
    }
}


