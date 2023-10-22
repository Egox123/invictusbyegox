#include "HitMarker.hpp"
#include "../Settings.hpp"
#include "../Render.hpp"

void C_HitMarker::Instance( )
{
	if ( !g_Settings->m_bHitMarker )
	{
		if ( !m_aHitMarkers.empty( ) )
			m_aHitMarkers.clear( );

		return;
	}

	if ( m_aHitMarkers.empty( ) )
		return;

	float_t flTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	for ( int i = 0; i < m_aHitMarkers.size( ); i++ )
	{
		if ( flTime - m_aHitMarkers[i] > 1.0f )
		{
			m_aHitMarkers.erase( m_aHitMarkers.begin( ) + i );
			continue;
		}

		int Width, Height;
		g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(Width, Height);

		Width /= 2;
		Height /= 2;

		g_Render->RenderLine(Width + 5, Height - 5, Width + 12, Height - 12, g_Settings->m_HitmarkerColor, 1.0f);
		g_Render->RenderLine(Width + 5, Height + 5, Width + 12, Height + 12, g_Settings->m_HitmarkerColor, 1.0f);
		g_Render->RenderLine(Width - 5, Height + 5, Width - 12, Height + 12, g_Settings->m_HitmarkerColor, 1.0f);
		g_Render->RenderLine(Width - 5, Height - 5, Width - 12, Height - 12, g_Settings->m_HitmarkerColor, 1.0f);
	}
}

void C_HitMarker::OnRageBotFire( Vector vecOrigin )
{
	if (g_Settings->m_bEffectMarker)
		g_Globals.m_Interfaces.m_Effects->Sparks(vecOrigin, 5, 1);

	if ( !g_Settings->m_bHitMarker )
	{
		if ( !m_aHitMarkers.empty( ) )
			m_aHitMarkers.clear( );

		return;
	}

	m_aHitMarkers.emplace_back(g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime);
}