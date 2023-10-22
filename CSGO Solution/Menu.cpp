#include <d3d9.h>
#include "Tools/Obfuscation/XorStr.hpp"
#include "SDK/Globals.hpp"

#include "Features/Model/Model.hpp"
#include <d3dx9.h>
#pragma comment (lib, "d3dx9.lib")

#include "Render.hpp"
#include "Config.hpp"
#include "Settings.hpp"
#include "Features/Packet/PacketManager.hpp"

#define GET_ARR_SIZE( x ) ( int )( sizeof( x ) / sizeof( x[0] ) )

enum STATE_ID {
	STATE_NONE = 0,
	STATE_SELECTING_WEAPON,
	STATE_SELECTING_PAINTKIT,
	STATE_CONFIGURING_WEAPON
};

void AlignSingularColorPicker()
{
	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 30 );
}

void InputSliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d")
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	ImGui::SliderInt(label, v, v_min, v_max, format);

	g_Globals.m_bRenderInputTxtFrame = false;
	ImGui::SameLine();
	std::string str = label;
	str += "li";
	ImGui::InputInt(str.c_str(), v, 0);
	g_Globals.m_bRenderInputTxtFrame = true;

	window->DC.CursorPos.y -= 7;
}

bool InputSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f", float power = 1.0f)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	float f = *v;

	ImGui::SliderFloat(label, v, v_min, v_max, format, power);

	g_Globals.m_bRenderInputTxtFrame = false;
	ImGui::SameLine();
	std::string str = label;
	str += "li";
	ImGui::InputFloat(str.c_str(), v, 0.f, 0.f, format);
	g_Globals.m_bRenderInputTxtFrame = true;

	bool b = false;

	if ( f != *v )
		b = true;

	window->DC.CursorPos.y -= 7;

	return b;
}

static const char* m_aPlayerModelList[] =
{
	"models/player/custom_player/legacy/ctm_diver_varianta.mdl", // Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman
	"models/player/custom_player/legacy/ctm_diver_variantb.mdl", // Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman
	"models/player/custom_player/legacy/ctm_diver_variantc.mdl", // Lieutenant Rex Krikey | SEAL Frogman
	"models/player/custom_player/legacy/ctm_fbi_varianth.mdl", // Michael Syfers | FBI Sniper
	"models/player/custom_player/legacy/ctm_fbi_variantf.mdl", // Operator | FBI SWAT
	"models/player/custom_player/legacy/ctm_fbi_variantb.mdl", // Special Agent Ava | FBI
	"models/player/custom_player/legacy/ctm_fbi_variantg.mdl", // Markus Delrow | FBI HRT
	"models/player/custom_player/legacy/ctm_gendarmerie_varianta.mdl", // Sous-Lieutenant Medic | Gendarmerie Nationale
	"models/player/custom_player/legacy/ctm_gendarmerie_variantb.mdl", // Chem-Haz Capitaine | Gendarmerie Nationale
	"models/player/custom_player/legacy/ctm_gendarmerie_variantc.mdl", // Chef d'Escadron Rouchard | Gendarmerie Nationale
	"models/player/custom_player/legacy/ctm_gendarmerie_variantd.mdl", // Aspirant | Gendarmerie Nationale
	"models/player/custom_player/legacy/ctm_gendarmerie_variante.mdl", // Officer Jacques Beltram | Gendarmerie Nationale
	"models/player/custom_player/legacy/ctm_sas_variantg.mdl", // D Squadron Officer | NZSAS
	"models/player/custom_player/legacy/ctm_sas_variantf.mdl", // B Squadron Officer | SAS
	"models/player/custom_player/legacy/ctm_st6_variante.mdl", // Seal Team 6 Soldier | NSWC SEAL
	"models/player/custom_player/legacy/ctm_st6_variantg.mdl", // Buckshot | NSWC SEAL
	"models/player/custom_player/legacy/ctm_st6_varianti.mdl", // Lt. Commander Ricksaw | NSWC SEAL
	"models/player/custom_player/legacy/ctm_st6_variantj.mdl", // 'Blueberries' Buckshot | NSWC SEAL
	"models/player/custom_player/legacy/ctm_st6_variantk.mdl", // 3rd Commando Company | KSK
	"models/player/custom_player/legacy/ctm_st6_variantl.mdl", // 'Two Times' McCoy | TACP Cavalry
	"models/player/custom_player/legacy/ctm_st6_variantm.mdl", // 'Two Times' McCoy | USAF TACP
	"models/player/custom_player/legacy/ctm_st6_variantn.mdl", // Primeiro Tenente | Brazilian 1st Battalion
	"models/player/custom_player/legacy/ctm_swat_variante.mdl", // Cmdr. Mae 'Dead Cold' Jamison | SWAT
	"models/player/custom_player/legacy/ctm_swat_variantf.mdl", // 1st Lieutenant Farlow | SWAT
	"models/player/custom_player/legacy/ctm_swat_variantg.mdl", // John 'Van Healen' Kask | SWAT
	"models/player/custom_player/legacy/ctm_swat_varianth.mdl", // Bio-Haz Specialist | SWAT
	"models/player/custom_player/legacy/ctm_swat_varianti.mdl", // Sergeant Bombson | SWAT
	"models/player/custom_player/legacy/ctm_swat_variantj.mdl", // Chem-Haz Specialist | SWAT
	"models/player/custom_player/legacy/ctm_swat_variantk.mdl", // Lieutenant 'Tree Hugger' Farlow | SWAT
	"models/player/custom_player/legacy/tm_professional_varj.mdl", // Getaway Sally | The Professionals
	"models/player/custom_player/legacy/tm_professional_vari.mdl", // Number K | The Professionals
	"models/player/custom_player/legacy/tm_professional_varh.mdl", // Little Kev | The Professionals
	"models/player/custom_player/legacy/tm_professional_varg.mdl", // Safecracker Voltzmann | The Professionals
	"models/player/custom_player/legacy/tm_professional_varf5.mdl", // Bloody Darryl The Strapped | The Professionals
	"models/player/custom_player/legacy/tm_professional_varf4.mdl", // Sir Bloody Loudmouth Darryl | The Professionals
	"models/player/custom_player/legacy/tm_professional_varf3.mdl", // Sir Bloody Darryl Royale | The Professionals
	"models/player/custom_player/legacy/tm_professional_varf2.mdl", // Sir Bloody Skullhead Darryl | The Professionals
	"models/player/custom_player/legacy/tm_professional_varf1.mdl", // Sir Bloody Silent Darryl | The Professionals
	"models/player/custom_player/legacy/tm_professional_varf.mdl", // Sir Bloody Miami Darryl | The Professionals
	"models/player/custom_player/legacy/tm_phoenix_varianti.mdl", // Street Soldier | Phoenix
	"models/player/custom_player/legacy/tm_phoenix_varianth.mdl", // Soldier | Phoenix
	"models/player/custom_player/legacy/tm_phoenix_variantg.mdl", // Slingshot | Phoenix
	"models/player/custom_player/legacy/tm_phoenix_variantf.mdl", // Enforcer | Phoenix
	"models/player/custom_player/legacy/tm_leet_variantj.mdl", // Mr. Muhlik | Elite Crew
	"models/player/custom_player/legacy/tm_leet_varianti.mdl", // Prof. Shahmat | Elite Crew
	"models/player/custom_player/legacy/tm_leet_varianth.mdl", // Osiris | Elite Crew
	"models/player/custom_player/legacy/tm_leet_variantg.mdl", // Ground Rebel | Elite Crew
	"models/player/custom_player/legacy/tm_leet_variantf.mdl", // The Elite Mr. Muhlik | Elite Crew
	"models/player/custom_player/legacy/tm_jungle_raider_variantf2.mdl", // Trapper | Guerrilla Warfare
	"models/player/custom_player/legacy/tm_jungle_raider_variantf.mdl", // Trapper Aggressor | Guerrilla Warfare
	"models/player/custom_player/legacy/tm_jungle_raider_variante.mdl", // Vypa Sista of the Revolution | Guerrilla Warfare
	"models/player/custom_player/legacy/tm_jungle_raider_variantd.mdl", // Col. Mangos Dabisi | Guerrilla Warfare
	"models/player/custom_player/legacy/tm_jungle_raider_variantb2.mdl", // 'Medium Rare' Crasswater | Guerrilla Warfare
	"models/player/custom_player/legacy/tm_jungle_raider_variantb.mdl", // Crasswater The Forgotten | Guerrilla Warfare
	"models/player/custom_player/legacy/tm_jungle_raider_varianta.mdl", // Elite Trapper Solman | Guerrilla Warfare
	"models/player/custom_player/legacy/tm_balkan_varianth.mdl", // 'The Doctor' Romanov | Sabre
	"models/player/custom_player/legacy/tm_balkan_variantj.mdl", // Blackwolf | Sabre
	"models/player/custom_player/legacy/tm_balkan_varianti.mdl", // Maximus | Sabre
	"models/player/custom_player/legacy/tm_balkan_variantf.mdl", // Dragomir | Sabre
	"models/player/custom_player/legacy/tm_balkan_variantg.mdl", // Rezan The Ready | Sabre
	"models/player/custom_player/legacy/tm_balkan_variantk.mdl", // Rezan the Redshirt | Sabre
	"models/player/custom_player/legacy/tm_balkan_variantl.mdl", // Dragomir | Sabre Footsoldier
};

IDirect3DTexture9* all_skins[36];

template < std::size_t ConfigArraySize >
const char* GetComboName( std::string& input, int size, std::array<bool, ConfigArraySize> var )
{
	int total = 0;

	for ( auto i = 0; i < size; i++ )
	{
		if ( var[i] == true )
		{
			total++;
		}
	}

	input = std::string("Selected ") + std::to_string(total) + std::string(" options.");

	total = 0;

	return input.c_str();
}

const char* GetComboName(std::string& input, int size, bool var[])
{
	int total = 0;

	for ( auto i = 0; i < size; i++ )
	{
		if ( var[i] == true )
		{
			total++;
		}
	}

	input = std::to_string(total) + std::string("/") + std::to_string(size) + std::string(" selected");

	total = 0;

	return input.c_str();
}

std::string get_wep(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			switch (custom_index)
			{
			case 0: return _S("weapon_knife");
			case 1: return _S("weapon_bayonet");
			case 2: return _S("weapon_knife_css");
			case 3: return _S("weapon_knife_skeleton");
			case 4: return _S("weapon_knife_outdoor");
			case 5: return _S("weapon_knife_cord");
			case 6: return _S("weapon_knife_canis");
			case 7: return _S("weapon_knife_flip");
			case 8: return _S("weapon_knife_gut");
			case 9: return _S("weapon_knife_karambit");
			case 10: return _S("weapon_knife_m9_bayonet");
			case 11: return _S("weapon_knife_tactical");
			case 12: return _S("weapon_knife_falchion");
			case 13: return _S("weapon_knife_survival_bowie");
			case 14: return _S("weapon_knife_butterfly");
			case 15: return _S("weapon_knife_push");
			case 16: return _S("weapon_knife_ursus");
			case 17: return _S("weapon_knife_gypsy_jackknife");
			case 18: return _S("weapon_knife_stiletto");
			case 19: return _S("weapon_knife_widowmaker");
			}
		}
		else
		{
			switch (custom_index)
			{
			case 0: return _S("ct_gloves"); //-V1037
			case 1: return _S("studded_bloodhound_gloves");
			case 2: return _S("t_gloves");
			case 3: return _S("ct_gloves");
			case 4: return _S("sporty_gloves");
			case 5: return _S("slick_gloves");
			case 6: return _S("leather_handwraps");
			case 7: return _S("motorcycle_gloves");
			case 8: return _S("specialist_gloves");
			case 9: return _S("studded_hydra_gloves");
			}
		}
	}
	else
	{
		switch (id)
		{
		case 0: return _S("knife");
		case 1: return _S("gloves");
		case 2: return _S("weapon_ak47");
		case 3: return _S("weapon_aug");
		case 4: return _S("weapon_awp");
		case 5: return _S("weapon_cz75a");
		case 6: return _S("weapon_deagle");
		case 7: return _S("weapon_elite");
		case 8: return _S("weapon_famas");
		case 9: return _S("weapon_fiveseven");
		case 10: return _S("weapon_g3sg1");
		case 11: return _S("weapon_galilar");
		case 12: return _S("weapon_glock");
		case 13: return _S("weapon_m249");
		case 14: return _S("weapon_m4a1_silencer");
		case 15: return _S("weapon_m4a1");
		case 16: return _S("weapon_mac10");
		case 17: return _S("weapon_mag7");
		case 18: return _S("weapon_mp5sd");
		case 19: return _S("weapon_mp7");
		case 20: return _S("weapon_mp9");
		case 21: return _S("weapon_negev");
		case 22: return _S("weapon_nova");
		case 23: return _S("weapon_hkp2000");
		case 24: return _S("weapon_p250");
		case 25: return _S("weapon_p90");
		case 26: return _S("weapon_bizon");
		case 27: return _S("weapon_revolver");
		case 28: return _S("weapon_sawedoff");
		case 29: return _S("weapon_scar20");
		case 30: return _S("weapon_ssg08");
		case 31: return _S("weapon_sg556");
		case 32: return _S("weapon_tec9");
		case 33: return _S("weapon_ump45");
		case 34: return _S("weapon_usp_silencer");
		case 35: return _S("weapon_xm1014");
		default: return _S("unknown");
		}
	}
}

IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
{
	IDirect3DTexture9* skin_image = nullptr;
	std::string vpk_path;

	if (strcmp(weapon_name, _S("unknown")) && strcmp(weapon_name, _S("knife")) && strcmp(weapon_name, _S("gloves"))) //-V526
	{
		if (skin_name.empty() || skin_name == _S("default"))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + _S(".png");
		else
			vpk_path = _S("resource/flash/econ/default_generated/") + std::string(weapon_name) + _S("_") + std::string(skin_name) + _S("_light_large.png");
	}
	else
	{
		if (!strcmp(weapon_name, _S("knife")))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
		else if (!strcmp(weapon_name, _S("gloves")))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else if (!strcmp(weapon_name, _S("unknown")))
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");

	}
	const auto handle = g_Globals.m_Interfaces.m_FileSystem->Open(vpk_path.c_str(), _S("r"), _S("GAME"));
	if (handle)
	{
		int file_len = g_Globals.m_Interfaces.m_FileSystem->Size(handle);
		char* image = new char[file_len]; //-V121

		g_Globals.m_Interfaces.m_FileSystem->Read(image, file_len, handle);
		g_Globals.m_Interfaces.m_FileSystem->Close(handle);

		D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
		delete[] image;
	}

	if (!skin_image)
	{
		std::string vpk_path;

		if (strstr(weapon_name, _S("bloodhound")) != NULL || strstr(weapon_name, _S("hydra")) != NULL)
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = _S("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + _S(".png");

		const auto handle = g_Globals.m_Interfaces.m_FileSystem->Open(vpk_path.c_str(), _S("r"), _S("GAME"));

		if (handle)
		{
			int file_len = g_Globals.m_Interfaces.m_FileSystem->Size(handle);
			char* image = new char[file_len]; //-V121

			g_Globals.m_Interfaces.m_FileSystem->Read(image, file_len, handle);
			g_Globals.m_Interfaces.m_FileSystem->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}
	}

	return skin_image;
}


struct tab_struct
{
	float size;
	bool active;
};

bool Tab(const char* label, const ImVec2& size_arg, const bool selected)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(ImVec2(size.x + 10, size.y), style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

	static std::map<ImGuiID, tab_struct> selected_animation;
	auto it_selected = selected_animation.find(ImGui::GetItemID());

	if (it_selected == selected_animation.end())
	{
		selected_animation.insert({ ImGui::GetItemID(), {0.0f, false} });
		it_selected = selected_animation.find(ImGui::GetItemID());
	}
	it_selected->second.size = ImClamp(it_selected->second.size + (5.f * ImGui::GetIO().DeltaTime * (selected || hovered ? 1.f : -1.f)), 0.0f, 1.f);

	ImU32 color_text = ImGui::GetColorU32(ImLerp(ImVec4(83 / 255.f, 83 / 255.f, 84 / 255.f, 1.0f), ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 1.0f), it_selected->second.size));

	const ImVec2 text_size = ImGui::CalcTextSize(label);

	if ( hovered || selected )
		window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(20, 20, 20, 220), 8.f);

	window->DrawList->AddText(ImVec2(bb.Min.x + size_arg.x / 2 - ImGui::CalcTextSize(label).x / 2, bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2 - 1), color_text, label);

	return pressed;
}

static float tab_alpha = 0.0f;
static float tab_padding = 0.0f;

void C_Menu::Instance()
{
	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha > 0.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha - fc / 255.f, 0.f, 1.f);
	}

	if (m_bIsMenuOpened && ImGui::GetStyle().Alpha < 1.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha + fc / 255.f, 0.f, 1.f);
		ImGui::GetStyle().WindowRounding = 0.f;
	}

	this->DrawSpectatorList( );
	this->DrawKeybindList( );
	this->WaterMark( );

	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha < 0.1f)
		return;

	int32_t iScreenSizeX, iScreenSizeY;
	g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(iScreenSizeX, iScreenSizeY);
	
	ImGui::SetNextWindowPos(ImVec2((iScreenSizeX / 2) - 325, (iScreenSizeY / 2) - 220), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(600, 700), ImVec2(600, 700));
	ImGui::Begin(this->GetMenuName(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
	
	static int tab = 0;

	static bool draw_anim = false;
	static int anim_tab = 0;

	tab_padding = ImClamp(tab_padding + (75.f * ImGui::GetIO().DeltaTime * (draw_anim && tab != anim_tab ? 1.f : -1.f)), 0.0f, 15.f);
	tab_alpha = ImClamp(tab_alpha + (5.f * ImGui::GetIO().DeltaTime * (draw_anim && tab != anim_tab ? 1.f : -1.f)), 0.0f, 1.f);

	if (tab_alpha >= 1.0f && tab_padding >= 10.0f)
	{
		draw_anim = false;
		tab = anim_tab;
	}

	//ImGui::SetCursorPos({ 150, 0 });
	ImGui::BeginGroup();
	{
		ImVec2 iSingularSize = { 66, 45 };

		ImGui::SetCursorPos({ 0, 665 });
		if (Tab(_S("RAGE"), iSingularSize, anim_tab == 0))
			anim_tab = 0, draw_anim = true;
		
		ImGui::SetCursorPos({ 66, 665 });
		if (Tab(_S("HVH"), iSingularSize, anim_tab == 1))
			anim_tab = 1, draw_anim = true;
		
		ImGui::SetCursorPos({ 132, 665 });
		if (Tab(_S("LEGIT"), iSingularSize, anim_tab == 2))
			anim_tab = 2, draw_anim = true;

		ImGui::SetCursorPos({ 198, 665 });
		if (Tab(_S("PLAYER"), iSingularSize, anim_tab == 3))
			anim_tab = 3, draw_anim = true;
	
		ImGui::SetCursorPos({ 264, 665 });
		if (Tab(_S("WORLD"), iSingularSize, anim_tab == 4))
			anim_tab = 4, draw_anim = true;

		ImGui::SetCursorPos({ 330, 665 });
		if (Tab(_S("VIEW"), iSingularSize, anim_tab == 5))
			anim_tab = 5, draw_anim = true;

		ImGui::SetCursorPos({ 396, 665 });
		if (Tab(_S("MISC"), iSingularSize, anim_tab == 6))
			anim_tab = 6, draw_anim = true;

		ImGui::SetCursorPos({ 462, 665 });
		if (Tab(_S("SKINS"), iSingularSize, anim_tab == 7))
			anim_tab = 7, draw_anim = true;

		ImGui::SetCursorPos({ 528, 665 });
		if ( Tab(_S("CFG"), iSingularSize, anim_tab == 8) )
			anim_tab = 8, draw_anim = true;
	}
	ImGui::EndGroup();

	ImGui::SetCursorPos({ 33, 50 + tab_padding });

	auto draw = ImGui::GetWindowDrawList();
	auto pos = ImGui::GetWindowPos();

	ImGui::PushClipRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + 1000, pos.y + 650), false);
	ImGui::BeginGroup();
	{
		switch (tab)
		{
		case 0:
			this->DrawRageTab();
			break;
		case 1:
			this->DrawAntiAimTab();
			break;
		case 2:
			this->DrawLegitTab();
			break;
		case 3:
			this->DrawPlayersTab();
			break;
		case 4:
			this->DrawWorldTab();
			break;
		case 5:
			this->DrawViewTab();
			break;
		case 6:
			this->DrawMiscTab();
			break;
		case 7:
			this->DrawInventoryTab();
			break;
		case 8:
			this->DrawConfigTab();
			break;
		}
	}
	ImGui::EndGroup();
	ImGui::PopClipRect();

	ImGui::End();
}

