#include "Gui.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>
#include <imgui_internal.h>

#include "AIMain.h"
#include "cfile.h"
#include "controls.h"
#include "ddio_common.h"
#include "game.h"
#include "gamecinematics.h"
#include "gamesave.h"
#include "gamesequence.h"
#include "grdefs.h"
#include "hlsoundlib.h"
#include "levelgoal.h"
#include "LoadLevel.h"
#include "Mission.h"
#include "multi.h"
#include "object.h"
#include "osiris_share.h"
#include "player.h"
#include "port/Enhancements/Cheats/Cheats.h"
#include "port/Enhancements/Menus/SaveSlots.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/Input/Keys.h"
#include "port/PortInit.hpp"
#include "ship.h"
#include "SmallViews.h"
#include "sounds.h"
#include "stringtable.h"
#include "UIWidgets.h"
#include "weapon.h"

using namespace UIWidgets;

static std::string PlainText(const char *text) {
  std::string plain;
  for (const char *c = text; *c; c++) {
    if (*c == GR_COLOR_CHAR) {
      for (int i = 0; i < 3 && c[1]; i++) {
        c++;
      }
    } else if ((unsigned char)*c >= ' ' || *c == '\n') {
      plain += *c;
    }
  }
  return plain;
}

static bool Appeared(int &drawn) {
  const bool appeared = ImGui::GetFrameCount() != drawn + 1;
  drawn = ImGui::GetFrameCount();
  return appeared;
}

static void Objectives() {
  bool any = false;
  for (bool secondary : {false, true}) {
    bool titled = false;
    for (int i = 0; i < Level_goals.GetNumGoals(); i++) {
      int status;
      Level_goals.GoalStatus(i, LO_GET_SPECIFIED, &status);
      if (!(status & LGF_ENABLED) || ((status & LGF_SECONDARY_GOAL) != 0) != secondary) {
        continue;
      }
      if (!titled) {
        ImGui::SeparatorText(secondary ? "Secondary" : "Primary");
        titled = true;
      }
      any = true;

      char text[1024];
      Level_goals.GoalGetName(i, text, sizeof(text));
      const ImVec4 color = (status & LGF_COMPLETED) ? ImVec4(0.4f, 0.9f, 0.4f, 1.0f)
                           : (status & LGF_FAILED)  ? ImVec4(0.9f, 0.4f, 0.4f, 1.0f)
                                                    : ImGui::GetStyleColorVec4(ImGuiCol_Text);
      const char *outcome = (status & LGF_COMPLETED) ? " (done)" : (status & LGF_FAILED) ? " (failed)" : "";
      ImGui::PushStyleColor(ImGuiCol_Text, color);
      ImGui::TextWrapped("%s%s", PlainText(text).c_str(), outcome);
      ImGui::PopStyleColor();

      Level_goals.GoalGetDesc(i, text, sizeof(text));
      if (*text) {
        ImGui::Indent();
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("%s", PlainText(text).c_str());
        ImGui::PopStyleColor();
        ImGui::Unindent();
      }
      ImGui::Spacing();
    }
  }
  if (!any) {
    ImGui::TextDisabled("No objectives yet.");
  }
}

// hudmessage.cpp
const char *GetHUDMessage(int index);
const char *GetGameMessage(int index);

static void Messages(const char *(*get)(int index), const char *none) {
  int count = 0;
  while (get(count)) {
    count++;
  }
  if (!count) {
    ImGui::TextDisabled("%s", none);
  }
  for (int i = count - 1; i >= 0; i--) {
    ImGui::TextWrapped("%s", PlainText(get(i)).c_str());
    ImGui::Spacing();
  }
}

static void ShipsLog() { Messages(GetGameMessage, "Nothing logged yet."); }

static void HudMessages() { Messages(GetHUDMessage, "No messages yet."); }

// TelComAutoMap.cpp
void TCAMStart();
void TCAMStop();
void TCAMMove(float forward, float vertical, float sideways, float pitch, float heading, float frametime);
void TCAMDraw(int x1, int y1, int x2, int y2);
extern int AM_terrain, AM_center_on_player;

// HardwareOpenGL.cpp
unsigned rend_StartOffscreen(int width, int height);
void rend_EndOffscreen();

static bool Briefing_only = false;
static bool Automap_started = false;

