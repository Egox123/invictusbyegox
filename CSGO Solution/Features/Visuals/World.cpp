#include "World.hpp"
#include "../Render.hpp"
#include "../Settings.hpp"
#include "../Tools/Tools.hpp"
#include "../Grenades/Warning.hpp"
#include "../RageBot/Autowall.hpp"

void C_World::Instance( ClientFrameStage_t Stage )
{
	if ( Stage != ClientFrameStage_t::FRAME_RENDER_START )
		return;

	if ( g_Settings->m_bUnhideConvars )
	{
		if ( !m_bDidUnlockConvars )
		{
			auto pIterator = **reinterpret_cast<ConCommandBase***>(reinterpret_cast<DWORD>(g_Globals.m_Interfaces.m_CVar) + 0x34);
			for (auto c = pIterator->m_pNext; c != nullptr; c = c->m_pNext) 
			{
				c->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
				c->m_nFlags &= ~FCVAR_HIDDEN;
			}

			m_bDidUnlockConvars = true;
		}
	}	
	
	if ( g_Settings->m_iViewmodelX != g_Globals.m_ConVars.m_ViewmodelX->GetInt( ) )
		g_Globals.m_ConVars.m_ViewmodelX->SetValue( (float)g_Settings->m_iViewmodelX );

	if ( g_Settings->m_iViewmodelY != g_Globals.m_ConVars.m_ViewmodelY->GetInt( ) )
		g_Globals.m_ConVars.m_ViewmodelY->SetValue( (float)g_Settings->m_iViewmodelY );

	if ( g_Settings->m_iViewmodelZ != g_Globals.m_ConVars.m_ViewmodelZ->GetInt( ) )
		g_Globals.m_ConVars.m_ViewmodelZ->SetValue( (float)g_Settings->m_iViewmodelZ );

	static uint32_t m_bUseCustomAutoExposureMin = g_NetvarManager->GetNetvar(FNV32("DT_EnvTonemapController"), FNV32("m_bUseCustomAutoExposureMin"));
	static uint32_t m_bUseCustomAutoExposureMax = g_NetvarManager->GetNetvar(FNV32("DT_EnvTonemapController"), FNV32("m_bUseCustomAutoExposureMax"));
	static uint32_t m_flCustomAutoExposureMin = g_NetvarManager->GetNetvar(FNV32("DT_EnvTonemapController"), FNV32("m_flCustomAutoExposureMin"));
	static uint32_t m_flCustomAutoExposureMax = g_NetvarManager->GetNetvar(FNV32("DT_EnvTonemapController"), FNV32("m_flCustomAutoExposureMax"));
	static uint32_t m_bUseCustomBloomScale = g_NetvarManager->GetNetvar(FNV32("DT_EnvTonemapController"), FNV32("m_bUseCustomBloomScale"));
	static uint32_t m_flCustomBloomScale = g_NetvarManager->GetNetvar(FNV32("DT_EnvTonemapController"), FNV32("m_flCustomBloomScale"));

	for ( int32_t i = 0; i < g_Globals.m_Interfaces.m_EntityList->GetHighestEntityIndex(); i++ )
	{
		C_ClientEntity* pBaseEntity = static_cast<C_ClientEntity*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntity(i));
		if ( !pBaseEntity || pBaseEntity->IsDormant() )
			continue;

		const auto client_class = pBaseEntity->GetClientClass();
		if ( client_class->m_ClassID == ClassId_CEnvTonemapController )
			continue; 

		*(bool*)((uintptr_t)pBaseEntity + m_bUseCustomBloomScale) = true;
		*(bool*)((uintptr_t)pBaseEntity + m_bUseCustomAutoExposureMin) = true;
		*(bool*)((uintptr_t)pBaseEntity + m_bUseCustomAutoExposureMax) = true;

		if ( g_Settings->m_bAmbientModulation ) {
			*(float*)((uintptr_t)pBaseEntity + m_flCustomBloomScale) = g_Settings->m_flBloom;
			*(float*)((uintptr_t)pBaseEntity + m_flCustomAutoExposureMin) = g_Settings->m_flExposure;
			*(float*)((uintptr_t)pBaseEntity + m_flCustomAutoExposureMax) = g_Settings->m_flExposure.Get() + 1;
		}
		else {
			*(float*)((uintptr_t)pBaseEntity + m_flCustomBloomScale) = 0.f;
			*(float*)((uintptr_t)pBaseEntity + m_flCustomAutoExposureMin) = 1.f;
			*(float*)((uintptr_t)pBaseEntity + m_flCustomAutoExposureMax) = 1.f;
		}
	}
	
	if ( g_Settings->m_bAmbientModulation )
		if ( g_Globals.m_ConVars.m_Ambient->GetFloat() != g_Settings->m_flAmbient )
			g_Globals.m_ConVars.m_Ambient->SetValue( g_Settings->m_flAmbient );

	g_Globals.m_Interfaces.m_CVar->FindVar(_S("con_filter_text"))->SetValue(_S("eqwoie2ue398129e8wuq12we9yw98d"));
	if (g_Globals.m_Interfaces.m_CVar->FindVar(_S("con_filter_enable"))->GetBool() != g_Settings->m_bFilterConsole)
	{
		g_Globals.m_Interfaces.m_CVar->FindVar(_S("con_filter_enable"))->SetValue(g_Settings->m_bFilterConsole);
		g_Globals.m_Interfaces.m_EngineClient->ExecuteClientCmd(_S("clear"));
	}

	this->RemoveHandShaking( );
	this->RemoveShadows( );
	this->RemoveSmokeAndPostProcess( );

	static C_Material* pBlurOverlay = g_Globals.m_Interfaces.m_MaterialSystem->FindMaterial( _S( "dev/scope_bluroverlay" ), TEXTURE_GROUP_OTHER );
	static C_Material* pScopeDirt = g_Globals.m_Interfaces.m_MaterialSystem->FindMaterial( _S( "models/weapons/shared/scope/scope_lens_dirt" ), TEXTURE_GROUP_OTHER );
	
	pBlurOverlay->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );	
	pScopeDirt->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, false );

	C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( pCombatWeapon )
	{
		if ( pCombatWeapon->IsSniper( ) )
		{
			if ( g_Globals.m_LocalPlayer->m_bIsScoped( ) )
			{
				if ( g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SCOPE ] )
				{
					pBlurOverlay->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
					pScopeDirt->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, true );
				}
			}
		}
	}

	this->Clantag( );
	if ( g_Settings->m_bForceCrosshair && !g_Globals.m_LocalPlayer->m_bIsScoped() )
		g_Globals.m_ConVars.m_WeaponDebugShowSpread->SetValue( 3 );
	else
		g_Globals.m_ConVars.m_WeaponDebugShowSpread->SetValue( 0 );

	this->DrawClientImpacts( );
	return this->SkyboxChanger( );
}

