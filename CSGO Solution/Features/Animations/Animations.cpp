#include "Animations.hpp"
#include "BoneManager.hpp"
#include "../../SDK/Math/Math.hpp"
#include "../../Multithread/threading.h"
#include "../../Settings.hpp"

#define LAYER_TOLERANCE 0.002f

void ExtrapolateOrigin(C_BasePlayer* pPlayer, Vector& vecOrigin, Vector& vecVelocity, int& iFlags, bool bWasOnGround)
{
	if (vecVelocity.Length2D() <= 10.f || ( !(iFlags & FL_ONGROUND) && pPlayer->m_flFallVelocity() < 75.f ) || bWasOnGround )
		return;

	static float svJumpImpulse = g_Globals.m_Interfaces.m_CVar->FindVar(_S("sv_jump_impulse"))->GetFloat();

	if (!(iFlags & FL_ONGROUND))
		vecVelocity.z -= (m_Globals()->m_flFrameTime * g_Globals.m_ConVars.m_SvGravity->GetFloat());
	else if (bWasOnGround)
		vecVelocity.z = svJumpImpulse;

	const Vector vecMin = pPlayer->GetCollideable()->OBBMins();
	const Vector vecMax = pPlayer->GetCollideable()->OBBMaxs();
	const Vector vecSrc = vecOrigin;
	
	Vector vecEnd = vecSrc + (vecVelocity * m_Globals()->m_flFrameTime);

	Ray_t tRay;
	tRay.Init(vecSrc, vecEnd, vecMin, vecMax);

	trace_t tTrace;
	CTraceFilter tFilter;
	tFilter.pSkip = (void*)(pPlayer);

	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(tRay, MASK_PLAYERSOLID, &tFilter, &tTrace);

	if (tTrace.fraction != 1.f)
	{
		for (int i = 0; i < 2; i++)
		{
			vecVelocity -= tTrace.plane.normal * vecVelocity.Dot(tTrace.plane.normal);

			const float flDot = vecVelocity.Dot(tTrace.plane.normal);
			
			if (flDot < 0.f)
			{
				vecVelocity.x -= flDot * tTrace.plane.normal.x;
				vecVelocity.y -= flDot * tTrace.plane.normal.y;
				vecVelocity.z -= flDot * tTrace.plane.normal.z;
			}

			vecEnd = tTrace.endpos + (vecVelocity * (m_Globals()->m_flIntervalPerTick * (1.f - tTrace.fraction)));

			tRay.Init(tTrace.endpos, vecEnd, vecMin, vecMax);
			g_Globals.m_Interfaces.m_EngineTrace->TraceRay(tRay, MASK_PLAYERSOLID, &tFilter, &tTrace);

			if (tTrace.fraction == 1.f)
				break;
		}
	}

	vecOrigin	= tTrace.endpos;
	vecEnd		= tTrace.endpos;
	vecEnd.z   -= 2.f;

	tRay.Init(vecOrigin, vecEnd, vecMin, vecMax);
	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(tRay, MASK_PLAYERSOLID, &tFilter, &tTrace);

	iFlags &= ~(1 << 0);

	if (tTrace.DidHit() && tTrace.plane.normal.z > 0.7f)
		iFlags |= (1 << 0);
}

