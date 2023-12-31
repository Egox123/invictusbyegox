#include <functional>
#include <fstream>
#include <unordered_map>
#include <filesystem>

#include "Config.hpp"
#include "Settings.hpp"

#include "SDK/Utils/JSON.hpp"
#include "Json.hpp"
#define CREATE_SERIALIZER( t, fun ) { _S( #t ), [ ]( std::any const& a ) -> std::string { return fun( std::any_cast< t const& >( a ) ); } }
#define CREATE_DESERIALIZER( t, fun ) { _S( #t ), fun }

#define SET_COLOR( json, color ) \
json[ _S( "r" ) ] = color[ 0 ];\
json[ _S( "g" ) ] = color[ 1 ];\
json[ _S( "b" ) ] = color[ 2 ];\
json[ _S( "a" ) ] = color[ 3 ];\

#define LOAD_COLOR( json, color ) \
color[ 0 ] = json[ _S( "r" ) ];\
color[ 1 ] = json[ _S( "g" ) ];\
color[ 2 ] = json[ _S( "b" ) ];\
color[ 3 ] = json[ _S( "a" ) ];\

static std::unordered_map< std::string, std::function< std::string( std::any const& ) >> SerializeVisitors
{
	CREATE_SERIALIZER( int, [ ]( int x ) -> std::string
	{
		return std::to_string( x );
	} ),
	CREATE_SERIALIZER( int32_t, [ ]( int32_t x ) -> std::string
	{
		return std::to_string( x );
	} ),
	CREATE_SERIALIZER( bool, [ ]( bool x ) -> std::string
	{
		return x ? _S( "true" ) : _S( "false" );
	} ),
	CREATE_SERIALIZER( Color, [ ]( Color c ) -> std::string
	{
		nlohmann::json r;
		r[0] = c.r( );
		r[1] = c.g( );
		r[2] = c.b( );
		r[3] = c.a( );
		return r.dump( );
	} ), 
	CREATE_SERIALIZER( C_PlayerSettings, [ ]( C_PlayerSettings aVisualSettings ) -> std::string
	{
		nlohmann::json jVisual;
		jVisual[ _S( "Draw Box" ) ] = aVisualSettings.m_BoundaryBox;
		jVisual[ _S( "Draw Name" ) ] = aVisualSettings.m_RenderName;
		jVisual[ _S( "Draw Weapon Text" ) ] = aVisualSettings.m_RenderWeaponText;
		jVisual[ _S( "Draw Weapon Icon" ) ] = aVisualSettings.m_RenderWeaponIcon;
		jVisual[ _S( "Draw Healthbar" ) ] = aVisualSettings.m_RenderHealthBar;
		jVisual[ _S( "Draw Healthbar Text" ) ] = aVisualSettings.m_RenderHealthText;
		jVisual[ _S( "Draw Ammobar" ) ] = aVisualSettings.m_RenderAmmoBar;
		jVisual[ _S( "Draw Ammobar Text" ) ] = aVisualSettings.m_RenderAmmoBarText;
		jVisual[ _S( "Glow style" ) ] = aVisualSettings.m_iGlowStyle;
		jVisual[ _S( "Render glow" ) ] = aVisualSettings.m_bRenderGlow;

		SET_COLOR( jVisual[ _S( "Box Color" ) ], aVisualSettings.m_aBoundaryBox );
		SET_COLOR( jVisual[ _S( "Name Color" ) ], aVisualSettings.m_aNameColor );
		SET_COLOR( jVisual[ _S( "Weapon Text Color" ) ], aVisualSettings.m_aWeaponText );
		SET_COLOR( jVisual[ _S( "Weapon Icon Color" ) ], aVisualSettings.m_aWeaponIcon );
		SET_COLOR( jVisual[ _S( "Healthbar Color" ) ], aVisualSettings.m_aHealthBar );
		SET_COLOR( jVisual[ _S( "Healthbar text Color" ) ], aVisualSettings.m_aHealthText );
		SET_COLOR( jVisual[ _S( "Ammobar Color" ) ], aVisualSettings.m_aAmmoBar );
		SET_COLOR( jVisual[ _S( "Ammobar text Color" ) ], aVisualSettings.m_aAmmoBarText );
		SET_COLOR( jVisual[ _S( "Glow Color" ) ], aVisualSettings.m_aGlow );

		for ( int nFuckPutin = 0; nFuckPutin < 5; nFuckPutin++ )
		{
			jVisual[ _S( "Flag Dick" ) ][ nFuckPutin ] = aVisualSettings.m_Flags[ nFuckPutin ];
			SET_COLOR( jVisual[ _S( "Flag Color" ) ][ nFuckPutin ], aVisualSettings.m_Colors[ nFuckPutin ] );
		}

		return jVisual.dump( );
	} ),
	CREATE_SERIALIZER( C_KeyBind, [ ]( C_KeyBind c ) -> std::string 
	{
		nlohmann::json jKeyBind;
		jKeyBind[ _S( "Bind" ) ] = c.m_iKeySelected;
		jKeyBind[ _S( "Mode" ) ] = c.m_iModeSelected;
		return jKeyBind.dump( );
	} ),
	CREATE_SERIALIZER( float_t, [ ]( float_t flValue ) -> std::string
	{
		return std::to_string( flValue );
	} ),
	CREATE_SERIALIZER( float, [ ]( float flValue ) -> std::string
	{
		return std::to_string( flValue );
	} ),
	CREATE_SERIALIZER( std::string, [ ]( std::string szValue ) -> std::string
	{
		return szValue;
	} )
};

