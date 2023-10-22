#include "../SDK/Game/NetChannel.hpp"

struct IncomingSequence
{
	int nInReliableState;
	int nSequenceNr;
	int nOutReliableState;
	float flRealTime;
};

class C_ExtendedBacktrack
{
public:
	virtual void UpdateSequences( );
	virtual void AddBacktrackingWindow( C_NetChannel* pChannel, float flDelta );
};

inline C_ExtendedBacktrack* g_Backtrack = new C_ExtendedBacktrack( );