void RebuildSetupVelocity( C_BasePlayer* pPlayer, Vector m_vVelocity )
{
	return;

	auto pState = pPlayer->m_PlayerAnimStateCSGO();
	float flCurrentMoveDirGoalFeetDelta = 0.f;
	float flGoalMoveDirGoalFeetDelta = 0.f;

	if ( m_vVelocity.Length2D() > 5.f )
	{
		float velAngle = (atan2(-m_vVelocity.y, -m_vVelocity.x) * 180.0f) * (1.0f / M_PI);

		if ( velAngle < 0.0f )
			velAngle += 360.0f;

		flGoalMoveDirGoalFeetDelta = Math::AngleNormalize(Math::AngleDiff(velAngle, pState->m_flFootYaw));
	}

	if ( pState->m_flDurationMoving <= 0.1f && pState->m_flMoveYaw <= 0.1f )
	{
		pPlayer->m_aPoseParameters().at(4) = flGoalMoveDirGoalFeetDelta;
	}
	else
	{
		if ( pPlayer->m_AnimationLayers()[6].m_flWeight >= 1.0f )
		{
			pPlayer->m_aPoseParameters().at(4) = flGoalMoveDirGoalFeetDelta;
		}
		else
		{
			float flDuckSpeedClamp = std::clamp(pState->m_flSpeedAsPortionOfCrouchTopSpeed, 0.0f, 1.0f);
			float flRunningSpeed = std::clamp(pState->m_flSpeedAsPortionOfWalkTopSpeed, 0.0f, 1.0f);
			float flBiasMove = Math::Bias(Math::Lerp(pState->m_flAnimDuckAmount, flDuckSpeedClamp, flRunningSpeed), 0.18f);
			pPlayer->m_aPoseParameters().at(4) = Math::AngleNormalize(((flBiasMove + 0.1f) * pState->m_flMoveYawCurrentToIdeal) + pState->m_flMoveYawIdeal);
		}
	}

	float eye_goalfeet_delta = Math::AngleDiff(pState->m_flEyeYaw - pState->m_flFootYaw, 360.0f);

	if ( eye_goalfeet_delta < 0.0f || pState->m_flAimYawMax == 0.0f )
	{
		if ( pState->m_flAimYawMin != 0.0f )
			pPlayer->m_aPoseParameters().at(6) = (eye_goalfeet_delta / pState->m_flAimYawMin) * -58.0f;
	}
	else
	{
		pPlayer->m_aPoseParameters().at(6) = (eye_goalfeet_delta / pState->m_flAimYawMax) * 58.0f;
	}

	pPlayer->m_aPoseParameters().at(1) = pState->m_flSpeedAsPortionOfWalkTopSpeed;
	pPlayer->m_aPoseParameters().at(9) = pState->m_flInAirSmoothValue * pState->m_flAnimDuckAmount;

	return;

	float normalizedPitch = Math::AngleNormalize(pState->m_flEyePitch);

	if ( normalizedPitch <= 0.0f )
		pPlayer->m_aPoseParameters().at(7) = (normalizedPitch / pState->m_flAimPitchMax) * -89.0f;
	else
		pPlayer->m_aPoseParameters().at(7) = (normalizedPitch / pState->m_flAimPitchMin) * 89.0f;
}

float_t PredictLBY( std::deque < C_LagRecord > pLagRecords, C_BasePlayer* pPlayer )
{
	float_t flPredictedLby = pPlayer->m_flLowerBodyYaw();
	C_LagRecord* aLatestRecord = &pLagRecords[0];

	for ( int i = 0; i < 12; i++ )
	{
		C_LagRecord* aRecord = &pLagRecords[i];

		if ( TIME_TO_TICKS(fabs(pPlayer->m_flSimulationTime() - aRecord->m_SimulationTime)) <= 17 )
			continue;

		float_t flDelta = fabs(Math::NormalizeAngle(aRecord->m_LowerBodyYaw - aLatestRecord->m_LowerBodyYaw));
		float_t flDeltaTime = aRecord->m_SimulationTime - aLatestRecord->m_SimulationTime;

		if ( flDelta > 35.f )
		{
			flPredictedLby = aRecord->m_LowerBodyYaw;
			break;
		}

		if ( flDeltaTime >= 0.22f && flDeltaTime <= 0.5f )
		{
			flPredictedLby = aRecord->m_LowerBodyYaw + (flDelta / flDeltaTime) * m_Globals()->m_flIntervalPerTick;
			break;
		}
		else if ( flDeltaTime > 0.5f )
		{
			flPredictedLby = aRecord->m_LowerBodyYaw;
			break;
		}

		aLatestRecord = aRecord;
	}

	return flPredictedLby;
}

