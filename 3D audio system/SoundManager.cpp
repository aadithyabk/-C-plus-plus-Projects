#include "SoundManager.h"
#include "../FileSystem/FileReader.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "../../PrimeEngine/Scene/DrawList.h"
#include "../../PrimeEngine/Scene/SH_DRAW.h"
#include "../../PrimeEngine/Events/Component.h"
#include "../../PrimeEngine/Scene/DebugRenderer.h"


#if APIABSTRACTION_D3D9_PC 
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <xapofx.h>
#pragma comment(lib,"xaudio2.lib")
#else
#include <c:\dxsdk\include\xapofx.h>
#pragma comment(lib,"xapofx.lib")
#endif
#endif


#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif
#if APIABSTRACTION_D3D9_PC
#define XAUDIO2FX_DEBUG 1
#define NOMINMAX
#define INPUTCHANNELS 1
#define OUTPUTCHANNELS 8
#include "../Scene/CameraManager.h"

//Little Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif   


const INT           XMIN = -10;
const INT           XMAX = 10;
const INT           ZMIN = -10;
const INT           ZMAX = 10;
struct AUDIO_STATE
{
	bool bInitialized;

	// XAudio2
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasteringVoice;
	IXAudio2SourceVoice* pSourceVoice;
	IXAudio2SubmixVoice* pSubmixVoice;
	IUnknown* pReverbEffect;
	BYTE* pbSampleData;

	// 3D
	X3DAUDIO_HANDLE x3DInstance;
	int nFrameToApply3DAudio;

	DWORD dwChannelMask;
	UINT32 nChannels;

	X3DAUDIO_DSP_SETTINGS dspSettings;
	X3DAUDIO_LISTENER listener;
	X3DAUDIO_EMITTER emitter;
	X3DAUDIO_CONE emitterCone;

	X3DAUDIO_VECTOR vListenerPos;
	X3DAUDIO_VECTOR vEmitterPos;

	float fListenerAngle;
	bool  fUseListenerCone;
	bool  fUseInnerRadius;
	bool  fUseRedirectToLFE;
	 bool  fUseEmitterCone;
	FLOAT32 emitterAzimuths[INPUTCHANNELS];
	FLOAT32 matrixCoefficients[INPUTCHANNELS * OUTPUTCHANNELS];
};

AUDIO_STATE g_audioState, g_audioStateFoot;


static const X3DAUDIO_CONE Listener_DirectionalCone = { X3DAUDIO_PI*5.0f/6.0f, X3DAUDIO_PI*11.0f/6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };

// Specify LFE level distance curve such that it rolls off much sooner than
// all non-LFE channels, making use of the subwoofer more dramatic.
static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_LFE_CurvePoints[3] = { 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f };
static const X3DAUDIO_DISTANCE_CURVE       Emitter_LFE_Curve          = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_LFE_CurvePoints[0], 3 };

// Specify reverb send level distance curve such that reverb send increases
// slightly with distance before rolling off to silence.

static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_Reverb_CurvePoints[3] = { 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, 0.0f };
static const X3DAUDIO_DISTANCE_CURVE       Emitter_Reverb_Curve          = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_Reverb_CurvePoints[0], 3 };


WAVEFORMAT wfx = {0};
XAUDIO2_BUFFER buffer = {0};
namespace PE {
namespace Components {
Handle SoundManager::s_hInstance;
bool SoundManager::s_isActive;
static float factor1 = 0.0001;
PE_IMPLEMENT_CLASS1(SoundManager, Component);

void SoundManager::do_UPDATE(PE::Events::Event *pEvt)
{
	
}	

void SoundManager::GenerateAudioPathName(PE::GameContext &context, const char *filename, const char *package, const char *assetType, char *out_path, int len)
{
	if (package != NULL && StringOps::length(package) > 0)
	{
#if APIABSTRACTION_IOS || APIABSTRACTION_PS3 || PE_PLAT_IS_PSVITA
		StringOps::concat(context.getMainFunctionArgs()->gameProjRoot(), "AssetsOut/", out_path, len);
		StringOps::concat(out_path, package, out_path, len);
		StringOps::concat(out_path, "/", out_path, len);
		StringOps::concat(out_path, assetType, out_path, len);
		StringOps::concat(out_path, "/", out_path, len);
#else
		StringOps::concat(context.getMainFunctionArgs()->gameProjRoot(), "AssetsOut\\", out_path, len);
		StringOps::concat(out_path, package, out_path, len);
		StringOps::concat(out_path, "\\", out_path, len);
		StringOps::concat(out_path, assetType, out_path, len);
		StringOps::concat(out_path, "\\", out_path, len);
		
#endif
	}
	else
	{
		// if package is not provided default to Default package
#if APIABSTRACTION_IOS
		StringOps::concat(context.getMainFunctionArgs()->gameProjRoot(), "AssetsOut/Default/", out_path, len);
		StringOps::concat(out_path, assetType, out_path, len);
		StringOps::concat(out_path, "/", out_path, len);
#else
		StringOps::concat(context.getMainFunctionArgs()->gameProjRoot(), "AssetsOut\\Default\\", out_path, len);
		StringOps::concat(out_path, assetType, out_path, len);
		StringOps::concat(out_path, "\\", out_path, len);
#endif
	}
	
	StringOps::concat(out_path, filename, out_path, len);

}

HRESULT SoundManager::FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
		return HRESULT_FROM_WIN32( GetLastError() );

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;
		int length = ReadFile( hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL );
		if( 0 == length )
			hr = HRESULT_FROM_WIN32( GetLastError() );