static std::unordered_map< std::string, std::function< void( const std::string& s, std::any* ) >> DeSerializeVisitors
{
	CREATE_DESERIALIZER(int, [](std::string content, std::any* ptr) -> void
	{
		ptr->emplace< int >(std::atoi(content.c_str()));
	}),
	CREATE_DESERIALIZER(int32_t, [](std::string content, std::any* ptr) -> void
	{
		ptr->emplace< int32_t >(std::atoi(content.c_str()));
	}),
	CREATE_DESERIALIZER(bool, [](std::string content, std::any* ptr) -> void
	{
		ptr->emplace< bool >(content == _S("true"));
	}),
	CREATE_DESERIALIZER(Color, [](std::string content, std::any* ptr) -> void
	{
		nlohmann::json r = nlohmann::json::parse(content);
		Color clr(r[0].get< int >(), r[1].get< int >(), r[2].get< int >(), r[3].get< int >());
		ptr->emplace< Color >(clr);
	}),
	CREATE_DESERIALIZER(C_KeyBind, [](std::string content, std::any* ptr) -> void
	{
		nlohmann::json jKeyBind = nlohmann::json::parse(content);

		C_KeyBind KeyBind;
		KeyBind.m_iKeySelected = jKeyBind[_S("Bind")].get< int >();
		KeyBind.m_iModeSelected = jKeyBind[_S("Mode")].get< int >();

		ptr->emplace< C_KeyBind >(KeyBind);
	}),
	CREATE_DESERIALIZER(C_PlayerSettings, [](std::string content, std::any* ptr) -> void
	{
		nlohmann::json jVisual = nlohmann::json::parse(content);

		C_PlayerSettings aVisualSettings;
		aVisualSettings.m_BoundaryBox = jVisual[_S("Draw Box")].get< bool >();
		aVisualSettings.m_RenderName = jVisual[_S("Draw Name")].get< bool >();
		aVisualSettings.m_RenderWeaponText = jVisual[_S("Draw Weapon Text")].get< bool >();
		aVisualSettings.m_RenderWeaponIcon = jVisual[_S("Draw Weapon Icon")].get< bool >();
		aVisualSettings.m_RenderHealthBar = jVisual[_S("Draw Healthbar")].get< bool >();
		aVisualSettings.m_RenderHealthText = jVisual[_S("Draw Healthbar Text")].get< bool >();
		aVisualSettings.m_RenderAmmoBar = jVisual[_S("Draw Ammobar")].get< bool >();
		aVisualSettings.m_RenderAmmoBarText = jVisual[_S("Draw Ammobar Text")].get< bool >();
		aVisualSettings.m_bRenderGlow = jVisual[_S("Render glow")].get< bool >();
		aVisualSettings.m_iGlowStyle = jVisual[_S("Glow style")].get< int >();

		LOAD_COLOR(jVisual[_S("Box Color")], aVisualSettings.m_aNameColor);
		LOAD_COLOR(jVisual[_S("Name Color")], aVisualSettings.m_aNameColor);
		LOAD_COLOR(jVisual[_S("Weapon Text Color")], aVisualSettings.m_aWeaponText);
		LOAD_COLOR(jVisual[_S("Weapon Icon Color")], aVisualSettings.m_aWeaponIcon);
		LOAD_COLOR(jVisual[_S("Healthbar Color")], aVisualSettings.m_aHealthBar);
		LOAD_COLOR(jVisual[_S("Healthbar text Color")], aVisualSettings.m_aHealthText);
		LOAD_COLOR(jVisual[_S("Ammobar Color")], aVisualSettings.m_aAmmoBar);
		LOAD_COLOR(jVisual[_S("Ammobar text Color")], aVisualSettings.m_aAmmoBarText);
		LOAD_COLOR(jVisual[_S("Glow Color")], aVisualSettings.m_aGlow);

		for ( int nFuckPutin = 0; nFuckPutin < 5; nFuckPutin++ )
		{
			aVisualSettings.m_Flags[nFuckPutin] = jVisual[_S("Flag Dick")][nFuckPutin];
			LOAD_COLOR(jVisual[_S("Flag Color")][nFuckPutin], aVisualSettings.m_Colors[nFuckPutin]);
		}

		ptr->emplace< C_PlayerSettings >(aVisualSettings);
	}),
	CREATE_DESERIALIZER(float_t, [](std::string content, std::any* ptr) -> void
	{
		ptr->emplace< float_t >(std::atof(content.c_str()));
	}),
	CREATE_DESERIALIZER(float, [](std::string content, std::any* ptr) -> void
	{
		ptr->emplace< float >(std::atof(content.c_str()));
	}),
	CREATE_DESERIALIZER(std::string, [](std::string content, std::any* ptr) -> void
	{
		ptr->emplace< std::string >(content);
	})
};

