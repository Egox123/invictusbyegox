#include "Antiaim.hpp"
#include "../Packet/PacketManager.hpp"
#include "../Settings.hpp"
#include "../Tools/Tools.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Exploits/Exploits.hpp"
#include "../Networking/Networking.hpp"
#include "../Features/RageBot/Autowall.hpp"

C_AntiAimSettings* GetSettings()
{
	if (g_PacketManager->GetModifableCommand()->m_nButtons & IN_JUMP || g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO()->m_bLanding || !(g_Globals.m_LocalPlayer->m_fFlags() & FL_ONGROUND) )
		return &g_Settings->m_aAntiAim.at(3);

	if (g_Globals.m_AccuracyData.m_bSlowWalkState)
		return &g_Settings->m_aAntiAim.at(2);

	if (g_Globals.m_LocalPlayer->m_vecVelocity().Length2D() > 23.40)
		return &g_Settings->m_aAntiAim.at(1);

	return &g_Settings->m_aAntiAim.at(0);
}

bool fnManualSide( int32_t& pInput )
{
	int32_t iBackup = pInput;

	pInput = 2;

	if ( g_Tools->IsBindActive(g_Settings->m_aManualLeft) )
	{
		pInput = -1;

		if ( iBackup != pInput )
		{
			g_Globals.m_KeyData.m_aToggledKeys[g_Settings->m_aManualBack->m_iKeySelected] = false;
			g_Globals.m_KeyData.m_aHoldedKeys[g_Settings->m_aManualBack->m_iKeySelected] = false;

			g_Globals.m_KeyData.m_aToggledKeys[g_Settings->m_aManualRight->m_iKeySelected] = false;
			g_Globals.m_KeyData.m_aHoldedKeys[g_Settings->m_aManualRight->m_iKeySelected] = false;

			return true;
		}
	}

	if ( g_Tools->IsBindActive(g_Settings->m_aManualBack) )
	{
		pInput = 0;

		if ( iBackup != pInput )
		{
			g_Globals.m_KeyData.m_aToggledKeys[g_Settings->m_aManualLeft->m_iKeySelected] = false;
			g_Globals.m_KeyData.m_aHoldedKeys[g_Settings->m_aManualLeft->m_iKeySelected] = false;

			g_Globals.m_KeyData.m_aToggledKeys[g_Settings->m_aManualRight->m_iKeySelected] = false;
			g_Globals.m_KeyData.m_aHoldedKeys[g_Settings->m_aManualRight->m_iKeySelected] = false;

			return true;
		}
	}

	if ( g_Tools->IsBindActive(g_Settings->m_aManualRight) )
	{
		pInput = 1;

		if ( iBackup != pInput )
		{
			g_Globals.m_KeyData.m_aToggledKeys[g_Settings->m_aManualBack->m_iKeySelected] = false;
			g_Globals.m_KeyData.m_aHoldedKeys[g_Settings->m_aManualBack->m_iKeySelected] = false;

			g_Globals.m_KeyData.m_aToggledKeys[g_Settings->m_aManualLeft->m_iKeySelected] = false;
			g_Globals.m_KeyData.m_aHoldedKeys[g_Settings->m_aManualLeft->m_iKeySelected] = false;

			return true;
		}
	}

	if ( pInput != 2 )
		return true;

	return false;
}

void C_AntiAim::FindTarget()
{
	float_t flBestDistance = FLT_MAX;

	for ( int32_t i = 1; i < 65; i++ )
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(i);
		if ( !pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive() || pPlayer->m_iTeamNum() == g_Globals.m_LocalPlayer->m_iTeamNum() || pPlayer->IsDormant() )
			continue;

		if ( pPlayer->m_fFlags() & FL_FROZEN )
			continue;

		float_t flDistanceToPlayer = g_Globals.m_LocalPlayer->m_vecOrigin().DistTo(pPlayer->m_vecOrigin());
		if ( flDistanceToPlayer > flBestDistance )
			continue;

		if ( flDistanceToPlayer > 1250.0f )
			continue;

		flBestDistance = flDistanceToPlayer;
		m_aTarget = pPlayer;
	}
}

