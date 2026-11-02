#pragma once

#include <nlohmann/json.hpp>

namespace Settings {

void Load();
void Save();
nlohmann::json &Root();

} // namespace Settings