static float Stick(ImGuiKey negative, ImGuiKey positive) {
  return ImGui::GetKeyData(positive)->AnalogValue - ImGui::GetKeyData(negative)->AnalogValue;
}

static void TelCom() {
  const bool briefing = Current_level->flags & LVLFLAG_BRIEFING;
  if (Button("Briefing", Inline,
             WidgetOptions().Disabled(!briefing).Tooltip("The briefing from the start of the level"))) {
    Gui_RunAfterClose([] {
      Briefing_only = true;
      Game_interface_mode = GAME_TELCOM_BRIEFINGS;
    });
  }
  ImGui::SameLine();
  if (Button("Center on Ship", Inline)) {
    AM_center_on_player = 1;
  }
  ImGui::SameLine();
  bool terrain = AM_terrain;
  if (Checkbox("Terrain", &terrain)) {
    AM_terrain = terrain;
  }
  ImGui::PushTextWrapPos(0.0f);
  if (!briefing) {
    ImGui::TextDisabled("This level has no briefing.");
  }
  ImGui::TextDisabled("In the briefing, up and down pick a button, left and right turn the page, A presses and B "
                      "closes it.");
  ImGui::TextDisabled("On the map, the left stick moves, the right stick turns, and the triggers rise and sink.");
  ImGui::PopTextWrapPos();

  if (!Automap_started) {
    TCAMStart();
    Automap_started = true;
  }
  ct_type ctype[CTLBINDS_PER_FUNC];
  ct_config_data data;
  uint8_t flags[CTLBINDS_PER_FUNC];
  Controller->get_controller_function(ctfPITCH_DOWNAXIS, ctype, &data, flags);
  const float pitch =
      Stick(ImGuiKey_GamepadRStickUp, ImGuiKey_GamepadRStickDown) * ((flags[0] & CTFNF_INVERT) ? -1 : 1);
  TCAMMove(Stick(ImGuiKey_GamepadLStickDown, ImGuiKey_GamepadLStickUp), Stick(ImGuiKey_GamepadL2, ImGuiKey_GamepadR2),
           Stick(ImGuiKey_GamepadLStickLeft, ImGuiKey_GamepadLStickRight), pitch,
           Stick(ImGuiKey_GamepadRStickLeft, ImGuiKey_GamepadRStickRight), ImGui::GetIO().DeltaTime);

  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  const float bottom = viewport->WorkPos.y + viewport->WorkSize.y - ImGui::GetStyle().WindowPadding.y * 2;
  const int width = (int)ImGui::GetContentRegionAvail().x;
  const int height = (int)std::min(width * 3.0f / 4.0f, bottom - ImGui::GetCursorScreenPos().y);
  if (width < 1 || height < 1) {
    return;
  }
  const unsigned texture = rend_StartOffscreen(width, height);
  TCAMDraw(0, 0, width - 1, height - 1);
  rend_EndOffscreen();
  ImGui::Image((ImTextureID)(uintptr_t)texture, ImVec2((float)width, (float)height), ImVec2(0.0f, 1.0f),
               ImVec2(1.0f, 0.0f));
}

static void CargoShip() {
  player &plr = Players[Player_num];
  const ship &shp = Ships[plr.ship_index];

  ImGui::SeparatorText("Ship Status");
  ImGui::Text("Shields: %d%%", (int)(std::max(Player_object->shields, 0.0f) / INITIAL_SHIELDS * 100.0f));
  ImGui::Text("Energy: %d%%", (int)(std::max(plr.energy, 0.0f) / INITIAL_ENERGY * 100.0f));

  for (bool secondary : {false, true}) {
    ImGui::SeparatorText(secondary ? "Secondaries" : "Primaries");
    const int first = secondary ? SECONDARY_INDEX : PRIMARY_INDEX;
    for (int i = first; i < first + MAX_PRIMARY_WEAPONS; i++) {
      if (!(plr.weapon_flags & HAS_FLAG(i))) {
        continue;
      }
      if (secondary || shp.static_wb[i].ammo_usage) {
        ImGui::Text("%s: %d", TXT(Static_weapon_names_msg[i]), plr.weapon_ammo[i]);
      } else {
        ImGui::TextUnformatted(TXT(Static_weapon_names_msg[i]));
      }
    }
  }
}

