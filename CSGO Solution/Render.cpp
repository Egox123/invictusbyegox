#include "Render.hpp"
#include "SDK/Globals.hpp"
#include "Features/Visuals/Players.hpp"
#include "Features/Log Manager/LogManager.hpp"
#include "Features/Visuals/World.hpp"
#include "Features/Movement/AutoPeek.hpp"
#include "Features/Markers/HitMarker.hpp"
#include "Features/Markers/Damage.hpp"
#include "Features/RageBot/Antiaim.hpp"
#include "Data/UserIcon.h"

void C_Render::Instance( )
{
	m_DrawList->Clear( );
	m_DrawList->PushClipRectFullScreen( );

	g_PlayerESP->Instance( );
	g_LogManager->Instance( );
	g_World->Grenades( );
	g_World->VelocityGraph();
	//g_World->AmbientModulation( );
	g_World->DrawScopeLines( );
	g_World->PenetrationCrosshair( );
	g_AutoPeek->DrawCircle( );
	g_HitMarkers->Instance( );
	g_DmgMarkers->Instance( );

	if ( g_Globals.m_LocalPlayer && g_Globals.m_LocalPlayer->IsAlive( ) )
	{
		int nScreenSizeX, nScreenSizeY;
		g_Globals.m_Interfaces.m_EngineClient->GetScreenSize( nScreenSizeX, nScreenSizeY );

		// �� ��� ���������, ��� ����� � ��� �������� � 5 ����
		if ( g_Settings->m_bAntiAim )
		{
			int nManualSide = g_AntiAim->GetManualSide( );
			if ( nManualSide != 1337 )
			{
				if ( nManualSide == 0 )
					m_DrawList->AddTriangleFilled( ImVec2(nScreenSizeX / 2, nScreenSizeY / 2 + 80), ImVec2(nScreenSizeX / 2 - 10, nScreenSizeY / 2 + 60), ImVec2(nScreenSizeX / 2 + 10, nScreenSizeY / 2 + 60), ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
				else if ( nManualSide == -1 )
					m_DrawList->AddTriangleFilled( ImVec2(nScreenSizeX / 2 - 55, nScreenSizeY / 2 + 10), ImVec2(nScreenSizeX / 2 - 75, nScreenSizeY / 2), ImVec2(nScreenSizeX / 2 - 55, nScreenSizeY / 2 - 10), ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
				else if ( nManualSide == 1 )
					m_DrawList->AddTriangleFilled( ImVec2(nScreenSizeX / 2 + 55, nScreenSizeY / 2 - 10), ImVec2(nScreenSizeX / 2 + 75, nScreenSizeY / 2), ImVec2(nScreenSizeX / 2 + 55, nScreenSizeY / 2 + 10), ImColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
			}
		}

		
	}


	g_RenderMutex.lock( );
	*m_DrawListAct = *m_DrawList;
	g_RenderMutex.unlock( );
}

ImColor C_Render::GetU32( Color aColor )
{
	return ( ( aColor[ 3 ] & 0xff ) << 24 ) + ( ( aColor[ 2 ] & 0xff ) << 16 ) + ( ( aColor[ 1 ] & 0xff ) << 8 ) + ( aColor[ 0 ] & 0xff );
}

void C_Render::ForceReload( std::string szFontName, int nSize )
{
	m_bReload = true;
	m_szFontName = szFontName;
	m_nFontSize = nSize;
}

void C_Render::DrawRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, Color color2, float thickness, float prog, bool fill_prog)
{
	//static float Step = (DirectX::XM_PI * 2.0f) / (points);
	//std::vector<ImVec2> point;
	//for (float lat = 0.f; lat <= DirectX::XM_PI * 2.0f; lat += Step)
	//{
	//	const auto& point3d = Vector(sin(lat), cos(lat), 0.f) * radius;
	//	Vector point2d;
	//	if (Math::WorldToScreen(Vector(x, y, z) + point3d, point2d))
	//		point.push_back(ImVec2(point2d.x, point2d.y));
	//}
	//m_DrawList->AddConvexPolyFilled(point.data(), point.size(), GetU32(color2));
	//m_DrawList->AddPolyline(point.data(), max(0, point.size() * prog), GetU32(color1), prog == 1.f, thickness);
}

void C_Render::ReloadFonts( )
{
	if ( !m_bReload )
		return;

	m_bReload = false;

	ImFont* m_Font = ImGui::GetIO( ).Fonts->AddFontFromFileTTF( std::string( "C:/Windows/Fonts/" + m_szFontName + ".ttf" ).c_str( ), m_nFontSize, NULL, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) );
	if ( !m_Font )
		return;

	if ( m_szFontName == m_szLastFont && m_nFontSize == m_nLastFontSize )
		return;
	
	ImGui_ImplDX9_DestroyFontsTexture( );

	ImFontConfig cfg;
	cfg.PixelSnapH = 0;
	cfg.OversampleH = 5;
	cfg.OversampleV = 5;
	cfg.RasterizerMultiply = 1.2f;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0xE000, 0xE226, // icons
		0,
	};

	cfg.GlyphRanges = ranges;
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 16.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF((void*)RobotoBold_compressed_data, RobotoBold_compressed_size, 40.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/segoeui.ttf"), 20.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/segoeui.ttf"), 14.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	//ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)icomoon, sizeof(icomoon), 24.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/segoeui.ttf"), 18.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	g_Globals.m_Fonts.m_MenuIcons = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Icons_compressed_data, Icons_compressed_size, 18.0f, &cfg);
	g_Globals.m_Fonts.m_LogIcons = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Icons_compressed_data, Icons_compressed_size, 12.0f, &cfg);
	g_Globals.m_Fonts.m_BombIcon = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(BombFont_compressed_data, BombFont_compressed_size, 15.0f, &cfg);
	g_Globals.m_Fonts.m_SegoeUI = m_Font;
	g_Globals.m_Fonts.m_SmallFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 9.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_LogFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/seguisb.ttf"), 16.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_WeaponIcon = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(WeaponIcons_compressed_data_base85, 16.f, &cfg );
	g_Globals.m_Fonts.m_BigIcons = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(WeaponIcons_compressed_data_base85, 24.f, &cfg );
	//g_Globals.m_Fonts.m_UserIcon = ImGui::GetIO( ).Fonts->AddFontFromMemoryCompressedTTF( usericon_compressed_data, usericon_compressed_size, 14.0f, NULL, NULL );

	ImGui::GetIO().Fonts->AddFontDefault();

	ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, 0x00);

	m_nLastFontSize = m_nFontSize;
	m_szLastFont = m_szFontName;
}