void C_World::OnBulletImpact( C_GameEvent* pEvent )
{
	Vector vecPosition = Vector( pEvent->GetInt( _S( "x" ) ), pEvent->GetInt( _S( "y" ) ), pEvent->GetInt( _S( "z" ) ) );

	C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( g_Globals.m_Interfaces.m_EngineClient->GetPlayerForUserID( pEvent->GetInt( _S( "userid" ) ) ) );
	if ( !pPlayer || !pPlayer->IsPlayer( ) || !pPlayer->IsAlive( ) )
		return;

	if ( pPlayer == g_Globals.m_LocalPlayer )
	{
		if ( g_Settings->m_bDrawServerImpacts )
			g_Globals.m_Interfaces.m_DebugOverlay->BoxOverlay
			(
				vecPosition,
				Vector( -2.0f, -2.0f, -2.0f ),
				Vector( 2.0f, 2.0f, 2.0f ),
				QAngle( 0.0f, 0.0f, 0.0f ),
				g_Settings->m_ServerImpacts->r( ),
				g_Settings->m_ServerImpacts->g( ),
				g_Settings->m_ServerImpacts->b( ),
				g_Settings->m_ServerImpacts->a( ),
				4.0f
			);

		if ( g_Settings->m_bDrawLocalTracers )
		{		
			C_BulletTrace BulletTrace;
	
			BulletTrace.m_bIsLocalTrace = true;
			BulletTrace.m_vecEndPosition = vecPosition;
			BulletTrace.m_vecStartPosition = g_Globals.m_LocalData.m_vecShootPosition;

			m_BulletTracers.emplace_back( BulletTrace );
		}

		return;
	}

	if ( !g_Settings->m_bDrawEnemyTracers || pPlayer->m_iTeamNum( ) == g_Globals.m_LocalPlayer->m_iTeamNum( ) )
		return;

	C_BulletTrace BulletTrace;
	
	BulletTrace.m_bIsLocalTrace = false;
	BulletTrace.m_vecEndPosition = vecPosition;
	BulletTrace.m_vecStartPosition = pPlayer->GetAbsOrigin( ) + pPlayer->m_vecViewOffset( );

	m_BulletTracers.emplace_back( BulletTrace );
}

void C_World::PreserveKillfeed( )
{
	if ( !g_Globals.m_Interfaces.m_EngineClient->IsInGame( ) || !g_Globals.m_Interfaces.m_EngineClient->IsConnected( ) )
		return;

	static PDWORD pHudDeathNotice = NULL;
	if ( !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive( ) )
	{
		pHudDeathNotice = NULL;
		return;
	}

	if ( g_Globals.m_LocalPlayer->m_fFlags( ) & FL_FROZEN || g_Globals.m_LocalPlayer->m_bGunGameImmunity( ) /*|| ( *g_Globals.m_Interfaces.m_GameRules )->IsFreezePeriod( )*/ )
	{
		pHudDeathNotice = NULL;
		return;
	}
	
	if ( !pHudDeathNotice )
	{
		pHudDeathNotice = g_Tools->FindHudElement( _S( "CCSGO_HudDeathNotice" ) );
		return;
	}

	PFLOAT pNoticeExpireTime = ( PFLOAT )( ( DWORD )( pHudDeathNotice ) + 0x50 );
	if ( pNoticeExpireTime )
		*pNoticeExpireTime = g_Settings->m_bPreserveKillfeed ? FLT_MAX : 1.5f;

	if ( g_Globals.m_RoundInfo.m_bShouldClearDeathNotices )
		( ( void( __thiscall* )( DWORD ) )( g_Globals.m_AddressList.m_ClearDeathList ) )( ( DWORD )( pHudDeathNotice ) - 20 );

	g_Globals.m_RoundInfo.m_bShouldClearDeathNotices = false;
}

// мне было лень думать и я кодил это на похер ибо диван сильнее
void C_World::Clantag( )
{
	auto apply = [](const char* tag) -> void
	{
		using Fn = int(__fastcall*)(const char*, const char*);
		static auto fn = reinterpret_cast<Fn>(g_Tools->FindPattern(GetModuleHandleA(_S("engine.dll")), _S("53 56 57 8B DA 8B F9 FF 15")));

		fn(tag, tag);
	};

	static auto removed = false;

#ifdef DEVELOPER_BUILD
	if (!g_Settings->m_bTagChanger && !removed)
	{
		removed = true;
		apply(_S(""));
		return;
	}
#else
	if ( !removed )
	{
		removed = true;
		apply(_S(""));
		return;
	}
#endif

	if (g_Settings->m_bTagChanger)
	{
		auto aNetChannel = g_Globals.m_Interfaces.m_EngineClient->GetNetChannelInfo( );

		if (!aNetChannel )
			return;

		static auto iTime = -1;

		auto iTicks = TIME_TO_TICKS(aNetChannel->GetAvgLatency(FLOW_OUTGOING)) + (float)g_Globals.m_Interfaces.m_GlobalVars->m_iTickCount; //-V807
		auto iIntervals = 0.5f / g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;

		auto iMainTime = (int)(iTicks / iIntervals) % 6;

		if ( iMainTime != iTime && !g_Globals.m_Interfaces.m_ClientState->m_nChokedCommands( ) )
		{
			auto tag = "invictus";
			switch ( iMainTime )
			{
			case 0: tag = "inv"; break;
			case 1: tag = "ic"; break;
			case 2: tag = "tus"; break;
			case 3: tag = "invictus"; break;
			case 4: tag = "invictus"; break;	
			case 5: tag = "invictus"; break;
			case 6: tag = "invictus"; break;
			}

			apply(tag);
			iTime = iMainTime;
		}

		removed = false;
	}
}

void C_World::PostFrame( ClientFrameStage_t Stage )
{		
	if (Stage == FRAME_RENDER_END)
	{
		this->FogModulation( );	
	}

	if ( Stage != ClientFrameStage_t::FRAME_START )
		return;

	return this->DrawBulletTracers( );
}

