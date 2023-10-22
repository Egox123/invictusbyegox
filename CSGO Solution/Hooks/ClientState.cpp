#include "../Hooks.hpp"
#include "../Features/Packet/PacketManager.hpp"
#include "../Features/Networking/Networking.hpp"

void __fastcall C_Hooks::hkPacketStart( LPVOID pEcx, uint32_t, int32_t iSequence, int32_t iCommand )
{
	if ( g_PacketManager->ShouldProcessPacketStart( iCommand ) )
		return g_Globals.m_Hooks.m_Originals.m_PacketStart( pEcx, iSequence, iCommand );
}

void __fastcall C_Hooks::hkPacketEnd( LPVOID pEcx, uint32_t )
{
	if (!g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive())
	{
		g_Globals.m_CorrectData.clear();
		return g_Globals.m_Hooks.m_Originals.m_PacketEnd(pEcx);
	}

	if (*(int32_t*)((DWORD)((C_ClientState*)(pEcx))+0x16C) != *(int32_t*)((DWORD)((C_ClientState*)(pEcx))+0x164))
		return g_Globals.m_Hooks.m_Originals.m_PacketEnd(pEcx);;
	
	auto ACK_CMD = *(int*)((uintptr_t)pEcx + 0x4D2C);
	auto CorrectData = std::find_if(g_Globals.m_CorrectData.begin(), g_Globals.m_CorrectData.end(),
		[&ACK_CMD](const C_Globals::CorrectionData& oData)
		{
			return oData.m_commandNumber == ACK_CMD;
		}
	);

	auto NetChannel = g_Globals.m_Interfaces.m_EngineClient->GetNetChannelInfo();

	if (NetChannel && CorrectData != g_Globals.m_CorrectData.end())
	{
		if (g_Globals.m_LocalData.m_flLastVelModifier > g_Globals.m_LocalPlayer->m_flVelocityModifier() + 0.1f)
		{
			auto CurrentWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();

			if (!CurrentWeapon || CurrentWeapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !CurrentWeapon->IsGrenade()) //-V648
			{
				for (auto& Number : g_PacketManager->PushChokedCommand())
				{
					auto NewCommand = &g_Globals.m_Interfaces.m_Input->m_commands[Number % MULTIPLAYER_BACKUP];
					auto NewVerified = &g_Globals.m_Interfaces.m_Input->m_verified_commands[Number % MULTIPLAYER_BACKUP];

					if (NewCommand->m_nButtons & (IN_ATTACK | IN_ATTACK2))
					{
						NewCommand->m_nButtons &= ~IN_ATTACK;

						NewVerified->m_Cmd = *NewCommand;
						NewVerified->m_CRC = NewCommand->GetChecksum();
					}
				}
			}
		}

		g_Globals.m_LocalData.m_flLastVelModifier = g_Globals.m_LocalPlayer->m_flVelocityModifier();
	}
	//g_Networking->OnPacketEnd( ( C_ClientState* )( pEcx ) );

	return g_Globals.m_Hooks.m_Originals.m_PacketEnd( pEcx );
}