void C_ConfigSystem::SaveConfig( const char* szConfigName )
{
	nlohmann::json jConfig;
	for ( auto& item : m_aItems )
	{
		nlohmann::json cur;

		cur[ 1 ] = item.Type;
		cur[ 0 ] = item.Name;

		if ( const auto it = SerializeVisitors.find( item.Type ); it != SerializeVisitors.cend( ) )
		{
			cur[ 2 ] = it->second( item.Var );
		}
		else 
		{
			cur[ 2 ] = "???";
		}

		jConfig[ _S( "Automatic" ) ].push_back(cur);
	}
	
	nlohmann::json& jManual = jConfig[ _S( "Manual" ) ];
	for ( int i = 0; i < g_Settings->m_aEquipment.size( ); i++ )
		jManual[ _S( "Equipment" ) ][ std::to_string( i ) ] = g_Settings->m_aEquipment[ i ];
	
	for ( int i = 0; i < g_Settings->m_aWorldRemovals.size( ); i++ )
		jManual[ _S( "Removals" ) ][ std::to_string( i ) ] = g_Settings->m_aWorldRemovals[ i ];

	for ( int i = 0; i < g_Settings->m_aFakelagTriggers.size( ); i++ )
		jManual[ _S( "Fakelags" ) ][ std::to_string( i ) ] = g_Settings->m_aFakelagTriggers[ i ];

	for ( int i = 0; i < g_Settings->m_aChamsSettings.size( ); i++ )
	{
		jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Render chams" ) ] = g_Settings->m_aChamsSettings[ i ].m_bRenderChams;
		jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Main material" ) ] = g_Settings->m_aChamsSettings[ i ].m_iMainMaterial;

		for ( int j = 0; j < 4; j++ )
		{
			jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ std::to_string( j ) ] = g_Settings->m_aChamsSettings[ i ].m_Color[ j ];
		
			jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Modifiers" ) ][ std::to_string( j ) ] = g_Settings->m_aChamsSettings[ i ].m_aModifiers[ j ];
			SET_COLOR( jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Modifiers Colors" ) ][ std::to_string( j ) ], g_Settings->m_aChamsSettings[ i ].m_aModifiersColors[ j ] );
		}
	}

	for ( int i = 0; i < g_Settings->m_aRageSettings.size( ); i++ )
	{
		jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Autostop" ) ] = g_Settings->m_aRageSettings[ i ].m_bAutoStop;
		jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Autoscope" ) ] = g_Settings->m_aRageSettings[ i ].m_bAutoScope;
		jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Mindamage" ) ] = g_Settings->m_aRageSettings[ i ].m_iMinDamage;
		jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Mindamage override" ) ] = g_Settings->m_aRageSettings[ i ].m_iMinDamageOverride;
		jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Hitchance" ) ] = g_Settings->m_aRageSettings[ i ].m_iHitChance;
		jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Head scale" ) ] = g_Settings->m_aRageSettings[ i ].m_iHeadScale;
		jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Body scale" ) ] = g_Settings->m_aRageSettings[ i ].m_iBodyScale;
	
		for ( int j = 0; j < 8; j++ )
		{
			jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Hitboxes" ) ][ std::to_string( j ) ] = g_Settings->m_aRageSettings[ i ].m_Hitboxes[ j ];
			jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Safe Hitboxes" ) ][ std::to_string( j ) ] = g_Settings->m_aRageSettings[ i ].m_SafeHitboxes[ j ];
			jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Multipoints" ) ][ std::to_string( j ) ] = g_Settings->m_aRageSettings[ i ].m_Multipoints[ j ];
		}

		for ( int j = 0; j < 3; j++ )
		{
			jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Modifiers" ) ][ std::to_string( j ) ] = g_Settings->m_aRageSettings[ i ].m_RageModifiers[ j ];
		}

		for ( int j = 0; j < 5; j++ )
		{
			jManual[_S("Rage settings")][std::to_string(i)][_S("Safe modifiers")][std::to_string(j)] = g_Settings->m_aRageSettings[i].m_SafeModifiers[j];
		}

		for ( int j = 0; j < 2; j++ )
			jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Autostop options" ) ][ std::to_string( j ) ] = g_Settings->m_aRageSettings[ i ].m_AutoStopOptions[ j ];

		for ( int j = 0; j < 2; j++ )
			jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Doubletap options" ) ][ std::to_string( j ) ] = g_Settings->m_aRageSettings[ i ].m_DoubleTapOptions[ j ];
	}


	for (int i = 0; i < g_Settings->m_aRageSettings.size(); i++)
	{
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Left desync")] = g_Settings->m_aAntiAim[i].m_iLeftDesync;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Right desync")] = g_Settings->m_aAntiAim[i].m_iRightDesync;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Yaw offset ")] = g_Settings->m_aAntiAim[i].m_iYawOffset;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Yaw left offset ")] = g_Settings->m_aAntiAim[i].m_iLeftYawOffset;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Yaw jitter")] = g_Settings->m_aAntiAim[i].m_iYawJitter;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Randomize jitter")] = g_Settings->m_aAntiAim[i].m_bRandomizeJitter;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Target")] = g_Settings->m_aAntiAim[i].m_bTargets;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Desync jitter")] = g_Settings->m_aAntiAim[i].m_bDesyncJitter;
		jManual[_S("Anti aim settings")][std::to_string(i)][_S("Randomize desync")] = g_Settings->m_aAntiAim[i].m_bRandomizeDesync;
	}

	for ( int i = 0; i < g_Settings->skins.skinChanger.size(); i++ )
	{
		jManual[_S("Skin changer")][std::to_string(i)][_S("IIDI")] = g_Settings->skins.skinChanger[i].itemIdIndex;
		jManual[_S("Skin changer")][std::to_string(i)][_S("IID")] = g_Settings->skins.skinChanger[i].itemId;
		jManual[_S("Skin changer")][std::to_string(i)][_S("EQVI")] = g_Settings->skins.skinChanger[i].entity_quality_vector_index;
		jManual[_S("Skin changer")][std::to_string(i)][_S("QLTY")] = g_Settings->skins.skinChanger[i].quality;
		jManual[_S("Skin changer")][std::to_string(i)][_S("PKVI")] = g_Settings->skins.skinChanger[i].paint_kit_vector_index;
		jManual[_S("Skin changer")][std::to_string(i)][_S("PK")] = g_Settings->skins.skinChanger[i].paintKit;
		jManual[_S("Skin changer")][std::to_string(i)][_S("DOVI")] = g_Settings->skins.skinChanger[i].definition_override_vector_index;
		jManual[_S("Skin changer")][std::to_string(i)][_S("DOI")] = g_Settings->skins.skinChanger[i].definition_override_index;
		jManual[_S("Skin changer")][std::to_string(i)][_S("SEED")] = g_Settings->skins.skinChanger[i].seed;
		jManual[_S("Skin changer")][std::to_string(i)][_S("SKNNM")] = g_Settings->skins.skinChanger[i].skin_name;
		jManual[_S("Skin changer")][std::to_string(i)][_S("STTRK")] = g_Settings->skins.skinChanger[i].stat_trak;
		jManual[_S("Skin changer")][std::to_string(i)][_S("WEAR")] = g_Settings->skins.skinChanger[i].wear;
		jManual[_S("Skin changer")][std::to_string(i)][_S("CKSNM")] = g_Settings->skins.custom_name_tag[i];
		std::string aRR = std::string(g_Settings->skins.skinChanger[i].custom_name);

		jManual[_S("Skin changer")][std::to_string(i)][_S("SKNCH")] = aRR;
	}

	std::ofstream out_file( ( ( std::string )( _S( "C:\\invictus by egox\\" ) ) + ( std::string )( szConfigName ) ) + ".cfg", std::ios::out | std::ios::trunc);
	out_file << jConfig.dump(4);
	out_file.close();
}

