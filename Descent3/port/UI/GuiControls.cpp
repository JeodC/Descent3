#include "Gui.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>

#include <imgui.h>

#include "controls.h"
#include "CtlCfgElem.h"
#include "ctlconfig.h"
#include "pilot.h"
#include "port/Input/Gamepad.h"
#include "stringtable.h"
#include "UIWidgets.h"
#include "weapon.h"

using namespace UIWidgets;

// ctlconfig.cpp
extern const int N_cfg_joy_elements;

#define CAPTURE_SECONDS 5

static struct {
  int fn = -1;
  ct_type type;
  std::string name;
  double start;
  bool armed;
} Capture;

static std::string FunctionName(int text) {
  std::string name = TXT(text);
  while (!name.empty() && (isspace((unsigned char)name.back()) || name.back() == ':')) {
    name.pop_back();
  }
  return TitleCase(name);
}

static ct_type BindingType(int fn) {
  for (const ct_function &need : Controller_needs) {
    if (need.id == fn) {
      return (need.ctype[0] == ctAxis) ? ctAxis : ctButton;
    }
  }
  return ctButton;
}

static void SetSlot(tCfgDataParts &parts, int slot, uint8_t ctrl, uint8_t bind) {
  if (slot) {
    parts.ctrl_1 = ctrl;
    parts.bind_1 = bind;
  } else {
    parts.ctrl_0 = ctrl;
    parts.bind_0 = bind;
  }
}

static void SetBinding(int fn, ct_type type, uint8_t ctrl, uint8_t bind) {
  ct_type ctype[CTLBINDS_PER_FUNC];
  uint8_t flags[CTLBINDS_PER_FUNC];
  ct_config_data data;
  tCfgDataParts parts;

  for (int i = 0; i < N_cfg_joy_elements; i++) {
    const int other = Cfg_joy_elements[i].fn_id;
    if (other == -1) {
      continue;
    }
    Controller->get_controller_function(other, ctype, &data, flags);
    parse_config_data(&parts, ctype[0], ctype[1], data);
    bool taken = false;
    for (int s = 0; s < CTLBINDS_PER_FUNC; s++) {
      if (ctype[s] == type && (s ? parts.bind_1 : parts.bind_0) == bind && (s ? parts.ctrl_1 : parts.ctrl_0) == ctrl) {
        SetSlot(parts, s, NULL_CONTROLLER, NULL_BINDING);
        taken = true;
      }
    }
    if (taken) {
      Controller->set_controller_function(other, ctype, unify_config_data(&parts), flags);
    }
  }

  Controller->get_controller_function(fn, ctype, &data, flags);
  parse_config_data(&parts, ctype[0], ctype[1], data);
  ctype[0] = type;
  SetSlot(parts, 0, ctrl, bind);
  Controller->set_controller_function(fn, ctype, unify_config_data(&parts), flags);
}

static void ClearBinding(int fn) {
  ct_type ctype[CTLBINDS_PER_FUNC];
  uint8_t flags[CTLBINDS_PER_FUNC];
  ct_config_data data;
  tCfgDataParts parts;

  Controller->get_controller_function(fn, ctype, &data, flags);
  parse_config_data(&parts, ctype[0], ctype[1], data);
  SetSlot(parts, 0, NULL_CONTROLLER, NULL_BINDING);
  Controller->set_controller_function(fn, ctype, unify_config_data(&parts), flags);
}

static void StartCapture(int fn, const std::string &name) {
  Capture.fn = fn;
  Capture.type = BindingType(fn);
  Capture.name = name;
  Capture.start = ImGui::GetTime();
  Capture.armed = false;
}

static void EndCapture() {
  if (Capture.armed) {
    SuspendControls();
  }
  Capture.fn = -1;
}

void GuiControls_Close() {
  if (Capture.fn != -1) {
    EndCapture();
  }
}

static bool PadHeld() {
  for (int key = ImGuiKey_GamepadStart; key <= ImGuiKey_GamepadRStickDown; key++) {
    if (ImGui::IsKeyDown((ImGuiKey)key)) {
      return true;
    }
  }
  return false;
}

