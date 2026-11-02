#include "Gui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#include <imgui.h>

#include "3d.h"
#include "ddio.h"
#include "descent.h"
#include "difficulty.h"
#include "init.h"
#include "manage.h"
#include "object.h"
#include "pilot.h"
#include "polymodel.h"
#include "renderer.h"
#include "ship.h"
#include "stringtable.h"
#include "UIWidgets.h"
#include "vecmat.h"

using namespace UIWidgets;

// pilot.cpp
std::filesystem::path StripCRCFileName(const std::filesystem::path &src);

// HardwareOpenGL.cpp
unsigned rend_StartOffscreen(int width, int height);
void rend_EndOffscreen();

static void Pilot() {
  if (!Gui_PilotPage()) {
    return;
  }
  char name[PILOT_STRING_SIZE];
  Current_pilot.get_name(name);
  ImGui::SeparatorText(name);

  const std::vector<const char *> difficulties = {TXT_TRAINEE, TXT_ROOKIE, TXT_HOTSHOT, TXT_ACE, TXT_INSANE};
  uint8_t difficulty;
  Current_pilot.get_difficulty(&difficulty);
  int level = difficulty;
  if (Combobox("Difficulty", &level, difficulties)) {
    Current_pilot.set_difficulty(level);
    ingame_difficulty = level;
  }
}

static std::vector<std::filesystem::path> Logos, Taunts;
static bool Files_read = false;

void GuiPilot_Close() { Files_read = false; }

static void ReadCustomFiles() {
  if (Files_read) {
    return;
  }
  Files_read = true;
  Logos.clear();
  Taunts.clear();
  ddio_DoForeachFile(LocalCustomGraphicsDir, std::regex(".+\\.o[ag]f"),
                     [](const std::filesystem::path &path) { Logos.push_back(path.filename()); });
  ddio_DoForeachFile(LocalCustomSoundsDir, std::regex(".+\\.osf"),
                     [](const std::filesystem::path &path) { Taunts.push_back(path.filename()); });
}

static bool FileCombobox(const char *label, char *pick, const std::vector<std::filesystem::path> &files) {
  std::vector<std::string> names = {"None"};
  int index = 0;
  for (size_t i = 0; i < files.size(); i++) {
    names.push_back(StripCRCFileName(files[i]).u8string());
    if (!stricmp(pick, files[i].u8string().c_str())) {
      index = (int)i + 1;
    }
  }
  std::vector<const char *> items;
  for (const std::string &name : names) {
    items.push_back(name.c_str());
  }
  if (!Combobox(label, &index, items)) {
    return false;
  }
  snprintf(pick, PAGENAME_LEN, "%s", index ? files[index - 1].u8string().c_str() : "");
  return true;
}

static void Ship() {
  if (!Gui_PilotPage()) {
    return;
  }
  ReadCustomFiles();

  ImGui::SeparatorText("Multiplayer Ship");
  char pship[PAGENAME_LEN];
  Current_pilot.get_ship(pship);
  std::vector<const char *> ships;
  int ship = 0;
  for (int i = 0; i < MAX_SHIPS; i++) {
    if (!Ships[i].used || (!stricmp(Ships[i].name, "Black Pyro") && !MercInstalled())) {
      continue;
    }
    if (!stricmp(Ships[i].name, pship)) {
      ship = (int)ships.size();
    }
    ships.push_back(Ships[i].name);
  }
  if (Combobox("Ship", &ship, ships)) {
    Current_pilot.set_ship(ships[ship]);
  }

  char logo[PAGENAME_LEN], taunts[4][PAGENAME_LEN];
  Current_pilot.get_multiplayer_data(logo, taunts[0], taunts[1], nullptr, taunts[2], taunts[3]);
  bool changed = FileCombobox("Logo", logo, Logos);

  ImGui::SeparatorText("Audio Taunts");
  for (int i = 0; i < 4; i++) {
    char label[16];
    snprintf(label, sizeof(label), "Taunt %d", i + 1);
    changed |= FileCombobox(label, taunts[i], Taunts);
  }
  if (changed) {
    Current_pilot.set_multiplayer_data(logo, taunts[0], taunts[1], nullptr, taunts[2], taunts[3]);
  }
  ImGui::PushTextWrapPos(0.0f);
  ImGui::TextDisabled("Logos are the .ogf files in gamedata/custom/graphics, and taunts the .osf files in "
                      "gamedata/custom/sounds.");
  ImGui::PopTextWrapPos();
}

static matrix Ship_orient;
static float Ship_drawn = -1.0f;

// UI3DWindow::OnDraw
static void ShipPreview() {
  if (!Gui_PilotLoaded()) {
    return;
  }
  char pship[PAGENAME_LEN];
  Current_pilot.get_ship(pship);
  const int ship = FindShipName(pship);
  if (ship == -1 || Ships[ship].model_handle == -1) {
    return;
  }
  const int model = Ships[ship].model_handle;
  float model_size;
  PageInPolymodel(model, OBJ_PLAYER, &model_size);
  poly_model *pm = GetPolymodelPointer(model);

  const float now = timer_GetTime();
  if (Ship_drawn < 0.0f) {
    vm_AnglesToMatrix(&Ship_orient, 0, 65535 / 2, 0);
    Ship_drawn = now;
  }
  matrix turn, orient;
  vm_AnglesToMatrix(&turn, 0, (angle)((now - Ship_drawn) * (65535 / 360) * 30), 0);
  vm_MatrixMul(&orient, &turn, &Ship_orient);
  vm_Orthogonalize(&orient);
  Ship_orient = orient;
  Ship_drawn = now;

  const vector middle = (pm->mins + pm->maxs) / 2.0f;
  vector diagonal = pm->maxs - pm->mins;
  const float radius = vm_GetMagnitude(&diagonal) / 2.0f;
  const float half_view = D3_DEFAULT_ZOOM * 3.0f / 4.0f;
  matrix inverse = orient;
  vm_TransposeMatrix(&inverse);
  vector pos = -(middle * inverse);

  const int size = (int)std::min(ImGui::GetContentRegionAvail().x, 360.0f * Scale);
  const unsigned texture = rend_StartOffscreen(size, size);
  vector eye{};
  eye.z() = -radius * std::sqrt(1.0f + half_view * half_view) / half_view;
  matrix eye_orient = IDENTITY_MATRIX;
  g3_StartFrame(&eye, &eye_orient, D3_DEFAULT_ZOOM);
  rend_SetFlatColor(0);
  rend_SetZBufferState(1);
  rend_SetAlphaType(AT_CONSTANT_TEXTURE);
  rend_SetAlphaValue(255);
  rend_SetLighting(LS_NONE);
  rend_SetColorModel(CM_MONO);
  rend_SetOverlayType(OT_NONE);
  float normalized_time[MAX_SUBOBJECTS];
  SetNormalizedTimeAnim(0, normalized_time, pm);
  vector light{};
  light.y() = -1.0f;
  light.z() = -1.0f;
  vm_NormalizeVector(&light);
  DrawPolygonModel(&pos, &orient, model, normalized_time, 0, &light, 0.8f, 0.8f, 0.8f);
  g3_EndFrame();
  rend_EndOffscreen();

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - size) / 2);
  ImGui::Image((ImTextureID)(uintptr_t)texture, ImVec2((float)size, (float)size), ImVec2(0.0f, 1.0f),
               ImVec2(1.0f, 0.0f));
}

tGuiHeader GuiPilot_Header() {
  return {"Pilot",
          {
              {"Pilot", {Pilot}},
              {"Ship", {Ship, ShipPreview}},
          },
          [] { return !Gui_InLevel(); }};
}
