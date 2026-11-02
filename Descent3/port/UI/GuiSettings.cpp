#include "Gui.h"

#include <cmath>
#include <string>
#include <vector>

#include <imgui.h>

#include "appdatabase.h"
#include "config.h"
#include "ctlconfig.h"
#include "d3music.h"
#include "descent.h"
#include "game.h"
#include "gamesequence.h"
#include "hlsoundlib.h"
#include "hud.h"
#include "networking.h"
#include "physics.h"
#include "pilot.h"
#include "port/Enhancements/Graphics/HideCursor.h"
#include "port/Enhancements/Graphics/Resolution.h"
#include "port/Enhancements/Graphics/Scaling.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"
#include "renderer.h"
#include "SmallViews.h"
#include "terrain.h"
#include "UIWidgets.h"
#include "voice.h"

using namespace UIWidgets;

static const std::vector<const char *> Off_on = {"Off", "On"};
static const std::vector<const char *> Off_text_graphical = {"Off", "Text", "Graphical"};
static const std::vector<const char *> Low_medium_high = {"Low", "Medium", "High"};
static const std::vector<const char *> None_medium_high = {"None", "Medium", "High"};

// config.cpp
#define MINIMUM_TERRAIN_DETAIL 4
#define MAXIMUM_TERRAIN_DETAIL 28
#define MINIMUM_RENDER_DIST 80
#define MAXIMUM_RENDER_DIST 200

static void GeneralMenu() {
  static const std::vector<const char *> themes(ColorNames, ColorNames + NUM_THEME_COLORS);

  ImGui::SeparatorText("Menu");
  int theme = Theme;
  if (Combobox("Menu Theme", &theme, themes)) {
    Theme = (Colors)theme;
  }
  SliderInt("Menu Background Opacity", &Gui_menu_opacity, 0, 100, 5, "%d%%");

  ImGui::SeparatorText("Toggles");
  Checkbox("Show Reticle", &Game_toggles.show_reticle);
  Checkbox("Guided Missile in Main View", &Game_toggles.guided_mainview,
           WidgetOptions().Tooltip("Flies guided missiles from the main view instead of a small window"));
  Checkbox("Ship Sounds", &Game_toggles.ship_noises);
  CVarCheckbox("Hide Mouse Cursor", CVAR_HIDE_CURSOR, HIDE_CURSOR_DEFAULT,
               WidgetOptions().Tooltip("Leaves the mouse cursor out of menus and the TelCom"));
}

static void GeneralFlight() {
  static const std::vector<const char *> sides = {"Off", "Left", "Right"};

  ImGui::SeparatorText("Auto-Leveling");
  int terrain = Default_player_terrain_leveling;
  if (Combobox("Outdoors", &terrain, None_medium_high)) {
    Default_player_terrain_leveling = terrain;
  }
  int mine = Default_player_room_leveling;
  if (Combobox("Indoors", &mine, None_medium_high)) {
    Default_player_room_leveling = mine;
  }

  ImGui::SeparatorText("Missile Camera");
  int side = (Missile_camera_window == SVW_LEFT) ? 1 : (Missile_camera_window == SVW_RIGHT) ? 2 : 0;
  if (Combobox("Window", &side, sides, WidgetOptions().Tooltip("Follows fired missiles in a small window"))) {
    Missile_camera_window = (side == 1) ? SVW_LEFT : (side == 2) ? SVW_RIGHT : -1;
  }
}

