#pragma once
#include "../SDK/Includes.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Render.hpp"
#include "../Settings.hpp"
#include <iomanip>

class C_GrenadePrediction
{
public:
	struct data_t {
		data_t() = default;
		data_t(C_BasePlayer* owner, int index, const Vector& origin, const Vector& velocity, float throw_time, int offset)
		{
			m_owner = owner;
			m_index = index;

			predict(origin, velocity, throw_time, offset);
		}

		bool physics_simulate() {
			if (m_detonated)
				return true;

			static const auto sv_gravity = g_Globals.m_ConVars.m_SvGravity;

			const auto new_velocity_z = m_velocity.z - (sv_gravity->GetFloat() * 0.4f) * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick;

			const auto move = Vector(
				m_velocity.x * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick,
				m_velocity.y * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick,
				(m_velocity.z + new_velocity_z) / 2.f * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick
			);

			m_velocity.z = new_velocity_z;

			auto trace = trace_t();

			physics_push_entity(move, trace);

			if (m_detonated)
				return true;

			if (trace.fraction != 1.f) {
				update_path< true >();

				perform_fly_collision_resolution(trace);
			}

			return false;
		}
		void physics_trace_entity(const Vector& src, const Vector& dst, std::uint32_t mask, trace_t& trace) {
			g_Globals.m_Interfaces.m_EngineTrace->TraceHull(
				src, dst, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f },
				mask, m_owner, m_collision_group, &trace
			);

			if (trace.startsolid
				&& (trace.contents & CONTENTS_CURRENT_90)) {
				trace.CleanOut();

				g_Globals.m_Interfaces.m_EngineTrace->TraceHull(
					src, dst, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f },
					mask & ~CONTENTS_CURRENT_90, m_owner, m_collision_group, &trace
				);
			}

			if (!trace.DidHit()
				|| !trace.hit_entity
				|| !reinterpret_cast<C_BaseEntity*>(trace.hit_entity)->IsPlayer())
				return;

			trace.CleanOut();

