#pragma once
#include <deque>
#include "../../SDK/Includes.hpp"
#include "../Settings.hpp"

#include "../Animations/LagCompensation.hpp"

enum RAGE_HITBOX_ID
{
	POINT_HEAD,
	POINT_U_CHEST,
	POINT_CHEST,
	POINT_L_CHEST,
	POINT_PELVIS,
	POINT_STOMACH,
	POINT_ARMS,
	POINT_LEGS
};

class C_RageBot
{
public:
	virtual void Instance( );
	virtual void SetupPacket( );
	virtual void AutoRevolver( );
	virtual int32_t GetMinDamage( C_BasePlayer* pPlayer );
	virtual int32_t GetAutoDamage( bool bHasArmour );
	virtual void FakeDuck( );
	virtual bool AutoStop( );
	virtual void AutoScope( );
	virtual void ScanPlayers( );
	virtual void TaserBot( );

	virtual void BackupPlayers( );
	virtual void RestorePlayers( );

	virtual void ResetData( );

	virtual void OnWeaponFire( C_GameEvent* pEvent );
	virtual void OnBulletImpact( C_GameEvent* pEvent );
	virtual void OnPlayerHurt( C_GameEvent* pEvent );
	virtual void OnNetworkUpdate( ClientFrameStage_t Stage );
	
	virtual C_RageSettings GetRageSettings( ) { return this->m_RageSettings; };
	static int GetHitgroupFromHitbox( int32_t iHitbox );

	virtual bool CanAutoStop( );
	virtual bool HasMaximumAccuracy( );
	virtual void UpdatePeekState( );

	virtual void SaveMovementData();

	virtual void ForceMovementData();

	virtual C_HitboxData ScanPlayerRecord( C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition );
	virtual C_HitboxData VisibleOnlyScan(C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition);

	virtual C_LagRecord GetFirstAvailableRecord( C_BasePlayer* pPlayer );
	virtual bool FindPlayerRecord( C_BasePlayer* pPlayer, C_LagRecord* OutRecord, C_HitboxData* OutHitbox );
	virtual std::vector < Vector > GetHitboxPoints( C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition, int32_t iHitbox );
	virtual bool DoesIntersectHitbox( C_BasePlayer* pPlayer, int32_t iHitbox, Vector vecStartPosition, Vector vecEndPosition );
	virtual bool IsSafePoint( C_BasePlayer* pPlayer, C_LagRecord LagRecord, Vector vecStartPosition, Vector vecEndPosition, int32_t iHitbox );
	virtual void AdjustPlayerBones( C_BasePlayer* pPlayer, std::array < matrix3x4_t, MAXSTUDIOBONES > aMatrix );
	virtual void AdjustPlayerRecord( C_BasePlayer* pPlayer, C_LagRecord LagRecord );
	virtual float_t GetHitChance( C_TargetData Target );
private:
	C_RageSettings m_RageSettings = C_RageSettings( );
	C_MoveData m_MoveData = C_MoveData( );

	int m_nLastRageID = 1;
	int m_nLastPeekID = 1;

	float m_flForwardMove = 0.0f;
	float m_flSideMove = 0.0f;

	std::array < C_LagRecord, 65 > m_BackupData;
};

inline C_RageBot* g_RageBot = new C_RageBot( );