void C_World::VelocityGraph()
{
	if (!g_Settings->m_bVelocityGraph)
		return;

	if (!g_Globals.m_LocalPlayer)
		return;

	if (!g_Globals.m_LocalPlayer->IsAlive())
		return;

	static std::vector<float> velData(44, 0);

	Vector vecVelocity = g_Globals.m_LocalPlayer->m_vecVelocity();
	float currentVelocity = sqrt((vecVelocity.x * vecVelocity.x) + (vecVelocity.y * vecVelocity.y) + (vecVelocity.z * vecVelocity.z));

	velData.erase(velData.begin());
	velData.push_back(currentVelocity);


	int width, height;

	g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(width, height);

	g_Render->RenderLine(width / 2 - 100, (height + 550) / 2 + 25, width / 2 - 100, (height + 550) / 2 + 145, Color(100, 100, 100, 175), 2.f);
	g_Render->RenderLine(width / 2 - 115, (height + 550) / 2 + 130, width / 2 + 115, (height + 550) / 2 + 130, Color(100, 100, 100, 175), 2.f);
	g_Render->RenderText(std::to_string(vecVelocity.Length2D()).c_str(), ImVec2(width / 2 - 25, (height + 550) / 2 + 140), g_Settings->m_VelocityGraphCol, false, true, g_Globals.m_Fonts.m_SegoeUI );

	for (auto i = 0; i < velData.size() - 1; i++)
	{
		int cur = velData.at(i);
		int next = velData.at(i + 1);
		bool landed = velData.at(i) && !velData.at(i + 1);

		g_Render->RenderLine(width / 2 + (velData.size() * 5 / 2) - (i - 1) * 5.f, height / 2 - (std::clamp(cur, 0, 450) * .2f) + 400, width / 2 + (velData.size() * 5 / 2) - i * 5.f, height / 2 - (std::clamp(next, 0, 450) * .2f) + 400, g_Settings->m_VelocityGraphCol, 2.f);
	}
}

void C_World::OnRageBotFire( Vector vecStartPosition, Vector vecEndPosition )
{
	if ( !g_Settings->m_bDrawLocalTracers )
		return;

	C_BulletTrace BulletTrace;
	
	BulletTrace.m_bIsLocalTrace = true;
	BulletTrace.m_vecStartPosition = vecStartPosition;
	BulletTrace.m_vecEndPosition = vecEndPosition;

	m_BulletTracers.emplace_back( BulletTrace );
}

template < typename T >
static T __GetVirtual(void* pClass, int nIndex)
{
	return reinterpret_cast<T>((*(int**)pClass)[nIndex]);
}


void CallDrawSurfaceGradient(int x, int y, int w, int h, Color first, Color second, int type)
{
	if ( !g_Globals.m_Interfaces.m_Surface )
		return;

	auto filled_rect_fade = [&](bool reversed, float alpha) {
		using Fn = void(__thiscall*)(VOID*, int, int, int, int, unsigned int, unsigned int, bool);
		__GetVirtual< Fn >(g_Globals.m_Interfaces.m_Surface, 123) (
			g_Globals.m_Interfaces.m_Surface, x, y, x + w, y + h,
			reversed ? alpha : 0,
			reversed ? 0 : alpha,
			type == 0);
	};

	static auto blend = [](const Color& first, const Color& second, float t) -> Color {
		return Color(
			first.r() + t * (second.r() - first.r()),
			first.g() + t * (second.g() - first.g()),
			first.b() + t * (second.b() - first.b()),
			first.a() + t * (second.a() - first.a()));
	};

	if ( first.a() == 255 || second.a() == 255 ) {
		g_Globals.m_Interfaces.m_Surface->DrawSetColor(blend(first, second, 0.5f));
		g_Globals.m_Interfaces.m_Surface->DrawFilledRect(x, y, x + w, y + h);
	}

	g_Globals.m_Interfaces.m_Surface->DrawSetColor(first);
	filled_rect_fade(true, first.a());

	g_Globals.m_Interfaces.m_Surface->DrawSetColor(second);
	filled_rect_fade(false, second.a());
}

void C_World::DrawScopeLines( )
{
	if ( !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive( ) )
		return;
	
	C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( pCombatWeapon )
	{
		if ( pCombatWeapon->IsSniper( ) )
		{
			static int32_t iOffset = 0;

			if ( g_Globals.m_LocalPlayer->m_bIsScoped() )
			{
				int32_t iScreenSizeX, iScreenSizeY;
				g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(iScreenSizeX, iScreenSizeY);

				if ( !g_Settings->m_bCustomCrosshair )
				{
					g_Render->RenderLine(0, iScreenSizeY / 2, iScreenSizeX, iScreenSizeY / 2, Color(0, 0, 0), 1.0f);
					g_Render->RenderLine(iScreenSizeX / 2, 0, iScreenSizeX / 2, iScreenSizeY, Color(0, 0, 0), 1.0f);

					return;
				}
				int32_t iLength = g_Settings->m_iCrosshairAxisLength;
				int32_t iThick = min(2, g_Settings->m_iCrosshairAxisFill);

				if ( !(iThick % 3) )
					iThick += 1; // maybe even minus ( allignment purposes... ).

				Color colBegin = g_Settings->m_colCrosshair;
				Color colEnd = g_Settings->m_colCrosshairS;

				if ( iOffset < g_Settings->m_iCrosshairAxisSpacing )
					if ( !(m_Globals()->m_iTickCount % 3) )
						iOffset += 1;

				iThick /= 2;

				CallDrawSurfaceGradient(iScreenSizeX / 2 + iOffset,
					(iScreenSizeY / 2) - iThick,
					iLength, g_Settings->m_iCrosshairAxisFill, colBegin, colEnd, 0);

				CallDrawSurfaceGradient(iScreenSizeX / 2 - iLength - iOffset,
					iScreenSizeY / 2 - iThick,
					iLength, g_Settings->m_iCrosshairAxisFill, colEnd, colBegin, 0);

				CallDrawSurfaceGradient(iScreenSizeX / 2 - iThick,
					iScreenSizeY / 2 + iOffset,
					g_Settings->m_iCrosshairAxisFill, iLength, colBegin, colEnd, 1);

				CallDrawSurfaceGradient(iScreenSizeX / 2 - iThick,
					iScreenSizeY / 2 - iLength - iOffset,
					g_Settings->m_iCrosshairAxisFill, iLength, colEnd, colBegin, 1);
				
			}
			else
				iOffset = 0;
		}
	}
}

