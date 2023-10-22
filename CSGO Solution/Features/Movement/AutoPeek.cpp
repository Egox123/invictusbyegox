#include "AutoPeek.hpp"
#include "../Tools/Tools.hpp"
#include "../Render.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Packet/PacketManager.hpp"

void C_AutoPeek::Instance( )
{
	if ( m_bRetreated )
		m_vecStartPosition.Zero( );
	else if ( g_Tools->IsBindActive( g_Settings->m_aAutoPeek ) )
	{
		if ( !m_bTurnedOn )
		{
			m_bTurnedOn = true;
			m_bWaitAnimationProgress = true;
			m_bNegativeSide = false;
			
			m_flAnimationTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime + .1f;

			m_vecStartPosition = g_Globals.m_LocalPlayer->GetAbsOrigin( );
		}
	}
	else if ( m_bTurnedOn )
	{
		m_bTurnedOn = false;
		m_bWaitAnimationProgress = true;
		m_bNegativeSide = true;

		m_flAnimationTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime + .1f;
	}

	if ( !m_bRetreat )
		if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_ATTACK )
			if ( g_Globals.m_LocalPlayer->m_hActiveWeapon( ) )
				if ( g_Globals.m_LocalPlayer->m_hActiveWeapon( )->IsGun( ) )
					if ( g_Globals.m_LocalPlayer->CanFire( ) )
						m_bRetreat = true;

	if ( !m_bRetreat || !m_bTurnedOn )
		return;
	

	auto vecDifference = g_Globals.m_LocalPlayer->GetAbsOrigin( ) - m_vecStartPosition;
	if ( vecDifference.Length2D( ) <= 5.0f )
	{
		m_bRetreat = false;
		return;
	}
	
	QAngle angWishAngles;
	g_Globals.m_Interfaces.m_EngineClient->GetViewAngles( &angWishAngles );

	float_t flVelocityX = vecDifference.x * cos( angWishAngles.yaw / 180.0f * M_PI) + vecDifference.y * sin( angWishAngles.yaw / 180.0f * M_PI );
	float_t flVelocityY = vecDifference.y * cos( angWishAngles.yaw / 180.0f * M_PI) - vecDifference.x * sin( angWishAngles.yaw / 180.0f * M_PI );

	g_PacketManager->GetModifableCommand( )->m_flForwardMove = -flVelocityX * 20.0f;
	g_PacketManager->GetModifableCommand( )->m_flSideMove = flVelocityY * 20.0f;
}

void C_AutoPeek::DrawCircle( )
{
	if ( !m_bTurnedOn )
	{
		m_vecStartPosition = Vector( 0, 0, 0 );
		return;
	}

	constexpr	float fRadius		= 25.f;	
	static		float fCurrRotation	= 0.0f;
	
	static Vector vecPeekPos	= Vector(0, 0, 0);
	static Vector vecPos		= Vector(0,0,0);

	if ( !m_vecStartPosition.IsZero() )
		vecPos = m_vecStartPosition;
	
	if (vecPos.IsZero())
		return;

	vecPeekPos		= vecPos;
    fCurrRotation	= fCurrRotation + 0.06f;

    g_Globals.m_Interfaces.m_Effects->EnergySplash(
		Vector(fRadius * cos(fCurrRotation) + vecPeekPos.x, fRadius * sin(fCurrRotation) + vecPeekPos.y, vecPeekPos.z), 
		Vector(0, 0, 0), 
		true);

    if (fCurrRotation > DirectX::XM_2PI)
		fCurrRotation = 0.0f;
}