void C_AntiAim::Instance( )
{
	if ( !g_Globals.m_LocalPlayer )
		return;

	if ( !g_Globals.m_LocalPlayer->IsAlive() )
		return;

	int32_t nCommand = g_PacketManager->GetModifableCommand( )->m_nCommand;
	if ( !g_Settings->m_bAntiAim )
	{
		if ( g_PacketManager->GetModifablePacket( ) )
			g_PacketManager->SetFakeAngles( g_PacketManager->GetModifableCommand( )->m_angViewAngles );

		return;
	}

	// reset slow walk state.
	g_Globals.m_AccuracyData.m_bSlowWalkState = false;

	// import settings.
	C_AntiAimSettings* Settings = GetSettings( );

	m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] = this->GetAntiAimConditions( );
	if ( !m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] )
	{
		if ( !g_Globals.m_Packet.m_bFakeDuck )
		{
			g_PacketManager->GetModifablePacket( ) = true;
			g_PacketManager->SetFakeAngles( g_PacketManager->GetModifableCommand( )->m_angViewAngles );
		}

		m_flOptimizedYawAngle = 0.0f;

		return;
	}
	
	float_t flFinalPitch = 0.0f;
	switch ( g_Settings->m_iPitchMode )
	{
		case 1: flFinalPitch = 89.0f; break;
		case 2: flFinalPitch = -89.0f; break;
		case 3: flFinalPitch = -540.0f; break;
		case 4: flFinalPitch = 540.0f; break;
		default: break;
	}

	if ( flFinalPitch )
		if ( m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] == ANTIAIM_FULL )
			g_PacketManager->GetModifableCommand( )->m_angViewAngles.pitch = flFinalPitch;
	
	bool bReScanSolution = (int32_t)(m_Globals()->m_flRealTime) % 2;

	if ( bReScanSolution )
		FindTarget();

	if ( m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] )
	{
		float_t flFinalYaw = g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw;
		if ( m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] == ANTIAIM_FULL )
		{
			flFinalYaw = g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw + 180.0f;

			if ( fnManualSide(m_iManualSide) )
			{
				flFinalYaw = Math::NormalizeAngle(flFinalYaw + (m_iManualSide < 2 ? 90.0f * m_iManualSide : (m_iDesyncSide > 0 ? Settings->m_iYawOffset : Settings->m_iLeftYawOffset)));
			}
			else if ( m_aTarget )
			{
				bool bActiveKnife = false;

				auto pWeapon = m_aTarget->m_hActiveWeapon().Get();

				if ( pWeapon )			
					if ( pWeapon->IsKnife( ) )
						bActiveKnife = true;
				

				if ( bActiveKnife ) {
					if ( m_aTarget->GetShootPosition().DistTo(g_Globals.m_LocalPlayer->m_vecOrigin()) < 90.f )
						flFinalYaw = Math::NormalizeAngle(GetTargetYaw(m_aTarget) - 180.f);		
				}
				else
				{
					if ( !g_Tools->IsBindActive(g_Settings->m_aFreestand) && !Settings->m_bTargets )
						flFinalYaw = Math::NormalizeAngle(flFinalYaw);
					else
					{
						if ( bReScanSolution )
						{
							flFinalYaw = m_flOptimizedYawAngle;
						}
						else
						{
							if ( !m_aTarget )
								flFinalYaw = Math::NormalizeAngle(flFinalYaw);
							else if ( g_Tools->IsBindActive(g_Settings->m_aFreestand) )
							{
								if ( !FreestandAngle(flFinalYaw, m_aTarget) )
									if ( Settings->m_bTargets )
										flFinalYaw = GetTargetYaw( m_aTarget );
							}
							else
								flFinalYaw = GetTargetYaw(m_aTarget);

							// save optimized yaw.
							m_flOptimizedYawAngle = flFinalYaw;
						}
					}
				}
			}
		}

		if ( g_PacketManager->GetModifablePacket( ) )
		{
			if ( g_Tools->IsBindActive( g_Settings->m_aInverter ) )
				m_iDesyncSide = 1;
			else
				m_iDesyncSide = -1;

			m_bShouldUseAlternativeSide = false;
			if (Settings->m_bDesyncJitter)
			{
				m_iFinalSide = -m_iFinalSide;
				m_bShouldUseAlternativeSide = true;
			}

			if ( !m_bShouldUseAlternativeSide )
				m_iFinalSide = m_iDesyncSide;

			if ( m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] == ANTIAIM_FULL )
			{
				int iJitterLimit = Settings->m_iYawJitter;

				if (Settings->m_bRandomizeJitter)
					iJitterLimit = g_Tools->RandomInt(0, iJitterLimit);

				if ( m_bSwitch )
					flFinalYaw += iJitterLimit;
				else
					flFinalYaw -= iJitterLimit;
			}

			m_bSwitch = !m_bSwitch;
		}
		
		flFinalYaw += m_iFinalSide > 0 ? Settings->m_iYawOffset : Settings->m_iLeftYawOffset;
		g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw = Math::NormalizeAngle( flFinalYaw );
	}

	int32_t iDesyncSide = m_iDesyncSide;
	if ( m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] == ANTIAIM_DESYNC )
		iDesyncSide = -iDesyncSide;

	if ( m_bShouldUseAlternativeSide )
		iDesyncSide = m_iFinalSide;

	g_PacketManager->SetFakeAngles( g_PacketManager->GetModifableCommand( )->m_angViewAngles );
	if ( m_AntiAimConditions[ nCommand % MULTIPLAYER_BACKUP ] == ANTIAIM_DESYNC )
		if ( m_AntiAimConditions[ ( nCommand - 1 ) % MULTIPLAYER_BACKUP ] == ANTIAIM_FULL )
			return;

	float_t flDesyncAmount = m_iDesyncSide > 0 ? Settings->m_iRightDesync : Settings->m_iLeftDesync;
	float_t flFinalDesyncYaw = g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw;

	flDesyncAmount = std::clamp(flDesyncAmount, 0.f, VelocityDeltaFormula());

	if ( Settings->m_bRandomizeDesync )
		flDesyncAmount = g_Tools->RandomInt(0, flDesyncAmount);

	if ( !g_PacketManager->GetModifablePacket( ) )
	{
		if ( g_Globals.m_LocalData.m_flNextLowerBodyYawUpdateTime < m_Globals()->m_flCurTime )
		{
			flFinalDesyncYaw += flDesyncAmount * (iDesyncSide * 2);
			g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw = Math::NormalizeAngle(flFinalDesyncYaw);
		}
		else 
		{
			flFinalDesyncYaw += flDesyncAmount * (-iDesyncSide);
			g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw = Math::NormalizeAngle(flFinalDesyncYaw);
		}
	}
	else 
	{
		flFinalDesyncYaw += flDesyncAmount * (-iDesyncSide);
		g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw = Math::NormalizeAngle(flFinalDesyncYaw);
	}
}