void C_World::Grenades( )
{
	if (!g_Globals.m_LocalPlayer)
		return;

	if (!g_Globals.m_LocalPlayer->IsAlive())
		return;

	if (!g_Settings->m_bPredictGrenades)
		return;

	g_GrenadePrediction->get_local_data().draw_local();

	static auto last_server_tick = g_Globals.m_Interfaces.m_ClientState->m_ClockDriftMgr().m_nServerTick;
	if (last_server_tick != g_Globals.m_Interfaces.m_ClientState->m_ClockDriftMgr().m_nServerTick) {
		g_GrenadePrediction->get_list().clear();

		last_server_tick = g_Globals.m_Interfaces.m_ClientState->m_ClockDriftMgr().m_nServerTick;
	}

	for (int32_t i = 1; i < g_Globals.m_Interfaces.m_EntityList->GetHighestEntityIndex(); i++)
	{
		C_BaseEntity* pBaseEntity = static_cast<C_BaseEntity*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntity(i));
		if (!pBaseEntity || pBaseEntity->IsDormant())
			continue;

		const auto client_class = pBaseEntity->GetClientClass();
		if (pBaseEntity->GetClientClass()->m_ClassID == ClassId_CInferno)
		{
			float_t flSpawnTime = *(float_t*)((DWORD)(pBaseEntity)+0x2D8);
			float_t flPercentage = ((flSpawnTime + 7.0f) - g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime) / 7.0f;

			Vector vecTimerPosition = pBaseEntity->m_vecOrigin();
			Vector vecScreenPosition = Vector(0, 0, 0);
			if (!g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition(pBaseEntity->m_vecOrigin(), vecScreenPosition))
			{
				if (g_Settings->m_GrenadeTimers && pBaseEntity->m_nExplodeEffectTickBegin() )
				{
					g_Render->RenderCircle2DFilled(Vector(vecTimerPosition.x, vecTimerPosition.y, vecTimerPosition.z), 180, 22, g_Settings->m_GrenadeWarningTimer);
					g_Render->RenderArc(vecScreenPosition.x, vecScreenPosition.y, 20, 0.f, 360 * flPercentage, Color::White, 3.f);
				}

				ImVec2 vecTextSize = g_Globals.m_Fonts.m_BigIcons->CalcTextSizeA(20.0f, FLT_MAX, NULL, _S("l"));
				g_Render->RenderText(_S("l"), ImVec2(vecScreenPosition.x - vecTextSize.x / 2 + 8, vecScreenPosition.y - vecTextSize.y / 2 - 2), Color(255, 0, 50), true, false, g_Globals.m_Fonts.m_BigIcons);
				g_Render->RenderCircle3D(pBaseEntity->m_vecOrigin(), 32, 170, Color(200, 0, 0));
			}
		}
		else if (pBaseEntity->GetClientClass()->m_ClassID == 157)
		{
			float_t flSpawnTime = *(float_t*)((DWORD)(pBaseEntity)+0x2D8);
			if (flSpawnTime > 0.0f )
			{
				float_t flAngle = (360 * (((flSpawnTime + 18.0f) - g_Globals.m_Interfaces.m_GlobalVars->m_flCurTime) / 15.0f)); // usually its too big for 45 degrees aka PI/4
				Vector vecTimerPosition = pBaseEntity->m_vecOrigin();

				Vector vecScreenPosition = Vector(0, 0, 0);
				if (!g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition(vecTimerPosition, vecScreenPosition))
				{
					if (g_Settings->m_GrenadeTimers && pBaseEntity->m_nExplodeEffectTickBegin() )
					{
						g_Render->RenderCircle2DFilled(Vector(vecTimerPosition.x, vecTimerPosition.y, vecTimerPosition.z), 180, 22, g_Settings->m_GrenadeWarningTimer);
						g_Render->RenderArc(vecScreenPosition.x, vecScreenPosition.y, 20, 0.f, flAngle, Color::White, 3.f);
					}

					ImVec2 vecTextSize = g_Globals.m_Fonts.m_BigIcons->CalcTextSizeA(20.0f, FLT_MAX, NULL, _S("k"));
					g_Render->RenderText(_S("k"), ImVec2(vecScreenPosition.x - vecTextSize.x / 2 + 6, vecScreenPosition.y - vecTextSize.y / 2 - 2), Color(0, 128, 255), true, false, g_Globals.m_Fonts.m_BigIcons);
					g_Render->RenderCircle3D(pBaseEntity->m_vecOrigin(), 32, 170, Color(0, 128, 255));
				}
			}
		}

		if (!client_class
			|| client_class->m_ClassID != 114 && client_class->m_ClassID != ClassId_CBaseCSGrenadeProjectile)
			continue;

		if (client_class->m_ClassID == ClassId_CBaseCSGrenadeProjectile) {
			const auto model = pBaseEntity->GetModel();
			if (!model)
				continue;

			const auto studio_model = g_Globals.m_Interfaces.m_ModelInfo->GetStudioModel(model);
			if (!studio_model
				|| std::string_view(studio_model->szName).find("fraggrenade") == std::string::npos)
				continue;
		}

		const auto handle = pBaseEntity->GetRefEHandle().ToLong();
		if (pBaseEntity->m_nExplodeEffectTickBegin())
		{
			g_GrenadePrediction->get_list().erase(handle);
			continue;
		}

		if (g_GrenadePrediction->get_list().find(handle) == g_GrenadePrediction->get_list().end()) {
			g_GrenadePrediction->get_list()[handle] =
				C_GrenadePrediction::data_t
				(
					reinterpret_cast<C_BaseCombatWeapon*>(pBaseEntity)->m_hThrower().Get(),
					client_class->m_ClassID == 114 ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
					pBaseEntity->m_vecOrigin(),
					reinterpret_cast<C_BasePlayer*>(pBaseEntity)->m_vecVelocity(),
					pBaseEntity->m_flCreationTime(),
					TIME_TO_TICKS(reinterpret_cast<C_BasePlayer*>(pBaseEntity)->m_flSimulationTime() - pBaseEntity->m_flCreationTime())
				);
		}

		if (g_GrenadePrediction->get_list().at(handle).draw())
			continue;			

		g_GrenadePrediction->get_list().erase(handle);
	}
}

