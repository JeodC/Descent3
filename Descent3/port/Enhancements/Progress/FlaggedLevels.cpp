#include <algorithm>
#include <iomanip>
#include <sstream>

#include "game.h"
#include "grdefs.h"
#include "LoadLevel.h"
#include "Mission.h"
#include "newui_core.h"
#include "pilot.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/Settings/Settings.h"

static nlohmann::json &Finished() {
  char pilot_name[PILOT_STRING_SIZE];
  Current_pilot.get_name(pilot_name);
  return Settings::Root()["Progress"][pilot_name][Current_mission.name];
}

static void RecordFinished() {
  if (Game_mode & GM_MULTI) {
    return;
  }
  int final_level = Current_mission.num_levels;
  for (int i = 0; i < Current_mission.num_levels; i++) {
    if (Current_mission.levels[i].flags & LVLFLAG_FINAL) {
      final_level = i + 1;
      break;
    }
  }
  nlohmann::json &finished = Finished();
  const int level = Current_mission.cur_level;
  if (level < final_level && level > (finished.is_number() ? finished.get<int>() : 0)) {
    finished = level;
    Settings::Save();
  }
}

static void AddFlaggedLevels(newuiListBox *list, int highest_allowed, int max_level) {
  if (max_level == -1) {
    return;
  }
  const nlohmann::json &finished = Finished();
  const int last = std::min(finished.is_number() ? finished.get<int>() + 1 : 0, Current_mission.num_levels);
  for (int i = highest_allowed; i < last; i++) {
    level_info info{};
    LoadLevelInfo(Current_mission.levels[i].filename, info);
    std::stringstream ss;
    ss << (char)GR_COLOR_CHAR << (char)120 << (char)120 << (char)120 << std::setw(2) << std::setfill('0') << i + 1
       << ". " << info.name;
    list->AddItem(ss.str().c_str());
  }
}

static void RegisterFlaggedLevels() {
  COND_HOOK(OnMissionStatusUpdate, true, RecordFinished);
  COND_HOOK(OnLevelWarpListFilled, true, AddFlaggedLevels);
}

static RegisterPortInitFunc initFunc(RegisterFlaggedLevels);
