#include "BoneManager.hpp"

void C_BoneManager::BuildMatrix(C_BasePlayer* pPlayer, matrix3x4_t* aMatrix, bool bSafeMatrix)
{
	// backup important!
	float_t flCurTime = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime;
	float_t flRealTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	float_t flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime;
	float_t flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime;
	int32_t iFrameCount = g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount;
	int32_t iTickCount = g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount;
	float_t flInterpolation = g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount;
	float_t flLeanBackup = pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight;

	// not mapped in csgo - unknown, but code is used in setup
	float_t* vSkeet1 = (float_t*)(uintptr_t(pPlayer) + (0x3ad0 + 0x4));
	float_t* vSkeet2 = (float_t*)(uintptr_t(pPlayer) + (0x6f20 + 0x4));

	// force full animation reset.
	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = pPlayer->m_flSimulationTime();
	g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = pPlayer->m_flSimulationTime();
	g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
	g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
	g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = -999;
	g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = TIME_TO_TICKS(pPlayer->m_flSimulationTime());
	g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = 0.0f;

	// compress to two layer
	std::array < float_t, 3 > aAnimationLayers =
	{
		pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight,
		pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flWeight,
		pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flCycle
	};

	// effect backup
	int32_t nClientEffects = pPlayer->m_nClientEffects();
	int32_t nLastSkipFramecount = pPlayer->m_nLastSkipFramecount();
	int32_t nOcclusionMask = pPlayer->m_nOcclusionMask();
	int32_t nOcclusionFrame = pPlayer->m_nOcclusionFrame();
	int32_t iEffects = pPlayer->m_fEffects();
	bool bJiggleBones = pPlayer->m_bJiggleBones();
	bool bMaintainSequenceTransition = pPlayer->m_bMaintainSequenceTransition();
	Vector vecAbsOrigin = pPlayer->GetAbsOrigin();

	int32_t iMask = BONE_USED_BY_ANYTHING;
	if ( bSafeMatrix )
		iMask = BONE_USED_BY_HITBOX;

	// dismiss whole bone cache
	pPlayer->InvalidateBoneCache();
	pPlayer->GetBoneAccessor().m_ReadableBones = NULL;
	pPlayer->GetBoneAccessor().m_WritableBones = NULL;

	float_t vBackupSkeet1 = *vSkeet1;
	float_t vBackupSkeet2 = *vSkeet2;

	*vSkeet1 = 0.0;
	*vSkeet2 = 0.0;

	if ( pPlayer->m_PlayerAnimStateCSGO() )
		pPlayer->m_PlayerAnimStateCSGO()->m_pWeaponLast = pPlayer->m_PlayerAnimStateCSGO()->m_pWeapon;

	pPlayer->m_nOcclusionFrame() = 0;
	pPlayer->m_nOcclusionMask() = 0;
	pPlayer->m_nLastSkipFramecount() = 0;

	if ( pPlayer != g_Globals.m_LocalPlayer )
		pPlayer->SetAbsoluteOrigin(pPlayer->m_vecOrigin());

	pPlayer->m_fEffects() |= EF_NOINTERP;
	pPlayer->m_nClientEffects() |= 2;
	pPlayer->m_bJiggleBones() = false;
	pPlayer->m_bMaintainSequenceTransition() = false;

	pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight = 0.0f;

	if ( bSafeMatrix )
		pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_pOwner = NULL;
	else if ( pPlayer == g_Globals.m_LocalPlayer )
	{
		if ( pPlayer->GetSequenceActivity(pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST )
		{
			pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flCycle = 0.0f;
			pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flWeight = 0.0f;
		}
	}

	g_Globals.m_AnimationData.m_bSetupBones = true;
	pPlayer->SetupBones(aMatrix, MAXSTUDIOBONES, iMask, pPlayer->m_flSimulationTime());
	g_Globals.m_AnimationData.m_bSetupBones = false;

	pPlayer->m_bMaintainSequenceTransition() = bMaintainSequenceTransition;
	pPlayer->m_nClientEffects() = nClientEffects;
	pPlayer->m_bJiggleBones() = bJiggleBones;
	pPlayer->m_fEffects() = iEffects;
	pPlayer->m_nLastSkipFramecount() = nLastSkipFramecount;
	pPlayer->m_nOcclusionFrame() = nOcclusionFrame;
	pPlayer->m_nOcclusionMask() = nOcclusionMask;

	*vSkeet1 = vBackupSkeet1;
	*vSkeet2 = vBackupSkeet2;

	if ( pPlayer != g_Globals.m_LocalPlayer )
		pPlayer->SetAbsoluteOrigin(vecAbsOrigin);

	pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight = aAnimationLayers[0];
	pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flWeight = aAnimationLayers[1];
	pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flCycle = aAnimationLayers[2];

	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = flCurTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = flRealTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = flFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = flAbsFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = iFrameCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = iTickCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = flInterpolation;
}

/*
// backup important!
	float_t flCurTime = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime;
	float_t flRealTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	float_t flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime;
	float_t flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime;
	int32_t iFrameCount = g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount;
	int32_t iTickCount = g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount;
	float_t flInterpolation = g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount;
	float_t flLeanBackup = pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight;

	// not mapped in csgo - unknown, but code is used in setup
	float_t* vSkeet1 = (float_t*)(uintptr_t(pPlayer) + (0x3ad0 + 0x4));
	float_t* vSkeet2 = (float_t*)(uintptr_t(pPlayer) + (0x6f20 + 0x4));

	// force full animation reset.
	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = pPlayer->m_flSimulationTime();
	g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = pPlayer->m_flSimulationTime();
	g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
	g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
	g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = -999;
	g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = TIME_TO_TICKS(pPlayer->m_flSimulationTime());
	g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = 0.0f;

	// compress to two layer
	float_t flLeanWeight = pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight;

	// effect backup
	int32_t nClientEffects = pPlayer->m_nClientEffects();
	int32_t nLastSkipFramecount = pPlayer->m_nLastSkipFramecount();
	int32_t nOcclusionMask = pPlayer->m_nOcclusionMask();
	int32_t nOcclusionFrame = pPlayer->m_nOcclusionFrame();
	int32_t iEffects = pPlayer->m_fEffects();
	bool bJiggleBones = pPlayer->m_bJiggleBones();
	bool bMaintainSequenceTransition = pPlayer->m_bMaintainSequenceTransition();
	Vector vecAbsOrigin = pPlayer->GetAbsOrigin();

	int32_t iMask = BONE_USED_BY_ANYTHING;
	if ( bSafeMatrix )
		iMask = BONE_USED_BY_HITBOX;

	// dismiss whole bone cache
	pPlayer->InvalidateBoneCache();
	pPlayer->GetBoneAccessor().m_ReadableBones = NULL;
	pPlayer->GetBoneAccessor().m_WritableBones = NULL;

	if ( pPlayer->m_PlayerAnimStateCSGO() )
		pPlayer->m_PlayerAnimStateCSGO()->m_pWeaponLast = pPlayer->m_PlayerAnimStateCSGO()->m_pWeapon;

	pPlayer->m_nOcclusionFrame() = 0;
	pPlayer->m_nOcclusionMask() = 0;
	pPlayer->m_nLastSkipFramecount() = 0;

	if ( pPlayer != g_Globals.m_LocalPlayer )
		pPlayer->SetAbsoluteOrigin(pPlayer->m_vecOrigin());

	pPlayer->m_fEffects() |= EF_NOINTERP;
	pPlayer->m_nClientEffects() |= 2;
	pPlayer->m_bJiggleBones() = false;
	pPlayer->m_bMaintainSequenceTransition() = false;

	pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight = 0.0f;

	if ( bSafeMatrix )
		pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_pOwner = NULL;
	else if ( pPlayer == g_Globals.m_LocalPlayer )
	{
		if ( pPlayer->GetSequenceActivity(pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST )
		{
			pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flCycle = 0.0f;
			pPlayer->m_AnimationLayers()[ANIMATION_LAYER_ADJUST].m_flWeight = 0.0f;
		}
	}

	float_t vBackupSkeet1 = *vSkeet1;
	float_t vBackupSkeet2 = *vSkeet2;

	*vSkeet1 = 0.0f;
	*vSkeet2 = 0.0f;

	// build bones.
	g_Globals.m_AnimationData.m_bSetupBones = true;
	pPlayer->SetupBones( aMatrix, MAXSTUDIOBONES, iMask, pPlayer->m_flSimulationTime( ) );
	g_Globals.m_AnimationData.m_bSetupBones = false;

	// backup all values.
	*vSkeet1 = vBackupSkeet1;
	*vSkeet2 = vBackupSkeet2;

	pPlayer->m_AnimationLayers()[ANIMATION_LAYER_LEAN].m_flWeight = flLeanWeight;

	pPlayer->m_bMaintainSequenceTransition() = bMaintainSequenceTransition;
	pPlayer->m_nClientEffects() = nClientEffects;
	pPlayer->m_bJiggleBones() = bJiggleBones;
	pPlayer->m_fEffects() = iEffects;
	pPlayer->m_nLastSkipFramecount() = nLastSkipFramecount;
	pPlayer->m_nOcclusionFrame() = nOcclusionFrame;
	pPlayer->m_nOcclusionMask() = nOcclusionMask;

	if ( pPlayer != g_Globals.m_LocalPlayer )
		pPlayer->SetAbsoluteOrigin( vecAbsOrigin );

	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = flCurTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime = flRealTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = flFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flAbsFrameTime = flAbsFrameTime;
	g_Globals.m_Interfaces.m_GlobalVars->m_iFrameCount = iFrameCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount = iTickCount;
	g_Globals.m_Interfaces.m_GlobalVars->m_flInterpolationAmount = flInterpolation;
*/