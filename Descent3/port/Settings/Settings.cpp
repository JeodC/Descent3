#include "Settings.h"

#include <filesystem>
#include <fstream>

#include "d3_platform_path.h"
#include "ddio.h"
#include "log.h"

namespace Settings {

static std::filesystem::path Path;
static nlohmann::json Document = nlohmann::json::object();

void Load() {
  Path = ddio_GetPrefPath(D3_PREF_ORG, D3_PREF_APP) / "descent3.cfg.json";
  std::ifstream file(Path);
  if (!file) {
    return;
  }
  nlohmann::json document = nlohmann::json::parse(file, nullptr, false);
  if (document.is_object()) {
    Document = std::move(document);
  } else {
    LOG_ERROR << "Can't read " << Path.u8string() << ", so the settings start over";
  }
}

void Save() {
  std::filesystem::path temp = Path;
  temp += ".tmp";
  {
    std::ofstream file(temp);
    file << Document.dump(2) << '\n';
    if (!file) {
      LOG_ERROR << "Can't write " << temp.u8string();
      return;
    }
  }
  std::error_code error;
  std::filesystem::rename(temp, Path, error);
  if (error) {
    LOG_ERROR << "Can't replace " << Path.u8string() << ": " << error.message();
  }
}

nlohmann::json &Root() { return Document; }

} // namespace Settings
