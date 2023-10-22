#include "Animations.hpp"
#include "../../SDK/Math/Math.hpp"
#define MAX_RECORD_COUNT 10
#define RESOLVE_USING_VELOCITY_CALCULATION
#define RIGHT_SIDED(x) x == ROTATE_RIGHT || x == ROTATE_LOW_RIGHT

// Master behind of all this autism in here is me. AKA Egox.
// Whoever dumps this good luck using it, wish you good life.

#define CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MAX 58.0f
#define CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MIN -58.0f
#define CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX 90.0f
#define CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN -90.0f

#define CSGO_ANIM_LOWER_CATCHUP_IDLE	100.0f
#define CSGO_ANIM_AIM_NARROW_WALK	0.8f
#define CSGO_ANIM_AIM_NARROW_RUN	0.5f
#define CSGO_ANIM_AIM_NARROW_CROUCHMOVING	0.5f
#define CSGO_ANIM_LOWER_CATCHUP_WITHIN	3.0f
#define CSGO_ANIM_LOWER_REALIGN_DELAY	1.1f
#define CSGO_ANIM_READJUST_THRESHOLD	120.0f
#define EIGHT_WAY_WIDTH 22.5f

// I might release some deleted parts on dc in future.
// https://discord.gg/3JCpts8pbk

float C_DeSyncResolver::VelocityDeltaFormula(C_BasePlayer* pPlayer)
{
	C_CSGOPlayerAnimationState* pState = pPlayer->m_PlayerAnimStateCSGO();

	if ( !pState || !pPlayer ) //-V704
		return 0.0f;

	return m_flDelta.at(pPlayer->EntIndex());
}

void C_DeSyncResolver::Initialize(C_BasePlayer* pEnt, C_LagRecord* pRecord, C_LagRecord* pPreviousRecord)
{
	m_Ent = pEnt;
	m_Record = pRecord;
	m_PreviousRecord = pPreviousRecord;
	m_bFinishedResolving = false;

	const auto fVelocity = m_Ent->m_vecVelocity().Length2D();
	if ( m_Ent->m_fFlags() & FL_ONGROUND )
	{
		if ( fVelocity > 45.f )
			m_flDelta.at(pEnt->EntIndex()) = 35.f;
		else if ( fVelocity <= 45.f && fVelocity > 6.5f )
			m_flDelta.at(pEnt->EntIndex()) = 50.f;
		else
			m_flDelta.at(pEnt->EntIndex()) = 60.f;
	}
	else
		m_flDelta.at(pEnt->EntIndex()) = 20.f;
};

void C_DeSyncResolver::Reset()
{
	m_flAnimationHistory.at(m_Ent->EntIndex()).at(0) = m_flPlaybackRates.at(m_Ent->EntIndex()).at(0);
	m_flAnimationHistory.at(m_Ent->EntIndex()).at(1) = m_flPlaybackRates.at(m_Ent->EntIndex()).at(1);
	m_flAnimationHistory.at(m_Ent->EntIndex()).at(2) = m_flPlaybackRates.at(m_Ent->EntIndex()).at(2);

	m_bDesyncJitter[m_Ent->EntIndex()] = false;
	m_bFinishedResolving = false;
	m_Ent = nullptr;
	m_Record = nullptr;
	m_PreviousRecord = nullptr;

	std::memcpy (m_flPlaybackRates.data(), new std::array<float_t, 3>, 12U);
}

void C_DeSyncResolver::SortData(int32_t pIndex, bool pInvalid, bool p_bWipe, int32_t pSide)
{
	if ( pIndex < 0 )
		return;

	ResolverHistory_Type ResolverData;

	ResolverData.m_iSide = pSide == ROTATE_LEFT ? false : (pSide == ROTATE_LOW_LEFT ? false : true);
	ResolverData.m_bFreestanding = m_bFreestanding[pIndex];

	// delete previously added since its invalid.
	if ( p_bWipe ) {

		if ( m_bMovement[pIndex] == HISTORY_MODE::MOVEMENT )
			m_flResolverHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).erase(m_flResolverHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).begin());
		else if ( m_bMovement[pIndex] == HISTORY_MODE::STANDING )
			m_flResolverHistory.at(pIndex).at(HISTORY_MODE::STANDING).erase(m_flResolverHistory.at(pIndex).at(HISTORY_MODE::STANDING).begin());

		return;
	}

	if ( pInvalid )
	{
		if ( m_bMovement[pIndex] == HISTORY_MODE::MOVEMENT )
			m_flMissHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).insert(m_flMissHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).begin(), ResolverData),
			m_flMissHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).resize(10);

		else if ( m_bMovement[pIndex] == HISTORY_MODE::STANDING )
			m_flMissHistory.at(pIndex).at(HISTORY_MODE::STANDING).insert(m_flMissHistory.at(pIndex).at(HISTORY_MODE::STANDING).begin(), ResolverData);
		m_flMissHistory.at(pIndex).at(HISTORY_MODE::STANDING).resize(10);
	}
	else
	{
		if ( m_bMovement[pIndex] == HISTORY_MODE::MOVEMENT )
			m_flResolverHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).insert(m_flResolverHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).begin(), ResolverData),
			m_flMissHistory.at(pIndex).at(HISTORY_MODE::MOVEMENT).resize(25);

		else if ( m_bMovement[pIndex] == HISTORY_MODE::STANDING )
			m_flResolverHistory.at(pIndex).at(HISTORY_MODE::STANDING).insert(m_flResolverHistory.at(pIndex).at(HISTORY_MODE::STANDING).begin(), ResolverData),
			m_flMissHistory.at(pIndex).at(HISTORY_MODE::STANDING).resize(25);
	}

	return;
}

