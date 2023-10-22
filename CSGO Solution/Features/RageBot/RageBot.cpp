#include "RageBot.hpp"
#include "Autowall.hpp"
#include "Antiaim.hpp"

#include "../Markers/HitMarker.hpp"
#include "../Markers/Damage.hpp"
#include "../Packet/PacketManager.hpp"

#include "../Exploits/Exploits.hpp"
#include "../Tools/Tools.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Prediction/EnginePrediction.hpp"
#include "../Visuals/ShotChams.hpp"
#include "../Log Manager/LogManager.hpp"
#include "../Animations/LocalAnimations.hpp"
#include "../Visuals/World.hpp"
#include "../Networking/Networking.hpp"
#include "../Settings.hpp"
#include "../Animations/Animations.hpp"
#include "RayTracer.h"

void C_RageBot::Instance( )
{
	if ( !g_Globals.m_LocalPlayer )
		return;

	if ( !g_Globals.m_LocalPlayer->IsAlive() )
		return;

	if (g_Globals.m_AccuracyData.m_bDoingSecondShot)
	{
		if (g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex() == WEAPON_AWP || (g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex() == WEAPON_SSG08) || !g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->CanShift())
		{
			g_Globals.m_AccuracyData.m_bDoingSecondShot = false;
		}
	}

	if ( !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( ) || !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->GetWeaponData( ) )
		return;

	g_Globals.m_RageData.m_CurrentTarget = C_TargetData( );
	if ( ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN ) /*|| ( *g_Globals.m_Interfaces.m_GameRules )->IsFreezePeriod( )*/ )
		return;

	if ( !g_Globals.m_AccuracyData.m_bCanFire_Default || !g_Globals.m_AccuracyData.m_bCanFire_Shift )
	{
		g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_ATTACK;
		return;
	}
	
	if ( !g_Settings->m_bEnabledRage )
		return;

	// knife is included too.
	if ( g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( ) == WEAPON_TASER || g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->IsKnife() )
		return this->TaserBot( );

	/*if ( g_Globals.m_AccuracyData.m_bForceAutoPeek && g_Globals.m_AccuracyData.m_flStopTimer )
	{
		if ( g_Globals.m_AccuracyData.m_flStopTimer > g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime )
			return;

		g_Globals.m_AccuracyData.m_bForceAutoPeek = false;
		g_Globals.m_AccuracyData.m_flStopTimer = 0.f;
	}
	else 
	{
		g_Globals.m_AccuracyData.m_bForceAutoPeek = false;
		g_Globals.m_AccuracyData.m_flStopTimer = 0.f;
	}*/

	this->AutoRevolver( );
	if ( !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->IsGun( ) )
		return;
	
	if ( g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER )
	{
		if ( !g_Globals.m_LocalPlayer->CanFire( NULL, true ) )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_ATTACK;
			return;
		}
	}

	this->ScanPlayers( );
	if ( !g_Globals.m_RageData.m_CurrentTarget.m_Player )
		return;

FINISH_RAGE:
	this->AutoStop();	

	float_t flCalculatedHitchance = this->GetHitChance(g_Globals.m_RageData.m_CurrentTarget);
	int32_t nHitChance = m_RageSettings.m_iHitChance;
	if ( flCalculatedHitchance < nHitChance )
	{
		if ( g_Globals.m_LocalPlayer->m_hActiveWeapon( )->IsSniper( ) )
		{
			if ( !g_Globals.m_LocalPlayer->m_bIsScoped( ) )
			{
				this->AutoScope( );

				g_PredictionSystem->RestoreNetvars( g_PacketManager->GetModifableCommand( )->m_nCommand );

				g_PredictionSystem->Repredict( );

				flCalculatedHitchance = this->GetHitChance( g_Globals.m_RageData.m_CurrentTarget );
				if ( flCalculatedHitchance < nHitChance )
					return;
			}
			else 
				return;
		}
		else
			return;
	}

	QAngle angCalculatedAngle = Math::CalcAngle( g_Globals.m_LocalData.m_vecShootPosition, g_Globals.m_RageData.m_CurrentTarget.m_Hitbox.m_vecPoint );

	if ( !g_Globals.m_Packet.m_bFakeDuck )
		g_PacketManager->GetModifablePacket( ) = true;

	/*if ( g_Tools->IsBindActive(g_Settings->m_aAutoPeek) )
		if ( g_Tools->IsBindActive(g_Settings->m_aDoubleTap) )
			if ( g_Globals.m_LocalPlayer->m_hActiveWeapon()->IsSniper() )
				if ( g_Globals.m_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex( ) != WEAPON_SCAR20 )
					if ( g_Globals.m_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex( ) != WEAPON_G3SG1 )
						g_Globals.m_AccuracyData.m_bForceAutoPeek = true;
	
	if ( g_Globals.m_AccuracyData.m_bForceAutoPeek )
		if ( !g_Globals.m_AccuracyData.m_flStopTimer )
			g_Globals.m_AccuracyData.m_flStopTimer = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime + 0.1f;*/

	g_PacketManager->GetModifableCommand( )->m_nTickCount = TIME_TO_TICKS( g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_SimulationTime + g_LagCompensation->GetLerpTime( ) );
	g_PacketManager->GetModifableCommand( )->m_angViewAngles = angCalculatedAngle;
	g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_ATTACK;

	if ( g_Globals.m_ConVars.m_WeaponRecoilScale->GetFloat( ) > 0.0f )
		g_PacketManager->GetModifableCommand( )->m_angViewAngles -= g_Globals.m_ConVars.m_WeaponRecoilScale->GetFloat( ) * g_Globals.m_LocalPlayer->m_aimPunchAngle( );

	C_ShotData ShotData;

	ShotData.m_Target = g_Globals.m_RageData.m_CurrentTarget;
	ShotData.m_vecStartPosition = g_Globals.m_LocalData.m_vecShootPosition;
	ShotData.m_vecEndPosition = g_Globals.m_RageData.m_CurrentTarget.m_Hitbox.m_vecPoint;
	ShotData.m_iShotTick = g_Networking->GetServerTick( );
	ShotData.m_bHasMaximumAccuracy = this->HasMaximumAccuracy( );

	if (g_Globals.m_AccuracyData.m_bDoingSecondShot)
	{
		g_Globals.m_ShotData.emplace_back(ShotData);
		g_Globals.m_AccuracyData.m_bDoingSecondShot = false;
		return;
	};

	g_Globals.m_ShotData.emplace_back(ShotData);
	g_Globals.m_AccuracyData.m_bDoingSecondShot = false;

	return g_ShotChams->OnRageBotFire( g_Globals.m_RageData.m_CurrentTarget.m_Player );
}

void C_RageBot::UpdatePeekState( )
{
	g_Globals.m_Peek.m_bIsPeeking = false;
	if ( g_Globals.m_LocalPlayer->m_vecVelocity( ).Length2D( ) < 5.40f )
		return;

	int nScannedTargets = 0;
	if ( m_nLastPeekID >= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients )
		m_nLastPeekID = 1;

	C_RageSettings Settings;
	std::memcpy( &Settings, &m_RageSettings, sizeof( C_RageSettings ) );

	// set hitboxes
	m_RageSettings.m_RageModifiers[ 0 ] = false;
	m_RageSettings.m_RageModifiers[ 1 ] = false;
	m_RageSettings.m_RageModifiers[ 2 ] = false;

	m_RageSettings.m_SafeModifiers[ 0 ] = false;
	m_RageSettings.m_SafeModifiers[ 1 ] = false;
	m_RageSettings.m_SafeModifiers[ 2 ] = false;
	m_RageSettings.m_SafeModifiers[ 3 ] = false;
	m_RageSettings.m_SafeModifiers[ 4 ] = false;

	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_HEAD] = false;
	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_U_CHEST] = false;
	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_L_CHEST] = false;
	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_CHEST] = false;
	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_PELVIS] = true;
	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_STOMACH] = true;
	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_ARMS] = false;
	m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_LEGS] = false;

	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_HEAD] = false;
	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_U_CHEST] = false;
	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_L_CHEST] = false;
	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_CHEST] = false;
	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_PELVIS] = false;
	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_STOMACH] = false;
	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_ARMS] = false;
	m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_LEGS] = false;

	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_HEAD] = false;
	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_U_CHEST] = false;
	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_L_CHEST] = false;
	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_CHEST] = false;
	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_PELVIS] = true;
	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_STOMACH] = true;
	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] = false;
	m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] = false;

	// disable delay things
	m_RageSettings.m_iHeadScale = 100;
	m_RageSettings.m_iBodyScale = 100;

	for ( ; m_nLastPeekID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; m_nLastPeekID++ )
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex( m_nLastPeekID );
		if ( !Player || !Player->IsPlayer( ) || !Player->IsAlive( ) || Player->IsDormant( ) || Player->m_iTeamNum( ) == g_Globals.m_LocalPlayer->m_iTeamNum( ) )
			continue;

		if ( Player->m_bGunGameImmunity( ) || ( Player->m_fFlags( ) & FL_FROZEN ) )
			continue;

		const auto m_LagRecords = g_Globals.m_CachedPlayerRecords[ m_nLastPeekID ];
		if ( m_LagRecords.empty( ) )
			continue;

		if ( nScannedTargets > 2 )
			break;

		// force player
		this->AdjustPlayerRecord( Player, m_LagRecords.back( ) );
	
		// next shoot pos
		Vector vecNextShootPosition = g_Globals.m_LocalData.m_vecShootPosition + ( g_Globals.m_LocalPlayer->m_vecVelocity( ) * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick ) * 4.0f;

		// scan record from next position
		C_HitboxData HitboxData = this->ScanPlayerRecord( Player, m_LagRecords.back( ), vecNextShootPosition );
		if ( !HitboxData.m_flDamage )
		{
			// increase
			nScannedTargets++;

			// we didn't find the player that we peek
			continue;
		}

		// we are actually peeking somebody
		g_Globals.m_Peek.m_Player = Player;
		g_Globals.m_Peek.m_bIsPeeking = true;

		break;
	}
	
	// restore settings
	std::memcpy( &m_RageSettings, &Settings, sizeof( C_RageSettings ) );
}

