#include "UIWidgets.h"

#include <algorithm>
#include <cctype>

#include <imgui_internal.h>

#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

namespace UIWidgets {

const char *const ColorNames[NUM_THEME_COLORS] = {
    "Red",       "Dark Red", "Orange", "Green",  "Dark Green", "Light Blue", "Sky Blue",  "Blue",
    "Dark Blue", "Indigo",   "Violet", "Purple", "Brown",      "Gray",       "Dark Gray",
};

static const ImVec4 Color_values[] = {
    {0.55f, 0.0f, 0.0f, 1.0f},  {0.3f, 0.0f, 0.0f, 1.0f},    {0.85f, 0.55f, 0.0f, 1.0f},  {0.0f, 0.55f, 0.0f, 1.0f},
    {0.0f, 0.3f, 0.0f, 1.0f},   {0.0f, 0.24f, 0.8f, 1.0f},   {0.0f, 0.75f, 1.0f, 1.0f},   {0.08f, 0.03f, 0.65f, 1.0f},
    {0.03f, 0.0f, 0.5f, 1.0f},  {0.35f, 0.0f, 0.87f, 1.0f},  {0.5f, 0.0f, 0.9f, 1.0f},    {0.31f, 0.0f, 0.67f, 1.0f},
    {0.37f, 0.18f, 0.0f, 1.0f}, {0.45f, 0.45f, 0.45f, 1.0f}, {0.15f, 0.15f, 0.15f, 1.0f},
};

Colors Theme = LightBlue;
float Scale = 1.0f;

static ImVec4 Alpha(Colors color, float alpha) {
  ImVec4 value = Color_values[color];
  value.w = alpha;
  return value;
}

void PushStyleButton(Colors color) {
  ImGui::PushStyleColor(ImGuiCol_Button, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Alpha(color, 0.8f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Alpha(color, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f * Scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Scaled(10.0f, 8.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 5.0f * Scale);
}

void PopStyleButton() {
  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(4);
}

static void PushStyleCheckbox(Colors color) {
  ImGui::PushStyleColor(ImGuiCol_FrameBg, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Alpha(color, 0.8f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Alpha(color, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
  ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 1.0f, 1.0f, 0.7f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f * Scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Scaled(10.0f, 6.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 5.0f * Scale);
}

static void PopStyleCheckbox() {
  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(5);
}

static void PushStyleCombobox(Colors color) {
  ImGui::PushStyleColor(ImGuiCol_FrameBg, Alpha(color, 0.8f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Alpha(color, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Alpha(color, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_Button, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Alpha(color, 0.8f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Header, Alpha(color, 0.5f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Alpha(color, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, Alpha(color, 0.6f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f * Scale);
  ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 3.0f * Scale);
  ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Scaled(10.0f, 6.0f));
}

static void PopStyleCombobox() {
  ImGui::PopStyleVar(4);
  ImGui::PopStyleColor(9);
}

static void PushStyleSlider(Colors color) {
  ImGui::PushStyleColor(ImGuiCol_FrameBg, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Border, Alpha(color, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
  ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
  ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.0f * Scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f * Scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Scaled(10.0f, 8.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
}

static void PopStyleSlider() {
  ImGui::PopStyleVar(4);
  ImGui::PopStyleColor(6);
}

void RenderText(ImVec2 pos, const char *text) {
  ImGuiContext &g = *GImGui;
  const char *end = ImGui::FindRenderedTextEnd(text);
  if (text != end) {
    g.CurrentWindow->DrawList->AddText(g.Font, g.FontSize, pos, ImGui::GetColorU32(ImGuiCol_Text), text, end);
  }
}

std::string TitleCase(std::string text) {
  for (size_t i = 0; i < text.size(); i++) {
    if (i == 0 || text[i - 1] == ' ') {
      text[i] = toupper((unsigned char)text[i]);
    }
  }
  return text;
}

static void BeginDisabledWidget(bool disabled) {
  ImGui::BeginDisabled(disabled);
  if (disabled) {
    ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
  }
}

static void EndDisabledWidget(bool disabled) {
  if (disabled) {
    ImGui::PopItemFlag();
  }
  ImGui::EndDisabled();
}

static void Tooltip(const char *tooltip) {
  if (tooltip && *tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_AllowWhenDisabled)) {
    ImGui::SetTooltip("%s", tooltip);
  }
}

bool Button(const char *label, ImVec2 size, const WidgetOptions &options) {
  BeginDisabledWidget(options.disabled);
  PushStyleButton(options.color);
  bool pressed = ImGui::Button(label, size);
  PopStyleButton();
  EndDisabledWidget(options.disabled);
  Tooltip(options.tooltip);
  return pressed;
}

bool Checkbox(const char *label, bool *value, const WidgetOptions &options) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems) {
    return false;
  }

  BeginDisabledWidget(options.disabled);
  PushStyleCheckbox(options.color);
  const ImGuiStyle &style = ImGui::GetStyle();
  const ImGuiID id = window->GetID(label);
  const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
  const float square_sz = ImGui::GetFrameHeight();
  const ImVec2 pos = window->DC.CursorPos;
  const ImRect total_bb(pos, pos + ImVec2(square_sz + style.ItemInnerSpacing.x * 2.0f + label_size.x,
                                          label_size.y + style.FramePadding.y * 2.0f));

  ImGui::ItemSize(total_bb, style.FramePadding.y);
  bool pressed = false;
  if (ImGui::ItemAdd(total_bb, id)) {
    bool hovered, held;
    pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) {
      *value = !*value;
      ImGui::MarkItemEdited(id);
    }
    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    ImGui::RenderNavCursor(total_bb, id);
    ImGui::RenderFrame(check_bb.Min, check_bb.Max,
                       ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive
                                          : hovered         ? ImGuiCol_FrameBgHovered
                                                            : ImGuiCol_FrameBg),
                       true, style.FrameRounding);
    if (*value) {
      const float pad = std::max(1.0f, IM_TRUNC(square_sz / 6.0f));
      ImGui::RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), ImGui::GetColorU32(ImGuiCol_CheckMark),
                             square_sz - pad * 2.0f);
    }
    RenderText(pos + ImVec2(square_sz + style.ItemInnerSpacing.x * 2.0f, style.FramePadding.y), label);
  }
  PopStyleCheckbox();
  EndDisabledWidget(options.disabled);
  Tooltip(options.tooltip);
  return pressed;
}

bool Combobox(const char *label, int *index, const std::vector<const char *> &items, const WidgetOptions &options) {
  bool changed = false;
  ImGui::PushID(label);
  ImGui::BeginGroup();
  BeginDisabledWidget(options.disabled);
  PushStyleCombobox(options.color);

  float width = 0.0f;
  for (const char *item : items) {
    width = std::max(width, ImGui::CalcTextSize(item).x);
  }
  width += ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2.0f;

  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(label);
  ImGui::SetNextItemWidth(width);
  const char *preview = (*index >= 0 && *index < (int)items.size()) ? items[*index] : "";
  if (ImGui::BeginCombo("##combo", preview)) {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Scaled(10.0f, 10.0f));
    for (int i = 0; i < (int)items.size(); i++) {
      if (ImGui::Selectable(items[i], i == *index)) {
        *index = i;
        changed = true;
      }
      if (i == *index) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::PopStyleVar();
    ImGui::EndCombo();
  }

  PopStyleCombobox();
  EndDisabledWidget(options.disabled);
  ImGui::EndGroup();
  Tooltip(options.tooltip);
  ImGui::PopID();
  return changed;
}

template <typename T>
static bool Slider(const char *label, T *value, T min, T max, T step, const char *format, const WidgetOptions &options,
                   ImGuiDataType type) {
  bool changed = false;
  ImGui::PushID(label);
  ImGui::BeginGroup();
  BeginDisabledWidget(options.disabled);
  PushStyleSlider(options.color);

  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(label);

  const WidgetOptions step_options = WidgetOptions().Color(options.color);
  if (Button("-", Inline, step_options) && *value > min) {
    *value = std::max(min, *value - step);
    changed = true;
  }
  ImGui::SameLine(0.0f, 3.0f * Scale);
  const float button_w = ImGui::CalcTextSize("+").x + ImGui::GetStyle().FramePadding.x * 2.0f + 3.0f * Scale;
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - button_w);
  if (ImGui::SliderScalar("##slider", type, value, &min, &max, format, ImGuiSliderFlags_AlwaysClamp)) {
    changed = true;
  }
  ImGui::SameLine(0.0f, 3.0f * Scale);
  if (Button("+", Inline, step_options) && *value < max) {
    *value = std::min(max, *value + step);
    changed = true;
  }

  PopStyleSlider();
  EndDisabledWidget(options.disabled);
  ImGui::EndGroup();
  Tooltip(options.tooltip);
  ImGui::PopID();
  return changed;
}

bool SliderInt(const char *label, int *value, int min, int max, int step, const char *format,
               const WidgetOptions &options) {
  return Slider(label, value, min, max, step, format, options, ImGuiDataType_S32);
}

bool SliderFloat(const char *label, float *value, float min, float max, float step, const char *format,
                 const WidgetOptions &options) {
  return Slider(label, value, min, max, step, format, options, ImGuiDataType_Float);
}

bool Confirm(const char *id, const char *question, const char *detail, const char *yes) {
  bool answer = false;
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal(
          id, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
    ImGui::TextUnformatted(question);
    ImGui::TextDisabled("%s", detail);
    if (Button(yes, Inline)) {
      answer = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (Button("Cancel", Inline)) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::SetItemDefaultFocus();
    ImGui::EndPopup();
  }
  return answer;
}

bool CVarCheckbox(const char *label, const char *cvar, bool defaultValue, const WidgetOptions &options) {
  bool value = CVarGetInteger(cvar, defaultValue);
  if (!Checkbox(label, &value, options)) {
    return false;
  }
  CVarSetInteger(cvar, value);
  PortInit::Init(cvar);
  return true;
}

bool CVarCombobox(const char *label, const char *cvar, int defaultValue, const std::vector<const char *> &items,
                  const WidgetOptions &options) {
  int index = CVarGetInteger(cvar, defaultValue);
  if (!Combobox(label, &index, items, options)) {
    return false;
  }
  CVarSetInteger(cvar, index);
  PortInit::Init(cvar);
  return true;
}

} // namespace UIWidgets