float C_AntiAim::VelocityDeltaFormula()
{
	C_CSGOPlayerAnimationState* pState = g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO();

	if ( !pState ) //-V704
		return 0.0f;

	auto flSpeedModifer = std::clamp(pState->m_flSpeedAsPortionOfWalkTopSpeed, 0.0f, 1.0f);
	auto flAverageSpeedModifier = (pState->m_flWalkToRunTransition * -0.3f - 0.2f) * flSpeedModifer + 1.0f;
	if ( pState->m_flAnimDuckAmount ) //-V550
	{
		auto flMaxVelocity = std::clamp(pState->m_flSpeedAsPortionOfCrouchTopSpeed, 0.0f, 1.0f);
		auto flDuckSpeed = pState->m_flAnimDuckAmount * flMaxVelocity;
		flAverageSpeedModifier += flDuckSpeed * (0.5f - flAverageSpeedModifier);
	}

	return min(std::abs(pState->m_flYawDesyncAdjustment() * flAverageSpeedModifier), 58.f);
}

float WallThickness(Vector from, Vector to, C_BasePlayer* skip, C_BasePlayer* skip2)
{
	Vector endpos1, endpos2;

	CTraceFilter filterSafetyCheck;
	filterSafetyCheck.pSkip = skip;

	Ray_t ray;
	ray.Init(from, to);

	trace_t safetyCheck;
	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filterSafetyCheck, &safetyCheck);

	if (safetyCheck.DidHit())
		if (safetyCheck.hit_entity == skip2)
			return -1;

	std::array < uintptr_t, 5 > aSkipTwoEntities =
	{
		*(uintptr_t*)(g_Globals.m_AddressList.m_TraceFilterSkipTwoEntities),
		(uintptr_t)(g_Globals.m_LocalPlayer),
		NULL,
		NULL,
		NULL
	};

	aSkipTwoEntities[4] = (uintptr_t)(skip);

	trace_t trace1, trace2;
	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, (CTraceFilter*)(aSkipTwoEntities.data()), &trace1);

	if (trace1.DidHit())
		endpos1 = trace1.endpos;
	else
		return -1.f;

	ray.Init(to, from);
	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, (CTraceFilter*)(aSkipTwoEntities.data()), &trace2);

	if (trace2.DidHit())
		endpos2 = trace2.endpos;

	return endpos1.DistTo(endpos2);
}