void C_RageBot::ScanPlayers( )
{
	int nScannedTargets = 0;
	if ( m_nLastRageID >= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients )
		m_nLastRageID = 1;

	for ( ; m_nLastRageID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; m_nLastRageID++ )
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( m_nLastRageID );
		if ( !pPlayer || !pPlayer->IsPlayer( ) || !pPlayer->IsAlive( ) || pPlayer->m_iTeamNum( ) == g_Globals.m_LocalPlayer->m_iTeamNum( ) )
			continue;
		
		if ( pPlayer->m_bGunGameImmunity( ) || ( pPlayer->m_fFlags( ) & FL_FROZEN ) )
			continue;

		const auto m_LagRecords = g_Globals.m_CachedPlayerRecords[ m_nLastRageID ];
		if ( m_LagRecords.empty( ) )
			continue;

		if ( nScannedTargets > 2 )
			break;

		Vector vecShootPosition = g_Globals.m_LocalData.m_vecShootPosition;

		if ((g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex() == WEAPON_TASER
			|| g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->IsKnife())
			&& g_Globals.m_LocalPlayer->m_vecOrigin().DistTo(pPlayer->m_vecOrigin()) > g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->GetWeaponData()->m_flRange)
			return;

		C_LagRecord LagRecord = this->GetFirstAvailableRecord( pPlayer );
		if ( !g_LagCompensation->IsValidTime( LagRecord.m_SimulationTime ) )
		{
			if ( m_RageSettings.m_AutoStopOptions[ AUTOSTOP_EARLY ] && m_RageSettings.m_bAutoStop )
			{
				this->AdjustPlayerRecord( pPlayer, m_LagRecords.back( ) );

				C_HitboxData HitboxData = this->ScanPlayerRecord( pPlayer, m_LagRecords.back( ), vecShootPosition );
				if ( HitboxData.m_flDamage >= this->GetMinDamage( pPlayer ) )
				{
					float_t flDistance = g_Globals.m_LocalPlayer->m_vecOrigin( ).DistTo( pPlayer->m_vecOrigin( ) );
					if ( flDistance )
					{
						int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( );
						if ( ( nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1 ) || flDistance > 430.0f )
							this->AutoScope( );
					}

					if ( this->CanAutoStop( ) )
						g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;
				}
			}

			continue;
		}

		this->AdjustPlayerRecord( pPlayer, LagRecord );
		if ( m_RageSettings.m_AutoStopOptions[ AUTOSTOP_EARLY ] && m_RageSettings.m_bAutoStop )
		{
			C_HitboxData HitboxData = this->ScanPlayerRecord( pPlayer, LagRecord, vecShootPosition );
			if ( HitboxData.m_flDamage >= this->GetMinDamage( pPlayer ) )
			{
				float_t flDistance = g_Globals.m_LocalPlayer->m_vecOrigin( ).DistTo( pPlayer->m_vecOrigin( ) );
				if ( flDistance )
				{
					int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( );
					if ( ( nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1 ) || flDistance > 430.0f )
						this->AutoScope( );
				}

				if ( this->CanAutoStop( ) )
					g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;
			}
		}

		C_HitboxData HitboxData = this->ScanPlayerRecord( pPlayer, LagRecord, vecShootPosition );
		if ( HitboxData.m_flDamage >= this->GetMinDamage( pPlayer ) )
		{
			g_Globals.m_RageData.m_CurrentTarget.m_Hitbox = HitboxData;
			g_Globals.m_RageData.m_CurrentTarget.m_LagRecord = LagRecord;
			g_Globals.m_RageData.m_CurrentTarget.m_Player = pPlayer;

			if ( this->CanAutoStop( ) )
				g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;

			float_t flDistance = g_Globals.m_LocalPlayer->m_vecOrigin( ).DistTo( pPlayer->m_vecOrigin( ) );
			if ( flDistance )
			{
				int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( );
				if ( ( nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1 ) || flDistance > 430.0f )
					this->AutoScope( );
			}

			this->AdjustPlayerRecord( g_Globals.m_RageData.m_CurrentTarget.m_Player, g_Globals.m_RageData.m_CurrentTarget.m_LagRecord );
			break;
		}
		
		C_LagRecord BTRecord = C_LagRecord( );
		C_HitboxData BTHitscan = C_HitboxData( );
		
		if ( !this->FindPlayerRecord( pPlayer, &BTRecord, &BTHitscan ) )
		{
			nScannedTargets++;

			this->AdjustPlayerRecord( pPlayer, m_LagRecords.back( ) );
			continue;
		}

		g_Globals.m_RageData.m_CurrentTarget.m_Player = pPlayer;
		g_Globals.m_RageData.m_CurrentTarget.m_LagRecord = BTRecord;
		g_Globals.m_RageData.m_CurrentTarget.m_Hitbox = BTHitscan;
		
		if ( this->CanAutoStop( ) )
			g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;
		
		float_t flDistance = g_Globals.m_LocalPlayer->GetAbsOrigin( ).DistTo( pPlayer->GetAbsOrigin( ) );
		if ( flDistance )
		{
			int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( );
			if ( ( nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1 ) || flDistance > 450.0f )
				this->AutoScope( );
		}

		this->AdjustPlayerRecord( g_Globals.m_RageData.m_CurrentTarget.m_Player, g_Globals.m_RageData.m_CurrentTarget.m_LagRecord );
		break;
	}
}

bool C_RageBot::FindPlayerRecord( C_BasePlayer* pPlayer, C_LagRecord* OutRecord, C_HitboxData* OutHitbox )
{
	bool bDidScanLastRecord = false;
	for ( auto LagRecord : g_Globals.m_CachedPlayerRecords[ pPlayer->EntIndex( ) ] )
	{
		if ( !g_LagCompensation->IsValidTime( LagRecord.m_SimulationTime ) )
			continue;

		// wtf
		Vector vecShootPosition = g_Globals.m_LocalData.m_vecShootPosition;
		if ( !LagRecord.m_bIsShooting && bDidScanLastRecord )
			continue;
		
		bDidScanLastRecord = true;
		this->AdjustPlayerRecord( pPlayer, LagRecord );

		C_HitboxData HitboxData = this->ScanPlayerRecord( pPlayer, LagRecord, vecShootPosition );
		if ( HitboxData.m_flDamage < this->GetMinDamage( pPlayer ) )
			continue;

		*OutRecord = LagRecord;
		*OutHitbox = HitboxData;

		return true;
	}

	return false;
}

int32_t C_RageBot::GetHitgroupFromHitbox( int32_t iHitbox )
{
	int32_t iHitgroup = 0;
	switch ( iHitbox )
	{
		case HITBOX_HEAD:
			iHitgroup = HITGROUP_HEAD;
		break;

		case HITBOX_CHEST:
			iHitgroup = HITGROUP_CHEST;
		break;

		case HITBOX_PELVIS:
			iHitgroup = HITGROUP_GENERIC;
		break;

		case HITBOX_LEFT_CALF:
		case HITBOX_LEFT_FOOT:
		case HITBOX_LEFT_THIGH:
			iHitgroup = HITGROUP_LEFTLEG;
		break;

		case HITBOX_RIGHT_CALF:
		case HITBOX_RIGHT_FOOT:
		case HITBOX_RIGHT_THIGH:
			iHitgroup = HITGROUP_RIGHTLEG;
		break;

		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_HAND:
		case HITBOX_LEFT_FOREARM:
			iHitgroup = HITGROUP_LEFTARM;
		break;

		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_HAND:
		case HITBOX_RIGHT_FOREARM:
			iHitgroup = HITGROUP_RIGHTARM;
		break;

		case HITBOX_STOMACH:
			iHitgroup = HITGROUP_STOMACH;
		break;

		default: break;
	}

	return iHitgroup;
}

bool SortHitboxes( C_HitboxHitscanData First, C_HitboxHitscanData Second )
{
	if ( First.m_iHitbox == HITBOX_HEAD )
		return false;
	else if ( Second.m_iHitbox == HITBOX_HEAD )
		return true;

	return First.m_flWeaponDamage > Second.m_flWeaponDamage;
}

bool WaitOverlap(C_BasePlayer* pPlayer)
{
	if (!(pPlayer->m_fFlags() & FL_ONGROUND))
		return true;

	if (pPlayer->m_vecVelocity().Length2D() > 6.5f)
	{
		auto pState = pPlayer->m_PlayerAnimStateCSGO();

		if ( pState->m_flMoveWeight <= 0.8 )
			return true;

		auto flSpeedModifer = std::clamp(pState->m_flSpeedAsPortionOfWalkTopSpeed, 0.0f, 1.0f);
		auto flAverageSpeedModifier = (pState->m_flWalkToRunTransition * -0.3f - 0.2f) * flSpeedModifer + 1.0f;

		if ( pState->m_flAnimDuckAmount ) //-V550
		{
			auto flMaxVelocity = std::clamp(pState->m_flSpeedAsPortionOfCrouchTopSpeed, 0.0f, 1.0f);
			auto flDuckSpeed = pState->m_flAnimDuckAmount * flMaxVelocity;

			flAverageSpeedModifier += flDuckSpeed * (0.5f - flAverageSpeedModifier);
		}

		// clamped to velocity.
		float flMaxDelta = min(pState->m_flYawDesyncAdjustment() * flAverageSpeedModifier, 58.f);

		if ( flMaxDelta > 40 )
			return false;
	}

	return true;
}

void C_RageBot::SaveMovementData()
{
	//m_flForwardMove = g_PacketManager->GetModifableCommand()->m_flForwardMove;
	//m_flSideMove = g_PacketManager->GetModifableCommand()->m_flSideMove;
}

void C_RageBot::ForceMovementData()
{
	//if (!g_Globals.m_AccuracyData.m_bRestoreAutoStop)
	//	return;

	//g_PacketManager->GetModifableCommand()->m_flForwardMove = m_flForwardMove;
	//g_PacketManager->GetModifableCommand()->m_flSideMove = m_flSideMove;

	//g_PredictionSystem->RestoreNetvars(g_PacketManager->GetModifableCommand()->m_nCommand);
	//return g_PredictionSystem->Repredict();
}

