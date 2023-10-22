#pragma once
#include <array>
#include "Tools/Obfuscation/XorStr.hpp"
#include "Config.hpp"
#include "../nSkinz/SkinChanger.h"
#include "../nSkinz/item_definitions.hpp"

struct C_KeyBind
{
	int32_t m_iModeSelected = 0;
	int32_t m_iKeySelected = 0;
};

enum REMOVALS
{
	REMOVALS_VISUAL_PUNCH = 0,
	REMOVALS_VISUAL_KICK = 1,
	REMOVALS_VISUAL_SCOPE = 2,
	REMOVALS_VISUAL_SMOKE = 3,
	REMOVALS_VISUAL_FLASH = 4,
	REMOVALS_VISUAL_POSTPROCESS = 5,
	REMOVALS_VISUAL_FOG = 6,
	REMOVALS_VISUAL_SHADOWS = 7,
	REMOVALS_VISUAL_LANDING_BOB = 8,
	REMOVALS_VISUAL_HAND_SHAKING = 9
};

enum AUTOSTOP
{
	AUTOSTOP_ACCURACY,
	AUTOSTOP_EARLY
};

enum AIM_POINTS
{
	PREFER_SAFE,
	FORCE_SAFE,
	PREFER_BODY,
	FORCE_BODY
};

enum DOUBLETAP_OPTIONS
{
	MOVE_BETWEEN_SHOTS,
	FULL_STOP
};

enum FAKELAG_TRIGGERS
{
	FAKELAG_MOVE,
	FAKELAG_AIR,
	FAKELAG_PEEK
};

struct item_setting
{
	void update()
	{
		itemId = game_data::weapon_names[itemIdIndex].definition_index;
		quality = game_data::quality_names[entity_quality_vector_index].index;

		const std::vector <SkinChanger::PaintKit>* kit_names;
		const game_data::weapon_name* defindex_names;

		if (itemId == GLOVE_T_SIDE)
		{
			kit_names = &SkinChanger::gloveKits;
			defindex_names = game_data::glove_names;
		}
		else
		{
			kit_names = &SkinChanger::skinKits;
			defindex_names = game_data::knife_names;
		}

		paintKit = (*kit_names)[paint_kit_vector_index].id;
		definition_override_index = defindex_names[definition_override_vector_index].definition_index;
		skin_name = (*kit_names)[paint_kit_vector_index].skin_name;
	}

	int itemIdIndex = 0;
	int itemId = 1;
	int entity_quality_vector_index = 0;
	int quality = 0;
	int paint_kit_vector_index = 0;
	int paintKit = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	int stat_trak = 0;
	float wear = 0.0f;
	char custom_name[24] = "\0";
	std::string skin_name;
};

struct C_LegitSettings
{
	bool m_bAutowall = false;
	bool m_bAutostop = false;
	
	int32_t m_iAimType = 1;
	int32_t m_iPriority = 0;
	int32_t m_iFovType = 0;
	int32_t m_iSmoothType = 0;
	int32_t m_iRcsType = 0;
	int32_t m_iHitboxPriority = 1;
	int32_t m_iShotDelay = 0;
	int32_t m_iKillDelay = 0;
	int32_t m_iRcsAbscissa = 100;
	int32_t m_iRcsOrdinate = 100;
	int32_t m_iRcsStart = 1;
	int32_t m_iMinDamage = 1;
	
	float_t m_flFieldOfView = 0.f;
	float_t m_flSilentFov = 0.f;
	float_t m_flRcsFov = 0.f;
	float_t m_flSmooth = 1;
	float_t m_flRcsSmooth = 0;
};

// thanks @es3n1n
template < typename T >
class C_CheatVar 
{
private:
	uint32_t m_VarValue;
public:
	C_CheatVar( std::string v1, std::string v2, T v3 ) 
	{
		m_VarValue = g_ConfigSystem->PushItem< T >( v1, v2, v3 );
	}