static void DrawCapture() {
  if (Capture.fn != -1 && !ImGui::IsPopupOpen("Bind")) {
    ImGui::OpenPopup("Bind");
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  if (!ImGui::BeginPopupModal(
          "Bind", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
    return;
  }

  const double left = CAPTURE_SECONDS - (ImGui::GetTime() - Capture.start);
  bool done = Capture.fn == -1 || left <= 0 || ImGui::IsKeyPressed(ImGuiKey_GamepadStart, false) ||
              ImGui::IsKeyPressed(ImGuiKey_Escape, false);
  if (!done) {
    ImGui::SeparatorText(Capture.name.c_str());
    ImGui::TextUnformatted(Capture.type == ctAxis ? "Move a stick or trigger" : "Press a button");
    ImGui::TextDisabled("START cancels (%d)", (int)std::ceil(left));

    if (!Capture.armed) {
      if (!PadHeld()) {
        ResumeControls();
        Control_poll_flag = false;
        Capture.armed = true;
      }
    } else {
      Controller->poll();
      ct_config_data data = Controller->get_controller_value(Capture.type);
      const int8_t ctrl = CONTROLLER_CTL1_INFO(CONTROLLER_INFO(data));
      if (ctrl != (int8_t)NULL_CONTROLLER) {
        SetBinding(Capture.fn, Capture.type, ctrl, CONTROLLER_CTL1_VALUE(CONTROLLER_VALUE(data)));
        done = true;
      }
    }
  }

  if (done) {
    GuiControls_Close();
    ImGui::CloseCurrentPopup();
  }
  ImGui::EndPopup();
}

static void BindingChip(int fn, const std::string &name, ct_type ctype, uint8_t ctrl, uint8_t bind) {
  const char *text = (ctrl != NULL_CONTROLLER) ? cfg_binding_text(ctype, ctrl, bind) : nullptr;
  char label[64];
  snprintf(label, sizeof(label), "%s##binding", text ? text : "-");

  if (Button(label, Fill, WidgetOptions().Color(text ? Theme : DarkGray))) {
    StartCapture(fn, name);
  }
  if (ImGui::IsItemFocused() &&
      (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft, false) || ImGui::IsKeyPressed(ImGuiKey_Delete, false))) {
    ClearBinding(fn);
  }
}

static void BindingRow(const t_cfg_element &element, bool axis_group) {
  const int fn = element.fn_id;
  const std::string name = FunctionName(element.text);
  ct_type ctype[CTLBINDS_PER_FUNC];
  uint8_t flags[CTLBINDS_PER_FUNC];
  ct_config_data data;
  tCfgDataParts parts;
  Controller->get_controller_function(fn, ctype, &data, flags);
  parse_config_data(&parts, ctype[0], ctype[1], data);

  ImGui::PushID(fn);
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(name.c_str());
  ImGui::TableNextColumn();
  BindingChip(fn, name, ctype[0], parts.ctrl_0, parts.bind_0);

  if (axis_group) {
    ImGui::TableNextColumn();
    if (BindingType(fn) == ctAxis) {
      bool invert = flags[0] & CTFNF_INVERT;
      if (Checkbox("Invert", &invert)) {
        flags[0] = invert ? (flags[0] | CTFNF_INVERT) : (flags[0] & ~CTFNF_INVERT);
        Controller->set_controller_function(fn, ctype, data, flags);
      }
    }
  }
  ImGui::PopID();
}

static void BindingGroups(int first, int last) {
  int group = -1;
  for (int i = 0; i < N_cfg_joy_elements; i++) {
    if (Cfg_joy_elements[i].fn_id != -1 || ++group < first || group > last) {
      continue;
    }

    bool axis_group = false;
    int end = i + 1;
    for (; end < N_cfg_joy_elements && Cfg_joy_elements[end].fn_id != -1; end++) {
      axis_group |= BindingType(Cfg_joy_elements[end].fn_id) == ctAxis;
    }

    ImGui::SeparatorText(FunctionName(Cfg_joy_elements[i].text).c_str());
    ImGui::PushID(group);
    if (ImGui::BeginTable("##bindings", axis_group ? 3 : 2, ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("##name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
      ImGui::TableSetupColumn("##binding", ImGuiTableColumnFlags_WidthStretch, 0.7f);
      if (axis_group) {
        ImGui::TableSetupColumn("##invert", ImGuiTableColumnFlags_WidthFixed);
      }
      for (int row = i + 1; row < end; row++) {
        BindingRow(Cfg_joy_elements[row], axis_group);
      }
      ImGui::EndTable();
    }
    ImGui::PopID();
  }
}

static void BindingsLeft() {
  if (!Gui_PilotPage()) {
    return;
  }
  ImGui::TextWrapped("A binds a control, X clears it. START and SELECT can't be bound.");
  if (Button("Reset to Defaults", Fill)) {
    for (ct_function &need : Controller_needs) {
      if (need.ctype[0] != ctKey) {
        Controller->assign_function(&need);
      }
    }
  }
  BindingGroups(0, 1);
  ImGui::SeparatorText("Markers");
  std::vector<const char *> buttons = {"None"};
  for (int i = JOYPAD_A; i <= JOYPAD_RT; i++) {
    buttons.push_back(Gamepad_ButtonLabel(i));
  }
  CVarCombobox("Drop Marker", "gControls.DropMarkerButton", 0, buttons,
               WidgetOptions().Tooltip("The gamepad button that drops a marker, without a message"));
}

static void BindingsRight() {
  if (Gui_PilotPage()) {
    BindingGroups(2, 3);
  }
  DrawCapture();
}

static void Sensitivity() {
  static const char *const axes[N_JOY_AXIS] = {"Left Stick X",  "Left Stick Y", "Right Stick X",
                                               "Right Stick Y", "Left Trigger", "Right Trigger"};
  if (!Gui_PilotPage()) {
    return;
  }

  ImGui::SeparatorText("Axis Sensitivity");
  for (int i = 0; i < N_JOY_AXIS; i++) {
    float value = Controller->get_axis_sensitivity(ctAxis, i + 1);
    if (SliderFloat(axes[i], &value, 0.0f, JOY_AXIS_SENS_RANGE, 0.1f)) {
      Controller->set_axis_sensitivity(ctAxis, i + 1, value);
      Current_pilot.joy_sensitivity[i] = value;
    }
  }
}

static void WeaponOrder(bool secondary) {
  auto get = secondary ? GetAutoSelectSecondaryWpnIdx : GetAutoSelectPrimaryWpnIdx;
  auto set = secondary ? SetAutoSelectSecondaryWpnIdx : SetAutoSelectPrimaryWpnIdx;
  const int count = secondary ? MAX_SECONDARY_WEAPONS : MAX_PRIMARY_WEAPONS;

  ImGui::SeparatorText(secondary ? "Secondary Weapons" : "Primary Weapons");
  ImGui::PushID(secondary);
  for (int i = 0; i < count; i++) {
    const uint16_t index = get(i);
    if (index == WPNSEL_INVALID) {
      continue;
    }
    ImGui::PushID(i);

    int swap = -1;
    PushStyleButton(Theme);
    ImGui::BeginDisabled(i == 0);
    if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
      swap = i - 1;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(i == count - 1);
    if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
      swap = i + 1;
    }
    ImGui::EndDisabled();
    PopStyleButton();
    ImGui::SameLine();

    bool used = !(index & WPNSEL_SKIP);
    if (Checkbox(TXT(Static_weapon_names_msg[index & ~WPNSEL_SKIP]), &used)) {
      set(i, index ^ WPNSEL_SKIP);
    }
    if (swap != -1) {
      set(i, get(swap));
      set(swap, index);
    }
    ImGui::PopID();
  }
  ImGui::PopID();
}

static void WeaponsPrimary() {
  ImGui::TextWrapped("Picking up a weapon switches to it when it ranks above the one in use. Unchecked weapons are "
                     "never switched to.");
  if (Button("Reset to Defaults", Fill)) {
    for (int i = 0; i < MAX_PRIMARY_WEAPONS; i++) {
      SetAutoSelectPrimaryWpnIdx(i, DefaultPrimarySelectList[i]);
    }
    for (int i = 0; i < MAX_SECONDARY_WEAPONS; i++) {
      SetAutoSelectSecondaryWpnIdx(i, DefaultSecondarySelectList[i]);
    }
  }
  WeaponOrder(false);
}

static void WeaponsSecondary() { WeaponOrder(true); }

tGuiHeader GuiControls_Header() {
  return {"Controls",
          {
              {"Bindings", {BindingsLeft, BindingsRight}},
              {"Sensitivity", {Sensitivity}},
              {"Weapons", {WeaponsPrimary, WeaponsSecondary}},
          }};
}