std::vector < std::string > C_ConfigSystem::GetConfigList( )
{
	std::filesystem::create_directory("C:\\invictus by egox\\");

	std::vector < std::string > m_ConfigList;
	for ( const auto& p : std::filesystem::recursive_directory_iterator( _S( "C:\\invictus by egox\\" ) ) )
	{
		if ( !std::filesystem::is_directory(p) && p.path().extension().string() == _S( ".cfg" ) )
		{
			auto file = p.path( ).filename( ).string( );
			m_ConfigList.push_back( file.substr( 0, file.size( ) - 4 ) );
		}
	}
	
	return m_ConfigList;
}

void C_ConfigSystem::EraseConfig( const char* szConfigName )
{
	std::string path = ( ( std::string )( _S( "C:\\invictus by egox\\" ) ) + ( std::string )( szConfigName ) + ".cfg" );

	const char * c = path.c_str();

	remove( c );
}

void C_ConfigSystem::LoadConfig( const char* szConfigName )
{
	nlohmann::json jConfig;

	std::ifstream config_file( ( ( std::string )( _S( "C:\\invictus by egox\\" ) ) + ( std::string )( szConfigName ) + ".cfg" ), std::ios::in);
	if ( !config_file.good( ) )
		return;

	config_file >> jConfig;
	config_file.close();

	for ( auto& item : jConfig[ _S( "Automatic" ) ] )
	{
		const std::string item_identifier = item[ 0 ].get< std::string >( );
		const std::string item_type = item[ 1 ].get< std::string >( );
		const std::string inner = item[ 2 ].get< std::string >( );

		auto item_index = -1;
		for ( auto i = 0u; i < m_aItems.size( ); i++ )
		{
			if ( m_aItems.at( i ).Name == item_identifier )
			{
				item_index = i;
				break;
			}
		}

		if ( item_index < 0 )
			continue;

		auto& cur_item = m_aItems.at( item_index );
		if ( const auto it = DeSerializeVisitors.find(item_type); it != DeSerializeVisitors.cend( ) )
		{
			it->second( inner, &cur_item.Var );
		}
	}

	nlohmann::json jManual = jConfig[ _S( "Manual" ) ];
	for ( int i = 0; i < g_Settings->m_aEquipment.size( ); i++ )
		 g_Settings->m_aEquipment[ i ] = jManual[ _S( "Equipment" ) ][ std::to_string( i ) ].get < bool >( );
	
	for ( int i = 0; i < g_Settings->m_aWorldRemovals.size( ); i++ )
		g_Settings->m_aWorldRemovals[ i ] = jManual[ _S( "Removals" ) ][ std::to_string( i ) ].get < bool >( );

	for ( int i = 0; i < g_Settings->m_aFakelagTriggers.size( ); i++ )
		g_Settings->m_aFakelagTriggers[ i ] = jManual[ _S( "Fakelags" ) ][ std::to_string( i ) ].get < bool > ( );
		
	for ( int i = 0; i < jManual[ _S( "Chams settings" ) ].size( ); i++ )
	{
		g_Settings->m_aChamsSettings[ i ].m_bRenderChams = jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Render chams" ) ].get < bool >( );
		g_Settings->m_aChamsSettings[ i ].m_iMainMaterial = jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Main material" ) ].get < int >( );

		for ( int j = 0; j < 4; j++ )
		{
			g_Settings->m_aChamsSettings[ i ].m_Color[ j ] = jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ std::to_string( j ) ].get < int >( );
		
			if ( jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Modifiers" ) ].is_object( ) )
			{
				g_Settings->m_aChamsSettings[ i ].m_aModifiers[ j ] = jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Modifiers" ) ][ std::to_string( j ) ].get < bool >( );
				LOAD_COLOR( jManual[ _S( "Chams settings" ) ][ std::to_string( i ) ][ _S( "Modifiers Colors" ) ][ std::to_string( j ) ], g_Settings->m_aChamsSettings[ i ].m_aModifiersColors[ j ] );
			}
		}
	}

	for ( int i = 0; i < g_Settings->m_aRageSettings.size( ); i++ )
	{
		g_Settings->m_aRageSettings[ i ].m_bAutoScope = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Autoscope" ) ].get < bool >( );
		g_Settings->m_aRageSettings[ i ].m_bAutoStop = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Autostop" ) ].get < bool >( );
		if ( jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Mindamage" ) ].is_number_integer( ) )
			g_Settings->m_aRageSettings[ i ].m_iMinDamage = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Mindamage" ) ].get < int >( );
		g_Settings->m_aRageSettings[ i ].m_iMinDamageOverride = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Mindamage override" ) ].get < int >( );
		g_Settings->m_aRageSettings[ i ].m_iHitChance = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Hitchance" ) ].get < int >( );
		g_Settings->m_aRageSettings[ i ].m_iHeadScale = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Head scale" ) ].get < int >( );
		g_Settings->m_aRageSettings[ i ].m_iBodyScale = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Body scale" ) ].get < int >( );
	
		for ( int j = 0; j < 8; j++ )
		{
			g_Settings->m_aRageSettings[ i ].m_Hitboxes[ j ] = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Hitboxes" ) ][ std::to_string( j ) ].get < bool >( );
			g_Settings->m_aRageSettings[ i ].m_SafeHitboxes[ j ] = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Safe Hitboxes" ) ][ std::to_string( j ) ].get < bool >( );
			g_Settings->m_aRageSettings[ i ].m_Multipoints[ j ] = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Multipoints" ) ][ std::to_string( j ) ].get < bool >( );
		}
		
		for ( int j = 0; j < 3; j++ )
		{
			g_Settings->m_aRageSettings[ i ].m_RageModifiers[ j ] = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Modifiers" ) ][ std::to_string( j ) ].get < bool >( );
		}

		for ( int j = 0; j < 5; j++ )
		{
			g_Settings->m_aRageSettings[i].m_SafeModifiers[j] = jManual[_S("Rage settings")][std::to_string(i)][_S("Safe modifiers")][std::to_string(j)].get < bool >();
		}

		for ( int j = 0; j < 2; j++ )
			g_Settings->m_aRageSettings[ i ].m_AutoStopOptions[ j ] = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Autostop options" ) ][ std::to_string( j ) ].get < bool >( );

		for ( int j = 0; j < 2; j++ )
			g_Settings->m_aRageSettings[ i ].m_DoubleTapOptions[ j ] = jManual[ _S( "Rage settings" ) ][ std::to_string( i ) ][ _S( "Doubletap options" ) ][ std::to_string( j ) ].get < bool >( );
	}

	for (int i = 0; i < g_Settings->m_aRageSettings.size(); i++)
	{
		g_Settings->m_aAntiAim[i].m_iLeftDesync = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Left desync")].get < int >();;
		g_Settings->m_aAntiAim[i].m_iRightDesync = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Right desync")].get < int >();;
		g_Settings->m_aAntiAim[i].m_iYawOffset = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Yaw offset ")].get < int >();;
		g_Settings->m_aAntiAim[i].m_iLeftYawOffset = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Yaw left offset ")].get < int >();;
		g_Settings->m_aAntiAim[i].m_iYawJitter = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Yaw jitter")].get < int >();;
		g_Settings->m_aAntiAim[i].m_bRandomizeJitter = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Randomize jitter")].get < bool >();
		g_Settings->m_aAntiAim[i].m_bTargets = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Target")].get < bool >();
		g_Settings->m_aAntiAim[i].m_bDesyncJitter = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Desync jitter")].get < bool >();
		g_Settings->m_aAntiAim[i].m_bRandomizeDesync = jManual[_S("Anti aim settings")][std::to_string(i)][_S("Randomize desync")].get < bool >();
	}

	for ( int i = 0; i < g_Settings->skins.skinChanger.size(); i++ )
	{
		g_Settings->skins.skinChanger[i].itemIdIndex = jManual[_S("Skin changer")][std::to_string(i)][_S("IIDI")].get< int >();
		g_Settings->skins.skinChanger[i].itemId = jManual[_S("Skin changer")][std::to_string(i)][_S("IID")].get< int >();
		g_Settings->skins.skinChanger[i].entity_quality_vector_index = jManual[_S("Skin changer")][std::to_string(i)][_S("EQVI")].get< int >();
		g_Settings->skins.skinChanger[i].quality = jManual[_S("Skin changer")][std::to_string(i)][_S("QLTY")].get< int >();
		g_Settings->skins.skinChanger[i].paint_kit_vector_index = jManual[_S("Skin changer")][std::to_string(i)][_S("PKVI")].get< int >();
		g_Settings->skins.skinChanger[i].paintKit = jManual[_S("Skin changer")][std::to_string(i)][_S("PK")].get< int >();
		g_Settings->skins.skinChanger[i].definition_override_vector_index = jManual[_S("Skin changer")][std::to_string(i)][_S("DOVI")].get< int >();
		g_Settings->skins.skinChanger[i].definition_override_index = jManual[_S("Skin changer")][std::to_string(i)][_S("DOI")].get< int >();		
		g_Settings->skins.skinChanger[i].seed = jManual[_S("Skin changer")][std::to_string(i)][_S("SEED")].get< int >();
		g_Settings->skins.skinChanger[i].skin_name = jManual[_S("Skin changer")][std::to_string(i)][_S("SKNNM")].get< std::string >();
		g_Settings->skins.skinChanger[i].stat_trak = jManual[_S("Skin changer")][std::to_string(i)][_S("STTRK")].get< int >();
		g_Settings->skins.skinChanger[i].wear = jManual[_S("Skin changer")][std::to_string(i)][_S("WEAR")].get< float >();
		//g_Settings->skins.custom_name_tag[i] = jManual[_S("Skin changer")][std::to_string(i)][_S("CKSNM")].get< std::string >();
		/*std::string customNameToString = jManual[_S("Skin changer")][std::to_string(i)][_S("SKNCH")].get< std::string >();

		strcpy_s(g_Settings->skins.skinChanger[i].custom_name, sizeof(g_Settings->skins.skinChanger[i].custom_name), customNameToString.c_str());*/
		// g_Settings->skins.skinChanger[i].update();
	}
}