C_HitboxData C_RageBot::ScanPlayerRecord( C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition )
{
	C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !pCombatWeapon )
		return C_HitboxData( );

	C_CSWeaponData* pWeaponData = pCombatWeapon->GetWeaponData( );
	if ( !pWeaponData )
		return C_HitboxData( );

	if ( m_RageSettings.m_RageModifiers[2] )
		if ( !WaitOverlap(pPlayer) )
			return C_HitboxData( );

	std::vector < C_HitboxHitscanData > aHitboxesData;
	
	bool bForcedSafety = g_Tools->IsBindActive( g_Settings->m_aSafePoint );

	if ( !bForcedSafety )
	{
		if ( !(pPlayer->m_fFlags()) & FL_ONGROUND )
		{
			bForcedSafety = m_RageSettings.m_SafeModifiers[3];
		}
		else
		{
			float_t flVelocity = pPlayer->m_vecVelocity().Length2D();

			if ( flVelocity < 6.f )
			{
				bForcedSafety = m_RageSettings.m_SafeModifiers[0];
			}
			else
			{
				if ( pPlayer->m_PlayerAnimStateCSGO()->m_flMoveWeight <= 0.8 )
					bForcedSafety = m_RageSettings.m_SafeModifiers[2];
				else 
					bForcedSafety = m_RageSettings.m_SafeModifiers[1];
			}
		}			
	}

	if ( m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_HEAD] )
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_HEAD, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_HEAD] || bForcedSafety ) );
	
	if ( m_RageSettings.m_Hitboxes[ RAGE_HITBOX_ID::POINT_CHEST ] )
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_CHEST, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_CHEST] || bForcedSafety ) );	
	
	if ( m_RageSettings.m_Hitboxes[ RAGE_HITBOX_ID::POINT_U_CHEST ] )
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_UPPER_CHEST, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_U_CHEST] || bForcedSafety ) );
	
	if ( m_RageSettings.m_Hitboxes[ RAGE_HITBOX_ID::POINT_L_CHEST] )
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_LOWER_CHEST, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_L_CHEST] || bForcedSafety ) );
	
	if ( m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_ARMS] )
	{
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_LEFT_FOREARM, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_ARMS] || bForcedSafety ) );
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_RIGHT_FOREARM, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_ARMS] || bForcedSafety ) );
	}

	if ( m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_PELVIS] )
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_PELVIS, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_PELVIS] || bForcedSafety ) );

	if ( m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_STOMACH] )
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_STOMACH, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_STOMACH] || bForcedSafety ) );
		
	if ( m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_LEGS] )
	{
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_LEFT_THIGH, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_LEGS] || bForcedSafety ) );
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_RIGHT_THIGH, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_LEGS] || bForcedSafety ) );
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_LEFT_CALF, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_LEGS] || bForcedSafety ) );
		aHitboxesData.emplace_back( C_HitboxHitscanData( HITBOX_RIGHT_CALF, m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_LEGS] || bForcedSafety ) );
	}
	
	int32_t iMinDamage = this->GetMinDamage( pPlayer );
	std::vector < C_HitboxData > m_ScanData;
	for ( auto Hitbox : aHitboxesData )
	{
		std::vector < Vector > m_Hitboxes = this->GetHitboxPoints( pPlayer, LagRecord, vecStartPosition, Hitbox.m_iHitbox );
		if ( m_Hitboxes.empty( ) )
			continue;

		for ( auto Point : m_Hitboxes )
		{
			float_t flDamage = g_AutoWall->GetPointDamage( vecStartPosition, Point );

			if ( flDamage < iMinDamage )
				continue;

			C_HitboxData ScanData;
			ScanData.pPlayer = pPlayer;
			ScanData.LagRecord = LagRecord;
			ScanData.m_bForcedToSafeHitbox = Hitbox.m_bForceSafe;
			ScanData.m_flDamage = flDamage;
			ScanData.m_iHitbox = Hitbox.m_iHitbox;
			ScanData.m_vecPoint = Point;
			ScanData.m_bIsSafeHitbox = this->IsSafePoint( pPlayer, LagRecord, vecStartPosition, Point, Hitbox.m_iHitbox );

			if ( flDamage >= pPlayer->m_iHealth( ) || pWeaponData->m_iDamage >= pPlayer->m_iHealth( ) )
				if ( m_RageSettings.m_SafeModifiers[4] && !ScanData.m_bIsSafeHitbox )
					continue;

			if ( Hitbox.m_bForceSafe && !ScanData.m_bIsSafeHitbox )
				continue;

			m_ScanData.emplace_back( ScanData );
		}
	}

	if ( m_ScanData.empty( ) )
		return C_HitboxData( );

	bool bHasLethalDamage = false;
	int nLethalPosition = 0;

	for ( int i = 0; i < m_ScanData.size( ); i++ )
	{
		if ( m_ScanData[ i ].m_flDamage < pPlayer->m_iHealth( ) || m_ScanData[ i ].m_iHitbox == HITBOX_HEAD )
			continue;

		if ( !m_ScanData[ i ].m_bIsSafeHitbox )
			if ( m_ScanData[ i ].m_bForcedToSafeHitbox || m_RageSettings.m_SafeModifiers[4] )
				continue;

		nLethalPosition = i;
		bHasLethalDamage = true;

		break;
	}

	if ( bHasLethalDamage )
		return m_ScanData[ nLethalPosition ];

	bool bHasHeadSafety = false;
	for ( int i = 0; i < m_ScanData.size( ); i++ )
	{
		if ( m_ScanData[ i ].m_iHitbox != HITBOX_HEAD || m_ScanData[ i ].m_flDamage < pPlayer->m_iHealth( ) )
			continue;

		if ( !m_ScanData[ i ].m_bIsSafeHitbox && !LagRecord.m_bAnimResolved || !( pPlayer->m_fFlags( ) & FL_ONGROUND ) )
			continue;

		return m_ScanData[ i ];
	}

	auto SortSafety = [ ]( C_HitboxData First, C_HitboxData Second ) -> bool
	{
		if ( First.m_bIsSafeHitbox )
			return false;

		return true;
	};

	auto SortBody = [ ]( C_HitboxData First, C_HitboxData Second ) -> bool
	{
		if ( First.m_iHitbox == HITBOX_HEAD )
			return false;

		return true;
	};

	auto SortDamage = [ ]( C_HitboxData First, C_HitboxData Second ) -> bool
	{
		return First.m_flDamage > Second.m_flDamage;
	};
	
	std::sort( m_ScanData.begin( ), m_ScanData.end( ), SortDamage );
	if ( m_RageSettings.m_RageModifiers[0] )
		std::sort( m_ScanData.begin( ), m_ScanData.end( ), SortBody );

	if ( m_RageSettings.m_RageModifiers[1] )
		std::sort( m_ScanData.begin( ), m_ScanData.end( ), SortSafety );

	return m_ScanData.front( );
}

C_LagRecord C_RageBot::GetFirstAvailableRecord( C_BasePlayer* pPlayer )
{
	const auto m_LagRecords = g_Globals.m_CachedPlayerRecords[ pPlayer->EntIndex( ) ];
	if ( m_LagRecords.empty( ) )
		return C_LagRecord( );
	
	C_LagRecord LagRecord = C_LagRecord( );
	for ( int32_t i = 0; i < m_LagRecords.size( ); i++ )
	{
		auto m_Record = m_LagRecords[ i ];
		if ( !g_LagCompensation->IsValidTime( m_Record.m_SimulationTime ) )
			continue;

		LagRecord = m_LagRecords[ i ];
	}

	return LagRecord;
}

void C_RageBot::FakeDuck( )
{
	if ( ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN ) /*|| ( *( g_Globals.m_Interfaces.m_GameRules ) )->IsFreezePeriod( )*/ )
		return;

	if ( ( *( g_Globals.m_Interfaces.m_GameRules ) )->IsValveDS( ) && g_Settings->m_bAntiUntrusted )
		return;

	if ( g_Settings->m_bInfinityDuck )
		if ( !( *g_Globals.m_Interfaces.m_GameRules )->IsValveDS( ) || !g_Settings->m_bAntiUntrusted )
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_BULLRUSH;

	bool bShouldFakeDuck = g_Tools->IsBindActive( g_Settings->m_aFakeDuck ) && ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND );
	bool bFakeDuckBackup = g_Globals.m_Packet.m_bFakeDuck;

	if ( bShouldFakeDuck )
		g_Globals.m_Packet.m_bVisualFakeDuck = true;
	else if ( g_Globals.m_LocalPlayer->m_flDuckAmount( ) == 0.0f || g_Globals.m_LocalPlayer->m_flDuckAmount( ) >= 1.0f )
		g_Globals.m_Packet.m_bVisualFakeDuck = false;

	g_Globals.m_Packet.m_bFakeDuck = bShouldFakeDuck;
	if ( !bFakeDuckBackup && bShouldFakeDuck )
	{
		g_Globals.m_Packet.m_bFakeDuck = true;
		if ( g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands( ) )
			g_Globals.m_Packet.m_bFakeDuck = false;
		else
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_DUCK;

		return;
	}
	else if ( !bShouldFakeDuck )
	{
		float m_flAwaitedDuck = ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_DUCK ) ? 1.0f : 0.0f;
		if ( m_flAwaitedDuck != g_Globals.m_LocalPlayer->m_flDuckAmount( ) )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_BULLRUSH;

			if ( m_flAwaitedDuck < g_Globals.m_LocalPlayer->m_flDuckAmount( ) )
				g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_DUCK;
			else
				g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_DUCK;

			g_Globals.m_Packet.m_bFakeDuck = true;
		}
		
		if ( !g_Globals.m_Packet.m_bFakeDuck )
			return;
	}
	else
	{
		g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_BULLRUSH;

		if ( g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands( ) < 7 )
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_DUCK;
		else
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_DUCK;
	}

	if ( !bShouldFakeDuck )
	{
		g_Globals.m_Packet.m_bFakeDuck = false;
		return;
	}

	if ( g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands( ) < 14 )
	{
		g_PacketManager->GetModifablePacket( ) = false;
		return;
	}

	g_PacketManager->GetModifablePacket( ) = true;
}

