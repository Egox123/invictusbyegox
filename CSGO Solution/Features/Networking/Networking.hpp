#pragma once
#include "../SDK/Includes.hpp"
#include "../Prediction/EnginePrediction.hpp"

class C_Networking
{
public:
	virtual void OnPacketEnd( C_ClientState* pClientState );
	virtual void SaveNetvarData( int nCommand );
	virtual void RestoreNetvarData( int nCommand );
	virtual void UpdateLatency( );
	virtual void ProcessInterpolation( ClientFrameStage_t Stage, bool bPostFrame );
	virtual void ResetData( );

	virtual void StartNetwork( );
	virtual void FinishNetwork( );

	virtual float_t GetLatency( );
	virtual int32_t GetServerTick( );
	virtual int32_t GetTickRate( );
private:

	float m_flInterp = 0.0f;
	int m_Sequence = 0;
	float_t m_Latency = 0.f;

	int32_t m_TickRate = 0;
	int32_t m_FinalPredictedTick = 0;
};

inline C_Networking* g_Networking = new C_Networking( );