void C_Render::RenderText( std::string strText, ImVec2 vecTextPosition, Color aTextColor, bool bForceCentered, bool bForceOutlined, ImFont* pDrawFont, bool bCalledFromMenu )
{
	ImVec2 ImTextSize = pDrawFont->CalcTextSizeA( pDrawFont->FontSize, FLT_MAX, 0.0f, strText.c_str( ) );
	if ( !pDrawFont->ContainerAtlas )
		return;

	ImDrawList* OldDrawList = m_DrawList;
	if ( bCalledFromMenu )
		m_DrawList = ImGui::GetOverlayDrawList( );
		
	m_DrawList->PushTextureID( pDrawFont->ContainerAtlas->TexID );
	if ( bForceCentered )
		vecTextPosition.x -= ImTextSize.x / 2.0f;

	if ( bForceOutlined ) 
	{
		m_DrawList->AddText( pDrawFont, pDrawFont->FontSize, ImVec2( vecTextPosition.x + 1, vecTextPosition.y + 1), GetU32( Color( 30, 30, 36, aTextColor.a( ) ) ), strText.c_str( ) );
		m_DrawList->AddText( pDrawFont, pDrawFont->FontSize, ImVec2( vecTextPosition.x - 1, vecTextPosition.y - 1), GetU32( Color( 30, 30, 36, aTextColor.a( ) ) ), strText.c_str( ) );
		m_DrawList->AddText( pDrawFont, pDrawFont->FontSize, ImVec2( vecTextPosition.x + 1, vecTextPosition.y - 1), GetU32( Color( 30, 30, 36, aTextColor.a( ) ) ), strText.c_str( ) );
		m_DrawList->AddText( pDrawFont, pDrawFont->FontSize, ImVec2( vecTextPosition.x - 1, vecTextPosition.y + 1), GetU32( Color( 30, 30, 36, aTextColor.a( ) ) ), strText.c_str( ) );
	}

	m_DrawList->AddText( pDrawFont, pDrawFont->FontSize, vecTextPosition, GetU32( aTextColor ), strText.c_str( ) );
	m_DrawList->PopTextureID( );

	if ( bCalledFromMenu )
		m_DrawList = OldDrawList;
}

