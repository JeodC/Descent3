include(FetchContent)
file(GLOB_RECURSE PORT_SOURCES CONFIGURE_DEPENDS port/*.cpp port/*.h port/*.hpp)
list(FILTER PORT_SOURCES EXCLUDE REGEX "/port/ModuleBridge/")
target_sources(Descent3 PRIVATE ${PORT_SOURCES})
set_target_properties(Descent3 PROPERTIES ENABLE_EXPORTS ON)
add_library(port_module_bridge STATIC port/ModuleBridge/ModuleBridge.cpp)
target_link_libraries(port_module_bridge PRIVATE ${CMAKE_DL_LIBS})

FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

target_link_libraries(Descent3 PRIVATE nlohmann_json::nlohmann_json)