void C_RageBot::SetupPacket( )
{
	C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !pCombatWeapon )
		return;

	C_CSWeaponData* pWeaponData = pCombatWeapon->GetWeaponData( );
	if ( !pWeaponData )
		return;

	float_t flDefaultInaccuracy = 0.0f;
	if (g_Globals.m_LocalPlayer->m_flDuckAmount())
	{
		if (g_Globals.m_LocalPlayer->m_bIsScoped())
			flDefaultInaccuracy = pWeaponData->m_flInaccuracyCrouchAlt;
		else
			flDefaultInaccuracy = pWeaponData->m_flInaccuracyCrouch;
	}
	else
	{
		if (g_Globals.m_LocalPlayer->m_bIsScoped())
			flDefaultInaccuracy = pWeaponData->m_flInaccuracyStandAlt;
		else
			flDefaultInaccuracy = pWeaponData->m_flInaccuracyStand;
	}
	g_Globals.m_AccuracyData.m_flMinInaccuracy = flDefaultInaccuracy;

	int32_t iCurrentWeapon = -1;
	switch ( pCombatWeapon->m_iItemDefinitionIndex( ) )
	{
		case WEAPON_AK47:
		case WEAPON_M4A1:
		case WEAPON_M4A1_SILENCER:
		case WEAPON_FAMAS:
		case WEAPON_SG553:
		case WEAPON_GALILAR:
			iCurrentWeapon = RAGE_WEAPON::RIFLE; break;
		case WEAPON_MAG7:
		case WEAPON_NOVA:
		case WEAPON_XM1014:
		case WEAPON_SAWEDOFF:
			iCurrentWeapon = RAGE_WEAPON::SHOTGUN; break;
		case WEAPON_MP7:
		case WEAPON_MP9:
		case WEAPON_P90:
		case WEAPON_M249:
		case WEAPON_NEGEV:
		case WEAPON_UMP45:
			iCurrentWeapon = RAGE_WEAPON::SMG; break;
		case WEAPON_SCAR20:
		case WEAPON_G3SG1:
			iCurrentWeapon = RAGE_WEAPON::AUTO; break;
		case WEAPON_GLOCK:
		case WEAPON_HKP2000: 
		case WEAPON_USP_SILENCER:
		case WEAPON_CZ75A:
		case WEAPON_TEC9:
		case WEAPON_ELITE:
		case WEAPON_FIVESEVEN:
		case WEAPON_P250:
			iCurrentWeapon = RAGE_WEAPON::PISTOL; break;
		case WEAPON_SSG08:
			iCurrentWeapon = RAGE_WEAPON::SCOUT; break;
		case WEAPON_AWP:
			iCurrentWeapon = RAGE_WEAPON::AWP; break;
		case WEAPON_DEAGLE:
			iCurrentWeapon = RAGE_WEAPON::DEAGLE; break;
		case WEAPON_REVOLVER:
			iCurrentWeapon = RAGE_WEAPON::REVOLVER; break;
		default: iCurrentWeapon = -1;
	}

	g_Globals.m_AccuracyData.m_bHasMaximumAccuracy = this->HasMaximumAccuracy( );
	if ( pCombatWeapon->m_iItemDefinitionIndex( ) == WEAPON_TASER || pCombatWeapon->m_iItemDefinitionIndex() == WEAPON_KNIFE )
	{
		m_RageSettings = C_RageSettings( );
		m_RageSettings.m_bAutoStop = true;

		m_RageSettings.m_iBodyScale = 80;
		m_RageSettings.m_iHitChance = 85;

		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_HEAD] = false;
		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_U_CHEST] = false;
		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_L_CHEST] = false;
		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_CHEST] = false;
		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_PELVIS] = true;
		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_STOMACH] = true;
		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_ARMS] = false;
		m_RageSettings.m_Hitboxes[RAGE_HITBOX_ID::POINT_LEGS] = false;

		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_HEAD] = false;
		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_U_CHEST] = false;
		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_L_CHEST] = false;
		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_CHEST] = false;
		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_PELVIS] = true;
		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_STOMACH] = true;
		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_ARMS] = false;
		m_RageSettings.m_SafeHitboxes[RAGE_HITBOX_ID::POINT_LEGS] = false;
		
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_HEAD] = false;
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_U_CHEST] = false;
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_L_CHEST] = false;
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_CHEST] = false;
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_PELVIS] = true;
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_STOMACH] = true;
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] = false;
		m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] = false;

		return; 
	}

	if ( iCurrentWeapon <= -1 )
	{
		m_RageSettings = C_RageSettings( );
		return;
	}

	m_RageSettings = g_Settings->m_aRageSettings[ iCurrentWeapon ];
}

int32_t C_RageBot::GetMinDamage( C_BasePlayer* pPlayer )
{
	int32_t iMinDamage = m_RageSettings.m_iMinDamage;

	if (g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex() == WEAPON_TASER)
		iMinDamage = 100;
	else if ( g_Tools->IsBindActive( g_Settings->m_aMinDamage ) )
		iMinDamage = m_RageSettings.m_iMinDamageOverride;

	iMinDamage = min( iMinDamage, pPlayer->m_iHealth( ) );

	if ( iMinDamage <= 0 )
		iMinDamage = this->GetAutoDamage( pPlayer->m_ArmourValue( ) != 0 );
	
	if ( iMinDamage > 99 )
		iMinDamage = pPlayer->m_iHealth( ) + ( iMinDamage - 100 );

	return iMinDamage;
}

int32_t C_RageBot::GetAutoDamage( bool bIsPlayerArmoured )
{
	int32_t nItemID = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( );
	switch ( nItemID )
	{
	case WEAPON_SCAR20:
	case WEAPON_G3SG1:
		return bIsPlayerArmoured ? 42 : 33;
	case WEAPON_SSG08:
		return 84;
	case WEAPON_AWP:
		return 100;
	case WEAPON_REVOLVER:
	case WEAPON_DEAGLE:
		return 44;
	case WEAPON_TEC9:
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
	case WEAPON_P250:
	case WEAPON_FLASHBANG:
	case WEAPON_CZ75A:
	case WEAPON_GLOCK:
	case WEAPON_ELITE:
		return bIsPlayerArmoured ? 20 : 34; 
	}

	return 0;
}

void C_RageBot::ResetData( )
{	
	g_Globals.m_AccuracyData.m_flSpread = 0.0f;
	g_Globals.m_AccuracyData.m_flInaccuracy = 0.0f;

	g_Globals.m_RageData.m_CurrentTarget = C_TargetData( );

	if ( !g_Globals.m_ShotData.empty( ) )
		return g_Globals.m_ShotData.clear( );
}

void C_RageBot::OnWeaponFire( C_GameEvent* pEvent )
{
	if ( !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive( ) )
		return;

	if ( g_Globals.m_Interfaces.m_EngineClient->GetPlayerForUserID( pEvent->GetInt( _S( "userid" ) ) ) != g_Globals.m_Interfaces.m_EngineClient->GetLocalPlayer( ) )
		return;

	C_ShotData* ShotData = nullptr;
	for ( int i = 0; i < g_Globals.m_ShotData.size( ); i++ )
	{
		if ( g_Networking->GetServerTick( ) - g_Globals.m_ShotData[ i ].m_iShotTick > g_Networking->GetTickRate( ) )
		{
			g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
			continue;
		}

		ShotData = &g_Globals.m_ShotData[ i ];
	}

	if ( ShotData )
		ShotData->m_bHasBeenFired = true;
}

void C_RageBot::OnPlayerHurt( C_GameEvent* pEvent )
{
	if ( !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive( ) )
		return;

	if ( g_Globals.m_Interfaces.m_EngineClient->GetPlayerForUserID( pEvent->GetInt( _S( "attacker" ) ) ) != g_Globals.m_Interfaces.m_EngineClient->GetLocalPlayer( ) )
		return;

	C_ShotData* ShotData = nullptr;
	for ( int i = 0; i < g_Globals.m_ShotData.size( ); i++ )
	{
		if ( g_Networking->GetServerTick( ) - g_Globals.m_ShotData[ i ].m_iShotTick > g_Networking->GetTickRate( ) )
		{
			g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
			continue;
		}

		ShotData = &g_Globals.m_ShotData[ i ];
	}

	if ( !ShotData )
		return;

	ShotData->m_bHasBeenHurted = true;
	ShotData->m_Damage = pEvent->GetInt( _S( "dmg_health" ) );
}

void C_RageBot::OnBulletImpact( C_GameEvent* pEvent )
{
	if ( !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive( ) )
		return;
	
	Vector vecBulletImpact = Vector( pEvent->GetInt( _S( "x" ) ), pEvent->GetInt( _S( "y" ) ), pEvent->GetInt( _S( "z" ) ) );
	if ( g_Globals.m_Interfaces.m_EngineClient->GetPlayerForUserID( pEvent->GetInt( _S( "userid" ) ) ) != g_Globals.m_Interfaces.m_EngineClient->GetLocalPlayer( ) )
		return;

	C_ShotData* ShotData = nullptr;
	for ( int i = 0; i < g_Globals.m_ShotData.size( ); i++ )
	{
		if ( g_Networking->GetServerTick( ) - g_Globals.m_ShotData[ i ].m_iShotTick > g_Networking->GetTickRate( ) )
		{
			g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
			continue;
		}

		ShotData = &g_Globals.m_ShotData[ i ];
	}
	
	if ( !ShotData )
		return;

	ShotData->m_vecImpacts.emplace_back( vecBulletImpact );
	ShotData->m_bHasBeenRegistered = true;
}

void C_RageBot::OnNetworkUpdate( ClientFrameStage_t Stage )
{
	if ( Stage != ClientFrameStage_t::FRAME_NET_UPDATE_END )
		return;

	if ( !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive( ) )
	{
		if ( !g_Globals.m_ShotData.empty( ) )
			g_Globals.m_ShotData.clear( );

		return;
	}

	for ( int i = 0; i < g_Globals.m_ShotData.size( ); i++ )
	{
		auto Shot = &g_Globals.m_ShotData[ i ];
		if ( !Shot->m_bHasBeenFired || !Shot->m_bHasBeenRegistered )
			continue;
	
		if ( !Shot->m_Target.m_Player )
		{
			g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
			continue;
		}
		
		g_Resolver->SortData(Shot->m_Target.m_Player->EntIndex(), false, false, Shot->m_Target.m_LagRecord.m_RotationMode);
		g_Globals.m_ResolverData.m_DidHitPrevious[Shot->m_Target.m_Player->EntIndex()] = true; 
		g_LogManager->PushLog(g_Resolver->PrintAnimationData(Shot->m_Target.m_Player->EntIndex()), _S("h"));

		if ( Shot->m_Target.m_Player->IsAlive( ) )
		{
			const auto LagRecords = g_Globals.m_CachedPlayerRecords[ Shot->m_Target.m_Player->EntIndex( ) ];
			if ( LagRecords.empty( ) )
			{
				g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
				continue;
			}

			this->AdjustPlayerRecord( Shot->m_Target.m_Player, Shot->m_Target.m_LagRecord );
			for ( auto& Impact : Shot->m_vecImpacts )
			{
				if (!this->DoesIntersectHitbox(Shot->m_Target.m_Player, Shot->m_Target.m_Hitbox.m_iHitbox, Shot->m_vecStartPosition, Impact))
				{
					g_Globals.m_ResolverData.m_DidHitPrevious[Shot->m_Target.m_Player->EntIndex()] = false;
					continue;
				}

				Shot->m_bDidIntersectPlayer = true;
				Shot->m_vecEndPosition = Impact;
				break;
			}
			this->AdjustPlayerRecord( Shot->m_Target.m_Player, LagRecords.back( ) );
		}
		
		g_World->OnRageBotFire( Shot->m_vecStartPosition, Shot->m_vecImpacts.back( ) );
		if ( Shot->m_bHasBeenHurted )
		{
			g_DmgMarkers->OnRageBotFire( Shot->m_vecEndPosition, Shot->m_Damage );
			for ( auto& Impact : Shot->m_vecImpacts )
				g_HitMarkers->OnRageBotFire( Impact );
		
			g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
			continue;
		}

		if ( !Shot->m_Target.m_Player->IsAlive( ) )
		{
			g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
			continue;
		}

		float_t flClientYaw = Math::AngleNormalize( Math::CalcAngle( Shot->m_vecStartPosition, Shot->m_Target.m_Hitbox.m_vecPoint ).yaw );
		float_t flServerYaw = Math::AngleNormalize( Math::CalcAngle( Shot->m_vecStartPosition, Shot->m_vecImpacts.back( ) ).yaw );

		bool bMissedShotDueToOcclusion = false;
		bool bMissedDueToResolver = false;
		if ( Shot->m_vecStartPosition.DistTo( Shot->m_vecImpacts.back( ) ) < Shot->m_vecStartPosition.DistTo( Shot->m_Target.m_Hitbox.m_vecPoint ) )
		{
			if ( fabs( Math::AngleNormalize( fabs( Math::AngleDiff( flClientYaw, flServerYaw ) ) ) ) <= 5.0f )
			{
				if ( g_AutoWall->GetPointDamage( Shot->m_vecStartPosition, Shot->m_vecImpacts.back( ) ) < 1.0f )
					bMissedShotDueToOcclusion = true;
			}
		}

		if ( bMissedShotDueToOcclusion )
		{
			if ( g_Settings->m_bLogMisses )
				g_LogManager->PushLog( _S( "Missed due to occlusion" ), _S( "h" ) );
		}
		else if ( Shot->m_bDidIntersectPlayer )
		{
			g_Globals.m_ResolverData.m_BruteSide[ Shot->m_Target.m_Player->EntIndex( ) ] = Shot->m_Target.m_LagRecord.m_RotationMode;
			g_Globals.m_ResolverData.m_MissedShots[ Shot->m_Target.m_Player->EntIndex( ) ]++;

			g_Resolver->SortData( Shot->m_Target.m_Player->EntIndex(), true, false, Shot->m_Target.m_LagRecord.m_RotationMode );
			bMissedDueToResolver = true;

			if ( g_Settings->m_bLogMisses )
				g_LogManager->PushLog("Missed due to unknown. DEBUG: " + std::to_string(g_Resolver->m_iResolverType[Shot->m_Target.m_Player->EntIndex()]), _S( "h" ) );
		}
		else
		{
			if ( g_Settings->m_bLogMisses )
				g_LogManager->PushLog( _S( "Missed due to spread" ), _S( "h" ) );
		}

		g_Globals.m_ResolverData.m_DidHitPrevious[Shot->m_Target.m_Player->EntIndex()] = !bMissedDueToResolver;
		g_Globals.m_ShotData.erase( g_Globals.m_ShotData.begin( ) + i );
	}
}

