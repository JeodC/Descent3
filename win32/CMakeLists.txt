set(CPPS
  winapp.cpp
  wincon.cpp
  WinController.cpp
  windata.cpp
  wintask.cpp
)

add_subdirectory(dd_grwin32)
add_library(win32 STATIC ${CPPS})
target_include_directories(win32 PRIVATE ${PROJECT_BINARY_DIR}/lib)
target_link_libraries(win32 PRIVATE
  ddio
)
