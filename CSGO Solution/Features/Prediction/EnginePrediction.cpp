#include "EnginePrediction.hpp"
#include "../Animations/LocalAnimations.hpp"
#include "../Packet/PacketManager.hpp"
#include "../Tools/Tools.hpp"

void C_PredictionSystem::Instance()
{
	m_flOldCurtime = g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime;
	m_flOldFrametime = g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime;

	m_bInPrediction_Backup = g_Globals.m_Interfaces.m_Prediction->m_bInPrediction();
	m_bIsFirstTimePredicted_Backup = g_Globals.m_Interfaces.m_Prediction->m_bIsFirstTimePredicted();

	if (!m_LastPredictedCmd || m_LastPredictedCmd->m_bHasBeenPredicted)
		m_nTickBase++;
	else
		m_nTickBase = g_Globals.m_LocalPlayer->m_nTickBase();

	g_Globals.m_Interfaces.m_Prediction->m_bInPrediction() = true;
	g_Globals.m_Interfaces.m_Prediction->m_bIsFirstTimePredicted() = false;

	/*if ( g_Globals.m_ConVars.m_OptimizePred->GetBool( ) )
		g_Globals.m_ConVars.m_OptimizePred->SetValue( false );*/

	return g_PredictionSystem->Repredict();
}

