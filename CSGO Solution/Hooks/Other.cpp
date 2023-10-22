#include "../Hooks.hpp"
#include "../Features/Packet/PacketManager.hpp"
#include "../Features/Model/Model.hpp"
#include "../Features/Exploits/Exploits.hpp"
#include "../Features/Networking/Networking.hpp"
#include "../Features/RageBot/Latency.hpp"

void __cdecl C_Hooks::hkCL_Move( float_t flFrametime, bool bIsFinalTick )
{
	g_Networking->UpdateLatency( );
	g_Networking->StartNetwork( );

	if ( !g_ExploitSystem->IsAllowedToRun( ) )
		return;

	g_Globals.m_Hooks.m_Originals.m_CL_Move( flFrametime, g_ExploitSystem->IsFinalTick( ) );
	
	g_ExploitSystem->PerformPackets( );
	return g_Networking->FinishNetwork( );
}

bool __fastcall C_Hooks::hkDispatchUserMessage( LPVOID pEcx, uint32_t, int32_t iMessageType, int32_t iArgument, int32_t iSecondArgument, LPVOID pData )
{
	if ( iMessageType == CS_UM_TextMsg || iMessageType == CS_UM_HudMsg || iMessageType == CS_UM_SayText )
		if ( g_Settings->m_bAdBlock && !( *( g_Globals.m_Interfaces.m_GameRules ) )->IsValveDS( ) )
			return true;

	return g_Globals.m_Hooks.m_Originals.m_DispatchUserMessage( pEcx, iMessageType, iArgument, iSecondArgument, pData );
}

void __fastcall C_Hooks::hkPerformScreenOverlay( LPVOID pEcx, uint32_t, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight )
{
	if ( !g_Settings->m_bAdBlock || ( *( g_Globals.m_Interfaces.m_GameRules ) )->IsValveDS( ) )
		return g_Globals.m_Hooks.m_Originals.m_PerformScreenOverlay( pEcx, x, y, iWidth, iHeight );
}

void __cdecl C_Hooks::hkShouldDrawFOG( )
{
	if ( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_FOG ] ) 
		return g_Globals.m_Hooks.m_Originals.m_ShouldDrawFog( );
}

void C_Hooks::hkFlashDuration( CRecvProxyData* Data, LPVOID pStruct, LPVOID pOut )
{
	if ( g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_FLASH ] )
	{
		*( float_t* )( pOut ) = 0.0f;
		return;
	}

	return g_Globals.m_Hooks.m_Originals.m_FlashDuration->GetOriginal( )( Data, pStruct, pOut );
}

bool __cdecl C_Hooks::hkHost_ShouldRun( )
{
	return true;
}

#include "../Tools/Tools.hpp"

struct CIncomingSequence {
	int InSequence;
	int ReliableState;
};

//std::vector<CIncomingSequence> IncomingSequences;

int __fastcall C_Hooks::hkSendDataGram(C_NetChannel* pNetwork, LPVOID pEcx, bf_write* pDataGram)
{
	if ( !g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
		return g_Globals.m_Hooks.m_Originals.m_SendDataGram(pNetwork, pDataGram);

	if ( pDataGram )
		return g_Globals.m_Hooks.m_Originals.m_SendDataGram(pNetwork, pDataGram);

	if ( !g_Tools->IsBindActive(g_Settings->m_aPingSpike) )
		return g_Globals.m_Hooks.m_Originals.m_SendDataGram(pNetwork, pDataGram);

	/*auto v55 = (C_NetChannelInfo*)(pNetwork);
	auto v10 = pNetwork->m_iInSequenceNr;
	auto v16 = pNetwork->m_iInReliableState;
	auto v17 = v55->GetLatency(FLOW_OUTGOING);
	
	if ( v17 < g_Settings->m_iWantedPing ) {
		auto v13 = pNetwork->m_iInSequenceNr - TIME_TO_TICKS( ((float_t)g_Settings->m_iWantedPing / 1000.f) - v17);
		pNetwork->m_iInSequenceNr = v13;
		for ( auto& seq : IncomingSequences ) {
			if ( seq.InSequence != v13 )
				continue;

			pNetwork->m_iInReliableState = seq.ReliableState;
		}
	}

	auto aResult = g_Globals.m_Hooks.m_Originals.m_SendDataGram(pNetwork, pDataGram);;
	pNetwork->m_iInSequenceNr = v10;
	pNetwork->m_iInReliableState = v16;

	return aResult;*/

	int iInState = pNetwork->m_iInReliableState;
	int iInSequenceNr = pNetwork->m_iInSequenceNr;

	g_Backtrack->AddBacktrackingWindow(pNetwork, g_Settings->m_iWantedPing / 1000.f);

	int aResult = g_Globals.m_Hooks.m_Originals.m_SendDataGram(pNetwork, pDataGram);;

	pNetwork->m_iInReliableState = iInState;
	pNetwork->m_iInSequenceNr = iInSequenceNr;

	return aResult;
}

void __fastcall C_Hooks::hkProcessPacket(C_NetChannel* pNetwork, LPVOID pEcx, LPVOID pPacket, bool pHeader)
{
	g_Globals.m_Hooks.m_Originals.m_ProcessPacket(pNetwork, pPacket, pHeader);

	/*IncomingSequences.push_back(CIncomingSequence{ pNetwork->m_iInSequenceNr, pNetwork->m_iInReliableState });
	for ( auto it = IncomingSequences.begin(); it != IncomingSequences.end(); ++it ) {
		auto delta = abs(pNetwork->m_iInSequenceNr - it->InSequence);
		if ( delta > 128 ) {
			it = IncomingSequences.erase(it);
		}
	}*/

	// get this from CL_FireEvents string "Failed to execute event for classId" in engine.dll
	for ( C_EventInfo* it{ g_Globals.m_Interfaces.m_ClientState->m_aEvents() }; it != nullptr; it = it->m_pNext ) {
		if ( !it->m_iClassID )
			continue;

		// set all delays to instant.
		it->m_flFireDelay = 0.f;
	}

	// game events are actually fired in OnRenderStart which is WAY later after they are received
	// effective delay by lerp time, now we call them right after theyre received (all receive proxies are invoked without delay).
	g_Globals.m_Interfaces.m_EngineClient->FireEvents();
}

void __cdecl C_Hooks::hkSequenceProxy(CRecvProxyData* Proxy, LPVOID pStruct, LPVOID pOut)
{
	SkinChanger::SequenceRemapping(Proxy, static_cast<C_BaseViewModel*>(pStruct));
	g_Globals.m_Hooks.m_Originals.m_SequenceProxy->GetOriginal()(Proxy, pStruct, pOut);
}

void __cdecl C_Hooks::hkSimulationTime(CRecvProxyData* Proxy, void* pStruct, void* pOut)
{
	C_BaseEntity* pEntity = static_cast <C_BaseEntity*> (pStruct);
	if ( !pEntity || !pEntity->IsPlayer() )
		return g_Globals.m_Hooks.m_Originals.m_SimulationTime->GetOriginal()(Proxy, pStruct, pOut);

	if ( Proxy->m_Value.m_Int == 0 )
		return;

	g_Globals.m_Hooks.m_Originals.m_SimulationTime->GetOriginal()(Proxy, pStruct, pOut);
}