		if( 0 == ReadFile( hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL ) )
			hr = HRESULT_FROM_WIN32( GetLastError() );

		switch (dwChunkType)
		{
		case fourccRIFF: // #define fourccRIFF 'RIFF'
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if( 0 == ReadFile( hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL ) )
				hr = HRESULT_FROM_WIN32( GetLastError() );
			break;

		default:
			if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, dwChunkDataSize, NULL, FILE_CURRENT ) )
			return HRESULT_FROM_WIN32( GetLastError() );            
		}

		dwOffset += sizeof(DWORD) * 2;
		
		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;
		
		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;
	
}

HRESULT SoundManager::ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, bufferoffset, NULL, FILE_BEGIN ) )
		return HRESULT_FROM_WIN32( GetLastError() );
	DWORD dwRead;
	if( 0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL ) )
		hr = HRESULT_FROM_WIN32( GetLastError() );
	return hr;
}


void SoundManager::MSDNtype(BYTE * pDataBuffer, const char* filename)
{
		DWORD dwChunkSize;
		DWORD dwChunkPosition;
		//check the file type, should be fourccWAVE or 'XWMA'
		DWORD filetype;
		static int i = 0;
		GenerateAudioPathName(*m_pContext, filename, "Default", "Sound", PEString::s_buf, PEString::BUF_SIZE);
		
		TCHAR * strFileName = __TEXT(PEString::s_buf);
		HRESULT hr;
		HANDLE hfile = CreateFile(strFileName, GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		
		if (hfile == INVALID_HANDLE_VALUE)
		   {
				hr = HRESULT_FROM_WIN32( GetLastError() );
		   }
		if( INVALID_SET_FILE_POINTER == SetFilePointer( hfile, 0, NULL, FILE_BEGIN ) )
			hr =  HRESULT_FROM_WIN32( GetLastError() );

		HRESULT hr1 = FindChunk(hfile, fourccRIFF,dwChunkSize,dwChunkPosition);
		hr1 = ReadChunkData(hfile,&filetype,sizeof(DWORD),dwChunkPosition);
		if (filetype != fourccWAVE) // #define fourccWAVE 'EVAW'
		 PEINFO("");
		
		hr = FindChunk(hfile,fourccFMT, dwChunkSize, dwChunkPosition );
		hr = ReadChunkData(hfile, &wfx, dwChunkSize, dwChunkPosition );	


		//look for 'RIFF' chunk identifier
		/*inFile.seekg(0, std::ios::beg);
		inFile.read(reinterpret_cast<char*>(&dwChunkId), sizeof(dwChunkId));*/
		SAFE_DELETE_ARRAY(pDataBuffer);


		hr = FindChunk(hfile,fourccDATA,dwChunkSize, dwChunkPosition );
		pDataBuffer = new BYTE[dwChunkSize];
		hr = ReadChunkData(hfile, pDataBuffer, dwChunkSize, dwChunkPosition);

		buffer.AudioBytes = dwChunkSize;  //buffer containing audio data
		buffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer
		i++;
		CloseHandle(hfile);
}


void SoundManager::CallInitAudio(Vector3 pos,Vector3 emitterPos,const float SourceDistance)
{
	InitAudio(pos,emitterPos,SourceDistance);
}

void SoundManager::CallPrepareAudio(SoundManager *soundMan,const char* filename, int loopCount,bool echo)
{
	soundMan->sourcePos = Vector3(0,0,float(ZMAX));
	PrepareAudio(soundMan,filename,loopCount,echo);
}


void SoundManager::CallUpdateAudio(float timeElapsed,Vector3 listenerPos, Vector3 listenerOrient,Vector3 emitterPos, Vector3 emitterOrient)
{
	SoundManager *pSound = SoundManager::Instance();
	pSound->listenerPos.m_x = listenerPos.m_x;
	pSound->listenerPos.m_y = listenerPos.m_y;
	pSound->listenerPos.m_z = listenerPos.m_z;
	
	pSound->listenerOrient.m_x = listenerOrient.m_x;
	pSound->listenerOrient.m_y = listenerOrient.m_y;
	pSound->listenerOrient.m_z = listenerOrient.m_z;

	UpdateAudio(timeElapsed,listenerPos, listenerOrient,emitterPos, emitterOrient);
}

void SoundManager::CallSetReverb(int nReverb)
{
	SetReverb(nReverb);
}

void SoundManager::SetVolume(float volume)
{
	g_audioState.pSourceVoice->SetVolume(volume);
}

void SoundManager::SetFxParameters()
{
	//g_audioState.pSourceVoice->SetEffectParameters(
}

HRESULT SoundManager::InitAudio(Vector3 listenerPos, Vector3 emitterPos, const float SourceDistance)
{
	
	ZeroMemory( &g_audioState, sizeof( AUDIO_STATE ) );

	//
	// Initialize XAudio2
	//
	CoInitializeEx( NULL, COINIT_MULTITHREADED );

	UINT32 flags = 0;
#ifdef _DEBUG
	flags |= XAUDIO2_DEBUG_ENGINE;
#endif

	HRESULT hr;

	if( FAILED( hr = XAudio2Create( &g_audioState.pXAudio2,0, XAUDIO2_DEFAULT_PROCESSOR ) ) )
		return hr;

	//
	// Create a mastering voice
	//
	if( FAILED( hr = g_audioState.pXAudio2->CreateMasteringVoice( &g_audioState.pMasteringVoice ) ) )
	{
		SAFE_RELEASE( g_audioState.pXAudio2 );
		return hr;
	}

	// Check device details to make sure it's within our sample supported parameters
	XAUDIO2_VOICE_DETAILS details;
	g_audioState.pMasteringVoice->GetVoiceDetails( &details ); 
  
	DWORD dwChannelMask;
	g_audioState.pMasteringVoice->GetChannelMask( &dwChannelMask );
	g_audioState.dwChannelMask = dwChannelMask;
	g_audioState.nChannels = details.InputChannels;

	//
	// Create reverb effect
	//
	flags = 0;
#ifdef _DEBUG
	flags |= XAUDIO2FX_DEBUG;
#endif

	if( FAILED( hr = XAudio2CreateReverb( &g_audioState.pReverbEffect, flags ) ) )
	{
		SAFE_RELEASE( g_audioState.pXAudio2 );
		return hr;
	}
	
	// Performance tip: you need not run global FX with the sample number
	// of channels as the final mix.  For example, this sample runs
	// the reverb in mono mode, thus reducing CPU overhead.
	XAUDIO2_EFFECT_DESCRIPTOR effects[] = { { g_audioState.pReverbEffect, TRUE, 1 } };
	XAUDIO2_EFFECT_CHAIN effectChain = { 1, effects };

	if( FAILED( hr = g_audioState.pXAudio2->CreateSubmixVoice( &g_audioState.pSubmixVoice, 1,
															   details.InputSampleRate, 0, 0,
															   NULL, &effectChain ) ) )
	{
		SAFE_RELEASE( g_audioState.pXAudio2 );
		SAFE_RELEASE( g_audioState.pReverbEffect );
		return hr;
	}

	// Set default FX params
	XAUDIO2FX_REVERB_PARAMETERS native;
	ReverbConvertI3DL2ToNative( &g_PRESET_PARAMS[0], &native );
	
	g_audioState.pSubmixVoice->SetEffectParameters( 0, &native, sizeof( native ) );
	
	{

		m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

		sprintf(PEString::s_buf, "Without Occlusion");
		DebugRenderer::Instance()->createTextMesh(
			PEString::s_buf, true, false, false, false, 4500,
			Vector3(0.03f, 0.19f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

		sprintf(PEString::s_buf, "Reflection energy = %d mB (hundredths of decibels)", g_PRESET_PARAMS[0].Reflections);
		DebugRenderer::Instance()->createTextMesh(
			PEString::s_buf, true, false, false, false, 4500,
			Vector3(0.03f, 0.22f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

		sprintf(PEString::s_buf, "Diffusion = %.2f % ", g_PRESET_PARAMS[0].Diffusion);
		DebugRenderer::Instance()->createTextMesh(
			PEString::s_buf, true, false, false, false, 4500,
			Vector3(0.03f, 0.25f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

		sprintf(PEString::s_buf, "Decay Time = %.2f seconds", g_PRESET_PARAMS[0].DecayTime);
		DebugRenderer::Instance()->createTextMesh(
			PEString::s_buf, true, false, false, false, 43500,
			Vector3(0.03f, 0.28f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);



		m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	}

	//
	// Initialize X3DAudio
	//  Speaker geometry configuration on the final mix, specifies assignment of channels
	//  to speaker positions, defined as per WAVEFORMATEXTENSIBLE.dwChannelMask
	//
	//  SpeedOfSound - speed of sound in user-defined world units/second, used
	//  only for doppler calculations, it must be >= FLT_MIN
	//
	const float SPEEDOFSOUND = X3DAUDIO_SPEED_OF_SOUND;

	X3DAudioInitialize( dwChannelMask, SPEEDOFSOUND, g_audioState.x3DInstance );

	g_audioState.vListenerPos = X3DAUDIO_VECTOR( listenerPos.m_x, listenerPos.m_y, listenerPos.m_z );
	//g_audioState.vListenerPos = X3DAUDIO_VECTOR( 0, 0,0 );Vector3(-5.12,0,45.74));
	g_audioState.vEmitterPos = X3DAUDIO_VECTOR(emitterPos.m_x, emitterPos.m_y, emitterPos.m_z);

	SceneNode* _CameraSceneNode = CameraManager::Instance()->getActiveCamera()->getCamSceneNode();
	Vector3 color = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 linepts1[] = {Vector3(emitterPos.m_x,0,emitterPos.m_z), color, Vector3(emitterPos.m_x,50,emitterPos.m_z), color};	
	//DebugRenderer::Instance()->createLineMesh(true, _CameraSceneNode->m_worldTransform, &linepts1[0].m_x, 2, 2000);
	
	g_audioState.fListenerAngle = 0;
	g_audioState.fUseListenerCone = TRUE;
	g_audioState.fUseInnerRadius = TRUE;
	g_audioState.fUseRedirectToLFE = ((dwChannelMask & SPEAKER_LOW_FREQUENCY) != 0);
	g_audioState.fUseEmitterCone = TRUE;
	//
	// Setup 3D audio structs
	//
	g_audioState.listener.Position = g_audioState.vListenerPos;
	g_audioState.listener.OrientFront = X3DAUDIO_VECTOR( 0, 0, 1 );
	g_audioState.listener.OrientTop = X3DAUDIO_VECTOR( 0, 1, 0 );
	g_audioState.listener.pCone = (X3DAUDIO_CONE*)&Listener_DirectionalCone;

	g_audioState.emitter.pCone = &g_audioState.emitterCone;
	g_audioState.emitter.pCone->InnerAngle = 0.0f;
	// Setting the inner cone angles to X3DAUDIO_2PI and
	// outer cone other than 0 causes
	// the emitter to act like a point emitter using the
	// INNER cone settings only.
	g_audioState.emitter.pCone->OuterAngle = 0.0f;
	// Setting the outer cone angles to zero causes
	// the emitter to act like a point emitter using the
	// OUTER cone settings only.
	g_audioState.emitter.pCone->InnerVolume = 0.0f;
	g_audioState.emitter.pCone->OuterVolume = 1.0f;
	g_audioState.emitter.pCone->InnerLPF = 0.0f;
	g_audioState.emitter.pCone->OuterLPF = 1.0f;
	g_audioState.emitter.pCone->InnerReverb = 0.0f;
	g_audioState.emitter.pCone->OuterReverb = 1.0f;

	g_audioState.emitter.Position = g_audioState.vEmitterPos;
	g_audioState.emitter.OrientFront = X3DAUDIO_VECTOR( 0, 0, 1 );
	g_audioState.emitter.OrientTop = X3DAUDIO_VECTOR( 0, 1, 0 );
	g_audioState.emitter.ChannelCount = INPUTCHANNELS;
	g_audioState.emitter.ChannelRadius = 1.0f;
	g_audioState.emitter.pChannelAzimuths = g_audioState.emitterAzimuths;

	// Use of Inner radius allows for smoother transitions as
	// a sound travels directly through, above, or below the listener.
	// It also may be used to give elevation cues.
	g_audioState.emitter.InnerRadius = 2.0f;
	g_audioState.emitter.InnerRadiusAngle = X3DAUDIO_PI/4.0f;;

	g_audioState.emitter.pVolumeCurve = (X3DAUDIO_DISTANCE_CURVE*)&X3DAudioDefault_LinearCurve;
	g_audioState.emitter.pLFECurve    = (X3DAUDIO_DISTANCE_CURVE*)&Emitter_LFE_Curve;
	g_audioState.emitter.pLPFDirectCurve = NULL; // use default curve
	g_audioState.emitter.pLPFReverbCurve = NULL; // use default curve
	g_audioState.emitter.pReverbCurve    = (X3DAUDIO_DISTANCE_CURVE*)&Emitter_Reverb_Curve;
	g_audioState.emitter.CurveDistanceScaler = SourceDistance; //Change this for Distance
	g_audioState.emitter.DopplerScaler = 1.0f;

	g_audioState.dspSettings.SrcChannelCount = INPUTCHANNELS;
	g_audioState.dspSettings.DstChannelCount = g_audioState.nChannels;
	g_audioState.dspSettings.pMatrixCoefficients = g_audioState.matrixCoefficients;

	//
	// Done
	//
	g_audioState.bInitialized = true;

	return S_OK;
}


HRESULT SoundManager::PrepareAudio(SoundManager* soundManager,const char* filename,int loopCount,bool echo)
{
	if( !g_audioState.bInitialized )
			return E_FAIL;

	static const  char* prevFile = "sample";

	/*if(strcmp(prevFile, filename)==0)
			return S_OK;*/

	 if( g_audioState.pSourceVoice )
	{
		g_audioState.pSourceVoice->Stop( 0 );
		g_audioState.pSourceVoice->DestroyVoice();
		g_audioState.pSourceVoice = 0;
	}
	 //soundManager->currentFile = filename;

	 XAUDIO2_SEND_DESCRIPTOR sendDescriptors[2];
	sendDescriptors[0].Flags = XAUDIO2_SEND_USEFILTER; // LPF direct-path
	sendDescriptors[0].pOutputVoice = g_audioState.pMasteringVoice;
	sendDescriptors[1].Flags = XAUDIO2_SEND_USEFILTER; // LPF reverb-path -- omit for better performance at the cost of less realistic occlusion
	sendDescriptors[1].pOutputVoice = g_audioState.pSubmixVoice;
	const XAUDIO2_VOICE_SENDS sendList = { 2, sendDescriptors };

	BYTE* buff = NULL;
	soundManager->MSDNtype(buff,filename);
	HRESULT hr;
	if(hr = FAILED(g_audioState.pXAudio2->CreateSourceVoice( &g_audioState.pSourceVoice, (WAVEFORMATEX*)&wfx, 0,2.0f, NULL, &sendList )))
		hr = HRESULT_FROM_WIN32( GetLastError() );
	
	IUnknown * pXAPO;
	CreateFX(__uuidof(FXReverb), &pXAPO);
	FXECHO_PARAMETERS XAPOParameters;

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
	if (echo)
	{
		hr = CreateFX(_uuidof(FXEcho), &pXAPO, NULL, 0);
		XAUDIO2_EFFECT_DESCRIPTOR descriptor;
		descriptor.InitialState = true;
		descriptor.OutputChannels = 1;
		descriptor.pEffect = pXAPO;
		XAUDIO2_EFFECT_CHAIN chain;
		chain.EffectCount = 1;
		chain.pEffectDescriptors = &descriptor;
		g_audioState.pSourceVoice->SetEffectChain(&chain);
		pXAPO->Release();

		XAPOParameters.Delay = 900.0f;//FXECHO_DEFAULT_DELAY;
		XAPOParameters.WetDryMix = FXECHO_DEFAULT_WETDRYMIX;
		XAPOParameters.Feedback = 0.3f;// FXECHO_DEFAULT_FEEDBACK;
		g_audioState.pSourceVoice->SetEffectParameters(0, &XAPOParameters, sizeof(FXECHO_PARAMETERS));
	}
#else
	hr = CreateFX(_uuidof(...), &effect);




#endif

	XAUDIO2_BUFFER buffer_new = {0};
	buffer_new = buffer;
	delete buff;
	//buffer_new.pAudioData = g_audioState.pbSampleData;
	buffer_new.Flags = XAUDIO2_END_OF_STREAM;
	buffer_new.LoopCount = loopCount;


	
	 g_audioState.pSourceVoice->SubmitSourceBuffer( &buffer_new );

	 g_audioState.pSourceVoice->Start( 0 );

	g_audioState.nFrameToApply3DAudio = 0;

	prevFile = filename;

	return S_OK;
}

HRESULT SoundManager::UpdateAudio( float fElapsedTime,Vector3 listenerPos,Vector3 listenerOrient,Vector3 emitterPos,Vector3 emitterOrient)
{
	static bool yHasChanged = false;
	
	if( !g_audioState.bInitialized )
		return S_FALSE;
	float *volume;
	 
	//g_audioState.pSourceVoice->GetVolume(volume);

	g_audioState.vListenerPos.x = listenerPos.m_x;
	g_audioState.vListenerPos.y = listenerPos.m_y;
	g_audioState.vListenerPos.z = listenerPos.m_z;

	g_audioState.vEmitterPos.x = emitterPos.m_x;
	g_audioState.vEmitterPos.y = emitterPos.m_y;
	g_audioState.vEmitterPos.z = emitterPos.m_z;
	if( g_audioState.nFrameToApply3DAudio == 0 )
	{
		
		// Calculate listener orientation in x-z plane
		if( g_audioState.vListenerPos.x != g_audioState.listener.Position.x
			|| g_audioState.vListenerPos.y != g_audioState.listener.Position.y
			   || g_audioState.vListenerPos.z != g_audioState.listener.Position.z )
		{
			

			Vector3 vlisPos = Vector3(g_audioState.vListenerPos.x,g_audioState.vListenerPos.y,g_audioState.vListenerPos.z);
			Vector3 lisPos = Vector3(g_audioState.listener.Position.x,g_audioState.listener.Position.y,g_audioState.listener.Position.z);
			X3DAUDIO_VECTOR res =  X3DAUDIO_VECTOR((vlisPos.m_x - lisPos.m_x),(vlisPos.m_y - lisPos.m_y),(vlisPos.m_z - lisPos.m_z));
			X3DAUDIO_VECTOR vDelta = res;

			g_audioState.fListenerAngle = float( atan2( vDelta.x, vDelta.z ) );

			Vector3 vDelt = Vector3(vDelta.x, vDelta.y, vDelta.z);
			vDelt.normalize();
			vDelta.x = vDelt.m_x;
			vDelta.y = vDelt.m_y;
			vDelta.z = vDelt.m_z;

			g_audioState.listener.OrientFront.x = vDelta.x;
			g_audioState.listener.OrientFront.y = vDelta.y;
			g_audioState.listener.OrientFront.z = vDelta.z;
			
			if (g_audioState.vListenerPos.y != g_audioState.listener.Position.y)
				yHasChanged = true;
		}
		else
		{
			//Rotating at place
			g_audioState.listener.OrientFront.x = -listenerOrient.m_x;
			g_audioState.listener.OrientFront.y = -listenerOrient.m_y;
			g_audioState.listener.OrientFront.z = -listenerOrient.m_z;
			
		}
	
		if (g_audioState.fUseListenerCone)
		{
			g_audioState.listener.pCone = (X3DAUDIO_CONE*)&Listener_DirectionalCone;
		}
		else
		{
			g_audioState.listener.pCone = NULL;
		}
		
		
		
		if (g_audioState.fUseInnerRadius)
		{
			g_audioState.emitter.InnerRadius = 2.0f;
			g_audioState.emitter.InnerRadiusAngle = X3DAUDIO_PI/4.0f;
		}
		else
		{
			g_audioState.emitter.InnerRadius = 0.0f;
			g_audioState.emitter.InnerRadiusAngle = 0.0f;
		}

		if( fElapsedTime > 0 )
		{
			Vector3 vlisPos = Vector3(g_audioState.vListenerPos.x,g_audioState.vListenerPos.y,g_audioState.vListenerPos.z);
			Vector3 lisPos = Vector3(g_audioState.listener.Position.x,g_audioState.listener.Position.y,g_audioState.listener.Position.z);
			 X3DAUDIO_VECTOR res =  X3DAUDIO_VECTOR((vlisPos.m_x - lisPos.m_x)/ fElapsedTime,(vlisPos.m_y - lisPos.m_y)/ fElapsedTime,(vlisPos.m_z - lisPos.m_z)/ fElapsedTime);
			X3DAUDIO_VECTOR lVelocity = ( res );
			g_audioState.listener.Position = g_audioState.vListenerPos;
			g_audioState.listener.Velocity = lVelocity;

			Vector3 vemiPos = Vector3(g_audioState.vEmitterPos.x,g_audioState.vEmitterPos.y,g_audioState.vEmitterPos.z);
			Vector3 emiPos = Vector3(g_audioState.emitter.Position.x,g_audioState.emitter.Position.y,g_audioState.emitter.Position.z);
			 X3DAUDIO_VECTOR res1 =  X3DAUDIO_VECTOR((vemiPos.m_x - emiPos.m_x)/ fElapsedTime,(vemiPos.m_y - emiPos.m_y)/ fElapsedTime,(vemiPos.m_z - emiPos.m_z)/ fElapsedTime);
			X3DAUDIO_VECTOR eVelocity = res1;
			g_audioState.emitter.Position = g_audioState.vEmitterPos;
			g_audioState.emitter.Velocity = eVelocity;
		}

		DWORD dwCalcFlags = X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER
			| X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB
			| X3DAUDIO_CALCULATE_REVERB;
		if (g_audioState.fUseRedirectToLFE)
		{
			// On devices with an LFE channel, allow the mono source data
			// to be routed to the LFE destination channel.
			dwCalcFlags |= X3DAUDIO_CALCULATE_REDIRECT_TO_LFE;
		}

		X3DAudioCalculate( g_audioState.x3DInstance, &g_audioState.listener, &g_audioState.emitter, dwCalcFlags,
						   &g_audioState.dspSettings );

		IXAudio2SourceVoice* voice = g_audioState.pSourceVoice;
		XAUDIO2_FILTER_TYPE filter;
		
		 factor1 = factor1 + 20;
		
		if( voice )
		{
			//if( factor1 < 0.6)
			if(GetAsyncKeyState('1')& 0x8000)
			{
				//Stone Room reverb
				voice->SetFrequencyRatio(500);
			}
			
			

			// Apply X3DAudio generated DSP settings to XAudio2
			voice->SetFrequencyRatio( g_audioState.dspSettings.DopplerFactor );
			voice->SetOutputMatrix( g_audioState.pMasteringVoice, INPUTCHANNELS, g_audioState.nChannels,
									g_audioState.matrixCoefficients );

			float length = Vector3(fabs(emitterPos.m_x - listenerPos.m_x), fabs(emitterPos.m_y - listenerPos.m_y), fabs(emitterPos.m_z - listenerPos.m_z)).length();

			m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

			sprintf(PEString::s_buf, "Varies Inversely with Distance to emitter ", g_audioState.matrixCoefficients[0]);
			DebugRenderer::Instance()->createTextMesh(
				PEString::s_buf, true, false, false, false, 1,
				Vector3(0.406f, 0.03f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

			sprintf(PEString::s_buf, "Varies depending on Orientation ", g_audioState.matrixCoefficients[0]);
			DebugRenderer::Instance()->createTextMesh(
				PEString::s_buf, true, false, false, false, 1,
				Vector3(0.406f, 0.06f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

			sprintf(PEString::s_buf, "Distance to Source = %.2f ", g_audioState.dspSettings.EmitterToListenerDistance);
			DebugRenderer::Instance()->createTextMesh(
				PEString::s_buf, true, false, false, false, 1,
				Vector3(0.406f, 0.09f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);


			sprintf(PEString::s_buf, "Sum of L and R speakers != 1 because of attenuation ", g_audioState.matrixCoefficients[0]);
			DebugRenderer::Instance()->createTextMesh(
				PEString::s_buf, true, false, false, false, 1,
				Vector3(0.400f, 0.12f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

			sprintf(PEString::s_buf, "Left Speaker Percentage = %.2f ",g_audioState.matrixCoefficients[0]);
			DebugRenderer::Instance()->createTextMesh(
				PEString::s_buf, true, false, false, false, 1,
				Vector3(0.406f, 0.15f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);


			sprintf(PEString::s_buf, "Right Speaker percentage = %.2f ", g_audioState.matrixCoefficients[1]);
			DebugRenderer::Instance()->createTextMesh(
				PEString::s_buf, true, false, false, false, 1,
				Vector3(0.406f, 0.18f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

			m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);


			

			voice->SetOutputMatrix(g_audioState.pSubmixVoice, 1, 1, &g_audioState.dspSettings.ReverbLevel);
			
				filter = BandPassFilter;
				m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);

				

				sprintf(PEString::s_buf, "Direct Cofficient = %.2f ", g_audioState.dspSettings.LPFDirectCoefficient);
				DebugRenderer::Instance()->createTextMesh(
					PEString::s_buf, true, false, false, false, 1,
					Vector3(0.03f, 0.30f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

				sprintf(PEString::s_buf, "Reverb Cofficient = %.2f ", g_audioState.dspSettings.LPFReverbCoefficient);
				DebugRenderer::Instance()->createTextMesh(
					PEString::s_buf, true, false, false, false, 1,
					Vector3(0.03f, 0.33f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);


				sprintf(PEString::s_buf, "Reverb Level = %.2f ", g_audioState.dspSettings.ReverbLevel);
				DebugRenderer::Instance()->createTextMesh(
					PEString::s_buf, true, false, false, false, 1,
					Vector3(0.03f, 0.36f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

				m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
		
			
			XAUDIO2_FILTER_PARAMETERS FilterParametersDirect = { filter, 2.0f * sinf(X3DAUDIO_PI/6.0f * g_audioState.dspSettings.LPFDirectCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
			voice->SetOutputFilterParameters(g_audioState.pMasteringVoice, &FilterParametersDirect);
			XAUDIO2_FILTER_PARAMETERS FilterParametersReverb = {filter, 2.0f * sinf(X3DAUDIO_PI/6.0f * g_audioState.dspSettings.LPFReverbCoefficient), 1.0f }; // see XAudio2CutoffFrequencyToRadians() in XAudio2.h for more information on the formula used here
			voice->SetOutputFilterParameters(g_audioState.pSubmixVoice, &FilterParametersReverb);
		}
	}
	
	g_audioState.nFrameToApply3DAudio++;
	g_audioState.nFrameToApply3DAudio &= 1;
	
	return S_OK;
}

HRESULT SoundManager::SetReverb( int nReverb )
{
	if( !g_audioState.bInitialized )
		return S_FALSE;

	if( nReverb < 0 || nReverb >= NUM_PRESETS )
		return E_FAIL;

	if( g_audioState.pSubmixVoice )
	{
		XAUDIO2FX_REVERB_PARAMETERS native;
		ReverbConvertI3DL2ToNative( &g_PRESET_PARAMS[ nReverb ], &native );
		g_audioState.pSubmixVoice->SetEffectParameters( 0, &native, sizeof( native ) );
	}
	//Display parameters
	{

		m_pContext->getGPUScreen()->AcquireRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
		
		sprintf(PEString::s_buf, "With Occlusion");
		DebugRenderer::Instance()->createTextMesh(
			PEString::s_buf, true, false, false, false, 3500,
			Vector3(0.66f, 0.21f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);


		sprintf(PEString::s_buf, "Reflection Energy = %d mB ", g_PRESET_PARAMS[nReverb].Reflections);
		DebugRenderer::Instance()->createTextMesh(
		PEString::s_buf, true, false, false, false, 3500,
		Vector3(0.66f, 0.24f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

		sprintf(PEString::s_buf, "Diffusion = %.2f % ", g_PRESET_PARAMS[nReverb].Diffusion);
		DebugRenderer::Instance()->createTextMesh(
			PEString::s_buf, true, false, false, false, 3500,
			Vector3(0.66f, 0.27f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);

		sprintf(PEString::s_buf, "Decay Time = %.2f seconds", g_PRESET_PARAMS[nReverb].DecayTime);
		DebugRenderer::Instance()->createTextMesh(
			PEString::s_buf, true, false, false, false, 3500,
			Vector3(0.66f, 0.30f, 0), 1.0f, m_pContext->m_gameThreadThreadOwnershipMask);
		


		m_pContext->getGPUScreen()->ReleaseRenderContextOwnership(m_pContext->m_gameThreadThreadOwnershipMask);
	}
	return S_OK;
}

void SoundManager::CleanupAudioFoot()
{
	if( !g_audioStateFoot.bInitialized )
		return;

	if( g_audioStateFoot.pSourceVoice )
	{
		g_audioStateFoot.pSourceVoice->DestroyVoice();
		g_audioStateFoot.pSourceVoice = NULL;
	}

	if( g_audioStateFoot.pSubmixVoice )
	{
		g_audioStateFoot.pSubmixVoice->DestroyVoice();
		g_audioStateFoot.pSubmixVoice = NULL;
	}

	if( g_audioStateFoot.pMasteringVoice )
	{
		g_audioStateFoot.pMasteringVoice->DestroyVoice();
		g_audioStateFoot.pMasteringVoice = NULL;
	}

	g_audioStateFoot.pXAudio2->StopEngine();
	SAFE_RELEASE( g_audioStateFoot.pXAudio2 );
	SAFE_RELEASE( g_audioStateFoot.pReverbEffect );

	SAFE_DELETE_ARRAY( g_audioStateFoot.pbSampleData );

	g_audioStateFoot.bInitialized = false;
}

void SoundManager::CleanupAudio()
{
	if( !g_audioState.bInitialized )
		return;

	if( g_audioState.pSourceVoice )
	{
		g_audioState.pSourceVoice->DestroyVoice();
		g_audioState.pSourceVoice = NULL;
	}

	if( g_audioState.pSubmixVoice )
	{
		g_audioState.pSubmixVoice->DestroyVoice();
		g_audioState.pSubmixVoice = NULL;
	}

	if( g_audioState.pMasteringVoice )
	{
		g_audioState.pMasteringVoice->DestroyVoice();
		g_audioState.pMasteringVoice = NULL;
	}

	g_audioState.pXAudio2->StopEngine();
	SAFE_RELEASE( g_audioState.pXAudio2 );
	SAFE_RELEASE( g_audioState.pReverbEffect );

	SAFE_DELETE_ARRAY( g_audioState.pbSampleData );

	g_audioState.bInitialized = false;
}
}; // namespace Components
}; // namespace PE
#endif

