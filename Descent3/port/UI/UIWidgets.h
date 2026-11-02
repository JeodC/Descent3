#pragma once

#include <string>
#include <vector>

#include <imgui.h>

namespace UIWidgets {

enum Colors {
  Red,
  DarkRed,
  Orange,
  Green,
  DarkGreen,
  LightBlue,
  SkyBlue,
  Blue,
  DarkBlue,
  Indigo,
  Violet,
  Purple,
  Brown,
  Gray,
  DarkGray,
  NUM_THEME_COLORS,
};

extern const char *const ColorNames[NUM_THEME_COLORS];

extern Colors Theme;
extern float Scale;
inline ImVec2 Scaled(float x, float y) { return ImVec2(x * Scale, y * Scale); }

struct WidgetOptions {
  const char *tooltip = nullptr;
  bool disabled = false;
  Colors color = Theme;

  WidgetOptions &Tooltip(const char *tooltip_) {
    tooltip = tooltip_;
    return *this;
  }
  WidgetOptions &Disabled(bool disabled_) {
    disabled = disabled_;
    return *this;
  }
  WidgetOptions &Color(Colors color_) {
    color = color_;
    return *this;
  }
};

inline const ImVec2 Fill(-1.0f, 0.0f);
inline const ImVec2 Inline(0.0f, 0.0f);

void PushStyleButton(Colors color);
void PopStyleButton();
void RenderText(ImVec2 pos, const char *text);
std::string TitleCase(std::string text);

bool Button(const char *label, ImVec2 size = Fill, const WidgetOptions &options = {});
bool Checkbox(const char *label, bool *value, const WidgetOptions &options = {});
bool Combobox(const char *label, int *index, const std::vector<const char *> &items, const WidgetOptions &options = {});
bool SliderInt(const char *label, int *value, int min, int max, int step, const char *format = "%d",
               const WidgetOptions &options = {});
bool SliderFloat(const char *label, float *value, float min, float max, float step, const char *format = "%.1f",
                 const WidgetOptions &options = {});
bool Confirm(const char *id, const char *question, const char *detail, const char *yes);
bool CVarCheckbox(const char *label, const char *cvar, bool defaultValue, const WidgetOptions &options = {});
bool CVarCombobox(const char *label, const char *cvar, int defaultValue, const std::vector<const char *> &items,
                  const WidgetOptions &options = {});

} // namespace UIWidgets
