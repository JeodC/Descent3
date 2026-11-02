#pragma once

#include <functional>
#include <vector>

union SDL_Event;

struct tGuiSection {
  const char *name;
  std::vector<void (*)()> columns;
};
struct tGuiHeader {
  const char *name;
  std::vector<tGuiSection> sections;
  bool (*shown)() = nullptr;
};

extern int Gui_menu_opacity;

bool Gui_SelectTapped();
void Gui_OpenMenu(const char *header = nullptr, const char *section = nullptr);
void Gui_RunAfterClose(std::function<void()> command);
bool Gui_FilterEvent(const SDL_Event *event);
bool Gui_InLevel();
void Gui_WarpToLevel(int level);
bool Gui_PilotLoaded();
bool Gui_PilotPage();

tGuiHeader GuiSettings_Header();
tGuiHeader GuiControls_Header();
tGuiHeader GuiMission_Header();
tGuiHeader GuiPilot_Header();

void GuiControls_Close();
void GuiMission_Close();
void GuiPilot_Close();
void GuiSettings_Close();
