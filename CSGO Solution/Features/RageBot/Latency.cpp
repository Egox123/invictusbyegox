#include "Latency.hpp"
#include "../../SDK/Globals.hpp"
#include "../../Tools/Tools.hpp"

static std::deque<IncomingSequence> m_Sequences;

void C_ExtendedBacktrack::UpdateSequences() 
{
	static int lastincomingsequencenumber = 0;

	if ( g_Globals.m_Interfaces.m_ClientState )
	{
		C_NetChannel* NetChan = g_Globals.m_Interfaces.m_ClientState->m_pNetChannel();

		if ( NetChan )
		{
			if ( NetChan->m_iInSequenceNr > lastincomingsequencenumber )
			{
				lastincomingsequencenumber = NetChan->m_iInSequenceNr;

				IncomingSequence ActiveSequence;

				ActiveSequence.flRealTime = m_Globals()->m_flRealTime;
				ActiveSequence.nInReliableState = NetChan->m_iInReliableState;
				ActiveSequence.nOutReliableState = NetChan->m_iOutReliableState;
				ActiveSequence.nSequenceNr = NetChan->m_iInSequenceNr;


				m_Sequences.push_front(ActiveSequence);
			}

			if ( m_Sequences.size() > 2048 )
				m_Sequences.pop_back();
		}
	}
}

void C_ExtendedBacktrack::AddBacktrackingWindow(C_NetChannel* pChannel, float flDelta)
{
	float_t iWantedPing = g_Settings->m_iWantedPing / 1000.f;

	for ( auto& Seq : m_Sequences )
	{
		if ( m_Globals()->m_flRealTime - Seq.flRealTime >= flDelta )
		{
			pChannel->m_iInReliableState = Seq.nInReliableState;
			pChannel->m_iInSequenceNr = Seq.nSequenceNr;
			break;
		}
	}
}