#include "SDK/Math/Math.hpp"
void C_Render::RenderArc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness)
{
	m_DrawList->PathArcTo( ImVec2 ( x, y ), radius, DEG2RAD( min_angle ), DEG2RAD( max_angle ), 32 );
	m_DrawList->PathStroke( GetU32( col ), false, thickness );
}

void C_Render::RenderCircle3D( Vector vecPosition, int32_t iPointCount, float_t flRadius, Color aColor )
{
	float_t flStep = ( float_t )( 3.14159265358979323846f ) * 2.0f / ( float_t )( iPointCount );
	for ( float a = 0; a < ( 3.14159265358979323846f * 2.0f); a += flStep )
	{
		Vector vecStart = Vector( flRadius * cosf( a ) + vecPosition.x, flRadius * sinf( a ) + vecPosition.y, vecPosition.z );
		Vector vecEnd = Vector( flRadius * cosf( a + flStep ) + vecPosition.x, flRadius * sinf( a + flStep ) + vecPosition.y, vecPosition.z );

		Vector vecStart2D, vecEnd2D;
		if ( g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition( vecStart, vecStart2D ) || g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition( vecEnd, vecEnd2D ) )
			return;

		this->RenderLine( vecStart2D.x, vecStart2D.y, vecEnd2D.x, vecEnd2D.y, aColor, 1.0f );
	}
}

void C_Render::RenderRect3DFilled(Vector vecPosition, Vector vecPosition2, float_t flThickness, Color aColor)
{
	Vector vMin, vMax;

	if (g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition(vecPosition, vMin))
		return;
	if (g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition(vecPosition2, vMax))
		return;
	
	RenderLine(vMin.x, vMin.y, vMax.x, vMin.y, aColor, flThickness); // bottom line.
	RenderLine(vMin.x, vMax.y, vMax.x, vMin.y, aColor, flThickness); // top line.
	RenderLine(vMin.x, vMin.y, vMin.x, vMax.y, aColor, flThickness); // bottom line.
	RenderLine(vMax.x, vMin.y, vMax.x, vMax.y, aColor, flThickness); // bottom line.
}

void C_Render::RenderCircle3DFilled( const Vector& vecOrigin, float_t flRadius, Color aColor )
{
	static Vector vecPreviousScreenPos = Vector( 0, 0, 0 );
	static float_t flStep = 3.14159265358979323846f * 2.0f / 72.0f;

	Vector vecScreenPosition = Vector( 0, 0, 0 );
	if ( g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition( vecOrigin, vecScreenPosition ) )
		return;

	for ( float_t flRotation = 0.0f; flRotation <= 3.14159265358979323846f * 2.0f; flRotation += flStep ) //-V1034
	{
		Vector vecWorldPosition = Vector( flRadius * cos( flRotation ) + vecOrigin.x, flRadius * sin( flRotation ) + vecOrigin.y, vecOrigin.z );
		if ( g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition( vecWorldPosition, vecScreenPosition ) )
			continue;
		
		RenderLine( vecPreviousScreenPos.x, vecPreviousScreenPos.y, vecScreenPosition.x, vecScreenPosition.y, aColor, 1.0f );
		RenderTriangle
		( 
			ImVec2( vecScreenPosition.x		,	vecScreenPosition.y			), 
			ImVec2( vecScreenPosition.x		,	vecScreenPosition.y			),
			ImVec2( vecPreviousScreenPos.x	,	vecPreviousScreenPos.y		),
			Color( aColor.r( ), aColor.g( ), aColor.b( ), aColor.a( ) / 2	)
		);

		vecPreviousScreenPos = vecScreenPosition;
	}
}

void C_Render::RenderCircle2D( Vector vecPosition, int32_t iPointCount, float_t flRadius, Color aColor )
{
	Vector vecScreenPosition = Vector( 0, 0, 0 );
	if ( g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition( vecPosition, vecScreenPosition ) )
		return;

	m_DrawList->AddCircle( ImVec2( vecScreenPosition.x, vecScreenPosition.y ), flRadius, GetU32( aColor ), iPointCount );
}

