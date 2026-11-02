#include <algorithm>
#include <cstring>
#include <string>
#include <utility>

#include "Mission.h"
#include "newui_core.h"
#include "pilot.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"

static int CoreRank(const std::filesystem::path &file) {
  const std::string name = file.u8string();
  if (!stricmp(name.c_str(), "d3.mn3")) {
    return 0;
  }
  if (!stricmp(name.c_str(), "merc.mn3")) {
    return 1;
  }
  return 2;
}

static void SortMissions(newuiListBox *list, std::vector<std::filesystem::path> *files) {
  std::vector<std::pair<std::filesystem::path, std::string>> missions;
  for (const std::filesystem::path &file : *files) {
    tMissionInfo info{};
    GetMissionInfo(file, &info);
    missions.emplace_back(file, info.name);
  }
  std::sort(missions.begin(), missions.end(), [](const auto &a, const auto &b) {
    const int rank_a = CoreRank(a.first), rank_b = CoreRank(b.first);
    return rank_a != rank_b ? rank_a < rank_b : stricmp(a.second.c_str(), b.second.c_str()) < 0;
  });
  list->RemoveAll();
  files->clear();
  for (const auto &mission : missions) {
    files->push_back(mission.first);
    list->AddItem(mission.second.c_str());
  }
}

static void OfferFinishedMission(va_list args) {
  int *highest = va_arg(args, int *);
  const int index = Current_pilot.find_mission_data(Current_mission.name);
  tMissionData data{};
  if (index != -1) {
    Current_pilot.get_mission_data(index, &data);
  }
  if (data.finished) {
    *highest = Current_mission.num_levels;
  }
}

static void RegisterNewGame() {
  COND_VB_SHOULD(VB_SORT_MISSION_LIST, true, { *should = false; });
  COND_HOOK(OnMissionListFilled, true, SortMissions);
  COND_VB_SHOULD(VB_NEW_GAME_HIGHEST_LEVEL, true, { OfferFinishedMission(args); });
}

static RegisterPortInitFunc initFunc(RegisterNewGame);
