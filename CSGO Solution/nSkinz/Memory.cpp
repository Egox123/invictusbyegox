// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "../Tools/Obfuscation/XorStr.hpp"

template <typename T>
static constexpr auto relativeToAbsolute(int* address) noexcept
{
    return reinterpret_cast<T>(reinterpret_cast<char*>(address + 1) + *address);
}

#define FIND_PATTERN(type, ...) \
reinterpret_cast<type>(findPattern(__VA_ARGS__))

template <typename T>
static constexpr auto MrelativeToAbsolute(uintptr_t address) noexcept
{
	return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

void Memory::initialize() noexcept
{
	auto temp = FIND_PATTERN(std::uintptr_t*, _S("client"), _S("\xB9????\xE8????\x8B\x5D\x08"), 1);
	hud = *temp;
	findHudElement = relativeToAbsolute<decltype(findHudElement)>(reinterpret_cast<int*>(reinterpret_cast<char*>(temp) + 5));
	clearHudWeapon = MrelativeToAbsolute<decltype(clearHudWeapon)>(findPattern(_S("client.dll"), "\xE8????\x8B\xF0\xC6\x44\x24??\xC6\x44\x24") + 1);
	itemSchema = relativeToAbsolute<decltype(itemSchema)>(FIND_PATTERN(int*, _S("client"), _S("\xE8????\x0F\xB7\x0F"), 1));
	equipWearable = FIND_PATTERN(decltype(equipWearable), _S("client"), _S("\x55\x8B\xEC\x83\xEC\x10\x53\x8B\x5D\x08\x57\x8B\xF9"));
}