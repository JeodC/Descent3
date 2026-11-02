#include "Gui.h"

#include <algorithm>
#include <cstring>
#include <string>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <imgui_internal.h>
#include <SDL3/SDL.h>

#include "controls.h"
#include "ddio.h"
#include "descent.h"
#include "game.h"
#include "gamesequence.h"
#include "init.h"
#include "Mission.h"
#include "montserrat.h"
#include "object.h"
#include "pilot.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/Input/Gamepad.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"
#include "port/Settings/Settings.h"
#include "UIWidgets.h"

using namespace UIWidgets;

#define CVAR_THEME "gSettings.Menu.Theme"
#define CVAR_OPACITY "gSettings.Menu.Opacity"

int Gui_menu_opacity = 85;

static std::vector<tGuiHeader> Headers;
static std::vector<int> Open_section;
static int Open_header = 0;
static SDL_Window *Window = nullptr;
static ImFont *Font_large = nullptr;

static bool Menu_open = false;
static bool Reopened = false;
static bool Close_requested = false;
static bool Suspended_controls = false;
static bool Hand_back_pending = false;
static std::function<void()> Picked_command;

static int Warp_level = 0;
static bool Quitting = false;

static void LoadFonts() {
  int w, h;
  SDL_GetWindowSizeInPixels(Window, &w, &h);
  Scale = std::max(1.0f, h / 720.0f);

  ImFontConfig config;
  config.FontDataOwnedByAtlas = false;
  void *ttf = const_cast<unsigned char *>(montserrat_regular_ttf);
  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->AddFontFromMemoryTTF(ttf, sizeof(montserrat_regular_ttf), 20.0f * Scale, &config);
  Font_large = io.Fonts->AddFontFromMemoryTTF(ttf, sizeof(montserrat_regular_ttf), 24.0f * Scale, &config);

  ImGuiStyle &style = ImGui::GetStyle();
  style.FramePadding = ImVec2(4.0f, 6.0f);
  style.ItemSpacing = ImVec2(8.0f, 6.0f);
  style.ScaleAllSizes(Scale);
}

static void HandBackControls() {
  if (!Hand_back_pending || Gamepad_Held()) {
    return;
  }
  Hand_back_pending = false;
  if (Suspended_controls) {
    ResumeControls();
    Suspended_controls = false;
  }
  if (Picked_command) {
    std::function<void()> command;
    command.swap(Picked_command);
    command();
  }
}

static void CloseMenu() {
  Menu_open = false;
  GuiControls_Close();
  GuiMission_Close();
  GuiPilot_Close();
  GuiSettings_Close();

  if (Gui_PilotLoaded()) {
    SaveControlConfig();
    PltWriteFile(&Current_pilot);
  }
  SaveGameSettings();
  CVarSetInteger(CVAR_THEME, Theme);
  CVarSetInteger(CVAR_OPACITY, Gui_menu_opacity);
  Settings::Save();

  Hand_back_pending = true;
  HandBackControls();
}

static ImU32 EntryColor(bool hovered, bool held) {
  return ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive
                            : hovered         ? ImGuiCol_ButtonHovered
                                              : ImGuiCol_Button);
}

static bool HeaderEntry(const char *label) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  const ImGuiStyle &style = ImGui::GetStyle();
  ImVec2 pos = window->DC.CursorPos;
  const ImGuiID id = window->GetID((std::string(label) + "##Header").c_str());
  const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
  const ImRect bb(pos, pos + label_size + style.FramePadding * 2);
  ImGui::ItemSize(bb, style.FramePadding.y);
  ImGui::ItemAdd(bb, id);
  bool hovered, held;
  const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
  window->DrawList->AddRectFilled(bb.Min, bb.Max, EntryColor(hovered, held), style.FrameRounding);
  RenderText(pos + style.FramePadding, label);
  return pressed;
}

static bool SidebarEntry(const char *label) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  const ImGuiStyle &style = ImGui::GetStyle();
  ImVec2 pos = window->DC.CursorPos;
  const ImGuiID id = window->GetID((std::string(label) + "##Sidebar").c_str());
  const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
  pos.y += style.FramePadding.y;
  pos.x = window->WorkRect.GetCenter().x - label_size.x / 2;
  const ImRect bb(pos - style.FramePadding, pos + label_size + style.FramePadding);
  ImGui::ItemSize(bb, style.FramePadding.y);
  ImGui::ItemAdd(bb, id);
  bool hovered, held;
  const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
  if (pressed) {
    ImGui::MarkItemEdited(id);
  }
  window->DrawList->AddRectFilled(bb.Min, bb.Max, EntryColor(hovered, held), style.FrameRounding);
  RenderText(pos, label);
  return pressed;
}

static bool PickEntry(bool (*entry)(const char *), const char *label, bool picked) {
  PushStyleButton(Theme);
  if (!picked) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  }
  const bool pressed = entry(label);
  if (!picked) {
    ImGui::PopStyleColor();
  }
  PopStyleButton();
  return pressed;
}

static bool Shown(const tGuiHeader &header) { return !header.shown || header.shown(); }