void C_PredictionSystem::Repredict()
{
	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = m_nTickBase * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;
	if ( !(g_Globals.m_LocalPlayer->m_fFlags() & FL_FROZEN) )
		g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;

	float_t flRecoilIndex = 0.0f;
	float_t flAccuracyPenalty = 0.0f;

	Vector vecPreviousOrigin = g_Globals.m_LocalPlayer->GetAbsOrigin();
	if ( g_Globals.m_LocalPlayer->m_hActiveWeapon().Get() )
	{
		flRecoilIndex = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_flRecoilIndex();
		flAccuracyPenalty = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_fAccuracyPenalty();
	}

	g_Globals.m_LocalPlayer->m_vecAbsVelocity() = g_Globals.m_LocalPlayer->m_vecVelocity();
	g_Globals.m_LocalPlayer->SetAbsoluteOrigin(g_Globals.m_LocalPlayer->m_vecOrigin());

	if ( g_Globals.m_Interfaces.m_ClientState->m_nDeltaTick() > 0 )
		g_Globals.m_Interfaces.m_Prediction->Update(g_Globals.m_Interfaces.m_ClientState->m_nDeltaTick(), true, g_Globals.m_Interfaces.m_ClientState->m_nLastCommandAck(), g_Globals.m_Interfaces.m_ClientState->m_nLastOutgoingCommand() + g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands());

	*(C_UserCmd**)((uintptr_t)(g_Globals.m_LocalPlayer) + 0x3348) = g_PacketManager->GetModifableCommand();
	(**((C_BasePlayer***)(g_Globals.m_AddressList.m_PredictionPlayer))) = g_Globals.m_LocalPlayer;
	(*(*(int32_t**)(g_Globals.m_AddressList.m_PredictionSeed))) = g_PacketManager->GetModifableCommand()->m_iRandomSeed;

	g_Globals.m_Interfaces.m_GameMovement->StartTrackPredictionErrors(g_Globals.m_LocalPlayer);
	g_Globals.m_Interfaces.m_MoveHelper->SetHost(g_Globals.m_LocalPlayer);

	static auto m_nImpulse = FindInDataMap(g_Globals.m_LocalPlayer->GetPredDescMap(), FNV32("m_nImpulse"));
	static auto m_nButtons = FindInDataMap(g_Globals.m_LocalPlayer->GetPredDescMap(), FNV32("m_nButtons"));
	static auto m_afButtonLast = FindInDataMap(g_Globals.m_LocalPlayer->GetPredDescMap(), FNV32("m_afButtonLast"));
	static auto m_afButtonPressed = FindInDataMap(g_Globals.m_LocalPlayer->GetPredDescMap(), FNV32("m_afButtonPressed"));
	static auto m_afButtonReleased = FindInDataMap(g_Globals.m_LocalPlayer->GetPredDescMap(), FNV32("m_afButtonReleased"));

	if ( g_PacketManager->GetModifableCommand()->m_nImpulse )
		*reinterpret_cast<uint32_t*>(uint32_t(g_Globals.m_LocalPlayer) + m_nImpulse) = g_PacketManager->GetModifableCommand()->m_nImpulse;

	int* buttons = reinterpret_cast<int*>(uint32_t(g_Globals.m_LocalPlayer) + m_nButtons);
	const int buttonsChanged = g_PacketManager->GetModifableCommand()->m_nButtons ^ *buttons;

	*reinterpret_cast<int*>(uint32_t(g_Globals.m_LocalPlayer) + m_afButtonLast) = *buttons;
	*reinterpret_cast<int*>(uint32_t(g_Globals.m_LocalPlayer) + m_nButtons) = g_PacketManager->GetModifableCommand()->m_nButtons;
	*reinterpret_cast<int*>(uint32_t(g_Globals.m_LocalPlayer) + m_afButtonPressed) = buttonsChanged & g_PacketManager->GetModifableCommand()->m_nButtons;
	*reinterpret_cast<int*>(uint32_t(g_Globals.m_LocalPlayer) + m_afButtonReleased) = buttonsChanged & ~g_PacketManager->GetModifableCommand()->m_nButtons;

	m_MoveData.m_flForwardMove = g_PacketManager->GetModifableCommand()->m_flForwardMove;
	m_MoveData.m_flSideMove = g_PacketManager->GetModifableCommand()->m_flSideMove;
	m_MoveData.m_flUpMove = g_PacketManager->GetModifableCommand()->m_flUpMove;
	m_MoveData.m_nButtons = g_PacketManager->GetModifableCommand()->m_nButtons;
	m_MoveData.m_vecViewAngles = Vector(
		g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch,
		g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw,
		g_PacketManager->GetModifableCommand()->m_angViewAngles.roll
	);
	m_MoveData.m_vecAngles = Vector(
		g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch,
		g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw,
		g_PacketManager->GetModifableCommand()->m_angViewAngles.roll
	);
	m_MoveData.m_nImpulseCommand = g_PacketManager->GetModifableCommand()->m_nImpulse;

	// pre think
	if ( g_Globals.m_LocalPlayer->PhsysicsRunThink(0) )
		g_Globals.m_LocalPlayer->PhyPreThink();

	// run think.
	int* thinktick = reinterpret_cast<int*>(reinterpret_cast<std::uint32_t>(g_Globals.m_LocalPlayer) + 0xFC);
	if ( *thinktick != -1 && *thinktick > 0 && *thinktick <= g_Globals.m_LocalPlayer->m_nTickBase() )
	{
		*thinktick = -1;

		auto fkUnknown = reinterpret_cast<void(__thiscall*)(int)>(g_Globals.m_AddressList.m_PhysicsUnknwnFn);
		fkUnknown(0);

		g_Globals.m_LocalPlayer->PhyThink();
	}

	g_Globals.m_Interfaces.m_Prediction->SetupMove(g_Globals.m_LocalPlayer, g_PacketManager->GetModifableCommand(), g_Globals.m_Interfaces.m_MoveHelper, &m_MoveData);
	g_Globals.m_Interfaces.m_GameMovement->ProcessMovement(g_Globals.m_LocalPlayer, &m_MoveData);
	g_Globals.m_Interfaces.m_Prediction->FinishMove(g_Globals.m_LocalPlayer, g_PacketManager->GetModifableCommand(), &m_MoveData);

	auto FinishPrediction = []() -> void
	{
		g_Globals.m_Interfaces.m_MoveHelper->SetHost(nullptr);
		g_Globals.m_Interfaces.m_GameMovement->FinishTrackPredictionErrors(g_Globals.m_LocalPlayer);
	};

	// rebuild post think
	/*auto RunPostThink = [](C_BasePlayer* player) -> int 
	{
		GetVirtual< void(__thiscall*)(void*) >(g_Globals.m_Interfaces.m_MDLCache, 33)(g_Globals.m_Interfaces.m_MDLCache);

		if ( player->IsAlive() || *reinterpret_cast<std::uint32_t*>( reinterpret_cast<std::uint32_t>(player) + 0x3A81 ) ) {
			GetVirtual< void(__thiscall*)(void*) >(player, 339)(player);

			if ( player->m_fFlags() & FL_ONGROUND )
				*reinterpret_cast<std::uintptr_t*>(std::uintptr_t(player) + 0x3014) = 0;

			if ( *reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint32_t>(player) + 0x28BC) == -1 )
				GetVirtual< void(__thiscall*)(void*, int) >(player, 218)(player, 0);

			GetVirtual< void(__thiscall*)(void*) >(player, 219)(player);

			static auto post_think_v_physics = reinterpret_cast<bool(__thiscall*)(C_BaseEntity*)>(n_utilities::find_pattern(g_Globals.m_ModuleList.m_ClientDll, "55 8B EC 83 E4 F8 81 ? ? ? ? ? 53 8B D9 56 57 83 ? ? ? ? ? ? 0F"));
			post_think_v_physics(player);
		}

		static auto simulate_player_simulated_entities = reinterpret_cast<bool(__thiscall*)(C_BaseEntity*)>(n_utilities::find_pattern(g_Globals.m_ModuleList.m_ClientDll, "56 8B F1 57 8B ? ? ? ? ? 83 EF 01 78 74"));
		simulate_player_simulated_entities(player);

		return GetVirtual< int(__thiscall*)(void*) >(g_Globals.m_Interfaces.m_MDLCache, 34)(g_Globals.m_Interfaces.m_MDLCache);
	};

	RunPostThink(g_Globals.m_LocalPlayer);*/

	g_Globals.m_LocalPlayer->PhyPostThink();
	FinishPrediction();

	*(C_UserCmd**)((uintptr_t)(g_Globals.m_LocalPlayer) + 0x3348) = NULL;
	(**((C_BasePlayer***)(g_Globals.m_AddressList.m_PredictionPlayer))) = NULL;
	(*(*(int32_t**)(g_Globals.m_AddressList.m_PredictionSeed))) = -1;

	m_LastPredictedCmd = g_PacketManager->GetModifableCommand();
	if (g_Globals.m_LocalPlayer->m_hActiveWeapon().Get())
	{
		g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_flRecoilIndex() = flRecoilIndex;
		g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->m_fAccuracyPenalty() = flAccuracyPenalty;
		g_Globals.m_LocalPlayer->m_hActiveWeapon().Get()->UpdateAccuracyPenalty();
	}

	g_LocalAnimations->SetupShootPosition();
	return g_Globals.m_LocalPlayer->SetAbsoluteOrigin(vecPreviousOrigin);
}