void C_World::FogModulation()
{
	static auto fog_override = g_Globals.m_Interfaces.m_CVar->FindVar(_S("fog_override")); //-V807

	if (!g_Settings->m_bFog)
	{
		if (fog_override->GetBool())
			fog_override->SetValue(FALSE);

		return;
	}

	if (!fog_override->GetBool())
		fog_override->SetValue(TRUE);

	static auto fog_start = g_Globals.m_Interfaces.m_CVar->FindVar(_S("fog_start"));

	if (fog_start->GetInt())
		fog_start->SetValue(0);

	static auto fog_end = g_Globals.m_Interfaces.m_CVar->FindVar(_S("fog_end"));

	if (fog_end->GetInt() != g_Settings->m_iFogDistance)
		fog_end->SetValue(g_Settings->m_iFogDistance);

	static auto fog_maxdensity = g_Globals.m_Interfaces.m_CVar->FindVar(_S("fog_maxdensity"));

	if (fog_maxdensity->GetFloat() != (float)g_Settings->m_iFogDensity * 0.01f) //-V550
		fog_maxdensity->SetValue((float)g_Settings->m_iFogDensity * 0.01f);

	char buffer_color[12];
	sprintf_s(buffer_color, 12, "%i %i %i", g_Settings->m_bFogColor->r(), g_Settings->m_bFogColor->g(), g_Settings->m_bFogColor->b());

	static auto fog_color = g_Globals.m_Interfaces.m_CVar->FindVar(_S("fog_color"));

	if (strcmp(fog_color->GetString(), buffer_color)) //-V526
		fog_color->SetValue(buffer_color);
}

void C_World::AmbientModulation()
{
	if (!g_Settings->m_bAmbientModulation)
		return;
	
	C_BaseEntity* Entity = nullptr;

	for (int i = 1; i <= g_Globals.m_Interfaces.m_EntityList->GetHighestEntityIndex(); i++)  //-V807
	{
		auto Entity = static_cast<C_BaseEntity*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntity(i));

		if (!Entity)
			continue;

		if (Entity->IsPlayer())
			continue;

		if (Entity->IsDormant())
			continue;

		auto client_class = Entity->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID != 69) {
			Entity = nullptr;
			continue;
		}
	}

	if (!Entity)
		return;

	if (Entity == nullptr)
		return;

	/*Entity->set_m_bUseCustomBloomScale(TRUE);
	Entity->set_m_flCustomBloomScale(g_Settings->m_iBloom * 0.01f);

	Entity->set_m_bUseCustomAutoExposureMin(TRUE);
	Entity->set_m_flCustomAutoExposureMin(g_Settings->m_iExposure * 0.001f);

	Entity->set_m_bUseCustomAutoExposureMax(TRUE);
	Entity->set_m_flCustomAutoExposureMax(g_Settings->m_iExposure * 0.001f);*/
}

void C_World::RemoveHandShaking( )
{
	if ( g_Settings->m_bVMMovement )
	{
		if ( g_Globals.m_ConVars.m_ClBobamtLat->GetFloat() != 0.1f ) 
		{
			g_Globals.m_ConVars.m_ClBobamtLat->SetValue(0.1f);
			g_Globals.m_ConVars.m_ClBobamtVert->SetValue(0.1f);
			g_Globals.m_ConVars.m_ClBombLowerAmt->SetValue(5);
		}
	}
	else 
	{
		if ( g_Globals.m_ConVars.m_ClBobamtLat->GetFloat() != 0.4f )
		{
			g_Globals.m_ConVars.m_ClBobamtLat->SetValue(0.4f);
			g_Globals.m_ConVars.m_ClBobamtVert->SetValue(0.25f);
			g_Globals.m_ConVars.m_ClBombLowerAmt->SetValue(21);
		}
	}

	if ( g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_HAND_SHAKING ] )
		return g_Globals.m_ConVars.m_ClWpnSwayAmount->SetValue(0.0f);
		
	return g_Globals.m_ConVars.m_ClWpnSwayAmount->SetValue( 1.6f );
}

void C_World::RemoveSmokeAndPostProcess( )
{
	static std::vector< std::string > aMaterialList =
	{
		_S( "particle/vistasmokev1/vistasmokev1_emods" ),
		_S( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ),
		_S( "particle/vistasmokev1/vistasmokev1_fire" ),
		_S( "particle/vistasmokev1/vistasmokev1_smokegrenade" ),
	};

	for ( auto strSmokeMaterial : aMaterialList )
	{
		C_Material* pMaterial = g_Globals.m_Interfaces.m_MaterialSystem->FindMaterial( strSmokeMaterial.c_str( ), _S( TEXTURE_GROUP_OTHER ) );
		if ( !pMaterial || pMaterial->GetMaterialVarFlag( MATERIAL_VAR_NO_DRAW ) )
			continue;

		pMaterial->SetMaterialVarFlag( MATERIAL_VAR_NO_DRAW, g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SMOKE ] );
	}

	if ( *( int32_t* )( *reinterpret_cast < uint32_t** >( ( uint32_t )( g_Globals.m_AddressList.m_SmokeCount ) ) ) != 0 )
	{
		if ( g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SMOKE ] )
			*( int32_t* )( *reinterpret_cast < uint32_t** >( ( uint32_t )( g_Globals.m_AddressList.m_SmokeCount ) ) ) = 0;
	}

	**reinterpret_cast < bool** > ( reinterpret_cast < uint32_t > ( g_Globals.m_AddressList.m_PostProcess ) ) = g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_POSTPROCESS ];
}

void C_World::RemoveShadows( )
{
	g_Globals.m_ConVars.m_ClFootContactShadows->SetValue( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SHADOWS ] );
	g_Globals.m_ConVars.m_ClCsmStaticPropShadows->SetValue( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SHADOWS ] );
	g_Globals.m_ConVars.m_ClCsmWorldShadows->SetValue( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SHADOWS ] );
	g_Globals.m_ConVars.m_ClCsmShadows->SetValue( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SHADOWS ] );
	g_Globals.m_ConVars.m_ClCsmViewmodelShadows->SetValue( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SHADOWS ] );
	g_Globals.m_ConVars.m_ClCsmSpriteShadows->SetValue( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SHADOWS ] );
	g_Globals.m_ConVars.m_ClCsmRopeShadows->SetValue( !g_Settings->m_aWorldRemovals[ REMOVALS_VISUAL_SHADOWS ] );
}

