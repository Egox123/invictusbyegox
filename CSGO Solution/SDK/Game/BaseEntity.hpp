#pragma once

class C_BaseEntity : public C_ClientEntity
{
public:
	PushVirtual( GetPredDescMap( ), 17, C_DataMap*( __thiscall* )( void* ) );
	PushVirtual( IsPlayer( ), 158, bool( __thiscall* )( void* ) );
	PushVirtual( SetModelIndex( int i ), 75, void( __thiscall* )( void*, int ), i );

	NETVAR( m_nEntityModelIndex		,	int32_t	, FNV32( "DT_BaseEntity" ), FNV32( "m_nModelIndex" ) );
	NETVAR( m_flSimulationTime		,	float_t	, FNV32( "DT_BaseEntity" ), FNV32( "m_flSimulationTime" ) );
	NETVAR( m_iTeamNum				,	int32_t	, FNV32( "DT_BaseEntity" ), FNV32( "m_iTeamNum" ) );
	NETVAR( m_vecOrigin				,	Vector	, FNV32( "DT_BaseEntity" ), FNV32( "m_vecOrigin" ) );
	NETVAR( m_nOwnerEntity			,   CBaseHandle, FNV32("DT_BaseEntity"), FNV32("m_hOwnerEntity"));

	void SetAbsoluteAngles( QAngle angViewAngle );
	void SetAbsoluteOrigin( Vector vecAbsOrigin );
	void SetWorldOrigin( Vector vecWorldOrigin );

	DATAMAP( int32_t	,	m_iEFlags			);
	DATAMAP( int32_t	,	m_nSequence			);
	DATAMAP( float_t	,	m_flStamina			);
	DATAMAP( float_t	,	m_flCycle			);
	DATAMAP( int32_t	,	m_fEffects			);
	DATAMAP( Vector		,	m_vecAbsVelocity	);
	DATAMAP( Vector		,	m_vecBaseVelocity	);

	bool IsBreakableEntity( )
	{
		const auto szObjectName = this->GetClientClass( )->m_strNetworkName;
		if ( szObjectName[ 0 ] == 'C' )
		{
			if ( szObjectName[ 7 ] == 't' || szObjectName[ 7 ] == 'b' )
				return true;
		}

		return ( ( bool( __thiscall* )( C_BaseEntity* ) )( g_Globals.m_AddressList.m_IsBreakableEntity ) )( this );
	}

	CUSTOM_OFFSET( m_rgflCoordinateFrame, matrix3x4_t, FNV32( "CoordinateFrame" ), 1092 );
	CUSTOM_OFFSET( m_flOldSimulationTime, float_t, FNV32( "OldSimulationTime" ), 0x26C );
	CUSTOM_OFFSET( m_flFlashTime, float_t, FNV32( "FlashTime" ), 0x10480 );
	CUSTOM_OFFSET( m_nExplodeEffectTickBegin, int32_t, FNV32( "m_nExplodeEffectTickBegin" ), 0x29F4 );
	CUSTOM_OFFSET( m_flCreationTime, float_t, FNV32( "CreationTime" ), 0x02D8 );
	CUSTOM_OFFSET( m_vecMins, Vector, FNV32( "Mins" ), 0x328 );
	CUSTOM_OFFSET( m_vecMaxs, Vector, FNV32( "Maxs" ), 0x334 );
	CUSTOM_OFFSET( m_nRenderMode, int32_t, FNV32( "RenderMode" ), 0x25B );

	void SetbUseCustomBloomScale(byte value)
	{
		*reinterpret_cast<byte*>(uintptr_t(this) + g_NetvarManager->GetOffsetByString(_S("CEnvTonemapController"), _S("m_bUseCustomBloomScale"))) = value;
	}

	void SetflCustomBloomScale(float value)
	{
		*reinterpret_cast<float*>(uintptr_t(this) + g_NetvarManager->GetOffsetByString(_S("CEnvTonemapController"), _S("m_flCustomBloomScale"))) = value;
	}

	void SetbUseCustomAutoExposureMin(byte value)
	{
		*reinterpret_cast<byte*>(uintptr_t(this) + g_NetvarManager->GetOffsetByString(_S("CEnvTonemapController"), _S("m_bUseCustomAutoExposureMin"))) = value;
	}

	void SetflCustomAutoExposureMin(float value)
	{
		*reinterpret_cast<float*>(uintptr_t(this) + g_NetvarManager->GetOffsetByString(_S("CEnvTonemapController"), _S("m_flCustomAutoExposureMin"))) = value;
	}

	void SetbUseCustomAutoExposureMax(byte value)
	{
		*reinterpret_cast<byte*>(uintptr_t(this) + g_NetvarManager->GetOffsetByString(_S("CEnvTonemapController"), _S("m_bUseCustomAutoExposureMax"))) = value;
	}

	void SetflCustomAutoExposureMax(float value)
	{
		*reinterpret_cast<float*>(uintptr_t(this) + g_NetvarManager->GetOffsetByString(_S("CEnvTonemapController"), _S("m_flCustomAutoExposureMax"))) = value;
	}
};