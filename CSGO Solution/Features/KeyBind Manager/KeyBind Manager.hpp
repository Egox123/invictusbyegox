#pragma once

#include "Settings.hpp"

//////////////////////////////////////////////////////
//
// Keybind override value.
// Purpose: override default values of any kind.
// It's applicable in all sorts of programmes - even
// if you build a rocket and decide to apply keybind
// to a button :).
// 
// - egox.
//////////////////////////////////////////////////////

struct C_Binding
{
	C_KeyBind m_aBind;

	std::string m_sVarName = "I get 0 bitches level of code.";

	LPVOID m_pOrigin = nullptr;

	int32_t m_aBackup;
	int32_t m_aNewValue;

	int8_t m_i8DataConstant;

	bool m_bActive = false;
};

constexpr int8_t m_i8InvertMemoryConstant = 33;

enum KEYBIND_TYPE : int8_t 
{
	KEYBIND_BOOL,
	KEYBIND_INT,
	KEYBIND_FLOAT,
	KEYBIND_ARRAY = 5
};

class C_MemoryManager
{
protected:
	void MemCopy(LPVOID pDest0, LPVOID pSrc0, int8_t pDataConstant);
	void MemSet(LPVOID pDest0, LPVOID pSrc0, int8_t pDataConstant);
};

class C_KeyBindManager : C_MemoryManager
{
public:
	void Instance();
	
	// menu interactions.
	void AddBind(LPVOID pParam0, LPVOID pParam1, LPVOID pParam2, int8_t pDataConstant, std::string pStr0); // interaction with menu
	void RemoveBind(int8_t pVal0); // interaction with menu
	void ResetValues(); // on config action in order to not mess with config values.
private:
	std::vector< C_Binding > m_vBindList{};

	bool m_bReloadBindValues = false;
	//bool m_bFirstInstanceLaunch = true;
};

inline C_KeyBindManager* g_KeyBindManager = new C_KeyBindManager();