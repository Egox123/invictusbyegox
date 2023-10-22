#include "LocalAnimations.hpp"
#include "BoneManager.hpp"
#include "../Packet/PacketManager.hpp"
#include "../Prediction/EnginePrediction.hpp"

#include "../Settings.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Exploits/Exploits.hpp"

void C_LocalAnimations::Instance( )
{
	float_t flCurtime				= g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime;
	float_t flRealTime				= g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	float_t flAbsFrameTime			= g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime;
	float_t flFrameTime				= g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime;
	float_t flInterpolationAmount	= g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount;
	float_t iTickCount				= g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount;
	float_t iFrameCount				= g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount;

	C_AnimationLayer gPreviousLayers[13];
    std::memcpy(gPreviousLayers, g_Globals.m_LocalPlayer->m_AnimationLayers(), 13 * sizeof(C_AnimationLayer));

	const auto bk = g_Globals.m_LocalPlayer->m_flThirdpersonRecoil();

	const auto movestate = g_Globals.m_LocalPlayer->m_iMoveState();
	const auto iswalking = g_Globals.m_LocalPlayer->m_bIsWalking();

	g_Globals.m_LocalPlayer->m_iMoveState() = 0;
	g_Globals.m_LocalPlayer->m_bIsWalking() = false;

	auto m_forward = g_PacketManager->GetModifableCommand()->m_nButtons & IN_FORWARD;
	auto m_back = g_PacketManager->GetModifableCommand()->m_nButtons & IN_BACK;
	auto m_right = g_PacketManager->GetModifableCommand()->m_nButtons & IN_MOVERIGHT;
	auto m_left = g_PacketManager->GetModifableCommand()->m_nButtons & IN_MOVELEFT;
	auto m_walking = g_PacketManager->GetModifableCommand()->m_nButtons & IN_SPEED;

	bool m_walk_state = m_walking ? true : false;

	if ( g_PacketManager->GetModifableCommand()->m_nButtons & IN_DUCK || g_Globals.m_LocalPlayer->m_flDuckAmount() || g_Globals.m_LocalPlayer->m_fFlags() & FL_DUCKING )
		m_walk_state = false;
	else if ( m_walking )
	{
		float m_max_speed = g_Globals.m_LocalPlayer->GetMaxPlayerSpeed() * 0.52f;

		if ( m_max_speed + 25.f > g_Globals.m_LocalPlayer->m_vecVelocity().Length() )
			g_Globals.m_LocalPlayer->m_bIsWalking() = true;
	}

	auto move_buttons_pressed = g_PacketManager->GetModifableCommand()->m_nButtons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_RUN);

	bool holding_forward_and_back;
	bool holding_right_and_left;

	if ( !m_forward )
		holding_forward_and_back = false;
	else
		holding_forward_and_back = m_back;

	if ( !m_right )
		holding_right_and_left = false;
	else
		holding_right_and_left = m_left;

	if ( move_buttons_pressed )
	{
		if ( holding_forward_and_back )
		{
			if ( holding_right_and_left )
				g_Globals.m_LocalPlayer->m_iMoveState() = 0;
			else if ( m_right || m_left )
				g_Globals.m_LocalPlayer->m_iMoveState() = 2;
			else
				g_Globals.m_LocalPlayer->m_iMoveState() = 0;
		}
		else
		{
			if ( holding_forward_and_back )
				g_Globals.m_LocalPlayer->m_iMoveState() = 0;
			else if ( m_back || m_forward )
				g_Globals.m_LocalPlayer->m_iMoveState() = 2;
			else
				g_Globals.m_LocalPlayer->m_iMoveState() = 0;
		}
	}

	if ( g_Globals.m_LocalPlayer->m_iMoveState() == 2 && m_walk_state )
		g_Globals.m_LocalPlayer->m_iMoveState() = 1;

	g_Globals.m_LocalPlayer->m_fFlags( ) = g_PredictionSystem->GetNetvars( g_PacketManager->GetModifableCommand( )->m_nCommand ).m_fFlags;
	if ( g_Globals.m_LocalPlayer->m_flSpawnTime( ) != g_Globals.m_LocalData.m_flSpawnTime )
	{
		g_Globals.m_LocalData.m_iFlags[ 0 ] = g_Globals.m_LocalData.m_iFlags[ 1 ] = g_Globals.m_LocalPlayer->m_fFlags( );
		g_Globals.m_LocalData.m_iMoveType[ 0 ] = g_Globals.m_LocalData.m_iMoveType[ 1 ] = g_Globals.m_LocalPlayer->GetMoveType( );
		g_Globals.m_LocalData.m_flSpawnTime = g_Globals.m_LocalPlayer->m_flSpawnTime( );

		std::memcpy( &g_Globals.m_LocalData.m_FakeAnimationState, g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( ), sizeof( C_CSGOPlayerAnimationState ) );
		std::memcpy( g_Globals.m_LocalData.m_FakeAnimationLayers.data( ), g_Globals.m_LocalPlayer->m_AnimationLayers( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
		std::memcpy( g_Globals.m_LocalData.m_FakePoseParameters.data( ), g_Globals.m_LocalPlayer->m_aPoseParameters( ).data( ), sizeof( float_t ) * 24 );
	}

	if ( g_Settings->m_bHoldFireAnimation || !g_PacketManager->GetModifablePacket( ) )
	{
		if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_ATTACK )
		{
			bool bIsRevolver = false;
			if ( g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( ) )
				bIsRevolver = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( )->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER;

			if ( g_ExploitSystem->GetShiftCommand( ) != g_PacketManager->GetModifableCommand( )->m_nCommand || g_ExploitSystem->GetActiveExploit( ) != HIDESHOTS )
			{
				if ( g_Globals.m_LocalPlayer->CanFire( 1, bIsRevolver ) )
				{
					if ( g_Settings->m_bHoldFireAnimation )
					{
						g_Globals.m_LocalData.m_flShotTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
						g_Globals.m_LocalData.m_angForcedAngles = g_PacketManager->GetModifableCommand( )->m_angViewAngles;
					}
					else if ( !g_PacketManager->GetModifablePacket( ) )
					{
						g_Globals.m_LocalData.m_bDidShotAtChokeCycle = true;
						g_Globals.m_LocalData.m_angShotChokedAngle = g_PacketManager->GetModifableCommand( )->m_angViewAngles;
					}
				}
			}
		}
	}

	int32_t iFlags = g_Globals.m_LocalPlayer->m_fFlags( );
	float_t flLowerBodyYaw = g_Globals.m_LocalPlayer->m_flLowerBodyYaw( );
	float_t flDuckSpeed = g_Globals.m_LocalPlayer->m_flDuckSpeed( );
	float_t flDuckAmount = g_Globals.m_LocalPlayer->m_flDuckAmount( );
	QAngle angVisualAngles = g_Globals.m_LocalPlayer->m_angVisualAngles( );

	g_Globals.m_LocalPlayer->m_vecAbsVelocity( ) = g_Globals.m_LocalPlayer->m_vecVelocity( );
	g_Globals.m_LocalPlayer->m_angVisualAngles( ) = g_PacketManager->GetModifableCommand( )->m_angViewAngles;

	g_Globals.m_LocalPlayer->m_flThirdpersonRecoil( ) = g_Globals.m_LocalPlayer->m_aimPunchAngle( ).pitch * g_Globals.m_ConVars.m_WeaponRecoilScale->GetFloat( );

	if ( flRealTime - g_Globals.m_LocalData.m_flShotTime <= 0.25f && g_Settings->m_bHoldFireAnimation )
		if ( g_PacketManager->GetModifablePacket( ) )
			g_Globals.m_LocalPlayer->m_angVisualAngles( ) = g_Globals.m_LocalData.m_angForcedAngles;

	if ( g_Globals.m_LocalData.m_bDidShotAtChokeCycle )
		if ( g_PacketManager->GetModifablePacket( ) )
			g_Globals.m_LocalPlayer->m_angVisualAngles( ) = g_Globals.m_LocalData.m_angShotChokedAngle;

	g_Globals.m_LocalPlayer->m_angVisualAngles().roll = 0.0f;

	g_Globals.m_LocalPlayer->m_flLowerBodyYaw( ) = g_Globals.m_LocalData.m_flLowerBodyYaw;
	if ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN /*|| ( *g_Globals.m_Interfaces.m_GameRules )->IsFreezePeriod( )*/ )
		g_Globals.m_LocalPlayer->m_flLowerBodyYaw( ) = flLowerBodyYaw;

	if ( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame > g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount - 1 )
		g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame = g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount - 1;
	
	this->DoAnimationEvent( 0 );
	for ( int iLayer = 0; iLayer < ANIMATION_LAYER_COUNT; iLayer++ )
		g_Globals.m_LocalPlayer->m_AnimationLayers( )[ iLayer ].m_pOwner = g_Globals.m_LocalPlayer;
	
	g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flLastUpdateIncrement = fmaxf(m_Globals()->m_flCurTime - g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flLastUpdateTime, 0.f);

	g_Globals.m_LocalPlayer->m_AnimationLayers()[ ANIMATION_LAYER_ADJUST ].m_flWeight = 0.0f;
	g_Globals.m_LocalPlayer->m_AnimationLayers()[ ANIMATION_LAYER_ADJUST ].m_flCycle = 0.0f;
	g_Globals.m_LocalPlayer->m_AnimationLayers()[ ANIMATION_LAYER_LEAN ].m_flWeight = 0.5f;

	bool bClientSideAnimation = g_Globals.m_LocalPlayer->m_bClientSideAnimation( );
	g_Globals.m_LocalPlayer->m_bClientSideAnimation( ) = true;

	g_Globals.m_AnimationData.m_bAnimationUpdate = true;
	g_Globals.m_LocalPlayer->UpdateClientSideAnimation( );
	g_Globals.m_AnimationData.m_bAnimationUpdate = false;

	g_Globals.m_LocalPlayer->m_bClientSideAnimation( ) = bClientSideAnimation;

	g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO()->m_flPrimaryCycle			= gPreviousLayers[6].m_flCycle;
    g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO()->m_flMoveWeight			= gPreviousLayers[6].m_flWeight;
    g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO()->m_flStrafeChangeCycle		= gPreviousLayers[7].m_flCycle;
    g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO()->m_nStrafeSequence			= gPreviousLayers[7].m_nSequence;
    g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO()->m_flStrafeChangeWeight	= gPreviousLayers[7].m_flWeight;
    g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO()->m_flAccelerationWeight	= gPreviousLayers[12].m_flWeight;

	std::memcpy( g_Globals.m_LocalData.m_PoseParameters.data( ), g_Globals.m_LocalPlayer->m_aPoseParameters( ).data( ), sizeof( float_t ) * 24 );
	std::memcpy( g_Globals.m_LocalData.m_AnimationLayers.data( ), g_Globals.m_LocalPlayer->m_AnimationLayers( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );

	if ( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flVelocityLengthXY > 0.1f || fabs( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flVelocityLengthZ ) > 100.0f )
	{
		g_Globals.m_LocalData.m_flNextLowerBodyYawUpdateTime = flCurtime + 0.22f;
		if ( g_Globals.m_LocalData.m_flLowerBodyYaw != Math::NormalizeAngle( g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw ) )
			g_Globals.m_LocalData.m_flLowerBodyYaw = g_Globals.m_LocalPlayer->m_flLowerBodyYaw( ) = Math::NormalizeAngle( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flEyeYaw );
	}
	else if ( flCurtime > g_Globals.m_LocalData.m_flNextLowerBodyYawUpdateTime )
	{
		float_t flAngleDifference = Math::AngleDiff( Math::NormalizeAngle( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flFootYaw ), Math::NormalizeAngle( g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw ) );
		if ( fabsf( flAngleDifference ) > 35.0f )
		{
			g_Globals.m_LocalData.m_flNextLowerBodyYawUpdateTime = flCurtime + 1.1f;
			if ( g_Globals.m_LocalData.m_flLowerBodyYaw != Math::NormalizeAngle( g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw ) )
				g_Globals.m_LocalData.m_flLowerBodyYaw = g_Globals.m_LocalPlayer->m_flLowerBodyYaw( ) = Math::NormalizeAngle( g_PacketManager->GetModifableCommand( )->m_angViewAngles.yaw );
		}
	}

	if ( g_PacketManager->GetModifablePacket( ) )
	{
		bool bShouldSetupMatrix = true;
		if ( g_ExploitSystem->GetActiveExploit() == HIDESHOTS )
			if ( g_ExploitSystem->GetShiftCommand() == g_PacketManager->GetModifableCommand()->m_nCommand )
				bShouldSetupMatrix = false;

		if ( bShouldSetupMatrix )
			g_BoneManager->BuildMatrix(g_Globals.m_LocalPlayer, g_Globals.m_LocalData.m_aMainBones.data(), false);
	}

	g_Globals.m_LocalPlayer->m_fFlags() = iFlags;
	g_Globals.m_LocalPlayer->m_flDuckAmount() = flDuckAmount;
	g_Globals.m_LocalPlayer->m_flDuckSpeed() = flDuckSpeed;
	g_Globals.m_LocalPlayer->m_flLowerBodyYaw() = flLowerBodyYaw;
	g_Globals.m_LocalPlayer->m_angVisualAngles() = angVisualAngles;

	if ( g_PacketManager->GetModifablePacket( ) )
	{
		C_CSGOPlayerAnimationState AnimationState;
		std::memcpy( &AnimationState, g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( ), sizeof( C_CSGOPlayerAnimationState ) );

		// десинк лееры
		std::memcpy( g_Globals.m_LocalPlayer->m_AnimationLayers( ), GetFakeAnimationLayers( ).data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
		std::memcpy( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( ), &g_Globals.m_LocalData.m_FakeAnimationState, sizeof( C_CSGOPlayerAnimationState ) );
		std::memcpy( g_Globals.m_LocalPlayer->m_aPoseParameters( ).data( ), g_Globals.m_LocalData.m_FakePoseParameters.data( ), sizeof( float_t ) * MAXSTUDIOPOSEPARAM );

		// обновляем десинк
		int32_t iSimulationTicks = g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands( ) + 1;
		for ( int32_t iSimulationTick = 1; iSimulationTick <= iSimulationTicks; iSimulationTick++ )
		{	
			int32_t iTickCount = g_PacketManager->GetModifableCommand( )->m_nTickCount - ( iSimulationTicks - iSimulationTick );
			g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = TICKS_TO_TIME( iTickCount );
			g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = TICKS_TO_TIME( iTickCount );
			g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
			g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
			g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = iTickCount;
			g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = iTickCount;

			g_Globals.m_LocalPlayer->m_vecAbsVelocity( ) = g_Globals.m_LocalPlayer->m_vecVelocity( );
			g_Globals.m_LocalPlayer->m_flThirdpersonRecoil( ) = g_Globals.m_LocalPlayer->m_aimPunchAngle( ).pitch * g_Globals.m_ConVars.m_WeaponRecoilScale->GetFloat( );

			g_Globals.m_LocalPlayer->m_angVisualAngles( ) = g_PacketManager->GetFakeAngles( );
			if ( ( iSimulationTicks - iSimulationTick ) < 1 )
			{
				if ( flRealTime - g_Globals.m_LocalData.m_flShotTime <= 0.25f && g_Settings->m_bHoldFireAnimation )
					g_Globals.m_LocalPlayer->m_angVisualAngles( ) = g_Globals.m_LocalData.m_angForcedAngles;

				if ( g_Globals.m_LocalData.m_bDidShotAtChokeCycle )
					g_Globals.m_LocalPlayer->m_angVisualAngles( ) = g_Globals.m_LocalData.m_angShotChokedAngle;

				g_Globals.m_LocalPlayer->m_angVisualAngles( ).roll = 0;
			}

			if ( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame == g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount )
				g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_nLastUpdateFrame = g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount - 1;
		
			this->DoAnimationEvent( 1 );
			for ( int iLayer = 0; iLayer < ANIMATION_LAYER_COUNT; iLayer++ )
				g_Globals.m_LocalPlayer->m_AnimationLayers( )[ iLayer ].m_pOwner = g_Globals.m_LocalPlayer;

			bool bClientSideAnimation = g_Globals.m_LocalPlayer->m_bClientSideAnimation( );
			g_Globals.m_LocalPlayer->m_bClientSideAnimation( ) = true;

			g_Globals.m_AnimationData.m_bAnimationUpdate = true;
			g_Globals.m_LocalPlayer->UpdateClientSideAnimation( );
			g_Globals.m_AnimationData.m_bAnimationUpdate = false;

			g_Globals.m_LocalPlayer->m_bClientSideAnimation( ) = bClientSideAnimation;
		}

		// build desync matrix
		g_BoneManager->BuildMatrix( g_Globals.m_LocalPlayer, g_Globals.m_LocalData.m_aDesyncBones.data( ), false );

		// copy lag matrix
		std::memcpy( g_Globals.m_LocalData.m_aLagBones.data( ), g_Globals.m_LocalData.m_aDesyncBones.data( ), sizeof( matrix3x4_t ) * MAXSTUDIOBONES );

		// сэйивим десинк лееры
		std::memcpy( &g_Globals.m_LocalData.m_FakeAnimationState, g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( ), sizeof( C_CSGOPlayerAnimationState ) );
		std::memcpy( g_Globals.m_LocalData.m_FakeAnimationLayers.data( ), g_Globals.m_LocalPlayer->m_AnimationLayers( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
		std::memcpy( g_Globals.m_LocalData.m_FakePoseParameters.data( ), g_Globals.m_LocalPlayer->m_aPoseParameters( ).data( ), sizeof( float_t ) * 24 );

		// ресторим лееры
		std::memcpy( g_Globals.m_LocalPlayer->m_AnimationLayers( ), GetAnimationLayers( ).data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
		std::memcpy( g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( ), &AnimationState, sizeof( C_CSGOPlayerAnimationState ) );
		std::memcpy( g_Globals.m_LocalPlayer->m_aPoseParameters( ).data( ), g_Globals.m_LocalData.m_PoseParameters.data( ), sizeof( float_t ) * 24 );

		// муваем матрицы
		for ( int i = 0; i < MAXSTUDIOBONES; i++ )
			g_Globals.m_LocalData.m_vecBoneOrigins[ i ] = g_Globals.m_LocalPlayer->GetAbsOrigin( ) - g_Globals.m_LocalData.m_aMainBones[ i ].GetOrigin( );

		for ( int i = 0; i < MAXSTUDIOBONES; i++ )
			g_Globals.m_LocalData.m_vecFakeBoneOrigins[ i ] = g_Globals.m_LocalPlayer->GetAbsOrigin( ) - g_Globals.m_LocalData.m_aDesyncBones[ i ].GetOrigin( );

		// резет углов
		g_Globals.m_LocalData.m_bDidShotAtChokeCycle = false;
		g_Globals.m_LocalData.m_angShotChokedAngle = QAngle( 0, 0, 0 );
	}

	g_Globals.m_LocalPlayer->m_fFlags( ) = iFlags;
	g_Globals.m_LocalPlayer->m_flDuckAmount( ) = flDuckAmount;
	g_Globals.m_LocalPlayer->m_flDuckSpeed( ) = flDuckSpeed;
	g_Globals.m_LocalPlayer->m_flLowerBodyYaw( ) = flLowerBodyYaw;
	g_Globals.m_LocalPlayer->m_angVisualAngles( ) = angVisualAngles;
	g_Globals.m_LocalPlayer->m_iMoveState() = movestate;
	g_Globals.m_LocalPlayer->m_bIsWalking() = iswalking;
	g_Globals.m_LocalPlayer->m_flThirdpersonRecoil() = bk;

	// ресторим глобалсы
	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = flCurtime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = flRealTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = flAbsFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = flFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = iTickCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = iFrameCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = flInterpolationAmount;
}

void C_LocalAnimations::SetupShootPosition( )
{
	std::memcpy( g_Globals.m_LocalPlayer->m_AnimationLayers( ), g_LocalAnimations->GetAnimationLayers( ).data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	std::memcpy( g_Globals.m_LocalPlayer->m_aPoseParameters( ).data( ), g_Globals.m_LocalData.m_PoseParameters.data( ), sizeof( float_t ) * 24 );

	float flOldBodyPitch = g_Globals.m_LocalPlayer->m_aPoseParameters( )[ 12 ];
	Vector vecOldOrigin = g_Globals.m_LocalPlayer->GetAbsOrigin( );

	g_Globals.m_LocalPlayer->SetAbsoluteAngles( QAngle( 0.0f, g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flFootYaw, 0.0f ) );
	g_Globals.m_LocalPlayer->SetAbsoluteOrigin( g_Globals.m_LocalPlayer->m_vecOrigin( ) );

	matrix3x4_t aMatrix[ MAXSTUDIOBONES ];

	g_Globals.m_LocalPlayer->m_aPoseParameters( )[ 12 ] = ( g_Globals.m_LocalPlayer->m_angEyeAngles( ).pitch + 89.0f ) / 178.0f;
	g_BoneManager->BuildMatrix( g_Globals.m_LocalPlayer, aMatrix, true );
	g_Globals.m_LocalPlayer->m_aPoseParameters( )[ 12 ] = flOldBodyPitch;
	
	g_Globals.m_LocalPlayer->SetAbsoluteOrigin( vecOldOrigin );
	std::memcpy( g_Globals.m_LocalPlayer->m_CachedBoneData( ).Base( ), aMatrix, sizeof( matrix3x4_t ) * g_Globals.m_LocalPlayer->m_CachedBoneData( ).Count( ) );
	
	g_Globals.m_LocalPlayer->ForceBoneCache( );
	g_Globals.m_LocalData.m_vecShootPosition = g_Globals.m_LocalPlayer->GetShootPosition( );
}

bool C_LocalAnimations::GetCachedMatrix( matrix3x4_t* aMatrix )
{
	std::memcpy( aMatrix, g_Globals.m_LocalData.m_aMainBones.data( ), sizeof( matrix3x4_t ) * g_Globals.m_LocalPlayer->m_CachedBoneData( ).Count( ) );
	return true;
}

std::array < matrix3x4_t, 128 > C_LocalAnimations::GetDesyncMatrix( )
{
	return g_Globals.m_LocalData.m_aDesyncBones;
}

std::array < matrix3x4_t, 128 > C_LocalAnimations::GetLagMatrix( )
{
	return g_Globals.m_LocalData.m_aLagBones;
}

void C_LocalAnimations::DoAnimationEvent( int type )
{
	if ( /*( *( g_Globals.m_Interfaces.m_GameRules ) )->IsFreezePeriod( ) ||*/ ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN ) )
	{
		g_Globals.m_LocalData.m_iMoveType[ type ] = MOVETYPE_NONE;
		g_Globals.m_LocalData.m_iFlags[ type ] = FL_ONGROUND;
	}

	C_AnimationLayer* pLandOrClimbLayer = &g_Globals.m_LocalPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ];
	if ( !pLandOrClimbLayer )
		return;

	C_AnimationLayer* pJumpOrFallLayer = &g_Globals.m_LocalPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ];
	if ( !pJumpOrFallLayer )
		return;

	if ( g_Globals.m_LocalData.m_iMoveType[ type ] != MOVETYPE_LADDER && g_Globals.m_LocalPlayer->GetMoveType( ) == MOVETYPE_LADDER )
		g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->SetLayerSequence( pLandOrClimbLayer, ACT_CSGO_CLIMB_LADDER );
	else if ( g_Globals.m_LocalData.m_iMoveType[ type ] == MOVETYPE_LADDER && g_Globals.m_LocalPlayer->GetMoveType( ) != MOVETYPE_LADDER )
		g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->SetLayerSequence( pJumpOrFallLayer, ACT_CSGO_FALL );
	else
	{
		if ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_ONGROUND )
		{
			if ( !( g_Globals.m_LocalData.m_iFlags[ type ] & FL_ONGROUND ) )
				g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->SetLayerSequence( pLandOrClimbLayer, g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->m_flDurationInAir > 1.0f && type == 0 ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT );
		}
		else if ( g_Globals.m_LocalData.m_iFlags[ type ] & FL_ONGROUND )
		{
			if ( g_Globals.m_LocalPlayer->m_vecVelocity( ).z > 0.0f )
				g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->SetLayerSequence( pJumpOrFallLayer, ACT_CSGO_JUMP );
			else
				g_Globals.m_LocalPlayer->m_PlayerAnimStateCSGO( )->SetLayerSequence( pJumpOrFallLayer, ACT_CSGO_FALL );
		}
	}

	g_Globals.m_LocalData.m_iMoveType[ type ] = g_Globals.m_LocalPlayer->GetMoveType( );
	g_Globals.m_LocalData.m_iFlags[ type ] = g_PredictionSystem->GetNetvars( g_PacketManager->GetModifableCommand( )->m_nCommand ).m_fFlags;
}

void C_LocalAnimations::OnUpdateClientSideAnimation( )
{
	for ( int i = 0; i < MAXSTUDIOBONES; i++ )
		g_Globals.m_LocalData.m_aMainBones[ i ].SetOrigin( g_Globals.m_LocalPlayer->GetAbsOrigin( ) - g_Globals.m_LocalData.m_vecBoneOrigins[ i ] );

	for ( int i = 0; i < MAXSTUDIOBONES; i++ )
		g_Globals.m_LocalData.m_aDesyncBones[ i ].SetOrigin( g_Globals.m_LocalPlayer->GetAbsOrigin( ) - g_Globals.m_LocalData.m_vecFakeBoneOrigins[ i ] );

	std::memcpy( g_Globals.m_LocalPlayer->m_CachedBoneData( ).Base( ), g_Globals.m_LocalData.m_aMainBones.data( ), sizeof( matrix3x4_t ) * g_Globals.m_LocalPlayer->m_CachedBoneData( ).Count( ) );
	std::memcpy( g_Globals.m_LocalPlayer->GetBoneAccessor( ).GetBoneArrayForWrite( ), g_Globals.m_LocalData.m_aMainBones.data( ), sizeof( matrix3x4_t ) * g_Globals.m_LocalPlayer->m_CachedBoneData( ).Count( ) );
	
	return g_Globals.m_LocalPlayer->SetupBones_AttachmentHelper( );
}

std::array< C_AnimationLayer, 13 > C_LocalAnimations::GetAnimationLayers( )
{
	std::array< C_AnimationLayer, 13 > aOutput;

	std::memcpy( aOutput.data( )											,	g_Globals.m_LocalPlayer->m_AnimationLayers( )												, sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL )		,	&g_Globals.m_LocalData.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL )		, sizeof( C_AnimationLayer ) );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB )		,	&g_Globals.m_LocalData.m_AnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB )		, sizeof( C_AnimationLayer ) );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_ALIVELOOP )					,	&g_Globals.m_LocalData.m_AnimationLayers.at( ANIMATION_LAYER_ALIVELOOP )					, sizeof( C_AnimationLayer ) );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_LEAN )						,	&g_Globals.m_LocalData.m_AnimationLayers.at( ANIMATION_LAYER_LEAN )							, sizeof( C_AnimationLayer ) );

	return aOutput;
}

