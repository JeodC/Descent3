#include "SaveSlots.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <system_error>

#include "cfile.h"
#include "gamesave.h"
#include "gamesequence.h"
#include "hud.h"
#include "Mission.h"
#include "newui.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "stringtable.h"

// gamesave.cpp
void LoadGameFromPath(const std::filesystem::path &pathname);

static int Picked_slot = -1;

static std::filesystem::path SlotPath(int slot) {
  char filename[16];
  snprintf(filename, sizeof(filename), "saveg%03d", slot);
  return cf_GetWritableBaseDirectory() / "savegame" / filename;
}

bool SaveSlot_Read(int slot, char *description) { return GetGameStateInfo(SlotPath(slot), description); }

void SaveSlot_Save(int slot) {
  Picked_slot = slot;
  Game_interface_mode = GAME_SAVE_INTERFACE;
}

void SaveSlot_Load(int slot) { LoadGameFromPath(SlotPath(slot)); }

void SaveSlot_Delete(int slot) {
  std::filesystem::path path = SlotPath(slot);
  std::error_code ec;
  std::filesystem::remove(path, ec);
  std::filesystem::remove(path.replace_extension(".cnt"), ec);
  if (Quicksave_game_slot == slot) {
    Quicksave_game_slot = -1;
  }
}

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

// SaveGameDialog
static void SavePickedSlot() {
  const std::filesystem::path path = SlotPath(Picked_slot);
  std::error_code ec;
  std::filesystem::create_directories(path.parent_path(), ec);
  char desc[GAMESAVE_DESCLEN + 1];
  NameSave(desc, sizeof(desc));
  if (SaveGameState(path, desc)) {
    AddHUDMessage(TXT_QUICKSAVE);
    Quicksave_game_slot = Picked_slot;
  } else {
    DoMessageBox("", TXT_SAVEGAMEFAILED, MSGBOX_OK);
  }
  Picked_slot = -1;
}

static void RegisterSaveSlots() {
  COND_VB_SHOULD(VB_SAVE_DESCRIPTION, true, {
    char *desc = va_arg(args, char *);
    NameSave(desc, va_arg(args, int));
  });
  COND_VB_SHOULD(VB_SHOW_SAVE_DIALOG, true, {
    if (Picked_slot != -1) {
      SavePickedSlot();
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterSaveSlots);
