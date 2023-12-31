#include "Damage.hpp"
#include "../Settings.hpp"
#include "../Render.hpp"
#include "../SDK/Math/Math.hpp"

void C_DamageMarker::Instance( )
{
	if ( !g_Settings->m_bDamageMarker )
	{
		if ( !m_aDmgMarkers.empty( ) )
			m_aDmgMarkers.clear( );

		return;
	}

	if ( m_aDmgMarkers.empty( ) )
		return;

	float_t flTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;

	for (int i = 0; i < m_aDmgMarkers.size(); i++)
	{
		auto Marker = &m_aDmgMarkers[i];

		if (Marker->m_flTime < flTime)
		{
			m_aDmgMarkers.erase(m_aDmgMarkers.begin() + i);
			continue;
		}

		Marker->m_vecOrigin.z = Math::Lerp(9 / 1.5f * g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime, Marker->m_vecOrigin.z, Marker->m_flEndZ);

		Vector vecScreenPosition = Vector(0, 0, 0);
		g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition(Marker->m_vecOrigin, vecScreenPosition);

		g_Render->RenderText(
			std::to_string( (int)Marker->m_flDamage ), 
			ImVec2(vecScreenPosition.x, vecScreenPosition.y),
			g_Settings->m_OnHitDamage,
			false, 
			true, 
			g_Globals.m_Fonts.m_Damage
		);
	}
}

void C_DamageMarker::OnRageBotFire( Vector vecOrigin, float_t flDamage )
{
	if ( !g_Settings->m_bDamageMarker )
	{
		if ( !m_aDmgMarkers.empty( ) )
			m_aDmgMarkers.clear( );

		return;
	}

	DamageMarker_t& DmgMarker = m_aDmgMarkers.emplace_back( );
	DmgMarker.m_flTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime + 3.f;
	DmgMarker.m_vecOrigin = vecOrigin;
	DmgMarker.m_flDamage = flDamage;
	DmgMarker.m_flEndZ = vecOrigin.z + 140;
}