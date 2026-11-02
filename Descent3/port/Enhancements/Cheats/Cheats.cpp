#include "Cheats.h"

#include "damage.h"
#include "game.h"
#include "hlsoundlib.h"
#include "hud.h"
#include "object.h"
#include "object_lighting.h"
#include "objinfo.h"
#include "player.h"
#include "port/UI/Gui.h"
#include "robot.h"
#include "room.h"
#include "ship.h"
#include "sounds.h"
#include "stringtable.h"
#include "weapon.h"

extern bool IsCheater;
extern uint8_t AutomapVisMap[MAX_ROOMS];

static void Cheated() {
  IsCheater = true;
  Players[Player_num].score = 0;
}

void Cheat_FullMap() {
  AddHUDMessage(TXT_FULLMAP);
  for (int i = 0; i < MAX_ROOMS; i++) {
    if (!(Rooms[i].flags & RF_SECRET)) {
      AutomapVisMap[i] = 1;
    }
  }
  Sound_system.Play2dSound(SOUND_CHEATER);
  Cheated();
}

void Cheat_Invulnerability() {
  if (Players[Player_num].flags & PLAYER_FLAGS_INVULNERABLE) {
    MakePlayerVulnerable(Player_num);
    AddHUDMessage(TXT_INVULNCHEATON);
  } else {
    MakePlayerInvulnerable(Player_num, 10000);
    AddHUDMessage(TXT_INVULNCHEAT);
    Sound_system.Play2dSound(SOUND_CHEATER);
  }
  Cheated();
}

void Cheat_Cloak() {
  if (Player_object->effect_info->type_flags & EF_CLOAKED) {
    MakeObjectVisible(Player_object);
    AddHUDMessage(TXT_CLOAKCHEATOFF);
  } else {
    MakeObjectInvisible(Player_object, 10000);
    AddHUDMessage(TXT_CLOAKCHEATON);
    Sound_system.Play2dSound(SOUND_CHEATER);
  }
  Cheated();
}

void Cheat_AllWeapons() {
  player &plr = Players[Player_num];
  object *objp = &Objects[plr.objnum];

  plr.weapon_flags = 0xFFFFFFFF;
  for (int i = 0; i < MAX_PLAYER_WEAPONS; i++) {
    if (HAS_FLAG(i)) {
      plr.weapon_ammo[i] = Ships[plr.ship_index].max_ammo[i];
    }
  }
  plr.energy = 200.0f;
  objp->shields = 200.0f;
  plr.afterburn_time_left = AFTERBURN_TIME;
  objp->dynamic_wb[LASER_INDEX].flags |= DWBF_QUAD;
  objp->dynamic_wb[SUPER_LASER_INDEX].flags |= DWBF_QUAD;

  const int quad_id = FindObjectIDName("QuadLaser");
  if (quad_id > -1) {
    plr.inventory.Add(OBJ_POWERUP, quad_id, nullptr, -1, -1, INVF_MISSIONITEM | INVF_TIMEOUTONSPEW);
  }

  const struct {
    const char *weapon, *powerup;
  } countermeasures[] = {{"Chaff", "chaff"},
                         {"Betty", "Betty4pack"},
                         {"SeekerMine", "Seeker3pack"},
                         {"Gunboy", "gunboypowerup"},
                         {"ProxMine", "ProxMinepowerup"}};
  for (int i = 0; i < 30; i++) {
    for (const auto &cm : countermeasures) {
      const int id = FindWeaponName(cm.weapon);
      if (id != -1) {
        plr.counter_measures.Add(OBJ_WEAPON, id, objp, OBJ_POWERUP, FindObjectIDName(cm.powerup));
      }
    }
  }

  AddHUDMessage(TXT_ALLWEAPONS);
  Sound_system.Play2dSound(SOUND_CHEATER);
  Cheated();
}

void Cheat_KillRobots() {
  AddHUDMessage(TXT_ALLROBOTSDEAD);
  Sound_system.Play2dSound(SOUND_CHEATER);
  for (int i = 0; i < MAX_OBJECTS; i++) {
    if ((Objects[i].type == OBJ_ROBOT || (Objects[i].type == OBJ_BUILDING && Objects[i].ai_info)) &&
        Objects[i].id != ROBOT_GUIDEBOT && Objects[i].id != ROBOT_GUIDEBOTRED) {
      SetObjectDeadFlag(&Objects[i]);
    }
  }
  Cheated();
}

void Cheat_LevelWarp(int level) {
  Gui_WarpToLevel(level);
  Cheated();
}