void C_World::DrawBulletTracers( )
{
	
	/*
	"sprites/blueglow1",
	"sprites/glow01",
	"sprites/physbeam",
	"sprites/purpleglow1",
	"sprites/purplelaser1", - neverlose thin.
	"sprites/radio",
	"sprites/white",
	*/

	std::array< const char*, 7 >  aTracerType =
	{
		"sprites/blueglow1.vmt",
		"sprites/glow01.vmt",
		"sprites/physbeam.vmt",
		"sprites/purpleglow1.vmt",
		"sprites/purplelaser1.vmt",
		"sprites/radio.vmt",
		"sprites/white.vmt",
	};

	for ( int32_t iPosition = 0; iPosition < m_BulletTracers.size( ); iPosition++ )
	{
		auto Trace = &m_BulletTracers[ iPosition ];

		Color aBulletColor = Color( g_Settings->m_EnemyTracers );
		if ( Trace->m_bIsLocalTrace )
			aBulletColor = Color( g_Settings->m_LocalTracers );

		BeamInfo_t BeamInfo = BeamInfo_t( );

		BeamInfo.m_vecStart = Trace->m_vecStartPosition;
		if ( Trace->m_bIsLocalTrace )
			BeamInfo.m_vecStart = Vector( Trace->m_vecStartPosition.x, Trace->m_vecStartPosition.y, Trace->m_vecStartPosition.z - 2.0f );

		BeamInfo.m_vecEnd = Trace->m_vecEndPosition;
		BeamInfo.m_nModelIndex = -1;
		BeamInfo.m_flHaloScale = 0.0f;
		BeamInfo.m_flLife = 4.0f;
		BeamInfo.m_flFadeLength = 0.0f;
		BeamInfo.m_flWidth = g_Settings->m_TracerWidth;
		BeamInfo.m_flEndWidth = g_Settings->m_TracerWidth;
		BeamInfo.m_flAmplitude = 1.f;

		BeamInfo.m_nStartFrame = 0;
		BeamInfo.m_flRed = aBulletColor.r( );
		BeamInfo.m_flGreen = aBulletColor.g( );
		BeamInfo.m_flBlue = aBulletColor.b( );
		BeamInfo.m_flBrightness = aBulletColor.a( );

		BeamInfo.m_bRenderable = true;
		BeamInfo.m_nSegments = 2;
		BeamInfo.m_nFlags = 0;

		BeamInfo.m_flFrameRate = 0.0f;
		BeamInfo.m_pszModelName = aTracerType.at( g_Settings->m_TracerType );
		BeamInfo.m_nType = TE_BEAMPOINTS;

		Beam_t* Beam = g_Globals.m_Interfaces.m_ViewRenderBeams->CreateBeamPoints( BeamInfo );
		if ( Beam )
			g_Globals.m_Interfaces.m_ViewRenderBeams->DrawBeam( Beam );

		m_BulletTracers.erase( m_BulletTracers.begin( ) + iPosition );
	}
}

void C_World::DrawClientImpacts ( )
{
	if ( !g_Settings->m_bDrawClientImpacts )
		return;

	auto& aClientImpactList = *( CUtlVector< ClientImpact_t >*)( ( uintptr_t )( g_Globals.m_LocalPlayer ) + 0x11C50 );
	for ( auto Impact = aClientImpactList.Count( ); Impact > m_iLastProcessedImpact; --Impact )
		g_Globals.m_Interfaces.m_DebugOverlay->BoxOverlay( 
			aClientImpactList[ Impact - 1 ].m_vecPosition, 
			Vector( -2.0f, -2.0f, -2.0f ),
			Vector( 2.0f, 2.0f, 2.0f ), 
			QAngle( 0.0f, 0.0f, 0.0f ), 
			g_Settings->m_ClientImpacts->r( ), 
			g_Settings->m_ClientImpacts->g( ),
			g_Settings->m_ClientImpacts->b( ),
			g_Settings->m_ClientImpacts->a( ),
			4.0f );

	if ( aClientImpactList.Count( ) != m_iLastProcessedImpact )
		m_iLastProcessedImpact = aClientImpactList.Count( );
}

void C_World::SkyboxChanger( )
{
	std::string strSkyBox = g_Globals.m_Interfaces.m_CVar->FindVar( _S( "sv_skyname" ) )->GetString( );
	switch ( g_Settings->m_iSkybox )
	{
		case 1:
			strSkyBox = _S( "cs_tibet" );
			break;
		case 2:
			strSkyBox = _S( "cs_baggage_skybox_" );
			break;
		case 3:
			strSkyBox = _S( "italy" );
			break;
		case 4:
			strSkyBox = _S( "jungle" );
			break;
		case 5:
			strSkyBox = _S( "office" );
			break;
		case 6:
			strSkyBox = _S( "sky_cs15_daylight01_hdr" );
			break;
		case 7:
			strSkyBox = _S( "sky_cs15_daylight02_hdr" );
			break;
		case 8:
			strSkyBox = _S( "vertigoblue_hdr" );
			break;
		case 9:
			strSkyBox = _S( "vertigo" );
			break;
		case 10:
			strSkyBox = _S( "sky_day02_05_hdr" );
			break;
		case 11:
			strSkyBox = _S( "nukeblank" );
			break;
		case 12:
			strSkyBox = _S( "sky_venice" );
			break;
		case 13:
			strSkyBox = _S( "sky_cs15_daylight03_hdr" );
			break;
		case 14:
			strSkyBox = _S( "sky_cs15_daylight04_hdr" );
			break;
		case 15:
			strSkyBox = _S( "sky_csgo_cloudy01" );
			break;
		case 16:
			strSkyBox = _S( "sky_csgo_night02" );
			break;
		case 17:
			strSkyBox = _S( "sky_csgo_night02b" );
			break;
		case 18:
			strSkyBox = _S( "sky_csgo_night_flat" );
			break;
		case 19:
			strSkyBox = _S( "sky_dust" );
			break;
		case 20:
			strSkyBox = _S( "vietnam" );
			break;
		case 21:
			strSkyBox = g_Settings->m_szCustomSkybox;
			break;
	}

	g_Globals.m_ConVars.m_3DSky->SetValue( false );
	if ( g_Settings->m_iSkybox <= 0 )
		g_Globals.m_ConVars.m_3DSky->SetValue( true );

	return g_Tools->SetSkybox( strSkyBox.c_str( ) );
}

