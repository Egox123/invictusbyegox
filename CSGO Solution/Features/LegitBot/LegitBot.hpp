#pragma once

#include "../../SDK/Includes.hpp"
#include "../Settings.hpp"

struct target_info
{
	float fov;
	int hitbox;
};

class C_LegitBot {
public:
	void OnMove(C_UserCmd* pCmd);
	bool IsEnabled(C_UserCmd* pCmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);
	
	C_LegitSettings m_Settings;

	bool IsRcs();
	float GetSmooth();
	float GetFov();

private:
	void RCS(QAngle& angle, C_BasePlayer* target, bool should_run);
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);

	bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
	bool IsNotSilent(float fov);

	C_BasePlayer* GetClosestPlayer(C_UserCmd* cmd, int& bestBone);
	
	float_t shot_delay_time;

	int32_t kill_delay_time;
	
	bool shot_delay = false;
	bool silent_enabled = false;
	bool is_delayed = false;
	bool kill_delay = false;
	
	QAngle CurrentPunch = { 0,0,0 };
	QAngle RCSLastPunch = { 0,0,0 };
	
	C_BasePlayer* m_Target = NULL;
};

inline C_LegitBot* g_LegitBot = new C_LegitBot();