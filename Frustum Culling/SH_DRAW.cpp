
PE_IMPLEMENT_SINGLETON_CLASS1(SingleHandler_DRAW, Component);

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
	int ipt = 0;

	
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
			
			ipt = 0;
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
			Vector3 pos_res0 = pos_res0 = base * pos0;
			Vector3 pos_res1 = pos_res1 = base * pos1;
			Vector3 pos_res2 = pos_res2 = base * pos2;
			Vector3 pos_res3 = pos_res3 = base * pos3;
			Vector3 pos_res4 = pos_res4 = base * pos4;
			Vector3 pos_res5 = pos_res5 = base * pos5;
			Vector3 pos_res6 = pos_res6 = base * pos6;
			Vector3 pos_res7 = pos_res7 = base * pos7;

				
		    
			CameraSceneNode *pcam = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
			nearTopLeft = pcam->m_nearTopLeft;
			nearTopRight = pcam->m_nearTopRight;
			nearLowerLeft = pcam->m_nearLowerLeft;
			nearLowerRight = pcam->m_nearLowerRight;
			farTopLeft = pcam->m_farTopLeft;
			farTopRight = pcam->m_farTopRight;
			farLowerLeft = pcam->m_farLowerLeft;
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
				ipt = 0;
            }
           else
            {
				//Needs to be culled
                 pInst->m_culledOut = true;
            }
			
        }
    }
    

	DrawList *pDrawList = pDrawEvent ? DrawList::Instance() : DrawList::ZOnlyInstance();
	
    //dbg
    //SceneNode *pRoot = RootSceneNode::Instance();

	// index buffer
	Handle hIBuf = pMeshCaller->m_hIndexBufferGPU;
	IndexBufferGPU *pibGPU = hIBuf.getObject<IndexBufferGPU>();

	// Check for vertex buffer(s)
	Handle hVertexBuffersGPU[4]; // list of bufers to pass to GPU
	Vector4 vbufWeights;
	int numVBufs = pMeshCaller->m_vertexBuffersGPUHs.m_size;
	assert(numVBufs < 4);
	for (int ivbuf = 0; ivbuf < numVBufs; ivbuf++)
	{
		hVertexBuffersGPU[ivbuf] = pMeshCaller->m_vertexBuffersGPUHs[ivbuf];
		vbufWeights.m_values[ivbuf] = hVertexBuffersGPU[ivbuf].getObject<VertexBufferGPU>()->m_weight;
	}

	if (numVBufs > 1)
	{
		for (int ivbuf = numVBufs; ivbuf < 4; ivbuf++)
		{
			hVertexBuffersGPU[ivbuf] = hVertexBuffersGPU[0];
			vbufWeights.m_values[ivbuf] = vbufWeights.m_values[0];
		}
		numVBufs = 4; // blend shape shader works with 4 shapes. so we extend whatever slots with base
	}

	// Check for material set
	if (!pMeshCaller->m_hMaterialSetGPU.isValid())
		return;

	GPUMaterialSet *pGpuMatSet = pMeshCaller->m_hMaterialSetGPU.getObject<GPUMaterialSet>();

	Matrix4x4 projectionViewWorldMatrix = pDrawEvent ? pDrawEvent->m_projectionViewTransform : pZOnlyDrawEvent->m_projectionViewTransform;

	Handle hParentSN = pCaller->getFirstParentByType<SceneNode>();
	if (!hParentSN.isValid())
	{
		// allow skeleton to be in chain
		 hParentSN = pCaller->getFirstParentByTypePtr<SkeletonInstance>()->getFirstParentByType<SceneNode>();
		 
	}
	Matrix4x4 worldMatrix;
	worldMatrix.loadIdentity();

	if (hParentSN.isValid())
		worldMatrix = hParentSN.getObject<SceneNode>()->m_worldTransform;
	
	projectionViewWorldMatrix = projectionViewWorldMatrix * worldMatrix;

	// draw all pixel ranges with different materials
	PrimitiveTypes::UInt32 numRanges = MeshHelpers::getNumberOfRangeCalls(pibGPU);
	
	for (PrimitiveTypes::UInt32 iRange = 0; iRange < numRanges; iRange++)
	{
		gatherDrawCallsForRange(pMeshCaller, pDrawList, &hVertexBuffersGPU[0], numVBufs, vbufWeights, iRange, pDrawEvent, pZOnlyDrawEvent);
	}
}


void SingleHandler_DRAW::drawLine(Mesh *pMeshCaller, SceneNode *myScene, Vector3 pos_res0, Vector3 pos_res1, Vector3 pos_res2, 
	Vector3 pos_res3, Vector3 pos_res4, Vector3 pos_res5, Vector3 pos_res6, Vector3 pos_res7)
{
			//Draw the bounding box for the mesh
			
			Vector3 pos0,pos1,pos2,pos3,pos4,pos5,pos6,pos7;
			int ipt = 0;
			Vector3 linepts[100];
			Vector3 color(0.5f,0.0f, 0.0f);

					
			Matrix4x4 base = myScene->m_worldTransform;
			linepts[ipt++] = pos_res0; linepts[ipt++] = color;
			linepts[ipt++] = pos_res1; linepts[ipt++] = color;
			linepts[ipt++] = pos_res1; linepts[ipt++] = color;
			linepts[ipt++] = pos_res3; linepts[ipt++] = color;
			linepts[ipt++] = pos_res0; linepts[ipt++] = color;
			linepts[ipt++] = pos_res2; linepts[ipt++] = color;
			linepts[ipt++] = pos_res2; linepts[ipt++] = color;
			linepts[ipt++] = pos_res3; linepts[ipt++] = color;
			linepts[ipt++] = pos_res0; linepts[ipt++] = color;
			linepts[ipt++] = pos_res4; linepts[ipt++] = color;
			linepts[ipt++] = pos_res1; linepts[ipt++] = color;
			linepts[ipt++] = pos_res5; linepts[ipt++] = color;
			linepts[ipt++] = pos_res4; linepts[ipt++] = color;
			linepts[ipt++] = pos_res5; linepts[ipt++] = color;
			linepts[ipt++] = pos_res4; linepts[ipt++] = color;
			linepts[ipt++] = pos_res7; linepts[ipt++] = color;
			linepts[ipt++] = pos_res5; linepts[ipt++] = color;
			linepts[ipt++] = pos_res6; linepts[ipt++] = color;
			linepts[ipt++] = pos_res6; linepts[ipt++] = color;
			linepts[ipt++] = pos_res7; linepts[ipt++] = color;
			linepts[ipt++] = pos_res2; linepts[ipt++] = color;
			linepts[ipt++] = pos_res7; linepts[ipt++] = color;
			linepts[ipt++] = pos_res3; linepts[ipt++] = color;
			linepts[ipt++] = pos_res6; linepts[ipt++] = color;
			
			DebugRenderer::Instance()->createLineMesh(false, base, &linepts[0].m_x, 24, 0);

}


}; // namespace Components
}; // namespace PE