static void Audio() {
  static const std::vector<const char *> qualities = {"Normal", "High"};

  ImGui::SeparatorText("Volume");
  int sound = (int)(Sound_system.GetMasterVolume() * 100.0f + 0.5f);
  if (SliderInt("Sound Effects", &sound, 0, 100, 10, "%d%%")) {
    Sound_system.SetMasterVolume(sound / 100.0f);
  }
  int music = (int)(D3MusicGetVolume() * 100.0f + 0.5f);
  if (SliderInt("Music", &music, 0, 100, 10, "%d%%")) {
    D3MusicSetVolume(music / 100.0f);
  }

  const WidgetOptions mixer = WidgetOptions().Disabled(Gui_InLevel());
  ImGui::SeparatorText("Mixer");
  if (Gui_InLevel()) {
    ImGui::TextDisabled("Can't change during a level.");
  }
  int quality = (Sound_system.GetSoundQuality() == SQT_HIGH) ? 1 : 0;
  if (Combobox("Sound Quality", &quality, qualities, mixer)) {
    Sound_system.SetSoundQuality(quality ? SQT_HIGH : SQT_NORMAL);
  }
  int quantity = Sound_system.GetLLSoundQuantity();
  if (SliderInt("Sounds Played at Once", &quantity, MIN_SOUNDS_MIXED, MAX_SOUNDS_MIXED, 1, "%d", mixer)) {
    Sound_system.SetLLSoundQuantity(quantity);
  }

  ImGui::SeparatorText("Voices");
  Checkbox("Voice Messages", &PlayVoices);
  Checkbox("Powerup Voices", &PlayPowerupVoice,
           WidgetOptions().Tooltip("Announces weapons and items as you pick them up"));
}

static void SetCustomDetail() {
  Default_detail_level = DETAIL_LEVEL_CUSTOM;
  Database->write("PredefDetailSetting", Default_detail_level);
}

static void GraphicsDisplay() {
  static const std::vector<const char *> presets = {"Low", "Medium", "High", "Very High", "Custom"};
  bool changed = false;

  ImGui::SeparatorText("Display");
  changed |= SliderFloat("Gamma", &Render_preferred_state.gamma, 1.0f, 3.0f, 0.1f, "%.1f");

  bool filtering = Render_preferred_state.filtering, vsync = Render_preferred_state.vsync_on;
  if (Checkbox("Bilinear Filtering", &filtering, WidgetOptions().Tooltip("Smooths textures seen up close"))) {
    Render_preferred_state.filtering = filtering;
    changed = true;
  }
  if (Checkbox("VSync", &vsync, WidgetOptions().Tooltip("Holds each frame for the screen's refresh"))) {
    Render_preferred_state.vsync_on = vsync;
    changed = true;
  }

  if (changed) {
    rend_SetPreferredState(&Render_preferred_state);
  }

  ImGui::SeparatorText("Detail");
  int level = Default_detail_level;
  if (Combobox("Detail Preset", &level, presets) && level != DETAIL_LEVEL_CUSTOM) {
    ConfigSetDetailLevel(level);
    Database->write("PredefDetailSetting", Default_detail_level);
  }

  int terrain = MAXIMUM_TERRAIN_DETAIL - (int)Detail_settings.Pixel_error;
  if (SliderInt("Terrain Detail", &terrain, MINIMUM_TERRAIN_DETAIL, MAXIMUM_TERRAIN_DETAIL, 1)) {
    Detail_settings.Pixel_error = (float)(MAXIMUM_TERRAIN_DETAIL - terrain);
    SetCustomDetail();
  }
  int distance = (int)(Detail_settings.Terrain_render_distance / TERRAIN_SIZE + 0.5f);
  if (SliderInt("Render Distance", &distance, MINIMUM_RENDER_DIST, MAXIMUM_RENDER_DIST, 10)) {
    Detail_settings.Terrain_render_distance = (float)(distance * TERRAIN_SIZE);
    SetCustomDetail();
  }
  int complexity = Detail_settings.Object_complexity;
  if (Combobox("Object Complexity", &complexity, Low_medium_high)) {
    Detail_settings.Object_complexity = complexity;
    SetCustomDetail();
  }
}

static bool Resized = false;

void GuiSettings_Close() {
  if (Resized) {
    Resolution_Apply();
  }
  Resized = false;
}