	T& Get( )
	{
		return *g_ConfigSystem->GetConfigValue< T >( m_VarValue );
	}

	T* GetPtr( )
	{
		return g_ConfigSystem->GetConfigValue< T >( m_VarValue );
	}
	
	operator T&( )
	{
		return Get( );
	}
	operator T* ( )
	{
		return GetPtr();
	}
	T* operator->( )
	{
		return GetPtr();
	}
	operator T&( ) const
	{
		return Get();
	}
};

struct C_ChamsSettings
{
	int32_t m_iMainMaterial = 0;

	bool m_bRenderChams = false;
	bool m_aModifiers[ 4 ] = { false, false, false };

	Color m_aModifiersColors[ 4 ] = { { 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 } };
	Color m_Color = Color( 255, 255, 255, 255 );
	
	int32_t m_iWireframeType = 0;
	int32_t m_iWireframeMaterial = 0;
	Color m_WireframeColor = Color( 255, 255, 255, 255 );
};

struct C_PlayerSettings
{
	bool m_BoundaryBox = false;
	bool m_RenderName = false;
	bool m_RenderHealthBar = false;
	bool m_RenderHealthText = false;
	bool m_RenderAmmoBar = false;
	bool m_RenderAmmoBarText = false;
	bool m_RenderWeaponText = false;
	bool m_RenderWeaponIcon = false;

	bool m_bRenderGlow = false;
	int m_iGlowStyle = 0;
	Color m_aGlow = Color( 255, 255, 255, 255 );

	Color m_aBoundaryBox = Color( 255, 255, 255, 255 );
	Color m_aNameColor = Color( 255, 255, 255, 255 );
	Color m_aHealthBar = Color( 255, 255, 255, 255 );
	Color m_aHealthText = Color( 255, 255, 255, 255 );
	Color m_aAmmoBar = Color( 255, 255, 255, 255 );
	Color m_aAmmoBarText = Color( 255, 255, 255, 255 );
	Color m_aArmorBar = Color( 255, 255, 255, 255 );
	Color m_aArmorBarText = Color( 255, 255, 255, 255 );
	Color m_aWeaponText = Color( 255, 255, 255, 255 );
	Color m_aWeaponIcon = Color( 255, 255, 255, 255 );

	bool m_Flags[ 5 ];
	Color m_Colors[ 5 ] = { Color(255, 255, 255, 255), Color(255, 255, 255, 255), Color(255, 255, 255, 255), Color(255, 255, 255, 255), Color(255, 255, 255, 255) };
};

struct C_AntiAimSettings
{
	int32_t m_iLeftDesync;
	int32_t m_iRightDesync;
	int32_t m_iYawOffset;
	int32_t m_iLeftYawOffset;
	int32_t m_iYawJitter;
	bool m_bRandomizeJitter;
	bool m_bTargets;
	bool m_bDesyncJitter;
	bool m_bRandomizeDesync;
};

struct C_RageSettings
{
	int32_t m_iFOV = 180;
	int32_t m_iHitChance = 0;
	
	int32_t m_iMinDamage = 0;
	int32_t m_iMinDamageOverride = 0;

	int32_t m_iHeadScale = 50;
	int32_t m_iBodyScale = 50;
	int32_t m_iLimbScale = 50;

	bool m_AutoStopOptions[ 2 ] = { false, false }; // Force Accuracy, Early
	bool m_DoubleTapOptions[ 2 ] = { false, false }; // Move between shots, stop
	bool m_Hitboxes[ 8 ] = { true, true, true, true, true, true, false, false }; // head, chest, arms, pelvis, stomach, legs
	bool m_SafeHitboxes[ 8 ] = { }; // head, chest, arms, pelvis, stomach, legs
	bool m_RageModifiers[ 3 ] = { }; // Prefer body, Prefer safe, Wait overlap
	bool m_SafeModifiers[ 5 ] = { }; // Stand, slow walk, walk, air, lethal
	bool m_Multipoints[ 8 ] = { }; // head, chest, arms, pelvis, stomach, legs

