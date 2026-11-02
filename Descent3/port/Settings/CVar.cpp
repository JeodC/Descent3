#include "CVar.h"

#include <string>

#include "Settings.h"

static nlohmann::json::json_pointer Pointer(const char *name) {
  std::string path = "/CVars/";
  for (const char *c = name; *c; c++) {
    path += *c == '.' ? '/' : *c;
  }
  return nlohmann::json::json_pointer(path);
}

static const nlohmann::json *Find(const char *name) {
  const nlohmann::json &root = Settings::Root();
  const nlohmann::json::json_pointer pointer = Pointer(name);
  return root.contains(pointer) ? &root.at(pointer) : nullptr;
}

int32_t CVarGetInteger(const char *name, int32_t defaultValue) {
  const nlohmann::json *value = Find(name);
  return value && value->is_number() ? value->get<int32_t>() : defaultValue;
}

void CVarSetInteger(const char *name, int32_t value) { Settings::Root()[Pointer(name)] = value; }