void C_AnimationSync::Instance( ClientFrameStage_t Stage )
{
	if ( Stage != ClientFrameStage_t::FRAME_NET_UPDATE_END )
		return;

	for ( int32_t iPlayerID = 1; iPlayerID <= m_Globals()->m_iMaxClients; iPlayerID++ )
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( iPlayerID );
		if ( !pPlayer || !pPlayer->IsPlayer( ) || !pPlayer->IsAlive( ) || pPlayer->m_iTeamNum( ) == g_Globals.m_LocalPlayer->m_iTeamNum( ) )
		{
			g_Globals.m_ResolverData.m_MissedShots[ iPlayerID ] = 0;
			continue;
		}

		bool bHasPreviousRecord = false;
		if ( pPlayer->m_flOldSimulationTime( ) >= pPlayer->m_flSimulationTime( ) )
		{
			if ( pPlayer->m_flOldSimulationTime( ) > pPlayer->m_flSimulationTime( ) )
				this->UnmarkAsDormant( iPlayerID );

			continue;
		}

		auto& LagRecords = g_Globals.m_CachedPlayerRecords[ iPlayerID ];
		if ( LagRecords.empty( ) )
			continue;

		C_LagRecord PreviousRecord = m_PreviousRecord[ iPlayerID ];
		if ( TIME_TO_TICKS( fabs( pPlayer->m_flSimulationTime( ) - PreviousRecord.m_SimulationTime ) ) <= 17 )
			bHasPreviousRecord = true;

		C_LagRecord& LatestRecord = LagRecords.back( );
		if ( this->HasLeftOutOfDormancy( iPlayerID ) )
			bHasPreviousRecord = false;

		if ( LatestRecord.m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP ).m_flCycle == PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP ).m_flCycle )
		{
			pPlayer->m_flSimulationTime( ) = pPlayer->m_flOldSimulationTime( );
			continue;
		}

		LatestRecord.m_UpdateDelay = TIME_TO_TICKS( pPlayer->m_flSimulationTime( ) - this->GetPreviousRecord( iPlayerID ).m_SimulationTime );	
		if ( LatestRecord.m_UpdateDelay > 17 )
			LatestRecord.m_UpdateDelay = 1;

		C_PlayerInfo PlayerInfo;
		g_Globals.m_Interfaces.m_EngineClient->GetPlayerInfo( iPlayerID, &PlayerInfo );

		if ( PlayerInfo.m_bIsFakePlayer || LatestRecord.m_UpdateDelay < 1 )
			LatestRecord.m_UpdateDelay = 1;

		Vector vecVelocity = LatestRecord.m_Velocity;
		if ( bHasPreviousRecord )
		{
			if (LatestRecord.m_UpdateDelay > 17)
				LatestRecord.m_UpdateDelay = 1;

			if (PlayerInfo.m_bIsFakePlayer || LatestRecord.m_UpdateDelay < 1)
				LatestRecord.m_UpdateDelay = 1;

			if (g_Settings->m_bSmoothAnimations)
			{
				const auto flSimtime = pPlayer->m_flSimulationTime() - pPlayer->m_flOldSimulationTime();
				const auto flSimulationTickDelta = ((flSimtime / m_Globals()->m_flIntervalPerTick) + 0.5) - 2;
				const auto flChokedTicks = (std::clamp(TIME_TO_TICKS(g_Globals.m_Interfaces.m_EngineClient->GetNetChannelInfo()->GetLatency(1) + g_Globals.m_Interfaces.m_EngineClient->GetNetChannelInfo()->GetLatency(0)) + m_Globals()->m_iTickCount - TIME_TO_TICKS(pPlayer->m_flSimulationTime() + g_LagCompensation->GetLerpTime()), 0, 100)) - flSimulationTickDelta;
				if (flChokedTicks)
				{
					if (g_Globals.m_CachedPlayerRecords[iPlayerID].size() >= 2)
					{
						int iItOfN = min(static_cast<int32_t>(flChokedTicks), 10);

						for (int32_t it = iItOfN; it > 0; it--)
						{
							auto vecOrigin = LatestRecord.m_Origin;
							auto iFlags = LatestRecord.m_Flags;

							ExtrapolateOrigin(pPlayer, vecOrigin, vecVelocity, iFlags, !(pPlayer->m_fFlags() & FL_ONGROUND) && (PreviousRecord.m_Flags & FL_ONGROUND));

							LatestRecord.m_SimulationTime += m_Globals()->m_flIntervalPerTick;
							LatestRecord.m_Origin = vecOrigin;
							LatestRecord.m_Velocity = vecVelocity;
						}
					}
				}
			}

			if ( !( LatestRecord.m_Flags & FL_ONGROUND ) )
			{
				vecVelocity = ( LatestRecord.m_Origin - this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_Origin ) * ( 1.0f / TICKS_TO_TIME( LatestRecord.m_UpdateDelay ) );

				float_t flWeight = 1.0f - LatestRecord.m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP ).m_flWeight;
				if ( flWeight > 0.0f )
				{
					float_t flPreviousRate = this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP ).m_flPlaybackRate;
					float_t flCurrentRate = LatestRecord.m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP ).m_flPlaybackRate;

					if ( flPreviousRate == flCurrentRate )
					{
						int32_t iPreviousSequence = this->GetPreviousRecord( pPlayer->EntIndex( ) ).m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP ).m_nSequence;
						int32_t iCurrentSequence = LatestRecord.m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP ).m_nSequence;

						if ( iPreviousSequence == iCurrentSequence )
						{
							float_t flSpeedNormalized = ( flWeight / 2.8571432f ) + 0.55f;
							if ( flSpeedNormalized > 0.0f )
							{
								float_t flSpeed = flSpeedNormalized * pPlayer->GetMaxPlayerSpeed( );
								if ( flSpeed > 0.0f )
									if ( vecVelocity.Length2D( ) > 0.0f )
										vecVelocity = ( vecVelocity / vecVelocity.Length( ) ) * flSpeed;
							}
						}
					} 
				}
				
				vecVelocity.z -= g_Globals.m_ConVars.m_SvGravity->GetFloat( ) * 0.5f * TICKS_TO_TIME( LatestRecord.m_UpdateDelay );
			}
			else
				vecVelocity.z = 0.0f; 
		}
		LatestRecord.m_Velocity = vecVelocity;

		g_Resolver->Initialize( pPlayer, &LatestRecord, &PreviousRecord );

		std::array < C_AnimationLayer, ANIMATION_LAYER_COUNT > AnimationLayers;
		std::array < float_t, MAXSTUDIOPOSEPARAM > PoseParameters;
		C_CSGOPlayerAnimationState AnimationState;

		// --------------------------
		std::memcpy( AnimationLayers.data( ), pPlayer->m_AnimationLayers( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
		std::memcpy( PoseParameters.data( ), pPlayer->m_aPoseParameters( ).data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );
		std::memcpy( &AnimationState, pPlayer->m_PlayerAnimStateCSGO( ), sizeof( AnimationState ) );

		// --------------------------
		for ( int32_t i = ROTATE_LEFT; i < ROTATE_LOW_LEFT; i++ )
		{
			// --------------------------
			LatestRecord.m_RotationMode = i;

			// --------------------------
			this->UpdatePlayerAnimations(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, LatestRecord.m_RotationMode);

			// --------------------------
			switch ( i )
			{
				case ROTATE_CENTER:
					g_Resolver->SetPlaybackRate( pPlayer->m_AnimationLayers( )[ 6 ].m_flPlaybackRate, 0 );
					break;
				case ROTATE_RIGHT:
					g_Resolver->SetPlaybackRate( pPlayer->m_AnimationLayers( )[ 6 ].m_flPlaybackRate, 1 );
					break;
				case ROTATE_LEFT:
					g_Resolver->SetPlaybackRate( pPlayer->m_AnimationLayers( )[ 6 ].m_flPlaybackRate, 2 );
					break;
			}	

			// --------------------------
			if ( i < ROTATE_LOW_LEFT )
				g_BoneManager->BuildMatrix( pPlayer, LatestRecord.m_Matricies[ i ].data( ), true );

			// --------------------------
			std::memcpy(pPlayer->m_AnimationLayers(), AnimationLayers.data(), sizeof(C_AnimationLayer)* ANIMATION_LAYER_COUNT);
			// --------------------------
			std::memcpy( pPlayer->m_aPoseParameters( ).data( ), PoseParameters.data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );
			std::memcpy( pPlayer->m_PlayerAnimStateCSGO( ), &AnimationState, sizeof( AnimationState ) );
		}

		if ( !LatestRecord.m_bIsShooting )
		{
			if ( LatestRecord.m_UpdateDelay > 1 && bHasPreviousRecord )
			{
				if ( !PlayerInfo.m_bIsFakePlayer )
				{
					g_Resolver->Instance();

					// rebuild his data.
					// RebuildSetupVelocity(pPlayer, vecVelocity);
				}

				this->UpdatePlayerAnimations( pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, LatestRecord.m_RotationMode );
			}
			else
				this->UpdatePlayerAnimations( pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, ROTATE_SERVER );
		}
		else
			this->UpdatePlayerAnimations( pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, ROTATE_SERVER );

		// форсим правильные лееры
		std::memcpy( pPlayer->m_AnimationLayers( ), AnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );

		// сэйвим позы
		std::memcpy( LatestRecord.m_PoseParameters.data( ), pPlayer->m_aPoseParameters( ).data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );

		// сетапим кости
		g_BoneManager->BuildMatrix( pPlayer, LatestRecord.m_Matricies[ ROTATE_SERVER ].data( ), false );

		// сэйвим кости
		for ( int i = 0; i < MAXSTUDIOBONES; i++ )
			m_BoneOrigins[ iPlayerID ][ i ] = pPlayer->GetAbsOrigin( ) - LatestRecord.m_Matricies[ ROTATE_SERVER ][ i ].GetOrigin( );

		// кэшируем кости
		std::memcpy( m_CachedMatrix[ iPlayerID ].data( ), LatestRecord.m_Matricies[ ROTATE_SERVER ].data( ), sizeof( matrix3x4_t ) * MAXSTUDIOBONES );

		// плеер вышел с дорманта
		this->UnmarkAsDormant( iPlayerID );

		// Reset resolver.
		g_Resolver->Reset( );
	}
}