	bool m_bAutoStop = false;
	bool m_bAutoScope = false;
};

#define SETTING( type, var, val ) C_CheatVar< type > var = C_CheatVar< type >( _S( #var ), _S( #type ), val )
class C_Settings
{
public:

	struct Skins_t
	{
		bool rare_animations;
		std::array <item_setting, 36> skinChanger;
		std::string custom_name_tag[36];
	} skins;

	SETTING( int, m_nModelCT, 0 );
	SETTING( int, m_nModelT, 0 );

	SETTING( bool, m_bBunnyHop, false );
	SETTING( bool, m_bAntiUntrusted, true );
	SETTING( bool, m_bAutoStrafe, false );
	SETTING( bool, m_bWASDStrafe, false );
	SETTING( bool, m_bSpeedBoost, false );
	SETTING( bool, m_bEdgeJump, false );
	SETTING( bool, m_bFilterConsole, false );
	SETTING( bool, m_bUnhideConvars, false );
	SETTING( bool, m_bRevealRanks, false );
	SETTING( bool, m_bAdBlock, false );
	SETTING( bool, m_bInfinityDuck, false );
	SETTING( bool, m_bFastStop, false );
	SETTING( bool, m_bTagChanger, false );
	SETTING( bool, m_bHitSound, false );
	SETTING( bool, m_bWaterMark, false );
	SETTING( bool, m_bSpectatorList, false );
	SETTING( bool, m_bDrawKeyBindList, false );
	SETTING( bool, m_bUnlockInventoryAccess, false );
	SETTING( bool, m_bOutOfViewArrows, false );
	SETTING( float, m_flAspectRatio, 1.77f );
	SETTING( int, m_nHitSound, 0 );

	SETTING(bool, m_bMotionBlur, false);
	SETTING(bool, m_bForwardBlur, false);
	SETTING(float, m_flFallingMin, 10.f);
	SETTING(float, m_flFallingMax, 10.f);
	SETTING(float, m_flFallingIntensitiy, 1.f);
	SETTING(float, m_flRotationIntensitiy, 1.f);
	SETTING(float, m_flBlurStrength, 1.f);

	SETTING( bool, m_CustomWeather, false );
	SETTING( int, m_WeatherType, 0 );
	SETTING( int, m_RainDensity, 100 );
	SETTING( int, m_RainLength, 50 );
	SETTING( int, m_RainWidth, 50 );
	SETTING( int, m_RainSpeed, 600 );
	SETTING( int, m_TracerType, 6 );
	SETTING( float, m_TracerWidth, 1.0f );

	SETTING( bool, m_bPenetrationCrosshair, false );
	SETTING( bool, m_bForceCrosshair, false );
	SETTING( bool, m_bCustomCrosshair, false );
	SETTING( bool, m_bHitMarker, false );
	SETTING( bool, m_bDamageMarker, false );
	SETTING( bool, m_bEffectMarker, false );
	SETTING( bool, m_bVelocityGraph, false );

	SETTING( bool, m_bPredictGrenades, false );
	SETTING( bool, m_bGrenadeTrajectory, false );
	SETTING( bool, m_GrenadeTimers, false );

	SETTING( bool, m_bHoldFireAnimation, false );
	SETTING( bool, m_bPreserveKillfeed, false );

	SETTING( bool, m_bFakeLagEnabled, false );
	SETTING( int32_t, m_iLagLimit, 0 );
	SETTING( int32_t, m_iTriggerLimit, 0 );
	SETTING( int32_t, m_iFlVariance, 0 );
	
	SETTING( bool, m_bAntiAim, false );
	SETTING( int, m_iSpeedWalk, 0 );
	SETTING( int, m_iPitchMode, 1 );
	SETTING( int, m_iLegMovement, 0 );
	SETTING( bool, m_bJitterMove, false );