void C_RageBot::BackupPlayers( )
{
	for ( int nPlayerID = 1; nPlayerID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; nPlayerID++ )
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( nPlayerID );
		if ( !pPlayer || !pPlayer->IsAlive( ) || pPlayer->IsDormant( ) || pPlayer->m_iTeamNum( ) == pPlayer->m_iTeamNum( ) || !pPlayer->IsPlayer( ) )
			continue;

		if ( g_Globals.m_CachedPlayerRecords[ nPlayerID ].empty( ) )
			continue;

		m_BackupData[ nPlayerID ] = g_Globals.m_CachedPlayerRecords[ nPlayerID ].back( );
		m_BackupData[ nPlayerID ].m_AdjustTick = g_PacketManager->GetModifableCommand( )->m_nCommand;
	}
}

void C_RageBot::RestorePlayers( )
{
	for ( int nPlayerID = 1; nPlayerID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; nPlayerID++ )
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( nPlayerID );
		if ( !pPlayer || !pPlayer->IsAlive( ) || pPlayer->IsDormant( ) || pPlayer->m_iTeamNum( ) == pPlayer->m_iTeamNum( ) || !pPlayer->IsPlayer( ) )
			continue;

		if ( m_BackupData[ nPlayerID ].m_AdjustTick != g_PacketManager->GetModifableCommand( )->m_nCommand )
			continue;

		pPlayer->m_fFlags( ) = m_BackupData[ nPlayerID ].m_Flags;
		pPlayer->m_flSimulationTime( ) = m_BackupData[ nPlayerID ].m_SimulationTime;
		pPlayer->m_angEyeAngles( ) = m_BackupData[ nPlayerID ].m_EyeAngles;

		pPlayer->SetWorldOrigin( m_BackupData[ nPlayerID ].m_Origin );
		pPlayer->SetAbsoluteOrigin( m_BackupData[ nPlayerID ].m_Origin );

		std::memcpy( pPlayer->m_AnimationLayers( ), m_BackupData[ nPlayerID ].m_AnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
		std::memcpy( pPlayer->m_aPoseParameters( ).data( ), m_BackupData[ nPlayerID ].m_PoseParameters.data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );
	
		pPlayer->GetCollideable( )->OBBMaxs( ) = m_BackupData[ nPlayerID ].m_Maxs;
		pPlayer->GetCollideable( )->OBBMins( ) = m_BackupData[ nPlayerID ].m_Mins;
	
		this->AdjustPlayerBones( pPlayer, m_BackupData[ nPlayerID ].m_Matricies[ ROTATE_SERVER ] );
	}
}

bool C_RageBot::AutoStop( )
{
	if ( !this->CanAutoStop( ) || !m_RageSettings.m_bAutoStop )
		return true;

	if ( !m_RageSettings.m_AutoStopOptions[ AUTOSTOP_ACCURACY ] )
	{
		if ( g_Globals.m_LocalPlayer->m_vecVelocity( ).Length2D( ) < g_Globals.m_LocalPlayer->GetMaxPlayerSpeed( ) * 0.34f )
		{
			g_AntiAim->SlowWalk( true );
			return true;
		}
	}

	QAngle angResistance = QAngle(0, 0, 0);
	Math::VectorAngles((g_Globals.m_LocalPlayer->m_vecVelocity() * -1.f), angResistance);

	angResistance.yaw = g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw - angResistance.yaw;
	angResistance.pitch = g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch - angResistance.pitch;

	Vector vecResistance = Vector(0, 0, 0);
	Math::AngleVectors(angResistance, vecResistance);

	g_PacketManager->GetModifableCommand()->m_flForwardMove = std::clamp(vecResistance.x, -450.f, 450.0f);
	g_PacketManager->GetModifableCommand()->m_flSideMove = std::clamp(vecResistance.y, -450.f, 450.0f);

	return false;
}

void C_RageBot::AutoScope( )
{
	if ( g_Globals.m_LocalPlayer->m_bIsScoped( ) || !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( ) || !g_Globals.m_LocalPlayer->m_hActiveWeapon( )->IsSniper( ) )
		return;

	if ( g_Settings->m_bRageAutoscope )
		g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_ATTACK2;
}

void C_RageBot::AdjustPlayerBones( C_BasePlayer* pPlayer, std::array < matrix3x4_t, MAXSTUDIOBONES > aMatrix )
{
	std::memcpy( pPlayer->m_CachedBoneData( ).Base( ), aMatrix.data( ), sizeof( matrix3x4_t ) * pPlayer->m_CachedBoneData( ).Count( ) );
	return pPlayer->InvalidateBoneCache( );
}

void C_RageBot::AdjustPlayerRecord( C_BasePlayer* pPlayer, C_LagRecord LagRecord )
{
	pPlayer->m_fFlags( ) = LagRecord.m_Flags;
	pPlayer->m_flSimulationTime( ) = LagRecord.m_SimulationTime;
	pPlayer->m_angEyeAngles( ) = LagRecord.m_EyeAngles;

	pPlayer->SetWorldOrigin( LagRecord.m_Origin );
	pPlayer->SetAbsoluteOrigin( LagRecord.m_Origin );

	std::memcpy( pPlayer->m_AnimationLayers( ), LagRecord.m_AnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	std::memcpy( pPlayer->m_aPoseParameters( ).data( ), LagRecord.m_PoseParameters.data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );
	
	pPlayer->GetCollideable( )->OBBMaxs( ) = LagRecord.m_Maxs;
	pPlayer->GetCollideable( )->OBBMins( ) = LagRecord.m_Mins;
	
	return this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_SERVER ] );
}

typedef int32_t( __fastcall* ClipRayToHitbox_t )( const Ray_t&, mstudiobbox_t*, matrix3x4_t&, trace_t& );
bool C_RageBot::DoesIntersectHitbox( C_BasePlayer* pPlayer, int32_t iHitbox, Vector vecStartPosition, Vector vecEndPosition )
{
	if ( !pPlayer || !pPlayer->IsAlive( ) )
		return false;

	trace_t Trace;
	Ray_t Ray;

	Ray.Init( vecStartPosition, vecEndPosition );
	Trace.fraction = 1.0f;
	Trace.startsolid = false;

	studiohdr_t* pStudioHdr = ( studiohdr_t* )( g_Globals.m_Interfaces.m_ModelInfo->GetStudioModel( pPlayer->GetModel( ) ) );
	if ( !pStudioHdr)
		return false;

	mstudiohitboxset_t* pHitset = pStudioHdr->GetHitboxSet( pPlayer->m_nHitboxSet( ) );
	if ( !pHitset )
		return false;

	mstudiobbox_t* pHitbox = pHitset->GetHitbox( iHitbox );
	if ( !pHitbox )
		return false;

	return ( ( ClipRayToHitbox_t )( g_Globals.m_AddressList.m_ClipRayToHitbox ) )( Ray, pHitbox, pPlayer->m_CachedBoneData( ).Base( )[ pHitbox->m_nBone ], Trace ) > -1;
}

bool C_RageBot::IsSafePoint( C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition, Vector vecEndPosition, int32_t iHitbox )
{
	this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_LEFT ] );
	if ( !this->DoesIntersectHitbox( pPlayer, iHitbox, vecStartPosition, vecEndPosition ) )
	{
		this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_SERVER ] );
		return false; 
	}

	this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_CENTER ] );
	if ( !this->DoesIntersectHitbox( pPlayer, iHitbox, vecStartPosition, vecEndPosition ) )
	{
		this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_SERVER ] );
		return false;
	}

	this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_RIGHT ] );
	if ( !this->DoesIntersectHitbox( pPlayer, iHitbox, vecStartPosition, vecEndPosition ) )
	{
		this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_SERVER ] );
		return false;
	}

	this->AdjustPlayerBones( pPlayer, LagRecord.m_Matricies[ ROTATE_SERVER ] );
	return true;
}