static void CargoItems() {
  for (bool countermeasures : {true, false}) {
    ImGui::SeparatorText(countermeasures ? "Countermeasures" : "Inventory");
    Inventory &inventory = countermeasures ? Players[Player_num].counter_measures : Players[Player_num].inventory;
    std::vector<tInvenList> items(inventory.Size());
    int selected;
    inventory.GetInventoryItemList(items.data(), (int)items.size(), &selected);
    if (items.empty()) {
      ImGui::TextDisabled("None");
    }
    for (int i = 0; i < (int)items.size(); i++) {
      char label[64];
      snprintf(label, sizeof(label), "%s: %d##%d", TitleCase(items[i].hud_name).c_str(), items[i].amount, i);
      if (Button(label, Fill, WidgetOptions().Disabled(!items[i].selectable).Color(i == selected ? Theme : DarkGray))) {
        inventory.GotoPos(i);
        Sound_system.Play2dSound(countermeasures ? SOUND_CHANGE_COUNTERMEASURE : SOUND_CHANGE_INVENTORY);
      }
    }
  }
}

// AIGame.cpp
#define GBC_EXIT_SHIP 0
#define GBC_RETURN_TO_SHIP 43

static gb_menu Guidebot_menu;

static void RunGuidebotCommand(int index) {
  gb_com command;
  command.action = COM_DO_ACTION;
  command.index = Guidebot_menu.command_id[index];
  command.ptr = nullptr;
  if (object *buddy = ObjGet(Buddy_handle[Player_num])) {
    AINotify(buddy, AIN_USER_DEFINED, &command);
  }
}

static void ReadGuidebotMenu() {
  Guidebot_menu.num_commands = 0;
  if (object *buddy = ObjGet(Buddy_handle[Player_num])) {
    gb_com command;
    command.action = COM_GET_MENU;
    command.ptr = &Guidebot_menu;
    AINotify(buddy, AIN_USER_DEFINED, &command);
  }
}

static void GuideBot() {
  if ((Game_mode & GM_MULTI) && !(Netgame.flags & NF_ALLOWGUIDEBOT)) {
    ImGui::TextDisabled("This game has no Guide-Bot.");
    return;
  }
  if ((Game_mode & GM_MULTI) && Netgame.local_role == LR_CLIENT) {
    if (Button("Guide-Bot Menu", Fill)) {
      Gui_RunAfterClose([] { Game_interface_mode = GAME_BUDDY_INTERFACE; });
    }
    return;
  }

  ReadGuidebotMenu();
  if (!Guidebot_menu.num_commands) {
    ImGui::TextDisabled("%s", TXT_NOGUIDEBOT);
    return;
  }

  ImGui::SeparatorText(PlainText(Guidebot_menu.title).c_str());
  for (int i = 0; i < Guidebot_menu.num_commands; i++) {
    if (Guidebot_menu.command_type[i] == GBCT_STRING_DIALOG) {
      continue;
    }
    ImGui::PushID(i);
    if (Button(PlainText(Guidebot_menu.command_text[i]).c_str())) {
      const int id = Guidebot_menu.command_id[i];
      if (id == GBC_EXIT_SHIP || id == GBC_RETURN_TO_SHIP) {
        RunGuidebotCommand(i);
      } else {
        Gui_RunAfterClose([i] { RunGuidebotCommand(i); });
      }
    }
    ImGui::PopID();
  }
}

// GameLoop.cpp
extern int Camera_view_mode[];
void SelectNextCameraView(int window);
enum { CV_NONE = 0, CV_REARVIEW = 1, CV_MARKER1 = 2, CV_GUIDEBOT = 10 };

static int CameraView(int window) {
  return GetSmallViewer(window) == OBJECT_HANDLE_NONE ? CV_NONE : Camera_view_mode[window];
}