void C_PredictionSystem::ResetData()
{
	m_aNetvarData = { };

	m_flOldCurtime = 0.0f;
	m_flOldFrametime = 0.0f;
	m_flVelocityModifier = 1.0f;
	m_iLastCommand = -1;
	m_bInPrediction_Backup = false;
	m_bIsFirstTimePredicted_Backup = false;
	m_LastPredictedCmd = NULL;
	m_MoveData = C_MoveData();
	m_nTickBase = 0;
}

void C_PredictionSystem::SaveVelocityModifier()
{
	m_flVelocityModifier = g_Globals.m_LocalPlayer->m_flVelocityModifier();
}

void C_PredictionSystem::SaveCommand(int32_t nCommand)
{
	m_iLastCommand = nCommand;
}

float_t C_PredictionSystem::GetVelocityModifier(int32_t nCommand)
{
	float_t flVelocityModifier = m_flVelocityModifier + (min(((nCommand - 1) - m_iLastCommand), 1) * (g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick * 0.4f));
	if (flVelocityModifier > 1.0f)
		return 1.0f;

	if (!(g_Globals.m_LocalPlayer->m_fFlags() & FL_ONGROUND))
		return g_Globals.m_LocalPlayer->m_flVelocityModifier();

	return flVelocityModifier;
}