float CalculateAimMatrixWidthRange(const C_CSGOPlayerAnimationState& playerState)
{
	float flAimMatrixWidthRange = Math::Lerp(std::clamp(playerState.m_flSpeedAsPortionOfWalkTopSpeed, 0.0f, 1.0f), 1.0f,
		Math::Lerp(playerState.m_flWalkToRunTransition, CSGO_ANIM_AIM_NARROW_WALK, CSGO_ANIM_AIM_NARROW_RUN));

	if ( playerState.m_flAnimDuckAmount > 0.0f )
	{
		flAimMatrixWidthRange = Math::Lerp(playerState.m_flAnimDuckAmount * std::clamp(playerState.m_flSpeedAsPortionOfCrouchTopSpeed, 0.0f, 1.0f),
			flAimMatrixWidthRange, CSGO_ANIM_AIM_NARROW_CROUCHMOVING);
	}

	return flAimMatrixWidthRange;
}

void C_DeSyncResolver::ResolveAir(C_CSGOPlayerAnimationState* pState, int32_t iIndex) {

	const float_t flEyeDiff = Math::AngleNormalize(Math::AngleDiff(m_Ent->m_angEyeAngles().yaw, m_Ent->m_PlayerAnimStateCSGO()->m_flFootYaw));

	if (flEyeDiff > pState->m_flAimYawMax)
	{
		m_Record->m_RotationMode = ROTATE_LOW_RIGHT;
	}
	else
	{
		if (pState->m_flAimYawMin > flEyeDiff)
			m_Record->m_RotationMode = ROTATE_LOW_LEFT;
	}

	m_bMovement[iIndex] = HISTORY_MODE::AIR;
	m_iResolverType[iIndex] = 0;
}

void C_DeSyncResolver::ResolveOnShot(int32_t iIndex) {
	g_Globals.m_ResolverData.m_MissedShots[iIndex] = g_Globals.m_ResolverData.m_LastMissedShots[iIndex];
	m_flDelta.at(iIndex) = 0;
	m_iLastSide[iIndex] = m_Record->m_RotationMode;
	m_bFinishedResolving = true;
	g_Globals.m_ResolverData.m_LastMissedShots[iIndex] = g_Globals.m_ResolverData.m_MissedShots[iIndex];

	return;
}

void C_DeSyncResolver::BruteForce( int32_t iIndex ) {
	auto fnInvertAngle = [](int32_t iInput) -> ROTATE_MODE
		{
			switch (iInput)
			{
			case ROTATE_LEFT:
				return ROTATE_RIGHT;
				break;
			case ROTATE_RIGHT:
				return ROTATE_LEFT;
				break;
			case ROTATE_LOW_RIGHT:
				return ROTATE_LOW_LEFT;
				break;
			case ROTATE_LOW_LEFT:
				return ROTATE_LOW_RIGHT;
				break;
			}
		};

	// brute angles
	if (m_Record->m_RotationMode == g_Globals.m_ResolverData.m_BruteSide[iIndex])
	{
		// standing entities.
		if (m_bMovement[iIndex] == HISTORY_MODE::STANDING)
		{
			if (!((g_Globals.m_ResolverData.m_MissedShots[iIndex] % 2) + 1))
				m_Record->m_RotationMode = fnInvertAngle(m_Record->m_RotationMode);
			else
				g_Globals.m_ResolverData.m_MissedShots[iIndex] = 0;
		}
		else
		{
			if (g_Globals.m_ResolverData.m_MissedShots[iIndex] == 1) {
				if (Math::AngleDiff(m_Record->m_LowerBodyYaw, m_PreviousRecord->m_LowerBodyYaw) > 15)
					m_Record->m_RotationMode = fnInvertAngle(m_Record->m_RotationMode);
			}
			else {
				if (!g_Globals.m_ResolverData.m_DidHitPrevious[iIndex])
					m_Record->m_RotationMode = fnInvertAngle(m_Record->m_RotationMode);
			}
		}
	}
}

