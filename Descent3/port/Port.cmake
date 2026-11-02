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

FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz
  URL_HASH SHA256=8e1bbc76c71d74fef2fb85db7e7ca8eba13d6a86623c54992b60162db554ffdb
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
target_link_libraries(imgui PUBLIC SDL3::SDL3)
if(D3_GLES)
  find_package(OpenGL REQUIRED COMPONENTS GLES3)
  target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_ES3)
  target_link_libraries(imgui PUBLIC OpenGL::GLES3)
endif()

cmake_path(SET PORT_GENERATED NORMALIZE ${CMAKE_CURRENT_BINARY_DIR}/port)
add_custom_command(
  OUTPUT ${PORT_GENERATED}/montserrat.h
  COMMAND ${CMAKE_COMMAND} -Dinput=Montserrat-Regular.ttf -Doutput=${PORT_GENERATED}/montserrat.h
    -Dname=montserrat_regular_ttf -P ../Embed.cmake
  DEPENDS port/Embed.cmake port/fonts/Montserrat-Regular.ttf
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/port/fonts
)
target_sources(Descent3 PRIVATE ${PORT_GENERATED}/montserrat.h)
target_include_directories(Descent3 PRIVATE ${PORT_GENERATED})

target_link_libraries(Descent3 PRIVATE imgui nlohmann_json::nlohmann_json)
