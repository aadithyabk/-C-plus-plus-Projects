namespace PE {
	namespace Components {

		PE_IMPLEMENT_CLASS1(GhostManager, Component);

		GhostManager::GhostManager(PE::GameContext &context, PE::MemoryArena arena, PE::NetworkContext &netContext, Handle hMyself)
			: Component(context, arena, hMyself)
			
		{
			recordedID=0;
			m_pNetContext = &netContext;
			m_ghostRecord = new GhostRecord;
			m_ghostRecord->m_mask = CharacterControl::NULL_MASK;
		}

		void GhostManager::addDefaultComponents()
		{
			Component::addDefaultComponents();
			PE_REGISTER_EVENT_HANDLER(Events::Event_UPDATE, StreamManager::do_UPDATE);
		}

		int GhostManager::haveGhostsToUpdate()
		{
  			//Check if there ghosts to be updated
  			return m_ghostToBeUpdated.size() > 0;
		}

		void GhostManager::debugRender(int &threadOwnershipMask, float xoffset/* = 0*/, float yoffset/* = 0*/, int clientID)
		{
			//A debug function that displays information about ghosts

			recordedID = clientID+1;
			float dy = 0;
			float dx = 0.3;

			char tmpBuf[1024];
			char tmpBuf2[1024];

			if (m_pContext->getIsServer())
			{
  				sprintf(PEString::s_buf, "Ghosts of Client %d", clientID + 1);
  				DebugRenderer::Instance()->createTextMesh(
  				PEString::s_buf, true, false, false, false, 0,
  				Vector3(0.09 + clientID * 0.65,0.15, 0), 1.0f, threadOwnershipMask);
  
  				sprintf(PEString::s_buf, "GhostID  Position Health");
  				DebugRenderer::Instance()->createTextMesh(
  					PEString::s_buf, true, false, false, false, 0,
  					Vector3(0.09 + clientID * 0.65, 0.18, 0), 1.0f, threadOwnershipMask);
  				
  				int dx = 0;
  				for (int i = 0; i < m_ghostList.size(); i++)
  				{
    					if (m_ghostList[i] != NULL)
    					{
      						if (m_ghostList[i]->m_clientId == clientID)
      						{
        							//print all the ghosts for this client
        							sprintf(PEString::s_buf, "%d", m_ghostList[i]->m_ghostId);
        							DebugRenderer::Instance()->createTextMesh(
        								PEString::s_buf, true, false, false, false, 0,
        								Vector3(0.09 + clientID * 0.65, 0.21 + dx * 0.13, 0), 1.0f, threadOwnershipMask);
        
        							sprintf(PEString::s_buf, "%d", m_ghostList[i]->positionMask);
        							DebugRenderer::Instance()->createTextMesh(
        								PEString::s_buf, true, false, false, false, 0,
        								Vector3(0.18 + clientID * 0.65, 0.21 + dx * 0.13, 0), 1.0f, threadOwnershipMask);
        
        							sprintf(PEString::s_buf, "%d", m_ghostList[i]->healthMask);
        							DebugRenderer::Instance()->createTextMesh(
        								PEString::s_buf, true, false, false, false, 0,
        								Vector3(0.27 + clientID * 0.65, 0.21 + dx * 0.13, 0), 1.0f, threadOwnershipMask);
        
        							dx++;
      						}
    					}
  				}
			 }
		}

		int GhostManager::fillInNextPacket(char *pDataStream, PE::TransmissionRecord *pRecord, int packetSizeAllocated, bool &out_usefulDataSent, bool &out_wantToSendMore)
		{
			//This function copies the contents of already filled transmission record(schedule()) to the packet

			int ghostsReallySent = 0;
			out_usefulDataSent = false;
			out_wantToSendMore = false;

			//Number of ghosts to be updated
			int ghostToUpdate = haveGhostsToUpdate();
			int ghostsReallyUpdated = 0;

			int size = 0;

			//Write this to the packet
			size += StreamManager::WriteInt32(ghostToUpdate, &pDataStream[size]);			//Write the number of ghosts to be updated

			if (ghostToUpdate == 0)
				return size;

			int sizeLeft = packetSizeAllocated - size;

			for (int i = 0; i < ghostToUpdate; ++i)
			{
  				int iGhost = i;
  				assert(iGhost < (int)(m_ghostToBeUpdated.size()));
  				GhostTransmitRecord &ghostTransmitRecord = m_ghostToBeUpdated[iGhost];
  
  				if (ghostTransmitRecord.m_size > sizeLeft)
  				{
  					// can't fit this ghost, break out
  					// note this code can be optimized to include next events that can potentailly fit in
  					out_wantToSendMore = true;
  					break;
  				}

  				// store this to be able to resolve which events were delivered or dropped on transmittion notification
  				pRecord->m_sentGhosts.push_back(ghostTransmitRecord);
  
  				//Copy the payload
  				memcpy(&pDataStream[size], &ghostTransmitRecord.m_payload[0], ghostTransmitRecord.m_size);
  				size += ghostTransmitRecord.m_size;
  				sizeLeft = packetSizeAllocated - size;
  
  				ghostsReallySent++;
  				m_transmitterNumGhostsNotAcked++;
  			}
  
  			if (ghostsReallySent > 0)
  			{
  				m_ghostToBeUpdated.erase(m_ghostToBeUpdated.begin(), m_ghostToBeUpdated.begin() + ghostsReallySent);
  			}
  
  			//write real value into the beginning of event chunk
  			//StreamManager::WriteInt32(ghostsReallySent, &pDataStream[0 /* the number of events is stored in the beginning and we already wrote value into here*/]);
  
  			// we are sending useful data only if we are sending events
  			out_usefulDataSent = ghostsReallySent > 0;
  
  			//return the size of the packet
  			return size;
		}

		void GhostManager::processNotification(TransmissionRecord *pTransmittionRecord, bool delivered)
		{
  			for (unsigned int i = 0; i < pTransmittionRecord->m_sentEvents.size(); ++i)
  			{
    				GhostTransmitRecord &ghostTR = pTransmittionRecord->m_sentGhosts[i];
    
    				if (delivered)
    				{
    					//we're good, can pop this event off front
    					m_transmitterNumGhostsNotAcked--; // will advance sliding window
    				}
    				else
    				{
    						// need to re-adjust our sliding window and make sure we start sending events starting at least this event
      					assert(!"Not supported for now!");
      					m_ghostToBeUpdated.push_front(ghostTR);
      					m_transmitterNumGhostsNotAcked--; // will advance sliding window since we need to resend this event
    				}
  			}
		}

		int GhostManager::receiveNextPacket(char *pDataStream,NetworkContext *receivingContext, bool recentPacketPresent)
		{
			//This function receives the next packet after the relevant data has been extracted by other layers like connection and event manager

			int read = 0;
			PrimitiveTypes::Int32 ghostsToUpdate;

			//number of ghosts to be updated on the client
			read += StreamManager::ReadInt32(&pDataStream[read], ghostsToUpdate);

			for (int i = 0; i < ghostsToUpdate; ++i)
			{
				PrimitiveTypes::Int32 ghostID,stateMask, pType;
				int64_t serverTimeStamp;

				read += StreamManager::ReadInt32(&pDataStream[read], pType);			//Retrieve packet type
				read += StreamManager::ReadInt32(&pDataStream[read], ghostID);			//Retrieve ghostID
				read += StreamManager::ReadInt32(&pDataStream[read], stateMask);		//Retireve stateMask
				read += StreamManager::ReadInt64(&pDataStream[read], serverTimeStamp);	//


				//get the ghost to be updated
				CharacterControl::Components::Ghost *ghostTobeUpdated = GetGhost(ghostID);
				read += ghostTobeUpdated->unpackStateData(stateMask, &pDataStream[read],serverTimeStamp,recentPacketPresent);
			}
			return read; 
		}

		void GhostManager::scheduleGhostForTransmission(int ghostID, int stateMasks,CharacterControl::Components::Object *object, int packetType)
		{
  			//This function creates the record with the given ghostID and statemask
  
  			
  			if (haveGhostsToUpdate() >= PE_MAX_EVENT_JAM)
  			{
  				assert(!"Sending too many events have to drop, need throttling mechanism here");
  				return;
  			}

  			m_ghostToBeUpdated.push_back(GhostTransmitRecord(ghostID, stateMasks));
  			GhostTransmitRecord &back = m_ghostToBeUpdated.back();
  			int dataSize = 0;
  
  			ServerNetworkManager *pNM = (ServerNetworkManager *)(m_pContext->getNetworkManager());
  
  			//indicate the masks that are being set so that the client knows what to receive
  			if (stateMasks & CharacterControl::POSITION_MASK)
  				object->positionMask = 1;
  
  			if (stateMasks & CharacterControl::HEALTH_MASK)
  				object->healthMask = 1;
  
  			dataSize += StreamManager::WriteInt32((int)packetType, &back.m_payload[dataSize]);
  			dataSize += StreamManager::WriteInt32(ghostID, &back.m_payload[dataSize]);
  			dataSize += StreamManager::WriteInt32(stateMasks, &back.m_payload[dataSize]);
  			dataSize += StreamManager::WriteInt64(pNM->serverCurrentTime, &back.m_payload[dataSize]);
  			dataSize += object->packStateData(stateMasks, &back.m_payload[dataSize]);
  			back.m_size = dataSize;

		}

		void GhostManager::CreateGhost(CharacterControl::Components::Ghost* ghostToBeAdded)
		{
		
  			//m_ghostList[ghostToBeAdded->m_ghostId] = ghostToBeAdded;
  			m_ghostList.insert(std::make_pair(ghostToBeAdded->m_ghostId, ghostToBeAdded));
		}

		CharacterControl::Components::Ghost* GhostManager::GetGhost(int ghostID)
		{
			return m_ghostList.find(ghostID)->second;
		}

		void GhostManager::changeGhostOnServer(int changeThisGhost, int nextState)
		{
  			//This function changes the ghost on the server. It is called only on the server
  
  			CharacterControl::Components::Pokemon *ghost = (CharacterControl::Components::Pokemon *)GetGhost(changeThisGhost);
  			ghost->ChangeState((CharacterControl::m_states)nextState);
		}
		
	}
}