static void GraphicsResolution() {
  static const std::vector<const char *> fill_choices = {"Keep Aspect", "Fill Screen"};

  std::vector<const char *> aspects;
  for (int i = 0; i < N_VIEW_ASPECTS; i++) {
    aspects.push_back(View_aspects[i].name);
  }

  int screen_w, screen_h;
  Resolution_ScreenSize(&screen_w, &screen_h);
  std::vector<std::string> height_names = {"Screen (" + std::to_string(screen_h) + "p)"};
  std::vector<int> heights = {0};
  int height = 0;
  for (int i = 0; i < N_VIEW_HEIGHTS && View_heights[i] < screen_h; i++) {
    if (View_heights[i] == CVarGetInteger(CVAR_VIEW_HEIGHT, 0)) {
      height = (int)heights.size();
    }
    heights.push_back(View_heights[i]);
    height_names.push_back(std::to_string(View_heights[i]) + "p");
  }
  std::vector<const char *> height_items;
  for (const std::string &name : height_names) {
    height_items.push_back(name.c_str());
  }

  ImGui::SeparatorText("Game View");
  Resized |= CVarCombobox("Aspect Ratio", CVAR_VIEW_ASPECT, 0, aspects);
  if (Combobox("Vertical Resolution", &height, height_items,
               WidgetOptions().Tooltip("Fewer lines draw faster and look blockier"))) {
    CVarSetInteger(CVAR_VIEW_HEIGHT, heights[height]);
    PortInit::Init(CVAR_VIEW_HEIGHT);
    Resized = true;
  }
  CVarCheckbox("Pixel Perfect", CVAR_PIXEL_PERFECT, false,
               WidgetOptions().Tooltip("Scales up by whole numbers only, for sharp pixels"));
  int view_w, view_h;
  Resolution_ViewSize(&view_w, &view_h);
  const bool pixel_perfect = CVarGetInteger(CVAR_PIXEL_PERFECT, 0);
  const bool screen_shape = view_w * screen_h == view_h * screen_w;
  CVarCheckbox("Stretch to Fill", CVAR_STRETCH, false,
               WidgetOptions()
                   .Disabled(screen_shape || pixel_perfect)
                   .Tooltip("Stretches a game view whose aspect ratio isn't the screen's over the whole screen"));
  if (screen_shape || pixel_perfect) {
    ImGui::TextDisabled(pixel_perfect ? "Pixel Perfect keeps the view's shape." : "The view is the screen's shape.");
  }
  ImGui::TextDisabled(Resized && GetScreenMode() == SM_GAME ? "Draws %d x %d once the menu closes" : "Draws %d x %d",
                      view_w, view_h);

  ImGui::SeparatorText("Menus and Movies");
  CVarCombobox("Menus", CVAR_MENU_FILL, FILL_DEFAULT, fill_choices,
               WidgetOptions().Tooltip("Menus, loading screens and the TelCom"));
  CVarCombobox("Movies", CVAR_MOVIE_FILL, FILL_DEFAULT, fill_choices);
}

static void GraphicsEffects() {
  struct tToggle {
    const char *name;
    bool *value;
    const char *tooltip;
  };
  const tToggle toggles[] = {
      {"Specular Mapping", &Detail_settings.Specular_lighting, "Shine on metal surfaces"},
      {"Fast Headlight", &Detail_settings.Fast_headlight_on,
       "A cheaper headlight that lights only the spot it points at"},
      {"Mirrored Surfaces", &Detail_settings.Mirrored_surfaces, "Reflections in mirrored floors and walls"},
      {"Dynamic Lighting", &Detail_settings.Dynamic_lighting, "Explosions and engines light up what's around them"},
      {"Fog", &Detail_settings.Fog_enabled, "Fog in the rooms and outdoor areas that have it"},
      {"Light Coronas", &Detail_settings.Coronas_enabled, "Glows around lights"},
      {"Procedural Textures", &Detail_settings.Procedurals_enabled, "Animated textures such as fire and water"},
      {"Powerup Halos", &Detail_settings.Powerup_halos, "Glows around powerups"},
      {"Scorch Marks", &Detail_settings.Scorches_enabled, "Marks that explosions and shots leave on walls"},
      {"Weapon Effects", &Detail_settings.Weapon_coronas_enabled, "Glows and trails on weapon fire"},
  };

  ImGui::SeparatorText("Effects");
  for (const tToggle &toggle : toggles) {
    if (Checkbox(toggle.name, toggle.value, WidgetOptions().Tooltip(toggle.tooltip))) {
      SetCustomDetail();
    }
  }
}