void C_PredictionSystem::SaveNetvars(int32_t nCommand)
{
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_fFlags = g_Globals.m_LocalPlayer->m_fFlags();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_hGroundEntity = g_Globals.m_LocalPlayer->m_hGroundEntity().Get();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_flDuckAmount = g_Globals.m_LocalPlayer->m_flDuckAmount();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_flDuckSpeed = g_Globals.m_LocalPlayer->m_flDuckSpeed();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_vecVelocity = g_Globals.m_LocalPlayer->m_vecVelocity();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_vecBaseVelocity = g_Globals.m_LocalPlayer->m_vecBaseVelocity();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_flFallVelocity = g_Globals.m_LocalPlayer->m_flFallVelocity();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_vecViewOffset = g_Globals.m_LocalPlayer->m_vecViewOffset();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_angAimPunchAngle = g_Globals.m_LocalPlayer->m_aimPunchAngle();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_vecAimPunchAngleVel = g_Globals.m_LocalPlayer->m_aimPunchAngleVel();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_angViewPunchAngle = g_Globals.m_LocalPlayer->m_viewPunchAngle();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_flVelocityModifier = g_Globals.m_LocalPlayer->m_flVelocityModifier();

	C_BaseCombatWeapon* pWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();
	if (!pWeapon)
		return;

	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_flRecoilIndex = pWeapon->m_flRecoilIndex();
	m_aNetvarData[nCommand % MULTIPLAYER_BACKUP].m_flAccuracyPenalty = pWeapon->m_fAccuracyPenalty();
}

inline bool IsVectorValid(Vector vecOriginal, Vector vecCurrent)
{
	Vector vecDelta = vecCurrent - vecOriginal;
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(vecDelta[i]) <= 0.03125f)
			return false;
	}

	return true;
}

inline bool IsVelocityValid(Vector vecOriginal, Vector vecCurrent)
{
	Vector vecDelta = vecCurrent - vecOriginal;
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(vecDelta[i]) <= 0.03125f)
			return false;
	}

	return true;
}

inline bool IsAngleValid(QAngle vecOriginal, QAngle vecCurrent)
{
	QAngle vecDelta = vecCurrent - vecOriginal;
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(vecDelta[i]) <= 0.03125f)
			return false;
	}

	return true;
}

inline bool IsFloatValid(float_t flOriginal, float_t flCurrent)
{
	if (fabsf(flCurrent - flOriginal) <= 0.03125f)
		return false;

	return true;
}