void filled_rect_world(Vector camera, const trace_t& tr, Color color, int size)
{
	Vector forward = Vector(size, size, size);
	Vector right = Math::CrossProduct(forward, tr.plane.normal);
	Vector up = Math::CrossProduct(tr.plane.normal, right);

	Vector a, b, c, d;
	if (Math::WorldToScreen(tr.endpos + up, a) && Math::WorldToScreen(tr.endpos - up, b) &&
		Math::WorldToScreen(tr.endpos + right, c) && Math::WorldToScreen(tr.endpos - right, d))
	{
		Vertex_t vertices[4];
		static int tex = g_Globals.m_Interfaces.m_Surface->CreateNewTextureID(true);

		g_Globals.m_Interfaces.m_Surface->DrawSetTexture(tex);
		g_Globals.m_Interfaces.m_Surface->DrawSetColor(color);

		vertices[0].Init(Vector2D(a.x, a.y));
		vertices[1].Init(Vector2D(c.x, c.y));
		vertices[2].Init(Vector2D(b.x, b.y));
		vertices[3].Init(Vector2D(d.x, d.y));

		//g_Globals.m_Interfaces.m_Surface->DrawTexturedPolygon(4, vertices, true);

		static int x[128];
		static int y[128];

		for (int i = 0; i < 4; i++)
		{
			x[i] = vertices[i].m_Position.x;
			y[i] = vertices[i].m_Position.y;
		}

		g_Globals.m_Interfaces.m_Surface->DrawSetColor(color.r(), color.g(), color.b(), 255);
		g_Globals.m_Interfaces.m_Surface->DrawPolyLine(x, y, 4);
	}
}

void C_World::PenetrationCrosshair( )
{
	if ( !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive( ) || !g_Settings->m_bPenetrationCrosshair )
		return;

	C_BaseCombatWeapon* pWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon( ).Get( );
	if ( !pWeapon || !pWeapon->IsGun( ) )
		return;

	C_CSWeaponData* pWeaponData = pWeapon->GetWeaponData( );
	if ( !pWeaponData )
		return;

	QAngle angLocalAngles;
	g_Globals.m_Interfaces.m_EngineClient->GetViewAngles( &angLocalAngles );

	Vector vecDirection;
	Math::AngleVectors( angLocalAngles, vecDirection );

	Color aColor = Color::Red;
	if ( g_AutoWall->IsPenetrablePoint( g_Globals.m_LocalData.m_vecShootPosition, g_Globals.m_LocalData.m_vecShootPosition + ( vecDirection * pWeaponData->m_flRange ) ) )
		aColor = Color::Green;

	CGameTrace enterTrace;
	Ray_t ray;

	auto maxrange = pWeaponData->m_flRange * 2;
	Vector end = g_Globals.m_LocalData.m_vecShootPosition + (vecDirection * maxrange);
	ray.Init(g_Globals.m_LocalData.m_vecShootPosition, end);

	uint32_t filter_[4] =
	{
		*(uint32_t*)(g_Globals.m_AddressList.m_TraceFilterSimple),
		(uint32_t)g_Globals.m_LocalPlayer,
		0,
		0
	};

	g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, (ITraceFilter*)&filter_, &enterTrace);

	filled_rect_world(vecDirection, enterTrace, aColor, 5);
}

struct MotionBlurHistory
{
	MotionBlurHistory() noexcept
	{
		lastTimeUpdate = 0.0f;
		previousPitch = 0.0f;
		previousYaw = 0.0f;
		previousPositon = Vector{ 0.0f, 0.0f, 0.0f };
		noRotationalMotionBlurUntil = 0.0f;
	}

	float lastTimeUpdate;
	float previousPitch;
	float previousYaw;
	Vector previousPositon;
	float noRotationalMotionBlurUntil;
};

static auto fromAngleAll(const QAngle& angle, Vector* forward, Vector* right, Vector* up) noexcept
{
	float sr = std::sin(DEG2RAD(angle.roll))
		, sp = std::sin(DEG2RAD(angle.pitch))
		, sy = std::sin(DEG2RAD(angle.yaw))
		, cr = std::cos(DEG2RAD(angle.roll))
		, cp = std::cos(DEG2RAD(angle.pitch))
		, cy = std::cos(DEG2RAD(angle.yaw));

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}

#include <Psapi.h>

static std::uintptr_t findPattern(const char* module, const char* pattern, size_t offset = 0) noexcept
{
	static auto id = 0;
	++id;

	if (MODULEINFO moduleInfo; GetModuleInformation(GetCurrentProcess(), GetModuleHandle(module), &moduleInfo, sizeof(moduleInfo))) {
		auto start = static_cast<const char*>(moduleInfo.lpBaseOfDll);
		const auto end = start + moduleInfo.SizeOfImage;

		auto first = start;
		auto second = pattern;

		while (first < end && *second) {
			if (*first == *second || *second == '?') {
				++first;
				++second;
			}
			else {
				first = ++start;
				second = pattern;
			}
		}

		if (!*second)
			return reinterpret_cast<std::uintptr_t>(const_cast<char*>(start) + offset);
	}

	return 0;
}