void C_Menu::DrawRageTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	const int iChildDoubleSizeX = 250;

	ImGui::SetCursorPos(ImVec2(33, 120));
	ImGui::BeginChild("Main", ImVec2(iChildDoubleSizeX, 400));
	ImGui::Spacing(); ImGui::Spacing();

	const std::vector< const char* > aRageWeapons =
	{
		( "Auto" ),
		( "Scout" ),
		( "AWP" ),
		( "Deagle" ),
		( "Revolver" ),
		( "Pistol" ),
		( "Rifle" )
	};

	const std::vector< const char* > aHitboxes =
	{
		( "Head" ),
		( "Upper chest" ),
		( "Chest" ),
		( "Lower chest" ),
		( "Pelvis" ),
		( "Stomach" ),
		("Arms"),
		( "Legs" ),
	};

	const std::vector< const char* > aModifiers = 
	{
		( "Prefer body" ),
		( "Prefer safe" ),
		( "Wait overlap" ),
	};

	const std::vector< const char* > aSafeModifiers =
	{
		("Standing"),
		("Walking"),
		("Slow walking"),
		("Air"),
		("Lethal")
	};


	static int wep = 0;

	ImGui::Checkbox( "Enable ragebot", g_Settings->m_bEnabledRage);

	ImGui::Checkbox( "Auto scope", g_Settings->m_bRageAutoscope);
	ImGui::Checkbox( "Smooth animations", g_Settings->m_bSmoothAnimations);

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text(_S("Safe"));
	ImGui::Keybind(_S("Force safe"), &g_Settings->m_aSafePoint->m_iKeySelected, &g_Settings->m_aSafePoint->m_iModeSelected);

	ImGui::Text(_S("Damage"));
	ImGui::Keybind(_S("Damage override"), &g_Settings->m_aMinDamage->m_iKeySelected, &g_Settings->m_aMinDamage->m_iModeSelected);

	ImGui::Text(_S("DT"));
	ImGui::Keybind(_S("Double tap"), &g_Settings->m_aDoubleTap->m_iKeySelected, &g_Settings->m_aDoubleTap->m_iModeSelected);

	ImGui::Text(_S("HS"));
	ImGui::Keybind(_S("Hide shots"), &g_Settings->m_aHideShots->m_iKeySelected, &g_Settings->m_aHideShots->m_iModeSelected);

	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(315, 120));

	ImGui::BeginChild( "Weapon settings", ImVec2( iChildDoubleSizeX, 540 ) );
	
	ImGui::Spacing();

	ImGui::SingleSelect("Weapon", &wep, aRageWeapons);

	ImGui::Checkbox("Auto stop", &g_Settings->m_aRageSettings[wep].m_bAutoStop);

	InputSliderInt("Minimum damage", &g_Settings->m_aRageSettings[wep].m_iMinDamage, 0, 120);

	InputSliderInt("Override damage", &g_Settings->m_aRageSettings[wep].m_iMinDamageOverride, 0, 120);

	InputSliderInt( "Hitchance", &g_Settings->m_aRageSettings[ wep ].m_iHitChance, 0, 100 );

	InputSliderFloat( "DT cooldown", g_Settings->m_flRechargeTime, 0.f, 2.f );
	
	std::string strPreview = "";

	const std::vector < const char* > aDoubleTap =
	{
		"Move between shots",
		"Full stop"
	};
		
	ImGui::Spacing();

	if ( ImGui::BeginCombo( _S( "Doubletap" ), GetComboName(strPreview, 2, g_Settings->m_aRageSettings[wep].m_DoubleTapOptions), 0, aDoubleTap.size( ) ) )
	{
		for ( int i = 0; i < aDoubleTap.size( ); i++ )
			ImGui::Selectable( aDoubleTap[ i ], &g_Settings->m_aRageSettings[ wep ].m_DoubleTapOptions[ i ], ImGuiSelectableFlags_DontClosePopups );

		ImGui::EndCombo( );
	}

	const std::vector < const char* > aAutoStop =
	{
		"Force accuracy",
		"Predictive"
	};

	if ( ImGui::BeginCombo( _S( "Autostop" ), GetComboName(strPreview, aAutoStop.size(), g_Settings->m_aRageSettings[wep].m_AutoStopOptions), 0, aAutoStop.size( ) ) )
	{
		for ( int i = 0; i < aAutoStop.size( ); i++ )
			ImGui::Selectable( aAutoStop[ i ], &g_Settings->m_aRageSettings[ wep ].m_AutoStopOptions[ i ], ImGuiSelectableFlags_DontClosePopups );

		ImGui::EndCombo( );
		strPreview = "None";
	}

	if (ImGui::BeginCombo(_S("Hitboxes"), GetComboName(strPreview, aHitboxes.size(), g_Settings->m_aRageSettings[wep].m_Hitboxes), 0, aHitboxes.size()))
	{
		for (int i = 0; i < aHitboxes.size(); i++)
			ImGui::Selectable(aHitboxes[i], &g_Settings->m_aRageSettings[wep].m_Hitboxes[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo(_S("Unsafe hitboxes"), GetComboName(strPreview, aHitboxes.size(), g_Settings->m_aRageSettings[wep].m_SafeHitboxes), 0, aHitboxes.size()))
	{
		for (int i = 0; i < aHitboxes.size(); i++)
			ImGui::Selectable(aHitboxes[i], &g_Settings->m_aRageSettings[wep].m_SafeHitboxes[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo(_S("Modifiers"), GetComboName(strPreview, aModifiers.size(), g_Settings->m_aRageSettings[wep].m_RageModifiers), 0, aModifiers.size()))
	{
		for (int i = 0; i < aModifiers.size(); i++)
			ImGui::Selectable(aModifiers[i], &g_Settings->m_aRageSettings[wep].m_RageModifiers[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	if ( ImGui::BeginCombo(_S("Force safe"), GetComboName(strPreview, aSafeModifiers.size(), g_Settings->m_aRageSettings[wep].m_SafeModifiers), 0, aSafeModifiers.size()) )
	{
		for ( int i = 0; i < aSafeModifiers.size(); i++ )
			ImGui::Selectable(aSafeModifiers[i], &g_Settings->m_aRageSettings[wep].m_SafeModifiers[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo(_S("Multipoints"), GetComboName(strPreview, aHitboxes.size(), g_Settings->m_aRageSettings[wep].m_Multipoints), 0, aHitboxes.size()))
	{
		for (int i = 0; i < aHitboxes.size(); i++)
			ImGui::Selectable(aHitboxes[i], &g_Settings->m_aRageSettings[wep].m_Multipoints[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	InputSliderInt("Head scale", &g_Settings->m_aRageSettings[wep].m_iHeadScale, 0, 100);
	InputSliderInt("Body scale", &g_Settings->m_aRageSettings[wep].m_iBodyScale, 0, 100);
	InputSliderInt("Limb scale", &g_Settings->m_aRageSettings[wep].m_iLimbScale, 0, 100);

	ImGui::EndChild();
}

void C_Menu::DrawAntiAimTab( )
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();
	static int iAntiAimMode = 0;

	int iChildDoubleSizeX = 255;

	ImGui::SetCursorPos(ImVec2(33, 120));
	ImGui::BeginChild( _S( "Main" ), ImVec2( iChildDoubleSizeX, 395 ), ImGuiWindowFlags_NoScrollWithMouse );

	ImGui::Spacing(); ImGui::Spacing();

	ImGui::SetCursorPosX( ImGui::GetCursorPosX() - 10 );
	ImGui::Checkbox( _S( "Enable desync" ), g_Settings->m_bAntiAim ); 	
	ImGui::SetCursorPosX( ImGui::GetCursorPosX() - 10 );
	ImGui::SingleSelect(_S("Mode"), &iAntiAimMode, { _S("Stand"), _S("Walk"), _S("Slow walk, autostop"), _S("Air")}); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);

	ImGui::Checkbox(_S("Target rotation"), &g_Settings->m_aAntiAim[iAntiAimMode].m_bTargets); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	ImGui::Checkbox(_S("Randomize desync"), &g_Settings->m_aAntiAim[iAntiAimMode].m_bRandomizeDesync); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	ImGui::Checkbox(_S("Desync jitter"), &g_Settings->m_aAntiAim[iAntiAimMode].m_bDesyncJitter); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	ImGui::Checkbox(_S("Randomize jitter"), &g_Settings->m_aAntiAim[iAntiAimMode].m_bRandomizeJitter); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	InputSliderInt(_S("Jitter limit"), &g_Settings->m_aAntiAim[iAntiAimMode].m_iYawJitter, 0, 180); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	InputSliderInt( _S( "Right offset" ), &g_Settings->m_aAntiAim[iAntiAimMode].m_iYawOffset, 0, 180 );ImGui::SetCursorPosX( ImGui::GetCursorPosX() - 10 );
	InputSliderInt( _S( "Left offset" ), &g_Settings->m_aAntiAim[iAntiAimMode].m_iLeftYawOffset, 0, 180 );ImGui::SetCursorPosX( ImGui::GetCursorPosX() - 10 );
	InputSliderInt(_S("Right desync"), &g_Settings->m_aAntiAim[iAntiAimMode].m_iRightDesync, 0, 60); ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
	InputSliderInt( _S( "Left desync" ), &g_Settings->m_aAntiAim[iAntiAimMode].m_iLeftDesync, 0, 60 );ImGui::SetCursorPosX( ImGui::GetCursorPosX() - 10 );

	ImGui::EndChild( );
	ImGui::SetCursorPos(ImVec2(315, 120));

	ImGui::BeginChild( _S( "Other" ), ImVec2( iChildDoubleSizeX, 500 ) );

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::SingleSelect( _S( "Leg movement" ), g_Settings->m_iLegMovement, {_S( "Animate" ), _S( "Slide" )} );

	const std::vector < const char* > PitchModes = { _S( "None" ), _S( "Down" ), _S( "Up" ), _S( "Fake down" ), _S( "Fake up" ) };
	ImGui::SingleSelect( _S( "Pitch" ), g_Settings->m_iPitchMode, PitchModes );

	ImGui::Checkbox( _S( "Break legs" ), g_Settings->m_bJitterMove );

	InputSliderInt( _S( "Override SW" ), g_Settings->m_iSpeedWalk, 0, 100 );

	ImGui::Spacing(); ImGui::Spacing();

	ImGui::Checkbox( _S( "Fakelag" ), g_Settings->m_bFakeLagEnabled );
	
	InputSliderInt( _S( "Limit" ), g_Settings->m_iLagLimit, 1, 14 );

	const std::vector < std::string > aLagTriggers
	=
	{
		_S( "Move" ),
		_S( "Air" ),
		_S( "Peek" )
	};

	std::string strPreview = "";

	if ( ImGui::BeginCombo (_S( "Triggers" ), GetComboName(strPreview, aLagTriggers.size(), g_Settings->m_aFakelagTriggers), 0, aLagTriggers.size( ) ) )
	{
		for ( int i = 0; i < aLagTriggers.size( ); i++ )
			ImGui::Selectable( aLagTriggers[ i ].c_str( ), &g_Settings->m_aFakelagTriggers[ i ], ImGuiSelectableFlags_DontClosePopups );

		ImGui::EndCombo( );
	}

	InputSliderInt( _S( "Trigger limit" ), g_Settings->m_iTriggerLimit, 1, 14 );

	ImGui::Spacing(); ImGui::Spacing();
	
	ImGui::Text( _S( "Invert" ) );
	ImGui::Keybind( _S( "InvertButton" ), &g_Settings->m_aInverter->m_iKeySelected, &g_Settings->m_aInverter->m_iModeSelected );

	ImGui::Text( _S( "Left" ) );
	ImGui::Keybind( _S( "MLEFT" ), &g_Settings->m_aManualLeft->m_iKeySelected, &g_Settings->m_aManualLeft->m_iModeSelected );

	ImGui::Text( _S( "Back" ) );
	ImGui::Keybind( _S( "MBACK" ), &g_Settings->m_aManualBack->m_iKeySelected, &g_Settings->m_aManualBack->m_iModeSelected );

	ImGui::Text( _S( "Right" ) );
	ImGui::Keybind( _S( "MRIGHT" ), &g_Settings->m_aManualRight->m_iKeySelected, &g_Settings->m_aManualRight->m_iModeSelected );

	ImGui::Text( _S( "Slow walk" ) );
	ImGui::Keybind( _S( "SW" ), &g_Settings->m_aSlowwalk->m_iKeySelected, &g_Settings->m_aSlowwalk->m_iModeSelected );
	
	ImGui::Text( _S( "Override SW" ) );
	ImGui::Keybind( _S( "CSW" ), &g_Settings->m_aCSlowwalk->m_iKeySelected, &g_Settings->m_aCSlowwalk->m_iModeSelected );

	ImGui::Text( _S( "Fake duck" ) );
	ImGui::Keybind( _S( "FD" ), &g_Settings->m_aFakeDuck->m_iKeySelected, &g_Settings->m_aFakeDuck->m_iModeSelected );	
	
	ImGui::Text( _S( "Edge rotation" ) );
	ImGui::Keybind( _S( "Frstndk" ), &g_Settings->m_aFreestand->m_iKeySelected, &g_Settings->m_aFreestand->m_iModeSelected );

	ImGui::Text( _S( "Auto peek" ) );///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ImGui::Keybind( _S( "AP" ), &g_Settings->m_aAutoPeek->m_iKeySelected, &g_Settings->m_aAutoPeek->m_iModeSelected );

	ImGui::EndChild( );
}

#include "Tools/Tools.hpp"
#include "nSkinz/SkinChanger.h"
#include "nSkinz/ItemSchema.h"

void C_Menu::DrawInventoryTab()
{
	static int current_profile = -1;

	// hey stewen, what r u doing there? he, hm heee, DRUGS
	static bool drugs = false;

	// some animation logic(switch)
	static bool active_animation = false;
	static bool preview_reverse = false;
	static float switch_alpha = 1.f;
	static int next_id = -1;
	if (active_animation)
	{
		if (preview_reverse)
		{
			if (switch_alpha == 1.f) //-V550
			{
				preview_reverse = false;
				active_animation = false;
			}

			switch_alpha = ImClamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
		else
		{
			if (switch_alpha == 0.01f) //-V550
			{
				preview_reverse = true;
			}

			switch_alpha = ImClamp(switch_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
	}
	else
		switch_alpha = ImClamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);

	// add
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;

	int iChildSizeX = (800 - ImGui::CalcTextSize(this->GetMenuName()).x - 40 - 30);
	int iChildSizeY = (565 - 30);

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 15 + 40;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 40;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;
	ImGui::PopFont();
	// add

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));

	{
		{
			//child_title(current_profile == -1 ? _S("Skinchanger") : game_data::weapon_names[current_profile].name);
			// we need to count our items in 1 line
			auto same_line_counter = 0;

			// if we didnt choose any weapon
			if (current_profile == -1)
			{
				ImGui::SetCursorPos(ImVec2(10, 120));
				ImGui::BeginChild(_S("Inventory"), ImVec2(615, 540));

				for (auto i = 0; i < g_Settings->skins.skinChanger.size(); i++)
				{
					// do we need update our preview for some reasons?
					if (!all_skins[i])
					{
						g_Settings->skins.skinChanger.at(i).update();
						all_skins[i] = get_skin_preview(get_wep(i, (i == 0 || i == 1) ? g_Settings->skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_Settings->skins.skinChanger.at(i).skin_name, g_Globals.m_Interfaces.m_DirectDevice); //-V810
					}

					// we licked on weapon
					if (ImGui::ImageButton(all_skins[i], ImVec2(105, 76)))
					{
						next_id = i;
						active_animation = true;
					}

					// if our animation step is half from all - switch profile
					if (active_animation && preview_reverse)
					{
						ImGui::SetScrollY(0);
						current_profile = next_id;
					}

					if (same_line_counter < 4) { // continue push same-line
						ImGui::SameLine();
						same_line_counter++;
					}
					else { // we have maximum elements in 1 line
						same_line_counter = 0;
					}
				}

				ImGui::EndChild();
			}
			else
			{

				// update skin preview bool
				static bool need_update[36];

				// we pressed _S("Save & Close") button
				static bool leave;

				// update if we have nullptr texture or if we push force update
				if (!all_skins[current_profile] || need_update[current_profile])
				{
					all_skins[current_profile] = get_skin_preview(get_wep(current_profile, (current_profile == 0 || current_profile == 1) ? g_Settings->skins.skinChanger.at(current_profile).definition_override_vector_index : -1, current_profile == 0).c_str(), g_Settings->skins.skinChanger.at(current_profile).skin_name, g_Globals.m_Interfaces.m_DirectDevice); //-V810
					need_update[current_profile] = false;
				}

				// get settings for selected weapon
				auto& selected_entry = g_Settings->skins.skinChanger[current_profile];
				selected_entry.itemIdIndex = current_profile;

				// search input later
				static char search_skins[64] = "\0";
				static auto item_index = selected_entry.paint_kit_vector_index;
				bool bbBB = false;

				ImGui::SetCursorPos(ImVec2(33, 120));

				ImGui::Columns(2, nullptr, false);

				ImGui::BeginChild(_S("Weapons"), ImVec2(255, 500));
				{

					ImGui::BeginGroup();
					ImGui::PushItemWidth(230);

					if (!current_profile)
					{
						bbBB = true;
						//ImGui::SetCursorPosX(-40);//padding
						if (ImGui::Combo(_S("Knife"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
							{
								*out_text = game_data::knife_names[idx].name;
								return true;
							}, nullptr, IM_ARRAYSIZE(game_data::knife_names)))
							need_update[current_profile] = true; // push force update
					}
					else if (current_profile == 1)
					{
						bbBB = true;
						//ImGui::SetCursorPosX(-40);//padding
						if (ImGui::Combo(_S("Gloves"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
							{
								*out_text = game_data::glove_names[idx].name;
								return true;
							}, nullptr, IM_ARRAYSIZE(game_data::glove_names)))
						{
							item_index = 0; // set new generated paintkits element to 0;
							need_update[current_profile] = true; // push force update
						}
					}
					else
						selected_entry.definition_override_vector_index = 0;

					if (current_profile != 1)
					{
						if (bbBB)
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
						
						ImGui::Text(_S("Search"));
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
						if (ImGui::InputText(_S("##search"), search_skins, sizeof(search_skins)))
							item_index = -1;

					}

					auto main_kits = current_profile == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits;
					auto display_index = 0;

					SkinChanger::displayKits = main_kits;

					// we dont need custom gloves
					if (current_profile == 1)
					{
						for (auto i = 0; i < main_kits.size(); i++)
						{
							auto main_name = main_kits.at(i).name;

							for (auto i = 0; i < main_name.size(); i++)
								if (iswalpha((main_name.at(i))))
									main_name.at(i) = towlower(main_name.at(i));

							char search_name[64];

							if (!strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, _S("Hydra")))
								strcpy_s(search_name, sizeof(search_name), _S("Bloodhound"));
							else
								strcpy_s(search_name, sizeof(search_name), game_data::glove_names[selected_entry.definition_override_vector_index].name);

							for (auto i = 0; i < sizeof(search_name); i++)
								if (iswalpha(search_name[i]))
									search_name[i] = towlower(search_name[i]);

							if (main_name.find(search_name) != std::string::npos)
							{
								SkinChanger::displayKits.at(display_index) = main_kits.at(i);
								display_index++;
							}
						}

						SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
					}
					else
					{
						if (strcmp(search_skins, _S(""))) //-V526
						{
							for (auto i = 0; i < main_kits.size(); i++)
							{
								auto main_name = main_kits.at(i).name;

								for (auto i = 0; i < main_name.size(); i++)
									if (iswalpha(main_name.at(i)))
										main_name.at(i) = towlower(main_name.at(i));

								char search_name[64];
								strcpy_s(search_name, sizeof(search_name), search_skins);

								for (auto i = 0; i < sizeof(search_name); i++)
									if (iswalpha(search_name[i]))
										search_name[i] = towlower(search_name[i]);

								if (main_name.find(search_name) != std::string::npos)
								{
									SkinChanger::displayKits.at(display_index) = main_kits.at(i);
									display_index++;
								}
							}

							SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
						}
						else
							item_index = selected_entry.paint_kit_vector_index;
					}

					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
					
					m_bStopChildFrameTextOutput = 1;
					if (!SkinChanger::displayKits.empty())
					{
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
						if (ImGui::ListBox(_S("    Skin kit"), &item_index, [](void* data, int idx, const char** out_text) //-V107
							{
								while (SkinChanger::displayKits.at(idx).name.find(_S("ё")) != std::string::npos) //-V807
									SkinChanger::displayKits.at(idx).name.replace(SkinChanger::displayKits.at(idx).name.find(_S("ё")), 2, _S("е"));

								*out_text = SkinChanger::displayKits.at(idx).name.c_str(); 

								return true;
							}, nullptr, SkinChanger::displayKits.size(), SkinChanger::displayKits.size() > 9 ? 9 : SkinChanger::displayKits.size()) || !all_skins[current_profile])
						{
							SkinChanger::scheduleHudUpdate();
							need_update[current_profile] = true;

							auto i = 0;

							while (i < main_kits.size())
							{
								if (main_kits.at(i).id == SkinChanger::displayKits.at(item_index).id)
								{
									selected_entry.paint_kit_vector_index = i;
									break;
								}

								i++;
							}

						}
					}
					ImGui::PopStyleVar();
					m_bStopChildFrameTextOutput = 0;

					ImGui::PushItemWidth(190);

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
					ImGui::Text(_S("Seed"));
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
					if (ImGui::InputInt(_S("   Seed"), &selected_entry.seed, 1, 100))
						SkinChanger::scheduleHudUpdate();


					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
					ImGui::Text(_S("StatTrak"));
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
					if (ImGui::InputInt(_S("   StatTrak"), &selected_entry.stat_trak, 1, 15))
						SkinChanger::scheduleHudUpdate();
					ImGui::PopItemWidth();

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
					if (ImGui::Combo(_S("Quality"), &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = game_data::quality_names[idx].name;
							return true;
						}, nullptr, IM_ARRAYSIZE(game_data::quality_names)))
						SkinChanger::scheduleHudUpdate();

						//ImGui::SetCursorPosX(-20);//padding
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
						if (InputSliderFloat(_S("Wear"), &selected_entry.wear, 0.0f, 1.0f))
							drugs = true;
						else if (drugs)
						{
							SkinChanger::scheduleHudUpdate();
							drugs = false;
						}

						if (current_profile != 1)
						{
							if (!g_Settings->skins.custom_name_tag[current_profile].empty())
								strcpy_s(selected_entry.custom_name, sizeof(selected_entry.custom_name), g_Settings->skins.custom_name_tag[current_profile].c_str());

							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
							ImGui::Text(_S("Name Tag"));
							ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

							ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
							if (ImGui::InputText(_S("##nametag"), selected_entry.custom_name, sizeof(selected_entry.custom_name)))
							{
								g_Settings->skins.custom_name_tag[current_profile] = selected_entry.custom_name;
								SkinChanger::scheduleHudUpdate();
							}

							ImGui::PopStyleVar();
						}

						ImGui::PopItemWidth();

						ImGui::EndGroup();

				}
				ImGui::EndChild();

				ImGui::NextColumn();

				ImGui::SetCursorPos(ImVec2(315, 120));
				ImGui::BeginChild(_S("Preview"), ImVec2(255, 450));
				{

					ImGui::BeginGroup();
					if (ImGui::ImageButton(all_skins[current_profile], ImVec2(270, 155)))
					{
						// maybe i will do smth later where, who knows :/
					}

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10);
					if (ImGui::Button(_S("Update weapon"), ImVec2(255, 25)))
					{
						// start animation
						active_animation = true;
						next_id = -1;
						leave = true;		
					}

					ImGui::EndGroup();

				}ImGui::EndChild();

				// update element
				selected_entry.update();

				// we need to reset profile in the end to prevent render images with massive's index == -1
				if (leave && (preview_reverse || !active_animation))
				{
					ImGui::SetScrollY(0);
					current_profile = next_id;
					leave = false;
				}

			}
		}
	}
}

const char* GetModelName(bool bInvert = false)
{
	if ( bInvert )
	{
		if ( g_Globals.m_LocalPlayer->m_iTeamNum() == 2 )
		{
			return "a";
		}

		return _S("models/player/custom_player/legacy/tm_phoenix_variantf.mdl");	
	}

	if ( g_Globals.m_LocalPlayer->m_iTeamNum() == 2 )
	{
		return _S("models/player/custom_player/legacy/tm_phoenix_variantf.mdl");
	}

	return "a";	
}

void C_Menu::DrawPlayersTab()
{
	ImVec2 vecWindowPosition = ImVec2( ImGui::GetWindowPos().x - 150,  ImGui::GetWindowPos().y );

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iChildDoubleSizeX = 255;
	int iChildDoubleSizeY = 420;
	ImGui::PopFont();

	static int32_t iPlayerESPType	= 0;
	static int32_t iActiveSubTab	= 0;
	static int32_t iChamsGroup		= 0;
	
	static bool bShowESP = false;

	if ( !iActiveSubTab )
	{
		if ( iPlayerESPType == 1 )
		{	
			if ( g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
				m_cPlayerModel = GetModelName();
			else
				m_cPlayerModel = "models/player/custom_player/legacy/ctm_fbi_variantb.mdl";
		}
		else if ( iPlayerESPType == 2 )
		{
			iChildDoubleSizeY = 470;

			if ( g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
			{
				if ( g_Globals.m_LocalPlayer->m_iTeamNum() == 2 )
				{
					m_cPlayerModel = !g_Settings->m_nModelT ? _S("models/player/custom_player/legacy/tm_phoenix_variantf.mdl") : m_aPlayerModelList[g_Settings->m_nModelT];
				}
				else
				{
					m_cPlayerModel = !g_Settings->m_nModelCT ? "models/player/custom_player/legacy/ctm_fbi_variantb.mdl" : m_aPlayerModelList[g_Settings->m_nModelCT];
				}
			}
			else
			{
				m_cPlayerModel = "models/player/custom_player/legacy/ctm_fbi_variantb.mdl";
			}
		}
		else
		{
			if ( g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
				m_cPlayerModel = GetModelName(true);
			else
				m_cPlayerModel = "models/player/custom_player/legacy/ctm_fbi_variantb.mdl";
		}			
	}
	else
	{
		iChildDoubleSizeY = 240;

		if ( iChamsGroup < 4 )
		{
			if ( g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
				m_cPlayerModel = GetModelName(true);
			else
				m_cPlayerModel = "models/player/custom_player/legacy/ctm_fbi_variantb.mdl";
		}
		else if ( iChamsGroup < 6 )
		{
			if ( g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
				m_cPlayerModel = GetModelName();
			else
				m_cPlayerModel = "models/player/custom_player/legacy/ctm_fbi_variantb.mdl";
		}
		else
		{
			if ( g_Globals.m_Interfaces.m_EngineClient->IsInGame() )
			{
				if ( g_Globals.m_LocalPlayer->m_iTeamNum() == 2 )
				{
					m_cPlayerModel = !g_Settings->m_nModelT ? _S("models/player/custom_player/legacy/tm_phoenix_variantf.mdl") : m_aPlayerModelList[g_Settings->m_nModelT];
				}
				else
				{
					m_cPlayerModel = !g_Settings->m_nModelCT ? "models/player/custom_player/legacy/ctm_fbi_variantb.mdl" : m_aPlayerModelList[g_Settings->m_nModelCT];
				}
			}
			else
			{
				m_cPlayerModel = m_aPlayerModelList[g_Settings->m_nModelCT];
			}
		}
	}

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::SetCursorPos(ImVec2(33, 120));
	ImGui::BeginChild("Player", ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));
		
	ImGui::Spacing( );
	ImGui::Spacing( );

	std::vector< const char* > aSubTabList =
	{
		_S("ESP"), 
		_S("Chams")
	};

	ImGui::SingleSelect(_S("Main"), &iActiveSubTab, aSubTabList);

	static C_PlayerSettings* Settings = NULL;

	if ( !iActiveSubTab )
	{
		std::vector< const char* > aESPPlayer = {
			_S("Enemy"), 
			_S("Team"),
			_S("Local")
		};

		ImGui::SingleSelect(_S("Type"), &iPlayerESPType, aESPPlayer);

		switch ( iPlayerESPType )
		{
			case 0: Settings = g_Settings->m_Enemies; break;
			case 1: Settings = g_Settings->m_Teammates; break;
			case 2: Settings = g_Settings->m_LocalPlayer; break;
		}

		const char* aPlayerModelNameList[] =
		{
			"None",
			"Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman",
			"Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman",
			"Lieutenant Rex Krikey | SEAL Frogman",
			"Michael Syfers | FBI Sniper",
			"Operator | FBI SWAT",
			"Special Agent Ava | FBI",
			"Markus Delrow | FBI HRT",
			"Sous-Lieutenant Medic | Gendarmerie Nationale",
			"Chem-Haz Capitaine | Gendarmerie Nationale",
			"Chef d'Escadron Rouchard | Gendarmerie Nationale",
			"Aspirant | Gendarmerie Nationale",
			"Officer Jacques Beltram | Gendarmerie Nationale",
			"D Squadron Officer | NZSAS",
			"B Squadron Officer | SAS",
			"Seal Team 6 Soldier | NSWC SEAL",
			"Buckshot | NSWC SEAL",
			"Lt. Commander Ricksaw | NSWC SEAL",
			"Blueberries' Buckshot | NSWC SEAL",
			"3rd Commando Company | KSK",
			"Two Times' McCoy | TACP Cavalry",
			"Two Times' McCoy | USAF TACP",
			"Primeiro Tenente | Brazilian 1st Battalion",
			"Cmdr. Mae 'Dead Cold' Jamison | SWAT",
			"1st Lieutenant Farlow | SWAT",
			"John 'Van Healen' Kask | SWAT",
			"Bio-Haz Specialist | SWAT",
			"Sergeant Bombson | SWAT",
			"Chem-Haz Specialist | SWAT",
			"Lieutenant 'Tree Hugger' Farlow | SWAT",
			"Getaway Sally | The Professionals",
			"Number K | The Professionals",
			"Little Kev | The Professionals",
			"Safecracker Voltzmann | The Professionals",
			"Bloody Darryl The Strapped | The Professionals",
			"Sir Bloody Loudmouth Darryl | The Professionals",
			"Sir Bloody Darryl Royale | The Professionals",
			"Sir Bloody Skullhead Darryl | The Professionals",
			"Sir Bloody Silent Darryl | The Professionals",
			"Sir Bloody Miami Darryl | The Professionals",
			"Street Soldier | Phoenix",
			"Soldier | Phoenix",
			"Slingshot | Phoenix",
			"Enforcer | Phoenix",
			"Mr. Muhlik | Elite Crew",
			"Prof. Shahmat | Elite Crew",
			"Osiris | Elite Crew",
			"Ground Rebel | Elite Crew",
			"The Elite Mr. Muhlik | Elite Crew",
			"Trapper | Guerrilla Warfare",
			"Trapper Aggressor | Guerrilla Warfare",
			"Vypa Sista of the Revolution | Guerrilla Warfare",
			"Col. Mangos Dabisi | Guerrilla Warfare",
			"Medium Rare' Crasswater | Guerrilla Warfare",
			"Crasswater The Forgotten | Guerrilla Warfare",
			"Elite Trapper Solman | Guerrilla Warfare",
			"The Doctor' Romanov | Sabre",
			"Blackwolf | Sabre",
			"Maximus | Sabre",
			"Dragomir | Sabre",
			"Rezan The Ready | Sabre",
			"Rezan the Redshirt | Sabre",
			"Dragomir | Sabre Footsoldier"
		};

		if (iPlayerESPType == 2)
		{
			ImGui::Combo(_S("T model"), g_Settings->m_nModelT.GetPtr(), aPlayerModelNameList, ARRAYSIZE(aPlayerModelNameList));
			ImGui::Combo(_S("CT model"), g_Settings->m_nModelCT.GetPtr(), aPlayerModelNameList, ARRAYSIZE(aPlayerModelNameList));
		}

		ImGui::Checkbox(_S("Name"), &Settings->m_RenderName);
		this->DrawColorEdit4(_S("Name##color"), &Settings->m_aNameColor);

		ImGui::Checkbox(_S("Box"), &Settings->m_BoundaryBox);
		this->DrawColorEdit4(_S("Box##color"), &Settings->m_aBoundaryBox);

		ImGui::Checkbox(_S("Health"), &Settings->m_RenderHealthBar);
		this->DrawColorEdit4(_S("m_aHealth##color"), &Settings->m_aHealthBar);

		ImGui::Checkbox(_S("Health bar text"), &Settings->m_RenderHealthText);
		this->DrawColorEdit4(_S("m_aHealthText##color"), &Settings->m_aHealthText);

		ImGui::Checkbox(_S("Ammo bar"), &Settings->m_RenderAmmoBar);
		this->DrawColorEdit4(_S("m_aAmmoBar##color"), &Settings->m_aAmmoBar);

		ImGui::Checkbox(_S("Ammo bar text"), &Settings->m_RenderAmmoBarText);
		this->DrawColorEdit4(_S("m_aAmmoBarText##color"), &Settings->m_aAmmoBarText);

		ImGui::Checkbox(_S("Weapon Text"), &Settings->m_RenderWeaponText);
		this->DrawColorEdit4(_S("m_aWeaponText##color"), &Settings->m_aWeaponText);

		ImGui::Checkbox(_S("Weapon Icon"), &Settings->m_RenderWeaponIcon);
		this->DrawColorEdit4(_S("m_aWeaponIcon##color"), &Settings->m_aWeaponIcon);
	
		ImGui::Checkbox(_S("Out of view arrows"), g_Settings->m_bOutOfViewArrows);
		this->DrawColorEdit4(_S("awerqweqw2e123412er412q4##color"), g_Settings->m_aOutOfViewArrows);

		const char* aFlags[ 5 ] 
		=
		{
			"Scoped",
			"Armor",
			"Flashed",
			"Location",
			"Money"
		};

		for ( int i = 0; i < 5; i++ )
		{
			ImGui::Checkbox( aFlags [ i ], &Settings->m_Flags[ i ] );
			this->DrawColorEdit4( ( "##" + std::to_string( i ) ).c_str( ), &Settings->m_Colors[ i ] );
		}

		ImGui::Checkbox( _S( "Glow" ), &Settings->m_bRenderGlow );
		this->DrawColorEdit4( _S( "##m_aGlowcolor" ), &Settings->m_aGlow );
		ImGui::SingleSelect( _S( "Glow style##1" ), &Settings->m_iGlowStyle, { _S( "Outline" ), _S( "Thin" ), _S( "Cover" ), _S( "Cover Pulse" ) } );
	
		ImGui::EndChild();
	}
	else
	{

		const char* aChamsGroup[]= 
		{	
			("Enemy visible"), 
			("Enemy invisble"), 
			("Backtrack"), 
			("Shot chams"), 
			("Team visible"), 
			("Team invisible"), 
			("Local"), 
			("Desync"), 
			("Lag") 
		};

		//static int32_t iChamsGroup = 0;
		ImGui::Combo(_S("Chams group"), &iChamsGroup, aChamsGroup, ARRAYSIZE(aChamsGroup));

		ImGui::Checkbox(_S("Enable chams"), &g_Settings->m_aChamsSettings[iChamsGroup].m_bRenderChams);
		this->DrawColorEdit4(_S("##qweqwe"), &g_Settings->m_aChamsSettings[iChamsGroup].m_Color);
		ImGui::SingleSelect(_S("Material"), &g_Settings->m_aChamsSettings[iChamsGroup].m_iMainMaterial, { _S("Flat"), _S("Regular") });

		ImGui::Checkbox(_S("Add glow"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[0]);
		this->DrawColorEdit4(_S("##512414 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[0]);
		ImGui::Checkbox(_S("Add ghost"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[1]);
		this->DrawColorEdit4(_S("##235235 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[1]);
		ImGui::Checkbox(_S("Add glass"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[2]);
		this->DrawColorEdit4(_S("##4124124 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[2]);
		ImGui::Checkbox(_S("Add pulsation"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiers[3]);
		this->DrawColorEdit4(_S("##123123 color"), &g_Settings->m_aChamsSettings[iChamsGroup].m_aModifiersColors[3]);
		ImGui::Checkbox(_S("Add ragdolls"), g_Settings->m_bDrawRagdolls);

		if ( bShowESP )
			switch ( iChamsGroup )
			{
			case 0:
				Settings = g_Settings->m_Enemies;
				break;
			case 1:
				Settings = g_Settings->m_Enemies;
				break;
			case 4:
				Settings = g_Settings->m_Teammates;
				break;
			case 5:
				Settings = g_Settings->m_Teammates;
				break;
			case 6:
				Settings = g_Settings->m_LocalPlayer;
				break;
			default:
				Settings = NULL;
				break;
			}

		ImGui::EndChild();

		m_bStopChildFrameTextOutput = 2;

		ImGui::SetCursorPos(ImVec2(750, 516));
		ImGui::BeginChild("Preview settings", ImVec2(190, 25));
		ImGui::Checkbox(_S("Preview ESP"), &bShowESP);
		ImGui::EndChild();

		m_bStopChildFrameTextOutput = 0;
	}

	// PREVIEW.
		
	// fix alligns
	vecWindowPosition = ImVec2(vecWindowPosition.x - 340, vecWindowPosition.y + 80);

	if ( iActiveSubTab )
	{
		if ( iChamsGroup < 7 )
		{
			g_DrawModel->SetGlow(Settings->m_iGlowStyle);

			if ( !Settings->m_bRenderGlow )
				g_DrawModel->SetGlow(-1);
			else
				g_DrawModel->SetGlowColor(Color(Settings->m_aGlow));
		}

		if ( iChamsGroup < 8 )
			g_DrawModel->SetChamsSettings(g_Settings->m_aChamsSettings[iChamsGroup]);

		/*ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 654 + 150, vecWindowPosition.y + 2), ImVec2(vecWindowPosition.x + 1100, vecWindowPosition.y + 440), ImColor(10, 10, 10, 250), 0.f);
		ImGui::GetOverlayDrawList()->AddRectFilledMultiColor(ImVec2(vecWindowPosition.x + 654 + 150, vecWindowPosition.y + 2), ImVec2(vecWindowPosition.x + 1100, vecWindowPosition.y + 320),
			ImColor(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 220 / 255.f),
			ImColor(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 220 / 255.f),
			ImColor(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 10 / 255.f),
			ImColor(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 10 / 255.f)
		);
		ImGui::PushFont(g_Globals.m_Fonts.m_TitleFont);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 655 + 146, vecWindowPosition.y - 15), ImColor(20, 20, 20), _S("PREVIEW"));
		ImGui::PopFont();
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 654 + 150, vecWindowPosition.y + 2), ImVec2(vecWindowPosition.x + 1100, vecWindowPosition.y + 4), ImColor(g_Settings->m_colMenuTheme->r()/ 255.f, g_Settings->m_colMenuTheme->g()/ 255.f, g_Settings->m_colMenuTheme->b() / 255.f), 0.f);*/

		if ( g_DrawModel->GetTexture() )
		{
			ImGui::GetForegroundDrawList()->AddImage(
				g_DrawModel->GetTexture()->pTextureHandles[0]->lpRawTexture,
				ImVec2(vecWindowPosition.x + 610 + 150, vecWindowPosition.y - 130),
				ImVec2(vecWindowPosition.x + 610 + 150 + g_DrawModel->GetTexture()->GetActualWidth(), vecWindowPosition.y + g_DrawModel->GetTexture()->GetActualHeight() - 130),
				ImVec2(0, 0), ImVec2(1, 1),
				ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	static ImVec2 vecPreviousMousePosition = ImVec2(0, 0);
	static ImVec2 vecLastMousePosition = ImVec2(0, 0);
	ImVec2 vecCurrentCursorPosition = ImGui::GetMousePos();

	static bool bIsActive = false;

	if ( iActiveSubTab && !bShowESP )
		return;

	if ( !Settings )
		return;

	// render box
	Color aBox = Color(Settings->m_aBoundaryBox);
	if (Settings->m_BoundaryBox)
	{
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 694 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 886 + 170, vecWindowPosition.y + 386), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 696 + 150, vecWindowPosition.y + 41), ImVec2(vecWindowPosition.x + 884 + 170, vecWindowPosition.y + 384), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 885 + 170, vecWindowPosition.y + 385), ImColor(aBox.r(), aBox.g(), aBox.b(), aBox.a()));
	}

	// flags.
	{
		const ImVec2 BBox = ImVec2(vecWindowPosition.x + 886 + 170, vecWindowPosition.y + 39);
		int32_t iOffset = 0;

		if ( Settings->m_Flags[0] )
		{
			g_Render->RenderText(_S("SCOPED"), ImVec2(BBox.x + 3, BBox.y + iOffset), Settings->m_Colors[0], false, true, g_Globals.m_Fonts.m_Flags, true);
			iOffset += 12;
		}

		if ( Settings->m_Flags[1] )
		{
			g_Render->RenderText(_S("HK/K"), ImVec2(BBox.x + 3, BBox.y + iOffset), Settings->m_Colors[1], false, true, g_Globals.m_Fonts.m_Flags, true);
			iOffset += 12;
		}

		if ( Settings->m_Flags[2] )
		{
			g_Render->RenderText(_S("FLASHED"), ImVec2(BBox.x + 3, BBox.y + iOffset), Settings->m_Colors[2], false, true, g_Globals.m_Fonts.m_Flags, true);
			iOffset += 12;
		}

		if ( Settings->m_Flags[3] )
		{
			g_Render->RenderText(_S("PLACE"), ImVec2(BBox.x + 3, BBox.y + iOffset), Settings->m_Colors[3], false, true, g_Globals.m_Fonts.m_Flags, true);
			iOffset += 12;
		}

		if ( Settings->m_Flags[4] )
		{
			g_Render->RenderText(_S("9999$"), ImVec2(BBox.x + 3, BBox.y + iOffset), Settings->m_Colors[4], false, true, g_Globals.m_Fonts.m_Flags, true);
			iOffset += 12;
		}
	}

	// render name
	Color aName = Color(Settings->m_aNameColor);
	if (Settings->m_RenderName)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("Player")).x / 2, vecWindowPosition.y + 22), ImColor(aName.r(), aName.g(), aName.b(), aName.a()), _S("Player"));
		ImGui::PopFont();
	}

	// render health
	Color aHealthBar = Color(Settings->m_aHealthBar);
	if (Settings->m_RenderHealthBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 689 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 693 + 150, vecWindowPosition.y + 385), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 690 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 692 + 150, vecWindowPosition.y + 385), ImColor(aHealthBar.r(), aHealthBar.g(), aHealthBar.b(), aHealthBar.a()));
	}

	Color aHealthText = Color(Settings->m_aHealthText);
	if (Settings->m_RenderHealthText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);

		if (Settings->m_RenderHealthBar)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 150 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37 ), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 691 + 150 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37 ), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));

		ImGui::PopFont();
	}

	Color aWeaponText = Color(Settings->m_aWeaponText);
	if (Settings->m_RenderWeaponText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("P2000")).x / 2, vecWindowPosition.y + 385 + 6), ImColor(aWeaponText.r(), aWeaponText.g(), aWeaponText.b(), aWeaponText.a()), _S("P2000"));
		ImGui::PopFont();
	}

	Color aWeaponIcon = Color(Settings->m_aWeaponIcon);
	if (Settings->m_RenderWeaponIcon)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_WeaponIcon);

		if (Settings->m_RenderWeaponText)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("E")).x / 2, vecWindowPosition.y + 385 + 22), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("E"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("E")).x / 2, vecWindowPosition.y + 385 + 8), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("E"));

		ImGui::PopFont();
	}

	Color aAmmoBar = Color(Settings->m_aAmmoBar);
	if (Settings->m_RenderAmmoBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 387), ImVec2(vecWindowPosition.x + 906 + 150, vecWindowPosition.y + 391), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 388), ImVec2(vecWindowPosition.x + 905 + 150, vecWindowPosition.y + 390), ImColor(aAmmoBar.r(), aAmmoBar.g(), aAmmoBar.b(), aAmmoBar.a()));
	}

	Color aAmmoText = Color(Settings->m_aAmmoBarText);
	if (Settings->m_RenderAmmoBarText)
	{
		ImGui::PushFont(g_Globals.m_Fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 150 + ImGui::CalcTextSize(_S("13")).x / 2, vecWindowPosition.y + 386), ImColor(aAmmoText.r(), aAmmoText.g(), aAmmoText.b(), aAmmoText.a()), _S("13"));
		ImGui::PopFont();
	}
}