static void DrawBar(ImVec2 size) {
  const ImVec2 pos = ImGui::GetCursorScreenPos();
  ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + size, IM_COL32_WHITE);
  ImGui::Dummy(size);
}

static void DrawMenu() {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, Gui_menu_opacity / 100.0f));
  ImGui::Begin("Menu", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  if (Reopened) {
    Reopened = false;
    if (GImGui->OpenPopupStack.Size > 0) {
      ImGui::ClosePopupToLevel(0, false);
    }
  }
  const ImGuiStyle &style = ImGui::GetStyle();
  ImGui::BeginChild("Menu Block", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

  ImGui::PushFont(Font_large);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Scaled(10.0f, 8.0f));
  ImGui::BeginChild("Header Selection", ImVec2(0.0f, 0.0f),
                    ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize,
                    ImGuiWindowFlags_NoTitleBar);
  for (int i = 0; !Shown(Headers[Open_header]) && i < (int)Headers.size(); i++) {
    Open_header = i;
  }
  bool first = true;
  for (int i = 0; i < (int)Headers.size(); i++) {
    if (!Shown(Headers[i])) {
      continue;
    }
    if (!first) {
      ImGui::SameLine();
    }
    first = false;
    if (PickEntry(HeaderEntry, Headers[i].name, i == Open_header)) {
      Open_header = i;
    }
  }
  ImGui::EndChild();

  const bool in_level = Gui_InLevel();
  const char *labels[] = {"Restart Level", "Quit to Menu", "Close"};
  float buttons_w = 0.0f;
  for (int i = in_level ? 0 : 2; i < 3; i++) {
    buttons_w += ImGui::CalcTextSize(labels[i]).x + style.FramePadding.x * 2.0f + (i < 2 ? style.ItemSpacing.x : 0.0f);
  }
  ImGui::SameLine();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttons_w);
  const char *question = nullptr;
  if (in_level) {
    if (Button(labels[0], Inline, WidgetOptions().Disabled(Game_mode & GM_MULTI))) {
      question = "Restart";
    }
    ImGui::SameLine();
    if (Button(labels[1], Inline)) {
      question = "Quit";
    }
    ImGui::SameLine();
  }
  if (Button(labels[2], Inline, WidgetOptions().Tooltip("Close the menu (SELECT)"))) {
    Close_requested = true;
  }
  ImGui::PopStyleVar();

  DrawBar(ImVec2(ImGui::GetContentRegionAvail().x, 4.0f * Scale));

  const ImVec2 avail = ImGui::GetContentRegionAvail();
  const float sidebar_w = std::min(245.0f * Scale, avail.x * 0.3f);
  const tGuiHeader &header = Headers[Open_header];
  int &open_section = Open_section[Open_header];
  ImGui::SetNextWindowSizeConstraints(ImVec2(sidebar_w, 0.0f), ImVec2(sidebar_w, avail.y));
  ImGui::BeginChild((std::string(header.name) + " Section").c_str(), ImVec2(sidebar_w, avail.y * 3),
                    ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize);
  for (int i = 0; i < (int)header.sections.size(); i++) {
    if (PickEntry(SidebarEntry, header.sections[i].name, i == open_section)) {
      open_section = i;
    }
  }
  ImGui::EndChild();
  ImGui::PopFont();

  ImGui::SameLine();
  DrawBar(ImVec2(4.0f * Scale, avail.y));
  ImGui::SameLine();

  const tGuiSection &section = header.sections[open_section];
  const std::string section_id = std::string(section.name) + " Settings";
  const float content_w = ImGui::GetContentRegionAvail().x;
  const int columns = viewport->WorkSize.x < 800.0f * Scale ? 1 : (int)section.columns.size();
  const float column_w = (content_w - style.ItemSpacing.x * columns) / columns;
  const bool use_columns = columns > 1;
  if (!use_columns) {
    ImGui::SetNextWindowSizeConstraints(ImVec2(content_w, 0.0f), ImVec2(content_w, avail.y));
    ImGui::BeginChild(section_id.c_str(), ImVec2(content_w, avail.y * 4), ImGuiChildFlags_AutoResizeY);
  }
  for (int i = 0; i < (int)section.columns.size(); i++) {
    if (use_columns) {
      ImGui::SetNextWindowSizeConstraints(ImVec2(column_w, 0.0f), ImVec2(column_w, avail.y));
      ImGui::BeginChild((section_id + " Column " + std::to_string(i)).c_str(), ImVec2(column_w, avail.y * 4),
                        ImGuiChildFlags_AutoResizeY);
    }
    section.columns[i]();
    if (use_columns) {
      ImGui::EndChild();
    }
    if (i < columns - 1) {
      ImGui::SameLine();
    }
  }
  if (!use_columns) {
    ImGui::EndChild();
  }

  if (question) {
    ImGui::OpenPopup(question);
  }
  if (Confirm("Restart", "Restart the level?", "Progress in this level is lost.", "Restart")) {
    Gui_RunAfterClose([] { Gui_WarpToLevel(Current_mission.cur_level); });
  }
  if (Confirm("Quit", "Quit to the main menu?", "Progress since your last save is lost.", "Quit")) {
    Gui_RunAfterClose([] {
      Quitting = true;
      Game_interface_mode = GAME_EXIT_CONFIRM;
    });
  }

  ImGui::EndChild();
  ImGui::End();

  if (Close_requested) {
    Close_requested = false;
    CloseMenu();
  }
}