std::array< C_AnimationLayer, 13 > C_LocalAnimations::GetFakeAnimationLayers( )
{
	std::array< C_AnimationLayer, 13 > aOutput;

	std::memcpy( aOutput.data( )											,	g_Globals.m_LocalPlayer->m_AnimationLayers( )												, sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL )		,	&g_Globals.m_LocalData.m_FakeAnimationLayers.at( ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL )	, sizeof( C_AnimationLayer ) );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB )		,	&g_Globals.m_LocalData.m_FakeAnimationLayers.at( ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB )	, sizeof( C_AnimationLayer ) );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_ALIVELOOP )					,	&g_Globals.m_LocalData.m_FakeAnimationLayers.at( ANIMATION_LAYER_ALIVELOOP )				, sizeof( C_AnimationLayer ) );
	std::memcpy( &aOutput.at( ANIMATION_LAYER_LEAN )						,	&g_Globals.m_LocalData.m_FakeAnimationLayers.at( ANIMATION_LAYER_LEAN )						, sizeof( C_AnimationLayer ) );

	return aOutput;
}

void C_LocalAnimations::ResetData( )
{
	g_Globals.m_LocalData.m_aDesyncBones = { };
	g_Globals.m_LocalData.m_aMainBones = { };

	g_Globals.m_LocalData.m_vecNetworkedOrigin = Vector( 0, 0, 0 );
	g_Globals.m_LocalData.m_angShotChokedAngle = QAngle( 0, 0, 0 );
	g_Globals.m_LocalData.m_vecBoneOrigins.fill( Vector( 0, 0, 0 ) );
	g_Globals.m_LocalData.m_vecFakeBoneOrigins.fill( Vector( 0, 0, 0 ) );

	g_Globals.m_LocalData.m_bDidShotAtChokeCycle = false;

	g_Globals.m_LocalData.m_AnimationLayers.fill( C_AnimationLayer( ) );
	g_Globals.m_LocalData.m_FakeAnimationLayers.fill( C_AnimationLayer( ) );

	g_Globals.m_LocalData.m_PoseParameters.fill( 0.0f );
	g_Globals.m_LocalData.m_FakePoseParameters.fill( 0.0f );

	g_Globals.m_LocalData.m_flShotTime = 0.0f;
	g_Globals.m_LocalData.m_angForcedAngles = QAngle( 0, 0, 0 );

	g_Globals.m_LocalData.m_flLowerBodyYaw = 0.0f;
	g_Globals.m_LocalData.m_flNextLowerBodyYawUpdateTime = 0.0f;
	g_Globals.m_LocalData.m_flSpawnTime = 0.0f;
	
	g_Globals.m_LocalData.m_iFlags[ 0 ] = g_Globals.m_LocalData.m_iFlags[ 1 ] = 0;
	g_Globals.m_LocalData.m_iMoveType[ 0 ] = g_Globals.m_LocalData.m_iMoveType[ 1 ] = 0;
}