static void Hud() {
  struct tHudItem {
    const char *name;
    uint16_t flags;
    bool graphical;
  };
  static const tHudItem items[] = {
      {"Ship Status", STAT_SHIP, true},
      {"Shields and Energy", STAT_SHIELDS | STAT_ENERGY, true},
      {"Weapon Loads", STAT_PRIMARYLOAD | STAT_SECONDARYLOAD, true},
      {"Afterburner", STAT_AFTERBURN, true},
      {"Warnings", STAT_WARNING, true},
      {"Countermeasures", STAT_CNTRMEASURE, true},
      {"Inventory", STAT_INVENTORY, false},
  };

  static const std::vector<const char *> views = {"Cockpit", "Fullscreen HUD"};

  if (!Gui_PilotPage()) {
    return;
  }
  uint8_t pilot_mode;
  uint16_t stat, grstat;
  Current_pilot.get_hud_data(&pilot_mode, &stat, &grstat);

  const bool flying = Gui_InLevel() && GetScreenMode() == SM_GAME;
  const tHUDMode mode = flying ? GetHUDMode() : (tHUDMode)pilot_mode;
  ImGui::SeparatorText("View");
  int view = (mode == HUD_FULLSCREEN) ? 1 : 0;
  if (Combobox("View", &view, views, WidgetOptions().Disabled(mode != HUD_COCKPIT && mode != HUD_FULLSCREEN))) {
    const tHUDMode new_mode = view ? HUD_FULLSCREEN : HUD_COCKPIT;
    if (flying) {
      SetHUDMode(new_mode);
    } else {
      uint8_t new_pilot_mode = new_mode;
      Current_pilot.set_hud_data(&new_pilot_mode);
    }
  }

  const WidgetOptions item_options = WidgetOptions().Disabled(mode == HUD_COCKPIT);
  ImGui::SeparatorText("Fullscreen HUD");
  if (mode == HUD_COCKPIT) {
    ImGui::TextDisabled("Only the fullscreen HUD shows these.");
  }
  for (const tHudItem &item : items) {
    int form = (stat & item.flags) ? 1 : (grstat & item.flags) ? 2 : 0;
    if (!Combobox(item.name, &form, item.graphical ? Off_text_graphical : Off_on, item_options)) {
      continue;
    }
    stat &= ~item.flags;
    grstat &= ~item.flags;
    if (form == 1) {
      stat |= item.flags;
    } else if (form == 2) {
      grstat |= item.flags;
    }
    Current_pilot.set_hud_data(nullptr, &stat, &grstat);
    if (flying && GetHUDMode() == HUD_FULLSCREEN) {
      SetHUDState(stat | STAT_TIMER | (Hud_stat_mask & STAT_FPS), grstat);
    }
  }
}

static void Multiplayer() {
  ImGui::SeparatorText("Network");
  int pps = nw_ReccomendPPS();
  if (SliderInt("Packets per Second", &pps, CFG_NETWORK_CLIENT_PPS_MIN, CFG_NETWORK_CLIENT_PPS_MAX, 1, "%d",
                WidgetOptions().Tooltip("How often the game asks the server for updates"))) {
    Database->write(CTLCONFIG_PPS_CLIENT_DB_KEY, pps);
  }
}

tGuiHeader GuiSettings_Header() {
  return {"Settings",
          {
              {"General", {GeneralMenu, GeneralFlight}},
              {"Audio", {Audio}},
              {"Graphics", {GraphicsDisplay, GraphicsResolution, GraphicsEffects}},
              {"HUD", {Hud}},
              {"Multiplayer", {Multiplayer}},
          }};
}