// SelectNextCameraView
static std::vector<std::pair<int, std::string>> CameraViews() {
  std::vector<std::pair<int, std::string>> views = {{CV_NONE, "Off"}, {CV_REARVIEW, TXT_VIEW_REAR}};
  const object *buddy = ObjGet(Buddy_handle[Player_num]);
  if (buddy && buddy->type == OBJ_ROBOT) {
    views.emplace_back(CV_GUIDEBOT, TXT_VIEW_GUIDEBOT);
  }
  const int max = (Game_mode & GM_MULTI) ? 2 : 8;
  for (int num = 0; num < max && Players[Player_num].num_markers > 0; num++) {
    bool found = false;
    for (int i = 0; i <= Highest_object_index && !found; i++) {
      found = Objects[i].type == OBJ_MARKER && Objects[i].id == Player_num * 2 + num;
    }
    if (!found) {
      break;
    }
    char name[32];
    snprintf(name, sizeof(name), TXT_VIEW_MARKER, num + 1);
    views.emplace_back(CV_MARKER1 + num, name);
  }
  return views;
}

static void CockpitWindows() {
  const std::vector<std::pair<int, std::string>> views = CameraViews();
  std::vector<const char *> names;
  for (const auto &view : views) {
    names.push_back(view.second.c_str());
  }
  ImGui::SeparatorText("Cockpit Windows");
  for (int window : {SVW_LEFT, SVW_RIGHT}) {
    int index = 0;
    for (int i = 0; i < (int)views.size(); i++) {
      if (views[i].first == CameraView(window)) {
        index = i;
      }
    }
    if (Combobox(window == SVW_LEFT ? "Left Window" : "Right Window", &index, names)) {
      for (int step = 0; step < (int)views.size() && CameraView(window) != views[index].first; step++) {
        SelectNextCameraView(window);
      }
    }
  }
}

static char Slot_description[GAMESAVE_SLOTS][GAMESAVE_DESCLEN + 1];
static bool Slot_used[GAMESAVE_SLOTS];
static int Slots_drawn = -2;

