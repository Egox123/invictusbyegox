#pragma once
class C_NetChannel
{
public:
	char pad_0x0000[0x18]; //0x0000
	int m_iOutSequenceNr;    //0x0018 last send outgoing sequence number
	int m_iInSequenceNr;     //0x001C last received incoming sequnec number
	int m_iOutSequenceNrAck; //0x0020 last received acknowledge outgoing sequnce number
	int m_iOutReliableState; //0x0024 state of outgoing reliable data (0/1) flip flop used for loss detection
	int m_iInReliableState;  //0x0028 state of incoming reliable data
	int m_iChokedCommands;   //0x002C number of choked packets
	char pad_0030[1044];         //0x0030

	//PushVirtual(SendDatagram(LPVOID Data), 46, int(__thiscall*)(LPVOID, LPVOID), Data);
}; //Size: 0x0444 0x0444