void C_PredictionSystem::RestoreNetvars(int32_t nCommand)
{
	volatile auto aNetVars = &m_aNetvarData[nCommand % MULTIPLAYER_BACKUP];

	if (aNetVars->m_nTickbase != g_Globals.m_LocalPlayer->m_nTickBase())
		return;

	if (IsVelocityValid(aNetVars->m_vecVelocity, g_Globals.m_LocalPlayer->m_vecVelocity()))
		g_Globals.m_LocalPlayer->m_vecVelocity() = aNetVars->m_vecVelocity;

	/*if (IsVectorValid(aNetVars->m_vecBaseVelocity, g_Globals.m_LocalPlayer->m_vecBaseVelocity()))
		g_Globals.m_LocalPlayer->m_vecBaseVelocity() = aNetVars->m_vecBaseVelocity;*/

	if (IsAngleValid(aNetVars->m_angAimPunchAngle, g_Globals.m_LocalPlayer->m_aimPunchAngle()))
		g_Globals.m_LocalPlayer->m_aimPunchAngle() = aNetVars->m_angAimPunchAngle;

	if (IsVectorValid(aNetVars->m_vecAimPunchAngleVel, g_Globals.m_LocalPlayer->m_aimPunchAngleVel()))
		g_Globals.m_LocalPlayer->m_aimPunchAngleVel() = aNetVars->m_vecAimPunchAngleVel;

	if (IsFloatValid(aNetVars->m_angViewPunchAngle.pitch, g_Globals.m_LocalPlayer->m_viewPunchAngle().pitch))
		g_Globals.m_LocalPlayer->m_viewPunchAngle().pitch = aNetVars->m_angViewPunchAngle.pitch;

	if (IsFloatValid(aNetVars->m_flFallVelocity, g_Globals.m_LocalPlayer->m_flFallVelocity()))
		g_Globals.m_LocalPlayer->m_flFallVelocity() = aNetVars->m_flFallVelocity;

	if (IsFloatValid(aNetVars->m_flDuckAmount, g_Globals.m_LocalPlayer->m_flDuckAmount()))
		g_Globals.m_LocalPlayer->m_flDuckAmount() = aNetVars->m_flDuckAmount;

	if (IsFloatValid(aNetVars->m_flDuckSpeed, g_Globals.m_LocalPlayer->m_flDuckSpeed()))
		g_Globals.m_LocalPlayer->m_flDuckSpeed() = aNetVars->m_flDuckSpeed;

	if (std::abs(g_Globals.m_LocalPlayer->m_vecViewOffset().z - aNetVars->m_vecViewOffset.z) <= 0.25f)
		g_Globals.m_LocalPlayer->m_vecViewOffset().z = aNetVars->m_vecViewOffset.z;

	if (std::abs(g_Globals.m_LocalPlayer->m_flVelocityModifier() - aNetVars->m_flVelocityModifier) <= 0.00625f)
		g_Globals.m_LocalPlayer->m_flVelocityModifier() = aNetVars->m_flVelocityModifier;

}

void C_PredictionSystem::ResetPacket()
{
	g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime = m_flOldCurtime;
	g_Globals.m_Interfaces.m_GlobalVars->m_flFrameTime = m_flOldFrametime;

	g_Globals.m_Interfaces.m_Prediction->m_bInPrediction() = m_bInPrediction_Backup;
	g_Globals.m_Interfaces.m_Prediction->m_bIsFirstTimePredicted() = m_bIsFirstTimePredicted_Backup;
}

void C_PredictionSystem::UpdatePacket()
{
	if (g_Globals.m_Interfaces.m_ClientState->m_nDeltaTick() < 0)
		return;

	return g_Globals.m_Interfaces.m_Prediction->Update(g_Globals.m_Interfaces.m_ClientState->m_nDeltaTick(), g_Globals.m_Interfaces.m_ClientState->m_nDeltaTick() > 0, g_Globals.m_Interfaces.m_ClientState->m_nLastCommandAck(), g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands() + g_Globals.m_Interfaces.m_ClientState->m_nLastOutgoingCommand());
}

void C_PredictionSystem::SaveViewmodelData()
{
	C_BaseViewModel* hViewmodel = g_Globals.m_LocalPlayer->m_hViewModel().Get();
	if (!hViewmodel)
		return;

	m_iAnimationParity = hViewmodel->m_iAnimationParity();
	m_iSequence = hViewmodel->m_iSequence();
	m_flCycle = hViewmodel->m_flCycle();
	m_flAnimTime = hViewmodel->m_flAnimTime();
}

void C_PredictionSystem::AdjustViewmodelData()
{
	C_BaseViewModel* hViewmodel = g_Globals.m_LocalPlayer->m_hViewModel().Get();
	if (!hViewmodel)
		return;

	if (m_iSequence != hViewmodel->m_iSequence() || m_iAnimationParity != hViewmodel->m_iAnimationParity())
		return;

	hViewmodel->m_flCycle() = m_flCycle;
	hViewmodel->m_flAnimTime() = m_flAnimTime;
}