bool C_AntiAim::FreestandAngle(float& Angle, C_BasePlayer* pFinalPlayer)
{
	float flEndRotation = FLT_MIN;
	float_t flThickest = -1.f;

	const auto flEyePos = g_Globals.m_LocalPlayer->m_vecOrigin() + g_Globals.m_LocalPlayer->m_vecViewOffset();
	const auto flHeadPos = g_Globals.m_LocalPlayer->HitboxPosition(HITBOX_HEAD, g_Globals.m_LocalData.m_aDesyncBones);
	const auto flOrigin = g_Globals.m_LocalPlayer->m_vecOrigin();
	const float flStep = DirectX::XM_2PI / 8.f;
	const float flRadius = fabs(Vector(flHeadPos - flOrigin).Length2D());

	if (!pFinalPlayer)
		return false;

	for (float flRotation = 0; flRotation < DirectX::XM_2PI; flRotation += flStep)
	{
		Vector vecNewHead( flRadius * cos( flRotation ) + flEyePos.x, flRadius * sin( flRotation ) + flEyePos.y, flEyePos.z );

		float flNewThickness = WallThickness( 
			pFinalPlayer->m_vecOrigin() + pFinalPlayer->m_vecViewOffset(),
			vecNewHead,
			pFinalPlayer,
			g_Globals.m_LocalPlayer
		);

		if ( flNewThickness > flThickest )
		{
			flThickest = flNewThickness;
			flEndRotation = flRotation;
		}
	}


	if ( flEndRotation == FLT_MIN )
		return false;

	Angle = Math::NormalizeAngle(flEndRotation);
	return true;
}

float C_AntiAim::GetTargetYaw(C_BasePlayer* pFinalPlayer)
{
	return Math::CalcAngle( g_Globals.m_LocalPlayer->GetAbsOrigin( ) + g_Globals.m_LocalPlayer->m_vecViewOffset( ), pFinalPlayer->GetAbsOrigin( ) ).yaw + 180.0f;
}

