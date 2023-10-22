#pragma once
#include "../../SDK/Includes.hpp"
#include "LagCompensation.hpp"

// velocity full fix.
//#define PLAYER_VELOCITY_FULLFIX

// enable player extrapolation.
//#define EXTRAPOLATE_PLAYERS

enum ADVANCED_ACTIVITY : int
{
	ACTIVITY_NONE = 0,
	ACTIVITY_JUMP,
	ACTIVITY_LAND
};

template < class T >
__forceinline T Interpolate( const T& flCurrent, const T& flTarget, const int iProgress, const int iMaximum )
{
	return flCurrent + ( ( flTarget - flCurrent ) / iMaximum ) * iProgress;
}

struct ResolverHistory_Data
{
	bool m_bFreestanding;
	bool m_iSide;

	// You can add whatever you want in here, those are just examples. 
	// I removed the good stuff since I don't want it around anymore.
};

typedef ResolverHistory_Data ResolverHistory_Type;

class C_DeSyncResolver {

public:
	virtual void Instance();
	virtual void ResolveAir(C_CSGOPlayerAnimationState* pState, int32_t iIndex);
	virtual void ResolveOnShot(int32_t iIndex);
	virtual void BruteForce(int32_t iIndex);
	virtual void HardReset();
	virtual void SetPlaybackRate( float val, int id ) { m_flPlaybackRates.at(m_Ent->EntIndex()).at(id) = val; };
	virtual void ResolveStandingEntity( );
	virtual void ResolveMovingEntity( );
	virtual void Reset( );
	virtual void ResolveFreestand( );
	virtual void DormancyReset( int32_t pIndex );
	virtual void SortData( int32_t pIndex, bool pInvalid, bool p_bWipe, int32_t pSide = ROTATE_MODE::ROTATE_SERVER );
	virtual void Initialize( C_BasePlayer* pEnt, C_LagRecord* pRecord, C_LagRecord* pPreviousRecord );
	virtual float VelocityDeltaFormula( C_BasePlayer* pPlayer );
	virtual float ResolveDelta( );
	virtual char EntityMoving( int pIndex ) { return m_bMovement[ pIndex ]; };
	virtual void RebuildMovement();
	virtual bool TraceVisible( const Vector& start, const Vector& end );
	std::string PrintAnimationData( int32_t ent_index )
	{
		std::string aOutput = "resolver info: ";

		aOutput += std::to_string(m_flAnimationHistory.at(ent_index).at(0));
		aOutput += " | ";
		aOutput += std::to_string(m_flAnimationHistory.at(ent_index).at(1));
		aOutput += " | ";
		aOutput += std::to_string(m_flAnimationHistory.at(ent_index).at(2));
		aOutput += " | ";
		aOutput += std::to_string(m_flDelta.at(ent_index));

		return aOutput;
	}

	short m_iResolverType[65];
	bool m_bFreestanding[65];
	bool m_bFinishedResolving = false;

private:
	enum HISTORY_MODE{
		MOVEMENT,
		STANDING,
		AIR
	};

	C_BasePlayer* m_Ent				= nullptr; 
	C_LagRecord* m_Record			= nullptr;
	C_LagRecord* m_PreviousRecord	= nullptr;
	
	char m_bMovement[65];
	bool m_bDesyncJitter[65];

	std::array< float_t, 65 > m_flOptimizationTimer;
	std::array< int32_t, 65 > m_iLastSide;
	std::array< int32_t, 65 > m_flDelta;
	std::array< std::array< std::vector< ResolverHistory_Type >, 2 > , 65 > m_flResolverHistory;
	std::array< std::array< std::vector< ResolverHistory_Type >, 2 > , 65 > m_flMissHistory;
	std::array< std::array< float_t, 3 >, 65 > m_flPlaybackRates = { 0,0,0 };
	std::array < float_t, 65 > m_flInvertTime = { };
	std::array< std::array< float_t, 3 >, 65 > m_flAnimationHistory = { };
};

inline C_DeSyncResolver* g_Resolver = new C_DeSyncResolver( );

class C_AnimationSync
{
public:
	virtual void Instance( ClientFrameStage_t Stage );
	virtual void UpdatePlayerAnimations( C_BasePlayer* pPlayer, C_LagRecord& LagRecord, C_LagRecord PreviousRecord, bool bHasPreviousRecord, int32_t iRotationMode );
	virtual void MarkAsDormant( int32_t iPlayerID ) { m_LeftDormancy[ iPlayerID ] = true; };
	virtual void UnmarkAsDormant( int32_t iPlayerID ) { m_LeftDormancy[ iPlayerID ] = false; };
	virtual bool HasLeftOutOfDormancy( int32_t iPlayerID ) { return m_LeftDormancy[ iPlayerID ]; };
	virtual void SetPreviousRecord( int32_t iPlayerID, C_LagRecord LagRecord ) { m_PreviousRecord[ iPlayerID ] = LagRecord; };
	virtual C_LagRecord GetPreviousRecord( int32_t iPlayerID ) { return m_PreviousRecord[ iPlayerID ]; };
	virtual bool GetCachedMatrix( C_BasePlayer* pPlayer, matrix3x4_t* aMatrix );
	virtual void OnUpdateClientSideAnimation( C_BasePlayer* pPlayer );
private:
	std::array < std::array < Vector, MAXSTUDIOBONES >, 65 > m_BoneOrigins;
	std::array < std::array < matrix3x4_t, MAXSTUDIOBONES >, 65 > m_CachedMatrix = { };
	std::array < C_LagRecord, 65 > m_PreviousRecord = { };
	std::array < bool, 65 > m_LeftDormancy = { };
};

inline C_AnimationSync* g_AnimationSync = new C_AnimationSync( );