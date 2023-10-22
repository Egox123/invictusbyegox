#pragma once
#include <map>
#include "Blur/BlurShaders/blur_x.h"
#include "ImGui/imgui_freetype.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_internal.h"
#include "Tools/Obfuscation/XorStr.hpp"
#pragma comment( lib, "freetype.lib" )

#include <d3dx9.h>
#include "SDK/Utils/Color/Color.hpp"

struct Bind_t
{
	std::string m_szName = "";
	float_t m_flAlphaPercent = 0.0f;
};

class C_Menu
{
public:
	virtual void Instance( );
	virtual void Initialize( );

	virtual void DrawRageTab( );
	virtual void DrawAntiAimTab( );
	virtual void DrawMiscTab( );
	virtual void DrawViewTab();
	virtual void DrawPlayersTab( );
	virtual void DrawWorldTab( ); 
	virtual void DrawConfigTab( ); 
	virtual void DrawLegitTab( );
	virtual void DrawInventoryTab();
	virtual void DrawColorEdit4( const char* strLabel, Color* aColor, int32_t bSingleElement = 1 );

	virtual void SetMenuState( bool bState ) { this->m_bIsMenuOpened = bState; };
	virtual bool IsMenuOpened( ) { return this->m_bIsMenuOpened; };
	virtual const char* GetMenuName() { return _S("invictus"); };
	virtual ImVec2 GetTabSize() {return m_vecTabSize;};

	virtual void WaterMark( );
	virtual void DrawKeybindList( );
	virtual void DrawSpectatorList( );
	virtual int32_t ShouldChildFrameOutputText( ) { return m_bStopChildFrameTextOutput; };
	virtual const char* GetPreviewModelPath() { return m_cPlayerModel; };
	virtual bool FixColorSpacing() { return m_bFixColorSpacing; };

	float m_cBuffer[4];
private:
	bool m_bIsMenuOpened = false;
	int m_bStopChildFrameTextOutput = 0;
	const char* m_cPlayerModel = "a";
	bool m_bFixColorSpacing = false;

	ImVec2 m_vecTabSize = ImVec2(0, 0);

	std::map < uint32_t, Bind_t > m_BindList;
};

inline C_Menu* g_Menu = new C_Menu( );