void C_Render::RenderCircle2DFilled( Vector vecPosition, int32_t iPointCount, float_t flRadius, Color aColor )
{
	Vector vecScreenPosition = Vector( 0, 0, 0 );
	if ( g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition( vecPosition, vecScreenPosition ) )
		return;

	m_DrawList->AddCircleFilled( ImVec2( vecScreenPosition.x, vecScreenPosition.y ), flRadius, GetU32( aColor ), iPointCount );
}

void C_Render::RenderTriangle( ImVec2 vecFirst, ImVec2 vecSecond, ImVec2 vecThird, Color aColor )
{
	m_DrawList->AddTriangleFilled( vecFirst, vecSecond, vecThird, GetU32( aColor ) );
}

void C_Render::RenderLine( float x1, float y1, float x2, float y2, Color aColor, float_t flThickness )
{
	m_DrawList->AddLine( ImVec2( x1, y1 ), ImVec2( x2, y2 ), GetU32( aColor ), flThickness );
}
void C_Render::RenderRect(float x1, float y1, float x2, float y2, Color aColor)
{
	m_DrawList->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(aColor), 0.0f);
}
void C_Render::RenderRectFilled( float x1, float y1, float x2, float y2, Color aColor )
{
	m_DrawList->AddRectFilled( ImVec2( x1, y1 ), ImVec2( x2, y2 ), GetU32( aColor ), 0.0f );
}

void C_Render::Initialize( )
{
	m_DrawList			= new ImDrawList( ImGui::GetDrawListSharedData( ) );
	m_DrawListAct		= new ImDrawList( ImGui::GetDrawListSharedData( ) );
	m_DrawListRender	= new ImDrawList( ImGui::GetDrawListSharedData( ) );

	ImFontConfig cfg;
	cfg.PixelSnapH = 0;
	cfg.OversampleH = 5;
	cfg.OversampleV = 5;
	cfg.RasterizerMultiply = 1.2f;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0xE000, 0xE226, // icons
		0,
	};

	static const ImWchar ffoontranges[] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x044F, // Cyrillic
		0,
	};

	ImFontConfig font_config;
	font_config.OversampleH = 1; //or 2 is the same
	font_config.OversampleV = 1;
	font_config.PixelSnapH = 1;

	cfg.GlyphRanges = ranges;
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 16.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF((void*)RobotoBold_compressed_data, RobotoBold_compressed_size, 40.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 20.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 14.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	//ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)icomoon, sizeof(icomoon), 24.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 18.f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	g_Globals.m_Fonts.m_MenuIcons	= ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Icons_compressed_data, Icons_compressed_size, 18.0f, &cfg);
	g_Globals.m_Fonts.m_LogIcons	= ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(Icons_compressed_data, Icons_compressed_size, 12.0f, &cfg);
	g_Globals.m_Fonts.m_BombIcon	= ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(BombFont_compressed_data, BombFont_compressed_size, 15.0f, &cfg);
	g_Globals.m_Fonts.m_SegoeUI		= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 15.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_TitleFont	= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/segoeui.ttf"), 66.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_LogFont		= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 16.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_WeaponIcon	= ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(WeaponIcons_compressed_data_base85, 16.f, &cfg );
	g_Globals.m_Fonts.m_WeaponMass  = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(WeaponIcons_compressed_data_base85, 40.f, &cfg );
	g_Globals.m_Fonts.m_BigIcons	= ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(WeaponIcons_compressed_data_base85, 24.f, &cfg );
	g_Globals.m_Fonts.m_SmallFont	= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/verdana.ttf"), 9.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	//g_Globals.m_Fonts.m_UserIcon	= ImGui::GetIO( ).Fonts->AddFontFromMemoryCompressedTTF( usericon_compressed_data, usericon_compressed_size, 14.0f, NULL, NULL );

	cfg.RasterizerMultiply = 1.1f;

	g_Globals.m_Fonts.m_ESP			= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/tahoma.ttf"), 14.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_Flags		= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/tahoma.ttf"), 13.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_Damage		= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/tahoma.ttf"), 20.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	g_Globals.m_Fonts.m_Warning		= ImGui::GetIO().Fonts->AddFontFromFileTTF(_S("C:/windows/fonts/tahoma.ttf"), 26.0f, &cfg, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	ImGui::GetIO().Fonts->AddFontDefault();

	ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, 0x00);
}

ImDrawList* C_Render::GetDrawList( )
{
	if ( g_RenderMutex.try_lock( ) ) 
	{
		*m_DrawListRender = *m_DrawListAct;
		g_RenderMutex.unlock( );
	}

	return m_DrawListRender;
}