static int Selected = 0;

void C_Menu::DrawMiscTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iChildDoubleSizeX = 255;
	int iChildDoubleSizeY = 363;

	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(33, 120));
	ImGui::BeginChild(_S("Movement"), ImVec2(iChildDoubleSizeX, 500));

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox(_S("Auto jump"), g_Settings->m_bBunnyHop);

	ImGui::Checkbox(_S("Legit strafe"), g_Settings->m_bAutoStrafe);

	if ( g_Settings->m_bWASDStrafe && g_Settings->m_bAutoStrafe)
		g_Settings->m_bWASDStrafe.Get() = false;

	ImGui::Checkbox(_S("Rage strafe"), g_Settings->m_bWASDStrafe);

	if ( g_Settings->m_bWASDStrafe && g_Settings->m_bAutoStrafe)
		g_Settings->m_bAutoStrafe.Get() = false;

	ImGui::Checkbox(_S("Start speed"), g_Settings->m_bSpeedBoost);
	ImGui::Checkbox(_S("Quick stop"), g_Settings->m_bFastStop);
	ImGui::Checkbox(_S("Edge jump"), g_Settings->m_bEdgeJump);
	ImGui::Checkbox(_S("Infinity duck"), g_Settings->m_bInfinityDuck);

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox( _S( "Buybot" ), g_Settings->m_bBuyBotEnabled);

	ImGui::Checkbox(_S("Force/save AWP"), g_Settings->m_bBuyBotKeepAWP);

	std::vector < std::string > aEquipment
		=
	{
	_S("Molotov"),
	_S("Smoke"),
	_S("Flashbang"),
	_S("Explosive grenade"),
	_S("Taser"),
	_S("Heavy armor"),
	_S("Helmet"),
	_S("Defuse kit")
	};

	ImGui::SingleSelect
	(
		_S("Primary"),
		g_Settings->m_BuyBotPrimaryWeapon,
		{
			_S("None"),
			_S("SCAR20/G3SG1"),
			_S("Scout"),
			_S("AWP"),
			_S("M4A1/AK47")
		}
	);
	ImGui::SingleSelect(
		_S("Secondary"),
		g_Settings->m_BuyBotSecondaryWeapon,
		{
			_S("None"),
			_S("FN57/TEC9"),
			_S("Dual elites"),
			_S("Deagle/Revolver"),
			_S("P2000/Glock-18"),
			_S("P250")
		}
	);

	std::string strPreview = "";

	if (ImGui::BeginCombo(_S("Equipment"), GetComboName(strPreview, aEquipment.size(), g_Settings->m_aEquipment), 0, 8))
	{
		for (int i = 0; i < aEquipment.size(); i++)
			ImGui::Selectable(aEquipment[i].c_str(), &g_Settings->m_aEquipment[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	};

	ImGui::EndChild( );

	ImGui::SetCursorPos(ImVec2(315, 120));

	ImGui::BeginChild(_S("Other"), ImVec2(iChildDoubleSizeX, 500));

	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox(_S("Anti Untrusted"), g_Settings->m_bAntiUntrusted);

	std::vector < std::string > aLogItems = { _S( "Hurt" ), _S( "Harm" ), _S( "Purchase" ), _S( "Bomb" ), _S( "Miss") };
	if (ImGui::BeginCombo(_S("Event logs"), GetComboName(strPreview, aLogItems.size(), g_Settings->m_bLogHurts), ImGuiComboFlags_HeightSmall, aLogItems.size()))
	{
		ImGui::Selectable(aLogItems[0].c_str(), g_Settings->m_bLogHurts.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[1].c_str(), g_Settings->m_bLogHarms.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[2].c_str(), g_Settings->m_bLogPurchases.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[3].c_str(), g_Settings->m_bLogBomb.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
		ImGui::Selectable(aLogItems[4].c_str(), g_Settings->m_bLogMisses.GetPtr(), ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	ImGui::Checkbox( _S( "Regulate server ads" ), g_Settings->m_bAdBlock );
	ImGui::Checkbox( _S( "Regulate console" ), g_Settings->m_bFilterConsole );
	ImGui::Checkbox( _S( "Display ranks" ), g_Settings->m_bRevealRanks );
	ImGui::Checkbox( _S( "Display spectators" ), g_Settings->m_bSpectatorList );
	ImGui::Checkbox( _S( "Display keybinds" ), g_Settings->m_bDrawKeyBindList );
	ImGui::Checkbox( _S( "Keep killfeed" ), g_Settings->m_bPreserveKillfeed );
	ImGui::Checkbox( _S( "Unlock inventory" ), g_Settings->m_bUnlockInventoryAccess );
	ImGui::Checkbox( _S( "Unlock convars" ), g_Settings->m_bUnhideConvars );
	
#ifdef DEV_BUILD
	ImGui::Checkbox( _S( "Watermark" ), g_Settings->m_bWaterMark );
	ImGui::Checkbox( _S( "Clantag" ), g_Settings->m_bTagChanger );
#endif

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text(_S("Fake ping"));
	ImGui::Keybind(_S("Override ping"), &g_Settings->m_aPingSpike->m_iKeySelected, &g_Settings->m_aPingSpike->m_iModeSelected);

	InputSliderInt(_S("Ping"), g_Settings->m_iWantedPing, 5, 200);
	ImGui::EndChild();

	/*ImGui::SetCursorPos(ImVec2(33, 260));
	ImGui::BeginChild(_S("Radio"), ImVec2(iChildDoubleSizeX, 100));

	ImGui::SingleSelect(_S("Radio Stations"), g_Settings->m_iRadioFrequence, 
		{ 
			"None" , 
			"Greatest Hits", 
			"Dance Hits", 
			"German Rap", 
			"Chill", 
			"Top 100", 
			"Best German-Rap", 
			"Hip Hop",
			"Mainstage",
			"Club",
			"Remixes",
			"Ambient",
			"Top 100 Remixes",
			"Hardstyle",
			"Monstercat",
			"Oldschool",
			"Summervibes" 
		}
	);
	InputSliderInt(_S("Volume"), g_Settings->m_iRadioVolume, 0, 100);
	ImGui::Text("%s", BASS::bass_metadata);

	ImGui::EndChild();*/
}

void C_Menu::DrawViewTab( )
{
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iChildDoubleSizeX = 255;
	int iChildDoubleSizeY = 363;
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(33, 120));
	ImGui::BeginChild(_S("View settings"), ImVec2(iChildDoubleSizeX, 400));

	ImGui::Spacing();ImGui::Spacing();
	ImGui::Text(_S("Thirdperson"));
	ImGui::Keybind(_S("ThirdPerson Bind"), &g_Settings->m_aThirdPerson->m_iKeySelected, &g_Settings->m_aThirdPerson->m_iModeSelected);
	ImGui::Spacing();
	ImGui::Checkbox(_S("Force distance while scoped"), g_Settings->m_bOverrideFOVWhileScoped);
	ImGui::Checkbox(_S("Scope model"), g_Settings->m_bViewmodelInScope);
	ImGui::Checkbox(_S("Single scope"), g_Settings->m_bSingleZoom);
	InputSliderInt( _S( "Distance" ), g_Settings->m_iThirdPersonDistance, 50, 255 );
	InputSliderInt( _S( "Field of view" ), g_Settings->m_iCameraDistance, 90, 140 );
	InputSliderFloat( _S( "Aspect ratio" ), g_Settings->m_flAspectRatio, 0.01f, 3.0f );

	ImGui::Spacing(); 
	ImGui::Spacing();
	
	ImGui::Checkbox(_S("Scope"), g_Settings->m_bCustomCrosshair);
	m_bFixColorSpacing = true;
	ImGui::Text(_S("First color"));
	this->DrawColorEdit4(_S("##12crsclr"), g_Settings->m_colCrosshair);
	ImGui::Text(_S("End color"));
	this->DrawColorEdit4(_S("##12crscslr"), g_Settings->m_colCrosshairS);
	m_bFixColorSpacing = false;
	InputSliderInt(_S("Axis length"), g_Settings->m_iCrosshairAxisLength, 2, 2000);
	InputSliderInt(_S("Axis spacing"), g_Settings->m_iCrosshairAxisSpacing, 2, 2000);
	InputSliderInt(_S("Axis fill"), g_Settings->m_iCrosshairAxisFill, 2, 15);

	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(315, 120));
	ImGui::BeginChild (_S( "Viewmodel" ), ImVec2( iChildDoubleSizeX, 400 ) );
	ImGui::Spacing();ImGui::Spacing();
	ImGui::Checkbox(_S("Lock movement"), g_Settings->m_bVMMovement);
	InputSliderInt( _S( "Viewmodel distance" ), g_Settings->m_iViewmodelDistance, 60, 140 );
	InputSliderInt( _S( "Viewmodel X axis" ), g_Settings->m_iViewmodelX, -20, 20 );
	InputSliderInt( _S( "Viewmodel Y axis" ), g_Settings->m_iViewmodelY, -20, 20 );
	InputSliderInt( _S( "Viewmodel Z axis" ), g_Settings->m_iViewmodelZ, -20, 20 );
	InputSliderInt( _S( "Viewmodel roll" ), g_Settings->m_iViewmodelRoll, -90, 90 );

	ImGui::Spacing();ImGui::Spacing();
	static int current_model = 0;
	ImGui::SingleSelect(_S("Model"), &current_model, { _S("Arms"), _S("Weapon"), _S("Attachments")});
	ImGui::Checkbox( _S( "Enable Chams" ), &g_Settings->m_aChamsSettings[current_model + 9].m_bRenderChams );
	this->DrawColorEdit4( _S( "##qweqwe" ), &g_Settings->m_aChamsSettings[current_model + 9].m_Color );
	ImGui::SingleSelect( _S( "Material" ), &g_Settings->m_aChamsSettings[current_model + 9].m_iMainMaterial, { _S( "Flat" ), _S( "Regular" ) } );
	ImGui::Checkbox( _S( "Add glow" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiers[ 0 ] );
	this->DrawColorEdit4( _S( "##512414 color" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiersColors[ 0 ] );
	ImGui::Checkbox( _S( "Add ghost" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiers[ 1 ] );
	this->DrawColorEdit4( _S( "##235235 color" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiersColors[ 1 ] );
	ImGui::Checkbox( _S( "Add glass" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiers[ 2 ] );
	this->DrawColorEdit4( _S( "##4124124 color" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiersColors[ 2 ] );
	ImGui::Checkbox( _S( "Add pulsation" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiers[ 3 ] );
	this->DrawColorEdit4( _S( "##123123 color" ), &g_Settings->m_aChamsSettings[current_model + 9].m_aModifiersColors[ 3 ] );
	//ImGui::SingleSelect(_S("Wireframe"), &g_Settings->m_aChamsSettings[current_model + 9].m_iWireframeType, { _S("None"), _S("Regular"), _S("Animated") });
	//ImGui::SingleSelect(_S("Wireframe material"), &g_Settings->m_aChamsSettings[current_model + 9].m_iWireframeMaterial, { _S("Flat"), _S("Regular"), _S("Glow") });
	//this->DrawColorEdit4( _S( "Wireframe color" ), &g_Settings->m_aChamsSettings[current_model + 9].m_WireframeColor );
	ImGui::EndChild();
}

void C_Menu::DrawWorldTab( )
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iChildDoubleSizeX = 255;
	int iChildDoubleSizeY = 380;
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(33, 120));
	ImGui::BeginChild (_S( "World" ), ImVec2( iChildDoubleSizeX, 335 ) );
	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox( _S( "Hold fire animation" ), g_Settings->m_bHoldFireAnimation );
	ImGui::SingleSelect( _S( "Skybox Changer" ), g_Settings->m_iSkybox.GetPtr( ), {
		_S("None"),
		_S("Tibet"),
		_S("Baggage"),
		_S("Italy"),
		_S("Aztec"),
		_S("Vertigo"),
		_S("Daylight"),
		_S("Daylight 2"),
		_S("Clouds"),
		_S("Clouds 2"),
		_S("Gray"),
		_S("Clear"),
		_S("Canals"),
		_S("Cobblestone"),
		_S("Assault"),
		_S("Clouds dark"),
		_S("Night"),
		_S("Night 2"),
		_S("Night flat"),
		_S("Dusty"),
		_S("Rainy"),
		//_S( "Custom" )
		} );

	m_bFixColorSpacing = true;

	ImGui::Text( _S( "World color" ) );
	this->DrawColorEdit4( _S( "##123123" ), g_Settings->m_WorldModulation );

	ImGui::Text( _S( "Props color" ) );
	this->DrawColorEdit4( _S( "##11233" ), g_Settings->m_PropModulation );

	ImGui::Text( _S( "Skybox color" ) );
	this->DrawColorEdit4( _S( "##51223" ), g_Settings->m_SkyModulation );

	m_bFixColorSpacing = false;

	ImGui::Spacing( );

	std::vector < const char* > aHitSounds =
	{
		"Metallic",
		"Bell"
	};

	std::vector < std::string > aWorldRemovals =
	{
		_S( "Visual punch" ),
		_S( "Visual kick" ),
		_S( "Scope" ),
		_S( "Smoke" ),
		_S( "Flash" ),
		_S( "Post process" ),
		_S( "Fog" ),
		_S( "Shadows" ),
		_S( "Landing bob" ),
		_S( "Hand shaking" )
	};

	ImGui::Checkbox( _S( "Grenade prediction" ), g_Settings->m_bPredictGrenades );
	this->DrawColorEdit4( _S( "##1234142124" ), g_Settings->m_GrenadeWarning );

	ImGui::Checkbox(_S("Grenade trajectory"), g_Settings->m_bGrenadeTrajectory );
	this->DrawColorEdit4(_S("##1234148154"), g_Settings->m_colGrenadeTrajectory );

	ImGui::Checkbox( _S( "Grenade timers" ), g_Settings->m_GrenadeTimers );
	this->DrawColorEdit4( _S( "##1234145151" ), g_Settings->m_GrenadeWarningTimer );

	std::string strPreview = "";
	if (ImGui::BeginCombo(_S("World removals"), GetComboName(strPreview, aWorldRemovals.size(), g_Settings->m_aWorldRemovals), 0, 8))
	{
		for (int i = 0; i < aWorldRemovals.size(); i++)
			ImGui::Selectable(aWorldRemovals[i].c_str(), &g_Settings->m_aWorldRemovals[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndCombo();
	}

	ImGui::Checkbox(_S("Modulation"), g_Settings->m_bAmbientModulation);
	InputSliderFloat(_S("Bloom"), g_Settings->m_flBloom, 0, 16, "%.1f");
	InputSliderFloat(_S("Exposure"), g_Settings->m_flExposure, 0, 20, "%.1f");
	InputSliderFloat(_S("Ambient"), g_Settings->m_flAmbient, 0, 15, "%.1f");

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Checkbox( _S( "Fog" ), g_Settings->m_bFog );
	this->DrawColorEdit4( _S( "##123as4" ), g_Settings->m_bFogColor );
	InputSliderInt("Distance", g_Settings->m_iFogDistance, 0, 2500);
	InputSliderInt("Density", g_Settings->m_iFogDensity, 0, 100);

	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(315, 120));

	ImGui::BeginChild(_S("Other"), ImVec2( iChildDoubleSizeX, 500 ));
	
	ImGui::Spacing( );
	ImGui::Spacing( );

	ImGui::Checkbox(_S("Velocity graph"), g_Settings->m_bVelocityGraph);
	this->DrawColorEdit4(_S("##velgraph73128"), g_Settings->m_VelocityGraphCol);
	ImGui::Checkbox( _S( "Penetration crosshair" ), g_Settings->m_bPenetrationCrosshair );
	ImGui::Checkbox( _S( "Force crosshair" ), g_Settings->m_bForceCrosshair );
	ImGui::Checkbox( _S( "On-hit marker" ), g_Settings->m_bHitMarker );
	this->DrawColorEdit4(_S("##hitmarkercol"), g_Settings->m_HitmarkerColor);
	ImGui::Checkbox( _S( "On-hit sound" ), g_Settings->m_bHitSound );
	ImGui::SingleSelect(_S("Sound"), g_Settings->m_nHitSound, aHitSounds);
	ImGui::Checkbox( _S( "On-hit effect" ), g_Settings->m_bEffectMarker );
	ImGui::Checkbox( _S( "On-hit damage" ), g_Settings->m_bDamageMarker);
	this->DrawColorEdit4(_S("##onhitdmg54"), g_Settings->m_OnHitDamage);

	ImGui::Checkbox(_S("Client bullet impacts"), g_Settings->m_bDrawClientImpacts);
	this->DrawColorEdit4( _S( "##41242354" ), g_Settings->m_ClientImpacts );
	ImGui::Checkbox( _S( "Server bullet impacts" ), g_Settings->m_bDrawServerImpacts );
	this->DrawColorEdit4( _S( "##412423154" ), g_Settings->m_ServerImpacts );

	ImGui::Checkbox( _S( "Local tracers" ), g_Settings->m_bDrawLocalTracers );
	this->DrawColorEdit4( _S( "##s78ads2" ), g_Settings->m_LocalTracers );
	ImGui::Checkbox( _S( "Enemy tracers" ), g_Settings->m_bDrawEnemyTracers );
	this->DrawColorEdit4( _S( "##sad987q" ), g_Settings->m_EnemyTracers ); 
	ImGui::SingleSelect(_S("Tracer type"), g_Settings->m_TracerType, {
		_S("Blue laser"),
		_S("Pu$$y smoke"),
		_S("I was drunk on this one"),
		_S("Beamboozled"),
		_S("Drunk glow"),
		_S("Lasserissimo"),
		_S("O'Radio"),
		});
	InputSliderFloat(_S("Tracer width"), g_Settings->m_TracerWidth, 0.f, 1.f, "%.2f", 0.01f);

	ImGui::Checkbox(_S("Motion blur"), g_Settings->m_bMotionBlur);
	ImGui::Checkbox(_S("Forward blur"), g_Settings->m_bForwardBlur);
	InputSliderFloat(_S("Blur weight"), g_Settings->m_flBlurStrength, 0.f, 8.f, "%.2f", 0.01f);
	InputSliderFloat(_S("Falling blur min"), g_Settings->m_flFallingMin, 0.f, 50.f, "%.2f", 0.01f);
	InputSliderFloat(_S("Falling blur max"), g_Settings->m_flFallingMax, 0.f, 50.f, "%.2f", 0.01f);
	InputSliderFloat(_S("Falling weight"), g_Settings->m_flFallingIntensitiy, 0.f, 8.f, "%.2f", 0.01f);
	InputSliderFloat(_S("Rotation weight"), g_Settings->m_flRotationIntensitiy, 0.f, 8.f, "%.2f", 0.01f);

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Checkbox(_S("Bomb"), g_Settings->m_bRenderC4Glow);
	this->DrawColorEdit4(_S("##C4Glowcolor"), g_Settings->m_aC4Glow);
	ImGui::SingleSelect(_S("Glow style##m_iC4GlowStyle"), g_Settings->m_iC4GlowStyle, { _S("Outline"), _S("Thin"), _S("Cover"), _S("Cover Pulse") });

	ImGui::Checkbox(_S("Dropped weapons"), g_Settings->m_bRenderDroppedWeaponGlow);
	this->DrawColorEdit4(_S("m_aDroppedWeaponGlow##color"), g_Settings->m_aDroppedWeaponGlow);
	ImGui::SingleSelect(_S("Glow style##m_iDroppedWeaponGlowStyle"), g_Settings->m_iDroppedWeaponGlowStyle, { _S("Outline"), _S("Thin"), _S("Cover"), _S("Cover Pulse") });

	ImGui::Checkbox(_S("Grenades"), g_Settings->m_bRenderProjectileGlow);
	this->DrawColorEdit4(_S("##Projectile"), g_Settings->m_aProjectileGlow);
	ImGui::SingleSelect(_S("Glow style##m_iProjectileGlowStyle"), g_Settings->m_iProjectileGlowStyle, { _S("Outline"), _S("Thin"), _S("Cover"), _S("Cover Pulse") });

	ImGui::EndChild();
}

void C_Menu::DrawConfigTab( )
{	
	static std::string selected_cfg = "";
    static char cfg_name[32];

	ImGui::SetCursorPos(ImVec2(33, 120));
    ImGui::BeginChild(_S("Config Settings"), ImVec2(255, 500));
	{
		ImGui::Spacing(); ImGui::Spacing();

        if (ImGui::InputText(_S(""), cfg_name, 32)) selected_cfg = std::string(cfg_name);
   
		if ( ImGui::Button(_S("Load config"), ImVec2(235, 25)) )
		{
			SkinChanger::scheduleHudUpdate();

			for ( auto i = 0; i < g_Settings->skins.skinChanger.size(); ++i )
				all_skins[i] = nullptr;

			g_ConfigSystem->LoadConfig(selected_cfg.c_str());
		}
			

		if (ImGui::Button(_S("Save config"), ImVec2(235, 25)))
			g_ConfigSystem->SaveConfig(selected_cfg.c_str());

		if (ImGui::Button(_S("Create config"), ImVec2(235, 25)))
		{
			std::ofstream(selected_cfg + ".cfg", std::ios_base::trunc);
			g_ConfigSystem->SaveConfig((selected_cfg + (std::string)(".cfg")).c_str());
		}	

		if (ImGui::Button(_S("Erase config"), ImVec2(235, 25)))
			g_ConfigSystem->EraseConfig(selected_cfg.c_str());

		ImGui::Spacing();
		ImGui::Spacing();

		m_bFixColorSpacing = true;

		ImGui::Text(_S("Menu")); //		
		this->DrawColorEdit4(_S("menucol1337"), g_Settings->m_colMenuTheme, 2);
		
		ImGui::Text(_S("Checkbox")); //		
		this->DrawColorEdit4(_S("sadaswq23"), g_Settings->m_colCheckbox, 2);
		
		ImGui::Text(_S("Slider")); //		
		this->DrawColorEdit4(_S("qeqweqw"), g_Settings->m_colKeybind, 2);
		
		ImGui::Text(_S("Keybinds")); //	
		this->DrawColorEdit4(_S("weqweqwe"), g_Settings->m_colSlider, 2);
		
		ImGui::Text(_S("Combo")); //		
		this->DrawColorEdit4(_S("pertuzwreu"), g_Settings->m_colCombo, 2);

		m_bFixColorSpacing = false;
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(315, 120));
	ImGui::BeginChild(_S("Config list"), ImVec2(255, 550));
	{
		ImGui::Spacing(); ImGui::Spacing();

		{
			for (auto cfg : g_ConfigSystem->GetConfigList())
				if (ImGui::Selectable(cfg.c_str(), cfg == selected_cfg))
					selected_cfg = cfg;
		}
	}
	ImGui::EndChild();
}

void C_Menu::DrawLegitTab()
{
	ImGui::SetCursorPos(ImVec2(33, 120));
	ImGui::BeginChild(_S("General"), ImVec2(255, 200));
	{
		ImGui::Spacing(); 
		ImGui::Spacing();

		ImGui::Checkbox(_S("Enable"), g_Settings->m_bLegitBot);
		ImGui::Text(_S("Aim key"));
		ImGui::Keybind(_S("lbsw54"), &g_Settings->m_aLegitKey->m_iKeySelected, &g_Settings->m_aLegitKey->m_iModeSelected);

		if ( g_Settings->m_bLegitBot )
			g_Settings->m_bEnabledRage.Get() = false;

		ImGui::Checkbox(_S("Automatic scope"), g_Settings->m_bLegitScope);
		ImGui::Checkbox(_S("Snipers in zoom only"), g_Settings->m_bScopeOnly);

		ImGui::Checkbox(_S("Ignore air"), g_Settings->m_bIgnoreInAir);
		ImGui::Checkbox(_S("Ignore flash"), g_Settings->m_bIgnoreFlash);
		ImGui::Checkbox(_S("Ignore smoke"), g_Settings->m_bIgnoreSmoke);

		ImGui::Keybind(_S("Trigger key"), &g_Settings->m_aTriggerKey->m_iKeySelected, &g_Settings->m_aTriggerKey->m_iModeSelected);
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(315, 120));
	ImGui::BeginChild(_S("Weapon"), ImVec2(255, 530));
	{
		ImGui::Spacing(); ImGui::Spacing();

		static int32_t iWeapon = 0;

		const std::vector< const char* > vecRCSType =
		{
			"Always on",
			"On target"
		};
		
		const std::vector< const char* > vecWeaponList =
		{
			"Pistols",
			"Heavy pistol",
			"Revoler",
			"Rifle",
			"SMG",
			"Heavy",
			"Shotgun",
			"Scout",
			"Auto",
			"AWP"
		};

		const std::vector< const char* > priorities = {
			"FOV",
			"Health",
			"Damage",
			"Distance"
		};

		const std::vector< const char* > aim_types = {
			"Hitbox",
			"Distance"
		};

		const std::vector< const char* > smooth_types = {
			"Static",
			"Dynamic"
		};

		const std::vector< const char* > fov_types = {
			"Static",
			"Dynamic"
		};
		
		const std::vector< const char* > rcs_types = {
			"None",
			"Standalone",
			"Synced"
		};

		const std::vector< const char* > hitbox_list = {
			"Head",
			"Neck",
			"Pelvis",
			"Stomach",
			"Lower chest",
			"Chest",
			"Upper chest",
		};

		ImGui::SingleSelect(_S("Weapon"), &iWeapon, vecWeaponList);
		ImGui::SingleSelect("Aim mode", &g_Settings->m_LegitBotItems[iWeapon].m_iAimType, aim_types);

		if ( g_Settings->m_LegitBotItems[iWeapon].m_iAimType == 0 ) {
			ImGui::SingleSelect("Hitbox priority", &g_Settings->m_LegitBotItems[iWeapon].m_iHitboxPriority, hitbox_list);
		}

		ImGui::SingleSelect("Target selection", &g_Settings->m_LegitBotItems[iWeapon].m_iPriority, priorities);
		ImGui::SingleSelect("FOV mode", &g_Settings->m_LegitBotItems[iWeapon].m_iFovType, fov_types);
		ImGui::SingleSelect("Smooth type", &g_Settings->m_LegitBotItems[iWeapon].m_iSmoothType, smooth_types);
		ImGui::SingleSelect("Recoil system", &g_Settings->m_LegitBotItems[iWeapon].m_iRcsType, rcs_types);
		InputSliderFloat("Max FOV", &g_Settings->m_LegitBotItems[iWeapon].m_flFieldOfView, 0, 30, "%.1f");
		InputSliderFloat("Silent FOV", &g_Settings->m_LegitBotItems[iWeapon].m_flSilentFov, 0, 20, "%.1f");
		InputSliderFloat("Smooth mooth :)", &g_Settings->m_LegitBotItems[iWeapon].m_flSmooth, 1, 20, "%.1f");
		InputSliderFloat("Recoil FOV", &g_Settings->m_LegitBotItems[iWeapon].m_flRcsFov, 0, 99, g_Settings->m_LegitBotItems[iWeapon].m_flRcsFov ? "%.1f" : "auto" );
		InputSliderFloat("Recoil smooth", &g_Settings->m_LegitBotItems[iWeapon].m_flRcsSmooth, 0, 99, g_Settings->m_LegitBotItems[iWeapon].m_flRcsSmooth ? "%.1f" : "auto");
		InputSliderInt("Recoil abscissa", &g_Settings->m_LegitBotItems[iWeapon].m_iRcsAbscissa, 0, 100, "%.0f");
		InputSliderInt("Recoil ordinate", &g_Settings->m_LegitBotItems[iWeapon].m_iRcsOrdinate, 0, 100, "%.0f");
		InputSliderInt("Recoil base", &g_Settings->m_LegitBotItems[iWeapon].m_iRcsStart, 1, 30, "%.0f");
		InputSliderInt("Trigger delay", &g_Settings->m_LegitBotItems[iWeapon].m_iShotDelay, 0, 100, "%.0f");
		InputSliderInt("Kill delay", &g_Settings->m_LegitBotItems[iWeapon].m_iKillDelay, 0, 1000, "%.0f");

		if ( g_Settings->m_LegitBotItems[iWeapon].m_bAutowall ) {
			InputSliderInt("##aimbot.min_damage", &g_Settings->m_LegitBotItems[iWeapon].m_iMinDamage, 1, 100, "%.0f");
		}

	}
	ImGui::EndChild();
}

void C_Menu::DrawColorEdit4( const char* strLabel, Color* aColor, int32_t bSingleElement )
{
	 float aColour[ 4 ] =
		{
            aColor->r( ) / 255.0f,
			aColor->g( ) / 255.0f,
			aColor->b( ) / 255.0f,
			aColor->a( ) / 255.0f
        };

	 if ( g_Menu->FixColorSpacing() )
	 {
		 ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 35);
	 }

	 if ( ImGui::ColorEdit4( strLabel, aColour, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_RGB ), bSingleElement )
		 aColor->SetColor( aColour[ 0 ], aColour[ 1 ], aColour[ 2 ], aColour[ 3 ] );
}

void C_Menu::Initialize()
{
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ImVec4(45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f);
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab] = ImVec4(65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f);
	ImGui::GetStyle().AntiAliasedFill = true;
	ImGui::GetStyle().AntiAliasedLines = true;
	ImGui::GetStyle().ScrollbarSize = 6;
	//ImGui::GetStyle().WindowRounding = 12;

	//D3DXCreateTextureFromFileInMemoryEx(g_Globals.m_Interfaces.m_DirectDevice, &avatarka, sizeof(avatarka), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &g_Menu->m_dTexture);
}

void C_Menu::WaterMark( )
{	
	/*if ( !g_Settings->m_bWaterMark )
		return;*/

	int nScreenSizeX, nScreenSizeY;
	g_Globals.m_Interfaces.m_EngineClient->GetScreenSize( nScreenSizeX, nScreenSizeY );
	
	static const int nTextLength = g_Globals.m_Fonts.m_SegoeUI->CalcTextSizeA( 15.0f, FLT_MAX, NULL, "invictus | download discord.gg/GhMayZRX" ).x + 25;
	
	ImGui::GetOverlayDrawList( )->AddRectFilledMultiColor( ImVec2(nScreenSizeX - nTextLength, 11), ImVec2(nScreenSizeX - 10, 30), ImColor( 20, 20, 20, 230 ), ImColor( 30, 30, 30, 160 ),ImColor( 20, 20, 20, 230 ), ImColor( 30, 30, 30, 160 ) );
	ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( nScreenSizeX - nTextLength, 10 ), ImVec2( nScreenSizeX - 10, 11 ), ImColor( g_Settings->m_colMenuTheme.Get().r(), g_Settings->m_colMenuTheme.Get().g(), g_Settings->m_colMenuTheme.Get().b(), g_Settings->m_colMenuTheme.Get().a()) );

	ImGui::PushFont( g_Globals.m_Fonts.m_SegoeUI );
	
	ImGui::GetOverlayDrawList( )->AddText( ImVec2( nScreenSizeX - nTextLength + 7 + ImGui::CalcTextSize("invictus |").x, 12 ), 
		ImColor( 255, 255, 255, 255 ), 
		_S(" download discord.gg/3JCpts8pbk") );
	ImGui::GetOverlayDrawList( )->AddText( ImVec2( nScreenSizeX - nTextLength + 7, 12 ), 
		ImColor( g_Settings->m_colMenuTheme.Get().r(), g_Settings->m_colMenuTheme.Get().g(), g_Settings->m_colMenuTheme.Get().b(), 255), 
		_S("invictus |") );
	
	ImGui::PopFont( );
}

#define PUSH_BIND( m_Variable, Name )\
if ( g_Tools->IsBindActive( m_Variable ) )\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = _S( Name );\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent + ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\
else\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = "";\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent - ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\

void C_Menu::DrawKeybindList( )
{
	if ( !g_Settings->m_bDrawKeyBindList )
		return;

	int m_Last = 0;
	PUSH_BIND( g_Settings->m_aFakeDuck, "Fake duck" );
	PUSH_BIND( g_Settings->m_aDoubleTap, "Double tap" );
	PUSH_BIND( g_Settings->m_aSlowwalk, "Slow walk" );
	PUSH_BIND( g_Settings->m_aCSlowwalk, "Override SW" );
	PUSH_BIND( g_Settings->m_aHideShots, "Hide shots" );
	PUSH_BIND( g_Settings->m_aSafePoint, "Safe points" );
	PUSH_BIND( g_Settings->m_aInverter, "Invert desync" ); 
	PUSH_BIND( g_Settings->m_aAutoPeek, "Auto peek" );
	PUSH_BIND( g_Settings->m_aMinDamage, "Damage override" );
	PUSH_BIND( g_Settings->m_aFreestand, "Edge rotation" );
	PUSH_BIND( g_Settings->m_aPingSpike, "Fake latency" );

	int32_t iCount = 0;
	for ( auto& Bind : m_BindList )
	{
		if ( Bind.second.m_szName.length( ) )
			iCount++;
	}

	if ( iCount <= 0 && !m_bIsMenuOpened )
		return;

	int nAdvancedFlag = 0;
	if ( !m_bIsMenuOpened )
		nAdvancedFlag = ImGuiWindowFlags_NoMove;
	
	ImGui::SetNextWindowSize( ImVec2(0, 560) );
	ImGui::DefaultBegin( _S( "Keybind List" ), g_Settings->m_bDrawKeyBindList, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag );
	{
		const int32_t x = 0;
		const int32_t y = 560;	
		static int TextSpacing = 0;

		if ( !TextSpacing ) {
			ImGui::PushFont( g_Globals.m_Fonts.m_MenuIcons );
			TextSpacing = ImGui::CalcTextSize(_S("a")).x;
			ImGui::PopFont();
		}

		ImGui::PushFont( g_Globals.m_Fonts.m_LogFont );

		static const ImVec2 TextSize = ImGui::CalcTextSize(_S("a"));
		static const ImVec2 KeyBindSize = ImVec2(210, TextSize.y + 4);

		for ( auto& Bind : m_BindList )
		{
			if ( !Bind.second.m_szName.length( ) )
				continue;
				
			// spacing: 15 pixels
			ImGui::GetOverlayDrawList( )->AddRectFilledMultiColor( ImVec2(x, y + (15 + KeyBindSize.y) * m_Last), ImVec2(x + KeyBindSize.x, y + ((15 + KeyBindSize.y) * m_Last) + KeyBindSize.y), 
				ImColor( 10, 10, 10, static_cast < int >( Bind.second.m_flAlphaPercent * 230 ) ),
				ImColor( 10, 10, 10, static_cast < int >( Bind.second.m_flAlphaPercent * 230 ) ),
				ImColor( 20, 20, 20, static_cast < int >( Bind.second.m_flAlphaPercent * 160 ) ), 
				ImColor( 20, 20, 20, static_cast < int >( Bind.second.m_flAlphaPercent * 160 ) ) 
			);

			// the line.
			ImGui::GetOverlayDrawList( )->AddRectFilled(ImVec2(x, y + (15 + KeyBindSize.y) * m_Last), ImVec2(x + KeyBindSize.x, y + ((15 + KeyBindSize.y) * m_Last) + 2), ImColor(g_Settings->m_colMenuTheme->r(), g_Settings->m_colMenuTheme->g(), g_Settings->m_colMenuTheme->b(), static_cast < int >( Bind.second.m_flAlphaPercent * g_Settings->m_colMenuTheme->a() )) );
			ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 20 + TextSpacing, y + ((15 + KeyBindSize.y) * m_Last) + 2), ImColor( 255, 255, 255, static_cast < int >( Bind.second.m_flAlphaPercent * 255.0f ) ), Bind.second.m_szName.c_str( ) );	
			
			// icon.
			ImGui::PushFont( g_Globals.m_Fonts.m_MenuIcons );
			ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 10, y + ((15 + KeyBindSize.y) * m_Last) + 2), ImColor(240, 240, 240, static_cast < int >( Bind.second.m_flAlphaPercent * 255.0f )), _S("a") ); 
			ImGui::PopFont();

			m_Last++;
		}

		ImGui::PopFont( );		
	}
	ImGui::DefaultEnd( );
}

void C_Menu::DrawSpectatorList( )
{
	std::vector < std::string > vecSpectatorList;
	
	if ( !g_Settings->m_bSpectatorList )
		return;

	if ( g_Globals.m_LocalPlayer && g_Globals.m_LocalPlayer->IsAlive( ) )
	{
		for ( int nPlayerID = 1; nPlayerID <= g_Globals.m_Interfaces.m_GlobalVars->m_iMaxClients; nPlayerID++ )
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex( nPlayerID );
			if ( !pPlayer || pPlayer->IsAlive( ) || !pPlayer->IsPlayer( ) || pPlayer->IsDormant( ) || !pPlayer->m_hObserverTarget( ) )
				continue;

			C_PlayerInfo m_TargetInfo;
			g_Globals.m_Interfaces.m_EngineClient->GetPlayerInfo( pPlayer->EntIndex( ), &m_TargetInfo );

			vecSpectatorList.emplace_back( ( std::string )( m_TargetInfo.m_strName ) );
		}
	}

	if ( !m_bIsMenuOpened && vecSpectatorList.empty( ) )
		return;

	int nAdvancedFlag = 0;
	if ( !m_bIsMenuOpened )
		nAdvancedFlag = ImGuiWindowFlags_NoMove;
	
	ImGui::SetNextWindowSize( ImVec2( 190, m_BindList.empty( ) ? 0 : 35 + ( 21.5f * vecSpectatorList.size( ) ) ) );
	ImGui::DefaultBegin( _S( "Spectator List" ), g_Settings->m_bSpectatorList, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag );
	{
		int32_t x = ImGui::GetCurrentWindow( )->Pos.x + 4.5f;
		int32_t y = ImGui::GetCurrentWindow( )->Pos.y;

		/*ImGui::PushFont( g_Globals.m_Fonts.m_UserIcon );
		ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 5, y + 2 ), ImColor( 71, 163, 255 ), _S( "a" ) );
		ImGui::PopFont( );*/

		/*ImGui::PushFont( g_Globals.m_Fonts.m_LogFont );
		ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 26, y + 2 ), ImColor( 255, 255, 255 ), _S( "Spectator list" ) );
		ImGui::PopFont( );*/

		int m_Last = 0;
		for ( auto& Spectator : vecSpectatorList )
		{
			ImGui::PushFont( g_Globals.m_Fonts.m_LogFont );
			ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 2, 23 + ( y + 16 * m_Last ) ), ImColor( 255, 255, 255, 255 ), Spectator.c_str( ) );
			ImGui::PopFont( );

			m_Last++;
		}
	}
	ImGui::DefaultEnd( );
}