	SETTING( bool, m_bDrawServerImpacts, false );
	SETTING( bool, m_bDrawClientImpacts, false );
	SETTING( bool, m_bDrawLocalTracers, false );
	SETTING( bool, m_bDrawEnemyTracers, false );
	SETTING( bool, m_bDrawRagdolls, false );

	SETTING( bool, m_bOverrideFOVWhileScoped, false );
	SETTING( bool, m_bViewmodelInScope, false );
	SETTING( bool, m_bSingleZoom, false );
	SETTING( int32_t, m_iThirdPersonDistance, 90 );
	SETTING( int32_t, m_iCameraDistance, 90 );
	SETTING( int32_t, m_iViewmodelDistance, 60 );
	SETTING( int32_t, m_iViewmodelX, 1 ); 
	SETTING( int32_t, m_iViewmodelY, 1 );
	SETTING( int32_t, m_iViewmodelZ, -1 );
	SETTING( int32_t, m_iViewmodelRoll, 0 );
	SETTING( int32_t, m_iCrosshairAxisLength, 0 );
	SETTING( int32_t, m_iCrosshairAxisSpacing, 0 );
	SETTING( int32_t, m_iCrosshairAxisFill, 0 );
	SETTING( int32_t, m_iSkybox, 0 );
	SETTING( float_t, m_flRechargeTime, 0.5f );
	SETTING( int32_t, m_iWantedPing, 5 );
	SETTING( int32_t, m_VisualInterpolation, 14 );
	SETTING( std::string, m_szCustomSkybox, "" );
	SETTING( std::string, m_szCustomFont, "" );

	SETTING( bool, m_bBuyBotEnabled, false );
	SETTING( bool, m_bBuyBotKeepAWP, false );
	SETTING( int, m_BuyBotPrimaryWeapon, 0 );
	SETTING( int, m_BuyBotSecondaryWeapon, 0 );

	SETTING( bool, m_bLogPurchases, false );
	SETTING( bool, m_bLogBomb, false );
	SETTING( bool, m_bLogHurts, false );
	SETTING( bool, m_bLogHarms, false );
	SETTING( bool, m_bLogMisses, false );
	SETTING( bool, m_bEnabledRage, false );
	SETTING( bool, m_bRageAutoscope, false );
	SETTING( bool, m_bSmoothAnimations, false );
	SETTING( bool, m_bOnShotUnscope, false );

	SETTING( bool, m_bRenderC4Glow, false );
	SETTING( int, m_iC4GlowStyle, 0 );

	SETTING( bool, m_bRenderDroppedWeaponGlow, false );
	SETTING( int, m_iDroppedWeaponGlowStyle, 0 );

	SETTING( bool, m_bRenderProjectileGlow, false );
	SETTING( int, m_iProjectileGlowStyle, 0 );

	SETTING( bool, m_bLegitBot, false );
	SETTING( bool, m_bLegitScope, false );
	SETTING( bool, m_bLegitPistol, false );
	SETTING( bool, m_bIgnoreSmoke, false );
	SETTING( bool, m_bIgnoreFlash, false );
	SETTING( bool, m_bIgnoreInAir, false );
	SETTING( bool, m_bScopeOnly, false );
	SETTING( int32_t, m_iFireDelay, 0 );

	SETTING( Color, m_aProjectileGlow, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_WorldModulation, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_PropModulation, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_SkyModulation, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_ClientImpacts, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_ServerImpacts, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_LocalTracers, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_EnemyTracers, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_VelocityGraphCol, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_OnHitDamage, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_GrenadeWarning, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_GrenadeWarningTimer, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_HitmarkerColor, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_aC4Glow, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_aDroppedWeaponGlow, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_aOutOfViewArrows, Color( 255, 255, 255, 255 ) );
	SETTING( Color, m_colMenuTheme, Color( 210, 76, 255, 255 ) );
	SETTING( Color, m_colTabFade, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colTabText, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colTabHeader, Color( 20, 20, 20, 255 ) );
	SETTING( Color, m_colCheckbox, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colSlider, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colCombo, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colKeybind, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colCrosshair, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colCrosshairS, Color(210, 76, 255, 255 ) );
	SETTING( Color, m_colGrenadeTrajectory, Color(210, 76, 255, 255 ) );

