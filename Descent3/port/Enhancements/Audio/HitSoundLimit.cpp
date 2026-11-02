#include "game.h"
#include "hlsoundlib.h"
#include "object.h"
#include "player.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

#define CVAR_HIT_SOUND_LIMIT "gEnhancements.Audio.HitSoundLimit"

// collide.cpp
#define MIN_PLAYER_WALL_SOUND_TIME .1f

static int Hit_sound[MAX_PLAYERS];

static void PlayHitSound(va_list args) {
  object *playerobj = va_arg(args, object *);
  const int sound = va_arg(args, int);
  int &playing = Hit_sound[playerobj->id];
  if (!Sound_system.IsSoundPlaying(playing)) {
    playing = Sound_system.Play3dSound(sound, SND_PRIORITY_HIGHEST, playerobj);
  }
}

static void RegisterHitSoundLimit() {
  const bool limit = CVarGetInteger(CVAR_HIT_SOUND_LIMIT, 1);
  COND_VB_SHOULD(VB_PLAY_PLAYER_HIT_SOUND, limit, {
    PlayHitSound(args);
    *should = false;
  });
  COND_VB_SHOULD(VB_PLAY_CONTACT_SOUND, limit, {
    float &last = Players[va_arg(args, int)].last_hit_wall_sound_time;
    *should = *should && last + MIN_PLAYER_WALL_SOUND_TIME < Gametime;
    if (*should) {
      last = Gametime;
    }
  });
  COND_VB_SHOULD(VB_WALL_DAMAGE_PLAYS_SOUND, limit, { *should = false; });
}

static RegisterPortInitFunc initFunc(RegisterHitSoundLimit, {CVAR_HIT_SOUND_LIMIT});