			g_Globals.m_Interfaces.m_EngineTrace->TraceLine(src, dst, mask, m_owner, m_collision_group, &trace);
		}
		void physics_push_entity(const Vector& push, trace_t& trace) {
			physics_trace_entity(m_origin, m_origin + push,
				m_collision_group == COLLISION_GROUP_DEBRIS
				? (MASK_SOLID | CONTENTS_CURRENT_90) & ~CONTENTS_MONSTER
				: MASK_SOLID | CONTENTS_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE | CONTENTS_CURRENT_90 | CONTENTS_HITBOX,
				trace
			);

			if (trace.startsolid) {
				m_collision_group = COLLISION_GROUP_INTERACTIVE_DEBRIS;

				g_Globals.m_Interfaces.m_EngineTrace->TraceLine(
					m_origin - push, m_origin + push,
					(MASK_SOLID | CONTENTS_CURRENT_90) & ~CONTENTS_MONSTER,
					m_owner, m_collision_group, &trace
				);
			}

			if (trace.fraction) {
				m_origin = trace.endpos;
			}

			if (!trace.hit_entity)
				return;

			if (reinterpret_cast<C_BaseEntity*>(trace.hit_entity)->IsPlayer()
				|| m_index != WEAPON_TAGRENADE && m_index != WEAPON_MOLOTOV && m_index != WEAPON_INCGRENADE)
				return;

			static const auto weapon_molotov_maxdetonateslope = g_Globals.m_Interfaces.m_CVar->FindVar(_S("weapon_molotov_maxdetonateslope"));

			if (m_index != WEAPON_TAGRENADE
				&& trace.plane.normal.z < std::cos(DEG2RAD(weapon_molotov_maxdetonateslope->GetFloat())))
				return;

			detonate< true >();
		}
		void perform_fly_collision_resolution(trace_t& trace) {
			auto surface_elasticity = 1.f;

			if (trace.hit_entity)
			{
				if (reinterpret_cast<C_BaseEntity*>(trace.hit_entity)->IsBreakableEntity()) {
				BREAKTROUGH:
					m_velocity *= 0.4f;

					return;
				}

				const auto is_player = reinterpret_cast<C_BaseEntity*>(trace.hit_entity)->IsPlayer();
				if (is_player) {
					surface_elasticity = 0.3f;
				}

				if (trace.hit_entity->EntIndex()) {
					if (is_player
						&& m_last_hit_entity == trace.hit_entity) {
						m_collision_group = COLLISION_GROUP_DEBRIS;

						return;
					}

					m_last_hit_entity = trace.hit_entity;
				}
			}

			auto velocity = Vector();

			const auto back_off = m_velocity.Dot(trace.plane.normal) * 2.f;

			for (auto i = 0u; i < 3u; i++) {
				const auto change = trace.plane.normal[i] * back_off;

				velocity[i] = m_velocity[i] - change;

				if (std::fabs(velocity[i]) >= 1.f)
					continue;

				velocity[i] = 0.f;
			}

			velocity *= std::clamp< float >(surface_elasticity * 0.45f, 0.f, 0.9f);

			if (trace.plane.normal.z > 0.7f) {
				const auto speed_sqr = velocity.LengthSqr();
				if (speed_sqr > 96000.f) {
					const auto l = velocity.Normalized().Dot(trace.plane.normal);
					if (l > 0.5f) {
						velocity *= 1.f - l + 0.5f;
					}
				}

				if (speed_sqr < 400.f) {
					m_velocity = {};
				}
				else {
					m_velocity = velocity;

					physics_push_entity(velocity * ((1.f - trace.fraction) * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick), trace);
				}
			}
			else {
				m_velocity = velocity;

				physics_push_entity(velocity * ((1.f - trace.fraction) * g_Globals.m_Interfaces.m_GlobalVars->m_flIntervalPerTick), trace);
			}

			if (m_bounces_count > 20)
				return detonate< false >();

			++m_bounces_count;
		}
		void think() {
			switch (m_index) {
			case WEAPON_SMOKEGRENADE:
				if (m_velocity.LengthSqr() <= 0.01f) {
					detonate< false >();
				}

				break;
			case WEAPON_DECOY:
				if (m_velocity.LengthSqr() <= 0.04f) {
					detonate< false >();
				}

				break;
			case WEAPON_FLASHBANG:
			case WEAPON_HEGRENADE:
			case WEAPON_MOLOTOV:
			case WEAPON_INCGRENADE:
				if (TICKS_TO_TIME(m_tick) > m_detonate_time) {
					detonate< false >();
				}

				break;
			}

			m_next_think_tick = m_tick + TIME_TO_TICKS(0.2f);
		}

		template < bool _bounced >
		void detonate() {
			m_detonated = true;

			update_path< _bounced >();
		}

		template < bool _bounced >
		void update_path() {
			m_last_update_tick = m_tick;

			m_path.emplace_back(m_origin, _bounced);
		}

		void predict(const Vector& origin, const Vector& velocity, float throw_time, int offset) {
			m_origin = origin;
			m_velocity = velocity;
			m_collision_group = COLLISION_GROUP_PROJECTILE;

			const auto tick = TIME_TO_TICKS(1.f / 30.f);

			m_last_update_tick = -tick;

			switch (m_index) {
			case WEAPON_SMOKEGRENADE: m_next_think_tick = TIME_TO_TICKS(1.5f); break;
			case WEAPON_DECOY: m_next_think_tick = TIME_TO_TICKS(2.f); break;
			case WEAPON_FLASHBANG:
			case WEAPON_HEGRENADE:
				m_detonate_time = 1.5f;
				m_next_think_tick = TIME_TO_TICKS(0.02f);

				break;
			case WEAPON_MOLOTOV:
			case WEAPON_INCGRENADE:
				static const auto molotov_throw_detonate_time = g_Globals.m_Interfaces.m_CVar->FindVar(_S("molotov_throw_detonate_time"));

				m_detonate_time = molotov_throw_detonate_time->GetFloat();
				m_next_think_tick = TIME_TO_TICKS(0.02f);

				break;
			}

			m_SourceTime = throw_time;
			for (; m_tick < TIME_TO_TICKS(60.f); ++m_tick) {
				if (m_next_think_tick <= m_tick) {
					think();
				}

				if (m_tick < offset)
					continue;

				if (physics_simulate())
					break;

				if (m_last_update_tick + tick > m_tick)
					continue;

				update_path< false >();
			}

			if (m_last_update_tick + tick <= m_tick) {
				update_path< false >();
			}

			m_ExpireTime = throw_time + TICKS_TO_TIME(m_tick);
		}

		inline float CSGO_Armores(float flDamage, int ArmorValue) const {
			float flArmorRatio = 0.5f;
			float flArmorBonus = 0.5f;
			if ( ArmorValue > 0 ) {
				float flNew = flDamage * flArmorRatio;
				float flArmor = (flDamage - flNew) * flArmorBonus;

				if ( flArmor > static_cast<float>(ArmorValue) ) {
					flArmor = static_cast<float>(ArmorValue) * (1.f / flArmorBonus);
					flNew = flDamage - flArmor;
				}

				flDamage = flNew;
			}
			return flDamage;
		}

		inline int CalculateGrenadeDamage(C_BasePlayer* pPlayer, Vector vecNadeOrigin) const {
			int iFinalDamage = 0;

			// get center of mass for player.
			auto origin = pPlayer->m_vecOrigin();
			auto collideable = pPlayer->GetCollideable();

			auto min = collideable->OBBMins() + origin;
			auto max = collideable->OBBMaxs() + origin;

			auto center = min + (max - min) * 0.5f;

			// get delta between center of mass and final nade pos.
			auto delta = center - vecNadeOrigin;

			// is within damage radius?
			if ( delta.Length() > 350.f )
				return 0;

			Vector NadeScreen;
			Math::WorldToScreen(vecNadeOrigin, NadeScreen);

			// main hitbox, that takes damage
			Vector vPelvis = pPlayer->GetHitboxPos(HITBOX_PELVIS);
			static const Vector hull[2] = { Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f) };
			Ray_t ray;
			ray.Init(vecNadeOrigin, vPelvis, hull[0], hull[1]);
			trace_t ptr; CTraceFilter filter;
			filter.ccIgnore = "Projectile";
			g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &ptr);

			//trace to it
			if ( ptr.hit_entity == pPlayer ) {
				Vector PelvisScreen;

				Math::WorldToScreen(vPelvis, PelvisScreen);

				// some magic values by VaLvO
				static float a = 105.0f;
				static float b = 25.0f;
				static float c = 140.0f;

				float d = ((delta.Length() - b) / c);
				float flDamage = a * exp(-d * d);

				auto dmg = max(static_cast<int>(ceilf(CSGO_Armores(flDamage, pPlayer->m_ArmourValue()))), 0);

				dmg = min(dmg, (pPlayer->m_ArmourValue() > 0) ? 57 : 98);
				iFinalDamage = dmg;
			}

			return iFinalDamage;
		}
		void draw_warning_circle(const char* name, Vector m_pos, float time, int distance, bool safe) const  {
			if ( Vector pos; Math::WorldToScreen(m_pos, pos) ) {
				ImVec4 line = ImLerp(ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f), ImVec4(255 / 255.f, 25 / 255.f, 15 / 255.f, 255 / 255.f), 1.f - ImClamp(distance, 0, 320) / 320.f);
				ImVec4 fill = ImLerp(ImVec4(255 / 255.f, 25 / 255.f, 15 / 255.f, 0 / 255.f), ImVec4(255 / 255.f, 25 / 255.f, 15 / 255.f, 155 / 255.f), 1.f - ImClamp(distance, 0, 320) / 320.f);
				if ( safe )
					fill = ImVec4(45 / 255.f, 255 / 255.f, 15 / 255.f, 25.f / 255.f);
				g_Render->GetDrawList()->AddCircleFilled(ImVec2(pos.x, pos.y - 34.f), 32.f, g_Render->GetU32(Color(24, 24, 24, 225)), 48);
				for ( int i = 32.f; i > 0; i-- )
				{
					g_Render->GetDrawList()->AddCircle(ImVec2(pos.x, pos.y - 34.f), i, ImColor(fill.x, fill.y, fill.z, fill.w * (34.f / i)), 64);
				}

				g_Render->RenderArc(pos.x, pos.y - 34.f, 32.f, 90.f, 360.f * time + 90, Color(line.x, line.y, line.z, line.w), 2.f);
				if ( name == "j" ) {
					int dmg = CalculateGrenadeDamage(g_Globals.m_LocalPlayer, m_pos);
					float damage_factor = std::clamp(dmg / 98.f, 0.f, 1.f);
					ImVec4 damage = ImLerp(ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f), ImVec4(255 / 255.f, 25 / 255.f, 15 / 255.f, 255 / 255.f), damage_factor);
					line = ImLerp(ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 255 / 255.f), ImVec4(255 / 255.f, 25 / 255.f, 15 / 255.f, 255 / 255.f), damage_factor);
					fill = ImLerp(ImVec4(255 / 255.f, 25 / 255.f, 15 / 255.f, 0 / 255.f), ImVec4(255 / 255.f, 25 / 255.f, 15 / 255.f, 155 / 255.f), damage_factor);

					g_Render->RenderText(name, ImVec2(pos.x, pos.y - 64.f), Color(230, 230, 230), true, true, g_Globals.m_Fonts.m_WeaponMass);
					g_Render->RenderText(safe ? "SAFE" : std::to_string(int(dmg)), ImVec2(pos.x, pos.y - 16.f), Color(255, 255, 255), true, true, g_Globals.m_Fonts.m_ESP);
				}
				else {
					g_Render->RenderText(name, ImVec2(pos.x, pos.y - 64.f), Color(230, 230, 230), true, true, g_Globals.m_Fonts.m_WeaponMass);
				}
			}
		}
		bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos) const
		{
			static auto LineGoesThroughSmokeFn =
				(bool (*)(Vector vStartPos,
					Vector vEndPos))g_Globals.m_AddressList.m_LineThroughSmoke;
			return LineGoesThroughSmokeFn(vStartPos, vEndPos);
		}
		bool draw() const {
			if (m_path.size() <= 1u
				|| TICKS_TO_TIME(m_Globals()->m_flCurTime) >= m_ExpireTime)
				return false;

			if (!g_Globals.m_LocalPlayer->IsAlive())
				return false;

			Vector vecPrevious = Vector(0, 0, 0);
			Vector vecPreviousOrigin = Vector(0, 0, 0);

			auto prev_screen = Vector();
			auto prev_on_screen = Math::WorldToScreen(std::get< Vector >(m_path.front()), prev_screen);

			for (auto i = 1u; i < m_path.size(); ++i) {
				auto cur_screen = Vector();
				const auto cur_on_screen = Math::WorldToScreen(std::get< Vector >(m_path.at(i)), cur_screen);

				if (prev_on_screen && cur_on_screen)
				{
					g_Render->RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_Settings->m_colGrenadeTrajectory->r(), g_Settings->m_colGrenadeTrajectory->g(), g_Settings->m_colGrenadeTrajectory->b(), 255), 1.f);
					g_Render->RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_Settings->m_colGrenadeTrajectory->r(), g_Settings->m_colGrenadeTrajectory->g(), g_Settings->m_colGrenadeTrajectory->b(), 100), 3.f);
					g_Render->RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_Settings->m_colGrenadeTrajectory->r(), g_Settings->m_colGrenadeTrajectory->g(), g_Settings->m_colGrenadeTrajectory->b(), 55), 5.f);
					g_Render->RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_Settings->m_colGrenadeTrajectory->r(), g_Settings->m_colGrenadeTrajectory->g(), g_Settings->m_colGrenadeTrajectory->b(), 25), 8.f);
					g_Render->RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_Settings->m_colGrenadeTrajectory->r(), g_Settings->m_colGrenadeTrajectory->g(), g_Settings->m_colGrenadeTrajectory->b(), 5), 11.f);
					g_Render->RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_Settings->m_colGrenadeTrajectory->r(), g_Settings->m_colGrenadeTrajectory->g(), g_Settings->m_colGrenadeTrajectory->b(), 1), 14.f);

					if ( i == m_path.size() - 1 ) 
					{
						vecPreviousOrigin = std::get< Vector >(m_path.at(i));
						g_Render->RenderCircle3D(std::get< Vector >(m_path.at(i)), 32, 171, Color(g_Settings->m_colGrenadeTrajectory));
					}
				}

				prev_screen = cur_screen;
				prev_on_screen = cur_on_screen;
				vecPreviousOrigin = std::get< Vector >(m_path.at(i));
			}

			if (g_Settings->m_GrenadeTimers)
			{
				float_t flPercentage = (m_ExpireTime - TICKS_TO_TIME(g_Globals.m_LocalPlayer->m_nTickBase())) / (m_ExpireTime - m_SourceTime);
			}

			if (m_index != WEAPON_MOLOTOV && m_index != WEAPON_INCGRENADE && m_index != WEAPON_HEGRENADE )
				return true;

			const char* icon_string = "";
			switch ( m_index ) {
			case WEAPON_MOLOTOV:
				icon_string = "l";
				break;
			case WEAPON_INCGRENADE:
				icon_string = "l";
				break;
			case WEAPON_HEGRENADE:
				icon_string = "j";
				break;
			}

			bool safe_molotov = false;
			Vector end_point = m_origin;
			trace_t trace;

			CTraceFilter filter; filter.pSkip = m_owner; filter.ccIgnore = "Projectile";
			Ray_t Ray;
			Ray.Init(m_origin, m_origin - Vector(0.f, 0.f, 256.f));

			g_Globals.m_Interfaces.m_EngineTrace->TraceRay(Ray, MASK_SHOT, &filter, &trace);

			if ( m_index == WEAPON_MOLOTOV )
			{
				if ( m_origin.DistTo(trace.endpos) > 128.f )
					safe_molotov = true;
				else
					end_point = trace.endpos;

				if ( IsLineGoesThroughSmoke(m_origin, trace.endpos) )
					safe_molotov = true, end_point = trace.endpos;
			}

			float time_expired = (m_ExpireTime - (m_Globals()->m_flCurTime)) / (m_ExpireTime - m_SourceTime);

			draw_warning_circle(icon_string, end_point, time_expired, g_Globals.m_LocalPlayer->GetAbsOrigin().DistTo(end_point), safe_molotov);

			return true;
		}
		bool draw_local() const {
			if ( m_path.size() <= 1u )
				return false;

			if ( !g_Globals.m_LocalPlayer->IsAlive() )
				return false;

			auto prev_screen = Vector();
			auto prev_on_screen = Math::WorldToScreen(std::get< Vector >(m_path.front()), prev_screen);

			for ( auto i = 1u; i < m_path.size(); ++i ) {
				auto cur_screen = Vector();
				const auto cur_on_screen = Math::WorldToScreen(std::get< Vector >(m_path.at(i)), cur_screen);

				if ( prev_on_screen && cur_on_screen )
				{
					g_Render->RenderLine(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_Settings->m_GrenadeWarning), 1.0f);

					if ( m_path.at(i).second || i == m_path.size() - 1 && !m_path.at(i).second ) {
						g_Render->RenderRect(prev_screen.x - 3, prev_screen.y - 3, prev_screen.x + 3, prev_screen.y + 3, Color(0, 0, 0));
						g_Render->RenderRectFilled(prev_screen.x - 2, prev_screen.y - 2, prev_screen.x + 2, prev_screen.y + 2, g_Settings->m_GrenadeWarning);
					}

					if ( i == m_path.size() - 1 ) {
						g_Render->RenderCircle3D(std::get< Vector >(m_path.at(i)), 32, 171, Color(g_Settings->m_GrenadeWarning));
					}
				}

				prev_screen = cur_screen;
				prev_on_screen = cur_on_screen;
			}

			return true;
		}
		bool m_detonated{};
		C_BasePlayer* m_owner{};
		Vector											m_origin{}, m_velocity{};
		C_ClientEntity* m_last_hit_entity{};
		float											m_SourceTime;
		int												m_collision_group{};
		float											m_detonate_time{}, m_ExpireTime{};
		int												m_index{}, m_tick{}, m_next_think_tick{},
			m_last_update_tick{}, m_bounces_count{};
		std::vector< std::pair< Vector, bool > >		m_path{};
	} m_data{};

	std::unordered_map< unsigned long, data_t > m_list{};
public:
	__forceinline C_GrenadePrediction() = default;

	virtual void OnCreateMove(C_UserCmd* cmd);

	virtual const data_t& get_local_data() const { return m_data; }

	std::unordered_map< unsigned long, data_t >& get_list() { return m_list; }
};

inline C_GrenadePrediction* g_GrenadePrediction = new C_GrenadePrediction();