std::vector < Vector > C_RageBot::GetHitboxPoints( C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition, int32_t iHitbox )
{
	float POINT_SCALE = 60.f;

	switch (iHitbox)
	{
	case HITBOX_HEAD:

		if ( !(pPlayer->m_fFlags() & FL_ONGROUND) )
			POINT_SCALE = 75;
		else 
			POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_HEAD] ? m_RageSettings.m_iHeadScale : 0.0000f;
		
		break;
	case HITBOX_NECK:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_HEAD] ? m_RageSettings.m_iHeadScale : 0.0000f;
		break;
	case HITBOX_CHEST:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_CHEST] ? m_RageSettings.m_iBodyScale : 0.0000f;
		break;
	case HITBOX_UPPER_CHEST:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_U_CHEST] ? m_RageSettings.m_iBodyScale : 0.0000f;
		break;
	case HITBOX_PELVIS:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_PELVIS] ? m_RageSettings.m_iBodyScale : 0.0000f;
		break;
	case HITBOX_STOMACH:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_STOMACH] ? m_RageSettings.m_iBodyScale : 0.0000f;
		break;
	case HITBOX_LOWER_CHEST:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_L_CHEST] ? m_RageSettings.m_iBodyScale : 0.0000f;
		break;
	case HITBOX_RIGHT_HAND:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_LEFT_HAND:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_RIGHT_UPPER_ARM:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_RIGHT_FOREARM:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_LEFT_UPPER_ARM:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_LEFT_FOREARM:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_ARMS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_RIGHT_THIGH:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_LEFT_THIGH:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_RIGHT_CALF:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_LEFT_CALF:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_RIGHT_FOOT:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	case HITBOX_LEFT_FOOT:
		POINT_SCALE = m_RageSettings.m_Multipoints[RAGE_HITBOX_ID::POINT_LEGS] ? m_RageSettings.m_iLimbScale : 0.0000f;
		break;
	default:
		break;
	}

	// percentage to decimal.
	POINT_SCALE *= 0.01f;

	studiohdr_t* pStudioHdr = (studiohdr_t*)(g_Globals.m_Interfaces.m_ModelInfo->GetStudioModel(pPlayer->GetModel()));
	if (!pStudioHdr)
		return { };

	mstudiohitboxset_t* pHitset = pStudioHdr->GetHitboxSet(pPlayer->m_nHitboxSet());
	if (!pHitset)
		return { };

	mstudiobbox_t* pHitbox = pHitset->GetHitbox(iHitbox);
	if (!pHitbox)
		return { };

	float_t flModifier = fmaxf(pHitbox->m_flRadius, 0.f);

	Vector vecMax;
	Vector vecMin;

	Math::VectorTransform(Vector(pHitbox->m_vecBBMax.x + flModifier, pHitbox->m_vecBBMax.y + flModifier, pHitbox->m_vecBBMax.z + flModifier), LagRecord.m_Matricies[ROTATE_SERVER][pHitbox->m_nBone], vecMax);
	Math::VectorTransform(Vector(pHitbox->m_vecBBMin.x - flModifier, pHitbox->m_vecBBMin.y - flModifier, pHitbox->m_vecBBMin.z - flModifier), LagRecord.m_Matricies[ROTATE_SERVER][pHitbox->m_nBone], vecMin);

	Vector vecCenter = (vecMin + vecMax) * 0.5f;

	QAngle angAngle = Math::CalcAngle(vecStartPosition, vecCenter);

	Vector vecForward;
	Math::AngleVectors(angAngle, vecForward);

	Vector vecRight = vecForward.Cross(Vector(0, 0, 2.33f));
	Vector vecLeft = Vector(-vecRight.x, -vecRight.y, vecRight.z);

	Vector vecTop = Vector(0, 0, 3.25f);
	Vector vecBottom = Vector(0, 0, -3.25f);

	int32_t iAngleToPlayer = (int32_t)(fabsf(Math::NormalizeAngle(Math::NormalizeAngle(Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw) - Math::NormalizeAngle(Math::CalcAngle(vecStartPosition, pPlayer->GetAbsOrigin()).yaw + 180.0f)))));
	int32_t iDistanceToPlayer = (int32_t)(g_Globals.m_LocalPlayer->GetAbsOrigin().DistTo(pPlayer->m_vecOrigin()));

	std::vector < Vector > aMultipoints = { };
	if (iHitbox == HITBOX_HEAD)
	{
		if (!m_RageSettings.m_Multipoints[0])
		{
			aMultipoints.emplace_back(vecCenter);
			return aMultipoints;
		}

		float_t flScale = m_RageSettings.m_iHeadScale;

		aMultipoints.emplace_back(vecCenter);
		aMultipoints.emplace_back(vecCenter + ((vecTop + vecRight) * POINT_SCALE));
		aMultipoints.emplace_back(vecCenter + ((vecTop + vecLeft) * POINT_SCALE));
	}
	else if (iHitbox == HITBOX_CHEST)
	{
		if (POINT_SCALE == 0.0000f)
		{
			aMultipoints.emplace_back(vecCenter);
			return aMultipoints;
		}

		aMultipoints.emplace_back(vecCenter + Vector(0, 0, 3));
		aMultipoints.emplace_back(vecCenter + vecRight * flModifier + Vector(0, 0, 3));
		aMultipoints.emplace_back(vecCenter + vecLeft * flModifier + Vector(0, 0, 3));
	}
	else if (iHitbox == HITBOX_STOMACH)
	{
		if (POINT_SCALE == 0.0000f)
		{
			aMultipoints.emplace_back(vecCenter);
			return aMultipoints;
		}

		aMultipoints.emplace_back(vecCenter + Vector(0, 0, 3.0f));
		aMultipoints.emplace_back(vecCenter + vecRight * flModifier + Vector(0.0f, 0.0f, 3.0f));
		aMultipoints.emplace_back(vecCenter + vecLeft * flModifier + Vector(0.0f, 0.0f, 3.0f));
	}
	else if (iHitbox == HITBOX_UPPER_CHEST)
	{
		if (POINT_SCALE == 0.0000f)
		{
			aMultipoints.emplace_back(vecCenter);
			return aMultipoints;
		}

		aMultipoints.emplace_back(vecCenter + Vector(0, 0, 3));
		aMultipoints.emplace_back(vecCenter + vecRight * flModifier + Vector(0, 0, 3));
		aMultipoints.emplace_back(vecCenter + vecLeft * flModifier + Vector(0, 0, 3));
	}
	else if (iHitbox == HITBOX_LOWER_CHEST)
	{
		if (POINT_SCALE == 0.0000f)
		{
			aMultipoints.emplace_back(vecCenter);
			return aMultipoints;
		}

		aMultipoints.emplace_back(vecCenter + Vector(0, 0, 3));
		aMultipoints.emplace_back(vecCenter + vecRight * flModifier + Vector(0, 0, 3));
		aMultipoints.emplace_back(vecCenter + vecLeft * flModifier + Vector(0, 0, 3));
	}
	else if (iHitbox == HITBOX_PELVIS)
	{
		if (POINT_SCALE == 0.0000f)
		{
			aMultipoints.emplace_back(vecCenter);
			return aMultipoints;
		}

		aMultipoints.emplace_back(vecCenter - Vector(0.0f, 0.0f, 2.0f));
		aMultipoints.emplace_back(vecCenter + vecRight * flModifier - Vector(0.0f, 0.0f, 2.0f));
		aMultipoints.emplace_back(vecCenter + vecLeft * flModifier - Vector(0.0f, 0.0f, 2.0f));
	}
	else if (iHitbox == HITBOX_LEFT_FOOT || iHitbox == HITBOX_RIGHT_FOOT || iHitbox == HITBOX_LEFT_THIGH || iHitbox == HITBOX_RIGHT_THIGH)
	{
		if (POINT_SCALE == 0.0000f)
		{
			aMultipoints.emplace_back(vecCenter);
			return aMultipoints;
		}

		Vector vecAddition = vecLeft;
		if (iHitbox == HITBOX_LEFT_FOOT || iHitbox == HITBOX_LEFT_THIGH)
			vecAddition = vecRight;
		else if (iHitbox == HITBOX_RIGHT_FOOT || iHitbox == HITBOX_RIGHT_THIGH)
			vecAddition = vecLeft;

		if (iHitbox == HITBOX_LEFT_THIGH || iHitbox == HITBOX_RIGHT_THIGH)
			vecCenter -= Vector(0.0f, 0.0f, 2.5f);

		aMultipoints.emplace_back(vecCenter - (vecAddition * 0.90f));
	}
	else if (iHitbox == HITBOX_LEFT_FOREARM || iHitbox == HITBOX_RIGHT_FOREARM)
		aMultipoints.emplace_back(vecCenter - (iHitbox == HITBOX_LEFT_FOREARM ? vecLeft : -vecLeft));

	return aMultipoints;
}

void C_RageBot::AutoRevolver( )
{
	if ( ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN ) || !g_Settings->m_bEnabledRage )
		return;

	C_BaseCombatWeapon* pWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !pWeapon || !g_Settings->m_bEnabledRage || pWeapon->m_iItemDefinitionIndex( ) != WEAPON_REVOLVER )
		return;

	g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_ATTACK2;
	if ( !g_Globals.m_LocalPlayer->CanFire( ) )
		return;

	if ( pWeapon->m_flPostponeFireReadyTime( ) <= TICKS_TO_TIME( g_Globals.m_LocalPlayer->m_nTickBase( ) ) )
	{
		if ( ( pWeapon->m_flNextSecondaryAttack( ) + ( g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick * -3.0f ) ) <= TICKS_TO_TIME( g_Globals.m_LocalPlayer->m_nTickBase( ) ) )
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_ATTACK;
	}
	else
		g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_ATTACK;

}