void C_AntiAim::JitterMove( )
{
	const float_t flSpeedPerTick = 5.0f / g_Networking->GetTickRate( );
	if ( !g_Settings->m_bJitterMove || ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN ) || ( g_Globals.m_LocalPlayer->m_bGunGameImmunity( ) )/* || ( *( g_Globals.m_Interfaces.m_GameRules ) )->IsFreezePeriod( )*/ )
		return;

	if ( g_Tools->IsBindActive( g_Settings->m_aSlowwalk ) )
		return;
    
	const float_t flTargetVelocity = ( ( g_PacketManager->GetModifableCommand( )->m_nCommand % g_Networking->GetTickRate( ) ) * flSpeedPerTick ) + 95.0f;
    if ( flTargetVelocity <= 100.0 ) 
		if ( flTargetVelocity >= 95.0 )
			m_flAccelerationSpeed = flTargetVelocity;
	else
		m_flAccelerationSpeed = 100.0;

    const float_t flWishVelocity = ( m_flAccelerationSpeed / 100.0 ) * g_Globals.m_LocalPlayer->GetMaxPlayerSpeed( );
    if ( flWishVelocity <= 0.0f )
		return;

    const float_t flMoveLength = Vector( g_PacketManager->GetModifableCommand( )->m_flForwardMove, g_PacketManager->GetModifableCommand( )->m_flSideMove, g_PacketManager->GetModifableCommand( )->m_flUpMove ).Length( );
    if ( flMoveLength < 10.0 || flMoveLength < flWishVelocity )
		return;

	g_PacketManager->GetModifableCommand( )->m_flForwardMove = ( g_PacketManager->GetModifableCommand( )->m_flForwardMove / flMoveLength ) * flWishVelocity;
    g_PacketManager->GetModifableCommand( )->m_flSideMove = ( g_PacketManager->GetModifableCommand( )->m_flSideMove / flMoveLength ) * flWishVelocity;
	g_PacketManager->GetModifableCommand( )->m_flUpMove = ( g_PacketManager->GetModifableCommand( )->m_flUpMove / flMoveLength ) * flWishVelocity;
}

void C_AntiAim::SlowWalk( bool bForcedWalk )
{
	bool bCanSlowwalk = true;
	if ( !g_Tools->IsBindActive( g_Settings->m_aSlowwalk ) )
		bCanSlowwalk = false;

	if ( ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN ) || ( g_Globals.m_LocalPlayer->m_bGunGameImmunity( ) ) /*|| ( *( g_Globals.m_Interfaces.m_GameRules ) )->IsFreezePeriod( )*/ )
		bCanSlowwalk = false;

	float_t flMovementLength = g_Globals.m_LocalPlayer->m_vecVelocity( ).Length2D( );
	if ( ( !bCanSlowwalk && !bForcedWalk ) || !( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND ) )
		return;

	float_t flMaxSpeed = g_Globals.m_LocalPlayer->GetMaxPlayerSpeed( );
	if (m_bWalkSwitch || bForcedWalk)
		flMaxSpeed *= g_Tools->IsBindActive(g_Settings->m_aCSlowwalk) && !bForcedWalk ? g_Settings->m_iSpeedWalk * 0.001 : 0.033999f;
	else
		flMaxSpeed *= g_Tools->IsBindActive(g_Settings->m_aCSlowwalk) && !bForcedWalk ? g_Settings->m_iSpeedWalk * 0.001 : 0.022999f;

	m_bWalkSwitch = !m_bWalkSwitch;
	if ( g_PacketManager->GetModifablePacket( ) && !bForcedWalk )
	{
		if ( g_Settings->m_bJitterMove )
		{
			if ( m_bMoveSwitch )
				g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_SPEED;
			else
				g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_SPEED;
		}

		m_bMoveSwitch = !m_bMoveSwitch;
	}

	g_Globals.m_AccuracyData.m_bSlowWalkState = true;

	float_t flDistanceToMinimalSpeed = flMovementLength / flMaxSpeed;
	if ( flDistanceToMinimalSpeed <= 0.0f )
		return;

	g_PacketManager->GetModifableCommand( )->m_flForwardMove /= flDistanceToMinimalSpeed;
	g_PacketManager->GetModifableCommand( )->m_flSideMove /= flDistanceToMinimalSpeed;
}

