
#include "LegitBot.hpp"

#include <SDK/Math/Math.hpp>
#include "../Tools/Tools.hpp"

#include "../Features/RageBot/Autowall.hpp"

void FixAngles(QAngle& angles) {
  Math::Normalize3(angles);
  Math::ClampAngles(angles);
}


bool C_LegitBot::IsRcs() 
{
  return g_Globals.m_LocalPlayer->m_iShotsFired() >= static_cast<bool>(m_Settings.m_iRcsType) + 1;
}

float GetRealDistanceFOV(float distance, QAngle angle, QAngle m_angViewAngles) {
  Vector aimingAt;
  Math::AngleVectors(m_angViewAngles, aimingAt);
  aimingAt *= distance;
  Vector aimAt;
  Math::AngleVectors(angle, aimAt);
  aimAt *= distance;
  return aimingAt.DistTo(aimAt) / 5;
}

float C_LegitBot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle) {
  QAngle delta = aimAngle - viewAngle;
  FixAngles(delta);
  return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}

bool C_LegitBot::IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos) 
{
  static auto LineGoesThroughSmokeFn =
      (bool (*)(Vector vStartPos,
                Vector vEndPos))g_Globals.m_AddressList.m_LineThroughSmoke;
  return LineGoesThroughSmokeFn(vStartPos, vEndPos);
}