C_HitboxData C_RageBot::VisibleOnlyScan(C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition)
{
	C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();
	if (!pCombatWeapon)
		return C_HitboxData();

	C_CSWeaponData* pWeaponData = pCombatWeapon->GetWeaponData();
	if (!pWeaponData)
		return C_HitboxData();

	std::vector < C_HitboxHitscanData > aHitboxesData;

	bool bForcedSafety = g_Tools->IsBindActive(g_Settings->m_aSafePoint);
	aHitboxesData.emplace_back(C_HitboxHitscanData(HITBOX_HEAD, bForcedSafety));
	aHitboxesData.emplace_back(C_HitboxHitscanData(HITBOX_CHEST, bForcedSafety));
	aHitboxesData.emplace_back(C_HitboxHitscanData(HITBOX_UPPER_CHEST, bForcedSafety));
	aHitboxesData.emplace_back(C_HitboxHitscanData(HITBOX_LOWER_CHEST, bForcedSafety));
	aHitboxesData.emplace_back(C_HitboxHitscanData(HITBOX_PELVIS, bForcedSafety));
	aHitboxesData.emplace_back(C_HitboxHitscanData(HITBOX_STOMACH, bForcedSafety));

	std::vector < C_HitboxData > m_ScanData;

	if (pCombatWeapon->m_iItemDefinitionIndex() == WEAPON_TASER)
	{
		for (auto Hitbox : aHitboxesData)
		{
			std::vector < Vector > m_Hitboxes = this->GetHitboxPoints(pPlayer, LagRecord, vecStartPosition, Hitbox.m_iHitbox);
			if (m_Hitboxes.empty())
				continue;

			for (auto Point : m_Hitboxes)
			{
				float_t flDamage = g_AutoWall->GetPointDamage(vecStartPosition, Point);

				if (flDamage < pPlayer->m_iHealth())
					continue;

				C_HitboxData ScanData;
				ScanData.pPlayer = pPlayer;
				ScanData.LagRecord = LagRecord;
				ScanData.m_bForcedToSafeHitbox = Hitbox.m_bForceSafe;
				ScanData.m_flDamage = flDamage;
				ScanData.m_iHitbox = Hitbox.m_iHitbox;
				ScanData.m_vecPoint = Point;
				ScanData.m_bIsSafeHitbox = this->IsSafePoint(pPlayer, LagRecord, vecStartPosition, Point, Hitbox.m_iHitbox);

				if (flDamage >= pPlayer->m_iHealth() || pWeaponData->m_iDamage >= pPlayer->m_iHealth())
					if (m_RageSettings.m_SafeModifiers[4] && !ScanData.m_bIsSafeHitbox)
						continue;

				if (Hitbox.m_bForceSafe && !ScanData.m_bIsSafeHitbox)
					continue;

				m_ScanData.emplace_back(ScanData);
			}
		}
	}
	else
	{
		for (auto Hitbox : aHitboxesData)
		{
			std::vector < Vector > m_Hitboxes = this->GetHitboxPoints(pPlayer, LagRecord, vecStartPosition, Hitbox.m_iHitbox);
			if (m_Hitboxes.empty())
				continue;

			for (auto Point : m_Hitboxes)
			{
				CTraceFilter aFilter;
				aFilter.pSkip = g_Globals.m_LocalPlayer;

				trace_t aTrace;
				Ray_t aRay;

				aRay.Init(g_Globals.m_LocalData.m_vecShootPosition, Point);

				g_Globals.m_Interfaces.m_EngineTrace->TraceRay(aRay, 0x200400B, &aFilter, &aTrace);

				if (!aTrace.IsVisible())
					continue;

				C_HitboxData ScanData;
				ScanData.pPlayer = pPlayer;
				ScanData.LagRecord = LagRecord;
				ScanData.m_bForcedToSafeHitbox = Hitbox.m_bForceSafe;
				ScanData.m_flDamage = 100;
				ScanData.m_iHitbox = Hitbox.m_iHitbox;
				ScanData.m_vecPoint = Point;
				ScanData.m_bIsSafeHitbox = this->IsSafePoint(pPlayer, LagRecord, vecStartPosition, Point, Hitbox.m_iHitbox);

				if (Hitbox.m_bForceSafe && !ScanData.m_bIsSafeHitbox)
					continue;

				m_ScanData.emplace_back(ScanData);
			}
		}
	}

	if (m_ScanData.empty())
		return C_HitboxData();

	bool bHasLethalDamage = false;
	int nLethalPosition = 0;

	for (int i = 0; i < m_ScanData.size(); i++)
	{
		if (m_ScanData[i].m_flDamage < pPlayer->m_iHealth() || m_ScanData[i].m_iHitbox == HITBOX_HEAD)
			continue;

		if (!m_ScanData[i].m_bIsSafeHitbox)
			if (m_ScanData[i].m_bForcedToSafeHitbox || m_RageSettings.m_SafeModifiers[2])
				continue;

		nLethalPosition = i;
		bHasLethalDamage = true;

		break;
	}

	if (bHasLethalDamage)
		return m_ScanData[nLethalPosition];

	bool bHasHeadSafety = false;
	for (int i = 0; i < m_ScanData.size(); i++)
	{
		if (m_ScanData[i].m_iHitbox != HITBOX_HEAD || m_ScanData[i].m_flDamage < pPlayer->m_iHealth())
			continue;

		if (!m_ScanData[i].m_bIsSafeHitbox && !LagRecord.m_bAnimResolved || !(pPlayer->m_fFlags() & FL_ONGROUND))
			continue;

		return m_ScanData[i];
	}

	auto SortSafety = [](C_HitboxData First, C_HitboxData Second) -> bool
	{
		if (First.m_bIsSafeHitbox)
			return false;

		return true;
	};

	auto SortBody = [](C_HitboxData First, C_HitboxData Second) -> bool
	{
		if (First.m_iHitbox == HITBOX_HEAD)
			return false;

		return true;
	};

	auto SortDamage = [](C_HitboxData First, C_HitboxData Second) -> bool
	{
		return First.m_flDamage > Second.m_flDamage;
	};

	std::sort(m_ScanData.begin(), m_ScanData.end(), SortDamage);
	
	if (m_RageSettings.m_RageModifiers[0])
		std::sort(m_ScanData.begin(), m_ScanData.end(), SortBody);

	if (m_RageSettings.m_RageModifiers[1])
		std::sort(m_ScanData.begin(), m_ScanData.end(), SortSafety);

	return m_ScanData.front();
}

int FindHitType(bool stab_type, const Vector& delta)
{
	auto minimum_distance = stab_type ? 32.0f : 48.0f;
	auto end = g_Globals.m_LocalData.m_vecShootPosition + delta * minimum_distance;

	CTraceFilter filter;
	filter.pSkip = g_Globals.m_LocalPlayer;

	trace_t trace;
	Ray_t ray;

	ray.Init(g_Globals.m_LocalData.m_vecShootPosition, end, Vector(-16.0f, -16.0f, -18.0f), Vector(16.0f, 16.0f, 18.0f));
	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, 0x200400B, &filter, &trace);

	if (trace.hit_entity != g_Globals.m_RageData.m_CurrentTarget.m_Player)
		return 0;

	auto cos_pitch = cos(DirectX::XMConvertToRadians(g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_EyeAngles.pitch));

	auto sin_yaw = 0.0f;
	auto cos_yaw = 0.0f;

	DirectX::XMScalarSinCos(&sin_yaw, &cos_yaw, DirectX::XMConvertToRadians(g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_EyeAngles.yaw));

	auto final_delta = g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_Origin - g_Globals.m_LocalData.m_vecShootPosition;
	return (int)(cos_yaw * cos_pitch * final_delta.x + sin_yaw * cos_pitch * final_delta.y >= 0.475f) + 1;
}

void C_RageBot::TaserBot( )
{
	int nScannedTargets = 0;

	if (m_nLastRageID >= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients)
		m_nLastRageID = 1;

	for (; m_nLastRageID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; m_nLastRageID++)
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(m_nLastRageID);
		if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive() || pPlayer->m_iTeamNum() == g_Globals.m_LocalPlayer->m_iTeamNum())
			continue;

		Vector vecShootPosition = g_Globals.m_LocalData.m_vecShootPosition;
		if (pPlayer->m_bGunGameImmunity() || (pPlayer->m_fFlags() & FL_FROZEN))
			continue;

		const auto m_LagRecords = g_Globals.m_CachedPlayerRecords[m_nLastRageID];
		if (m_LagRecords.empty())
			continue;

		if (nScannedTargets > 2)
			break;

		if ((g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex() == WEAPON_TASER
			|| g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->IsKnife())
			&& g_Globals.m_LocalPlayer->m_vecOrigin().DistTo(pPlayer->m_vecOrigin()) > g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->GetWeaponData()->m_flRange)
			return;

		C_LagRecord LagRecord = this->GetFirstAvailableRecord(pPlayer);
		if (!g_LagCompensation->IsValidTime(LagRecord.m_SimulationTime))
		{
			if (m_RageSettings.m_AutoStopOptions[AUTOSTOP_EARLY] && m_RageSettings.m_bAutoStop)
			{
				this->AdjustPlayerRecord(pPlayer, m_LagRecords.back());

				C_HitboxData HitboxData = this->VisibleOnlyScan(pPlayer, m_LagRecords.back(), vecShootPosition);
				if (HitboxData.m_flDamage >= this->GetMinDamage(pPlayer))
				{
					float_t flDistance = g_Globals.m_LocalPlayer->m_vecOrigin().DistTo(pPlayer->m_vecOrigin());
					if (flDistance)
					{
						int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex();
						if ((nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1) || flDistance > 430.0f)
							this->AutoScope();
					}

					if (this->CanAutoStop())
						g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;
				}
			}

			continue;
		}

		this->AdjustPlayerRecord(pPlayer, LagRecord);
		if (m_RageSettings.m_AutoStopOptions[AUTOSTOP_EARLY] && m_RageSettings.m_bAutoStop)
		{
			C_HitboxData HitboxData = this->VisibleOnlyScan(pPlayer, LagRecord, vecShootPosition);
			if (HitboxData.m_flDamage >= this->GetMinDamage(pPlayer))
			{
				float_t flDistance = g_Globals.m_LocalPlayer->m_vecOrigin().DistTo(pPlayer->m_vecOrigin());
				if (flDistance)
				{
					int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex();
					if ((nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1) || flDistance > 430.0f)
						this->AutoScope();
				}

				if (this->CanAutoStop())
					g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;
			}
		}

		C_HitboxData HitboxData = this->VisibleOnlyScan(pPlayer, LagRecord, vecShootPosition);
		if (HitboxData.m_flDamage >= this->GetMinDamage(pPlayer))
		{
			g_Globals.m_RageData.m_CurrentTarget.m_Hitbox = HitboxData;
			g_Globals.m_RageData.m_CurrentTarget.m_LagRecord = LagRecord;
			g_Globals.m_RageData.m_CurrentTarget.m_Player = pPlayer;

			if (this->CanAutoStop())
				g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;

			float_t flDistance = g_Globals.m_LocalPlayer->m_vecOrigin().DistTo(pPlayer->m_vecOrigin());
			if (flDistance)
			{
				int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex();
				if ((nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1) || flDistance > 430.0f)
					this->AutoScope();
			}

			this->AdjustPlayerRecord(g_Globals.m_RageData.m_CurrentTarget.m_Player, g_Globals.m_RageData.m_CurrentTarget.m_LagRecord);
			break;
		}

		C_LagRecord BTRecord = C_LagRecord();
		C_HitboxData BTHitscan = C_HitboxData();

		if (!this->FindPlayerRecord(pPlayer, &BTRecord, &BTHitscan))
		{
			nScannedTargets++;

			this->AdjustPlayerRecord(pPlayer, m_LagRecords.back());
			continue;
		}

		g_Globals.m_RageData.m_CurrentTarget.m_Player = pPlayer;
		g_Globals.m_RageData.m_CurrentTarget.m_LagRecord = BTRecord;
		g_Globals.m_RageData.m_CurrentTarget.m_Hitbox = BTHitscan;

		if (this->CanAutoStop())
			g_Globals.m_AccuracyData.m_bRestoreAutoStop = false;

		float_t flDistance = g_Globals.m_LocalPlayer->GetAbsOrigin().DistTo(pPlayer->GetAbsOrigin());
		if (flDistance)
		{
			int nIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_iItemDefinitionIndex();
			if ((nIndex != WEAPON_SCAR20 && nIndex != WEAPON_G3SG1) || flDistance > 450.0f)
				this->AutoScope();
		}

		this->AdjustPlayerRecord(g_Globals.m_RageData.m_CurrentTarget.m_Player, g_Globals.m_RageData.m_CurrentTarget.m_LagRecord);
		break;
	}

	if (!g_Globals.m_RageData.m_CurrentTarget.m_Player)
		return;

	if (g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->IsKnife())
	{
			auto vecOrigin = g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_Origin;

			auto vecOBBMins = g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_Mins;
			auto vecOBBMaxs = g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_Maxs;

			auto vecMins = vecOBBMins + vecOrigin;
			auto vecMaxs = vecOBBMaxs + vecOrigin;

			auto vecEyePos = g_Globals.m_RageData.m_CurrentTarget.m_Player->GetShootPosition();

			if (vecMins < vecEyePos)
				vecMins = vecEyePos;

			if (vecMins > vecMaxs)
				vecMins = vecMaxs;

			auto vecDelta = vecMins - g_Globals.m_LocalData.m_vecShootPosition;

			if (vecDelta.Length() > 60.0f)
				return;

			vecDelta.Normalize();
			auto delta = fabs(Math::NormalizeAngle(g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_EyeAngles.yaw - Math::CalcAngle(g_Globals.m_RageData.m_CurrentTarget.m_Player->GetShootPosition(), g_Globals.m_LocalPlayer->GetAbsOrigin()).yaw));

			if (g_Globals.m_RageData.m_CurrentTarget.m_Player->m_iHealth() > 46 && delta < 120.0f)
			{
				g_PacketManager->GetModifableCommand()->m_nTickCount = TIME_TO_TICKS(g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_SimulationTime + g_LagCompensation->GetLerpTime()); Vector toQAngle = vecDelta.ToEulerAngles();
				g_PacketManager->GetModifableCommand()->m_angViewAngles = QAngle(toQAngle.x, toQAngle.y, toQAngle.z);
				g_PacketManager->GetModifableCommand()->m_nButtons |= IN_ATTACK;
			}

			if (!FindHitType(1, vecDelta))
				return;

			g_PacketManager->GetModifableCommand()->m_nTickCount = TIME_TO_TICKS(g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_SimulationTime + g_LagCompensation->GetLerpTime()); Vector toQAngle = vecDelta.ToEulerAngles();
			g_PacketManager->GetModifableCommand()->m_angViewAngles = QAngle(toQAngle.x, toQAngle.y, toQAngle.z);
			g_PacketManager->GetModifableCommand()->m_nButtons |= IN_ATTACK2;

			return;
	}

	this->AutoStop();

	float_t flCalculatedHitchance = this->GetHitChance(g_Globals.m_RageData.m_CurrentTarget);
	if (flCalculatedHitchance < 75)
		return;

	QAngle angCalculatedAngle = Math::CalcAngle(g_Globals.m_LocalData.m_vecShootPosition, g_Globals.m_RageData.m_CurrentTarget.m_Hitbox.m_vecPoint);
	if (!g_Globals.m_Packet.m_bFakeDuck)
		g_PacketManager->GetModifablePacket() = true;

	g_PacketManager->GetModifableCommand()->m_nTickCount = TIME_TO_TICKS(g_Globals.m_RageData.m_CurrentTarget.m_LagRecord.m_SimulationTime + g_LagCompensation->GetLerpTime());
	g_PacketManager->GetModifableCommand()->m_angViewAngles = angCalculatedAngle;
	g_PacketManager->GetModifableCommand()->m_nButtons |= IN_ATTACK;

	if (g_Globals.m_ConVars.m_WeaponRecoilScale->GetFloat() > 0.0f)
		g_PacketManager->GetModifableCommand()->m_angViewAngles -= g_Globals.m_ConVars.m_WeaponRecoilScale->GetFloat() * g_Globals.m_LocalPlayer->m_aimPunchAngle();

	C_ShotData ShotData;

	ShotData.m_Target = g_Globals.m_RageData.m_CurrentTarget;
	ShotData.m_vecStartPosition = g_Globals.m_LocalData.m_vecShootPosition;
	ShotData.m_vecEndPosition = g_Globals.m_RageData.m_CurrentTarget.m_Hitbox.m_vecPoint;
	ShotData.m_iShotTick = g_Networking->GetServerTick();
	ShotData.m_bHasMaximumAccuracy = this->HasMaximumAccuracy();

	g_Globals.m_ShotData.emplace_back(ShotData);

	return g_ShotChams->OnRageBotFire(g_Globals.m_RageData.m_CurrentTarget.m_Player);
}