void C_AnimationSync::UpdatePlayerAnimations( C_BasePlayer* pPlayer, C_LagRecord& LagRecord, C_LagRecord PreviousRecord, bool bHasPreviousRecord, int32_t iRotationMode )
{
	float_t flCurTime = m_Globals()->m_flCurTime;
	float_t flRealTime = m_Globals()->m_flRealTime;
	float_t flAbsFrameTime = m_Globals()->m_flAbsFrameTime;
	float_t flFrameTime = m_Globals()->m_flFrameTime;
	float_t iFrameCount = m_Globals()->m_iFrameCount;
	float_t iTickCount = m_Globals()->m_iTickCount;
	float_t flInterpolationAmount = m_Globals()->m_flInterpolationAmount;

	float_t flLowerBodyYaw = LagRecord.m_LowerBodyYaw;
	float_t flDuckAmount = LagRecord.m_DuckAmount;
	int32_t iFlags = LagRecord.m_Flags;
	int32_t iEFlags = pPlayer->m_iEFlags( );

	if ( this->HasLeftOutOfDormancy( pPlayer->EntIndex( ) ) )
	{
		float_t flLastUpdateTime = LagRecord.m_SimulationTime - m_Globals()->m_flIntervalPerTick;
		
#ifdef PLAYER_VELOCITY_FULLFIX

		pPlayer->m_vecVelocity() = LagRecord.m_Velocity;

		pPlayer->m_iEFlags() &= ~0x1800;

		if (pPlayer->m_fFlags() & FL_ONGROUND && pPlayer->m_vecVelocity().Length() > 0.0f && LagRecord.m_AnimationLayers.at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flWeight <= 0.0f)
			pPlayer->m_vecVelocity().Zero();

		pPlayer->m_vecAbsVelocity() = pPlayer->m_vecVelocity();

#else 
		if ( pPlayer->m_fFlags( ) & FL_ONGROUND )
		{
			pPlayer->m_PlayerAnimStateCSGO( )->m_bLanding = false;
			pPlayer->m_PlayerAnimStateCSGO( )->m_bOnGround = true;

			float_t flLandTime = 0.0f;
			if ( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flCycle > 0.0f && 
				 LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate > 0.0f )
			{ 
				int32_t iLandActivity = pPlayer->GetSequenceActivity( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_nSequence );
				if ( iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY )
				{
					flLandTime = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flCycle / LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate;
					if ( flLandTime > 0.0f )
						flLastUpdateTime = LagRecord.m_SimulationTime - flLandTime;
				}
			}

			LagRecord.m_Velocity.z = 0.0f;
		}
		else
		{
			float_t flJumpTime = 0.0f;
			if ( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle > 0.0f && 
				 LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate > 0.0f )
			{ 
				int32_t iJumpActivity = pPlayer->GetSequenceActivity( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_nSequence );
				if ( iJumpActivity == ACT_CSGO_JUMP )
				{
					flJumpTime = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle / LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate;
					if ( flJumpTime > 0.0f )
						flLastUpdateTime = LagRecord.m_SimulationTime - flJumpTime;
				}
			}
			
			pPlayer->m_PlayerAnimStateCSGO( )->m_bOnGround = false;
			pPlayer->m_PlayerAnimStateCSGO( )->m_flDurationInAir = flJumpTime - m_Globals()->m_flIntervalPerTick;
		}

		float_t flWeight = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flWeight;
		if ( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flPlaybackRate < 0.00001f )
			LagRecord.m_Velocity.Zero( );
		else
		{
			float_t flPostVelocityLength = pPlayer->m_vecVelocity( ).Length( );
			if ( flWeight > 0.0f && flWeight < 0.95f )
		{
				float_t flMaxSpeed = pPlayer->GetMaxPlayerSpeed( );
				if ( flPostVelocityLength > 0.0f )
				{
					float_t flMaxSpeedMultiply = 1.0f;
					if ( pPlayer->m_fFlags( ) & 6 )
						flMaxSpeedMultiply = 0.34f;
					else if ( pPlayer->m_bIsWalking( ) )
						flMaxSpeedMultiply = 0.52f;

					LagRecord.m_Velocity = ( LagRecord.m_Velocity / flPostVelocityLength ) * ( flWeight * ( flMaxSpeed * flMaxSpeedMultiply ) );
				}
			}
		}

		pPlayer->m_PlayerAnimStateCSGO( )->m_flLastUpdateTime = flLastUpdateTime;
#endif 
	}

	if ( bHasPreviousRecord )
	{
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeCycle = PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeWeight = PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_nStrafeSequence = PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_nSequence;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flPrimaryCycle = PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flMoveWeight = PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flAccelerationWeight = PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_LEAN ).m_flWeight;
		std::memcpy( pPlayer->m_AnimationLayers( ), PreviousRecord.m_AnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	}
	else
	{
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeCycle = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flStrafeChangeWeight = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_nStrafeSequence = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ).m_nSequence;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flPrimaryCycle = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flMoveWeight = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_MOVE ).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO( )->m_flAccelerationWeight = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_LEAN ).m_flWeight;
		std::memcpy( pPlayer->m_AnimationLayers( ), LagRecord.m_AnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	}

	if ( LagRecord.m_UpdateDelay > 1 )
	{
		int32_t iActivityTick = 0;
		int32_t iActivityType = 0;

		if ( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flWeight > 0.0f && PreviousRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flWeight <= 0.0f )
		{
			int32_t iLandSequence = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_nSequence;
			if ( iLandSequence > 2 )
			{
				int32_t iLandActivity = pPlayer->GetSequenceActivity( iLandSequence );
				if ( iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY )
				{
					float_t flCurrentCycle = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flCycle;
					float_t flCurrentRate = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate;
	
					if ( flCurrentCycle > 0.0f && flCurrentRate > 0.0f )
					{	
						float_t flLandTime = ( flCurrentCycle / flCurrentRate );
						if ( flLandTime > 0.0f )
						{
							iActivityTick = TIME_TO_TICKS( LagRecord.m_SimulationTime - flLandTime ) + 1;
							iActivityType = ACTIVITY_LAND;
						}
					}
				}
			}
		}

		if ( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle > 0.0f && LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate > 0.0f )
		{
			int32_t iJumpSequence = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_nSequence;
			if ( iJumpSequence > 2 )
			{
				int32_t iJumpActivity = pPlayer->GetSequenceActivity( iJumpSequence );
				if ( iJumpActivity == ACT_CSGO_JUMP )
				{
					float_t flCurrentCycle = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle;
					float_t flCurrentRate = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate;
	
					if ( flCurrentCycle > 0.0f && flCurrentRate > 0.0f )
					{	
						float_t flJumpTime = ( flCurrentCycle / flCurrentRate );
						if ( flJumpTime > 0.0f )
						{
							iActivityTick = TIME_TO_TICKS( LagRecord.m_SimulationTime - flJumpTime ) + 1;
							iActivityType = ACTIVITY_JUMP;
						}
					}
				}
			}
		}

		if ( LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flCycle > 0.0f )
			LagRecord.m_bJumped = true;

		for ( int32_t iSimulationTick = 1; iSimulationTick <= LagRecord.m_UpdateDelay; iSimulationTick++ )
		{
			float_t flSimulationTime = PreviousRecord.m_SimulationTime + TICKS_TO_TIME( iSimulationTick );
			m_Globals()->m_flCurTime = flSimulationTime;
			m_Globals()->m_flRealTime = flSimulationTime;
			m_Globals()->m_flFrameTime = m_Globals()->m_flIntervalPerTick;
			m_Globals()->m_flAbsFrameTime = m_Globals()->m_flIntervalPerTick;
			m_Globals()->m_iFrameCount = TIME_TO_TICKS( flSimulationTime );
			m_Globals()->m_iTickCount = TIME_TO_TICKS( flSimulationTime );
			m_Globals()->m_flInterpolationAmount = 0.0f;
			
			pPlayer->m_flDuckAmount( )		= Interpolate( PreviousRecord.m_DuckAmount, LagRecord.m_DuckAmount, iSimulationTick, LagRecord.m_UpdateDelay );
			pPlayer->m_vecVelocity( )		= Interpolate( PreviousRecord.m_Velocity, LagRecord.m_Velocity, iSimulationTick, LagRecord.m_UpdateDelay );
			pPlayer->m_vecAbsVelocity( )	= Interpolate( PreviousRecord.m_Velocity, LagRecord.m_Velocity, iSimulationTick, LagRecord.m_UpdateDelay );

			pPlayer->m_iEFlags() &= ~0x1800;

			if ( iSimulationTick < LagRecord.m_UpdateDelay )
			{
				int32_t iCurrentSimulationTick = TIME_TO_TICKS( flSimulationTime );
				if ( iActivityType > ACTIVITY_NONE )
				{
					bool bIsOnGround = pPlayer->m_fFlags( ) & FL_ONGROUND;
					if ( iActivityType == ACTIVITY_JUMP )
					{
						if ( iCurrentSimulationTick == iActivityTick - 1 )
							bIsOnGround = true;
						else if ( iCurrentSimulationTick == iActivityTick )
						{
							// reset animation layer
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_flCycle = 0.0f;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_nSequence = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_nSequence;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_flPlaybackRate = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ).m_flPlaybackRate;

							// reset player ground state
							bIsOnGround = false;
						}
						
					}
					else if ( iActivityType == ACTIVITY_LAND )
					{
						if ( iCurrentSimulationTick == iActivityTick - 1 )
							bIsOnGround = false;
						else if ( iCurrentSimulationTick == iActivityTick )
						{
							// reset animation layer
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_flCycle = 0.0f;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_nSequence = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_nSequence;
							pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_flPlaybackRate = LagRecord.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ).m_flPlaybackRate;

							// reset player ground state
							bIsOnGround = true;
						}
					}

					if ( bIsOnGround )
						pPlayer->m_fFlags( ) |= FL_ONGROUND;
					else
						pPlayer->m_fFlags( ) &= ~FL_ONGROUND;
				}

				switch (iRotationMode)
				{
					case ROTATE_RIGHT:
						pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw + (g_Resolver->m_bFinishedResolving ? g_Resolver->ResolveDelta() : 58));
						break;
					case ROTATE_LEFT:
						pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw - (g_Resolver->m_bFinishedResolving ? g_Resolver->ResolveDelta() : 58));
						break;
					case ROTATE_LOW_RIGHT:
						pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw + 13);
						break;
					case ROTATE_LOW_LEFT:
						pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw - 13);
						break;
				}

			}
			else
			{
				pPlayer->m_vecVelocity( ) = LagRecord.m_Velocity;
				pPlayer->m_vecAbsVelocity( ) = LagRecord.m_Velocity;
				pPlayer->m_flDuckAmount( ) = LagRecord.m_DuckAmount;
				pPlayer->m_fFlags( ) = LagRecord.m_Flags;

				pPlayer->m_iEFlags() &= ~0x1800;
			}

			if ( pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame > m_Globals()->m_iFrameCount - 1 )
				pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame = m_Globals()->m_iFrameCount - 1;

			bool bClientSideAnimation = pPlayer->m_bClientSideAnimation( );
			pPlayer->m_bClientSideAnimation( ) = true;
		
			for ( int32_t iLayer = NULL; iLayer < ANIMATION_LAYER_COUNT; iLayer++ )
				pPlayer->m_AnimationLayers( )[ iLayer ].m_pOwner = pPlayer;

			g_Globals.m_AnimationData.m_bAnimationUpdate = true;
			pPlayer->UpdateClientSideAnimation( );
			g_Globals.m_AnimationData.m_bAnimationUpdate = false;
		
			pPlayer->m_bClientSideAnimation( ) = bClientSideAnimation;
		}
	}
	else
	{
		m_Globals()->m_flCurTime = LagRecord.m_SimulationTime;
		m_Globals()->m_flRealTime = LagRecord.m_SimulationTime;
		m_Globals()->m_flFrameTime = m_Globals()->m_flIntervalPerTick;
		m_Globals()->m_flAbsFrameTime = m_Globals()->m_flIntervalPerTick;
		m_Globals()->m_iFrameCount = TIME_TO_TICKS( LagRecord.m_SimulationTime );
		m_Globals()->m_iTickCount = TIME_TO_TICKS( LagRecord.m_SimulationTime );
		m_Globals()->m_flInterpolationAmount = 0.0f;

		pPlayer->m_vecVelocity( ) = LagRecord.m_Velocity;
		pPlayer->m_vecAbsVelocity( ) = LagRecord.m_Velocity;

		pPlayer->m_iEFlags() &= ~0x1800;

		switch ( iRotationMode )
		{
		case ROTATE_RIGHT:
			pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw + (g_Resolver->m_bFinishedResolving ? g_Resolver->ResolveDelta() : 58));
			break;
		case ROTATE_LEFT:
			pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw - (g_Resolver->m_bFinishedResolving ? g_Resolver->ResolveDelta() : 58));
			break;
		case ROTATE_LOW_RIGHT:
			pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw + 13);
			break;
		case ROTATE_LOW_LEFT:
			pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw - 13);
			break;
		}

		if ( pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame > m_Globals()->m_iFrameCount - 1 )
			pPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame = m_Globals()->m_iFrameCount - 1;

		bool bClientSideAnimation = pPlayer->m_bClientSideAnimation( );
		pPlayer->m_bClientSideAnimation( ) = true;
		
		for ( int32_t iLayer = NULL; iLayer < ANIMATION_LAYER_COUNT; iLayer++ )
			pPlayer->m_AnimationLayers( )[ iLayer ].m_pOwner = pPlayer;

		g_Globals.m_AnimationData.m_bAnimationUpdate = true;
		pPlayer->UpdateClientSideAnimation( );
		g_Globals.m_AnimationData.m_bAnimationUpdate = false;
		
		pPlayer->m_bClientSideAnimation( ) = bClientSideAnimation;
	}

	pPlayer->m_flLowerBodyYaw( ) = flLowerBodyYaw;
	pPlayer->m_flDuckAmount( ) = flDuckAmount;
	pPlayer->m_iEFlags( ) = iEFlags;
	pPlayer->m_fFlags( ) = iFlags;

	m_Globals()->m_flCurTime = flCurTime;
	m_Globals()->m_flRealTime = flRealTime;
	m_Globals()->m_flAbsFrameTime = flAbsFrameTime;
	m_Globals()->m_flFrameTime = flFrameTime;
	m_Globals()->m_iFrameCount = iFrameCount;
	m_Globals()->m_iTickCount = iTickCount;
	m_Globals()->m_flInterpolationAmount = flInterpolationAmount;

	//pPlayer->InvalidatePhysicsRecursive( SEQUENCE_CHANGED );
	//pPlayer->InvalidatePhysicsRecursive( ANGLES_CHANGED );
	pPlayer->InvalidatePhysicsRecursive( ANIMATION_CHANGED );
}

