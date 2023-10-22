#pragma once
#include "../SDK/Includes.hpp"

class C_AntiAim
{
public:
	virtual void Instance( );
	virtual float VelocityDeltaFormula();
	virtual bool FreestandAngle(float& Angle, C_BasePlayer* pFinalPlayer);
	virtual void Micromovement( );
	virtual void SlowWalk( bool bForcedWalk = false );
	virtual void JitterMove( );
	virtual void LegMovement( );
	virtual void FindTarget();
	virtual int GetManualSide( ) { return m_iManualSide; };
	virtual int32_t GetAntiAimConditions( );
	virtual float GetTargetYaw(C_BasePlayer* pFinalPlayer);
private:
	int32_t m_iDesyncSide = -1;
	int32_t m_iManualSide = 0;
	int32_t m_iFinalSide = -1;
	int32_t m_nFakePickTick = 0;
	int32_t m_nServerTick = 0;

	std::array < int32_t, MULTIPLAYER_BACKUP > m_AntiAimConditions;

	float_t m_flAccelerationSpeed = 95.0f;
	float_t m_flOptimizedYawAngle = 0.0f;

	bool m_bSwitch = false;
	bool m_bMoveSwitch = false;
	bool m_bWalkSwitch = false;
	bool m_bSlideSwitch = false;
	bool m_bShouldUseAlternativeSide = false;

	C_BasePlayer* m_aTarget = nullptr;
};

inline C_AntiAim* g_AntiAim = new C_AntiAim( );