static std::vector<std::tuple<float, float, float>> precomputed_seeds = {};
void BulidSeedTable() {

	if (!precomputed_seeds.empty())
		return;

	for (auto i = 0; i < 255; i++) {
		g_Tools->RandomSeed(i + 1);

		const auto pi_seed = g_Tools->RandomFloat(0.f, twopi);

		precomputed_seeds.emplace_back(g_Tools->RandomFloat(0.f, 1.f),
			sin(pi_seed), cos(pi_seed));
	}
}

void sin_cos(float radian, float* sin, float* cos) {
	*sin = std::sin(radian);
	*cos = std::cos(radian);
}

void fast_rsqrt(float a, float* out)
{
	const auto xx = _mm_load_ss(&a);
	auto xr = _mm_rsqrt_ss(xx);
	auto xt = _mm_mul_ss(xr, xr);
	xt = _mm_mul_ss(xt, xx);
	xt = _mm_sub_ss(_mm_set_ss(3.f), xt);
	xt = _mm_mul_ss(xt, _mm_set_ss(0.5f));
	xr = _mm_mul_ss(xr, xt);
	_mm_store_ss(out, xr);
}

float fast_vec_normalize(Vector& vec)
{
	const auto sqrlen = vec.LengthSqr() + 1.0e-10f;
	float invlen;
	fast_rsqrt(sqrlen, &invlen);
	vec.x *= invlen;
	vec.y *= invlen;
	vec.z *= invlen;
	return sqrlen * invlen;
}

void AngleVectors(const Vector& angles, Vector& forward)
{
	float sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void AngleVectors(const Vector& angles, Vector& forward, Vector& right, Vector& up)
{
	float sr, sp, sy, cr, cp, cy;

	DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles[0]));
	DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles[1]));
	DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angles[2]));

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr * cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}

float_t C_RageBot::GetHitChance( C_TargetData Target )
{
	if (g_Globals.m_AccuracyData.m_bDoingSecondShot)
		return 100.f;

	Vector vecShootPosition = g_Globals.m_LocalData.m_vecShootPosition;
	QAngle angViewAngles = Math::CalcAngle( vecShootPosition, Target.m_Hitbox.m_vecPoint );

	C_BaseCombatWeapon* pWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !pWeapon )
		return 0.0f;

	C_CSWeaponData* pWeaponData = pWeapon->GetWeaponData( );
	if ( !pWeaponData )
		return 0.0f;

	float flMaxDistance = pWeaponData->m_flRange;
	if ( vecShootPosition.DistTo( Target.m_Hitbox.m_vecPoint ) > flMaxDistance )
		return false;

	if ( this->HasMaximumAccuracy( ) )
		return 100.0f;
		
	float_t flWeaponSpread = pWeapon->GetSpread( );
	float_t flWeaponInaccuracy = pWeapon->GetInaccuracy( );

	auto m_local = g_Globals.m_LocalPlayer;

	BulidSeedTable();

	static auto weapon_recoil_scale = g_Globals.m_Interfaces.m_CVar->FindVar(("weapon_recoil_scale"));
	const auto aim_angle = angViewAngles - (m_local->m_aimPunchAngle() * weapon_recoil_scale->GetFloat());
	auto forward = Vector(0,0,0);
	auto right = Vector(0, 0, 0);
	auto up = Vector(0, 0, 0);
	auto current = 0;
	int awalls_hit = 0;
	Vector total_spread, spread_angle, end;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed;

	Vector m_AimAngle = Vector(aim_angle.pitch, aim_angle.yaw, aim_angle.roll);

	AngleVectors(m_AimAngle, forward, right, up);
	fast_vec_normalize(forward);
	fast_vec_normalize(right);
	fast_vec_normalize(up);

	for (auto i = 0u; i < 255; i++)
	{
		seed = &precomputed_seeds[i];
		inaccuracy = std::get<0>(*seed) * flWeaponInaccuracy;
		spread_x = std::get<2>(*seed) * inaccuracy;
		spread_y = std::get<1>(*seed) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y);
		total_spread.Normalize();
		Math::VectorAngles(total_spread, spread_angle);
		AngleVectors(spread_angle, end);
		end.Normalize();
		end = g_Globals.m_LocalData.m_vecShootPosition + end * pWeaponData->m_flRange;
		auto position = g_Globals.m_LocalData.m_vecShootPosition + g_PredictionSystem->GetNetvars(g_PacketManager->GetModifableCommand()->m_nCommand).m_vecVelocity * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick * 8;

		if (DoesIntersectHitbox(Target.m_Player, Target.m_Hitbox.m_iHitbox, g_Globals.m_LocalData.m_vecShootPosition, end))
			current++;

		if ((static_cast<float>(current) / 255.f) * 100.f >= m_RageSettings.m_iHitChance)
		{
			return (static_cast<float>(current) / 255.f) * 100.f;
		}
		if ((static_cast<float>(current + 255 - i) / 255.f) * 100.f < m_RageSettings.m_iHitChance)
		{
			return (static_cast<float>(current + 255 - i) / 255.f) * 100.f;
		}
	}

	return (static_cast<float>(current) / 255.f) * 100.f;
}

bool C_RageBot::HasMaximumAccuracy( )
{
	C_BaseCombatWeapon* pWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !pWeapon )
		return false;

	if ( !pWeapon->IsGun( ) )
		return false;

	C_CSWeaponData* pWeaponData = pWeapon->GetWeaponData( );
	if ( !pWeaponData )
		return false;

	if ( g_Globals.m_LocalPlayer->m_flVelocityModifier( ) < 1.0f || pWeapon->m_flNextPrimaryAttack( ) <= g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime )
		return false;

	return pWeapon->GetInaccuracy( ) - g_Globals.m_AccuracyData.m_flMinInaccuracy < 0.0001f;
}

bool C_RageBot::CanAutoStop( )
{
	if ( !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( ) || !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->IsGun( ) )
		return false;

	if ( !g_Globals.m_AccuracyData.m_bCanFire_Default || !g_Globals.m_AccuracyData.m_bCanFire_Shift || g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iClip1( ) <= 0 )
		return false;

	if ( !( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND ) )
		return false;

	if ( !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( ) || !g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->GetWeaponData( ) )
		return false;

	return m_RageSettings.m_bAutoStop;
}