void C_DeSyncResolver::Instance()
{
	if ( !g_Globals.m_LocalPlayer )
		return;

	if ( !g_Globals.m_LocalPlayer->IsAlive() )
		return;

	const int32_t	index = m_Ent->EntIndex();
	const auto		pState = m_Ent->m_PlayerAnimStateCSGO();

	if ( std::fabsf(!pState->m_bOnGround || !(m_Ent->m_fFlags() & FL_ONGROUND)) )
		return ResolveAir(pState, index);

	// check if he is moving. - be a retard and change this :clown:
	if ( m_Record->m_Velocity.Length2D() > 5.f || m_Record->m_AnimationLayers.at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flWeight )
		m_bMovement[index] = HISTORY_MODE::MOVEMENT;
	else 
		m_bMovement[index] = HISTORY_MODE::STANDING;

	// os.<
	if ( m_Record->m_bIsShooting )
		return ResolveOnShot( index );

	// resolve based on movement.
	switch ( m_bMovement[index] )
	{
	case HISTORY_MODE::STANDING:
		ResolveStandingEntity();
		break;
	case HISTORY_MODE::MOVEMENT:
		ResolveMovingEntity();
		break;
	}

	if ( m_Record->m_RotationMode == ROTATE_LEFT )
		m_flDelta.at(index) = pState->m_flAimYawMax * CalculateAimMatrixWidthRange(*pState);
	else
		m_flDelta.at(index) = pState->m_flAimYawMin * CalculateAimMatrixWidthRange(*pState);
		
	BruteForce(index);

	m_iLastSide[index] = m_Record->m_RotationMode;
	m_bFinishedResolving = true;
	g_Globals.m_ResolverData.m_LastMissedShots[index] = g_Globals.m_ResolverData.m_MissedShots[index];
}

