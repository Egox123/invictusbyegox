#include "KeyBind Manager.hpp"

#include "Tools/Tools.hpp"
#include "Settings.hpp"

// Maybe in some future releases, if I feel like giving it away.

// the template one since it's more unstable and hell to optimize will be released in that case :)))

// with love, Egox.

void C_KeyBindManager::Instance()
{

}

void C_KeyBindManager::AddBind(LPVOID pParam0, LPVOID pParam1, LPVOID pParam2, int8_t pDataConstant, std::string pStr0)
{
	// pParam0 -> bind.
	// pParam1 -> where we write new value to and where we get our backup from.
	// pParam2 -> new value.

	// declare new bind.
	C_Binding aBind;
	
	aBind.m_aBind = *(C_KeyBind*)pParam0;
	aBind.m_pOrigin = pParam1;
	aBind.m_sVarName = pStr0;

	C_KeyBindManager::C_MemoryManager::MemCopy(&aBind.m_aBackup, pParam1, aBind.m_i8DataConstant);
	C_KeyBindManager::C_MemoryManager::MemCopy(&aBind.m_aNewValue, pParam2, aBind.m_i8DataConstant);
}

void C_KeyBindManager::RemoveBind(int8_t pVal0)
{
	// delete at index.
	m_vBindList.erase(m_vBindList.begin() + pVal0);
}

void C_KeyBindManager::ResetValues()
{
	// run on save config. -> stop keybind interference with default config values.
	for (auto& aBind : m_vBindList)
		C_KeyBindManager::C_MemoryManager::MemSet(aBind.m_pOrigin, &aBind.m_aNewValue, aBind.m_i8DataConstant);

	m_bReloadBindValues = true;
}

void C_MemoryManager::C_MemoryManager::MemCopy(LPVOID pDest0, LPVOID pSrc0, int8_t pDataConstant)
{
	
}

void C_MemoryManager::C_MemoryManager::MemSet(LPVOID pDest0, LPVOID pSrc0, int8_t pDataConstant)
{
	
}