bool Gui_SelectTapped() {
  if (!ImGui::GetCurrentContext() || !ImGui::GetIO().BackendRendererUserData) {
    return false;
  }
  Gui_OpenMenu();
  return true;
}

void Gui_OpenMenu(const char *header, const char *section) {
  for (int i = 0; header && i < (int)Headers.size(); i++) {
    if (strcmp(Headers[i].name, header)) {
      continue;
    }
    Open_header = i;
    for (int j = 0; section && j < (int)Headers[i].sections.size(); j++) {
      if (!strcmp(Headers[i].sections[j].name, section)) {
        Open_section[i] = j;
      }
    }
  }
  if (Menu_open || !ImGui::GetCurrentContext()) {
    return;
  }
  ddio_KeyFlush();
  if (!Font_large) {
    LoadFonts();
  }
  Menu_open = true;
  Hand_back_pending = false;
  Picked_command = nullptr;
  Reopened = true;

  if (Control_poll_flag) {
    SuspendControls();
    Suspended_controls = true;
  }
}

void Gui_RunAfterClose(std::function<void()> command) {
  Picked_command = std::move(command);
  Close_requested = true;
}

bool Gui_FilterEvent(const SDL_Event *event) {
  if (!Menu_open) {
    if (event->type == SDL_EVENT_GAMEPAD_BUTTON_UP || event->type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
      HandBackControls();
    }
    return false;
  }
  if (!ImGui::GetIO().BackendPlatformUserData) {
    CloseMenu();
    return false;
  }

  switch (event->type) {
  case SDL_EVENT_GAMEPAD_BUTTON_UP:
    if (event->gbutton.button == SDL_GAMEPAD_BUTTON_BACK) {
      Close_requested = true;
    }
    break;
  case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
  case SDL_EVENT_GAMEPAD_AXIS_MOTION:
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
  case SDL_EVENT_TEXT_INPUT:
  case SDL_EVENT_MOUSE_MOTION:
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP:
  case SDL_EVENT_MOUSE_WHEEL:
    break;
  default:
    return false;
  }
  ImGui_ImplSDL3_ProcessEvent(event);
  return true;
}

bool Gui_InLevel() { return GetFunctionMode() == GAME_MODE && Player_object; }

void Gui_WarpToLevel(int level) {
  Warp_level = level;
  Game_interface_mode = GAME_LEVEL_WARP;
}

bool Gui_PilotLoaded() {
  char name[PILOT_STRING_SIZE];
  Current_pilot.get_name(name);
  return Controller && name[0];
}

bool Gui_PilotPage() {
  if (Gui_PilotLoaded()) {
    return true;
  }
  ImGui::TextDisabled("Load a pilot to see this.");
  return false;
}

static void CreateContext() {
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_NoMouseCursorChange;

  Headers = {GuiSettings_Header(), GuiControls_Header(), GuiMission_Header(), GuiPilot_Header()};
  Open_section.assign(Headers.size(), 0);

  const int theme = CVarGetInteger(CVAR_THEME, Theme);
  if (theme >= 0 && theme < NUM_THEME_COLORS) {
    Theme = (Colors)theme;
  }
  Gui_menu_opacity = CVarGetInteger(CVAR_OPACITY, Gui_menu_opacity);
}

static void StartBackends(SDL_Window *window, void *gl_context) {
  if (!ImGui::GetCurrentContext()) {
    CreateContext();
  }
  Window = window;
  ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
#ifdef D3_GLES
  ImGui_ImplOpenGL3_Init("#version 300 es");
#else
  ImGui_ImplOpenGL3_Init("#version 150");
#endif
}

// HardwareOpenGL.cpp
void rend_FreeOffscreen();

static void StopBackends() {
  if (ImGui::GetCurrentContext() && ImGui::GetIO().BackendRendererUserData) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
  }
  rend_FreeOffscreen();
}

static void DrawFrame() {
  if (!Menu_open || !ImGui::GetIO().BackendRendererUserData) {
    return;
  }
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  DrawMenu();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void RegisterGui() {
  COND_HOOK(OnRendContextCreated, true, StartBackends);
  COND_HOOK(OnRendContextClosing, true, StopBackends);
  COND_HOOK(OnRendFlip, true, DrawFrame);
  COND_VB_SHOULD(VB_SHOW_LEVEL_WARP_DIALOG, true, {
    if (Warp_level) {
      *va_arg(args, int *) = Warp_level;
      Warp_level = 0;
      *should = false;
    }
  });
  COND_VB_SHOULD(VB_CONFIRM_EXIT_GAME, true, {
    if (Quitting) {
      Quitting = false;
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterGui);