void C_DeSyncResolver::HardReset()
{
	if ( g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
		return;

	for ( int i = 0; i < 65; i++ )
	{
		m_flMissHistory.at(i).at(0).clear();
		m_flMissHistory.at(i).at(1).clear();
		m_flResolverHistory.at(i).at(0).clear();
		m_flResolverHistory.at(i).at(1).clear();
	}
}

float C_DeSyncResolver::ResolveDelta()
{
	if ( m_Record->m_RotationMode == ROTATE_SERVER )
		return Math::NormalizeAngle(m_Ent->m_angEyeAngles().yaw);

	return VelocityDeltaFormula(m_Ent);
}

void C_DeSyncResolver::DormancyReset(int32_t pIndex)
{
	m_flOptimizationTimer.at(pIndex) = FLT_MAX; /* force it! */
}

void C_DeSyncResolver::ResolveMovingEntity()
{
	// check history.
	if ( m_Record->m_AnimationLayers.at(6).m_flWeight * 1000.f != m_Record->m_AnimationLayers.at(6).m_flWeight * 1000.f )
	{
		m_Record->m_RotationMode = m_PreviousRecord->m_RotationMode;
		m_iResolverType[m_Ent->EntIndex()] = -1;

		return;
	};

	// deltas.
	const float_t flCenter = std::fabsf(m_Record->m_AnimationLayers.at(6).m_flPlaybackRate - m_flPlaybackRates.at(m_Ent->EntIndex()).at(0)); // center
	const float_t flRight = std::fabsf(m_Record->m_AnimationLayers.at(6).m_flPlaybackRate - m_flPlaybackRates.at(m_Ent->EntIndex()).at(1)); // right
	const float_t flLeft = std::fabsf(m_Record->m_AnimationLayers.at(6).m_flPlaybackRate - m_flPlaybackRates.at(m_Ent->EntIndex()).at(2)); // left.

	// history.
	const float_t flHistoryCenter = std::fabsf(m_PreviousRecord->m_AnimationLayers.at(6).m_flPlaybackRate - m_flAnimationHistory.at(m_Ent->EntIndex()).at(0));
	const float_t flHistoryRight = std::fabsf(m_PreviousRecord->m_AnimationLayers.at(6).m_flPlaybackRate - m_flAnimationHistory.at(m_Ent->EntIndex()).at(1));
	const float_t flHistoryLeft = std::fabsf(m_PreviousRecord->m_AnimationLayers.at(6).m_flPlaybackRate - m_flAnimationHistory.at(m_Ent->EntIndex()).at(2));
	
	if ( flRight > flCenter || flRight > flLeft || flHistoryRight < flRight )
	{
		m_Record->m_RotationMode = ROTATE_RIGHT;
		m_iResolverType[m_Ent->EntIndex()] = 91;
	}
	else
	{
		// update freestanding.
		ResolveFreestand();

		if ( Math::AngleNormalize(Math::AngleDiff(Math::AngleNormalize(m_Ent->m_flLowerBodyYaw()), Math::AngleNormalize(m_Ent->m_angEyeAngles().yaw))) > 0.0f )
		{
			if ( m_bFreestanding[m_Ent->EntIndex()] )
			{
				m_Record->m_RotationMode = ROTATE_RIGHT;
				m_iResolverType[m_Ent->EntIndex()] = 541;
			}
			else
			{
				if ( flRight > flCenter && flRight > flLeft )
				{
					m_Record->m_RotationMode = ROTATE_RIGHT;
					m_iResolverType[m_Ent->EntIndex()] = 551;
				}
				else
				{
					m_Record->m_RotationMode = flHistoryRight <= flRight ? ROTATE_LEFT : ROTATE_RIGHT;
					m_iResolverType[m_Ent->EntIndex()] = 542;
				}
			}
		}
		else
		{
			if ( m_bFreestanding[m_Ent->EntIndex()] )
			{
				m_Record->m_RotationMode = ROTATE_LEFT;
				m_iResolverType[m_Ent->EntIndex()] = 543;
			}
			else
			{
				if ( flLeft > flRight && flHistoryLeft > flRight && flLeft >= flCenter && flHistoryCenter >= flCenter )
				{
					m_Record->m_RotationMode = ROTATE_RIGHT;
					m_iResolverType[m_Ent->EntIndex()] = 552;
				}
				else
				{
					m_Record->m_RotationMode = ROTATE_RIGHT;
					m_iResolverType[m_Ent->EntIndex()] = 544; // causes most misses.
				}
			}
		}
	}

	// records.
	// deleted. - you can play with the records here and add more elements to them.
}

void C_DeSyncResolver::ResolveStandingEntity()
{
	// update freestanding.
	ResolveFreestand();

	if ( Math::AngleNormalize(Math::AngleDiff(Math::AngleNormalize(m_Ent->m_flLowerBodyYaw()), Math::AngleNormalize(m_Ent->m_angEyeAngles().yaw))) > 0.0f )
	{
		if ( m_bFreestanding[m_Ent->EntIndex()] )
		{
			m_Record->m_RotationMode = ROTATE_RIGHT;
			m_iResolverType[m_Ent->EntIndex()] = 41;
		}
		else
		{
			m_iResolverType[m_Ent->EntIndex()] = 42;
		}
	}
	else
	{
		if ( m_bFreestanding[m_Ent->EntIndex()] )
		{
			m_Record->m_RotationMode = ROTATE_LEFT;
			m_iResolverType[m_Ent->EntIndex()] = 43;
		}
		else
		{
			m_iResolverType[m_Ent->EntIndex()] = 44; // causes most misses.
		}
	}
}

bool C_DeSyncResolver::TraceVisible(const Vector& start, const Vector& end)
{
	trace_t trace;

	Ray_t ray;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip = g_Globals.m_LocalPlayer;

	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);

	return trace.hit_entity == m_Ent || trace.fraction == 1.0f; //-V550
}

void C_DeSyncResolver::ResolveFreestand()
{
	static float flLockSide[65] = {};

	if ( g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime - flLockSide[m_Ent->EntIndex()] > 2.0f )
	{
		auto bFirstVisible = TraceVisible(g_Globals.m_LocalData.m_vecShootPosition, m_Ent->HitboxPosition(HITBOX_HEAD, m_Record->m_Matricies.at(ROTATE_SERVER)));
		auto bSecondVisible = TraceVisible(g_Globals.m_LocalData.m_vecShootPosition, m_Ent->HitboxPosition(HITBOX_HEAD, m_Record->m_Matricies.at(ROTATE_SERVER)));

		if ( bFirstVisible != bSecondVisible )
		{
			m_bFreestanding[m_Ent->EntIndex()] = bSecondVisible;
			flLockSide[m_Ent->EntIndex()] = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime;
		}
		else
		{
			auto flPrimaryDistance = g_Globals.m_LocalData.m_vecShootPosition.DistTo(m_Ent->HitboxPosition(HITBOX_HEAD, m_Record->m_Matricies.at(ROTATE_SERVER)));
			auto flSecondaryDistance = g_Globals.m_LocalData.m_vecShootPosition.DistTo(m_Ent->HitboxPosition(HITBOX_HEAD, m_Record->m_Matricies.at(ROTATE_SERVER)));

			if ( fabs(flPrimaryDistance - flSecondaryDistance) > 1.0f )
				m_bFreestanding[m_Ent->EntIndex()] = flPrimaryDistance > flSecondaryDistance;
		}
	}
}

// removed.
void C_DeSyncResolver::RebuildMovement()
{
	return;
}