bool C_LegitBot::IsEnabled(C_UserCmd* pCmd) 
{
    if (!g_Settings->m_bLegitBot) 
        return false;

    if (
        !g_Globals.m_Interfaces.m_EngineClient->IsConnected() ||
        !g_Globals.m_LocalPlayer || !g_Globals.m_LocalPlayer->IsAlive()
        ) 
    {
    return false;
    }

    C_BaseCombatWeapon* pCombatWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();
    if ( !pCombatWeapon )
        return false;

    auto ID = pCombatWeapon->m_iItemDefinitionIndex();
    
    static int32_t iSettingsID = 0;
    static int32_t iLastWeaponID = 0;

      if ( iLastWeaponID != ID )
      {
          switch ( pCombatWeapon->m_iItemDefinitionIndex() )
          {
          case WEAPON_AK47:
          case WEAPON_M4A1:
          case WEAPON_M4A1_SILENCER:
          case WEAPON_FAMAS:
          case WEAPON_SG553:
          case WEAPON_GALILAR:
              iSettingsID = 3; break;
          case WEAPON_MAG7:
          case WEAPON_NOVA:
          case WEAPON_XM1014:
          case WEAPON_SAWEDOFF:
              iSettingsID = 6; break;
          case WEAPON_MP7:
          case WEAPON_MP9:
          case WEAPON_P90:
          case WEAPON_UMP45:
              iSettingsID = 4; break;
          case WEAPON_M249:
          case WEAPON_NEGEV:
              iSettingsID = 5; break;
          case WEAPON_SCAR20:
          case WEAPON_G3SG1:
              iSettingsID = 8; break;
          case WEAPON_GLOCK:
          case WEAPON_HKP2000:
          case WEAPON_USP_SILENCER:
          case WEAPON_CZ75A:
          case WEAPON_TEC9:
          case WEAPON_ELITE:
          case WEAPON_FIVESEVEN:
          case WEAPON_P250:
              iSettingsID = 0; break;
          case WEAPON_SSG08:
              iSettingsID = 7; break;
          case WEAPON_AWP:
              iSettingsID = 9; break;
          case WEAPON_DEAGLE:
              iSettingsID = 1; break;
          case WEAPON_REVOLVER:
              iSettingsID = 2; break;
      
          default: iSettingsID = -1;
          }

          memcpy(&m_Settings, &g_Settings->m_LegitBotItems[iSettingsID], 72U);
          iLastWeaponID = ID;
      }

    if (
        ( pCombatWeapon->m_iItemDefinitionIndex() == WEAPON_AWP || pCombatWeapon->m_iItemDefinitionIndex() == WEAPON_SSG08) &&
        g_Settings->m_bScopeOnly && 
        !g_Globals.m_LocalPlayer->m_bIsScoped()
        ) 
    {
    return false;
    }

    return g_Tools->IsBindActive(g_Settings->m_aLegitKey);
}
//--------------------------------------------------------------------------------
float C_LegitBot::GetSmooth() {
  float flSmooth = IsRcs() && m_Settings.m_flRcsSmooth ? m_Settings.m_flRcsSmooth
                                                  : m_Settings.m_flSmooth;
  return flSmooth;
}
//--------------------------------------------------------------------------------
void C_LegitBot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle) {
  auto smooth_value = GetSmooth();
  if (smooth_value <= 1) {
    return;
  }

  QAngle delta = aimAngle - currentAngle;
  FixAngles(delta);

  // ничего не трогать и не менять
  if (m_Settings.m_iSmoothType == 1) {
    float flDeltaLength = fmaxf(
        sqrtf((delta.pitch * delta.pitch) + (delta.yaw * delta.yaw)), 0.01f);
    delta *= (1.0f / flDeltaLength);

    g_Tools->RandomSeed(m_Globals()->m_iTickCount);
    float flRandomize = g_Tools->RandomFloat(-0.1f, 0.1f);
    smooth_value = fminf((m_Globals()->m_flIntervalPerTick * 64.0f) /
                             (flRandomize + smooth_value * 0.15f),
        flDeltaLength);
  } else {
    smooth_value = (m_Globals()->m_flIntervalPerTick * 64.0f) / smooth_value;
  }

  delta *= smooth_value;
  angle = currentAngle + delta;
  FixAngles(angle);
}
//--------------------------------------------------------------------------------
void C_LegitBot::RCS(QAngle& angle, C_BasePlayer* m_Target, bool should_run) {
  
  if (!static_cast<bool>(m_Settings.m_iRcsType)) 
  {
    RCSLastPunch.Init();
    return;
  }

  if (m_Settings.m_iRcsAbscissa == 0 && m_Settings.m_iRcsOrdinate == 0) {
    RCSLastPunch.Init();
    return;
  }

  QAngle punch = g_Globals.m_LocalPlayer->m_aimPunchAngle() * 2.0f;

  auto weapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();
  if (weapon && weapon->m_flNextPrimaryAttack() > m_Globals()->m_flCurTime) {
    auto delta_angles = punch - RCSLastPunch;
    auto delta = weapon->m_flNextPrimaryAttack() - m_Globals()->m_flCurTime;
    if (delta >= m_Globals()->m_flIntervalPerTick)
      punch = RCSLastPunch +
              delta_angles / static_cast<float>(TIME_TO_TICKS(delta));
  }

  CurrentPunch = punch;
  if (m_Settings.m_iRcsType == 0 && !should_run)
    punch -= {RCSLastPunch.pitch, RCSLastPunch.yaw, 0.f};

  RCSLastPunch = CurrentPunch;
  if (!IsRcs()) {
    return;
  }

  angle.pitch -= punch.pitch * (m_Settings.m_iRcsAbscissa / 100.0f);
  angle.yaw -= punch.yaw * (m_Settings.m_iRcsOrdinate / 100.0f);

  FixAngles(angle);
}
//--------------------------------------------------------------------------------
float C_LegitBot::GetFov() {
  if (IsRcs() && static_cast<bool>(m_Settings.m_iRcsType) && static_cast<bool>(m_Settings.m_flRcsFov))
    return m_Settings.m_flRcsFov;
  
  if (!silent_enabled) 
      return m_Settings.m_flFieldOfView;
    
  return m_Settings.m_flSilentFov > m_Settings.m_flFieldOfView ? m_Settings.m_flSilentFov
                                                : m_Settings.m_flFieldOfView;
}
//--------------------------------------------------------------------------------
bool IsNotTarget(C_BasePlayer* e) 
{
  if (!e)                                       return true;
  if (e == g_Globals.m_LocalPlayer)             return true;
  if (e->m_iHealth() <= 0)                      return true;
  if (e->m_bGunGameImmunity())                  return true;
  if (e->m_fFlags() & FL_FROZEN)                return true;
  if (e->m_iTeamNum() == g_Globals.m_LocalPlayer->m_iTeamNum()) return true;

  return false;
}
//--------------------------------------------------------------------------------
bool CanSeePlayer(C_BasePlayer* pPlayer, const Vector& vecPos) {
  CGameTrace tr;
  Ray_t ray;
  CTraceFilter filter;

  filter.pSkip = g_Globals.m_LocalPlayer;

  ray.Init(Vector(
                      g_Globals.m_LocalPlayer->m_vecOrigin() +
                      Vector
                      (
                          0.0f, 
                          0.0f, 
                          ((1.0f - g_Globals.m_LocalPlayer->m_flDuckAmount()) * 18.0f) + 46.0f
                      )
                 ), 
           vecPos);

  g_Globals.m_Interfaces.m_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

  return tr.hit_entity == pPlayer || tr.fraction > 0.97f;
}
//--------------------------------------------------------------------------------
C_BasePlayer* C_LegitBot::GetClosestPlayer(C_UserCmd* cmd, int& bestBone) 
{  
  QAngle ang;
  Vector eVecTarget;
  Vector pVecTarget =
      g_Globals.m_LocalPlayer->m_vecOrigin() +
      Vector(
            0.0f, 
            0.0f,
            ((1.0f - g_Globals.m_LocalPlayer->m_flDuckAmount()) * 18.0f) + 46.0f
            );
  
  if (m_Target && !kill_delay && m_Settings.m_iKillDelay > 0 && IsNotTarget(m_Target)) 
  {
    m_Target    = NULL;
    shot_delay  = false;
    kill_delay  = true;
    
    kill_delay_time = (int)GetTickCount() + m_Settings.m_iKillDelay;
  }
  
  if (kill_delay) 
  {
    if (kill_delay_time <= (int)GetTickCount())
      kill_delay = false;
    else
      return NULL;
  }

  C_BasePlayer* player;
  m_Target = NULL;
  
  float_t bestFov       = FLT_MAX;
  float_t bestBoneFov   = FLT_MAX;
  float_t bestDistance  = FLT_MAX;

  float_t bestDamage = 0.f;
  float_t fov        = 0.f;
  float_t damage     = 0.f;
  float_t distance   = 0.f;

  int32_t bestHealth = 100;
  int32_t health;
  
  const int32_t fromBone    = m_Settings.m_iAimType == 1 ? 0 : m_Settings.m_iHitboxPriority;
  const int32_t toBone      = m_Settings.m_iAimType == 1 ? 7 : m_Settings.m_iHitboxPriority;
  
  for (int i = 1; i < g_Globals.m_Interfaces.m_EngineClient->GetMaxClients(); ++i) 
  {
    damage = 0.f;
    player = static_cast<C_BasePlayer*>(g_Globals.m_Interfaces.m_EntityList->GetClientEntity(i));
    
    if (IsNotTarget(player))
      continue;

    for (int bone = fromBone; bone <= toBone; bone++) 
    { 
      eVecTarget = player->GetHitboxPos(bone);
      
      Math::VectorAngles(eVecTarget - pVecTarget, ang);
      FixAngles(ang);
      
      distance = pVecTarget.DistTo(eVecTarget);
      
      if (m_Settings.m_iFovType == 1)
        fov = GetRealDistanceFOV(distance, ang, cmd->m_angViewAngles + RCSLastPunch);
      else
        fov = GetFovToPlayer(cmd->m_angViewAngles + RCSLastPunch, ang);

      if (fov > GetFov()) 
          continue;

      if (!CanSeePlayer(player, eVecTarget)) 
      {
        if (!m_Settings.m_bAutowall) 
            continue;

        damage = g_AutoWall->GetPointDamage(pVecTarget, eVecTarget);

        if (damage < m_Settings.m_iMinDamage) 
            continue;
      }
      
      if ((m_Settings.m_iPriority == 1 || m_Settings.m_iPriority == 2) && damage == 0.f)
        damage = g_AutoWall->GetPointDamage(pVecTarget, eVecTarget);

      health = player->m_iHealth() - damage;
      
      if (!g_Settings->m_bIgnoreSmoke && IsLineGoesThroughSmoke(pVecTarget, eVecTarget))
        continue;

      if (!g_Settings->m_bIgnoreInAir && !(g_Globals.m_LocalPlayer->m_fFlags() & FL_ONGROUND))
          continue;

      if (m_Settings.m_iAimType == 1 && bestBoneFov < fov)
        continue;
      
      bestBoneFov = fov;
      
      if (
          (m_Settings.m_iPriority == 0 && fov < bestFov)           ||
          (m_Settings.m_iPriority == 1 && health < bestHealth)     ||
          (m_Settings.m_iPriority == 2 && damage > bestDamage)     ||
          (m_Settings.m_iPriority == 3 && distance < bestDistance)   
         ) 
      {
        bestFov         = fov;
        bestHealth      = health;
        bestDamage      = damage;
        bestDistance    = distance;
        bestBone = bone;
        m_Target = player;
  } } }
  
  return m_Target;
}
//--------------------------------------------------------------------------------
bool C_LegitBot::IsNotSilent(float fov) 
{
  return IsRcs() || !silent_enabled || (silent_enabled && fov > m_Settings.m_flSilentFov);
}
//--------------------------------------------------------------------------------
void C_LegitBot::OnMove(C_UserCmd* pCmd) 
{
  if (!IsEnabled(pCmd)) 
  {
    if (g_Globals.m_LocalPlayer &&
        g_Globals.m_Interfaces.m_EngineClient->IsInGame() &&
        g_Globals.m_LocalPlayer->IsAlive() && g_Settings->m_bEnabledRage &&
        m_Settings.m_iRcsType == 1) {
        auto pWeapon = g_Globals.m_LocalPlayer->m_hActiveWeapon();
        if (pWeapon && pWeapon->IsGun()) {
        RCS(pCmd->m_angViewAngles, m_Target, false);
        FixAngles(pCmd->m_angViewAngles);
        g_Globals.m_Interfaces.m_EngineClient->SetViewAngles(
            &pCmd->m_angViewAngles);
        }
    } 
    else 
    {
      RCSLastPunch = {0, 0, 0};
    }

    is_delayed = false;
    shot_delay = false;
    kill_delay = false;
    
    silent_enabled = (bool)m_Settings.m_flSilentFov;
    
    m_Target = NULL;
    
    return;
  }

  g_Tools->RandomSeed(pCmd->m_nCommand);

  auto weapon = g_Globals.m_LocalPlayer->m_hActiveWeapon().Get();
  if (!weapon) return;

  auto weapon_data = weapon->GetWeaponData();
  if (!weapon_data) return;

  bool should_do_rcs = false;
  QAngle angles = pCmd->m_angViewAngles;
  QAngle current = angles;
  float fov = 180.f;
  if (!(g_Settings->m_bIgnoreFlash &&
        g_Globals.m_LocalPlayer->m_flFlashTime() >= 0.1f)) {
    int bestBone = -1;
    if (GetClosestPlayer(pCmd, bestBone)) {
      Math::VectorAngles(
          m_Target->GetHitboxPos(bestBone) -
              Vector(
                  g_Globals.m_LocalPlayer->m_vecOrigin() +
                  Vector(0.0f, 0.0f,
                         ((1.0f - g_Globals.m_LocalPlayer->m_flDuckAmount()) *
                          18.0f) +
                             46.0f)),
          angles);
      FixAngles(angles);
      if (m_Settings.m_iFovType == 1)
        fov = GetRealDistanceFOV(
            Vector(g_Globals.m_LocalPlayer->m_vecOrigin() +
                   Vector(0.0f, 0.0f,
                          ((1.0f - g_Globals.m_LocalPlayer->m_flDuckAmount()) *
                           18.0f) +
                              46.0f))
                .DistTo(m_Target->GetHitboxPos(bestBone)),
            angles, pCmd->m_angViewAngles);
      else
        fov = GetFovToPlayer(pCmd->m_angViewAngles, angles);

      should_do_rcs = true;

      if (m_Settings.m_flSilentFov && !is_delayed && !shot_delay &&
          m_Settings.m_iShotDelay > 0) {
        is_delayed = true;
        shot_delay = true && g_Tools->IsBindActive(g_Settings->m_aTriggerKey);
        shot_delay_time = GetTickCount() + m_Settings.m_iShotDelay;
      }

      if (shot_delay && shot_delay_time <= GetTickCount()) {
        shot_delay = false;
      }

      if (shot_delay) {
        pCmd->m_nButtons &= ~IN_ATTACK;
      }

      if (g_Tools->IsBindActive(g_Settings->m_aTriggerKey)) {
        if (!weapon_data->m_bFullAuto) {
          if (pCmd->m_nCommand % 2 == 0) {
            pCmd->m_nButtons &= ~IN_ATTACK;
          } else {
            pCmd->m_nButtons |= IN_ATTACK;
          }
        } else {
          pCmd->m_nButtons |= IN_ATTACK;
        }
      }

      if (m_Settings.m_bAutostop) {
        pCmd->m_flForwardMove = pCmd->m_flSideMove = 0;
      }
    }
  }

  if (IsNotSilent(fov) && (should_do_rcs || m_Settings.m_iRcsType == 1)) {
    RCS(angles, m_Target, should_do_rcs);
  }

  if (m_Target && IsNotSilent(fov)) {
    Smooth(current, angles, angles);
  }

  FixAngles(angles);
  pCmd->m_angViewAngles = angles;
  if (IsNotSilent(fov)) {
    g_Globals.m_Interfaces.m_EngineClient->SetViewAngles(&angles);
  }

  silent_enabled = false;
  /*if ( g_Globals.m_LocalPlayer->m_hActiveWeapon()->Is() && g_Settings->m_bLegitPistol ) 
  { 
    float server_time = g_Globals.m_LocalPlayer->m_nTickBase() * m_Globals()->m_flIntervalPerTick;
    float next_shot = g_Globals.m_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time; 
    if ( next_shot > 0 ) 
        pCmd->m_nButtons &= ~IN_ATTACK;
  }*/
}