void C_AntiAim::LegMovement( )
{
	if ( !g_Settings->m_bAntiAim || !( ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND ) ) )
		return;
	
	int32_t Conditions = this->GetAntiAimConditions( );
	if ( Conditions != ANTIAIM_FULL )
		return;

	g_PacketManager->GetModifableCommand( )->m_nButtons &= ~( IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK );
	if ( !g_Settings->m_iLegMovement )
	{
		if ( g_PacketManager->GetModifableCommand( )->m_flForwardMove > 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_FORWARD;
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_BACK;
		}
		else if ( g_PacketManager->GetModifableCommand( )->m_flForwardMove < 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_FORWARD;
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_BACK;
		}

		if ( g_PacketManager->GetModifableCommand( )->m_flSideMove > 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_MOVERIGHT;
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_MOVELEFT;
		}
		else if ( g_PacketManager->GetModifableCommand( )->m_flSideMove < 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_MOVERIGHT;
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_MOVELEFT;
		}
	}
	else
	{
		if ( g_PacketManager->GetModifableCommand( )->m_flForwardMove > 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_FORWARD;
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_BACK;
		}
		else if ( g_PacketManager->GetModifableCommand( )->m_flForwardMove < 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_FORWARD;
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_BACK;
		}

		if ( g_PacketManager->GetModifableCommand( )->m_flSideMove > 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_MOVERIGHT;
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_MOVELEFT;
		}
		else if ( g_PacketManager->GetModifableCommand( )->m_flSideMove < 0.0f )
		{
			g_PacketManager->GetModifableCommand( )->m_nButtons &= ~IN_MOVERIGHT;
			g_PacketManager->GetModifableCommand( )->m_nButtons |= IN_MOVELEFT;
		}
	}
}

void C_AntiAim::Micromovement( ) 
{
	if ( !g_Settings->m_bAntiAim || g_PacketManager->GetModifableCommand( )->m_nButtons & ( IN_MOVELEFT | IN_MOVERIGHT | IN_BACK | IN_FORWARD ) )
		return;

	float_t flVelocityLength = g_Globals.m_LocalPlayer->m_vecVelocity( ).Length2D( );
	if ( flVelocityLength > 5.0f )
		return;
	
	float_t flSpeed = 2.3f;
	if ( ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_DUCK ) || g_Globals.m_Packet.m_bFakeDuck )
		flSpeed *= 3.3333334f;

	if ( g_PacketManager->GetModifableCommand( )->m_nCommand & 1 )
		flSpeed = -flSpeed;

	g_PacketManager->GetModifableCommand( )->m_flSideMove = flSpeed;
}

int32_t C_AntiAim::GetAntiAimConditions( )
{
	if ( !g_Settings->m_bAntiAim )
		return 0;

	C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !pCombatWeapon )
		return 0;

	if ( ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN ) /*|| ( *g_Globals.m_Interfaces.m_GameRules )->IsFreezePeriod( )*/ )
		return 0;

	if ( g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_LADDER )
	{
		if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_BACK )
			return 0;

		if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_FORWARD )
			return 0;
	}

	if ( g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_NOCLIP )
		return 0;

	if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_ATTACK )
	{
		bool bIsRevolver = false;
		if ( pCombatWeapon )
			if ( pCombatWeapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER )
				bIsRevolver = true;

		int32_t nShiftAmount = g_ExploitSystem->GetShiftAmount( );
		if ( nShiftAmount > 9 || bIsRevolver )
			nShiftAmount = 0;

		if ( g_Globals.m_LocalPlayer->CanFire( nShiftAmount, bIsRevolver ) )
			return 0;
	}

	if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_ATTACK2 )
		if ( pCombatWeapon->IsKnife( ) )
			return 0;

	if ( pCombatWeapon->IsGrenade( ) )
		if ( pCombatWeapon->m_fThrowTime( ) > 0.0f )
			return 0;

	if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_USE )
		return 2;

	return 1;
}