static void SavedGames() {
  static int picked = 0;

  if (Game_mode & GM_MULTI) {
    ImGui::TextDisabled("Multiplayer games can't be saved or loaded.");
    return;
  }
  if (Appeared(Slots_drawn)) {
    for (int i = 0; i < GAMESAVE_SLOTS; i++) {
      Slot_used[i] = SaveSlot_Read(i, Slot_description[i]);
    }
  }

  ImGui::SeparatorText("Saved Games");
  const char *question = nullptr;
  if (ImGui::BeginTable("##slots", 4, ImGuiTableFlags_SizingStretchProp)) {
    ImGui::TableSetupColumn("##description", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("##save", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("##load", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("##delete", ImGuiTableColumnFlags_WidthFixed);
    for (int i = 0; i < GAMESAVE_SLOTS; i++) {
      ImGui::PushID(i);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      if (Slot_used[i]) {
        ImGui::Text("%d. %s", i + 1, Slot_description[i]);
      } else {
        ImGui::TextDisabled("%d. Empty", i + 1);
      }

      ImGui::TableNextColumn();
      if (Button("Save", Inline, WidgetOptions().Disabled(Cinematic_inuse))) {
        picked = i;
        if (Slot_used[i]) {
          question = "Overwrite";
        } else {
          Gui_RunAfterClose([i] { SaveSlot_Save(i); });
        }
      }
      ImGui::TableNextColumn();
      if (Button("Load", Inline, WidgetOptions().Disabled(!Slot_used[i]))) {
        picked = i;
        question = "Load";
      }
      ImGui::TableNextColumn();
      if (Button("Delete", Inline, WidgetOptions().Disabled(!Slot_used[i]).Color(Red))) {
        picked = i;
        question = "Delete";
      }
      ImGui::PopID();
    }
    ImGui::EndTable();
  }

  if (question) {
    ImGui::OpenPopup(question);
  }
  if (Confirm("Overwrite", "Save over this game?", Slot_description[picked], "Save")) {
    Gui_RunAfterClose([slot = picked] { SaveSlot_Save(slot); });
  }
  if (Confirm("Load", "Load this game? Progress since your last save is lost.", Slot_description[picked], "Load")) {
    Gui_RunAfterClose([slot = picked] { SaveSlot_Load(slot); });
  }
  if (Confirm("Delete", "Delete this game?", Slot_description[picked], "Delete")) {
    SaveSlot_Delete(picked);
    Slots_drawn = -2;
  }
}

static void NetgameScreens() {
  if (!(Game_mode & GM_MULTI)) {
    ImGui::TextDisabled("Only in multiplayer games.");
    return;
  }
  if (Button("On-Screen Menu", Fill, WidgetOptions().Tooltip("Teams, observing and the netgame's options"))) {
    Gui_RunAfterClose([] { Input_KeyPress(KEY_F6); });
  }
  if (Button("Score Screen", Fill, WidgetOptions().Tooltip("Shows or hides the scores"))) {
    Gui_RunAfterClose([] { Input_KeyPress(KEY_F7); });
  }
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextDisabled("In the on-screen menu, up and down move, right opens an entry, left goes back, A picks and B "
                      "closes it.");
  ImGui::PopTextWrapPos();
}

// DisplayLevelWarpDlg
static std::vector<std::string> LevelNames() {
  int library = -1;
  if (!stricmp(Current_mission.filename, "d3.mn3")) {
    library = cf_OpenLibrary(std::filesystem::path("missions") / "d3_2.mn3");
  } else if (!stricmp(Current_mission.filename, "d3_2.mn3")) {
    library = cf_OpenLibrary(std::filesystem::path("missions") / "d3.mn3");
  }
  std::vector<std::string> names;
  for (int i = 0; i < Current_mission.num_levels; i++) {
    level_info info{};
    LoadLevelInfo(Current_mission.levels[i].filename, info);
    char name[128];
    snprintf(name, sizeof(name), "%02d. %s", i + 1, info.name);
    names.push_back(name);
  }
  if (library != -1) {
    cf_CloseLibrary(library);
  }
  return names;
}

static std::vector<std::string> Level_names;
static int Warp_level = 0;

static void Cheats() {
  if (Game_mode & GM_MULTI) {
    ImGui::TextDisabled("Cheats are off in multiplayer games.");
    return;
  }
  ImGui::TextDisabled("Any cheat clears your score, and the levels after it don't count toward New Game.");

  ImGui::SeparatorText("Ship");
  bool invulnerable = Players[Player_num].flags & PLAYER_FLAGS_INVULNERABLE;
  if (Checkbox("Invulnerability", &invulnerable)) {
    Cheat_Invulnerability();
  }
  bool cloaked = Player_object->effect_info->type_flags & EF_CLOAKED;
  if (Checkbox("Cloak", &cloaked)) {
    Cheat_Cloak();
  }
  if (Button("All Weapons", Fill,
             WidgetOptions().Tooltip("Every weapon with full ammo, quad lasers, countermeasures, and full shields "
                                     "and energy"))) {
    Cheat_AllWeapons();
  }

  ImGui::SeparatorText("Level");
  if (Button("Full Map", Fill, WidgetOptions().Tooltip("Shows every room on the automap but the secret ones"))) {
    Cheat_FullMap();
  }
  if (Button("Kill All Robots", Fill, WidgetOptions().Tooltip("Destroys every robot but the Guide-Bot"))) {
    Cheat_KillRobots();
  }

  if (Level_names.empty()) {
    Level_names = LevelNames();
    Warp_level = Current_mission.cur_level - 1;
  }
  std::vector<const char *> levels;
  for (const std::string &name : Level_names) {
    levels.push_back(name.c_str());
  }
  Combobox("Level", &Warp_level, levels);
  if (Button("Warp to Level", Fill)) {
    Gui_RunAfterClose([level = Warp_level + 1] { Cheat_LevelWarp(level); });
  }
}

void GuiMission_Close() {
  Level_names.clear();
  Slots_drawn = -2;
  if (Automap_started) {
    TCAMStop();
    Automap_started = false;
  }
}

tGuiHeader GuiMission_Header() {
  return {"Mission",
          {
              {"Objectives", {Objectives}},
              {"TelCom", {TelCom}},
              {"Cargo", {CargoShip, CargoItems}},
              {"Ship's Log", {ShipsLog}},
              {"HUD Messages", {HudMessages}},
              {"Guide-Bot", {GuideBot}},
              {"Cockpit Windows", {CockpitWindows}},
              {"Saved Games", {SavedGames}},
              {"Netgame", {NetgameScreens}},
              {"Cheats", {Cheats}},
          },
          Gui_InLevel};
}

static void RegisterGuiMission() {
  COND_VB_SHOULD(VB_TELCOM_OPENS_MAIN_MENU, true, {
    if (Briefing_only) {
      Briefing_only = false;
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterGuiMission);