	SETTING( C_KeyBind, m_aSlowwalk, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aCSlowwalk, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aThirdPerson, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aAutoPeek, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aFakeDuck, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aInverter, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aMinDamage, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aSafePoint, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aLegitKey, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aTriggerKey, C_KeyBind( ) );

	SETTING( C_KeyBind, m_aDoubleTap, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aHideShots, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aPingSpike, C_KeyBind( ) );

	SETTING( C_KeyBind, m_aManualLeft, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aManualBack, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aManualRight, C_KeyBind( ) );
	SETTING( C_KeyBind, m_aFreestand, C_KeyBind( ) );

	// 0 - enemy
	// 1 - teammate
	// 2 - localplayer
	SETTING( C_PlayerSettings, m_LocalPlayer, C_PlayerSettings( ) );
	SETTING( C_PlayerSettings, m_Teammates, C_PlayerSettings( ) );
	SETTING( C_PlayerSettings, m_Enemies, C_PlayerSettings( ) );

	// 0 - enemy
	// 1 - enemy invisible
	// 2 - backtrack 
	// 3 - shot chams
	// 4 - team
	// 5 - team invis
	// 6 - local
	// 7 - desync
	// 8 - lag
	// 9 - hands
	// 10 - weapons
	// 11 - attachments
	std::array < C_ChamsSettings, 12 > m_aChamsSettings = { };

	// 0 - punch
	// 1 - kick
	// 2 - scope
	// 3 - smoke
	// 4 - flash
	// 5 - postprocessing
	// 6 - fog
	// 7 - shadows
	// 8 - landing bob
	// 9 - hand shaking
	std::array < bool, 10 > m_aWorldRemovals = { };

	// fakelag triggers
	// 0 - Move
	// 1 - Air
	// 2 - Peek
	std::array < bool, 3 > m_aFakelagTriggers = { };

	// rage settings
	// 0 - Autosniper
	// 1 - Scout
	// 2 - AWP
	// 3 - Deagle
	// 4 - Revolver
	// 5 - Pistols
	// 6 - Rifles

	std::array < C_RageSettings, 7 > m_aRageSettings = { };

	// equipment
	// 0 - Fire/Molotov
	// 1 - Smoke grenade
	// 2 - Flash grenade
	// 3 - HE grenade
	// 4 - Taser
	// 5 - Heavy armor
	// 6 - Helmet
	// 7 - Defuser
	std::array < bool, 7 > m_aEquipment = { };

	// fog and world.
	SETTING( bool, m_bFog, false ); 
	SETTING( bool, m_bAmbientModulation, false ); 
	SETTING( Color, m_bFogColor, Color(0,0,0) ); 
	SETTING( int, m_iFogDistance, 0 ); 
	SETTING( int, m_iFogDensity, 0 ); 

	SETTING(float, m_flBloom, 0.0f);
	SETTING(float, m_flExposure, 0.0f);
	SETTING(float, m_flAmbient, 0.0f);

	// anti aim
	// 0 - standing 
	// 1 - move
	// 2 - slow walk
	// 3 - air
	std::array < C_AntiAimSettings, 4 > m_aAntiAim = { };
	
	// legit bot
	// 0 - pistol
	// 1 - heavy pistol
	// 2 - r8
	// 3 - rifle
	// 4 - smg
	// 5 - heavy
	// 6 - shotgun
	// 7 - scout
	// 8 - auto
	// 9 - awp
	std::array < C_LegitSettings, 10 > m_LegitBotItems = {};

	SETTING(bool, m_bVMMovement, false);
};

inline C_Settings* g_Settings = new C_Settings( );