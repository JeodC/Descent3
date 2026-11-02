#include <algorithm>
#include <cstring>
#include <string>

#include "crossplat.h"
#include "descent.h"
#include "linux/lnxdatabase.h"
#include "log.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "program.h"
#include "Settings.h"

class JsonDatabase final : public oeLnxAppDatabase {
public:
  ~JsonDatabase() override { Settings::Save(); }

  bool create_record(const char *pathname) override;
  bool lookup_record(const char *pathname) override;
  bool read(const char *label, char *entry, int *entrylen) override;
  bool read(const char *label, void *entry, int wordsize) override;
  bool read(const char *label, bool *entry) override;
  bool write(const char *label, const char *entry, int entrylen) override;
  bool write(const char *label, int entry) override;

private:
  std::string current_record;
};

static nlohmann::json::iterator Find(nlohmann::json &object, const char *name) {
  for (auto it = object.begin(); it != object.end(); ++it) {
    if (!stricmp(it.key().c_str(), name)) {
      return it;
    }
  }
  return object.end();
}

static const nlohmann::json *Value(const std::string &record, const char *label) {
  nlohmann::json &root = Settings::Root();
  auto values = root.find(record);
  if (values == root.end() || !values->is_object()) {
    return nullptr;
  }
  auto it = Find(*values, label);
  return it == values->end() ? nullptr : &*it;
}

static void SetValue(const std::string &record, const char *label, nlohmann::json value) {
  nlohmann::json &values = Settings::Root()[record];
  if (!values.is_object()) {
    values = nlohmann::json::object();
  }
  auto it = Find(values, label);
  if (it == values.end()) {
    values[label] = std::move(value);
  } else {
    *it = std::move(value);
  }
}

bool JsonDatabase::create_record(const char *pathname) {
  nlohmann::json &root = Settings::Root();
  auto it = Find(root, pathname);
  if (it == root.end()) {
    root[pathname] = nlohmann::json::object();
    current_record = pathname;
  } else {
    current_record = it.key();
  }
  return true;
}

bool JsonDatabase::lookup_record(const char *pathname) {
  nlohmann::json &root = Settings::Root();
  auto it = Find(root, pathname);
  if (it == root.end()) {
    return false;
  }
  current_record = it.key();
  return true;
}

bool JsonDatabase::read(const char *label, char *entry, int *entrylen) {
  const nlohmann::json *value = Value(current_record, label);
  if (!value || *entrylen < 1) {
    return false;
  }
  const std::string text = value->is_string() ? value->get<std::string>() : value->dump();
  const int len = std::min((int)text.size(), *entrylen - 1);
  memcpy(entry, text.data(), len);
  entry[len] = '\0';
  *entrylen = len;
  return true;
}

bool JsonDatabase::read(const char *label, void *entry, int wordsize) {
  const nlohmann::json *value = Value(current_record, label);
  if (!value || !(value->is_number() || value->is_boolean())) {
    return false;
  }
  const int data = value->is_boolean() ? value->get<bool>() : value->get<int>();
  switch (wordsize) {
  case 1:
    *((uint8_t *)entry) = (uint8_t)data;
    break;
  case 2:
    *((uint16_t *)entry) = (uint16_t)data;
    break;
  case 4:
    *((uint32_t *)entry) = (uint32_t)data;
    break;
  default:
    LOG_ERROR.printf("Unable to read key %s, unsupported size", label);
    return false;
  }
  return true;
}

bool JsonDatabase::read(const char *label, bool *entry) {
  bool data;
  if (!read(label, &data, sizeof(bool))) {
    return false;
  }
  *entry = data != 0;
  return true;
}

bool JsonDatabase::write(const char *label, const char *entry, int entrylen) {
  SetValue(current_record, label, entry);
  return true;
}

bool JsonDatabase::write(const char *label, int entry) {
  SetValue(current_record, label, entry);
  return true;
}

static void UseJsonDatabase() {
  static JsonDatabase json;
  json.create_record("Descent3");
  for (const char *label : {"local directory", "net directory"}) {
    char path[_MAX_PATH];
    int len = sizeof(path);
    if (Database->read(label, path, &len)) {
      json.write(label, path, len + 1);
    }
  }
  for (const char *label : {"TrackerLogin", "TrackerPassword", "DirectIP"}) {
    char value[2];
    int len = sizeof(value);
    if (!json.read(label, value, &len)) {
      json.write(label, "", 1);
    }
  }
  int bypass_chat;
  if (!json.read_int("BypassChat", &bypass_chat)) {
    json.write("BypassChat", 1);
  }
  Database = &json;
}

static void RegisterJsonDatabase() {
  COND_HOOK(OnDatabaseCreated, true, UseJsonDatabase);
  COND_VB_SHOULD(VB_EXPORT_REGISTRY, true, { *should = false; });
  COND_VB_SHOULD(VB_SAVE_PROGRAM_VERSION, true, {
    nlohmann::json &version = Settings::Root()["Version"];
    version["Major"] = Program_version.major;
    version["Minor"] = Program_version.minor;
    version["Build"] = Program_version.build;
    *should = false;
  });
}

static RegisterPortInitFunc initFunc(RegisterJsonDatabase);
