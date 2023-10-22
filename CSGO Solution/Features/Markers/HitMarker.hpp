#pragma once
#include <vector>
#include "../SDK/Includes.hpp"

struct HitMarker_t
{
	float_t m_flTime = 0.0f;
};

class C_HitMarker
{
public:
	virtual void OnRageBotFire( Vector vecOrigin );
	virtual void Instance( );
private:
	std::vector < float_t > m_aHitMarkers = { };
};

inline C_HitMarker* g_HitMarkers = new C_HitMarker( );