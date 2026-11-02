#pragma once

#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct PortInit {
  static std::unordered_map<std::string, std::vector<std::function<void()>>> &GetAll() {
    static std::unordered_map<std::string, std::vector<std::function<void()>>> portInitFuncs;
    return portInitFuncs;
  }

  static void InitAll() { PortInit::Init("*"); }

  static void Init(const std::string &path) {
    auto &portInitFuncs = PortInit::GetAll();
    for (const auto &initFunc : portInitFuncs[path]) {
      initFunc();
    }
  }
};

/**
 * @brief Register a function to execute on boot and (optionally) in other situations
 *
 * @param initFunc The function to execute
 * @param updatePaths Strings to specify additional situations in which to execute the function
 *
 * ### Examples:
 *
 * #### Execute function `bar` on boot
 *
 * ```cpp
 * static RegisterPortInitFunc foo(bar);
 * ```
 *
 * #### Execute function `bar` on boot and when the CVar `baz` might have changed
 *
 * ```cpp
 * static RegisterPortInitFunc foo(bar, { "baz" });
 * ```
 *
 * ### Additional Information:
 *
 * To get a better sense of when your function will be executed
 * you can look for `PortInit::Init` calls throughout the codebase
 */
struct RegisterPortInitFunc {
  RegisterPortInitFunc(std::function<void()> initFunc, const std::set<std::string> &updatePaths = {}) {
    auto &portInitFuncs = PortInit::GetAll();

    portInitFuncs["*"].push_back(initFunc);

    for (const auto &path : updatePaths) {
      portInitFuncs[path].push_back(initFunc);
    }
  }
};
