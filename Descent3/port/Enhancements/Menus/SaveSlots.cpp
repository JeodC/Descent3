#include <cstdio>
#include <cstring>

#include "Mission.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"

static void NameSave(char *desc, int size) {
  const bool retribution = !strcmp(Current_mission.name, "Descent 3: Retribution");
  const char *mission = retribution                                             ? "D3"
                        : !strcmp(Current_mission.name, "Descent 3: Mercenary") ? "Merc"
                                                                                : Current_mission.name;
  if (retribution && Current_mission.cur_level > 15) {
    snprintf(desc, size, "D3 Secret Level %02d", Current_mission.cur_level - 15);
  } else {
    snprintf(desc, size, "%.22s Level %02d", mission, Current_mission.cur_level);
  }
}

static void RegisterSaveSlots() {
  COND_VB_SHOULD(VB_SAVE_DESCRIPTION, true, {
    char *desc = va_arg(args, char *);
    NameSave(desc, va_arg(args, int));
  });
}

static RegisterPortInitFunc initFunc(RegisterSaveSlots);