bool C_AnimationSync::GetCachedMatrix( C_BasePlayer* pPlayer, matrix3x4_t* aMatrix )
{
	std::memcpy( aMatrix, m_CachedMatrix[ pPlayer->EntIndex( ) ].data( ), sizeof( matrix3x4_t ) * pPlayer->m_CachedBoneData( ).Count( ) );
	return true;
}

void C_AnimationSync::OnUpdateClientSideAnimation( C_BasePlayer* pPlayer )
{
	for ( int i = 0; i < MAXSTUDIOBONES; i++ )
		m_CachedMatrix[ pPlayer->EntIndex( ) ][ i ].SetOrigin( pPlayer->GetAbsOrigin( ) - m_BoneOrigins[ pPlayer->EntIndex( ) ][ i ] );

	std::memcpy( pPlayer->m_CachedBoneData( ).Base( ), m_CachedMatrix[ pPlayer->EntIndex( ) ].data( ), sizeof( matrix3x4_t ) * pPlayer->m_CachedBoneData( ).Count( ) );
	std::memcpy( pPlayer->GetBoneAccessor( ).GetBoneArrayForWrite( ), m_CachedMatrix[ pPlayer->EntIndex( ) ].data( ), sizeof( matrix3x4_t ) * pPlayer->m_CachedBoneData( ).Count( ) );
	
	return pPlayer->SetupBones_AttachmentHelper( );
}