template <typename T>
static constexpr auto relativeToAbsolute(uintptr_t address) noexcept
{
	return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

void C_World::MotionBlur(C_ViewSetup* pSetup)
{
	if (!g_Globals.m_LocalPlayer || !g_Settings->m_bMotionBlur)
		return;
	
	if ( !g_Globals.m_LocalPlayer->IsAlive() )
		return;

	static MotionBlurHistory history;
	static float motionBlurValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (pSetup)
	{
		const float timeElapsed = m_Globals()->m_flRealTime - history.lastTimeUpdate;

		const auto viewangles = pSetup->angView;

		const float currentPitch = Math::AngleNormalize(viewangles.pitch);
		const float currentYaw = Math::AngleNormalize(viewangles.yaw);

		Vector currentSideVector;
		Vector currentForwardVector;
		Vector currentUpVector;
		fromAngleAll(pSetup->angView, &currentForwardVector, &currentSideVector, &currentUpVector);

		Vector currentPosition = pSetup->vecOrigin;
		Vector positionChange = history.previousPositon - currentPosition;

		if ((positionChange.Length() > 30.0f) && (timeElapsed >= 0.5f))
		{
			motionBlurValues[0] = 0.0f;
			motionBlurValues[1] = 0.0f;
			motionBlurValues[2] = 0.0f;
			motionBlurValues[3] = 0.0f;
		}
		else if (timeElapsed > (1.0f / 15.0f))
		{
			motionBlurValues[0] = 0.0f;
			motionBlurValues[1] = 0.0f;
			motionBlurValues[2] = 0.0f;
			motionBlurValues[3] = 0.0f;
		}
		else if (positionChange.Length() > 50.0f)
		{
			history.noRotationalMotionBlurUntil = m_Globals()->m_flRealTime + 1.0f;
		}
		else
		{
			const float horizontalFov = pSetup->flFOV;
			const float verticalFov = (pSetup->flAspectRatio <= 0.0f) ? (pSetup->flFOV) : (pSetup->flFOV / pSetup->flAspectRatio);
			const float viewdotMotion = currentForwardVector.Dot(positionChange);

			if (g_Settings->m_bForwardBlur)
				motionBlurValues[2] = viewdotMotion;

			const float sidedotMotion = currentSideVector.Dot(positionChange);
			float yawdiffOriginal = history.previousYaw - currentYaw;
			if (((history.previousYaw - currentYaw > 180.0f) || (history.previousYaw - currentYaw < -180.0f)) &&
				((history.previousYaw + currentYaw > -180.0f) && (history.previousYaw + currentYaw < 180.0f)))
				yawdiffOriginal = history.previousYaw + currentYaw;

			float yawdiffAdjusted = yawdiffOriginal + (sidedotMotion / 3.0f);

			if (yawdiffOriginal < 0.0f)
				yawdiffAdjusted = std::clamp(yawdiffAdjusted, yawdiffOriginal, 0.0f);
			else
				yawdiffAdjusted = std::clamp(yawdiffAdjusted, 0.0f, yawdiffOriginal);

			const float undampenedYaw = yawdiffAdjusted / horizontalFov;
			motionBlurValues[0] = undampenedYaw * (1.0f - (fabsf(currentPitch) / 90.0f));

			const float pitchCompensateMask = 1.0f - ((1.0f - fabsf(currentForwardVector[2])) * (1.0f - fabsf(currentForwardVector[2])));
			const float pitchdiffOriginal = history.previousPitch - currentPitch;
			float pitchdiffAdjusted = pitchdiffOriginal;

			if (currentPitch > 0.0f)
				pitchdiffAdjusted = pitchdiffOriginal - ((viewdotMotion / 2.0f) * pitchCompensateMask);
			else
				pitchdiffAdjusted = pitchdiffOriginal + ((viewdotMotion / 2.0f) * pitchCompensateMask);


			if (pitchdiffOriginal < 0.0f)
				pitchdiffAdjusted = std::clamp(pitchdiffAdjusted, pitchdiffOriginal, 0.0f);
			else
				pitchdiffAdjusted = std::clamp(pitchdiffAdjusted, 0.0f, pitchdiffOriginal);

			motionBlurValues[1] = pitchdiffAdjusted / verticalFov;
			motionBlurValues[3] = undampenedYaw;
			motionBlurValues[3] *= (fabs(currentPitch) / 90.0f) * (fabs(currentPitch) / 90.0f) * (fabs(currentPitch) / 90.0f);

			if (timeElapsed > 0.0f)
				motionBlurValues[2] /= timeElapsed * 30.0f;
			else
				motionBlurValues[2] = 0.0f;

			motionBlurValues[2] = std::clamp((fabsf(motionBlurValues[2]) - g_Settings->m_flFallingMin) / (g_Settings->m_flFallingMax.Get() - g_Settings->m_flFallingMin.Get()), 0.0f, 1.0f) * (motionBlurValues[2] >= 0.0f ? 1.0f : -1.0f);
			motionBlurValues[2] /= 30.0f;
			motionBlurValues[0] *= g_Settings->m_flRotationIntensitiy * .15f * g_Settings->m_flBlurStrength;
			motionBlurValues[1] *= g_Settings->m_flRotationIntensitiy * .15f * g_Settings->m_flBlurStrength.Get();
			motionBlurValues[2] *= g_Settings->m_flRotationIntensitiy * .15f * g_Settings->m_flBlurStrength;
			motionBlurValues[3] *= g_Settings->m_flFallingIntensitiy * .15f * g_Settings->m_flBlurStrength;

		}

		if (m_Globals()->m_flRealTime < history.noRotationalMotionBlurUntil)
		{
			motionBlurValues[0] = 0.0f;
			motionBlurValues[1] = 0.0f;
			motionBlurValues[3] = 0.0f;
		}
		else
		{
			history.noRotationalMotionBlurUntil = 0.0f;
		}
		history.previousPositon = currentPosition;

		history.previousPitch = currentPitch;
		history.previousYaw = currentYaw;
		history.lastTimeUpdate = m_Globals()->m_flRealTime;
		return;
	}

	const auto material = g_Globals.m_Interfaces.m_MaterialSystem->FindMaterial("dev/motion_blur", "RenderTargets", false);
	if (!material)
		return;

	const auto MotionBlurInternal = material->FindVar("$MotionBlurInternal", nullptr, false);

	MotionBlurInternal->setVecComponentValue(motionBlurValues[0], 0);
	MotionBlurInternal->setVecComponentValue(motionBlurValues[1], 1);
	MotionBlurInternal->setVecComponentValue(motionBlurValues[2], 2);
	MotionBlurInternal->setVecComponentValue(motionBlurValues[3], 3);

	const auto MotionBlurViewPortInternal = material->FindVar("$MotionBlurViewportInternal", nullptr, false);

	MotionBlurViewPortInternal->setVecComponentValue(0.0f, 0);
	MotionBlurViewPortInternal->setVecComponentValue(0.0f, 1);
	MotionBlurViewPortInternal->setVecComponentValue(1.0f, 2);
	MotionBlurViewPortInternal->setVecComponentValue(1.0f, 3);

	static auto memToDrawFunction = relativeToAbsolute<uintptr_t>(findPattern(_S("client.dll"), _S("\xE8????\x83\xC4\x0C\x8D\x4D\xF8")) + 1);
	const auto m_drawFunction = memToDrawFunction;
	
	int w, h; 
	g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(w, h); 
	__asm {
		__asm push h
		__asm push w
		__asm push 0
		__asm xor edx, edx
		__asm mov ecx, material
		__asm call m_drawFunction
		__asm add esp, 12
	}
}